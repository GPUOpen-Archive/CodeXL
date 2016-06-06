//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osStopWatch.cpp
///
//=====================================================================

//------------------------------ osStopWatch.cpp ------------------------------

// Standard C:
#include "Limits.h"
#include "math.h"

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osStopWatch.h>


// ---------------------------------------------------------------------------
// Name:        osStopWatch::osStopWatch
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        28/2/2005
// ---------------------------------------------------------------------------
osStopWatch::osStopWatch()
    : _timeInterval(0.0), _pastRunsTimeInterval(0.0), _isRunning(false)
{
    // Initialize _startMeasureTime:
    _startMeasureTime = 0;

    // Query the system performance counter frequency:
    LARGE_INTEGER perfCounterFrequency;
    BOOL rc = ::QueryPerformanceFrequency(&perfCounterFrequency);
    GT_ASSERT(rc != FALSE);

    // Translate it to an (1 / amount of time units per second) resolution:
    // (The low part of the large integer is an unsigned long, therefore, the high
    //  part, that represents the bits that are more significant than it should be
    //  multiplied by ULONG_MAX):
    _stopWatchResolution = (double)perfCounterFrequency.HighPart * (double)ULONG_MAX;
    _stopWatchResolution += (double)perfCounterFrequency.LowPart;
    _stopWatchResolution = 1.0 / _stopWatchResolution;
}


// ---------------------------------------------------------------------------
// Name:        osStopWatch::~osStopWatch
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        28/2/2005
// ---------------------------------------------------------------------------
osStopWatch::~osStopWatch()
{
}


// ---------------------------------------------------------------------------
// Name:        osStopWatch::start
// Description: Starts / restarts the stop-watch run.
// Author:      AMD Developer Tools Team
// Date:        28/2/2005
// ---------------------------------------------------------------------------
bool osStopWatch::start()
{
    bool retVal = false;

    // Store the start measure time:
    LARGE_INTEGER startMeasureTimeAsLI;
    BOOL rc = ::QueryPerformanceCounter(&startMeasureTimeAsLI);
    GT_IF_WITH_ASSERT(rc != FALSE)
    {
        _startMeasureTime = startMeasureTimeAsLI.QuadPart;

        _isRunning = true;
        retVal = true;
    }

    // "Start" resets the Pause / Resume mechanism:
    _pastRunsTimeInterval = 0.0;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osStopWatch::stop
// Description: Stops the stop-watch run.
// Author:      AMD Developer Tools Team
// Date:        28/2/2005
// ---------------------------------------------------------------------------
bool osStopWatch::stop()
{
    // Calculate the time interval passed since this stop-watch was activated:
    bool retVal = calculateTimeInterval(_timeInterval);
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
    // Calculate the time interval passed since this stop-watch was activated:
    bool retVal = calculateTimeInterval(_pastRunsTimeInterval);
    _timeInterval = _pastRunsTimeInterval;
    // Mark that the stop-watch stopped:
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
    bool retVal = false;

    // Store the start measure time:
    LARGE_INTEGER startMeasureTimeAsLI;
    BOOL rc = ::QueryPerformanceCounter(&startMeasureTimeAsLI);
    GT_IF_WITH_ASSERT(rc != FALSE)
    {
        _startMeasureTime = startMeasureTimeAsLI.QuadPart;

        _isRunning = true;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osStopWatch::isRunning
// Description: Returns true iff the stop-watch is running.
// Author:      AMD Developer Tools Team
// Date:        28/2/2005
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
// Date:        28/2/2005
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
    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        osStopWatch::calculateTimeInterval
// Description: Calculates and returns the time passed since this stop-watch
//               was started (in seconds).
// Author:      AMD Developer Tools Team

// Date:        28/2/2005
// ---------------------------------------------------------------------------
bool osStopWatch::calculateTimeInterval(double& timeInterval) const
{
    bool retVal = false;
    timeInterval = 0.0;

    // Get the current time:
    LARGE_INTEGER currentTimeAsLI;
    BOOL rc = ::QueryPerformanceCounter(&currentTimeAsLI);
    GT_IF_WITH_ASSERT(rc != FALSE)
    {
        // Calculate the time interval between the start measure time and the current time:
        gtUInt64 currentTime = currentTimeAsLI.QuadPart;

        timeInterval = (double)(currentTime - _startMeasureTime);
        timeInterval *= _stopWatchResolution;

        // Add the time from previous splits:
        timeInterval += _pastRunsTimeInterval;

        retVal = true;
    }

    GT_RETURN_WITH_ASSERT(retVal);
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
    LARGE_INTEGER startMeasureTimeAsLI;
    BOOL rc = ::QueryPerformanceCounter(&startMeasureTimeAsLI);
    GT_IF_WITH_ASSERT(rc != FALSE)
    {
        // Print only the last 4 digits:
        timeStr.appendFormattedString(L"%llu", (gtUInt64)startMeasureTimeAsLI.QuadPart);
    }
}

bool osGetCurrentTime(gtUInt64& currentTimeAsMilliseconds)
{
    bool retVal = false;

    LARGE_INTEGER startMeasureTimeAsLI;
    BOOL rc = ::QueryPerformanceCounter(&startMeasureTimeAsLI);
    GT_IF_WITH_ASSERT(rc != FALSE)
    {
        retVal = true;
        currentTimeAsMilliseconds = (gtUInt64)startMeasureTimeAsLI.QuadPart;
    }
    return retVal;
}