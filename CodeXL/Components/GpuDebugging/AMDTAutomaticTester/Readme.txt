CodeXL Automatic Integration Test (CAIT) - User Guide
Version 2.0 / 2016

Supported platform 
	Windows 7/8.1/10 32bit & 64bit

Prerequisites 
	- x86 & x86_64 Visual C++ 2015 redistributable runtime libraries (https://www.microsoft.com/en-us/download/details.aspx?id=48145) .
	- AMD GPU driver set.
	- Python 2.7 (if running with method 1).
	- If OpenCL driver is not installed in system folder need to set environment variable SU_SYSTEM_MODULES_DIRS with OpenCL driver folder.

Installation
	- Extract the CAIT.ZIP folder into a local folder.

Running 

X86 Tests
	Method 1:
		Run Python script-
		1. Go to \CAIT\X86 folder
		2. Run the CAIT.py script
		
	Method 2:
		Run tests manually (or combine with other tests scripts)- 
		1. Go to \CAIT\X86 folder.
		2. Set the TEMP environment variable (%TEMP%) to \CAIT\X86\TestTemp folder. 
		3. Set the SU_SYSTEM_MODULES_DIRS environment variable for openCL driver folder if needed. 
		4. Execute the three test sets from command line
			a. CodeXLGpuDebuggingTest.exe CodeXL_GPU_Debugging_Tests.xml --gtest_output=xml:CodeXL_Results_GPU_Debugging.xml
			b. CodeXL_GpuProfiler_TestCLGetKernelInfoAMD.exe --gtest_output=xml:CodeXL_Results_GPU_Profiling_GetKernelInfo.xml
			c. CodeXL_GpuProfiler_TestCLPMCAPI.exe --gtest_output=xml:CodeXL_Results_GPU_Profiling_PMCAPI.xml
			
X86_64 Tests

	Method 1:
		Run Python script-
		1. Go to \CAIT\X86_64 folder
		2. Run the CAIT.py script
		
	Method 2:
		Run tests manually (or combine with other tests scripts)- 
		1. Go to \CAIT\X86_64 folder.
		2. Set the TEMP environment variable (%TEMP%) to \CAIT\X86_64\TestTemp folder. 
		3. Set the SU_SYSTEM_MODULES_DIRS environment variable for openCL driver folder if needed. 
		4. Execute the three test sets from command line
			a. CodeXLGpuDebuggingTest.exe CodeXL_GPU_Debugging_Tests_x64.xml --gtest_output=xml:CodeXL_Results_GPU_Debugging_Test_x64.xml
			b. CodeXL_GpuProfiler_TestCLGetKernelInfoAMD-x64.exe --gtest_output=xml:CodeXL_Results_GPU_Profiling_GetKernelInfo-x64.xml
			c. CodeXL_GpuProfiler_TestCLPMCAPI-x64.exe --gtest_output=xml:CodeXL_Results_GPU_Profiling_PMCAPI-x64.xml


Results 
	- The results are collected into a XML files with JUnit format. 
	- Tests results can be collected from the \CAIT\X86 or/and \CAIT\X86_64 folder as "CodeXL_Results*.xml". 
	
Additional information 
	- GPU Debugging intermediate tests logs can be found at %TEMP%\AMDAutomaticTestResultsTemp\*.log
