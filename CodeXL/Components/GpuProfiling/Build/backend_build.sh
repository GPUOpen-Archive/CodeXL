#!/bin/bash

#define path
SPROOT=`dirname $(readlink -f "$0")`/..

# Command line args

# Build Framework (BaseTools/OSWrappers) -- only makes sense to skip if doing an incremental build
bBuildFramework=true

# Build HSA Profiler
bBuildHSAProfiler=true

# Build OCL Profiler
bBuildOCLProfiler=true

# Generate zip file
bZip=false

# Only generate zip file
bZipOnly=false

# Only execute clean target (mutually exclusive with incremental build)
bCleanOnly=false

# Build number
BUILD=0

# Incremental build
bIncrementalBuild=false

# Append to existing log file
bAppendToLog=false

# Build log file
bLogFileSpecified=false

# Debug build requested
bDebugBuild=false
MAKE_TARGET=
DEBUG_SUFFIX=
CODEXL_FRAMEWORK_BUILD_CONFIG_DIR=release

# Build 32-bit build
b32bitbuild=true

# HSA directory override
HSA_DIR_OVERRIDE=

# Set build flag
while [ "$*" != "" ]
do
   if [ "$1" = "zip" ]; then
      bZip=true
   elif [ "$1" = "zip-only" ]; then
      bZipOnly=true
   elif [ "$1" = "skip-hsaprofiler" ]; then
      bBuildHSAProfiler=false
   elif [ "$1" = "skip-oclprofiler" ]; then
      bBuildOCLProfiler=false
   elif [ "$1" = "skip-framework" ]; then
      bBuildFramework=false
      bIncrementalBuild=true
   elif [ "$1" = "skip-32bitbuild" ]; then
      b32bitbuild=false
   elif [ "$1" = "incremental" ]; then
      bIncrementalBuild=true
      bCleanOnly=false
   elif [ "$1" = "quick" ]; then
      bIncrementalBuild=true
      bCleanOnly=false
   elif [ "$1" = "clean" ]; then
      bCleanOnly=true
      bIncrementalBuild=false
   elif [ "$1" = "bldnum" ]; then
      shift
      BUILD="$1"
   elif [ "$1" = "logfile" ]; then
      bLogFileSpecified=true
      shift
      SPECIFIED_LOGFILE="$1"
   elif [ "$1" = "appendlog" ]; then
      bAppendToLog=true
   elif [ "$1" = "debug" ]; then
      bDebugBuild=true
      MAKE_TARGET=Dbg
      DEBUG_SUFFIX=-d
      CODEXL_FRAMEWORK_BUILD_CONFIG_DIR=debug
   elif [ "$1" = "hsadir" ]; then
      shift
      HSA_DIR_OVERRIDE="HSA_DIR=$1"
   fi
   shift
done

BUILD_PATH=$SPROOT/Build

echo "SPROOT=$SPROOT"

if ($bLogFileSpecified) ; then
   LOGFILE=$SPECIFIED_LOGFILE
else
   LOGFILE=$BUILD_PATH/GpuProfilerBackend_Build.log
fi

COMMON=$SPROOT/../../../Common
CODEXL_DIR=$SPROOT/../../../CodeXL
DOC=$SPROOT/Doc
BIN=$SPROOT/bin
BACKENDCOMMON=$SPROOT/Backend/Common
BACKENDDEVICEINFO=$SPROOT/Backend/DeviceInfo
CLCOMMON=$SPROOT/Backend/CLCommon
SPROFILE=$SPROOT/Backend/sprofile
PROFILER_OUTPUT=$SPROOT/Backend/CodeXLGpuProfiler
CLPROFILE=$SPROOT/Backend/CLProfileAgent
CLTRACE=$SPROOT/Backend/CLTraceAgent
CLOCCUPANCY=$SPROOT/Backend/CLOccupancyAgent
HSAFDNCOMMON=$SPROOT/Backend/HSAFdnCommon
HSAFDNPMC=$SPROOT/Backend/HSAFdnPMC
HSAFDNTRACE=$SPROOT/Backend/HSAFdnTrace
PRELOADXINITTHREADS=$SPROOT/Backend/PreloadXInitThreads
LINUXRESOURCES=$SPROOT/Backend/LinuxResources
ACTIVITYLOGGER=CXLActivityLogger
ACTIVITYLOGGERDIR=$SPROOT/../../../Common/Src/AMDTActivityLogger/
GPA=$COMMON/Lib/AMD/GPUPerfAPI/2_20
JQPLOT_PATH=$SPROOT/Backend/Common/jqPlot
COMMONPROJECTS=$COMMON/Src/
DOXYGENBIN=$COMMON/DK/Doxygen/doxygen-1.5.6/bin/doxygen

GPACL=libGPUPerfAPICL.so
GPACL32=libGPUPerfAPICL32.so

GPAHSA=libGPUPerfAPIHSA.so

GPACOUNTER=libGPUPerfAPICounters.so
GPACOUNTER32=libGPUPerfAPICounters32.so

GPU_PROFILER_LIB_PREFIX=CXLGpuProfiler

COMMONLIB=lib${GPU_PROFILER_LIB_PREFIX}Common$DEBUG_SUFFIX.a
COMMONLIB32=lib${GPU_PROFILER_LIB_PREFIX}Common32$DEBUG_SUFFIX.a
CLCOMMONLIB=lib${GPU_PROFILER_LIB_PREFIX}CLCommon$DEBUG_SUFFIX.a
CLCOMMONLIB32=lib${GPU_PROFILER_LIB_PREFIX}CLCommon32$DEBUG_SUFFIX.a
HSACOMMONLIB=lib${GPU_PROFILER_LIB_PREFIX}HSACommon$DEBUG_SUFFIX.a
DEVICEINFOLIB=lib${GPU_PROFILER_LIB_PREFIX}DeviceInfo$DEBUG_SUFFIX.a
DEVICEINFOLIB32=lib${GPU_PROFILER_LIB_PREFIX}DeviceInfo32$DEBUG_SUFFIX.a

SPROFILEBIN=CodeXLGpuProfiler$DEBUG_SUFFIX
SPROFILEBIN32=CodeXLGpuProfiler32$DEBUG_SUFFIX
PROFILEBIN=lib${GPU_PROFILER_LIB_PREFIX}CLProfileAgent$DEBUG_SUFFIX.so
PROFILEBIN32=lib${GPU_PROFILER_LIB_PREFIX}CLProfileAgent32$DEBUG_SUFFIX.so
TRACEBIN=lib${GPU_PROFILER_LIB_PREFIX}CLTraceAgent$DEBUG_SUFFIX.so
TRACEBIN32=lib${GPU_PROFILER_LIB_PREFIX}CLTraceAgent32$DEBUG_SUFFIX.so
OCCUPANCYBIN=lib${GPU_PROFILER_LIB_PREFIX}CLOccupancyAgent$DEBUG_SUFFIX.so
OCCUPANCYBIN32=lib${GPU_PROFILER_LIB_PREFIX}CLOccupancyAgent32$DEBUG_SUFFIX.so
HSAPROFILEAGENTBIN=lib${GPU_PROFILER_LIB_PREFIX}HSAProfileAgent$DEBUG_SUFFIX.so
HSATRACEAGENTBIN=lib${GPU_PROFILER_LIB_PREFIX}HSATraceAgent$DEBUG_SUFFIX.so
PRELOADXINITTHREADSBIN=lib${GPU_PROFILER_LIB_PREFIX}PreloadXInitThreads$DEBUG_SUFFIX.so
PRELOADXINITTHREADSBIN32=lib${GPU_PROFILER_LIB_PREFIX}PreloadXInitThreads32$DEBUG_SUFFIX.so

ACTIVITYLOGGERBIN=libCXLActivityLogger.so
ACTIVITYLOGGERBIN32=libCXLActivityLogger.so

MAKE_TARGET_SUFFIX_X86=x86

PRODUCTNAME=GPUProfilingBackend

VERSION_FILE=$BACKENDCOMMON/Version.h

DATE=$(date +"%Y-%m-%d")

VERSION_MAJOR=$(grep -m 1 "GPUPROFILER_BACKEND_MAJOR_VERSION" $VERSION_FILE | awk '{print $3}')
VERSION_MINOR=$(grep -m 1 "GPUPROFILER_BACKEND_MINOR_VERSION" $VERSION_FILE | awk '{print $3}')
VERSION="$VERSION_MAJOR.$VERSION_MINOR"
VERSION_STR=$DATE-v$VERSION.$BUILD

CODEXL_ARCHIVE_BASE=GPUProfilingBackendCodeXL-v$VERSION.$BUILD.tgz
CODEXL_ARCHIVE=$BUILD_PATH/$CODEXL_ARCHIVE_BASE

if !($bZipOnly) ; then

   #-----------------------------------------
   # Update Version.h to include build number
   #-----------------------------------------
   CURRENT_VERSION_H_FILE_PERMISSIONS=$(stat --format %a $VERSION_FILE)
   chmod 777 $VERSION_FILE
   old=$(grep -E "#define GPUPROFILER_BACKEND_BUILD_NUMBER [0-9]+" $VERSION_FILE)
   new="#define GPUPROFILER_BACKEND_BUILD_NUMBER $BUILD"
   sed -i "s/$old/$new/g" $VERSION_FILE
   chmod $CURRENT_VERSION_H_FILE_PERMISSIONS $VERSION_FILE

   # delete log file
   if !($bAppendToLog) ; then
      rm -f $LOGFILE
   fi

   echo "=====Building GPU Profiler Backend======"  | tee -a $LOGFILE

   if ($bBuildFramework) ; then
      #-----------------------------------------
      # build the AMDT framework libraries based on the script of CodeXL
      #-----------------------------------------
      commandLineArgs=$*

      if ($bCleanOnly); then
         commandLineArgs="$commandLineArgs -c"
      fi

      NUM_ERRORS=0

      # Display a start message:
      echo | tee -a $LOGFILE
      echo "Building infra projects" | tee -a $LOGFILE
      echo "===========================" | tee -a $LOGFILE
      echo "Build arguments passed to scons: $commandLineArgs" | tee -a $LOGFILE
      if [ "x${AMD_OUTPUT}x" = "xx" ]
      then
         # If not, it means this script was invoked by unknown means.
         export AMD_OUTPUT_PROFILING=${SPROOT}/../
      fi
      export CXL_common_dir=${COMMON}
      echo "========================================== " | tee -a $LOGFILE
      echo "----------- Start building --------------- " | tee -a $LOGFILE
      cd ${SPROOT}/Build
      date | tee -a $LOGFILE

      if ($bDebugBuild) ; then
         echo "----------- Building debug version --------------- " | tee -a $LOGFILE
         echo "scons -C ${SPROOT}/Build CXL_prefix=${SPROOT} CXL_build=debug CXL_build_type=static $commandLineArgs" | tee -a $LOGFILE
         eval "scons -C ${SPROOT}/Build CXL_prefix=${SPROOT} CXL_build=debug CXL_build_type=static $commandLineArgs >> $LOGFILE 2>&1"
      else
         echo "========================================== " | tee -a $LOGFILE
         echo "scons -C ${SPROOT}/Build CXL_prefix=${SPROOT} CXL_build_type=static $commandLineArgs" | tee -a $LOGFILE
         eval "scons -C ${SPROOT}/Build CXL_prefix=${SPROOT} CXL_build_type=static $commandLineArgs >> $LOGFILE 2>&1"
      fi
      RC1=$?
      if [ ${RC1} -ne 0 ]
      then
         echo "*** ERROR during the build of the 64 bit framework ***" | tee -a $LOGFILE
      fi

      RC2=0
      if $b32bitbuild; then
         if ($bDebugBuild) ; then
            echo "----------- Building debug 32-bit version --------------- " | tee -a $LOGFILE
            echo "scons -C ${SPROOT}/Build CXL_prefix=${SPROOT} CXL_arch=x86 CXL_build=debug CXL_build_type=static $commandLineArgs" | tee -a $LOGFILE
            eval "scons -C ${SPROOT}/Build CXL_prefix=${SPROOT} CXL_arch=x86 CXL_build=debug CXL_build_type=static $commandLineArgs >> $LOGFILE 2>&1"
         else
            echo "========================================== " | tee -a $LOGFILE
            echo "scons -C ${SPROOT}/Build CXL_prefix=${SPROOT} CXL_arch=x86 CXL_build_type=static $commandLineArgs" | tee -a $LOGFILE
            eval "scons -C ${SPROOT}/Build CXL_prefix=${SPROOT} CXL_arch=x86 CXL_build_type=static $commandLineArgs >> $LOGFILE 2>&1"
         fi
         RC2=$?
         if [ ${RC2} -ne 0 ]
         then
            echo "*** ERROR during the build of the 32 bit framework ***" | tee -a $LOGFILE
         fi
      fi

      echo "========================================== " | tee -a $LOGFILE
      echo "----------- End building ----------------- " | tee -a $LOGFILE
      date | tee -a $LOGFILE
      echo "========================================== " | tee -a $LOGFILE

      NUM_ERRORS=`expr ${NUM_ERRORS} + ${RC1} + ${RC2}`
      if [ ${NUM_ERRORS} -ne 0 ]
      then
         echo "*** ERROR ***"
         echo "*** the build failed - see the logs for details ***"
         exit 1
      else
         echo "*** SUCCESS ***"
      fi

      if ($bCleanOnly); then
         rm -rf $SPROOT/Output_x86_64/
         rm -rf $SPROOT/Output_x86/
      fi
   fi

   cd $SPROFILE

   #delete old build if it exists
   rm -f ./$SPROFILEBIN
   rm -f ./$SPROFILEBIN32
   rm -f ./$PROFILEBIN
   rm -f ./$PROFILEBIN32
   rm -f ./$TRACEBIN
   rm -f ./$TRACEBIN32
   rm -f ./$HSATRACEAGENTBIN
   rm -f ./$HSAPROFILEAGENTBIN
   rm -f ./$OCCUPANCYBIN
   rm -f ./$OCCUPANCYBIN32
   rm -f ./$PRELOADXINITTHREADSBIN
   rm -f ./$PRELOADXINITTHREADSBIN32

   BUILD_DIRS="$BACKENDCOMMON $BACKENDDEVICEINFO $CLCOMMON"

   if $bBuildOCLProfiler ; then
      BUILD_DIRS="$BUILD_DIRS $CLPROFILE $CLTRACE $CLOCCUPANCY"
   fi

   if $bBuildHSAProfiler; then
      BUILD_DIRS="$BUILD_DIRS $HSAFDNCOMMON $HSAFDNTRACE $HSAFDNPMC"
   fi

   BUILD_DIRS="$BUILD_DIRS $SPROFILE $PRELOADXINITTHREADS"

   for SUBDIR in $BUILD_DIRS; do
      BASENAME=`basename $SUBDIR`

      if !($bIncrementalBuild) ; then
         make -C $SUBDIR spotless >> $LOGFILE 2>&1
      fi

      if !($bCleanOnly); then
         #make 64 bit
         echo "Build ${BASENAME}, 64-bit..." | tee -a $LOGFILE

         if ! make -C $SUBDIR -j$CPU_COUNT $HSA_DIR_OVERRIDE $MAKE_TARGET >> $LOGFILE 2>&1; then
            echo "Failed to build ${BASENAME}, 64 bit"
            exit 1
         fi

         if $b32bitbuild; then
            if [ "$SUBDIR" = "$HSAFDNTRACE" ]; then
               continue;
            fi

            if [ "$SUBDIR" = "$HSAFDNPMC" ]; then
               continue;
            fi

            #make 32 bit
            echo "Build ${BASENAME}, 32-bit..." | tee -a $LOGFILE

            if ! make -C $SUBDIR -j$CPU_COUNT $HSA_DIR_OVERRIDE $MAKE_TARGET$MAKE_TARGET_SUFFIX_X86 >> $LOGFILE 2>&1; then
               echo "Failed to build ${BASENAME}, 32 bit"
               exit 1
            fi
         fi
      fi
   done

   if !($bCleanOnly); then
      cp -f $GPA/Bin/Linx64/$GPACOUNTER $PROFILER_OUTPUT
      cp -f $GPA/Bin/Linx86/$GPACOUNTER32 $PROFILER_OUTPUT

      if $bBuildOCLProfiler ; then
         cp -f $GPA/Bin/Linx64/$GPACL $PROFILER_OUTPUT
         cp -f $GPA/Bin/Linx86/$GPACL32 $PROFILER_OUTPUT
      fi

      if $bBuildHSAProfiler ; then
         cp -f $GPA/Bin/Linx64/$GPAHSA $PROFILER_OUTPUT
      fi
   else
      rm -rf $PROFILER_OUTPUT
   fi

   #-----------------------------------------
   #clean up bin folder
   #-----------------------------------------
   rm -rf $BIN

   if !($bCleanOnly); then
      #-----------------------------------------
      #check if bin folder exist
      #-----------------------------------------
      if [ ! -e $BIN ]; then
         mkdir $BIN
      fi

      if [ ! -e $BIN/$ACTIVITYLOGGER ]; then
         mkdir $BIN/$ACTIVITYLOGGER
      fi

      if [ ! -e $BIN/$ACTIVITYLOGGER/bin ]; then
         mkdir $BIN/$ACTIVITYLOGGER/bin
      fi

      if [ ! -e $BIN/$ACTIVITYLOGGER/doc ]; then
         mkdir $BIN/$ACTIVITYLOGGER/doc
      fi

      if [ ! -e $BIN/$ACTIVITYLOGGER/bin/x86 ]; then
         mkdir $BIN/$ACTIVITYLOGGER/bin/x86
      fi

      if [ ! -e $BIN/$ACTIVITYLOGGER/bin/x86_64 ]; then
         mkdir $BIN/$ACTIVITYLOGGER/bin/x86_64
      fi

      if [ ! -e $BIN/$ACTIVITYLOGGER/include ]; then
         mkdir $BIN/$ACTIVITYLOGGER/include
      fi

      if [ ! -e $BIN/jqPlot ]; then
         mkdir $BIN/jqPlot
      fi

      #-----------------------------------------
      #copy to bin folder
      #-----------------------------------------
      # x64
      cp $PROFILER_OUTPUT/$SPROFILEBIN $BIN/$SPROFILEBIN
      cp $PROFILER_OUTPUT/$PRELOADXINITTHREADSBIN $BIN/$PRELOADXINITTHREADSBIN
      if $bBuildOCLProfiler ; then
         cp $PROFILER_OUTPUT/$PROFILEBIN $BIN/$PROFILEBIN
         cp $PROFILER_OUTPUT/$TRACEBIN $BIN/$TRACEBIN
         cp $PROFILER_OUTPUT/$OCCUPANCYBIN $BIN/$OCCUPANCYBIN
         cp $GPA/Bin/Linx64/$GPACL $BIN/$GPACL
      fi
      if $bBuildHSAProfiler ; then
         cp $PROFILER_OUTPUT/$HSATRACEAGENTBIN $BIN/$HSATRACEAGENTBIN
         cp $PROFILER_OUTPUT/$HSAPROFILEAGENTBIN $BIN/$HSAPROFILEAGENTBIN
         cp $GPA/Bin/Linx64/$GPAHSA $BIN/$GPAHSA
      fi
      cp $GPA/Bin/Linx64/$GPACOUNTER $BIN/$GPACOUNTER
      cp $LINUXRESOURCES/CodeXLGpuProfilerRun $BIN

      #x86
      if $b32bitbuild; then
         if $bBuildOCLProfiler ; then
            cp $PROFILER_OUTPUT/$SPROFILEBIN32 $BIN/$SPROFILEBIN32
            cp $PROFILER_OUTPUT/$PRELOADXINITTHREADSBIN32 $BIN/$PRELOADXINITTHREADSBIN32
            cp $PROFILER_OUTPUT/$PROFILEBIN32 $BIN/$PROFILEBIN32
            cp $PROFILER_OUTPUT/$TRACEBIN32 $BIN/$TRACEBIN32
            cp $PROFILER_OUTPUT/$OCCUPANCYBIN32 $BIN/$OCCUPANCYBIN32
            cp $GPA/Bin/Linx86/$GPACL32 $BIN/$GPACL32
            cp $GPA/Bin/Linx86/$GPACOUNTER32 $BIN/$GPACOUNTER32
            cp $LINUXRESOURCES/CodeXLGpuProfilerRun32 $BIN
         fi
      fi

      #AMDTActivityLogger files
      cp $SPROOT/Output_x86_64/$CODEXL_FRAMEWORK_BUILD_CONFIG_DIR/bin/$ACTIVITYLOGGERBIN $BIN/$ACTIVITYLOGGER/bin/x86_64/$ACTIVITYLOGGERBIN
      cp $SPROOT/Output_x86/$CODEXL_FRAMEWORK_BUILD_CONFIG_DIR/bin/$ACTIVITYLOGGERBIN32 $BIN/$ACTIVITYLOGGER/bin/x86/$ACTIVITYLOGGERBIN32
      cp $ACTIVITYLOGGERDIR/AMDTActivityLogger.h $BIN/$ACTIVITYLOGGER/include/$ACTIVITYLOGGER.h
      cp $ACTIVITYLOGGERDIR/Doc/AMDTActivityLogger.pdf $BIN/$ACTIVITYLOGGER/doc/AMDTActivityLogger.pdf
      #jqPlot files
      cp $JQPLOT_PATH/* $BIN/jqPlot
   fi
fi

#-----------------------------------------
# Copy to CodeXL output location
#-----------------------------------------
CODEXLOUTPUT=$CODEXL_DIR/../Output_x86_64
CODEXLOUTPUTREL=$CODEXLOUTPUT/release
CODEXLOUTPUTDBG=$CODEXLOUTPUT/debug

if [ -d $CODEXLOUTPUTREL ]; then
   rm -rf $CODEXLOUTPUTREL/bin/$ACTIVITYLOGGER
   rm -rf $CODEXLOUTPUTREL/bin/jqPlot

   if !($bCleanOnly); then
      echo "Copying GPU Profiler backend files to CodeXL Output release directory" | tee -a $LOGFILE
      cp -R $BIN/$ACTIVITYLOGGER $CODEXLOUTPUTREL/bin
      cp -R $BIN/jqPlot $CODEXLOUTPUTREL/bin

      cp -f $BIN/* $CODEXLOUTPUTREL/bin
   fi
fi

if [ -d $CODEXLOUTPUTDBG ]; then
   rm -rf $CODEXLOUTPUTDBG/bin/$ACTIVITYLOGGER
   rm -rf $CODEXLOUTPUTDBG/bin/jqPlot

   if !($bCleanOnly); then
      echo "Copying GPU Profiler backend files to CodeXL Output debug directory" | tee -a $LOGFILE
      cp -R $BIN/$ACTIVITYLOGGER $CODEXLOUTPUTDBG/bin
      cp -R $BIN/jqPlot $CODEXLOUTPUTDBG/bin

      cp -f $BIN/* $CODEXLOUTPUTDBG/bin
   fi
fi

#-----------------------------------------
# zip
#-----------------------------------------
if $bZip || $bZipOnly ; then
   if [ ! -e $BUILD_PATH ]; then
      mkdir $BUILD_PATH
   fi

   cd $BUILD_PATH
   rm -f ./*.tgz

   # pack x64 version
   echo "Creating public build tarball..." | tee -a $LOGFILE
   if [ ! -e $BUILD_PATH/$PRODUCTNAME-$VERSION ]; then
      mkdir $BUILD_PATH/$PRODUCTNAME-$VERSION
      mkdir $BUILD_PATH/$PRODUCTNAME-$VERSION/bin
   fi

   cp $BIN/* $BUILD_PATH/$PRODUCTNAME-$VERSION/bin
   cp -R $BIN/$ACTIVITYLOGGER $BUILD_PATH/$PRODUCTNAME-$VERSION
   cp -R $BIN/jqPlot $BUILD_PATH/$PRODUCTNAME-$VERSION
   chmod -R 755 $BUILD_PATH/$PRODUCTNAME-$VERSION

   # create artifact for CodeXL
   cd $BUILD_PATH/$PRODUCTNAME-$VERSION
   tar cvzf $CODEXL_ARCHIVE bin/ jqPlot/ $ACTIVITYLOGGER/
   chmod 755 $CODEXL_ARCHIVE
   cd $BUILD_PATH

   # cleanup
   rm -rf $BUILD_PATH/$PRODUCTNAME-$VERSION/

   # Check artifacts, write to log.
   if [ -e $CODEXL_ARCHIVE ] ; then
      echo "$CODEXL_ARCHIVE_BASE" >> $LOGFILE
   else
      echo "Failed to generate $CODEXL_ARCHIVE" >> $LOGFILE
      exit 1
   fi
fi

exit 0
