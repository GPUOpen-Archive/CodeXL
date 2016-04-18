import os
import sys
workspace = os.path.dirname(os.path.realpath(__file__))
tempfolder = workspace+"\\TestTemp"
os.environ ["TEMP"] = tempfolder
currenttemp =  os.environ.get ("TEMP")
print ("Temp folder set to - "+currenttemp)
print ("")

# Run CodeXL Debugger test
os.chdir(workspace )
os.system( 'CodeXLGpuDebuggingTest.exe CodeXL_Tests.xml --gtest_output=xml:CodeXL_GPU_Debugging_Test_Results.xml"' )

# Run CodeXL GPU Profiling test
gpu_profiling_folder = workspace+"\\GPU_Profiling"
# Enable runing from jenkins for GPU Debugger only tests
if os.path.exists (gpu_profiling_folder):
    os.chdir (gpu_profiling_folder+"\\x86")
    os.system( '"TestCLGetKernelInfoAMD.exe --gtest_output=xml:..\..\CodeXL_GPU_Profiling_GetKernelInfo_Test_Results.xml"' )
    os.system( '"TestCLPMCAPI.exe --gtest_output=xml:..\..\CodeXL_GPU_Profiling_PMCAPI_Test_Results.xml"' )
    os.chdir (gpu_profiling_folder+"\\x86_64")
    os.system( '"TestCLGetKernelInfoAMD-x64.exe --gtest_output=xml:..\..\CodeXL_GPU_Profiling_GetKernelInfo64_Test_Results.xml"' )
    os.system( '"TestCLPMCAPI-x64.exe --gtest_output=xml:..\..\CodeXL_GPU_Profiling_PMCAPI64_Test_Results.xml"' )
else:
    print "GPU Profiling folder was not found"
