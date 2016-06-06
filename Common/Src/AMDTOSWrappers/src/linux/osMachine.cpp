//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osMachine.cpp
///
//=====================================================================

//------------------------------ osMachine.cpp ------------------------------

// POSIX:
#include <unistd.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osLinuxProcFileSystemReader.h>
#include <AMDTOSWrappers/Include/osMachine.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>

// For systems that don't define HOST_NAME_MAX:
#ifndef HOST_NAME_MAX
    #define HOST_NAME_MAX 255
#endif


// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineName
// Description: Retrieves the name of the local machine (computer)
// Arguments:   localMachineName - Will get the local machine name.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        24/1/2007
// ---------------------------------------------------------------------------
bool osGetLocalMachineName(gtASCIIString& localMachineName)
{
    bool retVal = false;

    // Get the local host name:
    char hostNameBuff[HOST_NAME_MAX];
    int rc1 = ::gethostname(hostNameBuff, HOST_NAME_MAX);
    GT_IF_WITH_ASSERT(rc1 == 0)
    {
        // Just to be on the safe side:
        hostNameBuff[HOST_NAME_MAX - 1] = 0;

        // Output the local machine name:
        localMachineName = hostNameBuff;

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineName
// Description: Retrieves the name of the local machine (computer)
// Arguments:   localMachineName - Will get the local machine name.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        13/9/2010
// ---------------------------------------------------------------------------
bool osGetLocalMachineName(gtString& localMachineName)
{
    // Get the machine name in ASCII string:
    gtASCIIString ansiiMachineName;
    bool retVal = osGetLocalMachineName(ansiiMachineName);

    // Convert to Unicode:
    localMachineName.fromASCIIString(ansiiMachineName.asCharArray());

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetAmountOfLocalMachineCPUs
// Description: Retrieves the amount of local machine CPUs.
// Arguments:   amounfOfCPUs - Will get the amount of local machine CPUs.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        15/5/2008
// ---------------------------------------------------------------------------
bool osGetAmountOfLocalMachineCPUs(int& amountOfCPUs)
{
    bool retVal = true;
    osLinuxProcFileSystemReader procReader;
    procReader.updateCPUsData();
    amountOfCPUs = procReader.cpusAmount();
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineMemoryLimits
// Description: Retrieves information about the local machine memory limits.
// Arguments:   physicalMemAmount - The amount of physical memory, measured in bytes.
//              virtualMemAmount - The amount of virtual memory, measured in bytes.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        15/5/2008
// ---------------------------------------------------------------------------
bool osGetLocalMachineMemoryLimits(gtSize_t& physicalMemAmount, gtSize_t& virtualMemAmount)
{
    bool retVal = true;

    // Get the local machine memory info:
    osLinuxProcFileSystemReader procReader;
    procReader.updatePhysicalMemoryData();
    const osPhysicalMemorySampledData& physicalMemData = procReader.physicalMemoryData();

    // Output the memory limits:
    physicalMemAmount = physicalMemData._totalPhysicalMemory;
    virtualMemAmount = physicalMemData._totalVirtualMemory;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineVirtualMemPageSize
// Description: Retrieves the local machine virtual memory page size.
// Arguments:   pageSize - Will get the page size, measured in bytes.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        14/8/2005
// ---------------------------------------------------------------------------
bool osGetLocalMachineVirtualMemPageSize(unsigned long& pageSize)
{
    bool retVal = true;
    // Get the local machine memory info:
    osLinuxProcFileSystemReader procReader;
    procReader.updatePhysicalMemoryData();
    const osPhysicalMemorySampledData& physicalMemData = procReader.physicalMemoryData();

    pageSize = physicalMemData._hugePageSize;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineMemoryInformationStrings
// Description: OGets the system memory information as a string, e.g. "512 MB"
// Arguments: totalRam - will contain total amount of RAM
//            availRam - will contain amount of available RAM
//            totalPage - will contain total amount of Page memory
//            availPage - will contain amount of available Page memory
//            totalVirtual - will contain total amount of Virtual memory
//            availVirtual - will contain amount of available Virtual memory
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        18/5/2008
// ---------------------------------------------------------------------------
bool osGetLocalMachineMemoryInformationStrings(gtString& totalRam, gtString& availRam, gtString& totalPage, gtString& availPage, gtString& totalVirtual, gtString& availVirtual)
{

    gtUInt64 totalRamUInt = 0;
    gtUInt64 availRamUInt = 0;
    gtUInt64 totalPageUInt = 0;
    gtUInt64 availPageUInt = 0;
    gtUInt64 totalVirtualUInt = 0;
    gtUInt64 availVirtualUInt = 0;

    bool retVal = osGetLocalMachineMemoryInformation(totalRamUInt, availRamUInt, totalPageUInt, availPageUInt, totalVirtualUInt, availVirtualUInt);

    if (retVal)
    {
        totalRamUInt = totalRamUInt / 1024 / 1024;
        availRamUInt = availRamUInt / 1024 / 1024;
        totalPageUInt = totalPageUInt / 1024 / 1024;
        availPageUInt = availPageUInt / 1024 / 1024;
        totalVirtualUInt = totalVirtualUInt / 1024 / 1024;
        availVirtualUInt = availVirtualUInt / 1024 / 1024;

        // Get the Total RAM
        if (totalRamUInt != 0)
        {
            totalRam.makeEmpty();
            totalRam.appendFormattedString(L"%d MB", totalRamUInt);
        }
        else
        {
            totalRam = OS_STR_NotAvailable;
        }

        // Get the Available RAM
        if (availRamUInt != 0)
        {
            availRam.makeEmpty();
            availRam.appendFormattedString(L"%d MB", availRamUInt);
        }
        else
        {
            availRam = OS_STR_NotAvailable;
        }

        // Get the Total Swap Memory
        if (totalPageUInt != 0)
        {
            totalPage.makeEmpty();
            totalPage.appendFormattedString(L"%d MB", totalPageUInt);
        }
        else
        {
            totalPage = OS_STR_NotAvailable;
        }

        // Get the Available Swap Memory
        if (availPageUInt != 0)
        {
            availPage.makeEmpty();
            availPage.appendFormattedString(L"%d MB", availPageUInt);
        }
        else
        {
            availPage = OS_STR_NotAvailable;
        }

        // Get the Total Virtual Memory
        if (totalVirtualUInt != 0)
        {
            totalVirtual.makeEmpty();
            totalVirtual.appendFormattedString(L"%d MB", totalVirtualUInt);
        }
        else
        {
            totalVirtual = OS_STR_NotAvailable;
        }

        // Get the Available Virtual Memory
        if (availVirtualUInt != 0)
        {
            availVirtual.makeEmpty();
            availVirtual.appendFormattedString(L"%d MB", availVirtualUInt);
        }
        else
        {
            availVirtual = OS_STR_NotAvailable;
        }
    }

    return retVal;
}

bool osGetLocalMachineMemoryInformation(gtUInt64& totalRam, gtUInt64& availRam, gtUInt64& totalPage, gtUInt64& availPage, gtUInt64& totalVirtual, gtUInt64& availVirtual)
{
    // Get the local machine memory info:
    osLinuxProcFileSystemReader procReader;
    bool result = procReader.updatePhysicalMemoryData();

    if (result)
    {
        const osPhysicalMemorySampledData& physicalMemData = procReader.physicalMemoryData();

        totalRam = physicalMemData._totalPhysicalMemory;
        availRam = physicalMemData._freePhysicalMemory;
        totalPage = physicalMemData._totalSwapMemory;
        availPage = physicalMemData._freeSwapMemory;
        totalVirtual = physicalMemData._totalVirtualMemory;
        availVirtual = physicalMemData._freeVirtualMemory;
    }

    return result;
}

// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineCPUInformationStrings
// Description: Gets the number of CPUs and their type (e.g. 586) and returns that
//              Information as Strings.
// Arguments: numberOfProcessors - will contain number of processors
//            processorType - will contain types of processors
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        21/5/2008
// ---------------------------------------------------------------------------
bool osGetLocalMachineCPUInformationStrings(gtString& numberOfProcessors, gtString& processorType)
{
    bool retVal = true;
    osLinuxProcFileSystemReader procReader;
    retVal = retVal && procReader.updateCPUsData();
    int numOfCPUsAsInt = procReader.cpusAmount();

    if (numOfCPUsAsInt > 0)
    {
        numberOfProcessors.makeEmpty();
        numberOfProcessors.appendFormattedString(L"%d", numOfCPUsAsInt);
    }
    else
    {
        numberOfProcessors = OS_STR_NotAvailable;
    }

    retVal = retVal && procReader.getCPUtype(0, processorType);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineUserAndDomain
// Description: Gets the current local machine username and domain
// Arguments: userName - user name goes here
//            userDomain - domain goes here
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        15/5/2008
// ---------------------------------------------------------------------------
bool osGetLocalMachineUserAndDomain(gtString& userName, gtString& userDomain)
{
    userName.makeEmpty();
    userDomain.makeEmpty();
    bool rcUser = osGetCurrentProcessEnvVariableValue(L"USER", userName);
    bool rcDomain = osGetCurrentProcessEnvVariableValue(L"USERDOMAIN", userDomain);

    if (userDomain.isEmpty())
    {
        osLinuxProcFileSystemReader procReader;
        rcDomain = procReader.getDomainName(userDomain);
        GT_ASSERT(rcDomain);

        if ((!rcDomain) || (userDomain.isEmpty()))
        {
            userDomain = OS_STR_NotAvailable;
        }
    }

    bool retVal = rcUser && rcDomain;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineSystemPathAndDelims
// Description:
// Arguments: computerPath - will contain the list of system paths, separated by computerPathDelims
//            computerPathDelims - will contain the path list delimiters, in this case
//              only the semicolon.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        15/5/2008
// ---------------------------------------------------------------------------
bool osGetLocalMachineSystemPathAndDelims(gtString& computerPath, gtString& computerPathDelims)
{
    bool retVal = osGetCurrentProcessEnvVariableValue(L"PATH", computerPath);
    computerPathDelims = osFilePath::osEnvironmentVariablePathsSeparator;
    return retVal;
}


