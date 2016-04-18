//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaSpyConnectionWaiterThread.h
///
//==================================================================================

//------------------------------ gaSpyConnectionWaiterThread.h ------------------------------

#ifndef __GASPYCONNECTIONWAITERTHREAD
#define __GASPYCONNECTIONWAITERTHREAD

// Forward declarations:
class osTCPSocketServer;

// Infra:
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAPIClasses/Include/apAPIConnectionType.h>


// ----------------------------------------------------------------------------------
// Class Name:           gaSpyConnectionWaiterThread : public osThread
// General Description:
//   A thread that waits and accepts the OpenGL32.dll spy socket connection call.
//
// Author:               Yaki Tebeka
// Creation Date:        15/2/2004
// ----------------------------------------------------------------------------------
class gaSpyConnectionWaiterThread : public osThread
{
public:
    gaSpyConnectionWaiterThread(apAPIConnectionType APIConnectionType);
    virtual ~gaSpyConnectionWaiterThread();

protected:
    void triggerAPIConnectionEstablishedEvent();

private:
    // Do not allow the use of my default constructor:
    gaSpyConnectionWaiterThread();

protected:
    // The type of the API connection to which this thread waits for:
    apAPIConnectionType _APIConnectionType;
};


#endif  // __GASPYCONNECTIONWAITERTHREAD
