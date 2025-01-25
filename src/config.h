#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <filesystem>

extern std::unordered_map<std::string, std::string> properties;

void CreateDefaultProperties(const std::string& filename, const std::unordered_map<std::string, std::string>& defaultValues);
std::unordered_map<std::string, std::string> ReadPropertiesFile(const std::string& filename);
void WritePropertiesFile(const std::string& filename, const std::unordered_map<std::string, std::string>& properties);