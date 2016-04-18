//==============================================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief A CPU timer class.
//==============================================================================

#include "CpuTimer.h"

#ifdef _WIN32
#if USE_RDTSC
__declspec(naked) UINT64 __cdecl rdtsc_time(void)
{
    __asm rdtsc
    __asm ret
}
#endif // USE_RDTSC
#endif

/**
****************************************************************************************************
*   CpuTimer::CpuTimer
*
*   @brief
*       Constructor.
*
*   @return
*       N/A
****************************************************************************************************
*/
CpuTimer::CpuTimer()
    : m_freq(0.0)
    , m_startTime(0)
{
#ifdef _WIN32
#if USE_RDTSC
    static const double CalibrationTime = 1.0;
    UINT64 t0 = rdtsc_time();
    Delay(CalibrationTime);
    UINT64 t1 = rdtsc_time();
    m_freq = static_cast<double>(t1 - t0) / CalibrationTime;
#else
    LARGE_INTEGER freq = {0};
    QueryPerformanceFrequency(&freq);
    m_freq = static_cast<double>(freq.QuadPart);
#endif
#endif
}

/**
****************************************************************************************************
*   CpuTimer::Delay
*
*   @brief
*       Spins for the given number of seconds.
*   @param delay time to spin wait for
*   @return
*       N/A
****************************************************************************************************
*/
void CpuTimer::Delay(double delay)
{
#ifdef _WIN32
    LARGE_INTEGER st = {0};
    LARGE_INTEGER et = {0};
    QueryPerformanceCounter(&st);

    while (1)
    {
        QueryPerformanceCounter(&et);

        double t = static_cast<double>(et.QuadPart - st.QuadPart) / m_freq;

        if (t >= delay)
        {
            break;
        }
    }

#endif
}

/**
****************************************************************************************************
*   CpuTimer::Start
*
*   @brief
*       Starts the timer.
*
*   @return
*       N/A
****************************************************************************************************
*/
void CpuTimer::Start()
{
#ifdef _WIN32
#if USE_RDTSC
    m_startTime = rdtsc_time();
#else
    LARGE_INTEGER t = {0};
    QueryPerformanceCounter(&t);
    m_startTime = t.QuadPart;
#endif
#endif
}

/**
****************************************************************************************************
*   CpuTimer::Stop
*
*   @brief
*       Stops the timer.
*
*   @return
*       The time elapsed since Start() in microseconds.
****************************************************************************************************
*/
double CpuTimer::Stop()
{
#ifdef _WIN32
    UINT64 endTime = m_startTime;

#if USE_RDTSC
    endTime = rdtsc_time();
#else
    LARGE_INTEGER t = {0};
    QueryPerformanceCounter(&t);
    endTime = t.QuadPart;
#endif

    return (static_cast<double>(endTime - m_startTime) / m_freq) * 1000000;
#else
    return 0;
#endif
}
