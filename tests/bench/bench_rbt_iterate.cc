#include "utils/RBTree.hpp"
#include "common.hpp"

#include <map>

using utils::RedBlackTree;

constexpr int N = 200'000;

auto main() -> int
{
  RedBlackTree<int, int> tree;
  std::map<int, int>     m;

  for (int i = 0; i < N; ++i)
  {
    tree.insert(i, i);
    m.emplace(i, i);
  }

  // -------- RBT iterate --------
  {
    volatile int sum = 0;
    Timer t;

    for (auto it = tree.begin_kv(); it != tree.end_kv(); ++it)
      sum += (*it).value;

    printResult("RBT iterate", t.elapsed_ms());
  }

  // -------- std::map iterate --------
  {
    volatile int sum = 0;
    Timer t;

    for (auto& [k, v] : m)
      sum += v;

    printResult("std::map iterate", t.elapsed_ms());
  }
}
