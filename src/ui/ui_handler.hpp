#ifndef FTXUI_HANDLER_HPP
#define FTXUI_HANDLER_HPP

#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <string>
#include <thread>

using namespace ftxui;

class MusicPlayer {
public:
  MusicPlayer(const std::map<std::string, 
              std::map<std::string, 
              std::map<unsigned int, 
              std::map<unsigned int, Song>>>>& initial_library)
    : library(initial_library) {
    InitializeData();
    CreateComponents();
  }

  void Run() {
    auto screen = ScreenInteractive::Fullscreen();
    std::thread refresh_thread([&] {
      while (!should_quit) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(0.1s);

        if (is_playing) {
          current_position += 0.1;
          if (current_position >= GetCurrentSongDuration()) {
            if (repeat_mode == RepeatMode::Single) {
              current_position = 0;
            } else {
              PlayNextSong();
            }
          }
        }
        screen.PostEvent(Event::Custom);
      }
    });

    refresh_thread.detach();
    screen_ = &screen;
    screen.Loop(renderer);
  }

private:
  enum class RepeatMode {
    None,
    Single,
    All
  };

  // Main data structure
  std::map<std::string, 
           std::map<std::string, 
           std::map<unsigned int, 
           std::map<unsigned int, Song>>>> library;

  // Navigation state
  std::string current_artist;
  unsigned int current_disc = 1;
  unsigned int current_track = 1;

  // Current view lists
  std::vector<std::string> current_artist_names;
  std::vector<std::string> current_song_names;

  // Player state
  int selected_artist = 0;
  int selected_song = 0;
  bool is_playing = false;
  RepeatMode repeat_mode = RepeatMode::None;
  int volume = 50;
  double current_position = 0;
  bool show_help = false;
  bool should_quit = false;
  bool focus_on_artists = true;
  ScreenInteractive* screen_ = nullptr;

  // UI Components
  Component artists_list;
  Component songs_list;
  Component controls;
  Component renderer;

  const std::vector<std::string> spinner_frames = {" ⠋", " ⠙", " ⠹", " ⠸", " ⠼",
                                                 " ⠴", " ⠦", " ⠧", " ⠇", " ⠏"};
  int spinner_frame = 0;

  void InitializeData() {
    // Initialize current_artist_names from library
    for (const auto& artist_pair : library) {
      current_artist_names.push_back(artist_pair.first);
    }
    
    // Sort artist names alphabetically
    std::sort(current_artist_names.begin(), current_artist_names.end());
    
    // Initialize first artist's songs if available
    if (!current_artist_names.empty()) {
      UpdateSongsForArtist(current_artist_names[0]);
    }
  }

  void UpdateSongsForArtist(const std::string& artist) {
    current_song_names.clear();

    if (library.count(artist) > 0) {
      // Group songs by album and year
      std::map<std::string, std::map<unsigned int, std::map<unsigned int, Song>>> albums;
      for (const auto& album_pair : library.at(artist)) {
        const std::string& album_name = album_pair.first;
        for (const auto& disc_pair : album_pair.second) {
          for (const auto& track_pair : disc_pair.second) {
            albums[album_name][disc_pair.first][track_pair.first] = track_pair.second;
          }
        }
      }

      // Format the album and song display
      for (const auto& [album_name, discs] : albums) {
        // Get year from the first song in the album
        const Song& first_song = discs.begin()->second.begin()->second;
        std::string album_info = album_name + " (" + std::to_string(first_song.metadata.year) + ")";
        current_song_names.push_back(album_info);

        for (const auto& [disc_number, tracks] : discs) {
          for (const auto& [track_number, song] : tracks) {
            std::stringstream ss;
            ss << "  " << disc_number << "-" << track_number << ": " << song.metadata.title;
            current_song_names.push_back(ss.str());
          }
        }
      }
    }

    // Update current artist
    current_artist = artist;
  }

  Song* GetCurrentSong() {
    if (current_artist_names.empty() || current_song_names.empty() ||
        selected_song >= current_song_names.size()) {
      return nullptr;
    }

    std::string selected = current_song_names[selected_song];
    size_t album_sep = selected.find(" | ");
    if (album_sep == std::string::npos) return nullptr;

    std::string album = selected.substr(0, album_sep);
    
    size_t dash_pos = selected.find('-', album_sep);
    size_t colon_pos = selected.find(':', album_sep);
    if (dash_pos == std::string::npos || colon_pos == std::string::npos) {
      return nullptr;
    }

    unsigned int disc = std::stoul(selected.substr(album_sep + 3, dash_pos - (album_sep + 3)));
    unsigned int track = std::stoul(selected.substr(dash_pos + 1, colon_pos - (dash_pos + 1)));

    if (library.count(current_artist) > 0 && 
        library.at(current_artist).count(album) > 0 &&
        library.at(current_artist).at(album).count(disc) > 0) {
      auto& disc_map = library.at(current_artist).at(album).at(disc);
      auto it = disc_map.find(track);
      if (it != disc_map.end()) {
        return &it->second;
      }
    }
    return nullptr;
  }

  int GetCurrentSongDuration()
  {
    if (!current_song_names.empty())
    {
      return 180;
    }
    return 0;
  }

  std::string FormatTime(int seconds) {
    int minutes = seconds / 60;
    seconds = seconds % 60;
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << minutes << ":" 
       << std::setfill('0') << std::setw(2) << seconds << " ";
    return ss.str();
  }

  void PlayNextSong() {
    if (!current_song_names.empty()) {
      selected_song = (selected_song + 1) % current_song_names.size();
      current_position = 0;
    }
  }

  void PlayPreviousSong() {
    if (!current_song_names.empty()) {
      if (current_position > 3.0) {
        current_position = 0;
      } else {
        selected_song = (selected_song - 1 + current_song_names.size()) % 
                       current_song_names.size();
        current_position = 0;
      }
    }
  }

  void CreateComponents() {
    MenuOption artist_menu_options;
    artist_menu_options.on_change = [&]() {
      if (focus_on_artists && selected_artist < current_artist_names.size()) {
        UpdateSongsForArtist(current_artist_names[selected_artist]);
      }
    };
    artist_menu_options.focused_entry = &selected_artist;

    MenuOption song_menu_options;
    song_menu_options.on_change = [&]() {};
    song_menu_options.focused_entry = &selected_song;

    artists_list = Menu(&current_artist_names, &selected_artist, artist_menu_options);
    songs_list = Menu(&current_song_names, &selected_song, song_menu_options);

    auto main_container = Container::Horizontal({
      artists_list,
      songs_list,
    });

    main_container |= CatchEvent([&](Event event) {
      if (event.is_mouse()) return false;

      if (event.is_character()) {
        switch (event.character()[0]) {
          case 'q':
          case 'Q':
            Quit();
            return true;
          case ' ':
            TogglePlayback();
            return true;
          case 'n':
            PlayNextSong();
            return true;
          case 'p':
            PlayPreviousSong();
            return true;
          case 'r':
            CycleRepeatMode();
            return true;
          case '=':
            volume = std::min(100, volume + 5);
            return true;
          case '-':
            volume = std::max(0, volume - 5);
            return true;
          case '?':
            show_help = !show_help;
            return true;
          case 'j':
            NavigateList(true);
            return true;
          case 'k':
            NavigateList(false);
            return true;
        }
      }
      
      if (event == Event::Tab) {
        focus_on_artists = !focus_on_artists;
        if (focus_on_artists) {
          artists_list->TakeFocus();
        } else {
          songs_list->TakeFocus();
        }
        return true;
      }
      if (event == Event::ArrowDown) {
        NavigateList(true);
        return true;
      }
      if (event == Event::ArrowUp) {
        NavigateList(false);
        return true;
      }

      return false;
    });

    renderer = Renderer(main_container, [&] {
      int duration = GetCurrentSongDuration();
      float progress = duration > 0 ? (float)current_position / duration : 0;

      if (is_playing) {
        spinner_frame = (spinner_frame + 1) % spinner_frames.size();
      }

      Elements layout;
      if (show_help) {
        layout.push_back(RenderHelpScreen());
      } else {
        layout.push_back(RenderMainInterface(progress));
      }

      return vbox(layout);
    });
  }

  void NavigateList(bool move_down) {
    if (focus_on_artists) {
      if (!current_artist_names.empty()) {
        if (move_down) {
          selected_artist = (selected_artist + 1) % current_artist_names.size();
        } else {
          selected_artist = (selected_artist - 1 + current_artist_names.size()) % 
                           current_artist_names.size();
        }
        UpdateSongsForArtist(current_artist_names[selected_artist]);
      }
    } else {
      if (!current_song_names.empty()) {
        if (move_down) {
          selected_song = (selected_song + 1) % current_song_names.size();
        } else {
          selected_song = (selected_song - 1 + current_song_names.size()) % 
                         current_song_names.size();
        }
      }
    }
  }

  Element RenderHelpScreen() {
    return vbox({
      text("inLimbo Controls") | bold | color(Color::Green),
      text("─────────────────") | color(Color::Green),
      vbox({
        text("Space  - Play/Pause"),
        text("n      - Next song"),
        text("p      - Previous song"),
        text("r      - Cycle repeat mode"),
        text("=      - Volume up"),
        text("-      - Volume down"),
        text("Tab    - Switch focus"),
        text("h      - Toggle help"),
        text("q      - Quit"),
      }) | color(Color::Green),
      text(""),
      text("Press '?' to return to player") | color(Color::Yellow),
    }) | border | flex;
  }

  Color GetCurrWinColor(bool focused) { 
    return focused ? Color::White : Color::GrayDark; 
  }

  Element RenderMainInterface(float progress) {
    std::string current_song_info;
    std::string year_info;
    std::string additional_info;

    if (Song* current_song = GetCurrentSong()) {
      const auto& metadata = current_song->metadata;
      current_song_info = metadata.artist + " - " + metadata.title;
      year_info = std::to_string(metadata.year);

      if (metadata.genre != "Unknown Genre") {
        additional_info += "Genre: " + metadata.genre + " | ";
      }
      if (metadata.comment != "No Comment") {
        additional_info += "💭 | ";
      }
      if (metadata.lyrics != "No Lyrics") {
        additional_info += "🎵 | ";
      }
    }

    std::string status = std::string("  ") + 
                        (is_playing ? "▶️" : "⏸️") + " " +
                        (repeat_mode == RepeatMode::None ? "↩️" :
                         repeat_mode == RepeatMode::Single ? "🔂" : "🔁") +
                        std::string("     ");

    auto left_pane = vbox({
      text(" Artists") | bold | color(Color::Green) | inverted,
      separator(),
      artists_list->Render() | frame | flex,
    }) | border | color(GetCurrWinColor(focus_on_artists));

    auto right_pane = vbox({
      text(" Songs") | bold | color(Color::Green) | inverted,
      separator(),
      songs_list->Render() | frame | flex,
    }) | border | color(GetCurrWinColor(!focus_on_artists));

    auto panes = vbox({
      hbox({
        left_pane | size(WIDTH, EQUAL, 100) | flex,
        right_pane | size(WIDTH, EQUAL, 100) | flex,
      }) | flex
    }) | flex;

    auto progress_style = is_playing ? color(Color::Green) : color(Color::White);
    auto progress_bar = hbox({
      text(FormatTime((int)current_position)) | progress_style,
      gauge(progress) | flex | progress_style,
      text(FormatTime(GetCurrentSongDuration())) | progress_style,
    });

    auto volume_bar = hbox({
      text(" Vol: ") | dim,
      gauge(volume / 100.0) | size(WIDTH, EQUAL, 10) | color(Color::Yellow),
      text(std::to_string(volume) + "%") | dim,
    });

    auto status_bar = hbox({
                        text(spinner_frames[spinner_frame]) | color(Color::Black),
                        text(status) | color(Color::Black),
                        text(current_song_info) | bold | color(Color::Red),
                        filler(),
                        text(additional_info) | color(Color::Blue),
                        text(year_info) | color(Color::Blue) | flex,
                      }) |
                      size(HEIGHT, EQUAL, 1) | bgcolor(Color::Yellow);

    return vbox({
      panes,
      hbox({
        progress_bar | border | flex,
        volume_bar | border | flex,
      }),
      status_bar,
    }) | flex;
  }

  void TogglePlayback() {
    is_playing = !is_playing;
  }

  void CycleRepeatMode() {
    repeat_mode = static_cast<RepeatMode>((static_cast<int>(repeat_mode) + 1) % 3);
  }

  void Quit() {
    should_quit = true;
    if (screen_) {
      screen_->ExitLoopClosure()();
    }
  }
};

#endif // FTXUI_HANDLER_HPP
