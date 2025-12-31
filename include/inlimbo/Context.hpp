#pragma once

#include "CmdLine.hpp"
#include "audio/Playback.hpp"
#include "core/InodeMapper.hpp"
#include "core/RBTree.hpp"
#include "core/SongTree.hpp"
#include "core/taglib/Parser.hpp"
#include "frontend/cmd-line/CMD-LINE.hpp"
#include "helpers/Directory.hpp"
#include "helpers/cmdline/Display.hpp"
#include "helpers/query/SongMap.hpp"
#include "thread/Map.hpp"
#include "toml/Parser.hpp"
#include "utils/timer/Timer.hpp"
#include <string>

using namespace core;

namespace inlimbo 
{

inline void setupArgs(cli::CmdLine& args)
{
    args.add<std::string>(
        "General",
        "song", 's',
        "Play song name",
        std::nullopt,
        [](const std::string& s) -> bool { return !s.empty(); },
        "song name cannot be empty, if not provided defaults to null!" 
    );

    args.add<float>(
        "General",
        "volume", 'v',
        "Initial playback volume (0-100)",
        75.0f,
        [](float vol) -> bool { return vol >= 0.0f && vol <= 100.0f; },
        "Volume must be between 0.0 and 100.0"
    );

    args.add<std::string>(
        "Modify",
        "edit-metadata", 'e',
        "Edit metadata (title only for now) before playback",
        std::nullopt,
        [](const std::string& s) -> bool { return !s.empty(); },
        "Metadata edit requires a non-empty song name, will default to null"
    );

    args.addFlag(
      "General",
      "rebuild-library", 'r',
      "Force rebuild of the music library cache"
    );

    args.addFlag(
        "Query",
        "print-artists", 'A',
        "Print all artists and exit"
    );

    args.addFlag(
        "Query",
        "print-albums", 'L',
        "Print all albums and exit"
    );

    args.addFlag(
        "Query",
        "print-genre", 'G',
        "Print all genres and exit"
    );

    args.addFlag(
        "Query",
        "print-summary", 'S',
        "Print library summary and exit"
    );

    args.addFlag(
        "Query",
        "songs-paths", 'P',
        "Print all song paths and exit"
    );

    args.add<std::string>(
        "Query",
        "songs-artist", 'a',
        "Print all songs by artist name",
        std::nullopt,
        [](const std::string& s) -> bool { return !s.empty(); },
        "Artist name defaults to null."
    );

    args.add<std::string>(
      "Query",
      "songs-album", 'l',
      "Print all songs by album name",
      std::nullopt,
      [](const std::string& s) -> bool { return !s.empty(); },
      "Album name defaults to null."
    );

    args.add<std::string>(
    "Query",
    "songs-genre", 'g',
    "Print all songs by genre name",
    std::nullopt,
    [](const std::string& s) -> bool { return !s.empty(); },
    "Genre name defaults to null."
    );

}

enum class PrintAction {
    None,
    Artists,
    Albums,
    Genres,
    SongPaths,
    SongsByArtist,
    SongsByAlbum,
    SongsByGenre,
    Summary
};

struct AppContext {

    explicit AppContext() = delete;
    AppContext(const std::string& program,
               const std::string& description)
        : cmdlineArgs(program, description),
          parser(dirsort::DEBUG_LOG_PARSE)
    {
        setupArgs(cmdlineArgs);
    }

    // cmdline parser 
    cli::CmdLine cmdlineArgs;
    
    // CLI
    std::string songName = {};
    bool editMetadata = false;
    float volume      = 1.0f;

    PrintAction printAction = PrintAction::None;

    // Paths
    std::string musicDir = {};
    std::string binPath = {};

    // Core objects
    TagLibParser parser;
    core::SongTree songTree = {};
    util::Timer<> timer = {};
};

inline auto resolvePrintAction(const cli::CmdLine& args) -> PrintAction
{
    if (args.has("print-artists")) return PrintAction::Artists;
    if (args.has("print-albums"))  return PrintAction::Albums;
    if (args.has("print-genre"))   return PrintAction::Genres;
    if (args.has("print-songs"))   return PrintAction::SongPaths;
    if (args.has("print-songs-by-artist")) return PrintAction::SongsByArtist;
    if (args.has("print-songs-by-album"))  return PrintAction::SongsByAlbum;
    if (args.has("print-songs-by-genre"))  return PrintAction::SongsByGenre;
    if (args.has("print-summary")) return PrintAction::Summary;
    return PrintAction::None;
}

inline void maybeHandlePrintActions(AppContext& ctx)
{ 
    if (ctx.printAction == PrintAction::None)
        return;

    switch (ctx.printAction) {
        case PrintAction::Artists:
            helpers::cmdline::printArtists(ctx.songTree);
            break;
        case PrintAction::Albums:
            helpers::cmdline::printAlbums(ctx.songTree);
            break;
        case PrintAction::Genres:
            helpers::cmdline::printGenres(ctx.songTree);
            break;
        case PrintAction::SongPaths:
            helpers::cmdline::printSongPaths(ctx.songTree);
            break;
        case PrintAction::SongsByArtist: {
            const std::string artistName =
                ctx.cmdlineArgs.getOptional<std::string>("print-songs-by-artist")
                .value_or("");
            helpers::cmdline::printSongsByArtist(ctx.songTree, artistName);
            break;
        }
        case PrintAction::SongsByAlbum: {
            const std::string albumName =
                ctx.cmdlineArgs.getOptional<std::string>("print-songs-by-album")
                .value_or("");
            helpers::cmdline::printSongsByAlbum(ctx.songTree, albumName);
            break;
        }
        case PrintAction::SongsByGenre: {
            const std::string genreName =
                ctx.cmdlineArgs.getOptional<std::string>("print-songs-by-genre")
                .value_or("");
            helpers::cmdline::printSongsByGenre(ctx.songTree, genreName);
            break;
        }
        case PrintAction::Summary:
            helpers::cmdline::printSummary(ctx.songTree);
            break;
        default:
            break;
    }

    LOG_INFO("Print action completed. Exiting.");
    std::exit(EXIT_SUCCESS);
}

inline auto initializeContext(int argc, char** argv) -> AppContext
{
    AppContext ctx("inLimbo", "inLimbo (CMD-LINE) music library tool");

    ctx.cmdlineArgs.parse(argc, argv);

    ctx.songName     = ctx.cmdlineArgs.getOptional<std::string>("song").value_or("");
    ctx.editMetadata = ctx.cmdlineArgs.has("edit-metadata");

    const float vol =
        ctx.cmdlineArgs.getOptional<float>("volume").value_or(75.0f);
    ctx.volume = std::clamp(vol / 100.0f, 0.0f, 1.5f);

    ctx.printAction = resolvePrintAction(ctx.cmdlineArgs);

    ctx.musicDir = parser::parseTOMLField("library", "directory");
    ctx.binPath  = utils::getConfigPath(LIB_BIN_NAME);

    LOG_INFO("Configured directory: {}, song query: {}",
             ctx.musicDir, ctx.songName);

    return ctx;
}

inline static void buildOrLoadLibrary(AppContext& ctx)
{
    bool rebuild = false;
    rebuild = ctx.cmdlineArgs.has("rebuild-library");

    try {
        ctx.songTree.loadFromFile(ctx.binPath);
        if (ctx.songTree.returnMusicPath() != ctx.musicDir) {
            LOG_WARN("Cached directory mismatch, rebuilding...");
            rebuild = true;
        }
    }
    catch (...) {
        rebuild = true;
    }

    if (!rebuild) {
        g_songMap.replace(ctx.songTree.returnSongMap());
        return;
    }

    ctx.timer.restart();
    ctx.songTree.clear();

    dirsort::RedBlackTree<ino_t, core::Song> rbt;
    core::InodeFileMapper mapper;

    rbt.setVisitCallback([&](const ino_t& inode, core::Song&) -> void {
        for (auto& [_, md] : ctx.parser.parseFromInode(inode, dirsort::DIRECTORY_FIELD))
            ctx.songTree.addSong(core::Song(inode, md));
    });

    helpers::processDirectory(ctx.musicDir, rbt, mapper);
    rbt.inorder();

    ctx.songTree.setMusicPath(ctx.musicDir);
    ctx.songTree.saveToFile(ctx.binPath);

    g_songMap.replace(ctx.songTree.returnSongMap());

    LOG_INFO("Library rebuilt in {:.2f} ms", ctx.timer.elapsed_ms());
}

inline static void maybeEditMetadata(AppContext& ctx)
{
    if (!ctx.editMetadata)
        return;

    auto song = helpers::query::songmap::read::findSongByName(g_songMap, ctx.songName);
    ASSERT_MSG(song, "Song not found");

    const std::string oldTitle = song->metadata.title;

    const auto newTitleOpt = ctx.cmdlineArgs.getOptional<std::string>("edit-metadata");
    if (!newTitleOpt || newTitleOpt->empty()) {
        LOG_WARN("Edit metadata requested but no new title provided. Skipping.");
        return;
    }

    const std::string& newTitle = *newTitleOpt;

    if (oldTitle == newTitle) {
        LOG_INFO("Metadata edit requested but title is unchanged: '{}'", oldTitle);
        return;
    }

    auto edited = *song;
    edited.metadata.title = newTitle;

    LOG_INFO(
        "Editing song metadata:\n"
        "  • File : {}\n"
        "  • Title: '{}' → '{}'",
        song->metadata.filePath,
        oldTitle,
        newTitle
    );

    const bool replaced =
        helpers::query::songmap::mut::replaceSongObjAndUpdateMetadata(
            g_songMap, *song, edited, ctx.parser
        );

    if (!replaced) {
        LOG_ERROR(
            "Failed to update metadata for file: {}",
            song->metadata.filePath
        );
        return;
    }

    // Rebuild SongTree + persist cache
    ctx.songTree.clear();
    ctx.songTree.newSongMap(g_songMap.snapshot());
    ctx.songTree.setMusicPath(ctx.musicDir);
    ctx.songTree.saveToFile(ctx.binPath);

    LOG_INFO(
        "Metadata update successful. Cache rewritten: '{}' → '{}'",
        oldTitle,
        newTitle
    );

    LOG_INFO("Exiting cleanly after metadata edit.");

    // need to exit here as in-memory song object name is wrong
    std::exit(EXIT_SUCCESS);
}

inline static void runFrontend(AppContext& ctx)
{
    auto song = helpers::query::songmap::read::findSongByName(g_songMap, ctx.songName);
    ASSERT_MSG(song, "Song not found");

    audio::AudioEngine engine;
    auto devices = engine.enumeratePlaybackDevices();
    ASSERT_MSG(!devices.empty(), "No audio devices found");

    // initialize the frontend
    //
    // NOTE!! THE SONG MAP WILL BECOME EMPTY AFTER THIS CALL!!!!
    //
    // future access points are all within the Interface object
    frontend::cmdline::Interface ui(g_songMap);
    const size_t devIdx = ui.selectAudioDevice(devices);

    engine.initEngineForDevice(devices[devIdx].name);
    engine.setVolume(ctx.volume);

    const auto idx = *engine.loadSound(song->metadata.filePath);
    engine.restart();
    ui.run(engine, song->metadata);
    engine.stop();
}

}
