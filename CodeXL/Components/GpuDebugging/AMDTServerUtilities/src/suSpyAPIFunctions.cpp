//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suSpyAPIFunctions.cpp
///
//==================================================================================

//------------------------------ suSpyAPIFunctions.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAPIClasses/Include/Events/apApiConnectionEstablishedEvent.h>
#include <AMDTAPIClasses/Include/Events/apApiConnectionEndedEvent.h>
#include <AMDTAPIClasses/Include/Events/apSpyProgressEvent.h>

// Local:
#include <src/suSpyToAPIConnector.h>
#include <src/suAPICallsHandlingThread.h>
#include <AMDTServerUtilities/Include/suAPIMainLoop.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>

// Forward decelerations:
void suOutputHandlingAPIFuncDebugLogPrintout(apAPIFunctionId functionId);
void suOutputEndedHandlingAPIFuncDebugLogPrintout(apAPIFunctionId functionId);

// Holds the API connections initialization status:
// (the i item contains true iff the client ended initializing the i API connection type)
static bool su_stat_isAPIConnectionInitialized[AP_AMOUNT_OF_API_CONNECTION_TYPES];

// Maps API function ID to the stub function that handles it:
static suAPIStubFunction su_stat_StubFunctionsMap[GA_AMOUNT_OF_API_FUNCTION_IDS];

// Contains true iff we are during direct function execution.
// (See gaBeforeDirectAPIFunctionExecution documentation for mode details)
static bool su_stat_isDuringDirectFunctionExecution = false;

// Contains true iff the API thread died and one of the technology spies
// destructor was called.
// This only happens if the spy was statically linked or preloaded, i.e. when
// the debugged process terminates, so it is irreverisble:
static bool su_stat_isInProcessTerminationAPILoop = false;

// Contains true iff gaTerminateDebuggedProcess was called, i.e. the API initiated
// the debugged process termination:
static bool su_stat_isTerminationInitiatedByAPI = false;

// Contains true iff spy events should be suppressed, i.e. when the spy should not send
// progress events etcetera, but only the most essential events:
static bool su_stat_supressSpyEvents = false;

// TO_DO: Replace osWaitForFlagToTurnOff on windows with osCondition.
//        I think that osSynchronizationObject is quite the same, check if we can
//        merge the two classes into osCondition.
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <AMDTOSWrappers/Include/osCondition.h>
    static osCondition stat_isDuringFunExecutionCondition;
#endif


// ---------------------------------------------------------------------------
// Name:        suRegisterAPIFunctionStub
// Description: Registers an API function stub.
// Arguments: functionId - The called API function id.
//            apiStubFunction - The API function stub that handles the API function call
//                              associated with the input function id.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        30/11/2009
// ---------------------------------------------------------------------------
bool suRegisterAPIFunctionStub(apAPIFunctionId functionId, suAPIStubFunction apiStubFunction)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((0 <= functionId) && (functionId < GA_AMOUNT_OF_API_FUNCTION_IDS))
    {
        // First, verify that an handler for this function does not already exist:
        if (su_stat_StubFunctionsMap[functionId] != NULL)
        {
            // Display an assertion failure:
            gtString functionIdAsString;
            apAPIFunctionIdToString(functionId, functionIdAsString);
            gtString errorMsg = SU_STR_DebugLog_registeringTwoHandlersForAnAPIFunction;
            errorMsg += functionIdAsString;
            GT_ASSERT_EX(false, errorMsg.asCharArray());
        }
        else
        {
            // Associate the input API function id with the input stub function:
            su_stat_StubFunctionsMap[functionId] = apiStubFunction;

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suRegisterAPIConnectionAsActive
// Description: Registers an API connection as active.
// Arguments: apiType - The type of the API connection to be registered as active.
// Author:      Yaki Tebeka
// Date:        1/12/2009
// ---------------------------------------------------------------------------
void suRegisterAPIConnectionAsActive(apAPIConnectionType apiType)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((0 <= apiType) && (apiType < AP_AMOUNT_OF_API_CONNECTION_TYPES))
    {
        apApiConnectionEstablishedEvent apiConnectionEstablishedEvent(apiType);
        bool rcEve = suForwardEventToClient(apiConnectionEstablishedEvent);
        GT_ASSERT(rcEve);
    }
}


// ---------------------------------------------------------------------------
// Name:        suRegisterAPIConnectionAsInactive
// Description: Registers an API connection as inactive.
// Arguments: apiType - The type of the API connection to be registered as inactive.
// Author:      Uri Shomroni
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void suRegisterAPIConnectionAsInactive(apAPIConnectionType apiType)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((0 <= apiType) && (apiType < AP_AMOUNT_OF_API_CONNECTION_TYPES))
    {
        apApiConnectionEndedEvent apiConnectionEndedEvent(apiType);
        bool rcEve = suForwardEventToClient(apiConnectionEndedEvent);
        GT_ASSERT(rcEve);
    }
}


// ---------------------------------------------------------------------------
// Name:        suMarkAPIConnectionAsInitialized
// Description: Is called after the client ended initializing an API connection.
// Arguments: apiType - The type of the API connection that was initialized.
// Author:      Yaki Tebeka
// Date:        28/12/2009
// ---------------------------------------------------------------------------
void suMarkAPIConnectionAsInitialized(apAPIConnectionType apiType)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((0 <= apiType) && (apiType < AP_AMOUNT_OF_API_CONNECTION_TYPES))
    {
        su_stat_isAPIConnectionInitialized[apiType] = true;
    }
}


// ---------------------------------------------------------------------------
// Name:        suMainThreadStarsInitializingTechnology
// Description: Is called by the main application thread when it starts initializing
//              an API connection.
// Arguments: apiType - The type of the initialized API connection.
// Return Val: bool  - true - The main thread may handle the API connection initialization.
//                     false - The main thread should not handle the API connection initialization,
//                             since this handling is done by the API thread.
// Author:      Yaki Tebeka
// Date:        28/12/2009
// ---------------------------------------------------------------------------
bool suMainThreadStarsInitializingAPIConnection(apAPIConnectionType apiType)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((0 <= apiType) && (apiType < AP_AMOUNT_OF_API_CONNECTION_TYPES))
    {
        // Notify the API calls handling thread:
        suSpyToAPIConnector& spyConnector = suSpyToAPIConnector::instance();
        suAPICallsHandlingThread* pAPICallsHandlingThread = spyConnector.apiCallsHandlingThread();
        GT_IF_WITH_ASSERT(pAPICallsHandlingThread != NULL)
        {
            retVal = pAPICallsHandlingThread->mainThreadStarsInitializingAPIConnection();
        }
    }

    // If the API initialization calls are handled by the main thread:
    if (retVal)
    {
        OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_APIInitCallsAreHandledByMainThread, OS_DEBUG_LOG_DEBUG);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suMainThreadEndedInitializingAPIConnection
// Description: Is called by the main application thread when after it ends initializing
//              an API connection.
// Arguments: apiType - The type of the initialized API connection.
// Author:      Yaki Tebeka
// Date:        28/12/2009
// ---------------------------------------------------------------------------
void suMainThreadEndedInitializingAPIConnection(apAPIConnectionType apiType)
{
    OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_APIInitCallsMainThreadHandlingEnded, OS_DEBUG_LOG_DEBUG);

    // Sanity check:
    GT_IF_WITH_ASSERT((0 <= apiType) && (apiType < AP_AMOUNT_OF_API_CONNECTION_TYPES))
    {
        // Notify the API calls handling thread:
        suSpyToAPIConnector& spyConnector = suSpyToAPIConnector::instance();
        suAPICallsHandlingThread* pAPICallsHandlingThread = spyConnector.apiCallsHandlingThread();
        GT_IF_WITH_ASSERT(pAPICallsHandlingThread != NULL)
        {
            pAPICallsHandlingThread->mainThreadEndedInitializingAPIConnection();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        suIsAPIConnectionInitialized
// Description:
//  Inputs an API connection type and returns true iff the client ended initializing it.
// Author:      Yaki Tebeka
// Date:        28/12/2009
// ---------------------------------------------------------------------------
bool suIsAPIConnectionInitialized(apAPIConnectionType apiType)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((0 <= apiType) && (apiType < AP_AMOUNT_OF_API_CONNECTION_TYPES))
    {
        retVal = su_stat_isAPIConnectionInitialized[apiType];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suSpiesAPISocket
// Description: Returns the Spies API socket.
// Return Val: osSocket* - Will get a pointer to the Spies API socket, or NULL if API
//                         connection with the debugger was not established yet.
// Author:      Yaki Tebeka
// Date:        20/11/2005
// ---------------------------------------------------------------------------
osSocket* suSpiesAPISocket()
{
    osSocket* retVal = NULL;

    // Get the API socket:
    suSpyToAPIConnector& spyConnector = suSpyToAPIConnector::instance();
    osSocket* pAPISocket = spyConnector.apiSocket();

    // Verify that we have an API socket:
    GT_IF_WITH_ASSERT(pAPISocket != NULL)
    {
        // Verify that the API connection is open:
        GT_IF_WITH_ASSERT(pAPISocket->isOpen())
        {
            retVal = pAPISocket;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suEventForwardingSocket
// Description: Returns the Spies API socket.
// Return Val: osSocket* - Will get a pointer to the Events forwarding socket,
//              or NULL if events connection with the debugger was not established.
// Author:      Uri Shomroni
// Date:        1/12/2009
// ---------------------------------------------------------------------------
osSocket* suEventForwardingSocket()
{
    osSocket* retVal = NULL;

    // Get the API socket:
    suSpyToAPIConnector& spyConnector = suSpyToAPIConnector::instance();
    osSocket* pEventSocket = spyConnector.eventForwardingSocket();

    // Verify that we have an API socket:
    GT_IF_WITH_ASSERT(pEventSocket != NULL)
    {
        // Verify that the API connection is open:
        GT_IF_WITH_ASSERT(pEventSocket->isOpen())
        {
            retVal = pEventSocket;
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        suForwardEventToClient
// Description: Forwards an event to the client through the Spies API socket.
// Return Val:  bool - Success / Failure
// Author:      Uri Shomroni
// Date:        3/11/2015
// ---------------------------------------------------------------------------
bool suForwardEventToClient(const apEvent& eve)
{
    bool retVal = false;

    osSocket* pEventsSocket = suEventForwardingSocket();
    GT_IF_WITH_ASSERT(pEventsSocket != NULL)
    {
        *pEventsSocket << (const osTransferableObject&)eve;

        retVal = true;
        apEvent::EventType eveType = eve.eventType();

        // Check if this is one of the event types that require confirmation:
        if (apEvent::DoesEventTypeRequireForwardingConfirmation(eveType))
        {
            // Uri, 26/6/11 - We must synchronize these events - see comment in gaIncomingSpyEventsListenerThread::entryPoint()
            bool rcSync = false;
            *pEventsSocket >> rcSync;
            GT_ASSERT(rcSync);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suIsAPIThreadRunning
// Description: Returns true iff the API thread started running and listens to
//              incoming API calls.
// Author:      Uri Shomroni
// Date:        20/12/2009
// ---------------------------------------------------------------------------
bool suIsAPIThreadRunning()
{
    bool retVal = false;

    // Get the API calls handling thread:
    suSpyToAPIConnector& theSpiesToAPIConnector = suSpyToAPIConnector::instance();
    suAPICallsHandlingThread* pAPICallsHandlingThread = theSpiesToAPIConnector.apiCallsHandlingThread();

    if (pAPICallsHandlingThread != NULL)
    {
        retVal = pAPICallsHandlingThread->isListeningToAPICalls() && pAPICallsHandlingThread->isAlive();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suBeforeEnteringTerminationAPILoop
// Description: Called before the termination API loop is entered (from the main
//              thread - after the API thread was terminated)
// Author:      Uri Shomroni
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void suBeforeEnteringTerminationAPILoop()
{
    // Mark that any API loops from now on are termination loops:
    su_stat_isInProcessTerminationAPILoop = true;

    // On Windows and Mac- send a debug string that notifies that the debugged process is going to
    // be terminated and also notify about the fact that the current thread is becoming the
    // new API calls handling thread:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
    {
        osThreadId currThreadId = osGetCurrentThreadId();
        gtString debugString = OS_STR_debuggedProcessIsTerminating;
        debugString.appendFormattedString(L"%lu", currThreadId);
        osOutputDebugString(debugString.asCharArray());
    }
#endif
}

// ---------------------------------------------------------------------------
// Name:        suRunTerminationAPILoop
// Description: Runs the termination-time API loop if this is needed on this platform.
// Author:      Uri Shomroni
// Date:        20/12/2009
// ---------------------------------------------------------------------------
void suRunTerminationAPILoop()
{
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
    // Uri, 28/1/09: Under Windows and Mac, the above breakpoint does not allow the API thread to answer
    // queries needed for checking for memory leaks.
    // On Linux/ the "Trigger breakpoint exception" function allows the API thread to
    // reply to queries, so calling the API Main loop again is not needed.

    // Get the API connecting socket:
    osSocket* pAPISocket = suSpiesAPISocket();
    GT_IF_WITH_ASSERT(pAPISocket != NULL)
    {
        OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_enteringDebuggedProcessTerminationAPILoop, OS_DEBUG_LOG_DEBUG);

        // Enter a main API loop that will server API calls during the debugged process termination:
        suAPIMainLoop(*pAPISocket);

        OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_exitDebuggedProcessTerminationAPILoop, OS_DEBUG_LOG_DEBUG);
    }
#endif
}

// ---------------------------------------------------------------------------
// Name:        suIsTerminationInitiatedByAPI
// Description: Returns true iff we are being terminated, and only if the termination
//              was initiated by the gaTerminateDebuggedProcess API function.
// Author:      Uri Shomroni
// Date:        21/12/2009
// ---------------------------------------------------------------------------
bool suIsTerminationInitiatedByAPI()
{
    return su_stat_isTerminationInitiatedByAPI;
}

// ---------------------------------------------------------------------------
// Name:        suBeforeDirectFunctionExecution
// Description:
//   Is called before direct functions execution.
//   During direct function execution this class does not listen to the API socket,
//   enabling the directly called API function stub to read the function input
//   arguments from the socket.
//
//   See gaBeforeDirectAPIFunctionExecution documentation for mode information about
//   direct function calls.
//
// Author:      Yaki Tebeka
// Date:        20/11/2005
// ---------------------------------------------------------------------------
void suBeforeDirectFunctionExecution()
{
    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_raisingDirectFuncExecutionFlag, OS_DEBUG_LOG_DEBUG);

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    // Lock the execution condition, so that we do not become stuck:
    bool rc1 = stat_isDuringFunExecutionCondition.lockCondition();
    GT_IF_WITH_ASSERT(rc1)
#endif
    {
        // Mark that the direct function execution process begins:
        su_stat_isDuringDirectFunctionExecution = true;
    }
}


// ---------------------------------------------------------------------------
// Name:        suAfterDirectFunctionExecution
// Description:
//   Is called after direct functions execution.
//   See suBeforeDirectFunctionExecution() documentation for more details.
// Author:      Yaki Tebeka
// Date:        20/11/2005
// ---------------------------------------------------------------------------
void suAfterDirectFunctionExecution()
{
    // Debug log printout:
    OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_loweringDirectFuncExecutionFlag, OS_DEBUG_LOG_DEBUG);

    // Mark that the direct function execution process is over:
    su_stat_isDuringDirectFunctionExecution = false;

    // Clear the function to be executed from the breakpoints manager:
    su_stat_theBreakpointsManager.setFunctionToBeExecutedDuringBreak(NULL);

    // If we are running on Linux:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    {
        /*        OS_OUTPUT_DEBUG_LOG(L"Waiting for condition to become locked", OS_DEBUG_LOG_DEBUG);
                while (!stat_isDuringFunExecutionCondition.isConditionLocked())
                {
                    osSleep(1);
                }
                OS_OUTPUT_DEBUG_LOG(L"Finished waiting for condition to become locked", OS_DEBUG_LOG_DEBUG);*/

        // Unlock the thread execution condition:
        bool rc1 = stat_isDuringFunExecutionCondition.unlockCondition();
        GT_IF_WITH_ASSERT(rc1)
        {
            // Debug log printout:
            OS_OUTPUT_DEBUG_LOG(L"signalling thread execution condition", OS_DEBUG_LOG_DEBUG);

            // Signal the thread execution condition:
            bool rc2 = stat_isDuringFunExecutionCondition.signalSingleThread();
            GT_ASSERT(rc2);
        }
    }
#endif

    // If we are running on Windows:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        // Trigger a breakpoint exception, letting the debugger know that the direct
        // function execution ended (See pdWin32ProcessDebugger::makeThreadExecuteFunction).
        osThrowBreakpointException();
    }
#endif
}


// ---------------------------------------------------------------------------
// Name:        suIsDuringDirectFunctionExecution
// Description: Returns true iff we are during the direct function execution process.
// Author:      Yaki Tebeka
// Date:        30/11/2009
// ---------------------------------------------------------------------------
bool suIsDuringDirectFunctionExecution()
{
    return su_stat_isDuringDirectFunctionExecution;
}


// ---------------------------------------------------------------------------
// Name:        suInitializeSpyAPIFunctionsInfra
// Description: Initializes the Spy API functions infrastructures.
// Author:      Yaki Tebeka
// Date:        1/12/2009
// ---------------------------------------------------------------------------
void suInitializeSpyAPIFunctionsInfra()
{
    // Initialize the API function id to function stub map:
    for (unsigned int i = 0; i < GA_AMOUNT_OF_API_FUNCTION_IDS; i++)
    {
        su_stat_StubFunctionsMap[i] = NULL;
    }

    // Initialize the API connections initialization status:
    for (unsigned int j = 0; j < AP_AMOUNT_OF_API_CONNECTION_TYPES; j++)
    {
        su_stat_isAPIConnectionInitialized[j] = false;
    }
}


// ---------------------------------------------------------------------------
// Name:        suExecuteAPIFunctionStub
// Description: Inputs an APU function id and executes the API function stub
//              that was associated with it by suRegisterAPIFunctionStub.
// Arguments: functionId - The input API function id.
//            apiSocket - A socket from which the function arguments will be
//                        read by the stub function.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        30/11/2009
// ---------------------------------------------------------------------------
bool suExecuteAPIFunctionStub(apAPIFunctionId functionId, osSocket& apiSocket)
{
    bool retVal = false;

    // Output debug log printout:
    suOutputHandlingAPIFuncDebugLogPrintout(functionId);

    // Sanity check:
    GT_IF_WITH_ASSERT((0 <= functionId) && (functionId < GA_AMOUNT_OF_API_FUNCTION_IDS))
    {
        // If there is no stub function that is associated with the input API function id:
        if (su_stat_StubFunctionsMap[functionId] == NULL)
        {
            gtString errorMsg;
            apAPIFunctionIdToString(functionId, errorMsg);
            errorMsg.prepend(SU_STR_DebugLog_cannotFindAPIFunctionStub).append(')');
            GT_ASSERT_EX(false, errorMsg.asCharArray());
        }
        else
        {
            // Call the API function stub:
            suAPIStubFunction& apiFunctionStub = su_stat_StubFunctionsMap[functionId];
            apiFunctionStub(apiSocket);

            retVal = true;
        }
    }

    // Output debug log printout:
    suOutputEndedHandlingAPIFuncDebugLogPrintout(functionId);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suWaitForDirectFunctionExecutionEnd
// Description: If needed, wait until a direct function execution is over.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        30/11/2009
// ---------------------------------------------------------------------------
bool suWaitForDirectFunctionExecutionEnd()
{
    bool retVal = false;

    // If we are not during a direct function execution:
    bool isDuringDirectFuncExec = suIsDuringDirectFunctionExecution();

    if (!isDuringDirectFuncExec)
    {
        // Nothing to be done:
        retVal = true;
    }
    else
    {
        // We are during direct function execution:
        OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_waitingForDirectFunctionExecution, OS_DEBUG_LOG_DEBUG);

        // TO_DO: Implement windows as Linux does (See comment above at stat_isDuringFunExecutionCondition definition).
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
        {
            // Uri, 12/9/12 - This is now done inside suBeforeDirectFunctionExecution, to avoid a race condition
            // between the API thread and executing thread. This would sometimes cause the call to unlockCondition()
            // to appear before this line, which in turn made it so that this would hang forever.
            // bool rc1 = stat_isDuringFunExecutionCondition.lockCondition();
            // GT_IF_WITH_ASSERT(rc1)
            {
                // Wait until the function execution ends:
                bool rc2 = stat_isDuringFunExecutionCondition.waitForCondition();
                GT_IF_WITH_ASSERT(rc2)
                {
                    retVal = true;
                }
            }
        }
#else
        {
            retVal = osWaitForFlagToTurnOff(su_stat_isDuringDirectFunctionExecution, 100000000);
        }
#endif

        // Debug log printout:
        OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_finishedWaitingForDirectFunctionExecution, OS_DEBUG_LOG_DEBUG);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suIsInProcessTerminationAPILoop
// Description: Used by the API loop to determine if it should stop running when
//              it gets a "resume" command.
// Author:      Uri Shomroni
// Date:        20/12/2009
// ---------------------------------------------------------------------------
bool suIsInProcessTerminationAPILoop()
{
    return su_stat_isInProcessTerminationAPILoop;
}


// ---------------------------------------------------------------------------
// Name:        suSetTerminationInitiatedByAPI
// Description: Lets the spy DLLs know that the process is being terminated by
//              gaTerminateDebuggedProcess, so we do not need to report this to
//              the debugger.
// Author:      Uri Shomroni
// Date:        21/12/2009
// ---------------------------------------------------------------------------
void suSetTerminationInitiatedByAPI()
{
    su_stat_isTerminationInitiatedByAPI = true;
}


// ---------------------------------------------------------------------------
// Name:        suHandleAPILoopTerminatingCalls
// Description: Handles API calls that might terminate the API calls handling loop.
// Arguments: executedFunctionId - The executed function id.
//            shouldAPILoopContinueRunning - true - the API calls handling loop should continue running.
//                                           false - the API calls handling loop should terminate.
// Author:      Yaki Tebeka
// Date:        28/12/2009
// ---------------------------------------------------------------------------
void suHandleAPILoopTerminatingCalls(apAPIFunctionId executedFunctionId, bool& shouldAPILoopContinueRunning)
{
    // The API loop should continue running until proved otherwise:
    shouldAPILoopContinueRunning = true;

    // Will get true iff a function that ends an API connection initialization was called:
    bool wasAPIInitEndedFunctionCalled = false;

    switch (executedFunctionId)
    {
        case GA_FID_gaTerminateDebuggedProcess:
        {
            // If we were asked to terminate the debugged process, exit the API calls handling loop as well:
            shouldAPILoopContinueRunning = false;
        }
        break;

        case GA_FID_gaIntializeAPIEnded:
        {
            // The Spies Utilities API initialization ended:
            suMarkAPIConnectionAsInitialized(AP_SPIES_UTILITIES_API_CONNECTION);
            wasAPIInitEndedFunctionCalled = true;
        }
        break;

        case GA_FID_gaOpenGLServerInitializationEnded:
        {
            // The OpenGL Server API initialization ended:
            suMarkAPIConnectionAsInitialized(AP_OPENGL_API_CONNECTION);
            wasAPIInitEndedFunctionCalled = true;
        }
        break;

        case GA_FID_gaOpenCLServerInitializationEnded:
        {
            suMarkAPIConnectionAsInitialized(AP_OPENCL_API_CONNECTION);
            wasAPIInitEndedFunctionCalled = true;
        }
        break;

        default:
            // All other API function calls should not terminate the API calls handling loop.
            break;
    }

    // If a function that ends an API connection initialization was called:
    if (wasAPIInitEndedFunctionCalled)
    {
        // If the API thread didn't start running yet, it means that the API loop was called by the main debugged application thread.
        // We should exit the loop to enable the main thread to continue running:
        bool isAPIThreadRunning = suIsAPIThreadRunning();

        if (!isAPIThreadRunning)
        {
            shouldAPILoopContinueRunning = false;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        suHandleAPILoopBehaviourDuringDebuggedProcessTermination
// Description:
//  Handles the API loop's behavior during the debugged process termination.
// Arguments: executedFunctionId - executedFunctionId - The executed function id.
//            shouldAPILoopContinueRunning - true - the API calls handling loop should continue running.
//                                           false - the API calls handling loop should terminate.
// Author:      Yaki Tebeka
// Date:        29/12/2009
// ---------------------------------------------------------------------------
void suHandleAPILoopBehaviourDuringDebuggedProcessTermination(apAPIFunctionId executedFunctionId, bool& shouldAPILoopContinueRunning)
{
    // If the current thread is the API thread:
    osThreadId currentThread = osGetCurrentThreadId();
    osThreadId apiThread = suSpiesAPIThreadId();

    if (currentThread == apiThread)
    {
        // If we somehow got here from the API thread, stop running:
        OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_apiThreadQuitsDueToProcessTermination, OS_DEBUG_LOG_DEBUG);
        shouldAPILoopContinueRunning = false;
    }
    else if (executedFunctionId == GA_FID_gaResumeDebuggedProcess)
    {
        // If we were asked to resume the debugged process run:
        OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_resumeCommandDuringTermination, OS_DEBUG_LOG_DEBUG);

        // Exit the API loop to continue the debugged process termination:
        shouldAPILoopContinueRunning = false;
    }
}



// ---------------------------------------------------------------------------
// Name:        suOutputHandlingAPIFuncDebugLogPrintout
// Description: Output a debug log message that notifies about an API function
//              call handling.
// Arguments:   functionId - The called API function id.
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
void suOutputHandlingAPIFuncDebugLogPrintout(apAPIFunctionId functionId)
{
    // If the debug log severity is "debug info" or greater:
    osDebugLogSeverity debugLogSeverity = osDebugLog::instance().loggedSeverity();

    if (debugLogSeverity >= OS_DEBUG_LOG_DEBUG)
    {
        // Build the debug message:
        gtString debugMessage;
        apAPIFunctionIdToString(functionId, debugMessage);
        debugMessage.prepend(SU_STR_DebugLog_APIFunctionCalled);

        // Output the debug message:
        OS_OUTPUT_DEBUG_LOG(debugMessage.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }
}


// ---------------------------------------------------------------------------
// Name:        suOutputEndedHandlingAPIFuncDebugLogPrintout
// Description: Outputs a debug log message that notifies that we finished
//              handling an API function call.
// Arguments:   functionId - The called API function id.
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
void suOutputEndedHandlingAPIFuncDebugLogPrintout(apAPIFunctionId functionId)
{
    // If the debug log severity is "debug info" or greater:
    osDebugLogSeverity debugLogSeverity = osDebugLog::instance().loggedSeverity();

    if (debugLogSeverity >= OS_DEBUG_LOG_DEBUG)
    {
        // Build the debug message:
        gtString calledAPIFunctionString;
        apAPIFunctionIdToString(functionId, calledAPIFunctionString);

        gtString debugMessage = SU_STR_DebugLog_APIFuncHandlingEnded;
        debugMessage += calledAPIFunctionString;

        // Output the debug message:
        OS_OUTPUT_DEBUG_LOG(debugMessage.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }
}

// ---------------------------------------------------------------------------
// Name:        suSupressSpyEvents
// Description: Toggles the "suppress spy events" flag, which removes spy events
//              which are not necessary.
// Author:      Uri Shomroni
// Date:        6/10/2013
// ---------------------------------------------------------------------------
void suSupressSpyEvents(bool supressSpyEvents)
{
    su_stat_supressSpyEvents = supressSpyEvents;
}

// ---------------------------------------------------------------------------
// Name:        suAreSpyEventsSuppressed
// Description: Return the current value of the "suppress spy events" flag.
// Author:      Uri Shomroni
// Date:        6/10/2013
// ---------------------------------------------------------------------------
bool suAreSpyEventsSuppressed()
{
    return su_stat_supressSpyEvents;
}

// ---------------------------------------------------------------------------
// Name:        suSendSpyProgressEvent
// Description: The function is used for long operations in the spy.
//              The function send CodeXL a progress events, just to say that
//              things are still happening
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        29/7/2010
// ---------------------------------------------------------------------------
void suSendSpyProgressEvent(int progress)
{
    if (!su_stat_supressSpyEvents)
    {
        // Send a progress event:
        apSpyProgressEvent progressEvent(progress);

        // Write the searching for memory leak event:
        bool rcEve = suForwardEventToClient(progressEvent);
        GT_ASSERT(rcEve);

        // Get the log severity (print the message only in debug level):
        osDebugLogSeverity debugLogSeverity = osDebugLog::instance().loggedSeverity();

        if (debugLogSeverity >= OS_DEBUG_LOG_DEBUG)
        {
            // Build the debug message:
            gtString spyProgressStr;
            spyProgressStr.appendFormattedString(SU_STR_SpyIsInProgress, progress);

            // Output the debug message:
            OS_OUTPUT_DEBUG_LOG(spyProgressStr.asCharArray(), OS_DEBUG_LOG_DEBUG);
        }
    }
}

