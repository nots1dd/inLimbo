#include <gtest/gtest.h>

#include "utils/string/SmallString.hpp"

using utils::string::SmallString;

// ------------------------------------------------------------
// Comparison
// ------------------------------------------------------------

TEST(SmallStringCompare, CompareWithCString)
{
  SmallString s("abc");
  EXPECT_TRUE(s == "abc");
  EXPECT_FALSE(s != "abc");
}

TEST(SmallStringCompare, CompareWithStringView)
{
  SmallString s("xyz");
  std::string_view sv = "xyz";
  EXPECT_TRUE(s == sv);
}

TEST(SmallStringCompare, CompareWithSmallString)
{
  SmallString a("same");
  SmallString b("same");
  EXPECT_TRUE(a == b);
}
