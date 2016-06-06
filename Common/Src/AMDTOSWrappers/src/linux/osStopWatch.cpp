//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osStopWatch.cpp
///
//=====================================================================

//------------------------------ osStopWatch.cpp ------------------------------

// POSIX:
#include <sys/time.h>
#include <unistd.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osStopWatch.h>


// ---------------------------------------------------------------------------
// Name:        osStopWatch::osGetCurrentTime
// Description: Returns the current time.
// Arguments:   currentTimeAsMilliseconds - Will get the current time, as milliseconds
//              elapsed from 00:00:00 UTC on January 1, 1970.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        7/11/2006
// ---------------------------------------------------------------------------
bool osGetCurrentTime(gtUInt64& currentTimeAsMilliseconds)
{
    bool retVal = false;

    currentTimeAsMilliseconds = 0;

    // Get the current time:
    struct timeval currentTime;
    int rc = ::gettimeofday(&currentTime, NULL);
    GT_IF_WITH_ASSERT(rc == 0)
    {
        // Translate the amount of seconds to milliseconds:
        currentTimeAsMilliseconds = currentTime.tv_sec * 1000;

        // Add the amount of microseconds, translated to milliseconds:
        currentTimeAsMilliseconds += (currentTime.tv_usec / 1000);

        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osStopWatch::osStopWatch
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        7/11/2006
// ---------------------------------------------------------------------------
osStopWatch::osStopWatch()
    : _stopWatchResolution(0), _startMeasureTime(0), _timeInterval(0.0), _pastRunsTimeInterval(0.0), _isRunning(false)
{
}


// ---------------------------------------------------------------------------
// Name:        osStopWatch::~osStopWatch
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        7/11/2006
// ---------------------------------------------------------------------------
osStopWatch::~osStopWatch()
{
}


// ---------------------------------------------------------------------------
// Name:        osStopWatch::start
// Description: Starts / restarts the stop-watch run.
// Author:      AMD Developer Tools Team
// Return Val:  bool - Success / failure.
// Date:        7/11/2006
// ---------------------------------------------------------------------------
bool osStopWatch::start()
{
    _isRunning = false;

    // Initialize _startMeasureTime to contain the current time:
    bool rc = osGetCurrentTime(_startMeasureTime);
    GT_IF_WITH_ASSERT(rc)
    {
        _isRunning = true;
    }

    // "Start" resets the Pause / Resume mechanism:
    _pastRunsTimeInterval = 0.0;

    return _isRunning;
}


// ---------------------------------------------------------------------------
// Name:        osStopWatch::stop
// Description: Stops the stop-watch run.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        7/11/2006
// ---------------------------------------------------------------------------
bool osStopWatch::stop()
{
    bool retVal = false;

    // Calculate the time interval passed since this stop-watch was activated:
    bool rc2 = calculateTimeInterval(_timeInterval);
    GT_IF_WITH_ASSERT(rc2)
    {
        retVal = true;
    }

    // Mark that the stop-watch stopped:
    _isRunning = false;

    // "Stop" resets the Pause / Resume mechanism:
    _pastRunsTimeInterval = 0.0;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osStopWatch::pause
// Description: "Pauses" the stop watch - storing its time for later use
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        7/12/2009
// ---------------------------------------------------------------------------
bool osStopWatch::pause()
{
    bool retVal = false;

    // Calculate the time interval passed since this stop-watch was activated:
    bool rc2 = calculateTimeInterval(_pastRunsTimeInterval);
    GT_IF_WITH_ASSERT(rc2)
    {
        _timeInterval = _pastRunsTimeInterval;
        retVal = true;
    }

    // Mark that the stop-watch paused:
    _isRunning = false;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osStopWatch::resume
// Description: Resumes the stop watch after it was paused
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        7/12/2009
// ---------------------------------------------------------------------------
bool osStopWatch::resume()
{
    _isRunning = false;

    // Initialize _startMeasureTime to contain the current time:
    bool rc = osGetCurrentTime(_startMeasureTime);
    GT_IF_WITH_ASSERT(rc)
    {
        _isRunning = true;
    }

    return _isRunning;

}


// ---------------------------------------------------------------------------
// Name:        osStopWatch::isRunning
// Description: Returns true iff the stop-watch is running.
// Author:      AMD Developer Tools Team
// Date:        7/11/2006
// ---------------------------------------------------------------------------
bool osStopWatch::isRunning() const
{
    return _isRunning;
}


// ---------------------------------------------------------------------------
// Name:        osStopWatch::getTimeInterval
// Description: Returns the time interval that this stop watch measured.
// Arguments: timeInterval - Will get the time interval, measured in milliseconds.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        7/11/2006
// ---------------------------------------------------------------------------
bool osStopWatch::getTimeInterval(double& timeInterval) const
{
    bool retVal = false;

    timeInterval = 0.0;

    // If the stop watch is still running:
    if (_isRunning)
    {
        // Calculate the time interval passed since this stop-watch was activated:
        retVal = calculateTimeInterval(timeInterval);
    }
    else
    {
        // No need to calculate the time interval, since the call to stop() already
        // calculated it.
        timeInterval = _timeInterval;
        retVal = true;
    }

    // Output the measured time interval:
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osStopWatch::calculateTimeInterval
// Description: Calculates and returns the time passed since this stop-watch
//              was started (in seconds).
// Arguments: timeInterval - Will get the calculated time interval, measured
//                           in seconds.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        7/11/2006
// ---------------------------------------------------------------------------
bool osStopWatch::calculateTimeInterval(double& timeInterval) const
{
    bool retVal = false;

    timeInterval = 0.0;

    // Get the current time:
    gtUInt64 currentTime;
    bool rc = osGetCurrentTime(currentTime);
    GT_IF_WITH_ASSERT(rc)
    {
        // Calculate the time interval between the start measure time and the current time:
        timeInterval = (currentTime - _startMeasureTime) / 1000.0;

        // Add the time from previous splits:
        timeInterval += _pastRunsTimeInterval;
        retVal = true;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        osStopWatch::appendCurrentTimeAsString
// Description: Get the current time milliseconds count, as string.
// Arguments:   gtString& timeStr
// Return Val:  void
// Author:      AMD Developer Tools Team
// Date:        18/8/2010
// ---------------------------------------------------------------------------
void osStopWatch::appendCurrentTimeAsString(gtString& timeStr)
{
    // Get the time:
    gtUInt64 time;
    bool rc = osGetCurrentTime(time);
    GT_IF_WITH_ASSERT(rc)
    {
        // Print the time stamp:
        timeStr.appendFormattedString(L"%llu", time);
    }
}
