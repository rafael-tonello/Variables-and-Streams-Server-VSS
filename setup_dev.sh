#!/bin/bash

setupdev.main(){
    setupdev.installGirHooks
    if [ -z "$_error" ]; then
        echo "Error: $_error"
        return 1
    fi
    echo "Development environment setup completed successfully."
    return 0
}

setupdev.installGirHooks(){
    local commit_msg_file="./devtools/hooks/commit-msg"
    if [ ! -f "$commit_msg_file" ]; then
        _error="Commit message hook file not found: $commit_msg_file"
        return 0
    fi
    rm -f .git/hooks/commit-msg
    cp "$commit_msg_file" .git/hooks/commit-msg
    chmod +x .git/hooks/commit-msg
    echo "Git commit message hook installed successfully."
    _error=0
    return 0
}

setupdev.main "$@"
exit $?
