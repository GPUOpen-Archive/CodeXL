:@ECHO OFF
cd

set VSIX_COPY_BATCH_FILE_FOLDER=%~dp0
set VSIX_COPY_VSPACKAGE_FOLDER=%VSIX_COPY_BATCH_FILE_FOLDER%
set VSIX_COPY_CODEXL_FOLDER=%VSIX_COPY_VSPACKAGE_FOLDER%..\
set VSIX_COPY_COMMON_FOLDER=%VSIX_COPY_CODEXL_FOLDER%..\Common\
set VSIX_COPY_COMMONPROJECTS_FOLDER=%VSIX_COPY_CODEXL_FOLDER%..\CommonProjects\

IF "%3"=="" (
set VSIX_COPY_TARGET_DIR=%CD%
ECHO copying to current dir "%CD%"
GOTO VSIX_COPY_TARGET_DIR_DEFINED
)
set VSIX_COPY_TARGET_DIR=%3
ECHO copying to parameter dir "%3"

:VSIX_COPY_TARGET_DIR_DEFINED
IF "%2"=="" GOTO BINARIES_PATH_NOT_DEFINED


IF "%1"=="Debug" ( 
xcopy /r /y %VSIX_COPY_COMMON_FOLDER%Lib\Ext\Qt\5.5\plugins\platforms\qwindowsd.dll %VSIX_COPY_TARGET_DIR%
xcopy /r /y %VSIX_COPY_COMMON_FOLDER%Lib\Ext\QT\5.5\plugins\platforms\qwindowsd.pdb %VSIX_COPY_TARGET_DIR%
)

IF "%1"=="Release" ( 
xcopy /r /y %VSIX_COPY_COMMON_FOLDER%Lib\Ext\QT\5.5\plugins\platforms\qwindows.dll %VSIX_COPY_TARGET_DIR%
)

:BINARIES_PATH_DEFINED
FOR %%C IN (Debug Release) DO FOR %%E IN (dll pdb exe) DO xcopy /r /d /y %2\%%C\bin\*%AMDT_BUILD_SUFFIX%.%%E %VSIX_COPY_TARGET_DIR%

IF "%1"=="Release" GOTO COPY_RELEASE_SPIES_WITH_PATH
:COPY_DEBUG_SPIES_WITH_PATH
FOR %%F IN (spies%AMDT_BUILD_SUFFIX% spies64%AMDT_BUILD_SUFFIX%) DO FOR %%E IN (dll pdb) DO xcopy /r /d /y %2\Debug\bin\%%F\*.%%E %VSIX_COPY_TARGET_DIR%%%F\
GOTO AFTER_SPIES
:COPY_RELEASE_SPIES_WITH_PATH
FOR %%F IN (spies%AMDT_BUILD_SUFFIX% spies64%AMDT_BUILD_SUFFIX%) DO FOR %%E IN (dll pdb) DO xcopy /r /d /y %2\Release\bin\%%F\*.%%E %VSIX_COPY_TARGET_DIR%%%F\
GOTO AFTER_SPIES
:AFTER_SPIES_WITH_PATH
GOTO COPY_COMMON


:BINARIES_PATH_NOT_DEFINED
FOR %%L IN (AMDTBaseTools AMDTOSWrappers AMDTAPIClasses AMDTApplicationComponents) DO FOR %%E IN (dll pdb) DO xcopy /r /d /y %VSIX_COPY_COMMON_FOLDER%Lib\AMD\%%L\1.0\lib\x86\*.%%E %VSIX_COPY_TARGET_DIR%
FOR %%L IN (AMDTApplicationFramework) DO FOR %%E IN (dll pdb) DO xcopy /r /d /y %VSIX_COPY_COMMON_FOLDER%Lib\AMD\%%L\2.0\lib\x86\*.%%E %VSIX_COPY_TARGET_DIR%
FOR %%L IN (AMDTSharedProfiling) DO FOR %%E IN (dll pdb) DO xcopy /r /d /y %VSIX_COPY_COMMON_FOLDER%Lib\AMD\%%L\0.5\lib\x86\*.%%E %VSIX_COPY_TARGET_DIR%
FOR %%L IN (AMDTBaseTools AMDTOSWrappers AMDTAPIClasses) DO FOR %%E IN (dll pdb) DO xcopy /r /d /y %VSIX_COPY_COMMON_FOLDER%Lib\AMD\%%L\1.0\lib\x64\*.%%E %VSIX_COPY_TARGET_DIR%
FOR %%C IN (Debug Release) DO FOR %%E IN (dll pdb exe) DO xcopy /r /d /y %VSIX_COPY_CODEXL_FOLDER%Components\Output\%%C\bin\*%AMDT_BUILD_SUFFIX%.%%E %VSIX_COPY_TARGET_DIR%


IF "%1"=="Release" GOTO COPY_RELEASE_SPIES
:COPY_DEBUG_SPIES
FOR %%F IN (spies%AMDT_BUILD_SUFFIX% spies64%AMDT_BUILD_SUFFIX%) DO FOR %%E IN (dll pdb) DO xcopy /r /d /y %VSIX_COPY_CODEXL_FOLDER%Components\Output\Debug\bin\%%F\*.%%E %VSIX_COPY_TARGET_DIR%%%F
GOTO AFTER_SPIES
:COPY_RELEASE_SPIES
FOR %%F IN (spies%AMDT_BUILD_SUFFIX% spies64%AMDT_BUILD_SUFFIX%) DO FOR %%E IN (dll pdb) DO xcopy /r /d /y %VSIX_COPY_CODEXL_FOLDER%Components\Output\Release\bin\%%F\*.%%E %VSIX_COPY_TARGET_DIR%%%F
GOTO AFTER_SPIES
:AFTER_SPIES
GOTO COPY_COMMON


:COPY_COMMON
ECHO copying perf sturio stuff
FOR %%E IN (dll pdb) DO xcopy /r /d /y %2\%1\bin\Plugins\*.%%E %VSIX_COPY_TARGET_DIR%Plugins\
FOR %%F IN (x86 x64) DO FOR %%E IN (dll pdb) DO xcopy /r /d /y %2\%1\bin\Plugins\%%F\*.%%E %VSIX_COPY_TARGET_DIR%Plugins\%%F\

rem copy common libs:
FOR %%L IN (HwDbgFacilities) DO FOR %%E IN (dll pdb) DO xcopy /r /d /y %VSIX_COPY_COMMON_FOLDER%Lib\AMD\HWDebugger\%%L\0.2\lib\x86\*.%%E %VSIX_COPY_TARGET_DIR%
FOR %%L IN (HwDbg) DO FOR %%E IN (dll pdb) DO xcopy /r /d /y %VSIX_COPY_COMMON_FOLDER%Lib\AMD\HWDebugger\%%L\0.19\lib\x86\*HSA*.%%E %VSIX_COPY_TARGET_DIR%
FOR %%L IN (OpenCLDebugAPI) DO FOR %%E IN (dll pdb) DO xcopy /r /d /y %VSIX_COPY_COMMON_FOLDER%Lib\AMD\%%L\1.3\lib\x86\*.%%E %VSIX_COPY_TARGET_DIR%

rem copy common libs that have 64-bit versions:
FOR %%L IN (HwDbgFacilities) DO FOR %%E IN (dll pdb) DO xcopy /r /d /y %VSIX_COPY_COMMON_FOLDER%Lib\AMD\HWDebugger\%%L\0.2\lib\x64\*.%%E %VSIX_COPY_TARGET_DIR%
FOR %%L IN (HwDbg) DO FOR %%E IN (dll pdb) DO xcopy /r /d /y %VSIX_COPY_COMMON_FOLDER%Lib\AMD\HWDebugger\%%L\0.19\lib\x64\*HSA*.%%E %VSIX_COPY_TARGET_DIR%
FOR %%L IN (OpenCLDebugAPI) DO FOR %%E IN (dll pdb) DO xcopy /r /d /y %VSIX_COPY_COMMON_FOLDER%Lib\AMD\%%L\1.3\lib\x64\*.%%E %VSIX_COPY_TARGET_DIR%

rem copy data needed for cpu profiling
xcopy /r /d /y %VSIX_COPY_CODEXL_FOLDER%Components\CpuProfiling\Data\Profiles\* %VSIX_COPY_TARGET_DIR%Data\Profiles\
xcopy /r /d /y %VSIX_COPY_CODEXL_FOLDER%Components\CpuProfiling\Data\Events\Public\* %VSIX_COPY_TARGET_DIR%Data\Events\
xcopy /r /d /y %VSIX_COPY_CODEXL_FOLDER%Components\CpuProfiling\Data\Views\* %VSIX_COPY_TARGET_DIR%Data\Views\

rem copy CodeXL version XML:
xcopy /r /d /y %VSIX_COPY_CODEXL_FOLDER%Data\Public\VersionSettings.xml %VSIX_COPY_TARGET_DIR%Data\

rem copy GPA Counters DLL needed by GPU Profiler
xcopy /r /d /y %VSIX_COPY_COMMON_FOLDER%Lib\AMD\GPUPerfAPI\2_21\Bin\x86\GPUPerfAPICounters.dll %VSIX_COPY_TARGET_DIR%x86\
xcopy /r /d /y %VSIX_COPY_COMMON_FOLDER%Lib\AMD\GPUPerfAPI\2_21\Bin\x64\GPUPerfAPICounters-x64.dll %VSIX_COPY_TARGET_DIR%x64\

rem copy debugger libs:

rem copy images
rem xcopy /r /d /y %VSIX_COPY_CODEXL_FOLDER%Components\GpuDebugging\AMDTGpuDebuggingComponents\Include\res\*.png %VSIX_COPY_TARGET_DIR%images
rem xcopy /r /d /y %VSIX_COPY_CODEXL_FOLDER%Components\GpuDebugging\AMDTGpuDebuggingComponents\Include\res\icons\*.png %VSIX_COPY_TARGET_DIR%images
xcopy /r /d /y %VSIX_COPY_CODEXL_FOLDER%Images\*.png %VSIX_COPY_TARGET_DIR%images
xcopy /r /d /y %VSIX_COPY_COMMONPROJECTS_FOLDER%AMDTApplication\res\*.ico %VSIX_COPY_TARGET_DIR%

rem copy EULA
xcopy /r /d /y %VSIX_COPY_CODEXL_FOLDER%Setup\Legal\Public\CodeXLEndUserLicenseAgreement-Win.rtf %VSIX_COPY_TARGET_DIR%

rem copy Qt libs:
FOR %%L IN (Core Gui Multimedia MultimediaWidgets Network OpenGL Positioning PrintSupport Qml Quick Sensors Sql WebChannel WebKit WebKitWidgets Widgets Xml) DO FOR %%D IN (Qt5%%L Qt5%%Ld) DO FOR %%E IN (dll pdb) DO xcopy /r /d /y %VSIX_COPY_COMMON_FOLDER%Lib\Ext\QT\5.5\bin\win32\%%D.%%E %VSIX_COPY_TARGET_DIR%
FOR %%L IN (icudt54 icuin54 icuuc54) DO xcopy /r /d /y %VSIX_COPY_COMMON_FOLDER%Lib\Ext\QT\5.5\bin\win32\%%L.dll %VSIX_COPY_TARGET_DIR%
FOR %%L IN (libEGL libGLESv2) DO FOR %%D IN (%%L %%Ld) DO FOR %%E IN (dll pdb) DO xcopy /r /d /y %VSIX_COPY_COMMON_FOLDER%Lib\Ext\QT\5.5\bin\win32\%%D.%%E %VSIX_COPY_TARGET_DIR%

rem copy QScintilla libs:
FOR %%C IN (Release Debug) DO xcopy /r /d /y %VSIX_COPY_COMMON_FOLDER%Lib\Ext\QScintilla\2.8-GPL\lib\win32\%%C\*.dll %VSIX_COPY_TARGET_DIR%

rem copy QCustomplot libs:
FOR %%C IN (Release Debug) DO FOR %%E IN (dll pdb) DO xcopy /r /d /y %VSIX_COPY_COMMON_FOLDER%Lib\Ext\qcustomplot\1.3.1\lib\Win32\%%C\*.%%E %VSIX_COPY_TARGET_DIR%

rem copy other libs:
xcopy /r /d /y %VSIX_COPY_COMMON_FOLDER%Lib\Ext\MicrooftDebuggingToolsForWindows\6.12.2.633\x86\bin\dbghelp.dll %VSIX_COPY_TARGET_DIR%

rem copy glew:
xcopy /r /d /y %VSIX_COPY_COMMON_FOLDER%Lib\Ext\glew\1.7.0\bin\x86\glew32.dll %VSIX_COPY_TARGET_DIR%

PAUSE
