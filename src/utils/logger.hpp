#ifndef HTTPS_SERVER_LOGGER_HPP
#define HTTPS_SERVER_LOGGER_HPP

#include <string>
#include <mutex>
#include <fstream>

namespace https_server {

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    Error = 3
};

class Logger {
public:
    static Logger& instance();
    
    void set_level(LogLevel level);
    void log(LogLevel level, const std::string& message);
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger() = default;
    
    std::mutex mutex_;
    LogLevel level_ = LogLevel::INFO;
    std::ofstream file_;
};

#define LOG_DEBUG(msg) https_server::Logger::instance().log(https_server::LogLevel::DEBUG, msg)
#define LOG_INFO(msg) https_server::Logger::instance().log(https_server::LogLevel::INFO, msg)
#define LOG_WARNING(msg) https_server::Logger::instance().log(https_server::LogLevel::WARNING, msg)
#define LOG_ERROR(msg) https_server::Logger::instance().log(https_server::LogLevel::Error, msg)

} // namespace https_server

#endif // HTTPS_SERVER_LOGGER_HPP