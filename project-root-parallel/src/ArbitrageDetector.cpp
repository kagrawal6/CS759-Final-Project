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
            std::cout << "Found better arbitrage opportunity from source " 
                      << graph.getCurrencyName(source)
                      << " with profit " << std::fixed << std::setprecision(4)
                      << bestResult.profit << "%" << std::endl;
        }
    }

    return bestResult;
}
