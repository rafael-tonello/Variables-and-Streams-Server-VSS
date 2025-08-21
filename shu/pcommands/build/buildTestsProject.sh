#!/bin/bash

source ./.shu/packages/common/misc.sh
cd tests
if [[ "$1" == "--clean" && -f makefile ]]; then
    #just use a existing make file to clean the project
    make clean
    echo "Project cleaned."
    cd ..
    return $?
fi

#handled by 'shu build debug'
rm -f ./makefile

#read binary name from ./shu.yaml
binaryName=tests
ccFlags="-D__TESTING__"
ccFlags="$ccFlags $(shu pprops get "build.cc-flags")"
lkFlags="-D__TESTING__"
lkFlags="$lkFlags $(shu pprops get "build.lk-flags")"


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
    cppfiles+="../$item "; 
done < <(shu pprops listarrayitems build.additional-files-and-dirs.cpp.cpp-files)

hfiles=""
while read item; do
    hfiles+="../$item "; 
done < <(shu pprops listarrayitems build.additional-files-and-dirs.cpp.h-files)

includeDirs="../sources ../.shu/packages/ "
while read item; do
    includeDirs+="../$item "; 
done < <(shu pprops listarrayitems build.additional-files-and-dirs.cpp.include-dirs)

#get cppfiles from 'source' folder
echo "Found project cpp files in ./sources folder..."
while read -r item; do
    #if is 'main.cpp', skip it
    if [[ "$item" != *"main.cpp" ]]; then
        cppfiles+="$item "
    fi
done < <(find "../sources" -type f \( -name "*.cpp" -o -name "*.cxx" \))

#get cppfiles from 'tests' folder
echo "Found project cpp files in ./tests folder..."
while read -r item; do
    cppfiles+="$item "
done < <(find "./sources" -type f \( -name "*.cpp" -o -name "*.cxx" \))

#main project header files
while read -r item; do
    #if is main.h, skip it
    if [[ "$item" != *"main.h" ]]; then
        hfiles+="$item "
    fi


    #extract the directory of the header file
    dir=$(dirname "$item")
    
    #check if $includeDirs already contains the directory
    if [[ ! " $includeDirs " =~ " $dir " ]]; then
        #add the directory to the includeDirs array
        includeDirs+="$dir "
    fi
done < <(find "../sources" -type f \( -name "*.hpp" -o -name "*.h" \))

#tests header files
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

echo "Preparing makefile for tests..."
#copy makefile.template to ./makefile
rm -f ./makefile
cp -f ../shu/pcommands/build/tests.makefile.template ./makefile

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
if [ "$1" == "--clean" ]; then
    make clean
    rm -f makefile
    echo "Project cleaned."
    cd ..
    return $?
fi
make debug -j 4
retCode=$?
if [ $retCode -ne 0 ]; then
    echo "Building tests failed." 1>&2
    return $retCode
fi
rm -f makefile

echo "Tests built with debug symbols."
cd ..
return $?
