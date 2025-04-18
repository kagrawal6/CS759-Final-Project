// src/ArbitrageDetector.cpp
#include "ArbitrageDetector.hpp"
#include <limits>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cmath>

ArbitrageOpportunity detectArbitrage(const ForexGraph& graph, bool verbose) {
    int V = graph.getVertexCount();
    const auto& edges = graph.getEdges();

    if (verbose) {
        std::cout << "\n===== ARBITRAGE DETECTION DETAILS =====\n";
        std::cout << "Number of vertices (currencies): " << V << std::endl;
        std::cout << "Number of edges (exchange rates): " << edges.size() << std::endl;
        std::cout << "\nAll exchange rates in the graph:\n";
        for (const auto& edge : edges) {
            std::cout << "  " << std::setw(4) << graph.getCurrencyName(edge.src)
                      << " -> " << std::setw(4) << graph.getCurrencyName(edge.dest)
                      << ": Rate = " << std::fixed << std::setprecision(6)
                      << std::exp(-edge.weight)
                      << " (Weight = " << std::setprecision(6) << edge.weight << ")\n";
        }
    }

    std::vector<double> dist(V, std::numeric_limits<double>::infinity());
    std::vector<int> predecessor(V, -1);
    int source = 0;
    dist[source] = 0;

    if (verbose) {
        std::cout << "\nRunning Bellman-Ford algorithm with source: "
                  << graph.getCurrencyName(source) << std::endl;
        std::cout << "Initial distances: ";
        for (int i = 0; i < V; i++) {
            if (i == source) std::cout << graph.getCurrencyName(i) << "=0 ";
            else             std::cout << graph.getCurrencyName(i) << "=∞ ";
        }
        std::cout << std::endl;
    }

    bool relaxed = false;
    for (int i = 0; i < V - 1; i++) {
        relaxed = false;
        if (verbose) std::cout << "\nIteration " << i+1 << " of " << V-1 << ":\n";

        for (const auto& edge : edges) {
            if (dist[edge.src] != std::numeric_limits<double>::infinity() &&
                dist[edge.src] + edge.weight < dist[edge.dest]) {

                if (verbose) {
                    std::cout << "  Relaxing edge " << graph.getCurrencyName(edge.src)
                              << " -> " << graph.getCurrencyName(edge.dest)
                              << ": " << dist[edge.dest] << " -> "
                              << (dist[edge.src] + edge.weight) << "\n";
                }
                dist[edge.dest] = dist[edge.src] + edge.weight;
                predecessor[edge.dest] = edge.src;
                relaxed = true;
            }
        }

        if (verbose && !relaxed) {
            std::cout << "  No edges relaxed in this iteration, converged early.\n";
            break;
        }
        if (verbose) {
            std::cout << "  Current distances: ";
            for (int j = 0; j < V; j++) {
                if (dist[j] == std::numeric_limits<double>::infinity())
                    std::cout << graph.getCurrencyName(j) << "=∞ ";
                else
                    std::cout << graph.getCurrencyName(j) << "="
                              << std::fixed << std::setprecision(4) << dist[j] << " ";
            }
            std::cout << "\n";
        }
    }

    ArbitrageOpportunity result;
    result.profit = 0.0;
    bool cycleFound = false;
    if (verbose) {
        std::cout << "\nChecking for negative weight cycles (arbitrage opportunities):\n";
    }
    for (const auto& edge : edges) {
        if (dist[edge.src] != std::numeric_limits<double>::infinity() &&
            dist[edge.src] + edge.weight < dist[edge.dest]) {

            cycleFound = true;
            if (verbose) {
                std::cout << "  Negative cycle detected via edge: "
                          << graph.getCurrencyName(edge.src) << " -> "
                          << graph.getCurrencyName(edge.dest) << "\n";
            }

            // Trace cycle
            std::vector<bool> visited(V, false);
            int curr = edge.dest;
            if (verbose) std::cout << "  Tracing cycle from: " << graph.getCurrencyName(curr) << "\n";
            while (!visited[curr]) {
                visited[curr] = true;
                curr = predecessor[curr];
                if (curr == -1) {
                    if (verbose) std::cout << "  Cannot complete cycle\n";
                    break;
                }
            }

            if (curr != -1) {
                int start = curr;
                std::vector<int> cycle;
                double logProfit = 0.0;
                if (verbose) std::cout << "\n  Complete cycle found: ";
                int v = start;
                do {
                    cycle.push_back(v);
                    int nxt = predecessor[v];
                    for (auto& e : edges) {
                        if (e.src == nxt && e.dest == v) {
                            logProfit += e.weight;
                            if (verbose) {
                                std::cout << graph.getCurrencyName(nxt) << " → "
                                          << graph.getCurrencyName(v)
                                          << " (rate: " << std::fixed << std::setprecision(6)
                                          << std::exp(-e.weight) << ") ";
                            }
                            break;
                        }
                    }
                    v = nxt;
                } while (v != start);

                std::reverse(cycle.begin(), cycle.end());
                double actualProfit = std::exp(-logProfit) - 1.0;
                result.cycle  = cycle;
                result.profit = actualProfit * 100.0;
                return result;
            }
        }
    }

    if (verbose && !cycleFound)
        std::cout << "  No arbitrage opportunities found.\n";

    return result;
}
