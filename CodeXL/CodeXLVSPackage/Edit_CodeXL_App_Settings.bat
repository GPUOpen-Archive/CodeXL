:@ECHO off
REM first parameter should be the full path location of the CodeXL output binaries.
REM second parameter should be the cuurent VSIX folder. 
SET CodeXL_FOLDER=%1
SET VSIX_Folder=%2
IF "%1" == "" (
Echo Must set location path of CodeXL binaries.
Exit /b 
)
IF NOT EXIST CodeXL_App_Settings.txt (
> CodeXL_App_Settings.txt (
Echo [CodeXL]
Echo CodeXL_Path=%CodeXL_FOLDER%)
echo copy file to %VSIX_Folder%)
xcopy /r /d /y /f CodeXL_App_Settings.txt %VSIX_Folder%* 
xcopy /r /d /y ..\..\Common\Src\AMDTApplication\res\*.ico %VSIX_Folder%