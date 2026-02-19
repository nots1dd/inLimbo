#include <gtest/gtest.h>

#include "utils/string/SmallString.hpp"

using utils::string::SmallString;

// ------------------------------------------------------------
// Append basics
// ------------------------------------------------------------

TEST(SmallStringAppend, AppendStringView)
{
  SmallString s("hello");
  s += std::string_view{" world"};
  EXPECT_EQ(s, "hello world");
}

TEST(SmallStringAppend, AppendCString)
{
  SmallString s("foo");
  s += "bar";
  EXPECT_EQ(s, "foobar");
}

TEST(SmallStringAppend, AppendChar)
{
  SmallString s;
  s += 'a';
  s += 'b';
  s += 'c';
  EXPECT_EQ(s, "abc");
}

TEST(SmallStringAppend, AppendBool)
{
  SmallString s;
  s += true;
  s += " ";
  s += false;
  EXPECT_EQ(s, "true false");
}

// ------------------------------------------------------------
// Append numeric
// ------------------------------------------------------------

TEST(SmallStringAppend, AppendIntegral)
{
  SmallString s("num=");
  s += 42;
  EXPECT_EQ(s, "num=42");
}

TEST(SmallStringAppend, AppendNegativeIntegral)
{
  SmallString s;
  s += -123;
  EXPECT_EQ(s, "-123");
}

TEST(SmallStringAppend, AppendFloating)
{
  SmallString s("pi=");
  s += 3.14;
  EXPECT_TRUE(s.view().starts_with("pi="));
}

enum class TestEnum : uint8_t { A = 1, B = 2 };

TEST(SmallStringAppend, AppendEnum)
{
  SmallString s;
  s += TestEnum::B;
  EXPECT_EQ(s, "2");
}

// ------------------------------------------------------------
// Append pointer
// ------------------------------------------------------------

TEST(SmallStringAppend, AppendPointer)
{
  SmallString s("ptr=");
  int x = 0;
  s += &x;
  EXPECT_TRUE(s.view().starts_with("ptr=0x"));
}
