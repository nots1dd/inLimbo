#include <gtest/gtest.h>
#include <random>
#include "utils/RBTree.hpp"

using utils::RedBlackTree;

TEST(RedBlackTree_Fuzz, RandomInsertErase)
{
  RedBlackTree<int, int> tree;
  std::mt19937 rng(123);

  std::set<int> reference;

  for (int i = 0; i < 2000; ++i)
  {
    int v = rng() % 500;

    if (rng() % 2)
    {
      tree.insert(v, v);
      reference.insert(v);
    }
    else
    {
      tree.erase(v);
      reference.erase(v);
    }

    EXPECT_EQ(tree.size(), reference.size());
  }
}
