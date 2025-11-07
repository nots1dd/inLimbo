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

    auto exampleFilePath = utils::songmap::findSongByName(g_songMap, "Weird Fishes / Arpeggi")->metadata.filePath;

    LOG_INFO("Example file from 'In Rainbows': {}", exampleFilePath);

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
        LOG_INFO("Initializing playback engine for device '{}'...", devices[selectedIndex].name);
        engine.initEngineForDevice(&devices[selectedIndex].id);
        LOG_INFO("Playback engine successfully initialized for '{}'", devices[selectedIndex].name);
    } catch (const std::exception& e) {
        LOG_CRITICAL("Failed to initialize playback engine for device '{}': {}", devices[selectedIndex].name, e.what());
        return 1;
    }

    engine.printDeviceInfo();
    engine.loadSound(exampleFilePath);
    engine.setVolume(0.5f);
    
    engine.startInteractiveLoop([](audio::AudioEngine& eng) {
        std::string cmd;
        while (eng.shouldRun()) {
            std::cout << "[play/pause/quit]: ";
            std::getline(std::cin, cmd);

            if (cmd == "pause") eng.pause();
            else if (cmd == "play") eng.play();
            else if (cmd == "quit") {
                std::cout << "Stopping playback...\n";
                eng.stopInteractiveLoop();
                break;
            }
        }
    });

    // Main thread stays responsive
    while (engine.shouldRun())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));


    //auto songs = utils::songmap::getSongsByAlbum(g_songMap, "Radiohead", "In Rainbows");

    //for (const auto& song : songs) {
    //  std::string s = std::format(" - {} (Disc {} :Track {})\n", song.metadata.title, song.metadata.discNumber, song.metadata.track);
    //  std::cout << s;
    //}
    
    return 0;
}
