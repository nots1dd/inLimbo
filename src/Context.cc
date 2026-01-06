#include "Context.hpp"
#include "Logger.hpp"
#include "core/InodeMapper.hpp"
#include "frontend/cmd-line/Interface.hpp"
#include "helpers/cmdline/Display.hpp"
#include "helpers/fs/Directory.hpp"
#include "mpris/backends/CmdLine.hpp"
#include "query/SongMap.hpp"
#include "thread/Map.hpp"
#include "toml/Parser.hpp"
#include "utils/PathResolve.hpp"
#include "utils/RBTree.hpp"

#include <algorithm>
#include <cstdlib>

using namespace core;

namespace inlimbo
{

// clang-format off
void setupArgs(cli::CmdLine& args)
{
  args.add<Title>(
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

  args.add<Title>(
    "Query",
    "print-song",
    'E',
    "Print song info and exit",
    std::nullopt,
    [](const Title& s) -> bool {
      return !s.empty();
    },
    "Song name defaults to null."
  );

  args.add<Artist>(
    "Query",
    "songs-artist",
    'a',
    "Print all songs by artist name",
    std::nullopt,
    [](const Artist& s) -> bool {
      return !s.empty();
    },
    "Artist name defaults to null."
  );

  args.add<Album>(
    "Query",
    "songs-album",
    'l',
    "Print all songs by album name",
    std::nullopt,
    [](const Album& s) -> bool {
      return !s.empty();
    },
    "Album name defaults to null."
  );

  args.add<Genre>(
    "Query",
    "songs-genre",
    'g',
    "Print all songs by genre name",
    std::nullopt,
    [](const Genre& s) -> bool {
      return !s.empty();
    },
    "Genre name defaults to null."
  );

  args.add<Album>(
    "Query",
    "discs-album",
    'd',
    "Print all discs in an album and exit",
    std::nullopt,
    [](const Album& s) -> bool {
      return !s.empty();
    },
    "Album name defaults to null."
  );
}
// clang-format on

// ------------------------------------------------------------

AppContext::AppContext(const std::string& program, const std::string& description)
    : m_cmdLine(program, description),
      m_debugLogTagLibField(tomlparser::Config::getString("debug", "taglib_parser_log")),
      m_tagLibParser({.debugLog = m_debugLogTagLibField == "true"})
{
  setupArgs(m_cmdLine);
}

// ------------------------------------------------------------

auto resolvePrintAction(const cli::CmdLine& args) -> PrintAction
{
  if (args.has("print-artists"))
    return PrintAction::Artists;
  if (args.has("print-song"))
    return PrintAction::SongInfo;
  if (args.has("print-albums"))
    return PrintAction::Albums;
  if (args.has("print-genre"))
    return PrintAction::Genres;
  if (args.has("discs-album"))
    return PrintAction::DiscsInAlbum;
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

// ------------------------------------------------------------

void maybeHandlePrintActions(AppContext& ctx)
{
  if (ctx.m_printAction == PrintAction::None)
    return;

  switch (ctx.m_printAction)
  {
    case PrintAction::Artists:
      helpers::cmdline::printArtists(ctx.m_songTree);
      break;
    case PrintAction::SongInfo:
      helpers::cmdline::printSongInfo(ctx.m_songTree,
                                      ctx.m_cmdLine.getOptional<Title>("print-song").value_or(""));
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
      helpers::cmdline::printSongsByArtist(
        ctx.m_songTree, ctx.m_cmdLine.getOptional<Artist>("songs-artist").value_or(""));
      break;
    case PrintAction::SongsByAlbum:
      helpers::cmdline::printSongsByAlbum(
        ctx.m_songTree, ctx.m_cmdLine.getOptional<Album>("songs-album").value_or(""));
      break;
    case PrintAction::SongsByGenre:
      helpers::cmdline::printSongsByGenre(
        ctx.m_songTree, ctx.m_cmdLine.getOptional<Genre>("songs-genre").value_or(""));
      break;
    case PrintAction::DiscsInAlbum:
      helpers::cmdline::printDiscsInAlbum(
        ctx.m_songTree, ctx.m_cmdLine.getOptional<Album>("discs-album").value_or(""));
      break;
    default:
      break;
  }

  LOG_INFO("Print action completed. Exiting.");
  std::exit(EXIT_SUCCESS);
}

auto initializeContext(int argc, char** argv) -> AppContext
{
  AppContext ctx("inLimbo", "inLimbo (CMD-LINE) music library tool");

  ctx.m_cmdLine.parse(argc, argv);
  ctx.m_songName     = ctx.m_cmdLine.getOptional<std::string>("song").value_or("");
  ctx.m_editMetadata = ctx.m_cmdLine.has("edit-metadata");

  const float vol = ctx.m_cmdLine.getOptional<float>("volume").value_or(75.0f);

  ctx.m_volume      = std::clamp(vol / 100.0f, 0.0f, 1.5f);
  ctx.m_printAction = resolvePrintAction(ctx.m_cmdLine);
  ctx.m_musicDir    = tomlparser::Config::getString("library", "directory");
  ctx.m_binPath     = utils::getConfigPathWithFile(LIB_BIN_NAME).c_str();

  LOG_INFO("Configured directory: {}, song query: {}", ctx.m_musicDir, ctx.m_songName);

  return ctx;
}

void buildOrLoadLibrary(AppContext& ctx)
{
  bool rebuild = ctx.m_cmdLine.has("rebuild-library");

  try
  {
    ctx.m_songTree.loadFromFile(ctx.m_binPath);
    if (ctx.m_songTree.returnMusicPath() != ctx.m_musicDir)
      rebuild = true;
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

  helpers::fs::dirWalkProcessAll(ctx.m_musicDir, rbt, mapper, ctx.m_tagLibParser, ctx.m_songTree);

  ctx.m_songTree.setMusicPath(ctx.m_musicDir);
  ctx.m_songTree.saveToFile(ctx.m_binPath);
  g_songMap.replace(ctx.m_songTree.returnSongMap());

  LOG_INFO("Library rebuilt in {:.3f} ms", ctx.m_timer.elapsed_ms());
}

void maybeEditMetadata(AppContext& ctx)
{
  if (!ctx.m_editMetadata)
    return;

  auto song = query::songmap::read::findSongByName(g_songMap, ctx.m_songName);

  ASSERT_MSG(song, "Song not found");

  const auto newTitleOpt = ctx.m_cmdLine.getOptional<std::string>("edit-metadata");

  if (!newTitleOpt || newTitleOpt->empty())
    return;

  auto edited           = *song;
  edited.metadata.title = *newTitleOpt;

  const bool replaced = query::songmap::mut::replaceSongObjAndUpdateMetadata(
    g_songMap, *song, edited, ctx.m_tagLibParser);

  if (!replaced)
    LOG_CRITICAL("Failed to update metadata for song: {}", song->metadata.title);

  ctx.m_songTree.clear();
  ctx.m_songTree.newSongMap(g_songMap.snapshot());
  ctx.m_songTree.saveToFile(ctx.m_binPath);

  std::exit(EXIT_SUCCESS);
}

void runFrontend(AppContext& ctx)
{
  auto song = query::songmap::read::findSongByName(g_songMap, ctx.m_songName);

  ASSERT_MSG(song, "Song not found");

  // ---------------------------------------------------------
  // Create AudioService (owns AudioEngine)
  // ---------------------------------------------------------
  audio::Service audio;

  mpris::cmdline::Backend mprisBackend(audio);
  mpris::Service          mprisService(mprisBackend, "inLimbo-cmdline");

  // ---------------------------------------------------------
  // Enumerate playback devices (optional UI selection)
  // ---------------------------------------------------------
  auto devices = audio.enumeratePlaybackDevices();

  frontend::cmdline::Interface ui(g_songMap, &mprisService);

  // ---------------------------------------------------------
  // Initialize backend
  // ---------------------------------------------------------
  audio.initDevice(); // default device
  audio.setVolume(ctx.m_volume);

  // ---------------------------------------------------------
  // Register track + add to playlist (NO decoding here)
  // ---------------------------------------------------------
  audio::service::SoundHandle handle = audio.registerTrack(*song);

  ASSERT_MSG(handle, "Failed to register track");

  audio.addToPlaylist(handle);

  // ---------------------------------------------------------
  // Start playback (lazy load happens here)
  // ---------------------------------------------------------
  audio.start();

  // ---------------------------------------------------------
  // Run UI loop
  // ---------------------------------------------------------
  ui.run(audio);

  // ---------------------------------------------------------
  // Clean shutdown
  // ---------------------------------------------------------
  audio.shutdown();
}

} // namespace inlimbo
