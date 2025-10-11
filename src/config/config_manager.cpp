#include "config_manager.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

// toml11 is header-only, will be fetched by CMake
#define TOML11_COLORIZE_ERROR_MESSAGE
#include <toml.hpp>

ConfigManager::ConfigManager(const std::string& configPath) : configPath(configPath), showHelp(false) {
    loadTomlConfig();
}

void ConfigManager::loadTomlConfig() {
    try {
        // Check if file exists
        std::ifstream file(configPath);
        if (!file.good()) {
            std::cerr << "Warning: Config file '" << configPath << "' not found. Using defaults.\n";
            return;
        }

        const auto data = toml::parse(configPath);

        // Load allocator section
        if (data.contains("allocator")) {
            const auto& allocator = toml::find(data, "allocator");
            if (allocator.contains("min_order")) {
                configValues["min-order"] = std::to_string(toml::find<int>(allocator, "min_order"));
            }
            if (allocator.contains("max_order")) {
                configValues["max-order"] = std::to_string(toml::find<int>(allocator, "max_order"));
            }
            if (allocator.contains("alignment")) {
                configValues["alignment"] = std::to_string(toml::find<int>(allocator, "alignment"));
            }
        }

        // Load testing section
        if (data.contains("testing")) {
            const auto& testing = toml::find(data, "testing");
            if (testing.contains("num_operations")) {
                configValues["ops"] = std::to_string(toml::find<int>(testing, "num_operations"));
            }
            if (testing.contains("duration_seconds")) {
                configValues["duration"] = std::to_string(toml::find<double>(testing, "duration_seconds"));
            }
            if (testing.contains("random_seed")) {
                configValues["seed"] = std::to_string(toml::find<int>(testing, "random_seed"));
            }
            if (testing.contains("threads")) {
                configValues["threads"] = std::to_string(toml::find<int>(testing, "threads"));
            }
        }

        // Load output section
        if (data.contains("output")) {
            const auto& output = toml::find(data, "output");
            if (output.contains("directory")) {
                configValues["out"] = toml::find<std::string>(output, "directory");
            }
            if (output.contains("format")) {
                configValues["format"] = toml::find<std::string>(output, "format");
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Warning: Failed to parse config file '" << configPath << "': " << e.what() << "\n";
        std::cerr << "Continuing with defaults.\n";
    }
}

void ConfigManager::parseCLI(int argc, char* argv[], const std::string& programName,
                             const std::string& description) {
    cxxopts::Options options(programName, description);

    options.add_options()("config", "Path to TOML config file", cxxopts::value<std::string>()->default_value("config/default.toml"))(
        "min-order", "Minimum buddy order (2^min-order bytes)", cxxopts::value<size_t>())(
        "max-order", "Maximum buddy order (2^max-order bytes)", cxxopts::value<size_t>())(
        "min-block", "Minimum block size in bytes (alternative to min-order)", cxxopts::value<size_t>())(
        "max-block", "Maximum block size in bytes (alternative to max-order)", cxxopts::value<size_t>())(
        "alignment", "Memory alignment in bytes", cxxopts::value<size_t>())(
        "threads", "Number of threads for multi-threaded tests", cxxopts::value<size_t>())(
        "ops", "Number of operations", cxxopts::value<size_t>())(
        "duration", "Test duration in seconds", cxxopts::value<double>())(
        "seed", "Random seed for reproducibility", cxxopts::value<size_t>())(
        "out", "Output directory or file path", cxxopts::value<std::string>())(
        "format", "Output format (csv or json)", cxxopts::value<std::string>())("h,help", "Print help");

    try {
        auto result = options.parse(argc, argv);

        if (result.count("help")) {
            showHelp = true;
            helpMessage = options.help();
            return;
        }

        // Store CLI values (they override config file)
        if (result.count("min-order")) {
            cliValues["min-order"] = std::to_string(result["min-order"].as<size_t>());
        }
        if (result.count("max-order")) {
            cliValues["max-order"] = std::to_string(result["max-order"].as<size_t>());
        }
        if (result.count("min-block")) {
            cliValues["min-block"] = std::to_string(result["min-block"].as<size_t>());
        }
        if (result.count("max-block")) {
            cliValues["max-block"] = std::to_string(result["max-block"].as<size_t>());
        }
        if (result.count("alignment")) {
            cliValues["alignment"] = std::to_string(result["alignment"].as<size_t>());
        }
        if (result.count("threads")) {
            cliValues["threads"] = std::to_string(result["threads"].as<size_t>());
        }
        if (result.count("ops")) {
            cliValues["ops"] = std::to_string(result["ops"].as<size_t>());
        }
        if (result.count("duration")) {
            cliValues["duration"] = std::to_string(result["duration"].as<double>());
        }
        if (result.count("seed")) {
            cliValues["seed"] = std::to_string(result["seed"].as<size_t>());
        }
        if (result.count("out")) {
            cliValues["out"] = result["out"].as<std::string>();
        }
        if (result.count("format")) {
            cliValues["format"] = result["format"].as<std::string>();
        }

        // If a different config file was specified, reload it
        if (result.count("config")) {
            std::string newConfigPath = result["config"].as<std::string>();
            if (newConfigPath != configPath) {
                configPath = newConfigPath;
                configValues.clear();
                loadTomlConfig();
            }
        }

    } catch (const cxxopts::exceptions::parsing& e) {
        std::cerr << "Error parsing options: " << e.what() << "\n";
        showHelp = true;
        helpMessage = options.help();
    }
}

std::string ConfigManager::getValue(const std::string& key, const std::string& defaultValue) const {
    // Precedence: CLI > config > default
    if (cliValues.find(key) != cliValues.end()) {
        return cliValues.at(key);
    }
    if (configValues.find(key) != configValues.end()) {
        return configValues.at(key);
    }
    return defaultValue;
}

size_t ConfigManager::getSize(const std::string& key, size_t defaultValue) const {
    std::string value = getValue(key, std::to_string(defaultValue));
    try {
        return static_cast<size_t>(std::stoull(value));
    } catch (const std::exception&) {
        return defaultValue;
    }
}

double ConfigManager::getDouble(const std::string& key, double defaultValue) const {
    std::string value = getValue(key, std::to_string(defaultValue));
    try {
        return std::stod(value);
    } catch (const std::exception&) {
        return defaultValue;
    }
}

std::string ConfigManager::getString(const std::string& key, const std::string& defaultValue) const {
    return getValue(key, defaultValue);
}

bool ConfigManager::getBool(const std::string& key, bool defaultValue) const {
    std::string value = getValue(key, defaultValue ? "true" : "false");
    return (value == "true" || value == "1" || value == "yes");
}

void ConfigManager::validate() const {
    size_t minOrder = getSize("min-order", 6);
    size_t maxOrder = getSize("max-order", 20);

    if (minOrder >= maxOrder) {
        throw std::invalid_argument("min-order must be less than max-order");
    }

    if (maxOrder > 30) {
        throw std::invalid_argument("max-order too large (would exceed reasonable memory limits)");
    }

    size_t alignment = getSize("alignment", 8);
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        throw std::invalid_argument("alignment must be a power of 2");
    }

    size_t threads = getSize("threads", 1);
    if (threads == 0) {
        throw std::invalid_argument("threads must be at least 1");
    }
}

std::string ConfigManager::getHelpMessage() const {
    return helpMessage;
}

