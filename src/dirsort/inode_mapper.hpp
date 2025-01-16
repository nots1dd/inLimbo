#ifndef INODE_MAPPER_HPP
#define INODE_MAPPER_HPP

#include "rbtree.hpp"
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

#define LIB_SYNC_NAME "lib.sync"
#define LIB_BIN_NAME  "lib.bin"

class InodeFileMapper
{
private:
  unordered_map<ino_t, string> inodeToPath;
  ofstream                     syncFile;

public:
  InodeFileMapper(const string& syncFileName, string useCacheFile)
  {
    if (useCacheFile == "true")
    {
      syncFile.open(syncFileName, std::ios::app);
    }
    else
    {
      syncFile.open(syncFileName);
    }

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

    string      fullPath = dirPath + "/" + entry->d_name;
    struct stat fileStat;
    if (stat(fullPath.c_str(), &fileStat) == 0)
    {
      rbt.insert(fileStat.st_ino);
      mapper.addMapping(fileStat.st_ino, fullPath, true);
    }
    else
    {
      cerr << "Warning: Could not stat file " << fullPath << endl;
    }
  }

  closedir(dir);
}

void processCacheFile(const std::string& cacheFilePath, RedBlackTree& rbt, InodeFileMapper& mapper)
{
  // Open file
  int fd = open(cacheFilePath.c_str(), O_RDONLY);
  if (fd == -1)
  {
    throw std::runtime_error("Could not open cache file " + cacheFilePath);
  }

  // Get file size
  struct stat sb;
  if (fstat(fd, &sb) == -1)
  {
    close(fd);
    throw std::runtime_error("Could not get file size");
  }

  // Memory map the file
  const char* data =
    static_cast<const char*>(mmap(nullptr, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0));

  if (data == MAP_FAILED)
  {
    close(fd);
    throw std::runtime_error("Could not memory map file");
  }

  const char* current = data;
  const char* end     = data + sb.st_size;

  while (current < end)
  {
    // Find end of inode line
    const char* lineEnd = static_cast<const char*>(memchr(current, '\n', end - current));
    if (!lineEnd)
      break;

    // Parse inode
    ino_t       inode      = 0;
    const char* inodeStart = current;
    while (current < lineEnd && *current >= '0' && *current <= '9')
    {
      inode = inode * 10 + (*current - '0');
      current++;
    }

    // Skip newline
    current = lineEnd + 1;
    if (current >= end)
      break;

    // Find end of filepath line
    lineEnd = static_cast<const char*>(memchr(current, '\n', end - current));
    if (!lineEnd)
      lineEnd = end;

    // Get filepath
    std::string filePath(current, lineEnd - current);

    if (inode > 0 && !filePath.empty())
    {
      rbt.insert(inode);
      mapper.addMapping(inode, std::move(filePath), false);
    }

    current = lineEnd + 1;
  }

  // Cleanup
  munmap(const_cast<char*>(data), sb.st_size);
  close(fd);
}

#endif // INODE_MAPPER_HPP
