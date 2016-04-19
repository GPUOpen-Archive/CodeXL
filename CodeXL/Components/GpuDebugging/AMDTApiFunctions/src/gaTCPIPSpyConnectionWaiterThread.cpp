//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaTCPIPSpyConnectionWaiterThread.cpp
///
//==================================================================================

//------------------------------ gaTCPIPSpyConnectionWaiterThread.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osTCPSocketServer.h>

// Local:
#include <src/gaTCPIPSpyConnectionWaiterThread.h>


// ---------------------------------------------------------------------------
// Name:        gaTCPIPSpyConnectionWaiterThread::gaTCPIPSpyConnectionWaiterThread
// Description: Constructor.
// Arguments: socketServer - The socket server to which I serve as an execution
//                           thread.
//            connectionHandler - The socket connection that will be established
//                                with the spy.
//            APIConnectionType - The type of the API connection to which this thread waits for.
// Author:      Yaki Tebeka
// Date:        15/2/2004
// ---------------------------------------------------------------------------
gaTCPIPSpyConnectionWaiterThread::gaTCPIPSpyConnectionWaiterThread(osTCPSocketServer& socketServer,
                                                                   osTCPSocketServerConnectionHandler& connectionHandler,
                                                                   apAPIConnectionType APIConnectionType)
    : gaSpyConnectionWaiterThread(APIConnectionType), _socketServer(socketServer), _connectionHandler(connectionHandler)
{
}


// ---------------------------------------------------------------------------
// Name:        gaTCPIPSpyConnectionWaiterThread::~gaTCPIPSpyConnectionWaiterThread
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        25/8/2005
// ---------------------------------------------------------------------------
gaTCPIPSpyConnectionWaiterThread::~gaTCPIPSpyConnectionWaiterThread()
{
}


// ---------------------------------------------------------------------------
// Name:        gaTCPIPSpyConnectionWaiterThread::entryPoint
// Description: The thread entry point.
//              Calls the osTCPSocketServer::accept() method on the connection
//              handler.
// Return Val:  int - 0   - Success
//                  -  -1 - Failure
// Author:      Yaki Tebeka
// Date:        15/2/2004
// ---------------------------------------------------------------------------
int gaTCPIPSpyConnectionWaiterThread::entryPoint()
{
    int retVal = -1;

    // Wait for an incoming call. When they arrive - accept them:
    bool rc = _socketServer.accept(_connectionHandler);

    // Notice: If we would like to support few parallel connections, we can
    //         create a new thread that will handle the incoming connection, and
    //         make this thread call _socketServer.accept() again.

    if (rc)
    {
        // Trigger an "API Connection established" event:
        triggerAPIConnectionEstablishedEvent();

        // Set the thread return value:
        retVal = 0;
    }

    return retVal;
}

