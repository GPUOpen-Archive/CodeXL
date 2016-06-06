//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTimer.cpp
///
//=====================================================================

//------------------------------ osTimer.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osTimer.h>


// ---------------------------------------------------------------------------
// Name:        osTimer::osTimer
// Description: Constructor
// Arguments: timerInterval - The timer interval, measured in milliseconds.
// Author:      AMD Developer Tools Team
// Date:        18/10/2006
// ---------------------------------------------------------------------------
osTimer::osTimer(long timerInterval)
    : osThread(L"osTimer"), _isOneShotTimer(true), _isActive(false),
      _timerInterval(timerInterval), _hNativeTimerHandle(0), _timerId(-1)
{
}


// ---------------------------------------------------------------------------
// Name:        osTimer::~osTimer
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        22/10/2006
// ---------------------------------------------------------------------------
osTimer::~osTimer()
{
    // If the timer is active - stop its run:
    if (isActive())
    {
        stopTimer();
    }
}


// ---------------------------------------------------------------------------
// Name:        osTimer::startTimer
// Description: Starts the timer's run.
//
// Arguments: oneShot - true - to make the timer trigger only once.
//                      false - to make the timer trigger on every time interval.
//                              continuously.
//
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        20/12/2007
// ---------------------------------------------------------------------------
bool osTimer::startTimer(bool oneShot)
{
    // Log the timer type:
    _isOneShotTimer = oneShot;
    // Execute the threads timer:
    bool retVal = osThread::execute();
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osTimer::stopTimer
// Description: Stops the timer run.
// Author:      AMD Developer Tools Team
// Date:        22/10/2006
// ---------------------------------------------------------------------------
void osTimer::stopTimer()
{
    // If the timer is active:
    if (_isActive)
    {
        // Mark that the timer is no longer active:
        _isActive = false;

        // Terminate the timer thread:
        osThread::terminate();
    }
}


// ---------------------------------------------------------------------------
// Name:        osTimer::entryPoint
// Description: The timer thread's entry point.
// Return Val: int - The threads return value (ignored).
// Author:      AMD Developer Tools Team
// Date:        30/6/2008
// ---------------------------------------------------------------------------
int osTimer::entryPoint()
{
    int retVal = 0;

    // Mark that the timer is active and start the timer loop:
    _isActive = true;

    while (_isActive)
    {
        // Wait timer interval period:
        osSleep(_timerInterval);

        // If we were not deactivated during the wait period:
        if (_isActive)
        {
            // Call the timer notification function:
            onTimerNotification();
        }

        // If this is a one shot timer:
        if (_isOneShotTimer)
        {
            _isActive = false;
        }
    }

    // Mark that the timer is no longer active:
    _isActive = false;

    // Exit the timer thread:
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osTimer::beforeTermination
// Description:
//   Thread exit point. Not used under Linux, since Linux timers implementation
//   uses signals and not threads.
// Author:      AMD Developer Tools Team
// Date:        20/12/2007
// ---------------------------------------------------------------------------
void osTimer::beforeTermination()
{
}
