//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apEventsHandler.h
///
//==================================================================================

//------------------------------ apEventsHandler.h ------------------------------

#ifndef __APEVENTSHANDLER_H
#define __APEVENTSHANDLER_H

// Forward declarations:
class gtString;
class apEvent;
class apIEventsFiller;
class apIEventsObserver;

// Infra:
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTOSWrappers/Include/osSynchronizedQueue.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>

// ----------------------------------------------------------------------------------
// Enum Name:           apEventsHandlingPriority
// General Description: Used to make sure the flow of events from one part of the application
//                      to the next is in the right order regardless of the registration
//                      order. Note that a higher number in this list means that the
//                      events will be handled later.
//                      This also allows the first priorities to veto events.
// Author:  AMD Developer Tools Team
// Creation Date:       20/4/2009
// ----------------------------------------------------------------------------------
enum apEventsHandlingPriority
{
    AP_PROCESS_DEBUGGER_EVENTS_HANDLING_PRIORITY = 0,
    AP_PERSISTENT_DATA_MANAGER_EVENTS_HANDLING_PRIORITY,
    AP_API_TO_SPY_CONNECTOR_EVENTS_HANDLING_PRIORITY,
    AP_GLOBAL_VARIABLES_MANAGER_EVENTS_HANDLING_PRIORITY,
    AP_PERFORMANCE_COUNTERS_READERS_EVENTS_HANDLING_PRIORITY,
    AP_APPLICATION_FRAMEWORK_EVENTS_HANDLING_PRIORITY,
    AP_APPLICATION_COMPONENTS_MANAGERS_EVENTS_HANDLING_PRIORITY,
    AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY,
    AP_NUMBER_OF_EVENTS_HANDLING_PRIORITIES
};

class AP_API apEventsHandler
{
public:
    ~apEventsHandler();
    static apEventsHandler& instance();
private:
    apEventsHandler(); // Only the apEventsHandler::instance is allowed to construct this class

public:
    // Enables adding data to debugged process events:
    void registerEventsFiller(gtAutoPtr<apIEventsFiller> aptrEventsFiller);

    // Enables clients to listen to events:
    void registerEventsObserver(apIEventsObserver& observer, apEventsHandlingPriority priority);
    void unregisterEventsObserver(apIEventsObserver& observer, bool assertOnFail = true);

    // Enables clients to listen to the registration of events:
    void registerEventsRegistrationObserver(apIEventsObserver& observer);
    void unregisterEventsRegistrationObserver(apIEventsObserver& observer);

    // Notification mechanism functions:
    // (See "Notification mechanism overview" at the top of pdProcessDebugger.cpp)
    typedef void PENDING_EVENT_NOTIFYER_PROC(const apEvent& pendingEvent);
    void registerPendingEventNotificationCallback(PENDING_EVENT_NOTIFYER_PROC* pFunc);
    void unregisterPendingEventNotificationCallback(PENDING_EVENT_NOTIFYER_PROC* pFunc);
    bool handlePendingDebugEvent();
    void suspendEventsNotifications(bool shouldSuspendEventsNotifications);
    bool handleDebugEvent(apEvent& eve);
    bool areNoEventsPending() {return _pendingEvents.isEmpty();};
    bool isDuringEventHandling() {return _isHandlingPendingDebugEvent;};

    // Notice - this function is called by the debugger thread asynchronously !
    void registerPendingDebugEvent(apEvent& eve);

private:
    void notifyObservers(const apEvent& eve);
    void notifyPendingEventsNotifiers(const apEvent& pendingEvent);
    void appendEventDescriptionToString(const apEvent& eve, gtString& eventMessage);
    void outputHandlingEventDebugLogMessage(const apEvent& eve);
    void outputRegisteringEventDebugLogMessage(const apEvent& eve);
    void outputOnEventHandling(const apIEventsObserver* pEventObserver, bool isStarting, bool wasVetoed);

private:
    friend class apSingeltonsDelete;
    // The "pending events queue" - Holds events that are waiting to be handled
    // by the application main thread:
    osSynchronizedQueue<apEvent*> _pendingEvents;
    int* m_pPendingEventsByType;

    // Adds data to debugged process events:
    apIEventsFiller* _pEventsFiller;

private:
    // Holds the registered events observers:
    gtVector<apIEventsObserver*> _registeredEventObservers[AP_NUMBER_OF_EVENTS_HANDLING_PRIORITIES];

    // Holds the registered events registration observers:
    gtVector<apIEventsObserver*> _registeredEventsRegistrationObservers;

    // Function that will be called asynchronously by the debugger thread
    // whenever a debugged process is placed in the "pending events queue".
    gtVector<PENDING_EVENT_NOTIFYER_PROC*> _pendingEventsNotifierFunctions;

    // If true, events are only logged and not notified to registered observers:
    bool _suspendEventsNotifications;

    static apEventsHandler* _pMySingleInstance;

    // True iff we are currently handling a pending debug event (otherwise wx idle
    // time during updating information will cause two events to be handled at the
    // same time:
    bool _isHandlingPendingDebugEvent;
    bool _areMoreEventsPending;

    // critical section for registering event notifiers:
    osCriticalSection m_registerCriticalSection;
};

#endif //__APEVENTSHANDLER_H

