//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apEventsHandler.cpp
///
//==================================================================================

//------------------------------ apEventsHandler.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTAPIClasses/Include/Events/apApiConnectionEndedEvent.h>
#include <AMDTAPIClasses/Include/Events/apApiConnectionEstablishedEvent.h>
#include <AMDTAPIClasses/Include/Events/apComputeContextCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apComputeContextDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessOutputStringEvent.h>
#include <AMDTAPIClasses/Include/Events/apDeferredCommandEvent.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>
#include <AMDTAPIClasses/Include/Events/apGDBErrorEvent.h>
#include <AMDTAPIClasses/Include/Events/apGDBOutputStringEvent.h>
#include <AMDTAPIClasses/Include/Events/apIEventsFiller.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTAPIClasses/Include/Events/apModuleLoadedEvent.h>
#include <AMDTAPIClasses/Include/Events/apModuleUnloadedEvent.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLQueueCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLQueueDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOutputDebugStringEvent.h>
#include <AMDTAPIClasses/Include/Events/apRenderContextCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apRenderContextDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apSpyProgressEvent.h>
#include <AMDTAPIClasses/Include/Events/apThreadCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apThreadTerminatedEvent.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>

apEventsHandler* apEventsHandler::_pMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        apEventsHandler::apEventsHandler
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        19/4/2009
// ---------------------------------------------------------------------------
apEventsHandler::apEventsHandler():
    m_pPendingEventsByType(nullptr), _pEventsFiller(NULL), _suspendEventsNotifications(false), _isHandlingPendingDebugEvent(false), _areMoreEventsPending(false)
{
    m_pPendingEventsByType = new int[apEvent::AP_NUMBER_OF_EVENT_TYPES];
    ::memset(m_pPendingEventsByType, 0, sizeof(int) * apEvent::AP_NUMBER_OF_EVENT_TYPES);
}

// ---------------------------------------------------------------------------
// Name:        apEventsHandler::~apEventsHandler
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        19/4/2009
// ---------------------------------------------------------------------------
apEventsHandler::~apEventsHandler()
{
    delete _pEventsFiller;
    _pEventsFiller = NULL;

    // Pop events from queue and delete memory:
    while (_pendingEvents.size() > 0)
    {
        apEvent* pEvent = _pendingEvents.front();
        _pendingEvents.pop();

        if (pEvent != NULL)
        {
            delete pEvent;
        }
    }

    delete[] m_pPendingEventsByType;
    m_pPendingEventsByType = nullptr;
}

// ---------------------------------------------------------------------------
// Name:        apEventsHandler::instance
// Description: Returns this class' single instance
// Author:  AMD Developer Tools Team
// Date:        20/4/2009
// ---------------------------------------------------------------------------
apEventsHandler& apEventsHandler::instance()
{
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new apEventsHandler;
    }

    return *_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        apEventsHandler::registerEventsFiller
// Description:
//  Registers the current debugged process events filler.
//  (A class that adds data to debugged process events)
// Author:  AMD Developer Tools Team
// Date:        30/9/2004
// ---------------------------------------------------------------------------
void apEventsHandler::registerEventsFiller(gtAutoPtr<apIEventsFiller> aptrEventsFiller)
{
    // Delete the former events filler (if exists):
    delete _pEventsFiller;

    // Register the new events filler:
    _pEventsFiller = aptrEventsFiller.releasePointedObjectOwnership();
}

// ---------------------------------------------------------------------------
// Name:        apEventsHandler::registerEventsObserver
// Description: Registers an observer to listen to the debugged process events.
// Arguments:   observer - The observer to be registered.
// Author:  AMD Developer Tools Team
// Date:        20/12/2003
// ---------------------------------------------------------------------------
void apEventsHandler::registerEventsObserver(apIEventsObserver& observer, apEventsHandlingPriority priority)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(priority < AP_NUMBER_OF_EVENTS_HANDLING_PRIORITIES)
    {
        bool wasObserverRegistered = false;

        // First - Try to look for an empty spot (NULL pointer) in the current vector:
        gtVector<apIEventsObserver*>& observersInPriority = _registeredEventObservers[priority];
        int numberOfObserversInPriority = (int)observersInPriority.size();

        for (int i = 0; numberOfObserversInPriority > i; i++)
        {
            apIEventsObserver*& rpCurrentObserver = observersInPriority[i];

            if (NULL == rpCurrentObserver)
            {
                rpCurrentObserver = &observer;
                wasObserverRegistered = true;
                break;
            }
            else if (&observer == rpCurrentObserver)
            {
                // verify no observer gets registered twice:
                GT_ASSERT(false);
                wasObserverRegistered = true;
                break;
            }
        }

        // If we didn't find an empty spot - add the registration to the end of the vector:
        if (!wasObserverRegistered)
        {
            observersInPriority.push_back(&observer);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        apEventsHandler::unregisterEventsObserver
// Description: Removes the registration of a debugged process observer.
//              I.E: It will stop getting notifications about the debugged process events.
// Arguments:   observer - The observer to be unregistered.
// Author:  AMD Developer Tools Team
// Date:        20/12/2003
// Implementation notes:
//  To remove an observer registration - we set its _registeredEventObservers to NULL.
//  Previous implementation removed it from the _registeredEventObservers vector. This
//  caused a crash when an observer called unregisterEventsObserver from an
//  observer callback function.
// ---------------------------------------------------------------------------
void apEventsHandler::unregisterEventsObserver(apIEventsObserver& observer, bool assertOnFail)
{
    bool observerFound = false;

    // Iterate the observer priorities:
    for (int pri = 0; pri < AP_NUMBER_OF_EVENTS_HANDLING_PRIORITIES; pri++)
    {
        gtVector<apIEventsObserver*>& observersInPriority = _registeredEventObservers[pri];
        int numberOfObserversInPriority = (int)observersInPriority.size();

        // Iterate the registered observers:
        for (int i = 0; numberOfObserversInPriority > i; i++)
        {
            // If we found the input observer:
            apIEventsObserver*& rpCurrentObserver = observersInPriority[i];

            if (&observer == rpCurrentObserver)
            {
                // Remove its registration:
                rpCurrentObserver = NULL;

                observerFound = true;

                // Exit the loop:
                break;
            }
        }
    }

    // The process debuggers manager unregisters all process debuggers periodically, so don't assert here in that case:
    GT_ASSERT(observerFound || !assertOnFail);
}

// ---------------------------------------------------------------------------
// Name:        apEventsHandler::registerEventsObserver
// Description: Registers an observer to listen to the registration of events.
// Arguments:   observer - The observer to be registered.
// Author:  AMD Developer Tools Team
// Date:        21/4/2009
// ---------------------------------------------------------------------------
void apEventsHandler::registerEventsRegistrationObserver(apIEventsObserver& observer)
{
    bool wasObserverRegistered = false;

    // First - Try to look for an empty spot (NULL pointer) in the current vector:
    int numberOfRegistrationObservers = (int)_registeredEventsRegistrationObservers.size();

    for (int i = 0; numberOfRegistrationObservers > i; i++)
    {
        apIEventsObserver*& rpCurrentRegistrationObserver = _registeredEventsRegistrationObservers[i];

        if (NULL == rpCurrentRegistrationObserver)
        {
            rpCurrentRegistrationObserver = &observer;
            wasObserverRegistered = true;
            break;
        }
        else if (&observer == rpCurrentRegistrationObserver)
        {
            // verify no observer gets registered twice:
            GT_ASSERT(false);
            wasObserverRegistered = true;
            break;
        }
    }

    // If we didn't find an empty spot - add the registration to the end of the vector:
    if (!wasObserverRegistered)
    {
        _registeredEventsRegistrationObservers.push_back(&observer);
    }
}

// ---------------------------------------------------------------------------
// Name:        apEventsHandler::unregisterEventsObserver
// Description: Removes the registration of an object registration observer.
//              I.E: It will stop getting notifications about events being registered.
// Arguments:   observer - The observer to be unregistered.
// Author:  AMD Developer Tools Team
// Date:        21/4/2009
// Implementation notes:
//  To remove an observer registration - we set its _registeredEventObservers to NULL.
//  Previous implementation removed it from the _registeredEventObservers vector. This
//  caused a crash when an observer called unregisterEventsObserver from an
//  observer callback function.
// ---------------------------------------------------------------------------
void apEventsHandler::unregisterEventsRegistrationObserver(apIEventsObserver& observer)
{
    int numberOfRegistrationObservers = (int)_registeredEventsRegistrationObservers.size();

    // Iterate the registered observers:
    for (int i = 0; numberOfRegistrationObservers > i; i++)
    {
        apIEventsObserver*& rpCurrentRegistrationObserver = _registeredEventsRegistrationObservers[i];

        // If we found the input observer:
        if (&observer == rpCurrentRegistrationObserver)
        {
            // Remove its registration:
            rpCurrentRegistrationObserver = NULL;
            // Exit the loop:
            break;
        }
    }

    // The process debuggers manager unregisters all process debuggers periodically, so don't assert here:
    // GT_ASSERT(observerFound);
}

// ---------------------------------------------------------------------------
// Name:        apEventsHandler::setPendingEventNotificationCallback
// Description:
//   Registers a function that will be called whenever a debug event is
//   placed in the pending debug events queue (I.E: This debug event is now
//   waiting to be handled by the main application thread):
//
// Arguments: pFunc - A function that will be called whenever a new debug event is
//                    placed in the "pending debug events" queue.
// Author:  AMD Developer Tools Team
// Date:        3/4/2004
// ---------------------------------------------------------------------------
void apEventsHandler::registerPendingEventNotificationCallback(PENDING_EVENT_NOTIFYER_PROC* pFunc)
{
    bool wasFunctionRegistered = false;

    m_registerCriticalSection.enter();

    // First - Try to look for an empty spot (NULL pointer) in the current vector:
    int numberOfNotifierFunctions = (int)_pendingEventsNotifierFunctions.size();

    for (int i = 0; numberOfNotifierFunctions > i; i++)
    {
        PENDING_EVENT_NOTIFYER_PROC*& rpCurrentNotifierFunction = _pendingEventsNotifierFunctions[i];

        if (NULL == rpCurrentNotifierFunction)
        {
            rpCurrentNotifierFunction = pFunc;
            wasFunctionRegistered = true;
        }
    }

    // If we didn't find an empty spot - add the registration to the end of the vector:
    if (!wasFunctionRegistered)
    {
        _pendingEventsNotifierFunctions.push_back(pFunc);
    }

    m_registerCriticalSection.leave();
}

// ---------------------------------------------------------------------------
// Name:        apEventsHandler::unregisterPendingEventNotificationCallback
// Description: Un-registers a function from being called when a debug event is
//              placed in the pending debug events queue
// Arguments: pFunc - The function to be unregistered.
// Author:  AMD Developer Tools Team
// Date:        22/11/2007
// ---------------------------------------------------------------------------
void apEventsHandler::unregisterPendingEventNotificationCallback(PENDING_EVENT_NOTIFYER_PROC* pFunc)
{
    m_registerCriticalSection.enter();

    bool foundFunction = false;

    // Iterate the registered functions:
    int numberOfNotifierFunctions = (int)_pendingEventsNotifierFunctions.size();

    for (int i = 0; numberOfNotifierFunctions > i; i++)
    {
        // If we found the input function pointer:
        PENDING_EVENT_NOTIFYER_PROC*& rpCurrentNotifierFunction = _pendingEventsNotifierFunctions[i];

        if (pFunc == rpCurrentNotifierFunction)
        {
            // Remove its registration:
            rpCurrentNotifierFunction = NULL;
            foundFunction = true;

            // Exit the loop:
            break;
        }
    }

    GT_ASSERT(foundFunction)

    m_registerCriticalSection.leave();
}

// ---------------------------------------------------------------------------
// Name:        apEventsHandler::handlePendingDebugEvent
// Description: Is called by the main application thread to handle events
//              that were logged by the debugger thread in the _pendingEvents
//              queue.
// Return Val:  bool - true - at least one event was handled.
//                     false - no events are pending to be handled.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
bool apEventsHandler::handlePendingDebugEvent()
{
    bool retVal = false;

    // If events notifications is not suspended:
    if (!_suspendEventsNotifications)
    {
        bool isThisEventIgnored = false;

        // If there is a pending debug event:
        if (!_pendingEvents.isEmpty())
        {
            // Get the front event out of the queue:
            apEvent* pEvent = _pendingEvents.front();

            // If we get a NULL event (should not happen):
            if (pEvent == NULL)
            {
                GT_ASSERT(pEvent != NULL);
                _pendingEvents.pop();
            }
            else
            {
                // Avoid this function being called over itsself in the stack:
                if (_isHandlingPendingDebugEvent)
                {
                    // Make this event be handled only when we go back up the stack:
                    _areMoreEventsPending = true;
                    isThisEventIgnored = true;
                }
                else
                {
                    _isHandlingPendingDebugEvent = true;

                    // Remove the event from the queue:
                    _pendingEvents.pop();
                    apEvent::EventType eveType = pEvent->eventType();
                    --(m_pPendingEventsByType[eveType]);
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
                    GT_ASSERT(0 <= m_pPendingEventsByType[eveType]);
#endif

                    // Handle the event:
                    handleDebugEvent(*pEvent);

                    // Delete the event:
                    delete pEvent;

                    retVal = true;

                    // Clear the flag:
                    _isHandlingPendingDebugEvent = false;
                }
            }
        }

        // If somewhere while handling events, we tried to handle another event, notify the observers:
        _areMoreEventsPending = _areMoreEventsPending || (!_pendingEvents.isEmpty());

        if (_areMoreEventsPending && (!isThisEventIgnored))
        {
            if (!_pendingEvents.isEmpty())
            {
                apEvent* pFormerlyIgnoredEvent = _pendingEvents.front();

                if (pFormerlyIgnoredEvent != NULL)
                {
                    notifyPendingEventsNotifiers(*pFormerlyIgnoredEvent);
                }
                else
                {
                    // Remove the NULL event:
                    _pendingEvents.pop();
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apEventsHandler::suspendEventsNotifications
// Description:
// Arguments:
//  shouldSuspendEventsNotifications - true - events are only logged and not
//                                            notified to registered observers.
//                                     false - logged events are notified to
//                                             registered observers.
// Author:  AMD Developer Tools Team
// Date:        16/5/2007
// ---------------------------------------------------------------------------
void apEventsHandler::suspendEventsNotifications(bool shouldSuspendEventsNotifications)
{
    _suspendEventsNotifications = shouldSuspendEventsNotifications;
}

// ---------------------------------------------------------------------------
// Name:        apEventsHandler::handleDebugEvent
// Description:
//  Handles a debug event immediately.
//  Is called by the application thread.
// Arguments: event - The event to be handled.
// Author:  AMD Developer Tools Team
// Date:        17/11/2005
// ---------------------------------------------------------------------------
bool apEventsHandler::handleDebugEvent(apEvent& eve)
{
    // Get the current log file debug log severity:
    osDebugLogSeverity currentDebugLogSeverity = osDebugLog::instance().loggedSeverity();

    // Output "starting to handle event" debug log message:
    if (OS_DEBUG_LOG_DEBUG <= currentDebugLogSeverity)
    {
        outputHandlingEventDebugLogMessage(eve);
    }

    // Fill external data into the event:
    if (_pEventsFiller != NULL)
    {
        _pEventsFiller->fillEvent(eve);
    }

    // Notify the registered observers of the debug event:
    notifyObservers(eve);

    // Output "ended handling event" message:
    if (OS_DEBUG_LOG_DEBUG <= currentDebugLogSeverity)
    {
        OS_OUTPUT_DEBUG_LOG(AP_STR_EndedEventHandling, OS_DEBUG_LOG_DEBUG);
    }

    // In the current implementation, we do not fail:
    return true;
}

// ---------------------------------------------------------------------------
// Name:        apEventsHandler::registerPendingDebugEvent
// Description:
//  Is called by the debugger thread asynchronously to register a debug event
//  to be handled by the main application thread.
// Arguments:   event - The event to be registered.
// Author:  AMD Developer Tools Team
// Date:        1/4/2004
// ---------------------------------------------------------------------------
void apEventsHandler::registerPendingDebugEvent(apEvent& pendingEvent)
{
    // Get the current log file debug log severity:
    osDebugLogSeverity currentDebugLogSeverity = osDebugLog::instance().loggedSeverity();

    // Output "starting to handle event" debug log message:
    if (OS_DEBUG_LOG_DEBUG <= currentDebugLogSeverity)
    {
        outputRegisteringEventDebugLogMessage(pendingEvent);
    }

    bool wasEventVetoed = false;

    // Notify the events registration observers about the event about to be
    // registered, allowing them to change or veto the event before it is
    // added to the events queue. Note that vetoing an event does not allow
    // its registration to be reported to other registration observers.
    int numberOfRegistrationObservers = (int)_registeredEventsRegistrationObservers.size();

    for (int i = 0; numberOfRegistrationObservers > i; i++)
    {
        apIEventsObserver* pCurrentRegistrationObserver = _registeredEventsRegistrationObservers[i];

        if (NULL != pCurrentRegistrationObserver)
        {
            // Print a log message that the event handling is starting:
            outputOnEventHandling(pCurrentRegistrationObserver, true, wasEventVetoed);

            pCurrentRegistrationObserver->onEventRegistration(pendingEvent, wasEventVetoed);

            // Print a log message that the event handling had ended:
            outputOnEventHandling(pCurrentRegistrationObserver, false, wasEventVetoed);
        }
    }

    // If the event is not vetoed, add it to the event queue:
    if (!wasEventVetoed)
    {
        // Clone the event:
        apEvent* pEventCopy = pendingEvent.clone();

        GT_IF_WITH_ASSERT(pEventCopy != NULL)
        {
            // Store the event in the _pendingEvents queue:
            _pendingEvents.push(pEventCopy);
            apEvent::EventType eveCopyType = pEventCopy->eventType();
            ++(m_pPendingEventsByType[eveCopyType]);
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
            GT_ASSERT(0 < m_pPendingEventsByType[eveCopyType]);
#endif

            // Notify pending event notifiers about the pending event:
            notifyPendingEventsNotifiers(pendingEvent);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        apEventsHandler::notifyObservers
// Description: Notifies all the registered observers that a debugged process
//              event occurred.
// Arguments:   event - A class holding information about the debugged process event.
//                      It should be down-casted to the appropriate event sub-class.
// Author:  AMD Developer Tools Team
// Date:        20/12/2003
// ---------------------------------------------------------------------------
void apEventsHandler::notifyObservers(const apEvent& eve)
{
    // Some observers can prevent the following observers from seeing the events:
    bool wasEventVetoed = false;

    // Certain event types are superseded by other events of the same type. Automatically veto these:
    apEvent::EventType eveType = eve.eventType();

    if ((apEvent::GD_MONITORED_OBJECT_SELECTED_EVENT    == eveType) ||
        (apEvent::GD_MONITORED_OBJECT_ACTIVATED_EVENT   == eveType) ||
        (apEvent::APP_UPDATE_UI_EVENT                   == eveType))
    {
        wasEventVetoed = (0 < m_pPendingEventsByType[eveType]);

        if (wasEventVetoed)
        {
            OS_OUTPUT_DEBUG_LOG(AP_STR_EventSuperseded, OS_DEBUG_LOG_DEBUG);
        }
    }

    // Iterate the observer priorities:
    for (int pri = 0; pri < AP_NUMBER_OF_EVENTS_HANDLING_PRIORITIES; pri++)
    {
        // Iterate the registered observers:
        gtVector<apIEventsObserver*>& observersInPriority = _registeredEventObservers[pri];
        int numberOfObserversInPriority = (int)observersInPriority.size();

        for (int i = 0; (numberOfObserversInPriority > i) && (!wasEventVetoed); i++)
        {
            apIEventsObserver* pCurrentObserver = observersInPriority[i];

            if (NULL != pCurrentObserver)
            {
                // Print a log message that the event handling is starting:
                outputOnEventHandling(pCurrentObserver, true, wasEventVetoed);

                // Notify the current observer:
                pCurrentObserver->onEvent(eve, wasEventVetoed);

                // Print a log message that the event handling had ended:
                outputOnEventHandling(pCurrentObserver, false, wasEventVetoed);
            }
        }

        if (wasEventVetoed)
        {
            // If we were vetoed, stop running:
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        apEventsHandler::notifyPendingEventsNotifiers
// Description:
//  Notifies pending events notifiers that there is a pending event
//  waiting to be handled.
//  This function is called a-synchronically by the debugged process.
//
// Arguments: pendingEvent - The pending event.
//
// Author:  AMD Developer Tools Team
// Date:        22/11/2007
// ---------------------------------------------------------------------------
void apEventsHandler::notifyPendingEventsNotifiers(const apEvent& pendingEvent)
{
    m_registerCriticalSection.enter();

    // Will hold the amount of pending event notifiers:
    int amountOfPendingEventsNotifiers = 0;

    // Iterate the registered pending event notifiers:
    int numberOfNotifierFunctions = (int)_pendingEventsNotifierFunctions.size();

    for (int i = 0; numberOfNotifierFunctions > i; i++)
    {
        PENDING_EVENT_NOTIFYER_PROC* pCurrentNotifierFunction = _pendingEventsNotifierFunctions[i];

        if (NULL != pCurrentNotifierFunction)
        {
            // Notify the current pending event notifiers:
            pCurrentNotifierFunction(pendingEvent);
            amountOfPendingEventsNotifiers++;
        }
    }

    // Verify that we have at least one pending event notifier:
    GT_ASSERT(0 < amountOfPendingEventsNotifiers);

    m_registerCriticalSection.leave();
}


// ---------------------------------------------------------------------------
// Name:        apEventsHandler::appendEventDescriptionToString
// Description:
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        5/7/2012
// ---------------------------------------------------------------------------
void apEventsHandler::appendEventDescriptionToString(const apEvent& eve, gtString& eventMessage)
{
    // Act according to the event type:
    apEvent::EventType eventType = eve.eventType();

    switch (eventType)
    {
        case apEvent::AP_API_CONNECTION_ESTABLISHED:
        {
            eventMessage += AP_STR_HandlingAPIConnectionEstablished;
            apAPIConnectionType establishedConnection = ((const apApiConnectionEstablishedEvent&)eve).establishedConnectionType();
            gtString establishedConnectionAsStr;
            apAPIConnectionTypeToString(establishedConnection, establishedConnectionAsStr);
            eventMessage += L": ";
            eventMessage += establishedConnectionAsStr;
        }
        break;

        case apEvent::AP_API_CONNECTION_ENDED:
        {
            eventMessage += AP_STR_HandlingAPIConnectionEnded;
            apAPIConnectionType endedConnection = ((const apApiConnectionEndedEvent&)eve).connectionType();
            gtString endedConnectionAsStr;
            apAPIConnectionTypeToString(endedConnection, endedConnectionAsStr);
            eventMessage += L": ";
            eventMessage += endedConnectionAsStr;
        }
        break;

        case apEvent::AP_COMPUTE_CONTEXT_CREATED_EVENT:
        {
            int contextId = ((const apComputeContextCreatedEvent&)eve).contextId();
            eventMessage.appendFormattedString(AP_STR_HandlingOpenCLComputeContextCreated, contextId);
        }
        break;

        case apEvent::AP_COMPUTE_CONTEXT_DELETED_EVENT:
        {
            int contextId = ((const apComputeContextDeletedEvent&)eve).contextId();
            eventMessage.appendFormattedString(AP_STR_HandlingOpenCLComputeContextDeleted, contextId);
        }
        break;

        case apEvent::AP_BEFORE_DEBUGGED_PROCESS_RUN_RESUMED:
        {
            eventMessage += AP_STR_HandlingBeforeProcessRunResumed;
        }
        break;

        case apEvent::AP_BREAKPOINT_HIT:
        {
            eventMessage += AP_STR_HandlingBreakPointHit;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_OUTPUT_STRING:
        {
            // Get the debugged process output string:
            const gtString& debuggedProcessOutputString = ((const apDebuggedProcessOutputStringEvent&)eve).debuggedProcessOutputString();
            eventMessage += AP_STR_HandlingDebuggedProcessOutput;
            eventMessage += debuggedProcessOutputString;
        }
        break;

        case apEvent::AP_DETECTED_ERROR_EVENT:
        {
            eventMessage += AP_STR_HandlingDebuggedProcessError;
        }
        break;

        case apEvent::AP_EXCEPTION:
        {
            const apExceptionEvent& excEve = (const apExceptionEvent&)eve;
            eventMessage.appendFormattedString(AP_STR_HandlingSecondChanceException, excEve.exceptionReason(), (unsigned long long)excEve.exceptionAddress());
        }
        break;

        case apEvent::AP_GDB_ERROR:
        {
            // Get the GDB's error string:
            const gtString& gdbErrorString = ((const apGDBErrorEvent&)eve).gdbErrorString();
            eventMessage += AP_STR_HandlingGDBError;
            eventMessage += gdbErrorString;
        }
        break;

        case apEvent::AP_GDB_LISTENER_THREAD_WAS_SUSPENDED_EVENT:
        {
            eventMessage += AP_STR_HandlingGDBListenerThreadSuspended;
        }
        break;

        case apEvent::AP_GDB_OUTPUT_STRING:
        {
            // Get the GDB's output string:
            const gtString& gdbOutputString = ((const apGDBOutputStringEvent&)eve).gdbOutputString();
            eventMessage += AP_STR_HandlingGDBOutput;
            eventMessage += gdbOutputString;
        }
        break;

        case apEvent::AP_INFRASTRUCTURE_ENDS_BEING_BUSY_EVENT:
        {
            eventMessage += AP_STR_HandlingInfraEndsBeingBusy;
        }
        break;

        case apEvent::AP_INFRASTRUCTURE_FAILURE_EVENT:
        {
            eventMessage += AP_STR_HandlingInfraFailure;
        }
        break;

        case apEvent::AP_INFRASTRUCTURE_STARTS_BEING_BUSY_EVENT:
        {
            eventMessage += AP_STR_HandlingInfraStartsBeingBusy;
        }
        break;

        case apEvent::AP_MEMORY_LEAK:
        {
            eventMessage += AP_STR_HandlingMemoryLeak;
        }
        break;

        case apEvent::AP_MODULE_LOADED:
        {
            // Get the loaded module path:
            const gtString& modulePath = ((const apModuleLoadedEvent&)eve).modulePath();

            // Create the log message:
            eventMessage += AP_STR_HandlingDLLLoad;
            eventMessage += modulePath;
        }
        break;

        case apEvent::AP_MODULE_UNLOADED:
        {
            // Get the unloaded module path:
            const gtString& modulePath = ((const apModuleUnloadedEvent&)eve).modulePath();

            // Create the log message:
            eventMessage += AP_STR_HandlingDLLUnload;

            if (!modulePath.isEmpty())
            {
                eventMessage += modulePath;
            }
            else
            {
                eventMessage += AP_STR_NotAvailable;
            }
        }
        break;

        case apEvent::AP_OUTPUT_DEBUG_STRING:
        {
            // Get the debug string:
            const gtString& debugString = ((const apOutputDebugStringEvent&)eve).debugString();
            eventMessage += AP_STR_HandlingDebugString;
            eventMessage += debugString;
        }
        break;

        case apEvent::AP_USER_WARNING:
        {
            eventMessage += AP_STR_HandlingUserWarning;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        {
            eventMessage += AP_STR_HandlingProcessCreation;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        {
            eventMessage += AP_STR_HandlingProcessRunResumed;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED:
        {
            eventMessage += AP_STR_HandlingProcessRunStarted;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED_EXTERNALLY:
        {
            eventMessage += AP_STR_HandlingProcessRunStartedExternally;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
        {
            eventMessage += AP_STR_HandlingProcessSuspension;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        {
            eventMessage += AP_STR_HandlingProcessTermination;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_IS_DURING_TERMINATION:
        {
            eventMessage += AP_STR_handlingProcessDuringTermination;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE:
        {
            eventMessage += AP_STR_HandlingProcessCreationFailure;
        }
        break;

        case apEvent::AP_RENDER_CONTEXT_CREATED_EVENT:
        {
            int contextId = ((const apRenderContextCreatedEvent&)eve).contextId();
            eventMessage.appendFormattedString(AP_STR_HandlingOpenGLRenderContextCreated, contextId);
        }
        break;

        case apEvent::AP_RENDER_CONTEXT_DELETED_EVENT:
        {
            int contextId = ((const apRenderContextDeletedEvent&)eve).contextId();
            eventMessage.appendFormattedString(AP_STR_HandlingOpenGLRenderContextDeleted, contextId);
        }
        break;

        case apEvent::AP_CONTEXT_UPDATED_EVENT:
        {
            eventMessage += AP_STR_handlingContextUpdate;
        }
        break;

        case apEvent::AP_GL_DEBUG_OUTPUT_MESSAGE:
        {
            eventMessage += AP_STR_HandlingGLDebugOutputMessage;
        }
        break;

        case apEvent::AP_SEARCHING_FOR_MEMORY_LEAKS:
        {
            eventMessage += AP_STR_HandlingSearchingForMemoryLeaks;
        }
        break;

        case apEvent::AP_THREAD_CREATED:
        {
            // Get the thread id:
            osThreadId threadId = ((const apThreadCreatedEvent&)eve).threadOSId();

            // Create the log message:
            eventMessage.appendFormattedString(AP_STR_HandlingThreadCreated, threadId);
        }
        break;

        case apEvent::AP_THREAD_TERMINATED:
        {
            // Get the thread id:
            osThreadId threadId = ((const apThreadTerminatedEvent&)eve).threadOSId();

            // Create the log message:
            eventMessage.appendFormattedString(AP_STR_HandlingThreadTerminated, threadId);
        }
        break;

        case apEvent::AP_OPENCL_ERROR:
        {
            eventMessage += AP_STR_HandlingOpenCLError;
        }
        break;

        case apEvent::AP_OPENCL_QUEUE_CREATED_EVENT:
        {
            eventMessage.appendFormattedString(AP_STR_HandlingOpenCLQueueCreated, ((const apOpenCLQueueCreatedEvent&)eve).queueID() + 1);
        }
        break;

        case apEvent::AP_OPENCL_QUEUE_DELETED_EVENT:
        {
            eventMessage.appendFormattedString(AP_STR_HandlingOpenCLQueueDeleted, ((const apOpenCLQueueDeletedEvent&)eve).queueID() + 1);
        }
        break;

        case apEvent::AP_OPENCL_PROGRAM_CREATED_EVENT:
        {
            eventMessage.appendFormattedString(AP_STR_HandlingOpenCLProgramCreated, ((const apOpenCLProgramCreatedEvent&)eve).programIndex() + 1);
        }
        break;

        case apEvent::AP_OPENCL_PROGRAM_DELETED_EVENT:
        {
            eventMessage.appendFormattedString(AP_STR_HandlingOpenCLProgramDeleted, ((const apOpenCLProgramDeletedEvent&)eve).programIndex() + 1);
        }
        break;

        case apEvent::AP_SPY_PROGRESS_EVENT:
        {
            eventMessage.appendFormattedString(AP_STR_HandlingSpyProgress, ((const apSpyProgressEvent&)eve).progress());
        }
        break;

        case apEvent::AP_TECHNOLOGY_MONITOR_FAILURE_EVENT:
        {
            eventMessage += AP_STR_handlingTechnologyMonitorFailure;
        }
        break;

        case apEvent::GD_MONITORED_OBJECT_SELECTED_EVENT:
        {
            eventMessage += AP_STR_handlingMonitoredObjectSelected;
        }
        break;

        case apEvent::GD_MONITORED_OBJECT_ACTIVATED_EVENT:
        {
            eventMessage += AP_STR_handlingMonitoredObjectActivated;
        }
        break;

        case apEvent::AP_BEFORE_KERNEL_DEBUGGING_EVENT:
        {
            eventMessage += AP_STR_handlingBeforeKernelDebugging;
        }
        break;

        case apEvent::AP_AFTER_KERNEL_DEBUGGING_EVENT:
        {
            eventMessage += AP_STR_handlingAfterKernelDebugging;
        }
        break;

        case apEvent::AP_KERNEL_CURRENT_WORK_ITEM_CHANGED_EVENT:
        {
            eventMessage += AP_STR_handlingKernelCurrentWorkItemChanged;
        }
        break;

        case apEvent::AP_KERNEL_DEBUGGING_FAILED_EVENT:
        {
            eventMessage += AP_STR_handlingKernelDebuggingFailed;
        }
        break;

        case apEvent::AP_KERNEL_DEBUGGING_INTERRUPTED_EVENT:
        {
            eventMessage += AP_STR_handlingKernelDebuggingInterrupted;
        }
        break;

        case apEvent::AP_FLUSH_TEXTURE_IMAGES_EVENT:
        {
            eventMessage += AP_STR_handlingTextureImagesFlush;
        }
        break;

        case apEvent::AP_BREAKPOINTS_UPDATED_EVENT:
        {
            eventMessage += AP_STR_handlingBreakpointsUpdated;
        }
        break;

        case apEvent::AP_KERNEL_SOURCE_BREAKPOINTS_UPDATED_EVENT:
        {
            eventMessage += AP_STR_handlingKernelSourceBreakpointsUpdated;
        }
        break;

        case apEvent::AP_OPENCL_PROGRAM_BUILD_EVENT:
        {
            eventMessage += AP_STR_handlingProgramBuild;
        }
        break;

        case apEvent::AP_OPENCL_PROGRAM_BUILD_FAILED_WITH_DEBUG_FLAGS_EVENT:
        {
            eventMessage += AP_STR_handlingProgramBuildFailedWithDebugFlags;
        }
        break;

        case apEvent::AP_EXECUTION_MODE_CHANGED_EVENT:
        {
            const apExecutionModeChangedEvent& execModeEve = (const apExecutionModeChangedEvent&)eve;
            eventMessage.append(AP_STR_handingExecutionModeChanged).append(execModeEve.modeType());
            const gtString& sessionType = execModeEve.sessionTypeName();

            if (!sessionType.isEmpty())
            {
                eventMessage.append(AP_STR_handingExecutionModeChangedSessionType).append(sessionType);
            }
        }
        break;

        case apEvent::AP_CALL_STACK_FRAME_SELECTED_EVENT:
        {
            eventMessage += AP_STR_HandlingCallStackFrameSelected;
        }
        break;

        case apEvent::AP_DEFERRED_COMMAND_EVENT:
        {
            const apDeferredCommandEvent& defCmdEve = (const apDeferredCommandEvent&)eve;
            eventMessage.appendFormattedString(AP_STR_handlingDeferredCommand, defCmdEve.command(), defCmdEve.commandTarget(), defCmdEve.getData());
        }
        break;

        case apEvent::AP_ADD_WATCH_EVENT:
        {
            eventMessage += AP_STR_handlingAddWatch;
        }
        break;

        case apEvent::AP_HEX_CHANGED_EVENT:
        {
            eventMessage += AP_STR_handlingHexChanged;
        }
        break;

        case apEvent::AP_MDI_CREATED_EVENT:
        {
            eventMessage += AP_STR_handlingMDICreated;
        }
        break;

        case apEvent::AP_MDI_ACTIVATED_EVENT:
        {
            eventMessage += AP_STR_handlingMDIActivated;
        }
        break;

        case apEvent::AP_MEMORY_ALLOCATION_FAILURE_EVENT:
        {
            eventMessage += AP_STR_handlingMemoryAllocationFailureEvent;
        }
        break;

        case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
        {
            eventMessage += AP_STR_HandlingGlobalVariableChange;
        }
        break;

        case apEvent::APP_UPDATE_UI_EVENT:
        {
            eventMessage += AP_STR_handlingUpdateUI;
        }
        break;

        case apEvent::AP_PROFILE_PROCESS_TERMINATED:
        {
            eventMessage += AP_STR_handlingProfileProcessTerminated;
        }
        break;

        case apEvent::AP_PROFILE_PROGRESS_EVENT:
        {
            eventMessage += AP_STR_handlingProfileProgress;
        }
        break;

        default:
        {
            eventMessage += AP_STR_HandlingUnknownEvent;
            eventMessage.appendFormattedString(L" (%d)", eventType);
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Name:        apEventsHandler::outputHandlingEventDebugLogMessage
// Description: Outputs an appropriate "starting to handle event" debug log message.
// Arguments:   event - The handled event.
// Author:  AMD Developer Tools Team
// Date:        31/8/2005
// ---------------------------------------------------------------------------
void apEventsHandler::outputHandlingEventDebugLogMessage(const apEvent& eve)
{
    // Will get the log message:
    gtString logMessage = AP_STR_HandlingEvent;

    appendEventDescriptionToString(eve, logMessage);

    // Output the debug log file message:
    OS_OUTPUT_DEBUG_LOG(logMessage.asCharArray(), OS_DEBUG_LOG_DEBUG);
}

// ---------------------------------------------------------------------------
// Name:        apEventsHandler::outputRegisteringEventDebugLogMessage
// Description: Called when an event is registered, writes its description to the log
// Author:  AMD Developer Tools Team
// Date:        9/2/2012
// ---------------------------------------------------------------------------
void apEventsHandler::outputRegisteringEventDebugLogMessage(const apEvent& eve)
{
    // Will get the log message:
    gtString logMessage = AP_STR_RegisteringEvent;

    appendEventDescriptionToString(eve, logMessage);

    // Output the debug log file message:
    OS_OUTPUT_DEBUG_LOG(logMessage.asCharArray(), OS_DEBUG_LOG_DEBUG);

}

// ---------------------------------------------------------------------------
// Name:        apEventsHandler::outputOnEventHandling
// Description: Output debug log message for the evnet handling start / end
// Arguments:   const apIEventsObserver* pEventObserver
//              bool isStarting
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        18/8/2010
// ---------------------------------------------------------------------------
void apEventsHandler::outputOnEventHandling(const apIEventsObserver* pEventObserver, bool isStarting, bool wasVetoed)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pEventObserver != NULL)
    {
        // If the current debug log severity if debug level or more, output a log message:
        if (osDebugLog::instance().loggedSeverity() >= OS_DEBUG_LOG_EXTENSIVE)
        {
            gtString dbgMessage;

            if (isStarting)
            {
                // Build the 'Starting handling event' message:
                dbgMessage.appendFormattedString(AP_STR_HandlingEventStart, pEventObserver->eventObserverName());
            }
            else
            {
                // Build the 'Ended handling event' message:
                dbgMessage.appendFormattedString(AP_STR_HandlingEventEnd, pEventObserver->eventObserverName());
            }

            if (wasVetoed)
            {
                dbgMessage.append(AP_STR_EventWasVetoed);
            }

            // Output the debug message:
            OS_OUTPUT_DEBUG_LOG(dbgMessage.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
        }
    }
}
