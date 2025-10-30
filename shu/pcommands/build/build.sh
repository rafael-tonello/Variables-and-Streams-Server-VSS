#!/bin/bash

source ./.shu/packages/common/misc.sh
if [ -z "$1" ]; then
    buildType="--release"
else
    buildType="$1"
fi


main(){
    lastBuildType=$(shu pvars get "lastBuildMode")

    #checks if the last build type is different from the current build type and 'buildType' is not --tests or --clean
    if [[ "$lastBuildType" != "$buildType" && "$buildType" != "--tests" && "$buildType" != "--clean" ]]; then
        echo "Last build type '$lastBuildType' is different from current build type '$buildType'. Cleaning previous build..."
        rm -rf ./build
        shu pvars set "lastBuildMode" "$buildType"
    fi


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



    mkdir -p ./build

    # copy assets, excluding .so files
    find ./sources/assets -type f ! -name '*.so' -exec cp --parents {} ./build/ \;


    provideLoggerSo

    go build -o ./build/vss ./main.go
    retCode=$?
    if [ $retCode -ne 0 ]; then
        echo "Build failed."
        return $retCode
    fi

    echo "Project built in $buildType mode."
    return $retCode

}

provideLoggerSo(){
    if [ -f ./build/logger.so ]; then return 0; fi

    local needsBuild=false

    #check if current architecture is x86_64
    arch=$(uname -m)
    echo "Current architecture: $arch"
    if [ "$arch" == "x86_64" ]; then
        #check if file ./sources/assets/logger.x86_64.so exists
        pwd
        echo "looking for ./sources/assets/logger.x86_64.so"
        if [ -f ./sources/assets/logger.x86_64.so ]; then
            cp ./sources/assets/logger.x86_64.so ./build/logger.so
            return 0
        fi
        needsBuild=true
    elif [ "$arch" == "arm64" ] || [ "$arch" == "aarch64" ]; then
        #check if file ./sources/assets/logger.arm64.so exists
        if [ -f ./sources/assets/logger.arm64.so ]; then
            cp ./sources/assets/logger.arm64.so ./build/logger.so
            return 0
        fi
        needsBuild=true
    else
        needsBuild=true
    fi

    if [ "$needsBuild" = true ]; then
        local currDir=$(pwd)

        cd /tmp
        rm -rf logger
        git clone https://github.com/rafael-tonello/Logger.git --recursive logger
        cd ./logger/projects/library

        echo "Building logger.so for architecture $arch..."
        #use tee to print but capture only stderr to a file
        nice -n 19 make all -j 4 | tee build.log
        retCode=$?
        if [ $retCode -ne 0 ]; then
            #get error lines from build.log
            for line in $(cat build.log); do
                if [[ "$line" == *"error:"* ]]; then
                    echo "$line" >> builderr.log
                fi
            done

            _error="Building Logger library failed: $(cat builderr.log )"
            cd "$currDir"
            return $retCode
        fi

        cp ./build/logger.so "$currDir/build/logger.so"
        cd "$currDir"
    fi
    return 0

}

main "$@"