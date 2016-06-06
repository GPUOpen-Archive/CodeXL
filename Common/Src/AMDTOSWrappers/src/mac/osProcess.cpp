//------------------------------ osProcess.cpp ------------------------------

//#define DEBUG 1
#ifdef _GR_IPHONE_BUILD
    // iPhone uses CFNetwork instead of Carbon
    #include <CFNetwork/CFNetwork.h>
#else
    // Carbon:
    #include <Carbon/Carbon.h>
#endif

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osProcess.h>


// ---------------------------------------------------------------------------
// Name:        osShowOrHideProcess
// Description: Shows or hides a given process.
// Arguments: processId - The given process id.
//            showProcess - true - show the process.
//                          false - hide the process.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/12/2008
// ---------------------------------------------------------------------------
bool osShowOrHideProcess(osProcessId processId, bool showProcess)
{
    bool retVal = false;

    // GUI functions are irrelevant to the iPhone
#ifndef _GR_IPHONE_BUILD
    // Transform the input pid into a ProcessSerialNumber:
    ProcessSerialNumber processSN;
    OSStatus rc1 = GetProcessForPID(processId, &processSN);
    GT_IF_WITH_ASSERT(rc1 == noErr)
    {
        // Show or hide the process:
        OSErr rc2 = ShowHideProcess(&processSN, showProcess);
        GT_IF_WITH_ASSERT(rc2 == noErr)
        {
            retVal = true;
        }
    }
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osTransformProcessToForegroundApplication
// Description: Transforms a process into a "Foreground process".
//              (A process that gets keyboard focus, etc)
// Arguments: processId - The input process id.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/12/2008
// ---------------------------------------------------------------------------
bool osTransformProcessToForegroundApplication(osProcessId processId)
{
    bool retVal = false;
    // GUI functions are irrelevant to the iPhone
#ifndef _GR_IPHONE_BUILD
    // Transform the input pid into a ProcessSerialNumber:
    ProcessSerialNumber processSN;
    OSStatus rc1 = GetProcessForPID(processId, &processSN);
    GT_IF_WITH_ASSERT(rc1 == noErr)
    {
        // Transform the process to a foreground application:
        OSErr rc2 = TransformProcessType(&processSN, kProcessTransformToForegroundApplication);
        GT_IF_WITH_ASSERT(rc2 == noErr)
        {
            retVal = true;
        }
    }
#endif

    return retVal;
}

