#pragma once

#include <iostream>
#include <iomanip>
#include <ctime>

#define Log(x) std::cout << GetTimeF() << x << '\n'
#define LogInline(x) std::cout << x

// Get formatted timestamp
inline std::string GetTimeF() {
    std::time_t t = time(NULL);
    std::tm tm = *std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "[ %H:%M:%S ] ");

    return oss.str();
}