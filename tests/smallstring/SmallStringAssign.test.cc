#include <gtest/gtest.h>

#include "utils/string/SmallString.hpp"

using utils::string::SmallString;

// ------------------------------------------------------------
// Assignment
// ------------------------------------------------------------

TEST(SmallStringAssign, CopyAssign)
{
  SmallString a("foo");
  SmallString b;
  b = a;
  EXPECT_EQ(b, "foo");
}

TEST(SmallStringAssign, MoveAssign)
{
  SmallString a("bar");
  SmallString b;
  b = std::move(a);
  EXPECT_EQ(b, "bar");
}

