_inLimbo_completions() {
    local -a opts
    opts=(
        "--help:Show help information"
        "--version:Display the version of inLimbo"
        "--clear-cache:Clear cached data"
        "--show-config-file:Show the configuration file path"
        "--show-log-dir:Show the log directory path"
        "--show-dbus-name:Show the DBus service name"
    )
    _describe -t options "inLimbo options" opts
}

compdef _inLimbo_completions inLimbo
