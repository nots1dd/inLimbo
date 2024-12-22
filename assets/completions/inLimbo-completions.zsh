# inLimbo-completion.zsh
# A very primitive and early conf for inLimbo completions for zsh

_inLimbo_completions() {
    local cur prev opts

    # Get the current word being typed
    cur="${words[$CURRENT]}"
    prev="${words[$CURRENT-1]}"

    # List of options for the music player
    opts="--play --pause --stop --next --prev --shuffle --repeat --volume --help"

    # If the current word starts with a dash (option), complete the options
    if [[ "$cur" == --* ]]; then
        COMPREPLY=($(compgen -W "$opts" -- "$cur"))
        return 0
    fi

    # If the previous word was an option that expects a file path, suggest file paths
    if [[ "$prev" == "--play" || "$prev" == "--next" || "$prev" == "--prev" ]]; then
        COMPREPLY=($(compgen -f -- "$cur"))
        return 0
    fi
}

# Register the completion for inLimbo using complete
complete -F _inLimbo_completions inLimbo

