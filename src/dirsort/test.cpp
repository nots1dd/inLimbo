#include "core/InodeMapper.hpp"
#include "Logger.hpp"
#include "StackTrace.hpp"

auto main(int argc, char* argv[]) -> int
{
  RECORD_FUNC_TO_BACKTRACE("MAIN");

  string          directoryPath = string(parseTOMLField("library", "directory"));

  LOG_INFO("Directory found: {}", directoryPath);

  string          libBinPath   = getConfigPath(LIB_BIN_NAME);
  dirsort::RedBlackTree    rbt;
  InodeFileMapper mapper;

  auto start = chrono::high_resolution_clock::now();
  dirsort::SongTree song_tree;

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
    rbt.inorderStoreMetadata();
    song_tree = rbt.returnSongTree();
    song_tree.saveToFile(libBinPath);
  }

  auto        library_map = song_tree.returnSongMap();
  auto end = chrono::high_resolution_clock::now();

  song_tree.display();

  cout << "Inode insertion, mapping and parsing time: "
       << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;

  DUMP_TRACE

  return 0;
}
