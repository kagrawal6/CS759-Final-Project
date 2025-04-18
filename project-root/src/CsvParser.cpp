// src/CsvParser.cpp
#include "CsvParser.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

std::vector<CurrencyPairData>
readCurrencyPairCsvs(const std::string& bidFile,
                     const std::string& askFile,
                     const std::string& base,
                     const std::string& quote)
{
    std::vector<CurrencyPairData> data;
    std::ifstream bidFs(bidFile), askFs(askFile);
    if (!bidFs.is_open()) {
        std::cerr << "Error opening bid file: " << bidFile << "\n";
        return data;
    }
    if (!askFs.is_open()) {
        std::cerr << "Error opening ask file: " << askFile << "\n";
        return data;
    }

    // Skip headers
    std::string line;
    std::getline(bidFs, line);
    std::getline(askFs, line);

    // Read row‑by‑row
    while (std::getline(bidFs, line) && std::getline(askFs, line)) {
        std::istringstream bss(line), ass(line);
        CurrencyPairData e;
        e.baseCurrency  = base;
        e.quoteCurrency = quote;

        // --- BID side ---
        // 0) Timestamp
        std::getline(bss, e.timestamp, ',');
        // 1) Open, 2) High, 3) Low (skip)
        for (int i = 0; i < 3; ++i) std::getline(bss, line, ',');
        // 4) Close → bid
        std::getline(bss, line, ',');
        e.bid = std::stod(line);
        // 5) Volume (skip)
        std::getline(bss, line, ',');

        // --- ASK side ---
        std::string askTs;
        std::getline(ass, askTs, ',');
        if (askTs != e.timestamp) {
            std::cerr << "Timestamp mismatch: " << e.timestamp
                      << " vs " << askTs << "\n";
            continue;
        }
        for (int i = 0; i < 3; ++i) std::getline(ass, line, ',');
        std::getline(ass, line, ',');
        e.ask = std::stod(line);
        std::getline(ass, line, ',');

        data.push_back(std::move(e));
    }

    return data;
}

ForexGraph buildForexGraphFromCsvs(
    const std::vector<std::tuple<std::string,std::string,std::string,std::string>>& files)
{
    ForexGraph g;
    for (auto& t : files) {
        auto& [askF, bidF, base, quote] = t;
        std::cout << "Processing " << base << "/" << quote << "\n";
        auto vec = readCurrencyPairCsvs(bidF, askF, base, quote);
        if (vec.empty()) {
            std::cerr << "  No data for " << base << "/" << quote << "\n";
            continue;
        }
        auto& fst = vec.front();
        g.addExchangeRate(fst.baseCurrency,
                          fst.quoteCurrency,
                          fst.bid,
                          fst.ask);
    }
    return g;
}
