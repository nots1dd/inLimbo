#include <gtest/gtest.h>
#include "utils/RBTree.hpp"

using utils::RedBlackTree;

TEST(RedBlackTree_Erase, EraseAllOrders)
{
  RedBlackTree<int, int> tree;

  for (int i = 1; i <= 200; ++i)
    tree.insert(i, i);

  for (int i = 1; i <= 200; ++i)
  {
    EXPECT_TRUE(tree.erase(i));
    EXPECT_EQ(tree.size(), 200 - i);
  }

  EXPECT_TRUE(tree.isEmpty());
}

TEST(RedBlackTree_Erase, EraseRootRepeatedly)
{
  RedBlackTree<int, int> tree;

  for (int i = 1; i <= 50; ++i)
    tree.insert(i, i);

  while (!tree.isEmpty())
  {
    auto rootKey = tree.begin().operator*().key;
    EXPECT_TRUE(tree.erase(rootKey));
  }
}
