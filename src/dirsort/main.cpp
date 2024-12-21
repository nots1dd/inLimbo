#include "rbtree.cpp"
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sys/stat.h>
#include <unordered_map>
#include <vector>

using namespace std;

class InodeFileMapper
{
private:
  unordered_map<ino_t, string> inodeToPath;
  ofstream                     syncFile;

public:
  InodeFileMapper(const string& syncFileName)
  {
    syncFile.open(syncFileName);
    if (!syncFile.is_open())
    {
      cerr << "Error: Could not open file " << syncFileName << endl;
      exit(EXIT_FAILURE);
    }
  }

  ~InodeFileMapper()
  {
    if (syncFile.is_open())
    {
      syncFile.close();
    }
  }

  void addMapping(ino_t inode, const string& filePath)
  {
    if (inodeToPath.find(inode) == inodeToPath.end())
    {
      inodeToPath[inode] = filePath;
      syncFile << filePath << endl;
    }
  }
};

void processDirectory(const string& dirPath, RedBlackTree& rbt, InodeFileMapper& mapper)
{
  DIR* dir = opendir(dirPath.c_str());
  if (!dir)
  {
    cerr << "Error: Could not open directory " << dirPath << endl;
    exit(EXIT_FAILURE);
  }

  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr)
  {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
    {
      continue;
    }

    string fullPath = dirPath + "/" + entry->d_name;
    rbt.insert(entry->d_ino);
    mapper.addMapping(entry->d_ino, fullPath);
  }

  closedir(dir);
}

int main(int argc, char* argv[])
{
  if (argc != 2)
  {
    cerr << "Usage: " << argv[0] << " <directory_path>" << endl;
    return EXIT_FAILURE;
  }

  string          directoryPath = argv[1];
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
