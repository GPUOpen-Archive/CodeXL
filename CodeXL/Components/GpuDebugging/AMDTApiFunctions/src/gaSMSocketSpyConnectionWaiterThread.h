//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaSMSocketSpyConnectionWaiterThread.h
///
//==================================================================================

//------------------------------ gaSMSocketSpyConnectionWaiterThread.h ------------------------------

Yaki 6 / 5 / 2005:
osSharedMemorySocket performance seems to be poor on dual core machines. Therefore, it was
replaced by osPipeSocket.

/*
#ifndef __GASMSOCKETSPYCONNECTIONWAITERTHREAD
#define __GASMSOCKETSPYCONNECTIONWAITERTHREAD

// Predecelerations:
class osSharedMemorySocketServer;

// Local:
#include <inc/gaSpyConnectionWaiterThread.h>


// ----------------------------------------------------------------------------------
// Class Name:           gaSMSocketSpyConnectionWaiterThread : public gaSpyConnectionWaiterThread
// General Description:
//   A thread that waits and accepts the OpenGL32.dll spy socket connection call.
//   This sub-class is responsible for handling shared memory socket connection calls.
// Author:               Yaki Tebeka
// Creation Date:        25/8/2005
// ----------------------------------------------------------------------------------
class gaSMSocketSpyConnectionWaiterThread : public gaSpyConnectionWaiterThread
{
public:
    gaSMSocketSpyConnectionWaiterThread(osSharedMemorySocketServer& socketServer);
    virtual ~gaSMSocketSpyConnectionWaiterThread();

protected:
    // Overrides osThread:
    virtual int entryPoint();

private:
    // Do not allow the use of my default constructor:
    gaSMSocketSpyConnectionWaiterThread();

private:
    // The shared memory socket server:
    osSharedMemorySocketServer& _socketServer;
};


#endif  // __GASMSOCKETSPYCONNECTIONWAITERTHREAD
*/
