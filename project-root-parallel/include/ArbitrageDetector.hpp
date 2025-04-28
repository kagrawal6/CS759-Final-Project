

// // ArbitrageDetector.hpp
// #pragma once

// #include <vector>
// #include "ForexGraph.hpp"

// /// Encapsulates a found arbitrage cycle (as a list of vertex indices)
// /// and the percentage profit (e.g. 0.5 means 0.5%).
// struct ArbitrageOpportunity {
//     std::vector<int> cycle;
//     double profit;
// };

// /// Runs a (parallel) Bellman–Ford to detect the best negative‑cycle arbitrage.
// /// @param graph        the currency graph (with –log(rate) weights)
// /// @param thread_count how many OpenMP threads to use (default 1 = sequential)
// /// @returns the best cycle found (or empty cycle + profit=0 if none)
// ArbitrageOpportunity detectArbitrage(const ForexGraph& graph, int thread_count = 1);


// ArbitrageDetector.hpp
#pragma once

#include <vector>
#include "ForexGraph.hpp"
#include <algorithm> 

/// Encapsulates a found arbitrage cycle (as indices) + profit (%)
struct ArbitrageOpportunity {
    std::vector<int> cycle;
    double           profit;
};

/// Runs a parallel Bellman–Ford + negative‐cycle detect.
/// @param graph        FX graph (weights = –log(rate))
/// @param thread_count OpenMP threads to use
ArbitrageOpportunity detectArbitrage(
    const ForexGraph& graph,
    int thread_count = 1
);
