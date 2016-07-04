//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTime.h
///
//=====================================================================

//------------------------------ osTime.h ------------------------------

#ifndef __OSTIME_H
#define __OSTIME_H

// Forward decelerations:
struct timeval;
class gtString;
class gtASCIIString;

// Local:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           osTime
// General Description: Represents an absolute data and time.
// Author:      AMD Developer Tools Team
// Creation Date:        17/5/2003
// ----------------------------------------------------------------------------------
class OS_API osTime
{
public:
    enum TimeFormat
    {
        WINDOWS_STYLE,
        UNIX_STYLE,
        UNDERSCORE_SAPERATOR,
        SLASH_SAPERATOR,
        FOR_EMAIL,
        NAME_SCHEME_DISPLAY,
        NAME_SCHEME_FILE,
        NAME_SCHEME_SHORT_FILE,
        DATE_TIME_DISPLAY
    };

    enum TimeZone
    {
        UTC,
        LOCAL,
    };

    osTime();
    osTime(gtInt64 secondsFrom1970);

    void setFromCurrentTime();
    bool setTime(TimeZone timeZone, int year, int month, int day, int hour, int minute, int second);
    void setTime(gtInt64 secondsFrom1970);

    bool setFromDate(TimeZone, const gtString& time, TimeFormat format);
    bool setFromDateTimeString(TimeZone, const gtString& time, TimeFormat format);
    bool setFromFileCompilationDateMacro(const wchar_t* dateMacro);
    bool setFromFileCompilationDateMacro(const char* dateMacro);
    bool setFromSecondsFrom1970String(const gtString& secondsString);

    void dateAsString(gtString& dateString, TimeFormat stringFormat, TimeZone timeZone) const;
    void dateAsString(gtASCIIString& dateString, TimeFormat stringFormat, TimeZone timeZone) const;
    void timeAsString(gtString& timeString, TimeFormat stringFormat, TimeZone timeZone) const;
    void timeAsTmStruct(struct tm& tmStruct, TimeZone timeZone) const;
    gtInt64 secondsFrom1970() const { return _secondsFrom1970; };
    bool secondsFrom1970AsString(gtString& secondsString) const;
    bool secondsFrom1970AsString(gtASCIIString& secondsString) const;

    void addSeconds(long secondsToBeAdded) { _secondsFrom1970 += secondsToBeAdded; };

    bool operator<(const osTime& other) const;
    bool operator>(const osTime& other) const;
    bool operator==(const osTime& other) const;
    bool operator!=(const osTime& other) const { return !(operator==(other)); };

    /// Get the current time in string format
    static void currentTimeAsString(gtString& currentTime, TimeFormat stringFormat, TimeZone timeZone);

    /// Get the current local time in string format, including milliseconds
    static bool currentPreciseTimeAsString(gtString& currentTime, TimeFormat stringFormat);

private:
    // Contains the number of seconds elapsed since midnight (00:00:00), January 1, 1970,
    // in coordinated universal time (UTC).
    // Notice that this number can also be negative, expressing time that occur before 1970.
    gtInt64 _secondsFrom1970;
};

void OS_API osTimeValFromMilliseconds(long timeAsMsec, struct timeval& timeAsTimeVal);


#endif //__OSTIME_H

