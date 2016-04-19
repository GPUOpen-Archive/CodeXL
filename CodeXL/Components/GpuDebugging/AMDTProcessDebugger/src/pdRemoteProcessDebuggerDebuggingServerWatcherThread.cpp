//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdRemoteProcessDebuggerDebuggingServerWatcherThread.cpp
///
//==================================================================================

//------------------------------ pdRemoteProcessDebuggerDebuggingServerWatcherThread.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessTerminatedEvent.h>

// Local:
#include <src/pdRemoteProcessDebuggerDebuggingServerWatcherThread.h>

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebuggerDebuggingServerWatcherThread::pdRemoteProcessDebuggerDebuggingServerWatcherThread
// Description: Constructor
// Author:      Uri Shomroni
// Date:        16/8/2009
// ---------------------------------------------------------------------------
pdRemoteProcessDebuggerDebuggingServerWatcherThread::pdRemoteProcessDebuggerDebuggingServerWatcherThread()
    : osThread(L"pdRemoteProcessDebuggerDebuggingServerWatcherThread"), _waitingForNewServer(true), _localMachineDebuggingServerProcId(0)
{

}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebuggerDebuggingServerWatcherThread::~pdRemoteProcessDebuggerDebuggingServerWatcherThread
// Description: Destructor
// Author:      Uri Shomroni
// Date:        16/8/2009
// ---------------------------------------------------------------------------
pdRemoteProcessDebuggerDebuggingServerWatcherThread::~pdRemoteProcessDebuggerDebuggingServerWatcherThread()
{
    terminateDebuggingServer();
}

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #pragma warning(push)
    #pragma warning(disable: 4702)
#endif
// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebuggerDebuggingServerWatcherThread::entryPoint
// Description: The thread's main function, watches the given remote debugging
//              server, notifies the main thread if it crashes and kills it if
//              the debugged process exits.
// Author:      Uri Shomroni
// Date:        16/8/2009
// ---------------------------------------------------------------------------
int pdRemoteProcessDebuggerDebuggingServerWatcherThread::entryPoint()
{
    int retVal = 0;

    // Loop indefinitely:
    for (;;)
    {
        // Wait until we are notified to watch a debugging server:
        osWaitForFlagToTurnOff(_waitingForNewServer, ULONG_MAX);

        //if we've got a process id:
        if (_localMachineDebuggingServerProcId != 0)
        {
            // Wait for the process to exit:
            bool rcWait = osWaitForProcessToTerminate(_localMachineDebuggingServerProcId, ULONG_MAX);

            if (rcWait)
            {
                // The process exited (crashed?) - report to the application:
                // Note: if we got an exit code from osWaitForProcessToTerminate, it would be
                //      the debugging server's code and not the debugged process's one.
                apDebuggedProcessTerminatedEvent eve(0);

                apEventsHandler::instance().registerPendingDebugEvent(eve);

                // Wait for a new server to watch:
                _waitingForNewServer = true;
                _localMachineDebuggingServerProcId = 0;
            }
            else
            {
                // Waiting failed, wait for the app to tell us to wait again:
                _waitingForNewServer = true;
                _localMachineDebuggingServerProcId = 0;
                GT_ASSERT(rcWait);
            }
        }
        else
        {
            // We shouldn't get here:
            _waitingForNewServer = true;
            GT_ASSERT(false);
        }
    }

    return retVal;
}
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #pragma warning(pop)
#endif

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebuggerDebuggingServerWatcherThread::beforeTermination
// Description: Called before this thread is terminated
// Author:      Uri Shomroni
// Date:        16/8/2009
// ---------------------------------------------------------------------------
void pdRemoteProcessDebuggerDebuggingServerWatcherThread::beforeTermination()
{
    // If this thread is being terminated, kill the debugging server:
    terminateDebuggingServer();
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebuggerDebuggingServerWatcherThread::monitorLocalMachineDebuggingServer
// Description: Starts monitoring a remote debugging server on the local machine
// Author:      Uri Shomroni
// Date:        16/8/2009
// ---------------------------------------------------------------------------
void pdRemoteProcessDebuggerDebuggingServerWatcherThread::monitorLocalMachineDebuggingServer(osProcessId debuggingServerProcID)
{
    // Terminate any previous server we were watching:
    terminateDebuggingServer();

    // Set the server parameters before changing the waiting flag, as that will
    // release the thread's main loop:
    _localMachineDebuggingServerProcId = debuggingServerProcID;

    // Start monitoring:
    _waitingForNewServer = false;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebuggerDebuggingServerWatcherThread::stopMonitoringDebuggingServer
// Description: Stop watching the debugging server and wait for a new one to be ser
// Author:      Uri Shomroni
// Date:        16/8/2009
// ---------------------------------------------------------------------------
void pdRemoteProcessDebuggerDebuggingServerWatcherThread::stopMonitoringDebuggingServer()
{
    terminateDebuggingServer();
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebuggerDebuggingServerWatcherThread::terminateDebuggingServer
// Description: Terminates the watched debugging server and resets the thread to wait
// Author:      Uri Shomroni
// Date:        16/8/2009
// ---------------------------------------------------------------------------
void pdRemoteProcessDebuggerDebuggingServerWatcherThread::terminateDebuggingServer()
{
    // Reset the flag before destroying the server, as the server termination will
    // release the thread's main loop:
    _waitingForNewServer = true;

    if (_localMachineDebuggingServerProcId != 0)
    {
        // Terminate the process:
        bool rcTerminated = osTerminateProcess(_localMachineDebuggingServerProcId);
        GT_IF_WITH_ASSERT(rcTerminated)
        {
            // Clear the process ID member:
            _localMachineDebuggingServerProcId = 0;
        }
    }
}

