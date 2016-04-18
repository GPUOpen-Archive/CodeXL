//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaIncomingSpyEventsPipeSocketListenerThread.cpp
///
//==================================================================================

//------------------------------ gaIncomingSpyEventsPipeSocketListenerThread.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osPipeSocketServer.h>

// Local:
#include <src/gaIncomingSpyEventsPipeSocketListenerThread.h>


// ---------------------------------------------------------------------------
// Name:        gaIncomingSpyEventsPipeSocketListenerThread::gaIncomingSpyEventsPipeSocketListenerThread
// Description: Constructor
// Arguments: pipeName - The pipe's name.
// Author:      Yaki Tebeka
// Date:        16/12/2009
// ---------------------------------------------------------------------------
gaIncomingSpyEventsPipeSocketListenerThread::gaIncomingSpyEventsPipeSocketListenerThread(const gtString& pipeName)
    : gaIncomingSpyEventsListenerThread()
{
    // Create and socket server:
    _pIncomingEventsSocket = new osPipeSocketServer(pipeName);


    // Set the pipe's read timeout to infinite:
    _pIncomingEventsSocket->setReadOperationTimeOut(OS_CHANNEL_INFINIT_TIME_OUT);

    // Open the pipe:
    bool rcOpen = _pIncomingEventsSocket->open();
    GT_ASSERT(rcOpen);

    // Failure cleanup:
    if (!rcOpen)
    {
        delete _pIncomingEventsSocket;
        _pIncomingEventsSocket = NULL;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaIncomingSpyEventsPipeSocketListenerThread::~gaIncomingSpyEventsPipeSocketListenerThread
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        16/12/2009
// ---------------------------------------------------------------------------
gaIncomingSpyEventsPipeSocketListenerThread::~gaIncomingSpyEventsPipeSocketListenerThread()
{
    // Terminate the connection, if our parent class hasn't done so yet:
    terminateConnection();
}


// ---------------------------------------------------------------------------
// Name:        gaIncomingSpyEventsPipeSocketListenerThread::connectAndWaitForClient
// Description: Waits for the client to connect to us.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        16/12/2009
// ---------------------------------------------------------------------------
bool gaIncomingSpyEventsPipeSocketListenerThread::connectAndWaitForClient()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pIncomingEventsSocket != NULL)
    {
        // Wait for the client's connection:
        bool rcWait = ((osPipeSocketServer*)_pIncomingEventsSocket)->waitForClientConnection();
        GT_IF_WITH_ASSERT(rcWait)
        {
            retVal = true;
        }
    }

    // Failure cleanup:
    if (!retVal)
    {
        delete _pIncomingEventsSocket;
        _pIncomingEventsSocket = NULL;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaIncomingSpyEventsPipeSocketListenerThread::terminateConnection
// Description: Terminate the connection with the spy
// Author:      Yaki Tebeka
// Date:        16/12/2009
// ---------------------------------------------------------------------------
void gaIncomingSpyEventsPipeSocketListenerThread::terminateConnection()
{
    // Close and destroy the pipe socket server:
    if (_pIncomingEventsSocket != NULL)
    {
        if (_pIncomingEventsSocket->isOpen())
        {
            _pIncomingEventsSocket->close();
        }

        // The base class will delete _pIncomingEventsSocket.
    }
}

