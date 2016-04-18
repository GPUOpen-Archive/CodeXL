//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/AMDTGpuProfilerDefs.h $
/// \version $Revision: #18 $
/// \brief  This file contains global declarations shared by the entire GPU Profiler plugin
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/AMDTGpuProfilerDefs.h#18 $
// Last checkin:   $DateTime: 2016/02/03 12:12:58 $
// Last edited by: $Author: tchiu $
// Change list:    $Change: 557941 $
//=====================================================================

#ifndef _AMDTGPUPROFILERDEFS_H_
#define _AMDTGPUPROFILERDEFS_H_

#include <stddef.h>

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QList>
#include <QString>

// Local:
#include <AMDTGpuProfiling/gpStringConstants.h>

// enable this define to turn on Object Inspector view, some manual enable required, search for "GP_OBJECT_VIEW_ENABLE(manual enable)"
//#define GP_OBJECT_VIEW_ENABLE

// Under Win32 builds - define: AMDT_GPU_PROF_API to be:
// - When building AMDTGpuProfiling.dll:          __declspec(dllexport).
// - When building other projects:                __declspec(dllimport).

#if defined(_WIN32)
    #if defined(AMDTGPUPROFILER_EXPORTS)
        #define AMDT_GPU_PROF_API __declspec(dllexport)
    #else
        #define AMDT_GPU_PROF_API __declspec(dllimport)
    #endif
#else
    #define AMDT_GPU_PROF_API
#endif

// undef the macro (from the backend Defs.h)
#ifdef SAFE_DELETE
    #undef SAFE_DELETE
#endif
/// safely delete a pointer and assign it to NULL
/// \param p pointer to delete
template<typename T>
inline void SAFE_DELETE(T& p)
{
    delete p;
    p = NULL;
}

// undef the macro (from the backend Defs.h)
#ifdef SAFE_ARRAY_DELETE
    #undef SAFE_ARRAY_DELETE
#endif
/// safely delete a pointer to array and assign it to NULL
/// \param p pointer to array to delete
template<typename T>
inline void SAFE_ARRAY_DELETE(T& p)
{
    delete[] p;
    p = NULL;
}

#define AGP_HIDDEN_QUOTE( s ) #s
#define AGP_HIDDEN_NUMQUOTE( n ) AGP_HIDDEN_QUOTE( n )

#ifdef _WIN32

    #define AGP_TODO(x)  __pragma( message( __FILE__ "(" AGP_HIDDEN_NUMQUOTE( __LINE__ ) "): TODO: " x ) )

#elif defined (__linux__) || defined (_LINUX) || defined(LINUX)

    // Macros do not seem to directly expand on Linux in #pragma statements
    #define AGP_DO_PRAGMA(x)    _Pragma(#x)
    #define AGP_TODO(x)  AGP_DO_PRAGMA( message( __FILE__ "(" AGP_HIDDEN_NUMQUOTE( __LINE__ ) "): TODO: " x ) )

#elif defined (__CYGWIN__)

    #define AGP_TODO(x)


#endif

//============== String constants ==============

namespace GeneratedFileHeader
{
static const QList<QString> Header = QList<QString>()
                                     << GPU_STR_FileHeader_ProfileFileVersion
                                     << GPU_STR_FileHeader_TraceFileVersion
                                     << GPU_STR_FileHeader_ProfilerVersion
                                     << GPU_STR_FileHeader_GpSessionVersion
                                     << GPU_STR_FileHeader_Application
                                     << GPU_STR_FileHeader_ApplicationArgs
                                     << GPU_STR_FileHeader_UserTimer
                                     << GPU_STR_FileHeader_PlatformVendor
                                     << GPU_STR_FileHeader_PlatformName
                                     << GPU_STR_FileHeader_PlatformVersion
                                     << GPU_STR_FileHeader_CLDriverVersion
                                     << GPU_STR_FileHeader_CLRuntimeVersion
                                     << GPU_STR_FileHeader_NumberAppAddressBits
                                     << GPU_STR_FileHeader_OSVersion;
} // end of namespace of GeneratedFileHeader


#endif  // _AMDTGPUPROFILERDEFS_H_
