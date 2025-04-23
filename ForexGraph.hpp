#pragma once
#include <cmath>
#include <string>
#include <unordered_map>
#include <vector>

struct Edge {
  int src;
  int dest;
  double weight;

  // Negative log turns it into a search for negative cycles
  Edge(int s, int d, double rate) : src(s), dest(d), weight(-std::log(rate)) {}
};

class ForexGraph {
private:
  int V;                                             // number of vertices
  std::vector<Edge> edges;                           // list of edges
  std::unordered_map<std::string, int> currencyToId; // map currency to id
  std::vector<std::string> idToCurrency;             // map id to currency

public:
  ForexGraph() : V(0) {}

  // Add a currency to the graph and return its id
  int addCurrency(const std::string &currency) {
    if (currencyToId.find(currency) == currencyToId.end()) {
      currencyToId[currency] = V++;
      idToCurrency.push_back(currency);
    }
    return currencyToId[currency];
  }

  // Add an exchange rate between two currencies
  void addExchangeRate(const std::string &from, const std::string &to, double bid, double ask) {
    int src = addCurrency(from);
    int dest = addCurrency(to);

    // use 1 / ask for buying dest with src
    edges.emplace_back(src, dest, bid);

    // use bid for selling dest to get src
    edges.emplace_back(dest, src, 1.0 / ask);
  }

  const std::vector<Edge> &getEdges() const {
    return edges;
  }

  int getVertexCount() const {
    return V;
  }

  const std::string &getCurrencyName(int id) const {
    return idToCurrency[id];
  }

  int getCurrencyId(const std::string &name) const {
    return currencyToId.at(name);
  }
};
