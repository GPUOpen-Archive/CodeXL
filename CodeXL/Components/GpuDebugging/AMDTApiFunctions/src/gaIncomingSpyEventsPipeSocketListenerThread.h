//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaIncomingSpyEventsPipeSocketListenerThread.h
///
//==================================================================================

//------------------------------ gaIncomingSpyEventsPipeSocketListenerThread.h ------------------------------

#ifndef __GAINCOMINGSPYEVENTSPIPESOCKETLISTENERTHREAD_H
#define __GAINCOMINGSPYEVENTSPIPESOCKETLISTENERTHREAD_H

// Local:
#include <src/gaIncomingSpyEventsListenerThread.h>


// ----------------------------------------------------------------------------------
// Class Name:          gaIncomingSpyEventsPipeSocketListenerThread : public gaIncomingSpyEventsListenerThread
// General Description: Implementation of pdIncomingSpyEventsListenerThread using TCP sockets.
// Author:      Yaki Tebeka
// Date:        16/12/2009
// ----------------------------------------------------------------------------------
class gaIncomingSpyEventsPipeSocketListenerThread : public gaIncomingSpyEventsListenerThread
{
public:
    gaIncomingSpyEventsPipeSocketListenerThread(const gtString& pipeName);
    ~gaIncomingSpyEventsPipeSocketListenerThread();

    // Overrides pdIncomingSpyEventsListenerThread:
    virtual bool connectAndWaitForClient();
    virtual void terminateConnection();

private:
    // Disallow use of my default constructor:
    gaIncomingSpyEventsPipeSocketListenerThread();
};


#endif //__GAINCOMINGSPYEVENTSPIPESOCKETLISTENERTHREAD_H

