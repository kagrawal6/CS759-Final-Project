

// // Timer.cpp
// #include "Timer.hpp"
// #include <iostream>
// #include <iomanip>
// #include <numeric>
// #include <algorithm>

// // Initialize static member
// std::map<std::string, std::vector<double>> Timer::records_;

// Timer::Timer(const std::string &label)
//     : label_(label), start_time_(omp_get_wtime())
// {}

// Timer::~Timer() {
//     if (!stopped_) stop();
// }

// void Timer::stop() {
//     double end_time = omp_get_wtime();
//     double duration_ms = (end_time - start_time_) * 1000.0;
//     records_[label_].push_back(duration_ms);
//     stopped_ = true;
// }

// void Timer::report() {
//     std::cout << "\n===== TIMING SUMMARY =====\n"
//               << std::left << std::setw(30) << "Stage"
//               << std::right << std::setw(10) << "Count"
//               << std::setw(12) << "Total ms"
//               << std::setw(10) << "Avg ms"
//               << std::setw(10) << "Min ms"
//               << std::setw(10) << "Max ms"
//               << "\n" << std::string(80, '-') << "\n";

//     for (auto &entry : records_) {
//         const auto &label = entry.first;
//         const auto &times = entry.second;
//         int count = int(times.size());
//         double total = std::accumulate(times.begin(), times.end(), 0.0);
//         double avg = total / count;
//         auto [min_it, max_it] = std::minmax_element(times.begin(), times.end());

//         std::cout << std::left  << std::setw(30) << label
//                   << std::right << std::setw(10) << count
//                   << std::setw(12) << total
//                   << std::setw(10) << avg
//                   << std::setw(10) << *min_it
//                   << std::setw(10) << *max_it
//                   << "\n";
//     }
//     std::cout << std::endl;
// }



// Timer.cpp
#include "Timer.hpp"
#include <iostream>
#include <iomanip>
#include <numeric>
#include <algorithm>

// Initialize static record storage
std::map<std::string, std::vector<double>> Timer::records_;

Timer::Timer(const std::string &label)
    : label_(label), start_time_(omp_get_wtime())
{}

Timer::~Timer() {
    if (!stopped_) stop();
}

void Timer::stop() {
    double end_time = omp_get_wtime();
    double duration_ms = (end_time - start_time_) * 1000.0;
    records_[label_].push_back(duration_ms);
    stopped_ = true;
}

void Timer::report() {
    std::cout << "\n===== TIMING SUMMARY =====\n"
              << std::left << std::setw(30) << "Stage"
              << std::right << std::setw(10) << "Count"
              << std::setw(12) << "Total(ms)"
              << std::setw(12) << "Avg(ms)"
              << std::setw(12) << "Min(ms)"
              << std::setw(12) << "Max(ms)"
              << "\n" << std::string(90, '-') << "\n";

    for (auto &entry : records_) {
        const auto &label = entry.first;
        const auto &times = entry.second;
        int count = static_cast<int>(times.size());
        double total = std::accumulate(times.begin(), times.end(), 0.0);
        double avg   = total / count;
        auto [min_it, max_it] = std::minmax_element(times.begin(), times.end());

        std::cout << std::left  << std::setw(30) << label
                  << std::right << std::setw(10) << count
                  << std::fixed << std::setprecision(3)
                  << std::setw(12) << total
                  << std::setw(12) << avg
                  << std::setw(12) << *min_it
                  << std::setw(12) << *max_it
                  << "\n";
    }
    std::cout << std::endl;
}
