//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apExecutionMode.h
///
//==================================================================================

//------------------------------ apExecutionMode.h ------------------------------

#ifndef __APEXECUTIONMODE
#define __APEXECUTIONMODE

// Describes the CodeXL execution mode: Debug /Profiling / Redundant state change:
typedef enum
{
    AP_DEBUGGING_MODE                           = 0,            // Debugging execution mode (default mode)
    AP_PROFILING_MODE                           = 1,            // Profiling mode
    AP_ANALYZE_MODE                             = 2             // Mode for redundant state debugging
} apExecutionMode;


#endif  // __APEXECUTIONMODE
