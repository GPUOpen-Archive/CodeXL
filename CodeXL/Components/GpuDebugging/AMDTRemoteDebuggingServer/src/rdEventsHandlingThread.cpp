//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file rdEventsHandlingThread.cpp
///
//==================================================================================

//------------------------------ rdEventsHandlingThread.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osCriticalSection.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>

// Local:
#include <src/rdEventsHandlingThread.h>

// Static items needed for synchronization:
static bool s_waitingForEvents = true;

// Uri - 12/7/2012 - The events handler now performs synchronization for us, so this is no longer needed.
// Having this in here causes a deadlock, since the rdHandler has the lock outside the apHandler, but the
// registration locks are reversed.
// Should this be needed separately, make sure to restore both critical section references to the same place.
// static osCriticalSection s_eventRegistrationAndHandlingCriticalSection;

// ---------------------------------------------------------------------------
// Name:        rdPendingEventNotificationFunc
// Description: Called when an event is pending, and tells this thread to handle it
// Author:      Uri Shomroni
// Date:        17/8/2009
// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// Name:        rdPendingEventNotificationFunc
// Description: A pending event notification callback function. Since the debugging
//              server doesn't need to synchronize with anything, we simply handle
//              pending events immediately.
// Author:      Uri Shomroni
// Date:        13/8/2009
// ---------------------------------------------------------------------------
void rdPendingEventNotificationFunc(const apEvent& pendingEvent)
{
    // Unused parameter:
    (void)(&pendingEvent);

    // Uri, 12/7/2012 - see note in the top of the file. Enter the critical section:
    // s_eventRegistrationAndHandlingCriticalSection.enter();

    // Set the flag:
    s_waitingForEvents = false;

    // Uri, 12/7/2012 - see note in the top of the file. Leave the critical section:
    // s_eventRegistrationAndHandlingCriticalSection.leave();
}

// ---------------------------------------------------------------------------
// Name:        rdEventsHandlingThread::rdEventsHandlingThread
// Description: Constructor
// Author:      Uri Shomroni
// Date:        17/8/2009
// ---------------------------------------------------------------------------
rdEventsHandlingThread::rdEventsHandlingThread(const gtString& threadName): osThread(threadName)
{

}

// ---------------------------------------------------------------------------
// Name:        rdEventsHandlingThread::~rdEventsHandlingThread
// Description: Destructor
// Author:      Uri Shomroni
// Date:        17/8/2009
// ---------------------------------------------------------------------------
rdEventsHandlingThread::~rdEventsHandlingThread()
{

}

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #pragma warning(push)
    #pragma warning(disable: 4702)
#endif
// ---------------------------------------------------------------------------
// Name:        rdEventsHandlingThread::entryPoint
// Description:
// Return Val: int
// Author:      Uri Shomroni
// Date:        17/8/2009
// ---------------------------------------------------------------------------
int rdEventsHandlingThread::entryPoint()
{
    int retVal = 0;

    apEventsHandler& theEventsHandler = apEventsHandler::instance();

    // Register the function as a pending event notification callback:
    theEventsHandler.registerPendingEventNotificationCallback(rdPendingEventNotificationFunc);

    for (;;)
    {
        // Wait until another thread tells us there are events waiting:
        osWaitForFlagToTurnOff(s_waitingForEvents, ULONG_MAX);

        // Uri, 12/7/2012 - see note in the top of the file. Enter the critical section so multiple events won't stack in the queue:
        // s_eventRegistrationAndHandlingCriticalSection.enter();

        // Handle one debugging event:
        bool rcHandle = theEventsHandler.handlePendingDebugEvent();

        // Only reset the flag if we handled the event and there are no more events waiting:
        s_waitingForEvents = rcHandle && theEventsHandler.areNoEventsPending();

        // Uri, 12/7/2012 - see note in the top of the file. Leave the critical section:
        // s_eventRegistrationAndHandlingCriticalSection.leave();
    }

    return retVal;
}
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #pragma warning(pop)
#endif

// ---------------------------------------------------------------------------
// Name:        rdEventsHandlingThread::beforeTermination
// Description: Called before this thread is terminated
// Author:      Uri Shomroni
// Date:        17/8/2009
// ---------------------------------------------------------------------------
void rdEventsHandlingThread::beforeTermination()
{
    apEventsHandler::instance().unregisterPendingEventNotificationCallback(rdPendingEventNotificationFunc);
}

