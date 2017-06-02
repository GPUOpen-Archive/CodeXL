import os
import string
import subprocess
import sys
import urllib
import zipfile
import tarfile
import platform

isPython3OrAbove = None
if sys.version_info[0] >= 3:
    isPython3OrAbove = True
	
if isPython3OrAbove:
	import urllib.request

# key = GitHub release link
# value = location
downloadMappingWin = {
    "https://github.com/GPUOpen-Tools/RGA/releases/download/1.0.0.0/rga-windows-x64-1.0.zip" : "../../Common/Lib/AMD/RGA/x64",
    "https://github.com/GPUOpen-Tools/RGA/releases/download/1.0.0.0/rga-windows-x86-1.0.zip" : "../../Common/Lib/AMD/RGA/x86",
    # Below RCP artifact links are only for internal uses and it should not be merged to GitHub master branch
    "http://bdclin64-gdt-jenkins:8080/view/GPU%20Profiling/job/RadeonComputeProfiler/lastSuccessfulBuild/artifact/RCP/Build/RadeonComputeProfiler.2017-06-02-v5.1.6423.zip" : "../../Common/Lib/AMD/RCP",
    "http://bdclin64-gdt-jenkins:8080/view/GPU%20Profiling/job/RadeonComputeProfiler/lastSuccessfulBuild/artifact/RCP/Build/RCPProfileDataParser.2017-06-02-v5.1.6423.zip" : "../../Common/Lib/AMD/RCP"
}
downloadMappingLin = {
    "https://github.com/GPUOpen-Tools/RGA/releases/download/1.0.0.0/rga-linux-1.0.tgz" : "../../Common/Lib/AMD/RGA",
    # Below RCP artifact links are only for internal uses and it should not be merged to GitHub master branch
    "http://bdclin64-gdt-jenkins:8080/view/GPU%20Profiling/job/RadeonComputeProfiler-Linux/6356/artifact/RCP/Build/Linux/RadeonComputeProfiler-v5.1.6356.tgz" : "../../Common/Lib/AMD/RCP",
    "http://bdclin64-gdt-jenkins:8080/view/GPU%20Profiling/job/RadeonComputeProfiler-Linux/6356/artifact/RCP/Build/Linux/RCPProfileDataParser-v5.1.6356.tgz" : "../../Common/Lib/AMD/RCP"
}

# to allow the script to be run from anywhere - not just the cwd - store the absolute path to the script file
scriptRoot = os.path.dirname(os.path.realpath(__file__))

# detect the OS
MACHINE_OS = ""
if "windows" in platform.system().lower():
    MACHINE_OS = "Windows"
elif "linux" in platform.system().lower():
    MACHINE_OS = "Linux"
else:
    print("Operating system not recognized correctly")
    sys.exit(1)

# reference the correct archive path
downloadMapping = ""
if MACHINE_OS == "Linux":
    downloadMapping = downloadMappingLin
else:
    downloadMapping = downloadMappingWin
	
# routine for downloading and unzipping an archive
def downloadandunzip(key, value):
    # convert targetPath to OS specific format
    tmppath = os.path.join(scriptRoot, "", value)
    # clean up path, collapsing any ../ and converting / to \ for Windows
    targetPath = os.path.normpath(tmppath)
    if False == os.path.isdir(targetPath):
        os.makedirs(targetPath)
    zipfileName = key.split('/')[-1].split('#')[0].split('?')[0]
    zipPath = os.path.join(targetPath, zipfileName)
    if False == os.path.isfile(zipPath):
        print("\nDownloading " + key + " into " + zipPath)
        if isPython3OrAbove:
            urllib.request.urlretrieve(key, zipPath)
        else:
            urllib.urlretrieve(key, zipPath)
        if os.path.splitext(zipPath)[1] == ".zip":
            zipfile.ZipFile(zipPath).extractall(targetPath)
            os.remove(zipPath)
        elif os.path.splitext(zipPath)[1] == ".tgz":
            tarfile.open(zipPath).extractall(targetPath)
            os.remove(zipPath)

for key in downloadMapping:
    downloadandunzip(key, downloadMapping[key])
