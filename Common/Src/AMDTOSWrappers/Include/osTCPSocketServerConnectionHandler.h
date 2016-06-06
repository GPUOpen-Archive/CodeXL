//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTCPSocketServerConnectionHandler.h
///
//=====================================================================

//------------------------------ osTCPSocketServerConnectionHandler.h ------------------------------

#ifndef __OSTCPSOCKETSERVERCONNECTIONHANDLER
#define __OSTCPSOCKETSERVERCONNECTIONHANDLER

// Local:
#include <AMDTOSWrappers/Include/osTCPSocket.h>


// ----------------------------------------------------------------------------------
// Class Name:           osTCPSocketServerConnectionHandler : public osTCPSocket
// General Description:
//   A server side TCP / IP socket, that was created to handle a client connection.
//   Such handler is created to handle every client connection. This enables
//   a server to server few concurrent clients.
//
// Author:      AMD Developer Tools Team
// Creation Date:        6/12/2003
// ----------------------------------------------------------------------------------
class OS_API osTCPSocketServerConnectionHandler : public osTCPSocket
{
public:
    osTCPSocketServerConnectionHandler();
    virtual ~osTCPSocketServerConnectionHandler();

private:
    // Only an osTCPSocketServer should initialize me:
    friend class osTCPSocketServer;
    void initialize(osSocketDescriptor socketDescriptor);
};


#endif  // __OSTCPSOCKETSERVERCONNECTIONHANDLER
