#include "Context.hpp"
#include "Logger.hpp"
#include "core/InodeMapper.hpp"
#include "frontend/Interface.hpp"
#include "helpers/cmdline/Display.hpp"
#include "helpers/fs/Directory.hpp"
#include "mpris/Service.hpp"
#include "mpris/backends/CmdLine.hpp"
#include "query/SongMap.hpp"
#include "thread/Map.hpp"
#include "toml/Parser.hpp"
#include "utils/PathResolve.hpp"
#include "utils/RBTree.hpp"
#include "utils/unix/Lockfile.hpp"

#include <algorithm>

using namespace core;

namespace inlimbo
{

static void addHelpAndVersion(CLI::App& app)
{
  app.set_help_flag("-h,--help", "Show help");

  app.set_version_flag("-v,--version",
                       std::string("Version : ") + INLIMBO_VERSION_STR + "\n" + "Commit  : " +
                         INLIMBO_GIT_COMMIT_HASH + "\n" + "Build ID: " + INLIMBO_BUILD_ID + "\n" +
                         "Branch  : " + INLIMBO_GIT_BRANCH + "\n" + "Dirty   : " +
                         (INLIMBO_GIT_DIRTY ? std::string("yes") : std::string("no")) + "\n",
                       "Show version information");
}

// clang-format off
void setupArgs(CLI::App& app, Args& args)
{
  addHelpAndVersion(app);

  app.add_option("-s,--song", args.song, "Play song name (fuzzy search)")
    ->check([](const Title& s) -> Title {
      if (s.empty()) return "Song name cannot be empty!";
      return {};
    });

  app.add_option("-f,--frontend", args.frontend, "Frontend name to plugin and play")
    ->check([](const frontend::PluginNameStr& s) -> frontend::PluginNameStr {
      if (s.empty()) return "Frontend plugin name cannot be empty";
      return {};
    });

  app.add_flag("-l,--list-frontend", args.listFrontend,
                    "List all built frontend plugin names");

  app.add_option("-V,--volume", args.volume, "Initial playback volume (0-100)")
    ->check(CLI::Range(0.0f, 100.0f))
    ->default_val("75");

  auto* edit = app.add_subcommand("edit", "Edit metadata");
  edit->add_option("-t,--edit-title", args.editTitle, "Edit title in metadata")
    ->check([](const Title& s) -> Title {
      if (s.empty()) return "Metadata edit requires a non-empty title";
      return {};
    });

  edit->add_option("-a,--edit-artist", args.editArtist, "Edit artist in metadata")
    ->check([](const Artist& s) -> Artist {
      if (s.empty()) return "Metadata edit requires a non-empty artist";
      return {};
    });

  edit->add_option("-b,--edit-album", args.editAlbum, "Edit album in metadata")
    ->check([](const Album& s) -> Album {
      if (s.empty()) return "Metadata edit requires a non-empty album";
      return {};
    });

  edit->add_option("-g,--edit-genre", args.editGenre, "Edit genre in metadata")
    ->check([](const Genre& s) -> Genre {
      if (s.empty()) return "Metadata edit requires a non-empty genre";
      return {};
    });

  edit->add_option("-y,--edit-lyrics", args.editLyrics, "Edit lyrics in metadata")
    ->check([](const Lyrics& s) -> Lyrics {
      if (s.empty()) return "Metadata edit requires non-empty lyrics";
      return {};
    });

  auto* modify = app.add_subcommand("modify", "Modify library");
  modify->add_flag("-r,--rebuild-library", args.rebuildLibrary,
                   "Force rebuild of the music library cache");

  auto* query = app.add_subcommand("query", "Query and print library information");

  query->add_flag("-z,--fuzzy-search", args.fuzzySearch,
                    "Enable fuzzy search on song queries");
  query->add_flag("-a,--print-artists", args.printArtists, "Print all artists and exit");
  query->add_flag("-b,--print-albums",  args.printAlbums,  "Print all albums and exit");
  query->add_flag("-g,--print-genre",   args.printGenre,   "Print all genres and exit");
  query->add_flag("-s,--print-summary", args.printSummary, "Print library summary and exit");
  query->add_flag("-p,--print-song-paths",   args.songsPaths,   "Print all song paths and exit");

  query->add_option("-i,--print-song-info", args.printSong, "Print song info and exit")
    ->check([](const Title& s) -> Title {
      if (s.empty()) return "Song name cannot be empty";
      return {};
    });

  query->add_option("-L,--print-song-lyrics", args.printLyrics, "Print song lyrics and exit")
    ->check([](const Title& s) -> Title {
      if (s.empty()) return "Song name cannot be empty";
      return {};
    });

  query->add_option("-A,--print-songs-by-artist", args.songsArtist, "Print all songs by artist name")
    ->check([](const Artist& s) -> Artist {
      if (s.empty()) return "Artist name cannot be empty";
      return {};
    });

  query->add_option("-B,--print-songs-by-album", args.songsAlbum, "Print all songs by album name")
    ->check([](const Album& s) -> Album {
      if (s.empty()) return "Album name cannot be empty";
      return {};
    });

  query->add_option("-G,--print-songs-by-genre", args.songsGenre, "Print all songs by genre name")
    ->check([](const Genre& s) -> Genre {
      if (s.empty()) return "Genre name cannot be empty";
      return {};
    });
}
// clang-format on

// ------------------------------------------------------------

AppContext::AppContext(CLI::App& cliApp)
    : m_taglibDbgLog(tomlparser::Config::getBool("debug", "taglib_parser_log")),
      m_tagLibParser({.debugLog = m_taglibDbgLog})
{
  setupArgs(cliApp, args);
}

// ------------------------------------------------------------

auto resolvePrintAction(const Args& args) -> PrintAction
{
  if (args.listFrontend)
    return PrintAction::Frontends;

  if (args.printArtists)
    return PrintAction::Artists;

  // "print-song" is an option, so check string non-empty
  if (!args.printSong.empty())
    return PrintAction::SongInfoByTitle;

  if (!args.printLyrics.empty())
    return PrintAction::Lyrics;

  if (args.printAlbums)
    return PrintAction::Albums;

  if (args.printGenre)
    return PrintAction::Genres;

  if (args.printSummary)
    return PrintAction::Summary;

  if (args.songsPaths)
    return PrintAction::SongPaths;

  if (!args.songsArtist.empty())
    return PrintAction::SongsByArtist;

  if (!args.songsAlbum.empty())
    return PrintAction::SongsByAlbum;

  if (!args.songsGenre.empty())
    return PrintAction::SongsByGenre;

  return PrintAction::None;
}

auto resolveEditAction(const Args& args) -> EditAction
{
  if (!args.editTitle.empty())
    return EditAction::Title;

  if (!args.editArtist.empty())
    return EditAction::Artist;

  if (!args.editAlbum.empty())
    return EditAction::Album;

  if (!args.editGenre.empty())
    return EditAction::Genre;

  if (!args.editLyrics.empty())
    return EditAction::Lyrics;

  return EditAction::None;
}

// ------------------------------------------------------------

auto maybeHandlePrintActions(AppContext& ctx) -> bool
{
  if (ctx.m_printAction == PrintAction::None)
    return false;

  const bool useFuzzySearch = ctx.args.fuzzySearch;

  LOG_DEBUG("Using fuzzy max distance as: {}", ctx.m_fuzzyMaxDist);

  auto logFuzzyFallback = [&](std::string_view type, const std::string& input,
                              const std::string& best) -> void
  {
    if (!input.empty() && !best.empty() && input != best)
    {
      LOG_WARN("No exact {} match for '{}' -> using fuzzy match '{}'", type, input, best);
    }
  };

  auto bestForSongTitle = [&](const Title& query) -> Title
  {
    if (!useFuzzySearch)
      return query;

    const Song* song =
      query::songmap::read::findSongByTitleFuzzy(g_songMap, query, ctx.m_fuzzyMaxDist);
    if (!song)
      return {};

    const auto best = song->metadata.title;
    logFuzzyFallback("song", query, best);
    return best;
  };

  auto bestForArtist = [&](const Artist& query) -> Artist
  {
    if (!useFuzzySearch)
      return query;

    const auto best = query::songmap::read::findArtistFuzzy(g_songMap, query, ctx.m_fuzzyMaxDist);
    logFuzzyFallback("artist", query, best);
    return best;
  };

  auto bestForAlbum = [&](const Album& query) -> Album
  {
    if (!useFuzzySearch)
      return query;

    const auto best = query::songmap::read::findAlbumFuzzy(g_songMap, query, ctx.m_fuzzyMaxDist);
    logFuzzyFallback("album", query, best);
    return best;
  };

  auto bestForGenre = [&](const Genre& query) -> Genre
  {
    if (!useFuzzySearch)
      return query;

    const auto best = query::songmap::read::findGenreFuzzy(g_songMap, query, ctx.m_fuzzyMaxDist);
    logFuzzyFallback("genre", query, best);
    return best;
  };

  switch (ctx.m_printAction)
  {
    case PrintAction::Frontends:
      helpers::cmdline::printFrontendPlugins();
      break;

    case PrintAction::Artists:
      helpers::cmdline::printArtists(g_songMap);
      break;

    case PrintAction::SongInfoByTitle:
    {
      const auto input = ctx.args.printSong;
      const auto best  = bestForSongTitle(input);
      helpers::cmdline::printSongInfoByTitle(g_songMap, best);
      break;
    }

    case PrintAction::Lyrics:
    {
      const auto input = ctx.args.printLyrics;
      const auto best  = bestForSongTitle(input);
      helpers::cmdline::printSongLyrics(g_songMap, best);
      break;
    }

    case PrintAction::Albums:
      helpers::cmdline::printAlbums(g_songMap);
      break;

    case PrintAction::Genres:
      helpers::cmdline::printGenres(g_songMap);
      break;

    case PrintAction::Summary:
      helpers::cmdline::printSummary(g_songMap);
      break;

    case PrintAction::SongPaths:
      helpers::cmdline::printSongPaths(g_songMap);
      break;

    case PrintAction::SongsByArtist:
    {
      const auto input = ctx.args.songsArtist;
      const auto best  = bestForArtist(input);
      helpers::cmdline::printSongsByArtist(g_songMap, best);
      break;
    }

    case PrintAction::SongsByAlbum:
    {
      const auto input = ctx.args.songsAlbum;
      const auto best  = bestForAlbum(input);
      helpers::cmdline::printSongsByAlbum(g_songMap, best);
      break;
    }

    case PrintAction::SongsByGenre:
    {
      const auto input = ctx.args.songsGenre;
      const auto best  = bestForGenre(input);
      helpers::cmdline::printSongsByGenre(g_songMap, best);
      break;
    }

    default:
      break;
  }

  LOG_INFO("Print action completed. Exiting.");
  return true;
}

// true to exit app, false to continue
auto maybeHandleEditActions(AppContext& ctx) -> bool
{
  if (ctx.m_editAction == EditAction::None)
    return false;

  if (ctx.m_songTitle.empty())
  {
    LOG_ERROR("No song title provided. Exiting...");
    return true;
  }

  const Song* song = query::songmap::read::findSongByTitle(g_songMap, ctx.m_songTitle);
  if (!song)
  {
    LOG_ERROR("Song not found: '{}'", ctx.m_songTitle);
    return true;
  }

  Song edited  = *song;
  bool touched = false;

  auto apply = [&](const std::string& value, auto Metadata::*field) -> void
  {
    if (!value.empty())
    {
      edited.metadata.*field = value;
      touched                = true;
    }
  };

  // chain edits (multiple at once)
  apply(ctx.args.editTitle, &Metadata::title);
  apply(ctx.args.editArtist, &Metadata::artist);
  apply(ctx.args.editAlbum, &Metadata::album);
  apply(ctx.args.editGenre, &Metadata::genre);
  apply(ctx.args.editLyrics, &Metadata::lyrics);

  if (!touched)
  {
    LOG_WARN("No edit options provided. Nothing to update.");
    return true;
  }

  if (!query::songmap::mut::replaceSongObjAndUpdateMetadata(g_songMap, *song, edited,
                                                            ctx.m_tagLibParser))
  {
    LOG_CRITICAL("Failed to update metadata for song: {}", song->metadata.title);
    return true;
  }

  // Persist updated SongMap to disk
  SongTree tempSongTree;
  tempSongTree.clear();
  tempSongTree.newSongMap(g_songMap.snapshot());
  tempSongTree.saveToFile(ctx.m_binPath);

  LOG_INFO("Metadata updated successfully. Exiting.");
  return true;
}

auto initializeContext(int argc, char** argv) -> AppContext
{
  CLI::App   cliApp{"inLimbo: Music Player tool", "inLimbo"};
  AppContext ctx(cliApp);

  try
  {
    cliApp.parse(argc, argv);
  }
  catch (const CLI::ParseError& e)
  {
    std::exit(cliApp.exit(e));
  }

  ctx.m_songTitle    = ctx.args.song;
  ctx.m_fuzzyMaxDist = tomlparser::Config::getInt("fuzzy", "max_dist");
  ctx.m_fePluginName = frontend::PluginName{ctx.args.frontend};

  const float vol = ctx.args.volume;

  ctx.m_volume      = std::clamp(vol / 100.0f, 0.0f, 1.5f);
  ctx.m_printAction = resolvePrintAction(ctx.args);
  ctx.m_editAction  = resolveEditAction(ctx.args);
  ctx.m_musicDir    = tomlparser::Config::getString("library", "directory");
  ctx.m_binPath     = utils::getAppConfigPathWithFile(LIB_BIN_NAME).c_str();

  LOG_INFO("Configured directory: {}, Song title query given: '{}'", ctx.m_musicDir,
           ctx.m_songTitle);

  return ctx;
}

void buildOrLoadLibrary(AppContext& ctx)
{
  SongTree tempSongTree;
  bool     rebuild = ctx.args.rebuildLibrary;

  try
  {
    tempSongTree.loadFromFile(ctx.m_binPath);
    if (tempSongTree.returnMusicPath() != ctx.m_musicDir)
      rebuild = true;
  }
  catch (...)
  {
    rebuild = true;
  }

  if (!rebuild)
  {
    g_songMap.replace(tempSongTree.moveSongMap());
    return;
  }

  ctx.m_timer.restart();
  tempSongTree.clear();

  utils::RedBlackTree<ino_t, utils::rbt::NilNode> rbt;
  core::InodeFileMapper                           mapper;

  helpers::fs::dirWalkProcessAll(ctx.m_musicDir, rbt, mapper, ctx.m_tagLibParser, tempSongTree);

  tempSongTree.setMusicPath(ctx.m_musicDir);
  tempSongTree.saveToFile(ctx.m_binPath);
  g_songMap.replace(tempSongTree.moveSongMap());

  // SongTree has destructor so mem shud clear here
  LOG_INFO("Library rebuilt in {:.3f} ms", ctx.m_timer.elapsed_ms());
}

void runFrontend(AppContext& ctx)
{
  // preliminary check for frontend plugin name
  if (ctx.m_fePluginName.empty())
  {
    LOG_ERROR("Plugin name found to be empty!!");
    helpers::cmdline::printFrontendPlugins();
    return;
  }

  // in the frontend only we care about locking the application so we run lock here, not in main
  // file.

  try
  {
    utils::unix::LockFile appLock(INLIMBO_DEFAULT_LOCKFILE_PATH);
    auto song = query::songmap::read::findSongByTitleFuzzy(g_songMap, ctx.m_songTitle);

    if (!song)
    {
      LOG_ERROR("Song not found: '{}'", ctx.m_songTitle);
      return;
    }

    LOG_INFO("Fuzzy search song title query returned: '{}'", song->metadata.title);

    // ---------------------------------------------------------
    // Create AudioService (owns AudioEngine)
    // ---------------------------------------------------------
    audio::Service audio(g_songMap);

    mpris::cmdline::Backend mprisBackend(audio);
    mpris::Service          mprisService(mprisBackend, "inLimbo");

    // ---------------------------------------------------------
    // Initialize abstract frontend interface
    // ---------------------------------------------------------
    frontend::Plugin    fePlugin(ctx.m_fePluginName);
    frontend::Interface ui(fePlugin, g_songMap, &mprisService);

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
  catch (const utils::unix::LockFileAlreadyLocked&)
  {
    LOG_ERROR("Another instance of inLimbo is already running.");
  }
  catch (const utils::unix::LockFileError& e)
  {
    LOG_ERROR("LockFileError: {}", e.what());
  }
  catch (const frontend::PluginError& e)
  {
    LOG_ERROR("Frontend error: {}", e.what());
  }
  catch (std::exception& e)
  {
    LOG_ERROR("Something bad happened: {}", e.what());
  }
}

} // namespace inlimbo
