#pragma once
#include <string>
#include <mutex>
#include <fstream>

enum class LogLevel {
    INFO,
    WARN,
    ALERT,
    ERROR
};

class Logger {
public:
    static Logger& getInstance();

    void log(LogLevel level, const std::string& message);
    void info(const std::string& msg);
    void warn(const std::string& msg);
    void alert(const std::string& msg);
    void error(const std::string& msg);

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::mutex mtx_;
    std::ofstream logFile_;

    std::string levelToString(LogLevel level);
    std::string getTimestamp();
};
