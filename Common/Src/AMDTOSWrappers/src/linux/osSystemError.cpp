//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osSystemError.cpp
///
//=====================================================================

//------------------------------ osSystemError.cpp ------------------------------

// POSIX:
#include <errno.h>
#include <string.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osSystemError.h>

// The size of a buffer that will get the system's error as a string:
#define OS_SYSTEM_ERROR_BUFF_SIZE 1024


// ---------------------------------------------------------------------------
// Name:        osGetLastSystemError
// Description: Returns the last error code recorded by the operating system
//              for the calling thread.
// Author:      AMD Developer Tools Team
// Date:        28/1/2008
// ---------------------------------------------------------------------------
osSystemErrorCode osGetLastSystemError()
{
    osSystemErrorCode retVal = errno;
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
#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
        // Get a string describing the system error:
        char buff[OS_SYSTEM_ERROR_BUFF_SIZE];
        char* pErrMsg = strerror_r(systemError, buff, OS_SYSTEM_ERROR_BUFF_SIZE);

        if (pErrMsg != NULL)
        {
            // Output the string we got:
            systemErrorAsString.fromASCIIString(pErrMsg);
        }

#elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
        // Get a string describing the system error:
        char buff[OS_SYSTEM_ERROR_BUFF_SIZE + 1];
        int rc1 = strerror_r(systemError, buff, OS_SYSTEM_ERROR_BUFF_SIZE);

        if (rc1 == 0)
        {
            // Null-terminate the string:
            buff[OS_SYSTEM_ERROR_BUFF_SIZE] = '\0';

            // Output the string we got:
            systemErrorAsString.fromASCIIString(buff);
        }

#else
#error Unknown Linux variant
#endif
    }
}

