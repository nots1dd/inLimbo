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

static auto printBanner()
{
  std::cout << "──────────────────────────────────────────────\n"
            << "\nAudio Control Commands:\n"
            << "  play, pause, restart, back <s>, forward <s>, seek <s>\n"
            << "  volume <0.0–1.0>, info, quit\n"
            << "──────────────────────────────────────────────\n";
}

static auto handleAudioCommand(audio::AudioEngine& eng,
                               const std::string& input,
                               const Metadata& meta) -> bool
{
    std::istringstream iss(input);
    std::string cmd;
    iss >> cmd;

    if (cmd.empty())
        return true;

    const auto toLower = [](std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return s;
    };

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
        std::cout << "\n📀 Metadata Info:\n"
                  << "  Title:    " << meta.title << "\n"
                  << "  Artist:   " << meta.artist << "\n"
                  << "  Album:    " << meta.album << "\n"
                  << "  Track:    " << meta.discNumber << "/" << meta.track << "\n"
                  << "  Year:     " << meta.year << "\n"
                  << "  Duration: " << meta.duration << " sec\n"
                  << "  Bitrate:  " << meta.bitrate << " kbps\n";
    }
    else if (cmd == "seek" || cmd == "back" || cmd == "forward") {
        double seconds = 0.0;
        if (!(iss >> seconds)) {
            std::cout << "Usage: " << cmd << " <seconds>\n";
            return true;
        }

        if (cmd == "seek")       eng.seekTo(seconds);
        else if (cmd == "back")  eng.seekBackward(seconds);
        else if (cmd == "forward") eng.seekForward(seconds);

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
    else if (cmd == "quit") {
        std::cout << "!! Stopping playback and exiting...\n";
        eng.stopInteractiveLoop();
        return false;
    }
    else {
        std::cout << "❓ Unknown command: " << cmd << "\n"
                  << "Available: play, pause, info, restart, seek, back, forward, volume, info, quit\n";
    }

    return true;
}

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
    auto exampleFileMetadata = exampleSong->metadata;

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
    engine.startInteractiveLoop([&exampleFileMetadata](audio::AudioEngine& eng) {
        std::string cmd;
        
        printBanner();

        while (eng.shouldRun()) {
            std::cout << "\n[audio]> ";
            if (!std::getline(std::cin, cmd))
                break;

            if (!handleAudioCommand(eng, cmd, exampleFileMetadata))
                break;
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
