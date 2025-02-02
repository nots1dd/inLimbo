#!/bin/bash

_inLimbo_completions() {
    local cur prev opts descriptions
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    opts="--help --version --clear-cache --show-config-file --show-log-dir --show-dbus-name"
    descriptions=(
        "--help                     Show help information"
        "--version                  Display the version of inLimbo"
        "--clear-cache              Clear cached data"
        "--show-config-file         Show the configuration file path"
        "--show-log-dir             Show the log directory path"
        "--show-dbus-name           Show the DBus service name"
        "--update-cache-run         Update the cache file and run the application"
        "--print-song-tree          Print the Song Map parsed from directory"
        "--print-artists-all        Print all parsed artists from Song Map"
        "--print-songs-by-artist    Print all the songs of a given artist"
        "--print-songs-by-genre-all Print all parsed genre and their song mappings"
        "--print-song-info          Print every parsed field of a song name / filepath"
    )

    # Provide completion for options
    if [[ ${cur} == -* ]]; then
        COMPREPLY=()
        for opt in ${opts}; do
            if [[ ${opt} == ${cur}* ]]; then
                COMPREPLY+=("${opt}")
            fi
        done
        return 0
    fi
}

complete -F _inLimbo_completions inLimbo
