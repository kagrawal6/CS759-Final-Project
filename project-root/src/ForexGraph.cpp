// src/ForexGraph.cpp
#include "ForexGraph.hpp"

int ForexGraph::addCurrency(const std::string &currency) {
    auto it = currencyToId.find(currency);
    if (it == currencyToId.end()) {
        currencyToId[currency] = V;
        idToCurrency.push_back(currency);
        return V++;
    }
    return it->second;
}

void ForexGraph::addExchangeRate(const std::string &from,
                                 const std::string &to,
                                 double bid,
                                 double ask)
{
    int u = addCurrency(from);
    int v = addCurrency(to);

    // buy 'to' with 'from' at 1/ask
    edges.emplace_back(u, v, 1.0/ask);

    // sell 'to' back to 'from' at bid
    edges.emplace_back(v, u, bid);
}
