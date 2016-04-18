//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suSpyBreakpointImplementation.h
///
//==================================================================================

//------------------------------ suSpyBreakpointImplementation.h ------------------------------

#ifndef __GSSPYBREAKPOINTIMPLEMENTATION_H
#define __GSSPYBREAKPOINTIMPLEMENTATION_H

// Forward declarations:
class osSocket;

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osCondition.h>

// ----------------------------------------------------------------------------------
// Class Name:          suSpyBreakpointImplementation
// General Description: A singleton class used to implement breakpoints in the spy on
//                      platforms where we don't have access to the debugger (e.g.
//                      iPhone on-device). This class uses a spin lock-type mechanism
//                      to lock the threads while still allowing us to execute function
//                      as needed.
// Author:              Uri Shomroni
// Creation Date:       1/11/2009
// ----------------------------------------------------------------------------------
class suSpyBreakpointImplementation
{
public:
    suSpyBreakpointImplementation();
    ~suSpyBreakpointImplementation();

    static suSpyBreakpointImplementation& instance();

    // Operative functions:
    void breakpointImplementation();
    bool breakAllThreads();
    bool resumeAllThreads();
    bool makeThreadExecuteFunction(osThreadId threadID, osProcedureAddress functionAddress);

private:
    friend class suSingletonsDelete;

private:
    // My single instance
    static suSpyBreakpointImplementation* _pMySingleInstance;

    // Do we want to break right now?
    bool _isRunSuspended;

    // Used to make threads stop (instead of spinning in a loop, which eats up performance)
    osCondition _runSuspensionCondition;

    // Used to execute functions on specific threads:
    osThreadId _threadToExecuteFunction;
    osProcedureAddress _functionToExecute;
    bool _executedFunctionRetVal;
    bool _isCurrentlyExecutingFunction;
};

#endif //__GSSPYBREAKPOINTIMPLEMENTATION_H

