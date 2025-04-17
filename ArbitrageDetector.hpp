#pragma once
#include <vector>
#include <limits>
#include <iostream>
#include <iomanip>
#include <omp.h>
#include <cmath>
#include "ForexGraph.hpp"

struct ArbitrageOpportunity {
    std::vector<int> cycle;
    double profit;
};

ArbitrageOpportunity detectArbitrage(const ForexGraph& graph, int thread_count = 1) {
    // set thread count for OpenMP
    omp_set_num_threads(thread_count);

    int V = graph.getVertexCount();
    const auto& edges = graph.getEdges();

    // Initialize distance vector
    std::vector<double> dist(V, std::numeric_limits<double>::infinity());
    std::vector<int> predecessor(V, -1);

    // Choose a source vertex
    int source = 0;
    dist[source] = 0;

    // Relax all edges V-1 times
    for (int i = 0; i < V - 1; i++) {
        for (const auto& edge : edges) {
            if (dist[edge.src] != std::numeric_limits<double>::infinity() &&
                dist[edge.src] + edge.weight < dist[edge.dest]) {

                dist[edge.dest] = dist[edge.src] + edge.weight;
                predecessor[edge.dest] = edge.src;
            }
        }
    }

    // Check for negative weight cycles
    ArbitrageOpportunity result;
    result.profit = 0.0;
    bool found = false;

    #pragma omp parallel shared(result, found)
    {
    ArbitrageOpportunity localResult;
    localResult.profit = 0.0;
    bool localFound = false;

    #pragma omp for nowait
    for (size_t e = 0; e < edges.size(); e++) {
        // Skip if negative cycle already found
        if (found) continue;

        const auto& edge = edges[e];
        if (dist[edge.src] != std::numeric_limits<double>::infinity() &&
            dist[edge.src] + edge.weight < dist[edge.dest]) {

            // Negative cycle exists - find it
            std::vector<bool> visited(V, false);
            int currVertex = edge.dest;

            // Follow predecessors until we find a cycle
            while (!visited[currVertex]) {
                visited[currVertex] = true;
                currVertex = predecessor[currVertex];

                // Handle case where we can't find a cycle
                if (currVertex == -1) {
                    break;
                }
            }

            if (currVertex != -1) {
                // We found a cycle, extract it
                int cycleStart = currVertex;
                std::vector<int> cycle;
                double logProfit = 0.0;

                // Follow the cycle
                int vertex = cycleStart;
                do {
                    cycle.push_back(vertex);
                    int nextVertex = predecessor[vertex];

                    // Find the edge from nextVertex to vertex to get its weight
                    for (const auto& e : edges) {
                        if (e.src == nextVertex && e.dest == vertex) {
                            logProfit += e.weight;
                            break;
                        }
                    }

                    vertex = nextVertex;
                } while (vertex != cycleStart);

                // Reverse to get correct order
                std::reverse(cycle.begin(), cycle.end());

                // Calculate actual profit
                double actualProfit = exp(-logProfit) - 1.0;

                localResult.cycle = cycle;
                localResult.profit = actualProfit * 100.0; // as percentage
                localFound = true;
            }
        }
    }
    // Update the global result if a better local result is found
    #pragma omp critical
    {
        if (localFound && (result.profit < localResult.profit || !found)) {
            result = localResult;
            found = true;
        }
    }
    }
    return result;
}