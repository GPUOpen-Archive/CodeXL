//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaTCPIPSpyConnectionWaiterThread.h
///
//==================================================================================

//------------------------------ gaTCPIPSpyConnectionWaiterThread.h ------------------------------

#ifndef __GATCPIPSPYCONNECTIONWAITERTHREAD
#define __GATCPIPSPYCONNECTIONWAITERTHREAD

// Forward decelerations:
class osTCPSocketServer;
class osTCPSocketServerConnectionHandler;

// Local:
#include <src/gaSpyConnectionWaiterThread.h>


// ----------------------------------------------------------------------------------
// Class Name:           gaTCPIPSpyConnectionWaiterThread : public gaSpyConnectionWaiterThread
// General Description:
//   A thread that waits and accepts the OpenGL32.dll spy socket connection call.
//   This sub-class is responsible for handling TCP / IP socket connection calls.
//
// Author:               Yaki Tebeka
// Creation Date:        25/8/2005
// ----------------------------------------------------------------------------------
class gaTCPIPSpyConnectionWaiterThread : public gaSpyConnectionWaiterThread
{
public:
    gaTCPIPSpyConnectionWaiterThread(osTCPSocketServer& socketServer,
                                     osTCPSocketServerConnectionHandler& connectionHandler,
                                     apAPIConnectionType APIConnectionType);

    virtual ~gaTCPIPSpyConnectionWaiterThread();

protected:
    // Overrides osThread:
    virtual int entryPoint();

private:
    // Disallow use of default constructor, copy constructor and assignment operator:
    gaTCPIPSpyConnectionWaiterThread() = delete;
    gaTCPIPSpyConnectionWaiterThread(const gaTCPIPSpyConnectionWaiterThread&) = delete;
    gaTCPIPSpyConnectionWaiterThread& operator=(const gaTCPIPSpyConnectionWaiterThread&) = delete;

private:
    // The socket server that listens to the port to which the OpenGL32.dll spy
    // will connect to:
    osTCPSocketServer& _socketServer;

    // The socket connection that will be established with the OpenGL32.dll spy:
    osTCPSocketServerConnectionHandler& _connectionHandler;
};


#endif  // __GATCPIPSPYCONNECTIONWAITERTHREAD
