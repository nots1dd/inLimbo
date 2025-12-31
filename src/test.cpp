#include "CmdLine.hpp"
#include "thread/Map.hpp"
#include "utils/signal/SignalHandler.hpp"
#include "Context.hpp"

threads::SafeMap<core::SongMap> g_songMap;

auto main(int argc, char* argv[]) -> int
{
    RECORD_FUNC_TO_BACKTRACE("<MAIN>");
    utils::SignalHandler::getInstance().setup();

    try {
        auto ctx = inlimbo::initializeContext(argc, argv);
        inlimbo::buildOrLoadLibrary(ctx);
        inlimbo::maybeHandlePrintActions(ctx);
        inlimbo::maybeEditMetadata(ctx);
        inlimbo::runFrontend(ctx);

        return EXIT_SUCCESS;
    }
    catch (const cli::CmdLine::HelpRequested& h) {
        std::cout << h.text << '\n';
        return EXIT_SUCCESS;
    }
    catch (const cli::CmdLine::CliError& e) {
        LOG_ERROR("CliError: {}", e.message);
        return EXIT_FAILURE;
    }
}
