#!/bin/bash

# This script will build the current project target in scons.
# you can pass any other scons parmaters here
commandLineArgs=$*
projectname=${PWD##*/}
echo "project folder = ${projectname}"

if [ -z "$AMD_CODEXL" ]; then
        # Absolute path to build script, e.g. $workspace/main/CodeXL/Util/linux/buildCodeXLFullLinuxProjects.sh
        BUILDSCRIPT=$(readlink -f "$0")
        BASEFOLDER=$(dirname "$BUILDSCRIPT")
        MAINFOLDER=$(basename ${BASEFOLDER})
        while [ ! ${MAINFOLDER} = "main" ]
                do
                BASEFOLDER=${BASEFOLDER%/*}
                MAINFOLDER=$(basename ${BASEFOLDER})
        done
        # Set AMD_CODEXL folder path in current workspace
        AMD_CODEXL=$(readlink -e "${BASEFOLDER}/CodeXL/")
fi
cd ${AMD_CODEXL}/Util/linux
./buildCodeXLFullLinuxProjects ${commandLineArgs} ${projectname}
# Go back to project folder
cd ${BASEFOLDER}
