CodeXL build instructions
===========================

## Cloning
CodeXL has several submodules. Clone with the `--recursive` flag to download all the source code required to build. For example:

```bash
git clone --recursive https://github.com/GPUOpen-Tools/CodeXL
```

To initialize the submodules after cloning the main repository run the following commands:

```bash
git submodule init
git submodule update
```

CodeXL uses:

* Radeon Compute Profiler for profiling OpenCL and HSA (Linux only) compute applications. See https://github.com/GPUOpen-Tools/RCP.
* Radeon Graphics Analyzer for analyzing Shaders and OpenCL Kernels. See https://github.com/GPUOpen-Tools/RGA.

To fetch these command line tools run the following command:

```bash
python CodeXL/Scripts/FetchDependencies.py
```

## Windows

#### Installations
* Latest AMD Radeon Software from http://support.amd.com/en-us/download
* Microsoft Visual Studio 2015 Community Edition or higher + Update 1
* Windows 10 SDK Version 10.0.10586.0 from https://developer.microsoft.com/en-US/windows/downloads/windows-10-sdk
* QT 5.9.5 from https://download.qt.io/archive/qt/5.9/5.9.5/qt-opensource-windows-x86-5.9.5.exe. Make sure you choose the "msvc2015 32-bit" and "Qt WebEngine" options at a minimum while installing.

#### Building CodeXL
* Open the following solution in Visual Studio: CodeXL\AllProjectsBuild\CodeXL-AllProjects.sln
* Build 64 bit configuration
* Build 32 bit configuration

#### CodeXL property sheets
* Most of project configurations are defined in dedicated property sheets
* In order to alter windows SDK installation directory use "Global-WindowsSDK" property sheet and change user macro "WindowsSDKDir"

#### Using an alternate Qt installation directory
* If Qt 5.9.5 has been installed in the default path, nothing additional needs to be done. If Qt 5.9.5 has been installed in a different location, prior to building CodeXL, set an environment variable "QT_DIR" with a value equal to the msvc2015 directroy in the Qt installation (the equivalent of C:\Qt\5.9.5\5.9.5\msvc2015 in a default installation).

## Linux

* CodeXL uses the SCons build system on Linux.
* CodeXL requires GCC version 4.8.5 or higher.
* CodeXL requires Qt version 5.9.5. Please install from https://download.qt.io/archive/qt/5.9/5.9.5/qt-opensource-linux-x64-5.9.5.run. Make sure you choose the "Desktop gcc 64-bit" and "Qt WebEngine" options at a minimum while installing.

#### Building on Ubuntu 15.04 or higher
* `sudo apt-get install gcc-multilib g++-multilib`
* `sudo apt-get install libglu1-mesa-dev mesa-common-dev libgtk2.0-dev`
* `sudo apt-get install zlib1g-dev libx11-dev:i386`
* `sudo apt-get install scons`
* `sudo apt-get install libjpeg9-dev`
* `sudo apt-get install libfltk1.3-dev`

##### Ubuntu 16.04 specific
* `sudo apt-get install libboost-all-dev`

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

#### Building CodeXL
* CD to local copy of /CodeXL/Util/linux/
* Run `./buildCodeXLFullLinuxProjects`

#### Build Switches
* all SCons general switches, like -c for clean , more info at http://scons.org/doc/HTML/scons-man.html
* __-j__ specify the number of concurrent jobs (-j6).
* __CXL\_build=[debug|release]__ - build type. If not stated, the default value is release.
* __CXL\_build\_verbose=1__ - verbose output
* __CXL\_boost\_lib\_dir=[path to boost libraries]__ - override the bundled boost libraries with files in given path
* __CXL\_boost\_include\_dir=[path to boost headers]__ - override the bundled boost headers with files in given path
* __CXL\_qt\_dir=[path to qt gcc_64 dir]__ - override the Qt base __gcc_64__ directory if Qt was not installed in the default location (~/Qt5.9.5/5.9.5/gcc_64 or /opt/Qt5.9.5/5.9.5/gcc_64)
* __CXL\_hsa=[true|false]__ - define to __true__ in order to build the HSA parts. If not stated, the default value is false (skip HSA)
* __-c__ - performs a "clean" of all build targets.

#### Specific build instructions recent Ubuntu versions
* Since recent Ubuntu versions come with newer gcc versions, use the installed system boost libraries:

```bash
./buildCodeXLFullLinuxProjects -j5 CXL\_build=debug CXL\_boost\_lib\_dir=/usr/lib/x86\_64-linux-gnu CXL\_boost\_include\_dir=/usr/include/boost
```

#### Running CodeXL
The folder containing the CodeXL binaries can be found in the root folder of CodeXL (Output_x86_64).

* If a license agreement is not displayed, copy the file CodeXL/Setup/Legal/Public/EndUserLicenseAgreement-Linux.htm to the Legal subdirectory in the CodeXL binaries folder (Output_x86_64/release/bin/Legal)
* If CodeXL displays an error indicating that it is unable to establish a connection with the CodeXL remote agent, copy the CodeXLRemoteAgentConfig.xml file into your CodeXL binary folder. The source file is in the folder CodeXL/CodeXL/Remote/AMDTRemoteAgent/CodeXLRemoteAgentConfig.xml

----------
