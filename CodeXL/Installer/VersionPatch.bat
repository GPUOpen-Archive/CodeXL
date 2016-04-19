@echo off
rem VersionPatch.bat
rem ================
rem Call the script that patches the file version and product version information embedded in DLL and EXE files.

rem Arg1 = Full path of the PowerShell script file
rem Arg2 = Full path for workspace root folder
rem Arg3 = Product Version 
rem Arg4 = GDT_Suffix (Public or NDA or Internal)
rem Arg5  = (Optional) "-debug True" - will output detailed information. 

Call Powershell -ExecutionPolicy Unrestricted -command %1 -workspace %2 -VersionNumber %3 -GDT_Build_Suffix %4
