#include <gtest/gtest.h>
#include "utils/RBTree.hpp"

using utils::RedBlackTree;

TEST(RedBlackTree_Iterator, SortedTraversal)
{
  RedBlackTree<int, int> tree;

  std::vector<int> input = {7, 3, 9, 1, 5, 8, 10};
  for (int v : input)
    tree.insert(v, v);

  int last = std::numeric_limits<int>::min();
  // explictly begin and end are mentioned to test the functionality of custom iterators
  for (auto it = tree.begin(); it != tree.end(); ++it)
  {
    EXPECT_GT((*it).key, last);
    last = (*it).key;
  }
}
