#include "logger.h"

#include "server.h"

// Reference for Escape Codes
// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797

namespace Betrock {

Logger::Logger() {
    if (logLevelText != LOG_NONE) {
        logFile.open(GetRealTimeFileFormat() + ".log", std::ios::out | std::ios::app);
        if (!logFile.is_open()) {
            throw std::runtime_error("Failed to open log file");
        }
    }
}

// Log a message with the passed Level
void Logger::Log(std::string message, int32_t level) {
    switch(level) {
        case LOG_CHAT:
            ChatMessage(message);
            break;
        case LOG_INFO:
            Info(message);
            break;
        case LOG_WARNING:
            Warning(message);
            break;
        case LOG_ERROR:
            Error(message);
            break;
        default:
            Message(message);
            break;
    }
}

// Log a chat message
void Logger::ChatMessage(std::string message) {
    std::string time = GetRealTime();
    if (logLevelTerminal & LOG_CHAT) 
    {
        std::cout << time << " " << HandleFormattingCodes(message) << "\n";
    }
    if (logLevelText & LOG_CHAT) {
        logFile << time << " " << message << "\n";
    }
}

// Log a message without a header
void Logger::Message(std::string message) {
    std::string time = GetRealTime();
    if (logLevelTerminal & LOG_MESSAGE) 
    {
        std::cout << time << " " << message << "\n";
    }
    if (logLevelText & LOG_MESSAGE) {
        logFile << time << " " << message << "\n";
    }
}

// Log a message with an INFO header
void Logger::Info(std::string message) {
    std::string time = GetRealTime();
    std::string header = "[INFO]";
    if (logLevelTerminal & LOG_INFO) 
    {
        std::cout << time << " \x1b[1;107m" << header << "\x1b[0m " << message << "\n";
    }
    if (logLevelText & LOG_INFO) {
        logFile << time << " " << header << " " << message << "\n";
    }
}

// Log a warning
void Logger::Warning(std::string message) {
    std::string time = GetRealTime();
    std::string header = "[WARNING]";
    if (logLevelTerminal & LOG_WARNING) 
    {
        std::cerr << time << " \x1b[1;43m" << header << "\e[0;33m " << message << "\x1b[0m " << "\n";
    }
    if (logLevelText & LOG_WARNING) {
        logFile << time << " " << header << " " << message << "\n";
    }
}

// Log an error
void Logger::Error(std::string message) {
    std::string time = GetRealTime();
    std::string header = "[ERROR]";
    if (logLevelTerminal & LOG_ERROR) 
    {
        std::cerr << time << " \x1b[1;101m" << header << " " << message << "\x1b[0m" << "\n";
    }
    if (logLevelText & LOG_ERROR) {
        logFile << time << " " << header << " " << message << "\n";
    }
}

// Log Debug Data
void Logger::Debug(std::string message) {
    std::string time = GetRealTime();
    std::string header = "[DEBUG]";
    if (logLevelTerminal & LOG_DEBUG) 
    {
        std::cerr << time << " \x1b[1;46m" << header << "\x1b[0m " << message << "\n";
    }
    if (logLevelText & LOG_DEBUG) {
        logFile << time << " " << header << " " << message << "\n";
    }
}
}