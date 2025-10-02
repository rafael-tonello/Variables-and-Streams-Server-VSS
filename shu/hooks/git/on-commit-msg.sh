#!/bin/bash

COMMIT_MSG_FILE="$1"
COMMIT_MSG=$(cat "$COMMIT_MSG_FILE")


if grep -qE '^Merge ' "$COMMIT_MSG_FILE"; then
    return 0
fi


if ! echo "$COMMIT_MSG" | grep -qE '^(feat|fix|patch|chore|docs|refactor|test|perf|ci|build|revert|style|BREAKING CHANGE):'; then
    echo "❌ wrong commit message format."
    echo "Use one of the following prefixes:"
    echo "  - feat: for new features"
    echo "  - fix: for bug fixes"
    echo "  - patch: for small bug fixes or improvements"
    echo "  - chore: for maintenance tasks"
    echo "  - docs: for documentation changes"
    echo "  - refactor: for code refactoring"
    echo "  - test: for adding or updating tests"
    echo "  - perf: for performance improvements"
    echo "  - ci: for continuous integration changes"
    echo "  - build: for build system changes"
    echo "  - revert: for reverting changes"
    echo "  - style: for code style changes (formatting, missing semi-colons, etc.)"
    echo "  - BREAKING CHANGE: for changes that break backward compatibility"
    echo "Example: 'feat: Add new user authentication feature'"
    
    return 1
fi

return 0
