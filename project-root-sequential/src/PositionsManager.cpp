// src/PositionsManager.cpp
#include "PositionsManager.hpp"
#include "ForexGraph.hpp"

#include <cmath>
#include <iostream>
#include <iomanip>

// Constructor
PositionsManager::PositionsManager(const std::string& base, double capital)
    : baseCurrency(base), initialCapital(capital)
{
    // Start with all capital in the base currency
    positions[baseCurrency] = initialCapital;
    
    // Record initial snapshot
    PortfolioSnapshot initial;
    initial.timestamp    = "INITIAL";
    initial.positions    = positions;
    initial.totalValueUSD = initialCapital;
    portfolioHistory.push_back(initial);
}

// Optional: load USD‐conversion rates
void PositionsManager::setUSDRates(
    const std::map<std::string, std::map<std::string, double>>& rates)
{
    usdRates = rates;
}

// Execute one arbitrage cycle on the current graph
void PositionsManager::executeArbitrageOpportunity(
    const TimeStampedArbitrage& opportunity,
    const ForexGraph& graph)
{
    const std::string ts = opportunity.timestamp;
    const auto& cycle    = opportunity.opportunity.cycle;
    
    std::cout << "Executing arbitrage at " << ts << ":\n";
    
    // 1) Find a currency we already hold
    int startIndex = 0;
    std::string startCurrency;
    double available = 0.0;
    for (size_t i = 0; i < cycle.size(); ++i) {
        auto cur = graph.getCurrencyName(cycle[i]);
        auto it  = positions.find(cur);
        if (it != positions.end() && it->second > 0.0) {
            startIndex     = int(i);
            startCurrency  = cur;
            available      = it->second;
            break;
        }
    }
    
    // 2) If we have none, convert 5% of baseCurrency
    if (available == 0.0) {
        startCurrency = graph.getCurrencyName(cycle[0]);
        std::cout << "  No position in cycle; converting 5% of "
                  << baseCurrency << " → " << startCurrency << "\n";
        double convertAmt = positions[baseCurrency] * 0.05;
        double convRate   = 0.0;
        // find the corresponding edge
        for (auto& e : graph.getEdges()) {
            if (graph.getCurrencyName(e.src) == baseCurrency &&
                graph.getCurrencyName(e.dest) == startCurrency) {
                convRate = std::exp(-e.weight);
                break;
            }
        }
        if (convRate <= 0.0) {
            std::cout << "  Conversion rate not found; skipping.\n";
            return;
        }
        double newAmt = convertAmt * convRate;
        positions[baseCurrency] -= convertAmt;
        positions[startCurrency] += newAmt;
        available = newAmt;
        
        tradeHistory.push_back(
            {ts + " (prep)", baseCurrency, startCurrency,
             convertAmt, newAmt, convRate}
        );
    }
    
    // 3) Use 50% of available in the arbitrage
    double tradeAmt = available * 0.5;
    std::cout << "  Using " << tradeAmt << " " << startCurrency << "\n";
    positions[startCurrency] -= tradeAmt;
    
    // 4) Reorder cycle so it starts from our currency
    std::vector<int> reordered;
    for (int i = 0; i < int(cycle.size()); ++i) {
        reordered.push_back(cycle[(startIndex + i) % cycle.size()]);
    }
    
    // 5) Walk the cycle
    double currAmt = tradeAmt;
    for (size_t i = 0; i < reordered.size(); ++i) {
        int from = reordered[i];
        int to   = reordered[(i+1) % reordered.size()];
        std::string fromC = graph.getCurrencyName(from);
        std::string toC   = graph.getCurrencyName(to);
        
        // find edge
        double rate = 0.0;
        for (auto& e : graph.getEdges()) {
            if (e.src == from && e.dest == to) {
                rate = std::exp(-e.weight);
                break;
            }
        }
        double newAmt = currAmt * rate;
        tradeHistory.push_back(
            {ts, fromC, toC, currAmt, newAmt, rate}
        );
        std::cout << "    " << currAmt << " " << fromC
                  << " → " << newAmt << " " << toC
                  << "  (rate=" << rate << ")\n";
        positions[toC] += newAmt;
        currAmt = newAmt;
    }
    
    // 6) Snapshot the portfolio
    updatePortfolioSnapshot(ts);
}

// Private helper: recalc total USD value and record a snapshot
void PositionsManager::updatePortfolioSnapshot(const std::string& ts)
{
    PortfolioSnapshot snap;
    snap.timestamp = ts;
    snap.positions = positions;
    
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
    snap.totalValueUSD = totalUSD;
    portfolioHistory.push_back(snap);
    
    double profit = totalUSD - initialCapital;
    double pct    = profit / initialCapital * 100.0;
    std::cout << "  Portfolio value: $"
              << std::fixed << std::setprecision(2) << totalUSD
              << "  (Profit: $" << profit << ", " << pct << "%)\n";
}

// Print-only methods are const
void PositionsManager::printCurrentPositions() const
{
    std::cout << "\n===== CURRENT POSITIONS =====\n";
    for (auto& [cur, amt] : positions) {
        std::cout << cur << ": " << std::fixed << std::setprecision(6) << amt << "\n";
    }
}

void PositionsManager::printTradeHistory() const
{
    std::cout << "\n===== TRADE HISTORY =====\n";
    for (auto& t : tradeHistory) {
        std::cout << t.timestamp << ": "
                  << t.fromAmount << " " << t.fromCurrency
                  << " → " << t.toAmount << " " << t.toCurrency
                  << "  (rate=" << t.rate << ")\n";
    }
}

void PositionsManager::printPortfolioHistory() const
{
    std::cout << "\n===== PORTFOLIO HISTORY =====\n";
    for (auto& p : portfolioHistory) {
        std::cout << p.timestamp << ": $"
                  << std::fixed << std::setprecision(2)
                  << p.totalValueUSD << "\n";
    }
}
