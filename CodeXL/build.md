CodeXL build instructions
===========================
## Windows
-------
#### Installations
* Latest AMD Radeon Software from http://support.amd.com/en-us/download
* Microsoft Visual Studio 2015 Community Edition or higher + Update 1
* Windows 10 SDK Version 10.0.10586.0 from https://developer.microsoft.com/en-US/windows/downloads/windows-10-sdk
* JDK from http://www.oracle.com/technetwork/java/javase/downloads/jdk8-downloads-2133151.html
  * CodeXL requires the Java JDK for CPU Profiling support of Java applications


#### Building CodeXL
* Open the following solution in Visual Studio: CodeXL\AllProjectsBuild\CodeXL-AllProjects.sln
* Build 64 bit configuration
* Build 32 bit configuration

#### CodeXL property sheets
* Most of project configurations are defined in dedicated property sheets
* In order to alter windows SDK installation directory use "Global-WindowsSDK" property sheet and change user macro "WindowsSDKDir"

## Linux
-------
CodeXL uses the SCons build system on Linux.
#### One time setup:
* sudo apt-get install gcc-multilib g++-multilib
* sudo apt-get install gcc-4.9-multilib g++-4.9-multilib (added for Ubuntu 15.10 or above)
* sudo apt-get install libglu1-mesa-dev mesa-common-dev libgtk2.0-dev
* sudo apt-get install zlib1g-dev libx11-dev:i386
* sudo apt-get install scons

#### Building on CENTOS 6.X
Install compiler 4.7.2
* sudo wget http://people.centos.org/tru/devtools-1.1/devtools-1.1.repo -P /etc/yum.repos.d
* sudo sh -c 'echo "enabled=1" >> /etc/yum.repos.d/devtools-1.1.repo'
* sudo yum install devtoolset-1.1
* wget http://people.centos.org/tru/devtools-1.1/6/i386/RPMS/devtoolset-1.1-libstdc++-devel-4.7.2-5.el6.i686.rpm
* sudo yum install devtoolset-1.1-libstdc++-devel-4.7.2-5.el6.i686.rpm
* sudo ln -s /opt/centos/devtoolset-1.1/root/usr/bin/* /usr/local/bin/
* hash -r
* gcc --version (verify that version 4.7.2 is displayed)
Install zlib
* yum install zlib-devel

Install glibc
* yum -y install glibc-devel.i686 glibc-devel

#### Building the HSA Profiler
* In order to build the HSA/ROCR profiler, the ROCR packages need to be installed (so that the header files are available at build time).
* The ROCR packages are available at https://github.com/RadeonOpenCompute/ROCR-Runtime

#### Building CodeXL
* CD to local copy of /CodeXL/Components/GpuProfiling/Build
* Run ./backend_build.sh
* CD to local copy of /CodeXL/Util/linux/
* Run ./buildCodeXLFullLinuxProjects

#### Build Switches
* all SCons general switches, like -c for clean , more info at http://scons.org/doc/HTML/scons-man.html
* –j specify the number of concurrent jobs (-j6).
* CXL_build=[debug/release] - build type
* CXL_build_verbose=1 - verbose output
* CXL_hsa=true build hsa support. without supplying this parameter hsa support is not built.
* When executing the backend_build.sh script, the following switches are supported:
  * __debug__: performs a debug build
  * __skip-32bitbuild__: skips building the 32-bit binaries
  * __skip-oclprofiler__: skips building the OpenCL profiler binaries
  * __skip-hsaprofiler__: skips building the HSA profiler binaries. If building on a system where you don't have HSA/ROCR header files, use this switch to avoid errors due to missing header files
  * __hsadir <dir>__: by default, when building the HSA Profiler binaries, the build scripts will look for the HSA/ROCR headers under /opt/rocm/hsa.  You can override this location using the "hsadir" switch.
  * __quick__ or __incremental__: performs an incremental build (as opposed to a from-scratch build)