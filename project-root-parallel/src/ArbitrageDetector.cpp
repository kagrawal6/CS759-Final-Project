// // src/ArbitrageDetector.cpp

#include "ArbitrageDetector.hpp"
#include <limits>
#include <iostream>
#include <iomanip>
#include <omp.h>
#include <cmath>
#include <algorithm>

ArbitrageOpportunity detectArbitrage(const ForexGraph& graph, int thread_count) {
    // set thread count for OpenMP
    omp_set_num_threads(thread_count);

    int V = graph.getVertexCount();
    const auto& edges = graph.getEdges();
    
    // Global best result
    ArbitrageOpportunity bestResult{ {}, 0.0 };

    // Try each vertex as a source
    for (int source = 0; source < V; ++source) {
        std::vector<double> dist(V, std::numeric_limits<double>::infinity());
        std::vector<int> predecessor(V, -1);
        dist[source] = 0.0;

        // Relax all edges V-1 times
        bool changed = true;
        for (int i = 0; i < V - 1 && changed; ++i) {
            changed = false;
            for (const auto& edge : edges) {
                if (dist[edge.src] != std::numeric_limits<double>::infinity() &&
                    dist[edge.src] + edge.weight < dist[edge.dest]) {

                    dist[edge.dest] = dist[edge.src] + edge.weight;
                    predecessor[edge.dest] = edge.src;
                    changed = true;
                }
            }
        }

        // Check for negative weight cycles
        ArbitrageOpportunity result{ {}, 0.0 };
        bool found = false;

        #pragma omp parallel shared(result, found)
        {
            ArbitrageOpportunity localResult{ {}, 0.0 };
            bool localFound = false;

            #pragma omp for nowait
            for (size_t e = 0; e < edges.size(); ++e) {
                if (found) continue;

                const auto& edge = edges[e];
                if (dist[edge.src] != std::numeric_limits<double>::infinity() &&
                    dist[edge.src] + edge.weight < dist[edge.dest]) {

                    std::vector<bool> visited(V, false);
                    int currVertex = edge.dest;

                    while (!visited[currVertex]) {
                        visited[currVertex] = true;
                        currVertex = predecessor[currVertex];
                        if (currVertex == -1) break;
                    }

                    if (currVertex != -1) {
                        int cycleStart = currVertex;
                        std::vector<int> cycle;
                        double logProfit = 0.0;
                        int vertex = cycleStart;

                        do {
                            cycle.push_back(vertex);
                            int nextVertex = predecessor[vertex];
                            for (const auto& ed : edges) {
                                if (ed.src == nextVertex && ed.dest == vertex) {
                                    logProfit += ed.weight;
                                    break;
                                }
                            }
                            vertex = nextVertex;
                        } while (vertex != cycleStart);

                        std::reverse(cycle.begin(), cycle.end());
                        double actualProfit = std::exp(-logProfit) - 1.0;

                        localResult.cycle = std::move(cycle);
                        localResult.profit = actualProfit * 100.0;
                        localFound = true;
                    }
                }
            }

            #pragma omp critical
            {
                if (localFound && (localResult.profit > result.profit || !found)) {
                    result = localResult;
                    found = true;
                }
            }
        }
        
        if (found && (bestResult.cycle.empty() || result.profit > bestResult.profit)) {
            bestResult = result;
            //std::cout << "Found better arbitrage opportunity from source " 
                     // << graph.getCurrencyName(source)
                     // << " with profit " << std::fixed << std::setprecision(4)
                      //<< bestResult.profit << "%" << std::endl;
        }
    }

    return bestResult;
}




// src/ArbitrageDetector.cpp

// #include "ArbitrageDetector.hpp"
// #include "ForexGraph.hpp"
// #include <limits>
// #include <cmath>
// #include <algorithm>
// #include <omp.h>
// #include <iostream>


// static constexpr double INF = std::numeric_limits<double>::infinity();

// ArbitrageOpportunity detectArbitrage(
//     const ForexGraph& graph,
//     int thread_count
// ) {
//     // Set OpenMP threads only for inner loops
//     omp_set_num_threads(thread_count);

//     int V = graph.getVertexCount();
//     const auto& edges = graph.getEdges();

//     ArbitrageOpportunity bestResult{ {}, 0.0 };

//     // Iterate sources sequentially for predictable workload
//     for (int source = 0; source < V; ++source) {
//         std::vector<double> dist(V, INF);
//         std::vector<int>    pred(V, -1);
//         dist[source] = 0.0;

//         // Relax edges V-1 times in parallel
//         bool changed = true;
//         for (int i = 0; i < V - 1 && changed; ++i) {
//             changed = false;
//             #pragma omp parallel for schedule(static) reduction(||:changed)
//             for (size_t e = 0; e < edges.size(); ++e) {
//                 const auto& ed = edges[e];
//                 double nd = dist[ed.src] + ed.weight;
//                 if (dist[ed.src] != INF && nd < dist[ed.dest]) {
//                     dist[ed.dest] = nd;
//                     pred[ed.dest] = ed.src;
//                     changed = true;
//                 }
//             }
//         }

//         // Parallel negative-cycle detection per edge
//         ArbitrageOpportunity localBest{ {}, 0.0 };
//         #pragma omp parallel
//         {
//             ArbitrageOpportunity threadBest{ {}, 0.0 };
//             #pragma omp for schedule(dynamic)
//             for (size_t e = 0; e < edges.size(); ++e) {
//                 const auto& ed = edges[e];
//                 if (dist[ed.src] == INF) continue;
//                 if (dist[ed.src] + ed.weight < dist[ed.dest]) {
//                     // Found cycle: unwind
//                     std::vector<bool> seen(V, false);
//                     int cur = ed.dest;
//                     while (!seen[cur]) {
//                         seen[cur] = true;
//                         cur = pred[cur];
//                         if (cur < 0) break;
//                     }
//                     if (cur < 0) continue;
//                     int start = cur;
//                     std::vector<int> cycle;
//                     double logSum = 0.0;
//                     do {
//                         cycle.push_back(cur);
//                         int prev = pred[cur];
//                         // find corresponding weight
//                         for (const auto& ed2 : edges) {
//                             if (ed2.src == prev && ed2.dest == cur) {
//                                 logSum += ed2.weight;
//                                 break;
//                             }
//                         }
//                         cur = prev;
//                     } while (cur != start);
//                     std::reverse(cycle.begin(), cycle.end());
//                     double profit = (std::exp(-logSum) - 1.0) * 100.0;
//                     if (profit > threadBest.profit) {
//                         threadBest = { std::move(cycle), profit };
//                     }
//                 }
//             }
//             #pragma omp critical
//             {
//                 if (threadBest.profit > localBest.profit) {
//                     localBest = std::move(threadBest);
//                 }
//             }
//         }

//         if (!localBest.cycle.empty() && localBest.profit > bestResult.profit) {
//             bestResult = std::move(localBest);
//             std::cout << "Found better arbitrage from source="
//                       << graph.getCurrencyName(source)
//                       << " profit=" << bestResult.profit << "%\n";
//         }
//     }

//     return bestResult;
// }
