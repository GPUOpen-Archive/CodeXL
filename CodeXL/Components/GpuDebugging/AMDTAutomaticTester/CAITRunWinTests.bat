REM Run CodeXLGPUDebugTest.exe
REM If no configuration file is specified, only the default test, CodeXLGPUDebuggingTest.xml, will be run
REM Usage:
REM    set WORKSPACE=<workspace-root-path>
REM    set TEMP=<temp-directory>
REM    CodeXLGpuDebuggingTest.exe [<test>.xml] [--gtest_output=xml:<gtest_details>.xml]
REM 
REM Assumes AMD_CodeXL_Win_<version>.exe and AutomaticTester-<version>.zip
REM have already been copied into <workspace-root-path>

REM TODO replace hard coded path with cli arguments and default path values.

setlocal EnableDelayedExpansion

cd "%WORKSPACE%"

REM unzip test kit
for /F %%f in ('dir /B AutomaticTester-*.zip') do "%WORKSPACE%\main\Common\DK\Gnu\cw1.3.12\bin\unzip" -q -o -d %WORKSPACE% %%f

REM edit files with correct directory paths
cd AutomaticTester
ReplaceGoldMasterPath.py --xmlFiles *Test.xml --GoldPath=GoldMasters_APPSDK_2_9 --LogPath=TestResults
mkdir TestResults
mkdir TestTemp
set TEMP="%WORKSPACE%\AutomaticTester\TestTemp"

REM Install CodeXL
cd "%WORKSPACE%"
REM for /F %%f in ('dir /B AMD_CodeXL_Win_*.exe') do %%f /i /passive /lva "%WORKSPACE%\%%f_install.log"

REM Run CodeXL Debugging tests

REM 32bit Release builds
cd AutomaticTester

REM for /F %%f in ('dir /B *Test.xml') do CodeXLGPUDebuggingTest.exe %%f --gtest_output=xml:%%f_Results.xml

REM CodeXLGPUDebuggingTest.exe --gtest_output=xml:CXLGPUDbgAcceptanceTestRelease_Results.xml
REM set SCRIPTSTATUS=!ERRORLEVEL!

for %%f in (AsyncDataTransfer,BinarySearch,BinomialOption,BinomialOptionMultiGPU,BitonicSort,BlackScholes,BlackScholesDP,BoxFilter,BoxFilterGL,BufferBandwidth,BufferImageInterop,ConcurrentKernel,ConstantBandwidth,CplusplusWrapper,DCT,DwtHaar1D,DwtHaar1DCPPKernel,EigenValue,FastWalshTransform,FFT,FloydWarshall,FluidSimulation2D,GaussianNoiseGL,GlobalMemoryBandwidth,HDRToneMapping,HelloWorld,Histogram,HistogramAtomics,ImageOverlap,IntroStaticCPPKernel,KernelLaunch,LDSBandwidth,LUDecomposition,Mandelbrot,MatrixMulImage,MatrixMultiplication,MatrixMultiplicationCPPKernel,MatrixTranspose,MemoryModel,MemoryOptimizations,MersenneTwister,MonteCarloAsian,MonteCarloAsianDP,MonteCarloAsianMultiGPU,NBody,PrefixSum,QuasiRandomSequence,RadixSort,RecursiveGaussian,Reduction,ScanLargeArrays,SimpleConvolution,SimpleDX10,SimpleDX11,SimpleDX9,SimpleGL,SimpleImage,SimpleMultiDevice,SobelFilter,SobelFilterImage,UnsharpMask,URNG,URNGNoiseGL) do (
	for %%m in (,X64) do (
		for %%t in (StepTest,StepIntoTest,LocalsTest,WorkItemsTest) do (
			CodeXLGPUDebuggingTest.exe %%f%%m_%%t.xml --gtest_output=xml:%%f%%mRelease_%%t_Results.xml
			if not !ERRORLEVEL!==0 set SCRIPTSTATUS=!ERROROLEVEL!
		)
	)
)

exit /b !SCRIPTSTATUS!