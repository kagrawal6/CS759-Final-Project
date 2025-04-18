#include "ForexGraph.hpp"
#include <cmath>

// Edge constructor
Edge::Edge(int s, int d, double rate)
  : src(s), dest(d), weight(-std::log(rate))
{}

// ForexGraph methods
ForexGraph::ForexGraph() : V(0) {}

int ForexGraph::addCurrency(const std::string &currency) {
    if (currencyToId.find(currency) == currencyToId.end()) {
        currencyToId[currency] = V++;
        idToCurrency.push_back(currency);
    }
    return currencyToId[currency];
}

void ForexGraph::addExchangeRate(const std::string &from,
                                 const std::string &to,
                                 double bid,
                                 double ask)
{
    int src  = addCurrency(from);
    int dest = addCurrency(to);

    // 1/ask for buy, bid for sell
    edges.emplace_back(src,  dest, 1.0 / ask);
    edges.emplace_back(dest, src,  bid);
}

const std::vector<Edge> &ForexGraph::getEdges() const {
    return edges;
}

int ForexGraph::getVertexCount() const {
    return V;
}

const std::string &ForexGraph::getCurrencyName(int id) const {
    return idToCurrency[id];
}

int ForexGraph::getCurrencyId(const std::string &name) const {
    return currencyToId.at(name);
}
