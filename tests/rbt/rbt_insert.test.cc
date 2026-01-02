#include <gtest/gtest.h>
#include "utils/RBTree.hpp"

using utils::RedBlackTree;

TEST(RedBlackTree_Insert, InsertSingle)
{
  RedBlackTree<int, int> tree;
  tree.insert(10, 42);

  EXPECT_EQ(tree.size(), 1);
  EXPECT_FALSE(tree.isEmpty());
}

TEST(RedBlackTree_Insert, InsertOverwrite)
{
  RedBlackTree<int, int> tree;
  tree.insert(1, 10);
  tree.insert(1, 20);

  EXPECT_EQ(tree.size(), 1);
}
