#!/bin/bash

# Script will copy the files in directory structure desired 
# to open source power profiler device driver code.
# Created directory will be tar and will be 
# used by the installer to install/uninstall power profiler module.

# save the current directory path 
MODULE_SOURCE="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
DRV_SRC=$MODULE_SOURCE
COMMON_INC_DIR="$MODULE_SOURCE/../inc/"
COMMON_SRC_DIR="$MODULE_SOURCE/../common/"
LINUX_INC_DIR="$MODULE_SOURCE/inc/"
LINUX_SRC_DIR="$MODULE_SOURCE/src/"

# Module Name
MODULE_NAME=amdtPwrProf
# read Module version from ./VERSION file
MODULE_VERSION=$(cat $DRV_SRC/CodeXLPwrProfVersion)
# create a tempaary directory in /tmp/
SOURCE_DIR=/tmp/$MODULE_NAME-$MODULE_VERSION

# source files destination for moodule
mkdir -p $SOURCE_DIR/src
# includes file destination for module
mkdir -p $SOURCE_DIR/inc
mkdir -p $SOURCE_DIR/inc/Smu8Header

# copy
cp -r $COMMON_INC_DIR/AMDT*.h $SOURCE_DIR/inc/
cp -r $COMMON_INC_DIR/Smu8Header/AMDT* $SOURCE_DIR/inc/Smu8Header/
cp -r $LINUX_INC_DIR/AMDT*.h $SOURCE_DIR/inc/
cp -r $COMMON_SRC_DIR/AMDT*.c $SOURCE_DIR/src/
cp -r $LINUX_SRC_DIR/AMDT*.c $SOURCE_DIR/src/

# add additional files for internal or nda build
if [ "$1" != "PUBLIC" ] ; then
    echo $1
    cp -r $DRV_SRC/../Non-OpenSource/AMDT*.c $SOURCE_DIR/src/
    cp -r $DRV_SRC/../Non-OpenSource/AMDT*.h $SOURCE_DIR/inc/
fi

files=""
for f in $SOURCE_DIR/src/*
do
    filename=$(basename "$f")
    files=$files" src\/"${filename%.*}"\.o"
done

# create temp Makefile and fill the object file names for 
# internal  or public object files 
cp $DRV_SRC/Makefile /tmp/Makefile.tmp
sed  -i "s/#FILE_NAME_OBJS#/${files}/g" /tmp/Makefile.tmp

cflags=""
if [ "$1" != "PUBLIC" ] ; then
    cflags="-DAMDT_INTERNAL_COUNTERS"
fi

sed  -i "s/#WRITE_CFLAGS#/${cflags}/g" /tmp/Makefile.tmp

cp /tmp/Makefile.tmp  $SOURCE_DIR/Makefile
cp $DRV_SRC/dkms.conf $SOURCE_DIR
cp $DRV_SRC/CodeXLPwrProfVersion $SOURCE_DIR

if [ -e /tmp/CodeXLPwrProfDriverSource.tar.gz ] ; then
    echo "Deleting stale CodeXLPwrProfDriverSource.tar.gz "
    rm /tmp/CodeXLPwrProfDriverSource.tar.gz
fi

# tar the files in current directory
tar -C /tmp/ -zcf /tmp/CodeXLPwrProfDriverSource.tar.gz $MODULE_NAME-$MODULE_VERSION/
echo "Created CodeXLPwrProfDriverSource.tar.gz."

# delete the temp file.
rm -rf $SOURCE_DIR 
rm -rf /tmp/Makefile.tmp

exit 0
