#include <gtest/gtest.h>

#include "utils/string/SmallString.hpp"

using utils::string::SmallString;

// ------------------------------------------------------------
// Clear / reserve
// ------------------------------------------------------------

TEST(SmallStringOps, Clear)
{
  SmallString s("data");
  s.clear();
  EXPECT_TRUE(s.empty());
  EXPECT_STREQ(s.c_str(), "");
}

TEST(SmallStringOps, Reserve)
{
  SmallString s;
  s.reserve(128);
  EXPECT_GE(s.capacity(), 128u);
}
