//==============================================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief A CPU timer class.
//==============================================================================

#ifndef __CPU_TIMER_H__
#define __CPU_TIMER_H__

#ifdef _WIN32
    #include <windows.h>
#endif

/// Controls the use of older timestamp method
#define USE_RDTSC 0

/**
****************************************************************************************************
* @brief A CPU timer class.
****************************************************************************************************
*/
class CpuTimer
{
public:

    CpuTimer();

    void Start();

    double Stop();

private:

    void Delay(double delay);

    double    m_freq; ///< Timer frequency
    long long m_startTime; ///< Timer start time
};

#endif // __CPU_TIMER_H__