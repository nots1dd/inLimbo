#include <gtest/gtest.h>
#include "utils/RBTree.hpp"
#include "rbt_validate.hpp"

using utils::RedBlackTree;

TEST(RedBlackTree_Invariants, InsertMany)
{
  RedBlackTree<int, int> tree;

  for (int i = 1; i <= 1000; ++i)
    tree.insert(i, i);

  auto root = tree.returnRoot();
  auto nil  = tree.returnNil();

  EXPECT_EQ(root->color, 'B');

  validateNoRedRed(root, nil);
  blackHeight(root, nil);
  validateBST(root, nil,
              std::numeric_limits<int>::min(),
              std::numeric_limits<int>::max());
}
