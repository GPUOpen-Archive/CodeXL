//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTCPSocketClient.cpp
///
//=====================================================================

//------------------------------ osTCPSocketClient.cpp ------------------------------

// POSIX:
#include <netdb.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osTCPSocketClient.h>

// ---------------------------------------------------------------------------
// Name:        osTCPSocketClient::osTCPSocketClient
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        3/1/2004
// ---------------------------------------------------------------------------
osTCPSocketClient::osTCPSocketClient()
    : osTCPSocket(L"osTCPSocketClient")
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

    // Get my OS socket descriptor:
    osSocketDescriptor socketDescriptor = OSSocketDescriptor();
    GT_IF_WITH_ASSERT(socketDescriptor != NO_OS_SOCKET_DESCRIPTOR)
    {
        // Translate portAddress into internet style port address:
        sockaddr_in internetSocketAddress;
        bool rc1 = portAddress.asSockaddr(internetSocketAddress, _blockOnDNS);
        GT_IF_WITH_ASSERT(rc1)
        {
            // Connect this socket to the input (local / remote) port:
            int rc2 = ::connect(socketDescriptor, (struct sockaddr*)&internetSocketAddress, sizeof(sockaddr_in));
            GT_IF_WITH_ASSERT(rc2 == 0)
            {
                retVal = true;
            }
        }
    }

    return retVal;
}


