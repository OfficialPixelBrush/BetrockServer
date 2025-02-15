#pragma once
#include <fstream>
#include <iostream>
#include <cstdint>
#include <string>

#include "loglevel.h"

namespace Betrock {
class Logger {
    private:
        std::ofstream logFile;
        int8_t logLevel = LOG_ALL;

        Logger() {
            logFile.open("logfile.log", std::ios::out | std::ios::app);
            if (!logFile.is_open()) {
                throw std::runtime_error("Failed to open log file");
            }
        }
        
        ~Logger() {
            if (logFile.is_open()) {
                logFile.close();
            }
        }

        Logger(const Logger &) = delete;
        Logger(const Logger &&) = delete;
        Logger &operator=(const Logger &) = delete;
        Logger &operator=(const Logger &&) = delete;
    public:
        static Logger &Instance() {
            // this will create the server instance just once and just return a
            // reference to the object instead of creating a new one
            static Logger instance;
            return instance;
        }
        void Log(std::string message, int level = LOG_MESSAGE);
        void Message(std::string message);
        void Info(std::string message);
        void Warning(std::string message);
        void Error(std::string message);
        void SetLogLevel(int8_t logLevel = LOG_ALL) { this->logLevel = logLevel; }
};
}