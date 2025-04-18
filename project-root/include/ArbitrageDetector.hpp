// include/ArbitrageDetector.hpp
#pragma once

#include <vector>
#include <string>
#include "ForexGraph.hpp"

struct ArbitrageOpportunity {
    std::vector<int> cycle;
    double profit;
};

// Detect arbitrage via Bellman-Ford negative cycle detection.
// If verbose=true, prints detailed steps.
ArbitrageOpportunity detectArbitrage(const ForexGraph& graph, bool verbose = true);
