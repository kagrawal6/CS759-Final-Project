#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "ForexGraph.hpp"

struct CurrencyPairData {
    int64_t    timestamp_ms;    // was std::string timestamp
    std::string baseCurrency;
    std::string quoteCurrency;
    double     bid;
    double     ask;
};

std::vector<CurrencyPairData> readCurrencyPairCsvs(
    const std::string& bidFilepath,
    const std::string& askFilepath,
    const std::string& baseCurrency,
    const std::string& quoteCurrency);

ForexGraph buildForexGraphFromCsvs(
    const std::vector<std::tuple<std::string, std::string, std::string, std::string>>& currencyFiles);
