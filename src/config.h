#pragma once

#include <iostream>
#include <shared_mutex>

#include <string>
#include <unordered_map>

namespace Betrock {

class GlobalConfig {
  private:
	struct TransparentHasher {
		using is_transparent = void;
		size_t operator()(std::string_view sv) const {
			std::hash<std::string_view> hasher;
			return hasher(sv);
		}
	};

	using ConfType = std::unordered_map<std::string, std::string, TransparentHasher, std::equal_to<>>;

  public:
	static GlobalConfig &Instance() {
		static GlobalConfig g;
		return g;
	}

	// get the value at key or a the default mapped_type if key doesn't exist
	std::string_view Get(const std::string &key) noexcept;

	std::string GetAsString(const std::string &key) { return std::string(this->Get(key)); }

	// get the value at key as number
	template <std::integral num_type> num_type GetAsNumber(const std::string &key) {
		return std::stoll(std::string(this->Get(key)));
	}

	// get the value at key as boolean
	bool GetAsBoolean(const std::string &key) {
		std::string val = std::string(this->Get(key));
		try {
			if (val == "true" || val == "1")
				return true;
			// All other cases default to false
			/*
			if (val == "false" || val == "0")
				return false;
			*/
		} catch (const std::exception &e) {
			std::cerr << "Error while writing: " << e.what() << "\n";
		}
		return false;
	}

	// set value at key.
	// will create key if it doesn't exist.
	void Set(const std::string &key, std::string_view value) noexcept;

	// overwrite the properties in memory
	void Overwrite(const ConfType &config) noexcept;

	// read a properties file from disk into memory.
	// returns false on error.
	bool LoadFromDisk() noexcept;

	// save the properties in memory to disk.
	// returns false on error.
	bool SaveToDisk() const noexcept;

	// set a new path to the properties file
	void SetPath(std::string_view path) noexcept;

	// get the current properties path
	std::string_view GetPath() const noexcept;

  private:
	GlobalConfig() = default;
	~GlobalConfig() = default;

	GlobalConfig(const GlobalConfig &) = delete;
	GlobalConfig(const GlobalConfig &&) = delete;

	GlobalConfig &operator=(const GlobalConfig &) = delete;
	GlobalConfig &operator=(const GlobalConfig &&) = delete;

	std::shared_mutex properties_mutex;

	std::string path = "server.properties";
	ConfType properties;
};
} // namespace Betrock
