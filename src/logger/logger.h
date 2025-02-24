#pragma once
#include <fstream>
#include <iostream>
#include <cstdint>
#include <string>
#include <chrono>
#include <iomanip>

#include "loglevel.h"
#include "style.h"

namespace Betrock {
class Logger {
    private:
        std::ofstream logFile;
        int8_t logLevelText = LOG_ALL;
        int8_t logLevelTerminal = LOG_ALL;

        Logger();
        
        ~Logger() {
            if (logFile.is_open()) {
                logFile.close();
            }
        }

        Logger(const Logger &) = delete;
        Logger(const Logger &&) = delete;
        Logger &operator=(const Logger &) = delete;
        Logger &operator=(const Logger &&) = delete;
        void Log(std::string message, int level = LOG_MESSAGE);
    public:
        static Logger &Instance() {
            // this will create the server instance just once and just return a
            // reference to the object instead of creating a new one
            static Logger instance;
            return instance;
        }
        void ChatMessage(std::string message);
        void Message(std::string message);
        void Info(std::string message);
        void Warning(std::string message);
        void Error(std::string message);
        void Debug(std::string message);
        void SetLogLevelTerminal(int8_t logLevel = LOG_ALL) { this->logLevelTerminal = logLevel; }
        void SetLogLevelText(int8_t logLevel = LOG_ALL) { this->logLevelText = logLevel; }
        void SetLogLevel(int8_t logLevel = LOG_ALL) {
            SetLogLevelText(logLevel);
            SetLogLevelTerminal(logLevel);
        }
};
}