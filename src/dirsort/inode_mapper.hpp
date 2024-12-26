#ifndef INODE_MAPPER_HPP
#define INODE_MAPPER_HPP

#include "rbtree.hpp"

using namespace std;

#define LIB_SYNC_PATH "lib.sync"

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

#endif
