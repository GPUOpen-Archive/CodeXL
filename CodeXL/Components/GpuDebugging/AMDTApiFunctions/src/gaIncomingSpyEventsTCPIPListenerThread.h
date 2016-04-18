//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaIncomingSpyEventsTCPIPListenerThread.h
///
//==================================================================================

//------------------------------ gaIncomingSpyEventsTCPIPListenerThread.h ------------------------------

#ifndef __GAINCOMINGSPYEVENTSTCPIPLISTENERTHREAD_H
#define __GAINCOMINGSPYEVENTSTCPIPLISTENERTHREAD_H

// Forward declarations:
class osPortAddress;
class osTCPSocketServer;

// Local:
#include <src/gaIncomingSpyEventsListenerThread.h>


// ----------------------------------------------------------------------------------
// Class Name:          gaIncomingSpyEventsTCPIPListenerThread : public gaIncomingSpyEventsListenerThread
// General Description: Implementation of pdIncomingSpyEventsListenerThread using TCP sockets.
// Author:              Uri Shomroni
// Creation Date:       29/10/2009
// ----------------------------------------------------------------------------------
class gaIncomingSpyEventsTCPIPListenerThread : public gaIncomingSpyEventsListenerThread
{
public:
    gaIncomingSpyEventsTCPIPListenerThread(const osPortAddress& portAddress);
    ~gaIncomingSpyEventsTCPIPListenerThread();

    // Overrides pdIncomingSpyEventsListenerThread:
    virtual bool connectAndWaitForClient();
    virtual void terminateConnection();

private:
    // Disallow use of my default constructor:
    gaIncomingSpyEventsTCPIPListenerThread();

private:
    // The TCP socket server:
    osTCPSocketServer* _pTCPSocketServer;
};


#endif //__GAINCOMINGSPYEVENTSTCPIPLISTENERTHREAD_H

