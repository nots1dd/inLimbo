#include "core/InodeMapper.hpp"
#include "toml/Parser.hpp"
#include "Logger.hpp"
#include "utils/signal/SignalHandler.hpp"
#include "utils/timer/Timer.hpp"

auto main(int argc, char* argv[]) -> int
{
    RECORD_FUNC_TO_BACKTRACE("MAIN");
    utils::SignalHandler::getInstance().setup();

    const std::string directoryPath = string(parser::parseTOMLField("library", "directory"));
    const std::string libBinPath    = utils::getConfigPath(LIB_BIN_NAME);

    LOG_INFO("Configured music directory: {}", directoryPath);

    util::Timer<> timer;
    dirsort::RedBlackTree<ino_t, dirsort::Song> rbt;
    InodeFileMapper mapper;
    dirsort::SongTree song_tree;

    // Try to load the cache; if invalid or missing, rebuild it
    bool needRebuild = false;
    try {
        song_tree.loadFromFile(libBinPath);

        const std::string cachedPath = song_tree.returnMusicPath();
        if (cachedPath != directoryPath) {
            LOG_WARN("Cached directory '{}' differs from configured '{}'. Rebuilding...",
                     cachedPath, directoryPath);
            needRebuild = true;
        } else {
            LOG_INFO("Loaded song tree from cache (directory: {}).", cachedPath);
        }
    }
    catch (const std::exception& e) {
        LOG_ERROR("Failed to load cache: {}. Rebuilding...", e.what());
        needRebuild = true;
    }

    // Define parsing callback once
    rbt.setVisitCallback([&song_tree](const ino_t& inode, dirsort::Song& song) {
        TagLibParser parser(dirsort::DEBUG_LOG_PARSE);
        auto metadataMap = parser.parseFromInode(inode, dirsort::DIRECTORY_FIELD);

        if (metadataMap.empty()) {
            LOG_WARN("No metadata found for inode {}: {}", inode, song.metadata.filePath);
            return;
        }

        for (const auto& [filename, metadata] : metadataMap)
            song_tree.addSong(dirsort::Song(inode, metadata));
    });

    if (needRebuild) {

        song_tree.clear();

        processDirectory(directoryPath, rbt, mapper);
        rbt.inorder(); // triggers parsing callback
        song_tree.setMusicPath(directoryPath);
        song_tree.saveToFile(libBinPath);
        LOG_INFO("Saved updated song tree to cache: {}", libBinPath);
    }

    auto library_map = song_tree.returnSongMap();

    song_tree.display(dirsort::DisplayMode::Summary);

    LOG_INFO("Inode mapping and parsing completed in {:.2f} ms", timer.elapsed_ms());
    return 0;
}
