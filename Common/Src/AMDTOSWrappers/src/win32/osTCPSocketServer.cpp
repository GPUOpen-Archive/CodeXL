//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTCPSocketServer.cpp
///
//=====================================================================

//------------------------------ osTCPSocketServer.cpp ------------------------------

// Win32:
#include "Winsock2.h"

// GRBaseTools:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osSystemError.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osTCPSocketServer.h>

// ---------------------------------------------------------------------------
// Name:        osTCPSocketServer::osTCPSocketServer
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        3/1/2004
// ---------------------------------------------------------------------------
osTCPSocketServer::osTCPSocketServer(): osTCPSocket(L"osTCPSocketServer")
{
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocketServer::~osTCPSocketServer
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        4/1/2004
// ---------------------------------------------------------------------------
osTCPSocketServer::~osTCPSocketServer()
{
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocketServer::close
// Description: Closes this socket.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        24/1/2004
// ---------------------------------------------------------------------------
bool osTCPSocketServer::close()
{
    // Notice: To terminate the socket in a graceful way, we need to make sure that
    //         all the pending read and write operation terminated.
    //         There is a nice explanation for how it should be done in the MSDN
    //         documentation of shutdown.

    // Close the socket:
    bool retVal = osTCPSocket::close();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocketServer::write
// Description: Write operations are not allowed on socket servers,
//              So - this function always fails.
// Return Val:  bool - always false.
// Author:      AMD Developer Tools Team
// Date:        17/5/2004
// ---------------------------------------------------------------------------
bool osTCPSocketServer::write(const gtByte* pDataBuffer, unsigned long dataSize)
{
    GT_UNREFERENCED_PARAMETER(pDataBuffer);
    GT_UNREFERENCED_PARAMETER(dataSize);

    return false;
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocketServer::read
// Description: Read operations are not allowed on socket servers,
//              So - this function always fails.
// Return Val:  bool - always false.
// Author:      AMD Developer Tools Team
// Date:        17/5/2004
// ---------------------------------------------------------------------------
bool osTCPSocketServer::read(gtByte* pDataBuffer, unsigned long dataSize)
{
    GT_UNREFERENCED_PARAMETER(pDataBuffer);
    GT_UNREFERENCED_PARAMETER(dataSize);

    return false;
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocketServer::bind
//
// Description: Associates a local port with this socket.
//              I.E: Any socket client connection to this port address will be mapped
//                   to this socket server.
//
// Arguments:   portAddress - The address of the port to which the socket will be bound.
//
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        3/1/2004
// ---------------------------------------------------------------------------
bool osTCPSocketServer::bind(const osPortAddress& portAddress)
{
    bool retVal = false;

    // Verify that the socket is open:
    if (isOpen())
    {
        // Translate portAddress into a win32 local port address:
        SOCKADDR_IN win32PortAddress;
        bool rc = portAddress.asSockaddr(win32PortAddress, _blockOnDNS);

        if (rc)
        {
            // Bind this socket to the input local port address:
            osSocketDescriptor socketDescriptor = OSSocketDescriptor();
            int rc1 = ::bind(socketDescriptor, (LPSOCKADDR)&win32PortAddress,
                             sizeof(SOCKADDR_IN));

            if (rc1 != SOCKET_ERROR)
            {
                retVal = true;
                _boundAddress = portAddress;
            }
            else
            {
                // An error occurred:
                gtString errMsg = OS_STR_bindError;
                errMsg += OS_STR_osReportedErrorIs;
                errMsg += OS_STR_host;
                errMsg += portAddress.hostName();
                errMsg += OS_STR_port;
                unsigned short portNum = portAddress.portNumber();
                errMsg.appendFormattedString(L"%u ", portNum);
                gtString systemError;
                osGetLastSystemErrorAsString(systemError);
                errMsg += systemError;
                GT_ASSERT_EX(false, errMsg.asCharArray());
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocketServer::listen
//
// Description: Start listening to client connections made to the port address to
//              which I am bound. The pending connections will be queued until
//              I will handle them (using accept()).
//
//              Notice that this method is relevant only for SOCKET_TYPE_STREAM
//              sockets (TCP) which use a connected stream protocol.
//
// Arguments:   backlog - The maximal amount of unhandled pending connections.
//                        (on which I didn't call accept())
//                        I.E: The pending connections queue size.
//
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/1/2004
// ---------------------------------------------------------------------------
bool osTCPSocketServer::listen(unsigned int backlog)
{
    bool retVal = false;

    // Get my Win32 socket descriptor:
    osSocketDescriptor socketDescriptor = OSSocketDescriptor();

    // Start listening to the port:
    int rc = ::listen(socketDescriptor, backlog);

    if (rc != SOCKET_ERROR)
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osTCPSocketServer::accept
//
// Description: Handle the next pending connection request to this socket.
//              If such a request does not exist - WAIT !!! for it.
//
//              Notice that if there are no pending connection requests, a call to
//              this function blocks the execution of the thread that called it
//              until the next connection request arrives.
//
// Arguments:   connectionHandler - A socket that will handle the accepted connection.
//
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        4/1/2004
// ---------------------------------------------------------------------------
bool osTCPSocketServer::accept(osTCPSocketServerConnectionHandler& connectionHandler)
{
    bool retVal = false;

    // Get my Win32 socket descriptor:
    osSocketDescriptor socketDescriptor = OSSocketDescriptor();

    // Handle the next pending connection (or wait for it):
    osSocketDescriptor handlingSocketDescriptor = ::accept(socketDescriptor, 0, 0);

    if (handlingSocketDescriptor != INVALID_SOCKET)
    {
        connectionHandler.initialize(handlingSocketDescriptor);
        retVal = true;
    }

    return retVal;
}


