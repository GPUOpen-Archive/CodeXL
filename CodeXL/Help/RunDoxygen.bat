@echo off
rem Copy the images used in the help file to target output folder:

XCopy /r /d /y "Images\General\*.png" "./Output\html\"
XCopy /r /d /y "Images\GPUDebugger\*.png" "./Output\html\"
XCopy /r /d /y "Images\CPUProfiler\*.png" "Output\html\"
XCopy /r /d /y "Images\GPUProfiler\*.png" "Output\html\"

rem Run the Doxygen script
"../../Common/DK/doxygen/doxygen-1.8.4/bin/doxygen.exe" "./HelpRelease.Doxyfile"
