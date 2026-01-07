#pragma once

#include "CmdLine.hpp"
#include "core/SongTree.hpp"
#include "core/taglib/Parser.hpp"
#include "thread/Map.hpp"
#include "utils/timer/Timer.hpp"
#include <string>

extern threads::SafeMap<SongMap> g_songMap;

namespace inlimbo
{

void setupArgs(cli::CmdLine& args);

enum class PrintAction
{
  None,
  Artists,
  Albums,
  Genres,
  Summary,
  SongInfo,
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

struct AppContext
{
  explicit AppContext() = delete;
  AppContext(const std::string& program, const std::string& description);

  // cmdline parser
  cli::CmdLine m_cmdLine;

  // CLI
  Title                      m_songTitle           = {};
  utils::string::SmallString m_debugLogTagLibField = {};
  float                      m_volume              = {};

  PrintAction m_printAction = PrintAction::None;
  EditAction  m_editAction  = EditAction::None;

  // Paths
  Directory m_musicDir = {};
  Path      m_binPath  = {};

  // Core objects
  core::TagLibParser m_tagLibParser;
  core::SongTree     m_songTree = {};
  utils::Timer<>     m_timer    = {};
};

auto resolvePrintAction(const cli::CmdLine& args) -> PrintAction;
void maybeHandlePrintActions(AppContext& ctx);
void maybeHandleEditActions(AppContext& ctx);
auto initializeContext(int argc, char** argv) -> AppContext;
void buildOrLoadLibrary(AppContext& ctx);
void runFrontend(AppContext& ctx);

} // namespace inlimbo
