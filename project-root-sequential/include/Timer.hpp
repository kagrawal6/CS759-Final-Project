// Timer.hpp
#pragma once
#include <string>
#include <chrono>
#include <map>
#include <vector>

class Timer {
public:
    // Use a steady, monotonic clock
    using Clock = std::chrono::steady_clock;

    // Start timing under this label
    explicit Timer(const std::string &label);

    // Auto-stop if needed
    ~Timer();

    // Manually stop (if you want)
    void stop();

    // Dump a summary of all timers to stdout
    static void report();

private:
    std::string               label_;
    Clock::time_point         start_;
    bool                      stopped_{false};

    // Store raw durations in nanoseconds
    static std::map<std::string, std::vector<int64_t>> records_;
};

