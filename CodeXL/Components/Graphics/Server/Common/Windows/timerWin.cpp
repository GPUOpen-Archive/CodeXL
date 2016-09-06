//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Statics and functions used for Windows-specific time related functions
//==============================================================================

#include <algorithm>
#include "../timer.h"
#include "../mymutex.h"
#include "../HookTimer.h"
#include <AMDTBaseTools/Include/gtAssert.h>

// #define TRACE_TIMER   // enable this to turn on API Trace logging for timer functions.
// Disabled by default due to volume of messages and performance implications

#ifdef TRACE_TIMER
    #include "Logger.h"
#endif

// Statics holding all the time variables used for pausing time
static LONGLONG s_TimeQPCCurrent = (LONGLONG)0; ///< Timing variable for pausing time
static DWORD s_TimeGTCCurrent = 0; ///< Timing variable for pausing time
static DWORD s_TimeTGTCurrent = 0; ///< Timing variable for pausing time

static LONGLONG s_TimeQPCPrevious = (LONGLONG)0; ///< Timing variable for pausing time
static DWORD s_TimeGTCPrevious = 0; ///< Timing variable for pausing time
static DWORD s_TimeTGTPrevious = 0; ///< Timing variable for pausing time

static LONGLONG s_TimeQPCIni = (LONGLONG)0; ///< Timing variable for pausing time
static DWORD s_TimeGTCIni = 0; ///< Timing variable for pausing time
static DWORD s_TimeTGTIni = 0; ///< Timing variable for pausing time

static LONGLONG s_TimeQPCFin = (LONGLONG)0; ///< Timing variable for pausing time
static DWORD s_TimeGTCFin = 0; ///< Timing variable for pausing time
static DWORD s_TimeTGTFin = 0; ///< Timing variable for pausing time

static LONGLONG s_TimeQPCFrozen = (LONGLONG)0; ///< Timing variable for pausing time
static DWORD s_TimeGTCFrozen = 0; ///< Timing variable for pausing time
static DWORD s_TimeTGTFrozen = 0; ///< Timing variable for pausing time

//---------------------------------------------------------------------
///
/// This function sets the current pause state. When transitioning from
/// not paused to paused, the initial times are stored. When transitioning from
/// paused to not paused, the amounts of time paused (frozen) are calculated.
///
/// \param bFreezeTime the current pause state
///
/// \return
//---------------------------------------------------------------------
void TimeControl::SetFreezeTime(bool bFreezeTime)
{
    if ((bFreezeTime == true) && (m_bFreezeTime == false))
    {
        s_TimeQPCIni = s_TimeQPCCurrent;
        s_TimeGTCIni = s_TimeGTCCurrent;
        s_TimeTGTIni = s_TimeTGTCurrent;
        m_bFreezeTime = true;
    }
    else if ((bFreezeTime == false) && (m_bFreezeTime == true))
    {
        s_TimeQPCFin = s_TimeQPCCurrent;
        s_TimeGTCFin = s_TimeGTCCurrent;
        s_TimeTGTFin = s_TimeTGTCurrent;

        s_TimeQPCFrozen += s_TimeQPCFin - s_TimeQPCIni;
        s_TimeGTCFrozen += s_TimeGTCFin - s_TimeGTCIni;
        s_TimeTGTFrozen += s_TimeTGTFin - s_TimeTGTIni;
        m_bFreezeTime = false;
    }
}

//---------------------------------------------------------------------
///
/// This function is our version of QueryPerformanceCounter.  First calculate
/// the delta time. If time is not paused, calculate the current time by adding
/// the delta modulated by speed to the current time. Then subtract the
/// cumulative paused time. If time is paused, return the time when pause
/// began less the cumulative paused time.
/// \param lpCount the output of QueryPerformanceCounter()
/// \return success of failure
//---------------------------------------------------------------------
BOOL WINAPI Mine_QueryPerformanceCounter(GPS_TIMESTAMP* lpCount)
{
#ifdef TRACE_TIMER
    LogTrace(traceMESSAGE, "Mine_QueryPerformanceCounter( %d )", lpCount);
#endif
    GPS_TIMESTAMP Cnt;

    static mutex mtx;
    ScopeLock t(&mtx);

    BOOL bRes = Real_QueryPerformanceCounter(&Cnt);
    LONGLONG delta = Cnt.QuadPart - s_TimeQPCPrevious;
    s_TimeQPCPrevious = Cnt.QuadPart;

    if (TimeControl::Singleton().GetFreezeTime() == false)
    {
        //
        // Only do the expensive casting if the speed != 1.0
        if (TimeControl::Singleton().GetPlaySpeed() != 1.0)
        {
            s_TimeQPCCurrent += std::max<LONGLONG>(1, (LONGLONG)(delta * (double)TimeControl::Singleton().GetPlaySpeed()));
        }
        else
        {
            s_TimeQPCCurrent += delta;
        }

        lpCount->QuadPart = s_TimeQPCCurrent - s_TimeQPCFrozen;
    }
    else
    {
        lpCount->QuadPart = s_TimeQPCIni - s_TimeQPCFrozen;
    }

    return bRes;
}

//---------------------------------------------------------------------
///
/// \see Mine_QueryPerformanceCounter
///
/// \return current calculated time
//---------------------------------------------------------------------
DWORD WINAPI Mine_GetTickCount()
{
#ifdef TRACE_TIMER
    LogTrace(traceMESSAGE, "Mine_GetTickCount()");
#endif
    static mutex mtx;
    ScopeLock t(&mtx);

    DWORD Cnt = Real_GetTickCount();
    DWORD delta = Cnt - s_TimeGTCPrevious;
    s_TimeGTCPrevious = Cnt;


    if (TimeControl::Singleton().GetFreezeTime() == false)
    {
        //
        // Only do the expensive casting if the speed != 1.0
        if (TimeControl::Singleton().GetPlaySpeed() != 1.0)
        {
            s_TimeGTCCurrent += std::max<DWORD>(1, (DWORD)(delta * (double)TimeControl::Singleton().GetPlaySpeed()));
        }
        else
        {
            s_TimeGTCCurrent += delta;
        }

        return  s_TimeGTCCurrent - s_TimeGTCFrozen;
    }
    else
    {
        return s_TimeGTCIni - s_TimeGTCFrozen;
    }
}

//---------------------------------------------------------------------
///
/// (see comment for Mine_QueryPerformanceCounter)
///
/// \return current calculated time
//---------------------------------------------------------------------
DWORD WINAPI Mine_timeGetTime()
{
#ifdef TRACE_TIMER
    LogTrace(traceMESSAGE, "Mine_timeGetTime()");
#endif
    static mutex mtx;
    ScopeLock t(&mtx);

    DWORD Cnt = Real_timeGetTime();
    DWORD delta = Cnt - s_TimeTGTPrevious;
    s_TimeTGTPrevious = Cnt;


    if (TimeControl::Singleton().GetFreezeTime() == false)
    {
        //
        // Only do the expensive casting if the speed != 1.0
        if (TimeControl::Singleton().GetPlaySpeed() != 1.0)
        {
            s_TimeTGTCurrent += std::max<DWORD>(1, (DWORD)(delta * (double)TimeControl::Singleton().GetPlaySpeed()));
        }
        else
        {
            s_TimeTGTCurrent += delta;
        }

        return s_TimeTGTCurrent - s_TimeTGTFrozen;
    }
    else
    {
        return s_TimeTGTIni - s_TimeTGTFrozen;
    }
}

//---------------------------------------------------------------------
///
/// Constructor for the Timer class
///
//---------------------------------------------------------------------
Timer::Timer()
{
    GPS_TIMESTAMP freq;
    QueryPerformanceFrequency(&freq);
    m_iFreq = freq.QuadPart;
    ResetTimer();
}

//---------------------------------------------------------------------
///
/// Reset method for the Timer class. This method sets the start time
///
//---------------------------------------------------------------------
void Timer::ResetTimer()
{
    GPS_TIMESTAMP time;

    if (Real_QueryPerformanceCounter == NULL)
    {
        QueryPerformanceCounter(&time);
    }
    else
    {
        Real_QueryPerformanceCounter(&time);
    }

    m_iStartTime = time.QuadPart;
}

//---------------------------------------------------------------------
///
/// Lap method for the Timer class.
///
/// \return the current time less the start time
//---------------------------------------------------------------------
unsigned long Timer::Lap()
{
    GPS_TIMESTAMP time;

    if (Real_QueryPerformanceCounter == NULL)
    {
        QueryPerformanceCounter(&time);
    }
    else
    {
        Real_QueryPerformanceCounter(&time);
    }

    return (unsigned long)((1000 * (time.QuadPart - m_iStartTime)) / m_iFreq);
}

//---------------------------------------------------------------------
///
/// Lap method for the Timer class. Returns double instead of int.
///
/// \return the current time less the start time
//---------------------------------------------------------------------
double Timer::LapDouble()
{
    GPS_TIMESTAMP time;

    if (Real_QueryPerformanceCounter == NULL)
    {
        QueryPerformanceCounter(&time);
    }
    else
    {
        Real_QueryPerformanceCounter(&time);
    }

    return (double)((1000.0 * (double)(time.QuadPart - m_iStartTime)) / (double)m_iFreq);
}

//---------------------------------------------------------------------
///
/// GetAbsolute method for the Timer class.
///
/// \return the current time in milliseconds
//---------------------------------------------------------------------
unsigned long Timer::GetAbsoluteMilliseconds()
{
    GPS_TIMESTAMP time;

    if (Real_QueryPerformanceCounter == NULL)
    {
        QueryPerformanceCounter(&time);
    }
    else
    {
        Real_QueryPerformanceCounter(&time);
    }

    return (unsigned long)((1000 * time.QuadPart) / m_iFreq);
}

//---------------------------------------------------------------------
///
/// GetAbsolute method for the Timer class.
///
/// \return the current time in microseconds
//---------------------------------------------------------------------
unsigned long Timer::GetAbsoluteMicroseconds()
{
    GPS_TIMESTAMP time;

    if (Real_QueryPerformanceCounter == NULL)
    {
        QueryPerformanceCounter(&time);
    }
    else
    {
        Real_QueryPerformanceCounter(&time);
    }

    GT_ASSERT(m_iFreq != 0);

    return static_cast<unsigned long>((1000000 * time.QuadPart) / m_iFreq);
}

//---------------------------------------------------------------------
/// Returns the raw value returend by QueryPerformanceCounter
///
/// \return the value from QueryPerformanceCounter
//---------------------------------------------------------------------
GPS_TIMESTAMP Timer::GetRaw() const
{
    GPS_TIMESTAMP time;

    if (Real_QueryPerformanceCounter == NULL)
    {
        QueryPerformanceCounter(&time);
    }
    else
    {
        Real_QueryPerformanceCounter(&time);
    }

    return time;
}
