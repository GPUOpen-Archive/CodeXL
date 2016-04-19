//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnConnectionWatcherThread.cpp
///
//==================================================================================

#include "dmnConnectionWatcherThread.h"
#include <AMDTBaseTools/Include/gtAssert.h>

dmnConnectionWatcherThread::dmnConnectionWatcherThread(const gtString& watcherThreadName, osTCPSocketServerConnectionHandler* pConnectionToWatch, const gtVector<osProcessId>& parentProcesses) :
    osThread(watcherThreadName), m_parentProcesses(parentProcesses), m_pConnectionToWatch(pConnectionToWatch), m_isConnectionBroken(false)
{
    GT_ASSERT(pConnectionToWatch != NULL);
}


dmnConnectionWatcherThread::~dmnConnectionWatcherThread()
{
}

int dmnConnectionWatcherThread::entryPoint()
{
    GT_IF_WITH_ASSERT((m_pConnectionToWatch != NULL))
    {
        // Block as long as the connection is alive.
        bool isAlive = false;
        bool isOk = m_pConnectionToWatch->isConnectionAlive(isAlive);

        GT_IF_WITH_ASSERT(isOk)
        {
            m_isConnectionBroken = !isAlive;

            if (m_isConnectionBroken)
            {
                // Terminate the debugged/profiled application, whose parent
                // should be in the m_processesToTerminate container.
                for (size_t i = 0; i < m_parentProcesses.size(); ++i)
                {
                    const osProcessId currProcId = m_parentProcesses[i];

                    if (currProcId > 0)
                    {
                        osTerminateChildren(currProcId);
                    }
                }
            }
        }
    }

    return 0;
}

bool dmnConnectionWatcherThread::isConnectionBroken() const
{
    return m_isConnectionBroken;
}
