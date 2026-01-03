#include "CmdLine.hpp"
#include "Context.hpp"
#include "Logger.hpp"
#include "thread/Map.hpp"
#include "toml/Parser.hpp"
#include "utils/signal/Handler.hpp"

// this is optional to have, but useful for debugging
#ifdef INLIMBO_DEBUG_BUILD
#warning "INLIMBO_DEBUG_BUILD is enabled!"
#endif

threads::SafeMap<core::SongMap> g_songMap;

auto main(int argc, char* argv[]) -> int
{
  RECORD_FUNC_TO_BACKTRACE("<MAIN>");
  tomlparser::Config::load();
  utils::signal::Handler::getInstance().setup();

  try
  {
    auto ctx = inlimbo::initializeContext(argc, argv);
    inlimbo::buildOrLoadLibrary(ctx);
    inlimbo::maybeHandlePrintActions(ctx);
    inlimbo::maybeEditMetadata(ctx);
    inlimbo::runFrontend(ctx);

    return EXIT_SUCCESS;
  }
  catch (const cli::CmdLine::HelpRequested& h)
  {
    std::cout << h.text << '\n';
    return EXIT_SUCCESS;
  }
  catch (const cli::CmdLine::CliError& e)
  {
    LOG_ERROR("CliError: {}", e.message);
    return EXIT_FAILURE;
  }
}
