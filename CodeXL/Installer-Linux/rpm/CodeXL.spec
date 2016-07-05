#--------------------------------------------------------------------------
# This section should be in a separate file
#
# NOTE: The following must be defined from command line
#   CXL_ver     The version on the rpm
#   CXL_Content The name of the CodeXL .tar.gz from an upstream job
#               less the .tar.gz

%define test64			%(test "`uname -i`" = "x86_64"; echo $?)
%define is64bit()		%{expand:%%{?%{test64}:1}%%{!?%{test64}:0}}
%define is32bit()		%{expand:%%{?%{test64}:0}%%{!?%{test64}:1}}

# Our directory layout at present is truly primitive and needs serious improvement
%define CXL_install_dir     /opt/CodeXL_%{CXL_ver}
%define CXL_app_icon_dir    /usr/share/applications
%define CXL_examples_dir    /usr/share/CodeXL_%{CXL_ver}

#get release
%define OS_Release %(lsb_release -sd)
%define OS_Suse %(lsb_release -sd | grep -i suse)
%define OS_CentOS %(lsb_release -sd | grep -i cent)

#%if %{is64bit}
#%define CXL_lib_dir		%{CXL_install_dir}/lib64
#%else
#%define CXL_lib_dir		%{CXL_install_dir}/lib
#%endif

%define SCONS_NUM_THREAD	-j4

# No stripping
%define __strip /bin/true

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
# First, we have to turn off the use of the 'default' checker, and use a modified script

# Apr 3 2013 Gilad Yarnitzky
# %define _use_internal_dependency_generator 0
# %define __find_requires %{_topdir}/SPECS/lcl-find-requires
# Manually supply the entire list of dependency since we need also the list from the teapot
# and do not want to depend on a script from the system that might change in later systems
AutoReqProv: no

#--------------------------------------------------------------------------
# Header
# 
Summary:       CodeXL 
Name:          %{CXL_name}
Version:       %{CXL_ver}
Release:       0
License:       TBD
URL:           TBD

Source0:       %{CXL_Content}.tar.gz
#Source0:       CodeXL-Full.tar.gz
#Source1:       AMDAPPProfiler-2.6.tar.gz

BuildRoot:     %(mktemp -ud %{_tmppath}/%{name}_%{version}_XXXXXX)

Requires:	gcc   >= 3.0
Requires:	glibc >= 2.12

# required by teapot:
Requires:	mesa-libGL >= 7.0
Requires:	mesa-libGLU >= 7.0
Requires:	libxml2 >= 2.0
Requires:	glib2 >= 2.0
Requires:	libXext >= 1.0
Requires:	zlib >= 1.0
Requires:	libXmu >= 1.0
Requires:	libXt >= 1.0
Requires:	libSM >= 1.0
Requires:	libICE >= 1.0
Requires:	libX11 >= 1.0
Requires:	atk >= 1.0
Requires:	cairo >= 1.0
Requires:	freetype >= 2.0
Requires:	fontconfig >= 2.0
Requires:	libXfixes >= 4.0
Requires:	libXrender >= 0.9
Requires:	libXinerama >= 1.0
Requires:	libXi >= 1.0
Requires:	libXrandr >= 1.0
Requires:	libXcursor >= 1.0
Requires:	libXcomposite >= 0.4
Requires:	libXdamage >= 1.0
Requires:	libuuid >= 2.0
Requires:	libxcb >= 1.0
Requires:	libselinux >= 2.0
Requires:	libpng >= 1.0
Requires:	pixman >= 0.15
Requires:	expat >= 2.0
Requires:	libXau >= 1.0

%description
CodeXL suite of development tools.

#--------------------------------------------------------------------------
%prep
# This is the top level directory we will build into/under
%setup -q -n %{CXL_Content}

#--------------------------------------------------------------------------
%install
rm -rf ${RPM_BUILD_ROOT}

echo %{CXL_Content}

mkdir -p ${RPM_BUILD_ROOT}/%{CXL_install_dir}
mkdir -p ${RPM_BUILD_ROOT}/%{CXL_app_icon_dir}
mkdir -p ${RPM_BUILD_ROOT}/%{CXL_examples_dir}

# Install CXL-Full
# This should come from the CodeXL-Full-Linux build artifacts which contains
# CodeXL main, CpuProfiling, GpuProfiling, GpuDebugging
cp -prf * 		${RPM_BUILD_ROOT}/%{CXL_install_dir}
cd ${RPM_BUILD_ROOT}/%{CXL_install_dir}
/usr/sbin/prelink -u ${RPM_BUILD_ROOT}/%{CXL_install_dir}/RuntimeLibs/x86_64/libstdc++.so.6.0.19
sed -i -e "s|CodeXL_Version_Menu|CodeXL_"%{CXL_ver}"|g" amdcodexlicon.desktop amdremoteagenticon.desktop
mv amdcodexlicon.desktop ${RPM_BUILD_ROOT}/%{CXL_app_icon_dir}
chmod +xw ${RPM_BUILD_ROOT}/%{CXL_app_icon_dir}/amdcodexlicon.desktop
echo "Exec=%{CXL_install_dir}/CodeXL" >> ${RPM_BUILD_ROOT}/%{CXL_app_icon_dir}/amdcodexlicon.desktop
echo "Icon=%{CXL_install_dir}/Images/ApplicationIcon_64.ico" >> ${RPM_BUILD_ROOT}/%{CXL_app_icon_dir}/amdcodexlicon.desktop
echo "Path=%{CXL_install_dir}/" >> ${RPM_BUILD_ROOT}/%{CXL_app_icon_dir}/amdcodexlicon.desktop
echo "Version=%{CXL_ver}" >> ${RPM_BUILD_ROOT}/%{CXL_app_icon_dir}/amdcodexlicon.desktop
mv amdremoteagenticon.desktop ${RPM_BUILD_ROOT}/%{CXL_app_icon_dir}
chmod +xw ${RPM_BUILD_ROOT}/%{CXL_app_icon_dir}/amdremoteagenticon.desktop
echo "Exec=%{CXL_install_dir}/CodeXLRemoteAgent" >> ${RPM_BUILD_ROOT}/%{CXL_app_icon_dir}/amdremoteagenticon.desktop
echo "Icon=%{CXL_install_dir}/Images/ApplicationIcon_64.ico" >> ${RPM_BUILD_ROOT}/%{CXL_app_icon_dir}/amdremoteagenticon.desktop
echo "Path=%{CXL_install_dir}/" >> ${RPM_BUILD_ROOT}/%{CXL_app_icon_dir}/amdremoteagenticon.desktop
echo "Version=%{CXL_ver}" >> ${RPM_BUILD_ROOT}/%{CXL_app_icon_dir}/amdremoteagenticon.desktop
mv ${RPM_BUILD_ROOT}/%{CXL_install_dir}/examples/ ${RPM_BUILD_ROOT}/%{CXL_examples_dir}/examples/

# Install GpuProfiling backend stuff
#   Comes along for free from copy of bin

#--------------------------------------------------------------------------
%files
%defattr(-,root,root,-)
%{CXL_install_dir}/*
%{CXL_app_icon_dir}/amdcodexlicon.desktop
%{CXL_app_icon_dir}/amdremoteagenticon.desktop
%{CXL_examples_dir}/*

#--------------------------------------------------------------------------
%clean
rm -rf ${RPM_BUILD_ROOT}

#--------------------------------------------------------------------------
# Upgrade issues:
#   According to the documentation, an upgrade operates as follows:
#   - an install of the new bits happens
#       %pre, [install], %post
#   - an erase of the old bits happens
#       %preun, [erase], %postun
# Which makes it tricky for us to create a symbolic link as part of the
# install.  According to:
#       http://www.rpm.org/max-rpm-snapshot/s1-rpm-inside-scripts.html
# the script sections get passed a single argument: the count of the number
# of copies installed after completion of the action.  Thus, in the upgrade
# case, we can expect to see:
#   %pre 2; [install] %post 2; %preun 1; [erase]; %postun 1
# Implementation would seem to have two choices:
#   %pre - rm -f of link && %post - create the link
# or
#   %post - install if the count is 1 && %postun - rm -f of link if the count is zero
# Go with the latter.
#--------------------------------------------------------------------------
%pre
exit 0


#--------------------------------------------------------------------------
%post
ln -s %{CXL_examples_dir}/examples/ %{CXL_install_dir}/examples
chmod -R 777 %{CXL_examples_dir}
cd  %{CXL_install_dir}
sh  CodeXLPwrProfDriver.sh install
exit 0


#--------------------------------------------------------------------------
%preun
if [ $1 -eq 0 ]; then 
    cd  %{CXL_install_dir}
    sh  CodeXLPwrProfDriver.sh uninstall
fi
exit 0


#--------------------------------------------------------------------------
%postun
# Get rid of symbolic link
rm -f %{CXL_install_dir}/CodeXL
# Cleanup of empty directories
rmdir --ignore-fail-on-non-empty %{CXL_install_dir}
rm -rf /usr/share/applications/amdcodexlicon.desktop
rm -rf /usr/share/applications/amdremoteagenticon.desktop
exit 0

