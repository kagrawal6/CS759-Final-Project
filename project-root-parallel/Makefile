# Makefile for Forex Arbitrage Project

# Compiler and flags
CXX := g++
CXXFLAGS := -std=c++17 -O3 -Wall -Wextra -Iinclude -fopenmp

# Source files
SRC := \
    main.cpp \
    src/ArbitrageDetector.cpp \
    src/CsvParser.cpp \
    src/PositionsManager.cpp \
    src/TimeSeriesArbitrageDetector.cpp \
    src/Timer.cpp \
    src/ForexGraph.cpp

# Object files
OBJ := $(SRC:.cpp=.o)

OBJ_NOMAIN := $(filter-out main.o,$(OBJ))

# Final executable
TARGET := arbitrage

.PHONY: all run-seq run-parallel clean

all: $(TARGET)

# Link step
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compile step
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run sequentially (1 thread)
run-seq: all
	@echo "Running sequential (1 thread)"
	./$(TARGET) 1

# Run in parallel (default 4 threads, override with PARALLEL_THREADS)
PARALLEL_THREADS ?= 4
run-parallel: all
	@echo "Running parallel ($(PARALLEL_THREADS) threads)"
	./$(TARGET) $(PARALLEL_THREADS)

# Clean up build artifacts
clean:
	rm -f $(OBJ) $(TARGET)

# Benchmark target:
benchmark: benchmark.cpp $(OBJ_NOMAIN)
	$(CXX) $(CXXFLAGS) -o $@ benchmark.cpp $(OBJ_NOMAIN) $(LDFLAGS)

.PHONY: bench-csv
bench-csv: benchmark
	@echo "Cleaning up old results…"
	@rm -f benchmark_results.csv
	@echo "Running benchmark and writing to benchmark_results.csv…"
	@./benchmark > benchmark_results.csv
	@echo "Done."

.PHONY: bench
bench: benchmark
	@echo "Running benchmark…"
	@./benchmark
