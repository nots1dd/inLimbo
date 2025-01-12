#ifndef KEYMAPS_HPP
#define KEYMAPS_HPP

#include "../parser/toml_parser.hpp"
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <unordered_map>

/**
 * @brief Struct to hold keybinding mappings.
 * 
 * This struct contains the mappings of keybinds used throughout the application.
 */
struct Keybinds
{
  char scroll_up; /**< Key for scrolling up */
  char scroll_down; /**< Key for scrolling down */
  char toggle_focus; /**< Key for toggling focus */
  char show_help; /**< Key for showing help */
  char toggle_play; /**< Key for toggling play/pause */
  char play_song; /**< Key for playing a song */
  char play_song_next; /**< Key for playing the next song */
  char play_song_prev; /**< Key for playing the previous song */
  char vol_up; /**< Key for increasing volume */
  char vol_down; /**< Key for decreasing volume */
  char toggle_mute; /**< Key for toggling mute */
  char quit_app; /**< Key for quitting the application */
  char seek_ahead_5; /**< Key for seeking ahead 5 seconds */
  char seek_behind_5; /**< Key for seeking behind 5 seconds */
  char view_lyrics; /**< Key for viewing lyrics */
  char goto_main_screen; /**< Key for going to the main screen */
  char replay_song; /**< Key for replaying the current song */
  char add_song_to_queue; /**< Key for adding a song to the queue */
  char add_artists_songs_to_queue; /**< Key for adding an artist's songs to the queue */
  char remove_song_from_queue; /**< Key for removing a song from the queue */
  char play_this_song_next; /**< Key for playing a song next */
  char view_song_queue; /**< Key for viewing the song queue */
};

/**
 * @brief Parses the keybinds from the TOML configuration.
 * 
 * This function reads the keybind configurations from the TOML file, maps them to their respective key values,
 * and returns a populated `Keybinds` struct.
 * 
 * @return A `Keybinds` struct with the key mappings for various actions.
 */
Keybinds parseKeybinds()
{
  Keybinds keybinds;

  // Mapping of special key names to their ASCII values
  const std::unordered_map<std::string_view, char> special_keys = {
    {"Tab", '\t'}, /**< Special key for Tab */
    {"Space", ' '}, /**< Special key for Space */
    {"Enter", '\n'}, /**< Special key for Enter */
    {"Esc", 27}, /**< Special key for Escape */
  };

  /**
   * @brief Reports an error for unsupported or empty keybind.
   * 
   * This lambda function prints an error message and terminates the program if an unsupported or empty keybind 
   * is detected in the configuration.
   * 
   * @param field The field in which the invalid keybind was found.
   * @param key The keybind that was invalid or empty.
   */
  auto reportError = [](const std::string& field, const std::string_view& key)
  {
    std::cerr << "Error: Unsupported or empty keybind '" << key << "' detected for field '" << field
              << "'. Please check your configuration." << std::endl;
    std::exit(EXIT_FAILURE); /**< Exit the program on invalid keybind */
  };

  /**
   * @brief Handles the mapping of special keys and single-character keys.
   * 
   * This lambda function checks whether a keybind is a special key (e.g., Tab, Enter) or a single character,
   * and returns the corresponding ASCII value. It reports an error if the key is invalid or unsupported.
   * 
   * @param key The keybind to be processed.
   * @param field The field name that the keybind belongs to.
   * @return The ASCII value of the key.
   */
  auto handle_special_keys = [&](const std::string_view& key, const std::string& field) -> char
  {
    if (key.empty())
    {
      reportError(field, key); /**< Report error if key is empty */
    }
    if (special_keys.find(key) != special_keys.end())
    {
      return special_keys.at(key); /**< Return the mapped special key */
    }
    if (key.size() == 1)
    {
      return key[0]; // Single-character keys
    }
    reportError(field, key); /**< Report error if the key is not supported */
    return -1; // Unreachable case
  };

  // Map each field name to the corresponding member in the Keybinds struct
  const std::unordered_map<std::string, char*> field_map = {
    {"scroll_up", &keybinds.scroll_up},
    {"scroll_down", &keybinds.scroll_down},
    {"toggle_focus", &keybinds.toggle_focus},
    {"show_help", &keybinds.show_help},
    {"toggle_play", &keybinds.toggle_play},
    {"play_song", &keybinds.play_song},
    {"play_song_next", &keybinds.play_song_next},
    {"play_song_prev", &keybinds.play_song_prev},
    {"vol_up", &keybinds.vol_up},
    {"vol_down", &keybinds.vol_down},
    {"toggle_mute", &keybinds.toggle_mute},
    {"quit_app", &keybinds.quit_app},
    {"seek_ahead_5", &keybinds.seek_ahead_5},
    {"seek_behind_5", &keybinds.seek_behind_5},
    {"view_lyrics", &keybinds.view_lyrics},
    {"replay_song", &keybinds.replay_song},
    {"goto_main_screen", &keybinds.goto_main_screen},
    {"add_song_to_queue", &keybinds.add_song_to_queue},
    {"add_artists_songs_to_queue", &keybinds.add_artists_songs_to_queue},
    {"remove_song_from_queue", &keybinds.remove_song_from_queue},
    {"play_this_song_next", &keybinds.play_this_song_next},
    {"view_song_queue", &keybinds.view_song_queue}
  };

  // Populate the keybinds struct by reading the fields from the TOML configuration
  for (const auto& [field, member_ptr] : field_map)
  {
    *member_ptr = handle_special_keys(parseTOMLField(PARENT_KEYBINDS, field), field); /**< Handle each keybind */
  }

  return keybinds; /**< Return the populated Keybinds struct */
}

#endif
