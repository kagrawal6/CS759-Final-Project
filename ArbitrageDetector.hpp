// NOT BEING USED AT THE MOMENT
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

ArbitrageOpportunity detectArbitrage(const ForexGraph& graph, bool verbose = true) {
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
                      << ": Rate = " << std::fixed << std::setprecision(6) << exp(-edge.weight)
                      << " (Weight = " << std::setprecision(6) << edge.weight << ")" << std::endl;
        }
    }
    
    // Initialize distance vector
    std::vector<double> dist(V, std::numeric_limits<double>::infinity());
    std::vector<int> predecessor(V, -1);
    
    // Choose a source vertex
    int source = 0;
    dist[source] = 0;
    
    if (verbose) {
        std::cout << "\nRunning Bellman-Ford algorithm with source: " 
                  << graph.getCurrencyName(source) << std::endl;
        std::cout << "Initial distances: ";
        for (int i = 0; i < V; i++) {
            if (i == source) {
                std::cout << graph.getCurrencyName(i) << "=0 ";
            } else {
                std::cout << graph.getCurrencyName(i) << "=∞ ";
            }
        }
        std::cout << std::endl;
    }
    
    // Relax all edges V-1 times
    bool relaxed = false;
    for (int i = 0; i < V - 1; i++) {
        relaxed = false;
        if (verbose) std::cout << "\nIteration " << i+1 << " of " << V-1 << ":" << std::endl;
        
        for (const auto& edge : edges) {
            if (dist[edge.src] != std::numeric_limits<double>::infinity() && 
                dist[edge.src] + edge.weight < dist[edge.dest]) {
                
                if (verbose) {
                    std::cout << "  Relaxing edge " << graph.getCurrencyName(edge.src) 
                              << " -> " << graph.getCurrencyName(edge.dest)
                              << ": " << dist[edge.dest] << " -> " 
                              << (dist[edge.src] + edge.weight) << std::endl;
                }
                
                dist[edge.dest] = dist[edge.src] + edge.weight;
                predecessor[edge.dest] = edge.src;
                relaxed = true;
            }
        }
        
        if (verbose && !relaxed) {
            std::cout << "  No edges relaxed in this iteration, algorithm converged early." << std::endl;
            break;
        }
        
        if (verbose) {
            std::cout << "  Current distances: ";
            for (int j = 0; j < V; j++) {
                if (dist[j] == std::numeric_limits<double>::infinity()) {
                    std::cout << graph.getCurrencyName(j) << "=∞ ";
                } else {
                    std::cout << graph.getCurrencyName(j) << "=" << std::fixed << std::setprecision(4) << dist[j] << " ";
                }
            }
            std::cout << std::endl;
        }
    }
    
    // Check for negative weight cycles
    ArbitrageOpportunity result;
    result.profit = 0.0;
    
    if (verbose) {
        std::cout << "\nChecking for negative weight cycles (arbitrage opportunities):" << std::endl;
    }
    
    bool cycleFound = false;
    for (const auto& edge : edges) {
        if (dist[edge.src] != std::numeric_limits<double>::infinity() && 
            dist[edge.src] + edge.weight < dist[edge.dest]) {
            
            cycleFound = true;
            if (verbose) {
                std::cout << "  Negative cycle detected through edge: " 
                          << graph.getCurrencyName(edge.src) << " -> " 
                          << graph.getCurrencyName(edge.dest) << std::endl;
            }
            
            // Negative cycle exists - find it
            std::vector<bool> visited(V, false);
            int currVertex = edge.dest;
            
            if (verbose) {
                std::cout << "  Tracing cycle from: " << graph.getCurrencyName(currVertex) << std::endl;
            }
            
            // Follow predecessors until we find a cycle
            while (!visited[currVertex]) {
                visited[currVertex] = true;
                currVertex = predecessor[currVertex];
                
                // Handle case where we can't find a cycle
                if (currVertex == -1) {
                    if (verbose) std::cout << "  Cannot complete cycle (reached vertex with no predecessor)" << std::endl;
                    break;
                }
                
                if (verbose) {
                    std::cout << "  → " << graph.getCurrencyName(currVertex);
                }
            }
            
            if (currVertex != -1) {
                // We found a cycle, extract it
                int cycleStart = currVertex;
                std::vector<int> cycle;
                double logProfit = 0.0;
                
                if (verbose) {
                    std::cout << "\n\n  Complete cycle found: ";
                }
                
                // Follow the cycle
                int vertex = cycleStart;
                do {
                    cycle.push_back(vertex);
                    int nextVertex = predecessor[vertex];
                    
                    // Find the edge from nextVertex to vertex to get its weight
                    for (const auto& e : edges) {
                        if (e.src == nextVertex && e.dest == vertex) {
                            logProfit += e.weight;
                            if (verbose) {
                                std::cout << graph.getCurrencyName(nextVertex) << " → " 
                                          << graph.getCurrencyName(vertex) 
                                          << " (rate: " << std::fixed << std::setprecision(6) 
                                          << exp(-e.weight) << ") ";
                            }
                            break;
                        }
                    }
                    
                    vertex = nextVertex;
                } while (vertex != cycleStart);
                
                // Reverse to get correct order
                std::reverse(cycle.begin(), cycle.end());
                
                // Calculate actual profit
                double actualProfit = exp(-logProfit) - 1.0;
                
                if (verbose) {
                    std::cout << "\n\n  Log profit: " << logProfit << std::endl;
                    std::cout << "  Actual profit: " << actualProfit * 100.0 << "%" << std::endl;
                    std::cout << "  Cycle in correct order: ";
                    for (size_t i = 0; i < cycle.size(); ++i) {
                        std::cout << graph.getCurrencyName(cycle[i]);
                        if (i < cycle.size() - 1) std::cout << " → ";
                    }
                    std::cout << " → " << graph.getCurrencyName(cycle[0]) << std::endl;
                }
                
                result.cycle = cycle;
                result.profit = actualProfit * 100.0; // as percentage
                
                return result;
            }
        }
    }
    
    if (verbose && !cycleFound) {
        std::cout << "  No negative cycles found. No arbitrage opportunities exist." << std::endl;
    }
    
    return result;
}