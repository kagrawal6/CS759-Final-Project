#pragma once
#include <vector>
#include <map>
#include <string>
#include "ForexGraph.hpp"
#include "ArbitrageDetector.hpp"
#include "CsvParser.hpp"
#include <chrono>

struct TimeStampedArbitrage
{
    std::string timestamp;
    ArbitrageOpportunity opportunity;
};

class TimeSeriesArbitrageDetector
{
private:
    std::vector<std::tuple<std::string, std::string, std::string, std::string>> currencyFiles;
    std::chrono::duration<double> totalArbitrageDetectionTime{};
    std::map<std::string, std::vector<CurrencyPairData>> timeSeriesData;
    std::map<std::string, ForexGraph> timestampGraphs;
    std::vector<TimeStampedArbitrage> arbitrageOpportunities;

public:
    TimeSeriesArbitrageDetector(
        const std::vector<std::tuple<std::string, std::string, std::string, std::string>> &files)
        : currencyFiles(files)
    {
        loadAllData();
    }

    void loadAllData()
    {
        // Load all data for all currency pairs
        std::map<std::string, std::map<std::pair<std::string, std::string>, CurrencyPairData>> dataByTimestamp;

        for (const auto &[askFile, bidFile, base, quote] : currencyFiles)
        {
            std::cout << "Loading data for " << base << "/" << quote << "..." << std::endl;

            auto pairData = readCurrencyPairCsvs(bidFile, askFile, base, quote);
            std::cout << "  Loaded " << pairData.size() << " data points" << std::endl;

            for (const auto &dataPoint : pairData)
            {
                dataByTimestamp[dataPoint.timestamp][{base, quote}] = dataPoint;
            }
        }

        // Build list of unique timestamps
        std::cout << "Found " << dataByTimestamp.size() << " unique timestamps in the data" << std::endl;

        // Convert to sorted structure
        for (const auto &[timestamp, pairDataMap] : dataByTimestamp)
        {
            for (const auto &[currencyPair, data] : pairDataMap)
            {
                timeSeriesData[timestamp].push_back(data);
            }
        }
    }

    void analyzeAllTimestamps(bool verbose = false)
    {
        int timestampCount = 0;
        int totalTimestamps = timeSeriesData.size();

        std::cout << "\nAnalyzing arbitrage opportunities across "
                  << totalTimestamps << " timestamps..." << std::endl;

        for (const auto &[timestamp, dataPoints] : timeSeriesData)
        {
            timestampCount++;
            if (timestampCount % 100 == 0 || timestampCount == 1 || timestampCount == totalTimestamps)
            {
                std::cout << "Processing timestamp " << timestampCount << "/" << totalTimestamps
                          << ": " << timestamp << std::endl;
            }

            // Build a graph for this timestamp
            ForexGraph graph;
            for (const auto &data : dataPoints)
            {
                graph.addExchangeRate(data.baseCurrency, data.quoteCurrency, data.bid, data.ask);
            }

            // Store the graph for future reference
            timestampGraphs[timestamp] = graph;

            // Detect arbitrage
            auto start = std::chrono::high_resolution_clock::now();
            auto arb = detectArbitrage(graph, 1);
            auto end = std::chrono::high_resolution_clock::now();
            totalArbitrageDetectionTime += end - start;

            if (!arb.cycle.empty())
            {
                TimeStampedArbitrage tsArb;
                tsArb.timestamp = timestamp;
                tsArb.opportunity = arb;
                arbitrageOpportunities.push_back(tsArb);

                std::cout << "Arbitrage found at " << timestamp << ": "
                          << std::fixed << std::setprecision(4) << arb.profit << "% profit" << std::endl;

                if (verbose)
                {
                    // Print cycle
                    std::cout << "  Cycle: ";
                    for (size_t i = 0; i < arb.cycle.size(); ++i)
                    {
                        std::cout << graph.getCurrencyName(arb.cycle[i]);
                        if (i < arb.cycle.size() - 1)
                        {
                            std::cout << " → ";
                        }
                    }
                    std::cout << " → " << graph.getCurrencyName(arb.cycle[0]) << std::endl;
                }
            }
        }

        std::cout << "\nAnalysis complete." << std::endl;
        std::cout << "Found " << arbitrageOpportunities.size()
                  << " arbitrage opportunities across " << totalTimestamps << " timestamps." << std::endl;
        std::cout << "Total time spent in detectArbitrage: "
                  << totalArbitrageDetectionTime.count() << " seconds";
    }

    void printAllOpportunities()
    {
        std::cout << "\n===== ALL ARBITRAGE OPPORTUNITIES =====\n"
                  << std::endl;

        if (arbitrageOpportunities.empty())
        {
            std::cout << "No arbitrage opportunities found." << std::endl;
            return;
        }

        for (const auto &tsArb : arbitrageOpportunities)
        {
            const auto &graph = timestampGraphs[tsArb.timestamp];
            const auto &arb = tsArb.opportunity;

            std::cout << "Timestamp: " << tsArb.timestamp << std::endl;
            std::cout << "Profit: " << std::fixed << std::setprecision(4) << arb.profit << "%" << std::endl;

            std::cout << "Cycle: ";
            for (size_t i = 0; i < arb.cycle.size(); ++i)
            {
                std::cout << graph.getCurrencyName(arb.cycle[i]);
                if (i < arb.cycle.size() - 1)
                {
                    std::cout << " → ";
                }
            }
            std::cout << " → " << graph.getCurrencyName(arb.cycle[0]) << std::endl;

            // Show execution steps
            std::cout << "Execution steps:" << std::endl;
            double amount = 1.0;
            for (size_t i = 0; i < arb.cycle.size(); i++)
            {
                int from = arb.cycle[i];
                int to = arb.cycle[(i + 1) % arb.cycle.size()];

                // Find the edge
                for (const auto &edge : graph.getEdges())
                {
                    if (edge.src == from && edge.dest == to)
                    {
                        double rate = exp(-edge.weight);
                        double newAmount = amount * rate;

                        std::cout << "  " << std::fixed << std::setprecision(6) << amount
                                  << " " << graph.getCurrencyName(from) << " → "
                                  << std::fixed << std::setprecision(6) << newAmount
                                  << " " << graph.getCurrencyName(to)
                                  << " (rate: " << rate << ")" << std::endl;

                        amount = newAmount;
                        break;
                    }
                }
            }

            std::cout << "  Final: " << std::fixed << std::setprecision(6) << amount
                      << " " << graph.getCurrencyName(arb.cycle[0]) << std::endl;
            std::cout << std::endl;
        }
    }

    // Get all opportunities
    const std::vector<TimeStampedArbitrage> &getOpportunities() const
    {
        return arbitrageOpportunities;
    }

    // Get the graph for a specific timestamp
    ForexGraph getGraphForTimestamp(const std::string &timestamp) const
    {
        if (timestampGraphs.count(timestamp))
        {
            return timestampGraphs.at(timestamp);
        }
        // Fallback to recreating the graph if not found
        ForexGraph graph;
        for (const auto &data : timeSeriesData.at(timestamp))
        {
            graph.addExchangeRate(data.baseCurrency, data.quoteCurrency, data.bid, data.ask);
        }
        return graph;
    }
};
