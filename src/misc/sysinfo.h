#pragma once
#include "sys/types.h"
#include "sys/sysinfo.h"
#include <fstream>
#include <cstring>
#include <iomanip>

extern int GetUsedMemory();
extern double GetUsedMemoryMB();
extern std::string GetUsedMemoryMBString();