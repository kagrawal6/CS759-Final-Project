
// // Timer.hpp
// #pragma once

// #include <string>
// #include <map>
// #include <vector>
// #include <omp.h>

// /// High-resolution timer that uses OpenMP wall-clock time
// /// Records durations (in ms) per label across all threads
// class Timer {
// public:
//     /// Start a timer under the given label
//     explicit Timer(const std::string &label);
//     ~Timer();

//     /// Stop the timer early (if needed)
//     void stop();

//     /// Print a summary of all recorded timers
//     static void report();

// private:
//     std::string                           label_;
//     double                                start_time_;  // seconds
//     bool                                  stopped_ = false;

//     /// Shared storage: label → list of durations (ms)
//     static std::map<std::string, std::vector<double>> records_;
// };
// Timer.hpp
#pragma once

#include <string>
#include <map>
#include <vector>
#include <omp.h>

/// High-resolution timer using OpenMP wall-clock times
/// Records durations in milliseconds per label across all threads
class Timer {
public:
    /// Construct and start a timer for the given label
    explicit Timer(const std::string &label);
    ~Timer();

    /// Stop the timer early and record duration
    void stop();

    /// Print a detailed summary of all recorded timers
    static void report();

private:
    std::string                                  label_;      ///< Label for this timer
    double                                       start_time_; ///< Start time (seconds)
    bool                                         stopped_ = false;

    /// Shared storage mapping each label to its recorded durations (ms)
    static std::map<std::string, std::vector<double>> records_;
};