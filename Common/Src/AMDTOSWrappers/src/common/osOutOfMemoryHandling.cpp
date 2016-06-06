//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osOutOfMemoryHandling.cpp
///
//=====================================================================
#include <AMDTOSWrappers/Include/osOutOfMemoryHandling.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osCallsStackReader.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTBaseTools/Include/gtAssert.h>


// ---------------------------------------------------------------------------
// Name:        osOutOfMemoryHelper
// Description: Auxiliary function that releases the reserved memory, extracts
//              the current thread's call stack, and writes down the current call
//              stack to the log, and then terminates the current process.
// Arguments:   None
// Author:      AMD Developer Tools Team
// Date:        2/2/2014
// ---------------------------------------------------------------------------
OS_API void osDumpCallStackAndExit()
{
    // First, free the reserved memory so that we don't get stuck.
    gtFreeReservedMemory();

    // Retrieve the call stack for the current thread.
    osCallsStackReader csReader;
    osCallStack csBuffer;
    bool isOk = csReader.getCurrentCallsStack(csBuffer, true, true);
    GT_ASSERT(isOk);

    // Extract a string representation of the call stack.
    gtString csFullString;
    gtString csBriefString;
    bool isSpyRelated = false;
    csBuffer.asString(csBriefString, csFullString, isSpyRelated);

    // Write down the call stack to the log.
    OS_OUTPUT_DEBUG_LOG(csFullString.asCharArray(), OS_DEBUG_LOG_ERROR);

    // Exit.
    osExitCurrentProcess(0xFFFFFFFFU);
}

