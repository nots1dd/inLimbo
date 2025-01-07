#ifndef FTXUI_HANDLER_HPP
#define FTXUI_HANDLER_HPP

#include "../music/audio_playback.hpp"
#include "./components/scroller.hpp"
#include "keymaps.hpp"
#include "misc.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <unordered_set>

using namespace ftxui;

/* MACROS FOR SONG DETAILS */
#define STATUS_PLAYING   "<>"
#define STATUS_PAUSED    "!!"
#define LYRICS_AVAIL     "L*"
#define ADDN_PROPS_AVAIL "&*"
#define STATUS_BAR_DELIM " | "

/* STRING TRUNCATION MACROS */
#define MAX_LENGTH_SONG_NAME   50
#define MAX_LENGTH_ARTIST_NAME 30

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
              PlayNextSong();
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

  Keybinds      global_keybinds = parseKeybinds();
  InLimboColors global_colors   = parseColors();

  std::unique_ptr<MiniAudioPlayer> audio_player;
  std::mutex                       play_mutex;
  std::atomic<bool>                is_processing{false};
  std::atomic<bool>                is_playing{false};
  std::atomic<bool>                is_thread_running{false};
  std::thread                      audio_thread;

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
  std::vector<std::string>  current_artist_names;
  std::vector<Element>      current_song_elements;
  std::vector<unsigned int> current_inodes;
  std::unordered_set<int>   album_name_indices;
  int albums_indices_traversed = 1; // first element of current_song_elements is always an album

  // Player state
  int selected_artist    = 0;
  int selected_inode     = 1; // First element is always going to album name so we ignore it
  int current_lyric_line = 0;
  std::vector<Element> lyricElements;
  int                  volume           = 50;
  bool                 muted            = false;
  int                  lastVolume       = volume;
  double               current_position = 0;
  int                  active_screen =
    0; // 0 -> Main UI ; 1 -> Show help ; 2 -> Show lyrics; 3 -> Songs queue screen
  bool               should_quit      = false;
  bool               focus_on_artists = true;
  ScreenInteractive* screen_          = nullptr;

  // UI Components
  Component artists_list;
  Component songs_list;
  Component scroller;
  Component lyrics_scroller;
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
    try
    {
      std::unique_lock<std::mutex> lock(play_mutex, std::try_to_lock);
      if (!lock || is_processing)
      {
        return; // Another invocation is already running
      }
      is_processing = true;

      if (is_thread_running)
      {
        if (audio_thread.joinable())
        {
          audio_thread.join(); // Wait for the previous thread to finish
        }
      }

      // Spawn a new thread to play the current song
      audio_thread = std::thread(
        [&]()
        {
          is_thread_running = true;

          if (!audio_player)
          {
            audio_player = std::make_unique<MiniAudioPlayer>();
          }

          Song* current_song = GetCurrentSongFromQueue();
          if (!current_song)
          {
            show_dialog       = true;
            dialog_message    = "Error: No current song found.";
            is_processing     = false;
            is_thread_running = false;
            return;
          }

          try
          {
            const std::string& file_path = current_song->metadata.filePath;
            if (file_path.empty())
            {
              show_dialog       = true;
              dialog_message    = "Error: Invalid file path.";
              is_processing     = false;
              is_thread_running = false;
              return;
            }

            if (is_playing)
            {
              audio_player->stop(); // Stop the current song if playing
              is_playing = false;
            }

            // Load the audio file
            int loadAudioFileStatus = audio_player->loadFile(file_path);
            if (loadAudioFileStatus != -1)
            {
              is_playing = true;
              audio_player->play(); // Play the song (it will play in a separate thread)
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

          // Mark processing as complete and cleanup the thread
          is_processing     = false;
          is_thread_running = false;
        });

      audio_thread.detach(); // Since we cant access this thread, we cant manually free it (WHICH
                             // CAUSES SOME ISSUES)
    }
    catch (const std::exception& e)
    {
      dialog_message = "Something went wrong! Info: " + std::string(e.what());
      show_dialog    = true;
      is_playing     = false;
    }
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

  void ReplaySong()
  {
    current_position = 0;
    PlayCurrentSong();
    return;
  }

  void PlayPreviousSong()
  {
    // If rewinding within the current song
    if (current_position > 3.0)
    {
      ReplaySong();
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
    current_inodes.clear();
    current_song_elements.clear();
    album_name_indices.clear();

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
        current_song_elements.push_back(
          renderAlbumName(album_name, first_song.metadata.year, global_colors.album_name_bg));
        album_name_indices.insert(current_song_elements.size() - 1);

        for (const auto& [disc_number, tracks] : discs)
        {
          for (const auto& [track_number, song] : tracks)
          {
            std::string disc_track_info =
              std::to_string(disc_number) + "-" + std::to_string(track_number) + "  ";
            current_inodes.push_back(song.inode);
            current_song_elements.push_back(
              renderSongName(disc_track_info, song.metadata.title, song.metadata.duration));
          }
        }
      }
    }

    current_artist = artist;
  }

  void ClearQueue()
  {
    song_queue.clear();
    current_song_queue_index = 0;
  }

  const Song& GetCurrentSong(const std::string& artist)
  {
    const auto& artist_data = library.at(artist);

    // Iterate through all albums, discs, and tracks
    for (const auto& album_pair : artist_data)
    {
      for (const auto& disc_pair : album_pair.second)
      {
        for (const auto& track_pair : disc_pair.second)
        {
          const Song& song = track_pair.second;

          if (current_inodes[selected_inode - albums_indices_traversed] == song.inode)
          {
            return song; // Return a reference to the matched song.
          }
        }
      }
    }

    throw std::runtime_error("Song not found.");
  }

  void EnqueueAllSongsByArtist(const std::string& artist)
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

          if (current_inodes[selected_inode - albums_indices_traversed] == song.inode)
          {
            start_enqueue = true;
          }
          // Add the song to the list
          if (start_enqueue)
            song_queue.push_back(song);
        }
      }
    }
  }

  void AddSongToQueue()
  {
    // Validate indices to prevent out-of-range access
    if (selected_artist >= current_artist_names.size() ||
        selected_inode - albums_indices_traversed >= current_inodes.size())
    {
      throw std::runtime_error("Invalid artist or song selection.");
    }

    // Get a const reference to the current song
    const Song& current_preview_song = GetCurrentSong(current_artist_names[selected_artist]);

    // Add a copy of the song to the queue
    song_queue.push_back(current_preview_song);

    dialog_message = "Song added to queue.";
    show_dialog    = true;
  }

  Song* GetCurrentSongFromQueue()
  {
    if (!song_queue.empty() && current_song_queue_index < song_queue.size())
    {
      return &song_queue[current_song_queue_index];
    }

    dialog_message = "Something went worng";
    show_dialog = true;
    return nullptr;
  }

  int GetCurrentSongDuration()
  {
    if (!current_inodes.empty() && audio_player)
    {
      return current_playing_state.duration;
    }
    return 0;
  }

  void UpdatePlayingState()
  {
    if (Song* current_song = GetCurrentSongFromQueue())
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
    song_menu_options.focused_entry = &selected_inode;

    artists_list = Menu(&current_artist_names, &selected_artist, artist_menu_options);
    songs_list   = Renderer(
      [&]() mutable
      {
        return RenderSongMenu(current_song_elements, &selected_inode,
                                global_colors.menu_cursor_bg); // This should return an Element
      });

    auto main_container = Container::Horizontal({artists_list, songs_list});

    /* Adding DEBOUNCE TIME (Invoking PlayCurrentSong() too fast causes resources to not be freed)
     */

    auto last_event_time = std::chrono::steady_clock::now(); // Tracks the last event time
    int  debounce_time = std::stoi(std::string(parseTOMLField(PARENT_DBG, "debounce_time_in_ms")));
    if (debounce_time < 500)
      debounce_time = 500; // min debounce time is 0.5s
    const int final_debounce_time = debounce_time;
    auto      debounce_duration   = std::chrono::milliseconds(final_debounce_time);

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

        if (is_keybind_match(global_keybinds.play_song) && !focus_on_artists)
        {
          if (!current_artist.empty())
          {
            EnqueueAllSongsByArtist(current_artist);

            if (Song* current_song = GetCurrentSongFromQueue())
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
          auto now = std::chrono::steady_clock::now();
          if (now - last_event_time < debounce_duration)
            return false;
          PlayNextSong();
          last_event_time = now; // Update the last event time
          return true;
        }
        else if (is_keybind_match(global_keybinds.play_song_prev))
        {
          auto now = std::chrono::steady_clock::now();
          if (now - last_event_time < debounce_duration)
            return false;
          last_event_time = now;
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
          if (current_position > 5)
          {
            seekBuffer = audio_player->seekTime(-5);
            current_position += seekBuffer;
          }
          else
          {
            current_position = 0; // resets to 0
            PlayCurrentSong();
          }
          UpdatePlayingState();
        }
        else if (is_keybind_match(global_keybinds.replay_song))
        {
          ReplaySong();
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
        else if (is_keybind_match(global_keybinds.add_song_to_queue))
        {
          AddSongToQueue();
          return true;
        }

        /* Default keys */
        /*if (event == Event::ArrowDown)*/
        /*{*/
        /*  NavigateList(true);*/
        /*  return true;*/
        /*}*/
        /*if (event == Event::ArrowUp)*/
        /*{*/
        /*  NavigateList(false);*/
        /*  return true;*/
        /*}*/

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
                   interface =
                     dbox({
                       interface,              // Dim the background
                       RenderDialog() | center // Center the dialog both horizontally and vertically
                     }) |
                     getTrueColor(TrueColors::Color::White);
                 }
                 if (active_screen == SHOW_LYRICS_SCREEN)
                 {
                   // TODO: Fix lyrics scroll
                   lyrics_scroller =
                     Scroller(Renderer([&]() mutable { return RenderLyricsAndInfoView(); }));
                   interface = lyrics_scroller->Render();
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

  void UpdateLyrics()
  {
    lyricElements.clear();
    auto          lyricLines    = formatLyrics(current_playing_state.lyrics);
    static size_t selected_line = 0; // Static to persist across frames

    for (const std::string s : lyricLines)
      lyricElements.push_back(vbox(text(s)));

    return;
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

    UpdateLyrics();

    scroller = Scroller(Renderer([&] { return vbox(lyricElements) | vscroll_indicator; }));

    std::string end_text = "Use arrow keys to scroll, Press '" +
                           std::string(1, static_cast<char>(global_keybinds.goto_main_screen)) +
                           "' to go back home.";

    auto lyrics_pane = Renderer(scroller,
                                [&]
                                {
                                  return vbox({
                                    vbox(lyricElements),
                                    separator(),
                                    text(end_text) | dim | center,
                                  });
                                });

    auto info_pane = window(text(" Additional Info ") | bold | center,
                            vbox(additionalPropertiesText) | frame | flex);

    return hbox({
      lyrics_pane->Render() | flex | border,
      info_pane | flex | border,
    });
  }

  void NavigateSongMenu(bool move_down)
  {
    int initial_inode = selected_inode; // Store the initial index to detect infinite loops
    do
    {
      if (move_down)
      {
        selected_inode = (selected_inode + 1) % current_song_elements.size();
      }
      else
      {
        selected_inode =
          (selected_inode - 1 + current_song_elements.size()) % current_song_elements.size();
      }
      if (album_name_indices.find(selected_inode) != album_name_indices.end())
      {
        if (move_down)
          albums_indices_traversed++;
        else
          albums_indices_traversed--;
      }
      // Break the loop if we traverse all elements (prevent infinite loop)
      if (selected_inode == initial_inode)
      {
        break;
      }
    } while (album_name_indices.find(selected_inode) !=
             album_name_indices.end()); // Skip album name indices
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
        selected_inode           = 1; // Reset to the first song
        albums_indices_traversed = 1;
      }
    }
    else
    {
      if (!current_inodes.empty())
      {
        NavigateSongMenu(move_down);
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
      if (!current_inodes.empty())
      {
        if (move_up)
        {
          // Navigate to the first valid song, skipping headers
          selected_inode = 1;
        }
        else
        {
          // Navigate to the last valid song
          selected_inode = current_song_elements.size() - 1;
        }

        if (selected_inode < 0 || selected_inode >= current_song_elements.size())
        {
          selected_inode = 1; // Fallback
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
        createRow(charToStr(global_keybinds.replay_song), "Replay current song",
                  TrueColors::Color::Orange),
        createRow(charToStr(global_keybinds.quit_app), "Quit", TrueColors::Color::LightRed),
        createRow(charToStr(global_keybinds.show_help), "Toggle this help",
                  TrueColors::Color::Cyan),
        createRow(charToStr(global_keybinds.add_song_to_queue), "Add song to queue",
                  TrueColors::Color::LightPink),
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
    return focused ? TrueColors::GetColor(global_colors.active_win_color)
                   : TrueColors::GetColor(TrueColors::Color::DarkGray);
  }

  Element RenderMainInterface(float progress)
  {
    std::string current_song_info;
    std::string year_info;
    std::string additional_info;

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

    std::string status = std::string("  ") + (is_playing ? STATUS_PLAYING : STATUS_PAUSED) + "  ";

    auto left_pane =
      vbox({
        text(" Artists") | bold | getTrueColor(TrueColors::Color::LightGreen) | inverted,
        separator(),
        artists_list->Render() | frame | flex | getTrueColor(TrueColors::Color::White),
      }) |
      borderHeavy | color(GetCurrWinColor(focus_on_artists));

    auto right_pane =
      vbox({
        text(" Songs") | bold | getTrueColor(TrueColors::Color::LightGreen) | inverted,
        separator(),
        songs_list->Render() | frame | flex | getTrueColor(TrueColors::Color::White),
      }) |
      borderHeavy | color(GetCurrWinColor(!focus_on_artists));

    auto panes = vbox({hbox({
                         left_pane | size(WIDTH, EQUAL, 100) | size(HEIGHT, EQUAL, 100) | flex,
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
    int         songs_left = song_queue.size() - current_song_queue_index - 1;
    if (songs_left > current_song_elements.size())
      songs_left = 0;
    queue_info += std::to_string(songs_left) + " songs left.";
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
