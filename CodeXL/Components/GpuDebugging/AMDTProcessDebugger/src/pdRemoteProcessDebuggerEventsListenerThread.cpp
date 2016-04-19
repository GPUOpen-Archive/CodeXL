//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdRemoteProcessDebuggerEventsListenerThread.cpp
///
//==================================================================================

//------------------------------ pdRemoteProcessDebuggerEventsListenerThread.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// Local:
#include <src/pdRemoteProcessDebuggerEventsListenerThread.h>
#include "osTimeInterval.h"

// Used for reading from the pipe:
static const gtSize_t s_sizeOfInt32 = sizeof(gtInt32);


// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebuggerEventsListenerThread::pdRemoteProcessDebuggerEventsListenerThread
// Description: Constructor
// Author:      Uri Shomroni
// Date:        12/8/2009
// ---------------------------------------------------------------------------
pdRemoteProcessDebuggerEventsListenerThread::pdRemoteProcessDebuggerEventsListenerThread()
    : osThread(L"pdRemoteProcessDebuggerEventsListenerThread"), _pEventChannel(NULL), _listeningPaused(true)
{

}

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebuggerEventsListenerThread::~pdRemoteProcessDebuggerEventsListenerThread
// Description: Destructor
// Author:      Uri Shomroni
// Date:        12/8/2009
// ---------------------------------------------------------------------------
pdRemoteProcessDebuggerEventsListenerThread::~pdRemoteProcessDebuggerEventsListenerThread()
{
    beforeTermination();
}

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #pragma warning(push)
    #pragma warning(disable: 4702)
#endif
// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebuggerEventsListenerThread::entryPoint
// Description: The thread's main loop - reads events from the process debugging
//              server and transfers them to the application
// Author:      Uri Shomroni
// Date:        12/8/2009
// ---------------------------------------------------------------------------
int pdRemoteProcessDebuggerEventsListenerThread::entryPoint()
{
    int retVal = 0;
    apEventsHandler& theEventsHandler = apEventsHandler::instance();

    while (false == _terminated)
    {
        // Pause if needed:
        osWaitForFlagToTurnOff(_listeningPaused, OS_CHANNEL_INFINIT_TIME_OUT);

        if (_pEventChannel != NULL)
        {
            // Read the event type:
            gtInt32 eventTypeAsInt32 = 0;
            bool rcRead = _pEventChannel->read((gtByte*)&eventTypeAsInt32, s_sizeOfInt32);

            // The above will fail when closing the pipe, so don't assert here:
            if (rcRead && _terminated == false)
            {
                gtAutoPtr<osTransferableObject> aptrEventAsTransferableObject;
                *_pEventChannel >> aptrEventAsTransferableObject;

                // Make sure we got a transferableObject and it is an event:
                GT_IF_WITH_ASSERT(aptrEventAsTransferableObject.pointedObject() != NULL)
                {
                    GT_IF_WITH_ASSERT(aptrEventAsTransferableObject->isEventObject())
                    {
                        apEvent* pEve = (apEvent*)aptrEventAsTransferableObject.pointedObject();

                        // Register the event:
                        if (pEve != NULL)
                        {
                            GT_ASSERT((apEvent::EventType)eventTypeAsInt32 == pEve->eventType());
                            theEventsHandler.registerPendingDebugEvent(*pEve);
                        }
                    }
                }
            }
            else
            {
                // We failed reading, so wait until we are told to start listening again:
                _listeningPaused = true;
            }
        }
        else // _pEventChannel == NULL
        {
            static bool isFirstTime = true;

            if (isFirstTime)
            {
                // Only report this once, to avoid flooding the log:
                isFirstTime = false;

                GT_ASSERT(_pEventChannel != NULL);
            }
        }
    }

    return retVal;
}
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #pragma warning(pop)
#endif

// ---------------------------------------------------------------------------
// Name:        pdRemoteProcessDebuggerEventsListenerThread::beforeTermination
// Description: Called before the thread is terminated
// Author:      Uri Shomroni
// Date:        12/8/2009
// ---------------------------------------------------------------------------
void pdRemoteProcessDebuggerEventsListenerThread::beforeTermination()
{
    _terminated = true;

    if (_pEventChannel != nullptr)
    {
        _pEventChannel->setReadOperationTimeOut(0);
    }

    osTimeInterval timeout;
    timeout.setAsMilliSeconds(100);
    waitForThreadEnd(timeout);
}

