#!/bin/bash

_inLimbo_completions() {
    local cur prev opts descriptions
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    opts="--help --version --clear-cache --show-config-file --show-log-dir --show-dbus-name"
    descriptions=(
        "--help               Show help information"
        "--version            Display the version of inLimbo"
        "--clear-cache        Clear cached data"
        "--show-config-file   Show the configuration file path"
        "--show-log-dir       Show the log directory path"
        "--show-dbus-name     Show the DBus service name"
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
