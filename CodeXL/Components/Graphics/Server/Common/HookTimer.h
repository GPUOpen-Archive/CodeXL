//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Support functions used to hook the CPU timer functions
//==============================================================================

#ifndef GPS_HOOKTIMER_H
#define GPS_HOOKTIMER_H

#ifdef _WIN32
    #include <windows.h>

    typedef  BOOL (WINAPI* QueryPerformanceCounter_type)(LARGE_INTEGER* lpPerformanceCount); ///< Function pointer typedef
    typedef  DWORD (WINAPI* GetTickCount_type)(); ///< Function pointer typedef
    typedef  DWORD (WINAPI* timeGetTime_type)(); ///< Function pointer typedef

    extern QueryPerformanceCounter_type Real_QueryPerformanceCounter;
    extern GetTickCount_type Real_GetTickCount;
    extern timeGetTime_type Real_timeGetTime;

#else

    #include <time.h>
    #include <sys/timeb.h>
    #include <sys/time.h>
    typedef int (*gettimeofday_type)(struct timeval* __restrict, __timezone_ptr_t); ///< Function pointer typedef
    typedef int (*ftime_type)(struct timeb*); ///< Function pointer typedef
    typedef int (*clock_gettime_type)(clockid_t, struct timespec*); ///< Function pointer typedef

    extern gettimeofday_type    Real_gettimeofday;  ///< Function pointer for timing interception
    extern ftime_type           Real_ftime;  ///< Function pointer for timing interception
    extern clock_gettime_type   Real_clock_gettime;  ///< Function pointer for timing interception

#endif // _WIN32

bool HookTimer();
bool UnhookTimer();

#endif // GPS_HOOKTIMER_H
