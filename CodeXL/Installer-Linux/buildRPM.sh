#!/bin/bash
# script to set up and perform the rpm packaging of CodeXL
# Issues:
#   Multiple RPM builds may be possible on a single host simultaneously, so
#   we need to construct this in such a fashion that it does not use any global
#   file locations.  Thus, we need to create our own rpmrc file and use it
# Inputs:
#   WORKSPACE environment variables must be set
#       Jenkins always sets these, but check (the case of manually running this script)
# Assumptions:
#   The CodeXL tarball to package will exist:
#       $RPM_BRANCH/main/CodeXL-Linux-*.tar.gz

# Bugs:
#   rpmbuild claims to have the ability to use --rcfile <location>
#       and that an entry of the form "topdir: <path>" will set the topdir.
#       This turns out to be untrue.  The only apparent way to get this to
#       work is to scribble the following to $HOME/.rpmmacros
#           echo "%_topdir %(echo ${RPMBUILD})" > ${HOME}/.rpmmacros
#       lots of experiments + google searches went into this.

# Our build number comes from the version encoded into the tarball.
# Assume the format of the incoming tarball is:
#   CodeXL-Linux-0.9.0.3-x86_64-debug.tar.gz

if [ ! -d ${WORKSPACE} ]
then
    echo "WORKSPACE is not a valid directory: [${WORKSPACE}]"
    exit 1
fi

if [ "$1" = "all_variants" ]
then
  all_variants="true"
fi

# Always wipe out the old rpmrc file and create a new one from scratch
RPMBUILD=${WORKSPACE}/rpmbuild
RPM_SOURCES=${RPMBUILD}/SOURCES
RPM_SPECS=${RPMBUILD}/SPECS
RPMRC_FILE=${RPMBUILD}/rpmrc
RPM_BRANCH=${WORKSPACE}

rm -rf ${RPMBUILD}

# Setup
mkdir -p ${RPM_SOURCES} ${RPM_SPECS}
# Oh, this is really moronic - there is no known way to set topdir except in ~/.rpmmacros
# echo "topdir: ${RPMBUILD}" > ${RPMRC_FILE}
echo "%_topdir %(echo ${RPMBUILD})" > ${HOME}/.rpmmacros
# Some spoofing required
# The problem is that some of the libraries and/or executables
# in the package may require libraries which are not installed via rpm.
# In our case, libOpenCL.so[.1] is installed as part of Catalyst, which
# may not have been installed via rpm.  So references to libOpenCL
# cannot possibly be satisfied, which will cause failed dependency messages
# at rpm --install time.
# The workaround is to simply exclude libOpenCL from appearing in the dependency list
# Another approach would be to turn off AutoReqProv (automatic requirements
# provisioning) but that seems to be far too coarse.
# Leave this here for documentation purposes
#
# What is required is to use a modified script in the RPM_SPECS directory
# And the spec file will set the __find_requires macro to that script

# At present, we get one single tarball containing CodeXL content
cp ${RPM_BRANCH}/CodeXL\_*.tar.gz ${RPM_SOURCES}
cp $RPM_BRANCH/CodeXL/Installer-Linux/rpm/CodeXL.spec ${RPM_SPECS}
cp $RPM_BRANCH/CodeXL/Installer-Linux/lcl-find-requires ${RPM_SPECS}

# Before we run the build, we do need to know the name of the tarball up to the .tar.gz
# The rpm spec file will utilize this
CXL_CONTENT=`basename ${RPM_BRANCH}/CodeXL_Linux*.tar.gz .tar.gz`
CXL_NAME=CodeXL_Linux
CXL_CONTENT_NDA=`basename ${RPM_BRANCH}/CodeXL_NDA_Only_Linux*.tar.gz .tar.gz`
CXL_NAME_NDA=CodeXL_NDA_Only_Linux
CXL_CONTENT_INTERNAL=`basename ${RPM_BRANCH}/CodeXL_Internal_Only_Linux*.tar.gz .tar.gz`
CXL_NAME_INTERNAL=CodeXL_Internal_Only_Linux
CXL_PKG_VERSION=`echo ${CXL_CONTENT} | awk -F_ '{print $5}'`


# And run the build
rpmbuild \
    --define "CXL_ver ${CXL_PKG_VERSION}" \
    --define "CXL_Content ${CXL_CONTENT}" \
    --define "CXL_name ${CXL_NAME}" \
    --ba ${RPM_SPECS}/CodeXL.spec

if [ "$all_variants" = "true" ]; then
  CXL_PKG_VERSION=`echo ${CXL_CONTENT_NDA} | awk -F_ '{print $7}'`

  rpmbuild \
      --define "CXL_ver ${CXL_PKG_VERSION}" \
      --define "CXL_name ${CXL_NAME_NDA}" \
      --define "CXL_Content ${CXL_CONTENT_NDA}" \
      --ba ${RPM_SPECS}/CodeXL.spec

  CXL_PKG_VERSION=`echo ${CXL_CONTENT_INTERNAL} | awk -F_ '{print $7}'`

  rpmbuild \
      --define "CXL_ver ${CXL_PKG_VERSION}" \
      --define "CXL_Content ${CXL_CONTENT_INTERNAL}" \
      --define "CXL_name ${CXL_NAME_INTERNAL}" \
      --ba ${RPM_SPECS}/CodeXL.spec
fi
#changing the name of the rpm-removing the "release" from the file name

#(ls -l ${RPMBUILD}/RPMS/x86_64/*release*.rpm | awk -F\  '{print $10}' && ls -l ${RPMBUILD}/RPMS/x86_64/*release*.rpm | awk -F\  '{print $10}' | sed '{s/release\.//}') | sort | xargs -l2 mv

# And our final result is sitting in ${RPMBUILD}/RPMS/x86_64/*.rpm
# We merely have to tell Jenkins to archive that.
