#ifndef FTXUI_HANDLER_HPP
#define FTXUI_HANDLER_HPP

#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

using namespace ftxui;

// This is under heavy dev currently (it barely works)

class MusicPlayer
{
public:
  MusicPlayer(
    const std::map<std::string,
                   std::map<std::string, std::map<unsigned int, std::map<unsigned int, Song>>>>&
      initial_library)
      : library(initial_library)
  {
    InitializeData();
    CreateComponents();
  }

  void Run()
  {
    auto screen = ScreenInteractive::Fullscreen();

    std::thread refresh_thread(
      [&]
      {
        while (!should_quit)
        {
          using namespace std::chrono_literals;
          std::this_thread::sleep_for(0.1s);

          if (is_playing)
          {
            current_position += 0.1;
            if (current_position >= GetCurrentSongDuration())
            {
              if (repeat_mode == RepeatMode::Single)
              {
                current_position = 0;
              }
              else
              {
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
  enum class RepeatMode
  {
    None,
    Single,
    All
  };

  enum class ViewMode
  {
    Library,
    Search
  };

  // Main data structure
  std::map<std::string, std::map<std::string, std::map<unsigned int, std::map<unsigned int, Song>>>>
    library;

  // Navigation state
  std::string  current_artist;
  std::string  current_album;
  unsigned int current_disc  = 1;
  unsigned int current_track = 1;

  // Current view lists
  std::vector<std::string> current_artist_names;
  std::vector<std::string> current_album_names;
  std::vector<std::string> current_song_names;

  // Player state
  int                selected_artist  = 0;
  int                selected_song    = 0;
  bool               is_playing       = false;
  RepeatMode         repeat_mode      = RepeatMode::None;
  ViewMode           current_view     = ViewMode::Library;
  int                volume           = 50;
  double             current_position = 0;
  std::string        search_query;
  bool               show_help        = false;
  bool               should_quit      = false;
  bool               focus_on_artists = true;
  ScreenInteractive* screen_          = nullptr;

  // UI Components
  Component artists_list;
  Component songs_list;
  Component controls;
  Component search_input;
  Component renderer;

  const std::vector<std::string> spinner_frames = {" ⠋", " ⠙", " ⠹", " ⠸", " ⠼",
                                                   " ⠴", " ⠦", " ⠧", " ⠇", " ⠏"};
  int                            spinner_frame  = 0;

  void InitializeData() { UpdateCurrentLists(); }

  void UpdateCurrentLists()
  {
    // Update artists list
    current_artist_names.clear();
    for (const auto& artist_pair : library)
    {
      current_artist_names.push_back(artist_pair.first);
    }

    // Update albums list if an artist is selected
    current_album_names.clear();
    if (!current_artist_names.empty())
    {
      current_artist     = current_artist_names[selected_artist];
      const auto& albums = library[current_artist];
      for (const auto& album_pair : albums)
      {
        current_album_names.push_back(album_pair.first);
      }
    }

    // Update songs list if an album is selected
    current_song_names.clear();
    if (!current_album_names.empty() && !library[current_artist].empty())
    {
      current_album     = current_album_names[selected_song];
      const auto& discs = library[current_artist][current_album];
      for (const auto& disc_pair : discs)
      {
        for (const auto& track_pair : disc_pair.second)
        {
          const auto&       song = track_pair.second;
          std::stringstream ss;
          ss << disc_pair.first << "-" << track_pair.first << ": " << song.metadata.title;
          if (!song.metadata.additionalProperties.empty())
          {
            ss << " [*]";
          }
          current_song_names.push_back(ss.str());
        }
      }
    }
  }

  Song* GetCurrentSong()
  {
    if (current_artist_names.empty() || current_album_names.empty() || current_song_names.empty())
    {
      return nullptr;
    }

    std::string selected  = current_song_names[selected_song];
    size_t      dash_pos  = selected.find('-');
    size_t      colon_pos = selected.find(':');
    if (dash_pos == std::string::npos || colon_pos == std::string::npos)
    {
      return nullptr;
    }

    unsigned int disc  = std::stoul(selected.substr(0, dash_pos));
    unsigned int track = std::stoul(selected.substr(dash_pos + 1, colon_pos - dash_pos - 1));

    auto& disc_map = library[current_artist][current_album][disc];
    auto  it       = disc_map.find(track);
    if (it != disc_map.end())
    {
      return &it->second;
    }
    return nullptr;
  }

  int GetCurrentSongDuration()
  {
    // miniaudio backend api class will be ported here
    return 180; // Placeholder: 3 minutes
  }

  std::string FormatTime(int seconds)
  {
    int minutes = seconds / 60;
    seconds     = seconds % 60;
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << minutes << ":" << std::setfill('0') << std::setw(2)
       << seconds;
    return ss.str();
  }

  void PlayNextSong()
  {
    if (current_song_names.empty())
      return;
    selected_song = (selected_song + 1) % current_song_names.size();

    current_position = 0;
  }

  void PlayPreviousSong()
  {
    if (current_song_names.empty())
      return;

    if (current_position > 3.0)
    {
      current_position = 0;
    }
    else
    {
      selected_song = (selected_song - 1 + current_song_names.size()) % current_song_names.size();
      current_position = 0;
    }
  }

  void CreateComponents()
  {
    MenuOption artist_menu_options;
    artist_menu_options.on_change     = [&]() {};
    artist_menu_options.focused_entry = &selected_artist;

    MenuOption song_menu_options;
    song_menu_options.on_change     = [&]() {};
    song_menu_options.focused_entry = &selected_song;

    artists_list = Menu(&current_artist_names, &selected_artist, artist_menu_options);
    songs_list   = Menu(&current_song_names, &selected_song, song_menu_options);
    search_input = Input(&search_query, "Search...");

    auto main_container = Container::Vertical({
      artists_list,
      songs_list,
      search_input,
    });

    main_container |= CatchEvent(
      [&](Event event)
      {
        if (event.is_mouse())
          return false;

        if (event.is_character())
        {
          switch (event.character()[0])
          {
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
            case 'v':
              current_view = ViewMode::Library;
              return true;
            case '/':
              current_view = ViewMode::Search;
              return true;
            case 'j':
              NavigateList(true);
              return true;
            case 'k':
              NavigateList(false);
              return true;
          }
        }

        if (event == Event::Tab)
        {
          focus_on_artists = !focus_on_artists;
          return true;
        }
        if (event == Event::ArrowDown)
        {
          NavigateList(true);
          return true;
        }
        if (event == Event::ArrowUp)
        {
          NavigateList(false);
          return true;
        }

        return false;
      });

    renderer = Renderer(main_container,
                        [&]
                        {
                          if (current_view == ViewMode::Library)
                          {
                            UpdateCurrentLists();
                          }

                          int   duration = GetCurrentSongDuration();
                          float progress = duration > 0 ? (float)current_position / duration : 0;

                          if (is_playing)
                          {
                            spinner_frame = (spinner_frame + 1) % spinner_frames.size();
                          }

                          Elements layout;
                          if (show_help)
                          {
                            layout.push_back(RenderHelpScreen());
                          }
                          else
                          {
                            layout.push_back(RenderMainInterface(progress));
                          }

                          return vbox(layout);
                        });
  }

  Element RenderHelpScreen()
  {
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
               text("v      - Library view"),
               text("/      - Search view"),
               text("q      - Quit"),
             }) |
               color(Color::Green),
             text(""),
             text("Press '?' to return to player") | color(Color::Yellow),
           }) |
           border | flex;
  }

  Color GetCurrWinColor(bool focused) { return focused ? Color::White : Color::GrayDark; }

  Element RenderMainInterface(float progress)
  {
    std::string current_song_info;
    std::string album_info;
    std::string additional_info;

    if (Song* current_song = GetCurrentSong())
    {
      const auto& metadata = current_song->metadata;
      current_song_info    = metadata.artist + " - " + metadata.title;
      album_info           = metadata.album + " (" + std::to_string(metadata.year) + ") ";

      if (metadata.genre != "Unknown Genre")
      {
        additional_info += "Genre: " + metadata.genre + " | ";
      }
      if (metadata.comment != "No Comment")
      {
        additional_info += "💭 | ";
      }
      if (metadata.lyrics != "No Lyrics")
      {
        additional_info += "🎵 | ";
      }
    }

    std::string status = std::string("  ") + (is_playing ? "▶️" : "⏸️") + " " +
                         (repeat_mode == RepeatMode::None     ? "↩️"
                          : repeat_mode == RepeatMode::Single ? "🔂"
                                                              : "🔁") +
                         std::string("     ");

    auto left_pane = vbox({
                       text(" Library") | bold | color(Color::Green) | inverted,
                       separator(),
                       artists_list->Render() | frame | flex,
                     }) |
                     border | color(GetCurrWinColor(focus_on_artists));

    auto right_pane = vbox({
                        text(" Songs") | bold | color(Color::Green) | inverted,
                        separator(),
                        songs_list->Render() | frame | flex,
                      }) |
                      border | color(GetCurrWinColor(!focus_on_artists));

    auto panes = vbox({hbox({
                         left_pane | size(WIDTH, EQUAL, 100),
                         right_pane | size(WIDTH, EQUAL, 100),
                       }) |
                       flex}) |
                 flex;

    auto progress_style = is_playing ? color(Color::Green) : color(Color::GrayDark);
    auto progress_bar   = hbox({
                          text(FormatTime((int)current_position)) | progress_style,
                          gauge(progress) | flex | progress_style,
                          text(FormatTime(GetCurrentSongDuration())) | progress_style,
                        }) |
                        flex;

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
                        text(album_info) | color(Color::Blue),
                      }) |
                      size(HEIGHT, EQUAL, 1) | bgcolor(Color::Yellow);

    return vbox({
             panes,
             hbox({
               progress_bar | border,
               volume_bar | border,
               text(current_view == ViewMode::Library ? " Library " : " Search ") | dim |
                 align_right | border,
             }),
             status_bar,
           }) |
           flex;
  }

  void NavigateList(bool move_down)
  {
    if (focus_on_artists)
    {
      if (!current_artist_names.empty())
      {
        if (move_down)
        {
          selected_artist = (selected_artist + 1) % current_artist_names.size();
        }
        else
        {
          selected_artist =
            (selected_artist - 1 + current_artist_names.size()) % current_artist_names.size();
        }
        selected_song = 0;
      }
    }
    else
    {
      if (!current_song_names.empty())
      {
        if (move_down)
        {
          selected_song = (selected_song + 1) % current_song_names.size();
        }
        else
        {
          selected_song =
            (selected_song - 1 + current_song_names.size()) % current_song_names.size();
        }
      }
    }
  }

  void TogglePlayback()
  {
    is_playing = !is_playing;
    if (is_playing && current_song_names.empty())
    {
      is_playing = false;
    }
  }

  void CycleRepeatMode()
  {
    switch (repeat_mode)
    {
      case RepeatMode::None:
        repeat_mode = RepeatMode::Single;
        break;
      case RepeatMode::Single:
        repeat_mode = RepeatMode::All;
        break;
      case RepeatMode::All:
        repeat_mode = RepeatMode::None;
        break;
    }
  }

  void SearchLibrary(const std::string& query)
  {
    if (query.empty())
    {
      current_view = ViewMode::Library;
      UpdateCurrentLists();
      return;
    }

    current_song_names.clear();
    std::string query_lower = query;
    std::transform(query_lower.begin(), query_lower.end(), query_lower.begin(), ::tolower);

    for (const auto& artist_pair : library)
    {
      for (const auto& album_pair : artist_pair.second)
      {
        for (const auto& disc_pair : album_pair.second)
        {
          for (const auto& track_pair : disc_pair.second)
          {
            const auto& song = track_pair.second;

            // Convert metadata fields to lower case for case-insensitive search (it doesnt work
            // lmao)
            std::string title_lower  = song.metadata.title;
            std::string artist_lower = song.metadata.artist;
            std::string album_lower  = song.metadata.album;
            std::transform(title_lower.begin(), title_lower.end(), title_lower.begin(), ::tolower);
            std::transform(artist_lower.begin(), artist_lower.end(), artist_lower.begin(),
                           ::tolower);
            std::transform(album_lower.begin(), album_lower.end(), album_lower.begin(), ::tolower);

            // Search across multiple fields
            if (title_lower.find(query_lower) != std::string::npos ||
                artist_lower.find(query_lower) != std::string::npos ||
                album_lower.find(query_lower) != std::string::npos)
            {
              std::stringstream ss;
              ss << artist_pair.first << " - " << song.metadata.title << " (" << album_pair.first
                 << ")";
              current_song_names.push_back(ss.str());
            }
          }
        }
      }
    }
  }

  void Quit()
  {
    should_quit = true;
    if (screen_)
    {
      screen_->Exit();
    }
  }
};

#endif
