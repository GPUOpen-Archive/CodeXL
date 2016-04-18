//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdLauncherProcessWatcherThread.cpp
///
//==================================================================================

//------------------------------ pdLauncherProcessWatcherThread.cpp ------------------------------

// POSIX:
#include <signal.h>
#include <sys/wait.h>

// Infra:
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessTerminatedEvent.h>

// Local:
#include <src/pdLauncherProcessWatcherThread.h>


// ---------------------------------------------------------------------------
// Name:        pdLauncherProcessWatcherThread::pdLauncherProcessWatcherThread
// Description: Constructor
// Author:      Uri Shomroni
// Date:        11/5/2009
// ---------------------------------------------------------------------------
pdLauncherProcessWatcherThread::pdLauncherProcessWatcherThread(osProcessId launcherProcessId)
    : osThread(L"pdLauncherProcessWatcherThread"), _launcherProcessId(launcherProcessId)
{
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);
}

// ---------------------------------------------------------------------------
// Name:        pdLauncherProcessWatcherThread::~pdLauncherProcessWatcherThread
// Description: Destructor
// Author:      Uri Shomroni
// Date:        11/5/2009
// ---------------------------------------------------------------------------
pdLauncherProcessWatcherThread::~pdLauncherProcessWatcherThread()
{
    // If the iPhone simulator still exists, kill it:
    ::kill(_launcherProcessId, SIGTERM);

    apEventsHandler::instance().unregisterEventsObserver(*this);
}

// ---------------------------------------------------------------------------
// Name:        pdLauncherProcessWatcherThread::entryPoint
// Description: Entry point for the watcher thread
// Author:      Uri Shomroni
// Date:        11/5/2009
// ---------------------------------------------------------------------------
int pdLauncherProcessWatcherThread::entryPoint()
{
    // Wait for the launcher to exit:
    ::waitpid(_launcherProcessId, NULL, 0);

    // Notify CodeXL that the launcher application died:
    apDebuggedProcessTerminatedEvent eve(0);
    apEventsHandler::instance().registerPendingDebugEvent(eve);

    return 0;
}

// ---------------------------------------------------------------------------
// Name:        pdLauncherProcessWatcherThread::beforeTermination
// Description: Called before the thread is terminated
// Author:      Uri Shomroni
// Date:        11/5/2009
// ---------------------------------------------------------------------------
void pdLauncherProcessWatcherThread::beforeTermination()
{
    // Do nothing for now...
}

// ---------------------------------------------------------------------------
// Name:        pdLauncherProcessWatcherThread::onEvent
// Description: Called when a debugged process event occurs
// Author:      Uri Shomroni
// Date:        11/5/2009
// ---------------------------------------------------------------------------
void pdLauncherProcessWatcherThread::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent); // unused

    if (eve.eventType() == apEvent::AP_DEBUGGED_PROCESS_TERMINATED)
    {
        // If the debugged process was terminated, kill the launcher process:
        ::kill(_launcherProcessId, SIGTERM);
    }
}

