#
# Script to replace AutomaticTester xml file GoldMaster directory path with actual path
#
#  This script replaces the path to the directory GoldMasters_APP_SDK_2_8 so that the
#  new path is <new-path>\GoldMasters_APP_SDK_2_8
#
import sys
from glob import glob
from optparse import OptionParser
import os


# Update xmlFile with actual path to test kit install directory
def SetGoldMasterPath(xmlFile, newPath):
    xmlData = xmlFile.readlines()
    numLines = len(xmlData)
    found = False
    newLine = "     <TestResultsDirectory>" + newPath + "</TestResultsDirectory>\n"

    # find all lines with "TestResultsDirectory" and update path
    for i in range(0, numLines, 1):
        index = xmlData[i].find("TestResultsDirectory");
        if(index > 0):
            found = True
            # Replace xml value string with actual path
            xmlData[i] = newLine

    if(found == False):
        # Not a CAIT xml file?
        print "Error: could not locate TestResultsDirectory in the file ", xmlFile.name()
        return None

    return xmlData


# Update xmlFile with desired log file directory path
def SetLogPath(xmlStrings, logPath):
    if(xmlStrings == None):
        return(None)

    newLine = "     <LogFilesDirectory>" + logPath + "\</LogFilesDirectory>\n"

    # find all lines with "LogFilesDirectory" and update path
    for i in range(0, len(xmlStrings), 1):
        index = xmlStrings[i].find("LogFilesDirectory")
        if(index > -1):
            xmlStrings[i] = newLine

    return xmlStrings

# Update xmlFile with desired binary file path
# Usage:
#   Expect binPath to be a folder in AMDAPPSDKSAMPLESROOT\samples\opencl\bin
#   between bin\ and x86, add binPath?
def SetBinPath(xmlStrings, binPath):
    if(xmlStrings == None):
        return(None)

    newBinDir = "bin\\" + binPath + "\\"

    # find all lines with "ExecutablePath" and "WorkingDirectory"
    # replace "bin\x86" with binPath
    for i in range(0, len(xmlStrings), 1):
        index = xmlStrings[i].find("ExecutablePath")
        if(index > -1):
            # replace with new path to binary
            tmpLine = "        <ExecutablePath>" + binPath + "</ExecutablePath>\n"
            xmlStrings[i] = tmpLine
        index = xmlStrings[i].find("WorkingDirectory")
        if(index > -1):
            # insert extra folder(s) in path to working directory
            
            tmpLine = "        <WorkingDirectory>" + os.path.dirname(binPath) + "</WorkingDirectory>"
            xmlStrings[i] = tmpLine

    return xmlStrings

# main
parser = OptionParser()
parser.add_option("--GoldPath", action="store",
                  type="string", dest="goldPath",
                  help="Optional: Specify which set of Gold Master files to use, for example GoldMasters_APPSDK_2_8 or HSAGoldMasterSDK2_8")
parser.add_option("--LogPath", action="store",
                  type="string", dest="logPath",
                  help="Optional: Path to store log files in")
parser.add_option("--xmlFiles", action="store",
                  type="string", dest="xmlFiles",
                  help="Required: xml file[s] to update")
# parser.add_option("--BinaryPath", action="store",
#                   type="string", dest="binPath",
#                  help="Optional: Alternate path, relative to AMDAPPSDKSAMPLESROOT, for the SDK Sample binary")

(options, args) = parser.parse_args(sys.argv)

if(options.xmlFiles == None):
    print "Error: You must specify at least one xml file to update"
    sys.stdout.flush()
    exit(1)
filePaths = glob(options.xmlFiles)

newGoldPath = None
if not (options.goldPath == None):
    newGoldPath = options.goldPath

newLogPath = None
if not (options.logPath == None):
    newLogPath = options.logPath

scriptStatus = 0
msgFileNotUpdating = "Error: File not updated: "

for filePath in (filePaths):
    # open xml file for read only
    xmlFile = open(filePath, 'r')
    if(xmlFile == None):
        print "Error: could not open file ", filePath
        scriptStatus = 1
    else:
        # Update xml file with new paths
        if not (newGoldPath == None):
            # Update Gold Master File folder name
            newData = SetGoldMasterPath(xmlFile, newGoldPath)
            xmlFile.close()

        if not (newLogPath == None):
            # Update log file directory path
            newData = SetLogPath(newData, newLogPath)

#        if not (options.binPath == None):
            # Update executable and working directory paths
#            newData = SetBinPath(newData, options.binPath)

        # Update file contents
        xmlFile = open(filePath, 'w')
        xmlFile.writelines(newData)
        xmlFile.flush()
        xmlFile.close()

exit(scriptStatus)
