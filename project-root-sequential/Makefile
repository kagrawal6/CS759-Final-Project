# Makefile (place in project-root/)

# Compiler
CXX       := g++
CXXFLAGS  := -std=c++17 -O3 -Wall -Wextra -Iinclude -fopenmp

# Source directories
SRC_DIR   := src
INC_DIR   := include

# All .cpp files (main.cpp + everything in src/)
SRCS      := main.cpp $(wildcard $(SRC_DIR)/*.cpp)

# Corresponding .o files
OBJS      := $(SRCS:.cpp=.o)

# Final executable
TARGET    := arbitrage

.PHONY: all run clean

all: $(TARGET)

# Link step
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compile any .cpp → .o
# This rule works both for main.cpp and src/*.cpp
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the program (expects to be run from project-root/)
run: $(TARGET)
	./$(TARGET)

# Clean up objects and binary
clean:
	rm -f $(OBJS) $(TARGET)
