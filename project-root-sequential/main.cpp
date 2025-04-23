#include <iostream>
#include <iomanip>
#include <tuple>
#include "ForexGraph.hpp"
#include "CsvParser.hpp"
#include "ArbitrageDetector.hpp"
#include "TimeSeriesArbitrageDetector.hpp"
#include "PositionsManager.hpp"
#include "Timer.hpp"

int main()
{
    std::cout << "========== FOREX ARBITRAGE DETECTOR ==========\n"
              << std::endl;

    // Define the CSV files for each currency pair (ask file, bid file, base currency, quote currency)
    std::vector<std::tuple<std::string, std::string, std::string, std::string>> currencyFiles = {
        {"data/ask/USDSGD_ASK.csv", "data/bid/USDSGD_BID.csv", "USD", "SGD"},
        {"data/ask/SGDJPY_ASK.csv", "data/bid/SGDJPY_BID.csv", "SGD", "JPY"},
        {"data/ask/USDJPY_ASK.csv", "data/bid/USDJPY_BID.csv", "USD", "JPY"},
    };

    std::cout << "Currency pairs to analyze:" << std::endl;
    for (const auto &[askFile, bidFile, base, quote] : currencyFiles)
    {
        std::cout << "  - " << base << "/" << quote << " (Ask: " << askFile << ", Bid: " << bidFile << ")" << std::endl;
    }
    std::cout << std::endl;

    // OPTION 1: Single timestamp analysis (existing approach)
    if (true)
    { // Set to true to run this section
        std::cout << "====== SINGLE TIMESTAMP ANALYSIS ======" << std::endl;
        // Build the forex graph from CSV files (first timestamp only)
        std::cout << "Building forex graph..." << std::endl;
        Timer tGraph("Graph construction");
        ForexGraph graph = buildForexGraphFromCsvs(currencyFiles);
        tGraph.stop();

        std::cout << "\nGraph construction complete." << std::endl;
        std::cout << "- Number of currencies: " << graph.getVertexCount() << std::endl;
        std::cout << "- Number of exchange rates: " << graph.getEdges().size() << std::endl;

        // Detect arbitrage opportunities
        std::cout << "\nDetecting arbitrage opportunities..." << std::endl;
        Timer tDetect("Arbitrage detection");
        auto arbitrage = detectArbitrage(graph);
        tDetect.stop();

        // Print results
        if (arbitrage.cycle.empty())
        {
            std::cout << "No arbitrage opportunities found." << std::endl;
        }
        else
        {
            std::cout << "Found arbitrage opportunity with " << std::fixed << std::setprecision(4)
                      << arbitrage.profit << "% profit." << std::endl;
        }
    }

    // OPTION 2: Time series analysis
    std::cout << "\n====== TIME SERIES ANALYSIS ======" << std::endl;
    TimeSeriesArbitrageDetector timeSeriesDetector(currencyFiles);
    timeSeriesDetector.analyzeAllTimestamps(true);
    timeSeriesDetector.printAllOpportunities();

    // OPTION 3: Positions management and trading simulation
    if (!timeSeriesDetector.getOpportunities().empty())
    {
        std::cout << "\n====== TRADING SIMULATION ======" << std::endl;

        // Initialize with 1 million USD
        PositionsManager positionsManager("USD", 1000000.0);

        // Execute the first 5 arbitrage opportunities (or all if less than 5)
        const auto &opportunities = timeSeriesDetector.getOpportunities();
        int numToExecute = std::min(5, (int)opportunities.size());

        for (int i = 0; i < numToExecute; i++)
        {
            const auto &opportunity = opportunities[i];
            // Get the graph for this specific timestamp
            Timer tTrade("Trade execution");
            ForexGraph graph = timeSeriesDetector.getGraphForTimestamp(opportunity.timestamp_ms);
            positionsManager.executeArbitrageOpportunity(opportunity, graph);
            tTrade.stop();
        }
        Timer tExport("Snapshot/CSV export");
        // Print results
        positionsManager.printCurrentPositions();
        positionsManager.printTradeHistory();
        positionsManager.printPortfolioHistory();
        tExport.stop();
    }
    // dump all timing data
    Timer::report();
    return 0;
}