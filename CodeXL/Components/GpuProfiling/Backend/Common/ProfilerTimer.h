//=======================================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains functions to enable/disable profiling agents based on time
//=======================================================================================

#ifndef _PROFILER_TIMER_H_
#define _PROFILER_TIMER_H_

#include <AMDTOSWrappers/Include/osTimer.h>
#include "TSingleton/TSingleton.h"

enum ProfilerTimerType
{
    NONE,
    PROFILEDELAYTIMER,
    PROFILEDURATIONTIMER
};

typedef void(*TimerEndHandler)(ProfilerTimerType);

//------------------------------------------------------------------------------------
/// This class facilitates to profiler agents to handle timer functionality
//------------------------------------------------------------------------------------
class ProfilerTimer :
    public osTimer
{
public:
    /// Constructor
    /// \param timerInterval time Interval for the timer in milliseconds
    ProfilerTimer(long timerInterval);

    /// Sets the timer type for the timer
    /// \param timerType Type of the timer
    void SetTimerType(ProfilerTimerType timerType);

    /// Accessor to the timer type
    /// \return timer type
    ProfilerTimerType GetTimerType();

    /// Sets the Call back function for the end of timer action
    /// \param timerHandler call back function
    void SetTimerFinishHandler(TimerEndHandler timerEndHandler);

protected:
    virtual void onTimerNotification();

private:
    /// Constructor
    ProfilerTimer();
    TimerEndHandler m_timerHandler;         ///< call back function
    ProfilerTimerType m_timerType;          ///< type of the timer
};

#endif //_PROFILER_TIMER_H_
