#!/bin/bash

# Script will copy the files in directory structure desired 
# to open source power profiler device driver code.
# Created directory will be tar and will be 
# used by the installer to install/uninstall power profiler module.

# save the current directory path 
MODULE_SOURCE=`pwd`
DRV_SRC="$MODULE_SOURCE/Components/PowerProfiling/Backend/AMDTPowerProfilingDrivers/Linux/"
COMMON_INC_DIR="$MODULE_SOURCE/Components/PowerProfiling/Backend/AMDTPowerProfilingDrivers/inc/"
COMMON_SRC_DIR="$MODULE_SOURCE/Components/PowerProfiling/Backend/AMDTPowerProfilingDrivers/common/"
LINUX_INC_DIR="$MODULE_SOURCE/Components/PowerProfiling/Backend/AMDTPowerProfilingDrivers/Linux/inc/"
LINUX_SRC_DIR="$MODULE_SOURCE/Components/PowerProfiling/Backend/AMDTPowerProfilingDrivers/Linux/src/"

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

cp $DRV_SRC/Makefile $SOURCE_DIR
cp $DRV_SRC/dkms.conf $SOURCE_DIR
cp $DRV_SRC/CodeXLPwrProfVersion $SOURCE_DIR

# tar the files in current directory
tar -C /tmp/ -zcf /tmp/CodeXLPwrProfDriverSource.tar.gz $MODULE_NAME-$MODULE_VERSION/
echo "CodeXLPwrProfDriverSource.tar.gz created."

# after taring delete the temp file.
rm -rf $SOURCE_DIR 

exit 0
