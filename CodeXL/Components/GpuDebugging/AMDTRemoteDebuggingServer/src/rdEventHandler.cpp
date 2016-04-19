//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file rdEventHandler.cpp
///
//==================================================================================

//------------------------------ rdEventHandler.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>

// Local:
#include <src/rdEventHandler.h>

// ---------------------------------------------------------------------------
// Name:        rdEventHandler::rdEventHandler
// Description: Constructor
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
rdEventHandler::rdEventHandler(osChannel& processDebuggerEventsChannel)
    : _processDebuggerEventsChannel(processDebuggerEventsChannel)
{
    apEventsHandler& theEventsHandler = apEventsHandler::instance();

    // We register this in a lower priority than the actual (win32) process debugger,
    // to give it a chance to veto events before we pass them over the channel.
    theEventsHandler.registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);
}

// ---------------------------------------------------------------------------
// Name:        rdEventHandler::~rdEventHandler
// Description: Destructor
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
rdEventHandler::~rdEventHandler()
{
    apEventsHandler::instance().unregisterEventsObserver(*this);
}

// ---------------------------------------------------------------------------
// Name:        rdEventHandler::onEvent
// Description: Called whenever a debug event occurs, this needs to pass the event
//              through the channel
// Author:      Uri Shomroni
// Date:        12/8/2009
// ---------------------------------------------------------------------------
void rdEventHandler::onEvent(const apEvent& eve, bool& vetoEvent)
{
    // Unused parameter:
    (void)(vetoEvent);

    apEvent::EventType eveType = eve.eventType();
    _processDebuggerEventsChannel << (gtInt32)eveType;
    _processDebuggerEventsChannel << (const osTransferableObject&)eve;

    // Uri, 10/10/13 - Linux issue workaround - sometimes the CodeXL client will not
    // correctly cleanup the RDS connection. This causes the RDS to stay up when it's unneeded,
    // which in turn can cause issues with the daemon and next RDS sessions.
    // To avoid this issue, we exit on the occasion when the client would have killed us:
    // after sending a process fatal event (process termination, process creation failure, etc.).
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS

    if ((apEvent::AP_DEBUGGED_PROCESS_TERMINATED == eveType) || (apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE == eveType))
    {
        ::exit(0);
    }

#endif // AMDT_BUILD_TARGET == AMDT_LINUX_OS
}

