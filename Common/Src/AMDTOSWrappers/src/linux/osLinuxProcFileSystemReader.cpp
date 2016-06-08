//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osLinuxProcFileSystemReader.cpp
///
//=====================================================================

//------------------------------ osLinuxProcFileSystemReader.cpp ------------------------------

// POSIX:
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

// Standard C:
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>
// Local:
#include <AMDTOSWrappers/Include/osLinuxProcFileSystemReader.h>


// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::osLinuxProcFileSystemReader
// Description: Constructor.
// Author:      AMD Developer Tools Team
// Date:        25/7/2007
// ---------------------------------------------------------------------------
osLinuxProcFileSystemReader::osLinuxProcFileSystemReader()
    : osSystemResourcesDataSampler(), _kernelMajorVersion(-1), _kernelMinorVersion(-1),
      _kernelRevision(-1), _is26KernelOrHigher(false), _cpusAmount(1)
{
    _readFileContentBuff[0] = 0;

    // Update the Linux kernel version:
    bool rc1 = updateKernelVersion();
    GT_ASSERT(rc1);
}


// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::~osLinuxProcFileSystemReader
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        25/7/2007
// ---------------------------------------------------------------------------
osLinuxProcFileSystemReader::~osLinuxProcFileSystemReader()
{
}


// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::getKernelVersion
// Description: Retrieves the Linux kernel version.
// Arguments: majorVersion - Major kernel version.
//            minorVersion - Minor kernel version.
//            revision - Kernel revision.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/7/2007
// ---------------------------------------------------------------------------
bool osLinuxProcFileSystemReader::getKernelVersion(int& majorVersion, int& minorVersion, int& revision)
{
    bool retVal = false;

    // If kernel data was not updated yet:
    bool isKernelVersionUpdated = (_kernelRevision != -1);

    if (!isKernelVersionUpdated)
    {
        // Update it now:
        isKernelVersionUpdated = updateKernelVersion();
    }

    // If we have an updated kernel version:
    if (isKernelVersionUpdated)
    {
        // Output it:
        majorVersion = _kernelMajorVersion;
        minorVersion = _kernelMinorVersion;
        revision = _kernelRevision;

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::updateCPUsData
// Description: Updates CPUs related data.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        25/7/2007
// ---------------------------------------------------------------------------
bool osLinuxProcFileSystemReader::updateCPUsData()
{
    bool retVal = false;

    // Read the /proc/stat file content:
    bool rc1 = readFileIntoContentBuffer("/proc/stat", _readFileContentBuff, OS_PROC_FS_FILE_CONTENT_BUF_SIZE);
    GT_IF_WITH_ASSERT(rc1)
    {
        // If the first file item is the global CPU data:
        GT_IF_WITH_ASSERT((_readFileContentBuff[0] == 'c') && (_readFileContentBuff[1] == 'p') &&
                          (_readFileContentBuff[2] == 'u') && (_readFileContentBuff[4] == ' '))
        {
            // Read the global CPU statistical data:
            const char* pCurrPos = _readFileContentBuff + 4;
            bool rc1 = readCPUStatistics(pCurrPos, _globalCPUData);

            if (rc1)
            {
                // Updating global CPU data is considered a success:
                retVal = true;

                // Find the next files line:
                pCurrPos = findNextLine(pCurrPos);

                if (pCurrPos != 0)
                {
                    // Update "per CPU" statistical data:
                    updatePerCPUStatisticalData(pCurrPos);
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::cpusAmount
// Description: Retrieves the amount of host CPUs.
// Return Val: int - The amount of host CPUs. 1 if we don't have per CPU data.
// Author:      AMD Developer Tools Team
// Date:        30/7/2007
// ---------------------------------------------------------------------------
int osLinuxProcFileSystemReader::cpusAmount() const
{
    return _cpusAmount;
}

// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::getGlobalCPUData
// Description:
//   Retrieves the global CPU statistical data. This data reflects the
//   activities of all system's CPUs.
// Arguments: cpuStatistics - Will get the global CPU statistics.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/7/2007
// ---------------------------------------------------------------------------
bool osLinuxProcFileSystemReader::getGlobalCPUData(osCPUSampledData& cpuStatistics) const
{
    cpuStatistics = _globalCPUData;
    return true;
}


// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::getCPUData
// Description: Returns the statistical data of a single given CPU.
// Arguments: int cpuIndex
//            osCPUSampledData& cpuStatistics
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        30/7/2007
// ---------------------------------------------------------------------------
bool osLinuxProcFileSystemReader::getCPUData(int cpuIndex, osCPUSampledData& cpuStatistics) const
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((0 <= cpuIndex) && (cpuIndex < _cpusAmount))
    {
        cpuStatistics = _cpuData[cpuIndex];
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::getCPUtype
// Description: reads the cpuinfo proc file to get the name of a certain CPU
// Arguments: cpuIndex - the number of the CPU to be queried
//            cpuType - the CPU's name goes here
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/5/2008
// ---------------------------------------------------------------------------
bool osLinuxProcFileSystemReader::getCPUtype(int cpuIndex, gtString& cpuType)
{
    bool retVal = false;

    gtASCIIString cpuTypeASCII;
    // Read the /proc/cpuinfo file content:
    bool rc1 = readFileIntoContentBuffer("/proc/cpuinfo", _readFileContentBuff, OS_PROC_FS_FILE_CONTENT_BUF_SIZE);
    cpuTypeASCII = "CPU not found";
    GT_IF_WITH_ASSERT(rc1)
    {
        gtASCIIString readFileContentBuff = _readFileContentBuff;
        bool goOn = true;
        int currentPosition = 0;
        gtASCIIString processorIndexAsString;
        processorIndexAsString.appendFormattedString("%d", cpuIndex);
        gtASCIIString currentProcessorID;

        while (goOn)
        {
            // look for lines beginning with "processor       :"
            currentPosition = readFileContentBuff.find("processor", currentPosition) + 1;

            if (currentPosition != -1)
            {
                int nextLineStart = readFileContentBuff.findNextLine(currentPosition);
                int colonPosition = readFileContentBuff.find(":", currentPosition);

                if ((nextLineStart == 0) || (colonPosition < nextLineStart - 1))
                {
                    // see if this is the right processor:
                    readFileContentBuff.getSubString(colonPosition + 2, nextLineStart - 1, currentProcessorID);

                    if (currentProcessorID == processorIndexAsString)
                    {
                        cpuTypeASCII = "Unknown CPU type";

                        // look for the line beginning with "model name     :"
                        currentPosition = nextLineStart;
                        int nextProcessorPosition = readFileContentBuff.find("processor", currentPosition);
                        int modelNamePosition = readFileContentBuff.find("model name", currentPosition);

                        if ((nextProcessorPosition == -1) || (modelNamePosition < nextProcessorPosition))
                        {
                            currentPosition = modelNamePosition;
                            nextLineStart = readFileContentBuff.findNextLine(currentPosition);
                            int colonPosition = readFileContentBuff.find(":", currentPosition);

                            if ((nextLineStart == 0) || (colonPosition < nextLineStart - 1))
                            {
                                readFileContentBuff.getSubString(colonPosition + 2, nextLineStart - 1, cpuTypeASCII);
                                retVal = true;
                            }
                        }

                        goOn = false;
                    }
                }
            }
            else
            {
                // We didn't find our processor
                goOn = false;
            }
        }
    }

    cpuType.fromASCIIString(cpuTypeASCII.asCharArray());

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::getDomainName
// Description:
// Arguments: gtString& domainName
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        26/5/2008
// ---------------------------------------------------------------------------
bool osLinuxProcFileSystemReader::getDomainName(gtString& domainName)
{
    bool retVal = false;
    domainName.makeEmpty();
    bool rc = readFileIntoContentBuffer("/proc/sys/kernel/domainname", _readFileContentBuff, OS_PROC_FS_FILE_CONTENT_BUF_SIZE);
    GT_IF_WITH_ASSERT(rc)
    {
        domainName.fromASCIIString(_readFileContentBuff);
        retVal = true;
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::updatePhysicalMemoryData
// Description: Updates physical memory related data.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        1/8/2007
// ---------------------------------------------------------------------------
bool osLinuxProcFileSystemReader::updatePhysicalMemoryData()
{
    bool retVal = false;

    // Read the /proc/meminfo file content:
    bool rc1 = readFileIntoContentBuffer("/proc/meminfo", _readFileContentBuff, OS_PROC_FS_FILE_CONTENT_BUF_SIZE);
    GT_IF_WITH_ASSERT(rc1)
    {
        retVal = true;

        // Iterate the file lines:
        const char* pCurrPos = _readFileContentBuff;

        while (pCurrPos != 0)
        {
            if (pCurrPos != 0)
            {
                // If the current line start with "M":
                if (pCurrPos[0] == 'M')
                {
                    // If this is a total physical memory line:
                    if (strncmp(pCurrPos, "MemTotal:", 9) == 0)
                    {
                        bool rc1 = readMemoryValue(pCurrPos + 11, _physicalMemoryData._totalPhysicalMemory);
                        GT_ASSERT(rc1);
                    }
                    else if (strncmp(pCurrPos, "MemFree:", 8) == 0)
                    {
                        // This is a free physical memory line:
                        bool rc2 = readMemoryValue(pCurrPos + 10, _physicalMemoryData._freePhysicalMemory);
                        GT_ASSERT(rc2);
                    }
                    else if (strncmp(pCurrPos, "MemShared:", 10) == 0)
                    {
                        // This is the total shared memory line:
                        bool rc3 = readMemoryValue(pCurrPos + 12, _physicalMemoryData._totalSharedMemory);
                        GT_ASSERT(rc3);
                    }
                }
                else if (pCurrPos[0] == 'C')
                {
                    if (strncmp(pCurrPos, "Cached:", 7) == 0)
                    {
                        // This is a cached memory line:
                        bool rc5 = readMemoryValue(pCurrPos + 9, _physicalMemoryData._cachedMemory);
                        GT_ASSERT(rc5);
                    }
                }
                else if (pCurrPos[0] == 'B')
                {
                    if (strncmp(pCurrPos, "Buffers:", 8) == 0)
                    {
                        // This is a buffer memory line:
                        bool rc5 = readMemoryValue(pCurrPos + 10, _physicalMemoryData._bufferMemory);
                        GT_ASSERT(rc5);
                    }
                }
                else if (pCurrPos[0] == 'V')
                {
                    if (strncmp(pCurrPos, "VmallocTotal:", 13) == 0)
                    {
                        bool rc5 = readMemoryValue(pCurrPos + 15, _physicalMemoryData._totalVirtualMemory);
                        GT_ASSERT(rc5);
                    }
                    else if (strncmp(pCurrPos, "VmallocChunk:", 13) == 0)
                    {
                        // Note: this isn't exactly the amount of free virtual memory but rather the largest contiguous
                        // chunk of memory that can be used as virtual memory, which is a good estimate. The exact amount
                        // of virtual memory available can only be assessed by using it all up, because of how the Linux
                        // virtual memory allocation system works.
                        bool rc5 = readMemoryValue(pCurrPos + 15, _physicalMemoryData._freeVirtualMemory);
                        GT_ASSERT(rc5);
                    }
                }
                else if (pCurrPos[0] == 'S')
                {
                    if (strncmp(pCurrPos, "SwapTotal:", 10) == 0)
                    {
                        bool rc5 = readMemoryValue(pCurrPos + 12, _physicalMemoryData._totalSwapMemory);
                        GT_ASSERT(rc5);
                    }
                    else if (strncmp(pCurrPos, "SwapFree:", 9) == 0)
                    {
                        bool rc5 = readMemoryValue(pCurrPos + 11, _physicalMemoryData._freeSwapMemory);
                        GT_ASSERT(rc5);
                    }
                }
                else if (pCurrPos[0] == 'H')
                {
                    if (strncmp(pCurrPos, "Hugepagesize:", 13) == 0)
                    {
                        gtUInt64 tmpMemoryPageSize;
                        bool rc5 = readMemoryValue(pCurrPos + 15, tmpMemoryPageSize);
                        _physicalMemoryData._hugePageSize = tmpMemoryPageSize;
                        GT_ASSERT(rc5);
                    }
                }
            }

            // Find the next files line:
            pCurrPos = findNextLine(pCurrPos);
        }
    }

    // Calculate missing parameters:
    _physicalMemoryData._usedPhysicalMemory = _physicalMemoryData._totalPhysicalMemory - _physicalMemoryData._freePhysicalMemory;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::getPhysicalMemoryData
// Description: Returns the physical memory statistical data.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        22/11/2009
// ---------------------------------------------------------------------------
bool osLinuxProcFileSystemReader::getPhysicalMemoryData(osPhysicalMemorySampledData& memoryStatistics) const
{
    memoryStatistics = _physicalMemoryData;
    return true;
}

// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::getPhysicalMemoryData
// Description: Returns the physical memory statistical data.
// Author:      AMD Developer Tools Team
// Date:        1/8/2007
// ---------------------------------------------------------------------------
const osPhysicalMemorySampledData& osLinuxProcFileSystemReader::physicalMemoryData() const
{
    return _physicalMemoryData;
}


// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::readFileIntoContentBuffer
// Description: Reads a given file content into a given buffer.
// Arguments: fileFullPath - The path of the file to be read.
//            pBuff - The buffer into which the file content will be read.
//            buffLength - The buffer's size.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        25/7/2007
// ---------------------------------------------------------------------------
bool osLinuxProcFileSystemReader::readFileIntoContentBuffer(const char* fileFullPath, char* pBuff, int buffLength)
{
    bool retVal = false;

    // Clear the buffer content:
    pBuff[0] = 0;

    // Open the file for reading:
    int fd = ::open(fileFullPath, O_RDONLY);
    GT_IF_WITH_ASSERT(0 < fd)
    {
        // Read the file content into the buffer:
        ssize_t len = ::read(fd, pBuff, buffLength - 1);
        GT_IF_WITH_ASSERT(0 < len)
        {
            // NULL terminate the read string:
            pBuff[len] = 0;

            retVal = true;
        }

        // Close the file:
        ::close(fd);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::updateKernelVersion
// Description: Updates the Linux kernel version.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/7/2007
// ---------------------------------------------------------------------------
bool osLinuxProcFileSystemReader::updateKernelVersion()
{
    bool retVal = false;

    // Read the /proc/sys/kernel/osrelease file content:
    bool rc1 = readFileIntoContentBuffer("/proc/sys/kernel/osrelease", _readFileContentBuff, OS_PROC_FS_FILE_CONTENT_BUF_SIZE);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Read the kernel version:
        int fieldsMatched = ::sscanf(_readFileContentBuff, "%d.%d.%d", &_kernelMajorVersion, &_kernelMinorVersion, &_kernelRevision);
        GT_IF_WITH_ASSERT(fieldsMatched == 3)
        {
            // Update kernel significant versions flags:
            _is26KernelOrHigher = ((2 < _kernelMajorVersion) || ((2 == _kernelMajorVersion) && (6 <= _kernelMinorVersion)));

            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::getVersionString
// Description: inserts the system version string into the osVersion variable
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        22/5/2008
// ---------------------------------------------------------------------------
bool osLinuxProcFileSystemReader::getVersionString(gtString& osVersion)
{
    bool retVal = false;
    osVersion = L"Could not retrieve OS version";
    bool rc1 = readFileIntoContentBuffer("/proc/version", _readFileContentBuff, OS_PROC_FS_FILE_CONTENT_BUF_SIZE);
    GT_IF_WITH_ASSERT(rc1)
    {
        osVersion.fromASCIIString(_readFileContentBuff);

        // This value sometimes has a newline at its end. Remove it:
        osVersion.removeTrailing('\n').removeTrailing('\r');
        retVal = true;
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::updatePerCPUStatisticalData
// Description: Updates "per CPU" statistical data.
// Arguments: pCurrPos - Pointer to the first /proc/stat file line
//                       that contains the "per CPU" statistics.
//                       NOTICE: The pointer will be advances as data is reed.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/7/2007
// ---------------------------------------------------------------------------
bool osLinuxProcFileSystemReader::updatePerCPUStatisticalData(const char*& pCurrPos)
{
    bool retVal = false;

    // Iterate the "per CPU" statistical data lines:
    for (int i = 0; ((i < OS_SUPPORTED_CPUS_AMOUNT) && (pCurrPos != NULL)); i++)
    {
        // Verify that the first token is "cpuX":
        if ((pCurrPos[0] != 'c') || (pCurrPos[1] != 'p') || (pCurrPos[2] != 'u'))
        {
            break;
        }
        else
        {
            // Find the first statistical data:
            pCurrPos = findNextToken(pCurrPos);

            // Read CPU i statistical data:
            bool rc1 = readCPUStatistics(pCurrPos, _cpuData[i]);

            if (rc1)
            {
                // Updating one CPU's data is considered a success:
                retVal = true;

                // _cpusAmount is initialized to 1:
                if (0 < i)
                {
                    _cpusAmount++;
                }

                // Go to the next cpu line:
                pCurrPos = findNextLine(pCurrPos);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::readCPUStatistics
// Description: Reads CPU statistics from a string.
// Arguments: pCurrPos - A string containing CPU statistics.
//                       NOTICE: The string pointer will be advanced as data is read.
//            cpuStatistics - Will get the read CPU statistics.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/7/2007
// ---------------------------------------------------------------------------
bool osLinuxProcFileSystemReader::readCPUStatistics(const char*& pCurrPos, osCPUSampledData& cpuStatistics)
{
    bool retVal = false;

    // Get none const access to pCurrPos (this is needed by strtoull):
    char* pCurrPosNoneConst = (char*)pCurrPos;

    cpuStatistics._userClockTicks = strtoull(pCurrPosNoneConst, &pCurrPosNoneConst, 10);
    cpuStatistics._niceClockTicks = strtoull(pCurrPosNoneConst, &pCurrPosNoneConst, 10);
    cpuStatistics._sysClockTicks = strtoull(pCurrPosNoneConst, &pCurrPosNoneConst, 10);
    cpuStatistics._idleClockTicks = strtoull(pCurrPosNoneConst, &pCurrPosNoneConst, 10);

    // If this is kernel version 2.6 or higher:
    // (Kernel 2.6 adds 3 additional performance counters)
    if (_is26KernelOrHigher)
    {
        cpuStatistics._IOWaitClockTicks = strtoull(pCurrPosNoneConst, &pCurrPosNoneConst, 10);
        cpuStatistics._IRQClockTicks = strtoull(pCurrPosNoneConst, &pCurrPosNoneConst, 10);
        cpuStatistics._softIRQClockTicks = strtoull(pCurrPosNoneConst, &pCurrPosNoneConst, 10);
    }
    else
    {
        // Older kernels do not export there counters:
        cpuStatistics._IOWaitClockTicks = 0;
        cpuStatistics._IRQClockTicks = 0;
        cpuStatistics._softIRQClockTicks = 0;
    }

    // Update the total clock ticks counter:
    cpuStatistics._totalClockTicks = cpuStatistics._userClockTicks + cpuStatistics._niceClockTicks +
                                     cpuStatistics._sysClockTicks + cpuStatistics._idleClockTicks +
                                     cpuStatistics._IOWaitClockTicks + cpuStatistics._IRQClockTicks +
                                     cpuStatistics._softIRQClockTicks;

    // Output our current string position:
    pCurrPos = pCurrPosNoneConst;

    retVal = true;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::readMemoryValue
// Description: Inputs a string that contains memory related values and outputs
//              the value as an gtUInt64.
// Arguments: pStr - The input string. Its format includes a number and units.
//                   Example: "7728 kB".
//            value - Will get the memory related value in byte units.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        1/8/2007
// ---------------------------------------------------------------------------
bool osLinuxProcFileSystemReader::readMemoryValue(const char* pStr, gtUInt64& value) const
{
    bool retVal = false;

    // Read the memory related value:
    char* pCurrPos = (char*)pStr;
    value = ::strtoull(pCurrPos, &pCurrPos, 10);

    // If the unit is "kB"
    if ((pCurrPos[1] == 'k') && (pCurrPos[2] == 'B'))
    {
        value *= 1024;
        retVal = true;
    }
    else if ((pCurrPos[1] == 'M') && (pCurrPos[2] == 'B'))
    {
        // The unit is "MB":
        value *= 1048576;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::findNextToken
// Description:
//  Inputs a string pointer and returns a pointer to the next token within
//  the input string (to the first char that appears after the first ' ' char).
// Arguments: pCurrPos - The input string position.
// Return Val: char* - Will get the next token position pointer, or 0 if the
//                     end of the string encountered before reaching a ' ' char.
// Author:      AMD Developer Tools Team
// Date:        29/7/2007
// ---------------------------------------------------------------------------
const char* osLinuxProcFileSystemReader::findNextToken(const char* pCurrPos)
{
    const char* retVal = 0;

    // Eat all none space chars:
    while ((*pCurrPos != ' ') && (*pCurrPos != 0))
    {
        pCurrPos++;
    }

    // If we didn't reach the end of the file:
    if (pCurrPos != 0)
    {
        // Eat all spaces:
        while (*pCurrPos == ' ')
        {
            pCurrPos++;
        }

        // If we didn't reach the end of the file:
        if (*pCurrPos != 0)
        {
            retVal = pCurrPos;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osLinuxProcFileSystemReader::findNextLine
// Description:
//  Inputs a string pointer and returns a pointer to the next line within
//  the input string (to the first char that appears after the first '\n' char).
// Arguments: pCurrPos - The input string position.
// Return Val: char* - Will get the next line position pointer, or 0 if the
//                     end of the string encountered before reaching a '\n' char.
// Author:      AMD Developer Tools Team
// Date:        29/7/2007
// ---------------------------------------------------------------------------
const char* osLinuxProcFileSystemReader::findNextLine(const char* pCurrPos)
{
    const char* retVal = 0;

    // While we didn't reach the end of string:
    while (*pCurrPos != 0)
    {
        // If we reached a line delimiter:
        if (*pCurrPos == '\n')
        {
            retVal = pCurrPos + 1;
            break;
        }
        else
        {
            pCurrPos++;
        }
    }

    return retVal;
}
