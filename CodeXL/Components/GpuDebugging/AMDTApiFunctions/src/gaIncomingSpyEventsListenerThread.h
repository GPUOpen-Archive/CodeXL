//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaIncomingSpyEventsListenerThread.h
///
//==================================================================================

//------------------------------ gaIncomingSpyEventsListenerThread.h ------------------------------

#ifndef __GAINCOMINGSPYEVENTSLISTENERTHREAD_H
#define __GAINCOMINGSPYEVENTSLISTENERTHREAD_H

// Forward declarations:
class osSocket;

// Infra:
#include <AMDTOSWrappers/Include/osThread.h>


// ----------------------------------------------------------------------------------
// Class Name:          gaIncomingSpyEventsListenerThread : public osThread
// General Description:
//  A thread that listens to debug events coming directly from the spies.
// Author:              Uri Shomroni
// Creation Date:       29/10/2009
// ----------------------------------------------------------------------------------
class gaIncomingSpyEventsListenerThread : public osThread
{
public:
    virtual ~gaIncomingSpyEventsListenerThread();

    // Overrides osThread:
    virtual int entryPoint();
    virtual void beforeTermination();

protected:
    // Only allow my child classes to use my default constructor:
    gaIncomingSpyEventsListenerThread();

    // Should be overwritten by child classes.
    virtual bool connectAndWaitForClient() = 0;
    virtual void terminateConnection() = 0;

protected:
    osSocket* _pIncomingEventsSocket;
};


#endif //__GAINCOMINGSPYEVENTSLISTENERTHREAD_H

