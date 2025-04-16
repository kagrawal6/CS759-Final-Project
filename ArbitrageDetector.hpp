#pragma once
#include <vector>
#include <limits>
#include <iostream>
#include <iomanip>
#include "ForexGraph.hpp"

struct ArbitrageOpportunity {
    std::vector<int> cycle;
    double profit;
};

ArbitrageOpportunity detectArbitrage(const ForexGraph& graph) {
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

    for (const auto& edge : edges) {
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

                result.cycle = cycle;
                result.profit = actualProfit * 100.0; // as percentage

                return result;
            }
        }
    }
    return result;
}