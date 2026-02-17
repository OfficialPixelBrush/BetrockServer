#include "sysinfo.h"
#include <sstream>

#ifdef _WIN32
    // Windows implementation

    int32_t GetUsedMemory() {
        // Returns memory usage in KB (Windows API)
        HANDLE hProcess = GetCurrentProcess();
        PROCESS_MEMORY_COUNTERS pmc;

        if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
            return static_cast<int32_t>(pmc.WorkingSetSize / 1024);
        }
        return -1;
    }

    double GetUsedMemoryMB(std::string text = "") {
        int32_t kb = GetUsedMemory();
        return (kb > 0) ? (double(kb) / 1024.0) : 0.0;
    }

    std::string GetUsedMemoryMBString() {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << GetUsedMemoryMB();
        return ss.str();
    }

    std::string GetDetailedMemoryMetricsMBString() {
        // Placeholder for Windows - can be expanded with more details
        return GetUsedMemoryMBString() + " MB";
    }

#else
    // POSIX/Linux implementation

    int32_t parseLine(char *line) {
        // This assumes that a digit will be found and the line ends in " Kb".
        int64_t i = int64_t(strlen(line));
        const char *p = line;
        while (*p < '0' || *p > '9')
            p++;
        line[i - 3] = '\0';
        i = atoi(p);
        return i;
    }

    int32_t GetUsedMemory() { // Note: this value is in KB!
        FILE *file = fopen("/proc/self/status", "r");
        int64_t result = -1;
        char line[128];

        while (fgets(line, 128, file) != NULL) {
            if (strncmp(line, "VmRSS:", 6) == 0) {
                result = parseLine(line);
                break;
            }
        }
        fclose(file);
        return result;
    }

    // Get the number of Megabytes used by BetrockServer
    double GetUsedMemoryMB(std::string text = "VmRSS:") {
        FILE *file = fopen("/proc/self/status", "r");
        int64_t result = -1;
        char line[128];

        while (fgets(line, 128, file) != NULL) {
            if (strncmp(line, text.c_str(), text.size()) == 0) {
                result = parseLine(line);
                break;
            }
        }
        fclose(file);
        return (double(result) / 1024.0);
    }

    std::string GetUsedMemoryMBString() {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2) << GetUsedMemoryMB("VmRSS:");
        return ss.str();
    }

    std::string GetDetailedMemoryMetricsMBString() {
        std::stringstream ss;
        ss << std::fixed << std::setprecision(2);
        ss << "Total: " << GetUsedMemoryMB("VmRSS:") << "MB, ";
        ss << "Data: " << GetUsedMemoryMB("VmData:") << "MB, ";
        ss << "Exe: " << GetUsedMemoryMB("VmExe:") << "MB, ";
        ss << "Stack: " << GetUsedMemoryMB("VmStk:") << "MB";
        return ss.str();
    }

#endif