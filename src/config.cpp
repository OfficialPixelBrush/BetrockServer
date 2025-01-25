#include "config.h"

std::unordered_map<std::string, std::string> properties;

void CreateDefaultProperties(const std::string& filename, const std::unordered_map<std::string, std::string>& defaultValues) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error creating default properties file.\n";
        return;
    }

    for (const auto& [key, value] : defaultValues) {
        file << key << "=" << value << "\n";
    }
    file.close();
    std::cout << "Default properties file created.\n";
}


std::unordered_map<std::string, std::string> ReadPropertiesFile(const std::string& filename) {
    std::unordered_map<std::string, std::string> properties;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening properties file.\n";
        return properties;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') continue;

        auto delimiterPos = line.find('=');
        if (delimiterPos == std::string::npos) {
            std::cerr << "Invalid line: " << line << "\n";
            continue;
        }

        std::string key = line.substr(0, delimiterPos);
        std::string value = line.substr(delimiterPos + 1);

        // Trim whitespace (optional)
        key.erase(key.find_last_not_of(" \t\n\r\f\v") + 1);
        value.erase(0, value.find_first_not_of(" \t\n\r\f\v"));

        properties[key] = value;
    }

    file.close();
    return properties;
}

void WritePropertiesFile(const std::string& filename, const std::unordered_map<std::string, std::string>& properties) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error writing to properties file.\n";
        return;
    }

    for (const auto& [key, value] : properties) {
        file << key << "=" << value << "\n";
    }
    file.close();
    std::cout << "Properties file updated.\n";
}