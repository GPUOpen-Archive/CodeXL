//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdLauncherProcessWatcherThread.h
///
//==================================================================================

//------------------------------ pdLauncherProcessWatcherThread.h ------------------------------

#ifndef __PDLAUNCHERPROCESSWATCHERTHREAD_H
#define __PDLAUNCHERPROCESSWATCHERTHREAD_H

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>



// ----------------------------------------------------------------------------------
// Class Name:           pdLauncherProcessWatcherThread : public apIEventsObserver, public osThread
// General Description: A thread used to monitor a process which launches our debugged
//                      process - such as the iPhone simulator.
//                      This thread triggers a termination event when the launcher dies,
//                      and kills the launcher when the debugged app dies.
// Author:               Uri Shomroni
// Creation Date:        11/5/2009
// ----------------------------------------------------------------------------------
class pdLauncherProcessWatcherThread : public apIEventsObserver, public osThread
{
public:
    pdLauncherProcessWatcherThread(osProcessId launcherProcessId);
    ~pdLauncherProcessWatcherThread();

    // Overrides osThread
    virtual int entryPoint();
    virtual void beforeTermination();

    // Overrides apIEventsObserver
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);

private:
    // Disallow use of my default constructor:
    pdLauncherProcessWatcherThread();

    // The process Id of the launcher process:
    osProcessId _launcherProcessId;
};

#endif //__PDLAUNCHERPROCESSWATCHERTHREAD_H

