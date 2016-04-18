//=====================================================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief Defines the entry point for the DLL application.
//=====================================================================================

#ifndef GPS_MICRODLL_INCLUDED
#define GPS_MICRODLL_INCLUDED

#include <windows.h>
#include "../Common/defines.h"

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the MICRODLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// MICRODLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef MICRODLL_EXPORTS
    #define MICRODLL_API __declspec( dllexport ) ///< Export macro
#else
    #define MICRODLL_API __declspec( dllimport ) ///< Export macro
#endif

extern char g_MicroDLLPath[ PS_MAX_PATH ]; ///< Micro dll path

/// This class is exported from the MicroDLL.dll
class MICRODLL_API CMicroDLL
{
public:
    CMicroDLL(void);
    // add your methods here.
};

#if 0
    extern MICRODLL_API int nMicroDLL;

    MICRODLL_API int fnMicroDLL(void);
#endif

#endif // GPS_MICRODLL_INCLUDED
