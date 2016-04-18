//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file AMDTPowerProfilingMidTier.h
///
//==================================================================================

#ifndef __AMDTPowerProfilingMidTier_h
#define __AMDTPowerProfilingMidTier_h

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the AMDTPOWERPROFILINGMIDTIER_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// AMDTPOWERPROFILINGMIDTIER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#if defined(_WIN32)
    #if defined(AMDTPOWERPROFILINGMIDTIER_EXPORTS)
        #define AMDTPOWERPROFILINGMIDTIER_API __declspec(dllexport)
    #else
        #define AMDTPOWERPROFILINGMIDTIER_API __declspec(dllimport)
    #endif
#else
    #define AMDTPOWERPROFILINGMIDTIER_API
#endif

#endif // __AMDTPowerProfilingMidTier_h
