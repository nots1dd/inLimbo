#include "query/sort/Stats.hpp"

namespace query::sort
{

auto buildStats(const SongMap& map) -> Stats
{
  Stats s;

  for (auto& [artist, albums] : map)
  {
    size_t artistTracks = 0;

    s.artistAlbumCount[artist] = albums.size();

    for (auto& [album, discs] : albums)
    {
      size_t albumTracks = 0;

      int bestTrack = std::numeric_limits<int>::max();
      int bestYear  = 0;

      for (auto& [disc, tracks] : discs)
        for (auto& [track, inodeMap] : tracks)
          for (auto& [inode, song] : inodeMap)
          {
            albumTracks++;
            artistTracks++;

            if ((int)track < bestTrack && song.metadata.year > 0)
            {
              bestTrack = track;
              bestYear  = song.metadata.year;
            }
          }

      s.albumTrackCount[{artist, album}] = albumTracks;
      s.albumYear[{artist, album}]       = bestYear;
    }

    s.artistTrackCount[artist] = artistTracks;
  }

  return s;
}

} // namespace query::sort
