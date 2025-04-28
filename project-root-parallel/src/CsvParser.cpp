

// //think aobut adding  Binary “tick dump to increase performance 


// #include "CsvParser.hpp"
// #include <sys/mman.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <unistd.h>
// #include <cstring>
// #include <cstdlib>
// #include <iostream>
// #include "Timer.hpp"

// // parse "DD.MM.YYYY hh:mm:ss.SSS" → ms since midnight
// static int64_t parseTimestampMs(const char *s) {
//     int D, M, Y, h, m, sec, ms;
//     std::sscanf(s, "%d.%d.%d %d:%d:%d.%d", &D, &M, &Y, &h, &m, &sec, &ms);
//     return int64_t(h)*3600000 + int64_t(m)*60000 + int64_t(sec)*1000 + ms;
// }

// // estimate lines for reserve()
// static size_t approximateLineCount(size_t sz, size_t avg=100) {
//     return (sz + avg - 1)/avg;
// }

// // mmap helper
// static char* mapFile(const std::string& path, size_t& sz) {
//     int fd = open(path.c_str(), O_RDONLY);
//     if (fd<0) { perror("open"); return nullptr; }
//     struct stat st; fstat(fd, &st); sz = st.st_size;
//    // char* data = (char*)mmap(nullptr, sz, PROT_READ, MAP_PRIVATE, fd, 0);
// 	 char* data = static_cast<char*>(mmap(nullptr,sz,PROT_READ | PROT_WRITE,MAP_PRIVATE,fd, 0));
//         if (data == MAP_FAILED) {
//         perror("mmap failed");
//         close(fd);
//         return nullptr;
//     }

//     close(fd);
//     return data;
// }
// static void unmapFile(char* data, size_t sz) {
//     if (data) munmap(data, sz);
// }

// // std::vector<CurrencyPairData> readCurrencyPairCsvs(
// //     const std::string& bi, const std::string& as,
// //     const std::string& base, const std::string& quote)
// // {
// //     Timer tIO("CSV I/O");
// //     size_t bsz, asz;
// //     char *bd = mapFile(bi, bsz), *ad = mapFile(as, asz);
// //     std::vector<CurrencyPairData> out;
// //     if (!bd||!ad) return out;
// //     out.reserve(approximateLineCount(std::min(bsz,asz)));

// //     char *bp=bd, *be=bd+bsz, *ap=ad, *ae=ad+asz;
// //     bp = (char*)memchr(bp,'\n',be-bp); ap = (char*)memchr(ap,'\n',ae-ap);
// //     if (!bp||!ap) { unmapFile(bd,bsz); unmapFile(ad,asz); return out; }
// //     bp++; ap++;
// //     tIO.stop();
// //     Timer tParse("Tokenization & parsing");
// //     while(bp<be && ap<ae) {
// //         char *bE=(char*)memchr(bp,'\n',be-bp), *aE=(char*)memchr(ap,'\n',ae-ap);
// //         if(!bE||!aE) break;
// //         *bE = *aE = '\0';

// //         char* bt[6]={}; char* t=strtok(bp,",");
// //         for(int i=0;i<6&&t;i++){ bt[i]=t; t=strtok(nullptr,","); }
// //         char* at[6]={}; t=strtok(ap,",");
// //         for(int i=0;i<6&&t;i++){ at[i]=t; t=strtok(nullptr,","); }

// //         if(bt[0]&&at[0]&&strcmp(bt[0],at[0])==0) {
// //             CurrencyPairData e;
// //             e.timestamp_ms = parseTimestampMs(bt[0]);
// //             e.baseCurrency = base;
// //             e.quoteCurrency= quote;
// //             e.bid          = std::stod(bt[4]);
// //             e.ask          = std::stod(at[4]);
// //             out.push_back(e);
// //         }

// //         bp = bE+1; ap = aE+1;
// //     }
// //     tParse.stop();

// //     unmapFile(bd, bsz);
// //     unmapFile(ad, asz);
    
// //     return out;
// // }
// std::vector<CurrencyPairData> readCurrencyPairCsvs(
//     const std::string& bi,
//     const std::string& as,
//     const std::string& base,
//     const std::string& quote)
// {
//     // 1) Map files; bail early if mapping fails
//     size_t bsz = 0, asz = 0;
//     char *bd = mapFile(bi, bsz);
//     char *ad = mapFile(as, asz);
//     if (!bd || !ad) {
//         if (bd) unmapFile(bd, bsz);
//         if (ad) unmapFile(ad, asz);
//         return {};
//     }

//     // 2) Time just the raw I/O part (mmap + header-skip + reserve)
//     Timer tIO("CSV I/O");
//     std::vector<CurrencyPairData> out;
//     out.reserve(approximateLineCount(std::min(bsz, asz)));

//     // Skip headers in both buffers
//     char *bp = static_cast<char*>(memchr(bd, '\n', bsz));
//     char *ap = static_cast<char*>(memchr(ad, '\n', asz));
//     if (!bp || !ap) {
//         // unmap before returning
//         unmapFile(bd, bsz);
//         unmapFile(ad, asz);
//         tIO.stop();
//         return out;
//     }
//     bp++;  // move past newline
//     ap++;

//     tIO.stop();

//     // 3) Now parse while buffers are still mapped
//     Timer tParse("Tokenization & parsing");
//     char *be = bd + bsz, *ae = ad + asz;
//     while (bp < be && ap < ae) {
//         char *bE = static_cast<char*>(memchr(bp, '\n', be - bp));
//         char *aE = static_cast<char*>(memchr(ap, '\n', ae - ap));
//         if (!bE || !aE) break;

//         *bE = *aE = '\0';  // terminate lines

//         // tokenize columns
//         char* bt[6] = {};
//         char* tok = std::strtok(bp, ",");
//         for (int i = 0; i < 6 && tok; ++i) {
//             bt[i] = tok;
//             tok = std::strtok(nullptr, ",");
//         }
//         char* at[6] = {};
//         tok = std::strtok(ap, ",");
//         for (int i = 0; i < 6 && tok; ++i) {
//             at[i] = tok;
//             tok = std::strtok(nullptr, ",");
//         }

//         // if timestamps match, build a record
//         if (bt[0] && at[0] && std::strcmp(bt[0], at[0]) == 0) {
//             CurrencyPairData e;
//             e.timestamp_ms  = parseTimestampMs(bt[0]);
//             e.baseCurrency  = base;
//             e.quoteCurrency = quote;
//             e.bid           = std::stod(bt[4]);
//             e.ask           = std::stod(at[4]);
//             out.push_back(std::move(e));
//         }

//         bp = bE + 1;
//         ap = aE + 1;
//     }
//     tParse.stop();

//     // 4) Finally unmap the buffers
//     unmapFile(bd, bsz);
//     unmapFile(ad, asz);

//     return out;
// }

// ForexGraph buildForexGraphFromCsvs(
//     const std::vector<std::tuple<std::string, std::string, std::string, std::string>>& currencyFiles)
// {
//     ForexGraph graph;
//     // Reserve for 2 edges per pair (bid+ask)
// //	std::cout << "Print 1" << std::endl;
//     graph.reserveEdges(currencyFiles.size() * 2);
// //	std::cout << "Print 2" << std::endl;
//     for (const auto& tpl : currencyFiles) {
//         // Tuple is { askFile, bidFile, base, quote }
// //		std::cout << "Print 3" << std::endl;
//         const auto& [askFile, bidFile, baseCurrency, quoteCurrency] = tpl;
// //		std::cout << "Print 4" << std::endl;
//         // Notice: bidFile first, then askFile
//         auto pairData = readCurrencyPairCsvs(bidFile, askFile, baseCurrency, quoteCurrency);
//         if (pairData.empty()) {
//             std::cerr << "Warning: No data for " << baseCurrency
//                       << "/" << quoteCurrency << "\n";
//             continue;
//         }
// //		std::cout << "Print 5" << std::endl;
//         // Use the first tick for single-timestamp graph
//         const auto& e = pairData[0];
// //		std::cout << "Print 6" << std::endl;
//         graph.addExchangeRate(baseCurrency, quoteCurrency, e.bid, e.ask);
//     }
// //	std::cout << "Print 7" << std::endl;
//     return graph;
// }

// CsvParser.cpp

#include "CsvParser.hpp"
#include "Timer.hpp"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <omp.h>

// Map a file into memory (read-only)
static char* mapFile(const std::string& path, size_t& sz) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) return nullptr;
    struct stat st;
    if (fstat(fd, &st) < 0) { close(fd); return nullptr; }
    sz = st.st_size;
    char* data = static_cast<char*>(
        //mmap(nullptr, sz, PROT_READ, MAP_PRIVATE, fd, 0)
        mmap(nullptr, sz, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)
    );
    close(fd);
    return data == MAP_FAILED ? nullptr : data;
}
static void unmapFile(char* data, size_t sz) {
    if (data) munmap(data, sz);
}

// Parse "DD.MM.YYYY hh:mm:ss.SSS" → ms since midnight
static int64_t parseTimestampMs(const char* s) {
    int D,M,Y,h,m,sec,ms;
    std::sscanf(s, "%d.%d.%d %d:%d:%d.%d",
                &D, &M, &Y, &h, &m, &sec, &ms);
    return int64_t(h)*3600000 + int64_t(m)*60000
         + int64_t(sec)*1000 + ms;
}

// Estimate number of lines (for reserve)
static size_t approximateLineCount(size_t sz, size_t avg = 100) {
    return (sz + avg - 1) / avg;
}

std::vector<CurrencyPairData> readCurrencyPairCsvs(
    const std::string& bidPath,
    const std::string& askPath,
    const std::string& base,
    const std::string& quote,
    int thread_count
) {
    // 1) mmap both files
    size_t bsz = 0, asz = 0;
    char* bd = mapFile(bidPath, bsz);
    char* ad = mapFile(askPath, asz);
    if (!bd || !ad) {
        if (bd) unmapFile(bd, bsz);
        if (ad) unmapFile(ad, asz);
        return {};
    }

    // Timer for raw I/O + header skip + reserve
    Timer tIO("CSV I/O");

    // Skip single header line
    char* bStart = static_cast<char*>(memchr(bd, '\n', bsz));
    char* aStart = static_cast<char*>(memchr(ad, '\n', asz));
    if (!bStart || !aStart) {
        tIO.stop();
        unmapFile(bd, bsz);
        unmapFile(ad, asz);
        return {};
    }
    bStart++;  aStart++;
    char* bEnd = bd + bsz;
    char* aEnd = ad + asz;

    // Build pointers to each data line
    std::vector<char*> bidLines, askLines;
    bidLines.reserve(approximateLineCount(bsz));
    askLines.reserve(approximateLineCount(asz));

    for (char* p = bStart; p < bEnd; ) {
        bidLines.push_back(p);
        char* nl = static_cast<char*>(memchr(p, '\n', bEnd - p));
        if (!nl) break;
        p = nl + 1;
    }
    for (char* p = aStart; p < aEnd; ) {
        askLines.push_back(p);
        char* nl = static_cast<char*>(memchr(p, '\n', aEnd - p));
        if (!nl) break;
        p = nl + 1;
    }

    size_t Nrec = std::min(bidLines.size(), askLines.size());
    std::vector<CurrencyPairData> out;
    out.reserve(Nrec);

    tIO.stop();

    // Timer for tokenization & parsing
    Timer tParse("Tokenization & parsing");

    // Compute chunk boundaries
    int T = std::max(1, thread_count);
    std::vector<size_t> idx(T+1);
    for (int i = 0; i <= T; ++i) idx[i] = (Nrec * i) / T;

    #pragma omp parallel for num_threads(T) schedule(static)
    for (int t = 0; t < T; ++t) {
        size_t lo = idx[t], hi = idx[t+1];
        std::vector<CurrencyPairData> local;
        local.reserve(hi - lo);

        for (size_t i = lo; i < hi; ++i) {
            char* bLine = bidLines[i];
            char* aLine = askLines[i];

            // Null-terminate each line
            char* bNL = static_cast<char*>(memchr(bLine, '\n', bEnd - bLine));
            char* aNL = static_cast<char*>(memchr(aLine, '\n', aEnd - aLine));
            if (!bNL || !aNL) continue;
            *bNL = *aNL = '\0';

            // Thread-safe tokenization
            char* bsave = nullptr;
            char* colsB[6] = {nullptr};
            char* tok = strtok_r(bLine, ",", &bsave);
            for (int j = 0; tok && j < 6; ++j) {
                colsB[j] = tok;
                tok = strtok_r(nullptr, ",", &bsave);
            }

            char* asave = nullptr;
            char* colsA[6] = {nullptr};
            tok = strtok_r(aLine, ",", &asave);
            for (int j = 0; tok && j < 6; ++j) {
                colsA[j] = tok;
                tok = strtok_r(nullptr, ",", &asave);
            }

            if (colsB[0] && colsA[0] && std::strcmp(colsB[0], colsA[0]) == 0) {
                CurrencyPairData rec;
                rec.timestamp_ms  = parseTimestampMs(colsB[0]);
                rec.baseCurrency  = base;
                rec.quoteCurrency = quote;
                rec.bid           = std::stod(colsB[4]);
                rec.ask           = std::stod(colsA[4]);
                local.push_back(std::move(rec));
            }

            // Restore newline (optional)
            *bNL = '\n';  *aNL = '\n';
        }

        // Merge thread-local results
        #pragma omp critical
        out.insert(out.end(), local.begin(), local.end());
    }

    tParse.stop();

    // Clean up
    unmapFile(bd, bsz);
    unmapFile(ad, asz);

    return out;
}

ForexGraph buildForexGraphFromCsvs(
    const std::vector<std::tuple<std::string, std::string, std::string, std::string>>& currencyFiles,
    int thread_count
) {
    Timer tGraph("Graph construction");

    ForexGraph graph;
    graph.reserveEdges(currencyFiles.size() * 2);

    #pragma omp parallel num_threads(thread_count)
    {
        std::vector<std::tuple<std::string, std::string, double, double>> localRates;

        #pragma omp for nowait
        for (int i = 0; i < (int)currencyFiles.size(); ++i) {
            const auto& [askFile, bidFile, base, quote] = currencyFiles[i];
            auto data = readCurrencyPairCsvs(bidFile, askFile, base, quote, thread_count);
            if (!data.empty()) {
                const auto& e = data[0];
                localRates.emplace_back(base, quote, e.bid, e.ask);
            }
        }

        #pragma omp critical
        for (auto& r : localRates) {
            graph.addExchangeRate(
                std::get<0>(r),
                std::get<1>(r),
                std::get<2>(r),
                std::get<3>(r)
            );
        }
    }

    tGraph.stop();
    return graph;
}
