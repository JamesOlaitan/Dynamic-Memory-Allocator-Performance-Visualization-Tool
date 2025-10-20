#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <cxxopts.hpp>
#include <map>
#include <optional>
#include <string>

/**
 * @class ConfigManager
 * @brief Manages configuration from TOML files and CLI arguments with proper precedence.
 *
 * Precedence order: CLI arguments > TOML config file > hardcoded defaults
 * This class uses toml11 for parsing TOML files and cxxopts for CLI parsing.
 */
class ConfigManager {
   public:
    /**
     * @brief Constructs a ConfigManager and loads the specified config file.
     * @param configPath Path to the TOML configuration file.
     */
    explicit ConfigManager(const std::string& configPath = "config/default.toml");

    /**
     * @brief Parses command-line arguments and merges with config.
     * @param argc Argument count from main.
     * @param argv Argument vector from main.
     * @param programName Name of the program for help text.
     * @param description Description of the program for help text.
     */
    void parseCLI(int argc, char* argv[], const std::string& programName, const std::string& description);

    /**
     * @brief Gets an integer configuration value with precedence.
     * @param key Configuration key.
     * @param defaultValue Fallback value if key not found.
     * @return Configuration value.
     */
    size_t getSize(const std::string& key, size_t defaultValue) const;

    /**
     * @brief Gets a double configuration value with precedence.
     * @param key Configuration key.
     * @param defaultValue Fallback value if key not found.
     * @return Configuration value.
     */
    double getDouble(const std::string& key, double defaultValue) const;

    /**
     * @brief Gets a string configuration value with precedence.
     * @param key Configuration key.
     * @param defaultValue Fallback value if key not found.
     * @return Configuration value.
     */
    std::string getString(const std::string& key, const std::string& defaultValue) const;

    /**
     * @brief Gets a boolean configuration value with precedence.
     * @param key Configuration key.
     * @param defaultValue Fallback value if key not found.
     * @return Configuration value.
     */
    bool getBool(const std::string& key, bool defaultValue) const;

    /**
     * @brief Validates allocator configuration parameters.
     * @throws std::invalid_argument if validation fails.
     */
    void validate() const;

    /**
     * @brief Checks if help was requested.
     * @return True if --help was passed.
     */
    bool helpRequested() const { return showHelp; }

    /**
     * @brief Gets the help message string.
     * @return Help text from cxxopts.
     */
    std::string getHelpMessage() const;

   private:
    std::map<std::string, std::string> cliValues;     ///< Values from CLI arguments
    std::map<std::string, std::string> configValues;  ///< Values from TOML file
    std::string configPath;                           ///< Path to config file
    bool showHelp;                                    ///< Whether --help was requested
    std::string helpMessage;                          ///< Cached help message

    /**
     * @brief Loads values from TOML configuration file.
     */
    void loadTomlConfig();

    /**
     * @brief Helper to get value with precedence: CLI > config > default.
     */
    std::string getValue(const std::string& key, const std::string& defaultValue) const;
};

#endif  // CONFIG_MANAGER_H
