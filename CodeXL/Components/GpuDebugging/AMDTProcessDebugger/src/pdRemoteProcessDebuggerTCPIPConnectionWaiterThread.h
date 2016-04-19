//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdRemoteProcessDebuggerTCPIPConnectionWaiterThread.h
///
//==================================================================================

//------------------------------ pdRemoteProcessDebuggerTCPIPConnectionWaiterThread.h ------------------------------

#ifndef __PDREMOTEPROCESSDEBUGGERTCPIPCONNECTIONWAITERTHREAD_H
#define __PDREMOTEPROCESSDEBUGGERTCPIPCONNECTIONWAITERTHREAD_H

// Forward declarations:
class osTCPSocketServer;
class osTCPSocketServerConnectionHandler;
class pdRemoteProcessDebugger;

// Infra:
#include <AMDTOSWrappers/Include/osThread.h>

// ----------------------------------------------------------------------------------
// Class Name:         pdRemoteProcessDebuggerTCPIPConnectionWaiterThread : public osThread
// General Description: A thread that waits on an incoming TCP/IP connection.
// Author:             Uri Shomroni
// Creation Date:      6/10/2013
// ----------------------------------------------------------------------------------
class pdRemoteProcessDebuggerTCPIPConnectionWaiterThread : public osThread
{
public:
    pdRemoteProcessDebuggerTCPIPConnectionWaiterThread(pdRemoteProcessDebugger& i_controllingProcessDebugger, osTCPSocketServer& i_listeningSocketServer, osTCPSocketServerConnectionHandler& o_acceptingSocketServerConnectionHandler);
    virtual ~pdRemoteProcessDebuggerTCPIPConnectionWaiterThread();

    bool waitForConnection(unsigned long timeoutMSec);
    bool wasConnectionSuccessful();

protected:
    virtual int entryPoint();

private:
    // Disallow use of default constructor, copy constructor and assignment operator:
    pdRemoteProcessDebuggerTCPIPConnectionWaiterThread() = delete;
    pdRemoteProcessDebuggerTCPIPConnectionWaiterThread(const pdRemoteProcessDebuggerTCPIPConnectionWaiterThread&) = delete;
    pdRemoteProcessDebuggerTCPIPConnectionWaiterThread& operator=(const pdRemoteProcessDebuggerTCPIPConnectionWaiterThread&) = delete;

private:
    pdRemoteProcessDebugger& m_controllingProcessDebugger;
    osTCPSocketServer& m_listeningSocketServer;
    osTCPSocketServerConnectionHandler& m_acceptingSocketServerConnectionHandler;

    bool m_isWaitingForConnection;
    bool m_wasConnectionSuccessful;
};

#endif //__PDREMOTEPROCESSDEBUGGERTCPIPCONNECTIONWAITERTHREAD_H

