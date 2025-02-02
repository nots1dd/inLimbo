# CHANGELOG 

This is the CHANGELOG file for [inLimbo](https://github.com/nots1dd/inLimbo)

All notable changes to this project will be documented in this file.

This is a markdown file that will contain all the logs of *ALPHA* and *STABLE* release commits with a decent overview of changes.

This file is useful iff:

1. A commit is hugely breaking and the changes have to be reverted
2. To better understand the progression of this project 
3. General debugging cases

## Format 

This CHANGELOG follows the below format:

```md 
----
## [Version X.Y] --- DD-MM-YYYY

## CHANGETYPE: [{CHANGETYPES}]

### Added
- Description of the new feature or functionality.
- Example: Added support for new audio file formats in the parser.

### Changed
- Description of modifications or improvements.
- Example: Improved performance of metadata parsing.

### Fixed
- Description of bug fixes or issues resolved.
- Example: Fixed crash when parsing invalid files.

### Removed
- Description of removed features or code.
- Example: Deprecated the old file format parser.

---
```

> [!IMPORTANT]
> 
> - **CHANGETYPES**: [FEAT, CHORE, NET, MINOR, MED, CLANG]
> 
> - **Versions are of two types:** [ALPHA, STABLE]
> 

------------------------------------- CHANGELOG STARTS ----------------------------------------

---

## [ALPHA 0.1] --- 26-12-2024

### Added
**NIL**

### Changed
- Made most the `src/dirsort` files as headers for easier and straightforward inclusion during compilation

- `main.cpp` in `src/dirsort` is renamed as `test.cpp`, as such that file can be used to test the song map that is created and its efficiency and other metrics

- Pushing forward with an integration of `src/dirsort` utilities with the `main.cpp` (which comprises of the FTXUI components)

- Addition of a new parent in `config.toml` ==> [debug]: will have all the fields required to turn on the debug logs during parsing, etc.

- Moved the InodeMapper class to a `src/dirsort/inode_mapper.hpp`

- Minor `.gitignore` changes

- `CMakeLists.txt` now requires taglib to compile

- Updated `src/network/NETWORK.md` file:- Now explains on how to change password and salt in detail

### Fixed
- Unnecessary logs that slow down the storing of metadata in song map (with debug field in config.toml)

### Removed
**NIL**

---

## [ALPHA 0.2] --- 26-12-2024

### Added
**NIL**

### Changed
- Attempted integration of `src/dirsort` with FTXUI components (NOT COMPLETE AT ALL)

- Setup macros for parents and fields of `src/parser/examples/config.toml` in `toml_parser.hpp`

- Moved MusicPlayer class to `src/ui/ui_handler.hpp`

- Added logo to the assets (with credits)

- Officially started to work on the audio backend api class in `src/music/audio_playback.hpp` using **miniaudio**

- Basic readme changes

### Fixed
**NIL**

### Removed
**NIL**

Overally pretty buggy, but integration was smoother than expected.

### Known Issues to fix in immediate commits

-> Fix emscripten build by introducing TagLib's web-assembly build 

-> Fix the FILENAME macro and its corresponding compilation problems in `src/parser/toml_parser.hpp`

---

## [ALPHA 0.3] --- 27-12-2024

### Added
**NIL**

### Changed
- Majorly developed FTXUI components and rendering problems

- Event::Tab now works fully as intended along with proper intended display of album -> songs (similar to CMUS)

- Finally shifted to reading TOML configs to `$HOME/.config/inLimbo/config.toml`

- Added filePath to Metadata struct (for miniaudio and simplicity)

### Fixed
- Compilation issues with `src/network` and `src/dirsort` source files due to improper accessing of `src/parser/examples/config.toml`

^ (every source file now reads from `$HOME/.config/inLimbo/config.toml` by default)

### Removed
**NIL**

Commit was clean but status bar components took a hit and are not rendering properly

### Known Issues to fix in immediate commits

-> Fix emscripten build by introducing TagLib's web-assembly build 

---

## [ALPHA 0.4] --- 29-12-2024

### Added
**NIL**

### Changed
- Started the integration of audio playback (miniaudio) and FTXUI class [A little rough on the edges but most of the functions in `src/music/audio_playback.hpp` work]

- Fixed the EMSCRIPTEN compilation issues regarding taglib (but it still will not work as intended as dummy functions are placed for EMSCRIPTEN compilation instead)

- Made album names distinct items in the songs pane 

- Clean up + Formatting

- Modified `src/music/audio_playback.hpp` to work in a different thread to avoid any freezing of tui and other issues

- Some API call changes to `miniaudio` for MiniAudioPlayer class to make functions like *getDuration()* work

- General `ui/ui_handler.hpp` changes to accomodate for the integration

### Fixed
- Web compilation using EMSCRIPTEN

### Removed
- taglib_parser.cpp (moved the definitions back to the header) [Subsequent changes to build files]

Commit is decently sized however now that integration is completed it is time to refactor and move to other goals

### Known Issues to fix in immediate commits
**NIL**

---

## [ALPHA 0.5] --- 31-12-2024

### Added
- `src/ui/keymaps.hpp` :: Basic middleware between TOML parser and MusicPlayer classes

### Changed
- Added keybind configuration and integrated them (**WITH THE EXCEPTION OF SPECIAL KEYS LIKE TAB, ENTER, ESC ...**)

- Updated UI code even further (no cleanup though) with better overall flow and neatness of the project 

- Added seek functionality in miniaudio backend (**BROKEN**)

- Current playing state struct now holds even more metadata of the currently playing song (in hopes to display them in the future)

- Updated `config.toml` with keybinds as a new parent

### Fixed
- The previously ugly status bar is now slightly better

- Bug when in help window, other keybinds would still work and cause mishap (so now only one keybind works when in help win)

### Removed
**NIL**

Commit is decently sized however now that integration is completed it is time to refactor and move to other goals

### Known Issues to fix in immediate commits
**NIL**

---

## [ALPHA 0.6] --- 31-12-2024

### Added
- `src/ui/misc.hpp` for miscellaneous functions that help the UI of the project

### Changed
- Added more fields to `config.toml`:- `goto_main_screen`, `toggle_mute` and `view_lyrics` with subsequent parsing and UI changes and integrations 

- Fixed forward seek (works fine) but backward seek is still bugged

- Added screen macros and better screen management to FTXUI

- Lyrics and additional info is now displayed in another window clearly (lyrics are still not scrollable however)

### Fixed
- Multiple screens code refactored the old code to just switch screens based on booleans

- Forward seek of a song

### Removed
**NIL**

Commit is medium sized with decent changes to UI and configs.

### Known Issues to fix in immediate commits
- Backward seek 

- Scrolling of lyrics

---

## [ALPHA 0.7] --- 02-01-2025

### Added
**NIL**

### Changed
- Finally fixed the keybinds for special keys like `Tab`, `Esc`, `Enter` and `Space` and is completely dynamic (depends on `config.toml` **ONLY**)

- Massively optimised `src/ui/keymaps.hpp` to have unordered_maps to hold and traverse required fields

- Some more documentation in `config.toml` and `src/network/NETWORK.md`

### Fixed
- Keybinds are now *FULLY* dynamic (regardless of whether the event is dynamic or just a character as per FTXUIs Event system)

### Removed
**NIL**

Commit is small but damn worth it. Fixed a very big issue and has solid optimisations

### Known Issues to fix in immediate commits
- Backward seek 

- Scrolling of lyrics

---

## [ALPHA 0.8] --- 03-01-2025

### Added
**NIL**

### Changed
- `seekTime` backend function to fix forward and backward seek times now

- Added a `charToStr` function for better code understanding

- UI bug fix for song seek

### Fixed
- Forward **AND** backward seek work now :) (an issue with the previous formula to calculate PCM frames)

- UI bug when the song would play from beginning but it would still not be shown as the beginning

### Removed
**NIL**

Very small commit but noteworthy as it fixes two bugs which were quite annoying

### Known Issues to fix in immediate commits
- Scrolling of lyrics

---

## [ALPHA 0.9] --- 03-01-2025

### Added
**NIL**

### Changed
- Changed `CMakeLists.txt` file to properly link and compile the project

- TagLib parser now parses the duration metadata as well

- Formatting of code 

- Slight refactor and optimisations of `src/ui/ui_handler.hpp`

- Now when `play_song` is invoked, all the songs of the current artist after the selected song are enqueued to a list, so even if we just view and switch artists,
  it will not change the current song

### Fixed
- Play next and previous songs are fixed thanks to Song queue implementation

### Removed
**NIL**

Decently sized commit but the issue of songs changing just by traversal is fixed now thanks to Song queue. This also is step forward to adding more keybinds to add song to queue, remove song from queue so on..

### Known Issues to fix in immediate commits
- Scrolling of lyrics

---

## [ALPHA 1.0] --- 04-01-2025

### Added
`src/ui/colors.hpp`

### Changed
- Finally shifted to all true colors by using `ftxui::Color::RGB` function

- Removed `spinner_frames`

- Updated main UI (Added number of songs in queue and next song display)

- Fixed help window to showcase the supported special chars (Tab, Enter, Space, Esc)

- Lyrics window is changed from (panes are now vertical not horizontal)

### Fixed
- Help window fix for special chars

- True colors implementation

### Removed
**NIL**

Medium sized commit that adds quite a few features and implementation but more work needs to be done.

### Known Issues to fix in immediate commits
- Scrolling of lyrics

---

## [ALPHA 1.1] --- 05-01-2025

### Added
**NIL**

### Changed
- Added 3 new fields to `config.toml` and `Adding Song to Queue` and `Replay Song` has been integrated and subsequent changes to `src/ui/keymaps.hpp` 

- Changed MiniAudioPlayer class's play() function thread sleep to 10ms

- Removed Repeat Mode UI for now

- Additional thread detach and creation handling to `PlayCurrentSong` in `src/ui/ui_handler.hpp`

### Fixed
- Some concurrency issues in `PlayCurrentSong` when invoking the function too fast 

### Removed
- Repeat Mode UI

Commit that proceeds to add queue features and fix issues (regarding threads) along the way

### Known Issues to fix in immediate commits
- Scrolling of lyrics

---

## [ALPHA 1.2] --- 07-01-2025

### Added
- Colors customizability setup (`COLORS.md`)

- Better Song Menu Viewing

- Better error handling of MusicPlayer class

- `src/ui/components` - Scroller component

### Changed
- Changing SongMenu from traditional menu to vector<Element> with apt rendering and scroll (**NOT DYNAMIC SCROLLING THO**)

- Added colors customizability with one field currently and subsequent changes to `src/ui/colors.hpp`

- UI overhaul (MAJOR)

- Better formatting of lyrics (FIXED)

- Formatting + some refactor

- Tried to make the player more safe with 

- New fields to `config.toml`

- New component `scroller` to try and render the scroller in render lyrics func

### Fixed
- Formatting of lyrics 

- Song menu UI

### Removed
- Some redundant functions like `format_song_info`

Very big commit in dev branch that is to be merged with main after some sanity checks

### Known Issues to fix in immediate commits
- Scrolling of lyrics and sone menu

- Runtime errors with respect to `PlayCurrentSong()` that may be due to detaching the audio thread

---

## [ALPHA 1.3] --- 10-01-2025

### Added
- New field in `config.toml` -> Remove Song From Queue

### Changed
- Added QUEUE SCREEN with subsequent proper integration and error handling (with testing) 

- Functionality of removing song from queue while in QUEUE SCREEN is added with testing

- CatchEvent changes with respect to active_screen to avoid confusion in UI

- Attempted to fix concurrency issues (still WIP)

### Fixed
**NIL**

### Removed
**NIL**

Decent commit with more to come, refactor TBD else its gonna be trouble

### Known Issues to fix in immediate commits
- Scrolling of lyrics and sone menu

- Runtime errors with respect to `PlayCurrentSong()` that may be due to detaching the audio thread

---

## [ALPHA 1.4] --- 11-01-2025

### Added
- Some concurrency safechecks in `src/music/audio_playback.hpp`

- Massive theming update, can use hexadecimal format to color the TUI with subsequent changes to `src/ui/colors.hpp` and `src/ui/ui_handler.hpp`

- Added multiple *DBG BUILD* options to check for issues in addressing, threads etc.

- Implemented loading the songmap from cache file ("lib.sync") itself [However, the time to access the cache and time to load the directory seem to be very similar]

### Changed
- Moved `.clang-format` to `/` in file source tree

- Changes to README regarding how to debug build

- Changes to COLORS.md regarding how to theme using hexadecimal

- New fields to `config.toml` under [colors] => songs, artists title's bg and fg

- Changed the InLimboColors struct in `src/ui/colors.hpp` to have `ftxui::Color` as member vars instead of `TrueColor::Color`

- Implementation of rendering lyrics is moved to a menu, sort of dynamic fixing it soon

### Fixed
- Bug with deleting the song from queue 

- Some keybind issues while in different screens

- A bug where `album_indices_traversed` was unaccounted for when navigating song menu to the top

### Removed
**NIL**

Pretty big commit that added a lot of changes, hopefully shifting toward refactoring in the next immediate commits

### Known Issues to fix in immediate commits
- Scrolling of lyrics and song menu (lyrics is almost done)

- Runtime errors with respect to `PlayCurrentSong()` that may be due to detaching the audio thread

---

## [ALPHA 1.5] --- 12-01-2025

### Added
- `src/dbus` -> To interact with dbus using `gio` (general purpose IO lib) and `glib` (utility lib)

- Added `gio` and `glib` as inLimbo's dependencies

### Changed
- Changes to README.md to showcase dependencies

- Complete integration of MPRISService class into inLimbo with subsequent changes and testing

- Metadata of current song is given to dbus that can be easily accessed by playerctl and similar daemons

### Fixed
**NIL**

### Removed
**NIL**

Minor commit that somehow added a pretty neat feature. Will continue to work on setting up implementations (if needed)

### Known Issues to fix in immediate commits
- Scrolling of lyrics and song menu (lyrics is almost done)

- Runtime errors with respect to `PlayCurrentSong()` that may be due to detaching the audio thread

---

## [ALPHA 1.6] --- 13-01-2025

### Added
- Doxygen documentation setup `.Doxyfile`

- New resized logo for Doxygen 

- Landing page basic html file

- Changes to all markdown files for doxygen docs

### Changed
- Comments to most headers to add Doxygen documentation

- Added `add_artists_songs_to_queue` and `play_this_song_next` fields to `config.toml` with subsequent integration

### Fixed
- Issue with RenderQueueScreen's menu disappearing when removing song from queue

### Removed
**NIL**

Weird commit that sets up for better and more transparent way of understanding the project thanks to doxygen

### Known Issues to fix in immediate commits
- Scrolling of lyrics and song menu (lyrics is almost done)

- Runtime errors with respect to `PlayCurrentSong()` that may be due to detaching the audio thread

---

## [ALPHA 1.7] --- 13-01-2025

### Added
- `src/ui/thread_manager.hpp` -> To handle concurrency in UI with testing + doxygen docs setup

- A LOT of new color fields to `config.toml` (Check it out) with subsequent integration in `src/ui/colors.hpp` and `src/ui/ui_handler.hpp`

- Some object deletion and freeing when calling MPRISService destructor 

- `src/signal` -> To catch all signals and handle gracefully (**FULL INTEGRATION NOT DONE YET**)

- Some changes to format lyrics function and added ComponentState in `src/ui/misc.hpp` to keep track of all components of UI together

### Changed
- Added more functions in misc.hpp to refactor code in ui_handler

### Fixed
- Finally fixed scrolling lyrics menu issue, now it completely works with testing

- Weird issue with deletion of song from queue is fully fixed with testing

### Removed
**NIL**

Sizeable commit with refactoring and more configuration options as its priority, more such commits expected in future

### Known Issues to fix in immediate commits
- Runtime errors with respect to `PlayCurrentSong()` that may be due to detaching the audio thread

---

## [ALPHA 1.8] --- 13-01-2025

### Added
- `.clang-tidy` file for better code practice

### Changed
- SignalHandler class now sends all errors with backtrace and other information to a log file in `$HOME/.cache/inLimbo/` directory

- Added unlockPlayMutex in `src/ui/thread_manager.hpp` with doxygen docs

- Formatting and other refactoring changes (small)

### Fixed
**NIL**

### Removed
**NIL**

Small commit that may fix codeql CI issue, going for refactor + small feature updates like cache, playlists, etc. (before the network integration)

### Known Issues to fix in immediate commits
- Runtime errors with respect to `PlayCurrentSong()` that may be due to detaching the audio thread

---

## [ALPHA 1.9] --- 14-01-2025

### Added
- Image Template processing kit `CImg.h` and `ftxui::image_view` component in `src/ui/components/` and `src/ui/components/libs`

- Thumbnail parsing function using taglib

### Changed
- New screen (SHOW_SONG_INFO_SCREEN) with thumbnail and currently playing song info with progress bar

- New field in `config.toml` to view song info screen (subsequent changes to `keymaps.hpp` and `ui_handler.hpp`)

- Modifications to build file (`CMakeLists.txt`) to include the component as library for proper linking 

- Readme changes

- `imagemagick` is now a required dependency for the inLimbo project

### Fixed
**NIL**

### Removed
**NIL**

Medium commit with a new screen, I should really start to refactor now...

### Known Issues to fix in immediate commits
- Runtime errors with respect to `PlayCurrentSong()` that may be due to detaching the audio thread

- Centering of the image_view

---

## [ALPHA 2.0] --- 16-01-2025

### Added
- Added `cereal` serialization and deserialization library (header only) to load songs statically (it is insanely fast as hoped)

### Changed
- inLimbo will now always try to load songs from a `$HOME/.config/inLimbo/lib.bin` file (static load) if not found dynamic load 

- Added documentation for `src/dirsort` headers

### Fixed
**NIL**

### Removed
**NIL**

Small commit that adds a new dependency and a greatly needed feature of caching the song directory

### Known Issues to fix in immediate commits
- Runtime errors with respect to `PlayCurrentSong()` that may be due to detaching the audio thread

- Centering of the image_view

---

## [ALPHA 2.1] --- 17-01-2025

### Added
- Added `src/arg-handler.hpp` and modified bash completions

### Changed
- Command line arguments setup with subsequent changes to `src/cmg-line-args.hpp` and `main.cpp`

### Fixed
- Issue with flac file thumbnail extraction

### Removed
**NIL**

Small commit with some issues + cmd line args setup

### Known Issues to fix in immediate commits
- Runtime errors with respect to `PlayCurrentSong()` that may be due to detaching the audio thread

- Centering of the image_view

---

## [ALPHA 2.2] --- 19-01-2025

### Added
- `assets/completions/inLimbo-completions.fish` and `assets/inLimbo.desktop`

### Changed
- Refactored the arg-handler and cmd-line-args headers

- Completions for `BASH`, `ZSH` and `FISH` shells work with testing

- inLimbo.desktop setup works (with rofi, not ags for some reason) 

- `init.sh` now sets up the preferred shell for command line args completion

- Added option to build globally 

### Fixed
- Completions for BASH and ZSH 

- Removed unnecessary header that broke CodeQL CI

### Removed
**NIL**

Small commit with some nice touch ups for user interaction with the binary

### Known Issues to fix in immediate commits
- Runtime errors with respect to `PlayCurrentSong()` that may be due to detaching the audio thread (BIG)

- Centering of the image_view (NOT THAT BIG)

---

## [ALPHA 2.3] --- 22-01-2025

### Added
- `src/ui/properties.hpp` -- To parse global properties from config.toml and subsequent changes to `ui_handler`

- `Makefile` for easier building without a hassle, subsequent changes to README

### Changed
- Added new color field (inactive_menu_cursor_bg) and complete integration

- New cmd-line-arg `--update-cache-run` :- Removes cache then reads the directory from config.toml and runs the application (subsequent changes to completions, `cmd-line-args.hpp`, `arg-handler.hpp`)

- Parsing bitrate using TagLib integrated and tested `mp3` and `flac` files

- Audio file MIME types are found and displayed in SongInfoScreen

- UI should not try to PlayNextSong() in Run() function if song_queue is empty.

### Fixed
**NIL**

### Removed
**NIL**

Sizeable commit with new fields, changes and features that bring the app together

### Known Issues to fix in immediate commits
- Runtime errors with respect to `PlayCurrentSong()` that may be due to detaching the audio thread (BIG)

- Centering of the image_view (NOT THAT BIG)

---

## [ALPHA 2.4] --- 24-01-2025

### Added
- Added tests/ directory to perform unit tests using GTest and subsequent new files and test suites

- `src/threads/` created to have a worker thread system for better concurrency handling

- Added a new screen to view current audio sinks (outputs)  

- Added a destructor to ThreadManager class to properly join all non-detached threads

### Changed
- Cleanups in `src/ui` , `src/music` `src/dirsort`

- Changed ThreadManager drastically

- Modified Run function in ui_handler to NOT be detached

- Got rid of mutex unlock and used lock_guard instead + more concurrency changes

- New field in config.toml -> toggle_audio_devices (will change this name later)

- Made mpris_dbus_thread a unique thread

- PlayCurrentSong now uses the WorkerThread Pool to enqueue all tasks and has a more robust way to handle concurrency

- New workflow to build tests

### Fixed
**NIL**

### Removed
**NIL**

Huge commit (I have not mentioned a lot changes here do read the commit history)

### Known Issues to fix in immediate commits
- Runtime errors with respect to `PlayCurrentSong()` that may be due to detaching the audio thread (BIG) [Seems to be solvable]

- Centering of the image_view (NOT THAT BIG)

---

## [ALPHA 2.5] --- 25-01-2025

### Added
- New tests for `taglib` and `toml_parser` with integration 

- BUILD.md for clear and concise build instructions using Makefile, CMake and web-asm

- `src/helpers` directory for general helpers -> Added `levenshtein.hpp` for measuring difference btw 2 strings (for possible suggestions)

- `run_tests.sh` to run all the tests in `tests/` directory and give a cumulative output (will use in workflow)

### Changed
- Cmd-line-arg string suggestion moved from naive findClosestMatch function to levenshtein distance measure

- Added a MACRO `INLIMBO_CONFIG_HOME` that allows for custom location of config.toml with subsequent changes to `toml_parser.hpp`

- Color field error now reports the field that is giving the error (better verbosity) for more clarity on the config issue 

- Workflow update to run tests as well

### Fixed
- Mild issue where color field error would be vague in error description

### Removed
**NIL**

Medium commit, moving strictly towards testing + refactor

### Known Issues to fix in immediate commits
- Runtime errors with respect to `PlayCurrentSong()` that may be due to detaching the audio thread (BIG) [Seems to be solvable]

- Centering of the image_view (NOT THAT BIG)

---

## [ALPHA 2.6] --- 26-01-2025

### Added
**NIL** 

### Changed
- Followed better trailing return types for a lot of functions 

- More verbosity for taglib parser logs using parseFile function call

- loadFile and getSongDuration functions in miniaudio are now an async functions that have significant performace improvement especially during fast UI calls to play a song

- UpdatePlayingState is now part of the worker thread pool with better struct data protection to avoid any corrupt values in fields

- Fetching the debugLogBool value should be faster now (previously there were so many string comparisons idk what i was thinking, now it is just a boolean)

- Keybinds and InLimboColors structs are initialized in main func and given as parameterized values to MusicPlayer

- Subsequent changes to `tests/audio` and `tests/taglib` test suites

### Fixed
- The MAJOR issue of the player just exiting if the song queue was <= ~6 songs is fixed :)

- Unknown songs being overwritten in song map to track 0 by default is fixed (now every unknown song that taglib could not parse is shown under `<Unknown Artist>`)

- When traversing back down to the last song, the album_indices_traversed value was buggy (would cause songs to not be played), now it is fixed

### Removed
- Unnecessary libsync file and constructor and destructor of InodeMapper (might add them later when logging is setup)

- Useless function processCacheFile in InodeMapper

Sizeable commit, a lot of bugs resolved which is great, still hoping for better refactor and more test suite inclusion

### Known Issues to fix in immediate commits
- Holding the keybind for PlayNextSong() / PlayPrevSong() doesnt break anything, but MiniAudioPlayer class is not as responsive as the UI, so it lags behind (MAJOR ISSUE)

(The outcome of the above issue would be that if you hold PlayNextSong() func call and it goes to Song A, the MiniAudioPlayer might still be playing Song B, which appears BEFORE Song A)

- Centering of the image_view (NOT THAT BIG)

---

## [ALPHA 2.7] --- 27-01-2025

### Added
**NIL** 

### Changed
- Overall version bump to 2.7

- Added `build-global`, `build-global-uninstall` to makefile

- Subsequent makefile, readme and build file changes

- CMakeLists.txt now posts a very neat verbose output of the build configuration it will undergo (useful for debugging)

- Moved `PlayingState` struct definition outside of `ui_handler` header

- Overall code formatting and minor fixes

### Fixed
- Debug builds for ASan and TSan using makefile

### Removed
- Removed debounce time for now, seems unnecessary but might add in the future

Small commit with some neat build changes that makes life pretty easy

### Known Issues to fix in immediate commits
- Holding the keybind for PlayNextSong() / PlayPrevSong() doesnt break anything, but MiniAudioPlayer class is not as responsive as the UI, so it lags behind (MAJOR ISSUE)

(The outcome of the above issue would be that if you hold PlayNextSong() func call and it goes to Song A, the MiniAudioPlayer might still be playing Song B, which appears BEFORE Song A)

- Centering of the image_view (NOT THAT BIG)

---

## [ALPHA 2.8] --- 29-01-2025

### Added
- New workflow to build in g++ and clang++ (x86 arch)

### Changed
- Makefile now gives the option to compile to any desired c++ compiler

- General code refactor + minor issues fix mainly in ui_handler

- UI overhaul + updates to several render functions 

- New function in miniaudio class to show case more details regarding the miniaudio engine, device config etc..

- Artist menu is now a scroller base component not a menu anymore

- New colors and keybinds fields (more updates soon) 

### Fixed
- Minor issue of a song first loading with full volume by default before getting updated (would sound weird and jarring)

### Removed
- 

Medium commit with some refactoring and small, nice QOL changes

### Known Issues to fix in immediate commits
- Holding the keybind for PlayNextSong() / PlayPrevSong() doesnt break anything, but MiniAudioPlayer class is not as responsive as the UI, so it lags behind (MAJOR ISSUE)

(The outcome of the above issue would be that if you hold PlayNextSong() func call and it goes to Song A, the MiniAudioPlayer might still be playing Song B, which appears BEFORE Song A)

- Centering of the image_view (NOT THAT BIG)

- Fixing current_position bug when run refresh thread is woken up a lot of times (BIG)

---

## [ALPHA 2.9] --- 01-02-2025

### Added
- Trie `src/helpers/trie.hpp` --> For efficient string search

- Powerful and neat string search for artists (songs does not work atm) with next and previous search match (circular match)

### Changed
- Every header now uses `#pragma once` instead of `#ifndef` (idk why i was doing that) 

- Overall refactor / code change to most headers

- Added 2 new debug parser logs (keybinds and colors)

- Much better UI for Song Info screen

- Readme changes to explain the default keybinds and their use

### Fixed
- Centered the Image View component in Song Info Screen

### Removed
- setDevice in MiniAudioPlayer (will add in future but not the priority right now) 

Medium commit with some refactoring and neat changes, getting to better UI and codebase maybe

### Known Issues to fix in immediate commits
- Holding the keybind for PlayNextSong() / PlayPrevSong() doesnt break anything, but MiniAudioPlayer class is not as responsive as the UI, so it lags behind (MAJOR ISSUE)

(The outcome of the above issue would be that if you hold PlayNextSong() func call and it goes to Song A, the MiniAudioPlayer might still be playing Song B, which appears BEFORE Song A)

- Fixing current_position bug when run refresh thread is woken up a lot of times (BIG)

---

## [ALPHA 3.0] --- 02-02-2025

### Added
- State management header `src/ui/states/state.hpp` that has the member structs and methods of Queue, PlayingState and Component for now (will add more soon)

- New and modified arguments `--print-song-tree`, `--print-songs-by-artist=`, `--print-songs-by-genre-all`, `--print-artists-all`

### Changed
- Refactor sizeable code in ui_handler for better state management

- Modified `--print-song-tree` argument for a better unicode tree like representation (like lsblk) 

- Changed all completion files to accomodate for new cmd-line-args

- Reading the version is no longer statically compiled, it now reads from `VERSION` file and prints it

- CommandLineArgs got a change to print the value from the positional argument for a more robust cli interface 

- Readme changes

### Fixed
- The issue of unknown tracks being numbered correctly only at dynamic load with taglib debug log on is fixed (now works as intended)

### Removed
**NIL**

Medium commit with very satisfactory refactoring changes, some features to the cli interface

### Known Issues to fix in immediate commits
- Holding the keybind for PlayNextSong() / PlayPrevSong() doesnt break anything, but MiniAudioPlayer class is not as responsive as the UI, so it lags behind (MAJOR ISSUE)

(The outcome of the above issue would be that if you hold PlayNextSong() func call and it goes to Song A, the MiniAudioPlayer might still be playing Song B, which appears BEFORE Song A)

- Fixing current_position bug when run refresh thread is woken up a lot of times (BIG)

---
