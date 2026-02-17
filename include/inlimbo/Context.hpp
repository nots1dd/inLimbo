#pragma once

#include "Args.hpp"
#include "CLI/CLI.hpp"
#include "frontend/Plugin.hpp"
#include "taglib/Parser.hpp"
#include "telemetry/Context.hpp"
#include "thread/Map.hpp"

#define APP_NAME      "inLimbo"
#define APP_AUTHOR    "nots1dd (Siddharth Karanam)"
#define APP_COPYRIGHT "Copyright (c) 2024-2026 nots1dd. All rights reserved."

#define APP_DESC "inLimbo: Advanced Audio Library Management and Playback Tool\n" APP_COPYRIGHT

extern threads::SafeMap<SongMap> g_songMap;

namespace inlimbo
{

enum class PrintAction
{
  None,
  Frontends,
  AudioBackends,
  AudioDevs,
  Artists,
  Albums,
  SongTree,
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
  Lyrics,
  FetchLyrics
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

  // Telemetry
  telemetry::Context m_telemetryCtx;

  // Frontend plugin
  PluginName m_fePluginName;

  // Core objects
  taglib::Parser m_tagLibParser;
};

auto resolvePrintAction(const Args& args) -> PrintAction;
auto resolveEditAction(const Args& args) -> EditAction;
auto maybeHandlePrintActions(AppContext& ctx) -> bool;
auto maybeHandleEditActions(AppContext& ctx) -> bool;
auto initializeContext(int argc, char** argv) -> AppContext;
void buildOrLoadLibrary(AppContext& ctx);
void runFrontend(AppContext& ctx);

} // namespace inlimbo
