@echo off

set CONFIG_NAME=%1

XCopy /s /i "..\..\..\..\Common\Lib\AMD\RCP\RadeonComputeProfiler\bin\*.*" "..\..\..\Output\%CONFIG_NAME%\bin\" /y
XCopy /s /i "..\..\..\..\Common\Lib\AMD\RCP\RadeonComputeProfiler\jqPlot\*.*" "..\..\..\Output\%CONFIG_NAME%\bin\jqPlot\" /y
XCopy /s /i "..\..\..\..\Common\Lib\AMD\RCP\RCPProfileDataParser\bin\*.*" "..\..\..\Output\%CONFIG_NAME%\bin\" /y
