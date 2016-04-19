import os
import sys
workspace = os.path.dirname(os.path.realpath(__file__))
tempfolder = workspace+"\\TestTemp"
os.environ ["TEMP"] = tempfolder
currenttemp =  os.environ.get ("TEMP")
print ("Temp folder set to - "+currenttemp)
print ("")

os.chdir(workspace )
# Run CodeXL Debugger test
os.system( 'CodeXLGpuDebuggingTest.exe CodeXL_GPU_Debugging_Tests.xml --gtest_output=xml:CodeXL_Results_GPU_Debugging.xml"' )

# Run CodeXL GPU Profiling test
os.system( '"CodeXL_GpuProfiler_TestCLGetKernelInfoAMD.exe --gtest_output=xml:CodeXL_Results_GPU_Profiling_GetKernelInfo.xml"' )
os.system( '"CodeXL_GpuProfiler_TestCLPMCAPI.exe --gtest_output=xml:CodeXL_Results_GPU_Profiling_PMCAPI.xml"' )

