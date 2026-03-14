#pragma once

#include <chrono>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>

namespace color {
    constexpr const char* reset   = "\033[0m";
    constexpr const char* bold    = "\033[1m";
    constexpr const char* dim     = "\033[2m";
    constexpr const char* red     = "\033[31m";
    constexpr const char* green   = "\033[32m";
    constexpr const char* yellow  = "\033[33m";
    constexpr const char* blue    = "\033[34m";
    constexpr const char* magenta = "\033[35m";
    constexpr const char* cyan    = "\033[36m";
    constexpr const char* gray    = "\033[90m";
    constexpr const char* bred    = "\033[91m";
    constexpr const char* bgreen  = "\033[92m";
    constexpr const char* byellow = "\033[93m";
    constexpr const char* bblue   = "\033[94m";
    constexpr const char* bcyan   = "\033[96m";
}

inline std::string timestamp() {
    using namespace std::chrono;
    auto now = system_clock::now();
    auto t   = system_clock::to_time_t(now);
    auto ms  = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    std::ostringstream ss;
    ss << std::put_time(std::localtime(&t), "%H:%M:%S")
       << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

inline const char* method_color(std::string_view method) {
    if (method == "GET")    return color::bgreen;
    if (method == "POST")   return color::byellow;
    if (method == "PUT")    return color::bblue;
    if (method == "DELETE") return color::bred;
    if (method == "PATCH")  return color::bcyan;
    return color::bold;
}

inline const char* status_color(unsigned code) {
    if (code < 300) return color::bgreen;
    if (code < 400) return color::byellow;
    if (code < 500) return color::bred;
    return color::red;
}

// Mutex global para output thread-safe no console
inline std::mutex& print_mutex() {
    static std::mutex mtx;
    return mtx;
}
