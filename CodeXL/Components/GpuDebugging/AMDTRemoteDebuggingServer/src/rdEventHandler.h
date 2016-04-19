//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file rdEventHandler.h
///
//==================================================================================

//------------------------------ rdEventHandler.h ------------------------------

#ifndef __RDEVENTHANDLER_H
#define __RDEVENTHANDLER_H

// Forward declarations:
class osChannel;

// Infra:
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>

// ----------------------------------------------------------------------------------
// Class Name:           rdEventHandler : public apIEventsObserver
// General Description: An Events observer used to pass incoming debug events from the
//                      remote debugging server to the main application's events handler
// Author:               Uri Shomroni
// Creation Date:        10/8/2009
// ----------------------------------------------------------------------------------
class rdEventHandler : public apIEventsObserver
{
public:
    rdEventHandler(osChannel& processDebuggerEventsChannel);
    ~rdEventHandler();

    // Overrides apIEventsObserver
    void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"RemoteDebuggingEventHandler"; };

private:
    // Disallow use of default constructor, copy constructor and assignment operator:
    rdEventHandler() = delete;
    rdEventHandler(const rdEventHandler&) = delete;
    rdEventHandler& operator=(const rdEventHandler&) = delete;

private:
    osChannel& _processDebuggerEventsChannel;
};

#endif //__RDEVENTHANDLER_H

