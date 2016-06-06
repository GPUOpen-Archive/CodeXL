//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTime.cpp
///
//=====================================================================

//------------------------------ osTime.cpp ------------------------------

// C:
#include <stdio.h>
#include <time.h>
#include <chrono>

//C++
#include <map>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>

// Local:
#include <AMDTOSWrappers/Include/osStopWatch.h>
#include <AMDTOSWrappers/Include/osTime.h>

// Per OS include file that contains timeval definition:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <Winsock2.h>
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <sys/types.h>
#else
    #error Error: unknow build platform!
#endif

// Translates int to day strings:
const char* intToWeekDayString[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
const wchar_t* intToWeekDayWideString[] = { L"Sunday", L"Monday", L"Tuesday", L"Wednesday", L"Thursday", L"Friday", L"Saturday" };

// Translates int to short day strings:
const char* intToShortWeekDayString[] = { "Sun", "Mon", "Tue", "Wed", "Thu",  "Fri", "Sat" };
const wchar_t* intToShortWeekDayWideString[] = { L"Sun", L"Mon", L"Tue", L"Wed", L"Thu",  L"Fri", L"Sat" };

// Translates int to Month strings:
const char* intToMonthString[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
const wchar_t* intToMonthWideString[] = { L"January", L"February", L"March", L"April", L"May", L"June", L"July", L"August", L"September", L"October", L"November", L"December" };

// Translates int to Short Month strings:
const char* intToShortMonthString[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
const wchar_t* intToShortMonthWideString[] = { L"Jan", L"Feb", L"Mar", L"Apr", L"May", L"Jun", L"Jul", L"Aug", L"Sep", L"Oct", L"Nov", L"Dec" };

// An intermediate c string buffer size:
#define GT_STRING_BUFF_SIZE 512

int MonthToInt(const gtString& monthStr)
{
    bool found(false);
    int i(0);

    for (; i < 12; i++)
    {
        if (0 == monthStr.compare(intToShortMonthWideString[i]))
        {
            found = true;
            break;
        }
    }

    return (found ? i + 1 : -1);
}

// ---------------------------------------------------------------------------
// Name:        osTime::osTime
// Description: Initialize me to midnight (00:00:00), January 1, 1970, in
//              coordinated universal time (UTC)
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
osTime::osTime()
    : _secondsFrom1970(0)
{
}


// ---------------------------------------------------------------------------
// Name:        osTime::osTime
// Description: Constructor
// Arguments: secondsFrom1970 - The number of seconds elapsed since
//                              midnight (00:00:00), January 1, 1970, in
//                              coordinated universal time (UTC).
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
osTime::osTime(gtInt64 secondsFrom1970)
    : _secondsFrom1970(secondsFrom1970)
{
}


// ---------------------------------------------------------------------------
// Name:        osTime::setFromCurrentTime
// Description: Set me to contain the current time and date.
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
void osTime::setFromCurrentTime()
{
    // Get the current time:
    time_t secondsFrom1970 = 0;
    time(&secondsFrom1970);

    // Set _secondsFrom1970 to contain the current time:
    _secondsFrom1970 = secondsFrom1970;
}


// ---------------------------------------------------------------------------
// Name:        osTime::setTime
// Description: Sets the current time.
// Arguments:
//  timeZone - The time zone in which the input time is given.
//  year - Year [1900, ...]
//  month - Month [1,12]
//  day - Day [1,31]
//  hour - Hour [0-23]
//  minute - Minute [0,59]
//  second - Second [0,59]
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        30/6/2004
// ---------------------------------------------------------------------------
bool osTime::setTime(TimeZone timeZone, int year, int month, int day,
                     int hour, int minute, int second)
{
    bool retVal = false;

    if (timeZone == osTime::LOCAL)
    {
        struct tm newTime;
        newTime.tm_year = year - 1900;
        newTime.tm_mon = month - 1; // tm_mon is Zero based
        newTime.tm_mday = day;
        newTime.tm_hour = hour;
        newTime.tm_min = minute;
        newTime.tm_sec = second;

        // Let C runtime check if "standard time" or "daylight savings" time is in effect:
        newTime.tm_isdst = -1;

        // Convert the input time into a time_t structure:
        time_t newTimeAsTimeT = mktime(&newTime);

        // If the conversion succeeded:
        if (newTimeAsTimeT != -1)
        {
            _secondsFrom1970 = newTimeAsTimeT;
            retVal = true;
        }
    }
    else
    {
        // We currently support only local time zone.
        GT_ASSERT(0);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osTime::setTime
// Description: Sets the time from a given number of seconds elapsed since
//              midnight (00:00:00), January 1, 1970, in coordinated universal
//              time (UTC).
// Author:      AMD Developer Tools Team
// Date:        28/10/2004
// ---------------------------------------------------------------------------
void osTime::setTime(gtInt64 secondsFrom1970)
{
    _secondsFrom1970 = secondsFrom1970;
}


// ---------------------------------------------------------------------------
// Name:        osTime::setFromDate
// Description: Sets the date from a given string into a osTime format
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        29/11/2004
// ---------------------------------------------------------------------------
bool osTime::setFromDate(TimeZone timeZone, const gtString& time, TimeFormat format)
{
    (void)(timeZone); // unused
    bool retVal = false;

    int dateInt = 0;
    int monthInt = 0;
    int yearInt = 0;

    if (format == SLASH_SAPERATOR)
    {
        // Split the date string:
        gtStringTokenizer tokenizer(time, L"/");

        gtString date;
        bool rc1 = tokenizer.getNextToken(date);

        if (rc1)
        {
            rc1 = date.isIntegerNumber();

            if (rc1)
            {
                rc1 = date.toIntNumber(dateInt);

                if (rc1)
                {
                    if ((dateInt < 0) || (dateInt > 31))
                    {
                        rc1 = false;
                    }
                }
            }
        }

        gtString month;
        bool rc2 = tokenizer.getNextToken(month);

        if (rc2)
        {
            rc2 = month.isIntegerNumber();

            if (rc2)
            {
                rc2 = month.toIntNumber(monthInt);

                if (rc2)
                {
                    if ((monthInt < 0) || (monthInt > 12))
                    {
                        rc2 = false;
                    }
                }
            }
        }

        gtString year;
        bool rc3 = tokenizer.getNextToken(year);

        if (rc3)
        {
            rc3 = year.isIntegerNumber();

            if (rc3)
            {
                rc3 = year.toIntNumber(yearInt);

                if (rc3)
                {
                    if ((yearInt < 1970) || (yearInt > 2037))
                    {
                        rc3 = false;
                    }
                }
            }
        }

        if (rc1 && rc2 && rc3)
        {
            retVal = setTime(osTime::LOCAL, yearInt, monthInt, dateInt, 0, 0, 0);
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osTime::setFromDateTimeString
// Description: Sets the date and time from a given string into a osTime format
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        06/02/2014
// ---------------------------------------------------------------------------

bool osTime::setFromDateTimeString(TimeZone timeZone, const gtString& dateTime, TimeFormat format)
{
    (void)(timeZone); // unused
    bool retVal = false;

    int dateInt = 0;
    int monthInt = 0;
    int yearInt = 0;

    int hrInt = 0;
    int minInt = 0;
    int secInt = 0;

    switch (format)
    {
        case NAME_SCHEME_FILE:
        {
            // Split the string into date and time
            // Feb-06-2014_14-17-30
            gtStringTokenizer tokenizer(dateTime, L"_");

            gtString date, time ;

            if (
                (!tokenizer.getNextToken(date)) ||
                (!tokenizer.getNextToken(time))
            )
            {
                // unknown format
                return false;
            }

            gtStringTokenizer dateTok(date, L"-");
            gtString dayStr, monthStr, yearStr;

            if (
                (!dateTok.getNextToken(monthStr)) ||
                (!dateTok.getNextToken(dayStr)) ||
                (!dateTok.getNextToken(yearStr))
            )
            {
                return false;
            }

            if (
                (!dayStr.isIntegerNumber()) ||
                (!dayStr.toIntNumber(dateInt)) ||
                ((dateInt < 0) || (dateInt > 31))
            )
            {
                return  false;
            }

            monthInt =  MonthToInt(monthStr);

            if ((monthInt < 0) || (monthInt > 12))
            {
                return false;
            }

            if (
                (!yearStr.isIntegerNumber()) ||
                (!yearStr.toIntNumber(yearInt)) ||
                ((yearInt < 1970) || (yearInt > 2037))
            )
            {
                return false;
            }

            gtStringTokenizer timeTok(time, L"-");

            gtString hrStr, minStr, secStr;

            if (
                (!timeTok.getNextToken(hrStr)) ||
                (!timeTok.getNextToken(minStr)) ||
                (!timeTok.getNextToken(secStr))
            )
            {
                return false;
            }

            if (
                (!hrStr.isIntegerNumber()) ||
                (!hrStr.toIntNumber(hrInt)) ||
                ((hrInt < 0) || (hrInt > 24))
            )
            {
                return  false;
            }

            if (
                (!minStr.isIntegerNumber()) ||
                (!minStr.toIntNumber(minInt)) ||
                ((minInt < 0) || (minInt > 60))
            )
            {
                return false;
            }

            if (
                (!secStr.isIntegerNumber()) ||
                (!secStr.toIntNumber(secInt)) ||
                ((secInt < 0) || (secInt > 60))
            )
            {
                return false;
            }


            retVal = setTime(osTime::LOCAL, yearInt, monthInt, dateInt, hrInt, minInt, secInt);
            break;
        }

        case WINDOWS_STYLE:
        case UNIX_STYLE:
        case UNDERSCORE_SAPERATOR:
        case SLASH_SAPERATOR:
        case FOR_EMAIL:
        case NAME_SCHEME_DISPLAY:
        case NAME_SCHEME_SHORT_FILE:
        case DATE_TIME_DISPLAY:
            return false;
    }

    GT_ASSERT(retVal);
    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        osTime::setFromFileCompilationDateMacro
// Description: Sets the date from a given Compilation date macro __DATE__
//              into a osTime format
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        30/11/2004
// ---------------------------------------------------------------------------
bool osTime::setFromFileCompilationDateMacro(const wchar_t* dateMacro)
{
    bool retVal = false;

    int dateInt = 0;
    int monthInt = 0;
    int yearInt = 0;

    // Split the date macro:
    gtStringTokenizer tokenizer(dateMacro, L" ");

    bool rc4 = false;
    gtString month;
    bool rc1 = tokenizer.getNextToken(month);

    if (rc1)
    {
        for (int i = 0; i <= 11; i++)
        {
            if (intToShortMonthWideString[i] == month)
            {
                monthInt = i + 1;
                rc4 = true;
                break;
            }
        }
    }

    gtString date;
    bool rc2 = tokenizer.getNextToken(date);

    if (rc2)
    {
        rc2 = date.isIntegerNumber();

        if (rc2)
        {
            rc2 = date.toIntNumber(dateInt);

            if (rc2)
            {
                if ((dateInt < 0) || (dateInt > 31))
                {
                    rc2 = false;
                }
            }
        }
    }

    gtString year;
    bool rc3 = tokenizer.getNextToken(year);

    if (rc3)
    {
        rc3 = year.isIntegerNumber();

        if (rc3)
        {
            rc3 = year.toIntNumber(yearInt);

            if (rc3)
            {
                if ((yearInt < 1970) || (yearInt > 2037))
                {
                    rc3 = false;
                }
            }
        }
    }

    if (rc1 && rc2 && rc3 && rc4)
    {
        retVal = setTime(osTime::LOCAL, yearInt, monthInt, dateInt, 0, 0, 0);
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osTime::setFromFileCompilationDateMacro
// Description: char* version of  setFromFileCompilationDateMacro
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        20/6/2011
// ---------------------------------------------------------------------------
bool osTime::setFromFileCompilationDateMacro(const char* dateMacro)
{
    gtString dateMacroStr;
    dateMacroStr.fromASCIIString(dateMacro);

    return setFromFileCompilationDateMacro(dateMacroStr.asCharArray());
}

// ---------------------------------------------------------------------------
// Name:        osTime::setFromSecondsFrom1970String
// Description: Set the date and time from a string that contains 1 integer
//              number: the number of seconds elapsed since midnight (00:00:00),
//              January 1, 1970, in coordinated universal time (UTC).
//              (Such a string may be generated by osTime::secondsFrom1970AsString.
//
// Arguments: secondsString - The input string.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        23/8/2006
// ---------------------------------------------------------------------------
bool osTime::setFromSecondsFrom1970String(const gtString& secondsString)
{
    bool retVal = false;

    long long readValue = 0;
    int amountOfFieldsRead = swscanf(secondsString.asCharArray(), L"%lld", &readValue);
    GT_IF_WITH_ASSERT(amountOfFieldsRead == 1)
    {
        _secondsFrom1970 = readValue;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osTime::dateAsString
// Description: Returns my date as a string.
// Arguments:   dateString - Will get the current date.
//              stringFormat - The format in which the string will be written.
//                             TO_DO: Write the formats ...
//              timeZone - The time zone in which the time will be expressed.
//                         (See osTime::TimeZone)
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
void osTime::dateAsString(gtString& dateString, TimeFormat stringFormat, TimeZone timeZone) const
{
    dateString.makeEmpty();

    // Will get the time represented by this class:
    struct tm timeAtTimeZone;

    // Get the time according to the time zone:
    timeAsTmStruct(timeAtTimeZone, timeZone);

    // Get the date as string:
    wchar_t dateAsString[GT_STRING_BUFF_SIZE];

    switch (stringFormat)
    {
        case UNDERSCORE_SAPERATOR:
        {
            swprintf(dateAsString, GT_STRING_BUFF_SIZE, L"%ls_%02d_%ls_%d",
                     intToWeekDayWideString[timeAtTimeZone.tm_wday],
                     timeAtTimeZone.tm_mday, intToMonthWideString[timeAtTimeZone.tm_mon],
                     (timeAtTimeZone.tm_year + 1900));

            dateString.append(dateAsString);
        }
        break;

        case WINDOWS_STYLE:
        {
            swprintf(dateAsString, GT_STRING_BUFF_SIZE, L"%ls, %ls %d, %d",
                     intToWeekDayWideString[timeAtTimeZone.tm_wday],
                     intToMonthWideString[timeAtTimeZone.tm_mon],
                     timeAtTimeZone.tm_mday,
                     (timeAtTimeZone.tm_year + 1900));

            dateString.append(dateAsString);
        }
        break;

        case UNIX_STYLE:
        {
            // TO_DO: Not implemented yet !!!
            GT_ASSERT(0);
        }
        break;

        case FOR_EMAIL:
        {
            swprintf(dateAsString, GT_STRING_BUFF_SIZE, L"%ls, %d %ls %d %02d:%02d:%02d",
                     intToShortWeekDayWideString[timeAtTimeZone.tm_wday],
                     timeAtTimeZone.tm_mday,
                     intToShortMonthWideString[timeAtTimeZone.tm_mon],
                     (timeAtTimeZone.tm_year + 1900),
                     timeAtTimeZone.tm_hour,
                     timeAtTimeZone.tm_min,
                     timeAtTimeZone.tm_sec);

            dateString.append(dateAsString);
        }
        break;

        case SLASH_SAPERATOR:
            swprintf(dateAsString, GT_STRING_BUFF_SIZE, L"%d/%d/%d",
                     timeAtTimeZone.tm_mday,
                     timeAtTimeZone.tm_mon + 1, // tm_mon is Zero based
                     (timeAtTimeZone.tm_year + 1900));

            dateString.append(dateAsString);
            break;

        case NAME_SCHEME_DISPLAY:
        {
            swprintf(dateAsString, GT_STRING_BUFF_SIZE, L"%ls %02d, %4d %02d:%02d:%02d",
                     intToShortMonthWideString[timeAtTimeZone.tm_mon],
                     timeAtTimeZone.tm_mday, (timeAtTimeZone.tm_year + 1900),
                     timeAtTimeZone.tm_hour,
                     timeAtTimeZone.tm_min,
                     timeAtTimeZone.tm_sec);

            dateString.append(dateAsString);
        }
        break;

        case NAME_SCHEME_FILE:
        {
            swprintf(dateAsString, GT_STRING_BUFF_SIZE, L"%ls-%02d-%4d_%02d-%02d-%02d",
                     intToShortMonthWideString[timeAtTimeZone.tm_mon],
                     timeAtTimeZone.tm_mday, (timeAtTimeZone.tm_year + 1900),
                     timeAtTimeZone.tm_hour,
                     timeAtTimeZone.tm_min,
                     timeAtTimeZone.tm_sec);

            dateString.append(dateAsString);
        }
        break;

        case NAME_SCHEME_SHORT_FILE:
        {
            swprintf(dateAsString, GT_STRING_BUFF_SIZE, L"%ls-%02d-%4d_%02d-%02d",
                intToShortMonthWideString[timeAtTimeZone.tm_mon],
                timeAtTimeZone.tm_mday, (timeAtTimeZone.tm_year + 1900),
                timeAtTimeZone.tm_hour,
                timeAtTimeZone.tm_min);

            dateString.append(dateAsString);
        }
        break;

        default:
        {
            // Unknown time zone - Issue an assert and get time string as UNDERSCORE_SAPERATOR:
            GT_ASSERT(0);
        }
        break;
    }
}


// ---------------------------------------------------------------------------
// Name:        osTime::dateAsString
// Description: Returns my date as a string. (ASCII)
// Arguments:   dateString - Will get the current date.
//              stringFormat - The format in which the string will be written.
//                             TO_DO: Write the formats ...
//              timeZone - The time zone in which the time will be expressed.
//                         (See osTime::TimeZone)
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
void osTime::dateAsString(gtASCIIString& dateString, TimeFormat stringFormat, TimeZone timeZone) const
{
    dateString.makeEmpty();

    // Will get the time represented by this class:
    struct tm timeAtTimeZone;

    // Get the time according to the time zone:
    timeAsTmStruct(timeAtTimeZone, timeZone);

    // Get the date as string:
    char dateAsString[GT_STRING_BUFF_SIZE];

    switch (stringFormat)
    {
        case UNDERSCORE_SAPERATOR:
        {
            sprintf(dateAsString, "%s_%02d_%s_%d",
                    intToWeekDayString[timeAtTimeZone.tm_wday],
                    timeAtTimeZone.tm_mday, intToMonthString[timeAtTimeZone.tm_mon],
                    (timeAtTimeZone.tm_year + 1900));

            dateString.append(dateAsString);
        }
        break;

        case WINDOWS_STYLE:
        {
            sprintf(dateAsString, "%s, %s %d, %d",
                    intToWeekDayString[timeAtTimeZone.tm_wday],
                    intToMonthString[timeAtTimeZone.tm_mon],
                    timeAtTimeZone.tm_mday,
                    (timeAtTimeZone.tm_year + 1900));

            dateString.append(dateAsString);
        }
        break;

        case UNIX_STYLE:
        {
            // TO_DO: Not implemented yet !!!
            GT_ASSERT(0);
        }
        break;

        case FOR_EMAIL:
        {
            sprintf(dateAsString, "%s, %d %s %d %02d:%02d:%02d",
                    intToShortWeekDayString[timeAtTimeZone.tm_wday],
                    timeAtTimeZone.tm_mday,
                    intToShortMonthString[timeAtTimeZone.tm_mon],
                    (timeAtTimeZone.tm_year + 1900),
                    timeAtTimeZone.tm_hour,
                    timeAtTimeZone.tm_min,
                    timeAtTimeZone.tm_sec);

            dateString.append(dateAsString);
        }
        break;

        case SLASH_SAPERATOR:
            sprintf(dateAsString, "%d/%d/%d",
                    timeAtTimeZone.tm_mday,
                    timeAtTimeZone.tm_mon + 1, // tm_mon is Zero based
                    (timeAtTimeZone.tm_year + 1900));

            dateString.append(dateAsString);
            break;

        case NAME_SCHEME_DISPLAY:
        {
            sprintf(dateAsString, "L%s %02d, %4d %02d:%02d:%02d",
                    intToShortMonthString[timeAtTimeZone.tm_mon],
                    timeAtTimeZone.tm_mday, (timeAtTimeZone.tm_year + 1900),
                    timeAtTimeZone.tm_hour,
                    timeAtTimeZone.tm_min,
                    timeAtTimeZone.tm_sec);

            dateString.append(dateAsString);
        }
        break;

        case NAME_SCHEME_FILE:
        {
            sprintf(dateAsString, "%s-%02d-%4d_%02d-%02d-%02d",
                    intToShortMonthString[timeAtTimeZone.tm_mon],
                    timeAtTimeZone.tm_mday, (timeAtTimeZone.tm_year + 1900),
                    timeAtTimeZone.tm_hour,
                    timeAtTimeZone.tm_min,
                    timeAtTimeZone.tm_sec);

            dateString.append(dateAsString);
        }
        break;

        default:
        {
            // Unknown time zone - Issue an assert and get time string as UNDERSCORE_SAPERATOR:
            GT_ASSERT(0);
        }
        break;
    }
}


// ---------------------------------------------------------------------------
// Name:        osTime::timeAsString
// Description:
// Arguments:   timeString - Will get the current time.
//              stringFormat - The format in which the string will be written.
//                             TO_DO: Write the formats ...
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
void osTime::timeAsString(gtString& timeString, TimeFormat stringFormat, TimeZone timeZone) const
{
    timeString.makeEmpty();

    // Will get the time represented by this class:
    struct tm timeAtTimeZone;

    // Get the time according to the time zone:
    timeAsTmStruct(timeAtTimeZone, timeZone);

    // Get the date as string:
    wchar_t timeAsString[GT_STRING_BUFF_SIZE];

    // Null terminate the C string. This is a precaution against the scenario
    // where a case below fails to initialize the C-string.
    timeAsString[0] = 0;

    switch (stringFormat)
    {
        case UNDERSCORE_SAPERATOR:
        {
            swprintf(timeAsString, GT_STRING_BUFF_SIZE, L"%02d_%02d_%02d",
                     timeAtTimeZone.tm_hour,
                     timeAtTimeZone.tm_min,
                     timeAtTimeZone.tm_sec);
        }
        break;

        case WINDOWS_STYLE:
        {
            swprintf(timeAsString, GT_STRING_BUFF_SIZE, L"%02d:%02d:%02d",
                     timeAtTimeZone.tm_hour,
                     timeAtTimeZone.tm_min,
                     timeAtTimeZone.tm_sec);
        }
        break;

        case DATE_TIME_DISPLAY:
        {
            swprintf(timeAsString, GT_STRING_BUFF_SIZE, L"%04d.%02d.%02d\t%02d:%02d:%02d",
                     timeAtTimeZone.tm_year + 1900,
                     timeAtTimeZone.tm_mon + 1,
                     timeAtTimeZone.tm_mday,
                     timeAtTimeZone.tm_hour,
                     timeAtTimeZone.tm_min,
                     timeAtTimeZone.tm_sec);
        }
        break;

        case UNIX_STYLE:
        {
            // TO_DO: Not implemented yet !!!
            GT_ASSERT(0);
        }
        break;

        case FOR_EMAIL:
        {
            // This setting is only to be used in date as string function
            GT_ASSERT(0);
        }
        break;

        case NAME_SCHEME_DISPLAY:
        {
            // This setting is only to be used in date as string function
            GT_ASSERT(0);
        }
        break;

        case NAME_SCHEME_FILE:
        {
            // This setting is only to be used in date as string function
            GT_ASSERT(0);
        }
        break;

        default:
        {
            // Unknown time zone - Issue an assert and get time string as UNDERSCORE_SAPERATOR:
            GT_ASSERT(0);
        }
        break;
    }

    // Copy the time string to the output parameter
    timeString.append(timeAsString);
}


// ---------------------------------------------------------------------------
// Name:        osTime::timeAsTmStruct
// Description: Translate my time into a struct tm.
// Arguments:   tmStruct - Will get my time.
//              timeZone - The time zone in which the time will be expressed.
//                         (See osTime::TimeZone)
// Author:      AMD Developer Tools Team
// Date:        17/5/2003
// ---------------------------------------------------------------------------
void osTime::timeAsTmStruct(struct tm& tmStruct, TimeZone timeZone) const
{
    // Get the time according to the time zone:
    switch (timeZone)
    {
        case UTC:
        {
            // Get time as UTC time:
            time_t secondsFrom1970 = _secondsFrom1970;
            tmStruct = *(gmtime(&secondsFrom1970));
        }
        break;

        case LOCAL:
        {
            // Convert from UTC to local time:
            time_t secondsFrom1970 = _secondsFrom1970;
            tmStruct = *(localtime(&secondsFrom1970));
        }
        break;

        default:
        {
            // Unknown time zone - Issue an assert and get time as UTC time:
            GT_ASSERT(0);
        }
        break;
    }
}


// ---------------------------------------------------------------------------
// Name:        osTime::secondsFrom1970AsString
// Description: Output a string that contains 1 integer number: the number
//              of seconds elapsed since midnight (00:00:00), January 1, 1970,
//              in coordinated universal time (UTC).
// Arguments: secondsString - Will get the output string.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        22/8/2006
// ---------------------------------------------------------------------------
bool osTime::secondsFrom1970AsString(gtString& secondsString) const
{
    bool retVal = false;

    wchar_t buff[250];
    long long secondsFrom1970 = _secondsFrom1970;
    int amountOfFieldsWrittern = swprintf(buff, 250, L"%lld", secondsFrom1970);
    GT_IF_WITH_ASSERT(1 < amountOfFieldsWrittern)
    {
        secondsString = buff;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osTime::secondsFrom1970AsString
// Description: Output a string that contains 1 integer number: the number
//              of seconds elapsed since midnight (00:00:00), January 1, 1970,
//              in coordinated universal time (UTC). - ASCII version
// Arguments:   secondsString - Will get the output string.
// Return Val:  bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        13/9/2010
// ---------------------------------------------------------------------------
bool osTime::secondsFrom1970AsString(gtASCIIString& secondsString) const
{
    bool retVal = false;

    char buff[250];
    long long secondsFrom1970 = _secondsFrom1970;
    int amountOfFieldsWrittern = sprintf(buff, "%lld", secondsFrom1970);
    GT_IF_WITH_ASSERT(1 < amountOfFieldsWrittern)
    {
        secondsString = buff;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osTime::operator<
// Description: Data and time comparison operator.
// Arguments:   other - Other time to which I am compared to.
// Return Val:  bool - true iff my date & time is earlier than other date & time.
// Author:      AMD Developer Tools Team
// Date:        30/6/2004
// ---------------------------------------------------------------------------
bool osTime::operator<(const osTime& other) const
{
    return (_secondsFrom1970 < other._secondsFrom1970);
}


// ---------------------------------------------------------------------------
// Name:        osTime::operator>
// Description: Data and time comparison operator.
// Arguments:   other - Other time to which I am compared to.
// Return Val:  bool - true iff other date & time is earlier than my date & time.
// Author:      AMD Developer Tools Team
// Date:        30/6/2004
// ---------------------------------------------------------------------------
bool osTime::operator>(const osTime& other) const
{
    return (_secondsFrom1970 > other._secondsFrom1970);
}


// ---------------------------------------------------------------------------
// Name:        osTime::operator<
// Description: Data and time comparison operator.
// Arguments:   other - Other time to which I am compared to.
// Return Val:  bool - true iff I represent exactly the same time as other.
// Author:      AMD Developer Tools Team
// Date:        30/6/2004
// ---------------------------------------------------------------------------
bool osTime::operator==(const osTime& other) const
{
    return (_secondsFrom1970 == other._secondsFrom1970);
}

// ---------------------------------------------------------------------------
// Name:        currentTimeAsString
// Description: Get the current time in string format
// Arguments:   strTime         - output string
//              stringFormat    - the format that will applied to the string
//              timeZone        - the timezone that will be used for the string representation
// Return Val:  void
// Author:      Doron Ofek
// Date:        Dec-24, 2015
// ---------------------------------------------------------------------------
void osTime::currentTimeAsString(gtString& strTime, TimeFormat stringFormat, TimeZone timeZone)
{
    osTime currentTime;
    currentTime.setFromCurrentTime();
    currentTime.timeAsString(strTime, stringFormat, timeZone);
}

// ---------------------------------------------------------------------------
// Name:        osTimeValFromMilliseconds
// Description:
//   Converts time given as milliseconds to a timeval structure.
//
// Arguments: timeAsMsec - The input time interval, given in milliseconds.
//            timeAsTimeVal - Will get the time interval.
//
// Author:      AMD Developer Tools Team
// Date:        3/1/2008
// ---------------------------------------------------------------------------
void osTimeValFromMilliseconds(long timeAsMsec, struct timeval& timeAsTimeVal)
{
    // Divide the time we got, measured in milliseconds to second and miliseconds reminder:
    long seconds = timeAsMsec / 1000L;
    long miliseconds = timeAsMsec % 1000L;

    // Output the time we got as second and microseconds (microsecond = 1/1,000,000 of a second):
    timeAsTimeVal.tv_sec = seconds;
    timeAsTimeVal.tv_usec = miliseconds * 1000;
}

