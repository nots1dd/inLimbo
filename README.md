<div align="center">
  <img src="assets/logo/inLimbo.jpg" alt="Logo">
</div>

<div align="center">
  <h1 style="font-weight: bold; font-size: 3em; color: #3498db;">inLimbo</h1>
</div>

<p align="center">
  <img src="https://img.shields.io/badge/License-GNU GPL%20v3-blue" alt="License" />
  <img src="https://img.shields.io/badge/Version-0.0.1%20alpha-red" alt="Version" />
  <img src="https://img.shields.io/github/actions/workflow/status/nots1dd/inLimbo/build-ubuntu.yml?label=ubuntu&logo=ubuntu" />
  <img src="https://img.shields.io/github/actions/workflow/status/nots1dd/inLimbo/build-fedora.yml?label=fedora&logo=fedora" />
  <img src="https://img.shields.io/github/actions/workflow/status/nots1dd/inLimbo/build-arch.yml?label=arch&logo=archlinux" />
</p>
<p align="center">
  <a href="https://nots1dd.github.io/inLimbo">Documentation</a> ·
  <a href="https://github.com/nots1dd/inLimbo?tab=readme-ov-file#features">Features</a> ·
  <a href="https://github.com/nots1dd/inLimbo?tab=readme-ov-file#looks">Looks</a> .
  <a href="https://github.com/nots1dd/inLimbo?tab=readme-ov-file#getting-started">Get Started</a> ·
  <a href="https://github.com/nots1dd/inLimbo/issues">Report a bug</a>
</p>

> [!WARNING]
>
> inLimbo is a WIP
>
> Most aspects of the project are under heavy development
> and no stable release is present as of yet.
>

The inLimbo project aims to be a new upcoming TUI music player for *NIX based operating systems that gives music lovers a clean and efficient environment to browse, play and interact with your favourite offline music.

## **WHY?**

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

## **GETTING STARTED**

You will have to install the following on your system:

| Dependency                     | **Ubuntu / Debian** | **Fedora**          | **Arch Linux**  |
| ------------------------------ | ------------------- | ------------------- | --------------- |
| **CMake & Make**               | `cmake`, `make`     | `cmake`, `make`     | `cmake`, `make` |
| **Git**                        | `git`               | `git`               | `git`           |
| **C++20 Compiler (GCC/Clang)** | `g++` / `clang`     | `gcc-c++` / `clang` | `gcc` / `clang` |
| **dbus-1 (libdbus-1)**         | `libdbus-1-dev`     | `dbus-devel`        | `dbus`          |
| **OpenSSL**                    | `libssl-dev`        | `openssl-devel`     | `openssl`       |
| **TagLib**                     | `libtag1-dev`       | `taglib-devel`      | `taglib`        |

### **Installing dependecies**

Note that it is advised to update your system before installing:

#### **Ubuntu / Debian**

```bash
sudo apt update && sudo apt install -y \
  cmake make git g++ clang \
  libdbus-1-dev libssl-dev libtag1-dev
```

#### **Fedora**

```bash
sudo dnf install -y \
  cmake make git gcc-c++ clang \
  dbus-devel openssl-devel taglib-devel
```

#### **Arch Linux**

```bash
sudo pacman -S --needed \
  cmake make git gcc clang \
  dbus openssl taglib
```

Then, you can simply run:

```bash
# clone repo
git clone https://github.com/nots1dd/inlimbo

# cd and initialize
cd inlimbo
make init

# note: this is cmake init for release build.
make buildx

# this is for debug build.
make buildx-dbg
```

## **BUILDING**

> [!IMPORTANT]
>
> inLimbo has been **TRIED AND TESTED** in the following GNU/Linux distros:
>
> 1. Arch Linux (x86-64 arch)
> 2. Ubuntu/Debian (x86-64 arch)
> 3. Fedora/Nobara Linux (x86-64 arch)
>
> As long as your distribution can install & build the requirements, there should *NOT* be any issue
>

Check out [BUILD.md](https://github.com/nots1dd/inLimbo/blob/main/BUILD.md) for more options to build targets

## **RUNNING**

Just run:

```bash
./build/inLimbo -h                              # for help

# A few examples
./build/inLimbo query -h                        # to show help for query options
./build/inLimbo query -s                        # to show summary of the library
./build/inLimbo modify -r query -a              # rebuild the library then query library to show all artists

./build/inLimbo -s "Song A" edit -t "Song B"    # Give title 'Song A' and edit the title to 'Song B'

./build/inLimbo -s "Song A" -f "cmdline" -V 100 # Run frontend `cmdline` with initial song `Song A` at 100% volume
```

A LOT of things are possible with your song library with inLimbo. Lots of read only queries at your fingertips, and it unleashes the potential of having locally downloaded music.

> [!IMPORTANT]
>
> Some arguments mentioned maybe outdated so always run `--help` or `-h`
> before running the above!!
>

## **PROJECT STATUS**

Check the draft PR for more [pull/5](https://github.com/nots1dd/inLimbo/pull/5)

## **CONFIG**

The configuration aspect of the inLimbo project comprises of the following fields:

1. Keybinds
2. Colors (Theme)
3. Library Sort
4. Audio backend

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

Check out [example config file](https://github.com/nots1dd/inLimbo/blob/main/examples/config/config.toml)

### **Config 1.1: Library Sorting**

Library sort is a feature in most music players and they do get the job done (sometimes UX maybe a bit scuffed), which is why I felt the need to add one that is fast, reliable and easy to set.

The idea behind the sort is that we can "reorder" the song map based on 3 field members:

1. Artist
2. Album
3. Track

More *could* be added if that is required but these are the most used fields.

Now each field has certain criteria upon which the map is reordered.

1. Artist: 
    - `Lexicographically Ascending/Descending (ASCII only)`
    - `Albums Count Ascending/Descending`
    - `Track Count Ascending/Descending`
2. Album: 
    - `Lexicographically Ascending/Descending (ASCII only)`
    - `Track Count Ascending/Descending`
3. Track: 
    - `Track Number Ascending/Descending`

These are self-explanatory and easy to understand what each sort does. You can check out [example config.toml](https://github.com/nots1dd/inLimbo/blob/main/examples/config/config.toml) for how to customize each sort.

You can set each field as however you desire and `inLimbo-core` is made in such a way that it supports **runtime change** of the sorting "plan".

This isn't to say there won't be more options coming (ex: Artist sorted by `Year Ascending/Descending`, etc.) but those are not a top priority at the moment and with the initial groundwork setup, I make it easy to add new sorting plans to the codebase without too many changes.

If you are wondering how to integrate this into the frontend, I will have a blog / doc coming up on it but for now I suggest looking at the `cmdline` frontend code here: [Frontend Interface (cmdline)](https://github.com/nots1dd/inLimbo/blob/main/src/frontend/cmdline/Interface.cc)

> [!NOTE]
> 
> As of writing this, I did **NOT** find any bugs, but 
> honestly this is a pretty dynamic feature that affects the 
> audio engine's playlist and frontend.
> 
> Am fairly certain editing the sort just as a song ends *may* cause a 
> problem but I could be wrong.
> 

### **Config 1.2: Audio Backend**

For now we can set the backend automatically via `config.toml` like so:

```toml
[audio]
backend = "alsa"
```

Note that audio backends will default to `ALSA` always. All builds will also cater to `ALSA` until I add specific
build options for other options.

I plan on adding `pipewire` for detecting device nodes that are not just audio hardware devices (that `ALSA` can find only) so that in the 
future inLimbo will be capable of changing audio sink within the app itself (and maybe changing other params in the future).


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

## **DOCUMENTATION**

Currently, the only source of docs for understanding the source code are in the header files itself.

I do plan on having atleast a few markdown files explaning the core structure before destroying the codebase
with doxygen comments everywhere.

I will also be writing a detailed blog post on this so hopefully that can also be helpful.

> [!NOTE]
> 
> More docs coming soon!
> 

## **PHILOSOPHY**

The core philosophy of inLimbo can be primarily summarized in:

1. No action is done without the user knowing (well if u read enough)
2. Easy, simple yet powerful to interact with (like a power vim user)
3. Hopefully good code with minimal tech debt lol

But most of all, since my goal with this project is for me to use it myself, I can only guarantee 
my very best effort was given in making this happen.

## **CONTRIBUTING**

The project is yet to be released as stable software, but contributions are much appreciated!

Check out [CONTRIBUTION GUIDE](https://github.com/nots1dd/inLimbo/CONTRIBUTING.md)

## **CREDITS**

**LOGO**:

> **!! DISCLAIMER !!**: This is not an original logo nor was it commisioned by me

It is a basic stock image found online. I got it from [here](https://unsplash.com/photos/black-and-white-decor-FrQKfzoTgsw)

1. **TOML PARSER: TOML++**:

To parse the [config.toml](https://github.com/nots1dd/inlimbo/blob/main/examples/config/config.toml) is thanks to [TOML++](https://github.com/marzer/tomlplusplus)

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

## **FAQ**

1. Will this work on Windows now or in the future?

> I highly doubt it. I myself do not have windows readily available 
> to test and run on (and would not want to install it anyway)
> so realistically, this will remain for *NIX systems only at best.

2. Do I need network capabilities on my PC to build and run inLimbo?

> You do not need network *per se* to use inLimbo, but you will need a stable network for:
> 
> 1. Building (as it fetches required submodules) the project
> 2. Fetching lyrics (either during `query --print-song-lyrics` or in `ftxui`)
> 
> Clearly, not a lot of places where we are using active network connection. Now point number
> 2 is totally optional and inLimbo's `HTTPS Client` will give clean errors anyway so 
> even if you do not have a network connection you can use inLimbo to the fullest, 
> but of course during building it has to find and get submodules to compile.

3. Can I trust inLimbo with my data?

> The only data inLimbo collects are all stored **LOCALLY** only:
> 
> 1. Library cache
> 2. Telemetry data (cached)
> 
> Both of these cache can be modified and deleted by the user
> at their own will and if it wasnt obvious, inLimbo does not
> share this data anyway.

4. Other TUI music players have great album art, why this so trash?

> Ok well firstly this isnt trash #iloveasciiart
> 
> Secondly, I did not find a library or software that supports
> proper image rendering within `FTXUI` - the main TUI for this project.
> 
> I did however, try to make the image as defined as possible without making
> it too pixelated (yes there is a tradeoff) so certain artwork do look
> bad (either very detailed ones or super plain ones suffer the most)
> 
> I could have an option in the future to switch between different
> rendering styles, but it is of low priority imo and most of the art
> already look better than I anticipated initially.
> 
> As for other TUI libraries, they were probably made with the notion
> of using the terminal emulators graphical protocols (like sixel/kitty)
> to get the job done. As far as I am aware, `FTXUI` did not consider it
> which is fine, it is simple and neat, and if you really want to see
> your thumbnail in 4k, change the frontend to `raylib` :) simple as that.


## **LICENSE**

inLimbo is Free and Open Source Software Licensed under [GNU GPL v3](https://github.com/nots1dd/inlimbo/blob/main/LICENSE)
