#include "frontend/ftxui/state/library/Impl.hpp"
#include "query/SongMap.hpp"
#include "utils/Index.hpp"
#include "utils/timer/Timer.hpp"

using namespace ftxui;

namespace frontend::tui::state::library
{

static auto trimTitle(const std::string& s, std::size_t max) -> std::string
{
  if (s.size() <= max)
    return s;

  if (max <= 3)
    return s.substr(0, max);

  return s.substr(0, max - 3) + "...";
}

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
  album_elements_base.clear();

  Album current_album;
  Disc  current_disc = -1;

  query::songmap::read::forEachSongInArtist(
    *m_songMapTS, artist,
    [&](const Album& album, const Disc disc, const Track track, const ino_t,
        const std::shared_ptr<Song>& song)
    {
      if (album != current_album)
      {
        current_album = album;
        current_disc  = -1;

        album_elements_base.push_back(
          hbox({text("▌ ") | color(Color::Cyan), text(trimTitle(album, 50))}) |
          bgcolor(Color::RGB(25, 25, 25)) | color(Color::Cyan) | bold | frame);

        view_song_objects.push_back(nullptr);
      }

      if (disc != current_disc)
      {
        current_disc = disc;

        album_elements_base.push_back(hbox({text("  "), text("Disc " + std::to_string(disc))}) |
                                      bgcolor(Color::RGB(35, 35, 35)) | color(Color::YellowLight) |
                                      bold | frame);

        view_song_objects.push_back(nullptr);
      }

      std::string line = (track < 10 ? "0" : "") + std::to_string(track) + "  " +
                         trimTitle(song->metadata.title, 50);

      std::string dur = utils::timer::fmtTime(song->metadata.duration);

      album_elements_base.push_back(hbox({text("  "), text(line), filler(), text(dur) | dim}) |
                                    frame);

      view_song_objects.push_back(song);
    });

  album_elements = album_elements_base;
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

void LibraryState::decorateAlbumViewSelection(int                            selectedIndex,
                                              const std::optional<Metadata>& playingMetadata)
{
  album_elements = album_elements_base;

  for (int i = 0; i < (int)album_elements.size(); ++i)
  {
    if (i == selectedIndex)
      album_elements[i] = album_elements[i] | bgcolor(Color::RGB(40, 60, 90)) | bold;

    if (playingMetadata && view_song_objects[i] &&
        view_song_objects[i]->metadata.title == playingMetadata->title)
    {
      album_elements[i] = album_elements[i] | color(Color::Cyan);
    }
  }
}

} // namespace frontend::tui::state::library
