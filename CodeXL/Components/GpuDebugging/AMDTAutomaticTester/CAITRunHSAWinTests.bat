REM Run CodeXLGPUDebugTest.exe
REM 
REM Usage:
REM    set WORKSPACE=<workspace-root-path>
REM    CAITRunHSAWinTests.bat
REM 
REM Assumes AMD_CodeXL_Win_<version>.exe and AutomaticTester-<version>.zip
REM have already been copied into <workspace-root-path>

setlocal EnableDelayedExpansion

cd %WORKSPACE%

REM unzip test kit
for /F %%f in ('dir /B AutomaticTester-*.zip') do %WORKSPACE%\main\Common\DK\Gnu\cw1.3.12\bin\unzip -q -o -d %WORKSPACE% %%f

REM edit files with correct directory paths
REM
cd AutomaticTester
mkdir TestResults
mkdir TestTemp
set TEMP=%WORKSPACE%\AutomaticTester\TestTemp
ReplaceGoldMasterPath.py --xmlFiles=*_StepTest.xml --GoldPath=HSAGoldMasters_APPSDK_2_9 --LogPath=TestResults

REM Install CodeXL
REM
cd %WORKSPACE%
for /F %%f in ('dir /B AMD_CodeXL_Win_*.exe') do %%f /i /passive /lva %WORKSPACE%\%%f_install.log

REM Run CodeXL Debugging tests

REM 32bit Release builds
REM   Only running passing tests.  Commented out tests fail
REM
cd AutomaticTester

REM FB10253 CodeXLGpuDebuggingTest BitonicSort_StepTest.xml --gtest_output="xml:BitonicSort_StepTest_Results.xml"
REM FB10517 CodeXLGpuDebuggingTest SobelFilter_StepTest.xml --gtest_output="xml:SobelFilter_StepTest_Results.xml"
CodeXLGpuDebuggingTest BinarySearch_StepTest.xml --gtest_output="xml:BinarySearch_StepTest_Results.xml"
REM FB10445 CodeXLGpuDebuggingTest BlackScholes_StepTest.xml --gtest_output="xml:BlackScholes_StepTest_Results.xml"
CodeXLGpuDebuggingTest URNG_StepTest.xml --gtest_output="xml:URNG_StepTest_Results.xml"
REM FB10535 CodeXLGpuDebuggingTest MatrixTranspose_StepTest.xml --gtest_output="xml:MatrixTranspose_StepTest_Results.xml"
REM FB10525 CodeXLGpuDebuggingTest MonteCarloAsian_StepTest.xml --gtest_output="xml:MonteCarloAsian_StepTest_Results.xml"
CodeXLGpuDebuggingTest MatrixMultiplication_StepTest.xml --gtest_output="xml:MatrixMultiplication_StepTest_Results.xml"
REM FB10533 CodeXLGpuDebuggingTest NBody_StepTest.xml --gtest_output="xml:NBody_StepTest_Results.xml"
REM FB10517 CodeXLGpuDebuggingTest BlackScholesDP_StepTest.xml --gtest_output="xml:BlackScholesDP_StepTest_Results.xml"
REM FB10452,10482 CodeXLGpuDebuggingTest MonteCarloAsianDP_StepTest.xml --gtest_output="xml:MonteCarloAsianDP_StepTest_Results.xml"
REM FB10483 CodeXLGpuDebuggingTest SimpleGL_StepTest.xml --gtest_output="xml:SimpleGL_StepTest_Results.xml"
REM App doesn't run yet CodeXLGpuDebuggingTest AtomicCounters_StepTest.xml --gtest_output="xml:AtomicCounters_StepTest_Results.xml"
REM FB10484,10485 CodeXLGpuDebuggingTest KmeansAutoclustering_StepTest.xml --gtest_output="xml:KmeansAutoclustering_StepTest_Results.xml"
REM App doesn't run yet CodeXLGpuDebuggingTest StringSearch_StepTest.xml --gtest_output="xml:StringSearch_StepTest_Results.xml"


exit /b %ERRORLEVEL%