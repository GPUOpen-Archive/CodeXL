//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains the entry point for the MicroDLL plugin.
//==============================================================================

#ifndef _MICRODLL_H_
#define _MICRODLL_H_

#include <windows.h>
#include <string>

/// \defgroup MicroDLL MicroDLL
/// This module handles the interception of the OpenCL and DX dll from the application.
/// Based on the dll loaded by the app, the module will load the appropriate backend server
/// to handle code injection and detouring for the specific API.
///
/// \ingroup Backend
// @{

#define MICRODLL_API __declspec( dllexport )

extern std::string g_strMicroDllPath;
extern std::string g_strOutputFile;
extern std::string g_strDLLPath;
extern std::string g_strCounterFile;

/// Get a string containing the MicroDLL string name.  We export
/// this function so that at least one function is exported for a DLL.
/// \return info string
MICRODLL_API const char* GetInfo();

// @}

#endif // _MICRODLL_H_
