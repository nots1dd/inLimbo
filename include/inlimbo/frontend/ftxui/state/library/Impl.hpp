#pragma once

#include "InLimbo-Types.hpp"
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <string>
#include <vector>

namespace frontend::tui::state::library
{

class LibraryState
{
public:
  explicit LibraryState(TS_SongMap* songMap);

  void rebuild();
  void rebuildForSelectedArtist(int selected_artist);
  void decorateAlbumViewSelection(int selectedIndex, const std::optional<Metadata>& playingMetadata,
                                  bool focused);

  void moveSelection(int delta);

  auto toggleFocus() -> void { focus_on_artists = !focus_on_artists; }

  [[nodiscard]] auto focusOnArtists() const noexcept -> bool { return focus_on_artists; }

  [[nodiscard]] auto returnAlbumElements() const noexcept -> std::vector<ftxui::Element>
  {
    return album_elements;
  }

  std::vector<Artist>                artists;
  std::vector<std::string>           album_view_lines;
  std::vector<int>                   album_header_indices;
  std::vector<int>                   disc_header_indices;
  std::vector<std::shared_ptr<Song>> view_song_objects;

  int selected_artist{0};
  int selected_album_index{0};
  int current_artist_cache{0};

private:
  TS_SongMap*                 m_songMapTS;
  bool                        focus_on_artists = true;
  std::vector<ftxui::Element> album_elements;
  std::vector<ftxui::Element> album_elements_base;

  void buildAlbumViewForArtist(const Artist& artist);
};

} // namespace frontend::tui::state::library
