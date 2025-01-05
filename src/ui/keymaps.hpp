#ifndef KEYMAPS_HPP
#define KEYMAPS_HPP

#include "../parser/toml_parser.hpp"
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <unordered_map>

struct Keybinds
{
  char scroll_up;
  char scroll_down;
  char toggle_focus;
  char show_help;
  char toggle_play;
  char play_song;
  char play_song_next;
  char play_song_prev;
  char vol_up;
  char vol_down;
  char toggle_mute;
  char quit_app;
  char seek_ahead_5;
  char seek_behind_5;
  char view_lyrics;
  char goto_main_screen;
  char replay_song;
  char add_song_to_queue;
  char play_this_song_next;
  char view_song_queue;
};

Keybinds parseKeybinds()
{
  Keybinds keybinds;

  // Mapping of special key names to their ASCII values
  const std::unordered_map<std::string_view, char> special_keys = {
    {"Tab", '\t'},
    {"Space", ' '},
    {"Enter", '\n'},
    {"Esc", 27},
  };

  auto reportError = [](const std::string& field, const std::string_view& key)
  {
    std::cerr << "Error: Unsupported or empty keybind '" << key << "' detected for field '" << field
              << "'. Please check your configuration." << std::endl;
    std::exit(EXIT_FAILURE);
  };

  // Helper to handle key mapping
  auto handle_special_keys = [&](const std::string_view& key, const std::string& field) -> char
  {
    if (key.empty())
    {
      reportError(field, key);
    }
    if (special_keys.find(key) != special_keys.end())
    {
      return special_keys.at(key);
    }
    if (key.size() == 1)
    {
      return key[0]; // Single-character keys
    }
    reportError(field, key);
    return -1; // Unreachable case
  };

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
    {"play_this_song_next", &keybinds.play_this_song_next},
    {"view_song_queue", &keybinds.view_song_queue}};

  for (const auto& [field, member_ptr] : field_map)
  {
    *member_ptr = handle_special_keys(parseTOMLField("keybinds", field), field);
  }

  return keybinds;
}

#endif
