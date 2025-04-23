#include "Timer.hpp"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <iomanip>

std::map<std::string, std::vector<double>> Timer::records_;

Timer::Timer(const std::string& label)
  : label_(label), start_(std::chrono::high_resolution_clock::now())
{}

void Timer::stop() {
    auto end = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(end - start_).count();
    records_[label_].push_back(ms);
}

void Timer::report() {
    std::cout << "\n===== TIMING SUMMARY =====\n"
              << std::left << std::setw(30) << "Stage"
              << std::right << std::setw(8) << "Count"
              << std::setw(12) << "Total ms"
              << std::setw(12) << "Avg ms"
              << std::setw(12) << "Min ms"
              << std::setw(12) << "Max ms"
              << "\n"
              << std::string(86, '-') << "\n";

    for (auto& [label, vec] : records_) {
        int   cnt   = static_cast<int>(vec.size());
        double total = std::accumulate(vec.begin(), vec.end(), 0.0);
        double avg   = total / cnt;
        double mn    = *std::min_element(vec.begin(), vec.end());
        double mx    = *std::max_element(vec.begin(), vec.end());

        std::cout << std::left << std::setw(30) << label
                  << std::right << std::setw(8)  << cnt
                  << std::setw(12) << std::fixed << std::setprecision(2) << total
                  << std::setw(12) << avg
                  << std::setw(12) << mn
                  << std::setw(12) << mx
                  << "\n";
    }
    std::cout << std::endl;
}
