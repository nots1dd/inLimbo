#pragma once

#include "Config.hpp"
#include "toml.hpp"
#include "Logger.hpp"
#include "utils/PathResolve.hpp"
#include <cstdlib>
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

namespace parser {


/**
 * @brief Macros for parent and field names used in the TOML configuration.
 *
 * These macros represent the sections and fields in the TOML configuration file.
 */
#define PARENT_LIB            "library"   /**< Parent section for library settings */
#define PARENT_LIB_FIELD_NAME "name"      /**< Field for the library name */
#define PARENT_LIB_FIELD_DIR  "directory" /**< Field for the library directory */

#define PARENT_DBG "debug" /**< Parent section for debug settings */
#define PARENT_DBG_FIELD_TAGLIB_PARSER_LOG \
  "taglib_parser_log" /**< Field for debug parser log setting */
#define PARENT_DBG_FIELD_COLORS_PARSER_LOG   "colors_parser_log"
#define PARENT_DBG_FIELD_KEYBINDS_PARSER_LOG "keybinds_parser_log"

/* SPECIAL KEYBINDS MACROS */
#define PARENT_KEYBINDS           "keybinds" /**< Parent section for keybinds */
#define SPECIAL_KEYBIND_ENTER_STR "Enter"    /**< Special keybind for Enter */
#define SPECIAL_KEYBIND_TAB_STR   "Tab"      /**< Special keybind for Tab */
#define SPECIAL_KEYBIND_SPACE_STR "Space"    /**< Special keybind for Space */

#define PARENT_COLORS "colors" /**< Parent section for color settings */

#define PARENT_UI "ui" /**< Parent section for ui settings */

/**
 * @brief Checks if the configuration file exists.
 *
 * This function checks if the `config.toml` file exists at the given file path.
 *
 * @param filePath The path to the configuration file.
 * @return `true` if the file exists, otherwise `false`.
 */
FORCE_INLINE auto configFileExists(const string& filePath) -> bool { return fs::exists(filePath); }

/**
 * @brief Loads the configuration file.
 *
 * This function loads the `config.toml` file and parses it using the `toml` library. If the file
 * does not exist, the program exits gracefully with an error message.
 *
 * @return A `toml::parse_result` object representing the parsed configuration.
 * @throws std::runtime_error If the configuration file does not exist or cannot be parsed.
 */
FORCE_INLINE auto loadConfig()
{
  string configFilePath = utils::getConfigPath("config.toml");

  if (!configFileExists(configFilePath))
  {
    LOG_ERROR("config.toml not found in '{}'!", configFilePath);
    exit(EXIT_FAILURE);
  }

  LOG_DEBUG("Loading config.toml file: '{}'", configFilePath);

  return toml::parse_file(configFilePath);
}

/** Parse the configuration file during the initialization */
inline auto config = loadConfig();

/**
 * @brief Parses a string field from the TOML configuration.
 *
 * This function retrieves the value of a specific field within a parent section of the TOML
 * configuration. If the field is not found, it returns an empty string view.
 *
 * @param parent The parent section name (e.g., "library").
 * @param field The field name within the parent section (e.g., "name").
 * @return A string view representing the value of the field.
 */
FORCE_INLINE auto parseTOMLField(string parent, string field) -> string_view
{
  return config[parent][field].value_or(
    ""sv); /**< If the field is not found, return an empty string view. */
}

/**
 * @brief Parses a string field from a custom TOML configuration that is called by the
 * INLIMBO_CONFIG_HOME macro at runtime
 *
 * This function retrieves the value of a specific field within a parent section of the TOML
 * configuration. If the field is not found, it returns an empty string view.
 *
 * @param parent The parent section name (e.g., "library").
 * @param field The field name within the parent section (e.g., "name").
 * @return A string view representing the value of the field.
 */
FORCE_INLINE auto parseTOMLFieldCustom(const toml::parse_result& custom_config, string parent,
                          string field) -> string_view
{
  return custom_config[parent][field].value_or(
    ""sv); /**< If the field is not found, return an empty string view. */
}

/**
 * @brief Parses an integer field from the TOML configuration.
 *
 * This function retrieves the value of a specific field as an integer from the TOML configuration.
 * If the field is not found, it returns -1 as a default value.
 *
 * @param parent The parent section name (e.g., "ftp").
 * @param field The field name within the parent section (e.g., "username").
 * @return The integer value of the field, or -1 if the field is not found.
 */
FORCE_INLINE auto parseTOMLFieldInt(string parent, string field) -> int64_t
{
  return config[parent][field].value_or(
    -1); /**< If the field is not found, return -1 as default. */
}

/**
 * @brief Parses an integer field from a custom TOML configuration set by the INLIMBO_CONFIG_HOME
 * macro at runtime.
 *
 * This function retrieves the value of a specific field as an integer from the TOML configuration.
 * If the field is not found, it returns -1 as a default value.
 *
 * @param parent The parent section name (e.g., "ftp").
 * @param field The field name within the parent section (e.g., "username").
 * @return The integer value of the field, or -1 if the field is not found.
 */
FORCE_INLINE auto parseTOMLFieldIntCustom(const toml::parse_result& custom_config, string parent,
                             string field) -> int64_t
{
  return custom_config[parent][field].value_or(
    -1); /**< If the field is not found, return -1 as default. */
}

FORCE_INLINE auto parseTOMLFieldBool(const string& parent, const string& field) -> bool
{
  if (string(parseTOMLField(parent, field)) == "true")
    return true;
  return false;
}

}
