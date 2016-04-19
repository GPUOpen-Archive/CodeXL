//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suSpyToAPIConnector.cpp
///
//==================================================================================

//------------------------------ suSpyToAPIConnector.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osPipeSocketClient.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osTCPSocketClient.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunStartedEvent.h>
#include <AMDTAPIClasses/Include/Events/apApiConnectionEstablishedEvent.h>

// Local:
#include <src/suAPICallsHandlingThread.h>
#include <src/suSpyToAPIConnector.h>
#include <AMDTServerUtilities/Include/suAPIMainLoop.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>

// iPhone on-device only:
#ifdef _GR_IPHONE_DEVICE_BUILD
    #include <AMDTServerUtilities/Include/suGlobalVariables.h>
    #include <AMDTServerUtilities/Include/suSpyBreakpointImplementation.h>
#endif

// Static members initializations:
suSpyToAPIConnector* suSpyToAPIConnector::_pMySingleInstance = NULL;


// ---------------------------------------------------------------------------
// Name:        suSpyToAPIConnector::instance
// Description: Returns the single instance of this class. Creates it on
//              the first call to this function.
// Author:      Yaki Tebeka
// Date:        15/2/2004
// ---------------------------------------------------------------------------
suSpyToAPIConnector& suSpyToAPIConnector::instance()
{
    // If my single instance was not created yet - create it:
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new suSpyToAPIConnector;
        GT_ASSERT(_pMySingleInstance);
    }

    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        suSpyToAPIConnector::suSpyToAPIConnector
// Description: Constructor.
// Author:      Yaki Tebeka
// Date:        15/2/2004
// ---------------------------------------------------------------------------
suSpyToAPIConnector::suSpyToAPIConnector()
    : _pSocketClient(NULL), _pEventForwardingSocket(NULL), _pAPICallsHandlingThread(NULL)
{
}


// ---------------------------------------------------------------------------
// Name:        suSpyToAPIConnector::~suSpyToAPIConnector
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        15/2/2004
// ---------------------------------------------------------------------------
suSpyToAPIConnector::~suSpyToAPIConnector()
{
    terminate();
}


// ---------------------------------------------------------------------------
// Name:        suSpyToAPIConnector::initialize
// Description: Initialize a TCP / IP  Spy <-> API connection.
// Arguments:   portAddress - The port address to which the Spy will connect to.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/2/2004
// ---------------------------------------------------------------------------
bool suSpyToAPIConnector::initialize(const osPortAddress& portAddress)
{
    bool retVal = false;

    // Create a TCP / IP socket client:
    osTCPSocketClient* pAPITCPSocketClient = new osTCPSocketClient;
    GT_IF_WITH_ASSERT(pAPITCPSocketClient != NULL)
    {
        // Set the socket client to force immediate resolution of DNSes:
        pAPITCPSocketClient->setBlockOnDNS(true);

        // Open the client side of the socket:
        bool rcSocketOpen = pAPITCPSocketClient->open();
        GT_IF_WITH_ASSERT(rcSocketOpen)
        {
            pAPITCPSocketClient->setReadOperationTimeOut(OS_CHANNEL_INFINIT_TIME_OUT);
            pAPITCPSocketClient->setWriteOperationTimeOut(OS_CHANNEL_INFINIT_TIME_OUT);

            // Display a message in the log:
            gtString logMsg;
            portAddress.toString(logMsg);
            logMsg.prepend(L"Trying to connect to port address: ");
            OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);

            // Connect it to the socket server:
            bool rcConnect = pAPITCPSocketClient->connect(portAddress);
            GT_IF_WITH_ASSERT(rcConnect)
            {
                _pSocketClient = pAPITCPSocketClient;

                // Create and run the API calls handling thread:
                bool rcAPIThread = createAndRunTheAPIHandlingThread();
                GT_IF_WITH_ASSERT(rcAPIThread)
                {
                    // Wait for the debugger to perform the API initialization calls:
                    handleAPIInitializationCalls();

                    retVal = true;
                }
            }
        }
    }

    // Cleanup on failure:
    if (!retVal)
    {
        _pSocketClient = NULL;
        delete pAPITCPSocketClient;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suSpyToAPIConnector::initialize
// Description: Initialize a shared memory Spy <-> API connection.
// Arguments:   sharedMemoryObjectName - The shared memory object name, that is
//                                       used for the connection.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        24/8/2005
// ---------------------------------------------------------------------------
bool suSpyToAPIConnector::initialize(const gtString& sharedMemoryObjectName)
{
    bool retVal = false;

    // Create the socket client:
    _pSocketClient = new osPipeSocketClient(sharedMemoryObjectName, L"SpyToAPIConnector Socket");


    // Connect the socket:
    bool rcSocketOpen = _pSocketClient->open();
    GT_IF_WITH_ASSERT(rcSocketOpen)
    {
        // Create and run the API calls handling thread:
        bool rcAPIThread = createAndRunTheAPIHandlingThread();
        GT_IF_WITH_ASSERT(rcAPIThread)
        {
            // Wait for the debugger to initialize the API:
            handleAPIInitializationCalls();

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suSpyToAPIConnector::terminate
// Description: Terminates the API <-> Spy connection.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        2/3/2004
// ---------------------------------------------------------------------------
bool suSpyToAPIConnector::terminate()
{
    bool retVal = false;

    bool rc2 = true;

    if (_pAPICallsHandlingThread != NULL)
    {
        // If this was called from inside the API thread, don't terminate it,
        // as this would cause the rest of the code not to execute...
        // Let this only be terminated in the main thread (i.e. in our destructor)
        osThreadId currentThreadId = osGetCurrentThreadId();

        if (currentThreadId != _pAPICallsHandlingThread->id())
        {
            // Terminate and delete the API requests handling thread:
            rc2 = _pAPICallsHandlingThread->terminate();
            delete _pAPICallsHandlingThread;
            _pAPICallsHandlingThread = NULL;
        }
    }

    if (_pSocketClient != NULL)
    {
        // Close and delete the socket client:
        _pSocketClient->close();
        delete _pSocketClient;
        _pSocketClient = NULL;
    }

    if (_pEventForwardingSocket != NULL)
    {
        // Close and delete the event forwarding socket:
        _pEventForwardingSocket->close();
        delete _pEventForwardingSocket;
        _pEventForwardingSocket = NULL;
    }

    // TO_DO: We need to check why does the sockets close() function fails (rc1 + rc3) !!
    retVal = rc2;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suSpyToAPIConnector::setEventForwardingSocket
// Description: Sets an event forwarding socket from a TCP / IP address.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        29/10/2009
// ---------------------------------------------------------------------------
bool suSpyToAPIConnector::setEventForwardingSocket(const osPortAddress& portAddress)
{
    bool retVal = false;

    osTCPSocketClient* pTCPSocket = new osTCPSocketClient;
    GT_IF_WITH_ASSERT(pTCPSocket != NULL)
    {
        // Set the socket client to force immediate resolution of DNSes:
        pTCPSocket->setBlockOnDNS(true);

        bool rcOpen = pTCPSocket->open();
        GT_IF_WITH_ASSERT(rcOpen)
        {
            bool rcConn = pTCPSocket->connect(portAddress);
            GT_IF_WITH_ASSERT(rcConn)
            {
                // Store the pipe:
                _pEventForwardingSocket = pTCPSocket;
                retVal = true;

#ifdef _GR_IPHONE_DEVICE_BUILD
                {
                    // When debugging the iPhone on-device, we don't have a real debugger that triggers the "Process run started" event.
                    // instead, we trigger it here:
                    osTime now;
                    now.setFromCurrentTime();
                    osProcessId currentProcessId = osGetCurrentProcessId();
                    apDebuggedProcessRunStartedEvent runStartedEvent(currentProcessId, now);
                    *_pEventForwardingSocket << runStartedEvent;
                }
#endif

                // Notify the debugger that the incoming events API connection was established:
                apApiConnectionEstablishedEvent apiConnectionEstablishedEvent(AP_INCOMING_EVENTS_API_CONNECTION);
                *_pEventForwardingSocket << apiConnectionEstablishedEvent;
            }
        }
    }

    if (!retVal)
    {
        // Clear the socket if we created it:
        delete pTCPSocket;
        pTCPSocket = NULL;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suSpyToAPIConnector::setEventForwardingPipe
// Description: Sets the name of the spies events forwarding pipe.
// Arguments:   eventsPipeName - The spies events forwarding pipe.
// Return Val:  bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        16/12/2009
// ---------------------------------------------------------------------------
bool suSpyToAPIConnector::setEventForwardingPipe(const gtString& eventsPipeName)
{
    bool retVal = false;

    // Create the events pipe socket client:
    osPipeSocketClient* pPipeSocketClient = new osPipeSocketClient(eventsPipeName, L"Events Socket");


    // Open the pipe:
    bool rcOpen = pPipeSocketClient->open();
    GT_IF_WITH_ASSERT(rcOpen)
    {
        // Store the pipe:
        _pEventForwardingSocket = pPipeSocketClient;
        retVal = true;
    }

    if (!retVal)
    {
        // Clear the socket if we created it:
        delete pPipeSocketClient;
        pPipeSocketClient = NULL;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suSpyToAPIConnector::handleAPIInitializationCalls
// Description: Wait for the debugger API initialization calls and handle them.
// Author:      Yaki Tebeka
// Date:        4/1/2009
// ---------------------------------------------------------------------------
void suSpyToAPIConnector::handleAPIInitializationCalls()
{
    // Handle the API functions initialization calls:
    OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_startedHandlingAPIInitCalls, OS_DEBUG_LOG_DEBUG);
    suAPIMainLoop(*_pSocketClient);
    OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_endeddHandlingAPIInitCalls, OS_DEBUG_LOG_DEBUG);

    // Notify the API thread that the Spies Utilities API initialization was done:
    GT_IF_WITH_ASSERT(_pAPICallsHandlingThread != NULL)
    {
        _pAPICallsHandlingThread->afterSpiesUtilitiesAPIInitializationEnded();
    }
}


// ---------------------------------------------------------------------------
// Name:        suSpyToAPIConnector::createAndRunTheAPIHandlingThread
// Description: Creates and runs the API calls handling thread.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        24/8/2005
// ---------------------------------------------------------------------------
bool suSpyToAPIConnector::createAndRunTheAPIHandlingThread()
{
    bool retVal = false;

    // If the socket client exists:
    if (_pSocketClient)
    {
        // Create the thread that will handle incoming API calls:
        _pAPICallsHandlingThread = new suAPICallsHandlingThread(*_pSocketClient);

        if (_pAPICallsHandlingThread)
        {
            // Start the thread run:
            bool rcThreadExe = _pAPICallsHandlingThread->execute();
            GT_IF_WITH_ASSERT(rcThreadExe)
            {
                // Log the API thread id:
                osThreadId apiThreadId = _pAPICallsHandlingThread->id();
                suSetSpiesAPIThreadId(apiThreadId);

                retVal = true;
            }
        }
    }

    return retVal;
}



