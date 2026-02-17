#pragma once

#ifdef _WIN32
    // Windows system information headers
    #include <windows.h>
    #include <psapi.h>
    #pragma comment(lib, "psapi.lib")
#else
    // POSIX system information headers
    #include "sys/sysinfo.h"
    #include "sys/types.h"
#endif

#include <cstring>
#include <fstream>
#include <iomanip>

extern int32_t GetUsedMemory();
extern double GetUsedMemoryMB();
extern std::string GetUsedMemoryMBString();
extern std::string GetDetailedMemoryMetricsMBString();