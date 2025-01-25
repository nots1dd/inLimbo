#include <gtest/gtest.h>
#include "../../src/dirsort/taglib_parser.h"
#include <fstream>
#include <sys/stat.h>
#include <iostream>

// ANSI escape codes for styling
#define BOLD "\033[1m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define RESET "\033[0m"

// Test fixture for TagLibParser
class TagLibParserTest : public ::testing::Test {
protected:
    TagLibParser parser; // The parser object to handle metadata extraction

    // Constructor to initialize the parser with debug logging enabled
    TagLibParserTest() : parser("true") {}

    // Setup method to create dummy files for testing
    void SetUp() override {
        // Run the Bash script to generate dummy audio files with metadata
        system("./dummy_file_gen.sh");

        // Create additional dummy audio files without metadata
        createDummyAudioFile("test_no_metadata.mp3", "Dummy MP3 Content without Metadata");
        createDummyAudioFile("test_no_metadata.flac", "Dummy FLAC Content without Metadata");
    }

    // TearDown method to clean up the generated test files
    void TearDown() override {
        remove("test_no_metadata.mp3");
        remove("test_no_metadata.flac");
    }

    // Helper function to create a dummy audio file without metadata
    void createDummyAudioFile(const std::string& filePath, const std::string& content) {
        std::ofstream file(filePath, std::ios::binary);
        if (file) {
            file.write(content.c_str(), content.size());
            file.close();
        }
    }
};

// Test parsing MP3 files with metadata
TEST_F(TagLibParserTest, ParseMP3WithMetadata) {
    Metadata metadata;
    bool result = parser.parseFile("dummy.mp3", metadata);

    ASSERT_TRUE(result) << RED << "Failed to parse MP3 file with metadata." << RESET;

    // Log the parsed metadata for clarity
    std::cout << BOLD << GREEN << "Parsed Metadata for dummy.mp3:" << RESET << "\n"
              << "Title: " << YELLOW << metadata.title << RESET << "\n"
              << "Artist: " << YELLOW << metadata.artist << RESET << "\n"
              << "Album: " << YELLOW << metadata.album << RESET << "\n";

    EXPECT_EQ(metadata.title, "Test Song");
    EXPECT_EQ(metadata.artist, "Test Artist");
    EXPECT_EQ(metadata.album, "Test Album");
}

// Test parsing MP3 files without metadata
TEST_F(TagLibParserTest, ParseMP3WithoutMetadata) {
    Metadata metadata;
    bool result = parser.parseFile("test_no_metadata.mp3", metadata);

    ASSERT_TRUE(result) << RED << "Failed to parse MP3 file without metadata." << RESET;

    // Log the default metadata values for clarity
    std::cout << BOLD << GREEN << "Parsed Metadata for test_no_metadata.mp3 (Expected Default Values):" << RESET << "\n"
              << "Title: " << YELLOW << metadata.title << RESET << "\n"
              << "Artist: " << YELLOW << metadata.artist << RESET << "\n"
              << "Album: " << YELLOW << metadata.album << RESET << "\n";

    EXPECT_EQ(metadata.title, "Unknown Title");
    EXPECT_EQ(metadata.artist, "Unknown Artist");
    EXPECT_EQ(metadata.album, "Unknown Album");
}

// Test parsing FLAC files with metadata
TEST_F(TagLibParserTest, ParseFLACWithMetadata) {
    Metadata metadata;
    bool result = parser.parseFile("dummy.flac", metadata);

    ASSERT_TRUE(result) << RED << "Failed to parse FLAC file with metadata." << RESET;

    // Log the parsed metadata for clarity
    std::cout << BOLD << GREEN << "Parsed Metadata for dummy.flac:" << RESET << "\n"
              << "Title: " << YELLOW << metadata.title << RESET << "\n"
              << "Artist: " << YELLOW << metadata.artist << RESET << "\n"
              << "Album: " << YELLOW << metadata.album << RESET << "\n";

    EXPECT_EQ(metadata.title, "Test Song");
    EXPECT_EQ(metadata.artist, "Test Artist");
    EXPECT_EQ(metadata.album, "Test Album");
}

// Test parsing FLAC files without metadata
TEST_F(TagLibParserTest, ParseFLACWithoutMetadata) {
    Metadata metadata;
    bool result = parser.parseFile("test_no_metadata.flac", metadata);

    ASSERT_FALSE(result) << RED << "Failed to parse FLAC file without metadata." << RESET;

    // Log the default metadata values for clarity
    std::cout << BOLD << GREEN << "Parsed Metadata for test_no_metadata.flac (Expected Default Values):" << RESET << "\n"
              << "Title: " << YELLOW << metadata.title << RESET << "\n"
              << "Artist: " << YELLOW << metadata.artist << RESET << "\n"
              << "Album: " << YELLOW << metadata.album << RESET << "\n";

    EXPECT_EQ(metadata.title, "Unknown Title");
    EXPECT_EQ(metadata.artist, "Unknown Artist");
    EXPECT_EQ(metadata.album, "Unknown Album");
}

// Test handling invalid file paths
TEST_F(TagLibParserTest, ParseInvalidFilePath) {
    Metadata metadata;
    bool result = parser.parseFile("non_existent_file.mp3", metadata);

    ASSERT_FALSE(result) << RED << "Unexpected success parsing a non-existent file." << RESET;

    // Log the result for invalid file paths
    std::cout << BOLD << RED << "Attempted to parse non_existent_file.mp3. Success: " << result << RESET << "\n";
}

// Test parsing from inode
TEST_F(TagLibParserTest, ParseFromInode) {
    struct stat fileStat;
    stat("dummy.mp3", &fileStat);

    auto metadataMap = parser.parseFromInode(fileStat.st_ino, ".");

    ASSERT_FALSE(metadataMap.empty()) << RED << "Failed to parse metadata from inode." << RESET;

    // Log the parsed metadata for clarity
    for (const auto& [path, metadata] : metadataMap) {
       printMetadata(metadata); 
    }

    EXPECT_EQ(metadataMap.begin()->second.title, "Test Song");
}

// Test extracting thumbnails
TEST_F(TagLibParserTest, ExtractThumbnailMP3) {
    bool result = extractThumbnail("dummy_art.mp3", "output_thumbnail.png");

    ASSERT_TRUE(result) << RED << "Failed to extract thumbnail from dummy_art.mp3." << RESET;

    struct stat buffer;
    EXPECT_EQ(stat("output_thumbnail.png", &buffer), 0) << RED << "Thumbnail file was not created." << RESET;

    std::cout << BOLD << GREEN << "Thumbnail successfully extracted to output_thumbnail.png." << RESET << "\n";

    remove("output_thumbnail.png");
}

TEST_F(TagLibParserTest, ExtractThumbnailFlac) {
    bool result = extractThumbnail("dummy_art.flac", "output_thumbnail.png");

    ASSERT_TRUE(result) << RED << "Failed to extract thumbnail from dummy_art.flac" << RESET;

    struct stat buffer;
    EXPECT_EQ(stat("output_thumbnail.png", &buffer), 0) << RED << "Thumbnail file was not created." << RESET;

    std::cout << BOLD << GREEN << "Thumbnail successfully extracted to output_thumbnail.png." << RESET << "\n";

    remove("output_thumbnail.png");
}

/*TEST_F(TagLibParserTest, ParseUnsupportedFileType) {*/
/*    Metadata metadata;*/
/*    bool result = parser.parseFile("Aight.ogg", metadata);*/
/**/
/*    ASSERT_FALSE(result) << RED << "Parsing unsupported file type should fail." << RESET;*/
/**/
/*    std::cout << BOLD << RED << "Attempted to parse unsupported_file.ogg. Success: " << result << RESET << "\n";*/
/*}*/

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);

    // Set Google Test verbosity
    ::testing::GTEST_FLAG(print_time) = true; // Print execution time for each test
    std::cout << "--> [INFO] Running all tests with verbosity enabled..." << std::endl;

    int run_tests = RUN_ALL_TESTS();

    remove("dummy.flac");
    remove("dummy.mp3");
    remove("dummy_art.mp3");
    remove("dummy_art.flac");

    return run_tests;
}
