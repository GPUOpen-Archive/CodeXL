//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnAppWatcherThread.cpp
///
//==================================================================================

#include "dmnAppWatcherThread.h"
#include <AMDTBaseTools/Include/gtAssert.h>

dmnAppWatcherThread::dmnAppWatcherThread(const gtString& watcherThreadName,
                                         osProcessId processToWatch) : osThread(watcherThreadName), m_procToWatch(processToWatch)
{
}

dmnAppWatcherThread::~dmnAppWatcherThread()
{
}

int dmnAppWatcherThread::entryPoint()
{
    // Wait as long as the target process is alive.
    long exitCode = 0;
    osWaitForProcessToTerminate(m_procToWatch, ULONG_MAX, &exitCode);

    // Notify the observers about the app's termination.
    for (IAppWatcherObserver* pObserver : m_observers)
    {
        if (pObserver != nullptr)
        {
            pObserver->onAppTerminated();
        }
    }

    return 0;
}

void dmnAppWatcherThread::registerToEventNotification(IAppWatcherObserver* pObserver)
{
    m_observers.push_back(pObserver);
}
