#!/bin/bash

echo "sourcing misc"

source ./.shu/packages/shu-common/misc.sh

#TODO: scans the added packaged for .h and .cpp files and adds its pathes to the shu.yaml file. This files will be used by 'build' and 'debug' scripts

misc.printYellow "New package added to the project. Scanning for .h and .cpp files in the project directory...\n"

folderToScan="$SHU_LAST_DEP_GET_FOLDER"
shucmd="$SHU_BINARY"

"$shucmd" scancppfiles "$folderToScan"
