

// TimeSeriesArbitrageDetector.cpp (Parallelized)
#include "TimeSeriesArbitrageDetector.hpp"
#include "Timer.hpp"
#include <iostream>
#include <iomanip>
#include <omp.h>

TimeSeriesArbitrageDetector::TimeSeriesArbitrageDetector(
    const std::vector<std::tuple<std::string, std::string, std::string, std::string>>& files,
    int thread_count
) : currencyFiles(files), thread_count_(thread_count) {
    loadAllData();
}

void TimeSeriesArbitrageDetector::loadAllData() {
    //Timer tLoad("Data loading");

    // Temporary buffer: timestamp -> (base,quote) -> CurrencyPairData
    std::map<int64_t, std::map<std::pair<std::string, std::string>, CurrencyPairData>> buf;

    int N = (int)currencyFiles.size();
    #pragma omp parallel for num_threads(thread_count_) schedule(dynamic)
    for (int i = 0; i < N; ++i) {
        auto& tpl = currencyFiles[i];
        const auto& [askFile, bidFile, base, quote] = tpl;
        auto vec = readCurrencyPairCsvs(bidFile, askFile, base, quote, thread_count_);
        for (auto& dp : vec) {
            #pragma omp critical
            buf[dp.timestamp_ms][{base, quote}] = dp;
        }
    }

    // Flatten into timeSeriesData
    for (auto& [ts, mp] : buf) {
        auto& vec = timeSeriesData[ts];
        vec.reserve(mp.size());
        for (auto& [_, dp] : mp) {
            vec.push_back(dp);
        }
    }

    //tLoad.stop();
}

void TimeSeriesArbitrageDetector::analyzeAllTimestamps(bool verbose) {
    //Timer tAnalyze("Time-series detection");

    // Extract timestamps to vector for parallel loop
    std::vector<int64_t> timestamps;
    timestamps.reserve(timeSeriesData.size());
    for (auto& kv : timeSeriesData) timestamps.push_back(kv.first);

    int M = (int)timestamps.size();
    #pragma omp parallel for num_threads(thread_count_) schedule(dynamic)
    for (int idx = 0; idx < M; ++idx) {
        int64_t ts = timestamps[idx];
        auto dataVec = timeSeriesData[ts];

        if (verbose) {
            #pragma omp critical
            std::cout << "[" << idx+1 << "/" << M << "] ts=" << ts << "\n";
        }

        // Build graph
        ForexGraph G;
        for (auto& dp : dataVec) {
            G.addExchangeRate(dp.baseCurrency, dp.quoteCurrency, dp.bid, dp.ask);
        }

        // Detect arbitrage
        auto arb = detectArbitrage(G, thread_count_);
        if (!arb.cycle.empty()) {
            #pragma omp critical
            {
                timestampGraphs[ts] = G;
                arbitrageOpportunities.push_back({ts, arb});
            }
        }
    }

    //tAnalyze.stop();
}

void TimeSeriesArbitrageDetector::printAllOpportunities() const {
    std::cout << "\n===== ARBITRAGE OPPORTUNITIES =====\n";
    for (const auto& tsArb : arbitrageOpportunities) {
        std::cout << "ts=" << tsArb.timestamp_ms
                  << " profit=" << std::fixed << std::setprecision(4)
                  << tsArb.opportunity.profit << "%\n";
    }
}

const std::vector<TimeStampedArbitrage>&
TimeSeriesArbitrageDetector::getOpportunities() const {
    return arbitrageOpportunities;
}

ForexGraph TimeSeriesArbitrageDetector::getGraphForTimestamp(int64_t timestamp_ms) const {
    auto it = timestampGraphs.find(timestamp_ms);
    if (it != timestampGraphs.end()) return it->second;
    // Fallback
    ForexGraph G;
    for (auto& dp : timeSeriesData.at(timestamp_ms)) {
        G.addExchangeRate(dp.baseCurrency, dp.quoteCurrency, dp.bid, dp.ask);
    }
    return G;
}