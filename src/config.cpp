#include "config.h"

#include <fstream>
#include <iostream>
#include <mutex>

namespace Betrock {

std::string_view GlobalConfig::Get(const std::string &key) noexcept {
	std::shared_lock read_lock{this->properties_mutex};
	return this->properties.contains(key) ? this->properties.at(key) : std::string_view();
}

void GlobalConfig::Set(const std::string &key, std::string_view value) noexcept {
	std::unique_lock write_lock{this->properties_mutex};
	this->properties[key] = value;
}

// overwrite the properties in memory
void GlobalConfig::Overwrite(const ConfType &config) noexcept {
	std::unique_lock write_lock{this->properties_mutex};
	this->properties = config;
}

bool GlobalConfig::LoadFromDisk() noexcept {
	std::ifstream file(this->path);

	if (!file.is_open()) {
		std::cerr << "Error opening properties file (load).\n";
		return false;
	}

	std::unique_lock lock{this->properties_mutex};

	this->properties.clear();

	std::string line;
	while (std::getline(file, line)) {
		// Skip empty lines and comments
		if (line.empty() || line[0] == '#')
			continue;

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

	return true;
}

bool GlobalConfig::SaveToDisk() const noexcept {
	std::ofstream file(this->path);
	if (!file.is_open()) {
		std::cerr << "Error opening properties file (save).\n";
		return false;
	}

	try {
		for (const auto &[key, value] : this->properties) {
			file << key << "=" << value << "\n";
		}
	} catch (const std::exception &e) {
		std::cerr << "Error while writing: " << e.what() << "\n";
		return false;
	}

	std::cerr << "wrote to properties file\n";
	return true;
}

void GlobalConfig::SetPath(std::string_view path) noexcept { this->path = path; }

std::string_view GlobalConfig::GetPath() const noexcept { return this->path; }
} // namespace Betrock
