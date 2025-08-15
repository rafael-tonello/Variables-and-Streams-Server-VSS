#!/bin/bash

COMMIT_MSG_FILE="$1"
COMMIT_MSG=$(cat "$COMMIT_MSG_FILE")



checkMain(){
    #if commit to main, runs ./shu/tools/apply-or-create-new-version.sh
    if [ "$(git rev-parse --abbrev-ref HEAD)" = "main" ]; then
        #source ./shu/tools/apply-or-create-new-version.sh
        shu new-version
    fi
}
checkMain
return 0
