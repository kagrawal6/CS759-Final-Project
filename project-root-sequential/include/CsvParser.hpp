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

std::vector<CurrencyPairData> readCurrencyPairCsvs(
    const std::string& bidFilepath,
    const std::string& askFilepath,
    const std::string& baseCurrency,
    const std::string& quoteCurrency);

ForexGraph buildForexGraphFromCsvs(
    const std::vector<std::tuple<std::string, std::string, std::string, std::string>>& currencyFiles);
