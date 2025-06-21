#!/bin/bash

#this script does the following:
#1. Check if there are pending git changes, if there are, the scripts will fail
#3. Execute git checkout develop
#4. change the version number in the main.cpp file (looks for the line starting with 'string INFO_VERSION' and change it to 'string INFO_VERSION = "<version>"')
#5. execute git add ./sources/main.cpp
#6. execute git commit -m "Changes version number to <version>"
#7. execute git push origin develop
#8. execute git checkout main
#9. execute git merge develop
#10. execute git push origin main
#11. execute git tag <version>
#12. execute git push origin <version>
version="$1"

tryCreateVersion(){
    local lastTag=""
    local lastVersion=""
    local version="$1"
    local addInfo="$2"
    local major=0
    local minor=0
    local patch=0
    #get last git tag
    lastTag=$(git describe --tags --abbrev=0 2>/dev/null)
    lastVersion="$lastTag"
    #check if contains '+', if true, separate version from aditional info
    if [[ "$lastVersion" == *"+"* ]]; then
        addInfo="${lastVersion#*+}"  #everything after the first '+' character
        lastVersion="${lastVersion%%+*}"  #remove everything after the first '+' character
    fi

    #with the version, removes the v (if present) fro the beginning of the string
    lastVersion="${lastVersion#v}"  #remove 'v' from the beginning of the string
    #check if lastTag is empty
    if [ -z "$lastVersion" ]; then
        echo "No previous version found, using 0.0.0 as base version"
        lastVersion="0.0.0"
    fi

    #separate major, minor and patch version numbers from lastVersion
    major=$(echo "$lastVersion" | cut -d '.' -f 1)
    minor=$(echo "$lastVersion" | cut -d '.' -f 2)
    patch=$(echo "$lastVersion" | cut -d '.' -f 3)

    INCREMENT_MAJOR=false
    INCREMENT_MINOR=false
    INCREMENT_PATCH=false

    #scrolls commits from lastVersion to the last one
    local commits=$(git log --pretty=format:"%H" "$lastTag"..HEAD)
    for commit in $commits; do
        local message=$(git log -1 --pretty=%B $commit)

        if echo "$message" | grep -q "BREAKING CHANGE"; then
            INCREMENT_MAJOR=true
        elif echo "$message" | grep -q "^feat"; then
            INCREMENT_MINOR=true
        elif echo "$message" | grep -qE "^(fix|chore|patch)"; then
            INCREMENT_PATCH=true
        fi
    done
    #increment version numbers
    local razon=""
    if [ "$INCREMENT_MAJOR" = true ]; then
        razon="Commits with BREAKING CHANGE"
        major=$((major + 1))
        minor=0
        patch=0
    elif [ "$INCREMENT_MINOR" = true ]; then
        razon="Commit(s) starting with 'feat'"
        ehco "inc minor"
        minor=$((minor + 1))
        patch=0
    elif [ "$INCREMENT_PATCH" = true ]; then
        razon="Commit(s) starting with 'fix'"
        echo "inc patch"
        patch=$((patch + 1))
    fi

    #create new version string
    local newVersion="$major.$minor.$patch"
    if [ -n "$addInfo" ]; then
        newVersion="v$newVersion+$addInfo"
    fi

    echo "You didn't specify a version number, so a new one was created based on the last tag:"
    echo "   Last tag: $lastTag"
    echo "   New tag (with version): $newVersion"
    if [ -n "$razon" ]; then
        echo "   Reason for incrementing: $razon"
    fi

    echo "   Commits:"
    for commit in $commits; do
        local message=$(git log -1 --pretty=%B $commit)
        echo "      - $message"
    done
    echo "Do you want to use this version? (y/n)"

    read -r answer1
    if [[ "$answer1" =~ ^[Yy]$ ]]; then
        _error=""
        _r="$newVersion"
        return 0
    else
        _error="User declined to use the automatically generated version"
        _r=""
        return 1
    fi

    
}

#check if version was passed as argument
if [ -z "$version" ]; then
    echo "No version number supplied"
    tryCreateVersion
    if [ -n "$_error" ]; then
        echo "Error: $_error"
        exit 1
    fi
    version="$_r"
fi

#check if there are pending changes
if [ -n "$(git status --porcelain)" ]; then
    echo "There are pending changes, please commit or stash them before running this script"
    exit 1
fi

#ask for confirmation
read -p "Are you sure you want to apply version $version? (y/n) " -r answer
if [[ ! "$answer" =~ ^[Yy]$ ]]; then
    echo -e "\nOperation cancelled."
    exit 0
fi

#checkout develop
git checkout develop

#change version number
sed -i "s/string INFO_VERSION = \".*\"/string INFO_VERSION = \"$version\"/g" ./sources/main.cpp

#add changes
git add ./sources/main.cpp

#commit changes
git commit -m "Changes version number to $version"

#push changes
git push origin develop

#checkout main
git checkout main

#merge develop
git merge develop

#push changes
git push origin main

#create tag
git tag $version

#push tag
git push origin $version

#checkout develop
git checkout develop

echo "Version $version applied successfully"
exit 0
