#include "dirsort/inode_mapper.hpp"
#include "signal/signalHandler.hpp"
#include "ui/ui_handler.hpp"
#include <memory>
#include <random>

int main(int argc, char* argv[])
{
  SignalHandler::getInstance().setup();
  auto            directoryPath = string(parseTOMLField(PARENT_LIB, PARENT_LIB_FIELD_DIR));
  string          libSyncPath  = getConfigPath(LIB_SYNC_NAME);
  string          libBinPath   = getConfigPath(LIB_BIN_NAME);
  RedBlackTree    rbt;
  InodeFileMapper mapper(libSyncPath, "false");

  auto start = chrono::high_resolution_clock::now();
  cout << "-- Reading directory..." << endl;

  SongTree song_tree;

  // Try to load the song tree from file
  try
  {
    song_tree.loadFromFile(libBinPath);
    cout << "-- Successfully loaded song tree from file." << endl;
  }
  catch (const std::exception& e)
  {
    cout << "-- Could not load song tree from file: " << e.what() << endl;
    cout << "-- Processing directory instead..." << endl;

    processDirectory(directoryPath, rbt, mapper);
    rbt.inorderStoreMetadata();
    song_tree = rbt.returnSongTree();
    song_tree.saveToFile(libBinPath);
  }

  auto        library_map = song_tree.returnSongMap();
  auto end = chrono::high_resolution_clock::now();
  MusicPlayer player(library_map);
  try
  {
    player.Run();
  }
  catch (std::exception e)
  {
    std::cout << "Some issue has occurred with inLimbo: " << e.what() << std::endl
              << "Exiting with code 1." << std::endl;
    return 1;
  }

  cout << "-- Inode insertion & mapping time: "
       << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;

  return 0;
}
