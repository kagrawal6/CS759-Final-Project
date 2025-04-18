// include/TimeSeriesArbitrageDetector.hpp
#pragma once

#include <vector>
#include <tuple>
#include <map>
#include <string>
#include "ForexGraph.hpp"
#include "ArbitrageDetector.hpp"
#include "CsvParser.hpp"

/// Holds a timestamp along with the detected arbitrage opportunity at that time.
struct TimeStampedArbitrage {
    std::string timestamp;
    ArbitrageOpportunity opportunity;
};

/// Analyzes a series of FX snapshots over time to find arbitrage cycles.
class TimeSeriesArbitrageDetector {
public:
    /// @param files
    ///   A vector of tuples: (askCsvPath, bidCsvPath, baseCurrency, quoteCurrency)
    TimeSeriesArbitrageDetector(
        const std::vector<std::tuple<std::string, std::string, std::string, std::string>>& files);

    /// Runs arbitrage detection on each timestamp in the loaded data.
    /// @param verbose  If true, prints detailed cycle information as it finds opportunities.
    void analyzeAllTimestamps(bool verbose = false);

    /// Prints all opportunities found across the entire time series.
    void printAllOpportunities();

    /// Returns the list of all detected opportunities with their timestamps.
    const std::vector<TimeStampedArbitrage>& getOpportunities() const;

    /// Retrieves the FX graph built for a specific timestamp.
    /// If the graph was cached during analysis, returns that; otherwise rebuilds it.
    ForexGraph getGraphForTimestamp(const std::string& timestamp) const;

private:
    /// Loads and groups all CSV data by timestamp.
    void loadAllData();

    std::vector<std::tuple<std::string, std::string, std::string, std::string>> currencyFiles;
    std::map<std::string, std::vector<CurrencyPairData>> timeSeriesData;
    std::map<std::string, ForexGraph>                     timestampGraphs;
    std::vector<TimeStampedArbitrage>                     arbitrageOpportunities;
};
