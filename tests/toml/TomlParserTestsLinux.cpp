#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../../src/parser/toml_parser.hpp"
#include <filesystem>
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem;

class TOMLParserTest : public ::testing::Test {
protected:
    std::string tempConfigDir;
    std::string originalConfigHome;

    void SetUp() override {
        std::cout << "[INFO] Setting up test environment..." << std::endl;

        // Store the original INLIMBO_CONFIG_HOME environment variable
        const char* configHomeEnv = getenv(CUSTOM_CONFIG_MACRO);
        originalConfigHome = (configHomeEnv != nullptr) ? configHomeEnv : "";

        // Create a temporary directory for the configuration
        tempConfigDir = (fs::temp_directory_path() / "test_config/").string();
        fs::create_directories(tempConfigDir);

        // Redirect the INLIMBO_CONFIG_HOME environment variable
        setenv(CUSTOM_CONFIG_MACRO, tempConfigDir.c_str(), 1);

        // Create a temporary config file for testing
        std::string testConfig = R"(
[library]
name = "MyLibrary"
directory = "/usr/lib/mylibrary"

[ftp]
username = "user"
salt = "salt_value"
password_hash = "hashed_password"

[debug]
parser_log = "true"
        )";

        // Write the test configuration file
        fs::path configPath = fs::path(getConfigPath("config.toml"));
        fs::create_directories(configPath.parent_path());
        std::ofstream configFile(configPath);
        configFile << testConfig;
        configFile.close();

        std::cout << "[INFO] Test configuration created at: " << configPath << std::endl;
    }

    void TearDown() override {
        std::cout << "[INFO] Cleaning up test environment..." << std::endl;

        // Remove the temporary directory
        //fs::remove_all(tempConfigDir);

        // Restore the original INLIMBO_CONFIG_HOME environment variable
        if (originalConfigHome.empty()) {
            unsetenv(CUSTOM_CONFIG_MACRO);
        } else {
            setenv(CUSTOM_CONFIG_MACRO, originalConfigHome.c_str(), 1);
        }

        std::cout << "[INFO] Test environment cleanup complete." << std::endl;
    }
};

// Test for getConfigPath (Using a macro to define a custom config.toml for manipulation testing of the config)
TEST_F(TOMLParserTest, GetConfigPath_ValidFilePath) {
    std::cout << "[TEST] Verifying getConfigPath with a valid file path..." << std::endl;
    std::string expectedPath = tempConfigDir + "config.toml";
    EXPECT_EQ(getConfigPath("config.toml"), expectedPath);
}

// Test for getCachePath (Will not use custom dir for this as we dont need to manipulate anything here)
TEST_F(TOMLParserTest, GetCachePath_ValidCachePath) {
    std::cout << "[TEST] Verifying getCachePath with a valid cache path..." << std::endl;
    std::string expectedCachePath = std::string(std::getenv("HOME"))  + "/.cache/inLimbo/";
    EXPECT_EQ(getCachePath(), expectedCachePath);
}

// Test for configFileExists when file exists
TEST_F(TOMLParserTest, ConfigFileExists_FileExists) {
    std::cout << "[TEST] Verifying configFileExists when the file exists..." << std::endl;
    EXPECT_TRUE(configFileExists(getConfigPath("config.toml")));
}

// Test for configFileExists when file does not exist
TEST_F(TOMLParserTest, ConfigFileExists_FileDoesNotExist) {
    std::cout << "[TEST] Verifying configFileExists when the file does not exist..." << std::endl;
    std::string nonExistentFile = tempConfigDir + "non_existent.toml";
    EXPECT_FALSE(configFileExists(nonExistentFile));
}

// Test for loadConfig (ensure it loads the file correctly)
TEST_F(TOMLParserTest, LoadConfig_FileExistsAndParsed) {
    std::cout << "[TEST] Verifying loadConfig with a valid configuration file..." << std::endl;

    auto config = loadConfig();

    // Check that a value exists for a given field
    EXPECT_EQ(string(parseTOMLFieldCustom(config, PARENT_LIB, PARENT_LIB_FIELD_NAME)), "MyLibrary");
    EXPECT_EQ(string(parseTOMLFieldCustom(config, PARENT_FTP, PARENT_FTP_FIELD_USER)), "user");
    EXPECT_EQ(string(parseTOMLFieldCustom(config, PARENT_DBG, PARENT_DBG_FIELD_PARSER_LOG)), "true");
}

// Test for parseTOMLField for non-existent field
TEST_F(TOMLParserTest, ParseTOMLField_FieldNotFound) {
    std::cout << "[TEST] Verifying parseTOMLField with a non-existent field..." << std::endl;
    auto config = loadConfig();

    EXPECT_EQ(parseTOMLFieldCustom(config, PARENT_LIB, "non_existent_field"), "");
    EXPECT_EQ(parseTOMLFieldCustom(config, "somefield", "non_existent_field"), "");
}

// Test for parseTOMLFieldInt for integer fields
TEST_F(TOMLParserTest, ParseTOMLFieldInt_ValidIntegerField) {
    std::cout << "[TEST] Verifying parseTOMLFieldInt with valid integer fields..." << std::endl;

    auto config = loadConfig();

    EXPECT_EQ(parseTOMLFieldIntCustom(config, PARENT_FTP, "salt"), -1); // No integer field for "salt"
    EXPECT_EQ(parseTOMLFieldIntCustom(config, PARENT_LIB, "directory"), -1); // No integer field for "directory"
    EXPECT_EQ(parseTOMLFieldIntCustom(config, PARENT_DBG, "parser_log"), -1); // No integer field for "parser_log"
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    // Set Google Test verbosity
    ::testing::GTEST_FLAG(print_time) = true; // Print execution time for each test
    std::cout << "[INFO] Running all tests with verbosity enabled..." << std::endl;

    return RUN_ALL_TESTS();
}
