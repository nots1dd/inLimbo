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

## Known Issues to fix in immediate commits

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

## Known Issues to fix in immediate commits

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

## Known Issues to fix in immediate commits
**NIL**

---
