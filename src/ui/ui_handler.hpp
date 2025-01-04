#ifndef FTXUI_HANDLER_HPP
#define FTXUI_HANDLER_HPP

#include "../music/audio_playback.hpp"
#include "keymaps.hpp"
#include "misc.hpp"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <thread>

using namespace ftxui;

#define ALBUM_DELIM "----------- "

/* MACROS FOR SONG DETAILS */
#define STATUS_PLAYING   "<>"
#define STATUS_PAUSED    "!!"
#define LYRICS_AVAIL     "L*"
#define ADDN_PROPS_AVAIL "&*"
#define STATUS_BAR_DELIM " | "

/* STRING TRUNCATION MACROS */
#define MAX_LENGTH_SONG_NAME   50
#define MAX_LENGTH_ARTIST_NAME 20

/* SCREEN MACROS */
#define SHOW_MAIN_UI       0
#define SHOW_HELP_SCREEN   1
#define SHOW_LYRICS_SCREEN 2
#define SHOW_QUEUE_SCREEN  3

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
          UpdateVolume();
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
    std::string                                  artist;
    std::string                                  title;
    std::string                                  genre;
    std::string                                  album;
    bool                                         has_comment = false;
    bool                                         has_lyrics  = false;
    int                                          duration;
    unsigned int                                 year       = 0;
    unsigned int                                 track      = 0;
    unsigned int                                 discNumber = 0;
    std::string                                  lyrics;
    std::string                                  comment;
    std::unordered_map<std::string, std::string> additionalProperties;
  };

  PlayingState current_playing_state;

  Keybinds global_keybinds = parseKeybinds();

  std::unique_ptr<MiniAudioPlayer> audio_player;

  std::vector<Song> song_queue;
  int               current_song_queue_index = 0;

  // Main data structure
  std::map<std::string, std::map<std::string, std::map<unsigned int, std::map<unsigned int, Song>>>>
    library;

  // Navigation state
  std::string  current_artist;
  unsigned int current_disc  = 1;
  unsigned int current_track = 1;
  bool         show_dialog   = false;
  std::string  dialog_message;
  double       seekBuffer;
  bool         first_g_pressed = false; // Track the state of the first 'g' press

  // Current view lists
  std::vector<std::string> current_artist_names;
  std::vector<std::string> current_song_names;

  // Player state
  int        selected_artist  = 0;
  int        selected_song    = 0;
  bool       is_playing       = false;
  RepeatMode repeat_mode      = RepeatMode::None;
  int        volume           = 50;
  bool       muted            = false;
  int        lastVolume       = volume;
  double     current_position = 0;
  int        active_screen =
    0; // 0 -> Main UI ; 1 -> Show help ; 2 -> Show lyrics; 3 -> Songs queue screen
  bool               should_quit      = false;
  bool               focus_on_artists = true;
  ScreenInteractive* screen_          = nullptr;

  // UI Components
  Component artists_list;
  Component songs_list;
  Component controls;
  Component renderer;

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
            audio_player->play(); // plays in a different thread
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
    if (current_song_queue_index + 1 < song_queue.size())
    {
      current_song_queue_index++;
      current_position = 0;
      PlayCurrentSong();
      UpdatePlayingState();
    }
    else
    {
      show_dialog    = true;
      dialog_message = "Error: No more songs in the queue.";
    }
  }

  void PlayPreviousSong()
  {
    // If rewinding within the current song
    if (current_position > 3.0)
    {
      current_position = 0;
      PlayCurrentSong();
      return;
    }

    // Move to the previous song if possible
    if (current_song_queue_index > 0)
    {
      current_song_queue_index--;
      current_position = 0;
      PlayCurrentSong();
      UpdatePlayingState();
    }
    else
    {
      show_dialog    = true;
      dialog_message = "Error: No previous song available.";
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
        std::string album_info = ALBUM_DELIM + std::string(" ") + album_name + " (" +
                                 std::to_string(first_song.metadata.year) + ") " + ALBUM_DELIM;
        current_song_names.push_back(album_info); // Mark as a header

        for (const auto& [disc_number, tracks] : discs)
        {
          for (const auto& [track_number, song] : tracks)
          {
            std::string song_info = format_song_info(disc_number, track_number, song.metadata.title,
                                                     song.metadata.duration);
            current_song_names.push_back(song_info);
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
        rendered_items.push_back(text(item.substr(8)) | bold |
                                 getTrueColor(TrueColors::Color::Yellow) | inverted);
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

  void ClearQueue()
  {
    song_queue.clear();
    current_song_queue_index = 0;
  }

  void EnqueueAllSongsByArtist(const std::string& artist, const std::string& song_name)
  {
    // Clear the existing song list
    ClearQueue();

    bool start_enqueue = false;

    // Check if the artist exists in the library
    if (library.find(artist) == library.end())
    {
      std::cerr << "Artist not found in the library: " << artist << std::endl;
      return;
    }

    const auto& artist_data = library.at(artist);

    // Iterate through all albums, discs, and tracks
    for (const auto& album_pair : artist_data)
    {
      for (const auto& disc_pair : album_pair.second)
      {
        for (const auto& track_pair : disc_pair.second)
        {
          const Song& song = track_pair.second;

          if (song_name == format_song_info(disc_pair.first, track_pair.first, song.metadata.title,
                                            song.metadata.duration))
            start_enqueue = true;

          // Add the song to the list
          if (start_enqueue)
            song_queue.push_back(song);
        }
      }
    }
  }

  Song* GetCurrentSong()
  {
    if (!song_queue.empty() && current_song_queue_index < song_queue.size())
    {
      return &song_queue[current_song_queue_index];
    }

    return nullptr;
  }

  int GetCurrentSongDuration()
  {
    if (!current_song_names.empty() && audio_player)
    {
      return current_playing_state.duration;
    }
    return 0;
  }

  void UpdatePlayingState()
  {
    if (Song* current_song = GetCurrentSong())
    {
      const auto& metadata = current_song->metadata;

      current_playing_state.artist      = metadata.artist;
      current_playing_state.title       = metadata.title;
      current_playing_state.album       = metadata.album;
      current_playing_state.genre       = metadata.genre;
      current_playing_state.comment     = metadata.comment;
      current_playing_state.year        = metadata.year;
      current_playing_state.track       = metadata.track;
      current_playing_state.discNumber  = metadata.discNumber;
      current_playing_state.lyrics      = metadata.lyrics;
      current_playing_state.has_comment = (metadata.comment != "No Comment");
      current_playing_state.has_lyrics  = (metadata.lyrics != "No Lyrics");

      // If there's additional properties, you can either copy them or process as needed
      for (const auto& [key, value] : metadata.additionalProperties)
      {
        current_playing_state.additionalProperties[key] = value;
      }
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
        auto is_keybind_match = [&](char key) -> bool
        {
          return (event.is_character() && event.character() == std::string(1, key)) ||
                 (event == Event::Special(std::string(1, static_cast<char>(key))));
        };

        if (active_screen == SHOW_HELP_SCREEN)
        {
          if (event.is_character() && (event.character()[0] == global_keybinds.show_help ||
                                       std::toupper(global_keybinds.quit_app) ||
                                       event.character()[0] == global_keybinds.quit_app))
          {
            active_screen = SHOW_MAIN_UI;
            return true;
          }
          else if (event.is_character() && (event.character()[0] == global_keybinds.view_lyrics ||
                                            std::toupper(global_keybinds.quit_app) ||
                                            event.character()[0] == global_keybinds.quit_app))
          {
            active_screen = SHOW_MAIN_UI;
            return true;
          }
          return false; // Prevent other keys from working
        }

        if (event.is_mouse())
          return false;

        if (is_keybind_match(global_keybinds.play_song))
        {
          if (!current_artist.empty())
          {
            EnqueueAllSongsByArtist(current_artist, current_song_names[selected_song]);

            if (Song* current_song = GetCurrentSong())
            {
              // Enqueue all songs by the current artist
              current_position = 0;
              PlayCurrentSong();
              UpdatePlayingState();
            }
          }
          else
          {
            show_dialog    = true;
            dialog_message = "No artist selected to play songs from.";
          }

          return true;
        }
        // Check against keybinds using if-else instead of switch
        else if (is_keybind_match(global_keybinds.quit_app) ||
                 is_keybind_match(std::toupper(global_keybinds.quit_app)))
        {
          Quit();
          return true;
        }
        else if (is_keybind_match(global_keybinds.toggle_play))
        {
          TogglePlayback();
          return true;
        }
        else if (is_keybind_match(global_keybinds.play_song_next))
        {
          PlayNextSong();
          return true;
        }
        else if (is_keybind_match(global_keybinds.play_song_prev))
        {
          PlayPreviousSong();
          return true;
        }
        else if (is_keybind_match(global_keybinds.seek_ahead_5))
        {
          seekBuffer = audio_player->seekTime(5);
          current_position += seekBuffer;
        }
        else if (is_keybind_match(global_keybinds.seek_behind_5))
        {
          if (current_position >= 5)
          {
            seekBuffer = audio_player->seekTime(-5);
            current_position += seekBuffer;
          }
          else
          {
            current_position = 0; // resets to 0
            PlayCurrentSong();
          }
        }
        else if (is_keybind_match('r'))
        {
          CycleRepeatMode();
          return true;
        }
        else if (is_keybind_match(global_keybinds.vol_up))
        {
          volume = std::min(100, volume + 5);
          UpdateVolume();
          return true;
        }
        else if (is_keybind_match(global_keybinds.vol_down))
        {
          volume = std::max(0, volume - 5);
          UpdateVolume();
          return true;
        }
        else if (is_keybind_match(global_keybinds.toggle_mute))
        {
          muted = !muted;
          if (muted)
          {
            lastVolume = volume;
            volume     = 0;
          }
          else
          {
            volume = lastVolume;
          }
          UpdateVolume();
          return true;
        }
        else if (is_keybind_match(global_keybinds.show_help))
        {
          active_screen = SHOW_HELP_SCREEN;
          return true;
        }
        else if (is_keybind_match(global_keybinds.scroll_down))
        {
          NavigateList(true);
          return true;
        }
        else if (is_keybind_match(global_keybinds.scroll_up))
        {
          NavigateList(false);
          return true;
        }
        else if (is_keybind_match('x'))
        { // Add key to dismiss dialog
          show_dialog = false;
          return true;
        }
        else if (is_keybind_match(global_keybinds.view_lyrics) &&
                 (current_playing_state.has_lyrics || current_playing_state.has_comment))
        {
          active_screen = SHOW_LYRICS_SCREEN;
          return true;
        }
        else if (is_keybind_match(global_keybinds.goto_main_screen))
        {
          active_screen = SHOW_MAIN_UI;
        }
        // Some default keybinds
        else if (is_keybind_match('g'))
        {

          if (!first_g_pressed)
          {
            // First 'g' press
            first_g_pressed = true;
          }
          else
          {
            // Second 'g' press
            NavigateListToTop(true);
            first_g_pressed = false; // Reset the state
            return true;
          }
        }
        else if (is_keybind_match('G'))
        {
          NavigateListToTop(false);
          return true;
        }
        else if (is_keybind_match(global_keybinds.toggle_focus))
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

        /* Default keys */
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

                 Element interface;
                 if (active_screen == SHOW_HELP_SCREEN)
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
                     interface,              // Dim the background
                     RenderDialog() | center // Center the dialog both horizontally and vertically
                   });
                 }
                 if (active_screen == SHOW_LYRICS_SCREEN)
                 {
                   interface = RenderLyricsAndInfoView();
                 }

                 return vbox(interface);
               });
  }

  Element RenderDialog()
  {
    return window(
             text(" File Information ") | bold | center | getTrueColor(TrueColors::Color::White) |
               getTrueBGColor(TrueColors::Color::Gray),
             vbox({
               text(dialog_message) | getTrueColor(TrueColors::Color::Coral),
               separator() | color(Color::GrayLight),
               text("Press 'x' to close") | dim | center | getTrueColor(TrueColors::Color::Pink),
             }) |
               center) |
           size(WIDTH, LESS_THAN, 60) | size(HEIGHT, LESS_THAN, 8) |
           getTrueBGColor(TrueColors::Color::Black) | borderHeavy;
  }

  Element RenderLyricsAndInfoView()
  {
    std::vector<Element> additionalPropertiesText;
    for (const auto& [key, value] : current_playing_state.additionalProperties)
    {
      if (key != "LYRICS")
      {
        additionalPropertiesText.push_back(hbox({text(key + ": "), text(value) | dim}));
      }
    }

    auto          lyricLines    = formatLyrics(current_playing_state.lyrics);
    static size_t selected_line = 0; // Static to persist across frames
    auto          container     = Container::Vertical({});

    container |= CatchEvent(
      [&](Event event)
      {
        if (event == Event::ArrowUp || event == Event::Character('k'))
        {
          if (selected_line > 0)
            selected_line--;
          return true;
        }
        else if (event == Event::ArrowDown || event == Event::Character('j'))
        {
          if (selected_line < lyricLines.size() - 1)
            selected_line++;
          return true;
        }
        return false;
      });

    auto renderer = Renderer(container,
                             [&]
                             {
                               Elements focused_elements;
                               for (size_t i = 0; i < lyricLines.size(); ++i)
                               {
                                 if (i == selected_line)
                                 {
                                   focused_elements.push_back(text(lyricLines[i]) | bold);
                                 }
                                 else
                                 {
                                   focused_elements.push_back(text(lyricLines[i]));
                                 }
                               }

                               // Ensure the scroll view adapts to the selected line
                               return vbox(focused_elements) | vscroll_indicator | yframe | flex;
                             });

    container->Add(renderer);

    std::string end_text = "Use arrow keys to scroll, Press '" +
                           std::string(1, static_cast<char>(global_keybinds.goto_main_screen)) +
                           "' to go back home.";

    auto lyrics_pane = window(text(" Lyrics ") | bold | center, vbox({
                                                                  renderer->Render() | flex,
                                                                  separator(),
                                                                  text(end_text) | dim | center,
                                                                }));

    auto info_pane = window(text(" Additional Info ") | bold | center,
                            vbox(additionalPropertiesText) | frame | flex);

    return hbox({
      lyrics_pane | flex,
      info_pane | flex,
    });
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

  void NavigateListToTop(bool move_up)
  {
    if (focus_on_artists)
    {
      if (!current_artist_names.empty())
      {
        // Navigate to the top of the artist list
        if (move_up)
        {
          selected_artist = 0;
        }
        else
        {
          selected_artist = current_artist_names.size() - 1;
        }
      }
    }
    else
    {
      if (!current_song_names.empty())
      {
        if (move_up)
        {
          // Navigate to the first valid song, skipping headers
          selected_song = 0;
          while (selected_song < current_song_names.size() &&
                 current_song_names[selected_song].rfind(ALBUM_DELIM, 0) == 0)
          {
            ++selected_song;
          }
        }
        else
        {
          // Navigate to the last valid song
          selected_song = current_song_names.size() - 1;
          while (selected_song >= 0 && current_song_names[selected_song].rfind(ALBUM_DELIM, 0) == 0)
          {
            --selected_song;
          }
        }

        if (selected_song < 0 || selected_song >= current_song_names.size())
        {
          selected_song = 0; // Fallback
        }
      }
    }
  }

  Element RenderHelpScreen()
  {
    auto title = text("inLimbo Controls") | bold | getTrueColor(TrueColors::Color::Teal);

    // Helper function to create a single keybind row
    auto createRow =
      [&](const std::string& key, const std::string& description, TrueColors::Color color)
    {
      return hbox({
        text(key) | getTrueColor(color),
        text("  --  ") | getTrueColor(TrueColors::Color::Gray),
        text(description) | getTrueColor(TrueColors::Color::White),
      });
    };

    auto controls_list =
      vbox({
        createRow(charToStr(global_keybinds.toggle_play), "Toggle playback",
                  TrueColors::Color::Teal),
        createRow(charToStr(global_keybinds.play_song_next), "Next song",
                  TrueColors::Color::LightCyan),
        createRow(charToStr(global_keybinds.play_song_prev), "Previous song",
                  TrueColors::Color::LightCyan),
        createRow("r", "Cycle repeat mode", TrueColors::Color::LightMagenta),
        createRow(charToStr(global_keybinds.vol_up), "Volume up", TrueColors::Color::LightGreen),
        createRow(charToStr(global_keybinds.vol_down), "Volume down",
                  TrueColors::Color::LightGreen),
        createRow(charToStr(global_keybinds.toggle_mute),
                  "Toggle muting the current instance of miniaudio", TrueColors::Color::LightRed),
        createRow(charToStr(global_keybinds.toggle_focus), "Switch focus",
                  TrueColors::Color::LightYellow),
        createRow("gg", "Go to top of the current list", TrueColors::Color::LightBlue),
        createRow("G", "Go to bottom of the current list", TrueColors::Color::LightBlue),
        createRow(charToStr(global_keybinds.seek_ahead_5), "Seek ahead by 5s",
                  TrueColors::Color::Orange),
        createRow(charToStr(global_keybinds.seek_behind_5), "Seek behind by 5s",
                  TrueColors::Color::Orange),
        createRow(charToStr(global_keybinds.quit_app), "Quit", TrueColors::Color::LightRed),
        createRow(charToStr(global_keybinds.show_help), "Toggle this help",
                  TrueColors::Color::Cyan),
        createRow(charToStr(global_keybinds.goto_main_screen), "Go to song tree view",
                  TrueColors::Color::Teal),
      }) |
      getTrueColor(TrueColors::Color::LightGreen);

    auto symbols_explanation = vbox({
      hbox({text(LYRICS_AVAIL), text(" -> "), text("The current song has lyrics metadata.")}) |
        getTrueColor(TrueColors::Color::LightCyan),
      hbox({text(ADDN_PROPS_AVAIL), text("  -> "),
            text("The current song has additional properties metadata.")}) |
        getTrueColor(TrueColors::Color::LightYellow),
    });

    std::string footer_text =
      "Press '" + charToStr(global_keybinds.show_help) + "' to return to inLimbo.";
    auto footer = text(footer_text) | getTrueColor(TrueColors::Color::LightYellow) | center;

    return vbox({
             title,
             controls_list | border | flex,
             text("Symbols Legend") | bold | getTrueColor(TrueColors::Color::LightBlue),
             symbols_explanation | border | flex,
             footer,
           }) |
           flex;
  }

  Color GetCurrWinColor(bool focused)
  {
    return focused ? TrueColors::GetColor(TrueColors::Color::White)
                   : TrueColors::GetColor(TrueColors::Color::DarkGray);
  }

  Element RenderMainInterface(float progress)
  {
    std::string current_song_info;
    std::string year_info;
    std::string additional_info;

    // Use the current_playing_state instead of GetCurrentSong()
    if (!current_playing_state.artist.empty())
    {
      current_song_info = " - " + current_playing_state.title;
      year_info         = std::to_string(current_playing_state.year) + " ";

      if (current_playing_state.genre != "Unknown Genre")
      {
        additional_info += "Genre: " + current_playing_state.genre + STATUS_BAR_DELIM;
      }
      if (current_playing_state.has_comment)
      {
        additional_info += ADDN_PROPS_AVAIL;
      }
      if (current_playing_state.has_lyrics)
      {
        additional_info += STATUS_BAR_DELIM;
        additional_info += LYRICS_AVAIL;
      }
      additional_info += STATUS_BAR_DELIM;
    }

    std::string status = std::string("  ") + (is_playing ? STATUS_PLAYING : STATUS_PAUSED) + "  " +
                         (repeat_mode == RepeatMode::None     ? "↩️"
                          : repeat_mode == RepeatMode::Single ? "🔂"
                                                              : "🔁") +
                         std::string("    ");

    auto left_pane =
      vbox({
        text(" Artists") | bold | getTrueColor(TrueColors::Color::LightGreen) | inverted,
        separator(),
        artists_list->Render() | frame | flex,
      }) |
      border | color(GetCurrWinColor(focus_on_artists));

    auto right_pane =
      vbox({
        text(" Songs") | bold | getTrueColor(TrueColors::Color::LightGreen) | inverted,
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

    auto progress_style = is_playing ? getTrueColor(TrueColors::Color::SlateBlue)
                                     : getTrueColor(TrueColors::Color::Crimson);
    auto progress_bar   = hbox({
      text(FormatTime((int)current_position)) | progress_style,
      gauge(progress) | flex | progress_style,
      text(FormatTime(GetCurrentSongDuration())) | progress_style,
    });

    auto volume_bar = hbox({
      text(" Vol: ") | dim,
      gauge(volume / 100.0) | size(WIDTH, EQUAL, 10) | getTrueColor(TrueColors::Color::LightYellow),
      text(std::to_string(volume) + "%") | dim,
    });

    std::string queue_info = " ";
    queue_info += std::to_string(song_queue.size()) + " songs in queue.";
    std::string up_next_song = " Next up: ";
    if (song_queue.size() > 1)
      up_next_song += song_queue[current_song_queue_index + 1].metadata.title + " by " +
                      song_queue[current_song_queue_index + 1].metadata.artist;
    else
      up_next_song += "Next song not available.";
    auto queue_bar = hbox({
      text(queue_info) | dim | border | bold,
      text(up_next_song) | dim | border | flex | size(WIDTH, LESS_THAN, MAX_LENGTH_SONG_NAME),
    });

    auto status_bar =
      hbox({text(status) | getTrueColor(TrueColors::Color::Black),
            text(current_playing_state.artist) | getTrueColor(TrueColors::Color::Gold) | bold |
              size(WIDTH, LESS_THAN, MAX_LENGTH_ARTIST_NAME),
            text(current_song_info) | bold | getTrueColor(TrueColors::Color::LightRed) |
              size(WIDTH, LESS_THAN, MAX_LENGTH_SONG_NAME),
            filler(), // Push the right-aligned content to the end
            hbox({text(additional_info) | getTrueColor(TrueColors::Color::Black) | flex,
                  text(year_info) | getTrueColor(TrueColors::Color::Black) |
                    size(WIDTH, LESS_THAN, 15),
                  text(" ")}) |
              align_right}) |
      size(HEIGHT, EQUAL, 1) | getTrueBGColor(TrueColors::Color::White);

    return vbox({
             panes,
             hbox({
               progress_bar | border | flex,
               volume_bar | border,
               queue_bar,
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
