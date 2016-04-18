REM Run CodeXLGPUDebugTest.exe
REM If no configuration is specified, only the default test, CodeXLGPUDebuggingTest.xml, will be run
REM Usage:
REM    set WORKSPACE=<workspace-root-path>
REM 
REM Assumes AMD_CodeXL_Win_<version>.exe and AutomaticTester-<version>.zip
REM have already been copied into <workspace-root-path>

REM TODO replace hard coded path with cli arguments and default path values.

setlocal EnableDelayedExpansion

cd %WORKSPACE%

REM unzip test kit
for /F %%f in ('dir /B AutomaticTester-*.zip') do %WORKSPACE%\main\Common\DK\Gnu\cw1.3.12\bin\unzip -q -o -d %WORKSPACE% %%f

REM edit files with correct directory paths
cd AutomaticTester
mkdir TestResults
mkdir TestTemp
set TEMP=%WORKSPACE%\AutomaticTester\TestTemp
ReplaceGoldMasterPath.py --xmlFiles=*Test.xml --GoldPath=GoldMasters_APPSDK_2_9 --LogPath=TestResults

REM Install CodeXL
cd %WORKSPACE%
for /F %%f in ('dir /B AMD_CodeXL_Win_*.exe') do %%f /i /passive /lva %WORKSPACE%\%%f_install.log

REM Run CodeXL Debugging tests

REM 32bit Release builds
cd AutomaticTester


REM Run tests not run during continuous integration testing

REM Step Tests
REM for %%f in (MonteCarloAsianDP, URNG, KmeansAutoclustering, SimpleGL, NBody) do (
for %%f in (ImageBandwidth,SoAversusAoS,TransferOverlap,TransferOverlapCPP) do (
	for %%m in (,X64) do (
		for %%t in (StepTest,StepIntoTest,LocalsTest,WorkItemsTest) do (
			CodeXLGPUDebuggingTest.exe %%f%%m_%%t.xml --gtest_output=xml:%%f%%mRelease_%%t_Results.xml
			if not !ERRORLEVEL!==0 set SCRIPTSTATUS=!ERROROLEVEL!
		)
	)
)

REM WorkItems Tests
REM for %%f in ('dir /B *_WorkItemsTest.xml') do (
REM    CodeXLGPUDebuggingTest.exe %%f --gtest_output=xml:%%f_Rel_Results.xml
REM if not !ERRORLEVEL!==0 set SCRIPTSTATUS=!ERRORLEVEL!
REM )

REM Run continuous integration tests 10 times in a row for stress testing
for %%f in (BitonicSort, SobelFilter, MatrixMultiplication, BinarySearch, MonteCarloAsianDP, BlackScholes) do (
    for /l %%i in (1,1,10) do (
            CodeXLGPUDebuggingTest.exe %%f_StepTest.xml --gtest_output=xml:%%f_StepTest_RelLoop%%i_Results.xml
            if not !ERRORLEVEL!==0 set SCRIPTSTATUS=!ERROROLEVEL!
    )
)

exit /b !SCRIPTSTATUS!
