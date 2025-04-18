// src/ArbitrageDetector.cpp
#include "ArbitrageDetector.hpp"
#include <limits>
#include <iostream>
#include <iomanip>
#include <omp.h>
#include <cmath>
#include <algorithm>

ArbitrageOpportunity detectArbitrage(
    const ForexGraph& graph,
    int thread_count)
{
    // configure OpenMP
    omp_set_num_threads(thread_count);

    int V = graph.getVertexCount();
    const auto& edges = graph.getEdges();

    // 1) Bellman–Ford relaxation to compute shortest paths in –log space
    std::vector<double> dist(V, std::numeric_limits<double>::infinity());
    std::vector<int>    predecessor(V, -1);
    int source = 0;
    dist[source] = 0.0;

    for (int i = 0; i < V - 1; ++i) {
        bool updated = false;
        for (const auto& e : edges) {
            if (dist[e.src] + e.weight < dist[e.dest]) {
                dist[e.dest]       = dist[e.src] + e.weight;
                predecessor[e.dest] = e.src;
                updated = true;
            }
        }
        if (!updated) break;
    }

    // 2) Parallel scan for negative‐cycle edges
    ArbitrageOpportunity result;
    result.profit = 0.0;
    bool found    = false;

    #pragma omp parallel shared(result, found)
    {
        ArbitrageOpportunity local;
        local.profit = 0.0;
        bool localFound = false;


        int E = static_cast<int>(edges.size());

        #pragma omp for nowait
        for (int ei = 0; ei < E; ++ei) {
            if (found) continue;  // skip work if global cycle already detected

            const auto& e = edges[ei];
            if (dist[e.src] + e.weight < dist[e.dest]) {
                // Negative cycle detected; trace it
                std::vector<bool> visited(V, false);
                int curr = e.dest;
                // follow predecessors until we loop
                while (!visited[curr] && curr >= 0) {
                    visited[curr] = true;
                    curr = predecessor[curr];
                }
                if (curr < 0) continue;  // no valid cycle

                // extract cycle starting at 'curr'
                int start = curr;
                std::vector<int> cycle;
                double logSum = 0.0;
                do {
                    cycle.push_back(curr);
                    int prev = predecessor[curr];
                    // find corresponding edge to accumulate weight
                    for (auto& ed : edges) {
                        if (ed.src == prev && ed.dest == curr) {
                            logSum += ed.weight;
                            break;
                        }
                    }
                    curr = prev;
                } while (curr != start);
                std::reverse(cycle.begin(), cycle.end());

                double profitPct = std::exp(-logSum) - 1.0;
                local.cycle  = std::move(cycle);
                local.profit = profitPct * 100.0;
                localFound   = true;
            }
        }

        // reduce into global result
        #pragma omp critical
        {
            if (localFound && (!found || local.profit > result.profit)) {
                result = local;
                found  = true;
            }
        }
    }

    return result;
}
