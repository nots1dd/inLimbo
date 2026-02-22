#include "frontend/ftxui/state/library/Impl.hpp"

#include "query/SongMap.hpp"
#include "utils/Index.hpp"
#include "utils/string/Transforms.hpp"
#include "utils/timer/Timer.hpp"
#include <algorithm>

using namespace ftxui;

namespace frontend::tui::state::library
{

static auto pulse_fast() -> bool
{
  using namespace std::chrono;
  auto now = steady_clock::now().time_since_epoch();
  return (duration_cast<milliseconds>(now).count() / 400) % 2;
}

static auto pulse_slow() -> bool
{
  using namespace std::chrono;
  auto now = steady_clock::now().time_since_epoch();
  return (duration_cast<milliseconds>(now).count() / 900) % 2;
}

LibraryState::LibraryState(threads::SafeMap<SongMap>* map) : m_songMapTS(map) {}

void LibraryState::rebuild()
{
  artists.clear();
  album_view_lines.clear();
  view_song_objects.clear();
  album_elements_base.clear();
  album_header_indices.clear();

  if (!m_songMapTS)
    return;

  query::songmap::read::forEachArtist(*m_songMapTS,
                                      [&](const Artist& artist, const AlbumMap&) -> void
                                      { artists.push_back(artist); });

  if (!artists.empty())
    buildAlbumViewForArtist(artists[selected_artist]);
}

void LibraryState::rebuildForSelectedArtist(int selected_artist_)
{
  if (selected_artist_ < 0 || selected_artist_ >= (int)artists.size())
    return;

  selected_artist      = selected_artist_;
  selected_album_index = 0;
  current_artist_cache = selected_artist_;
  buildAlbumViewForArtist(artists[selected_artist]);
}

void LibraryState::buildAlbumViewForArtist(const Artist& artist)
{
  album_elements_base.clear();
  view_song_objects.clear();
  album_header_indices.clear();
  disc_header_indices.clear();

  Album current_album;
  Disc  current_disc = -1;

  query::songmap::read::forEachSongInArtist(
    *m_songMapTS, artist,
    [&](const Album& album, Disc disc, Track track, const ino_t,
        const std::shared_ptr<Song>& song) -> void
    {
      if (album != current_album)
      {
        current_album = album;
        current_disc  = -1;

        int header_index = (int)album_elements_base.size();

        album_elements_base.push_back(
          hbox({text("▌ "), text(utils::string::transform::trim(album, 50))}) | frame);

        album_header_indices.push_back(header_index);
        view_song_objects.push_back(nullptr);
      }

      if (disc != current_disc)
      {
        current_disc = disc;

        album_elements_base.push_back(hbox({text("  "), text("Disc " + std::to_string(disc))}) |
                                      frame);

        disc_header_indices.push_back((int)album_elements_base.size() - 1);
        view_song_objects.push_back(nullptr);
      }

      std::string line = (track < 10 ? "0" : "") + std::to_string(track) + "  " +
                         utils::string::transform::trim(song->metadata.title, 50);

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

    auto   current = (size_t)selected_artist;
    size_t count   = artists.size();

    std::optional<size_t> next =
      delta > 0 ? utils::index::nextWrap(current, count) : utils::index::prevWrap(current, count);

    if (next)
      rebuildForSelectedArtist((int)*next);

    return;
  }

  if (view_song_objects.empty())
    return;

  size_t count   = view_song_objects.size();
  auto   current = (size_t)selected_album_index;
  size_t idx     = current;

  do
  {
    std::optional<size_t> next =
      delta > 0 ? utils::index::nextWrap(idx, count) : utils::index::prevWrap(idx, count);

    if (!next)
      return;

    idx = *next;

    if (idx == current)
      break;

  } while (!view_song_objects[idx]);

  if (view_song_objects[idx])
    selected_album_index = (int)idx;
}

void LibraryState::decorateAlbumViewSelection(int                            selectedIndex,
                                              const std::optional<Metadata>& playingMetadata,
                                              bool                           focused)
{
  album_elements = album_elements_base;

  int active_album_header = -1;
  for (int idx : album_header_indices)
  {
    if (idx <= selectedIndex)
      active_album_header = idx;
    else
      break;
  }

  int active_album_end = album_elements.size();
  for (int idx : album_header_indices)
  {
    if (idx > active_album_header)
    {
      active_album_end = idx;
      break;
    }
  }

  const bool album_pulse = pulse_fast();
  const bool disc_pulse  = pulse_slow();

  for (int i = 0; i < (int)album_elements.size(); ++i)
  {
    const bool is_selected_row = (i == selectedIndex);
    const bool is_album_header =
      std::ranges::find(album_header_indices, i) != album_header_indices.end();

    const bool is_disc_header =
      std::ranges::find(disc_header_indices, i) != disc_header_indices.end();

    if (is_album_header)
    {
      if (i == active_album_header)
      {
        album_elements[i] = album_elements[i] | bgcolor(Color::RGB(20, 30, 40)) |
                            color(album_pulse ? Color::CyanLight : Color::Cyan) | bold;
      }
      else
      {
        album_elements[i] = album_elements[i] | color(Color::RGB(90, 110, 130));
      }
    }

    if (is_disc_header)
    {
      const bool in_active_album = i > active_album_header && i < active_album_end;

      if (in_active_album)
      {
        album_elements[i] =
          album_elements[i] | color(disc_pulse ? Color::YellowLight : Color::Yellow) | bold;
      }
      else
      {
        album_elements[i] = album_elements[i] | dim;
      }
    }

    if (is_selected_row)
    {
      album_elements[i] = album_elements[i] |
                          bgcolor(focused ? Color::RGB(40, 60, 90) : Color::GrayDark) |
                          color(focused ? Color::White : Color::RGB(180, 180, 180)) | bold;
    }
    else if (!is_album_header && !is_disc_header)
    {
      album_elements[i] = album_elements[i] | dim;
    }

    if (playingMetadata && view_song_objects[i] &&
        view_song_objects[i]->metadata.title == playingMetadata->title)
    {
      album_elements[i] = album_elements[i] | inverted;
    }
  }
}

} // namespace frontend::tui::state::library
