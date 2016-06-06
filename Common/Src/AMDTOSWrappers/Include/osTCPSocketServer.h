//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTCPSocketServer.h
///
//=====================================================================

//------------------------------ osTCPSocketServer.h ------------------------------

#ifndef __OSTCPSOCKETSERVER
#define __OSTCPSOCKETSERVER

// Local:
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osTCPSocket.h>
#include <AMDTOSWrappers/Include/osTCPSocketServerConnectionHandler.h>

// Test if an address can be used by this computer as a TCP socket server:
OS_API bool osCanAddressBeUsedByTCPSocketServer(const osPortAddress& testedAddress);

// ----------------------------------------------------------------------------------
// Class Name:           osTCPSocketServer : public osTCPSocket
// General Description:
//   Represents the server side of a TCP / IP socket.
//   A socket server usually:
//   a. Binds itself to a port address (of the local machine).
//      I.E: Any socket client connection to this port will be mapped to this socket
//           server.
//   b. Listens to this port.
//   c. When an incoming client connection arrives - creates an osTCPSocketConnectionHandler
//      that will handle the client connection.
//      This allows a server to serve many clients simultaneously.
//
// Author:      AMD Developer Tools Team
// Creation Date:        6/12/2003
// ----------------------------------------------------------------------------------
class OS_API osTCPSocketServer : public osTCPSocket
{
public:
    osTCPSocketServer();
    virtual ~osTCPSocketServer();

    // Overrides osTCPSocket:
    virtual bool close();
    virtual bool write(const gtByte* pDataBuffer, unsigned long dataSize);
    virtual bool read(gtByte* pDataBuffer, unsigned long dataSize);

    // Self functions:
    bool bind(const osPortAddress& networkPort);
    bool listen(unsigned int backlog);
    bool accept(osTCPSocketServerConnectionHandler& connectionHandler);

    const osPortAddress& boundAddress() {return _boundAddress;};

private:
    osPortAddress _boundAddress;
};

#endif  // __OSTCPSOCKETSERVER
