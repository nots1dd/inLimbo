# A very primitive and basic completion system for inLimbo for bash

_inLimbo_completions() {
    local cur prev opts

    # Get the current word being typed
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"

    # List of options for the music player
    opts="--play --pause --stop --next --prev --shuffle --repeat --volume --help"

    # If the previous word is an option, suggest further options
    if [[ "$cur" == -* ]]; then
        COMPREPLY=($(compgen -W "$opts" -- "$cur"))
        return 0
    fi

    # If the previous word is a command that expects a file path, suggest file paths
    if [[ "$prev" == "--play" || "$prev" == "--next" || "$prev" == "--prev" ]]; then
        COMPREPLY=($(compgen -f -- "$cur"))
        return 0
    fi
}

# Attach the completion function to inLimbo
complete -F _inLimbo_completions inLimbo
