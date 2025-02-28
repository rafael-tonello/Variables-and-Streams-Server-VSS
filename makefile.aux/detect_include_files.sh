#!/bin/bash

# detect included files in .h and .cpp files. This file should be called by makefile (make command) 
# to list all files in use by a C++ project and also to prevent problems with multiple definitions
# that can occur when working with git submodules.

# Usage:
#   - call this script in the makefile with the following command:
#       $(shell bash detect_include_files.sh "withTheOutputSeparator \n" "withPrefix" "with"...
#
#   - arguments are setted with optional function, which begins with 'with' and receives arguments
#
#   - The optional functions are:
#       - withTheOutputSeparator: set the separator between the files in the output
#       - withPrefix: set the prefix of the files in the output
#       - withSufix: set the sufix of the files in the output
#       - withAlternativeExtension: set the alternative extension of the files to be checked. 
# By default, the script checks for .h and .cpp files. If you want to check for .hpp files, for 
# example, you can set the alternative extension to 'hpp'. If you use this option, all the output 
# files will have its extension changed to the alternative extension.
#       - withCheckOfFilesExistenceAfterExtensionChange: check if the file exists after changing the 
# extension
#       - withNoExclusionOfTestsFolders: include the 'tests' folders in the search of files. By 
# default, tests folders are excluded.

#


this_init(){ 

    for arg in "$@"; do
        #check if arg begins with 'with'
        if [[ $arg == "with"* ]]; then
            #get all before first space
            fName=$(echo $arg | cut -d' ' -f1)
            #get all after first space
            fValue=$(echo $arg | cut -d' ' -f2-)

            #call the function
            eval "$fName \"$fValue\""
        fi
    done

    if [ -z "$this_outputSeparator" ]; then
        this_outputSeparator="\n"
    fi

    if [ -z "$this_excludeTestsFolder" ]; then
        this_excludeTestsFolder=true
    fi

    if [ -z "$this_includeMainHAndCpp" ]; then
        this_includeMainHAndCpp=true
    fi

    if [ -z "$this_checkIfFileExists" ]; then
        this_checkIfFileExists=false
    fi

    if [ -z "$this_excludeFiles" ]; then
        this_excludeFiles=()
    fi

    if [ -z "$this_path" ]; then
        this_path="."
    fi

    this_CppFiles=()
    this_inUseFiles=()

    this_listAllCppFiles
    this_detectIncludedFiles

    #print the used files
    local output=""
    for usedFile in ${this_inUseFiles[@]}; do
        output+=$this_prefix$usedFile$this_sufix$this_outputSeparator
    done

    printf "$output"


}

this_listAllCppFiles(){
    #list all cppFiles (.h and .cpp) in all directories (incluing inside subdirectories). Skip .git and 'tests' folders
    if [ $this_excludeTestsFolder == true ]; then
        this_CppFiles=($(find "$this_path" -type f -name "*.cpp" -o -name "*.h" | grep -v ".git" | grep -v "/tests/"))
    else
        this_CppFiles=($(find "$this_path" -type f -name "*.cpp" -o -name "*.h" | grep -v ".git"))
    fi
}

this_detectIncludedFiles(){
    #for each cppFile
    for cppFile in ${this_CppFiles[@]}; do
        #check if the file is the main.cpp or main.h
        if $this_includeMainHAndCpp; then
            if [[ $cppFile == *"main.cpp" ]] || [[ $cppFile == *"main.h" ]]; then
                this_registerUsedFile "$cppFile"
                continue
            fi
        fi

        #list the included files in the cppFile
        cppFileIncludes=($(grep -oP '#include\s*"\K[^"]+' $cppFile))

        #for each included file found in the #include directive
        for includedFile in $cppFileIncludes; do
            includedFile="/$includedFile"
            
            #check if the included file is in the list of cppFiles and, if true, register it (the check is made scrolling the list of cppFiles - yep, again)
            for cppFile2 in ${this_CppFiles[@]}; do
                if [[ $cppFile2 == *$includedFile ]]; then
                    this_registerUsedFile "$cppFile2"
                    break;
                fi
            done
        done
    done
}

this_registerUsedFile(){ local file="$1"
    #check if the file is already in the list of used files
    if [ ! -z "$this_alternativeExtension" ]; then
        #get $file without extension
        fileNoExtension=$(echo $file | sed 's/\(.*\)\..*/\1/')

        file="$fileNoExtension.$this_alternativeExtension"

        if $this_checkIfFileExists; then
            if [ ! -f "$file" ]; then
                return
            fi
        fi
    fi

    for usedFile in ${this_inUseFiles[@]}; do
        if [[ $usedFile == $file ]]; then
            return
        fi
    done

    #check file in the exclusion list
    for excludedFile in ${this_excludeFiles[@]}; do
        if [[ $file == *$excludedFile ]]; then
            return
        fi
    done

    this_inUseFiles+=("$file")
}


withTheOutputSeparator(){ 
    this_outputSeparator="$1"
}

withPrefix(){ 
    this_prefix="$1"
}

withSufix(){ 
    this_sufix="$1"
}

withAlternativeExtension(){ 
    this_alternativeExtension="$1"
}

withNoMainHAndCpp(){
    this_includeMainHAndCpp=false
}

withCheckOfFilesExistenceAfterExtensionChange(){
    this_checkIfFileExists=true
}

withNoExclusionOfTestsFolders(){
    this_excludeTestsFolder=false
}

withPath(){
    this_path="$1"
}

withExclusionOfTheFiles(){
    #copy the args to an internal array
    this_excludeFiles=("$@")
}


this_init "$@"

