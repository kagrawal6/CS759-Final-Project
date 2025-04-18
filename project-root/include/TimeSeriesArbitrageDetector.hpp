// include/TimeSeriesArbitrageDetector.hpp
#pragma once

#include <vector>
#include <tuple>
#include <map>
#include <string>
#include "ForexGraph.hpp"
#include "ArbitrageDetector.hpp"
#include "CsvParser.hpp"

struct TimeStampedArbitrage {
    std::string timestamp;
    ArbitrageOpportunity opportunity;
};

class TimeSeriesArbitrageDetector {
private:
    std::vector<std::tuple<std::string, std::string, std::string, std::string>> currencyFiles;
    std::map<std::string, std::vector<CurrencyPairData>> timeSeriesData;
    std::map<std::string, ForexGraph> timestampGraphs;
    std::vector<TimeStampedArbitrage> arbitrageOpportunities;

    void loadAllData();

public:
    TimeSeriesArbitrageDetector(const std::vector<std::tuple<std::string, std::string, std::string, std::string>>& files);
    void analyzeAllTimestamps(bool verbose = false);
    void printAllOpportunities() const;
    const std::vector<TimeStampedArbitrage>& getOpportunities() const;
    ForexGraph getGraphForTimestamp(const std::string& timestamp) const;
};
