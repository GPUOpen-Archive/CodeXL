//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTimer.cpp
///
//=====================================================================

//------------------------------ osTimer.cpp ------------------------------

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#define _WIN32_WINNT 0x0500
#include <Windows.h>

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
    : osThread(L"osTimer"), _isActive(false), _isOneShotTimer(true),
      _timerInterval(timerInterval), _hNativeTimerHandle(NULL), _timerId(-1)
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
// Date:        18/10/2006
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
// Date:        18/10/2006
// ---------------------------------------------------------------------------
int osTimer::entryPoint()
{
    int retVal = 0;

    // Create a waitable manual-reset notification timer:
    _hNativeTimerHandle = ::CreateWaitableTimer(NULL, TRUE, NULL);
    GT_IF_WITH_ASSERT(_hNativeTimerHandle != NULL)
    {
        // Transform the timer interval to a large integer accepted by SetWaitableTimer.
        // This large integer:
        // - Represents 100 nanosecond intervals.
        // - Negative values indicate relative time (from now).
        LARGE_INTEGER liDueTime;
        liDueTime.QuadPart = _timerInterval * -10000;

        // Mark that the timer is active and start the timer loop:
        _isActive = true;

        while (_isActive)
        {
            // Activate the timer to wait for 10 seconds.
            BOOL rc1 = ::SetWaitableTimer(_hNativeTimerHandle, &liDueTime, 0, NULL, NULL, 0);
            GT_IF_WITH_ASSERT(rc1 != FALSE)
            {
                // Wait for the timer.
                DWORD rc2 = ::WaitForSingleObject(_hNativeTimerHandle, INFINITE);
                GT_IF_WITH_ASSERT(rc2 == WAIT_OBJECT_0)
                {
                    // If we were not deactivated during the wait period:
                    if (_isActive)
                    {
                        // Call the timer notification function:
                        onTimerNotification();
                    }
                }
            }

            // If this is a one shot timer:
            if (_isOneShotTimer)
            {
                _isActive = false;
            }
        }

        // Mark that the timer is no longer active:
        _isActive = false;

        // Clean up:
        if (_hNativeTimerHandle != NULL)
        {
            ::CloseHandle(_hNativeTimerHandle);
            _hNativeTimerHandle = NULL;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osTimer::beforeTermination
// Description: Is called before the thread is terminated.
// Author:      AMD Developer Tools Team
// Date:        18/10/2006
// ---------------------------------------------------------------------------
void osTimer::beforeTermination()
{
    // Mark that the timer is no longer active:
    _isActive = false;

    // Clean up:
    if (_hNativeTimerHandle != NULL)
    {
        ::CloseHandle(_hNativeTimerHandle);
        _hNativeTimerHandle = NULL;
    }
}

