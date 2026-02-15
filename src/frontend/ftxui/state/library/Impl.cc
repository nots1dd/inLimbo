#include "frontend/ftxui/state/library/Impl.hpp"
#include "query/SongMap.hpp"
#include "utils/Index.hpp"

namespace frontend::tui::state::library
{

LibraryState::LibraryState(threads::SafeMap<SongMap>* map) : m_songMapTS(map) {}

void LibraryState::rebuild()
{
  artists.clear();
  album_view_lines.clear();
  view_song_objects.clear();

  if (!m_songMapTS)
    return;

  query::songmap::read::forEachArtist(*m_songMapTS,
                                      [&](const Artist& artist, const AlbumMap&) -> void
                                      { artists.push_back(artist); });

  if (!artists.empty())
    buildAlbumViewForArtist(artists[0]);
}

void LibraryState::rebuildForSelectedArtist(int selected_artist)
{
  if (selected_artist < 0 || selected_artist >= (int)artists.size())
    return;

  buildAlbumViewForArtist(artists[selected_artist]);
  selected_album_index = 0;
  current_artist_cache = selected_artist;
}

void LibraryState::buildAlbumViewForArtist(const Artist& artist)
{
  album_view_lines.clear();
  view_song_objects.clear();

  Album current_album;
  Disc  current_disc = -1;

  query::songmap::read::forEachSong(
    *m_songMapTS,
    [&](const Artist& a, const Album& album, const Disc disc, const Track track, const ino_t,
        const std::shared_ptr<Song>& song) -> void
    {
      if (a != artist)
        return;

      if (album != current_album)
      {
        current_album = album;
        current_disc  = -1;
        album_view_lines.push_back(album);
        view_song_objects.push_back(nullptr);
      }

      if (disc != current_disc)
      {
        current_disc = disc;
        album_view_lines.push_back("Disc " + std::to_string(disc));
        view_song_objects.push_back(nullptr);
      }

      std::string line =
        (track < 10 ? "0" : "") + std::to_string(track) + "  " + song->metadata.title;

      album_view_lines.push_back(line);
      view_song_objects.push_back(song);
    });
}

void LibraryState::moveSelection(int delta)
{
  if (focus_on_artists)
  {
    if (artists.empty())
      return;

    auto   current = static_cast<size_t>(selected_artist);
    size_t count   = artists.size();

    std::optional<size_t> next;

    if (delta > 0)
      next = utils::index::nextWrap(current, count);
    else if (delta < 0)
      next = utils::index::prevWrap(current, count);
    else
      return;

    if (next)
    {
      selected_artist = static_cast<int>(*next);
      rebuildForSelectedArtist(selected_artist);
    }

    return;
  }

  if (view_song_objects.empty())
    return;

  size_t count   = view_song_objects.size();
  auto   current = static_cast<size_t>(selected_album_index);

  size_t new_index = current;

  do
  {
    std::optional<size_t> next;

    if (delta > 0)
      next = utils::index::nextWrap(new_index, count);
    else if (delta < 0)
      next = utils::index::prevWrap(new_index, count);
    else
      return;

    if (!next)
      return;

    new_index = *next;

    if (new_index == current)
      break;

  } while (!view_song_objects[new_index]);

  if (view_song_objects[new_index])
    selected_album_index = static_cast<int>(new_index);
}

} // namespace frontend::tui::state::library
