//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file is the only header that must be included by an application that
///        wishes to use APP Profiler user PMC extension.
//==============================================================================

#ifndef _AP_TRACE_USER_PMC_EXT_H_
#define _AP_TRACE_USER_PMC_EXT_H_

#if defined(_WIN32) || defined(__CYGWIN__)
    #include<Windows.h>
#else
    typedef unsigned long long ULONGLONG;
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
    #ifdef __cplusplus
        #define AP_DECL extern "C" __declspec( dllexport )
    #else
        #define AP_DECL __declspec( dllexport )
    #endif
#else
    #ifdef __cplusplus
        #define AP_DECL extern "C"
    #else
        #define AP_DECL
    #endif
#endif

#define PMC_IMP(name)   \
    const char* g_sz##name = #name; \
    AP_DECL ULONGLONG AP_Get##name()

AP_DECL void InitPMCs(const char** * pppCounters, size_t* pSize);

typedef ULONGLONG(*GetPMCProc)();
typedef void (*InitPMCsProc)(const char** * pppCounters, size_t* pSize);

#define BEGIN_DEF_PMC static const char* g_sCounters[] = {

#define END_DEF_PMC }; \
    AP_DECL void InitPMCs(const char*** pppCounters, size_t* pSize) \
    { \
        *pppCounters = g_sCounters; \
        *pSize = sizeof(g_sCounters) / sizeof(const char*); \
    }

#endif //_AP_TRACE_USER_PMC_EXT_H_
