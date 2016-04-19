echo off

@rem We are in the main\Components\CodeXLDrivers\Windows\Scripts directory
echo cd
pushd %OUTDIR32%

REM c:\bin\Signtool sign /v /ac C:\bin\Keys\MSCV-VSClass3.cer /n "Advanced Micro Devices, Inc." /t http://timestamp.verisign.com/scripts/timstamp.dll .\pcore.sys
call d:\batch\sign.cmd .\pcore.sys
if %ERRORLEVEL% NEQ 0 GOTO error_during_signingactivity
c:\bin\signtool verify /kp /v .\pcore.sys
if %ERRORLEVEL% NEQ 0 GOTO error_during_signingactivity

REM c:\bin\Signtool sign /v /ac C:\bin\Keys\MSCV-VSClass3.cer /n "Advanced Micro Devices, Inc." /t http://timestamp.verisign.com/scripts/timstamp.dll .\CpuProf.sys
call d:\batch\sign.cmd .\CpuProf.sys
if %ERRORLEVEL% NEQ 0 GOTO error_during_signingactivity
c:\bin\signtool verify /kp /v .\CpuProf.sys
if %ERRORLEVEL% NEQ 0 GOTO error_during_signingactivity

REM c:\bin\Signtool sign /v /ac C:\bin\Keys\MSCV-VSClass3.cer /n "Advanced Micro Devices, Inc." /t http://timestamp.verisign.com/scripts/timstamp.dll .\AMDTPwrProf.sys
call d:\batch\sign.cmd .\AMDTPwrProf.sys
if %ERRORLEVEL% NEQ 0 GOTO error_during_signingactivity
c:\bin\signtool verify /kp /v .\AMDTPwrProf.sys
if %ERRORLEVEL% NEQ 0 GOTO error_during_signingactivity

popd

pushd %OUTDIR64%

REM c:\bin\Signtool sign /v /ac C:\bin\Keys\MSCV-VSClass3.cer /n "Advanced Micro Devices, Inc." /t http://timestamp.verisign.com/scripts/timstamp.dll .\pcore.sys
call d:\batch\sign.cmd .\pcore.sys
if %ERRORLEVEL% NEQ 0 GOTO error_during_signingactivity
c:\bin\signtool verify /kp /v .\pcore.sys
if %ERRORLEVEL% NEQ 0 GOTO error_during_signingactivity

REM c:\bin\Signtool sign /v /ac C:\bin\Keys\MSCV-VSClass3.cer /n "Advanced Micro Devices, Inc." /t http://timestamp.verisign.com/scripts/timstamp.dll .\CpuProf.sys
call d:\batch\sign.cmd .\CpuProf.sys
if %ERRORLEVEL% NEQ 0 GOTO error_during_signingactivity
c:\bin\signtool verify /kp /v .\CpuProf.sys
if %ERRORLEVEL% NEQ 0 GOTO error_during_signingactivity

REM c:\bin\Signtool sign /v /ac C:\bin\Keys\MSCV-VSClass3.cer /n "Advanced Micro Devices, Inc." /t http://timestamp.verisign.com/scripts/timstamp.dll .\AMDTPwrProf.sys
call d:\batch\sign.cmd .\AMDTPwrProf.sys
if %ERRORLEVEL% NEQ 0 GOTO error_during_signingactivity
c:\bin\signtool verify /kp /v .\AMDTPwrProf.sys
if %ERRORLEVEL% NEQ 0 GOTO error_during_signingactivity

popd
GOTO END

:error_during_signingactivity
ECHO Error during signing activity, aborting!
del /Q /F %DevRoot%\pkgsuccess

:END
