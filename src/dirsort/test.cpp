#include "inode_mapper.hpp"

auto main(int argc, char* argv[]) -> int
{
  string          directoryPath = string(parseTOMLField("library", "directory"));
  string          libBinPath   = getConfigPath(LIB_BIN_NAME);
  RedBlackTree    rbt;
  InodeFileMapper mapper;

  auto start = chrono::high_resolution_clock::now();
  SongTree song_tree;

  // Try to load the song tree from file
  try
  {
    song_tree.loadFromFile(libBinPath);
    cout << "-- Successfully loaded song tree from cache file." << endl;
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

  rbt.printSongTree();

  cout << "Inode insertion, mapping and parsing time: "
       << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;

  return 0;
}

/************************************************************************************************************************
 *
 * @SONG MAP
 *
 * Steps:
 *
 * -> Read the required songs directory
 * -> Take in every inode into an rbtree and map them all to lib.sync
 * -> Perform inorder traversal of rbtree and during traversal, call TagLibParser to parse the
 *metadata of each inode
 * -> Print everything onto the terminal
 *
 *
 * Ideal process of forming song tree:
 *
 * 1. Traverse rbtree for each inode and parse its metadata
 * 2. Send over the metadata and inode to songtree class
 * 3. The data structure can be a nested maps for easier tracking and decently fast access times
 * 4. Checking order for a song addition: Artist -> Album -> Disc no -> Track no
 *
 * Average time: 218ms for 274 songs
 *
 *************************************************************************************************************************/
