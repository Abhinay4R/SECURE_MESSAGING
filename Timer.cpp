#include "Timer.hpp"
#include <iostream>

Timer::Timer(const std::string& timer_name) : name(timer_name) {
    start_time = std::chrono::high_resolution_clock::now();
}

Timer::~Timer() {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time);
    std::cout << name << ": " << duration.count() << " ns" << std::endl;
}