

// #include <chrono>
// #include <ctime>
// #include <iostream>
// #include <vector>
// #include <tuple>
// #include <string>
// #include <iomanip>
// #include <numeric>
// #include <cmath>
// #include <cstdlib>
// #include <omp.h>

// #include "CsvParser.hpp"
// #include "ForexGraph.hpp"
// #include "ArbitrageDetector.hpp"

// // A simple struct to hold one run’s metrics
// struct Metrics {
//     int    threads;   // number of threads used
//     double wall_ms;   // wall-clock time in milliseconds
//     double cpu_ms;    // CPU time (sum across threads) in milliseconds
// };

// // List of currency‐pair CSVs (same as in your main.cpp)
// static const std::vector<std::tuple<std::string, std::string, std::string, std::string>> currencyFiles = {
//     {"data/ask/AUDCAD_ASK.csv", "data/bid/AUDCAD_BID.csv", "AUD", "CAD"},
//     {"data/ask/AUDJPY_ASK.csv", "data/bid/AUDJPY_BID.csv", "AUD", "JPY"},
//     {"data/ask/AUDSGD_ASK.csv", "data/bid/AUDSGD_BID.csv", "AUD", "SGD"},
//     {"data/ask/AUDUSD_ASK.csv", "data/bid/AUDUSD_BID.csv", "AUD", "USD"},
//     {"data/ask/CADJPY_ASK.csv", "data/bid/CADJPY_BID.csv", "CAD", "JPY"},
//     {"data/ask/EURAUD_ASK.csv", "data/bid/EURAUD_BID.csv", "EUR", "AUD"},
//     {"data/ask/EURCAD_ASK.csv", "data/bid/EURCAD_BID.csv", "EUR", "CAD"},
//     {"data/ask/EURGBP_ASK.csv", "data/bid/EURGBP_BID.csv", "EUR", "GBP"},
//     {"data/ask/EURJPY_ASK.csv", "data/bid/EURJPY_BID.csv", "EUR", "JPY"},
//     {"data/ask/EURSGD_ASK.csv", "data/bid/EURSGD_BID.csv", "EUR", "SGD"},
//     {"data/ask/EURUSD_ASK.csv", "data/bid/EURUSD_BID.csv", "EUR", "USD"},
//     {"data/ask/GBPAUD_ASK.csv", "data/bid/GBPAUD_BID.csv", "GBP", "AUD"},
//     {"data/ask/GBPCAD_ASK.csv", "data/bid/GBPCAD_BID.csv", "GBP", "CAD"},
//     {"data/ask/GBPJPY_ASK.csv", "data/bid/GBPJPY_BID.csv", "GBP", "JPY"},
//     {"data/ask/GBPUSD_ASK.csv", "data/bid/GBPUSD_BID.csv", "GBP", "USD"},
//     {"data/ask/SGDJPY_ASK.csv", "data/bid/SGDJPY_BID.csv", "SGD", "JPY"},
//     {"data/ask/USDCAD_ASK.csv", "data/bid/USDCAD_BID.csv", "USD", "CAD"},
//     {"data/ask/USDJPY_ASK.csv", "data/bid/USDJPY_BID.csv", "USD", "JPY"},
//     {"data/ask/USDSGD_ASK.csv", "data/bid/USDSGD_BID.csv", "USD", "SGD"}
// };

// // Core work: build the graph and run arbitrage detection
// void run_work(int threads) {
//     // Disable dynamic teams and bind threads to cores
//     omp_set_dynamic(0);
//     setenv("OMP_PROC_BIND", "TRUE", 1);
//     setenv("OMP_PLACES", "cores", 1);

//     omp_set_num_threads(threads);
//     // Build graph from CSVs using given thread count
//     ForexGraph graph = buildForexGraphFromCsvs(currencyFiles, threads);
//     // Detect arbitrage opportunities
//     auto opp = detectArbitrage(graph, threads);
//     // Prevent optimization
//     volatile double dummy = opp.profit;
//     (void)dummy;
// }

// // Measure wall-clock and CPU time for a single invocation
// Metrics measure_run(int threads) {
//     auto wc_start = std::chrono::high_resolution_clock::now();
//     std::clock_t cpu_start = std::clock();

//     run_work(threads);

//     auto wc_end = std::chrono::high_resolution_clock::now();
//     std::clock_t cpu_end = std::clock();

//     double wall_ms = std::chrono::duration<double, std::milli>(wc_end - wc_start).count();
//     double cpu_ms  = (double)(cpu_end - cpu_start) / CLOCKS_PER_SEC * 1000.0;
//     return { threads, wall_ms, cpu_ms };
// }

// int main() {
//     // Thread counts to test (including serial 1)
//     std::vector<int> thread_counts = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
//     const int reps = 5;  // number of measured repetitions per config

//     // Print CSV header
//     std::cout << "threads,mean_wall_ms,stddev_wall_ms,mean_cpu_ms,stddev_cpu_ms,speedup,efficiency,f_e\n";

//     // Get serial baseline T1
//     Metrics serial = measure_run(1);

//     for (int p : thread_counts) {
//         // Warm-up (not measured)
//         measure_run(p);

//         // Collect measurements
//         std::vector<double> walls, cpus;
//         for (int i = 0; i < reps; ++i) {
//             Metrics m = measure_run(p);
//             walls.push_back(m.wall_ms);
//             cpus.push_back(m.cpu_ms);
//         }
//         // Compute mean and stddev for wall times
//         double mean_wall = std::accumulate(walls.begin(), walls.end(), 0.0) / walls.size();
//         double var_wall  = 0.0;
//         for (double x : walls) var_wall += (x - mean_wall)*(x - mean_wall);
//         double stddev_wall = std::sqrt(var_wall / walls.size());
//         // Compute mean cpu time
//         double mean_cpu = std::accumulate(cpus.begin(), cpus.end(), 0.0) / cpus.size();

//         // Speedup, efficiency, and Karp–Flatt serial fraction
//         double S  = serial.wall_ms / mean_wall;
//         double E  = S / p;
//         double fe = (1.0/S - 1.0/p) / (1.0 - 1.0/p);

//         // Emit CSV line
//         std::cout
//             << p << ","
//             << mean_wall  << "," << stddev_wall << ","
//             << mean_cpu   << ","
//             << S          << ","
//             << E          << ","
//             << fe         << "\n";
//     }
//     return 0;
// }
// benchmark.cpp

// benchmark.cpp
// benchmark.cpp

#include <chrono>
#include <ctime>
#include <iostream>
#include <vector>
#include <tuple>
#include <string>
#include <iomanip>
#include <numeric>
#include <cmath>
#include <cstdlib>
#include <omp.h>

#include "CsvParser.hpp"
#include "ForexGraph.hpp"
#include "ArbitrageDetector.hpp"
#include "TimeSeriesArbitrageDetector.hpp"
#include "PositionsManager.hpp"

// Metrics from one benchmark run
struct Metrics {
    int    threads;      // number of threads used
    double wall_ms;      // wall-clock time in ms
    double cpu_ms;       // CPU time (sum) in ms
    double profit_usd;   // net profit in USD
    int    trade_count;  // number of trades executed
};

// Same currency files list as main.cpp
static const std::vector<std::tuple<std::string,std::string,std::string,std::string>> currencyFiles = {
    {"data/ask/AUDCAD_ASK.csv","data/bid/AUDCAD_BID.csv","AUD","CAD"},
    {"data/ask/AUDJPY_ASK.csv","data/bid/AUDJPY_BID.csv","AUD","JPY"},
    {"data/ask/AUDSGD_ASK.csv","data/bid/AUDSGD_BID.csv","AUD","SGD"},
    {"data/ask/AUDUSD_ASK.csv","data/bid/AUDUSD_BID.csv","AUD","USD"},
    {"data/ask/CADJPY_ASK.csv","data/bid/CADJPY_BID.csv","CAD","JPY"},
    {"data/ask/EURAUD_ASK.csv","data/bid/EURAUD_BID.csv","EUR","AUD"},
    {"data/ask/EURCAD_ASK.csv","data/bid/EURCAD_BID.csv","EUR","CAD"},
    {"data/ask/EURGBP_ASK.csv","data/bid/EURGBP_BID.csv","EUR","GBP"},
    {"data/ask/EURJPY_ASK.csv","data/bid/EURJPY_BID.csv","EUR","JPY"},
    {"data/ask/EURSGD_ASK.csv","data/bid/EURSGD_BID.csv","EUR","SGD"},
    {"data/ask/EURUSD_ASK.csv","data/bid/EURUSD_BID.csv","EUR","USD"},
    {"data/ask/GBPAUD_ASK.csv","data/bid/GBPAUD_BID.csv","GBP","AUD"},
    {"data/ask/GBPCAD_ASK.csv","data/bid/GBPCAD_BID.csv","GBP","CAD"},
    {"data/ask/GBPJPY_ASK.csv","data/bid/GBPJPY_BID.csv","GBP","JPY"},
    {"data/ask/GBPUSD_ASK.csv","data/bid/GBPUSD_BID.csv","GBP","USD"},
    {"data/ask/SGDJPY_ASK.csv","data/bid/SGDJPY_BID.csv","SGD","JPY"},
    {"data/ask/USDCAD_ASK.csv","data/bid/USDCAD_BID.csv","USD","CAD"},
    {"data/ask/USDJPY_ASK.csv","data/bid/USDJPY_BID.csv","USD","JPY"},
    {"data/ask/USDSGD_ASK.csv","data/bid/USDSGD_BID.csv","USD","SGD"}
};

// Perform full pipeline: time-series detection + trading simulation
Metrics measure_run(int threads, double initial_capital) {
    // Setup OpenMP
    omp_set_dynamic(0);
    setenv("OMP_PROC_BIND","TRUE",1);
    setenv("OMP_PLACES","cores",1);
    omp_set_num_threads(threads);

    // Start timers
    auto wc_start = std::chrono::high_resolution_clock::now();
    std::clock_t cpu_start = std::clock();

    // Time-series arbitrage detection
    TimeSeriesArbitrageDetector detector(currencyFiles, threads);
    detector.analyzeAllTimestamps(false);
    auto opportunities = detector.getOpportunities();

    // Trading simulation
    PositionsManager pm("USD", initial_capital);
    for (const auto& opp : opportunities) {
        ForexGraph graph = detector.getGraphForTimestamp(opp.timestamp_ms);
        pm.executeArbitrageOpportunity(opp, graph);
    }
    // Final portfolio value
    const auto& history = pm.getPortfolioHistory();
    double final_value = history.empty() ? initial_capital : history.back().totalValueUSD;
    double profit_usd = final_value - initial_capital;
    int trades = opportunities.size();

    // Stop timers
    auto wc_end = std::chrono::high_resolution_clock::now();
    std::clock_t cpu_end = std::clock();
    double wall_ms = std::chrono::duration<double,std::milli>(wc_end - wc_start).count();
    double cpu_ms  = (double)(cpu_end - cpu_start)/CLOCKS_PER_SEC*1000.0;

    return {threads, wall_ms, cpu_ms, profit_usd, trades};
}

int main() {
    std::vector<int> thread_counts = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20};
    const int reps = 5;
    const double initial_capital = 1000.0;

    // CSV header
    std::cout << "threads,mean_wall_ms,stddev_wall_ms,mean_cpu_ms,stddev_cpu_ms,";
    std::cout << "speedup,efficiency,f_e,mean_profit_usd,profit_usd_per_s,trade_count\n";

    // Serial baseline (full run)
    Metrics base = measure_run(1, initial_capital);

    for (int p : thread_counts) {
        // Warm-up
        measure_run(p, initial_capital);

        // Collect metrics
        std::vector<double> walls, cpus, profits;
        std::vector<int> trades;
        for (int i = 0; i < reps; ++i) {
            Metrics m = measure_run(p, initial_capital);
            walls.push_back(m.wall_ms);
            cpus.push_back(m.cpu_ms);
            profits.push_back(m.profit_usd);
            trades.push_back(m.trade_count);
        }
        // Compute means & stddev
        double mean_wall = std::accumulate(walls.begin(), walls.end(), 0.0)/walls.size();
        double var_wall=0; for(double x:walls)var_wall+=(x-mean_wall)*(x-mean_wall);
        double stddev_wall=sqrt(var_wall/walls.size());
        double mean_cpu = std::accumulate(cpus.begin(), cpus.end(),0.0)/cpus.size();
        double mean_profit = std::accumulate(profits.begin(),profits.end(),0.0)/profits.size();
        int    mean_trades = std::accumulate(trades.begin(),trades.end(),0)/trades.size();

        // Performance metrics
        double S  = base.wall_ms/mean_wall;
        double E  = S/p;
        double fe = (1.0/S - 1.0/p)/(1.0 - 1.0/p);
        double profit_per_s = mean_profit*1000.0/mean_wall;

        // Emit CSV line
        std::cout<<p<<","<<mean_wall<<","<<stddev_wall<<","<<mean_cpu<<",";
        std::cout<<S<<","<<E<<","<<fe<<",";
        std::cout<<mean_profit<<","<<profit_per_s<<","<<mean_trades<<"\n";
    }
    return 0;
}
