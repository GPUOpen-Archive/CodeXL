//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTimeInterval.cpp
///
//=====================================================================

// Local:
#include <AMDTOSWrappers/Include/osTimeInterval.h>

#define NANOSECONDS_IN_SINGLE_MILLISECOND 1000000.0
#define NANOSECONDS_IN_SINGLE_SECOND   1000000000.0

osTimeInterval::osTimeInterval()
{
    m_timeIntervalNanoSeconds = 0;
}

osTimeInterval::osTimeInterval(const gtUInt64& intervalInNanoSeconds)
{
    m_timeIntervalNanoSeconds = intervalInNanoSeconds;
}

void osTimeInterval::setAsMilliSeconds(const double& intervalInMilliSeconds)
{
    m_timeIntervalNanoSeconds = static_cast<gtUInt64>(intervalInMilliSeconds * NANOSECONDS_IN_SINGLE_MILLISECOND);
}

void osTimeInterval::getAsMilliSeconds(double& intervalInMilliSeconds) const
{
    intervalInMilliSeconds = static_cast<double>(m_timeIntervalNanoSeconds) / NANOSECONDS_IN_SINGLE_MILLISECOND;
}

void osTimeInterval::getAsWholeSecondsAndRemainder(gtUInt64& seconds, gtUInt64& nanoSecondsRemainder) const
{
    seconds = static_cast<gtUInt64>(m_timeIntervalNanoSeconds / NANOSECONDS_IN_SINGLE_SECOND);
    nanoSecondsRemainder = m_timeIntervalNanoSeconds - (seconds * static_cast<gtUInt64>(NANOSECONDS_IN_SINGLE_SECOND));
}

gtString osTimeInterval::AsString() const
{
    gtUInt64 timeInterval = m_timeIntervalNanoSeconds;
    gtUInt64 microseconds = timeInterval % 1000;
    timeInterval -= microseconds;
    gtUInt64 milliseconds = timeInterval % 1000000;
    timeInterval -= milliseconds;
    gtUInt64 seconds = timeInterval % 1000000000;
    timeInterval -= seconds;
    gtUInt64 minutes = timeInterval % (1000000000LL * 60L);
    timeInterval -= minutes;
    gtUInt64 hours = timeInterval / (1000000000LL * 60L * 60L);

    // HH:MM:SS:mmm.uuu
    gtString retVal;
    retVal.appendFormattedString(L"%0.2d:%0.2d:%0.2d:%0.3d.%0.3d", hours, minutes, seconds, milliseconds, microseconds);

    return retVal;
}
