//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afProcessDebuggerEventHandler.cpp
///
//==================================================================================

// Local:
#include <src/afProcessDebuggerPendingEventEvent.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <src/afProcessDebuggerEventHandler.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osTransferableObjectType.h>

// Static member initializations:
afProcessDebuggerEventHandler* afProcessDebuggerEventHandler::m_pMySingleInstance = nullptr;

// Event Handler buffer size
#define AF_EVENT_HANDLER_OVERFLOW_LIMIT 2000

// ---------------------------------------------------------------------------
// Name:        gdProcessDebuggerPendingEventNotificationFunc
//
// Description:
//  Is called when a debugged process event occurs.
//  Triggers a gdDebuggerPendingEventEvent, who's handler function will
//  handle the debugged process event.
//  This enables the handling of the event by the main application thread instead
//  of the the debugger thread.
//
// Author:      Yaki Tebeka
// Date:        6/4/2004
// ---------------------------------------------------------------------------
void gdProcessDebuggerPendingEventNotificationFunc(const apEvent& pendingEvent)
{
    GT_UNREFERENCED_PARAMETER(pendingEvent);

    // fire the event
    afProcessDebuggerPendingEventEvent::instance().emitEvent();
}


// ---------------------------------------------------------------------------
// Name:        afProcessDebuggerEventHandler::instance
// Description: Returns the single instance of this class.
//              (If it does not exist - create it)
// Author:      Yaki Tebeka
// Date:        20/7/2007
// ---------------------------------------------------------------------------
afProcessDebuggerEventHandler& afProcessDebuggerEventHandler::instance()
{
    if (m_pMySingleInstance == nullptr)
    {
        m_pMySingleInstance = new afProcessDebuggerEventHandler;
        GT_ASSERT(m_pMySingleInstance);
    }

    return *m_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        afProcessDebuggerEventHandler::afProcessDebuggerEventHandler
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        19/5/2004
// ---------------------------------------------------------------------------
afProcessDebuggerEventHandler::afProcessDebuggerEventHandler()
{
    // Register gdProcessDebuggerPendingEventNotificationFunc as the event
    // notification function:
    apEventsHandler::instance().registerPendingEventNotificationCallback(&gdProcessDebuggerPendingEventNotificationFunc);

    // connect to the event:
    connect(&afProcessDebuggerPendingEventEvent::instance(), SIGNAL(pendingDebugEvent()) , this, SLOT(OnDebuggedProcessEvent()), Qt::QueuedConnection);
}


// ---------------------------------------------------------------------------
// Name:        afProcessDebuggerEventHandler::afProcessDebuggerEventHandler
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        19/5/2004
// ---------------------------------------------------------------------------
afProcessDebuggerEventHandler::~afProcessDebuggerEventHandler()
{
    // Unregister gdProcessDebuggerPendingEventNotificationFunc as the events
    // notification function:
    apEventsHandler::instance().unregisterPendingEventNotificationCallback(&gdProcessDebuggerPendingEventNotificationFunc);

    // make sure the connection is broken, in case reconnected no double events will be received:
    disconnect(&afProcessDebuggerPendingEventEvent::instance(), SIGNAL(pendingDebugEvent()) , this, SLOT(OnDebuggedProcessEvent()));
}


// ---------------------------------------------------------------------------
// Name:        afProcessDebuggerEventHandler::OnDebuggedProcessEvent
// Description: Is called when a debugged process event occurs.
//              Calls the process debugger handlePendingDebugEvent() to handle
//              the event.
// Author:      Yaki Tebeka
// Date:        7/4/2004
// ---------------------------------------------------------------------------
void afProcessDebuggerEventHandler::OnDebuggedProcessEvent()
{
    // Get the events handler instance:
    apEventsHandler& theEventsHandler = apEventsHandler::instance();

    // Handle the pending debug event:
    theEventsHandler.handlePendingDebugEvent();
}
