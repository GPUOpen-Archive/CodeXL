//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTCPSocketClient.cpp
///
//=====================================================================

//------------------------------ osTCPSocketClient.cpp ------------------------------

// Win32:
#include "Winsock2.h"

// Local:
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osTCPSocketClient.h>

// ---------------------------------------------------------------------------
// Name:        osTCPSocketClient::osTCPSocketClient
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        3/1/2004
// ---------------------------------------------------------------------------
osTCPSocketClient::osTCPSocketClient(): osTCPSocket(L"osTCPSocketClient")
{
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocketClient::~osTCPSocketClient
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        4/1/2004
// ---------------------------------------------------------------------------
osTCPSocketClient::~osTCPSocketClient()
{
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocketClient::connect
//
// Description: Connect this socket to a local / remote host port.
//              I.E: The will call the local / remote host port. If there is
//                   a server socket that listens to this port, it will connect
//                   this socket to once of its child sockets.
//
// Arguments:   portAddress - The address of the local / remote host port.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/1/2004
// ---------------------------------------------------------------------------
bool osTCPSocketClient::connect(const osPortAddress& portAddress)
{
    bool retVal = false;

    // Get my Win32 socket descriptor:
    osSocketDescriptor socketDescriptor = OSSocketDescriptor();

    // Translate portAddress into a win32 port address:
    SOCKADDR_IN win32PortAddress;
    retVal = portAddress.asSockaddr(win32PortAddress, _blockOnDNS);

    if (retVal)
    {
        // Connect this socket to the local / remote port:
        int rc = ::connect(socketDescriptor, (LPSOCKADDR)&win32PortAddress, sizeof(SOCKADDR_IN));
        retVal = (rc != SOCKET_ERROR);
    }

    return retVal;
}


