#!/bin/bash

_inLimbo_completions() {
    local cur prev opts
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    opts="--help --version --clear-cache --show-config-file --show-log-dir --show-dbus-name"

    # Provide completion for options
    if [[ ${cur} == -* ]]; then
        COMPREPLY=( $(compgen -W "${opts}" -- "${cur}") )
        return 0
    fi
}

complete -F _inLimbo_completions inLimbo
