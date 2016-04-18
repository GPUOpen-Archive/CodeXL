//=====================================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief Defines the name of the DLL
//=====================================================================================

// MicroDLL is renamed for inclusion in CodeXL.  Since it is still
// referenced in legacy PerfStudio code which we don't want to change
// we need to maintain both forms of the name.
// This file defines a few macros that simplify this process

#ifndef GPS_MICRODLLNAME_INCLUDED
#define GPS_MICRODLLNAME_INCLUDED

#ifdef CODEXL_GRAPHICS
    #define MICRODLLNAME "CXLGraphicsServerInterceptor" ///< Micro dll name for CodeXL
    #define PERFSERVERNAME "CXLGraphicsServer" ///< Server name for CodeXL
#else // CODEXL_GRAPHICS
    #define MICRODLLNAME "MicroDLL" ///< Micro dll name for GPU PerfStudio
    #define PERFSERVERNAME "GPUPerfServer" ///< Server name for PerfStudio
#endif // CODEXL_GRAPHICS

#endif // GPS_MICRODLLNAME_INCLUDED
