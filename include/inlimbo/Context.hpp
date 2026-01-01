#pragma once

#include "CmdLine.hpp"
#include "audio/Playback.hpp"
#include "core/InodeMapper.hpp"
#include "core/SongTree.hpp"
#include "core/taglib/Parser.hpp"
#include "frontend/cmd-line/CMD-LINE.hpp"
#include "helpers/Directory.hpp"
#include "helpers/cmdline/Display.hpp"
#include "helpers/query/SongMap.hpp"
#include "thread/Map.hpp"
#include "toml/Parser.hpp"
#include "utils/RBTree.hpp"
#include "utils/timer/Timer.hpp"
#include <string>

using namespace core;

namespace inlimbo
{

// clang-format off
inline void setupArgs(cli::CmdLine& args)
{
  args.add<std::string>(
    "General",
    "song",
    's',
    "Play song name",
    std::nullopt,
    [](const std::string& s) -> bool {
      return !s.empty();
    },
    "song name cannot be empty, if not provided defaults to null!"
  );

  args.add<float>(
    "General",
    "volume",
    'v',
    "Initial playback volume (0-100)",
    75.0f,
    [](float vol) -> bool {
      return vol >= 0.0f && vol <= 100.0f;
    },
    "Volume must be between 0.0 and 100.0"
  );

  args.add<std::string>(
    "Modify",
    "edit-metadata",
    'e',
    "Edit metadata (title only for now) before playback",
    std::nullopt,
    [](const std::string& s) -> bool {
      return !s.empty();
    },
    "Metadata edit requires a non-empty song name, will default to null"
  );

  args.addFlag(
    "Modify",
    "rebuild-library",
    'r',
    "Force rebuild of the music library cache"
  );

  args.addFlag(
    "Query",
    "print-artists",
    'A',
    "Print all artists and exit"
  );

  args.addFlag(
    "Query",
    "print-albums",
    'L',
    "Print all albums and exit"
  );

  args.addFlag(
    "Query",
    "print-genre",
    'G',
    "Print all genres and exit"
  );

  args.addFlag(
    "Query",
    "print-summary",
    'S',
    "Print library summary and exit"
  );

  args.addFlag(
    "Query",
    "songs-paths",
    'P',
    "Print all song paths and exit"
  );

  args.add<std::string>(
    "Query",
    "songs-artist",
    'a',
    "Print all songs by artist name",
    std::nullopt,
    [](const std::string& s) -> bool {
      return !s.empty();
    },
    "Artist name defaults to null."
  );

  args.add<std::string>(
    "Query",
    "songs-album",
    'l',
    "Print all songs by album name",
    std::nullopt,
    [](const std::string& s) -> bool {
      return !s.empty();
    },
    "Album name defaults to null."
  );

  args.add<std::string>(
    "Query",
    "songs-genre",
    'g',
    "Print all songs by genre name",
    std::nullopt,
    [](const std::string& s) -> bool {
      return !s.empty();
    },
    "Genre name defaults to null."
  );
}
// clang-format on

enum class PrintAction
{
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

struct AppContext
{

  explicit AppContext() = delete;
  AppContext(const std::string& program, const std::string& description)
      : m_cmdLine(program, description),
        m_debugLogTagLibField(parser::parseTOMLField("debug", "taglib_parser_log")),
        m_tagLibParser(m_debugLogTagLibField)
  {
    setupArgs(m_cmdLine);
  }

  // cmdline parser
  cli::CmdLine m_cmdLine;

  // CLI
  std::string m_songName            = {};
  std::string m_debugLogTagLibField = {};
  bool        m_editMetadata        = false;
  float       m_volume              = {};

  PrintAction m_printAction = PrintAction::None;

  // Paths
  std::string m_musicDir = {};
  std::string m_binPath  = {};

  // Core objects
  TagLibParser   m_tagLibParser;
  core::SongTree m_songTree = {};
  utils::Timer<> m_timer    = {};
};

inline auto resolvePrintAction(const cli::CmdLine& args) -> PrintAction
{
  if (args.has("print-artists"))
    return PrintAction::Artists;
  if (args.has("print-albums"))
    return PrintAction::Albums;
  if (args.has("print-genre"))
    return PrintAction::Genres;
  if (args.has("print-summary"))
    return PrintAction::Summary;
  if (args.has("songs-paths"))
    return PrintAction::SongPaths;
  if (args.has("songs-artist"))
    return PrintAction::SongsByArtist;
  if (args.has("songs-album"))
    return PrintAction::SongsByAlbum;
  if (args.has("songs-genre"))
    return PrintAction::SongsByGenre;
  return PrintAction::None;
}

inline void maybeHandlePrintActions(AppContext& ctx)
{
  if (ctx.m_printAction == PrintAction::None)
    return;

  switch (ctx.m_printAction)
  {
    case PrintAction::Artists:
      helpers::cmdline::printArtists(ctx.m_songTree);
      break;
    case PrintAction::Albums:
      helpers::cmdline::printAlbums(ctx.m_songTree);
      break;
    case PrintAction::Genres:
      helpers::cmdline::printGenres(ctx.m_songTree);
      break;
    case PrintAction::Summary:
      helpers::cmdline::printSummary(ctx.m_songTree);
      break;
    case PrintAction::SongPaths:
      helpers::cmdline::printSongPaths(ctx.m_songTree);
      break;
    case PrintAction::SongsByArtist:
    {
      const std::string artistName =
        ctx.m_cmdLine.getOptional<std::string>("songs-artist").value_or("");
      helpers::cmdline::printSongsByArtist(ctx.m_songTree, artistName);
      break;
    }
    case PrintAction::SongsByAlbum:
    {
      const std::string albumName =
        ctx.m_cmdLine.getOptional<std::string>("songs-album").value_or("");
      helpers::cmdline::printSongsByAlbum(ctx.m_songTree, albumName);
      break;
    }
    case PrintAction::SongsByGenre:
    {
      const std::string genreName =
        ctx.m_cmdLine.getOptional<std::string>("songs-genre").value_or("");
      helpers::cmdline::printSongsByGenre(ctx.m_songTree, genreName);
      break;
    }
    default:
      break;
  }

  LOG_INFO("Print action completed. Exiting.");
  std::exit(EXIT_SUCCESS);
}

inline auto initializeContext(int argc, char** argv) -> AppContext
{
  AppContext ctx("inLimbo", "inLimbo (CMD-LINE) music library tool");

  ctx.m_cmdLine.parse(argc, argv);

  ctx.m_songName     = ctx.m_cmdLine.getOptional<std::string>("song").value_or("");
  ctx.m_editMetadata = ctx.m_cmdLine.has("edit-metadata");

  const float vol = ctx.m_cmdLine.getOptional<float>("volume").value_or(75.0f);
  ctx.m_volume    = std::clamp(vol / 100.0f, 0.0f, 1.5f);

  ctx.m_printAction = resolvePrintAction(ctx.m_cmdLine);

  ctx.m_musicDir = parser::parseTOMLField("library", "directory");
  ctx.m_binPath  = utils::getConfigPath(LIB_BIN_NAME);

  LOG_INFO("Configured directory: {}, song query: {}", ctx.m_musicDir, ctx.m_songName);

  return ctx;
}

inline static void buildOrLoadLibrary(AppContext& ctx)
{
  bool rebuild = false;
  rebuild      = ctx.m_cmdLine.has("rebuild-library");

  try
  {
    ctx.m_songTree.loadFromFile(ctx.m_binPath);
    if (ctx.m_songTree.returnMusicPath() != ctx.m_musicDir)
    {
      LOG_ERROR("Cached directory mismatch, rebuilding...");
      rebuild = true;
    }
  }
  catch (...)
  {
    rebuild = true;
  }

  if (!rebuild)
  {
    g_songMap.replace(ctx.m_songTree.returnSongMap());
    return;
  }

  ctx.m_timer.restart();
  ctx.m_songTree.clear();

  utils::RedBlackTree<ino_t, utils::rbt::NilNode> rbt;
  core::InodeFileMapper                           mapper;

  helpers::processDirectory(ctx.m_musicDir, rbt, mapper);

  for (const auto [inode] : rbt)
  {
    for (auto& [_, md] : ctx.m_tagLibParser.parseFromInode(inode, ctx.m_musicDir))
    {
      ctx.m_songTree.addSong(core::Song{inode, md});
    }
  }

  ctx.m_songTree.setMusicPath(ctx.m_musicDir);
  ctx.m_songTree.saveToFile(ctx.m_binPath);

  g_songMap.replace(ctx.m_songTree.returnSongMap());

  LOG_INFO("Library rebuilt in {:.3f} ms", ctx.m_timer.elapsed_ms());
}

inline static void maybeEditMetadata(AppContext& ctx)
{
  if (!ctx.m_editMetadata)
    return;

  auto song = helpers::query::songmap::read::findSongByName(g_songMap, ctx.m_songName);
  ASSERT_MSG(song, "Song not found");

  const std::string oldTitle = song->metadata.title;

  const auto newTitleOpt = ctx.m_cmdLine.getOptional<std::string>("edit-metadata");
  if (!newTitleOpt || newTitleOpt->empty())
  {
    LOG_WARN("Edit metadata requested but no new title provided. Skipping.");
    return;
  }

  const std::string& newTitle = *newTitleOpt;

  if (oldTitle == newTitle)
  {
    LOG_INFO("Metadata edit requested but title is unchanged: '{}'", oldTitle);
    return;
  }

  auto edited           = *song;
  edited.metadata.title = newTitle;

  LOG_INFO("Editing song metadata:\n"
           "  • File : {}\n"
           "  • Title: '{}' → '{}'",
           song->metadata.filePath, oldTitle, newTitle);

  const bool replaced = helpers::query::songmap::mut::replaceSongObjAndUpdateMetadata(
    g_songMap, *song, edited, ctx.m_tagLibParser);

  if (!replaced)
  {
    LOG_ERROR("Failed to update metadata for file: {}", song->metadata.filePath);
    return;
  }

  // Rebuild SongTree + persist cache
  ctx.m_songTree.clear();
  ctx.m_songTree.newSongMap(g_songMap.snapshot());
  ctx.m_songTree.setMusicPath(ctx.m_musicDir);
  ctx.m_songTree.saveToFile(ctx.m_binPath);

  LOG_INFO("Metadata update successful. Cache rewritten: '{}' → '{}'", oldTitle, newTitle);

  LOG_INFO("Exiting cleanly after metadata edit.");

  // need to exit here as in-memory song object name is wrong
  std::exit(EXIT_SUCCESS);
}

inline static void runFrontend(AppContext& ctx)
{
  auto song = helpers::query::songmap::read::findSongByName(g_songMap, ctx.m_songName);
  ASSERT_MSG(song, "Song not found");

  audio::AudioEngine engine;
  auto               devices = engine.enumeratePlaybackDevices();
  ASSERT_MSG(!devices.empty(), "No audio devices found");

  // initialize the frontend
  //
  // NOTE!! THE SONG MAP WILL BECOME EMPTY AFTER THIS CALL!!!!
  //
  // future access points are all within the Interface object
  frontend::cmdline::Interface ui(g_songMap);
  const size_t                 devIdx = ui.selectAudioDevice(devices);

  engine.initEngineForDevice(devices[devIdx].name);
  engine.setVolume(ctx.m_volume);

  const auto idx = *engine.loadSound(song->metadata.filePath);
  engine.restart();
  ui.run(engine, song->metadata);
  engine.stop();
}

} // namespace inlimbo
