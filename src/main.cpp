#include "dirsort/inode_mapper.hpp"
#include "ui/ui_handler.hpp"
#include <memory>
#include <random>

int main()
{
  string          directoryPath = string(parseTOMLField(PARENT_LIB, PARENT_LIB_FIELD_DIR));
  RedBlackTree    rbt;
  InodeFileMapper mapper(LIB_SYNC_PATH);

  auto start = chrono::high_resolution_clock::now();
  processDirectory(directoryPath, rbt, mapper);
  cout << "Inorder traversal of inodes: ";
  rbt.inorderStoreMetadata();
  cout << endl;
  auto end = chrono::high_resolution_clock::now();

  cout << "Inode insertion, mapping and parsing time: "
       << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;

  auto        song_tree   = rbt.returnSongTree();
  auto        library_map = song_tree.returnSongMap();
  MusicPlayer player(library_map);
  player.Run();
  return 0;
}
