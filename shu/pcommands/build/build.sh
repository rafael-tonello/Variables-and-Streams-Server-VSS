#!/bin/bash

source ./.shu/packages/common/misc.sh
if [ -z "$1" ]; then
    buildType="--release"
else
    buildType="$1"
fi


main(){
    lastBuildType=$(shu pvars get "lastBuildMode")


    if [ "$buildType" == "--tests" ] ||  [ "$buildType" == "tests" ]; then
        source ./shu/pcommands/build/buildTestsProject.sh
        return $?
    elif [[ "$buildType" == "--clean" && -f makefile ]]; then
        rm -rf ./build
        echo "Project cleaned."
        return $?
    #check if is not "--debug" or "--release"
    elif [[ "$buildType" != "--debug" && "$buildType" != "--release" && "$buildType" != "--clean" && "$buildType" != "" ]]; then
        echo "Unknown argument '$buildType'. Please use '--debug', '--release', '--tests' or '--clean'." 1>&2
        return 1
    fi

    rm -rf ./build/vss



    mkdir -p ./build

    # copy assets, excluding .so files
    find ./sources/assets -type f ! -name '*.so' -exec cp --parents {} ./build/ \;

    go build -o ./build/vss ./main.go
    retCode=$?
    if [ $retCode -ne 0 ]; then
        echo "Build failed."
        return $retCode
    fi

    echo "Project built in $buildType mode."
    return $retCode

}

main "$@"