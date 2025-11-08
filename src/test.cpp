#include "Config.hpp"
#include "core/InodeMapper.hpp"
#include "thread/Map.hpp"
#include "thread/Map.hpp"
#include "toml/Parser.hpp"
#include "Logger.hpp"
#include "query/SongMap.hpp"
#include "utils/signal/SignalHandler.hpp"
#include "utils/timer/Timer.hpp"
#include "audio/Playback.hpp"
#include "frontend/cmd-line/CMD-LINE.hpp"

threads::SafeMap<dirsort::SongMap> g_songMap;

auto main(int argc, char* argv[]) -> int
{
    RECORD_FUNC_TO_BACKTRACE("MAIN");
    utils::SignalHandler::getInstance().setup();
    
    TagLibParser parser(dirsort::DEBUG_LOG_PARSE);

    if (argc <= 1 || (argc > 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h"))) {
        std::cout << "Usage: " << argv[0] << " [example_song_name] [--edit-metadata]\n"
                  << "  example_song_name : Name of a song to demonstrate playback (default: 'Example Song')\n"
                  << "  --edit-metadata   : Simulate editing metadata of the example song after playback\n";
        return 0;
    }
    
    const std::string exampleSongName = argc > 1 ? argv[1] : "Example Song";

    const std::string directoryPath = string(parser::parseTOMLField("library", "directory"));
    const std::string libBinPath    = utils::getConfigPath(LIB_BIN_NAME);

    LOG_INFO("Configured music directory: {} and found song name: {}", directoryPath, exampleSongName);

    util::Timer<> timer;
    dirsort::RedBlackTree<ino_t, dirsort::Song> rbt;
    InodeFileMapper mapper;
    dirsort::SongTree song_tree;

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
    rbt.setVisitCallback([&song_tree, &parser](const ino_t& inode, dirsort::Song& song) {
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

    query::songmap::read::forEachSong(g_songMap, [](const Artist& artist, const Album& album, const Disc disc, const Track track, const ino_t inode, const auto& song){
      LOG_INFO("Found song {}: {} by {} in {} ({}/{})", inode, song.metadata.title, artist, album, disc, track);
  });

    // synchronous display
    timer.reset();
    song_tree.display(dirsort::DisplayMode::Summary);
    LOG_INFO("Library displayed in {:.2f} ms", timer.elapsed_ms());  

    auto exampleSong = query::songmap::read::findSongByName(g_songMap, exampleSongName);

    ASSERT_MSG(exampleSong != nullptr, "Example file not found in the library!");

    auto exampleFilePath = exampleSong->metadata.filePath;
    auto exampleFileMetadata = exampleSong->metadata;

    LOG_INFO("Example file: {}", exampleFilePath);

    audio::AudioEngine engine;

    auto devices = engine.enumeratePlaybackDevices();
    if (devices.empty()) {
        LOG_ERROR("No playback devices found. Exiting.");
        return 1;
    }

    // now you can do some cool things with g_songMap like querying songs by artist/album, etc. (check query/SongMap.hpp)
    // 
    //auto songs = utils::songmap::read::getSongsByAlbum(g_songMap, "Radiohead", "In Rainbows");

    //for (const auto& song : songs) {
    //  std::string s = std::format(" - {} (Disc {} :Track {})\n", song.metadata.title, song.metadata.discNumber, song.metadata.track);
    //  std::cout << s;
    //}
    
        if (argc > 2 && std::string(argv[2]) == "--edit-metadata")
    {
      // now let us modify the metadata of the example file and see if we can update the song map
      // 
      // something to note is that we arent actually changing the file's metadata on disk here,
      // we are just simulating a metadata change in memory and updating the song map accordingly.
      //
      // in the future we can use taglib itself to modify the required metadata.
      auto newSongObj = *exampleSong; // copy existing
      newSongObj.metadata.title = exampleSong->metadata.title + " (Edited)";
      
      LOG_INFO("Setting new title as : {} -> {} to '{}' path.", exampleSong->metadata.title, newSongObj.metadata.title, newSongObj.metadata.filePath);
      
      LOG_INFO("replaceSongObjAndUpdateMetadata(): looking for oldSong.inode={}, newSong.inode={}",
         exampleSong->inode, newSongObj.inode);

      bool replaced = query::songmap::mut::replaceSongObjAndUpdateMetadata(
          g_songMap,
          *exampleSong,
          newSongObj, parser);

      // now we need to play this in the song tree and save it back to disk via serialization
      if (replaced) {
          LOG_INFO("Successfully replaced song object in SongMap.");
        
          // firstly update the true metadata using taglib 

          // Rebuild SongTree from updated SongMap
          song_tree.clear();
          song_tree.newSongMap(g_songMap.snapshot());
          song_tree.setMusicPath(directoryPath);
          song_tree.saveToFile(libBinPath);
          LOG_INFO("Saved updated song tree to cache after metadata modification: {}", libBinPath);
      }
      else {
          LOG_ERROR("Failed to replace song object in SongMap.");
      }
    }
    
    // initialize the frontend 
    //
    // NOTE!! THE SONG MAP WILL BECOME EMPTY AFTER THIS CALL!!!!
    // 
    // future access points are all within the cmdInterface object
    frontend::cmdline::Interface cmdInterface(g_songMap); 
    const size_t selectedIndex = cmdInterface.selectAudioDevice(devices);

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
    engine.startInteractiveLoop([&cmdInterface, &exampleFileMetadata](audio::AudioEngine& eng) { 
        cmdInterface.run(eng, exampleFileMetadata);
    });

    // Main thread stays responsive (we are assuming the UI runs here)
    while (engine.shouldRun())
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // frontend should cleanup and exit gracefully. song map is destroyed within the frontend object 
    // 
    // other things to note is that the frontend is responsible for saving any changes to the song map back to disk
    // via cereal serialization through SongTree object.
  
    return 0;
}
