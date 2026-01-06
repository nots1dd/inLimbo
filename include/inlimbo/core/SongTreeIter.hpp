#pragma once

#include "SongTree.hpp"

namespace core
{

class SongTreeIterator
{
public:
  using value_type = const Song&;

  SongTreeIterator() = default;

  SongTreeIterator(const SongTree* tree, query::song::SongPredicate pred)
      : m_tree(tree), m_songMap(m_tree->returnSongMap()), m_pred(std::move(pred))
  {
    if (m_tree)
    {
      itArtist = m_songMap.begin();
      advance();
    }
  }

  auto operator*() const -> const Song& { return *m_current; }
  auto operator->() const -> const Song* { return m_current; }

  auto operator++() -> SongTreeIterator&
  {
    advance();
    return *this;
  }

  auto operator==(std::default_sentinel_t) const -> bool { return m_current == nullptr; }

private:
  const SongTree*            m_tree = nullptr;
  const SongMap              m_songMap;
  query::song::SongPredicate m_pred;
  const Song*                m_current = nullptr;

  // Iterators at each level
  SongMap::const_iterator  itArtist;
  AlbumMap::const_iterator itAlbum;
  DiscMap::const_iterator  itDisc;
  TrackMap::const_iterator itTrack;
  InodeMap::const_iterator itSong;

  void advance()
  {
    m_current = nullptr;

    while (itArtist != m_songMap.end())
    {
      if (itAlbum == AlbumMap::const_iterator{})
        itAlbum = itArtist->second.begin();

      while (itAlbum != itArtist->second.end())
      {
        if (itDisc == DiscMap::const_iterator{})
          itDisc = itAlbum->second.begin();

        while (itDisc != itAlbum->second.end())
        {
          if (itTrack == TrackMap::const_iterator{})
            itTrack = itDisc->second.begin();

          while (itTrack != itDisc->second.end())
          {
            if (itSong == InodeMap::const_iterator{})
              itSong = itTrack->second.begin();

            while (itSong != itTrack->second.end())
            {
              const Song& s = itSong->second;

              if (!m_pred ||
                  m_pred(itArtist->first, itAlbum->first, itDisc->first, itTrack->first, s))
              {
                m_current = &s;
                ++itSong;
                return;
              }
              ++itSong;
            }
            itSong = {};
            ++itTrack;
          }
          itTrack = {};
          ++itDisc;
        }
        itDisc = {};
        ++itAlbum;
      }
      itAlbum = {};
      ++itArtist;
    }

    m_tree = nullptr;
  }
};

class SongTreeRange
{
public:
  SongTreeRange(const SongTree& tree, query::song::SongPredicate pred)
      : m_tree(&tree), m_pred(std::move(pred))
  {
  }

  [[nodiscard]] auto begin() const -> SongTreeIterator { return {m_tree, m_pred}; }

  [[nodiscard]] auto end() const -> std::default_sentinel_t { return {}; }

private:
  const SongTree*            m_tree;
  query::song::SongPredicate m_pred;
};

inline auto SongTree::range(query::song::SongPredicate pred) const -> SongTreeRange
{
  return {*this, std::move(pred)};
}

} // namespace core
