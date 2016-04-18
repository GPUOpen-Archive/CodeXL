//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaPipeSocketSpyConnectionWaiterThread.cpp
///
//==================================================================================

//------------------------------ gaPipeSocketSpyConnectionWaiterThread.cpp ------------------------------

// Infra:
#include <AMDTOSWrappers/Include/osPipeSocketServer.h>

// Local:
#include <src/gaPipeSocketSpyConnectionWaiterThread.h>


// ---------------------------------------------------------------------------
// Name:        gaPipeSocketSpyConnectionWaiterThread::gaPipeSocketSpyConnectionWaiterThread
// Description: Constructor.
// Arguments:   socketServer - The shared memory socket server.
//              APIConnectionType - The type of the API connection to which this thread waits for.
// Author:      Yaki Tebeka
// Date:        1/1/2007
// ---------------------------------------------------------------------------
gaPipeSocketSpyConnectionWaiterThread::gaPipeSocketSpyConnectionWaiterThread(osPipeSocketServer& socketServer, apAPIConnectionType APIConnectionType)
    : gaSpyConnectionWaiterThread(APIConnectionType), _socketServer(socketServer)
{
}


// ---------------------------------------------------------------------------
// Name:        gaPipeSocketSpyConnectionWaiterThread::~gaPipeSocketSpyConnectionWaiterThread
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        1/1/2007
// ---------------------------------------------------------------------------
gaPipeSocketSpyConnectionWaiterThread::~gaPipeSocketSpyConnectionWaiterThread()
{
}


// ---------------------------------------------------------------------------
// Name:        gaPipeSocketSpyConnectionWaiterThread::entryPoint
// Description: The thread's entry point.
// Return Val:  int - 0   - Success
//                  -  -1 - Failure
// Author:      Yaki Tebeka
// Date:        1/1/2007
// ---------------------------------------------------------------------------
int gaPipeSocketSpyConnectionWaiterThread::entryPoint()
{
    int retVal = -1;

    // Wait for an incoming client connection:
    bool rc = _socketServer.waitForClientConnection();

    if (rc)
    {
        // Trigger an "API Connection established" event:
        triggerAPIConnectionEstablishedEvent();

        // Set the thread return value:
        retVal = 0;
    }

    return retVal;
}

