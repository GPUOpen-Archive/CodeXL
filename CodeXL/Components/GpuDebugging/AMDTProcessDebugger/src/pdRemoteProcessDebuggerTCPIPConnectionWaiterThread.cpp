//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdRemoteProcessDebuggerTCPIPConnectionWaiterThread.cpp
///
//==================================================================================

//------------------------------ pdRemoteProcessDebuggerTCPIPConnectionWaiterThread.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osTCPSocketServer.h>
#include <AMDTOSWrappers/Include/osTCPSocketServerConnectionHandler.h>

// Local:
#include <src/pdRemoteProcessDebuggerTCPIPConnectionWaiterThread.h>
#include <src/pdRemoteProcessDebugger.h>


#define PD_REMOTE_PROCESS_DEBUGGER_TCP_IP_CONNECTION_WAITER_PARTIAL_TIMEOUT 500

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebuggerTCPIPConnectionWaiterThread::pdRemoteProcessDebuggerTCPIPConnectionWaiterThread
// Description: Constructor
// Author:      Uri Shomroni
// Date:        6/10/2013
// ---------------------------------------------------------------------------
pdRemoteProcessDebuggerTCPIPConnectionWaiterThread::pdRemoteProcessDebuggerTCPIPConnectionWaiterThread(pdRemoteProcessDebugger& i_controllingProcessDebugger, osTCPSocketServer& i_listeningSocketServer, osTCPSocketServerConnectionHandler& o_acceptingSocketServerConnectionHandler)
    : osThread(L"pdRemoteProcessDebuggerTCPIPConnectionWaiterThread"), m_controllingProcessDebugger(i_controllingProcessDebugger),
      m_listeningSocketServer(i_listeningSocketServer), m_acceptingSocketServerConnectionHandler(o_acceptingSocketServerConnectionHandler),
      m_isWaitingForConnection(true), m_wasConnectionSuccessful(false)
{

}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebuggerTCPIPConnectionWaiterThread::~pdRemoteProcessDebuggerTCPIPConnectionWaiterThread
// Description: Destructor
// Author:      Uri Shomroni
// Date:        6/10/2013
// ---------------------------------------------------------------------------
pdRemoteProcessDebuggerTCPIPConnectionWaiterThread::~pdRemoteProcessDebuggerTCPIPConnectionWaiterThread()
{

}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebuggerTCPIPConnectionWaiterThread::waitForConnection
// Description: Waits for the thread to finish. Note that the return value does not indicate
//              a successful connection, only a successful wait (i.e. no time out)
// Author:      Uri Shomroni
// Date:        6/10/2013
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebuggerTCPIPConnectionWaiterThread::waitForConnection(unsigned long timeoutMSec)
{
    bool retVal = false;
    bool goOn = true;
    unsigned long remainingTimeOutMSec = timeoutMSec;

    while (goOn)
    {
        // Wait in increments of PD_..._PARTIAL_TIMEOUT:
        if (remainingTimeOutMSec > PD_REMOTE_PROCESS_DEBUGGER_TCP_IP_CONNECTION_WAITER_PARTIAL_TIMEOUT)
        {
            // Wait and decrease by the partial timeout:
            retVal = osWaitForFlagToTurnOff(m_isWaitingForConnection, PD_REMOTE_PROCESS_DEBUGGER_TCP_IP_CONNECTION_WAITER_PARTIAL_TIMEOUT);
            remainingTimeOutMSec -= PD_REMOTE_PROCESS_DEBUGGER_TCP_IP_CONNECTION_WAITER_PARTIAL_TIMEOUT;

            // Stop if we succeeded:
            goOn = goOn && (!retVal);
        }
        else // remainingTimeOutMSec <= PD_REMOTE_PROCESS_DEBUGGER_TCP_IP_CONNECTION_WAITER_PARTIAL_TIMEOUT
        {
            // Wait for the remaining amount:
            retVal = osWaitForFlagToTurnOff(m_isWaitingForConnection, remainingTimeOutMSec);

            // Either way, we timed out, we can stop:
            remainingTimeOutMSec = 0;
            goOn = false;
        }

        // After every partial timeout, check if the RDS is still alive (otherwise, it exited without connecting to us, and we need to exit)
        if (!m_controllingProcessDebugger.isRemoteDebuggingServerAlive(false, true))
        {
            goOn = false;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebuggerTCPIPConnectionWaiterThread::wasConnectionSuccessful
// Description: Query did the connection succeed:
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        6/10/2013
// ---------------------------------------------------------------------------
bool pdRemoteProcessDebuggerTCPIPConnectionWaiterThread::wasConnectionSuccessful()
{
    return m_wasConnectionSuccessful;
}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebuggerTCPIPConnectionWaiterThread::entryPoint
// Description: The thread entry point function. Waits for a connection from the TCP server
// Author:      Uri Shomroni
// Date:        6/10/2013
// ---------------------------------------------------------------------------
int pdRemoteProcessDebuggerTCPIPConnectionWaiterThread::entryPoint()
{
    int retVal = 0;

    if (m_listeningSocketServer.isOpen())
    {
        m_wasConnectionSuccessful = m_listeningSocketServer.accept(m_acceptingSocketServerConnectionHandler);
    }

    m_isWaitingForConnection = false;

    return retVal;
}



