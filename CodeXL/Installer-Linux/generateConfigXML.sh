#!/bin/bash
# Script to generate a config.xml file which gets used by the auto-updater + web site
# the config file goes on the web site, and the auto-updater trolls a known fixed
# location on the web site, and uses the content to determine if there is a newer
# version of whatever product is available
#
# Passed in as parameters: Major, Minor, Build, Arch
# Output all goes to stdout, and needs to be directed to the appropriate
# .xml file by the caller

# The web site with the current product information
export PRODUCT_WEBSITE="http://developer.amd.com/tools/heterogeneous-computing/codexl/"
# The string (can have spaces) describing the product
export BASE_PRODUCT_STRING="AMD CodeXL"

# The base URL to the tarball/msi
export BASE_PRODUCT_URL="http://developer.amd.com/Downloads"
# The base tarball name
export BASE_PACKAGE="CodeXL"

NARGS=$#
if [ ${NARGS} -ne 5 ]
then
    echo "Usage: $0 [Major] [Minor] [Build] [Update] [x86|x86_64]"
    exit 1
fi
V_MAJOR=$1
V_MINOR=$2
V_BUILD=$3
V_UPDATE=$4
PKG_ARCH=$5

BUILD_YEAR=`date +%Y`
BUILD_MONTH=`date +%m`
BUILD_DAY=`date +%d`

# CodeXL-Linux-0.93.351.0-x86_64-release.tar.gz
FULL_PACKAGE_NAME=${BASE_PACKAGE}-${V_MAJOR}.${V_MINOR}.${V_BUILD}.${V_UPDATE}-${PKG_ARCH}-release.tar.gz

echo "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
echo "<Program_Info>"
echo "	<Program_Version_Major>${V_MAJOR}</Program_Version_Major>"
echo "	<Program_Version_Minor>${V_MINOR}</Program_Version_Minor>"
echo "	<Program_Version_Build>${V_BUILD}</Program_Version_Build>"
echo "	<Program_Release_Year>${BUILD_YEAR}</Program_Release_Year>"
echo "	<Program_Release_Month>${BUILD_MONTH}</Program_Release_Month>"
echo "	<Program_Release_Day>${BUILD_DAY}</Program_Release_Day>"
echo "	<Program_Name>${BASE_PRODUCT_STRING}</Program_Name>"
echo "  <Program_File>${BASE_PRODUCT_URL}/${FULL_PACKAGE_NAME}</Program_File>"
echo "  <Program_Location>${PRODUCT_WEBSITE}</Program_Location>"
echo "</Program_Info>"

# And we are done
exit 0
