#pragma once
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include "ForexGraph.hpp"

struct CurrencyPairData {
    std::string timestamp;
    std::string baseCurrency;
    std::string quoteCurrency;
    double bid;
    double ask;
};

// Parse a pair of bid and ask CSV files for a currency pair
std::vector<CurrencyPairData> readCurrencyPairCsvs(
    const std::string& bidFilepath,
    const std::string& askFilepath,
    const std::string& baseCurrency,
    const std::string& quoteCurrency) {
        
    std::vector<CurrencyPairData> data;
    std::ifstream bidFileStream(bidFilepath);
    std::ifstream askFileStream(askFilepath);
    
    if (!bidFileStream.is_open()) {
        std::cerr << "Error opening bid file: " << bidFilepath << std::endl;
        return data;
    }
    
    if (!askFileStream.is_open()) {
        std::cerr << "Error opening ask file: " << askFilepath << std::endl;
        return data;
    }
    
    // Skip header lines
    std::string line;
    std::getline(bidFileStream, line);
    std::getline(askFileStream, line);
    
    // Read data rows
    std::string bidLine, askLine;
    while (std::getline(bidFileStream, bidLine) && std::getline(askFileStream, askLine)) {
        std::istringstream bidSs(bidLine);
        std::istringstream askSs(askLine);
        
        CurrencyPairData entry;
        entry.baseCurrency = baseCurrency;
        entry.quoteCurrency = quoteCurrency;
        
        // Parse BID CSV line: Gmt time,Open,High,Low,Close,Volume
        std::string bidTimestamp, bidOpen, bidHigh, bidLow, bidClose, bidVolume;
        
        std::getline(bidSs, bidTimestamp, ',');  // Gmt time
        std::getline(bidSs, bidOpen, ',');       // Open
        std::getline(bidSs, bidHigh, ',');       // High
        std::getline(bidSs, bidLow, ',');        // Low
        std::getline(bidSs, bidClose, ',');      // Close
        std::getline(bidSs, bidVolume, ',');     // Volume
        
        // Parse ASK CSV line: Gmt time,Open,High,Low,Close,Volume
        std::string askTimestamp, askOpen, askHigh, askLow, askClose, askVolume;
        
        std::getline(askSs, askTimestamp, ',');  // Gmt time
        std::getline(askSs, askOpen, ',');       // Open
        std::getline(askSs, askHigh, ',');       // High
        std::getline(askSs, askLow, ',');        // Low
        std::getline(askSs, askClose, ',');      // Close
        std::getline(askSs, askVolume, ',');     // Volume
        
        // Verify timestamps match
        if (bidTimestamp != askTimestamp) {
            std::cerr << "Warning: Timestamp mismatch between bid and ask files at: " 
                      << bidTimestamp << " vs " << askTimestamp << std::endl;
            continue;
        }
        
        // Store data in our structure
        entry.timestamp = bidTimestamp;
        entry.bid = std::stod(bidClose);
        entry.ask = std::stod(askClose);
        
        data.push_back(entry);
    }
    
    return data;
}

// Load multiple currency pairs and build a forex graph
ForexGraph buildForexGraphFromCsvs(
    const std::vector<std::tuple<std::string, std::string, std::string, std::string>>& currencyFiles) {
    
    ForexGraph graph;
    
    for (const auto& [bidFilepath, askFilepath, baseCurrency, quoteCurrency] : currencyFiles) {
        std::cout << "Processing " << baseCurrency << "/" << quoteCurrency 
                  << " from " << bidFilepath << " and " << askFilepath << std::endl;
        
        auto pairData = readCurrencyPairCsvs(bidFilepath, askFilepath, baseCurrency, quoteCurrency);
        
        if (pairData.empty()) {
            std::cerr << "Warning: No data found for " << baseCurrency << "/" << quoteCurrency << std::endl;
            continue;
        }
        
        std::cout << "  Loaded " << pairData.size() << " data points" << std::endl;
        
        // Use the first timestamp's data for now (can be modified to use specific timestamps)
        const auto& entry = pairData[0];
        graph.addExchangeRate(baseCurrency, quoteCurrency, entry.bid, entry.ask);
    }
    
    return graph;
}