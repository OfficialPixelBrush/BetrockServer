#pragma once
#include "sys/sysinfo.h"
#include "sys/types.h"
#include <cstring>
#include <fstream>
#include <iomanip>

extern int GetUsedMemory();
extern double GetUsedMemoryMB();
extern std::string GetUsedMemoryMBString();
extern std::string GetDetailedMemoryMetricsMBString();