//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osStopWatch.h
///
//=====================================================================

//------------------------------ osStopWatch.h ------------------------------

#ifndef __OSSTOPWATCH
#define __OSSTOPWATCH

// Local:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>


// ----------------------------------------------------------------------------------
// Class Name:           osStopWatch
//
// General Description:
//   A stop watch that measures the amount of time spent since it was last started.
//
// Author:      AMD Developer Tools Team
// Creation Date:        29/3/2004
// ----------------------------------------------------------------------------------
class OS_API osStopWatch
{
public:
    osStopWatch();
    virtual ~osStopWatch();

    bool start();
    bool stop();
    bool pause();
    bool resume();
    bool isRunning() const;
    bool getTimeInterval(double& timeInterval) const;

    // Get current time in milliseconds as string:
    static void appendCurrentTimeAsString(gtString& timeStr);

private:
    bool calculateTimeInterval(double& timeInterval) const;

private:
    // Contains the stop watch resolution (1 / amount of time units per second):
    double _stopWatchResolution;

    // Contains the time in which this stop-watch was stated / restarted:
    gtUInt64 _startMeasureTime;

    // Contains the time interval that this stop-watch measured (in seconds):
    double _timeInterval;
    double _pastRunsTimeInterval;

    // Contains true iff the stop watch is running:
    bool _isRunning;
};

/// Return the current time in milliseconds:
/// \param[out] currentTimeAsMilliseconds the current time in milliseconds:
/// \return true iff succeeded
bool OS_API osGetCurrentTime(gtUInt64& currentTimeAsMilliseconds);



#endif  // __OSSTOPWATCH
