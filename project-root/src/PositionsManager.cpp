// src/PositionsManager.cpp
#include "PositionsManager.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>

PositionsManager::PositionsManager(const std::string& base, double capital)
    : baseCurrency(base), initialCapital(capital) {
    positions[baseCurrency] = initialCapital;
    PortfolioSnapshot initial{ "INITIAL", positions, initialCapital };
    portfolioHistory.push_back(initial);
}

void PositionsManager::setUSDRates(const std::map<std::string, std::map<std::string, double>>& rates) {
    usdRates = rates;
}

void PositionsManager::executeArbitrageOpportunity(const TimeStampedArbitrage& opportunity, const ForexGraph& graph) {
    std::string timestamp = opportunity.timestamp;
    const auto& cycle = opportunity.opportunity.cycle;

    std::cout << "Executing arbitrage at " << timestamp << ":\n";

    // Find starting currency and available amount
    int startIndex = 0;
    std::string startCurrency;
    double availableAmount = 0.0;
    for (size_t i = 0; i < cycle.size(); ++i) {
        std::string c = graph.getCurrencyName(cycle[i]);
        if (positions.count(c) && positions.at(c) > 0.0) {
            startIndex = i;
            startCurrency = c;
            availableAmount = positions[c];
            break;
        }
    }

    // If none, convert from base
    if (availableAmount == 0.0) {
        startCurrency = graph.getCurrencyName(cycle[0]);
        std::cout << "  No positions in cycle currencies, converting from base.\n";
        double conversionAmount = positions[baseCurrency] * 0.05;
        double conversionRate = 0.0;
        for (const auto& e : graph.getEdges()) {
            if (graph.getCurrencyName(e.src) == baseCurrency &&
                graph.getCurrencyName(e.dest) == startCurrency) {
                conversionRate = std::exp(-e.weight);
                break;
            }
        }
        if (conversionRate > 0.0) {
            std::cout << "  Converting " << conversionAmount << " " << baseCurrency << " to " << startCurrency << "\n";
            double newAmt = conversionAmount * conversionRate;
            positions[baseCurrency] -= conversionAmount;
            positions[startCurrency] += newAmt;
            availableAmount = newAmt;
            tradeHistory.push_back({timestamp + " (prep)", baseCurrency, startCurrency,
                                    conversionAmount, newAmt, conversionRate});
        } else {
            std::cout << "  Conversion rate not found, skipping.\n";
            return;
        }
    }

    double tradeAmount = availableAmount * 0.5;
    std::cout << "  Using " << tradeAmount << " " << startCurrency << "\n";
    positions[startCurrency] -= tradeAmount;

    std::vector<int> reordered;
    for (size_t i = startIndex; i < cycle.size() + startIndex; ++i)
        reordered.push_back(cycle[i % cycle.size()]);

    double current = tradeAmount;
    for (size_t i = 0; i < reordered.size(); ++i) {
        int from = reordered[i];
        int to   = reordered[(i+1)%reordered.size()];
        std::string fromC = graph.getCurrencyName(from);
        std::string toC   = graph.getCurrencyName(to);
        for (const auto& e : graph.getEdges()) {
            if (e.src == from && e.dest == to) {
                double rate = std::exp(-e.weight);
                double newAmt = current * rate;
                tradeHistory.push_back({timestamp, fromC, toC, current, newAmt, rate});
                std::cout << "  " << current << " " << fromC
                          << " -> " << newAmt << " " << toC
                          << " (rate: " << rate << ")\n";
                positions[toC] += newAmt;
                current = newAmt;
                break;
            }
        }
    }

    updatePortfolioSnapshot(timestamp);
}

void PositionsManager::updatePortfolioSnapshot(const std::string& timestamp) {
    PortfolioSnapshot snap;
    snap.timestamp = timestamp;
    snap.positions = positions;
    double total = 0.0;
    for (auto& [cur, amt] : positions) {
        double rate = (cur == "USD" ? 1.0 :
            usdRates.count(timestamp) && usdRates.at(timestamp).count(cur) ?
            usdRates.at(timestamp).at(cur) : 1.0);
        total += amt * rate;
    }
    snap.totalValueUSD = total;
    portfolioHistory.push_back(snap);
    double profit = total - initialCapital;
    double pct = profit / initialCapital * 100.0;
    std::cout << "  Portfolio: $" << total << " (Profit: $" << profit << ", " << pct << "%)\n";
}

void PositionsManager::printCurrentPositions() const {
    std::cout << "\n===== CURRENT POSITIONS =====\n";
    for (auto& [cur, amt] : positions) {
        std::cout << cur << ": " << amt << "\n";
    }
}

void PositionsManager::printTradeHistory() const {
    std::cout << "\n===== TRADE HISTORY =====\n";
    for (auto& t : tradeHistory) {
        std::cout << t.timestamp << ": " << t.fromAmount << " " << t.fromCurrency
                  << " -> " << t.toAmount << " " << t.toCurrency
                  << " (rate: " << t.rate << ")\n";
    }
}

void PositionsManager::printPortfolioHistory() const {
    std::cout << "\n===== PORTFOLIO HISTORY =====\n";
    for (auto& p : portfolioHistory) {
        std::cout << p.timestamp << ": $" << p.totalValueUSD << "\n";
    }
}
