#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>
#include <string>

class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    std::string name;
    
public:
    Timer(const std::string& timer_name);
    ~Timer();
};

#endif // TIMER_HPP