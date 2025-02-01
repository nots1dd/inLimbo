#pragma GCC optimize("O3")  // Enable high-level optimizations for this function

#include "trie.hpp"
#include <iostream>

using namespace std;

auto main(int argc, char* argv[]) -> int {
    std::vector<std::string> current_artist_names = {"String 1", "String 2", "Test", "Radio", "head"};
    Trie trie;

    for (size_t i = 0; i < current_artist_names.size(); ++i) {
        trie.insert(current_artist_names[i], i);
    }

    std::string search_query = argv[1];  // Example input (use command line args)

    std::vector<int> matching_indices = trie.search(search_query);

    std::cout << "Matching artist indices for prefix \"" << search_query << "\":\n";
    for (int idx : matching_indices) {
        std::cout << idx << " -> " << current_artist_names[idx] << "\n";
    }

    return 0;
}
