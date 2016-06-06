//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osPipeSocketClient.h
///
//=====================================================================

//------------------------------ osPipeSocketClient.h ------------------------------

#ifndef __OSPIPESOCKETCLIENT_H
#define __OSPIPESOCKETCLIENT_H

// Local:
#include <AMDTOSWrappers/Include/osPipeSocket.h>


// ----------------------------------------------------------------------------------
// Class Name:           OS_API osPipeSocketClient : public osPipeSocket
// General Description:
//   The client side of a pipe based socket.
//   Connects itself to a waiting pipe socket server.
// Author:      AMD Developer Tools Team
// Creation Date:        31/12/2006
// ----------------------------------------------------------------------------------
class OS_API osPipeSocketClient : public osPipeSocket
{
public:
    osPipeSocketClient(const gtString& pipeName, const gtString& socketName);
    osPipeSocketClient(osPipeHandle incomingPipe, osPipeHandle outgoingPipe, const gtString& socketName);
    virtual ~osPipeSocketClient();

    // Overrides osSocket:
    virtual bool open();

private:
    // Do not allow the use of my default constructor:
    osPipeSocketClient();
};


#endif //__OSPIPESOCKETCLIENT_H

