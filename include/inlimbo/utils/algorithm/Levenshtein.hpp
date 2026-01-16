#pragma once

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

/**
 * The Levenshtein distance is a metric for measuring the difference between two strings.
 * It calculates the minimum number of single-character edits (insertions, deletions, or
 * substitutions) required to transform one string into another.
 */

/**
 * Computes the Levenshtein distance between two strings.
 *
 * The Levenshtein distance is calculated by finding the minimum number of operations required to
 * transform one string into another. The allowed operations are:
 * - **Insertion**: Insert a character into one string.
 * - **Deletion**: Delete a character from one string.
 * - **Substitution**: Replace a character in one string with another.
 *
 * This implementation uses dynamic programming to compute the Levenshtein distance. A 2D table is
 * constructed, where each entry at [i][j] contains the distance between the first i characters of
 * s1 and the first j characters of s2. The final distance is found at the bottom-right corner of
 * the table. The time complexity of this algorithm is O(m * n), where m and n are the lengths of
 * the two input strings.
 */

namespace utils::algorithm
{

class StringDistance final
{
public:
  StringDistance()  = delete;
  ~StringDistance() = delete;

  StringDistance(const StringDistance&)                    = delete;
  auto operator=(const StringDistance&) -> StringDistance& = delete;

  StringDistance(StringDistance&&)                    = delete;
  auto operator=(StringDistance&&) -> StringDistance& = delete;

  static auto levenshteinDistance(const std::string& s1, const std::string& s2,
                                  size_t maxDistance = SIZE_MAX) -> size_t
  {
    const size_t len1 = s1.size();
    const size_t len2 = s2.size();

    if (maxDistance != SIZE_MAX)
    {
      const size_t diff = (len1 > len2) ? (len1 - len2) : (len2 - len1);
      if (diff > maxDistance)
        return maxDistance + 1;
    }

    if (len1 == 0)
      return (len2 <= maxDistance) ? len2 : (maxDistance + 1);
    if (len2 == 0)
      return (len1 <= maxDistance) ? len1 : (maxDistance + 1);

    // 2-row DP (less memory than 2D table)
    std::vector<size_t> prev(len2 + 1);
    std::vector<size_t> cur(len2 + 1);

    for (size_t j = 0; j <= len2; ++j)
      prev[j] = j;

    for (size_t i = 1; i <= len1; ++i)
    {
      cur[0] = i;

      size_t jStart = 1;
      size_t jEnd   = len2;

      if (maxDistance != SIZE_MAX)
      {
        jStart = (i > maxDistance) ? (i - maxDistance) : 1;
        jEnd   = std::min(len2, i + maxDistance);

        for (size_t j = 1; j < jStart; ++j)
          cur[j] = maxDistance + 1;
        for (size_t j = jEnd + 1; j <= len2; ++j)
          cur[j] = maxDistance + 1;
      }

      size_t rowMin = SIZE_MAX;

      for (size_t j = jStart; j <= jEnd; ++j)
      {
        const size_t cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;

        const size_t del = prev[j] + 1;
        const size_t ins = cur[j - 1] + 1;
        const size_t sub = prev[j - 1] + cost;

        const size_t v = std::min({del, ins, sub});
        cur[j]         = v;

        if (v < rowMin)
          rowMin = v;
      }

      if (maxDistance != SIZE_MAX && rowMin > maxDistance)
        return maxDistance + 1;

      std::swap(prev, cur);
    }

    const size_t result = prev[len2];
    return (maxDistance != SIZE_MAX && result > maxDistance) ? (maxDistance + 1) : result;
  }

  static auto bestCandidate(const std::vector<std::string>& candidates, const std::string& query)
    -> std::string
  {
    if (candidates.empty() || query.empty())
      return {};

    size_t      bestDist = SIZE_MAX;
    std::string best;

    for (const auto& c : candidates)
    {
      if (c.empty())
        continue;

      const size_t d = levenshteinDistance(c, query);
      if (d < bestDist)
      {
        bestDist = d;
        best     = c;
      }
    }

    return best;
  }
};

} // namespace utils::algorithm
