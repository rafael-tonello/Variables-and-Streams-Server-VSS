#!/bin/bash

source ./.shu/packages/common/misc.sh
if [ -z "$1" ]; then
    buildType="--release"
else
    buildType="$1"
fi

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
    #just use a existing make file to clean the project
    make clean
    echo "Project cleaned."
    return $?
#check if is not "--debug" or "--release"
elif [[ "$buildType" != "--debug" && "$buildType" != "--release" && "$buildType" != "--clean" && "$buildType" != "" ]]; then
    echo "Unknown argument '$buildType'. Please use '--debug', '--release', '--tests' or '--clean'." 1>&2
    return 1
fi

#handled by 'shu build debug'
rm -f ./makefile

#read binary name from ./shu.yaml
binaryName=$(shu pprops get "binary-name")
ccFlags=$(shu pprops get "build.cc-flags"); if [ "$ccFlags" == "null" ]; then ccFlags=""; fi
lkFlags=$(shu pprops get "build.lk-flags"); if [ "$lkFlags" == "null" ]; then lkFlags=""; fi


#find compilers
releaseCompiler=clang++
debugCompiler=g++

if ! command -v clang++ &> /dev/null; then
    misc.PrintYellow "clang++ not found. Using g++ to build release.\n"
    releaseCompiler=g++
fi
if ! command -v g++ &> /dev/null; then
    misc.PrintYellow "g++ not found. Using clang++ to build debug.\n"
    debugCompiler=clang++
fi

#get data from shu.yaml (get cpp files from .shu/packages)
echo "Reading additional files and directories from project properties..."
cppfiles=""
while read item; do
    cppfiles+="$item "; 
done < <(shu pprops listarrayitems build.additional-files-and-dirs.cpp.cpp-files)

hfiles=""
while read item; do
    hfiles+="$item "; 
done < <(shu pprops listarrayitems build.additional-files-and-dirs.cpp.h-files)

includeDirs="./.shu/packages/ "
while read item; do
    includeDirs+="$item "; 
done < <(shu pprops listarrayitems build.additional-files-and-dirs.cpp.include-dirs)


#get cppfiles from 'source' folder
echo "Found project cpp files in ./sources folder..."
while read -r item; do
    cppfiles+="$item "
done < <(find "./sources" -type f \( -name "*.cpp" -o -name "*.cxx" \))

while read -r item; do
    hfiles+="$item "

    #extract the directory of the header file
    dir=$(dirname "$item")
    
    #check if $includeDirs already contains the directory
    if [[ ! " $includeDirs " =~ " $dir " ]]; then
        #add the directory to the includeDirs array
        includeDirs+="$dir "
    fi
done < <(find "./sources" -type f \( -name "*.hpp" -o -name "*.h" \))

echo "Preparing makefile for project..."
#copy makefile.template to ./makefile
rm -f ./makefile
cp -f ./shu/pcommands/build/makefile.template ./makefile

#replace placeholders in the makefile:
#   <binname> by the binary name
#   <includePaths> by $includeDirs variable
#   <cppFiles> by $cppfiles variable
#   <hFiles> by $hfiles variable
#   <ccFlags> by $ccFlags variable
#   <lkFlags> by $lkFlags variable
#   <releaseCompiler> by the release compiler from project properties
#   <debugCompiler> by the debug compiler from project properties
sed -i "s|<binname>|$binaryName|g" ./makefile
sed -i "s|<includePaths>|$includeDirs|g" ./makefile
sed -i "s|<cppFiles>|$cppfiles|g" ./makefile
sed -i "s|<hFiles>|$hfiles|g" ./makefile
sed -i "s|<ccFlags>|$ccFlags|g" ./makefile
sed -i "s|<lkFlags>|$lkFlags|g" ./makefile
sed -i "s|<releaseCompiler>|$releaseCompiler|g" ./makefile
sed -i "s|<debugCompiler>|$debugCompiler|g" ./makefile
if [ "$1" == "--debug" ]; then
    make debug -j 4 2>&1 | tee build.log
    retCode=$?
    if [ $retCode -ne 0 ]; then
        echo "Build failed in debug mode." 1>&2
        return $retCode
    fi
    rm -f makefile

    echo "Project built in debug mode."
    return $?
elif [ "$buildType" == "--clean" ]; then
    make clean
    rm -f makefile
    echo "Project cleaned."
    return $?
else
    make all -j 4 | tee build.log
    retCode=$?
    if [ $retCode -ne 0 ]; then
        echo "Build failed in release mode." 1>&2
        return $retCode
    fi
    rm -f makefile

    echo "Project built in release mode."
    return $?
fi
