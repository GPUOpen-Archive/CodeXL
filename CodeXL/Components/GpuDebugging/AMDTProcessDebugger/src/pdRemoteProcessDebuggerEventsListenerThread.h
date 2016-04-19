//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdRemoteProcessDebuggerEventsListenerThread.h
///
//==================================================================================

//------------------------------ pdRemoteProcessDebuggerEventsListenerThread.h ------------------------------

#ifndef __PDREMOTEPROCESSDEBUGGEREVENTSLISTENERTHREAD_H
#define __PDREMOTEPROCESSDEBUGGEREVENTSLISTENERTHREAD_H

// Infra:
#include <AMDTOSWrappers/Include/osThread.h>

// Forward declarations:
class osChannel;

// ----------------------------------------------------------------------------------
// Class Name:           pdRemoteProcessDebuggerEventsListenerThread : public osThread
// General Description: A thread that reads the events thrown from the remote debugging server
// Author:               Uri Shomroni
// Creation Date:        12/8/2009
// ----------------------------------------------------------------------------------
class pdRemoteProcessDebuggerEventsListenerThread : public osThread
{
public:
    pdRemoteProcessDebuggerEventsListenerThread();
    virtual ~pdRemoteProcessDebuggerEventsListenerThread();

    // Overrides osThread:
    virtual int entryPoint();

    void setEventsChannel(osChannel* pEventsChannel) {_pEventChannel = pEventsChannel;}
    void stopListening() {_listeningPaused = true;}
    void startListening() {_listeningPaused = false;}
protected:
    virtual void beforeTermination() override;
private:
    osChannel* _pEventChannel;

    bool _listeningPaused;
    bool _terminated = false;
};

#endif //__PDREMOTEPROCESSDEBUGGEREVENTSLISTENERTHREAD_H

