// // src/PositionsManager.cpp
#include "PositionsManager.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>

PositionsManager::PositionsManager(const std::string& base,
                                   double capital)
  : baseCurrency(base), initialCapital(capital)
{
    // Start with all capital in the base currency
    positions[baseCurrency] = initialCapital;

    // Record initial snapshot at timestamp_ms = 0
    portfolioHistory.push_back({0, positions, initialCapital});
}

void PositionsManager::setUSDRates(
    const std::map<int64_t, std::map<std::string,double>>& rates)
{
    usdRates = rates;
}

void PositionsManager::executeArbitrageOpportunity(
    const TimeStampedArbitrage& opp,
    const ForexGraph& graph)
{
    int64_t ts = opp.timestamp_ms;
    const auto& cycle = opp.opportunity.cycle;

    std::cout << "\n--- Executing arbitrage at ts=" << ts << " ---\n";

    // 1) Find a currency we already hold
    std::string startCurrency;
    double available = 0.0;
    for (int idx : cycle) {
        std::string cur = graph.getCurrencyName(idx);
        auto it = positions.find(cur);
        if (it != positions.end() && it->second > 0.0) {
            startCurrency = cur;
            available = it->second;
            break;
        }
    }

    // 2) If none, convert 5% of baseCurrency into first cycle currency
    if (available <= 0.0) {
        startCurrency = graph.getCurrencyName(cycle[0]);
        double convertAmt = positions[baseCurrency] * 0.05;
        double convRate = 0.0;
        // find edge base→startCurrency
        for (const auto& e : graph.getEdges()) {
            if (graph.getCurrencyName(e.src) == baseCurrency &&
                graph.getCurrencyName(e.dest) == startCurrency) {
                convRate = std::exp(-e.weight);
                break;
            }
        }
        if (convRate <= 0.0) {
            std::cout << "  Conversion rate not found; skipping trade.\n";
            return;
        }
        double newAmt = convertAmt * convRate;
        positions[baseCurrency] -= convertAmt;
        positions[startCurrency] += newAmt;
        available = newAmt;
        tradeHistory.push_back({ts, baseCurrency, startCurrency,
                                convertAmt, newAmt, convRate});
        std::cout << "  Prep convert: "
                  << convertAmt << " " << baseCurrency
                  << " → " << newAmt << " " << startCurrency
                  << " (rate=" << convRate << ")\n";
    }

    // 3) Use 50% of available in the arbitrage
    double tradeAmt = available * 0.5;
    positions[startCurrency] -= tradeAmt;
    std::cout << "  Trading " << tradeAmt << " of " << startCurrency << "\n";

    // 4) Reorder cycle so it starts from startCurrency
    std::vector<int> reordered;
    int startIndex = 0;
    for (int i = 0; i < (int)cycle.size(); ++i) {
        if (graph.getCurrencyName(cycle[i]) == startCurrency) {
            startIndex = i;
            break;
        }
    }
    for (int i = 0; i < (int)cycle.size(); ++i) {
        reordered.push_back(cycle[(startIndex + i) % cycle.size()]);
    }

    // 5) Walk the cycle
    double currAmt = tradeAmt;
    for (size_t i = 0; i < reordered.size(); ++i) {
        int from = reordered[i];
        int to   = reordered[(i+1) % reordered.size()];
        std::string fromC = graph.getCurrencyName(from);
        std::string toC   = graph.getCurrencyName(to);
        double rate = 0.0;
        // find edge
        for (const auto& e : graph.getEdges()) {
            if (e.src == from && e.dest == to) {
                rate = std::exp(-e.weight);
                break;
            }
        }
        double newAmt = currAmt * rate;
        tradeHistory.push_back({ts, fromC, toC, currAmt, newAmt, rate});
        positions[toC] += newAmt;
        std::cout << "    " << currAmt << " " << fromC
                  << " → " << newAmt << " " << toC
                  << " (rate=" << rate << ")\n";
        currAmt = newAmt;
    }

    // 6) Snapshot the portfolio
    updatePortfolioSnapshot(ts);
}

void PositionsManager::updatePortfolioSnapshot(int64_t ts) {
    double totalUSD = 0.0;
    for (auto& [cur, amt] : positions) {
        double r = 1.0;
        if (cur != baseCurrency &&
            usdRates.count(ts) &&
            usdRates.at(ts).count(cur)) {
            r = usdRates.at(ts).at(cur);
        }
        totalUSD += amt * r;
    }
    portfolioHistory.push_back({ts, positions, totalUSD});
    std::cout << "  Portfolio value at ts=" << ts << ": $"
              << std::fixed << std::setprecision(2) << totalUSD << "\n";
}

void PositionsManager::printCurrentPositions() const {
    std::cout << "\n===== CURRENT POSITIONS =====\n";
    for (auto& [cur, amt] : positions) {
        std::cout << cur << ": " << std::fixed << std::setprecision(6)
                  << amt << "\n";
    }
}

void PositionsManager::printTradeHistory() const {
    std::cout << "\n===== TRADE HISTORY =====\n";
    for (auto& t : tradeHistory) {
        std::cout << "[" << t.timestamp_ms << "] "
                  << t.fromAmount << " " << t.fromCurrency
                  << " → " << t.toAmount << " " << t.toCurrency
                  << " (rate=" << t.rate << ")\n";
    }
}

void PositionsManager::printPortfolioHistory() const {
    std::cout << "\n===== PORTFOLIO HISTORY =====\n";
    for (auto& p : portfolioHistory) {
        std::cout << "[" << p.timestamp_ms << "] $"
                  << std::fixed << std::setprecision(2)
                  << p.totalValueUSD << "\n";
    }
}
