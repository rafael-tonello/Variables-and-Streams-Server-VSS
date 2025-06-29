#!/bin/bash

echo "Check if the current directory is a git repository..."
if [ ! -d .git ]; then
    echo "GIt hooks cannot be installed because the current directory is not a git project."
    return 1
fi

echo installing git hooks:
echo "    installing commit-msg hook"
cp ./shu/hooks/git/commit-msg .git/hooks/commit-msg 2> /tmp/shu-git-hooks-error.log
if [ $? -ne 0 ]; then
    echo "Error installing commit-msg hook: $(cat /tmp/shu-git-hooks-error.log)" >&2
    return 1
fi

echo "Git hooks installed with success."
