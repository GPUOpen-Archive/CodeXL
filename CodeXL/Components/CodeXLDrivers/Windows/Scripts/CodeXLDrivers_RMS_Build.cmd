call D:\Batch\VS_2013.cmd x86_amd64

SET WINVERCONFIG=Win7
SET OUTDIRBASE=%DEVROOT%\CodeXL\Components\CodeXLDrivers\Windows
IF NOT EXIST %OUTDIRBASE%\Output mkdir %OUTDIRBASE%\Output
IF NOT EXIST %OUTDIRBASE%\Output\Release mkdir %OUTDIRBASE%\Output\Release
IF NOT EXIST %OUTDIRBASE%\Output\Release\bin mkdir %OUTDIRBASE%\Output\Release\bin
IF NOT EXIST %OUTDIRBASE%\Output\Release\bin\x86 mkdir %OUTDIRBASE%\Output\Release\bin\x86
IF NOT EXIST %OUTDIRBASE%\Output\Release\bin\x64 mkdir %OUTDIRBASE%\Output\Release\bin\x64

SET OUTDIR32=%OUTDIRBASE%\Output\Release\bin\x86
SET OUTDIR64=%OUTDIRBASE%\Output\Release\bin\x64

@rem set the WDK 8.1 environment
call D:\Batch\winddk81.cmd

@rem We are in the CodeXL\Components\CodeXLDrivers\Windows\Scripts directory
echo cd
@rem set the path to the pcore libraries from current directory
@rem SET PCORE_ROOT_DIR=..\..\..\..\..\amdpcore

pushd ".."

@rem build 32-bit Windows driver
ECHO build pcore
call msbuild /t:clean /t:build CodeXLDrivers.sln /p:Configuration="%WINVERCONFIG% Release" /p:Platform=Win32
if %ERRORLEVEL% NEQ 0 del /Q /F %DevRoot%\success
if %ERRORLEVEL%==0 copy /Y %OUTDIRBASE%\Output\bin\Win32\%WINVERCONFIG%Release\pcore.sys %OUTDIR32%
if %ERRORLEVEL%==0 copy /Y %OUTDIRBASE%\Output\bin\Win32\%WINVERCONFIG%Release\pcore.pdb %OUTDIR32%
if %ERRORLEVEL%==0 copy /Y %OUTDIRBASE%\Output\bin\Win32\%WINVERCONFIG%Release\CpuProf.sys %OUTDIR32%
if %ERRORLEVEL%==0 copy /Y %OUTDIRBASE%\Output\bin\Win32\%WINVERCONFIG%Release\CpuProf.pdb %OUTDIR32%
if %ERRORLEVEL%==0 copy /Y %OUTDIRBASE%\Output\bin\Win32\%WINVERCONFIG%Release\AMDTPwrProf.sys %OUTDIR32%
if %ERRORLEVEL%==0 copy /Y %OUTDIRBASE%\Output\bin\Win32\%WINVERCONFIG%Release\AMDTPwrProf.pdb %OUTDIR32%
if %ERRORLEVEL%==0 C:\bin\touch.exe %DEVROOT%\success

@rem build 64-bit Windows driver
ECHO build pcore
call msbuild /t:clean /t:build CodeXLDrivers.sln /p:Configuration="%WINVERCONFIG% Release" /p:Platform=x64
if %ERRORLEVEL% NEQ 0 del /Q /F %DevRoot%\success
if %ERRORLEVEL%==0 copy /Y %OUTDIRBASE%\Output\bin\x64\%WINVERCONFIG%Release\pcore.sys %OUTDIR64%
if %ERRORLEVEL%==0 copy /Y %OUTDIRBASE%\Output\bin\x64\%WINVERCONFIG%Release\pcore.pdb %OUTDIR64%
if %ERRORLEVEL%==0 copy /Y %OUTDIRBASE%\Output\bin\x64\%WINVERCONFIG%Release\CpuProf.sys %OUTDIR64%
if %ERRORLEVEL%==0 copy /Y %OUTDIRBASE%\Output\bin\x64\%WINVERCONFIG%Release\CpuProf.pdb %OUTDIR64%
if %ERRORLEVEL%==0 copy /Y %OUTDIRBASE%\Output\bin\x64\%WINVERCONFIG%Release\AMDTPwrProf.sys %OUTDIR64%
if %ERRORLEVEL%==0 copy /Y %OUTDIRBASE%\Output\bin\x64\%WINVERCONFIG%Release\AMDTPwrProf.pdb %OUTDIR64%
if %ERRORLEVEL%==0 C:\bin\touch.exe %DEVROOT%\success

popd
