#ifndef TOML_PARSER_HPP
#define TOML_PARSER_HPP

#include "toml.hpp"
#include <iostream>
#include <filesystem>
#include <cstdlib>

using namespace std;
namespace fs = std::filesystem;

// Defined the path to the config.toml file in $HOME/.config/inLimbo/
#define CONFIG_PATH "$HOME/.config/inLimbo/config.toml"

// Current parents & fields macros definitions
#define PARENT_LIB "library"
#define PARENT_LIB_FIELD_NAME "name"
#define PARENT_LIB_FIELD_DIR "directory"

#define PARENT_FTP "ftp"
#define PARENT_FTP_FIELD_USER "username"
#define PARENT_FTP_FIELD_SALT "salt"
#define PARENT_FTP_FIELD_PWD_HASH "password_hash"

#define PARENT_DBG "debug"
#define PARENT_DBG_FIELD_PARSER_LOG "parser_log"

// Function to get the path for config.toml
string getConfigPath() {
  const char* homeDir = std::getenv("HOME");
  if (!homeDir) {
    cerr << "ERROR: HOME environment variable not found." << endl;
    exit(EXIT_FAILURE);
  }

  // Construct the path to the config.toml in $HOME/.config/inLimbo/
  string configFilePath = string(homeDir) + "/.config/inLimbo/config.toml";
  return configFilePath;
}

// Function to check if the config.toml file exists
bool configFileExists(const string& filePath) {
  return fs::exists(filePath);
}

// Load the configuration file
auto loadConfig() {
  string configFilePath = getConfigPath();

  if (!configFileExists(configFilePath)) {
    cerr << "ERROR: config.toml not found in " << configFilePath << endl;
    exit(EXIT_FAILURE);  // Exit gracefully if the file is not found
  }

  return toml::parse_file(configFilePath);
}

// Use the loaded configuration
auto config = loadConfig();

// Function to parse a TOML field from a given parent and field name
string_view parseTOMLField(string parent, string field) {
  return config[parent][field].value_or(""sv);
}

#endif
