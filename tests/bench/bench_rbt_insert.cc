#include "utils/RBTree.hpp"
#include "common.hpp"

#include <map>
#include <random>
#include <vector>

using utils::RedBlackTree;

constexpr int N = 200'000;

auto main() -> int
{
  std::vector<int> keys;
  keys.reserve(N);

  std::mt19937 rng(42);
  for (int i = 0; i < N; ++i)
    keys.push_back(rng());

  // ---------------- RBT ----------------
  {
    RedBlackTree<int, int> tree;
    Timer t;

    for (int k : keys)
      tree.insert(k, k);

    printResult("RBT insert", t.elapsed_ms());
  }

  // ---------------- std::map ----------------
  {
    std::map<int, int> m;
    Timer t;

    for (int k : keys)
      m.emplace(k, k);

    printResult("std::map insert", t.elapsed_ms());
  }
}
