#ifndef PROPERTIES_HPP
#define PROPERTIES_HPP

#include "../parser/toml_parser.hpp"
#include <iostream>
#include <string_view>
#include <unordered_map>

/**
 * @brief Struct to hold global property settings.
 *
 * This struct contains the global settings and properties used throughout the application.
 */
struct GlobalProps
{
  bool show_bitrate; /**< Flag to control bitrate display */
                     // Add more properties here as needed
};

/**
 * @brief Parses the global properties from the TOML configuration.
 *
 * This function reads the global property configurations from the TOML file
 * and returns a populated GlobalProps struct. Unlike parseKeybinds, this function
 * will warn on errors but continue execution.
 *
 * @return A GlobalProps struct with the configured settings.
 */
GlobalProps parseProps()
{
  GlobalProps gprops;

  /**
   * @brief Reports a warning for invalid property values.
   *
   * This lambda function prints a warning message when an invalid property
   * is detected, but allows the program to continue with a default value.
   *
   * @param field The field in which the invalid value was found.
   * @param value The value that was invalid.
   * @param default_val The default value that will be used instead.
   */
  auto reportWarning =
    [](const std::string& field, const std::string_view& value, const std::string& default_val)
  {
    std::cerr << "!! Warning: Invalid value '" << value << "' detected for field '" << field
              << "'. Using default value '" << default_val << "'." << std::endl;
  };

  /**
   * @brief Handles boolean property parsing with error checking.
   *
   * This lambda function safely converts string values to booleans,
   * providing appropriate warnings for invalid values.
   *
   * @param value The string value to convert.
   * @param field The field name for error reporting.
   * @param default_val The default boolean value to use if parsing fails.
   * @return The parsed boolean value or the default.
   */
  auto handle_bool_prop = [&](const std::string_view& value, const std::string& field,
                              bool default_val) -> bool
  {
    if (value.empty())
    {
      reportWarning(field, value, default_val ? "true" : "false");
      return default_val;
    }

    std::string value_lower;
    value_lower.reserve(value.size());
    for (char c : value)
    {
      value_lower += std::tolower(c);
    }

    if (value_lower == "true" || value_lower == "1" || value_lower == "yes")
    {
      return true;
    }
    if (value_lower == "false" || value_lower == "0" || value_lower == "no")
    {
      return false;
    }

    reportWarning(field, value, default_val ? "true" : "false");
    return default_val;
  };

  const std::unordered_map<std::string, std::pair<bool&, bool>> field_map = {
    {"show_bitrate", {gprops.show_bitrate, false}},
  };

  for (const auto& [field, field_info] : field_map)
  {
    const auto& [field_ref, default_val] = field_info;
    std::string_view value               = parseTOMLField(PARENT_UI, field);
    field_ref                            = handle_bool_prop(value, field, default_val);
  }

  return gprops;
}

#endif
