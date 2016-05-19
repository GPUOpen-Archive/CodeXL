#!/bin/bash

VERSION_VER=$(find . -name 'CodeXL*.tar.gz' | awk -F"." '{ print $4}') 
BASE_VERSION=2.1
BASE_REVISION=$VERSION_VER
INTERNAL_VERSION=${BASE_VERSION}.$VERSION_VER
VERSION=${BASE_VERSION}-$VERSION_VER
PACAKGENAME="codexl"
echo $VERSION
# Change Package name if NDA or INTERNAL version
NDASET=$(find . -name 'CodeXL*.tar.gz' | grep -q "NDA"; [ $? -eq 0 ] && echo "nda")
INTERNALSET=$(find . -name 'CodeXL*.tar.gz' | grep -q "Internal"; [ $? -eq 0 ] && echo "internal")
if ! [ -z "$NDASET" ];then
	PACAKGENAME="${PACAKGENAME}-${NDASET}"
elif ! [ -z "$INTERNALSET" ];then
	PACAKGENAME="${PACAKGENAME}-${INTERNALSET}"
fi

#Un-tar source files
echo "Unpack files" 
mkdir AMDExtractFolder
tar xf CodeXL*.tar.gz --strip 1 -C AMDExtractFolder

echo "Setting files and folder"
# Set Executables Icons
chmod +xw AMDExtractFolder/amdcodexlicon.desktop
chmod +xw AMDExtractFolder/amdremoteagenticon.desktop
echo "Version=$VERSION" >> AMDExtractFolder/amdcodexlicon.desktop
echo "Exec=/opt/CodeXL_$VERSION/CodeXL" >> AMDExtractFolder/amdcodexlicon.desktop
echo "Icon=/opt/CodeXL_$VERSION/Images/ApplicationIcon_64.ico" >> AMDExtractFolder/amdcodexlicon.desktop
echo "Path=/opt/CodeXL_$VERSION/" >> AMDExtractFolder/amdcodexlicon.desktop
echo "Version=$VERSION" >> AMDExtractFolder/amdremoteagenticon.desktop
echo "Exec=/opt/CodeXL_$VERSION/CodeXLRemoteAgent" >> AMDExtractFolder/amdremoteagenticon.desktop
echo "Icon=/opt/CodeXL_$VERSION/Images/ApplicationIcon_64.ico" >> AMDExtractFolder/amdremoteagenticon.desktop 
echo "Path=/opt/CodeXL_$VERSION/" >> AMDExtractFolder/amdremoteagenticon.desktop 

# Move files to relative destination
mkdir -p opt/CodeXL_$VERSION
cp -r AMDExtractFolder/. opt/CodeXL_$VERSION/
mkdir -p usr/share/CodeXL_$VERSION/
mv opt/CodeXL_$VERSION/examples/ usr/share/CodeXL_$VERSION/examples/
ln -s /usr/share/CodeXL_$VERSION/examples/ opt/CodeXL_$VERSION/examples
mkdir -p usr/share/applications
cp AMDExtractFolder/amdcodexlicon.desktop usr/share/applications/amdcodexlicon.desktop
cp AMDExtractFolder/amdremoteagenticon.desktop usr/share/applications/amdremoteagenticon.desktop 

# Edit Debian package files with current version
echo "Setting debian package files"
chmod +w control
chmod +w postinst
chmod +w prerm
chmod +w changelog
sed -i "s/Version: /Version: ${VERSION}/g" control 
sed -i "s/CodeXL_[^/]*/CodeXL_${VERSION}/g" postinst
sed -i "s/CodeXL_[^/]*/CodeXL_${VERSION}/g" prerm
sed -i "s/()/(${VERSION})/g" changelog
#get build time
buildtime=$(date +"%a, %d %b %Y %H:%M:%S %z")
sed -i "s/com>/com> ${buildtime}/g" changelog

#Running FPM command
# Reference command - fpm -f -C ~/Downloads/FromRPM --workdir ~/Downloads/testrpmdeb/ --deb-custom-control ~/Downloads/debscripts/control --after-install ~/Downloads/debscripts/postinst --before-remove ~/Downloads/debscripts/prerm -s dir -t deb -n amdcodexl -v 1.8-9000 opt/ usr/
echo "Running fpm... (pack the directories to deb package)"
fpm -f -C . --deb-custom-control control --after-install postinst --before-remove prerm --deb-changelog changelog -s dir -t deb -n ${PACAKGENAME} -v ${VERSION} opt/ usr/
mv codexl*.deb ../../../../

#Clean local files and folders 
rm -rf AMDExtractFolder
rm -rf opt
rm -rf usr
rm -rf CodeXL*.tar.gz

