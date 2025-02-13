#pragma once
#include <fstream>
#include <iostream>
#include <cstdint>
#include <string>

#include "loglevel.h"

namespace Betrock {
class Logger {
    private:
        int8_t logLevel = LOG_ALL;
        std::ofstream logFile;
    public:
        Logger(std::string filename = "log.txt");
        void Info(std::string message);
        void Warning(std::string message);
        void Danger(std::string message);
};
}