//==================================================================================
// Copyright (c) 2004 - 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTDebug.h
///
//==================================================================================

//------------------------------ AMDTDebug.h ----------------------------

#ifndef __AMDTDEBUG_H
#define __AMDTDEBUG_H

#include <stdio.h>

#if defined(WIN32)
    #define __PRETTY_FUNCTION__ __FUNCSIG__
#endif // #if defined(WIN32)


/*
 * Some helpful debugging/tracing macros, gcc likes these but MSVC will probably choke.
 */
#ifdef _DEBUG
#ifndef TRACE_LEVEL
    #define TRACE_LEVEL                    (300)
#endif //#ifndef TRACE_LEVEL
#ifndef DEBUG_LEVEL
    #define DEBUG_LEVEL                    (300)
#endif //#ifndef DEBUG_LEVEL


#if defined(WIN32)
#define TRACE(lvl, fmt, ...)                                                                                            \
    do {                                                                                                                    \
        if ((lvl) >= TRACE_LEVEL)   {                                                                                       \
            wchar_t str[1024];                                                                                              \
            wchar_t fnc[1024];                                                                                              \
            ::mbstowcs(fnc, __PRETTY_FUNCTION__, 1024);                                                                     \
            ::_snwprintf(str, 1024, L"TRACE: %3d:%s: " L ## fmt L"\n" , __LINE__, fnc, __VA_ARGS__);                        \
            ::OutputDebugString(str);                                                                                       \
        }                                                                                                                   \
    } while (0)

#define DDD(lvl, fmt, ...)                                                                                              \
    do {                                                                                                                    \
        if ((lvl) >= DEBUG_LEVEL)   {                                                                                       \
            wchar_t str[1024];                                                                                              \
            wchar_t fnc[1024];                                                                                              \
            ::mbstowcs(fnc, __PRETTY_FUNCTION__, 1024);                                                                     \
            ::_snwprintf(str, 1024, L"DEBUG: %3d:%s: " L ## fmt L"\n" , __LINE__, fnc, __VA_ARGS__);                        \
            ::OutputDebugString(str);                                                                                       \
        }                                                                                                                   \
    } while (0)

#elif defined(__linux__)    // #if defined(WIN32)
#define TRACE(lvl, fmt, args...)                                                                                    \
    do {                                                                                                                \
        if ((lvl) >= TRACE_LEVEL) fprintf(stderr, "TRACE: %3d:%s: " fmt "\n" , __LINE__, __PRETTY_FUNCTION__, ##args);  \
    } while (0)

#define DDD(lvl, fmt, args...)                                                                                      \
    do {                                                                                                                \
        if ((lvl) >= DEBUG_LEVEL) fprintf(stderr, "DEBUG: %3d:%s: " fmt "\n" , __LINE__, __PRETTY_FUNCTION__, ##args);  \
    } while (0)
#endif // #elif defined(__linux__)

#else
#if defined(WIN32)
    #define TRACE(lvl, fmt, ...)
    #define DDD(lvl, fmt, ...)
#elif defined(__linux__)    // #if defined(WIN32)
    #define TRACE(lvl, fmt, args...)
    #define DDD(lvl, fmt, args...)
#endif // #elif defined(__linux__)
#endif

#endif // #ifndef __AMDTDEBUG_H
