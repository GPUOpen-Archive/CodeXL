#!/bin/bash

#####################################################
#
# build.sh : Generic Jenkins Linux build script for CommonProjects
#
# AUTHOR  : Suravee Suthikulpanit <suravee.suthikulpanit@amd.com
#
# VERSION : 1.0
#
# DESCRIPTIONS:
# This is a generic script which is designed to build any
# Linux product in the CommonProjects directory.
#
# PRE-REQUISITE:
# * Makefile with the following options
# 	* make clean
# 	* make all
# 	* make package BULD=$BUILD_NO (which generate the *.tgz file)
#   Or "INSTALL $BUILD_NO" shell script
#
# * COMMON_DIR and COMMONPROJECTS_DIR will be set as environment variable
#   to be used in this build environment.
# 
# OUTPUTS:
# * Jenkins/<ProductName>-Build<BuildNumber>.log
# * Jenkins/*.tgz


##############################
# Default values for options
##############################

# Workspace
WORKSPACE=

# Product name
PRODUCTNAME=

# Build number
BUILD_NO=0

# Complete build
bCompleteBuild=true

# Run unit tests (forced off for now)
bRunUnitTests= true

# Generate zip file
bZip=true

do_options ()
{
	while [ "$#" -ne 0 ]
	do
		arg=`printf %s $1 | awk -F= '{print $1}'`
		val=`printf %s $1 | awk -F= '{print $2}'`
		shift
		if test -z "$val"; then
			local possibleval=$1
			printf %s $1 "$possibleval" | grep ^- >/dev/null 2>&1
			if test "$?" != "0"; then
				val=$possibleval
				if [ "$#" -ge 1 ]; then
					shift
				fi
			fi
		fi

		# NOTES: These variables must have default value
		#        defined above
		case "$arg" in
			--workspace)
				WORKSPACE=$val
				;;
			--productName)
				PRODUCTNAME=$val
				;;
			--buildNum)
				BUILD_NO=$val
				;;
			--noBuild)
				bCompleteBuild=false
                                ;;
			--noUnitTest)
				bRunUnitTests=false
				;;
			--nozip)
				bZip=false
				;;
			*)
				echo "ERROR: Unknown option \"$arg\". See help " >&2
				exit 1;
				;;
		esac
	done
}


errorOut()
{
	echo $1 >> $LOGFILE
	exit 1
}

checkErrorOut()
{
        if test "$1" != "0" ; then
		ErrorOut $2
        fi;
}

do_initLogFile ()
{
	echo "Logfile : $LOGFILE"
	echo "#########################################################" > $LOGFILE
	echo "Product          : $PRODUCTNAME" >> $LOGFILE
	echo "Build number     : $BUILD_NO" >> $LOGFILE
	echo "Date/Time        : `date`" >> $LOGFILE
	echo "Kernel           : `uname -a`" >> $LOGFILE
	echo "Build system     :" >> $LOGFILE
	echo "`cat /etc/*-release`" >> $LOGFILE
	echo "#########################################################" >> $LOGFILE
}

do_setup ()
{
	if test -z $WORKSPACE; then
		echo "build.sh: ERROR: Please specify --workspace."
		exit 1
	fi
	
	if test -z $PRODUCTNAME ; then
		echo "build.sh: ERROR: Please specify --productName."
		exit 1
	fi

	COMMON_DIR=$WORKSPACE/main/Common
	export COMMON_DIR=$COMMON_DIR

	COMMONPROJECTS_DIR=$WORKSPACE/main/CommonProjects
	export COMMONPROJECTS_DIR=$COMMONPROJECTS_DIR

	PRODUCT_DIR=$WORKSPACE/main/CommonProjects/$PRODUCTNAME/
	INSTALLER_PATH=$PRODUCT_DIR/Jenkins
	LOGFILE=$INSTALLER_PATH/$PRODUCTNAME-Build$BUILD_NO.log

	if  test ! -d $INSTALLER_PATH; then
		mkdir -p $INSTALLER_PATH
	fi

	do_initLogFile
}

do_build ()
{
	if !($bCompleteBuild) ; then
		return
	fi

	echo "################## BEGIN BUILD ##########################" >> $LOGFILE
	echo "Building ... $PRODUCT_DIR" | tee -a $LOGFILE

	cd $PRODUCT_DIR

	if test -f INSTALL; then
                sh -x INSTALL $BUILD_NO | tee -a $LOGFILE
	       	checkErrorOut $? "ERROR: Failed to build release."
	else
		make clean | tee -a $LOGFILE
		make all | tee -a $LOGFILE
	       	checkErrorOut $? "ERROR: Failed to build release."
	fi
	cd $OLDPWD
	echo "################## END BUILD ############################" >> $LOGFILE
}

do_zip ()
{
	if !($bZip) ; then
		return
	fi

	echo "################### BEGIN ZIP ###########################" >> $LOGFILE
	echo "packaging ... $PRODUCT_DIR" | tee -a $LOGFILE

	cd $PRODUCT_DIR
	make package BUILD=$BUILD_NO | tee -a $LOGFILE
	checkErrorOut $? "ERROR: Failed to package release."

	rm -f $INSTALLER_PATH/*.tgz
	cp *.tgz $INSTALLER_PATH/

	# Check artifacts, write to log.
	result="`ls $INSTALLER_PATH/*.tgz`"
	if test -n "$result" ; then
		echo "SUCCESS: Generated $result" >> $LOGFILE
	else
	       	errorOut "ERROR: Failed to generate $PRODUCTNAME"
	fi

	cd $OLDPWD
	echo "##################### END ZIP ###########################" >> $LOGFILE
}

do_options $@
do_setup
do_build
do_zip

exit 0
