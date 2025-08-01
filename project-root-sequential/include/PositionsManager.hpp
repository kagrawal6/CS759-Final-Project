
// PositionsManager.hpp
#pragma once

#include <unordered_map>
#include <map>
#include <string>
#include <vector>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include "TimeSeriesArbitrageDetector.hpp"

struct TradeExecution {
    int64_t timestamp_ms;
    std::string fromCurrency;
    std::string toCurrency;
    double fromAmount;
    double toAmount;
    double rate;
};

struct PortfolioSnapshot {
    int64_t timestamp_ms;
    std::unordered_map<std::string, double> positions;
    double totalValueUSD;
};

class PositionsManager {
private:
    std::unordered_map<std::string, double> positions;
    std::vector<TradeExecution> tradeHistory;
    std::vector<PortfolioSnapshot> portfolioHistory;
    std::string baseCurrency;
    double initialCapital;
    std::map<std::string, std::map<std::string, double>> usdRates;

public:
    PositionsManager(const std::string& base = "USD", double capital = 1000000.0);
    void setUSDRates(const std::map<std::string, std::map<std::string, double>>& rates);
    void executeArbitrageOpportunity(const TimeStampedArbitrage& opportunity, const ForexGraph& graph);
    void updatePortfolioSnapshot(int64_t timestamp_ms);
    void printCurrentPositions() const;
    void printTradeHistory() const;
    void printPortfolioHistory() const;
};

