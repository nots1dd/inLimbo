#include <gtest/gtest.h>

#include "utils/string/SmallString.hpp"

using utils::string::SmallString;

// ------------------------------------------------------------
// Capacity / SSO behavior
// ------------------------------------------------------------

TEST(SmallStringCapacity, SSOInitially)
{
  SmallString s("short");
  EXPECT_TRUE(s.is_sso());
}

TEST(SmallStringCapacity, HeapAfterGrowth)
{
  SmallString s;
  for (int i = 0; i < 64; ++i)
    s += 'a';

  EXPECT_GT(s.size(), SmallString::m_ssoCAPACITY);
  EXPECT_FALSE(s.is_sso());
}
