#!/bin/bash

# Script will copy the files in directory structure desired 
# to open source power profiler device driver code.
# Created directory will be tar and will be 
# used by the installer to install/uninstall power profiler module.

# save the current directory path 
MODULE_SOURCE=`pwd`

# Module Name
MODULE_NAME=amdtPwrProf
# read Module version from ./VERSION file
MODULE_VERSION=$(cat CodeXLPwrProfVersion)
# create a tempaary directory in /tmp/
SOURCE_DIR=/tmp/$MODULE_NAME-$MODULE_VERSION

# source files destination for moodule
mkdir -p $SOURCE_DIR/src
# includes file destination for module
mkdir -p $SOURCE_DIR/inc
mkdir -p $SOURCE_DIR/inc/Smu8Header

# copy
cp -r ../inc/AMDT*.h $SOURCE_DIR/inc/
cp -r ../inc/Smu8Header/AMDT* $SOURCE_DIR/inc/Smu8Header/
cp -r inc/AMDT*.h $SOURCE_DIR/inc/
cp -r ../common/AMDT*.c $SOURCE_DIR/src/
cp -r src/AMDT*.c $SOURCE_DIR/src/
cp Makefile $SOURCE_DIR
cp dkms.conf $SOURCE_DIR
cp CodeXLPwrProfVersion $SOURCE_DIR

# remove debug files
rm -f src/AMDTPwrProfDebugHelper.c 2> /dev/null
rm -f inc/AMDTPwrProfDebugHelper.h 2> /dev/null

# tar the files in current directory
cd /tmp/
tar -zcf $MODULE_SOURCE/CodeXLPwrProfDriverSource.tar.gz $MODULE_NAME-$MODULE_VERSION/
echo "CodeXLPwrProfDriverSource.tar.gz created."
cd $MODULE_SOURCE

# after taring delete the temp file.
rm -rf $SOURCE_DIR 

exit 0
