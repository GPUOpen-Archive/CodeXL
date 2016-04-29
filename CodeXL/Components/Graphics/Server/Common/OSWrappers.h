//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  OS-Specific wrapper functions. Use a generic platform-agnostic function
///         to perform OS-Specific functionality
//==============================================================================

#ifndef GPS_OSWRAPPERS_H
#define GPS_OSWRAPPERS_H

#include <AMDTOSWrappers/Include/osModuleArchitecture.h>
#include <AMDTOSWrappers/Include/osThread.h>

#ifdef _LINUX
#include "Linux/WinDefs.h"
#endif

namespace OSWrappers
{
/// Get the executable type (whether it is 32 or 64 bit)
/// \param moduleName the name of the executable to test
/// \param arch the executable file type returned if the module
/// name is a valid executable
/// \return true if successful, false if error.
bool GetBinaryType(const char* moduleName, osModuleArchitecture* arch);

/// Get the current thread handle
/// \return the OS-specific current thread handle
osThreadHandle GetCurrentThreadHandle();

/// Retrieves the current value of the high-resolution performance counter
/// \param lpPerformanceCount A pointer to a variable that receives the current performance-counter value, in counts
/// \return true if successful, false if error
bool QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount);

/// The standard message box
int MessageBox(const char* text, const char* caption, unsigned int flags);

/// Emit a GPUView event or something similar for Linux
void EmitSystemEvent();

//--------------------------------------------------------------------------
/// Write basic system and device info into the incoming string.
/// \returns The string to be filled with formatted system information.
//--------------------------------------------------------------------------
void WriteSystemInfoString(std::string& outSystemInfo);

//--------------------------------------------------------------------------
/// Query if a process is running in the system
/// \param szProcessName the name of the process to query
/// \param useSubString indicates whether szProcessName is the full process
/// or a substring. If it's a substring, a substring comparison will be used
/// on the process name. Useful when certain processes contain platform and
/// build information on the end (ie GPUPerfStudio-x64-d.exe)
/// \return true if the process specified is running, false if not
//--------------------------------------------------------------------------
bool IsProcessRunning(const char* szProcessName, bool useSubString);
}

#ifdef _LINUX
    void OutputDebugString(const char* lpOutputString);

    BOOL  QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency);
#endif

#endif //GPS_OSUTILS_H
