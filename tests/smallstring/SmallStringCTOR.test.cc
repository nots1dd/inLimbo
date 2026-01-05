#include <gtest/gtest.h>

#include "utils/string/SmallString.hpp"

using utils::string::SmallString;

// ------------------------------------------------------------
// Construction
// ------------------------------------------------------------

TEST(SmallStringCtor, DefaultConstruct)
{
  SmallString s;
  EXPECT_TRUE(s.empty());
  EXPECT_STREQ(s.c_str(), "");
}

TEST(SmallStringCtor, FromCString)
{
  SmallString s("hello");
  EXPECT_EQ(s.size(), 5u);
  EXPECT_EQ(s, "hello");
}

TEST(SmallStringCtor, FromNullCString)
{
  const char* p = nullptr;
  SmallString s(p);
  EXPECT_TRUE(s.empty());
}

TEST(SmallStringCtor, FromStringView)
{
  std::string_view sv = "world";
  SmallString s(sv);
  EXPECT_EQ(s, "world");
}

TEST(SmallStringCtor, CopyConstructor)
{
  SmallString a("copy");
  SmallString b(a);
  EXPECT_EQ(a, b);
}

TEST(SmallStringCtor, MoveConstructor)
{
  SmallString a("move");
  SmallString b(std::move(a));
  EXPECT_EQ(b, "move");
}
