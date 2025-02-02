#pragma once

#include "../../dirsort/songmap.hpp"
#include "../../helpers/trie.hpp"
#include <ftxui/component/component.hpp>
#include <mutex>
#include <string>
#include <unordered_map>

using namespace ftxui;

using namespace std;

/**
 * @struct ComponentState
 * @brief Holds the components used for rendering the UI.
 *
 * This structure stores the various components that make up the user interface, including
 * components for artists list, songs list, songs queue, lyrics scroller, and more.
 */
struct ComponentState
{
  Component artists_list;      /**< Component for the list of artists */
  Component songs_list;        /**< Component for the list of songs */
  Component songs_queue_comp;  /**< Component for the song queue */
  Component lyrics_scroller;   /**< Component for displaying lyrics */
  Component MainRenderer;      /**< Component for the main renderer */
  Component ThumbnailRenderer; /**< Component for the thumbnail renderer */
  Component audioDeviceMenu;   /**< Component for the audio device menu */
};

/**
 * @struct PlayingState
 * @brief Represents the current state of the song being played.
 *
 * This structure holds the metadata for the song currently playing, including the artist,
 * title, album, genre, lyrics, comments, and more.
 */
struct PlayingState
{
  std::string  artist;      /**< The artist of the currently playing song */
  std::string  title;       /**< The title of the currently playing song */
  std::string  genre;       /**< The genre of the currently playing song */
  std::string  album;       /**< The album of the currently playing song */
  bool         has_comment; /**< Whether the song has a comment */
  bool         has_lyrics;  /**< Whether the song has lyrics */
  int          duration;    /**< The duration of the song in seconds */
  int          bitrate;     /**< The bitrate of the song */
  unsigned int year;        /**< The year of release for the song */
  unsigned int track;       /**< The track number of the song */
  unsigned int discNumber;  /**< The disc number the song is on */
  std::string  lyrics;      /**< The lyrics of the song */
  std::string  comment;     /**< The comment for the song */
  std::unordered_map<std::string, std::string>
              additionalProperties; /**< Any additional properties of the song */
  std::string filePath;             /**< The file path of the song */

  /**
   * @brief Copies the metadata from a given Metadata object.
   *
   * This method copies the metadata fields from the provided Metadata structure into the
   * PlayingState structure.
   *
   * @param metadata The Metadata object to copy from.
   */
  void copyMetadata(const Metadata& metadata)
  {
    artist               = metadata.artist;
    title                = metadata.title;
    album                = metadata.album;
    genre                = metadata.genre;
    comment              = metadata.comment;
    year                 = metadata.year;
    track                = metadata.track;
    discNumber           = metadata.discNumber;
    lyrics               = metadata.lyrics;
    has_comment          = (metadata.comment != "No Comment");
    has_lyrics           = (metadata.lyrics != "No Lyrics");
    filePath             = metadata.filePath;
    bitrate              = metadata.bitrate;
    additionalProperties = metadata.additionalProperties;

    return;
  }
  /**
   * @brief Checks if the song has lyrics.
   *
   * This method returns whether the song has lyrics available.
   *
   * @return True if the song has lyrics, false otherwise.
   */
  auto HasLyrics() -> bool { return has_lyrics; }
  /**
   * @brief Checks if the song has comments.
   *
   * This method returns whether the song has any comments.
   *
   * @return True if the song has comments, false otherwise.
   */
  auto HasComments() -> bool { return has_comment; }
  /**
   * @brief Formats the lyrics into a vector of strings.
   *
   * This method processes the lyrics, breaking them into lines and handling special characters
   * such as square and curly brackets. The result is a vector of strings representing the
   * formatted lyrics.
   *
   * @return A vector of formatted lyrics lines.
   */
  auto formatLyrics() -> vector<string>
  {
    vector<string> lines;
    std::string    currentLine;
    bool           insideSquareBrackets = false;
    bool           insideCurlBrackets   = false;
    bool           lastWasUppercase     = false;
    bool           lastWasSpecialChar   = false; // Tracks special characters within words
    char           previousChar         = '\0';

    for (char c : lyrics)
    {
      if (c == '[' || c == '(')
      {
        if (!currentLine.empty())
        {
          lines.push_back(currentLine);
          currentLine.clear();
        }
        if (c == '[')
          insideSquareBrackets = true;
        else
          insideCurlBrackets = true;
        currentLine += c;
        continue;
      }

      if (insideSquareBrackets || insideCurlBrackets)
      {
        currentLine += c;
        if (c == ']' && insideSquareBrackets)
        {
          lines.push_back(currentLine);
          currentLine.clear();
          insideSquareBrackets = false;
        }

        else if (c == ')' && insideCurlBrackets)
        {
          lines.push_back(currentLine);
          currentLine.clear();
          insideCurlBrackets = false;
        }
        continue;
      }

      if (c == '\'' || c == '-')
      {
        currentLine += c;
        lastWasSpecialChar = true;
        continue;
      }

      if (std::isupper(c) && !lastWasUppercase && !lastWasSpecialChar && !currentLine.empty() &&
          previousChar != '\n' && previousChar != ' ')
      {
        lines.push_back(currentLine);
        currentLine.clear();
      }

      currentLine += c;

      if (c == '\n')
      {
        if (!currentLine.empty())
        {
          lines.push_back(currentLine);
          currentLine.clear();
        }
      }

      lastWasUppercase   = std::isupper(c);
      lastWasSpecialChar = false;
      previousChar       = c;
    }

    if (!currentLine.empty())
    {
      lines.push_back(currentLine);
    }

    // Trim empty lines (optional)
    lines.erase(std::remove_if(lines.begin(), lines.end(),
                               [](const std::string& line) { return line.empty(); }),
                lines.end());

    // assign formmated lyrics to member
    return lines;
  }
};

/**
 * @struct SearchState
 * @brief Holds the state for searching artists and songs.
 *
 * This structure stores the current search state, including search queries and indices for
 * navigating through the search results for artists and songs.
 */
struct SearchState
{
  Trie   ArtistSearchTrie; /**< Trie for searching artists */
  Trie   SongSearchTrie;   /**< Trie for searching songs */
  int    artistIndex = 0;  /**< Index for the current artist search result */
  int    songIndex   = 0;  /**< Index for the current song search result */
  string input       = ""; /**< The current search input */
  int    albumsIndex = 0;  /**< Index for the current album search result */
  mutex  mtx;              /**< Mutex for synchronizing access to search state */
};

/**
 * @struct QueueState
 * @brief Represents the state of the song queue.
 *
 * This structure holds the song queue and the indices for navigating and managing the queue.
 */

struct QueueState
{
  vector<Song>   song_queue;       /**< The list of songs in the queue */
  vector<string> song_queue_names; /**< The list of song names in the queue */
  int            qIndex       = 0; /**< Index of the current song in the queue */
  int            qScreenIndex = 0; /**< Index of the current song being displayed on the screen */

  /**
   * @brief Clears the song queue.
   *
   * This method removes all songs from the queue and resets the queue index.
   */
  void clearQueue()
  {
    song_queue.clear();
    qIndex = 0;
  }
  /**
   * @brief Increments the queue index.
   *
   * This method increments the index to point to the next song in the queue.
   */
  void incrementQIndex() { qIndex++; }
  /**
   * @brief Decrements the queue index.
   *
   * This method decrements the index to point to the previous song in the queue.
   */
  void decrementQIndex() { qIndex--; }
  /**
   * @brief Inserts a song at the current index in the queue.
   *
   * This method inserts a new song at the position immediately after the current queue index.
   *
   * @param newSong The song to insert into the queue.
   */
  void insertSongToIndex(const Song& newSong)
  {
    song_queue.insert(song_queue.begin() + qIndex + 1, newSong);
  }
  /**
   * @brief Pushes a song to the end of the queue.
   *
   * This method appends a song to the end of the song queue.
   *
   * @param song The song to add to the queue.
   */
  void qPush(const Song& song) { song_queue.push_back(song); }
  /**
   * @brief Pops the song at the current screen index from the queue.
   *
   * This method removes the song at the current screen index from the queue.
   */
  void qPopIndex() { song_queue.erase(song_queue.begin() + qScreenIndex); }
  /**
   * @brief Returns the current size of the song queue.
   *
   * This method returns the number of songs currently in the queue.
   *
   * @return The size of the song queue.
   */
  auto getQueueSize() -> int { return song_queue.size(); }
  /**
   * @brief Retrieves the current song from the queue.
   *
   * This method returns a pointer to the song currently selected in the queue.
   *
   * @return A pointer to the current song in the queue, or nullptr if the queue is empty.
   */
  auto GetCurrentSongFromQueue() -> Song*
  {
    if (!song_queue.empty() && qIndex < song_queue.size())
    {
      return &song_queue[qIndex];
    }

    return nullptr;
  }
  /**
   * @brief Updates the list of song names in the queue.
   *
   * This method updates the `song_queue_names` list with the names of the songs from the queue,
   * marking the current song with an asterisk.
   */
  void UpdateSongQueueList()
  {
    song_queue_names.clear();

    for (long unsigned int i = qIndex; i < song_queue.size(); i++)
    {
      std::string eleName = song_queue[i].metadata.title + " by " + song_queue[i].metadata.artist;
      if (i == qIndex)
        eleName += "  *";
      song_queue_names.push_back(eleName);
    }

    return;
  }
};
