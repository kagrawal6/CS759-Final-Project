// //CsvParser.hpp
// #pragma once
// #include <cstdint>
// #include <string>
// #include <vector>
// #include "ForexGraph.hpp"

// struct CurrencyPairData {
//     int64_t    timestamp_ms;    // was std::string timestamp
//     std::string baseCurrency;
//     std::string quoteCurrency;
//     double     bid;
//     double     ask;
// };

// std::vector<CurrencyPairData> readCurrencyPairCsvs(
//     const std::string& bidFilepath,
//     const std::string& askFilepath,
//     const std::string& baseCurrency,
//     const std::string& quoteCurrency);

// ForexGraph buildForexGraphFromCsvs(
//     const std::vector<std::tuple<std::string, std::string, std::string, std::string>>& currencyFiles);
// CsvParser.hpp
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <tuple>
#include "ForexGraph.hpp"

/// A single bid/ask tick record
struct CurrencyPairData {
    int64_t    timestamp_ms;    // milliseconds since midnight
    std::string baseCurrency;
    std::string quoteCurrency;
    double     bid;
    double     ask;
};

/// Read and parse bid/ask CSVs in parallel.
/// @param bidFilepath   path to the bid CSV
/// @param askFilepath   path to the ask CSV
/// @param baseCurrency  e.g. "AUD"
/// @param quoteCurrency e.g. "CAD"
/// @param thread_count  number of OpenMP threads
std::vector<CurrencyPairData> readCurrencyPairCsvs(
    const std::string& bidFilepath,
    const std::string& askFilepath,
    const std::string& baseCurrency,
    const std::string& quoteCurrency,
    int thread_count = 1
);

/// Build a ForexGraph from multiple CSVs in parallel.
/// @param currencyFiles vector of (askFile, bidFile, base, quote)
/// @param thread_count  number of OpenMP threads
ForexGraph buildForexGraphFromCsvs(
    const std::vector<std::tuple<std::string, std::string, std::string, std::string>>& currencyFiles,
    int thread_count = 1
);
