#include "Context.hpp"
#include "Logger.hpp"
#include "config/sort/Model.hpp"
#include "frontend/Interface.hpp"
#include "helpers/cmdline/Display.hpp"
#include "helpers/fs/Directory.hpp"
#include "helpers/fuzzy/Search.hpp"
#include "lrc/Client.hpp"
#include "mpris/Service.hpp"
#include "mpris/backends/Common.hpp"
#include "query/SongMap.hpp"
#include "thread/Map.hpp"
#include "toml/Parser.hpp"
#include "utils/fs/Paths.hpp"
#include "utils/timer/Timer.hpp"
#include "utils/unix/Lockfile.hpp"

#include <algorithm>

threads::SafeMap<SongMap> g_songMap;

using namespace core;
namespace fuzzy = helpers::fuzzy;

namespace inlimbo
{

static auto buildVersionString() -> std::string
{
  std::ostringstream oss;

  oss << "Version  : " << INLIMBO_VERSION_STR << "\n";
  oss << "Commit   : " << INLIMBO_GIT_COMMIT_HASH << "\n";
  oss << "Build ID : " << INLIMBO_BUILD_ID << "\n";
  oss << "Branch   : " << INLIMBO_GIT_BRANCH << "\n";
  oss << "Dirty    : " << (INLIMBO_GIT_DIRTY ? "yes" : "no") << "\n";
  oss << "Compiler : " << INLIMBO_COMPILER_NAME << " " << INLIMBO_COMPILER_VERSION_FULL_STR << "\n";

  return oss.str();
}

static void addHelpAndVersion(CLI::App& app)
{
  app.set_help_flag("-h,--help", "Show help");

  app.set_version_flag("-v,--version", buildVersionString(), "Show version information");
}

// clang-format off
void setupArgs(CLI::App& app, Args& args)
{
    addHelpAndVersion(app);

#define INLIMBO_ADD_OPTION(appObj, argsObj, name, cli, desc) \
    do { \
        (appObj).add_option(cli, (argsObj).name, desc); \
    } while(0);

#define INLIMBO_ADD_FLAG(appObj, argsObj, name, cli, desc) \
    do { \
        (appObj).add_flag(cli, (argsObj).name, desc); \
    } while(0);

#define INLIMBO_ADD_OPTIONAL(appObj, argsObj, name, cli, desc) \
    do { \
        auto* _opt = (appObj).add_option(cli, (argsObj).name, desc); \
        _opt->expected(0,1); \
    } while(0);

#define ARG(name, type, kind, cli, desc, action) \
    INLIMBO_ADD_##kind(app, args, name, cli, desc)

#define OPTIONAL_ARG(name, type, cli, desc, action) \
    INLIMBO_ADD_OPTIONAL(app, args, name, cli, desc)

#include "defs/args/General.def"

#undef ARG
#undef OPTIONAL_ARG

    [[maybe_unused]] auto* edit =
        app.add_subcommand("edit", "Edit audio metadata fields (tags) via TagLib");

#define ARG(name, type, kind, cli, desc, action) \
    INLIMBO_ADD_##kind((*edit), args, name, cli, desc)

#define OPTIONAL_ARG(name, type, cli, desc, action) \
    INLIMBO_ADD_OPTIONAL((*edit), args, name, cli, desc)

#include "defs/args/Edit.def"

#undef ARG
#undef OPTIONAL_ARG

    [[maybe_unused]] auto* modify =
        app.add_subcommand("modify", "Modify library settings and cache behaviour");

#define ARG(name, type, kind, cli, desc, action) \
    INLIMBO_ADD_##kind((*modify), args, name, cli, desc)

#define OPTIONAL_ARG(name, type, cli, desc, action) \
    INLIMBO_ADD_OPTIONAL((*modify), args, name, cli, desc)

#include "defs/args/Modify.def"

#undef ARG
#undef OPTIONAL_ARG

    [[maybe_unused]] auto* query =
        app.add_subcommand("query", "Query library and telemetry data");

#define ARG(name, type, kind, cli, desc, action) \
    INLIMBO_ADD_##kind((*query), args, name, cli, desc)

#define OPTIONAL_ARG(name, type, cli, desc, action) \
    INLIMBO_ADD_OPTIONAL((*query), args, name, cli, desc)

#include "defs/args/Query.def"

#undef ARG
#undef OPTIONAL_ARG

#undef INLIMBO_ADD_OPTION
#undef INLIMBO_ADD_FLAG
#undef INLIMBO_ADD_OPTIONAL
}
// clang-format on

AppContext::AppContext(CLI::App& cliApp)
    : m_taglibDbgLog(tomlparser::Config::getBool("debug", "taglib_parser_log")),
      m_telemetryCtx(tomlparser::Config::getInt("telemetry", "min_playback_event_time", 30)),
      m_tagLibParser({.debugLog = m_taglibDbgLog})
{
  setupArgs(cliApp, args);
}

auto resolvePrintAction(const Args& a) -> PrintAction
{

#define CHECK_PRINT_None(a, name)

#define CHECK_PRINT_Frontends(a, name) \
  if (a.name)                          \
    return PrintAction::Frontends;

#define CHECK_PRINT_AudioBackends(a, name) \
  if (a.name)                              \
    return PrintAction::AudioBackends;

#define CHECK_PRINT_AudioDevs(a, name) \
  if (a.name)                          \
    return PrintAction::AudioDevs;

#define CHECK_PRINT_Artists(a, name) \
  if (a.name)                        \
    return PrintAction::Artists;

#define CHECK_PRINT_Albums(a, name) \
  if (a.name)                       \
    return PrintAction::Albums;

#define CHECK_PRINT_SongTree(a, name) \
  if (a.name)                         \
    return PrintAction::SongTree;

#define CHECK_PRINT_Genres(a, name) \
  if (a.name)                       \
    return PrintAction::Genres;

#define CHECK_PRINT_Summary(a, name) \
  if (a.name)                        \
    return PrintAction::Summary;

#define CHECK_PRINT_SongPaths(a, name) \
  if (a.name)                          \
    return PrintAction::SongPaths;

#define CHECK_PRINT_SongInfoByTitle(a, name) \
  if (!a.name.empty())                       \
    return PrintAction::SongInfoByTitle;

#define CHECK_PRINT_Lyrics(a, name) \
  if (!a.name.empty())              \
    return PrintAction::Lyrics;

#define CHECK_PRINT_SongsByArtist(a, name) \
  if (!a.name.empty())                     \
    return PrintAction::SongsByArtist;

#define CHECK_PRINT_SongsByAlbum(a, name) \
  if (!a.name.empty())                    \
    return PrintAction::SongsByAlbum;

#define CHECK_PRINT_SongsByGenre(a, name) \
  if (!a.name.empty())                    \
    return PrintAction::SongsByGenre;

#define ARG(name, type, kind, cli, desc, action) CHECK_PRINT_##action(a, name)
#define OPTIONAL_ARG(name, type, cli, desc, action) \
  if (a.name.has_value())                           \
    return PrintAction::action;

#include "defs/args/General.def"
#include "defs/args/Query.def"

#undef ARG
#undef OPTIONAL_ARG

  return PrintAction::None;
}

auto resolveEditAction(const Args& a) -> EditAction
{

#define CHECK_EDIT_None(a, name)

#define CHECK_EDIT_Title(a, name) \
  if (!a.name.empty())            \
    return EditAction::Title;

#define CHECK_EDIT_Artist(a, name) \
  if (!a.name.empty())             \
    return EditAction::Artist;

#define CHECK_EDIT_Album(a, name) \
  if (!a.name.empty())            \
    return EditAction::Album;

#define CHECK_EDIT_Genre(a, name) \
  if (!a.name.empty())            \
    return EditAction::Genre;

#define CHECK_EDIT_Lyrics(a, name) \
  if (!a.name.empty())             \
    return EditAction::Lyrics;

#define CHECK_EDIT_FetchLyrics(a, name) \
  if (a.name)                           \
    return EditAction::FetchLyrics;

#define ARG(name, type, kind, cli, desc, action) CHECK_EDIT_##action(a, name)
#define OPTIONAL_ARG(name, type, cli, desc, action) \
  if (a.name.has_value())                           \
    return EditAction::action;

#include "defs/args/Edit.def"

#undef ARG
#undef OPTIONAL_ARG

  return EditAction::None;
}

auto maybeHandlePrintActions(AppContext& ctx) -> bool
{
  if (ctx.m_printAction == PrintAction::None)
    return false;

  const bool useFuzzySearch = ctx.args.fuzzySearch;

  LOG_DEBUG("Using fuzzy max distance as: {}", ctx.m_fuzzyMaxDist);

  switch (ctx.m_printAction)
  {
    case PrintAction::Frontends:
      helpers::cmdline::printFrontendPlugins();
      break;

    case PrintAction::AudioBackends:
    {
      helpers::cmdline::printAudioBackends(audio::enumerateBackends());
      break;
    }

    case PrintAction::AudioDevs:
    {
      audio::Service tempService(g_songMap, ctx.m_audioBackendName);
      auto           devs = tempService.enumerateDevices();
      helpers::cmdline::printAudioDevices(devs);
      break;
    }

    case PrintAction::Artists:
      helpers::cmdline::printArtists(g_songMap);
      break;

    case PrintAction::SongInfoByTitle:
    {
      helpers::fuzzy::fuzzyDispatch<fuzzy::FuzzyKind::Title>(
        g_songMap, ctx.args.printSong, ctx.m_fuzzyMaxDist, useFuzzySearch,
        [&](const auto& best) { helpers::cmdline::printSongInfoByTitle(g_songMap, best); });
      break;
    }

    case PrintAction::Lyrics:
    {
      helpers::fuzzy::fuzzyDispatch<fuzzy::FuzzyKind::Title>(
        g_songMap, ctx.args.printLyrics, ctx.m_fuzzyMaxDist, useFuzzySearch,
        [&](const auto& best) { helpers::cmdline::printSongLyrics(g_songMap, best); });
      break;
    }

    case PrintAction::Albums:
    {
      helpers::fuzzy::fuzzyDispatch<fuzzy::FuzzyKind::Artist>(
        g_songMap, ctx.args.printAlbums, ctx.m_fuzzyMaxDist, useFuzzySearch,
        [&](const auto& best) { helpers::cmdline::printAlbums(g_songMap, best); });
      break;
    }

    case PrintAction::SongTree:
    {
      helpers::fuzzy::fuzzyDispatch<fuzzy::FuzzyKind::Artist>(
        g_songMap, ctx.args.printSongTree, ctx.m_fuzzyMaxDist, useFuzzySearch,
        [&](const auto& best) { helpers::cmdline::printSongTree(g_songMap, best); });
      break;
    }

    case PrintAction::Genres:
    {
      helpers::fuzzy::fuzzyDispatch<fuzzy::FuzzyKind::Artist>(
        g_songMap, ctx.args.printGenres, ctx.m_fuzzyMaxDist, useFuzzySearch,
        [&](const auto& best) { helpers::cmdline::printGenres(g_songMap, best); });
      break;
    }

    case PrintAction::Summary:
      helpers::cmdline::printSummary(g_songMap, ctx.m_telemetryCtx);
      break;

    case PrintAction::SongPaths:
      helpers::cmdline::printSongPaths(g_songMap);
      break;

    case PrintAction::SongsByArtist:
    {
      helpers::fuzzy::fuzzyDispatch<fuzzy::FuzzyKind::Artist>(
        g_songMap, ctx.args.songsArtist, ctx.m_fuzzyMaxDist, useFuzzySearch,
        [&](const auto& best) { helpers::cmdline::printSongsByArtist(g_songMap, best); });
      break;
    }

    case PrintAction::SongsByAlbum:
    {
      helpers::fuzzy::fuzzyDispatch<fuzzy::FuzzyKind::Album>(
        g_songMap, ctx.args.songsAlbum, ctx.m_fuzzyMaxDist, useFuzzySearch,
        [&](const auto& best) { helpers::cmdline::printSongsByAlbum(g_songMap, best); });
      break;
    }

    case PrintAction::SongsByGenre:
    {
      helpers::fuzzy::fuzzyDispatch<fuzzy::FuzzyKind::Genre>(
        g_songMap, ctx.args.songsGenre, ctx.m_fuzzyMaxDist, useFuzzySearch,
        [&](const auto& best) { helpers::cmdline::printSongsByGenre(g_songMap, best); });
      break;
    }

    default:
      break;
  }

  LOG_DEBUG("Print action completed. Exiting app...");
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

  auto song = query::songmap::read::findSongObjByTitleFuzzy(g_songMap, ctx.m_songTitle);
  if (!song)
  {
    LOG_ERROR("Song not found: '{}'", ctx.m_songTitle);
    return true;
  }

  auto edited  = std::make_shared<Song>(*song);
  bool touched = false;

  auto apply = [&](const std::string& value, auto Metadata::* field) -> void
  {
    if (!value.empty())
    {
      edited->metadata.*field = value;
      touched                 = true;
    }
  };

  // chain edits (multiple at once)
  apply(ctx.args.editTitle, &Metadata::title);
  apply(ctx.args.editArtist, &Metadata::artist);
  apply(ctx.args.editAlbum, &Metadata::album);
  apply(ctx.args.editGenre, &Metadata::genre);

  // lyrics can be fetched from lrclib as well via fetchLyricsMode
  switch (ctx.m_editAction)
  {
    case EditAction::FetchLyrics:
    {
      LOG_INFO("Fetching lyrics from LRCLIB...");

      lrc::Client lrcClient;

      lrc::Query q;
      q.artist = song->metadata.artist;
      q.track  = song->metadata.title;
      q.album  = song->metadata.album;

      auto res = lrcClient.fetchBestMatchAndCache(q);

      if (!res.ok())
      {
        LOG_ERROR("Failed to fetch lyrics: {}", res.error.message);
        return true;
      }

      auto& [lyrics, path] = res.value;

      if (!lyrics.plainLyrics)
      {
        LOG_WARN("No plain lyrics returned from LRCLIB");
        return true;
      }

      edited->metadata.lyrics = *lyrics.plainLyrics;
      touched                 = true;

      LOG_INFO("Lyrics fetched successfully from LRCLIB and cached to file path: '{}'", path);
      break;
    }

    case EditAction::Lyrics:
    {
      apply(ctx.args.editLyrics, &Metadata::lyrics);
      break;
    }

    default:
      break;
  }

  if (!touched)
  {
    LOG_WARN("No edit options provided. Nothing to update.");
    return true;
  }

  if (!query::songmap::mut::replaceSongObjAndUpdateMetadata(g_songMap, song, edited,
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

  LOG_DEBUG("Song object metadata updated successfully. Exiting app...");
  return true;
}

auto initializeContext(int argc, char** argv) -> AppContext
{
  tomlparser::Config::load();

  CLI::App   cliApp{APP_DESC, APP_NAME};
  AppContext ctx(cliApp);

  try
  {
    cliApp.parse(argc, argv);
  }
  catch (const CLI::ParseError& e)
  {
    std::exit(cliApp.exit(e));
  }

  if (ctx.args.deleteTelemetry)
  {
    try
    {
      std::filesystem::remove(
        utils::fs::getAppConfigPathWithFile(INLIMBO_DEFAULT_TELEMETRY_BIN_NAME));
      std::filesystem::remove(
        utils::fs::getAppConfigPathWithFile(INLIMBO_DEFAULT_TELEMETRY_REGISTRY_BIN_NAME));
      LOG_INFO("Deleted all telemetry cache.");
    }
    catch (std::exception& e)
    {
      LOG_ERROR("Error while deleting telemetry cache: {}", e.what());
    }
  }

  // ---------------------------------------------------------
  // Create or load telemetry service
  // ---------------------------------------------------------
  ctx.m_telemetryCtx.isStoreLoaded = ctx.m_telemetryCtx.store.load(
    utils::fs::getAppConfigPathWithFile(INLIMBO_DEFAULT_TELEMETRY_BIN_NAME));
  ctx.m_telemetryCtx.isRegistryLoaded = ctx.m_telemetryCtx.registry.load(
    utils::fs::getAppConfigPathWithFile(INLIMBO_DEFAULT_TELEMETRY_REGISTRY_BIN_NAME));
  if (ctx.m_telemetryCtx.isStoreLoaded && ctx.m_telemetryCtx.isRegistryLoaded)
    LOG_INFO("Successfully fetched telemetry registry and store.");
  else
    LOG_ERROR("Something went wrong when loading telemetry data (registry or store)!");

  ctx.m_songTitle        = ctx.args.song;
  ctx.m_fuzzyMaxDist     = tomlparser::Config::getInt("fuzzy", "max_dist");
  ctx.m_audioBackendName = tomlparser::Config::getString("audio", "backend", "alsa");
  ctx.m_fePluginName     = PluginName{ctx.args.frontend};

  float vol = ctx.args.volume;

  if (vol < 0.0 || vol > 150.0)
  {
    LOG_DEBUG("Invalid volume found in arguments, fetching default from config...");
    vol = tomlparser::Config::getInt("audio", "volume", 75);
  }

  // volume fetched from user is (0-150) range but is normalized to 0-1.5
  // in the audio backend.
  ctx.m_volume      = std::clamp(vol / 100.0f, 0.0f, 1.5f);
  ctx.m_printAction = resolvePrintAction(ctx.args);
  ctx.m_editAction  = resolveEditAction(ctx.args);
  ctx.m_musicDir    = tomlparser::Config::getString("library", "directory");
  ctx.m_binPath     = utils::fs::getAppConfigPathWithFile(INLIMBO_DEFAULT_CACHE_BIN_NAME).c_str();

  LOG_INFO(
    "Initialized inLimbo context for directory: '{}', Playback song title query provided: '{}'",
    ctx.m_musicDir, ctx.m_songTitle);

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
    LOG_INFO("No song map rebuild. Loading song map and sorting...");
    g_songMap.replace(tempSongTree.moveSongMap());
    const auto plan = config::sort::loadRuntimeSortPlan();
    query::songmap::mut::sortSongMap(g_songMap, plan);
    return;
  }

  utils::Timer<> timer;
  timer.start();
  tempSongTree.clear();

  helpers::fs::dirWalkProcessAll(ctx.m_musicDir, ctx.m_tagLibParser, tempSongTree);

  tempSongTree.setMusicPath(ctx.m_musicDir);
  g_songMap.replace(tempSongTree.moveSongMap());
  // the song map is unordered so lets sort it
  //
  // note that by default, we are only sorting this following:
  //
  // -> artists in ascending order (lexicographically),
  // -> albums in ascending order (lexicographically),
  // -> tracks in ascending order (numerically)
  //
  // as that is most logical.
  //
  // We can change this sort logic via config.toml
  // (check out examples/config/config.toml for more!)
  const auto plan = config::sort::loadRuntimeSortPlan();
  query::songmap::mut::sortSongMap(g_songMap, plan);

  // now let us save the newly sorted song map to disk
  tempSongTree.newSongMap(g_songMap.snapshot());
  tempSongTree.saveToFile(ctx.m_binPath);

  // SongTree has destructor so mem shud clear here
  LOG_INFO("Library rebuilt in {:.3f} ms", timer.elapsed_ms());
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
  //
  // so this lets us run cmd line args of the main inLimbo binary without lock (as they are *mostly*
  // read ops) but if we launch a frontend then it will not allow multiple procs to run.
  //
  // Even if some args write to the actual file (like edit subcommand), the frontends are designed
  // to use the in memory `threads::SafeMap<SongMap>` which does not know about the changes made -
  // if any change is too destructive (say file path has changed) it will give a neat runtime error
  // and exit the app.
  //
  // The next time you run the binary, inLimbo will notice these changes and immediately do a
  // re-cache.

  try
  {
    utils::unix::LockFile appLock(INLIMBO_DEFAULT_LOCKFILE_PATH);
    auto song = query::songmap::read::findSongObjByTitleFuzzy(g_songMap, ctx.m_songTitle);

    if (!song)
    {
      LOG_ERROR("Song not found: '{}'", ctx.m_songTitle);
      return;
    }

    LOG_INFO("Fuzzy search song title query returned: '{}'", song->metadata.title);

    // ---------------------------------------------------------
    // Create AudioService (owns AudioEngine)
    // ---------------------------------------------------------
    audio::Service audio(g_songMap, ctx.m_audioBackendName);

    mpris::backend::Common mprisBackend(audio);
    mpris::Service         mprisService(mprisBackend, APP_NAME);

    // ---------------------------------------------------------
    // Initialize abstract frontend interface
    // ---------------------------------------------------------
    frontend::Plugin    fePlugin(ctx.m_fePluginName);
    frontend::Interface ui(fePlugin, g_songMap, ctx.m_telemetryCtx, &mprisService);

    if (!ui.ready())
    {
      LOG_CRITICAL("Frontend plugin '{}' is not ready. Aborting...", ctx.m_fePluginName);
      return;
    }

    LOG_INFO("---- Frontend Plugin Loaded ----");

    // ---------------------------------------------------------
    // Initialize backend
    // ---------------------------------------------------------
    audio.initForDevice(); // default device
    audio.setVolume(ctx.m_volume);

    // ---------------------------------------------------------
    // Register track + add to playlist (NO decoding here)
    // ---------------------------------------------------------
    audio::service::SoundHandle handle = audio.registerTrack(song);

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
    ui.destroy();

    LOG_INFO("---- Frontend Plugin Ended ----");

    // ---------------------------------------------------------
    // Save current telemetry context
    // ---------------------------------------------------------
    if (ctx.m_telemetryCtx.registry.save(
          utils::fs::getAppConfigPathWithFile(INLIMBO_DEFAULT_TELEMETRY_REGISTRY_BIN_NAME)))
      LOG_INFO("Saved telemetry registry successfully!");
    if (ctx.m_telemetryCtx.store.save(
          utils::fs::getAppConfigPathWithFile(INLIMBO_DEFAULT_TELEMETRY_BIN_NAME)))
      LOG_INFO("Saved telemetry store successfully!");
  }
  catch (const utils::unix::LockFileAlreadyLocked&)
  {
    LOG_ERROR("unix::LockFileAlreadyLocked: Another instance of inLimbo is already running.");
  }
  catch (const utils::unix::LockFileError& e)
  {
    LOG_ERROR("Context threw LockFileError: {}", e.what());
  }
  catch (const frontend::PluginError& e)
  {
    LOG_ERROR("frontend::PluginError: Context threw Frontend error: {}", e.what());
  }
  catch (std::exception& e)
  {
    LOG_ERROR("Context threw generic std::exception: '{}'", e.what());
  }
}

} // namespace inlimbo
