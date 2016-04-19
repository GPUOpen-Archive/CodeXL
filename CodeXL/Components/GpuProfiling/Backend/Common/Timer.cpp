//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief A utility for timing operations (adapted from the OpenCL perforce).
//==============================================================================

#ifdef _LINUX
    #include <time.h>
#elif !defined(_WIN32)
    #include <mach/mach_time.h>
#else
    #include <windows.h>
    #include <time.h>
#endif

#include <math.h>
#include <stdio.h>

#include "Timer.h"

Timer::Timer()
    : m_bTimerStopped(true)
    , m_llQfpTicksPerSec(0)
    , m_llStopTime(0)
    , m_llLastElapsedTime(0)
    , m_llBaseTime(0)
{
#ifdef _WIN32
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    m_llQfpTicksPerSec = frequency.QuadPart;
#endif

}

unsigned long long
Timer::TimeNanos()
{
#ifdef _LINUX
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return (unsigned long long) tp.tv_sec * (1000ULL * 1000ULL * 1000ULL) +
           (unsigned long long) tp.tv_nsec;
#elif !defined(_WIN32)
    uint64_t n = mach_absolute_time();
    return n * timebaseInfo.numer / timebaseInfo.denom;
#else
    LARGE_INTEGER current;
    QueryPerformanceCounter(&current);
    return (unsigned long long)((double) current.QuadPart / m_llQfpTicksPerSec * 1e9);
#endif
}

void
Timer::Reset()
{
    unsigned long long llQwTime = GetAdjustedCurrentTime();

    m_llBaseTime        = llQwTime;
    m_llLastElapsedTime = llQwTime;
    m_llStopTime        = 0;
    m_bTimerStopped     = false;
}

void
Timer::Start()
{
    // Get the current time
    unsigned long long llQwTime = 0;

    llQwTime = TimeNanos();

    if (m_bTimerStopped)
    {
        m_llBaseTime += llQwTime - m_llStopTime;
    }

    m_llStopTime        = 0;
    m_llLastElapsedTime = llQwTime;
    m_bTimerStopped     = false;
}

void
Timer::Stop()
{
    if (!m_bTimerStopped)
    {
        unsigned long long llQwTime = TimeNanos();
        m_llStopTime                = llQwTime;
        m_llLastElapsedTime         = llQwTime;
        m_bTimerStopped             = true;
    }
}

unsigned long long
Timer::GetAbsoluteTime()
{
    return TimeNanos();
}

unsigned long long
Timer::GetTime()
{
    unsigned long long llQwTime = GetAdjustedCurrentTime();

    unsigned long long dAppTime = llQwTime - m_llBaseTime;

    return dAppTime;
}


unsigned long long
Timer::GetElapsedTime()
{
    unsigned long long llQwTime = GetAdjustedCurrentTime();

    long long dElapsedTime = llQwTime - m_llLastElapsedTime;
    m_llLastElapsedTime = llQwTime;

    if (dElapsedTime < 0)
    {
        dElapsedTime = 0;
    }

    return dElapsedTime;
}

unsigned long long
Timer::GetAdjustedCurrentTime()
{
    unsigned long long llQwTime;

    if (m_llStopTime != 0)
    {
        llQwTime = m_llStopTime;
    }
    else
    {
        llQwTime = TimeNanos();
    }

    return llQwTime;
}

bool
Timer::IsStopped()
{
    return m_bTimerStopped;
}

//--------------------------------------------------------------------------------------
// Limit the current thread to one processor (the current one). This ensures that timing code
// runs on only one processor, and will not suffer any ill effects from power management.
// See "Game Timing and Multicore Processors" for more details
//--------------------------------------------------------------------------------------
void
Timer::LimitThreadAffinityToCurrentProc()
{
#ifdef _WIN32
    HANDLE hCurrentProcess = GetCurrentProcess();

    // Get the processor affinity mask for this process
    DWORD_PTR dwProcessAffinityMask = 0;
    DWORD_PTR dwSystemAffinityMask = 0;

    if (GetProcessAffinityMask(hCurrentProcess, &dwProcessAffinityMask, &dwSystemAffinityMask) != 0 &&
        dwProcessAffinityMask)
    {
        // Find the lowest processor that our process is allows to run against
        DWORD_PTR dwAffinityMask = (dwProcessAffinityMask & ((~dwProcessAffinityMask) + 1));

        // Set this as the processor that our thread must always run against
        // This must be a subset of the process affinity mask
        HANDLE hCurrentThread = GetCurrentThread();

        if (INVALID_HANDLE_VALUE != hCurrentThread)
        {
            SetThreadAffinityMask(hCurrentThread, dwAffinityMask);
            CloseHandle(hCurrentThread);
        }
    }

    CloseHandle(hCurrentProcess);
#endif // _WIN32
}
