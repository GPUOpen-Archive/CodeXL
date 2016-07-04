//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaAPIToSpyConnector.cpp
///
//==================================================================================

//------------------------------ gaAPIToSpyConnector.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osPipeSocketServer.h>
#include <AMDTOSWrappers/Include/osTCPSocketServer.h>
#include <AMDTAPIClasses/Include/Events/apApiConnectionEndedEvent.h>
#include <AMDTAPIClasses/Include/Events/apApiConnectionEstablishedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>

// Local:
#include <src/gaStringConstants.h>
#include <src/gaDebuggedProcessEventsFiller.h>
#include <src/gaPipeSocketSpyConnectionWaiterThread.h>
#include <src/gaIncomingSpyEventsPipeSocketListenerThread.h>
#include <src/gaIncomingSpyEventsTCPIPListenerThread.h>
#include <src/gaTCPIPSpyConnectionWaiterThread.h>
#include <src/gaAPIToSpyConnector.h>

// The size of a shared memory communication buffer:
#define GA_SM_COMMUNICATION_BUFF_SIZE 10240


// Static members initializations:
gaAPIToSpyConnector* gaAPIToSpyConnector::_pMySingleInstance = NULL;


// ---------------------------------------------------------------------------
// Name:        gaProcessDebuggerPendingEventNotificationFunc
//
// Description:
//  Is notified, a-synchronically by the debugger thread, when a debugged process
//  event is added to the pending events queue.
//
// Arguments: pendingEvent - The pending event.
//
// Author:      Yaki Tebeka
// Date:        6/4/2004
// ---------------------------------------------------------------------------
void gaProcessDebuggerPendingEventNotificationFunc(const apEvent& pendingEvent)
{
    // Notify the Spy connector about the pending event:
    gaAPIToSpyConnector& apiToSpyConnector = gaAPIToSpyConnector::instance();
    apiToSpyConnector.onPendingDebugEvent(pendingEvent);
}


// ---------------------------------------------------------------------------
// Name:        gaAPIToSpyConnector::instance
// Description: Returns the single instance of this class. Creates it on
//              the first call to this function.
// Author:      Yaki Tebeka
// Date:        15/2/2004
// ---------------------------------------------------------------------------
gaAPIToSpyConnector& gaAPIToSpyConnector::instance()
{
    // If my single instance was not created yet - create it:
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new gaAPIToSpyConnector;
        GT_ASSERT(_pMySingleInstance);
    }

    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        gaAPIToSpyConnector::gaAPIToSpyConnector
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        15/2/2004
// ---------------------------------------------------------------------------
gaAPIToSpyConnector::gaAPIToSpyConnector()
    : _pSpiesAPIConnectingSocket(NULL),
      _pSpiesAPIConnectionWaiterThread(NULL),
      _pIncomingSpyEventsListenerThread(NULL),
      _isDuringSecondChanceExceptionHandling(false),
      _pPortListener(NULL)
{
    // Initialize the _isAPIConnectionActive array:
    clearIsAPIConnectionActiveArray();

    // Register myself to listen to debugged process events:
    apEventsHandler& theEventsHandler = apEventsHandler::instance();
    theEventsHandler.registerEventsObserver(*this, AP_API_TO_SPY_CONNECTOR_EVENTS_HANDLING_PRIORITY);

    // Register the event filler:
    gtAutoPtr<apIEventsFiller> aptrEventFiller = new gaDebuggedProcessEventsFiller;
    theEventsHandler.registerEventsFiller(aptrEventFiller);

    // Register my pending events notifier:
    theEventsHandler.registerPendingEventNotificationCallback(&gaProcessDebuggerPendingEventNotificationFunc);
}


// ---------------------------------------------------------------------------
// Name:        gaAPIToSpyConnector::~gaAPIToSpyConnector
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        15/2/2004
// ---------------------------------------------------------------------------
gaAPIToSpyConnector::~gaAPIToSpyConnector()
{
    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);

    // Terminate the API connection:
    terminate();
}


// ---------------------------------------------------------------------------
// Name:        gaAPIToSpyConnector::initialize
// Description: Initialize a TCP / IP API <-> Spy connection.
// Arguments:   portAddress - The port address to which the Spy will connect to.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/2/2004
// ---------------------------------------------------------------------------
bool gaAPIToSpyConnector::initialize(const osPortAddress& spyAPIPortAddress, const osPortAddress& incomingEventsPortAddress)
{
    bool retVal = false;

    // Clean up previous connection left overs (if exists):
    terminate();

    // Assert that the port listener was cleared properly, and create a new one
    // if that is the case:
    GT_IF_WITH_ASSERT(_pPortListener == NULL)
    {
        _pPortListener = new osTCPSocketServer;
    }

    if ((_pPortListener != NULL) && !(spyAPIPortAddress == _pPortListener->boundAddress()))
    {
        // If the port listener is open - close it:
        // (We do this because I don't think that we can re-bind an open socket)
        if (_pPortListener->isOpen())
        {
            _pPortListener->close();
        }

        // Open the port listener socket:
        bool rc = _pPortListener->open();
        GT_IF_WITH_ASSERT(rc)
        {
            // Bind the port listener to the input port:
            rc = _pPortListener->bind(spyAPIPortAddress);
            GT_IF_WITH_ASSERT(rc)
            {
                // Listen to the port - allow 1 unhanded pending connections.
                bool rcListen = _pPortListener->listen(1);
                GT_IF_WITH_ASSERT(rcListen)
                {
                    // Create the spy connecting socket:
                    _pSpiesAPIConnectingSocket = new osTCPSocketServerConnectionHandler;

                    // Uri, 4/11/09 - This timeout can be inifinite, since the if process will be stopped, we will kill the thread.
                    _pSpiesAPIConnectingSocket->setReadOperationTimeOut(OS_CHANNEL_INFINITE_TIME_OUT);

                    // Create a threads that will accept and handle the first incoming connection requests:
                    _pSpiesAPIConnectionWaiterThread = new gaTCPIPSpyConnectionWaiterThread(*_pPortListener, *((osTCPSocketServerConnectionHandler*)_pSpiesAPIConnectingSocket), AP_SPIES_UTILITIES_API_CONNECTION);

                    // Cleanup (see comment at gaAPIToSpyConnector::terminate)
                    delete _pIncomingSpyEventsListenerThread;
                    _pIncomingSpyEventsListenerThread = NULL;

                    // Create the spy events listener thread:
                    _pIncomingSpyEventsListenerThread = new gaIncomingSpyEventsTCPIPListenerThread(incomingEventsPortAddress);

                    // Execute the threads:
                    bool rc3 = _pSpiesAPIConnectionWaiterThread->execute();
                    bool rc4 = _pIncomingSpyEventsListenerThread->execute();
                    GT_IF_WITH_ASSERT(rc3 && rc4)
                    {
                        retVal = true;
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaAPIToSpyConnector::initialize
// Description: Initialize a shared memory connection with the spy.
// Arguments:   spyAPIPipeName - The name of the spies API pipe.
//              incomingEventsPipeName - The name of the spies incoming events pipe.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        25/8/2005
// ---------------------------------------------------------------------------
bool gaAPIToSpyConnector::initialize(const gtString& spyAPIPipeName, const gtString& incomingEventsPipeName)
{
    bool retVal = false;

    // Clean up previous connection left overs (if exists):
    terminate();

    // Create the spies API socket server:
    _pSpiesAPIConnectingSocket = new osPipeSocketServer(spyAPIPipeName);

    // Open the API socket server side (our side):
    bool rcAPISocket = _pSpiesAPIConnectingSocket->open();
    GT_IF_WITH_ASSERT(rcAPISocket)
    {
        // Create a thread that will accept and handle the API connection requests:
        _pSpiesAPIConnectionWaiterThread = new gaPipeSocketSpyConnectionWaiterThread(*((osPipeSocketServer*)_pSpiesAPIConnectingSocket), AP_SPIES_UTILITIES_API_CONNECTION);

        // Cleanup (see comment at gaAPIToSpyConnector::terminate)
        delete _pIncomingSpyEventsListenerThread;
        _pIncomingSpyEventsListenerThread = NULL;

        // Create a thread that will handle the incoming spy events:
        _pIncomingSpyEventsListenerThread = new gaIncomingSpyEventsPipeSocketListenerThread(incomingEventsPipeName);

        // Execute the threads:
        bool rcAPIThread = _pSpiesAPIConnectionWaiterThread->execute();
        bool rcSpyEventsThread = _pIncomingSpyEventsListenerThread->execute();
        GT_IF_WITH_ASSERT(rcAPIThread && rcSpyEventsThread)
        {
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaAPIToSpyConnector::terminate
// Description: Terminates the API <-> Spy connection.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        2/3/2004
// ---------------------------------------------------------------------------
bool gaAPIToSpyConnector::terminate()
{
    bool rc1 = true;
    bool rc2 = true;
    bool rc3 = true;

    // Terminate and delete the API connection waiter thread:
    if (_pSpiesAPIConnectionWaiterThread != NULL)
    {
        if (_pSpiesAPIConnectionWaiterThread->isAlive())
        {
            rc1 = _pSpiesAPIConnectionWaiterThread->terminate();
            GT_ASSERT(rc1);
        }

        delete _pSpiesAPIConnectionWaiterThread;
        _pSpiesAPIConnectionWaiterThread = NULL;
    }

    // Terminate and delete the API connection waiter thread:
    if (_pIncomingSpyEventsListenerThread != NULL)
    {
        if (_pIncomingSpyEventsListenerThread->isAlive())
        {
            rc1 = _pIncomingSpyEventsListenerThread->terminate();
            GT_ASSERT(rc1);
        }

        // Yaki 5/1/2010:
        // _pIncomingSpyEventsListenerThread will be deleted by the next call to gaAPIToSpyConnector::initialize.
        // This prevent a state in which the thread run was not physically canceled yet, enabling the thread to access
        // the incoming events socket (which is deleted by gaIncomingSpyEventsListenerThread destructor).
    }

    // Close and delete the port listener:
    if (_pPortListener != NULL)
    {
        if (_pPortListener->isOpen())
        {
            rc2 = _pPortListener->close();
            GT_ASSERT(rc2);
        }

        delete _pPortListener;
        _pPortListener = NULL;
    }

    // Close and delete the spy connecting socket:
    if (_pSpiesAPIConnectingSocket != NULL)
    {
        if (_pSpiesAPIConnectingSocket->isOpen())
        {
            rc3 = _pSpiesAPIConnectingSocket->close();
            GT_ASSERT(rc3);
        }

        // Delete the spy connector:
        delete _pSpiesAPIConnectingSocket;
        _pSpiesAPIConnectingSocket = NULL;
    }

    // Clear the second chance exception flag:
    _isDuringSecondChanceExceptionHandling = false;

    bool retVal = rc1 && rc2 && rc3;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaAPIToSpyConnector::isAPIConnectionActive
// Description: Inputs an API connection type and returns true iff it is active.
// Author:      Yaki Tebeka
// Date:        24/12/2009
// ---------------------------------------------------------------------------
bool gaAPIToSpyConnector::isAPIConnectionActive(apAPIConnectionType apiType) const
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((0 <= apiType) && (apiType < AP_AMOUNT_OF_API_CONNECTION_TYPES))
    {
        // If there is a socket server thread:
        if (_pSpiesAPIConnectionWaiterThread != NULL)
        {
            // If we are not during a debugged process second change exception handling:
            if (!_isDuringSecondChanceExceptionHandling)
            {
                // Verify that the Spy API thread is running:
                pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();
                bool isAPIThreadRunning = theProcessDebugger.isSpiesAPIThreadRunning();

                if (isAPIThreadRunning)
                {
                    // Check if the queried API is active:
                    retVal = _isAPIConnectionActive[apiType];
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaAPIToSpyConnector::onPendingDebugEvent
// Description:
//  Is called, a-synchronically by the debugger thread, when a debugged process
//  event is added to the pending events queue.
//
// Arguments: pendingEventType - The pending event type.
//
// Author:      Yaki Tebeka
// Date:        22/11/2007
// ---------------------------------------------------------------------------
void gaAPIToSpyConnector::onPendingDebugEvent(const apEvent& pendingEvent)
{
    // Will get true iff the debugged process died or is going to die:
    bool isDebuggedProcessDying = false;

    // Get the event's type:
    apEvent::EventType pendingEventType = pendingEvent.eventType();

    // If the pending event is "process terminated event", or a "process creation failure" event:
    if ((pendingEventType == apEvent::AP_DEBUGGED_PROCESS_TERMINATED) || (pendingEventType == apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE))
    {
        isDebuggedProcessDying = true;
    }
    else if (pendingEventType == apEvent::AP_EXCEPTION)
    {
        // The debugged process got an exception:

        // If the debugged process is about to die:
        const apExceptionEvent& exceptionEvent = (const apExceptionEvent&)pendingEvent;
        bool isSecondChangeException = exceptionEvent.isSecondChance();
        bool isFatalSignal = exceptionEvent.isFatalLinuxSignal();

        if (isSecondChangeException || isFatalSignal)
        {
            isDebuggedProcessDying = true;
        }
    }
    else if (pendingEventType == apEvent::AP_API_CONNECTION_ESTABLISHED)
    {
        onAPIConnectionEstablishedEvent((const apApiConnectionEstablishedEvent&)pendingEvent);
    }
    else if (pendingEventType == apEvent::AP_API_CONNECTION_ENDED)
    {
        onAPIConnectionEndedEvent((const apApiConnectionEndedEvent&)pendingEvent);
    }

    // If the debugged process died or is going to die:
    if (isDebuggedProcessDying)
    {
        // Preventing the main API thread from waiting forever for the debugged process
        // that just died (See case 3050).
        // This is done by:
        // - Clearing the "is API connection active array"
        // - Decreasing the API socket read timeout. This terminates active read/write operations.
        // - Closing the pipe. This prevents future read/write operations.
        if (_pSpiesAPIConnectingSocket != NULL)
        {
            clearIsAPIConnectionActiveArray();

            _pSpiesAPIConnectingSocket->setReadOperationTimeOut(1000);
            _pSpiesAPIConnectingSocket->setWriteOperationTimeOut(1000);

            if (_pSpiesAPIConnectingSocket->isOpen())
            {
                _pSpiesAPIConnectingSocket->close();
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gaAPIToSpyConnector::beforeForcedDebuggedProcessTermination
// Description: Is called before the debugged process is terminated by the
//              "Terminate debugged process" command.
// Author:      Yaki Tebeka
// Date:        10/1/2010
// ---------------------------------------------------------------------------
void gaAPIToSpyConnector::beforeForcedDebuggedProcessTermination()
{
    clearIsAPIConnectionActiveArray();
}


// ---------------------------------------------------------------------------
// Name:        gaAPIToSpyConnector::onEvent
// Description: Is called when a debugged process event occurs.
// Arguments:   eve - A class that represents the event.
// Author:      Yaki Tebeka
// Date:        7/6/2004
// ---------------------------------------------------------------------------
void gaAPIToSpyConnector::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent); // unused
    // Handle the event according to its type:
    apEvent::EventType eventType = eve.eventType();

    switch (eventType)
    {
        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        case apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE:
            onProcessTerminatedEvent();
            break;

        case apEvent::AP_EXCEPTION:
            onExceptionEvent((const apExceptionEvent&)eve);
            break;

        default:
            // An event that we currently don't handle.
            break;
    }
}




// ---------------------------------------------------------------------------
// Name:        gaAPIToSpyConnector::onProcessTerminatedEvent
// Description: Is called when the debugged process is terminated.
// Author:      Yaki Tebeka
// Date:        24/12/2009
// ---------------------------------------------------------------------------
void gaAPIToSpyConnector::onProcessTerminatedEvent()
{
    // Terminated the API connection:
    bool rc = terminate();
    GT_ASSERT(rc);

    // Clear the second change handling flag:
    _isDuringSecondChanceExceptionHandling = false;

    // Clear the is API connection active array:
    clearIsAPIConnectionActiveArray();
}


// ---------------------------------------------------------------------------
// Name:        gaAPIToSpyConnector::onExceptionEvent
// Description: Is called when an exception event is triggered.
// Arguments: eve - Holds the event data.
// Author:      Yaki Tebeka
// Date:        24/12/2009
// ---------------------------------------------------------------------------
void gaAPIToSpyConnector::onExceptionEvent(const apExceptionEvent& eve)
{
    // If this is a second change exception:
    bool isSecondChanceExeption = eve.isSecondChance();

    if (isSecondChanceExeption)
    {
        // Set the second change handling flag:
        _isDuringSecondChanceExceptionHandling = true;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaAPIToSpyConnector::onAPIConnectionEstablishedEvent
// Description: Is called when an API connection established event is triggered.
// Arguments: eve - Holds the event data.
// Author:      Yaki Tebeka
// Date:        24/12/2009
// ---------------------------------------------------------------------------
void gaAPIToSpyConnector::onAPIConnectionEstablishedEvent(const apApiConnectionEstablishedEvent& eve)
{
    // Get the established connection type:
    apAPIConnectionType apiType = eve.establishedConnectionType();
    GT_IF_WITH_ASSERT((0 <= apiType) && (apiType < AP_AMOUNT_OF_API_CONNECTION_TYPES))
    {
        // Mark the API is active:
        _isAPIConnectionActive[apiType] = true;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaAPIToSpyConnector::onAPIConnectionEndedEvent
// Description: Is called when an API connection ended event is triggered.
// Arguments: eve - Holds the event data.
// Author:      Uri Shomroni
// Date:        13/1/2010
// ---------------------------------------------------------------------------
void gaAPIToSpyConnector::onAPIConnectionEndedEvent(const apApiConnectionEndedEvent& eve)
{
    // Get the established connection type:
    apAPIConnectionType apiType = eve.connectionType();
    GT_IF_WITH_ASSERT((0 <= apiType) && (apiType < AP_AMOUNT_OF_API_CONNECTION_TYPES))
    {
        // Mark the API is inactive:
        _isAPIConnectionActive[apiType] = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaAPIToSpyConnector::clearIsAPIConnectionActiveArray
// Description: Clears the _isAPIConnectionActive array.
// Author:      Yaki Tebeka
// Date:        24/12/2009
// ---------------------------------------------------------------------------
void gaAPIToSpyConnector::clearIsAPIConnectionActiveArray()
{
    for (unsigned int i = 0; i < AP_AMOUNT_OF_API_CONNECTION_TYPES; i++)
    {
        _isAPIConnectionActive[i] = false;
    }
}

