//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suAPICallsHandlingThread.cpp
///
//==================================================================================

//------------------------------ suAPICallsHandlingThread.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTAPIClasses/Include/Events/apThreadCreatedEvent.h>

// Local:
#include <src/suAPICallsHandlingThread.h>
#include <AMDTServerUtilities/Include/suAPIMainLoop.h>
#include <src/suSpyToAPIConnector.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>


// ---------------------------------------------------------------------------
// Name:        suAPICallsHandlingThread::suAPICallsHandlingThread
// Description: Constructor.
//
// Arguments: socketClient - The socket clients that serves as a communication
//                           channel between this Spy DLL and its API.
//
// Author:      Yaki Tebeka
// Date:        15/2/2004
// ---------------------------------------------------------------------------
suAPICallsHandlingThread::suAPICallsHandlingThread(osSocket& socketClient)
    : osThread(L"suAPICallsHandlingThread"), _socketClient(socketClient), _isListeningToAPICalls(false),
      _shouldWaitForSpiesUtilitiesAPIInitialization(true)
{
}


// ---------------------------------------------------------------------------
// Name:        suAPICallsHandlingThread::~suAPICallsHandlingThread
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        15/2/2004
// ---------------------------------------------------------------------------
suAPICallsHandlingThread::~suAPICallsHandlingThread()
{
    // Terminate the OS thread:
    if (isAlive())
    {
        terminate();
    }
}


// ---------------------------------------------------------------------------
// Name:        suAPICallsHandlingThread::isListeningToAPICalls
// Description: Returns true iff the API thread is listening to API calls
// Author:      Yaki Tebeka
// Date:        28/12/2009
// ---------------------------------------------------------------------------
bool suAPICallsHandlingThread::isListeningToAPICalls() const
{
    bool retVal = _isListeningToAPICalls;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suAPICallsHandlingThread::afterSpiesUtilitiesAPIInitializationEnded
// Description: Is called after the spies utilities API initialization is done.
// Author:      Yaki Tebeka
// Date:        28/12/2009
// ---------------------------------------------------------------------------
void suAPICallsHandlingThread::afterSpiesUtilitiesAPIInitializationEnded()
{
    _shouldWaitForSpiesUtilitiesAPIInitialization = false;
}


// ---------------------------------------------------------------------------
// Name:        suAPICallsHandlingThread::mainThreadStarsInitializingAPIConnection
// Description: Is called by the main thread when it starts initializing an API connection.
// Return Val: bool  - true - The main thread may handle the API connection initialization.
//                     false - The main thread should not handle the API connection initialization,
//                             since this handling is done by the API thread.
// Author:      Yaki Tebeka
// Date:        28/12/2009
// ---------------------------------------------------------------------------
bool suAPICallsHandlingThread::mainThreadStarsInitializingAPIConnection()
{
    bool retVal = false;

    // Entering the critical section guarding the API connection initialization:
    _APIConnectionInitializationCS.enter();
    OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_mainThreadEnteredAPIInitializationCS, OS_DEBUG_LOG_DEBUG);

    // If the API thread started listening to API calls:
    if (_isListeningToAPICalls)
    {
        // The API thread already started running, therefore, it will handle the API connection initialization calls.
        // Return false, meaning that the main thread should not handle the API connection initialization calls:
        retVal = false;

        // Leave the critical section guarding the API connection initialization:
        _APIConnectionInitializationCS.leave();
        OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_mainThreadLeftAPIInitializationCS, OS_DEBUG_LOG_DEBUG);
    }
    else
    {
        // The API thread didn't start listening to API calls yet.
        // Return true - yes, the main thread should handle the API connection initialization calls:
        retVal = true;

        // We will leave the critical section locked to make sure that the API thread does
        // not start listening to API calls while the main thread handles the API connection initialization calls.
        // (The critical section will be unlocked by suAPICallsHandlingThread::mainThreadEndedInitializingAPIConnection)
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suAPICallsHandlingThread::mainThreadEndedInitializingAPIConnection
// Description: Is called when the main thread ends initializing an API connection.
// Author:      Yaki Tebeka
// Date:        28/12/2009
// ---------------------------------------------------------------------------
void suAPICallsHandlingThread::mainThreadEndedInitializingAPIConnection()
{
    // Leave the critical section guarding the API connection initialization,
    // enabling the API thread to start running:
    _APIConnectionInitializationCS.leave();
    OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_mainThreadLeftAPIInitializationCS, OS_DEBUG_LOG_DEBUG);
}


// ---------------------------------------------------------------------------
// Name:        suAPICallsHandlingThread::entryPoint
// Description:
//   The incoming API calls handling thread entry point.
//   Executes the API main loop.
//
// Return Val:  int -  0 - Success
//                    -1 - Failure
// Author:      Yaki Tebeka
// Date:        15/2/2004
// ---------------------------------------------------------------------------
int suAPICallsHandlingThread::entryPoint()
{
    // The spies utilities module initialization is done by the main application thread.
    // The API calls handling thread should start listening to API calls only after this initialization is done.
    if (_shouldWaitForSpiesUtilitiesAPIInitialization)
    {
        OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_apiThreadIsWaitingForServersAPIInitialization, OS_DEBUG_LOG_DEBUG);
        osWaitForFlagToTurnOff(_shouldWaitForSpiesUtilitiesAPIInitialization, LONG_MAX);
        OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_apiThreadFinishedWaitingForServersAPIInitialization, OS_DEBUG_LOG_DEBUG);
    }

    // Enter the critical section guarding that the API thread does not start running when the main thread
    // initializes an API connection:
    _APIConnectionInitializationCS.enter();
    OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_apiThreadEnteredAPIInitializationCS, OS_DEBUG_LOG_DEBUG);

    // Mark that the API calls handling thread started listening to API calls:
    _isListeningToAPICalls = true;

    // Leave the critical section:
    _APIConnectionInitializationCS.leave();
    OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_apiThreadLeftAPIInitializationCS, OS_DEBUG_LOG_DEBUG);

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    {
        // On Mac, gdb does not report thread creation. As a result, we need to create an event here that will
        // go to the GUI and tell all items there that the API thread was created.
        osSocket* pEventsSocket = suSpyToAPIConnector::instance().eventForwardingSocket();
        GT_IF_WITH_ASSERT(pEventsSocket != NULL)
        {
            osTime now;
            now.setFromCurrentTime();
            apThreadCreatedEvent apiThreadCreatedEvent(osGetCurrentThreadId(), OS_NO_THREAD_ID, now, (osInstructionPointer)NULL);
            *pEventsSocket << apiThreadCreatedEvent;
        }
    }
#endif // (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)

    // Execute the main API loop, starting to listen to API calls:
    OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_apiThreadStartedListening, OS_DEBUG_LOG_DEBUG);
    suAPIMainLoop(_socketClient);

    // Mark that the API thread ended listening to API calls:
    _isListeningToAPICalls = false;
    OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_apiThreadEndedListening, OS_DEBUG_LOG_DEBUG);

    return 0;
}
