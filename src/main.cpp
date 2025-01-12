#include "dirsort/inode_mapper.hpp"
#include "ui/ui_handler.hpp"
#include "signal/signalHandler.hpp"
#include <memory>
#include <random>

int main(int argc, char* argv[])
{
  SignalHandler::getInstance().setup();
  string          directoryPath = string(parseTOMLField(PARENT_LIB, PARENT_LIB_FIELD_DIR));
  string          libSyncPath   = getConfigPath(LIB_SYNC_NAME);
  RedBlackTree    rbt;
  InodeFileMapper mapper(libSyncPath, "false");

  auto start = chrono::high_resolution_clock::now();
  cout << "-- Reading directory..." << endl;
  processDirectory(directoryPath, rbt, mapper);
  rbt.inorderStoreMetadata();
  auto        end         = chrono::high_resolution_clock::now();
  auto        song_tree   = rbt.returnSongTree();
  auto        library_map = song_tree.returnSongMap();
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
