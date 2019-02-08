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
    "https://github.com/GPUOpen-Tools/RGA/releases/download/2.0.1/rga-windows-x86-2.0.1-cli-only.zip" : "../../Common/Lib/AMD/RGA/x86",
    "https://github.com/GPUOpen-Tools/RGA/releases/download/2.0.1/rga-windows-x64-2.0.1.zip" : "../../Common/Lib/AMD/RGA/x64",
    "https://github.com/GPUOpen-Tools/RCP/releases/download/v5.6/RadeonComputeProfiler-v5.6.7246.zip" : "../../Common/Lib/AMD/RCP",
    "https://github.com/GPUOpen-Tools/RCP/releases/download/v5.6/RCPProfileDataParser-v5.6.7246.zip" : "../../Common/Lib/AMD/RCP",
    "https://github.com/GPUOpen-Tools/GPA/releases/download/v3.3/GPUPerfAPI-3.3.799.zip" : "../../Common/Lib/AMD/GPUPerfAPI"
}
downloadMappingLin = {
    "https://github.com/GPUOpen-Tools/RGA/releases/download/2.0.1/rga-linux-2.0.1.tgz" : "../../Common/Lib/AMD/RGA",
    "https://github.com/GPUOpen-Tools/RCP/releases/download/v5.6/RadeonComputeProfiler-v5.6.7254.tgz" : "../../Common/Lib/AMD/RCP",
    "https://github.com/GPUOpen-Tools/RCP/releases/download/v5.6/RCPProfileDataParser-v5.6.7254.tgz" : "../../Common/Lib/AMD/RCP",
    "https://github.com/GPUOpen-Tools/GPA/releases/download/v3.3/GPUPerfAPI-3.3.1078.tgz" : "../../Common/Lib/AMD/GPUPerfAPI"
}

githubMappingLin = {
    "https://github.com/GPUOpen-Tools/RCP" : ["v5.6", "../../Common/Lib/AMD/RCP/repo/RCP", "Scripts", "UpdateCommon.py"]
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

def clonerepo(repoUrl, branch, location, scriptlocation, script):
    # convert targetPath to OS specific format
    tmppath = os.path.join(scriptRoot, "", location)
    # clean up path, collapsing any ../ and converting / to \ for Windows
    targetPath = os.path.normpath(tmppath)
    if False == os.path.isdir(targetPath):
        # clone
        commandArgs = ["git", "clone", "--branch", branch, repoUrl, targetPath]
        p = subprocess.Popen(commandArgs)
        p.wait()
        scriptloc = os.path.join(tmppath, scriptlocation)
        scriptpath = os.path.join(scriptloc, script)
        p = subprocess.Popen(["python", scriptpath], cwd=scriptloc)
        p.wait()
    else:
        # pull
        p = subprocess.Popen(["git","pull"], cwd=targetPath)
        p.wait()
    sys.stdout.flush()
    sys.stderr.flush()

for key in downloadMapping:
    downloadandunzip(key, downloadMapping[key])

if MACHINE_OS == "Linux":
   for key in githubMappingLin:
       clonerepo(key, githubMappingLin[key][0], githubMappingLin[key][1], githubMappingLin[key][2], githubMappingLin[key][3])
