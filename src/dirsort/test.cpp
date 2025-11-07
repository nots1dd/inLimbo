#include "core/InodeMapper.hpp"
#include "thread/Map.hpp"
#include "thread/Map.hpp"
#include "toml/Parser.hpp"
#include "Logger.hpp"
#include "utils/SongMap.hpp"
#include "utils/signal/SignalHandler.hpp"
#include "utils/timer/Timer.hpp"
#include "audio/Playback.hpp"
#include <format>

auto main(int argc, char* argv[]) -> int
{
    RECORD_FUNC_TO_BACKTRACE("MAIN");
    utils::SignalHandler::getInstance().setup();

    const std::string directoryPath = string(parser::parseTOMLField("library", "directory"));
    const std::string libBinPath    = utils::getConfigPath(LIB_BIN_NAME);

    LOG_INFO("Configured music directory: {}", directoryPath);

    util::Timer<> timer;
    dirsort::RedBlackTree<ino_t, dirsort::Song> rbt;
    InodeFileMapper mapper;
    dirsort::SongTree song_tree;

    // Global thread-safe SongMap instance
    static threads::SafeMap<dirsort::SongMap> g_songMap;

    bool needRebuild = false;
    try {
        song_tree.loadFromFile(libBinPath);

        const std::string cachedPath = song_tree.returnMusicPath();
        if (cachedPath != directoryPath) {
            LOG_WARN("Cached directory '{}' differs from configured '{}'. Rebuilding...",
                     cachedPath, directoryPath);
            needRebuild = true;
        } else {
            LOG_INFO("Loaded song tree from cache (directory: {}).", cachedPath);
        }
    }
    catch (const std::exception& e) {
        LOG_ERROR("Failed to load cache: {}. Rebuilding...", e.what());
        needRebuild = true;
    }

    // Define parsing callback once
    rbt.setVisitCallback([&song_tree](const ino_t& inode, dirsort::Song& song) {
        TagLibParser parser(dirsort::DEBUG_LOG_PARSE);
        auto metadataMap = parser.parseFromInode(inode, dirsort::DIRECTORY_FIELD);

        if (metadataMap.empty()) {
            LOG_WARN("No metadata found for inode {}: {}", inode, song.metadata.filePath);
            return;
        }

        for (const auto& [filename, metadata] : metadataMap)
            song_tree.addSong(dirsort::Song(inode, metadata));
    });

    if (needRebuild) {
        song_tree.clear();

        processDirectory(directoryPath, rbt, mapper);
        rbt.inorder(); // triggers parsing callback
        song_tree.setMusicPath(directoryPath);
        song_tree.saveToFile(libBinPath);
        LOG_INFO("Saved updated song tree to cache: {}", libBinPath);
    }

    // Generate SongMap from SongTree
    auto library_map = song_tree.returnSongMap();

    LOG_INFO("Inode mapping and parsing completed in {:.2f} ms", timer.elapsed_ms());

    // Thread-safe replacement (atomic swap)
    g_songMap.replace(std::move(library_map));

    // synchronous display
    timer.reset();
    song_tree.display(dirsort::DisplayMode::Summary);
    LOG_INFO("Library displayed in {:.2f} ms", timer.elapsed_ms());  

    auto exampleSong = utils::songmap::findSongByName(g_songMap, "duvet");

    assert(exampleSong != nullptr && "Example file not found in the library!");

    auto exampleFilePath = exampleSong->metadata.filePath;

    LOG_INFO("Example file: {}", exampleFilePath);

    audio::AudioEngine engine;

    auto devices = engine.enumeratePlaybackDevices();
    if (devices.empty()) {
        LOG_ERROR("No playback devices found. Exiting.");
        return 1;
    }

    std::cout << "\nAvailable Playback Devices:\n";
    for (size_t i = 0; i < devices.size(); ++i)
        std::cout << "  [" << i << "] " << devices[i].name << "\n";

    // Ask user for device index
    size_t selectedIndex = 0;
    while (true) {
        std::cout << "\nSelect a playback device index: ";
        if (!(std::cin >> selectedIndex)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            LOG_WARN("Invalid input. Please enter a number.");
            continue;
        }

        if (selectedIndex >= devices.size()) {
            LOG_WARN("Invalid index {}. Valid range: [0 - {}]", selectedIndex, devices.size() - 1);
            continue;
        }
        break;
    }

    LOG_INFO("Selected device index: {} -> {}", selectedIndex, devices[selectedIndex].name);

    try {
        engine.initEngineForDevice(&devices[selectedIndex].id);
        LOG_INFO("Playback engine successfully initialized for '{}'", devices[selectedIndex].name);
    } catch (const std::exception& e) {
        LOG_CRITICAL("Failed to initialize playback engine for device '{}': {}", devices[selectedIndex].name, e.what());
        return 1;
    }

    engine.printDeviceInfo();
    engine.loadSound(exampleFilePath);
    engine.setVolume(1.0f);
    
    // simple interactive loop to connect with audio backend (should work well with any UI frontend)
    engine.startInteractiveLoop([](audio::AudioEngine& eng) {
        std::string cmd;

        std::cout << "\nAudio Control Commands:\n"
                  << "  play              → Resume playback\n"
                  << "  pause             → Pause playback\n"
                  << "  restart           → Restart current track\n"
                  << "  back <seconds>    → Rewind by N seconds\n"
                  << "  forward <seconds> → Skip ahead by N seconds\n"
                  << "  seek <seconds>    → Seek to absolute N seconds\n"
                  << "  volume <0.0-1.0>  → Set playback volume\n"
                  << "  quit              → Stop and exit\n"
                  << "──────────────────────────────────────────────\n";

        while (eng.shouldRun()) {
            std::cout << "\n[audio]> ";
            if (!std::getline(std::cin, cmd))
                break; // EOF or input stream closed

            std::istringstream iss(cmd);
            std::string action;
            iss >> action;

            if (action == "play") {
                eng.play();
                std::cout << "Resumed playback.\n";
            } 
            else if (action == "pause") {
                eng.pause();
                std::cout << "Paused.\n";
            }
            else if (action == "restart") {
                eng.restart();
                std::cout << "Restarted track.\n";
            }
            else if (action == "seek") {
                double seconds = 0.0;
                if (iss >> seconds) {
                    eng.seekTo(seconds);
                    std::cout << "Seeked to " << seconds << " seconds.\n";
                } else {
                    std::cout << "Usage: seek <seconds>\n";
                }
            }
            else if (action == "back") {
                double seconds = 0.0;
                if (iss >> seconds) {
                    eng.seekBackward(seconds);
                    std::cout << "Rewound " << seconds << " seconds.\n";
                } else {
                    std::cout << "Usage: back <seconds>\n";
                }
            }
            else if (action == "forward") {
                double seconds = 0.0;
                if (iss >> seconds) {
                    eng.seekForward(seconds);
                    std::cout << "Skipped ahead " << seconds << " seconds.\n";
                } else {
                    std::cout << "Usage: forward <seconds>\n";
                }
            }
            else if (action == "volume") {
                float vol = -1.0f;
                if (iss >> vol && vol >= 0.0f && vol <= 1.0f) {
                    eng.setVolume(vol);
                    std::cout << "Volume set to " << vol * 100.0f << "%.\n";
                } else {
                    std::cout << "Usage: volume <0.0 - 1.0>\n";
                }
            }
            else if (action == "quit") {
                std::cout << "🛑  Stopping playback and exiting...\n";
                eng.stopInteractiveLoop();
                break;
            }
            else if (!action.empty()) {
                std::cout << "Unknown command: " << action << "\n"
                          << "Type one of: play, pause, restart, seek, back, forward, volume, quit\n";
            }
        }
    });

    // Main thread stays responsive (we are assuming the UI runs here)
    while (engine.shouldRun())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));


    //auto songs = utils::songmap::getSongsByAlbum(g_songMap, "Radiohead", "In Rainbows");

    //for (const auto& song : songs) {
    //  std::string s = std::format(" - {} (Disc {} :Track {})\n", song.metadata.title, song.metadata.discNumber, song.metadata.track);
    //  std::cout << s;
    //}
    
    return 0;
}
