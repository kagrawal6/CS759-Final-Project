
// TimeSeriesArbitrageDetector.hpp (Parallelized)
#pragma once
#include <cstdint>
#include <vector>
#include <tuple>
#include <map>
#include <string>
#include "ForexGraph.hpp"
#include "ArbitrageDetector.hpp"
#include "CsvParser.hpp"

/// Holds an arbitrage opportunity at a given timestamp
struct TimeStampedArbitrage {
    int64_t timestamp_ms;
    ArbitrageOpportunity opportunity;
};

class TimeSeriesArbitrageDetector {
public:
    /// @param files          list of (askCsv, bidCsv, baseCurrency, quoteCurrency)
    /// @param thread_count   number of threads for parsing & detection
    TimeSeriesArbitrageDetector(
        const std::vector<std::tuple<std::string, std::string, std::string, std::string>>& files,
        int thread_count = 1
    );

    /// Scan all timestamps; if verbose, print progress
    void analyzeAllTimestamps(bool verbose = false);

    /// Print all found arbitrage opportunities
    void printAllOpportunities() const;

    /// Get found opportunities
    const std::vector<TimeStampedArbitrage>& getOpportunities() const;

    /// Get or rebuild graph for a specific timestamp
    ForexGraph getGraphForTimestamp(int64_t timestamp_ms) const;

private:
    /// Load/flatten CSV data into timeSeriesData
    void loadAllData();

    int thread_count_;
    std::vector<std::tuple<std::string, std::string, std::string, std::string>> currencyFiles;
    std::map<int64_t, std::vector<CurrencyPairData>> timeSeriesData;
    std::map<int64_t, ForexGraph> timestampGraphs;
    std::vector<TimeStampedArbitrage> arbitrageOpportunities;
};