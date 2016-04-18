//=====================================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc.
/// \author GPU Developer Tools
/// \file
/// \brief Exports for the DXGI plugin dll
//=====================================================================================

#ifndef GPS_DXGIDLLMAIN_H
#define GPS_DXGIDLLMAIN_H

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the DX9WRAPPER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// DXGIWRAPPER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef DXGIWRAPPER_EXPORTS
    #define DXGIWRAPPER_API __declspec( dllexport ) ///< define export funtion
#else
    #define DXGIWRAPPER_API __declspec( dllimport ) ///< define export funtion
#endif

#endif // GPS_DX11DLLMAIN_H
