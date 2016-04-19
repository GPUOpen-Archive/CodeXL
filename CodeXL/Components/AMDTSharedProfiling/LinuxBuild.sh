#!/bin/sh
# builds AMDTSharedProfiling.  One for each of 32-bit and 64-bit versions, and debug and
# release builds
# Set the BUILD_TOP environment variable to point to the correct

# *** NOTE ***
# You will need to check out the entire tree to perform a build, due to permissions
# issues.
#
# The target layout of the location of generated content.  CentOS6.2 because
# that is the host OS of the build
#   BUILD_TOP/Build/CentOS6.2/....
#                           /x86/
#                               /debug/include
#                               /debug/lib
#                               /debug/bin
#                               /release/include
#                               /release/lib
#                               /release/bin
#                           /x86_64/
#                               /debug/include
#                               /debug/lib
#                               /debug/bin
#                               /release/include
#                               /release/lib
#                               /release/bin
# Downstream projects need to point at these locations
export BUILD_TOP=${HOME}/Perforce/devtools/main/CommonProjects/AMDTSharedProfiling
export INST_TOP=${HOME}/Perforce/devtools/main/Common/Lib/AMD/AMDTSharedProfiling/0.5/Build/CentOS6.2
mkdir -p ${INST_TOP}
IAM=`whoami`
THIS_LOG=${INST_TOP}/${IAM}.fullmake.log
make all > ${THIS_LOG} 2>&1
BUILD_RC=$?
if [ $BUILD_RC -ne 0 ]
then
    echo "Build failed see - ${THIS_LOG} for details"
fi

# Now move the subtrees
for i in x86/debug x86_64/debug x86/release x86_64/release
do
    echo "rm -rf ${INST_TOP}/${i}"
    rm -rf ${INST_TOP}/${i}
    echo "mkdir -p ${INST_TOP}/${i}"
    mkdir -p ${INST_TOP}/${i}
done

for tgt_dir in include lib
do
    echo cp -r OBJx64DEBUG/${tgt_dir} ${INST_TOP}/x86_64/debug
    cp -r OBJx64DEBUG/${tgt_dir} ${INST_TOP}/x86_64/debug
    echo cp -r OBJx64/${tgt_dir} ${INST_TOP}/x86_64/release
    cp -r OBJx64/${tgt_dir} ${INST_TOP}/x86_64/release
    echo cp -r OBJx64DEBUG/${tgt_dir} ${INST_TOP}/x86/debug
    cp -r OBJx64DEBUG/${tgt_dir} ${INST_TOP}/x86/debug
    echo cp -r OBJx64/${tgt_dir} ${INST_TOP}/x86/release
    cp -r OBJx64/${tgt_dir} ${INST_TOP}/x86/release
done

echo "Symbolic links need to be created by hand...."
