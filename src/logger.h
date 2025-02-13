#pragma once
#include <fstream>
#include <iostream>
#include <cstdint>
#include <string>

#include "loglevel.h"

namespace Betrock {
class Logger {
    private:
        Logger() = default;
        ~Logger() = default;

        // =====================================================
        // delete copy and move constructor and assignment operator to be sure no one
        // can create a second Logger instance
        // (deleting is basically making them unavailable)

        Logger(const Logger &) = delete;
        Logger(const Logger &&) = delete;

        Logger &operator=(const Logger &) = delete;
        Logger &operator=(const Logger &&) = delete;

	    // =====================================================

        int8_t logLevel = LOG_ALL;
        std::ofstream logFile;
    public:
        static Logger &Instance() {
            // this will create the server instance just once and just return a
            // reference to the object instead of creating a new one
            static Logger instance;
            return instance;
        }
        Logger(std::string filename = "log.txt");
        void Info(std::string message);
        void Warning(std::string message);
        void Danger(std::string message);
};
}