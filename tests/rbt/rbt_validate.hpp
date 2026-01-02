#pragma once

#include <gtest/gtest.h>

template <typename Node>
auto blackHeight(Node* n, Node* nil) -> int
{
  if (n == nil)
    return 1;

  int left  = blackHeight(n->left, nil);
  int right = blackHeight(n->right, nil);

  EXPECT_EQ(left, right) << "Black height mismatch";

  return left + (n->color == 'B');
}

template <typename Node>
void validateNoRedRed(Node* n, Node* nil)
{
  if (n == nil)
    return;

  if (n->color == 'R')
  {
    EXPECT_EQ(n->left->color, 'B');
    EXPECT_EQ(n->right->color, 'B');
  }

  validateNoRedRed(n->left, nil);
  validateNoRedRed(n->right, nil);
}

template <typename Node, typename Key>
void validateBST(Node* n, Node* nil, Key min, Key max)
{
  if (n == nil)
    return;

  EXPECT_GT(n->key, min);
  EXPECT_LT(n->key, max);

  validateBST(n->left, nil, min, n->key);
  validateBST(n->right, nil, n->key, max);
}
