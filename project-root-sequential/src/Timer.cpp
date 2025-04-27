// Timer.cpp
#include "Timer.hpp"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <iomanip>

std::map<std::string, std::vector<int64_t>> Timer::records_;

Timer::Timer(const std::string &label)
  : label_(label), start_(Clock::now())
{}

Timer::~Timer() {
    if (!stopped_) stop();
}

void Timer::stop() {
    auto end = Clock::now();
    int64_t ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_).count();
    records_[label_].push_back(ns);
    stopped_ = true;
}

void Timer::report() {
    std::cout << "\n===== TIMING SUMMARY =====\n"
              << std::fixed << std::setprecision(3)
              << std::left << std::setw(30) << "Stage"
              << std::right << std::setw(8)  << "Count"
              << std::setw(12) << "Total ms"
              << std::setw(10) << "Avg ms"
              << std::setw(10) << "Min ms"
              << std::setw(10) << "Max ms"
              << "\n" << std::string(80, '-') << "\n";

    for (auto & [label, vec] : records_) {
        int cnt = int(vec.size());
        int64_t sum_ns = std::accumulate(vec.begin(), vec.end(), int64_t(0));
        auto [mn_it, mx_it] = std::minmax_element(vec.begin(), vec.end());
        double total_ms = sum_ns / 1e6;
        double avg_ms   = total_ms / cnt;
        double min_ms   = *mn_it / 1e6;
        double max_ms   = *mx_it / 1e6;

        std::cout << std::left  << std::setw(30) << label
                  << std::right << std::setw(8)  << cnt
                  << std::setw(12) << total_ms
                  << std::setw(10) << avg_ms
                  << std::setw(10) << min_ms
                  << std::setw(10) << max_ms
                  << "\n";
    }
    std::cout << std::endl;
}

