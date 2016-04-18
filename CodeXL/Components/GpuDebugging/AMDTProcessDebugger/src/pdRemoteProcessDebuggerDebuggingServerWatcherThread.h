//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdRemoteProcessDebuggerDebuggingServerWatcherThread.h
///
//==================================================================================

//------------------------------ pdRemoteProcessDebuggerDebuggingServerWatcherThread.h ------------------------------

#ifndef __PDREMOTEPROCESSDEBUGGERDEBUGGINGSERVERWATCHERTHREAD_H
#define __PDREMOTEPROCESSDEBUGGERDEBUGGINGSERVERWATCHERTHREAD_H

// Infra:
#include <AMDTOSWrappers/Include/osThread.h>


// ----------------------------------------------------------------------------------
// Class Name:          pdRemoteProcessDebuggerDebuggingServerWatcherThread : public osThread
// General Description: A thread to monitor the remote debugging server, and terminate it when
//                      the debugged process is terminated.
// Author:              Uri Shomroni
// Creation Date:       16/8/2009
// ----------------------------------------------------------------------------------
class pdRemoteProcessDebuggerDebuggingServerWatcherThread : public osThread
{
public:
    pdRemoteProcessDebuggerDebuggingServerWatcherThread();
    ~pdRemoteProcessDebuggerDebuggingServerWatcherThread();

    // Overrides osThread:
    virtual int entryPoint();
    virtual void beforeTermination();

    // Used to start and stop monitoring debugging servers:
    void monitorLocalMachineDebuggingServer(osProcessId debuggingServerProcID);
    void stopMonitoringDebuggingServer();

    // Are we currently watching a process:
    bool isMonitoringRemoteDebuggingServer() const {return (0 != _localMachineDebuggingServerProcId);};

private:
    void terminateDebuggingServer();

private:
    bool _waitingForNewServer;

    osProcessId _localMachineDebuggingServerProcId;
};

#endif //__PDREMOTEPROCESSDEBUGGERDEBUGGINGSERVERWATCHERTHREAD_H

