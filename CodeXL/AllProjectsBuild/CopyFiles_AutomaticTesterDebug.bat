@echo off

XCopy /y "..\..\Common\Lib\AMD\OpenCLDebugAPI\1.3\lib\x64\AMDOpenCLDebug-x64-d.dll" "..\Output\Debug\bin\"
XCopy /y "..\..\Common\Lib\AMD\OpenCLDebugAPI\1.3\lib\x64\amdopencldebug-x64-d.pdb" "..\Output\Debug\bin\"
XCopy /y "..\..\Common\Lib\AMD\OpenCLDebugAPI\1.3\lib\x86\AMDOpenCLDebug-d.dll" "..\Output\Debug\bin\"
XCopy /y "..\..\Common\Lib\AMD\OpenCLDebugAPI\1.3\lib\x86\amdopencldebug-d.pdb" "..\Output\Debug\bin\"

echo Copying MS Detours .dll files
echo =============================
XCopy /y "..\..\Common\Lib\Ext\Detours\x64\detoured.dll" "..\Output\Debug\bin\spies64\"
XCopy /y "..\..\Common\Lib\Ext\Detours\x64\detoured.dll" "..\Output\Release\bin\spies64\"
XCopy /y "..\..\Common\Lib\Ext\Detours\x86\detoured.dll" "..\Output\Debug\bin\spies\"
XCopy /y "..\..\Common\Lib\Ext\Detours\x86\detoured.dll" "..\Output\Release\bin\spies\"


echo Copying DbgHelp.dll
echo ===================
echo.
XCopy /y "..\..\Common\Lib\Ext\MicrooftDebuggingToolsForWindows\6.12.2.633\bin\dbghelp.dll" "..\Output\Debug\bin\"

