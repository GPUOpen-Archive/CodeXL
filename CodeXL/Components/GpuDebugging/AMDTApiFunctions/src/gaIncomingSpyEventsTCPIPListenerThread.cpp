//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaIncomingSpyEventsTCPIPListenerThread.cpp
///
//==================================================================================

//------------------------------ gaIncomingSpyEventsTCPIPListenerThread.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osTCPSocketServer.h>
#include <AMDTOSWrappers/Include/osTCPSocketServerConnectionHandler.h>

// Local:
#include <src/gaIncomingSpyEventsTCPIPListenerThread.h>

// ---------------------------------------------------------------------------
// Name:        gaIncomingSpyEventsTCPIPListenerThread::gaIncomingSpyEventsTCPIPListenerThread
// Description: Constructor
// Author:      Uri Shomroni
// Date:        29/10/2009
// ---------------------------------------------------------------------------
gaIncomingSpyEventsTCPIPListenerThread::gaIncomingSpyEventsTCPIPListenerThread(const osPortAddress& portAddress)
    : gaIncomingSpyEventsListenerThread(), _pTCPSocketServer(NULL)
{
    // Create and socket server:
    _pTCPSocketServer = new osTCPSocketServer;


    _pTCPSocketServer->setReadOperationTimeOut(OS_CHANNEL_INFINIT_TIME_OUT);

    bool connectionSuccessful = false;

    // Open it:
    bool rcOpen = _pTCPSocketServer->open();
    GT_IF_WITH_ASSERT(rcOpen)
    {
        // Bind the address:
        bool rcBind = _pTCPSocketServer->bind(portAddress);
        GT_IF_WITH_ASSERT(rcBind)
        {
            // Listen to the port:
            bool rcListen = _pTCPSocketServer->listen(1);
            GT_ASSERT(rcListen);

            connectionSuccessful = rcListen;
        }

        // Close the connection if we failed here:
        if (!connectionSuccessful)
        {
            _pTCPSocketServer->close();
        }
    }

    // Failure cleanup:
    if (!connectionSuccessful)
    {
        delete _pTCPSocketServer;
        _pTCPSocketServer = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaIncomingSpyEventsTCPIPListenerThread::~gaIncomingSpyEventsTCPIPListenerThread
// Description: Destructor
// Author:      Uri Shomroni
// Date:        29/10/2009
// ---------------------------------------------------------------------------
gaIncomingSpyEventsTCPIPListenerThread::~gaIncomingSpyEventsTCPIPListenerThread()
{
    // Terminate the connection, if our parent class hasn't done so yet:
    terminateConnection();
}

// ---------------------------------------------------------------------------
// Name:        gaIncomingSpyEventsTCPIPListenerThread::connectAndWaitForClient
// Description: Opens the connection socket and waits for the client to connect
//              to us:
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        29/10/2009
// ---------------------------------------------------------------------------
bool gaIncomingSpyEventsTCPIPListenerThread::connectAndWaitForClient()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pTCPSocketServer != NULL)
    {
        osTCPSocketServerConnectionHandler* pConnectionHandler = new osTCPSocketServerConnectionHandler;


        pConnectionHandler->setReadOperationTimeOut(LONG_MAX);

        bool rcAccept = _pTCPSocketServer->accept(*pConnectionHandler);
        GT_IF_WITH_ASSERT(rcAccept)
        {
            // We succeeded, use the socket we opened:
            GT_ASSERT(_pIncomingEventsSocket == NULL);
            _pIncomingEventsSocket = pConnectionHandler;
            retVal = true;
        }
        else
        {
            delete pConnectionHandler;
        }
    }

    // Failure cleanup:
    if (!retVal)
    {
        delete _pTCPSocketServer;
        _pTCPSocketServer = NULL;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaIncomingSpyEventsTCPIPListenerThread::terminateConnection
// Description: Terminate the connection with the spy
// Author:      Uri Shomroni
// Date:        29/10/2009
// ---------------------------------------------------------------------------
void gaIncomingSpyEventsTCPIPListenerThread::terminateConnection()
{
    // Close the connection channel:
    if (_pIncomingEventsSocket != NULL)
    {
        if (_pIncomingEventsSocket->isOpen())
        {
            _pIncomingEventsSocket->close();
        }

        // The base class will delete _pIncomingEventsSocket.
    }

    // Close and destroy the TCP socket server:
    if (_pTCPSocketServer != NULL)
    {
        if (_pTCPSocketServer->isOpen())
        {
            _pTCPSocketServer->close();
        }

        delete _pTCPSocketServer;
        _pTCPSocketServer = NULL;
    }
}

