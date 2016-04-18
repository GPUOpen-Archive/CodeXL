//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnAppWatcherThread.h
///
//==================================================================================

#ifndef __dmnLaunchedAppWatcherThread_h
#define __dmnLaunchedAppWatcherThread_h

// Local.
#include <AMDTRemoteAgent/IAppWatcherObserver.h>

// Infra.
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osProcess.h>

class dmnAppWatcherThread :
    public osThread
{
public:

    // CTOR.
    dmnAppWatcherThread(const gtString& watcherThreadName, osProcessId processToWatch);

    // DTOR.
    virtual ~dmnAppWatcherThread();

    // Register to the events.
    void registerToEventNotification(IAppWatcherObserver* pObserver);

    // Thread entry point.
    virtual int entryPoint() override;

private:

    // The list of observers to be notified.
    gtList<IAppWatcherObserver*> m_observers;

    // The handle to the process to be watched.
    osProcessId m_procToWatch;
};

#endif // __dmnLaunchedAppWatcherThread_h
