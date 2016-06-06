//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osPipeSocketClient.cpp
///
//=====================================================================

//------------------------------ osPipeSocketClient.cpp ------------------------------

// Win32:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osPipeSocketClient.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>


// ---------------------------------------------------------------------------
// Name:        osPipeSocketClient::osPipeSocketClient
// Description: Constructor
// Arguments:   pipeName - The pipe name.
// Author:      AMD Developer Tools Team
// Date:        4/5/2007
// ---------------------------------------------------------------------------
osPipeSocketClient::osPipeSocketClient(const gtString& pipeName, const gtString& socketName)
    : osPipeSocket(pipeName, socketName)
{
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocketClient::~osPipeSocketClient
// Description: Destructor.
// Author:      AMD Developer Tools Team
// Date:        4/5/2007
// ---------------------------------------------------------------------------
osPipeSocketClient::~osPipeSocketClient()
{
    // Close the pipe connection and release all used resources:
    this->close();
}


// ---------------------------------------------------------------------------
// Name:        osPipeSocketClient::open
// Description: Connects to the pipe to the pipe server.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/5/2007
// ---------------------------------------------------------------------------
bool osPipeSocketClient::open()
{
    // Calculate the pipe FIFO files paths:
    gtString incomingPipeFilePath;
    gtString outgoingPipeFilePath;
    osPipeSocket::getPipeFIFOFilesPaths(outgoingPipeFilePath, incomingPipeFilePath);

    bool outgoingPipeOpened = false;
    bool incomingPipeOpened = false;

    // Open the outgoing pipe:
    _outgoingPipe = ::CreateFile(
                        outgoingPipeFilePath.asCharArray(), // Pipe name.
                        GENERIC_WRITE,                      // Write access.
                        0,                                  // No sharing.
                        NULL,                               // Use the default security attributes.
                        OPEN_EXISTING,                      // Opens only an existing pipe.
                        0,                                  // Use default attributes.
                        NULL);                              // No template file.

    GT_IF_WITH_ASSERT(_outgoingPipe != INVALID_HANDLE_VALUE)
    {
        outgoingPipeOpened = true;
    }

    // Open the incoming pipe:
    _incomingPipe = ::CreateFile(
                        incomingPipeFilePath.asCharArray(), // Pipe name.
                        GENERIC_READ,                       // Read access.
                        0,                                  // No sharing.
                        NULL,                               // Use the default security attributes.
                        OPEN_EXISTING,                      // Opens only an existing pipe.
                        0,                                  // Use default attributes.
                        NULL);                              // No template file.

    GT_IF_WITH_ASSERT(_incomingPipe != INVALID_HANDLE_VALUE)
    {
        incomingPipeOpened = true;
    }

    if (outgoingPipeOpened && incomingPipeOpened)
    {
        _isOpen = true;
    }

    return _isOpen;
}

