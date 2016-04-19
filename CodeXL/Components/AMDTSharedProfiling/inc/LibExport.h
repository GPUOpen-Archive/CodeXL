//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file LibExport.h
///
//==================================================================================

#ifndef _LIBEXPORT_H
#define _LIBEXPORT_H

#if defined(_WIN32)

    #ifdef AMDTSHAREDPROFILING_EXPORTS
        #define AMDTSHAREDPROFILING_API __declspec(dllexport)
    #else
        #define AMDTSHAREDPROFILING_API __declspec(dllimport)
    #endif

#else

    #define AMDTSHAREDPROFILING_API

#endif

#endif //_LIBEXPORT_H
