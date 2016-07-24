## Teapot

The Teapot sample application draws a spinning teapot with steam coming out of its spout. The steam vapors movement is simulated using OpenCL. The rendering of the teapot and steam is implemented using OpenGL. In the Linux version of Teapot, the window, menu bar and UI widgets are implemented using the FLTK library.
The Teapot application can be used to demonstrate the features of CodeXL’s GPU Debugger, GPU Profiler and Static Analyzer. 
 
## Building the Teapot sample application

CodeXL includes a pre-built binary of the Teapot application. Follow the steps below if you wish to build it yourself.

### On Windows

Follow these steps to build AMDTTeapot on Windows from the default install location:
* Launch Visual Studio with administrator privileges. 
* From the CodeXL menu, select the ‘Open Teapot Sample Project’ command.
* Wait for the project to open and press F7.
* **Note:** Teapot is preconfigured to use Windows 10 SDK release 10.0.10586.0. You can build Teapot with other Windows SDK releases by setting the Visual Studio project settings: Project > right-click > properties > General > target platform version.

### On Linux

Follow these steps to build AMDTTeapot on Linux:
* Install the FLTK library
  * The bundled binaries of AMD Teapot were built with 64-bit binaries of FLTK 1.1.10.
  * Download FLTK sources from http://www.fltk.org
  * Extract and build the fltk libraries from the source files, then install the created fltk libraries
* Modifying The AMDTTeaPot Makefile
  * There are two makefiles in the Teapot sample folder. One is the /examples/Teapot/AMDTTeaPot/Makefile which you’ll need to edit, and the other is the /examples/Teapot/AMDTTeaPotLib/Makefile that does not need to be changed.
  * Open /examples/Teapot/AMDTTeaPot/Makefile in a text editor.
  * Replace the -L"Replace with path to your local FLTK lib folder" with the path of your local FLTK libraries, for example –L/usr/lib64 in case you placed the libraries in the system folder and you are running a 64bit system
  * Replace the -I"Replace with path to your local FLTK headers folder" with the path of your local FLTK include files. Please note that the FLTK headers are placed in a folder named “FL” and the path you provide should point to the parent of the “FL” folder. Example –I/user/include if you placed them in the system include and not –I/user/include/FL
* Building the Teapot
  * Teapot consists of a library and an application so first the library needs to be built.
  * cd to the /examples/Teapot/AMDTTeaPotLib folder
  * make all
  * if everything went well you’ll see libAMDTTeaPot.a in that folder
  * Now the main application can be built.
  * cd to /examples/Teapot/AMDTTeaPot
  * make all
  * If successful the output should be in /examples/Teapot/release. Look for a file named AMDTTeaPot-bin.

Note that CodeXL RPM and Debian packages install CodeXL and is bundled sample applications under the /opt folder, which requires elevated privileges to write to on some Linux distributions.
