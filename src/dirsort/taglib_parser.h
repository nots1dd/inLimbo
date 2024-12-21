#ifndef TAGLIB_PARSER_H
#define TAGLIB_PARSER_H

#include <string>
#include <unordered_map>

// Metadata structure
struct Metadata
{
  std::string                                  title      = "Unknown Title";
  std::string                                  artist     = "Unknown Artist";
  std::string                                  album      = "Unknown Album";
  std::string                                  genre      = "Unknown Genre";
  std::string                                  comment    = "No Comment";
  unsigned int                                 year       = 0;
  unsigned int                                 track      = 0;
  unsigned int                                 discNumber = 0;
  std::string                                  lyrics     = "No Lyrics";
  std::unordered_map<std::string, std::string> additionalProperties;
};

// TagLibParser class for parsing metadata
class TagLibParser
{
public:
  explicit TagLibParser();

  bool parseFile(const std::string& filePath, Metadata& metadata);

  std::unordered_map<std::string, Metadata> parseFromInode(ino_t              inode,
                                                           const std::string& directory);
};

// Function to print metadata
void printMetadata(const Metadata& metadata);

#endif // TAGLIB_PARSER_H
