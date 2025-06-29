#!/bin/bash

echo "touch called with args: $@"
cd "$SHU_PROJECT_WORK_DIR"

className=${1^}
fName=$(echo "${1,,}").h
fName2=$(echo "${1,,}").cpp
nameUpper=${1^^}
includeName=__${nameUpper}__H__

echo "#ifndef ${includeName}"> $fName
echo "#define ${includeName} ">> $fName
echo "">> $fName
echo "class $className{">> $fName
echo "public:">> $fName
echo "    $className();">> $fName
echo "    ~$className();">> $fName
echo "};">> $fName
echo "">> $fName
echo "#endif">> $fName


echo "#include \"$fName\"">> $fName2
echo "">> $fName2
echo "$className::$className()">> $fName2
echo "{">> $fName2
echo "    ">> $fName2
echo "}">> $fName2
echo "">> $fName2

echo "$className::~$className()">> $fName2
echo "{">> $fName2
echo "    ">> $fName2
echo "}">> $fName2

echo "Generated files: $(pwd)/$fName and $(pwd)/$fName2"
exit 1

