
//PostionsManager.hpp
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include "TimeSeriesArbitrageDetector.hpp"
#include "ForexGraph.hpp"

/// Records a single trade execution.
struct TradeExecution {
    int64_t      timestamp_ms;
    std::string  fromCurrency;
    std::string  toCurrency;
    double       fromAmount;
    double       toAmount;
    double       rate;
};

/// Snapshot of all positions and total USD value at a given time.
struct PortfolioSnapshot {
    int64_t                                    timestamp_ms;
    std::unordered_map<std::string,double>     positions;
    double                                     totalValueUSD;
};

/// Manages positions, executes arbitrage trades, and tracks history.
class PositionsManager {
public:
    /// @param baseCurrency   Code of base/reference currency (e.g. "USD")
    /// @param initialCapital Starting capital in base currency
    PositionsManager(const std::string& baseCurrency = "USD",
                     double initialCapital = 1e6);

    /// (Optional) Provide USD conversion rates per timestamp
    void setUSDRates(
        const std::map<int64_t,
            std::map<std::string, double>>& rates);

    /// Execute one arbitrage opportunity on the given graph
    void executeArbitrageOpportunity(
        const TimeStampedArbitrage& opportunity,
        const ForexGraph& graph);

    /// Print current positions across all currencies
    void printCurrentPositions() const;

    /// Print the history of all trades executed
    void printTradeHistory() const;

    /// Print evolution of portfolio total value in USD
    void printPortfolioHistory() const;

private:
    /// Recompute and record a portfolio snapshot at the given timestamp
    void updatePortfolioSnapshot(int64_t timestamp_ms);

    std::string                                        baseCurrency;
    double                                             initialCapital;
    std::unordered_map<std::string,double>             positions;
    std::vector<TradeExecution>                        tradeHistory;
    std::vector<PortfolioSnapshot>                     portfolioHistory;
    std::map<int64_t, std::map<std::string,double>>    usdRates;
};
