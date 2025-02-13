#include "logger.h"

// Reference for Escape Codes
// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797

namespace Betrock {
Logger::Logger(std::string filename) {
    logFile.open(filename, std::ios::out | std::ios::app);  
}

void Logger::Info(std::string message) {
    std::string header = "INFO";
    std::cout << "\x1b[1m"    << header << "\x1b[22m " << message << "\x1b[0m" << std::endl;
    logFile << header << " " << message << std::endl;
}
void Logger::Warning(std::string message) {
    std::string header = "WARNING";
    std::cout << "\x1b[1;33m" << header << "\x1b[22m " << message << "\x1b[0m" << std::endl;
    logFile << header << " " << message << std::endl;
}
void Logger::Danger(std::string message) {
    std::string header = "DANGER";
    std::cout << "\x1b[1;31m" << header << "\x1b[22m " << message << "\x1b[0m" << std::endl;
    logFile << header << " " << message << std::endl;
}
}