#pragma once
#include <string>
#include <chrono>
#include <map>
#include <vector>

class Timer {
public:
    // Start a timer with the given label
    Timer(const std::string& label);

    // Stop this timer and record the elapsed duration
    void stop();

    // Print a summary of **all** timers to stdout
    static void report();

private:
    std::string label_;
    std::chrono::high_resolution_clock::time_point start_;

    // store all durations (ms) for each label
    static std::map<std::string, std::vector<double>> records_;
};
