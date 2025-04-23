// // src/TimeSeriesArbitrageDetector.cpp
#include "TimeSeriesArbitrageDetector.hpp"
#include "ArbitrageDetector.hpp"
#include <iostream>
#include <iomanip>

TimeSeriesArbitrageDetector::TimeSeriesArbitrageDetector(
    const std::vector<std::tuple<std::string,std::string,std::string,std::string>>& files
) : currencyFiles(files) {
    loadAllData();
}

void TimeSeriesArbitrageDetector::loadAllData() {
    // Group CurrencyPairData by timestamp_ms
        std::map<int64_t,
             std::map<std::pair<std::string,std::string>, CurrencyPairData>> buf;

    for (const auto& tpl : currencyFiles) {
        // Tuple is { askFile, bidFile, base, quote }
        const auto& [askFile, bidFile, baseCurrency, quoteCurrency] = tpl;

        // Pass bidFile first, askFile second:
        auto vec = readCurrencyPairCsvs(bidFile, askFile, baseCurrency, quoteCurrency);
        for (auto& dp : vec) {
            buf[dp.timestamp_ms][{baseCurrency, quoteCurrency}] = dp;
        }
    }

    // Flatten into timeSeriesData
    for (auto& [ts, mp] : buf) {
        auto& vec = timeSeriesData[ts];
        for (auto& [_, dp] : mp) {
            vec.push_back(dp);
        }
    }
}

void TimeSeriesArbitrageDetector::analyzeAllTimestamps(bool verbose) {
    int count = 0, total = (int)timeSeriesData.size();
    for (auto& [ts, dataVec] : timeSeriesData) {
        if (verbose) {
            std::cout << "[" << ++count << "/" << total << "] ts=" << ts << "\n";
        }

        // Build graph for this timestamp
        ForexGraph G;
        for (auto& dp : dataVec) {
            G.addExchangeRate(dp.baseCurrency, dp.quoteCurrency, dp.bid, dp.ask);
        }
        timestampGraphs[ts] = G;

        // Detect arbitrage
        auto arb = detectArbitrage(G);
        if (!arb.cycle.empty()) {
            arbitrageOpportunities.push_back({ts, arb});
        }
    }
}

void TimeSeriesArbitrageDetector::printAllOpportunities() const {
    std::cout << "\n===== ARBITRAGE OPPORTUNITIES =====\n";
    for (const auto& tsArb : arbitrageOpportunities) {
        std::cout << "ts=" << tsArb.timestamp_ms
                  << " profit=" << tsArb.opportunity.profit << "%\n";
    }
}

const std::vector<TimeStampedArbitrage>&
TimeSeriesArbitrageDetector::getOpportunities() const {
    return arbitrageOpportunities;
}

ForexGraph
TimeSeriesArbitrageDetector::getGraphForTimestamp(int64_t timestamp_ms) const {
    auto it = timestampGraphs.find(timestamp_ms);
    if (it != timestampGraphs.end()) {
        return it->second;
    }
    // Fallback: rebuild on the fly
    ForexGraph G;
    for (auto& dp : timeSeriesData.at(timestamp_ms)) {
        G.addExchangeRate(dp.baseCurrency, dp.quoteCurrency, dp.bid, dp.ask);
    }
    return G;
}
