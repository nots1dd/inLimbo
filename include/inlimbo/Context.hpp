#pragma once

#include "Args.hpp"
#include "CLI/CLI.hpp"
#include "core/taglib/Parser.hpp"
#include "frontend/Plugin.hpp"
#include "thread/Map.hpp"
#include "utils/timer/Timer.hpp"

extern threads::SafeMap<SongMap> g_songMap;

namespace inlimbo
{

enum class PrintAction
{
  None,
  Frontends,
  Artists,
  Albums,
  Genres,
  Summary,
  SongInfoByTitle,
  Lyrics,
  SongPaths,
  SongsByArtist,
  SongsByAlbum,
  SongsByGenre,
};

enum class EditAction
{
  None,
  Title,
  Artist,
  Album,
  Genre,
  Lyrics
};

void setupArgs(CLI::App& app, Args& args);

struct AppContext
{
  explicit AppContext() = delete;
  AppContext(CLI::App& cliApp);

  inlimbo::Args args;

  // CLI
  Title m_songTitle;
  bool  m_taglibDbgLog;
  float m_volume;

  PrintAction m_printAction = PrintAction::None;
  EditAction  m_editAction  = EditAction::None;

  int m_fuzzyMaxDist = 2;

  // Paths
  Directory m_musicDir;
  Path      m_binPath;

  // Frontend plugin
  frontend::PluginName m_fePluginName;

  // Core objects
  core::TagLibParser m_tagLibParser;
  utils::Timer<>     m_timer;
};

auto resolvePrintAction(const Args& args) -> PrintAction;
auto resolveEditAction(const Args& args) -> EditAction;
auto maybeHandlePrintActions(AppContext& ctx) -> bool;
auto maybeHandleEditActions(AppContext& ctx) -> bool;
auto initializeContext(int argc, char** argv) -> AppContext;
void buildOrLoadLibrary(AppContext& ctx);
void runFrontend(AppContext& ctx);

} // namespace inlimbo
