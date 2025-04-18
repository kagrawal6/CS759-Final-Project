// // src/CsvParser.cpp
// #include "CsvParser.hpp"
// #include <fstream>
// #include <sstream>
// #include <iostream>

// // Parse a pair of bid and ask CSV files for a currency pair
// std::vector<CurrencyPairData> readCurrencyPairCsvs(
//     const std::string& bidFilepath,
//     const std::string& askFilepath,
//     const std::string& baseCurrency,
//     const std::string& quoteCurrency) {
        
//     std::vector<CurrencyPairData> data;
//     std::ifstream bidFileStream(bidFilepath);
//     std::ifstream askFileStream(askFilepath);
    
//     if (!bidFileStream.is_open()) {
//         std::cerr << "Error opening bid file: " << bidFilepath << std::endl;
//         return data;
//     }
    
//     if (!askFileStream.is_open()) {
//         std::cerr << "Error opening ask file: " << askFilepath << std::endl;
//         return data;
//     }
    
//     // Skip header lines
//     std::string line;
//     std::getline(bidFileStream, line);
//     std::getline(askFileStream, line);
    
//     // Read data rows
//     std::string bidLine, askLine;
//     while (std::getline(bidFileStream, bidLine) && std::getline(askFileStream, askLine)) {
//         std::istringstream bidSs(bidLine);
//         std::istringstream askSs(askLine);
        
//         CurrencyPairData entry;
//         entry.baseCurrency = baseCurrency;
//         entry.quoteCurrency = quoteCurrency;
        
//         // Parse BID CSV line: Gmt time,Open,High,Low,Close,Volume
//         std::string bidTimestamp, bidOpen, bidHigh, bidLow, bidClose, bidVolume;
        
//         std::getline(bidSs, bidTimestamp, ',');  // Gmt time
//         std::getline(bidSs, bidOpen, ',');       // Open
//         std::getline(bidSs, bidHigh, ',');       // High
//         std::getline(bidSs, bidLow, ',');        // Low
//         std::getline(bidSs, bidClose, ',');      // Close
//         std::getline(bidSs, bidVolume, ',');     // Volume
        
//         // Parse ASK CSV line: Gmt time,Open,High,Low,Close,Volume
//         std::string askTimestamp, askOpen, askHigh, askLow, askClose, askVolume;
        
//         std::getline(askSs, askTimestamp, ',');  // Gmt time
//         std::getline(askSs, askOpen, ',');       // Open
//         std::getline(askSs, askHigh, ',');       // High
//         std::getline(askSs, askLow, ',');        // Low
//         std::getline(askSs, askClose, ',');      // Close
//         std::getline(askSs, askVolume, ',');     // Volume
        
//         // Verify timestamps match
//         if (bidTimestamp != askTimestamp) {
//             std::cerr << "Warning: Timestamp mismatch between bid and ask files at: " 
//                       << bidTimestamp << " vs " << askTimestamp << std::endl;
//             continue;
//         }
        
//         // Store data in our structure
//         entry.timestamp = bidTimestamp;
//         entry.bid = std::stod(bidClose);
//         entry.ask = std::stod(askClose);
        
//         data.push_back(entry);
//     }
    
//     return data;
// }



// // Load multiple currency pairs and build a forex graph
// ForexGraph buildForexGraphFromCsvs(
//     const std::vector<std::tuple<std::string, std::string, std::string, std::string>>& currencyFiles) {
    
//     ForexGraph graph;
    
//     for (const auto& [bidFilepath, askFilepath, baseCurrency, quoteCurrency] : currencyFiles) {
//         std::cout << "Processing " << baseCurrency << "/" << quoteCurrency 
//                   << " from " << bidFilepath << " and " << askFilepath << std::endl;
        
//         auto pairData = readCurrencyPairCsvs(bidFilepath, askFilepath, baseCurrency, quoteCurrency);
        
//         if (pairData.empty()) {
//             std::cerr << "Warning: No data found for " << baseCurrency << "/" << quoteCurrency << std::endl;
//             continue;
//         }
        
//         std::cout << "  Loaded " << pairData.size() << " data points" << std::endl;
        
//         // Use the first timestamp's data for now (can be modified to use specific timestamps)
//         const auto& entry = pairData[0];
//         graph.addExchangeRate(baseCurrency, quoteCurrency, entry.bid, entry.ask);
//     }
    
//     return graph;
// }


#include "CsvParser.hpp"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>
#include <cstring>

// Helper function to map file
static char* mapFile(const std::string& filepath, size_t& size) {
    int fd = open(filepath.c_str(), O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        return nullptr;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("fstat failed");
        close(fd);
        return nullptr;
    }
    size = sb.st_size;

    char* data = static_cast<char*>(mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0));
    if (data == MAP_FAILED) {
        perror("mmap failed");
        close(fd);
        return nullptr;
    }

    close(fd);
    return data;
}

// Helper function to unmap file
static void unmapFile(char* data, size_t size) {
    if (munmap(data, size) == -1) {
        perror("munmap failed");
    }
}

// Modified function using mmap
std::vector<CurrencyPairData> readCurrencyPairCsvs(
    const std::string& bidFilepath,
    const std::string& askFilepath,
    const std::string& baseCurrency,
    const std::string& quoteCurrency) {
    
    std::vector<CurrencyPairData> data;

    size_t bidSize = 0, askSize = 0;
    char* bidData = mapFile(bidFilepath, bidSize);
    char* askData = mapFile(askFilepath, askSize);

    if (!bidData || !askData) {
        std::cerr << "Error mapping files.\n";
        return data;
    }

    char* bidPtr = bidData;
    char* askPtr = askData;
    char* bidEnd = bidData + bidSize;
    char* askEnd = askData + askSize;

    // Skip headers (first line)
    bidPtr = static_cast<char*>(memchr(bidPtr, '\n', bidEnd - bidPtr));
    askPtr = static_cast<char*>(memchr(askPtr, '\n', askEnd - askPtr));

    if (!bidPtr || !askPtr) {
        std::cerr << "Malformed CSV (no header newline).\n";
        unmapFile(bidData, bidSize);
        unmapFile(askData, askSize);
        return data;
    }
    bidPtr++;
    askPtr++;

    while (bidPtr < bidEnd && askPtr < askEnd) {
        char* bidLineEnd = static_cast<char*>(memchr(bidPtr, '\n', bidEnd - bidPtr));
        char* askLineEnd = static_cast<char*>(memchr(askPtr, '\n', askEnd - askPtr));

        if (!bidLineEnd || !askLineEnd) break;

        *bidLineEnd = '\0';
        *askLineEnd = '\0';

        // Tokenize bid line
        char* bidTokens[6];
        char* token = strtok(bidPtr, ",");
        for (int i = 0; i < 6 && token; i++) {
            bidTokens[i] = token;
            token = strtok(nullptr, ",");
        }

        // Tokenize ask line
        char* askTokens[6];
        token = strtok(askPtr, ",");
        for (int i = 0; i < 6 && token; i++) {
            askTokens[i] = token;
            token = strtok(nullptr, ",");
        }

        // Verify timestamps
        if (strcmp(bidTokens[0], askTokens[0]) != 0) {
            std::cerr << "Timestamp mismatch: " << bidTokens[0] << " vs " << askTokens[0] << "\n";
        } else {
            CurrencyPairData entry;
            entry.timestamp = bidTokens[0];
            entry.baseCurrency = baseCurrency;
            entry.quoteCurrency = quoteCurrency;
            entry.bid = std::stod(bidTokens[4]); // Close price
            entry.ask = std::stod(askTokens[4]); // Close price
            data.push_back(entry);
        }

        bidPtr = bidLineEnd + 1;
        askPtr = askLineEnd + 1;
    }

    unmapFile(bidData, bidSize);
    unmapFile(askData, askSize);

    return data;
}


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
        
        const auto& entry = pairData[0];
        graph.addExchangeRate(baseCurrency, quoteCurrency, entry.bid, entry.ask);
    }
    
    return graph;
}
