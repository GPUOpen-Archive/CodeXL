//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file dmnConnectionWatcherThread.h
///
//==================================================================================

#pragma once
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osTCPSocketServerConnectionHandler.h>

class dmnConnectionWatcherThread :
    public osThread
{
public:
    dmnConnectionWatcherThread(const gtString& watcherThreadName,
                               osTCPSocketServerConnectionHandler* pConnectionToWatch, const gtVector<osProcessId>& connectionRelatedProcesses);
    virtual ~dmnConnectionWatcherThread();
    virtual int entryPoint() override;

    // Returns true if the watched connection has been broken.
    bool isConnectionBroken() const;

private:

    // Holds the processes whose children should be terminated if the connection is no longer alive (rcprof, RDS).
    gtVector<osProcessId> m_parentProcesses;

    // The TCP connection which is being watched.
    osTCPSocketServerConnectionHandler* m_pConnectionToWatch;

    // A flag stating whether the connection was found to be alive broken.
    bool m_isConnectionBroken;
};

