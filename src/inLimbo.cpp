#include "Context.hpp"
#include "Logger.hpp"
#include "StackTrace.hpp"
#include "utils/signal/Handler.hpp"

// this is optional to have, but useful for debugging
#ifdef INLIMBO_DEBUG_BUILD
#warning "INLIMBO_DEBUG_BUILD is enabled!"
#endif

auto main(int argc, char** argv) -> int
{
  RECORD_FUNC_TO_BACKTRACE("<MAIN>");
  utils::signal::Handler::getInstance().setup();

  try
  {
    auto ctx = inlimbo::initializeContext(argc, argv);
    inlimbo::buildOrLoadLibrary(ctx);
    if (inlimbo::maybeHandlePrintActions(ctx))
      return EXIT_SUCCESS;
    if (inlimbo::maybeHandleEditActions(ctx))
      return EXIT_SUCCESS;
    inlimbo::runFrontend(ctx);

    return EXIT_SUCCESS;
  }
  catch (std::exception& e)
  {
    LOG_ERROR("inLimbo main thread threw error: {}", e.what());
    return EXIT_FAILURE;
  }
}
