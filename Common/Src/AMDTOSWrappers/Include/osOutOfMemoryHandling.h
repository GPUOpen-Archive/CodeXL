//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osOutOfMemoryHandling.h
///
//=====================================================================
#ifndef __osOutOfMemoryHandling_h
#define __osOutOfMemoryHandling_h
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osOSWrappersDLLBuild.h>

// This function extracts the current thread's call stack, writes it down to the log
// (using AMDTOSWrappers log facilities), and terminates the current process.
OS_API void osDumpCallStackAndExit();

#endif // osOutOfMemoryHandling_h__
