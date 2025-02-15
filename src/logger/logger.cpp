#include "logger.h"

// Reference for Escape Codes
// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797

namespace Betrock {

void Logger::Log(std::string message, int logLevel) {
    switch(logLevel) {
        case LOG_INFO:
            Info(message);
            break;
        case LOG_WARNING:
            Warning(message);
            break;
        case LOG_DANGER:
            Danger(message);
            break;
        default:
            Message(message);
            break;
    }
}

void Logger::Message(std::string message) {
    std::cout << message << std::endl;
    logFile << message << std::endl;
}

void Logger::Info(std::string message) {
    std::string header = "INFO";
    std::cout << "\x1b[1;107m" << header << "\x1b[0m " << message << std::endl;
    logFile << header << " " << message << std::endl;
}

void Logger::Warning(std::string message) {
    std::string header = "WARNING";
    std::cerr << "\x1b[1;43m" << header << "\e[0;33m " << message << "\x1b[0m " << std::endl;
    logFile << header << " " << message << std::endl;
}
void Logger::Danger(std::string message) {
    std::string header = "DANGER";
    std::cerr << "\x1b[1;101m" << header << " " << message << "\x1b[0m " << std::endl;
    logFile << header << " " << message << std::endl;
}
}