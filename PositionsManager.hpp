#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include "TimeSeriesArbitrageDetector.hpp"

struct TradeExecution {
    std::string timestamp;
    std::string fromCurrency;
    std::string toCurrency;
    double fromAmount;
    double toAmount;
    double rate;
};

struct PortfolioSnapshot {
    std::string timestamp;
    std::unordered_map<std::string, double> positions;
    double totalValueUSD;  // everything converted to USD
};

class PositionsManager {
private:
    std::unordered_map<std::string, double> positions;
    std::vector<TradeExecution> tradeHistory;
    std::vector<PortfolioSnapshot> portfolioHistory;
    std::string baseCurrency;
    double initialCapital;
    
    // For USD conversion rates at each timestamp
    std::map<std::string, std::map<std::string, double>> usdRates;

public:
    PositionsManager(const std::string& base = "USD", double capital = 1000000.0) 
        : baseCurrency(base), initialCapital(capital) {
        // Initialize with base currency position
        positions[baseCurrency] = initialCapital;
        
        // Create initial portfolio snapshot
        PortfolioSnapshot initial;
        initial.timestamp = "INITIAL";
        initial.positions = positions;
        initial.totalValueUSD = initialCapital;
        portfolioHistory.push_back(initial);
    }
    
    void setUSDRates(const std::map<std::string, std::map<std::string, double>>& rates) {
        usdRates = rates;
    }
    
    void executeArbitrageOpportunity(const TimeStampedArbitrage& opportunity, const ForexGraph& graph) {
        std::string timestamp = opportunity.timestamp;
        const auto& cycle = opportunity.opportunity.cycle;
        
        std::cout << "Executing arbitrage at " << timestamp << ":" << std::endl;
        
        // Find a currency in the cycle that we already have
        int startIndex = 0;
        std::string startCurrency;
        double availableAmount = 0.0;
        
        // Try to find a currency in the cycle that we already have
        for (size_t i = 0; i < cycle.size(); i++) {
            std::string currency = graph.getCurrencyName(cycle[i]);
            if (positions.count(currency) && positions[currency] > 0) {
                startIndex = i;
                startCurrency = currency;
                availableAmount = positions[currency];
                break;
            }
        }
        
        // If we couldn't find a currency we have in the cycle, convert from our base currency
        if (availableAmount == 0) {
            startCurrency = graph.getCurrencyName(cycle[0]);
            std::cout << "  No positions in any cycle currency. Converting from base currency." << std::endl;
            
            // Convert 5% of our base currency to the start currency of the cycle
            double conversionAmount = positions[baseCurrency] * 0.05;
            double conversionRate = 0.0;
            
            // Find the exchange rate from base to start currency
            for (const auto& edge : graph.getEdges()) {
                if (graph.getCurrencyName(edge.src) == baseCurrency && 
                    graph.getCurrencyName(edge.dest) == startCurrency) {
                    conversionRate = exp(-edge.weight);
                    break;
                }
            }
            
            if (conversionRate > 0) {
                // Execute the conversion
                std::cout << "  Converting " << std::fixed << std::setprecision(2) << conversionAmount 
                          << " " << baseCurrency << " to " << startCurrency << std::endl;
                
                double newAmount = conversionAmount * conversionRate;
                positions[baseCurrency] -= conversionAmount;
                positions[startCurrency] += newAmount;
                availableAmount = newAmount;
                
                // Record the trade
                TradeExecution trade;
                trade.timestamp = timestamp + " (prep)";
                trade.fromCurrency = baseCurrency;
                trade.toCurrency = startCurrency;
                trade.fromAmount = conversionAmount;
                trade.toAmount = newAmount;
                trade.rate = conversionRate;
                tradeHistory.push_back(trade);
            } else {
                std::cout << "  Couldn't find conversion rate from " << baseCurrency 
                          << " to " << startCurrency << ". Skipping opportunity." << std::endl;
                return;
            }
        }
        
        // Now execute the arbitrage starting from our position
        double tradeAmount = availableAmount * 0.5;  // Use 50% of available
        
        std::cout << "  Using " << std::fixed << std::setprecision(6) << tradeAmount 
                  << " " << startCurrency << " from available " << availableAmount << std::endl;
        
        // Execute each leg of the arbitrage
        double currentAmount = tradeAmount;
        // positions[startCurrency] -= tradeAmount;  // Subtract from starting currency
        
        // Reorder the cycle to start from our currency
        std::vector<int> reorderedCycle;
        for (size_t i = startIndex; i < cycle.size() + startIndex; i++) {
            reorderedCycle.push_back(cycle[i % cycle.size()]);
        }
        
        for (size_t i = 0; i < reorderedCycle.size(); i++) {
            int from = reorderedCycle[i];
            int to = reorderedCycle[(i+1) % reorderedCycle.size()];
            
            std::string fromCurrency = graph.getCurrencyName(from);
            std::string toCurrency = graph.getCurrencyName(to);
            
            // Find the edge for this conversion
            for (const auto& edge : graph.getEdges()) {
                if (edge.src == from && edge.dest == to) {
                    double rate = exp(-edge.weight);
                    double newAmount = currentAmount * rate;
                    
                    // Record the trade
                    TradeExecution trade;
                    trade.timestamp = timestamp;
                    trade.fromCurrency = fromCurrency;
                    trade.toCurrency = toCurrency;
                    trade.fromAmount = currentAmount;
                    trade.toAmount = newAmount;
                    trade.rate = rate;
                    tradeHistory.push_back(trade);
                    
                    std::cout << "  Convert " << std::fixed << std::setprecision(6) << currentAmount 
                              << " " << fromCurrency << " to " 
                              << std::fixed << std::setprecision(6) << newAmount 
                              << " " << toCurrency 
                              << " (rate: " << rate << ")" << std::endl;
                    
                    // Update position
                    positions[fromCurrency] -= currentAmount;
                    positions[toCurrency] += newAmount;
                    currentAmount = newAmount;
                    break;
                }
            }
        }
        
        // Create portfolio snapshot after this arbitrage
        updatePortfolioSnapshot(timestamp);
    }
    
    void updatePortfolioSnapshot(const std::string& timestamp) {
        PortfolioSnapshot snapshot;
        snapshot.timestamp = timestamp;
        snapshot.positions = positions;
        
        // Calculate total value in USD
        double totalUSD = 0.0;
        for (const auto& [currency, amount] : positions) {
            double usdRate = 1.0;  // Default for USD
            
            if (currency != "USD" && usdRates.count(timestamp) && usdRates[timestamp].count(currency)) {
                usdRate = usdRates[timestamp][currency];
            }
            
            totalUSD += amount * usdRate;
        }
        
        snapshot.totalValueUSD = totalUSD;
        portfolioHistory.push_back(snapshot);
        
        double profit = totalUSD - initialCapital;
        double profitPercent = (profit / initialCapital) * 100.0;
        
        std::cout << "  Portfolio value: $" << std::fixed << std::setprecision(2) << totalUSD 
                  << " (Profit: $" << profit << ", " << std::setprecision(4) << profitPercent << "%)" << std::endl;
    }
    
    void printCurrentPositions() {
        std::cout << "\n===== CURRENT POSITIONS =====\n" << std::endl;
        
        for (const auto& [currency, amount] : positions) {
            std::cout << currency << ": " << std::fixed << std::setprecision(6) << amount << std::endl;
        }
    }
    
    void printTradeHistory() {
        std::cout << "\n===== TRADE HISTORY =====\n" << std::endl;
        
        for (const auto& trade : tradeHistory) {
            std::cout << trade.timestamp << ": " 
                      << std::fixed << std::setprecision(6) << trade.fromAmount << " " << trade.fromCurrency 
                      << " → " << std::fixed << std::setprecision(6) << trade.toAmount << " " << trade.toCurrency 
                      << " (rate: " << trade.rate << ")" << std::endl;
        }
    }
    
    void printPortfolioHistory() {
        std::cout << "\n===== PORTFOLIO HISTORY =====\n" << std::endl;
        
        for (const auto& snapshot : portfolioHistory) {
            std::cout << snapshot.timestamp << ": Total Value = $" 
                      << std::fixed << std::setprecision(2) << snapshot.totalValueUSD << std::endl;
            
            // Optionally print positions at each snapshot
            // for (const auto& [currency, amount] : snapshot.positions) {
            //     if (amount > 0) {
            //         std::cout << "  " << currency << ": " << std::fixed << std::setprecision(6) << amount << std::endl;
            //     }
            // }
        }
    }
};