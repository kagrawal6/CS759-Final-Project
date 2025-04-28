// // // src/PositionsManager.cpp

#include "PositionsManager.hpp"
#include <cmath>
#include <algorithm>

PositionsManager::PositionsManager(const std::string& base, double capital)
    : baseCurrency(base), initialCapital(capital) {
    positions[baseCurrency] = initialCapital;
    PortfolioSnapshot initial{0, positions, initialCapital};
    portfolioHistory.push_back(initial);
}

void PositionsManager::setUSDRates(const std::map<std::string, std::map<std::string, double>>& rates) {
    usdRates = rates;
}

void PositionsManager::executeArbitrageOpportunity(const TimeStampedArbitrage& opportunity, const ForexGraph& graph) {
    int64_t ts = opportunity.timestamp_ms;
    const auto& cycle = opportunity.opportunity.cycle;
    std::cout << "Executing arbitrage at ms " << ts << ":" << std::endl;

    int startIndex = 0;
    std::string startCurrency;
    double availableAmount = 0.0;
    for (size_t i = 0; i < cycle.size(); ++i) {
        std::string currency = graph.getCurrencyName(cycle[i]);
        if (positions.count(currency) && positions[currency] > 0) {
            startIndex = static_cast<int>(i);
            startCurrency = currency;
            availableAmount = positions[currency];
            break;
        }
    }

    if (availableAmount == 0.0) {
        startCurrency = graph.getCurrencyName(cycle[0]);
        std::cout << "  No positions in any cycle currency. Converting from base." << std::endl;
        double conversionAmount = positions[baseCurrency] * 0.05;
        double conversionRate = 0.0;
        for (const auto& edge : graph.getEdges()) {
            if (graph.getCurrencyName(edge.src) == baseCurrency &&
                graph.getCurrencyName(edge.dest) == startCurrency) {
                conversionRate = std::exp(-edge.weight);
                break;
            }
        }
        if (conversionRate > 0.0) {
            std::cout << "  Converting " << std::fixed << std::setprecision(2) << conversionAmount
                      << " " << baseCurrency << " to " << startCurrency << std::endl;
            double newAmount = conversionAmount * conversionRate;
            positions[baseCurrency] -= conversionAmount;
            positions[startCurrency] += newAmount;
            availableAmount = newAmount;
            tradeHistory.push_back({ts, baseCurrency, startCurrency, conversionAmount, newAmount, conversionRate});
        } else {
            std::cout << "  Couldn't find conversion rate from " << baseCurrency
                      << " to " << startCurrency << ". Skipping." << std::endl;
            return;
        }
    }

    double tradeAmount = availableAmount * 0.5;
    std::cout << "  Using " << std::fixed << std::setprecision(6) << tradeAmount
              << " " << startCurrency << " (available " << availableAmount << ")" << std::endl;
    double currentAmount = tradeAmount;
    std::vector<int> reordered;
    for (size_t i = startIndex; i < cycle.size() + startIndex; ++i)
        reordered.push_back(cycle[i % cycle.size()]);

    for (size_t i = 0; i < reordered.size(); ++i) {
        int from = reordered[i];
        int to = reordered[(i+1) % reordered.size()];
        std::string fromCurr = graph.getCurrencyName(from);
        std::string toCurr = graph.getCurrencyName(to);
        for (const auto& edge : graph.getEdges()) {
            if (edge.src == from && edge.dest == to) {
                double rate = std::exp(-edge.weight);
                double newAmount = currentAmount * rate;
                tradeHistory.push_back({ts, fromCurr, toCurr, currentAmount, newAmount, rate});
                std::cout << "  Convert " << std::fixed << std::setprecision(6)
                          << currentAmount << " " << fromCurr << " to " << newAmount
                          << " " << toCurr << " (rate: " << rate << ")" << std::endl;
                positions[fromCurr] -= currentAmount;
                positions[toCurr] += newAmount;
                currentAmount = newAmount;
                break;
            }
        }
    }

    updatePortfolioSnapshot(ts);
}

void PositionsManager::updatePortfolioSnapshot(int64_t timestamp_ms) {
    PortfolioSnapshot snapshot{timestamp_ms, positions, 0.0};
    double totalUSD = 0.0;
    for (const auto& [currency, amount] : positions) {
        double usdRate = 1.0;
        if (currency != "USD" && usdRates.count(std::to_string(timestamp_ms)) && usdRates.at(std::to_string(timestamp_ms)).count(currency)) {
            usdRate = usdRates.at(std::to_string(timestamp_ms)).at(currency);
        }
        totalUSD += amount * usdRate;
    }
    snapshot.totalValueUSD = totalUSD;
    portfolioHistory.push_back(snapshot);
    double profit = totalUSD - initialCapital;
    double profitPct = (profit / initialCapital) * 100.0;
    std::cout << "  Portfolio value at ms " << timestamp_ms << ": $"
              << std::fixed << std::setprecision(2) << totalUSD
              << " (Profit: $" << profit << ", " << std::setprecision(4)
              << profitPct << "% )" << std::endl;
}

void PositionsManager::printCurrentPositions() const {
    std::cout << "\n===== CURRENT POSITIONS =====\n";
    for (const auto& [currency, amount] : positions) {
        std::cout << currency << ": " << std::fixed << std::setprecision(6)
                  << amount << std::endl;
    }
}

void PositionsManager::printTradeHistory() const {
    std::cout << "\n===== TRADE HISTORY =====\n";
    for (const auto& t : tradeHistory) {
        std::cout << t.timestamp_ms << ": " << std::fixed << std::setprecision(6)
                  << t.fromAmount << " " << t.fromCurrency << " → "
                  << t.toAmount << " " << t.toCurrency << " (rate: "
                  << t.rate << ")" << std::endl;
    }
}

void PositionsManager::printPortfolioHistory() const {
    std::cout << "\n===== PORTFOLIO HISTORY =====\n";
    for (const auto& s : portfolioHistory) {
        std::cout << s.timestamp_ms << ": Total = $" << std::fixed
                  << std::setprecision(2) << s.totalValueUSD << std::endl;
    }
}
