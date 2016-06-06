//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osGeneralFunctions.cpp
///
//=====================================================================

//------------------------------ osGeneralFunctions.cpp ------------------------------

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#define _WIN32_WINNT 0x0501

#pragma comment(lib, "netapi32.lib")
#include <Windows.h>
#include <VersionHelpers.h>
#include <lm.h>
// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>

// Definitions missing in Windows.h:
typedef void (WINAPI* PGNSI)(LPSYSTEM_INFO);
#define BUFSIZE 80
#define VER_SUITE_WH_SERVER 0x00008000

void GetWinOSVersionNumberHelper(int& majorVersion, int& minorVersion);
// ---------------------------------------------------------------------------
// Name:        osGetWindowsVersion
// Description: Retrieves the Windows OS version on which this process runs.
// Arguments: windowsVersion - Will get the Windows OS version.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        9/5/2007
// Implementations notes:
//  The MSDN documentation of the "OSVERSIONINFOEX Structure" contains a table
//  that explains how to identify the windows version on which this program runs.
//  The URL is: http://msdn.microsoft.com/en-us/library/ms724833(VS.85).aspx
// ---------------------------------------------------------------------------

bool osGetWindowsVersion(osWindowsVersion& windowsVersion)
{
    bool retVal = false;
    // Will contain the Windows version information:
    OSVERSIONINFOEX osVersionInfo;
    ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOEX));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    // Get the Windows version information:
    BOOL rc = ::GetVersionEx((OSVERSIONINFO*)&osVersionInfo);
    GT_IF_WITH_ASSERT(rc != FALSE)
    {
        // Get the Windows system information:
        SYSTEM_INFO systemInfo;
        ::GetNativeSystemInfo(&systemInfo);

        // Calculate the Windows OS version:
        if (osVersionInfo.dwMajorVersion == 4)
        {
            // Version 4 products:
            // ------------------

            if (osVersionInfo.dwMinorVersion == 10)
            {
                windowsVersion = OS_WIN_98;
                retVal = true;
            }
            else if (osVersionInfo.dwMinorVersion == 90)
            {
                windowsVersion = OS_WIN_ME;
                retVal = true;
            }
        }
        else if (osVersionInfo.dwMajorVersion == 5)
        {
            // Version 5 products:
            // ------------------

            if (osVersionInfo.dwMinorVersion == 0)
            {
                windowsVersion = OS_WIN_2000;
                retVal = true;
            }
            else if (osVersionInfo.dwMinorVersion == 1)
            {
                windowsVersion = OS_WIN_XP;
                retVal = true;
            }
            else if (osVersionInfo.dwMinorVersion == 2)
            {
                if (GetSystemMetrics(SM_SERVERR2) != 0)
                {
                    windowsVersion = OS_WIN_SERVER_2003_R2;
                    retVal = true;
                }
                else if (osVersionInfo.wSuiteMask == VER_SUITE_WH_SERVER)
                {
                    windowsVersion = OS_WINDOWS_HOME_SERVER;
                    retVal = true;
                }
                else if ((osVersionInfo.wProductType == VER_NT_WORKSTATION) && (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64))
                {
                    windowsVersion = OS_WINDOWS_XP_PRO_X64;
                    retVal = true;
                }
                else
                {
                    windowsVersion = OS_WIN_SERVER_2003;
                    retVal = true;
                }
            }
        }
        else if (osVersionInfo.dwMajorVersion == 6)
        {
            // Version 6 products:
            // ------------------
            int major, minor;
            GetWinOSVersionNumberHelper(major, minor);

            if (10 == major)//PATCH : probably running in un-manifested windows 10 exe, thus getversion returned '6'
            {
                windowsVersion = OS_WIN_10;
                retVal = true;
            }
            else if (osVersionInfo.dwMinorVersion == 0)
            {
                if (osVersionInfo.wProductType == VER_NT_WORKSTATION)
                {
                    windowsVersion = OS_WIN_VISTA;
                    retVal = true;
                }
                else
                {
                    windowsVersion = OS_WIN_SERVER_2008;
                    retVal = true;
                }
            }
            else if (osVersionInfo.dwMinorVersion == 1)
            {
                if (osVersionInfo.wProductType == VER_NT_WORKSTATION)
                {
                    windowsVersion = OS_WIN_7;
                    retVal = true;
                }
                else
                {
                    windowsVersion = OS_WIN_SERVER_2008_R2;
                    retVal = true;
                }
            }
            else if (osVersionInfo.dwMinorVersion == 2)
            {
                if (osVersionInfo.wProductType == VER_NT_WORKSTATION)
                {
                    windowsVersion = OS_WIN_8;
                    retVal = true;
                }
                else
                {
                    windowsVersion = OS_WIN_SERVER_2012;
                    retVal = true;
                }
            }
            else if (osVersionInfo.dwMinorVersion == 3)
            {
                if (osVersionInfo.wProductType == VER_NT_WORKSTATION)
                {
                    windowsVersion = OS_WIN_8_1;
                    retVal = true;
                }
                else
                {
                    windowsVersion = OS_WIN_SERVER_2012_R2;
                    retVal = true;
                }
            }
            else if (osVersionInfo.dwMinorVersion == 4)
            {
                if (osVersionInfo.wProductType == VER_NT_WORKSTATION)
                {
                    windowsVersion = OS_WIN_10;
                    retVal = true;
                }
            }
        }
        else if (osVersionInfo.dwMajorVersion == 10)
        {
            if (osVersionInfo.dwMinorVersion == 0)
            {
                if (osVersionInfo.wProductType == VER_NT_WORKSTATION)
                {
                    windowsVersion = OS_WIN_10;
                    retVal = true;
                }
            }
        }

        // If we have DEBUG level logging, create a debug string here:
        if (osDebugLog::instance().loggedSeverity() >= OS_DEBUG_LOG_DEBUG)
        {
            gtString logMsg;
            osWindowsVersionToString(windowsVersion, logMsg);
            logMsg.prependFormattedString(L"Interpreted Windows version %d.%d with product type %#x and suite mask %#x as ", (int)osVersionInfo.dwMajorVersion, (int)osVersionInfo.dwMinorVersion, (int)osVersionInfo.wProductType, (int)osVersionInfo.wSuiteMask);
            OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osWindowsVersionToString
// Description: Inputs a windows version and returns a short string that describes it.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/7/2007
// ---------------------------------------------------------------------------
bool osWindowsVersionToString(const osWindowsVersion& windowsVersion, gtString& windowsVersionStr)
{
    bool retVal = true;

    switch (windowsVersion)
    {
        case OS_WIN_98:
            windowsVersionStr = L"Windows 98";
            break;

        case OS_WIN_ME:
            windowsVersionStr = L"Windows Millennium";
            break;

        case OS_WIN_2000:
            windowsVersionStr = L"Windows 2000";
            break;

        case OS_WIN_XP:
            windowsVersionStr = L"Windows XP";
            break;

        case OS_WINDOWS_XP_PRO_X64:
            windowsVersionStr = L"Windows XP Professional x64 Edition";
            break;

        case OS_WINDOWS_HOME_SERVER:
            windowsVersionStr = L"Windows Home Server";
            break;

        case OS_WIN_SERVER_2003:
            windowsVersionStr = L"Windows Server 2003";
            break;

        case OS_WIN_SERVER_2003_R2:
            windowsVersionStr = L"Windows Server 2003 R2";
            break;

        case OS_WIN_VISTA:
            windowsVersionStr = L"Windows Vista";
            break;

        case OS_WIN_SERVER_2008:
            windowsVersionStr = L"Windows Server 2008";
            break;

        case OS_WIN_SERVER_2008_R2:
            windowsVersionStr = L"Windows Server 2008 R2";
            break;

        case OS_WIN_7:
            windowsVersionStr = L"Windows 7";
            break;

        case OS_WIN_8:
            windowsVersionStr = L"Windows 8";
            break;

        case OS_WIN_SERVER_2012:
            windowsVersionStr = L"Windows Server 2012";
            break;

        case OS_WIN_8_1:
            windowsVersionStr = L"Windows 8.1";
            break;

        case OS_WIN_10:
            windowsVersionStr = L"Windows 10";
            break;

        case OS_WIN_SERVER_2012_R2:
            windowsVersionStr = L"Windows Server 2012 R2";
            break;

        default:
        {
            // Unknown windows version.
            windowsVersionStr.makeEmpty();
            retVal = false;
        }
        break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osWindowsVersionToShortString
// Description: Inputs a windows version and returns a short string that describes it.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/7/2007
// ---------------------------------------------------------------------------
bool osWindowsVersionToShortString(const osWindowsVersion& windowsVersion, gtString& windowsVersionStr)
{
    bool retVal = true;

    switch (windowsVersion)
    {
        case OS_WIN_98:
            windowsVersionStr = L"Win98";
            break;

        case OS_WIN_ME:
            windowsVersionStr = L"WinME";
            break;

        case OS_WIN_2000:
            windowsVersionStr = L"Win2000";
            break;

        case OS_WIN_XP:
            windowsVersionStr = L"WinXP";
            break;

        case OS_WINDOWS_XP_PRO_X64:
            windowsVersionStr = L"WinXPProX64";
            break;

        case OS_WINDOWS_HOME_SERVER:
            windowsVersionStr = L"WinHomeServer";
            break;

        case OS_WIN_SERVER_2003:
            windowsVersionStr = L"Win2003Server";
            break;

        case OS_WIN_SERVER_2003_R2:
            windowsVersionStr = L"Win2003ServerR2";
            break;

        case OS_WIN_VISTA:
            windowsVersionStr = L"WinVista";
            break;

        case OS_WIN_SERVER_2008:
            windowsVersionStr = L"Win2008Server";
            break;

        case OS_WIN_SERVER_2008_R2:
            windowsVersionStr = L"Win2008ServerR2";
            break;

        case OS_WIN_7:
            windowsVersionStr = L"Win7";
            break;

        case OS_WIN_8:
            windowsVersionStr = L"Win8";
            break;

        case OS_WIN_SERVER_2012:
            windowsVersionStr = L"Win2012Server";
            break;

        case OS_WIN_8_1:
            windowsVersionStr = L"Win8.1";
            break;

        case OS_WIN_10:
            windowsVersionStr = L"Win10";
            break;

        case OS_WIN_SERVER_2012_R2:
            windowsVersionStr = L"Win2012ServerR2";
            break;

        default:
        {
            // Unknown windows version.
            windowsVersionStr = OS_STR_NotAvailable;
            retVal = false;
        }
        break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetOSShortDescriptionString
// Description: Retrieves a short string describing the Operating System on which
//              this program runs.
// Author:      AMD Developer Tools Team
// Return Val: bool  - Success / failure.
// Date:        11/7/2007
// ---------------------------------------------------------------------------
bool osGetOSShortDescriptionString(gtString& osDescriptionString)
{
    bool retVal = false;
    osDescriptionString.makeEmpty();

    // Get the version of the Windows OS on which this program runs:
    osWindowsVersion windowsVersion = OS_WIN_XP;
    bool rc1 = osGetWindowsVersion(windowsVersion);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Translate it to a string:
        gtString windowsVersionStr;
        bool rc2 = osWindowsVersionToShortString(windowsVersion, windowsVersionStr);
        GT_IF_WITH_ASSERT(rc2)
        {
            // Get the OS address space:
            gtString osAddressSpace;
            osGetOSAddressSpaceString(osAddressSpace);

            // Build the returned string:
            osDescriptionString = windowsVersionStr;
            osDescriptionString += L"-";
            osDescriptionString += osAddressSpace;

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osGetOSAddressSpaceString
// Description:
//   Retrieves a string describing the address space type of the
//   Operating System on which we run.
// Author:      AMD Developer Tools Team
// Date:        4/2/2009
// ---------------------------------------------------------------------------
void osGetOSAddressSpaceString(gtString& addressSpaceString)
{
    // Get the Windows system information:
    SYSTEM_INFO systemInfo;
    ::GetNativeSystemInfo(&systemInfo);

    if (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
    {
        addressSpaceString = OS_64_BIT_ADDRESS_SPACE_AS_STR;
    }
    else if (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
    {
        addressSpaceString = OS_ITANIUM_64_BIT_ADDRESS_SPACE_AS_STR;
    }
    else if (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
    {
        addressSpaceString = OS_32_BIT_ADDRESS_SPACE_AS_STR;
    }
    else
    {
        addressSpaceString = L"Unknown";
    }
}


// ---------------------------------------------------------------------------
// Name:        osGetOSAddressSpace
// Description: Return address space
// Arguments: int& addressSpace
// Return Val: OS_API void
// Author:      AMD Developer Tools Team
// Date:        20/10/2009
// ---------------------------------------------------------------------------
OS_API bool osGetOSAddressSpace(int& addressSpace)
{
    bool retVal = true;
    // Get the Windows system information:
    SYSTEM_INFO systemInfo;
    ::GetNativeSystemInfo(&systemInfo);

    if ((systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) ||
        (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64))
    {
        addressSpace = AMDT_64_BIT_ADDRESS_SPACE;
    }

    else if (systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
    {
        addressSpace = AMDT_32_BIT_ADDRESS_SPACE;
    }
    else
    {
        retVal = false;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetOperatingSystemVersionNumber
// Description: Get the Operating System Version as Int
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        24/5/2006
// ---------------------------------------------------------------------------
bool osGetOperatingSystemVersionNumber(int& majorVersion, int& minorVersion, int& buildNumber)
{
    OSVERSIONINFOEX osvi;
    BOOL bOsVersionInfoEx;

    // Try calling GetVersionEx using the OSVERSIONINFOEX structure.
    // If that fails, try using the OSVERSIONINFO structure.

    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO*)&osvi);

    if (!bOsVersionInfoEx)
    {
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

        if (! GetVersionEx((OSVERSIONINFO*) &osvi))
        {
            return FALSE;
        }
    }

    GetWinOSVersionNumberHelper(majorVersion, minorVersion);

    if (static_cast<int>(osvi.dwMajorVersion) >= majorVersion && static_cast<int>(osvi.dwMinorVersion) >= minorVersion)
    {
        majorVersion = osvi.dwMajorVersion;
        minorVersion = osvi.dwMinorVersion;
        buildNumber = osvi.dwBuildNumber;
    }
    else
    {
        buildNumber = 0;//we can't figure out the build number
    }

    return true;
}


// ---------------------------------------------------------------------------
// Name:        osGetOperatingSystemVersionString
// Description: Outputs a string representing the operating system version name
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/2/2009
// ---------------------------------------------------------------------------
bool osGetOperatingSystemVersionString(gtString& osVersionName)
{
    bool retVal = false;

    // Get the current windows version:
    osWindowsVersion windowsVersion;
    bool rc1 = osGetWindowsVersion(windowsVersion);
    GT_IF_WITH_ASSERT(rc1)
    {
        bool rc2 = osWindowsVersionToString(windowsVersion, osVersionName);
        GT_IF_WITH_ASSERT(rc2)
        {
            retVal = true;
        }
    }

    return retVal;
}

void GetWinOSVersionNumberHelper(int& majorVersion, int& minorVersion)
{
    static int MajorVersion = -1, MinorVersion = -1;

    if (-1 == MajorVersion || -1 == MinorVersion)
    {
        const DWORD dwLevel = 100;
        LPWKSTA_INFO_100 pBuf = nullptr;
        NET_API_STATUS nStatus;
        //
        // Call the NetWkstaGetInfo function, specifying level 100.
        //
        nStatus = NetWkstaGetInfo(nullptr, dwLevel, (LPBYTE*)&pBuf);

        //
        // If the call is successful, print the workstation data.
        //
        GT_IF_WITH_ASSERT(nStatus == NERR_Success)
        {
            MajorVersion = pBuf->wki100_ver_major;
            MinorVersion = pBuf->wki100_ver_minor;
        }

        //
        // Free the allocated memory.
        //
        if (pBuf != nullptr)
        {
            GT_ASSERT(NERR_Success == NetApiBufferFree(pBuf));
        }
    }

    majorVersion = MajorVersion;
    minorVersion = MinorVersion;

}