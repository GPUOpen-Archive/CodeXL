@ECHO OFF

REM AMD64 is set to 1 for x64 driver build env
IF "%AMD64%"=="" (SET ExtDir=i386) ELSE (SET ExtDir=amd64)
IF "%AMD64%"=="" (SET BuildEnd=32) ELSE (SET BuildEnd=64)
IF "%AMD64%"=="" (SET BinExt=x86) ELSE (SET BinExt=x64)

SET PCORE_ROOT_DIR=..\..\..\..\..\..\..\..\amdpcore\Src\amd-pcore
SET DEVROOT=%CD%\..

IF NOT EXIST %DEVROOT%\Output mkdir %DEVROOT%\Output
IF NOT EXIST %DEVROOT%\Output\Debug mkdir %DEVROOT%\Output\Debug
IF NOT EXIST %DEVROOT%\Output\Debug\bin mkdir %DEVROOT%\Output\Debug\bin
IF NOT EXIST %DEVROOT%\Output\Debug\bin\%BinExt% mkdir %DEVROOT%\Output\Debug\bin\%BinExt%

SET OUTDIR=%DEVROOT%\Output\Debug\bin\%BinExt%

pushd %PCORE_ROOT_DIR%
  
IF NOT EXIST obj%BUILD_ALT_DIR%\%ExtDir% mkdir obj%BUILD_ALT_DIR%\%ExtDir%

ECHO build pcore
IF EXIST obj%BUILD_ALT_DIR%\%ExtDir%\pcore.sys del obj%BUILD_ALT_DIR%\%ExtDir%\pcore.sys
call nmake /f "amd-pcore%BuildEnd%.mak" all

copy /Y obj%BUILD_ALT_DIR%\%ExtDir%\pcore.sys %OUTDIR%
copy /Y obj%BUILD_ALT_DIR%\%ExtDir%\pcore.pdb %OUTDIR%

popd

@rem build personality Windows driver
pushd "..\src"

@rem set the path to the pcore libraries from current directory
SET PCORE_DIR=%PCORE_ROOT_DIR%\obj%BUILD_ALT_DIR%

IF NOT EXIST obj%BUILD_ALT_DIR%\%ExtDir% mkdir obj%BUILD_ALT_DIR%\%ExtDir%

ECHO build CpuProf Personality
IF EXIST obj%BUILD_ALT_DIR%\%ExtDir%\CpuProf.sys del obj%BUILD_ALT_DIR%\%ExtDir%\CpuProf.sys
call nmake /f "CpuProf%BuildEnd%.mak" all
copy /Y obj%BUILD_ALT_DIR%\%ExtDir%\CpuProf.sys %OUTDIR%
copy /Y obj%BUILD_ALT_DIR%\%ExtDir%\CpuProf.pdb %OUTDIR%

popd


ECHO build AMDTPwrProf Personality
IF EXIST obj%BUILD_ALT_DIR%\%ExtDir%\AMDTPwrProf.sys del obj%BUILD_ALT_DIR%\%ExtDir%\AMDTPwrProf.sys
call nmake /f "AMDTPwrProf%BuildEnd%.mak" all
copy /Y obj%BUILD_ALT_DIR%\%ExtDir%\AMDTPwrProf.sys %OUTDIR%
copy /Y obj%BUILD_ALT_DIR%\%ExtDir%\AMDTPwrProf.pdb %OUTDIR%

popd

IF NOT EXIST UnitTesting.cer ECHO Note: Needs test signature. Run "InstallTestCertOnTargetAsAdmin.cmd" as Admin and try again

pushd %OUTDIR%
ECHO Signing drivers
Signtool sign /v /a /s "CpuProf Test Store" /n "CpuProf Unit Test" /t http://timestamp.verisign.com/scripts/timstamp.dll .\pcore.sys
Signtool sign /v /a /s "CpuProf Test Store" /n "CpuProf Unit Test" /t http://timestamp.verisign.com/scripts/timstamp.dll .\CpuProf.sys
Signtool sign /v /a /s "CpuProf Test Store" /n "CpuProf Unit Test" /t http://timestamp.verisign.com/scripts/timstamp.dll .\AMDTPwrProf.sys
popd

ECHO Test Drivers built and test signed.
