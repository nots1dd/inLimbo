#include "core/InodeMapper.hpp"
#include "Logger.hpp"
#include "StackTrace.hpp"
#include "core/SongTree.hpp"
#include "toml/Parser.hpp"
#include "utils/Env-Vars.hpp"
#include "utils/signal/SignalHandler.hpp"
#include "utils/timer/Timer.hpp"

auto main(int argc, char* argv[]) -> int
{
  RECORD_FUNC_TO_BACKTRACE("MAIN");

  utils::SignalHandler::getInstance().setup();

  string          directoryPath = string(parser::parseTOMLField("library", "directory"));

  LOG_INFO("Directory found: {}", directoryPath);

  string          libBinPath   = utils::getConfigPath(LIB_BIN_NAME);
  dirsort::RedBlackTree<ino_t, dirsort::Song> rbt;
  InodeFileMapper mapper;

  util::Timer<> timer;
  dirsort::SongTree song_tree;

  rbt.setVisitCallback([&song_tree](const ino_t& inode, dirsort::Song& song) {
        // TagLib parsing per-inode — reuse the same parser instance if desired
        // DEBUG_LOG_PARSE is assumed to be available in your config or parser module
        TagLibParser parser(dirsort::DEBUG_LOG_PARSE);
        auto metadataMap = parser.parseFromInode(inode, dirsort::DIRECTORY_FIELD);
        if (metadataMap.empty()) {
            LOG_WARN("No metadata found for inode {}: {}", inode, song.metadata.filePath);
            return;
        }

        for (const auto& pair : metadataMap) {
            // pair.first = filename, pair.second = metadata (as in your previous code)
            song_tree.addSong(dirsort::Song(inode, pair.second));
        }
  });

  // Try to load the song tree from file
  try
  {
    song_tree.loadFromFile(libBinPath);
    LOG_INFO("Successfully loaded song tree from cache file.");
  }
  catch (const std::exception& e)
  {
    LOG_ERROR("Could not load song tree from file: {}", e.what());
    LOG_INFO("Processing directory instead...");

    processDirectory(directoryPath, rbt, mapper);
    rbt.inorder();
    song_tree.saveToFile(libBinPath);

    LOG_INFO("Saved generated song tree to cache: {}", libBinPath);
  }

  auto        library_map = song_tree.returnSongMap();

  song_tree.display();

  LOG_INFO("Inode insertion, mapping, and parsing completed in {:.2f} ms", timer.elapsed_ms());

  return 0;
}
