//==============================================================================
// Copyright (c) 2014 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Contains functions relating to process injection
//==============================================================================

#ifndef GPS_INJECT_INCLUDED
#define GPS_INJECT_INCLUDED

#include "../Common/misc.h"

/// define a vector of ProcessInfo strcuts
typedef std::vector<ProcessInfo> ProcessInfoList;

/// Gets a list of processes which have linked the specified library
/// \param szLibrary the library to search for
/// \param out the resulting list of processes
/// \return true if the library was found in at least one process; false otherwise
bool get_process_list(const char* szLibrary, std::vector< ProcessInfo >& out);

/// Determines if the specified library has been loaded into the specified process.
/// \param dwPID the process ID to check
/// \param szLibrary the library to look for
/// \param outExePath the path to the exe if the function returns true
/// \return true if the library is in the process; false otherwise
bool IsLibraryLoadedInProcess(unsigned long dwPID, const char* szLibrary, char* outExePath);

/// Terminates the process with the specified PID
/// \param pid the process ID of the process to terminate
/// \return true if the process was terminated; false otherwise
bool KillProcess(unsigned long pid);

#endif // GPS_INJECT_INCLUDED
