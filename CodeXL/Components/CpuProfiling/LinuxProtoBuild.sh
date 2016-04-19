#!/bin/sh
# Impromptu script to build the Linux version of the CpuProfiling component of CodeXL
# and turn it into a compressed tarball.

# This is undoubtedly wrong, but it is a starter...

# Invocation: build.sh <workspace path> <build number>
# Both are mandatory


# Note: we hard code the API Major and Minor number knowledge into this script
export V_MAJOR=0
export V_MINOR=5

if [ $# -ne 2 ]
then
    echo "Invocation: $0 <workspace path> <build number>"
    echo "None of the options are optional, spaces are NOT permitted"
    exit 1
fi

export WORKSPACE=$1
export BUILDNUM=$2

# Step 1 - verify that the requisite utilities are visible within our environment

# Verify that tar exists and is in our path
export TAR=`which tar`
if [ "xx${TAR}xx" = "xxxx" ]
then
    echo "Cannot find 'tar' in your path.  Fix your path"
    echo "aborting"
    exit 1
fi

# Verify that gzip exists and is in our path
# Whoa!  gzip uses the GZIP environment variable within itself, so invoking ${GZIP} results
# in very strange behavior - "gzip: /usr/bin/gzip is not a directory or a regular file - ignored"
# So use GZIP_TOOL for the locator.
export GZIP_TOOL=`which gzip`
if [ "xx${GZIP_TOOL}xx" = "xxxx" ]
then
    echo "Cannot find 'gzip' in your path.  Fix your path"
    echo "aborting"
    exit 1
fi

# Verify that date exists and is in our path
export DATE=`which date`
if [ "xx${DATE}xx" = "xxxx" ]
then
    echo "Cannot find 'date' in your path.  Fix your path"
    echo "aborting"
    exit 1
fi

# Verify that cp exists and is in our path
export CP=`which cp`
if [ "xx${CP}xx" = "xxxx" ]
then
    echo "Cannot find 'cp' in your path.  Fix your path"
    echo "aborting"
    exit 1
fi

# Verify that mkdir exists and is in our path
export MKDIR=`which mkdir`
if [ "xx${MKDIR}xx" = "xxxx" ]
then
    echo "Cannot find 'mkdir' in your path.  Fix your path"
    echo "aborting"
    exit 1
fi

# Verify that rm exists and is in our path
export RM=`which rm`
if [ "xx${RM}xx" = "xxxx" ]
then
    echo "Cannot find 'rm' in your path.  Fix your path"
    echo "aborting"
    exit 1
fi

# Verify that scons exists and is in our path
export SCONS_PATH=`which scons`
if [ "xx${SCONS_PATH}xx" = "xxxx" ]
then
    echo "Cannot find scons in your path.  Is it installed?"
    echo "aborting"
    exit 1
fi

if [ ! -e $WORKSPACE ]; then
    echo "ERROR: WORKSPACE is set to a non-existent directory: $WORKSPACE"
    exit
fi


export CPUPROF_TOP=${WORKSPACE}/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling

export BUILD_RESULT
export BUILD_INVOKE

# effenctively a boolean.  If zero, the build step failed
export BUILD_SUCCESS=1

# First: build the x86_64 versions in both proto (debug/x86_64) mode
# Be a bit paranoid - do cleans each time first

# throughput Optimization: multiple builds at once
export NCPUS
NCPUS=`grep processor /proc/cpuinfo | wc | awk '{print $1}'`
if [ $? -ne 0 ]
then
    echo "Warning - Unable to determine the number of CPU cores - assuming one core"
    NCPUS=1
fi
# NCPUS=`expr ${NCPUS} \* 2`
echo "Building ${NCPUS} at a time"

cd ${CPUPROF_TOP};

# 
for i in proto
do
    for j in x86_64     # redundant - proto means 64-bit
    do
        # clean
        BUILD_INVOKE="${SCONS_PATH} -j ${NCPUS} -c build=${i} arch=${j}"
        echo "scons build command: [${BUILD_INVOKE}]"
        ${BUILD_INVOKE}
        BUILD_RESULT=$?
        if [ ${BUILD_RESULT} -ne 0 ]
        then
            echo "Error: Unable to perform clean: [${BUILD_INVOKE}]"
            BUILD_SUCCESS=0
            exit 1
        fi

        # build
        BUILD_INVOKE="${SCONS_PATH} -j ${NCPUS} build=${i} arch=${j}"
        echo "scons build command: [${BUILD_INVOKE}]"
        ${BUILD_INVOKE}
        BUILD_RESULT=$?
        if [ ${BUILD_RESULT} -ne 0 ]
        then
            echo "Error: Unable to cleanly build: [${BUILD_INVOKE}]"
            BUILD_SUCCESS=0
            exit 1
        fi
    done
done

# At this point, if the build failed, exit
if [ ${BUILD_SUCCESS} -eq 0 ]
then
    echo "The build failed, unable to continue further"
    exit 1
fi

echo "I don't know what to package or how to package it yet.  Exiting cleanly"
exit 0


# Now to construct the package contents in a reasonable layout
# Use a scratch directory: LinuxTarDir to assemble it all
# Use fully qualified location so we can cd into it
export RELEASEDIR=${CURPROF_TOP}/LinuxTarDir/

export RM_INVOKE="${RM} -rf ${RELEASEDIR}"
${RM_INVOKE}
RM_RESULT=$?
if [ ${RM_RESULT} -ne 0 ]
then
    echo "Unable to build final target directory: [${RM_INVOKE}]"
    exit 1
fi

export MKDIR_INVOKE="${MKDIR} -p ${RELEASEDIR}/Include ${RELEASEDIR}/Lib/x86 ${RELEASEDIR}/Lib/x86_64"
${MKDIR_INVOKE}
MKDIR_RESULT=$?
if [ ${MKDIR_RESULT} -ne 0 ]
then
    echo "Unable to build final target directory: [${MKDIR_INVOKE}]"
    exit 1
fi

# Copy the API doc and the readme
export CP_INVOKE
CP_INVOKE="${CP} AMDOpenCLDebug.doc readme.txt ${RELEASEDIR}"
${CP_INVOKE}
CP_RESULT=$?
if [ ${CP_RESULT} -ne 0 ]
then
    echo "Unable to copy files: [${CP_INVOKE}]"
    exit 1
fi

# Copy the OpenCLDebugAPI Includes: AMDOpenCLDebug.h, CLIntercept.h cl_icd_amd.h CLDispatchTable.h
CP_INVOKE="${CP} Include/AMDOpenCLDebug.h Include/CLIntercept.h Include/cl_icd_amd.h Include/CLDispatchTable.h ${RELEASEDIR}/Include"
${CP_INVOKE}
CP_RESULT=$?
if [ ${CP_RESULT} -ne 0 ]
then
    echo "Unable to copy files: [${CP_INVOKE}]"
    exit 1
fi

# Copy the ShaderDebugger Includes: CLWrappers.h
CP_INVOKE="${CP} ../ShaderDebugger/Include/CLWrappers.h ${RELEASEDIR}/Include"
${CP_INVOKE}
CP_RESULT=$?
if [ ${CP_RESULT} -ne 0 ]
then
    echo "Unable to copy files: [${CP_INVOKE}]"
    exit 1
fi

# Release build bits
# Copy the 32-bit library
CP_INVOKE="${CP} bin/Release-Linux32/libAMDOpenCLDebugAPI32.so ${RELEASEDIR}/Lib/x86"
${CP_INVOKE}
CP_RESULT=$?
if [ ${CP_RESULT} -ne 0 ]
then
    echo "Unable to copy files: [${CP_INVOKE}]"
    exit 1
fi

# Copy the 64-bit library
CP_INVOKE="${CP} bin/Release-Linux64/libAMDOpenCLDebugAPI64.so ${RELEASEDIR}/Lib/x86_64"
${CP_INVOKE}
CP_RESULT=$?
if [ ${CP_RESULT} -ne 0 ]
then
    echo "Unable to copy files: [${CP_INVOKE}]"
    exit 1
fi

# Debug build bits
# Copy the 32-bit library
CP_INVOKE="${CP} bin/Debug-Linux32/libAMDOpenCLDebugAPI32.so ${RELEASEDIR}/Lib/x86/libAMDOpenCLDebugAPI32-d.so"
${CP_INVOKE}
CP_RESULT=$?
if [ ${CP_RESULT} -ne 0 ]
then
    echo "Unable to copy files: [${CP_INVOKE}]"
    exit 1
fi

# Copy the 64-bit library
CP_INVOKE="${CP} bin/Debug-Linux64/libAMDOpenCLDebugAPI64.so ${RELEASEDIR}/Lib/x86_64/libAMDOpenCLDebugAPI64-d.so"
${CP_INVOKE}
CP_RESULT=$?
if [ ${CP_RESULT} -ne 0 ]
then
    echo "Unable to copy files: [${CP_INVOKE}]"
    exit 1
fi

# Build the tarball
# Binaries in bin/<build>-Linux<arch>
# Construct a tarball of the release build using the version
# The format of this will be: AMDOpenCLDebugAPI-yyyy-mm-dd-v<major>.<minor>.<build>.tar

export DATE_INVOKE="${DATE} +%Y-%m-%d"
THE_DATE=`${DATE_INVOKE}`
DATE_RESULT=$?
if [ ${DATE_RESULT} -ne 0 ]
then
    echo "Unable to determine the date: [${DATE_INVOKE}]"
    exit 1
fi

echo "cd ${RELEASEDIR}"
cd ${RELEASEDIR}
export TARBALL_NAME="AMDOpenCLDebugAPI-${THE_DATE}-v${V_MAJOR}.${V_MINOR}.${BUILDNUM}.tar"

# Watch out for recursion - doing tar <foo.tar> . will add foo.tar to the tarball (an error)
export TAR_INVOKE="${TAR} cvf ../${TARBALL_NAME} ."
echo "----------"
echo "${TAR_INVOKE}"
${TAR_INVOKE}
TAR_RESULT=$?
if [ ${TAR_RESULT} -ne 0 ]
then
    echo "Unable to build final tarball: [${TAR_INVOKE}]"
    exit 1
fi

# Build the test tarball
# Unit test Binaries in testbin/<build>-Linux<arch>
# Construct a tarball of the release build using the version
# The format of this will be: OCLDA_UnitTests-yyyy-mm-dd-v<major>.<minor>.<build>.tar
echo "cd ${CPUPROF_TOP}"
cd ${CPUPROF_TOP}
export TEST_TARBALL_NAME="OCLDA_UnitTests-${THE_DATE}-v${V_MAJOR}.${V_MINOR}.${BUILDNUM}.tar"

export TEST_TAR_INVOKE="${TAR} cvf ${TEST_TARBALL_NAME} UnitTests"
echo "----------"
echo "${TEST_TAR_INVOKE}"
${TEST_TAR_INVOKE}
TEST_TAR_RESULT=$?
if [ ${TEST_TAR_RESULT} -ne 0 ]
then
    echo "Unable to build final unit test tarball: [${TEST_TAR_INVOKE}]"
    exit 1
fi


# And back to the top OpenCLDebugAPI directory (where the tarballs gets constructed)
cd ${CPUPROF_TOP}

# gzip the tarballs, then build the file index
# the fileindex.txt file is known about by the debugger test project, and is known
# to contain the list of files to wget
echo "rm -f ${CPUPROF_TOP}/fileindex.txt"
rm -f ${CPUPROF_TOP}/fileindex.txt

export GZIP_INVOKE="${GZIP_TOOL} ${TARBALL_NAME}"
echo "----------"
echo "${GZIP_INVOKE}"
eval ${GZIP_INVOKE}
GZIP_RESULT=$?
if [ ${GZIP_RESULT} -ne 0 ]
then
    echo "Unable to build final gzipped tarball: [${GZIP_INVOKE}]"
    exit 1
fi
echo "${TARBALL_NAME}.gz" >> ${CPUPROF_TOP}/fileindex.txt

export TEST_GZIP_INVOKE="${GZIP_TOOL} ${TEST_TARBALL_NAME}"
echo "----------"
echo "${TEST_GZIP_INVOKE}"
eval ${TEST_GZIP_INVOKE}
TEST_GZIP_RESULT=$?
if [ ${TEST_GZIP_RESULT} -ne 0 ]
then
    echo "Unable to build final gzipped test tarball: [${TEST_GZIP_INVOKE}]"
    exit 1
fi
echo "${TEST_TARBALL_NAME}.gz" >> ${CPUPROF_TOP}/fileindex.txt

echo "----------"
echo "Contents of fileindex.txt are:"
cat ${CPUPROF_TOP}/fileindex.txt
echo "----------"

exit 0
