//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdDebuggedProcessWatcherThread.cpp
///
//==================================================================================

//------------------------------ pdDebuggedProcessWatcherThread.cpp ------------------------------

// Signalling (kill):
#include <signal.h>

// Infra:
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>

// Local:
#include <src/pdDebuggedProcessWatcherThread.h>
#include <src/pdGDBDriver.h>
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>


// ---------------------------------------------------------------------------
// Name:        pdDebuggedProcessWatcherThread::pdDebuggedProcessWatcherThread
// Description: Constructor
// Arguments: debuggedProcessPID - the PID to watch
//            rGDBDriver - the driver to flush when the PID dies
// Author:      Uri Shomroni
// Date:        2/6/2009
// ---------------------------------------------------------------------------
pdDebuggedProcessWatcherThread::pdDebuggedProcessWatcherThread(osProcessId debuggedProcessPID, pdGDBDriver& rGDBDriver)
    : osThread(L"pdDebuggedProcessWatcherThread"), _debuggedProcessPID(debuggedProcessPID), _gdbDriver(rGDBDriver)
{

}

// ---------------------------------------------------------------------------
// Name:        pdDebuggedProcessWatcherThread::~pdDebuggedProcessWatcherThread
// Description: Destructor
// Author:      Uri Shomroni
// Date:        2/6/2009
// ---------------------------------------------------------------------------
pdDebuggedProcessWatcherThread::~pdDebuggedProcessWatcherThread()
{

}

// ---------------------------------------------------------------------------
// Name:        pdDebuggedProcessWatcherThread::entryPoint
// Description: The thread's main run: watch the debugged process and report
//              when it dies.
// Author:      Uri Shomroni
// Date:        2/6/2009
// ---------------------------------------------------------------------------
int pdDebuggedProcessWatcherThread::entryPoint()
{
    // Wait for the debugged process to exit:
    for (;;)
    {
        osSleep(500);

        if (::kill(_debuggedProcessPID, 0) != 0)
        {
            break;
        }
    }

    // Make sure we don't already know the process exited:
    //if (pdProcessDebugger::instance().debuggedProcessExists())
    {
        // Send an empty command to GDB so it will flush out the exit data:
        _gdbDriver.flushCommandOutput();
    }

    return 0;
}

// ---------------------------------------------------------------------------
// Name:        pdDebuggedProcessWatcherThread::beforeTermination
// Description: Called before this thread is terminated
// Author:      Uri Shomroni
// Date:        2/6/2009
// ---------------------------------------------------------------------------
void pdDebuggedProcessWatcherThread::beforeTermination()
{
}

