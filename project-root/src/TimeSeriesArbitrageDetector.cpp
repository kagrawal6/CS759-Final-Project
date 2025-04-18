// src/TimeSeriesArbitrageDetector.cpp
#include "TimeSeriesArbitrageDetector.hpp"
#include <iostream>
#include <iomanip>

TimeSeriesArbitrageDetector::TimeSeriesArbitrageDetector(
    const std::vector<std::tuple<std::string, std::string, std::string, std::string>>& files)
    : currencyFiles(files) {
    loadAllData();
}

void TimeSeriesArbitrageDetector::loadAllData() {
    std::map<std::string, std::map<std::pair<std::string, std::string>, CurrencyPairData>> dataByTimestamp;

    for (const auto& [askFile, bidFile, base, quote] : currencyFiles) {
        std::cout << "Loading data for " << base << "/" << quote << "...\n";
        auto pairData = readCurrencyPairCsvs(bidFile, askFile, base, quote);
        std::cout << "  Loaded " << pairData.size() << " data points\n";

        for (const auto& dataPoint : pairData) {
            dataByTimestamp[dataPoint.timestamp][{base, quote}] = dataPoint;
        }
    }

    std::cout << "Found " << dataByTimestamp.size() << " unique timestamps\n";

    for (const auto& [timestamp, pairDataMap] : dataByTimestamp) {
        for (const auto& [currPair, dataPoint] : pairDataMap) {
            timeSeriesData[timestamp].push_back(dataPoint);
        }
    }
}

void TimeSeriesArbitrageDetector::analyzeAllTimestamps(bool verbose) {
    int timestampCount = 0;
    int totalTimestamps = static_cast<int>(timeSeriesData.size());

    std::cout << "\nAnalyzing " << totalTimestamps << " timestamps...\n";
    for (const auto& [timestamp, dataPoints] : timeSeriesData) {
        ++timestampCount;
        if (timestampCount == 1 || timestampCount % 100 == 0 || timestampCount == totalTimestamps) {
            std::cout << "Processing timestamp " << timestampCount
                      << "/" << totalTimestamps << ": " << timestamp << "\n";
        }

        ForexGraph graph;
        for (const auto& data : dataPoints) {
            graph.addExchangeRate(data.baseCurrency, data.quoteCurrency, data.bid, data.ask);
        }
        timestampGraphs[timestamp] = graph;

        auto arb = detectArbitrage(graph, false);
        if (!arb.cycle.empty()) {
            arbitrageOpportunities.push_back({timestamp, arb});
            std::cout << "Arbitrage at " << timestamp << ": "
                      << std::fixed << std::setprecision(4) << arb.profit << "%\n";
            if (verbose) {
                std::cout << "  Cycle: ";
                for (size_t i = 0; i < arb.cycle.size(); ++i) {
                    std::cout << graph.getCurrencyName(arb.cycle[i]);
                    if (i + 1 < arb.cycle.size()) std::cout << " -> ";
                }
                std::cout << " -> " << graph.getCurrencyName(arb.cycle[0]) << "\n";
            }
        }
    }

    std::cout << "\nAnalysis complete. Found "
              << arbitrageOpportunities.size() << " opportunities.\n";
}

void TimeSeriesArbitrageDetector::printAllOpportunities() const {
    std::cout << "\n===== ALL ARBITRAGE OPPORTUNITIES =====\n";
    if (arbitrageOpportunities.empty()) {
        std::cout << "No opportunities.\n";
        return;
    }

    for (const auto& ts : arbitrageOpportunities) {
        const auto& graph = timestampGraphs.at(ts.timestamp);
        const auto& arb = ts.opportunity;

        std::cout << "Timestamp: " << ts.timestamp
                  << " | Profit: " << std::fixed << std::setprecision(4)
                  << arb.profit << "%\n";
        std::cout << "Cycle: ";
        for (size_t i = 0; i < arb.cycle.size(); ++i) {
            std::cout << graph.getCurrencyName(arb.cycle[i]) << " -> ";
        }
        std::cout << graph.getCurrencyName(arb.cycle[0]) << "\n";

        std::cout << "Execution steps:\n";
        double amount = 1.0;
        for (size_t i = 0; i < arb.cycle.size(); ++i) {
            int from = arb.cycle[i];
            int to   = arb.cycle[(i + 1) % arb.cycle.size()];
            for (const auto& e : graph.getEdges()) {
                if (e.src == from && e.dest == to) {
                    double rate = std::exp(-e.weight);
                    double newAmt = amount * rate;
                    std::cout << "  " << amount << " " << graph.getCurrencyName(from)
                              << " -> " << newAmt << " " << graph.getCurrencyName(to)
                              << " (rate: " << rate << ")\n";
                    amount = newAmt;
                    break;
                }
            }
        }
        std::cout << "  Final: " << amount << " "
                  << graph.getCurrencyName(arb.cycle[0]) << "\n\n";
    }
}

const std::vector<TimeStampedArbitrage>& TimeSeriesArbitrageDetector::getOpportunities() const {
    return arbitrageOpportunities;
}

ForexGraph TimeSeriesArbitrageDetector::getGraphForTimestamp(const std::string& timestamp) const {
    auto it = timestampGraphs.find(timestamp);
    if (it != timestampGraphs.end()) {
        return it->second;
    }
    ForexGraph graph;
    for (const auto& data : timeSeriesData.at(timestamp)) {
        graph.addExchangeRate(data.baseCurrency, data.quoteCurrency, data.bid, data.ask);
    }
    return graph;
}
