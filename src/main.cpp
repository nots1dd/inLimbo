#include "./arg-handler.hpp"
#include "./ui/keymaps.hpp"
#include "network/instance.hpp"
#include "dirsort/inode_mapper.hpp"
#include "signal/signalHandler.hpp"
#include "ui/ui_handler.hpp"

/**
 * @brief Handles the command line arguments.
 *
 * This function processes the command line arguments and sets the appropriate paths for
 * configuration, library binary, and cache directory. It returns a `Paths` structure containing
 * the necessary paths for the program's operation.
 *
 * @param argc The number of command line arguments.
 * @param argv The array of command line arguments.
 * @return The `Paths` structure containing the paths for the configuration, library binary, and
 * cache directory.
 */
auto handleArguments(int argc, char* argv[]) -> ArgumentHandler::Paths
{
  string                 configPath = getConfigPath("config.toml");
  string                 cacheDir   = getCachePath();
  string                 libBinPath = getConfigPath(LIB_BIN_NAME);
  CommandLineArgs        cmdArgs(argc, argv);
  ArgumentHandler::Paths paths{
    .configPath = configPath, .libBinPath = libBinPath, .cacheDir = cacheDir};
  ArgumentHandler::handleArguments(cmdArgs, argv[0], paths);
  return paths;
}

/**
 * @brief Loads or processes the song tree from the given directory.
 *
 * This function attempts to load a previously cached song tree from a file. If the song tree
 * cannot be loaded, it processes the directory to generate a new song tree, stores it in a
 * red-black tree structure, and saves the tree to a file for future use.
 *
 * @param libBinPath The path to the library binary directory.
 * @param directoryPath The path to the directory containing the music files.
 * @return The processed or loaded `SongTree` object.
 */
auto loadOrProcessSongTree(const string& libBinPath, const string& directoryPath) -> SongTree
{
  RedBlackTree    rbt;
  InodeFileMapper mapper;
  SongTree        song_tree;

  try
  {
    song_tree.loadFromFile(libBinPath);
    cout << "-- Successfully loaded song tree from cache file." << endl;
  }
  catch (const std::exception& e)
  {
    cout << "-- Could not load song tree from file: " << e.what() << endl;
    cout << "-- Processing directory instead..." << endl;
    processDirectory(directoryPath, rbt, mapper);
    rbt.inorderStoreMetadata();
    song_tree = rbt.returnSongTree();
    song_tree.saveToFile(libBinPath);
  }
  return song_tree;
}

/**
 * @brief Runs the music player with the provided song tree.
 *
 * This function initializes the music player with the provided song tree, keybinds, and color
 * settings, and then runs the player. If any exceptions are encountered, an error message is
 * displayed, and the program exits with a non-zero code.
 *
 * @param song_tree The `SongTree` object containing the music library.
 * @return 0 if the music player runs successfully, or 1 if an error occurs.
 */
auto runMusicPlayer(SongTree& song_tree) -> int
{
  auto          library_map     = song_tree.returnSongMap();
  Keybinds      global_keybinds = parseKeybinds();
  InLimboColors global_colors   = parseColors();
  MusicPlayer   player(library_map, global_keybinds, global_colors);
  try
  {
    player.Run();
  }
  catch (const std::exception& e)
  {
    std::cout << "Some issue has occurred with inLimbo: " << e.what() << std::endl
              << "Exiting with code 1." << std::endl;
    return 1;
  }
  return 0;
}

/**
 * @brief The main entry point of the program.
 *
 * This is the main function that initializes signal handling, parses command line arguments,
 * processes the song tree, and runs the music player. It also measures the time taken for the
 * inode insertion and mapping process and prints the duration to the console.
 *
 * Also binds a Unix domain socket and ensures that only one instance is running.
 *
 * @param argc The number of command line arguments.
 * @param argv The array of command line arguments.
 * @return 0 if the music player runs successfully, or 1 if an error occurs.
 */
auto main(int argc, char* argv[]) -> int
{
  SignalHandler::getInstance().setup();
  auto socketInstance = std::make_unique<SingleInstanceLock>();  // RAII 
  string                 directoryPath = string(parseTOMLField(PARENT_LIB, PARENT_LIB_FIELD_DIR));
  ArgumentHandler::Paths paths         = handleArguments(argc, argv);

  auto     start     = chrono::high_resolution_clock::now();
  SongTree song_tree = loadOrProcessSongTree(paths.libBinPath, directoryPath);

  if (parseSongTree)
  {
    ArgumentHandler::processSongTreeArguments(song_tree);
    return 0;
  }
  if (awaitSocketInstance)
  {
    ArgumentHandler::processSocketInstance(*socketInstance);
    return 0;
  }

  if (!socketInstance->isLocked())
  {
    return EXIT_FAILURE;
  }

  auto end = chrono::high_resolution_clock::now();

  int result = runMusicPlayer(song_tree);
  cout << "-- Inode insertion & mapping time: "
       << chrono::duration_cast<chrono::milliseconds>(end - start).count() << " ms" << endl;

  return result;
}
