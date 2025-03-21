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


#check if version was passed as argument
if [ -z "$1" ]
  then
    echo "No version number supplied"
    exit 1
fi

#check if there are pending changes
if [ -n "$(git status --porcelain)" ]; then
  echo "There are pending changes, please commit or stash them before running this script"
  exit 1
fi

#ask for confirmation
read -p "Are you sure you want to apply version $1? (y/n) " -n 1 -r

#checkout develop
git checkout develop

#change version number
sed -i "s/string INFO_VERSION = \".*\"/string INFO_VERSION = \"$1\"/g" ./sources/main.cpp

#add changes
git add ./sources/main.cpp

#commit changes
git commit -m "Changes version number to $1"

#push changes
git push origin develop

#checkout main
git checkout main

#merge develop
git merge develop

#push changes
git push origin main

#create tag
git tag $1

#push tag
git push origin $1

#checkout develop
git checkout develop

echo "Version $1 applied successfully"
exit 0
