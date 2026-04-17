#include "logger.h"
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

Logger::Logger() {
    logFile_.open("mcms_system.log", std::ios::app);
    log(LogLevel::INFO, "=== MCMS Logger Initialized ===");
}

Logger::~Logger() {
    log(LogLevel::INFO, "=== MCMS Logger Shutdown ===");
    if (logFile_.is_open()) logFile_.close();
}

std::string Logger::getTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_info;
    localtime_r(&t, &tm_info);
    std::ostringstream oss;
    oss << std::put_time(&tm_info, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string Logger::levelToString(LogLevel level) {
    switch(level) {
        case LogLevel::INFO:  return "INFO ";
        case LogLevel::WARN:  return "WARN ";
        case LogLevel::ALERT: return "ALERT";
        case LogLevel::ERROR: return "ERROR";
        default:              return "UNKNOWN";
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mtx_);
    std::string entry = "[" + getTimestamp() + "] [" + levelToString(level) + "] " + message;

    switch(level) {
        case LogLevel::INFO:  std::cout << "\033[32m" << entry << "\033[0m\n"; break;
        case LogLevel::WARN:  std::cout << "\033[33m" << entry << "\033[0m\n"; break;
        case LogLevel::ALERT: std::cout << "\033[35m" << entry << "\033[0m\n"; break;
        case LogLevel::ERROR: std::cout << "\033[31m" << entry << "\033[0m\n"; break;
    }

    if (logFile_.is_open()) logFile_ << entry << "\n";
    logFile_.flush();
}

void Logger::info(const std::string& msg)  { log(LogLevel::INFO,  msg); }
void Logger::warn(const std::string& msg)  { log(LogLevel::WARN,  msg); }
void Logger::alert(const std::string& msg) { log(LogLevel::ALERT, msg); }
void Logger::error(const std::string& msg) { log(LogLevel::ERROR, msg); }
