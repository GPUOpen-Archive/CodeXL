//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Statics and functions used for the Linux-specific
///         time related functions
//==============================================================================

#include <dlfcn.h>                      // header required for dlsym()
#include <algorithm>
#include "timer.h"
#include "mymutex.h"
#include "HookTimer.h"
#include <AMDTBaseTools/Include/gtAssert.h>

// #define TRACE_TIMER   // enable this to turn on API Trace logging for timer functions.
// Disabled by default due to volume of messages and performance implications

#ifdef TRACE_TIMER
    #include "Logger.h"
#endif

static const LONGLONG ONE_BILLION = 1000000000;

// Statics holding all the time varibales used for pausing time
static LONGLONG s_TimeGTDCurrent = 0;
static LONGLONG s_TimeGFTCurrent = 0;
static LONGLONG s_TimeCGTCurrent = 0;

static LONGLONG s_TimeGTDPrevious = 0;
static LONGLONG s_TimeGFTPrevious = 0;
static LONGLONG s_TimeCGTPrevious = 0;

static LONGLONG s_TimeGTDIni = 0;
static LONGLONG s_TimeGFTIni = 0;
static LONGLONG s_TimeCGTIni = 0;

static LONGLONG s_TimeGTDFin = 0;
static LONGLONG s_TimeGFTFin = 0;
static LONGLONG s_TimeCGTFin = 0;

static LONGLONG s_TimeGTDFrozen = 0;
static LONGLONG s_TimeGFTFrozen = 0;
static LONGLONG s_TimeCGTFrozen = 0;

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
        s_TimeGTDIni = s_TimeGTDCurrent;
        s_TimeGFTIni = s_TimeGFTCurrent;
        s_TimeCGTIni = s_TimeCGTCurrent;
        m_bFreezeTime = true;
    }
    else if ((bFreezeTime == false) && (m_bFreezeTime == true))
    {
        s_TimeGTDFin = s_TimeGTDCurrent;
        s_TimeGFTFin = s_TimeGFTCurrent;
        s_TimeCGTFin = s_TimeCGTCurrent;

        s_TimeGTDFrozen += s_TimeGTDFin - s_TimeGTDIni;
        s_TimeGFTFrozen += s_TimeGFTFin - s_TimeGFTIni;
        s_TimeCGTFrozen += s_TimeCGTFin - s_TimeCGTIni;
        m_bFreezeTime = false;
    }
}

//---------------------------------------------------------------------
///
/// Linux replacement timing functions. A NULL test needs to be done here
/// incase any other apps have inadvertently preloaded this shared
/// library and haven't set up the Real function pointer
///
/// Works in the same way as QueryPerformanceCounter.
///
/// \param the inputs of gettimeofday
/// \return the output of gettimeofday
//---------------------------------------------------------------------
int     gettimeofday(struct timeval* __restrict tv, __timezone_ptr_t tz)
{
    if (Real_gettimeofday == NULL)
    {
        gettimeofday_type fn = (gettimeofday_type)dlsym(RTLD_NEXT, "gettimeofday");
        return fn(tv, tz);
    }

#ifdef TRACE_TIMER
    LogTrace(traceMESSAGE, "gettimeofday( %d )", (uint64)tv->tv_sec * 1000000 + tv->tv_usec);
#endif
    static mutex mtx;
    ScopeLock t(&mtx);

    struct timeval real_tv;
    int result = Real_gettimeofday(&real_tv, tz);

    // convert struct to microsecond time
    LONGLONG Cnt = (LONGLONG)real_tv.tv_sec * 1000000 + real_tv.tv_usec;

    LONGLONG delta = Cnt - s_TimeGTDPrevious;
    s_TimeGTDPrevious = Cnt;

    LONGLONG time;

    if (TimeControl::Singleton().GetFreezeTime() == false)
    {
        // Only do the expensive casting if the speed != 1.0
        if (TimeControl::Singleton().GetPlaySpeed() != 1.0)
        {
            s_TimeGTDCurrent += std::max<LONGLONG>(1, (LONGLONG)(delta * (double)TimeControl::Singleton().GetPlaySpeed()));
        }
        else
        {
            s_TimeGTDCurrent += delta;
        }

        time = s_TimeGTDCurrent - s_TimeGTDFrozen;
    }
    else
    {
        time = s_TimeGTDIni - s_TimeGTDFrozen;
    }

    tv->tv_sec = time / 1000000;
    tv->tv_usec = time % 1000000;

    return result;
}

//---------------------------------------------------------------------
///
/// Works in the same way as QueryPerformanceCounter.
///
/// \param the inputs of ftime
/// \return the output of ftime
//---------------------------------------------------------------------
int     ftime(struct timeb* tb)
{
    if (Real_ftime == NULL)
    {
        ftime_type fn = (ftime_type)dlsym(RTLD_NEXT, "ftime");
        return fn(tb);
    }

#ifdef TRACE_TIMER
    LogTrace(traceMESSAGE, "ftime( %d )", (tb->time * 1000) + tb->millitm);
#endif
    static mutex mtx;
    ScopeLock t(&mtx);

    struct timeb real_tb;
    int result = Real_ftime(&real_tb);

    // convert struct to microsecond time
    LONGLONG Cnt = (LONGLONG)real_tb.time * 1000 + real_tb.millitm;

    LONGLONG delta = Cnt - s_TimeGFTPrevious;
    s_TimeGFTPrevious = Cnt;

    LONGLONG time;

    if (TimeControl::Singleton().GetFreezeTime() == false)
    {
        // Only do the expensive casting if the speed != 1.0
        if (TimeControl::Singleton().GetPlaySpeed() != 1.0)
        {
            s_TimeGFTCurrent += std::max<LONGLONG>(1, (LONGLONG)(delta * (double)TimeControl::Singleton().GetPlaySpeed()));
        }
        else
        {
            s_TimeGFTCurrent += delta;
        }

        time = s_TimeGFTCurrent - s_TimeGFTFrozen;
    }
    else
    {
        time = s_TimeGFTIni - s_TimeGFTFrozen;
    }

    tb->time = time / 1000;
    tb->millitm = time % 1000;

    return result;
}

//---------------------------------------------------------------------
///
/// Works in the same way as QueryPerformanceCounter.
///
/// \param the inputs of clock_gettime
/// \return the output of clock_gettime
//---------------------------------------------------------------------
int     clock_gettime(clockid_t id, struct timespec* ts)
{
    if (Real_clock_gettime == NULL)
    {
        clock_gettime_type fn = (clock_gettime_type)dlsym(RTLD_NEXT, "clock_gettime");
        return fn(id, ts);
    }

#ifdef TRACE_TIMER
    LogTrace(traceMESSAGE, "clock_gettime( %lld )", (ts->tv_sec * ONE_BILLION) + ts->tv_nsec);
#endif
    static mutex mtx;
    ScopeLock t(&mtx);

    struct timespec real_ts;
    int result = Real_clock_gettime(id, &real_ts);

    // convert struct to microsecond time
    LONGLONG Cnt = (LONGLONG)(real_ts.tv_sec * ONE_BILLION) + real_ts.tv_nsec;

    LONGLONG delta = Cnt - s_TimeCGTPrevious;
    s_TimeCGTPrevious = Cnt;

    LONGLONG time;

    if (TimeControl::Singleton().GetFreezeTime() == false)
    {
        // Only do the expensive casting if the speed != 1.0
        if (TimeControl::Singleton().GetPlaySpeed() != 1.0)
        {
            s_TimeCGTCurrent += std::max<LONGLONG>(1, (LONGLONG)(delta * (double)TimeControl::Singleton().GetPlaySpeed()));
        }
        else
        {
            s_TimeCGTCurrent += delta;
        }

        time = s_TimeCGTCurrent - s_TimeCGTFrozen;
    }
    else
    {
        time = s_TimeCGTIni - s_TimeCGTFrozen;
    }

    ts->tv_sec = time / ONE_BILLION;
    ts->tv_nsec = time % ONE_BILLION;

    return result;
}

//---------------------------------------------------------------------
///
/// Constructor for the Timer class
///
//---------------------------------------------------------------------
Timer::Timer()
{
    m_iFreq = ONE_BILLION;
    ResetTimer();
}

//---------------------------------------------------------------------
///
/// Reset method for the Timer class. This method sets the start time
///
//---------------------------------------------------------------------
void Timer::ResetTimer()
{
    struct timespec ts;

    if (Real_clock_gettime == NULL)
    {
        clock_gettime_type fn = (clock_gettime_type)dlsym(RTLD_NEXT, "clock_gettime");
        fn(CLOCK_REALTIME, &ts);
    }
    else
    {
        Real_clock_gettime(CLOCK_REALTIME, &ts);
    }

    m_iStartTime = ts.tv_sec;
    m_iStartTime *= ONE_BILLION;
    m_iStartTime += ts.tv_nsec;
}

//---------------------------------------------------------------------
///
/// Lap method for the Timer class.
///
/// \return the current time less the start time
//---------------------------------------------------------------------
unsigned long Timer::Lap()
{
    LONGLONG time;

    struct timespec ts;

    if (Real_clock_gettime == NULL)
    {
        clock_gettime_type fn = (clock_gettime_type)dlsym(RTLD_NEXT, "clock_gettime");
        fn(CLOCK_REALTIME, &ts);
    }
    else
    {
        Real_clock_gettime(CLOCK_REALTIME, &ts);
    }

    time = ts.tv_sec;
    time *= ONE_BILLION;
    time += ts.tv_nsec;

    return (unsigned long)((1000 * (time - m_iStartTime)) / m_iFreq);
}

//---------------------------------------------------------------------
///
/// Lap method for the Timer class. Returns double instead of int.
///
/// \return the current time less the start time
//---------------------------------------------------------------------
double Timer::LapDouble()
{
    LONGLONG time;

    struct timespec ts;

    if (Real_clock_gettime == NULL)
    {
        clock_gettime_type fn = (clock_gettime_type)dlsym(RTLD_NEXT, "clock_gettime");
        fn(CLOCK_REALTIME, &ts);
    }
    else
    {
        Real_clock_gettime(CLOCK_REALTIME, &ts);
    }

    time = ts.tv_sec;
    time *= ONE_BILLION;
    time += ts.tv_nsec;

    return (double)((1000.0 * (double)(time - m_iStartTime)) / (double)m_iFreq);
}

//---------------------------------------------------------------------
///
/// GetAbsolute method for the Timer class.
///
/// \return the current time in milliseconds
//---------------------------------------------------------------------
unsigned long Timer::GetAbsoluteMilliseconds()
{
    LONGLONG time;

    struct timespec ts;

    if (Real_clock_gettime == NULL)
    {
        clock_gettime_type fn = (clock_gettime_type)dlsym(RTLD_NEXT, "clock_gettime");
        fn(CLOCK_REALTIME, &ts);
    }
    else
    {
        Real_clock_gettime(CLOCK_REALTIME, &ts);
    }

    GT_ASSERT (m_iFreq != 0);

    time = ts.tv_sec;
    time *= ONE_BILLION;
    time += ts.tv_nsec;

    return static_cast<unsigned long>((1000 * time) / m_iFreq);
}

//---------------------------------------------------------------------
///
/// GetAbsolute method for the Timer class.
///
/// \return the current time in micro seconds
//---------------------------------------------------------------------
unsigned long Timer::GetAbsoluteMicroseconds()
{
    LONGLONG time;

    struct timespec ts;

    if (Real_clock_gettime == NULL)
    {
        clock_gettime_type fn = (clock_gettime_type)dlsym(RTLD_NEXT, "clock_gettime");
        fn(CLOCK_REALTIME, &ts);
    }
    else
    {
        Real_clock_gettime(CLOCK_REALTIME, &ts);
    }

    GT_ASSERT (m_iFreq != 0);

    time = ts.tv_sec;
    time *= ONE_BILLION;
    time += ts.tv_nsec;

    return static_cast<unsigned long>((1000000 * time) / m_iFreq);
}

//---------------------------------------------------------------------
/// Returns the raw value returend by QueryPerformanceCounter
///
/// \return the value from QueryPerformanceCounter
//---------------------------------------------------------------------
GPS_TIMESTAMP Timer::GetRaw() const
{
    GPS_TIMESTAMP time;

    struct timespec ts;

    if (Real_clock_gettime == NULL)
    {
        clock_gettime_type fn = (clock_gettime_type)dlsym(RTLD_NEXT, "clock_gettime");
        fn(CLOCK_REALTIME, &ts);
    }
    else
    {
        Real_clock_gettime(CLOCK_REALTIME, &ts);
    }

    time.QuadPart = ts.tv_sec;
    time.QuadPart *= ONE_BILLION;
    time.QuadPart += ts.tv_nsec;
    return time;
}
