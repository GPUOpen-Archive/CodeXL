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
    "https://github.com/GPUOpen-Tools/RGA/releases/download/1.2.0.0/rga-windows-x86-1.2.zip" : "../../Common/Lib/AMD/RGA/x86",
    "https://github.com/GPUOpen-Tools/RGA/releases/download/1.2.0.0/rga-windows-x64-1.2.zip" : "../../Common/Lib/AMD/RGA/x64",
    "https://github.com/GPUOpen-Tools/RCP/releases/download/v5.3/RadeonComputeProfiler-v5.3.6710.zip" : "../../Common/Lib/AMD/RCP",
    "https://github.com/GPUOpen-Tools/RCP/releases/download/v5.3/RCPProfileDataParser-v5.3.6710.zip" : "../../Common/Lib/AMD/RCP",
    "https://github.com/GPUOpen-Tools/GPA/releases/download/v2.23.1/GPUPerfAPI-2.23.2392.1.zip" : "../../Common/Lib/AMD/GPUPerfAPI"
}
downloadMappingLin = {
    "https://github.com/GPUOpen-Tools/RGA/releases/download/1.2.0.0/rga-linux-1.2.tgz" : "../../Common/Lib/AMD/RGA",
    "https://github.com/GPUOpen-Tools/RCP/releases/download/v5.3/RadeonComputeProfiler-v5.3.6657.tgz" : "../../Common/Lib/AMD/RCP",
    "https://github.com/GPUOpen-Tools/RCP/releases/download/v5.3/RCPProfileDataParser-v5.3.6657.tgz" : "../../Common/Lib/AMD/RCP",
    "https://github.com/GPUOpen-Tools/GPA/releases/download/v2.23.1/GPUPerfAPI.2.23.1986-lnx.tgz" : "../../Common/Lib/AMD/GPUPerfAPI"
}

# to allow the script to be run from anywhere - not just the cwd - store the absolute path to the script file
scriptRoot = os.path.dirname(os.path.realpath(__file__))

# detect the OS
MACHINE_OS = ""
if "windows" in platform.system().lower():
    MACHINE_OS = "Windows"
elif "cygwin" in platform.system().lower():
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
