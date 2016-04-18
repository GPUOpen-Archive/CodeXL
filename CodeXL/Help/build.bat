mkdir Output
if EXIST Output\CodeXLHelp.chm del /Q /F Output\CodeXLHelp.chm
..\..\Common\DK\Doxygen\doxygen-1.8.1\bin\doxygen.exe HelpRelease.Doxyfile
