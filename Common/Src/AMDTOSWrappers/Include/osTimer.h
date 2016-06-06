//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTimer.h
///
//=====================================================================

//------------------------------ osTimer.h ------------------------------

#ifndef __OSTIMER_H
#define __OSTIMER_H

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <AMDTOSWrappers/Include/osThread.h>


// ----------------------------------------------------------------------------------
// Class Name:           OS_API osTimer : protected osThread
// General Description:
//   Represents a timer: enables executing code at specified time intervals.
//   Notice that osTimer uses its own thread; therefore, class users should
//   (if needed) perform threads synchronization.
//
// Author:      AMD Developer Tools Team
// Creation Date:        18/10/2006
// ----------------------------------------------------------------------------------
class OS_API osTimer : protected osThread
{
public:
    osTimer(long timerInterval);
    virtual ~osTimer();

    bool startTimer(bool oneShot);
    void stopTimer();
    bool isActive() const { return _isActive; };
    long timerInterval() const { return _timerInterval; };

protected:
    // Must be overridden by sub-classes:
    virtual void onTimerNotification() = 0;

    // Overrides osThread:
    virtual int entryPoint();
    virtual void beforeTermination();

private:
    // Do not allow the use of my default constructor:
    osTimer();

private:
    // true - for a "one shot" timer.
    // false - for a "continuous" timer.
    bool _isOneShotTimer;

    // Contains true iff the timer is active:
    bool _isActive;

    // The timer interval, measured in milliseconds:
    long _timerInterval;

    // The native OS timer handle:
    osTimerHandle _hNativeTimerHandle;

    // An internal timer id:
    int _timerId;
};


#endif //__OSTIMER_H
