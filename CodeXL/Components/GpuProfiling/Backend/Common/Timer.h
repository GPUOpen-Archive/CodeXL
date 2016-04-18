//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief A utility for timing operations (adapted from the OpenCL perforce).
//==============================================================================

#ifndef _TIMER_H_
#define _TIMER_H_

/// \addtogroup Common
// @{

/// This class handles timing operations
class Timer
{
public:
    Timer();

    ~Timer() {}

    //! Resets the timer
    void Reset();

    //! Starts the timer
    void Start();

    //! Stop (or pause) the timer
    void Stop();

    //! Gets the absolute system time
    unsigned long long GetAbsoluteTime();

    //! Gets the current time
    unsigned long long GetTime();

    //! Gets the time that elapsed between Get*ElapsedTime() calls
    unsigned long long GetElapsedTime();

    //! returns true if timer stopped
    bool IsStopped();

    //! Limit the current thread to one processor (the current one). This ensures that timing code runs
    //! on only one processor, and will not suffer any ill effects from power management.
    void LimitThreadAffinityToCurrentProc();

    //! Get the absolute time in nanoseconds
    unsigned long long TimeNanos();

protected:
    //! If stopped, returns time when stopped otherwise returns current time
    unsigned long long GetAdjustedCurrentTime();


    bool    m_bTimerStopped;

    unsigned long long  m_llQfpTicksPerSec;
    unsigned long long  m_llStopTime;
    unsigned long long  m_llLastElapsedTime;
    unsigned long long  m_llBaseTime;
};

// @}

#endif // _TIMER_H_
