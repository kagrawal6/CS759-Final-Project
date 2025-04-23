#pragma once

#include <string>
#include <vector>
#include <unordered_map>

/// Single directed edge in the FX graph, weight = –log(rate)
struct Edge {
    int   src;
    int   dest;
    double weight;

    /// Construct an edge from src→dest with given “rate” (we store –log(rate))
    Edge(int s, int d, double rate);
};

/// A graph whose vertices are currencies and whose edges encode FX bid/ask rates
class ForexGraph {
public:
    ForexGraph();
	/// Reserve space in the internal edge array to avoid reallocations
	void reserveEdges(size_t n) { edges.reserve(n); }
    /// Add a currency if missing, return its internal ID
    int addCurrency(const std::string &currency);

    /// Add two directed edges for the pair of rates (bid and 1/ask)
    void addExchangeRate(const std::string &from,
                         const std::string &to,
                         double bid,
                         double ask);

    /// All edges in the graph
    const std::vector<Edge> &getEdges() const;

    /// Number of distinct currencies
    int getVertexCount() const;

    /// Look up currency name by its internal ID
    const std::string &getCurrencyName(int id) const;

    /// Look up internal ID by currency code (throws if not found)
    int getCurrencyId(const std::string &name) const;

private:
    int V;    // next vertex ID
    std::vector<Edge> edges;
    std::unordered_map<std::string,int> currencyToId;
    std::vector<std::string> idToCurrency;
};
