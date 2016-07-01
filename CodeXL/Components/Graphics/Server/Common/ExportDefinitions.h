//==============================================================================
// Copyright (c) 2013-2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Define export macros for use by DLLs on Windows OS
//==============================================================================

#ifndef EXPORT_DEFINITIONS_H
#define EXPORT_DEFINITIONS_H

#include <AMDTBaseTools/Include/AMDTDefinitions.h>

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    #ifdef GPS_PLUGIN_EXPORTS
        #define GPS_PLUGIN_API extern "C" __declspec( dllexport ) ///< DLL Export Definition
    #else
        #define GPS_PLUGIN_API extern "C" __declspec( dllimport ) ///< DLL Import Definition
    #endif
#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS)
    #ifdef GPS_PLUGIN_STATIC
        #define GPS_PLUGIN_API extern "C" __attribute__ ((visibility ("default"))) ///< DLL definition
    #else
        #define GPS_PLUGIN_API
    #endif 
#else
    #error Unknown build target! No valid value for AMDT_BUILD_TARGET.
#endif

#endif // EXPORT_DEFINITIONS_H
