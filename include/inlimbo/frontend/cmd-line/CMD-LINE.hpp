#pragma once

#include <iostream>
#include <sstream>
#include <algorithm>

#include "audio/Playback.hpp"
#include "core/SongTree.hpp"
#include "core/taglib/Parser.hpp"
#include "query/SongMap.hpp"

namespace frontend::cmdline
{

class Interface
{
public:
    explicit Interface(threads::SafeMap<dirsort::SongMap>& songMap)
        : songs_(std::move(songMap)) {}

    void run(audio::AudioEngine& eng, const Metadata& meta)
    {
        printBanner();
        std::string input;

        while (eng.shouldRun()) {
            std::cout << "\n[audio]> ";
            if (!std::getline(std::cin, input))
                break;

            if (!handleCommand(eng, input, meta))
                break;
        }
    }

    auto selectAudioDevice(const std::vector<audio::DeviceInfo>& devices) -> size_t
    {
        std::cout << "\nAvailable Playback Devices:\n";
        for (size_t i = 0; i < devices.size(); ++i)
            std::cout << "  [" << i << "] " << devices[i].name << "\n";

        size_t selectedIndex = 0;
        while (true) {
            std::cout << "\nSelect a playback device index: ";
            if (!(std::cin >> selectedIndex)) {
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Invalid input. Please enter a number.\n";
                continue;
            }

            if (selectedIndex >= devices.size()) {
                std::cout << "Invalid index " << selectedIndex
                          << ". Valid range: [0 - " << devices.size() - 1 << "]\n";
                continue;
            }
            break;
        }
        // Clear the newline left in the input buffer
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return selectedIndex;
    }

private:
    threads::SafeMap<dirsort::SongMap> songs_;

    static void printBanner()
    {
        std::cout << "──────────────────────────────────────────────\n"
                  << " Audio Control Interface\n"
                  << "──────────────────────────────────────────────\n"
                  << " Commands:\n"
                  << "  play, pause, restart, back <s>, forward <s>, seek <s>\n"
                  << "  volume <0.0–1.0>, info, list songs, list artists, quit\n"
                  << "──────────────────────────────────────────────\n";
    }

    static auto toLower(std::string s) -> std::string
    {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return s;
    }

    auto handleCommand(audio::AudioEngine& eng,
                       const std::string& input,
                       const Metadata& meta) -> bool
    {
        std::istringstream iss(input);
        std::string cmd;
        iss >> cmd;
        if (cmd.empty())
            return true;

        cmd = toLower(cmd);

        if (cmd == "play") {
            eng.play();
            std::cout << "-- Resumed playback.\n";
        }
        else if (cmd == "pause") {
            eng.pause();
            std::cout << "-- Paused.\n";
        }
        else if (cmd == "restart") {
            eng.restart();
            std::cout << "-- Restarted track.\n";
        }
        else if (cmd == "info") {
            printMetadata(meta);
        }
        else if (cmd == "seek" || cmd == "back" || cmd == "forward") {
            double seconds = 0.0;
            if (!(iss >> seconds)) {
                std::cout << "Usage: " << cmd << " <seconds>\n";
                return true;
            }

            if (cmd == "seek") eng.seekTo(seconds);
            else if (cmd == "back") eng.seekBackward(seconds);
            else eng.seekForward(seconds);

            std::cout << "-- Seek: " << cmd << " " << seconds << "s\n";
        }
        else if (cmd == "volume") {
            float vol = -1.0f;
            if (!(iss >> vol) || vol < 0.0f || vol > 1.0f) {
                std::cout << "Usage: volume <0.0 - 1.0>\n";
                return true;
            }
            eng.setVolume(vol);
            std::cout << "-- Volume set to " << vol * 100.0f << "%.\n";
        }
        else if (cmd == "list") {
            std::string what;
            iss >> what;
            if (what == "songs")
                listSongs();
            else if (what == "artists")
                listArtists();
            else
                std::cout << "Usage: list <songs|artists>\n";
        }
        else if (cmd == "quit") {
            std::cout << "!! Stopping playback and exiting...\n";
            eng.stopInteractiveLoop();
            return false;
        }
        else {
            std::cout << "?? Unknown command: " << cmd << "\n"
                      << "Available: play, pause, restart, seek, back, forward, "
                         "volume, info, list songs, list artists, quit\n";
        }

        return true;
    }

    void listArtists() const
    {
      query::songmap::read::forEachArtist(songs_, [](const Artist& artist, const dirsort::AlbumMap& albums) {
          std::cout << "Artist: " << artist << "\n";
          std::cout << "  Albums (" << albums.size() << "):\n";

          for (const auto& [albumName, discs] : albums) {
              std::cout << "    - " << albumName << " (" << discs.size() << " discs)\n";
          }
      });
    }

    void listSongs() const
    {
      query::songmap::read::forEachSong(songs_, [](const Artist& artist, const Album& album, const Disc disc, const Track track, const ino_t inode, const dirsort::Song& song) {
          std::cout << "Artist: " << artist
                    << " | Album: " << album
                    << " | Disc: " << disc
                    << " | Track: " << track
                    << " | Inode: " << inode
                    << " | Title: " << song.metadata.title
                    << " | File: " << song.metadata.filePath
                    << "\n";
      });
    }
};

} // namespace frontend::cmdline
