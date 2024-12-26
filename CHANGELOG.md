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
