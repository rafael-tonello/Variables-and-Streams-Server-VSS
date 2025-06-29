#!/bin/bash

folderToScan="$1"
shucmd="$SHU_BINARY"

#scan cpp files
find "$folderToScan" -type f \( -name "*.cpp" -o -name "*.cxx" \) | while read -r file; do
    #ignore folders if /tests/ or 'main.cpp' file
    if [[ "$file" == *"/test"* ]] || [[ "$(basename "$file")" == "main.cpp" ]]; then
        continue
    fi

    shu.Main pprops addarrayitem "build.additional-files-and-dirs.cpp.cpp-files" "$file" >/dev/null
    echo "Added C++ file '$file' to the project."
done


includeDirs=();
#scan .h and .hpp files
find "$folderToScan" -type f \( -name "*.h" -o -name "*.hpp" \) | while read -r file; do
    #ignore folders if /tests/
    if [[ "$file" == *"/test"* ]] || [[ "$(basename "$file")" == "main.h" ]]; then
        continue
    fi

    shu.Main pprops addarrayitem "build.additional-files-and-dirs.cpp.h-files" "$file" >/dev/null
    echo "Added header file '$file' to the project"

    #extract the directory of the header file
    dir=$(dirname "$file")
    #check if $includeIdrs already contains the directory
    if [[ ! " ${includeDirs[@]} " =~ " $dir " ]]; then
        #add the directory to the includeDirs array
        includeDirs+=("$dir")
        #add the directory to the project properties
        shu.Main pprops addarrayitem "build.additional-files-and-dirs.cpp.include-dirs" "$dir" >/dev/null
        echo "Added include directory '$dir' to the project"
    fi
done

