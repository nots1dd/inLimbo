#ifndef LEVENSHTEIN_DIST_HPP
#define LEVENSHTEIN_DIST_HPP

#include <algorithm>
#include <vector>
#include <string>

/**
 * @file levenshteinDist.hpp
 * @brief Contains the implementation of the Levenshtein distance algorithm.
 * 
 * The Levenshtein distance is a metric for measuring the difference between two strings.
 * It calculates the minimum number of single-character edits (insertions, deletions, or substitutions) required to transform one string into another.
 */

/**
 * @brief Computes the Levenshtein distance between two strings.
 * 
 * The Levenshtein distance is calculated by finding the minimum number of operations required to transform one string into another.
 * The allowed operations are:
 * - **Insertion**: Insert a character into one string.
 * - **Deletion**: Delete a character from one string.
 * - **Substitution**: Replace a character in one string with another.
 * 
 * @param s1 The first string.
 * @param s2 The second string.
 * @return The Levenshtein distance between the two strings.
 * 
 * @details
 * This implementation uses dynamic programming to compute the Levenshtein distance. A 2D table is constructed, where each entry at [i][j] contains the distance 
 * between the first i characters of s1 and the first j characters of s2. The final distance is found at the bottom-right corner of the table.
 * The time complexity of this algorithm is O(m * n), where m and n are the lengths of the two input strings.
 */
size_t levenshteinDistance(const std::string& s1, const std::string& s2)
{
    size_t len1 = s1.size(); ///< Length of the first string.
    size_t len2 = s2.size(); ///< Length of the second string.
    
    // 2D table to store the distances between substrings of s1 and s2.
    std::vector<std::vector<size_t>> dist(len1 + 1, std::vector<size_t>(len2 + 1));

    // Initialize the first column and row of the table.
    for (size_t i = 0; i <= len1; ++i)
        dist[i][0] = i; ///< Cost of deleting characters from s1 to match empty s2.

    for (size_t j = 0; j <= len2; ++j)
        dist[0][j] = j; ///< Cost of inserting characters into s1 to match s2.

    // Fill the rest of the table with the calculated distances.
    for (size_t i = 1; i <= len1; ++i)
    {
        for (size_t j = 1; j <= len2; ++j)
        {
            // If the characters at the current positions are the same, no cost to substitute.
            size_t cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;

            // Calculate the minimum of deletion, insertion, or substitution operations.
            dist[i][j] = std::min({ dist[i - 1][j] + 1,       ///< Deletion
                                    dist[i][j - 1] + 1,       ///< Insertion
                                    dist[i - 1][j - 1] + cost ///< Substitution
                                  });
        }
    }

    // Return the computed Levenshtein distance, which is the value at the bottom-right corner of the table.
    return dist[len1][len2];
}

#endif
