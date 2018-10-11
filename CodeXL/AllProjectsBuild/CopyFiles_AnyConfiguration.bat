@echo off

set CONFIG_NAME=%1
set D_SUFFIX=%2
set HYPHEN_D_SUFFIX=%3

rem Create the Output Folders:
if not exist ..\Output\%CONFIG_NAME% mkdir ..\Output\%CONFIG_NAME%
if not exist ..\Output\%CONFIG_NAME%\bin mkdir ..\Output\%CONFIG_NAME%\bin
if not exist ..\Output\%CONFIG_NAME%\bin\x64 mkdir ..\Output\%CONFIG_NAME%\bin\x64
if not exist ..\Output\%CONFIG_NAME%\bin\x86 mkdir ..\Output\%CONFIG_NAME%\bin\x86
if not exist ..\Output\%CONFIG_NAME%\bin\Legal mkdir ..\Output\%CONFIG_NAME%\bin\Legal
if not exist ..\Output\%CONFIG_NAME%\bin\platforms mkdir ..\Output\%CONFIG_NAME%\bin\platforms
if not exist ..\Output\%CONFIG_NAME%\bin\HTML mkdir ..\Output\%CONFIG_NAME%\bin\HTML
if not exist ..\Output\%CONFIG_NAME%\bin\jqPlot mkdir ..\Output\%CONFIG_NAME%\bin\jqPlot
if not exist ..\Output\%CONFIG_NAME%\bin\Images mkdir ..\Output\%CONFIG_NAME%\bin\Images

rem Copy welcome page resources
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\welcome.css" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\low_resolution.css" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\Welcome.html" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\WelcomeAnalyzeNA.html" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\welcomepage_debug.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\welcomepage_debug_large.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\welcomepage_debug_selected.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\welcomepage_profile.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\welcomepage_profile_large.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\welcomepage_profile_selected.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\welcomepage_frame_analyze.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\welcomepage_frame_analyze_large.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\welcomepage_frame_analyze_selected.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\welcomepage_analyze.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\welcomepage_analyze_large.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\welcomepage_analyze_selected.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\analyze_add.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\analyze_new.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\frame_analyze_new.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\arrow.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\debug_new.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\empty.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\profile_attach.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\profile_new.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\profile_system.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\recent_projects.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\samples.png" "..\Output\%CONFIG_NAME%\bin\HTML\"
XCopy /r /d /y "..\AMDTApplicationFramework\Include\res\welcome\StartupPage_CodeXLLogo.png" "..\Output\%CONFIG_NAME%\bin\HTML\"

rem Copy Legal Files:
XCopy /r /d /y "..\Setup\Legal\Boost_Software_License.txt" "..\Output\%CONFIG_NAME%\bin\Legal\"
XCopy /r /d /y "..\Setup\Legal\GNU_LESSER_GENERAL_PUBLIC_LICENSE2_1.pdf" "..\Output\%CONFIG_NAME%\bin\Legal\"
XCopy /r /d /y "..\Setup\Legal\jqPlot.txt" "..\Output\%CONFIG_NAME%\bin\Legal\"
XCopy /r /d /y "..\Setup\Legal\LibDwarf.txt" "..\Output\%CONFIG_NAME%\bin\Legal\"
XCopy /r /d /y "..\Setup\Legal\QScintilla_COMMERCIAL_LICENSE_AGREEMENT.txt" "..\Output\%CONFIG_NAME%\bin\Legal\"
XCopy /r /d /y "..\Setup\Legal\zlibdoc.txt" "..\Output\%CONFIG_NAME%\bin\Legal\"

rem Copy Setup Files:
XCopy /r /d /y "..\Setup\Legal\Public\CodeXLEndUserLicenseAgreement-Win.rtf" "..\Output\%CONFIG_NAME%\bin\Legal\"
XCopy /r /d /y "..\Data\Public\VersionSettings.xml" "..\Output\%CONFIG_NAME%\bin\Data\"
XCopy /r /d /y "..\Setup\CodeXL_Release_Notes.pdf" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\Setup\Legal\Readme.txt" "..\Output\%CONFIG_NAME%\bin\"

rem Copy Debugger backend files:
XCopy /r /d /y "..\..\Common\Lib\AMD\OpenCLDebugAPI\1.3\lib\x64\AMDOpenCLDebug-x64%HYPHEN_D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\AMD\OpenCLDebugAPI\1.3\lib\x64\amdopencldebug-x64%HYPHEN_D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\AMD\OpenCLDebugAPI\1.3\lib\x86\AMDOpenCLDebug%HYPHEN_D_SUFFIX%.dll" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\AMD\OpenCLDebugAPI\1.3\lib\x86\amdopencldebug%HYPHEN_D_SUFFIX%.pdb" "..\Output\%CONFIG_NAME%\bin\"

rem Copy DbgHelp.dll
XCopy /r /d /y "..\..\Common\Lib\Ext\MicrooftDebuggingToolsForWindows\6.12.2.633\bin\dbghelp.dll" "..\Output\%CONFIG_NAME%\bin\"

rem Copy CodeXL Images:
XCopy /r /d /y "..\Images\CodeXLAboutLogo.png" "..\Output\%CONFIG_NAME%\bin\Images\"
XCopy /r /d /y "..\Images\CodeXLDialogSmallLogo.png" "..\Output\%CONFIG_NAME%\bin\Images\"
XCopy /r /d /y "..\Images\CodeXLSplashScreen.png" "..\Output\%CONFIG_NAME%\bin\Images\"
XCopy /r /d /y "..\Images\StartupPage_CodeXLLogo.png" "..\Output\%CONFIG_NAME%\bin\Images\"
XCopy /r /d /y "..\Images\StartupPage_Documentation.png" "..\Output\%CONFIG_NAME%\bin\Images\"
XCopy /r /d /y "..\Images\StartupPage_Recent.png" "..\Output\%CONFIG_NAME%\bin\Images\"
XCopy /r /d /y "..\Images\DefaultFontTexture.png" "..\Output\%CONFIG_NAME%\bin\Images\"
XCopy /r /d /y "..\Images\GRSourceCodeIcon.png" "..\Output\%CONFIG_NAME%\bin\Images\"
XCopy /r /d /y "..\Images\InformationIcon.png" "..\Output\%CONFIG_NAME%\bin\Images\"
XCopy /r /d /y "..\Images\InformationIconSmall.png" "..\Output\%CONFIG_NAME%\bin\Images\"
XCopy /r /d /y "..\Images\LightBulbIconSmall.png" "..\Output\%CONFIG_NAME%\bin\Images\"
XCopy /r /d /y "..\Images\ShadingProgramIcon.png" "..\Output\%CONFIG_NAME%\bin\Images\"
XCopy /r /d /y "..\Images\WarningIconOrange.png" "..\Output\%CONFIG_NAME%\bin\Images\"
XCopy /r /d /y "..\Images\WarningIconOrangeSmall.png" "..\Output\%CONFIG_NAME%\bin\Images\"
XCopy /r /d /y "..\Images\WarningIconRed.png" "..\Output\%CONFIG_NAME%\bin\Images\"
XCopy /r /d /y "..\Images\WarningIconRedSmall.png" "..\Output\%CONFIG_NAME%\bin\Images\"
XCopy /r /d /y "..\Images\WarningIconYellow.png" "..\Output\%CONFIG_NAME%\bin\Images\"
XCopy /r /d /y "..\Images\WarningIconYellowSmall.png" "..\Output\%CONFIG_NAME%\bin\Images\"
XCopy /r /d /y "..\Images\CheckForUpdates.png" "..\Output\%CONFIG_NAME%\bin\Images\"

rem Copying Teapot Sample Files:
XCopy /r /d /y /i "..\Examples\AMDTTeaPot\AMDTTeaPot.exe" "%Public%\Documents\CodeXL\Examples\Teapot\Release\"
XCopy /r /d /y /i "..\Examples\AMDTTeaPot\AMDTTeaPot.pdb" "%Public%\Documents\CodeXL\Examples\Teapot\Release\"
XCopy /r /d /y /i "..\Examples\AMDTTeaPot\AMDTSamples.props" "%Public%\Documents\CodeXL\Examples\Teapot"
XCopy /r /d /y /i "..\Examples\AMDTTeaPot\AMDTTeaPot.exe.manifest" "%Public%\Documents\CodeXL\Examples\Teapot"
XCopy /r /d /y /i "..\Examples\AMDTTeaPot\AMDTTeaPot*.sln" "%Public%\Documents\CodeXL\Examples\Teapot"
XCopy /r /d /y /i /s /h "..\Examples\AMDTTeaPot\*.suo" "%Public%\Documents\CodeXL\Examples\Teapot"
XCopy /r /d /y /i "..\Examples\AMDTTeaPot\AMDTTeapotLicense.txt" "%Public%\Documents\CodeXL\Examples\Teapot"
XCopy /r /d /y /i "..\Examples\AMDTTeaPot\AMDTTeapot\AMDTTeaPot*.*" "%Public%\Documents\CodeXL\Examples\Teapot\AMDTTeapot\"
XCopy /r /d /y /i "..\Examples\AMDTTeaPot\AMDTTeapot\resource.h" "%Public%\Documents\CodeXL\Examples\Teapot\AMDTTeapot\"
XCopy /r /d /y /i "..\Examples\AMDTTeaPot\AMDTTeapot\targetver.h" "%Public%\Documents\CodeXL\Examples\Teapot\AMDTTeapot\"
XCopy /r /d /y /i "..\Examples\AMDTTeaPot\AMDTTeapot\inc\*.*" "%Public%\Documents\CodeXL\Examples\Teapot\AMDTTeapot\inc\"
XCopy /r /d /y /i "..\Examples\AMDTTeaPot\AMDTTeapot\res\*.*" "%Public%\Documents\CodeXL\Examples\Teapot\AMDTTeapot\res\"
XCopy /r /d /y /i "..\Examples\AMDTTeaPot\AMDTTeapot\src\*.*" "%Public%\Documents\CodeXL\Examples\Teapot\AMDTTeapot\src\"
XCopy /r /d /y /i "..\Examples\AMDTTeaPot\AMDTTeapotLib\AMDTTeaPotLib*.*" "%Public%\Documents\CodeXL\Examples\Teapot\AMDTTeapotLib\"
XCopy /r /d /y /i "..\Examples\AMDTTeaPot\AMDTTeapotLib\inc\*.*" "%Public%\Documents\CodeXL\Examples\Teapot\AMDTTeapotLib\inc\"
XCopy /r /d /y /i "..\Examples\AMDTTeaPot\AMDTTeapotLib\inc\CL\*.*" "%Public%\Documents\CodeXL\Examples\Teapot\AMDTTeapotLib\inc\CL\"
XCopy /r /d /y /i "..\Examples\AMDTTeaPot\AMDTTeapotLib\inc\GL\*.*" "%Public%\Documents\CodeXL\Examples\Teapot\AMDTTeapotLib\inc\GL\"
XCopy /r /d /y /i "..\Examples\AMDTTeaPot\AMDTTeapotLib\res\*.*" "%Public%\Documents\CodeXL\Examples\Teapot\AMDTTeapotLib\res\"
XCopy /r /d /y /i "..\Examples\AMDTTeaPot\AMDTTeapotLib\src\*.*" "%Public%\Documents\CodeXL\Examples\Teapot\AMDTTeapotLib\src\"
XCopy /r /d /y /i "..\Examples\AMDTTeaPot\AMDTTeaPotLib\res\*.*" "%Public%\Documents\CodeXL\Examples\Teapot\res\"



rem Copying CodeXL Remote Agent files:
XCopy /r /d /y "..\Remote\AMDTRemoteAgent\CodeXLRemoteAgentConfig.xml" "..\Output\%CONFIG_NAME%\bin\"

rem Copy CodeXL Help files:
XCopy /r /d /y "..\Help\CodeXL_Quick_Start_Guide.pdf" "..\Output\%CONFIG_NAME%\bin\Help\"

rem Copy symsrv files:
XCopy /r /d /y "..\..\Common\Lib\Ext\MicrooftDebuggingToolsForWindows\6.12.2.633\x86\bin\symsrv.yes" "..\Output\%CONFIG_NAME%\bin\"
XCopy /r /d /y "..\..\Common\Lib\Ext\MicrooftDebuggingToolsForWindows\6.12.2.633\x86\bin\symsrv.dll" "..\Output\%CONFIG_NAME%\bin\"

rem Copy MS DIA files:
XCopy /r /d /y "%VS140COMNTOOLS%\..\IDE\msdia140.dll" "..\Output\%CONFIG_NAME%\bin\"

rem Copy Analyzer backend files:
XCopy /r /d /y "..\..\Common\Lib\Ext\Windows-Kits\10\bin\x86\d3dcompiler_47.dll" "..\Output\%CONFIG_NAME%\bin\x86"
XCopy /r /d /y "..\..\Common\Lib\Ext\Windows-Kits\10\bin\x64\d3dcompiler_47.dll" "..\Output\%CONFIG_NAME%\bin\x64"
XCopy /r /d /y "..\..\Common\Lib\AMD\Vulkan\rev_1_0_0\Release\win32\amdspv.exe" "..\Output\%CONFIG_NAME%\bin\x86"
XCopy /r /d /y "..\..\Common\Lib\AMD\Vulkan\rev_1_0_0\Release\win32\spvgen.dll" "..\Output\%CONFIG_NAME%\bin\x86"
XCopy /r /d /y "..\..\Common\Lib\AMD\Vulkan\rev_1_0_0\Release\win64\amdspv.exe" "..\Output\%CONFIG_NAME%\bin\x64"
XCopy /r /d /y "..\..\Common\Lib\AMD\Vulkan\rev_1_0_0\Release\win64\spvgen.dll" "..\Output\%CONFIG_NAME%\bin\x64"
XCopy /r /d /y "..\..\Common\Lib\AMD\OpenGL\VirtualContext\Release\win32\VirtualContext.exe" "..\Output\%CONFIG_NAME%\bin\x86"
XCopy /r /d /y "..\..\Common\Lib\AMD\OpenGL\VirtualContext\Release\win64\VirtualContext.exe" "..\Output\%CONFIG_NAME%\bin\x64"
XCopy /r /d /y "..\..\Common\Lib\AMD\ShaderAnalysis\Windows\x86\shae.exe" "..\Output\%CONFIG_NAME%\bin\x86"

rem Copy glew files:
XCopy /r /d /y "..\..\Common\Lib\Ext\glew\1.7.0\bin\x86\glew32.dll" "..\Output\%CONFIG_NAME%\bin\"

rem copy AGS file
XCopy /r /d /y "..\..\Common\Lib\AMD\ags\ags_lib_v4.0.0\lib\amd_ags_x86.dll" "..\Output\%CONFIG_NAME%\bin\"