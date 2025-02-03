_inLimbo_completions() {
    local -a opts
    opts=(
        "--help:Show help information"
        "--version:Display the version of inLimbo"
        "--clear-cache:Clear cached data"
        "--show-config-file:Show the configuration file path"
        "--show-log-dir:Show the log directory path"
        "--show-dbus-name:Show the DBus service name"
        "--update-cache-run:Update the cache file and run the application"
        "--print-song-tree:Print the Song Map parsed from directory"
        "--print-artists-all:Print all parsed artists from Song Map"
        "--print-songs-by-artist:Print all the songs of a given artist"
        "--print-songs-by-genre-all:Print all parsed genre and their song mappings"
        "--print-song-info:Print every parsed field of a song name / filepath"
        "--socket-info:Gives information on the socket binding of inLimbo"
        "--socket-unlink-force:Forcibly remove the socket binding if present and undesired"
    )
    _describe -t options "inLimbo options" opts
}

compdef _inLimbo_completions inLimbo
