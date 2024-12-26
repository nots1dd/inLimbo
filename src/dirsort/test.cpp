#include "inode_mapper.hpp"

int main(int argc, char* argv[])
{
  /*if (argc != 2)*/
  /*{*/
  /*  cerr << "Usage: " << argv[0] << " <directory_path>" << endl;*/
  /*  return EXIT_FAILURE;*/
  /*}*/

  string          directoryPath = string(parseField("library", "directory"));
  RedBlackTree    rbt;
  InodeFileMapper mapper("lib.sync");

  auto start = chrono::high_resolution_clock::now();
  processDirectory(directoryPath, rbt, mapper);
  cout << "Inorder traversal of inodes: ";
  rbt.inorderStoreMetadata();
  cout << endl;
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
 * -> Perform inorder traversal of rbtree and during traversal, call TagLibParser to parse the metadata of each inode
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
