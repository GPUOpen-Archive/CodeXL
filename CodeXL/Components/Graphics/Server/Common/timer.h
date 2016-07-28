//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Class used for time related override functions. Used to override the games's time values.
//==============================================================================

#ifndef TIMER_H
#define TIMER_H

#include "CommonTypes.h"

/// Base class which maintains the logic for play speed and real-pause
class TimeControl
{
public:
    /// Destructor
    ~TimeControl();

    /// singleton
    static TimeControl& Singleton();

    /// sets play speed factor, x1, x2 x3...  x0.1
    void SetPlaySpeed(float fPlaySpeed);

    /// GetPlaySpeed
    float GetPlaySpeed();

    /// Sets the RealPause mode
    void SetRealPause(bool bRealPause);
    bool GetRealPause();

    /// Sets the FreezeTime mode
    void SetFreezeTime(bool bFreezeTime);
    bool GetFreezeTime();

private:

    /// Private constructor to follow Singleton pattern
    TimeControl();

    /// Stores current freeze time state - needed in SetFreezeTime()
    bool m_bFreezeTime;

    /// Indicates if the user has requested to use freezetime (true) or slowmotion (false) for pausing
    bool m_RealPause;

    /// The play speed
    float m_SpeedControl;
};

/// Helper class to measure time
class Timer
{
public:
    // inits the timer
    Timer();

    // resets timer to zero
    void ResetTimer();

    /// reads the time (in ms) but does not stop the timer
    unsigned long Lap();

    /// reads the time (in ms, double precision) but does not stop the timer
    double LapDouble();

    /// Gets the QPC time ( in ms )
    unsigned long GetAbsolute();

    /// Gets the raw QPC time
    GPS_TIMESTAMP GetRaw() const;

private:
    /// time when the timer was reset
    INT64 m_iStartTime;

    /// frequency of QPC timer
    INT64 m_iFreq;
};

#endif // TIMER_H
