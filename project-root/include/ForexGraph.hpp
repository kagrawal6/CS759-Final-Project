// include/ForexGraph.hpp
#pragma once

#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

// An edge in the FX-graph: weight = –log(rate)
struct Edge {
    int src;
    int dest;
    double weight;
    Edge(int s, int d, double rate)
      : src(s), dest(d), weight(-std::log(rate)) {}
};

class ForexGraph {
private:
    int V = 0;
    std::vector<Edge> edges;
    std::unordered_map<std::string,int> currencyToId;
    std::vector<std::string>    idToCurrency;

public:
    ForexGraph() = default;

    // Add currency if new, return its integer ID
    int addCurrency(const std::string &currency);

    // Add both directions of the FX pair (ask & bid)
    void addExchangeRate(const std::string &from,
                         const std::string &to,
                         double bid,
                         double ask);

    const std::vector<Edge>& getEdges() const noexcept { return edges; }
    int getVertexCount()           const noexcept { return V;     }
    const std::string& getCurrencyName(int id) const { return idToCurrency[id]; }
    int getCurrencyId(const std::string &name)   const { return currencyToId.at(name); }
};
