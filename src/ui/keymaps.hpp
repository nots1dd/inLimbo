#ifndef KEYMAPS_HPP
#define KEYMAPS_HPP

#include "../parser/toml_parser.hpp"
#include <string_view>
#include <unordered_map>

using namespace std;

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
  char quit_app;
  char seek_ahead_5;
  char seek_behind_5;

  // Map for special keybinds (like Tab, Enter, etc.) {Thought might cook but failed}
  unordered_map<string, char> special_keys;
};

Keybinds parseKeybinds()
{
  Keybinds keybinds;

  // Helper lambda to convert special keybinds (like Tab, Space, Enter) to their ASCII values
  auto handle_special_keys = [](const std::string_view& key) -> char
  {
    if (key.empty())
      return -1; // Handle empty keys
    if (key == SPECIAL_KEYBIND_TAB_STR)
      return '\t';
    if (key == SPECIAL_KEYBIND_SPACE_STR)
      return ' ';
    if (key == SPECIAL_KEYBIND_ENTER_STR)
      return '\n';
    if (key == "Esc")
      return 27; // Escape key
    if (key.size() == 1)
      return key[0]; // For single-character keys
    return -1;       // Return -1 for invalid keys
  };

  keybinds.scroll_up      = handle_special_keys(parseTOMLField("keybinds", "scroll_up"));
  keybinds.scroll_down    = handle_special_keys(parseTOMLField("keybinds", "scroll_down"));
  keybinds.toggle_focus   = handle_special_keys(parseTOMLField("keybinds", "toggle_focus"));
  keybinds.show_help      = handle_special_keys(parseTOMLField("keybinds", "show_help"));
  keybinds.toggle_play    = handle_special_keys(parseTOMLField("keybinds", "toggle_play"));
  keybinds.play_song      = handle_special_keys(parseTOMLField("keybinds", "play_song"));
  keybinds.play_song_next = handle_special_keys(parseTOMLField("keybinds", "play_song_next"));
  keybinds.play_song_prev = handle_special_keys(parseTOMLField("keybinds", "play_song_prev"));
  keybinds.vol_up         = handle_special_keys(parseTOMLField("keybinds", "vol_up"));
  keybinds.vol_down       = handle_special_keys(parseTOMLField("keybinds", "vol_down"));
  keybinds.quit_app       = handle_special_keys(parseTOMLField("keybinds", "quit_app"));
  keybinds.seek_ahead_5   = handle_special_keys(parseTOMLField("keybinds", "seek_ahead_5"));
  keybinds.seek_behind_5   = handle_special_keys(parseTOMLField("keybinds", "seek_behind_5"));

  return keybinds;
}

#endif
