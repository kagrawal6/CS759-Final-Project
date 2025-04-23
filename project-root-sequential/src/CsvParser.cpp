

//think aobut adding  Binary “tick dump to increase performance 


#include "CsvParser.hpp"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include "Timer.hpp"

// parse "DD.MM.YYYY hh:mm:ss.SSS" → ms since midnight
static int64_t parseTimestampMs(const char *s) {
    int D, M, Y, h, m, sec, ms;
    std::sscanf(s, "%d.%d.%d %d:%d:%d.%d", &D, &M, &Y, &h, &m, &sec, &ms);
    return int64_t(h)*3600000 + int64_t(m)*60000 + int64_t(sec)*1000 + ms;
}

// estimate lines for reserve()
static size_t approximateLineCount(size_t sz, size_t avg=100) {
    return (sz + avg - 1)/avg;
}

// mmap helper
static char* mapFile(const std::string& path, size_t& sz) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd<0) { perror("open"); return nullptr; }
    struct stat st; fstat(fd, &st); sz = st.st_size;
   // char* data = (char*)mmap(nullptr, sz, PROT_READ, MAP_PRIVATE, fd, 0);
	 char* data = static_cast<char*>(mmap(nullptr,sz,PROT_READ | PROT_WRITE,MAP_PRIVATE,fd, 0));
        if (data == MAP_FAILED) {
        perror("mmap failed");
        close(fd);
        return nullptr;
    }

    close(fd);
    return data;
}
static void unmapFile(char* data, size_t sz) {
    if (data) munmap(data, sz);
}

// std::vector<CurrencyPairData> readCurrencyPairCsvs(
//     const std::string& bi, const std::string& as,
//     const std::string& base, const std::string& quote)
// {
//     Timer tIO("CSV I/O");
//     size_t bsz, asz;
//     char *bd = mapFile(bi, bsz), *ad = mapFile(as, asz);
//     std::vector<CurrencyPairData> out;
//     if (!bd||!ad) return out;
//     out.reserve(approximateLineCount(std::min(bsz,asz)));

//     char *bp=bd, *be=bd+bsz, *ap=ad, *ae=ad+asz;
//     bp = (char*)memchr(bp,'\n',be-bp); ap = (char*)memchr(ap,'\n',ae-ap);
//     if (!bp||!ap) { unmapFile(bd,bsz); unmapFile(ad,asz); return out; }
//     bp++; ap++;
//     tIO.stop();
//     Timer tParse("Tokenization & parsing");
//     while(bp<be && ap<ae) {
//         char *bE=(char*)memchr(bp,'\n',be-bp), *aE=(char*)memchr(ap,'\n',ae-ap);
//         if(!bE||!aE) break;
//         *bE = *aE = '\0';

//         char* bt[6]={}; char* t=strtok(bp,",");
//         for(int i=0;i<6&&t;i++){ bt[i]=t; t=strtok(nullptr,","); }
//         char* at[6]={}; t=strtok(ap,",");
//         for(int i=0;i<6&&t;i++){ at[i]=t; t=strtok(nullptr,","); }

//         if(bt[0]&&at[0]&&strcmp(bt[0],at[0])==0) {
//             CurrencyPairData e;
//             e.timestamp_ms = parseTimestampMs(bt[0]);
//             e.baseCurrency = base;
//             e.quoteCurrency= quote;
//             e.bid          = std::stod(bt[4]);
//             e.ask          = std::stod(at[4]);
//             out.push_back(e);
//         }

//         bp = bE+1; ap = aE+1;
//     }
//     tParse.stop();

//     unmapFile(bd, bsz);
//     unmapFile(ad, asz);
    
//     return out;
// }
std::vector<CurrencyPairData> readCurrencyPairCsvs(
    const std::string& bi,
    const std::string& as,
    const std::string& base,
    const std::string& quote)
{
    // 1) Map files; bail early if mapping fails
    size_t bsz = 0, asz = 0;
    char *bd = mapFile(bi, bsz);
    char *ad = mapFile(as, asz);
    if (!bd || !ad) {
        if (bd) unmapFile(bd, bsz);
        if (ad) unmapFile(ad, asz);
        return {};
    }

    // 2) Time just the raw I/O part (mmap + header-skip + reserve)
    Timer tIO("CSV I/O");
    std::vector<CurrencyPairData> out;
    out.reserve(approximateLineCount(std::min(bsz, asz)));

    // Skip headers in both buffers
    char *bp = static_cast<char*>(memchr(bd, '\n', bsz));
    char *ap = static_cast<char*>(memchr(ad, '\n', asz));
    if (!bp || !ap) {
        // unmap before returning
        unmapFile(bd, bsz);
        unmapFile(ad, asz);
        tIO.stop();
        return out;
    }
    bp++;  // move past newline
    ap++;

    tIO.stop();

    // 3) Now parse while buffers are still mapped
    Timer tParse("Tokenization & parsing");
    char *be = bd + bsz, *ae = ad + asz;
    while (bp < be && ap < ae) {
        char *bE = static_cast<char*>(memchr(bp, '\n', be - bp));
        char *aE = static_cast<char*>(memchr(ap, '\n', ae - ap));
        if (!bE || !aE) break;

        *bE = *aE = '\0';  // terminate lines

        // tokenize columns
        char* bt[6] = {};
        char* tok = std::strtok(bp, ",");
        for (int i = 0; i < 6 && tok; ++i) {
            bt[i] = tok;
            tok = std::strtok(nullptr, ",");
        }
        char* at[6] = {};
        tok = std::strtok(ap, ",");
        for (int i = 0; i < 6 && tok; ++i) {
            at[i] = tok;
            tok = std::strtok(nullptr, ",");
        }

        // if timestamps match, build a record
        if (bt[0] && at[0] && std::strcmp(bt[0], at[0]) == 0) {
            CurrencyPairData e;
            e.timestamp_ms  = parseTimestampMs(bt[0]);
            e.baseCurrency  = base;
            e.quoteCurrency = quote;
            e.bid           = std::stod(bt[4]);
            e.ask           = std::stod(at[4]);
            out.push_back(std::move(e));
        }

        bp = bE + 1;
        ap = aE + 1;
    }
    tParse.stop();

    // 4) Finally unmap the buffers
    unmapFile(bd, bsz);
    unmapFile(ad, asz);

    return out;
}

ForexGraph buildForexGraphFromCsvs(
    const std::vector<std::tuple<std::string, std::string, std::string, std::string>>& currencyFiles)
{
    ForexGraph graph;
    // Reserve for 2 edges per pair (bid+ask)
//	std::cout << "Print 1" << std::endl;
    graph.reserveEdges(currencyFiles.size() * 2);
//	std::cout << "Print 2" << std::endl;
    for (const auto& tpl : currencyFiles) {
        // Tuple is { askFile, bidFile, base, quote }
//		std::cout << "Print 3" << std::endl;
        const auto& [askFile, bidFile, baseCurrency, quoteCurrency] = tpl;
//		std::cout << "Print 4" << std::endl;
        // Notice: bidFile first, then askFile
        auto pairData = readCurrencyPairCsvs(bidFile, askFile, baseCurrency, quoteCurrency);
        if (pairData.empty()) {
            std::cerr << "Warning: No data for " << baseCurrency
                      << "/" << quoteCurrency << "\n";
            continue;
        }
//		std::cout << "Print 5" << std::endl;
        // Use the first tick for single-timestamp graph
        const auto& e = pairData[0];
//		std::cout << "Print 6" << std::endl;
        graph.addExchangeRate(baseCurrency, quoteCurrency, e.bid, e.ask);
    }
//	std::cout << "Print 7" << std::endl;
    return graph;
}

