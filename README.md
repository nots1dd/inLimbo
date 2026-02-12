<div align="center">
  <img src="assets/logo/inLimbo.jpg" alt="Logo">
</div>

<div align="center">
  <h1 style="font-weight: bold; font-size: 3em; color: #3498db;">inLimbo</h1>
</div>

<p align="center">
  <img src="https://img.shields.io/badge/License-GNU GPL%20v3-blue" alt="License" />
  <img src="https://img.shields.io/badge/Version-3.0%20alpha-red" alt="Version" />
  <img src="https://img.shields.io/badge/Build-passing-brightgreen" alt="Build" />
</p>
<p align="center">
  <a href="https://nots1dd.github.io/inLimbo">Documentation</a> ·
  <a href="https://github.com/nots1dd/inLimbo?tab=readme-ov-file#features">Features</a> ·
  <a href="https://github.com/nots1dd/inLimbo?tab=readme-ov-file#looks">Looks</a> .
  <a href="https://github.com/nots1dd/inLimbo?tab=readme-ov-file#building">Get Started</a> ·
  <a href="https://github.com/nots1dd/inLimbo/issues">Report a bug</a>
</p>

> [!CAUTION]
>
> This is the develop branch!
>
> You will have to install **CMake & Make**, **Git**, **C++20 Compiler (GCC/Clang)**, **dbus-1**, **OpenSSL**, **TagLib**!!
>
> Have the repo cloned **WITH THE SUBMODULES**:
>
> ```bash
> # clone repo (develop branch)
> git clone -b develop https://github.com/nots1dd/inlimbo
>
> cd inlimbo
> make init
> make buildx
> ```
>

> [!WARNING]
>
> inLimbo is a WIP
>
> Most aspects of the project are under heavy development
> and no stable release is present as of yet.
>

The inLimbo project aims to be a new upcoming TUI music player for *NIX based operating systems that gives music lovers a clean and efficient environment to browse, play and interact with your favourite offline music.

## **Why**

There are a dozen of great music players out there that do a really good job of making my music experience top notch, but nothing was ever perfect. So I tried making something that in my opinion comes closest to my ideal TUI music player.

Why would you want to stay in Limbo?

Sometimes it feels good to be stuck in a weird place in your life, and it feels even better to be stuck in Limbo with music :)

## **DEPENDENCIES**

You can check the deps in `ci/<distro-pkg-manager>/` (ex: If you are on Arch Linux, check out `ci/pacman/packages.txt`)

To install these deps you *could* run:

> [!CAUTION]
>
> Although the Makefile will also give a similar prompt,
> do your due diligence in understanding what the `install-deps`
> command is going to execute!
>
> This project is not responsible for what happens to
> your system should any problems arise.
>

```bash
# first verify if you have all the deps (SAFE - NO CHANGES TO SYSTEM)
make verify-deps

# install deps (!! RUN IT AT YOUR OWN DISCRETION !!)
make install-deps
```

Note that `install-deps` will only work for:

1. Arch Linux
2. Fedora
3. Ubuntu/Debian

## **BUILDING**

> [!IMPORTANT]
>
> inLimbo has been **TRIED AND TESTED** in the following GNU/Linux distros:
>
> 1. Arch Linux (x86-64 arch)
> 2. Ubuntu (x86-64 arch)
> 3. Fedora/Nobara Linux (x86-64 arch)
>
> As long as your distribution can install & build the requirements, there should *NOT* be any issue
>

Check out [BUILD.md](https://github.com/nots1dd/inLimbo/blob/develop/BUILD.md) for more options to build targets

## **RUNNING**

Just run:

```bash
./build/inLimbo -h                           # for help

# A few examples
./build/inLimbo query -h                     # to show help for query options
./build/inLimbo query -s                     # to show summary of the library
./build/inLimbo modify -r query -a           # rebuild the library then query library to show all artists

./build/inLimbo -s "Song A" edit -t "Song B" # Give title 'Song A' and edit the title to 'Song B'
```

A LOT of things are possible with your song library with inLimbo. Lots of read only queries at your fingertips, and it unleashes the potential of having locally downloaded music.

> [!IMPORTANT]
>
> Some arguments mentioned maybe outdated so always run `--help` or `-h`
> before running the above!!
>

## **CONFIG**

The configuration aspect of the inLimbo project comprises of the following fields:

1. Keybinds
2. Colors (Theme)
3. Library Sort

Something that is quite cool and almost standardised nowadays that inLimbo has is **hot-reloading** of the config file.

> [!NOTE]
>
> **What is hot-reloading of config file?**
>
> If you edit the config file (`~/.config/inLimbo/config.toml`) (without killing inLimbo process),
> it will immediately show up on the frontend! This allows you to make themes and vibe to your music :)
>
> Note that this is **only** available for `keybinds` and `colors` fields only!!
>

Although more is to come for this, the architecture is made in such a way that the colors and keybinds config and watcher are created by
`inLimbo-core`, but the frontend takes the onus of actually finding the required fields, validation, reloading of config and whatnot.

So currently for example, the `cmdline` frontend is super dynamic and can be hot-reloaded meanwhile, `raylib` is just a proof of concept.
The config and the app itself is just to show how multiple frontends can exist for inLimbo. (config is statically compiled)

Check out [example config file](https://github.com/nots1dd/inLimbo/blob/develop/examples/config/config.toml)

### **Config 1.1: Library Sorting**

Library sort is a feature in most music players and they do get the job done (sometimes UX maybe a bit scuffed), which is why I felt the need to add one that is fast, reliable and easy to set.

The idea behind the sort is that we can "reorder" the song map based on 3 field members:

1. Artist
2. Album
3. Track

More *could* be added if that is required but these are the most used fields.

Now each field has certain criteria upon which the map is reordered.

1. Artist: `Lexicographically Ascending/Descending (ASCII only)`, `Albums Count Ascending/Descending`, `Track Count Ascending/Descending`
2. Album: `Lexicographically Ascending/Descending (ASCII only)`, `Track Count Ascending/Descending`
3. Track: `Track Number Ascending/Descending`

These are self-explanatory and easy to understand what each sort does. You can check out [example config.toml](https://github.com/nots1dd/inLimbo/tree/develop/examples/config/config.toml) for how to customize each sort.

You can set each field as however you desire and `inLimbo-core` is made in such a way that it supports **runtime change** of the sorting "plan".

This isn't to say there won't be more options coming (ex: Artist sorted by `Year Ascending/Descending`, etc.) but those are not a top priority at the moment and with the initial groundwork setup, I make it easy to add new sorting plans to the codebase without too many changes.

If you are wondering how to integrate this into the frontend, I will have a blog / doc coming up on it but for now I suggest looking at the `cmdline` frontend code here: [Frontend Interface (cmdline)](https://github.com/nots1dd/inLimbo/tree/develop/src/frontend/cmdline/Interface.cc)

> [!NOTE]
> 
> As of writing this, I did **NOT** find any bugs, but 
> honestly this is a pretty dynamic feature that affects the 
> audio engine's playlist and frontend.
> 
> Am fairly certain editing the sort just as a song ends *may* cause a 
> problem but I could be wrong.
> 


## **MISC**

### **MISC 1.1: Lyrics**

inLimbo can showcase lyrics in this manner:

```bash 
inLimbo query -L "some song"
```

The underlying logic of finding the lyrics is like so:

```text
Metadata Obj fetched of "some song"
            |
            |
            V
        If lyrics present
          and not empty          -----> PRINT and EXIT
            |
            | (else)
            V
Check $HOME/.config/inLimbo/LRC/<LRC-GEN-PATH>.lrc
for "some song"
            |
            |
            V
        If file present
         and not emtpy         ------> PRINT and EXIT
            |
            | (else)
            V
Fetch from lrclib.net via utils::unix::net::HTTPSClient
and cache JSON body and LRC in $HOME/.config/inLimbo/
            |
            |
            V
       PRINT and EXIT
```

The logic is made in such a way that we wont need to query `lrclib.net` multiple times on the same song (unless the cache is removed ofc).

So in frontend, displaying the lyrics should follow the same flow as the above to avoid minimal network queries.

## **PHILOSOPHY**

The core philosophy of inLimbo can be primarily summarized in:

1. No action is done without the user knowing

## **CREDITS**

**LOGO**:

> **!! DISCLAIMER !!**: This is not an original logo nor was it commisioned by me

It is a basic stock image found online. I got it from [here](https://unsplash.com/photos/black-and-white-decor-FrQKfzoTgsw)

1. **TOML PARSER: TOML++**:

To parse the [config.toml](https://github.com/nots1dd/inlimbo/blob/main/src/parser/examples/config.toml) is thanks to [TOML++](https://github.com/marzer/tomlplusplus)

2. **Command Line Parsing (Arguments): CLI11**:

To parse the command line arguments (and sparing me time to work on the actual project) is thanks to [CLI11](https://github.com/CLIUtils/CLI11)

3. **Serialization: Cereal**:

Easy and fast serialization and deserialization of song map is thanks to [cereal](https://github.com/USCiLab/cereal/)

4. **Logging: spdlog**:

Modular and fast logging is thanks to [spdlog](https://github.com/gabime/spdlog)

5. **Backtrace: libbacktrace**:

Very convenient stack trace for easy debugging thanks to [libbacktrace](https://github.com/ianlancetaylor/libbacktrace)

6. **Datastructures: unordered_dense**:

A super easy and better drop in replacement of `std::unordered_map` with `ankerl::unordered_dense::map` thanks to [unordered_dense](https://github.com/martinus/unordered_dense)

7. **JSON Parser: nlohmann/json**:

For easy HTTP body parsing is thanks to [nlohmann/json](https://github.com/nlohmann/json)

**INSPIRATIONS**:

This music player's UI flow is heavily inspired by [CMUS](https://github.com/cmus/cmus), it seemed (to me atleast) the fastest and most responsive and logical UI/UX design of a TUI that has a great balance between looking good and feeling smooth

A lot of the features and UX came to my head thanks to [namida](https://github.com/namidaco/namida), very sleek cross platform player with great UI.

inLimbo is Free and Open Source Software Licensed under [GNU GPL v3](https://github.com/nots1dd/inlimbo/blob/main/LICENSE)
