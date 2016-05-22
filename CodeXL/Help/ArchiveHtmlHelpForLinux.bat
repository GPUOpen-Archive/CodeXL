@echo off
REM 
REM This batch file archives the generated HTML files of the CodeXL User Guide
REM so the Linux build can bundle it in CodeXL tarball, RPM and Debian packages.
REM
REM Argument1 = Perforce Workspace root path. Usualy this is D:\jenkins\workspace\CodeXL-Doc\

@echo on
REM Remove any old archive
cd %1%
del /f /q ".\CodeXL\Help\CodeXL User Guide\CodeXL_User_Guide_HTML.tar.gz"

REM Add the location of the tar utility to the path so we can use it
set PATH=%PATH%;%1%\Common\DK\Gnu\cw1.3.12\bin

cd ".\CodeXL\Help\CodeXL User Guide\webhelp"

REM Archive all the files in the current directory
tar.exe cvf ..\CodeXL_User_Guide_HTML.tar.gz *