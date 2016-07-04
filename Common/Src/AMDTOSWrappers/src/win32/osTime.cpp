//------------------------------ osTime.cpp ------------------------------
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>


#include <AMDTOSWrappers/Include/osTime.h>

// ---------------------------------------------------------------------------
// Name:        currentPreciseTimeAsString
// Description: Get the current local time in string format, including milliseconds
// Arguments:   strTime         - output string
//              stringFormat    - the format that will applied to the string
// Return Val:  void
// Author:      Doron Ofek
// Date:        Dec-24, 2015
// ---------------------------------------------------------------------------
bool osTime::currentPreciseTimeAsString(gtString& strTime, TimeFormat stringFormat)
{
    FILETIME utcFileTime, localFileTime;
    GetSystemTimeAsFileTime(&utcFileTime);
    FileTimeToLocalFileTime(&utcFileTime, &localFileTime);
    SYSTEMTIME sysTime;
    FileTimeToSystemTime(&localFileTime, &sysTime);
    bool retVal = false;

    switch (stringFormat)
    {
        case UNDERSCORE_SAPERATOR:
        {
            strTime.appendFormattedString(L"%02d_%02d_%02d_%03d",
                                          static_cast<int>(sysTime.wHour),
                                          static_cast<int>(sysTime.wMinute),
                                          static_cast<int>(sysTime.wSecond),
                                          static_cast<int>(sysTime.wMilliseconds));
            retVal = true;
        }
        break;

        case WINDOWS_STYLE:
        {
            strTime.appendFormattedString(L"%02d:%02d:%02d.%03d",
                                          static_cast<int>(sysTime.wHour),
                                          static_cast<int>(sysTime.wMinute),
                                          static_cast<int>(sysTime.wSecond),
                                          static_cast<int>(sysTime.wMilliseconds));
            retVal = true;
        }
        break;

        case DATE_TIME_DISPLAY:
        {
            strTime.appendFormattedString(L"%04d.%02d.%02d\t%02d:%02d:%02d.%03d",
                                          static_cast<int>(sysTime.wYear),
                                          static_cast<int>(sysTime.wMonth),
                                          static_cast<int>(sysTime.wDay),
                                          static_cast<int>(sysTime.wHour),
                                          static_cast<int>(sysTime.wMinute),
                                          static_cast<int>(sysTime.wSecond),
                                          static_cast<int>(sysTime.wMilliseconds));
            retVal = true;
        }
        break;

        case UNIX_STYLE:
        {
            // TO_DO: Not implemented yet !!!
            // All unimplemented cases are handled together so fall through to next case
        }

        case FOR_EMAIL:
        {
            // This setting is only to be used in date as string function
            // All unimplemented cases are handled together so fall through to next case
        }

        case NAME_SCHEME_DISPLAY:
        {
            // This setting is only to be used in date as string function
            // All unimplemented cases are handled together so fall through to next case
        }

        case NAME_SCHEME_FILE:
        {
            // This setting is only to be used in date as string function
            GT_ASSERT(0);
        }
        break;

        default:
        {
            // Unknown string format - Issue an assert and get time string as DATE_TIME_DISPLAY:
            GT_ASSERT(0);

            strTime.appendFormattedString(L"%04d.%02d.%02d\t%02d:%02d:%02d.%03d",
                                          static_cast<int>(sysTime.wYear),
                                          static_cast<int>(sysTime.wMonth),
                                          static_cast<int>(sysTime.wDay),
                                          static_cast<int>(sysTime.wHour),
                                          static_cast<int>(sysTime.wMinute),
                                          static_cast<int>(sysTime.wSecond),
                                          static_cast<int>(sysTime.wMilliseconds));
            retVal = true;
        }
        break;
    }

    return retVal;
}