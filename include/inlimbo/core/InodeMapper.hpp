#pragma once

#include "Logger.hpp"
#include "RBTree.hpp"
#include "core/SongTree.hpp"
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>

using namespace std;

#define LIB_BIN_NAME "lib.bin"

class InodeFileMapper
{
private:
  unordered_map<ino_t, string> inodeToPath;
  ofstream                     syncFile;

public:
  void addMapping(ino_t inode, const string& filePath, bool writeSyncFile)
  {
    if (inodeToPath.find(inode) == inodeToPath.end())
    {
      inodeToPath[inode] = filePath;
      if (writeSyncFile && syncFile.is_open())
      {
        syncFile << inode << endl;
        syncFile << filePath << endl;
      }
    }
  }

  void printMappings() const
  {
    for (const auto& pair : inodeToPath)
    {
      cout << "Inode: " << pair.first << " -> " << pair.second << endl;
    }
  }
};

void processDirectory(const string& dirPath, dirsort::RedBlackTree<ino_t, dirsort::Song>& rbt, InodeFileMapper& mapper)
{
  DIR* dir = opendir(dirPath.c_str());
  if (!dir)
  {
    LOG_ERROR("Could not open directory: {}", dirPath);
    exit(EXIT_FAILURE);
  }

  struct dirent* entry;
  while ((entry = readdir(dir)) != nullptr)
  {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
    {
      continue;
    }

    const string      fullPath = dirPath + "/" + entry->d_name;
    struct stat fileStat;
    if (stat(fullPath.c_str(), &fileStat) == 0)
    {
      rbt.insert(fileStat.st_ino);
      mapper.addMapping(fileStat.st_ino, fullPath, true);
    }
    else
    {
      LOG_WARN("Could not stat file: {}!", fullPath);
    }
  }

  closedir(dir);
}
