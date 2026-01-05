#include <gtest/gtest.h>

#include "utils/string/SmallString.hpp"

using utils::string::SmallString;

// ------------------------------------------------------------
// Path helpers
// ------------------------------------------------------------

TEST(SmallStringPath, ExtensionSimple)
{
  SmallString s("file.txt");
  EXPECT_EQ(s.extension(), ".txt");
}

TEST(SmallStringPath, ExtensionNested)
{
  SmallString s("/a/b/c.tar.gz");
  EXPECT_EQ(s.extension(), ".gz");
}

TEST(SmallStringPath, ExtensionNone)
{
  SmallString s("README");
  EXPECT_TRUE(s.extension().empty());
}

TEST(SmallStringPath, FilenameSimple)
{
  SmallString s("dir/file.wav");
  EXPECT_EQ(s.filename(), "file.wav");
}

TEST(SmallStringPath, FilenameTrailingSlash)
{
  SmallString s("/dir/subdir/");
  EXPECT_TRUE(s.filename().empty());
}

TEST(SmallStringPath, FilenameOnly)
{
  SmallString s("song.mp3");
  EXPECT_EQ(s.filename(), "song.mp3");
}
