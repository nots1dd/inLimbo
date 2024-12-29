#ifndef FTXUI_HANDLER_HPP
#define FTXUI_HANDLER_HPP

#include "../music/audio_playback.hpp"
#include <algorithm>
#include <chrono>
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <iomanip>
#include <sstream>
#include <string>
#include <thread>

using namespace ftxui;

#define ALBUM_DELIM "----------- "

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
    auto        screen = ScreenInteractive::Fullscreen();
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

  struct PlayingState
  {
    std::string artist;
    std::string title;
    std::string year;
    std::string genre;
    bool        has_comment = false;
    bool        has_lyrics  = false;
    int         duration;
  };

  PlayingState current_playing_state;

  std::unique_ptr<MiniAudioPlayer> audio_player;

  // Main data structure
  std::map<std::string, std::map<std::string, std::map<unsigned int, std::map<unsigned int, Song>>>>
    library;

  // Navigation state
  std::string  current_artist;
  unsigned int current_disc  = 1;
  unsigned int current_track = 1;
  bool         show_dialog   = false;
  std::string  dialog_message;

  // Current view lists
  std::vector<std::string> current_artist_names;
  std::vector<std::string> current_song_names;

  // Player state
  int                selected_artist  = 0;
  int                selected_song    = 0;
  bool               is_playing       = false;
  RepeatMode         repeat_mode      = RepeatMode::None;
  int                volume           = 50;
  double             current_position = 0;
  bool               show_help        = false;
  bool               should_quit      = false;
  bool               focus_on_artists = true;
  ScreenInteractive* screen_          = nullptr;

  // UI Components
  Component artists_list;
  Component songs_list;
  Component controls;
  Component renderer;

  const std::vector<std::string> spinner_frames = {" ⠋", " ⠙", " ⠹", " ⠸", " ⠼",
                                                   " ⠴", " ⠦", " ⠧", " ⠇", " ⠏"};
  int                            spinner_frame  = 0;

  void InitializeData()
  {
    // Initialize current_artist_names from library
    for (const auto& artist_pair : library)
    {
      current_artist_names.push_back(artist_pair.first);
    }

    // Sort artist names alphabetically
    std::sort(current_artist_names.begin(), current_artist_names.end());

    // Initialize first artist's songs if available
    if (!current_artist_names.empty())
    {
      UpdateSongsForArtist(current_artist_names[0]);
    }
  }

  /* Miniaudio class integrations */

  void PlayCurrentSong()
  {
    std::thread(
      [&]
      {
        // Ensure we have a valid audio player instance
        if (!audio_player)
        {
          show_dialog    = true;
          dialog_message = "Error: Audio player is not initialized.";
          sleep(1);
          audio_player = std::make_unique<MiniAudioPlayer>();
        }

        Song* current_song = GetCurrentSong();
        if (!current_song)
        {
          show_dialog    = true;
          dialog_message = "Error: No current song found.";
          return;
        }

        try
        {
          // Check if the file path is valid before attempting to load
          const std::string& file_path = current_song->metadata.filePath;
          if (file_path.empty())
          {
            show_dialog    = true;
            dialog_message = "Error: Invalid file path.";
            return;
          }

          if (is_playing)
          {
            audio_player->stop();
            is_playing = false;
          }

          // Load the audio file
          int loadAudioFileStatus = audio_player->loadFile(file_path);

          if (loadAudioFileStatus != -1)
          {
            is_playing = true;
            audio_player->play();
            current_playing_state.duration = audio_player->getDuration();
          }
          else
          {
            show_dialog    = true;
            dialog_message = "Error: Failed to load the audio file.";
            is_playing     = false;
          }
        }
        catch (const std::exception& e)
        {
          show_dialog    = true;
          dialog_message = "Err: " + std::string(e.what());
          is_playing     = false;
        }
      })
      .detach();
  }

  void TogglePlayback()
  {
    if (!audio_player)
      return;

    if (is_playing)
    {
      audio_player->pause();
      is_playing = false;
    }
    else
    {
      if (audio_player)
      {
        audio_player->resume();
      }
      else
      {
        PlayCurrentSong();
      }
      is_playing = true;
    }
  }

  void UpdateVolume()
  {
    if (audio_player)
    {
      audio_player->setVolume(volume / 100.0f);
    }
  }

  void PlayNextSong()
  {
    if (!current_song_names.empty())
    {
      do
      {
        selected_song = (selected_song + 1) % current_song_names.size();
      } while (current_song_names[selected_song].rfind(ALBUM_DELIM, 0) == 0);

      current_position = 0;
      PlayCurrentSong();
      UpdatePlayingState();
    }
  }

  void PlayPreviousSong()
  {
    if (!current_song_names.empty())
    {
      if (current_position > 3.0)
      {
        current_position = 0;
        PlayCurrentSong();
      }
      else
      {
        do
        {
          selected_song =
            (selected_song - 1 + current_song_names.size()) % current_song_names.size();
        } while (current_song_names[selected_song].rfind(ALBUM_DELIM, 0) == 0);

        current_position = 0;
        PlayCurrentSong();
        UpdatePlayingState();
      }
    }
  }

  /* ------------------------------- */

  void UpdateSongsForArtist(const std::string& artist)
  {
    current_song_names.clear();

    if (library.count(artist) > 0)
    {
      // Group songs by album and year
      std::map<std::string, std::map<unsigned int, std::map<unsigned int, Song>>> albums;
      for (const auto& album_pair : library.at(artist))
      {
        const std::string& album_name = album_pair.first;
        for (const auto& disc_pair : album_pair.second)
        {
          for (const auto& track_pair : disc_pair.second)
          {
            albums[album_name][disc_pair.first][track_pair.first] = track_pair.second;
          }
        }
      }

      // Format the album and song display
      for (const auto& [album_name, discs] : albums)
      {
        // Get year from the first song in the album
        const Song& first_song = discs.begin()->second.begin()->second;
        std::string album_info = album_name + " (" + std::to_string(first_song.metadata.year) + ")";
        current_song_names.push_back(ALBUM_DELIM + album_info + " " +
                                     ALBUM_DELIM); // Mark as a header

        for (const auto& [disc_number, tracks] : discs)
        {
          for (const auto& [track_number, song] : tracks)
          {
            std::stringstream ss;
            ss << disc_number << "-" << track_number << ": " << song.metadata.title;
            current_song_names.push_back(ss.str());
          }
        }
      }
    }

    current_artist = artist;
  }

  Element RenderSongsList()
  {
    Elements rendered_items;
    for (size_t i = 0; i < current_song_names.size(); ++i)
    {
      const std::string& item = current_song_names[i];
      if (item.rfind(ALBUM_DELIM, 0) == 0)
      {
        rendered_items.push_back(text(item.substr(8)) | bold | color(Color::Yellow) | inverted);
      }
      else
      {
        bool is_selected = (i == selected_song);
        auto style       = is_selected ? inverted : nothing;
        rendered_items.push_back(text(item) | style);
      }
    }

    return vbox(std::move(rendered_items)) | frame | flex;
  }

  Song* GetCurrentSong()
  {
    // Check if we have valid data to fetch the song
    if (current_artist.empty() || selected_song < 0 || selected_song >= current_song_names.size())
    {
      return nullptr;
    }

    // Get the selected song name
    const std::string& selected_song_name = current_song_names[selected_song];

    // Try to find the artist in the library
    if (library.find(current_artist) == library.end())
    {
      return nullptr;
    }

    const auto& artist_data = library.at(current_artist);

    // Loop over all albums in the artist's library
    for (const auto& album_pair : artist_data)
    {
      const std::string& album_name = album_pair.first;

      // Loop over all discs in the album
      for (const auto& disc_pair : album_pair.second)
      {
        const unsigned int disc_number = disc_pair.first;

        // Loop over all tracks in the disc
        for (const auto& track_pair : disc_pair.second)
        {
          const unsigned int track_number = track_pair.first;
          const Song&        song         = track_pair.second;

          // Create a formatted string that matches the song display format
          std::stringstream song_identifier;
          song_identifier << disc_number << "-" << track_number << ": " << song.metadata.title;

          // If the formatted song string matches the selected song
          if (selected_song_name == song_identifier.str())
          {
            return const_cast<Song*>(&song); // Return the song
          }
        }
      }
    }
    return nullptr; // Return null if no match found
  }

  int GetCurrentSongDuration()
  {
    if (!current_song_names.empty() && audio_player)
    {
      return current_playing_state.duration;
    }
    return 0;
  }

  std::string FormatTime(int seconds)
  {
    int minutes = seconds / 60;
    seconds     = seconds % 60;
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << minutes << ":" << std::setfill('0') << std::setw(2)
       << seconds << " ";
    return ss.str();
  }

  void UpdatePlayingState()
  {
    if (Song* current_song = GetCurrentSong())
    {
      const auto& metadata              = current_song->metadata;
      current_playing_state.artist      = metadata.artist;
      current_playing_state.title       = metadata.title;
      current_playing_state.year        = std::to_string(metadata.year);
      current_playing_state.genre       = metadata.genre;
      current_playing_state.has_comment = (metadata.comment != "No Comment");
      current_playing_state.has_lyrics  = (metadata.lyrics != "No Lyrics");
    }
  }

  void CreateComponents()
  {
    MenuOption artist_menu_options;
    artist_menu_options.on_change = [&]()
    {
      if (focus_on_artists && selected_artist < current_artist_names.size())
      {
        UpdateSongsForArtist(current_artist_names[selected_artist]);
      }
    };
    artist_menu_options.focused_entry = &selected_artist;

    MenuOption song_menu_options;
    song_menu_options.on_change     = [&]() {};
    song_menu_options.focused_entry = &selected_song;

    artists_list = Menu(&current_artist_names, &selected_artist, artist_menu_options);
    songs_list   = Menu(&current_song_names, &selected_song, song_menu_options);

    auto main_container = Container::Horizontal({
      artists_list,
      Renderer([&] { return RenderSongsList(); }),
    });

    main_container |= CatchEvent(
      [&](Event event)
      {
        if (event.is_mouse())
          return false;

        if (event == Event::Return)
        { // Handle Enter key
          if (Song* current_song = GetCurrentSong())
          {
            current_position = 0;
            PlayCurrentSong();
            UpdatePlayingState();
          }
          return true;
        }

        // Handle other events
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
              UpdateVolume();
              return true;
            case '-':
              volume = std::max(0, volume - 5);
              UpdateVolume();
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
            case 'x': // Add key to dismiss dialog
              show_dialog = false;
              return true;
          }
        }

        if (event == Event::Tab)
        {
          focus_on_artists = !focus_on_artists;
          if (focus_on_artists)
          {
            artists_list->TakeFocus();
          }
          else
          {
            songs_list->TakeFocus();
          }
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

    renderer =
      Renderer(main_container,
               [&]
               {
                 int   duration = GetCurrentSongDuration();
                 float progress = duration > 0 ? (float)current_position / duration : 0;

                 if (is_playing)
                 {
                   spinner_frame = (spinner_frame + 1) % spinner_frames.size();
                 }

                 Element interface;
                 if (show_help)
                 {
                   interface = RenderHelpScreen();
                 }
                 else
                 {
                   interface = RenderMainInterface(progress);
                 }

                 if (show_dialog)
                 {
                   // Create a semi-transparent overlay with the dialog box
                   interface = dbox({
                     interface | dim,        // Dim the background
                     RenderDialog() | center // Center the dialog both horizontally and vertically
                   });
                 }

                 return vbox(interface);
               });
  }

  Element RenderDialog()
  {
    return window(text("File Information") | bold | center,
                  vbox({
                    text(dialog_message),
                    separator(),
                    text("Press 'x' to close") | dim | center,
                  })) |
           size(WIDTH, LESS_THAN, 60) | size(HEIGHT, LESS_THAN, 5) | bgcolor(Color::White) | border;
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
        UpdateSongsForArtist(current_artist_names[selected_artist]);
      }
    }
    else
    {
      if (!current_song_names.empty())
      {
        do
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
        } while (current_song_names[selected_song].rfind(ALBUM_DELIM, 0) == 0); // Skip headers
      }
    }
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
    std::string year_info;
    std::string additional_info;

    // Use the current_playing_state instead of GetCurrentSong()
    if (!current_playing_state.artist.empty())
    {
      current_song_info = current_playing_state.artist + " - " + current_playing_state.title;
      year_info         = current_playing_state.year;

      if (current_playing_state.genre != "Unknown Genre")
      {
        additional_info += "Genre: " + current_playing_state.genre + " | ";
      }
      if (current_playing_state.has_comment)
      {
        additional_info += "💭 | ";
      }
      if (current_playing_state.has_lyrics)
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
                       text(" Artists") | bold | color(Color::Green) | inverted,
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
                         left_pane | size(WIDTH, EQUAL, 100) | flex,
                         right_pane | size(WIDTH, EQUAL, 100) | flex,
                       }) |
                       flex}) |
                 flex;

    auto progress_style = is_playing ? color(Color::Green) : color(Color::White);
    auto progress_bar   = hbox({
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
           }) |
           flex;
  }

  void CycleRepeatMode()
  {
    repeat_mode = static_cast<RepeatMode>((static_cast<int>(repeat_mode) + 1) % 3);
  }

  void Quit()
  {
    if (audio_player)
      audio_player->stop();
    should_quit = true;
    if (screen_)
    {
      screen_->ExitLoopClosure()();
    }
  }
};

#endif // FTXUI_HANDLER_HPP
