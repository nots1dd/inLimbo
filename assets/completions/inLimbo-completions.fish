function _inLimbo_completions
    set -l cur (commandline -ct)
    set -l opts \
        "--help" "Show help information" \
        "--version" "Display the version of inLimbo" \
        "--clear-cache" "Clear cached data" \
        "--show-config-file" "Show the configuration file path" \
        "--show-log-dir" "Show the log directory path" \
        "--show-dbus-name" "Show the DBus service name" \
        "--update-cache-run"   "Update the cache file and run the application"
    for i in (seq 1 2 (count $opts))
        set -l opt (string trim -- $opts[$i])
        set -l desc $opts[(math $i + 1)]
        if test -z "$cur"
            echo -e "$opt\t$desc\n"
        else if string match -q -- "$cur*" $opt
            echo -e "$opt\t$desc\n"
        end
    end
end
complete -f -c inLimbo -a "(_inLimbo_completions)"
