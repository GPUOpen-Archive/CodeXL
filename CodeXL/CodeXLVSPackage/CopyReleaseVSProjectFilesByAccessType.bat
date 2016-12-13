:@ECHO OFF
REM first parameter should be "", "VS11" or "VS12", for the file names.
IF "%1" == "" (
SET VSVER_FOLDER=VS10
GOTO VSVER_FOLDER_DEFINED
)
SET VSVER_FOLDER=%1

:VSVER_FOLDER_DEFINED
IF "%AMDT_BUILD_SUFFIX%" == "" (
SET VSBINS_FOLDER_SUFFIX=-public
GOTO VSBINS_FOLDER_SUFFIX_DEFINED
)
SET VSBINS_FOLDER_SUFFIX=%AMDT_BUILD_SUFFIX%

:VSBINS_FOLDER_SUFFIX_DEFINED
IF NOT EXIST ..\Output\Release%VSBINS_FOLDER_SUFFIX%\bin\ MKDIR ..\Output\Release%VSBINS_FOLDER_SUFFIX%\bin\%VSVER_FOLDER%\
FOR %%E IN (dll pdb) DO xcopy /r /d /y CodeXLVSPackage\Release\CodeXLVSPackage%1.%%E ..\Output\Release%VSBINS_FOLDER_SUFFIX%\bin\%VSVER_FOLDER%\
FOR %%E IN (dll pdb) DO xcopy /r /d /y CodeXLVSPackageUi\Release\CodeXLVSPackage%1UI.%%E ..\Output\Release%VSBINS_FOLDER_SUFFIX%\bin\%VSVER_FOLDER%\
FOR %%E IN (dll pdb) DO xcopy /r /d /y CodeXLVSPackage\CodeXLVSPackage%1VSIX\Release\CodeXLVSPackage%1VSIX.%%E ..\Output\Release%VSBINS_FOLDER_SUFFIX%\bin\%VSVER_FOLDER%\
xcopy /r /d /y CodeXLVSPackage\CodeXLVSPackage%1VSIX\Release\extension.vsixmanifest ..\Output\Release%VSBINS_FOLDER_SUFFIX%\bin\%VSVER_FOLDER%\
