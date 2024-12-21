#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <dirent.h>
#include <sys/stat.h>
#include <cstring>
#include <fstream>
#include <unordered_map>
#include "rbtree.cpp"

using namespace std;

class InodeFileMapper {
private:
    unordered_map<ino_t, string> inodeToPath;
    ofstream syncFile;

public:
    InodeFileMapper(const string& syncFileName)
    {
        syncFile.open(syncFileName);
        if (!syncFile.is_open()) {
            cerr << "Error: Could not open file " << syncFileName << endl;
            exit(EXIT_FAILURE);
        }
    }

    ~InodeFileMapper()
    {
        if (syncFile.is_open()) {
            syncFile.close();
        }
    }

    void addMapping(ino_t inode, const string& filePath)
    {
        if (inodeToPath.find(inode) == inodeToPath.end()) {
            inodeToPath[inode] = filePath;
            syncFile << filePath << endl;
        }
    }
};

void processDirectory(const string& dirPath, RedBlackTree& rbt, InodeFileMapper& mapper)
{
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {
        cerr << "Error: Could not open directory " << dirPath << endl;
        exit(EXIT_FAILURE);
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
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
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <directory_path>" << endl;
        return EXIT_FAILURE;
    }

    string directoryPath = argv[1];
    RedBlackTree rbt;
    InodeFileMapper mapper("lib.sync");

    auto start = chrono::high_resolution_clock::now();
    processDirectory(directoryPath, rbt, mapper);
    cout << "Inorder traversal of inodes: ";
    rbt.inorderPrintMetadata();
    cout << endl;
    auto end = chrono::high_resolution_clock::now();


    cout << "Inode insertion, mapping and parsing time: "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;


    return 0;
}
