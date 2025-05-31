#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace InMemoryDB {

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

class Logger {
private:
    static Logger* instance_;
    std::ofstream log_file_;
    LogLevel min_level_;

public:
    static Logger* getInstance() {
        if (!instance_) {
            instance_ = new Logger();
        }
        return instance_;
    }
    
    Logger() : min_level_(LogLevel::INFO) {
        log_file_.open("plsql_db.log", std::ios::app);
    }
    
    ~Logger() {
        if (log_file_.is_open()) {
            log_file_.close();
        }
    }
    
    void log(LogLevel level, const std::string& message) {
        if (level < min_level_) return;
        
        std::string timestamp = getCurrentTimestamp();
        std::string level_str = levelToString(level);
        
        std::string log_entry = "[" + timestamp + "] [" + level_str + "] " + message;
        
        // Log to console
        std::cout << log_entry << std::endl;
        
        // Log to file
        if (log_file_.is_open()) {
            log_file_ << log_entry << std::endl;
            log_file_.flush();
        }
    }
    
    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warning(const std::string& message) { log(LogLevel::WARNING, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }
    
private:
    std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
        return ss.str();
    }
    
    std::string levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }
};

Logger* Logger::instance_ = nullptr;

}