//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaIncomingSpyEventsListenerThread.cpp
///
//==================================================================================

//------------------------------ gaIncomingSpyEventsListenerThread.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTOSWrappers/Include/osTransferableObject.h>
#include <AMDTAPIClasses/Include/Events/apApiConnectionEstablishedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>

// Local:
#include <src/gaIncomingSpyEventsListenerThread.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// ---------------------------------------------------------------------------
// Name:        gaIncomingSpyEventsListenerThread::gaIncomingSpyEventsListenerThread
// Description: Constructor
// Author:      Uri Shomroni
// Date:        29/10/2009
// ---------------------------------------------------------------------------
gaIncomingSpyEventsListenerThread::gaIncomingSpyEventsListenerThread()
    : osThread(L"gaIncomingSpyEventsListenerThread"), _pIncomingEventsSocket(NULL)
{

}

// ---------------------------------------------------------------------------
// Name:        gaIncomingSpyEventsListenerThread::~gaIncomingSpyEventsListenerThread
// Description: Destructor
// Author:      Uri Shomroni
// Date:        29/10/2009
// ---------------------------------------------------------------------------
gaIncomingSpyEventsListenerThread::~gaIncomingSpyEventsListenerThread()
{
    if (_pIncomingEventsSocket != NULL)
    {
        _pIncomingEventsSocket->close();
        delete _pIncomingEventsSocket;
        _pIncomingEventsSocket = NULL;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaIncomingSpyEventsListenerThread::entryPoint
// Description: The thread main loop. Using a virtual function, we connect to
//              the spy, wait for the spy to open the socket client, then read
//              and forward the debug events we get from the spy.
// Author:      Uri Shomroni
// Date:        29/10/2009
// ---------------------------------------------------------------------------
int gaIncomingSpyEventsListenerThread::entryPoint()
{
    int retVal = -1;

    // Wait for the socket client connection:
    bool rcSock = connectAndWaitForClient();
    GT_IF_WITH_ASSERT(rcSock && (_pIncomingEventsSocket != NULL))
    {
        retVal = 0;

        // Run while the socket us open:
        while ((_pIncomingEventsSocket != NULL) && (_pIncomingEventsSocket->isOpen()))
        {
            // Read events from the pipe and register them with the apEventsHandler:
            gtAutoPtr<osTransferableObject> aptrEventAsTransferableObject;
            *_pIncomingEventsSocket >> aptrEventAsTransferableObject;

            // Stop the loop if we got closed:
            if (!_pIncomingEventsSocket->isOpen())
            {
                // Stop the loop execution:
                break;
            }

            // Make sure we got a transferableObject and it is an event:
            GT_IF_WITH_ASSERT(aptrEventAsTransferableObject.pointedObject() != NULL)
            {
                GT_IF_WITH_ASSERT(aptrEventAsTransferableObject->isEventObject())
                {
                    apEvent* pEve = (apEvent*)aptrEventAsTransferableObject.pointedObject();

                    // Register the event:
                    if (pEve != NULL)
                    {
                        apEventsHandler& theEventsHandler = apEventsHandler::instance();
                        theEventsHandler.registerPendingDebugEvent(*pEve);

                        // Uri, 26/6/11 - We have to synchronize these three events to avoid race conditions
                        // with the process debugger:
                        apEvent::EventType eveType = pEve->eventType();

                        // This type list must match the list in suForwardEventToClient (suSpyAPIFunctions.cpp)!
                        if ((apEvent::AP_BEFORE_KERNEL_DEBUGGING_EVENT == eveType) || (apEvent::AP_AFTER_KERNEL_DEBUGGING_EVENT == eveType) || (apEvent::AP_MEMORY_LEAK == eveType) || (apEvent::AP_DEBUGGED_PROCESS_IS_DURING_TERMINATION == eveType))
                        {
                            // Write back a boolean to synchronize:
                            *_pIncomingEventsSocket << true;
                        }
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaIncomingSpyEventsListenerThread::beforeTermination
// Description: Called before this thread is terminated
// Author:      Uri Shomroni
// Date:        29/10/2009
// ---------------------------------------------------------------------------
void gaIncomingSpyEventsListenerThread::beforeTermination()
{
    terminateConnection();
}
