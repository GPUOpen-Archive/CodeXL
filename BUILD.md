CodeXL build instructions
===========================

## Windows

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

* CodeXL uses the SCons build system on Linux.
* CodeXL require GCC version 4.8.5 or higher. 

#### Building on Ubuntu 15.04 or higher
* `sudo apt-get install gcc-multilib g++-multilib`
* `sudo apt-get install libglu1-mesa-dev mesa-common-dev libgtk2.0-dev`
* `sudo apt-get install zlib1g-dev libx11-dev:i386`
* `sudo apt-get install scons`
* `sudo apt-get install libjpeg9-dev`
* `sudo apt-get install libfltk1.3-dev`

##### Ubuntu 16.04 specific 
* `sudo apt-get install libboost-all-dev`

 Go to the [Building the JAVA Agent](#Building-the-JAVA-Agent) section

#### Building on CENTOS 7.X

##### Install SCons
* `wget http://prdownloads.sourceforge.net/scons/scons-2.5.0-1.noarch.rpm`
* `sudo rpm -ivh scons-2.5.0-1.noarch.rpm`

##### Install additional dependencies  
* `sudo yum install libX11-devel mesa-libGL-devel mesa-libGLU-devel`
* `sudo yum install zlib-devel gtk2-devel libpng12`
* `sudo yum install fltk-devel libjpeg-turbo-devel`

##### Install x86 dependencies
* `yum -y install glibc-devel.i686 libstdc++-static.i686`

#### Building the JAVA Agent: <a id="Building-the-JAVA-Agent"></a>
* Install Java JDK (version 1.7.x or higher) from Linux distribution (`sudo yum install java-1.8.0-openjdk-devel` \ `sudo apt-get install java-1.8.0-openjdk-devel`).
* Or download and install latest JDK for linux Oracle site: http://www.oracle.com/technetwork/java/javase/downloads/jdk8-downloads-2133151.html
*  Before build please define JAVA\_HOME variable for example : export JAVA\_HOME=/opt/java/jdk1.8.0\_77
* If JAVA\_HOME variable is not defined the build will skip the Java agent project.

#### Building the HSA/ROCm Profiler
* In order to build the HSA/ROCm profiler, the rocm-dev package needs to be installed (so that the ROCR header files are available at build time).
* The ROCm packages are available at https://github.com/RadeonOpenCompute/ROCm.  Please see the instructions in the [README.md](https://github.com/GPUOpen-Tools/CodeXL/releases) contained in that repository. To build CodeXL, only the **hsa-rocr-dev** package is needed.  In order to run and profile HSA/ROCm applications, the rocm package is needed.
* If the ROCR header files are not available on the build system, you can skip this part of the build.  See the Build Switches section below for information on how to do this.

#### Building CodeXL
* CD to local copy of /CodeXL/Util/linux/
* Run `./buildCodeXLFullLinuxProjects`
* CD to local copy of /CodeXL/Components/GpuProfiling/Build
* Run `./backend_build.sh`

#### Build Switches
* all SCons general switches, like -c for clean , more info at http://scons.org/doc/HTML/scons-man.html
* __-j__ specify the number of concurrent jobs (-j6).
* __CXL\_build=[debug|release]__ - build type. If not stated default value is release. 
* __CXL\_build\_verbose=1__ - verbose output
* __CXL\_boost\_dir=[path to boost libraries]__ - override the bundled boost libraries with files in given path
* __CXL\_hsa=[true|false]__ - define if to build HSA parts. If not stated default value is false (skip HSA)
* __-c__ - performs a "clean" of all build targets.
* When executing the backend\_build.sh script, the following switches are supported:
    * __debug__: performs a debug build
    * __skip-32bitbuild__: skips building the 32-bit binaries
    * __skip-oclprofiler__: skips building the OpenCL profiler binaries
    * __skip-hsaprofiler__: skips building the HSA profiler binaries. If building on a system where you don't have HSA/ROCR header files, use this switch to avoid errors due to missing header files
    * __hsadir 'dir'__: by default, when building the HSA Profiler binaries, the build scripts will look for the HSA/ROCR headers under /opt/rocm/hsa.  You can override this location using the "hsadir" switch.
    * __quick__ or __incremental__: performs an incremental build (as opposed to a from-scratch build)
    * __clean__: performs a "clean" of all build targets, removing all intermediate and final output files

#### Specific build instructions Ubuntu 16.04
* Since Ubuntu 16.04 comes with gcc 5.3, use the installed system boost libraries. example - 
./buildCodeXLFullLinuxProjects -j5 CXL\_build=debug CXL\_boost\_dir=/usr/lib/x86\_64-linux-gnu

#### Running CodeXL
If CodeXL displays an error indicating that it is unable to establish a connection with the CodeXL remote agent, copy the CodeXLRemoteAgentConfig.xml file into your CodeXL binary folder. The source file is in the folder CodeXL/CodeXL/Remote/AMDTRemoteAgent/CodeXLRemoteAgentConfig.xml


----------
