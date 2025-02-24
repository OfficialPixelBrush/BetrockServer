#include "logger.h"

// Reference for Escape Codes
// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797

namespace Betrock {

void Logger::Log(std::string message, int level) {
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

void Logger::ChatMessage(std::string message) {
    if (logLevelTerminal & LOG_CHAT) 
    {
        std::cout << HandleFormattingCodes(message) << std::endl;
    }
    if (logLevelText & LOG_CHAT) {
        logFile << message << std::endl;
    }
}

void Logger::Message(std::string message) {
    if (logLevelTerminal & LOG_MESSAGE) 
    {
        std::cout << message << std::endl;
    }
    if (logLevelText & LOG_MESSAGE) {
        logFile << message << std::endl;
    }
}

void Logger::Info(std::string message) {
    std::string header = "[INFO]";
    if (logLevelTerminal & LOG_INFO) 
    {
        std::cout << "\x1b[1;107m" << header << "\x1b[0m " << message << std::endl;
    }
    if (logLevelText & LOG_INFO) {
        logFile << header << " " << message << std::endl;
    }
}

void Logger::Warning(std::string message) {
    std::string header = "[WARNING]";
    if (logLevelTerminal & LOG_WARNING) 
    {
        std::cerr << "\x1b[1;43m" << header << "\e[0;33m " << message << "\x1b[0m " << std::endl;
    }
    if (logLevelText & LOG_WARNING) {
        logFile << header << " " << message << std::endl;
    }
}

void Logger::Error(std::string message) {
    std::string header = "[ERROR]";
    if (logLevelTerminal & LOG_ERROR) 
    {
        std::cerr << "\x1b[1;101m" << header << " " << message << "\x1b[0m" << std::endl;
    }
    if (logLevelText & LOG_ERROR) {
        logFile << header << " " << message << std::endl;
    }
}

void Logger::Debug(std::string message) {
    std::string header = "[DEBUG]";
    if (logLevelTerminal & LOG_DEBUG) 
    {
        std::cerr << "\x1b[1;46m" << header << "\x1b[0m " << message << std::endl;
    }
    if (logLevelText & LOG_DEBUG) {
        logFile << header << " " << message << std::endl;
    }
}
}