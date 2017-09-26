#!/bin/bash

# This script will build both debian and RPM packages for CodeXL
# To create RPM package, execute ./buildDEB.sh rpm
# To create DEB package, execute ./buildDEB.sh OR ./buildDEB.sh deb

doBuildDEB=true
doBuildRPM=false

while [ "$*" != "" ]
do
    if [ "$1" = "deb" ]; then
        doBuildDEB=true
        doBuildRPM=false
    elif [ "$1" = "rpm" ]; then
        doBuildRPM=true
        doBuildDEB=false
    fi
        shift
done

VERSION_VER=$(find . -name 'CodeXL*.tar.gz' | awk -F"." '{ print $4}') 
BASE_VERSION=2.5
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
#Set package folder size
psize=$(du -s AMDExtractFolder/| sed 's/\s.*$//')
sed -i "s/Installed-Size: /Installed-Size: ${psize}/g" control

#Running FPM command
if ($doBuildDEB) ; then
    # Reference command - fpm -f -C ~/Downloads/FromRPM --workdir ~/Downloads/testrpmdeb/ --deb-custom-control ~/Downloads/debscripts/control --after-install ~/Downloads/debscripts/postinst --before-remove ~/Downloads/debscripts/prerm -s dir -t deb -n amdcodexl -v 1.8-9000 opt/ usr/
    echo "Running fpm... (pack the directories to deb package)"
    fpm -f -C . --deb-custom-control control --after-install postinst --before-remove prerm --deb-changelog changelog -s dir -t deb -n ${PACAKGENAME} -v ${VERSION} opt/ usr/
    mv codexl*.deb ../../../../
fi

if ($doBuildRPM) ; then
    echo "Running fpm... (pack the directories to rpm  package)"
    echo $DEPENDS
    fpm -f -C . --after-install postinst --before-remove prerm -d 'gcc >= 3.0 ' -d 'glibc >= 2.12' -d 'mesa-libGL >= 7.0' -d 'mesa-libGLU >= 7.0' -d 'libxml2 >= 2.0' -d 'glib2 >= 2.0' -d 'libXext >= 1.0' -d 'zlib >= 1.0' -d 'libXmu >= 1.0' -d 'libXt >= 1.0' -d 'libSM >= 1.0' -d 'libICE >= 1.0' -d 'libX11 >= 1.0' -d 'atk >= 1.0' -d 'cairo >= 1.0' -d 'freetype >= 2.0' -d 'fontconfig >= 2.0' -d 'libXfixes >= 4.0' -d 'libXrender >= 0.9' -d 'libXinerama >= 1.0' -d 'libXi >= 1.0' -d 'libXrandr >= 1.0' -d 'libXcursor >= 1.0' -d 'libXcomposite >= 0.4' -d 'libXdamage >= 1.0' -d 'libuuid >= 2.0' -d 'libxcb >= 1.0' -d 'libselinux >= 2.0' -d 'libpng >= 1.0' -d 'pixman >= 0.15' -d 'expat >= 2.0' -d 'libXau >= 1.0' -s dir -t rpm -n ${PACAKGENAME} -v ${BASE_VERSION} --iteration ${VERSION_VER} --description 'CodeXL suite of development tools.' --url 'http://gpuopen.com/compute-product/codexl/' opt/ usr/
    mv codexl*.rpm ../../../../
fi

#Clean local files and folders 
rm -rf AMDExtractFolder
rm -rf opt
rm -rf usr
rm -rf CodeXL*.tar.gz
