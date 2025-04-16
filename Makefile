CXX = g++
CXXFLAGS = -std=c++17 -Wall -O3

all: arbitrage

arbitrage: main.cpp ForexGraph.hpp CsvParser.hpp ArbitrageDetector.hpp TimeSeriesArbitrageDetector.hpp
	$(CXX) $(CXXFLAGS) -o arbitrage main.cpp

clean:
	rm -f arbitrage

run: arbitrage
	./arbitrage
