//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTTimer.h
///
//==================================================================================

//------------------------------ AMDTTimer.h ------------------------------

#ifndef __AMDTTIMER_H
#define __AMDTTIMER_H

#ifdef _WIN32

#include <windows.h>
#include <MMSYSTEM.H>

#pragma comment(lib, "winmm.lib")

struct AMDTTimer
{
    LARGE_INTEGER _start, _stop, _freq;

    AMDTTimer()
    {
        QueryPerformanceFrequency(&_freq);
        start();
    }

    void start()
    {
        QueryPerformanceCounter(&_start);
    }

    double elapsed()
    {
        QueryPerformanceCounter(&_stop);
        return double(_stop.QuadPart - _start.QuadPart) / double(_freq.QuadPart);
    }

    double restart()
    {
        QueryPerformanceCounter(&_stop);
        double ret = double(_stop.QuadPart - _start.QuadPart) / double(_freq.QuadPart);
        _start = _stop;
        return ret;
    }
};

#else // !_WIN32

#include <sys/time.h>

struct AMDTTimer
{
    unsigned long long _start, _stop;

    AMDTTimer()
    {
        start();
    }

    void start()
    {
        struct timeval now;
        gettimeofday(&now, 0);
        _start = (now.tv_sec * 1000000L) + now.tv_usec;
    }

    double elapsed()
    {
        struct timeval now;
        gettimeofday(&now, 0);
        unsigned long long m_stop = (now.tv_sec * 1000000L) + now.tv_usec;
        return double(m_stop - _start) * 0.000001;
    }

    double restart()
    {
        struct timeval now;
        gettimeofday(&now, 0);
        unsigned long long m_stop = (now.tv_sec * 1000000L) + now.tv_usec;
        double ret = double(m_stop - _start) * 0.000001;
        _start = m_stop;
        return ret;
    }
};

#endif // _WIN32

#endif // __AMDTTIMER_H
