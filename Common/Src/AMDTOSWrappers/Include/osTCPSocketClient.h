//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osTCPSocketClient.h
///
//=====================================================================

//------------------------------ osTCPSocketClient.h ------------------------------

#ifndef __OSSOCKETCLIENT
#define __OSSOCKETCLIENT

// Pre-declarations:
class osPortAddress;

// Local:
#include <AMDTOSWrappers/Include/osTCPSocket.h>


// ----------------------------------------------------------------------------------
// Class Name:           osTCPSocketClient : public osTCPSocket
// General Description:
//   Represents the client side of a TCP / IP socket.
//   A TCP / IP socket client usually:
//   a. Connects itself to a remote server.
//   b. Sends and receives data to / from the server.
//
// Author:      AMD Developer Tools Team
// Creation Date:        6/12/2003
// ----------------------------------------------------------------------------------
class OS_API osTCPSocketClient : public osTCPSocket
{
public:
    osTCPSocketClient();
    virtual ~osTCPSocketClient();

    bool connect(const osPortAddress& portAddress);
};


#endif  // __OSSOCKETCLIENT
