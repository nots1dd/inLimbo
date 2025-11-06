#pragma once

#include "Logger.hpp"
#include "core/RBTree.hpp"
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

inline void iterateDirectory(
    const std::string& dirPath,
    const std::function<void(const std::string& fullPath,
                             const struct stat& fileStat)>& onEntry)
{
    RECORD_FUNC_TO_BACKTRACE("IterateDirectory");

    DIR* dir = opendir(dirPath.c_str());
    if (!dir)
    {
        LOG_ERROR("Could not open directory: {}", dirPath);
        return; // Do not exit here — allow caller to decide
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        std::string fullPath = dirPath + "/" + entry->d_name;
        struct stat fileStat{};
        if (stat(fullPath.c_str(), &fileStat) != 0)
        {
            LOG_WARN("Could not stat file: {}!", fullPath);
            continue;
        }

        // Invoke callback for every valid entry
        onEntry(fullPath, fileStat);
    }

    closedir(dir);
}

inline void processDirectory(const std::string& dirPath,
                             dirsort::RedBlackTree<ino_t, dirsort::Song>& rbt,
                             InodeFileMapper& mapper)
{
    RECORD_FUNC_TO_BACKTRACE("ProcessDirectory");

    iterateDirectory(dirPath, [&](const std::string& fullPath, const struct stat& fileStat)
    {
        if (S_ISREG(fileStat.st_mode))
        {
            rbt.insert(fileStat.st_ino);
            mapper.addMapping(fileStat.st_ino, fullPath, true);
        }
        else
        {
            LOG_DEBUG("Skipping non-regular file: {}", fullPath);
        }
    });
}
