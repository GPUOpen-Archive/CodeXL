//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osPipeSocketServer.h
///
//=====================================================================

//------------------------------ osPipeSocketServer.h ------------------------------

#ifndef __OSPIPESOCKETSERVER_H
#define __OSPIPESOCKETSERVER_H

// Local:
#include <AMDTOSWrappers/Include/osPipeSocket.h>


// ----------------------------------------------------------------------------------
// Class Name:           osPipeSocketServer : public osPipeSocket
// General Description:
//   The server side of a pipe based socket.
//   Is responsible for creating the pipe and waiting for a client connection.
//
// Author:      AMD Developer Tools Team
// Creation Date:        21/12/2006
// ----------------------------------------------------------------------------------
class OS_API osPipeSocketServer : public osPipeSocket
{
public:
    osPipeSocketServer(const gtString& pipeName);
    virtual ~osPipeSocketServer();

    // Overrides osSocket:
    virtual bool open();
    virtual bool close();

    // Self functions:
    bool waitForClientConnection();

private:
    // Contains true iff the FIFO files were created successfully:
    bool _fifoFilesExists;
};


#endif //__OSPIPESOCKETSERVER_H

