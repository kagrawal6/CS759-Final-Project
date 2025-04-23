

#pragma once
#include <cstdint>
#include <vector>
#include <tuple>
#include <map>
#include <string>
#include "ForexGraph.hpp"
#include "ArbitrageDetector.hpp"
#include "CsvParser.hpp"

/// Holds an arbitrage opportunity at a given timestamp.
struct TimeStampedArbitrage {
    int64_t timestamp_ms;
    ArbitrageOpportunity opportunity;
};

class TimeSeriesArbitrageDetector {
public:
    /// files: vector of (askCsv, bidCsv, baseCurrency, quoteCurrency)
    TimeSeriesArbitrageDetector(
        const std::vector<
          std::tuple<std::string,std::string,std::string,std::string>
        >& files
    );

    /// Scan all timestamps; if verbose, print progress and cycles found.
    void analyzeAllTimestamps(bool verbose = false);

    /// Print all found arbitrage opportunities.
    void printAllOpportunities() const;

    /// Get the list of found opportunities.
    const std::vector<TimeStampedArbitrage>& getOpportunities() const;

    /// Rebuild (or fetch cached) graph for a specific timestamp.
    ForexGraph getGraphForTimestamp(int64_t timestamp_ms) const;

private:
    void loadAllData();

    std::vector<std::tuple<std::string,std::string,std::string,std::string>> currencyFiles;
    std::map<int64_t,std::vector<CurrencyPairData>> timeSeriesData;
    std::map<int64_t,ForexGraph>                    timestampGraphs;
    std::vector<TimeStampedArbitrage>               arbitrageOpportunities;
};

