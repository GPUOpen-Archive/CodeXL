@echo off

set CONFIG_NAME=%1

rem Create the x86 subfolders:
if not exist ..\..\..\Output\%CONFIG_NAME%\bin\x86 mkdir ..\..\..\Output\%CONFIG_NAME%\bin\x86
if not exist ..\..\..\Output\%CONFIG_NAME%\bin\x86\x86 mkdir ..\..\..\Output\%CONFIG_NAME%\bin\x86\x86

rem Copy core files:
XCopy /r /d /y "..\..\..\..\Common\Lib\AMD\RGA\x86\bin\rga.exe" "..\..\..\Output\%CONFIG_NAME%\bin\x86"
XCopy /r /d /y "..\..\..\..\Common\Lib\AMD\RGA\x86\bin\x86\amdspv.exe" "..\..\..\Output\%CONFIG_NAME%\bin\x86\x86"
XCopy /r /d /y "..\..\..\..\Common\Lib\AMD\RGA\x86\bin\x86\spvgen.dll" "..\..\..\Output\%CONFIG_NAME%\bin\x86\x86"
XCopy /r /d /y "..\..\..\..\Common\Lib\AMD\RGA\x86\bin\x86\shae.exe" "..\..\..\Output\%CONFIG_NAME%\bin\x86\x86"
XCopy /r /d /y "..\..\..\..\Common\Lib\AMD\RGA\x86\bin\x86\VirtualContext.exe" "..\..\..\Output\%CONFIG_NAME%\bin\x86\x86"

rem Create the x64 subfolders:
if not exist ..\Output\%CONFIG_NAME%\bin\x64 mkdir ..\..\..\Output\%CONFIG_NAME%\bin\x64
if not exist ..\Output\%CONFIG_NAME%\bin\x64\x64 mkdir ..\..\..\Output\%CONFIG_NAME%\bin\x64\x64 

rem Copy core files:
XCopy /r /d /y "..\..\..\..\Common\Lib\AMD\RGA\x64\bin\rga.exe" "..\..\..\Output\%CONFIG_NAME%\bin\x64"
XCopy /r /d /y "..\..\..\..\Common\Lib\AMD\RGA\x64\bin\x64\amdspv.exe" "..\..\..\Output\%CONFIG_NAME%\bin\x64\x64"
XCopy /r /d /y "..\..\..\..\Common\Lib\AMD\RGA\x64\bin\x64\spvgen.dll" "..\..\..\Output\%CONFIG_NAME%\bin\x64\x64"
XCopy /r /d /y "..\..\..\..\Common\Lib\AMD\RGA\x64\bin\x64\shae.exe" "..\..\..\Output\%CONFIG_NAME%\bin\x64\x64"
XCopy /r /d /y "..\..\..\..\Common\Lib\AMD\RGA\x64\bin\x64\VirtualContext.exe" "..\..\..\Output\%CONFIG_NAME%\bin\x64\x64"
