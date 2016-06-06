//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osPipeSocketServer.cpp
///
//=====================================================================

//------------------------------ osPipeSocketServer.cpp ------------------------------

// Win32:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <win32/osPrivateData.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osPipeSocketServer.h>


// ---------------------------------------------------------------------------
// Name:        osPipeSocketServer::osPipeSocketServer
// Description: Constructor
// Arguments:   pipeName - The pipe name.
// Author:      AMD Developer Tools Team
// Date:        4/5/2007
// ---------------------------------------------------------------------------
osPipeSocketServer::osPipeSocketServer(const gtString& pipeName)
    : osPipeSocket(pipeName, L"osPipeSocketServer"), _fifoFilesExists(false)
{
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocketServer::~osPipeSocketServer
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        4/5/2007
// ---------------------------------------------------------------------------
osPipeSocketServer::~osPipeSocketServer()
{
    // Close the pipe connection and release all used resources:
    this->close();
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocketServer::open
// Description: Create the server pipe, to which the client pipe will connect
//              later on.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/5/2007
// ---------------------------------------------------------------------------
bool osPipeSocketServer::open()
{
    bool retVal = false;

    bool incomingPipeCreated = false;
    bool outgoingPipeCreated = false;

    // Get the names of the 2 half duplex pipes, used for implementing this full duplex pipe:
    gtString incomingPipeName;
    gtString outgoingPipeName;
    osPipeSocket::getPipeFIFOFilesPaths(incomingPipeName, outgoingPipeName);

    // Create the incoming pipe:
    _incomingPipe = ::CreateNamedPipe(
                        incomingPipeName.asCharArray(),     // Pipe name.
                        PIPE_ACCESS_INBOUND,                // Read access only.
                        PIPE_TYPE_MESSAGE |                 // Data is written to the pipe as a stream of messages.
                        PIPE_READMODE_MESSAGE |             // Message-read mode.
                        PIPE_WAIT,                          // This is a blocking pipe.
                        1,                                  // Maximal amount of pipe instances.
                        0,                                  // Output buffer size
                        OS_PIPE_SOCKET_BUFF_SIZE,           // Input buffer size
                        NMPWAIT_USE_DEFAULT_WAIT,           // Client time-out
                        NULL);                              // Use default security attribute

    // If the pipe was created successfully:
    GT_IF_WITH_ASSERT(_incomingPipe != INVALID_HANDLE_VALUE)
    {
        incomingPipeCreated = true;
    }


    // Create the outgoing pipe:
    _outgoingPipe = ::CreateNamedPipe(
                        outgoingPipeName.asCharArray(),     // Pipe name.
                        PIPE_ACCESS_OUTBOUND,               // Write access only.
                        PIPE_TYPE_MESSAGE |                 // Data is written to the pipe as a stream of messages.
                        PIPE_READMODE_MESSAGE |             // Message-read mode.
                        PIPE_WAIT,                          // This is a blocking pipe.
                        1,                                  // Maximal amount of pipe instances.
                        OS_PIPE_SOCKET_BUFF_SIZE,           // Output buffer size
                        0,                                  // Input buffer size
                        NMPWAIT_USE_DEFAULT_WAIT,           // Client time-out
                        NULL);                              // Use default security attribute


    // If the pipe was created successfully:
    GT_IF_WITH_ASSERT(_outgoingPipe != INVALID_HANDLE_VALUE)
    {
        outgoingPipeCreated = true;
    }

    if (incomingPipeCreated && outgoingPipeCreated)
    {
        _fifoFilesExists = true;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocketServer::close
// Description: Terminates the pipe communication.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/5/2007
// ---------------------------------------------------------------------------
bool osPipeSocketServer::close()
{
    // Call base class implementation:
    bool retVal = osPipeSocket::close();
    // Mark that the pipe is closed:
    _fifoFilesExists = false;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocketServer::waitForClientConnection
// Description: Suspends the calling thread until a client connection is
//              established.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/5/2007
// ---------------------------------------------------------------------------
bool osPipeSocketServer::waitForClientConnection()
{
    bool retVal = false;

    // Sanity check:
    if (_fifoFilesExists)
    {
        _isDuringPipeConnectionWait = true;

        // Wait until the client connects to the incoming pipe:
        bool incomingPipeConnected = false;
        BOOL rc1 = ::ConnectNamedPipe(_incomingPipe, NULL);

        if (rc1 != FALSE)
        {
            incomingPipeConnected = true;
        }
        else if (::GetLastError() == ERROR_PIPE_CONNECTED)
        {
            // If the client connected itself to the pipe in the interval between the call to
            // CreateNamedPipe and the call to ConnectNamedPipe:
            incomingPipeConnected = true;
        }

        // Wait until the client connects to the outgoing pipe:
        bool outgoingPipeConnected = false;
        BOOL rc2 = ::ConnectNamedPipe(_outgoingPipe, NULL);

        if (rc2 != FALSE)
        {
            outgoingPipeConnected = true;
        }
        else if (::GetLastError() == ERROR_PIPE_CONNECTED)
        {
            // If the client connected itself to the pipe in the interval between the call to
            // CreateNamedPipe and the call to ConnectNamedPipe:
            outgoingPipeConnected = true;
        }

        _isDuringPipeConnectionWait = false;

        GT_IF_WITH_ASSERT(incomingPipeConnected && outgoingPipeConnected)
        {
            _isOpen = true;
            retVal = true;
        }
    }

    return retVal;
}



