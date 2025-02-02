#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>  // For std::transform

/**
 * @file Trie.h
 * @brief This file defines the Trie data structure for efficient prefix-based string searching.
 *
 * The Trie (Prefix Tree) is an efficient way to store strings that allows for fast lookups and
 * retrievals of words with common prefixes. This implementation stores artist names and allows
 * searching for all names starting with a given prefix.
 */

/**
 * @class TrieNode
 * @brief Represents a single node in the Trie.
 *
 * A TrieNode stores children nodes for each possible character, a list of indices representing the
 * positions of words in the original dataset, and a flag indicating if the node is the end of a
 * valid word.
 *
 * @note The `children` unordered_map stores the next character in the string as a key and the
 * corresponding child node as the value.
 */
class TrieNode
{
public:
  std::unordered_map<char, TrieNode*> children;
  std::vector<int> indices;
  bool isEndOfWord;

  TrieNode() : isEndOfWord(false) {}
};

/**
 * @class Trie
 * @brief Implements the Trie (Prefix Tree) data structure.
 *
 * This class provides methods to insert artist names into the Trie and search for all names that
 * start with a given prefix. It stores a list of indices for each node, making it efficient to
 * retrieve all matching names for a prefix.
 */
class Trie
{
private:
  TrieNode* root;

  // Helper function to convert characters to lowercase
  static auto toLowerCase(const std::string& str) -> std::string
  {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
  }

  // Helper function to delete a word from the Trie recursively
  auto deleteWord(TrieNode* node, const std::string& word, int index, size_t depth = 0) -> bool
  {
    if (!node)
      return false;

    if (depth == word.size())
    {
      // Remove the index from this node
      auto& indices = node->indices;
      auto it = std::find(indices.begin(), indices.end(), index);
      if (it != indices.end())
      {
        indices.erase(it);
      }
      node->isEndOfWord = !indices.empty();
      return node->children.empty() && !node->isEndOfWord;
    }

    char c = toLowerCase(std::string(1, word[depth]))[0];
    if (deleteWord(node->children[c], word, index, depth + 1))
    {
      // Clean up the current node if it has no children and isn't the end of any word
      delete node->children[c];
      node->children.erase(c);
      return node->children.empty() && !node->isEndOfWord;
    }
    return false;
  }

public:
  Trie() : root(new TrieNode()) {}

  /**
   * @brief Inserts a word into the Trie.
   *
   * @param word The word to insert.
   * @param index The index associated with the word.
   */
  void insert(const std::string& word, int index)
  {
    TrieNode* node = root;
    std::string lower_word = toLowerCase(word);

    for (char c : lower_word)
    {
      if (!node->children.count(c))
      {
        node->children[c] = new TrieNode();
      }
      node = node->children[c];
      node->indices.push_back(index);
    }

    node->isEndOfWord = true;
  }

  /**
   * @brief Searches for words starting with a given prefix.
   *
   * @param prefix The search prefix.
   * @return A vector of indices of matching words.
   */
  auto search(const std::string& prefix) -> std::vector<int>
  {
    TrieNode* node = root;
    std::string lower_prefix = toLowerCase(prefix);

    for (char c : lower_prefix)
    {
      if (!node->children.count(c))
      {
        return {};  // No match found
      }
      node = node->children[c];
    }

    return node->indices;
  }

  /**
   * @brief Deletes a word from the Trie.
   *
   * @param word The word to delete.
   * @param index The index associated with the word.
   */
  void deleteWord(const std::string& word, int index)
  {
    deleteWord(root, word, index);
  }

  /**
   * @brief Clears all nodes in the Trie.
   */
  void clear()
  {
    delete root;
    root = new TrieNode();
  }

  /**
   * @brief Checks if the Trie is empty.
   *
   * @return True if the Trie is empty, false otherwise.
   */
  [[nodiscard]] auto is_empty() const -> bool
  {
    return root->children.empty();
  }
};
