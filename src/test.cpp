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
#include "CmdLine.hpp"

threads::SafeMap<dirsort::SongMap> g_songMap;

auto main(int argc, char* argv[]) -> int
{
    RECORD_FUNC_TO_BACKTRACE("MAIN");
    utils::SignalHandler::getInstance().setup();
    
    cli::CmdLine args("inLimbo", "inLimbo (CMD-LINE) music library tool");

    args.add<std::string>(
        "General",
        "song", 's',
        "Example song name",
        std::nullopt,
        [](const std::string& s) -> bool { return !s.empty(); },
        "song name cannot be empty",
        cli::Requirement::Required
    );

    args.add<float>(
        "General",
        "volume", 'v',
        "Playback volume (0-100)",
        75.0,
        [](const float& vol) -> bool { return vol >= 0 && vol <= 100; },
        "Volume must be between 0.0 and 100.0"
    );

    args.addFlag(
        "General",
        "edit-metadata", 'e',
        "Simulate editing metadata after playback"
    );

    try 
    {
      args.parse(argc, argv);
      const auto exampleSongName = args.get<std::string>("song");
      const bool editMetadata = args.has("edit-metadata");
      const float volume =
          args.getOptional<float>("volume").value_or(75.0f);

      const float normalizedVolume = std::clamp(volume / 100.0f, 0.0f, 1.5f);
   
      TagLibParser parser(dirsort::DEBUG_LOG_PARSE); 

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
      rbt.setVisitCallback([&song_tree, &parser](const ino_t& inode, dirsort::Song& song) -> void {
          auto metadataMap = parser.parseFromInode(inode, dirsort::DIRECTORY_FIELD);

          if (metadataMap.empty()) {
              LOG_WARN("No metadata found for inode {}: {}", inode, song.metadata.filePath);
              return;
          }

          for (const auto& [filename, metadata] : metadataMap)
              song_tree.addSong(dirsort::Song(inode, metadata));
      });

      if (needRebuild) {
          timer.restart();
          song_tree.clear();

          processDirectory(directoryPath, rbt, mapper);
          rbt.inorder(); // triggers parsing callback
          song_tree.setMusicPath(directoryPath);
          timer.stop();
          song_tree.saveToFile(libBinPath);
          LOG_INFO("Saved updated song tree to cache: {} in {} ms", libBinPath, timer.elapsed_ms());
      }

      // Generate SongMap from SongTree
      const auto library_map = song_tree.returnSongMap();

      LOG_INFO("Inode mapping and parsing completed in {:.2f} ms", timer.elapsed_ms());

      // Thread-safe replacement (atomic swap)
      g_songMap.replace(std::move(library_map));
      
      // now you can do some cool things with g_songMap like querying songs by artist/album, etc. (check query/SongMap.hpp)
      // 
      // auto songs = query::songmap::read::getSongsByAlbum(g_songMap, "Radiohead", "In Rainbows");

      // for (const auto& song : songs) {
      //   std::string s = std::format(" - {} (Disc {} :Track {})\n", song.metadata.title, song.metadata.discNumber, song.metadata.track);
      //   std::cout << s;
      // }

      query::songmap::read::forEachSong(g_songMap, [](const Artist& artist, const Album& album, const Disc disc, const Track track, const ino_t inode, const auto& song) -> void {
        LOG_TRACE("Found song {}: {} by {} in {} ({}/{})", inode, song.metadata.title, artist, album, disc, track);
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

      
      if (editMetadata)
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
          // useless
          engine.initEngineForDevice(devices[selectedIndex].name);
          LOG_INFO("Playback engine successfully initialized for '{}'", devices[selectedIndex].name);
      } catch (const std::exception& e) {
          LOG_CRITICAL("Failed to initialize playback engine for device '{}': {}", devices[selectedIndex].name, e.what());
          return 1;
      }

      const auto songIdx = *engine.loadSound(exampleFilePath);
      engine.setVolume(normalizedVolume);

      engine.restart();
      cmdInterface.run(engine, exampleFileMetadata);
      engine.stop();    

      // frontend should cleanup and exit gracefully. song map is destroyed within the frontend object 
      // 
      // other things to note is that the frontend is responsible for saving any changes to the song map back to disk
      // via cereal serialization through SongTree object.
    
      return EXIT_SUCCESS;
  } 
  catch (const cli::CmdLine::HelpRequested& h) 
  {
      std::cout << h.text << std::endl;
      return EXIT_SUCCESS;
  }
  catch (const cli::CmdLine::CliError& e) 
  {
      std::cerr << e.message << std::endl;
      return EXIT_FAILURE;
  }
}
