#pragma once

#include "Config.hpp"
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

namespace core
{

class INLIMBO_API_CPP InodeFileMapper
{
private:
  unordered_map<ino_t, string> m_inodeToPath;
  ofstream                     m_syncFile;

public:
  void addMapping(ino_t inode, const string& filePath, bool writeSyncFile)
  {
    if (m_inodeToPath.find(inode) == m_inodeToPath.end())
    {
      m_inodeToPath[inode] = filePath;
      if (writeSyncFile && m_syncFile.is_open())
      {
        m_syncFile << inode << endl;
        m_syncFile << filePath << endl;
      }
    }
  }

  void printMappings() const
  {
    for (const auto& pair : m_inodeToPath)
    {
      cout << "Inode: " << pair.first << " -> " << pair.second << endl;
    }
  }
};

} // namespace core
