#include "utils/RBTree.hpp"
#include "common.hpp"

#include <map>
#include <vector>

using utils::RedBlackTree;

constexpr int N = 100'000;

auto main() -> int
{
  std::vector<int> keys(N);
  for (int i = 0; i < N; ++i)
    keys[i] = i;

  // -------- RBT --------
  {
    RedBlackTree<int, int> tree;
    for (int k : keys)
      tree.insert(k, k);

    Timer t;
    for (int k : keys)
      tree.erase(k);

    printResult("RBT erase", t.elapsed_ms());
  }

  // -------- std::map --------
  {
    std::map<int, int> m;
    for (int k : keys)
      m.emplace(k, k);

    Timer t;
    for (int k : keys)
      m.erase(k);

    printResult("std::map erase", t.elapsed_ms());
  }
}
