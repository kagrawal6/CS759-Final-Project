// src/TimeSeriesArbitrageDetector.cpp
#include "TimeSeriesArbitrageDetector.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>   

TimeSeriesArbitrageDetector::TimeSeriesArbitrageDetector(
    const std::vector<std::tuple<std::string, std::string, std::string, std::string>>& files)
  : currencyFiles(files)
{
    loadAllData();
}

void TimeSeriesArbitrageDetector::loadAllData()
{
    std::map<std::string,
        std::map<std::pair<std::string, std::string>, CurrencyPairData>> dataByTimestamp;

    for (const auto& t : currencyFiles) {
        const auto& [askFile, bidFile, base, quote] = t;
        std::cout << "Loading data for " << base << "/" << quote << "...\n";
        auto pairData = readCurrencyPairCsvs(bidFile, askFile, base, quote);
        std::cout << "  Loaded " << pairData.size() << " data points\n";

        for (auto& dp : pairData) {
            dataByTimestamp[dp.timestamp][{base, quote}] = dp;
        }
    }

    std::cout << "Found " << dataByTimestamp.size() << " unique timestamps in the data\n";

    for (auto& kv : dataByTimestamp) {
        const auto& timestamp = kv.first;
        const auto& pdMap    = kv.second;
        for (auto& pdKv : pdMap) {
            timeSeriesData[timestamp].push_back(pdKv.second);
        }
    }
}

void TimeSeriesArbitrageDetector::analyzeAllTimestamps(bool verbose)
{
    int total = static_cast<int>(timeSeriesData.size()), count = 0;
    std::cout << "\nAnalyzing arbitrage opportunities across " << total << " timestamps...\n";

    for (auto& kv : timeSeriesData) {
        const auto& timestamp  = kv.first;
        const auto& dataPoints = kv.second;
        ++count;
        if (count == 1 || count % 100 == 0 || count == total) {
            std::cout << "Processing timestamp " << count << "/" << total
                      << ": " << timestamp << "\n";
        }

        ForexGraph graph;
        for (auto& dp : dataPoints) {
            graph.addExchangeRate(dp.baseCurrency, dp.quoteCurrency, dp.bid, dp.ask);
        }
        timestampGraphs[timestamp] = graph;

        auto arb = detectArbitrage(graph);
        if (!arb.cycle.empty()) {
            arbitrageOpportunities.push_back({timestamp, arb});
            std::cout << "Arbitrage found at " << timestamp
                      << ": " << std::fixed << std::setprecision(4)
                      << arb.profit << "% profit\n";

            if (verbose) {
                std::cout << "  Cycle: ";
                for (size_t i = 0; i < arb.cycle.size(); ++i) {
                    std::cout << graph.getCurrencyName(arb.cycle[i]);
                    if (i + 1 < arb.cycle.size()) std::cout << " → ";
                }
                std::cout << "\n";
            }
        }
    }

    std::cout << "\nAnalysis complete. Found "
              << arbitrageOpportunities.size()
              << " opportunities across " << total << " timestamps.\n";
}

void TimeSeriesArbitrageDetector::printAllOpportunities()
{
    std::cout << "\n===== ALL ARBITRAGE OPPORTUNITIES =====\n\n";
    if (arbitrageOpportunities.empty()) {
        std::cout << "No arbitrage opportunities found.\n";
        return;
    }

    for (auto& tsArb : arbitrageOpportunities) {
        const auto& graph = timestampGraphs.at(tsArb.timestamp);
        const auto& arb   = tsArb.opportunity;

        std::cout << "Timestamp: " << tsArb.timestamp << "\n"
                  << "Profit: " << std::fixed << std::setprecision(4)
                  << arb.profit << "%\nCycle: ";
        for (size_t i = 0; i < arb.cycle.size(); ++i) {
            std::cout << graph.getCurrencyName(arb.cycle[i]);
            if (i + 1 < arb.cycle.size()) std::cout << " → ";
        }
        std::cout << "\nExecution steps:\n";

        double amount = 1.0;
        for (size_t i = 0; i < arb.cycle.size(); ++i) {
            int from = arb.cycle[i];
            int to   = arb.cycle[(i + 1) % arb.cycle.size()];
            for (auto& e : graph.getEdges()) {
                if (e.src == from && e.dest == to) {
                    double rate = std::exp(-e.weight);
                    double next = amount * rate;
                    std::cout << "  " << amount << " "
                              << graph.getCurrencyName(from)
                              << " → " << next << " "
                              << graph.getCurrencyName(to)
                              << " (rate: " << rate << ")\n";
                    amount = next;
                    break;
                }
            }
        }
        std::cout << "  Final: " << amount << " "
                  << graph.getCurrencyName(arb.cycle[0]) << "\n\n";
    }
}

const std::vector<TimeStampedArbitrage>&
TimeSeriesArbitrageDetector::getOpportunities() const
{
    return arbitrageOpportunities;
}

ForexGraph TimeSeriesArbitrageDetector::getGraphForTimestamp(const std::string& ts) const
{
    auto it = timestampGraphs.find(ts);
    if (it != timestampGraphs.end()) {
        return it->second;
    }
    // fallback
    ForexGraph graph;
    for (auto& dp : timeSeriesData.at(ts)) {
        graph.addExchangeRate(dp.baseCurrency, dp.quoteCurrency, dp.bid, dp.ask);
    }
    return graph;
}
