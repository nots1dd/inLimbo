#ifndef FTXUI_HANDLER_HPP
#define FTXUI_HANDLER_HPP

#include "../dbus/mpris-service.hpp"
#include "../music/audio_playback.hpp"
#include "../threads/thread_manager.hpp"
#include "./components/scroller.hpp"
#include "./properties.hpp"
#include "keymaps.hpp"
#include "misc.hpp"
#include <ftxui/dom/elements.hpp>
#include <unordered_set>

using namespace ftxui;

/** MACROS FOR SONG DETAILS */
#define STATUS_PLAYING   "<>"
#define STATUS_PAUSED    "!!"
#define LYRICS_AVAIL     "L*"
#define ADDN_PROPS_AVAIL "&*"
#define STATUS_BAR_DELIM " | "

/** STRING TRUNCATION MACROS */
#define MAX_LENGTH_SONG_NAME   50
#define MAX_LENGTH_ARTIST_NAME 30

/** SCREEN MACROS */
#define SHOW_MAIN_UI           0
#define SHOW_HELP_SCREEN       1
#define SHOW_LYRICS_SCREEN     2
#define SHOW_QUEUE_SCREEN      3
#define SHOW_SONG_INFO_SCREEN  4
#define SHOW_AUDIO_CONF_SCREEN 5

#define MIN_DEBOUNCE_TIME_IN_MS 500

/**
 * @class MusicPlayer
 * @brief A terminal-based music player.
 *
 * This class handles playback, playlist management, and user interactions.
 */

class MusicPlayer
{
public:
  MusicPlayer(
    const std::map<std::string,
                   std::map<std::string, std::map<unsigned int, std::map<unsigned int, Song>>>>&
      initial_library, Keybinds& keybinds, InLimboColors& colors)
      : library(initial_library), INL_Thread_Manager(std::make_unique<ThreadManager>()),
        INL_Thread_State(INL_Thread_Manager->getThreadState()),
        global_keybinds(keybinds),
        global_colors(colors),
        song_queue() // Initialize the queue vector to avoid any abrupt exits 
  {
    InitializeData();
    CreateComponents();
  }

  void Run()
  {
    mprisService = std::make_unique<MPRISService>("inLimbo");

    INL_Thread_State.mpris_dbus_thread = std::make_unique<std::thread>(
      [this]
      {
        GMainLoop* loop = g_main_loop_new(nullptr, FALSE);
        g_main_loop_run(loop);
        g_main_loop_unref(loop);
      });

    INL_Thread_State.mpris_dbus_thread
      ->detach(); // We will not be concerned with this thread entirely, hence it is detached

    auto              screen = ScreenInteractive::Fullscreen();
    std::atomic<bool> screen_active{true};

    std::thread refresh_thread(
      [&]()
      {
        while (screen_active)
        {
          try
          {
            using namespace std::chrono_literals;

            // Safely update shared resources
            UpdateVolume();

            std::this_thread::sleep_for(0.1s);

            if (INL_Thread_State.is_playing)
            {
              current_position += 0.1;
              if (current_position >= GetCurrentSongDuration())
              {
                PlayNextSong();
                UpdatePlayingState();
              }
            }

            SafePostEvent(screen, Event::Custom);
          }
          catch (const std::exception& e)
          {
            SetDialogMessage("Error in refresh thread: " + std::string(e.what()));
          }
          catch (...)
          {
            SetDialogMessage("Unknown error occurred in refresh thread.");
          }
        }
      });

    screen_ = &screen;

    try
    {
      screen.Loop(INL_Component_State.MainRenderer);
    }
    catch (const std::exception& e)
    {
      SetDialogMessage("Error in UI loop: " + std::string(e.what()));
    }
    catch (...)
    {
      SetDialogMessage("Unknown error occurred in UI loop.");
    }

    // Signal the thread to stop and wait for cleanup (Should be better than detaching the thread)
    screen_active = false;
    if (refresh_thread.joinable())
    {
      refresh_thread.join();
    }
  }

private:
  struct PlayingState
  {
    std::string                                  artist;
    std::string                                  title;
    std::string                                  genre;
    std::string                                  album;
    bool                                         has_comment     = false;
    bool                                         has_lyrics      = false;
    int                                          duration;
    int                                          bitrate;
    unsigned int                                 year       = 0;
    unsigned int                                 track      = 0;
    unsigned int                                 discNumber = 0;
    std::string                                  lyrics;
    std::string                                  comment;
    std::unordered_map<std::string, std::string> additionalProperties;
    std::string                                  filePath;
  };

  PlayingState current_playing_state;

  std::unique_ptr<MPRISService> mprisService;

  Keybinds      global_keybinds;
  GlobalProps   global_props    = parseProps();
  InLimboColors global_colors;

  std::shared_ptr<MiniAudioPlayer> audio_player;
  std::unique_ptr<ThreadManager>   INL_Thread_Manager; // Smart pointer to ThreadManager
  ThreadManager::ThreadState&      INL_Thread_State;

  std::vector<Song> song_queue;
  int               current_song_queue_index = 0;

  // Main data structure
  std::map<std::string, std::map<std::string, std::map<unsigned int, std::map<unsigned int, Song>>>>
    library;

  // Navigation state
  std::string  current_artist;
  unsigned int current_disc       = 1;
  unsigned int current_track      = 1;
  bool         show_dialog        = false;
  bool         show_audio_devices = false;
  std::string  dialog_message;
  double       seekBuffer;
  bool         first_g_pressed = false; // Track the state of the first 'g' press

  // Current view lists
  std::vector<std::string>  current_artist_names;
  std::vector<std::string>  song_queue_names;
  std::vector<std::string>  lyricLines;
  std::vector<std::string>  audioDevices;
  std::vector<AudioDevice>  audioDeviceMap;
  std::vector<Element>      current_song_elements;
  std::vector<unsigned int> current_inodes;
  std::unordered_set<int>   album_name_indices;
  int albums_indices_traversed = 1; // first element of current_song_elements is always an album

  std::mutex               state_mutex, event_mutex;
  std::queue<ftxui::Event> event_queue;

  // Player state
  int selected_artist         = 0;
  int selected_song_queue     = 0;
  int selected_audio_dev_line = 0;
  int selected_inode          = 1; // First element is always going to album name so we ignore it
  int current_lyric_line      = 0;
  std::vector<Element> lyricElements;
  int                  volume           = 50;
  bool                 muted            = false;
  int                  lastVolume       = volume;
  double               current_position = 0;
  int                  active_screen =
    0; // 0 -> Main UI ; 1 -> Show help ; 2 -> Show lyrics; 3 -> Songs queue screen; 4 -> Song info screen; 5 -> Audio sinks screen
  bool               should_quit      = false;
  bool               focus_on_artists = true;
  ScreenInteractive* screen_          = nullptr;

  // UI Components
  ComponentState INL_Component_State;

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

  void SetDialogMessage(const std::string& message)
  {
    std::lock_guard<std::mutex> lock(state_mutex);
    dialog_message = message;
    show_dialog    = true;
  }

  void SafePostEvent(ScreenInteractive& screen, const ftxui::Event& event)
  {
    {
      std::lock_guard<std::mutex> lock(event_mutex);
      event_queue.push(event);
    }
    screen.PostEvent(event);
  }

  void ProcessEvents(ScreenInteractive& screen)
  {
    std::lock_guard<std::mutex> lock(event_mutex);
    while (!event_queue.empty())
    {
      // Process the event if needed
      event_queue.pop();
    }
  }

  /* Miniaudio class integrations */

  std::shared_ptr<MiniAudioPlayer> getOrCreateAudioPlayer()
  {
    static std::mutex player_mutex;
    if (!audio_player)
    {
      audio_player = std::make_shared<MiniAudioPlayer>();
    }
    return audio_player;
  }

  void PlayCurrentSong()
  {
    INL_Thread_Manager->lockPlayMutex(INL_Thread_State);
    if (INL_Thread_State.is_processing)
    {
      return; // Another invocation is already running
    }
    INL_Thread_State.is_processing = true;

    // Enqueue the song playback task
    INL_Thread_Manager->getWorkerThreadPool().enqueue(
      [this]()
      {
        try
        {
          audio_player       = getOrCreateAudioPlayer();
          audioDeviceMap     = audio_player->enumerateDevices();
          Song* current_song = GetCurrentSongFromQueue();
          if (!current_song)
          {
            throw std::runtime_error("Error: No current song found.");
          }
          const std::string& file_path = current_song->metadata.filePath;
          if (file_path.empty())
          {
            throw std::runtime_error("Error: Invalid file path.");
          }
          // Stop previous song if any
          if (INL_Thread_State.is_playing)
          {
            audio_player->stop();
            /*INL_Thread_State.is_playing = false;*/
          }
          // Load the audio file
          auto load_future = audio_player->loadFileAsync(file_path, true);

          // Wait for the asynchronous load to complete
          int loadAudioFileStatus = load_future.get();
          if (loadAudioFileStatus == -1)
          {
            throw std::runtime_error("Error: Failed to load the audio file.");
          }
          INL_Thread_State.is_playing = true;
          audio_player->play(); // Play the song (it will play in a separate thread)
          auto durationFuture            = audio_player->getDurationAsync();
          current_playing_state.duration = durationFuture.get();
        }
        catch (const std::exception& e)
        {
          SetDialogMessage(e.what());
          audio_player->stop();
          INL_Thread_State.is_playing = false;
        }

        // Reset processing state
        INL_Thread_State.is_processing = false;
      });
  }

  void TogglePlayback()
  {
    if (!audio_player)
      return;

    if (INL_Thread_State.is_playing)
    {
      audio_player->pause();
      INL_Thread_State.is_playing = false;
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
        UpdatePlayingState();
      }
      INL_Thread_State.is_playing = true;
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
    try
    {
      // Queue state validations
      if (song_queue.empty())
      {
        SetDialogMessage("Error: Queue is empty.");
        INL_Thread_State.is_playing = false;
        return;
      }

      if (current_song_queue_index + 1 >= song_queue.size())
      {
        SetDialogMessage("Error: No more songs in the queue.");
        return;
      }

      // Increment song index
      current_song_queue_index++;

      // Get current song
      Song* current_song = GetCurrentSongFromQueue();
      if (!current_song)
      {
        INL_Thread_State.is_playing = false;
        SetDialogMessage("Error: Invalid song in queue.");
        return;
      }

      current_position = 0;
      PlayCurrentSong();
      UpdatePlayingState();
    }
    catch (std::exception e)
    {
      show_dialog    = true;
      dialog_message = "Error: Invalid!!.";
      return;
    }
  }

  void ReplaySong()
  {
    current_position = 0;
    PlayCurrentSong();
    UpdatePlayingState();
    return;
  }

  void PlayPreviousSong()
  {
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
      SetDialogMessage("Error: No previous song available.");
    }
  }

  void findAudioSinks()
  {
    audioDevices.clear();
    for (const auto& device : audioDeviceMap)
    {
      audioDevices.push_back(device.name);
    }
    return;
  }

  Element listAudioSinks()
  {
    INL_Component_State.audioDeviceMenu = CreateMenu(&audioDevices, &selected_audio_dev_line);
    findAudioSinks();
    if (audioDevices.empty())
      return vbox({text("No sinks available.") | bold | border});

    auto title = text(" Audio Devices ") | bold | getTrueColor(TrueColors::Color::LightBlue) |
                 underlined | center;

    auto audioDevComp = vbox({INL_Component_State.audioDeviceMenu->Render() | flex}) | border;

    return vbox({title, separator(), audioDevComp | frame | flex}) | flex;
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
              " " + std::to_string(disc_number) + "-" + std::to_string(track_number) + "  ";
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

  void PlayThisSongNext(const std::string& artist)
  {
    const Song& get_curr_song = GetCurrentSong(artist);

    try
    {
      song_queue.insert(song_queue.begin() + current_song_queue_index + 1, get_curr_song);
    }
    catch (std::exception e)
    {
      SetDialogMessage("Could not play this song next!");
    }

    current_song_queue_index++;
    return;
  }

  void EnqueueAllSongsByArtist(const std::string& artist, bool clearQueue)
  {
    // Clear the existing song list
    if (clearQueue)
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

    NavigateSongMenu(true);
  }

  void RemoveSongFromQueue()
  {
    if (selected_song_queue == 0)
    {
      SetDialogMessage("Unable to remove song... This is playing right now!");
      return;
    }
    if (selected_song_queue < song_queue.size())
    {
      song_queue.erase(song_queue.begin() + selected_song_queue);
    }
  }

  Song* GetCurrentSongFromQueue()
  {
    if (!song_queue.empty() && current_song_queue_index < song_queue.size())
    {
      return &song_queue[current_song_queue_index];
    }

    SetDialogMessage("Something went wrong");
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
    // Retrieve the current song from the queue
    Song* current_song = GetCurrentSongFromQueue();
    if (!current_song)
    {
      return; // No song available
    }

    const auto& metadata = current_song->metadata;

    // Offload metadata updates to a worker thread
    INL_Thread_Manager->getWorkerThreadPool().enqueue(
      [this, metadata]()
      {
        // Perform updates locally before committing to the shared state
        PlayingState new_state = current_playing_state;

        if (new_state.filePath == metadata.filePath)
        {
          return;
        }

        new_state.artist      = metadata.artist;
        new_state.title       = metadata.title;
        new_state.album       = metadata.album;
        new_state.genre       = metadata.genre;
        new_state.comment     = metadata.comment;
        new_state.year        = metadata.year;
        new_state.track       = metadata.track;
        new_state.discNumber  = metadata.discNumber;
        new_state.lyrics      = metadata.lyrics;
        new_state.has_comment = (metadata.comment != "No Comment");
        new_state.has_lyrics  = (metadata.lyrics != "No Lyrics");
        new_state.filePath    = metadata.filePath;
        new_state.bitrate     = metadata.bitrate;

        new_state.additionalProperties = metadata.additionalProperties;

        // Commit to shared state under mutex
        {
          std::lock_guard<std::mutex> lock(state_mutex);
          current_playing_state = std::move(new_state);
        }

        mprisService->updateMetadata(current_playing_state.title, current_playing_state.artist,
                                     current_playing_state.album,
                                     static_cast<int64_t>(current_playing_state.duration),
                                     current_playing_state.comment, current_playing_state.genre,
                                     current_playing_state.track, current_playing_state.discNumber);

        current_lyric_line = 0;
      });
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

    INL_Component_State.artists_list =
      Menu(&current_artist_names, &selected_artist, artist_menu_options);
    INL_Component_State.songs_list = Scroller(
      Renderer(
        [&]() mutable
        {
          return RenderSongMenu(current_song_elements); // This should return an Element
        }),
      &selected_inode, global_colors.menu_cursor_bg, global_colors.inactive_menu_cursor_bg);

    auto main_container =
      Container::Horizontal({INL_Component_State.artists_list, INL_Component_State.songs_list});

    /* Adding DEBOUNCE TIME (Invoking PlayCurrentSong() too fast causes resources to not be freed)
     * [TODO] REMOVED DEBOUNCE FOR NOW
     */

    auto last_event_time = std::chrono::steady_clock::now(); // Tracks the last event time
    int  debounce_time = std::stoi(std::string(parseTOMLField(PARENT_DBG, "debounce_time_in_ms")));
    if (debounce_time < MIN_DEBOUNCE_TIME_IN_MS)
      debounce_time = MIN_DEBOUNCE_TIME_IN_MS; // min debounce time is 0.5s
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
          return false; // Prevent other keys from working
        }

        else if (active_screen == SHOW_LYRICS_SCREEN)
        {
          if (is_keybind_match(global_keybinds.goto_main_screen))
          {
            active_screen = SHOW_MAIN_UI;
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
        }

        else if (active_screen == SHOW_QUEUE_SCREEN)
        {
          if (is_keybind_match(global_keybinds.remove_song_from_queue))
          {
            RemoveSongFromQueue();
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
          else if (is_keybind_match(global_keybinds.goto_main_screen))
          {
            active_screen = SHOW_MAIN_UI;
            return true;
          }
          else if (is_keybind_match('x'))
          { // Add key to dismiss dialog
            show_dialog = false;
            return true;
          }
        }

        else if (event.is_mouse())
          return false;

        else if (active_screen == SHOW_SONG_INFO_SCREEN)
        {
          if (is_keybind_match(global_keybinds.goto_main_screen))
          {
            active_screen = SHOW_MAIN_UI;
            return true;
          }
        }

        else if (active_screen == SHOW_AUDIO_CONF_SCREEN)
        {
          if (is_keybind_match(global_keybinds.goto_main_screen))
          {
            show_audio_devices = false;
            active_screen      = SHOW_MAIN_UI;
            return true;
          }
        }

        else if (active_screen == SHOW_MAIN_UI)
        {

          if (is_keybind_match(global_keybinds.play_song) && !focus_on_artists)
          {
            if (!current_artist.empty())
            {
              EnqueueAllSongsByArtist(current_artist, true);

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
              SetDialogMessage("No artist selected to play songs from.");
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
            /*auto now = std::chrono::steady_clock::now();*/
            /*if (now - last_event_time < debounce_duration)*/
            /*{*/
            /*  audio_player->stop();*/
            /*  return false;*/
            /*}*/
            PlayNextSong();
            /*last_event_time = now; // Update the last event time*/
            return true;
          }
          else if (is_keybind_match(global_keybinds.play_song_prev))
          {
            /*auto now = std::chrono::steady_clock::now();*/
            /*if (now - last_event_time < debounce_duration)*/
            /*  return false;*/
            /*last_event_time = now;*/
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
              ReplaySong();
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
          else if (is_keybind_match(global_keybinds.toggle_audio_devices))
          {
            show_audio_devices = !show_audio_devices;
            if (show_audio_devices)
            {
              active_screen = SHOW_AUDIO_CONF_SCREEN;
            }
            else
            {
              active_screen = SHOW_MAIN_UI;
            }
            return true;
          }
          else if (is_keybind_match(global_keybinds.view_lyrics) &&
                   (current_playing_state.has_lyrics || current_playing_state.has_comment))
          {
            active_screen = SHOW_LYRICS_SCREEN;
            return true;
          }
          else if (is_keybind_match(global_keybinds.view_song_queue) && !song_queue.empty())
          {
            active_screen = SHOW_QUEUE_SCREEN;
            return true;
          }
          else if (is_keybind_match(global_keybinds.goto_main_screen))
          {
            active_screen = SHOW_MAIN_UI;
            return true;
          }
          else if (is_keybind_match(global_keybinds.view_current_song_info))
          {
            active_screen = SHOW_SONG_INFO_SCREEN;
            return true;
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
              INL_Component_State.artists_list->TakeFocus();
            }
            else
            {
              INL_Component_State.songs_list->TakeFocus();
            }
            return true;
          }
          else if (is_keybind_match(global_keybinds.add_song_to_queue))
          {
            AddSongToQueue();
            return true;
          }
          else if (is_keybind_match(global_keybinds.add_artists_songs_to_queue) && focus_on_artists)
          {
            EnqueueAllSongsByArtist(current_artist_names[selected_artist], false);
            NavigateList(true);
            return true;
          }
          else if (is_keybind_match(global_keybinds.play_this_song_next) && !focus_on_artists)
          {
            PlayThisSongNext(current_artist_names[selected_artist]);
            return true;
          }
        }

        /*if (event == Event::ArrowDown && active_screen == SHOW_QUEUE_SCREEN)*/
        /*{*/
        /*  NavigateList(true);*/
        /*  return true;*/
        /*}*/
        /*if (event == Event::ArrowUp && active_screen == SHOW_QUEUE_SCREEN)*/
        /*{*/
        /*  NavigateList(false);*/
        /*  return true;*/
        /*}*/

        return false;
      });

    INL_Component_State.MainRenderer =
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
                 if (active_screen == SHOW_MAIN_UI)
                 {
                   interface = RenderMainInterface(progress);
                 }
                 if (active_screen == SHOW_LYRICS_SCREEN)
                 {
                   interface = RenderLyricsAndInfoView();
                 }
                 if (active_screen == SHOW_QUEUE_SCREEN)
                 {
                   interface = RenderQueueScreen();
                 }
                 if (active_screen == SHOW_AUDIO_CONF_SCREEN)
                 {
                   interface = listAudioSinks();
                 }
                 if (active_screen == SHOW_SONG_INFO_SCREEN)
                 {
                   interface = RenderThumbnail(
                     current_playing_state.filePath, getCachePath(), current_playing_state.title,
                     current_playing_state.artist, current_playing_state.album,
                     current_playing_state.genre, current_playing_state.year,
                     current_playing_state.track, current_playing_state.discNumber, progress);
                 }
                 if (show_dialog)
                 {
                   // Create a semi-transparent overlay with the dialog box
                   interface =
                     dbox({
                       interface,              // Dim the background
                       RenderDialog() | center // Center the dialog both horizontally and vertically
                     }) |
                     getTrueColor(TrueColors::Color::White) | flex;
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
    lyricLines.clear();
    lyricLines = formatLyrics(current_playing_state.lyrics);
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

    INL_Component_State.lyrics_scroller = CreateMenu(&lyricLines, &current_lyric_line);

    UpdateLyrics();

    std::string end_text = "Use arrow keys to scroll, Press '" +
                           std::string(1, static_cast<char>(global_keybinds.goto_main_screen)) +
                           "' to go back home.";

    auto lyrics_pane = vbox({
      INL_Component_State.lyrics_scroller->Render() | flex,
    });

    auto info_pane = window(text(" Additional Info ") | bold | center | inverted,
                            vbox(additionalPropertiesText) | frame | flex);

    return hbox({vbox({
                   lyrics_pane | frame | flex | border,
                   separator(),
                   text(end_text) | dim | center | border,
                 }) |
                   flex,
                 vbox({info_pane}) | flex}) |
           flex;
  }

  void UpdateSongQueueList()
  {
    song_queue_names.clear();

    for (long unsigned int i = current_song_queue_index; i < song_queue.size(); i++)
    {
      std::string eleName = song_queue[i].metadata.title + " by " + song_queue[i].metadata.artist;
      if (i == current_song_queue_index)
        eleName += "  *";
      song_queue_names.push_back(eleName);
    }

    return;
  }

  Element RenderQueueScreen()
  {
    INL_Component_State.songs_queue_comp = CreateMenu(&song_queue_names, &selected_song_queue);
    UpdateSongQueueList();

    auto title = text(" Song Queue ") | bold | getTrueColor(TrueColors::Color::LightBlue) |
                 underlined | center;

    auto separator_line = separator() | dim | flex;

    std::string end_text = "Use '" + charToStr(global_keybinds.remove_song_from_queue) +
                           "' to remove selected song from queue, Press '" +
                           charToStr(global_keybinds.goto_main_screen) + "' to go back home.";

    auto queue_container = vbox({
                             INL_Component_State.songs_queue_comp->Render() |
                               color(global_colors.song_queue_menu_fg) | flex,
                           }) |
                           border | color(global_colors.song_queue_menu_bor_col);

    return vbox({
             title,
             queue_container | frame | flex,
             separator(),
             text(end_text) | dim | center,
           }) |
           flex;
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
      if (selected_inode == 0 && move_down)
      {
        albums_indices_traversed = 1;
      }
      if (selected_inode == current_song_elements.size() - 1 && !move_down)
      {
        albums_indices_traversed = album_name_indices.size();
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
    if (active_screen == SHOW_MAIN_UI)
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
    else if (active_screen == SHOW_QUEUE_SCREEN)
    {
      if (move_down)
      {
        selected_song_queue = (selected_song_queue + 1) % song_queue_names.size();
      }
      else
      {
        selected_song_queue =
          (selected_song_queue - 1 + song_queue_names.size()) % song_queue_names.size();
      }
    }
    else if (active_screen == SHOW_LYRICS_SCREEN)
    {
      if (move_down)
      {
        current_lyric_line = (current_lyric_line + 1) % lyricLines.size();
      }
      else
      {
        current_lyric_line = (current_lyric_line - 1 + lyricLines.size()) % lyricLines.size();
      }
    }
  }

  void NavigateListToTop(bool move_up)
  {
    if (active_screen == SHOW_MAIN_UI)
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
          UpdateSongsForArtist(current_artist_names[selected_artist]);
          selected_inode           = 1; // Reset to the first song
          albums_indices_traversed = 1;
        }
      }
      else
      {
        if (!current_inodes.empty())
        {
          if (move_up)
          {
            // Navigate to the first valid song, skipping headers
            selected_inode           = 1;
            albums_indices_traversed = 1;
          }
          else
          {
            // Navigate to the last valid song
            selected_inode           = current_song_elements.size() - 1;
            albums_indices_traversed = album_name_indices.size();
          }

          if (selected_inode < 0 || selected_inode >= current_song_elements.size())
          {
            selected_inode           = 1; // Fallback
            albums_indices_traversed = 1;
          }
        }
      }
    }
    else if (active_screen == SHOW_QUEUE_SCREEN)
    {
      if (move_up)
      {
        selected_song_queue = 0;
      }
      else
      {
        selected_song_queue = song_queue_names.size() - 1;
      }
    }
    else if (active_screen == SHOW_LYRICS_SCREEN)
    {
      if (move_up)
      {
        current_lyric_line = 0;
      }
      else
      {
        current_lyric_line = lyricLines.size() - 1;
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
        createRow(charToStr(global_keybinds.add_song_to_queue), "Remove song from queue",
                  TrueColors::Color::Teal),
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
    return focused ? global_colors.active_win_border_color
                   : global_colors.inactive_win_border_color;
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
      if (global_props.show_bitrate)
      {
        additional_info += STATUS_BAR_DELIM;
        additional_info += std::to_string(current_playing_state.bitrate) + " kbps";
      }
      additional_info += STATUS_BAR_DELIM;
    }

    std::string status =
      std::string("  ") + (INL_Thread_State.is_playing ? STATUS_PLAYING : STATUS_PAUSED) + "  ";

    auto left_pane = vbox({
                       text(" Artists") | bold | color(global_colors.artists_title_bg) | inverted,
                       separator(),
                       INL_Component_State.artists_list->Render() | frame | flex |
                         getTrueColor(TrueColors::Color::White),
                     }) |
                     borderHeavy | color(GetCurrWinColor(focus_on_artists));

    auto right_pane = vbox({
                        text(" Songs") | bold | color(global_colors.songs_title_bg) | inverted,
                        separator(),
                        INL_Component_State.songs_list->Render() | frame | flex |
                          getTrueColor(TrueColors::Color::White),
                      }) |
                      borderHeavy | color(GetCurrWinColor(!focus_on_artists));

    auto panes = vbox({hbox({
                         left_pane | size(WIDTH, EQUAL, 100) | size(HEIGHT, EQUAL, 100) | flex,
                         right_pane | size(WIDTH, EQUAL, 100) | size(HEIGHT, EQUAL, 100) | flex,
                       }) |
                       flex}) |
                 flex;

    auto progress_style = INL_Thread_State.is_playing
                            ? color(global_colors.progress_bar_playing_col)
                            : color(global_colors.progress_bar_not_playing_col);
    auto progress_bar   = hbox({
      text(FormatTime((int)current_position)) | progress_style,
      gauge(progress) | flex | progress_style,
      text(FormatTime(GetCurrentSongDuration())) | progress_style,
    });

    auto volume_bar = hbox({
      text(" Vol: ") | dim,
      gauge(volume / 100.0) | size(WIDTH, EQUAL, 10) | color(global_colors.volume_bar_col),
      text(std::to_string(volume) + "%") | dim,
    });

    std::string queue_info = " ";
    int         songs_left = song_queue.size() - current_song_queue_index - 1;
    if (songs_left > song_queue.size())
      songs_left = 0;
    queue_info += std::to_string(songs_left) + " songs left.";
    std::string up_next_song = " Next up: ";
    if (song_queue.size() > 1 && songs_left > 0)
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
            text(current_playing_state.artist) | color(global_colors.status_bar_artist_col) | bold |
              size(WIDTH, LESS_THAN, MAX_LENGTH_ARTIST_NAME),
            text(current_song_info) | bold | color(global_colors.status_bar_song_col) |
              size(WIDTH, LESS_THAN, MAX_LENGTH_SONG_NAME),
            filler(), // Push the right-aligned content to the end
            hbox({text(additional_info) | getTrueColor(TrueColors::Color::Black) | flex,
                  text(year_info) | getTrueColor(TrueColors::Color::Black) |
                    size(WIDTH, LESS_THAN, 15),
                  text(" ")}) |
              align_right}) |
      size(HEIGHT, EQUAL, 1) | bgcolor(global_colors.status_bar_bg);

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
    should_quit = true;

    {
      INL_Thread_Manager->lockPlayMutex(INL_Thread_State);
      if (audio_player)
      {
        audio_player->stop();
      }
    }

    if (INL_Thread_State.play_future.valid())
    {
      auto status = INL_Thread_State.play_future.wait_for(std::chrono::milliseconds(50));
      if (status != std::future_status::ready)
      {
        // Handle timeout - future didn't complete in time
        SetDialogMessage("Warning: Audio shutdown timed out");
      }
    }

    if (screen_)
    {
      screen_->ExitLoopClosure()();
    }
  }
};

#endif // FTXUI_HANDLER_HPP
