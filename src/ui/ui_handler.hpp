#pragma once

#include "../dbus/mpris-service.hpp"
#include "../music/audio_playback.hpp"
#include "../threads/thread_manager.hpp"
#include "./components/scroller.hpp"
#include "./properties.hpp"
#include "dirsort/songmap.hpp"
#include "misc.hpp"
#include "states/state.hpp"

using namespace ftxui;

/** MACROS FOR SONG DETAILS */
#define STATUS_PLAYING "<>"
#define STATUS_PAUSED  "!!"

/** SCREEN MACROS */
#define SHOW_MAIN_UI           0
#define SHOW_HELP_SCREEN       1
#define SHOW_LYRICS_SCREEN     2
#define SHOW_QUEUE_SCREEN      3
#define SHOW_SONG_INFO_SCREEN  4
#define SHOW_AUDIO_CONF_SCREEN 5
#define SHOW_SEARCH_INPUT      6

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
              initial_library,
    Keybinds& keybinds, InLimboColors& colors)
      : library(initial_library), INL_Thread_Manager(std::make_unique<ThreadManager>()),
        INL_Thread_State(INL_Thread_Manager->getThreadState()), global_keybinds(keybinds),
        global_colors(colors), searchIndices() // Initialize these vectors to avoid any abrupt exits
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

            UpdateVolume();

            std::this_thread::sleep_for(0.1s);

            if (INL_Thread_State.is_playing)
            {
              current_position += 0.1;
              // [TODO]: FIX BUG=> If we have a lot of activity (say we spam a lot of up/down,
              // eventually after 10-15s, it will force itself to the next song)
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
  PlayingState current_playing_state;
  SearchState  current_search_state;
  ComponentState INL_Component_State;
  QueueState   qState;

  std::unique_ptr<MPRISService> mprisService;

  Keybinds      global_keybinds;
  GlobalProps   global_props = parseProps();
  InLimboColors global_colors;

  std::shared_ptr<MiniAudioPlayer> audio_player;
  std::unique_ptr<ThreadManager>   INL_Thread_Manager; // Smart pointer to ThreadManager
  ThreadManager::ThreadState&      INL_Thread_State;

  // Main data structure
  std::map<std::string, std::map<std::string, std::map<unsigned int, std::map<unsigned int, Song>>>>
    library;

  // Navigation state
  std::string current_artist;
  bool        show_dialog        = false;
  bool        is_search_active   = false;
  bool        show_audio_devices = false;
  std::string dialog_message;
  double      seekBuffer;
  bool        first_g_pressed = false; // Track the state of the first 'g' press

  // Current view lists
  std::vector<std::string>  current_artist_names;
  std::vector<std::string>  current_artist_song_names;
  std::vector<std::string>  lyricLines;
  std::vector<std::string>  audioDevices;
  std::vector<int>          searchIndices;
  std::vector<AudioDevice>  audioDeviceMap;
  std::vector<std::string>  audioDeviceConf;
  std::vector<Element>      current_song_elements;
  std::vector<unsigned int> current_inodes;
  std::unordered_set<int>   album_name_indices;
  int albums_indices_traversed = 1; // first element of current_song_elements is always an album

  std::mutex               state_mutex, event_mutex;
  std::queue<ftxui::Event> event_queue;

  // Player state
  int selected_artist         = 0;
  int selected_audio_dev_line = 0;
  int selected_inode          = 1; // First element is always going to album name so we ignore it
  int current_lyric_line      = 0;
  std::vector<Element> lyricElements;
  int                  volume           = 50;
  bool                 muted            = false;
  int                  lastVolume       = volume;
  double               current_position = 0;
  int active_screen = 0; // 0 -> Main UI ; 1 -> Show help ; 2 -> Show lyrics; 3 -> Songs queue
                         // screen; 4 -> Song info screen; 5 -> Audio sinks screen
  bool               should_quit      = false;
  bool               focus_on_artists = true;
  ScreenInteractive* screen_          = nullptr;

  void InitializeData()
  {
    for (const auto& artist_pair : library)
    {
      current_artist_names.push_back(artist_pair.first);
    }
    // Sort artist names alphabetically
    std::sort(current_artist_names.begin(), current_artist_names.end());

    if (!current_artist_names.empty())
    {
      UpdateSongsForArtist(current_artist_names[0]);
    }
    // Update the Search Query for Artists (it wont be updated again during the lifetime of the
    // player so doing it here is fine)
    for (size_t i = 0; i < current_artist_names.size(); ++i)
      current_search_state.ArtistSearchTrie.insert(current_artist_names[i], i);
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

  auto getOrCreateAudioPlayer() -> std::shared_ptr<MiniAudioPlayer>
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
          Song* current_song = qState.GetCurrentSongFromQueue();
          if (!current_song)
          {
            throw std::runtime_error("Error: No current song found.");
          }
          const std::string& file_path = current_song->metadata.filePath;
          if (file_path.empty())
          {
            throw std::runtime_error("Error: Invalid file path.");
          }

          if (INL_Thread_State.is_playing)
          {
            audio_player->stop();
          }

          auto load_future = audio_player->loadFileAsync(file_path, true);
          // Wait for the asynchronous load to complete
          int loadAudioFileStatus = load_future.get();
          if (loadAudioFileStatus == -1)
          {
            throw std::runtime_error("Error: Failed to load the audio file.");
          }
          INL_Thread_State.is_playing = true;
          UpdateVolume();
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
      if (qState.song_queue.empty())
      {
        SetDialogMessage("Error: Queue is empty.");
        INL_Thread_State.is_playing = false;
        return;
      }

      if (qState.qIndex + 1 >= qState.song_queue.size())
      {
        SetDialogMessage("Error: No more songs in the queue.");
        return;
      }

      qState.incrementQIndex();

      Song* current_song = qState.GetCurrentSongFromQueue();
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
    try
    {
      if (qState.song_queue.empty())
      {
        SetDialogMessage("Error: Queue is empty.");
        INL_Thread_State.is_playing = false;
        return;
      }

      if (qState.qIndex + 1 >= qState.song_queue.size())
      {
        SetDialogMessage("Error: No more previous songs in the queue.");
        return;
      }

      qState.decrementQIndex();

      Song* current_song = qState.GetCurrentSongFromQueue();
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

  void findAudioSinks()
  {
    audioDevices.clear();
    for (const auto& device : audioDeviceMap)
    {
      audioDevices.push_back(device.name);
    }
    return;
  }

  auto RenderAudioConsole() -> Element
  {
    INL_Component_State.audioDeviceMenu = CreateMenu(&audioDevices, &selected_audio_dev_line);
    findAudioSinks();
    if (audioDevices.empty())
      return vbox({text("No sinks available.") | bold | border});

    auto title = text(" Audio Console ") | bold | getTrueColor(TrueColors::Color::LightBlue) |
                 underlined | center;

    auto audioDevComp = vbox({INL_Component_State.audioDeviceMenu->Render() | flex}) | border;

    audioDeviceConf = audio_player->getAudioPlaybackDetails();

    vector<Element> audioConfDetails;
    for (const auto& i : audioDeviceConf)
      audioConfDetails.push_back(text(i) | frame);

    auto finalAudioConfDetailsElement = vbox(audioConfDetails);

    return vbox({title, separator(), audioDevComp | frame | flex, separator(),
                 finalAudioConfDetailsElement | flex | border}) |
           flex | borderRounded;
  }

  /* ------------------------------- */

  void UpdateSongsForArtist(const std::string& artist)
  {
    current_inodes.clear();
    current_song_elements.clear();
    album_name_indices.clear();
    current_search_state.SongSearchTrie.clear();
    current_search_state.songIndex = 0;

    if (library.count(artist) > 0)
    {
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

      for (const auto& [album_name, discs] : albums)
      {
        const Song& first_song = discs.begin()->second.begin()->second;
        current_song_elements.push_back(renderAlbumName(album_name, first_song.metadata.year,
                                                        global_colors.album_name_bg,
                                                        global_colors.album_name_fg));
        album_name_indices.insert(current_song_elements.size() - 1);

        for (const auto& [disc_number, tracks] : discs)
        {
          for (const auto& [track_number, song] : tracks)
          {
            std::string disc_track_info = formatDiscTrackInfo(disc_number, track_number);
            current_inodes.push_back(song.inode);
            current_search_state.SongSearchTrie.insert(song.metadata.title,
                                                       current_search_state.songIndex++);
            current_song_elements.push_back(
              renderSongName(disc_track_info, song.metadata.title, song.metadata.duration));
          }
        }
      }
    }

    current_artist           = artist;
    selected_inode           = 1;
    albums_indices_traversed = 1;
  }

  auto GetCurrentSong(const std::string& artist) -> const Song&
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
            return song;
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
      qState.insertSongToIndex(get_curr_song);
    }
    catch (std::exception e)
    {
      SetDialogMessage("Could not play this song next!");
    }

    qState.incrementQIndex();
    return;
  }

  void EnqueueAllSongsByArtist(const std::string& artist, bool clearQueue)
  {
    if (clearQueue)
      qState.clearQueue();

    bool start_enqueue = false;

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
          if (start_enqueue)
            qState.qPush(song);
        }
      }
    }
  }

  void AddSongToQueue()
  {
    if (selected_artist >= current_artist_names.size() ||
        selected_inode - albums_indices_traversed >= current_inodes.size())
    {
      throw std::runtime_error("Invalid artist or song selection.");
    }

    // Get a const reference to the current song
    const Song& current_preview_song = GetCurrentSong(current_artist_names[selected_artist]);

    qState.qPush(current_preview_song);

    NavigateSongMenu(true);
  }

  void RemoveSongFromQueue()
  {
    if (qState.qScreenIndex == 0)
    {
      SetDialogMessage("Unable to remove song... This is playing right now!");
      return;
    }
    if (qState.qScreenIndex < qState.getQueueSize())
    {
      qState.qPopIndex();
    }
  }

  auto GetCurrentSongDuration() -> int
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
    Song* current_song = qState.GetCurrentSongFromQueue();
    if (!current_song)
    {
      SetDialogMessage("Current Song not available!");
      return;
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

        new_state.copyMetadata(metadata);

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
    INL_Component_State.artists_list = Scroller(
      Renderer([&]() mutable { return RenderArtistMenu(current_artist_names); }), &selected_artist,
      global_colors.artists_menu_col_bg, global_colors.inactive_menu_cursor_bg);
    INL_Component_State.songs_list = Scroller(
      Renderer(
        [&]() mutable
        {
          return RenderSongMenu(current_song_elements);
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

        if (is_search_active)
        {
          if (event.is_character())
          {
            current_search_state.input += event.character();

            current_search_state.artistIndex = 0;
            current_search_state.songIndex   = 0;

            if (focus_on_artists)
            {
              searchIndices =
                current_search_state.ArtistSearchTrie.search(current_search_state.input);
              if (!searchIndices.empty())
              {
                selected_artist = searchIndices[current_search_state.artistIndex];
                UpdateSongsForArtist(current_artist_names[selected_artist]);
              }
            }
            else
            {
              searchIndices =
                current_search_state.SongSearchTrie.search(current_search_state.input);
              if (!searchIndices.empty())
              {
                selected_inode = searchIndices[current_search_state.songIndex];
              }
            }
            return true;
          }
          else if (event == Event::Backspace)
          {
            if (!current_search_state.input.empty())
              current_search_state.input.pop_back();
            return true;
          }
          else if (event == Event::Return || event == Event::Escape)
          {
            is_search_active = false;
            current_search_state.input.clear();
            return true;
          }
        }

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
        }

        else if (active_screen == SHOW_QUEUE_SCREEN)
        {
          if (is_keybind_match(global_keybinds.remove_song_from_queue))
          {
            RemoveSongFromQueue();
            return true;
          }
          else if (is_keybind_match(global_keybinds.goto_main_screen))
          {
            active_screen = SHOW_MAIN_UI;
            return true;
          }
        }

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

              if (Song* current_song = qState.GetCurrentSongFromQueue())
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
          else if (is_keybind_match(global_keybinds.seek_ahead_5) && INL_Thread_State.is_playing)
          {
            seekBuffer = audio_player->seekTime(5);
            current_position += seekBuffer;
            return true;
          }
          else if (is_keybind_match(global_keybinds.seek_behind_5) && INL_Thread_State.is_playing)
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
            return true;
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
            volume = handleToggleMute(&volume, &lastVolume, &muted);
            UpdateVolume();
            return true;
          }
          else if (is_keybind_match(global_keybinds.show_help))
          {
            active_screen = SHOW_HELP_SCREEN;
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
                   (current_playing_state.HasLyrics() || current_playing_state.HasComments()))
          {
            active_screen = SHOW_LYRICS_SCREEN;
            return true;
          }
          else if (is_keybind_match(global_keybinds.view_song_queue) && !qState.song_queue.empty())
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
            if (!INL_Thread_State.is_playing)
            {
              current_position = 0;
              PlayCurrentSong();
              UpdatePlayingState();
            }
            NavigateList(true);
            return true;
          }
          else if (is_keybind_match(global_keybinds.play_this_song_next) && !focus_on_artists)
          {
            PlayThisSongNext(current_artist_names[selected_artist]);
            return true;
          }
          else if (is_keybind_match(global_keybinds.search_menu))
          {
            is_search_active = true;
            searchIndices.clear();
            current_search_state.input.clear();
            return true;
          }
          else if (is_keybind_match(global_keybinds.search_item_next))
          {
            if (focus_on_artists)
            {
              // Increment index and wrap around if necessary
              UpdateSelectedIndex(current_search_state.artistIndex, searchIndices.size(), true);
              selected_artist = searchIndices[current_search_state.artistIndex];
              UpdateSongsForArtist(current_artist_names[selected_artist]);
              return true;
            }
            else
            {
              // Inc index and wrap around if necessary
              UpdateSelectedIndex(current_search_state.songIndex, searchIndices.size(), true);
              selected_inode = searchIndices[current_search_state.songIndex];
              return true;
            }
          }
          else if (is_keybind_match(global_keybinds.search_item_prev))
          {
            if (focus_on_artists)
            {
              // Decrement index and wrap around if necessary
              UpdateSelectedIndex(current_search_state.artistIndex, searchIndices.size(), false);
              selected_artist = searchIndices[current_search_state.artistIndex];
              UpdateSongsForArtist(current_artist_names[selected_artist]);
              return true;
            }
            else
            {
              // Decrement index and wrap around if necessary
              UpdateSelectedIndex(current_search_state.songIndex, searchIndices.size(), false);
              selected_inode = searchIndices[current_search_state.songIndex];
              return true;
            }
          }
        }

        if (is_keybind_match(global_keybinds.scroll_down) || event == Event::ArrowDown)
        {
          NavigateList(true);
          return true;
        }
        else if (is_keybind_match(global_keybinds.scroll_up) || event == Event::ArrowUp)
        {
          NavigateList(false);
          return true;
        }
        /* @Some default keybinds */
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
            first_g_pressed = false;
            return true;
          }
        }
        else if (is_keybind_match('G'))
        {
          NavigateListToTop(false);
          return true;
        }
        else if (is_keybind_match('x'))
        {
          show_dialog = false;
          return true;
        }

        return false;
      });

    INL_Component_State.MainRenderer =
      Renderer(main_container,
               [&]
               {
                 int   duration = GetCurrentSongDuration();
                 float progress = duration > 0 ? (float)current_position / duration : 0;

                 Element interface;
                 switch (active_screen)
                 {
                   case SHOW_HELP_SCREEN:
                     interface = RenderHelpScreen(global_keybinds);
                     break;
                   case SHOW_MAIN_UI:
                     interface = RenderMainInterface(progress);
                     break;
                   case SHOW_LYRICS_SCREEN:
                     interface = RenderLyricsAndInfoView();
                     break;
                   case SHOW_QUEUE_SCREEN:
                     interface = RenderQueueScreen();
                     break;
                   case SHOW_AUDIO_CONF_SCREEN:
                     interface = RenderAudioConsole();
                     break;
                   case SHOW_SONG_INFO_SCREEN:
                     interface = RenderThumbnail(
                       current_playing_state.filePath, getCachePath(), current_playing_state.title,
                       current_playing_state.artist, current_playing_state.album,
                       current_playing_state.genre, current_playing_state.year,
                       current_playing_state.track, current_playing_state.discNumber, progress);
                     break;
                 }
                 if (show_dialog)
                 {
                   interface = dbox({interface, RenderDialog(dialog_message) | center}) |
                               getTrueColor(TrueColors::Color::White) | flex;
                 }
                 return window(text(" inLimbo ") | bold, vbox(interface));
               });
  }

  void UpdateLyrics()
  {
    lyricLines.clear();
    lyricLines = current_playing_state.formatLyrics();
    return;
  }

  auto RenderLyricsAndInfoView() -> Element
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
                           charToStr(global_keybinds.goto_main_screen) + "' to go back home.";

    auto lyrics_pane = vbox({
      INL_Component_State.lyrics_scroller->Render() | flex,
    });

    auto info_pane = window(text(" Additional Info ") | bold | center,
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

  auto RenderQueueScreen() -> Element
  {
    INL_Component_State.songs_queue_comp =
      CreateMenu(&qState.song_queue_names, &qState.qScreenIndex);
    qState.UpdateSongQueueList();

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
             filler(),
             text(end_text) | dim | center,
           }) |
           flex;
  }

  void NavigateSongMenu(bool move_down)
  {
    if (current_song_elements.empty())
      return;

    int initial_inode = selected_inode;
    do
    {
      UpdateSelectedIndex(selected_inode, current_song_elements.size(), move_down);

      if (album_name_indices.count(selected_inode))
      {
        move_down ? ++albums_indices_traversed : --albums_indices_traversed;
      }

      if (selected_inode == 0 && move_down)
      {
        albums_indices_traversed = 1;
      }
      else if (selected_inode == current_song_elements.size() - 1 && !move_down)
      {
        albums_indices_traversed = album_name_indices.size();
      }

      if (selected_inode == initial_inode)
        break;
    } while (album_name_indices.count(selected_inode)); // Skip album headers
  }

  void NavigateList(bool move_down)
  {
    switch (active_screen)
    {
      case SHOW_MAIN_UI:
        if (focus_on_artists && !current_artist_names.empty())
        {
          UpdateSelectedIndex(selected_artist, current_artist_names.size(), move_down);
          UpdateSongsForArtist(current_artist_names[selected_artist]);
        }
        else if (!current_inodes.empty())
        {
          NavigateSongMenu(move_down);
        }
        break;

      case SHOW_QUEUE_SCREEN:
        UpdateSelectedIndex(qState.qScreenIndex, qState.song_queue_names.size(), move_down);
        break;

      case SHOW_LYRICS_SCREEN:
        UpdateSelectedIndex(current_lyric_line, lyricLines.size(), move_down);
        break;
      case SHOW_AUDIO_CONF_SCREEN:
        UpdateSelectedIndex(selected_audio_dev_line, audioDevices.size(), move_down);
        break;
    }
  }

  void NavigateListToTop(bool move_up)
  {
    switch (active_screen)
    {
      case SHOW_MAIN_UI:
        if (focus_on_artists && !current_artist_names.empty())
        {
          selected_artist = move_up ? 0 : current_artist_names.size() - 1;
          UpdateSongsForArtist(current_artist_names[selected_artist]);
          selected_inode           = 1;
          albums_indices_traversed = 1;
        }
        else if (!current_inodes.empty())
        {
          selected_inode           = move_up ? 1 : current_song_elements.size() - 1;
          albums_indices_traversed = move_up ? 1 : album_name_indices.size();
        }
        break;

      case SHOW_QUEUE_SCREEN:
        qState.qScreenIndex = move_up ? 0 : qState.song_queue_names.size() - 1;
        break;

      case SHOW_LYRICS_SCREEN:
        current_lyric_line = move_up ? 0 : lyricLines.size() - 1;
        break;
    }
  }

  auto GetCurrWinColor(bool focused) -> Color
  {
    return focused ? global_colors.active_win_border_color
                   : global_colors.inactive_win_border_color;
  }

  auto RenderProgressBar(float progress) -> Element
  {
    auto progress_style = INL_Thread_State.is_playing
                            ? color(global_colors.progress_bar_playing_col)
                            : color(global_colors.progress_bar_not_playing_col);

    return hbox({
      text(FormatTime((int)current_position)) | progress_style,
      gauge(progress) | flex | progress_style,
      text(FormatTime(GetCurrentSongDuration())) | progress_style,
    });
  }

  auto RenderQueueBar() -> Element
  {
    std::string queue_info = " ";
    int         songs_left = qState.getQueueSize() - qState.qIndex - 1;
    if (songs_left >= qState.getQueueSize() && songs_left < 0)
      songs_left = 0;
    queue_info += std::to_string(songs_left) + " songs left.";

    std::string up_next_song = " Next up: ";
    if (qState.getQueueSize() > 1 && songs_left > 0)
      up_next_song += qState.song_queue[qState.qIndex + 1].metadata.title + " by " +
                      qState.song_queue[qState.qIndex + 1].metadata.artist;
    else
      up_next_song += "Next song not available.";

    return hbox({
      text(queue_info) | dim | border | bold,
      text(up_next_song) | dim | border | flex | size(WIDTH, LESS_THAN, MAX_LENGTH_SONG_NAME),
    });
  }

  auto RenderMainInterface(float progress) -> Element
  {
    std::string current_song_info;
    std::string year_info;
    std::string additional_info;

    if (!current_playing_state.artist.empty())
    {
      current_song_info = SONG_TITLE_DELIM + current_playing_state.title;
      year_info         = std::to_string(current_playing_state.year) + " ";
      additional_info   = formatAdditionalInfo(
        current_playing_state.genre, current_playing_state.has_comment,
        current_playing_state.has_lyrics, global_props.show_bitrate, current_playing_state.bitrate);
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

    auto progress_bar = RenderProgressBar(progress);
    auto volume_bar   = RenderVolumeBar(volume, global_colors.volume_bar_col);
    auto queue_bar    = RenderQueueBar();
    auto status_bar   = RenderStatusBar(status, current_song_info, additional_info, year_info,
                                        global_colors, current_playing_state.artist);
    auto search_bar =
      is_search_active == true ? RenderSearchBar(current_search_state.input) : filler();

    return vbox({panes,
                 hbox({
                   progress_bar | border | flex_grow,
                   volume_bar | border,
                   queue_bar,
                 }),
                 status_bar, search_bar}) |
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
