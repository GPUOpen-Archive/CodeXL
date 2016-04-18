//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaPipeSocketSpyConnectionWaiterThread.h
///
//==================================================================================

//------------------------------ gaPipeSocketSpyConnectionWaiterThread.h ------------------------------

#ifndef __GAPIPESOCKETSPYCONNECTIONWAITERTHREAD
#define __GAPIPESOCKETSPYCONNECTIONWAITERTHREAD

// Forward decelerations:
class osPipeSocketServer;

// Local:
#include <src/gaSpyConnectionWaiterThread.h>


// ----------------------------------------------------------------------------------
// Class Name:           gaPipeSocketSpyConnectionWaiterThread : public gaSpyConnectionWaiterThread
// General Description:
//   A thread that waits and accepts the OpenGL32 spy socket connection call.
//   This sub-class is responsible for handling pipe socket connection calls.
// Author:               Yaki Tebeka
// Creation Date:        1/1/2007
// ----------------------------------------------------------------------------------
class gaPipeSocketSpyConnectionWaiterThread : public gaSpyConnectionWaiterThread
{
public:
    gaPipeSocketSpyConnectionWaiterThread(osPipeSocketServer& socketServer, apAPIConnectionType APIConnectionType);
    virtual ~gaPipeSocketSpyConnectionWaiterThread();

protected:
    // Overrides osThread:
    virtual int entryPoint();

private:
    // Disallow use of default constructor, copy constructor and assignment operator:
    gaPipeSocketSpyConnectionWaiterThread() = delete;
    gaPipeSocketSpyConnectionWaiterThread(const gaPipeSocketSpyConnectionWaiterThread&) = delete;
    gaPipeSocketSpyConnectionWaiterThread& operator=(const gaPipeSocketSpyConnectionWaiterThread&) = delete;

private:
    // The pipe socket server:
    osPipeSocketServer& _socketServer;
};


#endif  // __GAPIPESOCKETSPYCONNECTIONWAITERTHREAD
