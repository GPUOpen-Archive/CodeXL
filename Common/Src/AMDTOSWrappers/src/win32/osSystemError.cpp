//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osSystemError.cpp
///
//=====================================================================

//------------------------------ osSystemError.cpp ------------------------------

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osSystemError.h>


// ---------------------------------------------------------------------------
// Name:        osGetLastSystemError
// Description: Returns the last error code recorded by the operating system
//              for the calling thread.
// Author:      AMD Developer Tools Team
// Date:        28/1/2008
// ---------------------------------------------------------------------------
osSystemErrorCode osGetLastSystemError()
{
    osSystemErrorCode retVal = ::GetLastError();
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osGetLastSystemErrorAsString
// Description: Outputs the last error code recorded by the operating system
//              for the calling thread, translated into a string.
// Author:      AMD Developer Tools Team
// Date:        Dec-29, 2015
// ---------------------------------------------------------------------------
void osGetLastSystemErrorAsString(gtString& systemErrorAsString)
{
    // Get the system's last recorded error code:
    osSystemErrorCode systemLastError = osGetLastSystemError();
    osGetSystemErrorAsString(systemLastError, systemErrorAsString);
}

// ---------------------------------------------------------------------------
// Name:        osGetLastSystemErrorAsString
// Description: Outputs the error code, translated into a string.
// Author:      Yaki Tebeka
// Date:        28/1/2008
// ---------------------------------------------------------------------------
void osGetSystemErrorAsString(osSystemErrorCode systemError, gtString& systemErrorAsString)
{
    systemErrorAsString = OS_STR_unknownSystemError;

    // If no system error was recorded:
    if (systemError == 0)
    {
        systemErrorAsString = OS_STR_noSystemError;
    }
    else
    {
        // Get a string describing the system error:
        LPVOID lpMsgBuf = NULL;
        DWORD rc1 = ::FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, systemError,
                                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&lpMsgBuf, 0, NULL);

        if ((0 < rc1) && (lpMsgBuf != NULL))
        {
            // Output the string we got:
            systemErrorAsString = (wchar_t*)lpMsgBuf;

            // Clean up:
            ::LocalFree(lpMsgBuf);
            lpMsgBuf = NULL;
        }
    }
}

