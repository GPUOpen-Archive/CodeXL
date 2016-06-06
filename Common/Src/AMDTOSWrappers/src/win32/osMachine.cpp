//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osMachine.cpp
///
//=====================================================================

//------------------------------ osMachine.cpp ------------------------------

// Windows user information:
#define WIN32_LEAN_AND_MEAN 1
#define _WIN32_WINNT 0x0501
#include <Windows.h>
#include <stdio.h>
#include <windows.h>
#include <lm.h>


// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osMachine.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>


// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineName
// Description: Returns the local machine name.
// Author:      AMD Developer Tools Team
// Date:        11/7/2005
// ---------------------------------------------------------------------------
bool osGetLocalMachineName(gtString& localMachineName)
{
    bool retVal = false;
    localMachineName.makeEmpty();

    wchar_t* compnameVar = NULL;
    size_t requiredSize = 0;

    // Get the required buffer size:
    _wgetenv_s(&requiredSize, NULL, 0, L"COMPUTERNAME");

    // Allocate the buffer:
    compnameVar = (wchar_t*) malloc(requiredSize * sizeof(wchar_t));
    GT_IF_WITH_ASSERT(compnameVar != NULL)
    {
        // Get the value of the COMPUTERNAME environment variable.
        _wgetenv_s(&requiredSize, compnameVar, requiredSize, L"COMPUTERNAME");

        localMachineName = compnameVar;

        retVal = true;
    }

    // Delete the raw memory buffer:
    free(compnameVar);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineName
// Description: Retrieves the name of the local machine (computer) (as ASCII)
// Arguments:   localMachineName - Will get the local machine name.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        13/9/2010
// ---------------------------------------------------------------------------
bool osGetLocalMachineName(gtASCIIString& localMachineName)
{
    bool retVal = false;

    // Get the machine name in ASCII string:
    gtString ansiiMachineName;
    retVal = osGetLocalMachineName(ansiiMachineName);

    // Convert to Unicode:
    localMachineName = ansiiMachineName.asASCIICharArray();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetAmountOfLocalMachineCPUs
// Description: Retrieves the amount of local machine CPUs.
// Arguments:   amountOfCPUs - Will get the amount of local machine CPUs.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        14/8/2005
// ---------------------------------------------------------------------------
bool osGetAmountOfLocalMachineCPUs(int& amountOfCPUs)
{
    amountOfCPUs = 1;

    // Get the current machine system information:
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);

    // Output the amount of machine processors:
    amountOfCPUs = siSysInfo.dwNumberOfProcessors;

    return true;
}


// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineMemoryLimits
// Description: Retrieves information about the local machine memory limits.
// Arguments:   physicalMemAmount - The amount of physical memory, measured in bytes.
//              virtualMemAmount - The amount of virtual memory, measured in bytes.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        14/8/2005
// ---------------------------------------------------------------------------
bool osGetLocalMachineMemoryLimits(gtSize_t& physicalMemAmount, gtSize_t& virtualMemAmount)
{
    // Get the local machine memory info:
    MEMORYSTATUS memStatus;
    GlobalMemoryStatus(&memStatus);

    // Output the memory limits:
    physicalMemAmount = memStatus.dwAvailPhys;
    virtualMemAmount = memStatus.dwAvailVirtual;

    return true;
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
    bool retVal = false;

    // If this is a Win32 machine:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // On Win32, the page size is 4K:
    pageSize = 4096;
    retVal = true;
#else
#error Unsupported build target!
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineMemoryInformationStrings
// Description: Gets the system memory information as a string, e.g. "512 MB"
// Arguments: totalRam - will contain total amount of RAM
//            availRam - will contain amount of available RAM
//            totalPage - will contain total amount of Page memory
//            availPage - will contain amount of available Page memory
//            totalVirtual - will contain total amount of Virtual memory
//            availVirtual - will contain amount of available Virtual memory
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        15/5/2008
// ---------------------------------------------------------------------------
bool osGetLocalMachineMemoryInformationStrings(gtString& totalRam, gtString& availRam, gtString& totalPage, gtString& availPage, gtString& totalVirtual, gtString& availVirtual)
{
    gtUInt64 totalRamSizet = 0;
    gtUInt64 availRamSizet = 0;
    gtUInt64 totalPageSizet = 0;
    gtUInt64 availPageSizet = 0;
    gtUInt64 totalVirtualSizet = 0;
    gtUInt64 availVirtualSizet = 0;

    bool retVal = osGetLocalMachineMemoryInformation(totalRamSizet, availRamSizet, totalPageSizet, availPageSizet, totalVirtualSizet, availVirtualSizet);

    if (retVal)
    {
        // Get the Total RAM
        totalRamSizet = totalRamSizet / 1024 / 1024;
        totalRam.appendFormattedString(L"%d", totalRamSizet);
        totalRam.append(L" MB");

        // Get the Available RAM
        availRamSizet = availRamSizet / 1024 / 1024;
        availRam.appendFormattedString(L"%d", availRamSizet);
        availRam.append(L" MB");

        // Get the Total Page Files
        totalPageSizet = totalPageSizet / 1024 / 1024;
        totalPage.appendFormattedString(L"%d", totalPageSizet);
        totalPage.append(L" MB");

        // Get the Available Page Files
        availPageSizet = availPageSizet / 1024 / 1024;
        availPage.appendFormattedString(L"%d", availPageSizet);
        availPage.append(L" MB");

        // Get the Total Virtual Memory
        totalVirtualSizet = totalVirtualSizet / 1024 / 1024;
        totalVirtual.appendFormattedString(L"%d", totalVirtualSizet);
        totalVirtual.append(L" MB");

        // Get the Available Virtual Memory
        availVirtualSizet = availVirtualSizet / 1024 / 1024;
        availVirtual.appendFormattedString(L"%d", availVirtualSizet);
        availVirtual.append(L" MB");
    }

    return retVal;
}


bool osGetLocalMachineMemoryInformation(gtUInt64& totalRam, gtUInt64& availRam, gtUInt64& totalPage, gtUInt64& availPage, gtUInt64& totalVirtual, gtUInt64& availVirtual)
{

    MEMORYSTATUSEX stMemStatus;
    stMemStatus.dwLength = sizeof(MEMORYSTATUSEX);

    bool retVal = TRUE ==  GlobalMemoryStatusEx(&stMemStatus);

    if (retVal)
    {
        // Get the Total RAM
        totalRam = stMemStatus.ullTotalPhys;


        // Get the Available RAM
        availRam = stMemStatus.ullAvailPhys;

        // Get the Total Page Files
        totalPage = stMemStatus.ullTotalPageFile;

        // Get the Available Page Files
        availPage = stMemStatus.ullAvailPageFile;

        // Get the Total Virtual Memory
        totalVirtual = stMemStatus.ullTotalVirtual;

        // Get the Available Virtual Memory
        availVirtual = stMemStatus.ullAvailVirtual;
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        osGetLocalMachineCPUInformationStrings
// Description: Gets the number of CPUs and their type (e.g. 586) and returns that
//              Information as Strings.
// Arguments: numberOfProcessors - will contain number of processors
//            processorType - will contain types of processors
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        15/5/2008
// ---------------------------------------------------------------------------
bool osGetLocalMachineCPUInformationStrings(gtString& numberOfProcessors, gtString& processorType)
{
    bool retVal = true;

    // Get system info:
    SYSTEM_INFO siSysInfo;
    GetSystemInfo(&siSysInfo);

    // Get number of processors:
    numberOfProcessors.appendFormattedString(L"%d", siSysInfo.dwNumberOfProcessors);

    // Get the processor type from processor architecture enumeration:
    if (siSysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
    {
        processorType = L"x86";
    }
    else if (siSysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
    {
        processorType = L"x64";
    }
    else if (siSysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
    {
        processorType = L"IA-64";
    }
    else
    {
        processorType = OS_STR_unknown;
    }

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
    bool retVal = true;
    computerPath = _wgetenv(L"PATH");
    computerPathDelims = osFilePath::osEnvironmentVariablePathsSeparator;
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
    bool retVal = true;
    userName = _wgetenv(L"USERNAME");
    userDomain = _wgetenv(L"USERDOMAIN");
    return retVal;
}
