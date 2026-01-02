#pragma once

#include "CmdLine.hpp"
#include "core/SongTree.hpp"
#include "core/taglib/Parser.hpp"
#include "utils/timer/Timer.hpp"
#include <string>

namespace inlimbo
{

void setupArgs(cli::CmdLine& args);

enum class PrintAction
{
  None,
  Artists,
  SongInfo,
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
  AppContext(const std::string& program, const std::string& description);

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

auto resolvePrintAction(const cli::CmdLine& args) -> PrintAction;
void maybeHandlePrintActions(AppContext& ctx);
auto initializeContext(int argc, char** argv) -> AppContext;
void buildOrLoadLibrary(AppContext& ctx);
void maybeEditMetadata(AppContext& ctx);
void runFrontend(AppContext& ctx);

} // namespace inlimbo
