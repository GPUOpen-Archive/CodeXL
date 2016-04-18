//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaPrivateAPIFunctions.cpp
///
//==================================================================================

//------------------------------ gaPrivateAPIFunctions.cpp ------------------------------

// -----------------------------------------------------------------------------------------
//  This file contains proxy functions (functions that have implementations in the spy side)
//  but are private.
// -----------------------------------------------------------------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osNULLSocket.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTAPIClasses/Include/apCounterID.h>
#include <AMDTAPIClasses/Include/apCounterInfo.h>
#include <AMDTAPIClasses/Include/apDetectedErrorParameters.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apInfrastructureEndsBeingBusyEvent.h>
#include <AMDTAPIClasses/Include/Events/apInfrastructureStartsBeingBusyEvent.h>
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>

// Local:
#include <src/gaStringConstants.h>
#include <src/gaAPIToSpyConnector.h>
#include <src/gaPrivateAPIFunctions.h>
#include <src/gaPersistentDataManager.h>
#include <AMDTAPIClasses/Include/apAPIFunctionId.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>


// ---------------------------------------------------------------------------
// Name:        gaGenerateUniquePipeName
// Description: Generates a unique pipe object name.
// Arguments:
//  pipeName - Will get the unique shared memory object name.
//
// Author:      Yaki Tebeka
// Date:        6/9/2005
// ---------------------------------------------------------------------------
void gaGenerateUniquePipeName(gtString& pipeName)
{
    pipeName = GA_SHARED_MEM_OBJ_PREFIX;

    // Get the current time and date as strings:
    osTime curretTime;
    curretTime.setFromCurrentTime();

    gtString dateAsString;
    curretTime.dateAsString(dateAsString, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);

    gtString timeAsString;
    curretTime.timeAsString(timeAsString, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);

    // Append the date and time to the file name:
    pipeName += L"-";
    pipeName += dateAsString;
    pipeName += L"-";
    pipeName += timeAsString;
}


// ---------------------------------------------------------------------------
// Name:        gaInitializeAPIConnection
// Description:
//   Initializes a API TCP / IP socket connection with the spy.
//   I.E: Listens to the port to which the spies will connect to.
//        When the connection attempt arrives, accepts it.
// Arguments:   portAddress - The address of the port to which the
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/6/2004
// ---------------------------------------------------------------------------
bool gaInitializeAPIConnection(const osPortAddress& spyAPIPortAddress, const osPortAddress& incomingEventsPortAddress)
{
    bool retVal = false;

    // Get (create) the Spy connector instance:
    gaAPIToSpyConnector& theSpyConnector = gaAPIToSpyConnector::instance();

    // Initialize the Spy connector:
    retVal = theSpyConnector.initialize(spyAPIPortAddress, incomingEventsPortAddress);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaInitializeAPIConnection
// Description:
//   Initializes a shared memory socket connection with the spy.
//   I.E: Creates a shared memory object and waits for the spies to
//        connect to it.
// Arguments:   spyAPIPipeName - The name of the spies API pipe.
//              incomingEventsPipeName - The name of the spies incoming events pipe.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/6/2004
// ---------------------------------------------------------------------------
bool gaInitializeAPIConnection(const gtString& spyAPIPipeName, const gtString& incomingEventsPipeName)
{
    bool retVal = false;

    // Get (create) the Spy connector instance:
    gaAPIToSpyConnector& theSpyConnector = gaAPIToSpyConnector::instance();

    // Initialize the Spy connector:
    retVal = theSpyConnector.initialize(spyAPIPipeName, incomingEventsPipeName);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaSpiesAPISocket
// Description: Returns the socket that connects the API with the spies side of
//              the API, running within the debugged process.
// Author:      Yaki Tebeka
// Date:        14/6/2004
// ---------------------------------------------------------------------------
osSocket& gaSpiesAPISocket()
{
    // A NULL socket that will be returned when we don't have a spy connecting
    // socket:
    static osNULLSocket static_NullSocket(L"StaticNullSocket");
    osSocket* pRetVal = &static_NullSocket;

    // Get the Spy connector:
    gaAPIToSpyConnector& theSpyConnector = gaAPIToSpyConnector::instance();

    // Get the spies API connecting socket:
    osSocket* pSpyConnectionSocket = theSpyConnector.spiesAPIConnectingSocket();

    if (pSpyConnectionSocket)
    {
        pRetVal = pSpyConnectionSocket;
    }
    else
    {
        // We don't have a socket to return:
        GT_ASSERT(false);
    }

    return *pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gaBeforeDirectAPIFunctionExecution
// Description:
//   Should be called before "direct" API function execution.
//   Direct API function execution is done by setting a debugged process thread's
//   execution context to point the API function stub function and then resuming
//   the thread's run.
//   For more details see pdWin32ProcessDebugger::makeThreadExecuteFunction documentation.
//
// Arguments: functionToBeCalled - The id of the API function that will be called.
// Return Val: void* - The address, in the debugged process address space,
//                     of the stub function of the input API function.
//                     (or NULL if this stub function does not support direct execution).
// Author:      Yaki Tebeka
// Date:        17/11/2005
// ---------------------------------------------------------------------------
osProcedureAddress64 gaBeforeDirectAPIFunctionExecution(apAPIFunctionId functionToBeCalled)
{
    osProcedureAddress64 retVal = OS_NULL_PROCEDURE_ADDRESS_64;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Get the functione execution mode:
        pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();
        pdProcessDebugger::FunctionExecutionMode funcExecMode = theProcessDebugger.functionExecutionMode();

        if (funcExecMode == pdProcessDebugger::PD_DIRECT_EXECUTION_MODE)
        {
            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaBeforeDirectAPIFunctionExecution;

            // Send arguments:
            spyConnectionSocket << (gtInt32)functionToBeCalled;

            // Receive return value:
            gtUInt64 retValAsUInt64 = 0;
            spyConnectionSocket >> retValAsUInt64;
            retVal = (osProcedureAddress64)retValAsUInt64;
        }
        else if (funcExecMode == pdProcessDebugger::PD_EXECUTION_IN_BREAK_MODE)
        {
            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaBeforeDirectAPIFunctionExecutionDuringBreak;

            // Send arguments:
            spyConnectionSocket << (gtInt32)functionToBeCalled;

            // See if we succeeded:
            bool rcFunc = false;
            spyConnectionSocket >> rcFunc;

            if (rcFunc)
            {
                // Return a non-null value:
                retVal = PD_EXECUTION_IN_BREAK_DUMMY_ADDRESS;
            }
        }
        else
        {
            // Something's wrong!!!
            GT_ASSERT(false);
        }
    }

    return retVal;
}

#if 0
// ---------------------------------------------------------------------------
// Name:        gaBeforeDirectAPIFunctionExecutionInBreakpoint
// Description:
//   Should be called before "direct" API function execution in a functions.
//   For more details see suBreakpointsManger::handleFunctionExecutionDuringBreak documentation.
//
// Arguments: functionToBeCalled - The id of the API function that will be called.
// Author:      Uri Shomroni
// Date:        12/2/2010
// ---------------------------------------------------------------------------
void gaBeforeDirectAPIFunctionExecutionInBreakpoint(apAPIFunctionId functionToBeCalled)
{
    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();
        // Is there missing code here?
    }
}
#endif

// ---------------------------------------------------------------------------
// Name:        gaGetBreakReason
// Description:
//   Returns the current debugged process break (breakpoint suspension) reason.
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        16/6/2004
// ---------------------------------------------------------------------------
bool gaGetBreakReason(apBreakReason& breakReason)
{
    bool retVal = false;

    if (gaIsHostBreakPoint())
    {
        breakReason = pdProcessDebugger::instance().hostBreakReason();
        retVal = (AP_FOREIGN_BREAK_HIT != breakReason);
    }

    if (!retVal)
    {
        // Verify that the API is active:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_SPIES_UTILITIES_API_CONNECTION))
        {
            // Get the Spy connecting socket:
            osSocket& spyConnectionSocket = gaSpiesAPISocket();

            // Send the function Id:
            spyConnectionSocket << (gtInt32)GA_FID_gaGetBreakReason;

            // Receive success value:
            spyConnectionSocket >> retVal;

            // Receive the retVal:
            gtInt32 breakReasonAsInt32 = AP_FOREIGN_BREAK_HIT;
            spyConnectionSocket >> breakReasonAsInt32;
            breakReason = (apBreakReason)breakReasonAsInt32;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetCurrentOpenGLError
// Description:
//   Returns the OpenGL error generated by the last OpenGL call (if exists).
//   This function should be called only when breaking at an OpenGL error
//   breakpoint.
// Arguments:   openGLError - The OpenGL error, as returned by glGetError
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        25/8/2004
// ---------------------------------------------------------------------------
bool gaGetCurrentOpenGLError(GLenum& openGLError)
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetCurrentOpenGLError;

        // Receive success value:
        spyConnectionSocket >> retVal;

        // Receive the retVal:
        gtInt32 openGLErrorAsInt32 = GL_NO_ERROR;
        spyConnectionSocket >> openGLErrorAsInt32;
        openGLError = (GLenum)openGLErrorAsInt32;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetDetectedErrorParameters
// Description: Retrieves the current detected error parameters.
// Arguments: detectedErrorParameters - Will get the current detected error
//                                      parameters.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        7/10/2007
// ---------------------------------------------------------------------------
bool gaGetDetectedErrorParameters(apDetectedErrorParameters& detectedErrorParameters)
{
    bool retVal = false;

    // Verify that the API is active:
    if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetDetectedErrorParameters;

        // Receive success value:
        bool rc1 = false;
        spyConnectionSocket >> rc1;

        GT_IF_WITH_ASSERT(rc1)
        {
            bool rc2 = detectedErrorParameters.readSelfFromChannel(spyConnectionSocket);
            GT_IF_WITH_ASSERT(rc2)
            {
                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaUpdateContextDataSnapshot
// Description:
//  Updates an input context data snapshot. The context data is data held per context by
//  the OpenGL32.dll spy. This data contains:
//  - State variable values.
//  - The enabled texturing mode.
//  - Linked programs active uniform values.
//  - etc
//
// Arguments:   contextId - The id of the context who's state variables snapshot
//                          will be updated.
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        11/5/2005
//
// Implementation Notes:
//  This function operates only when the the debugged process is suspended.
//  Only the the thread that has the input context as its "current render context" can
//  access the context data. Therefore:
//
//  - If the render context does not have a "current thread":
//      a. Make the API thread "current context" the input context.
//      b. Update the context data snapshot.
//      c. Return the API thread "current context" to be NULL.
//
//  - Else
//      There is a thread that its "current context" is the input context. This thread is
//      now in suspended mode. So, we will make this thread update the render context data.
//      This is done by:
//      a. Changes the thread "execution context" (instruction counter) to point the
//         gaUpdateCurrentThreadRenderContextDataSnapshotStub function start address.
//      b. Resume the thread run.
//      c. When gaUpdateCurrentThreadRenderContextDataSnapshotStub is done, it writes
//         the return value into the API socket and triggers a breakpoint exception.
//      d. The process debugger catch this breakpoint and return the thread "execution context"
//         to the position it was before step a.
// ---------------------------------------------------------------------------
bool gaUpdateContextDataSnapshot(int contextId)
{
    bool retVal = false;

    // Update functions should not be used while the debugged process is being
    // Terminated. Note that this assumes that the Persistent Data Manager gets
    // the first notification of debugged process termination
    bool isDuringProcessTermination = gaPersistentDataManager::instance().isDuringDebuggedProcessTermination();
    bool isInKernelDebugging = gaPersistentDataManager::instance().isKernelDebuggingOnGoing();

    if (!isDuringProcessTermination && !isInKernelDebugging)
    {
        // Verify that the API is active and suspended:
        if (gaIsAPIConnectionActiveAndDebuggedProcessSuspended(AP_OPENGL_API_CONNECTION))
        {
            // If this is a real context:
            if (0 < contextId)
            {
                // Get the thread that is the queried context "current thread":
                osThreadId threadId = OS_NO_THREAD_ID;
                bool rc = gaGetRenderContextCurrentThread(contextId, threadId);

                if (rc)
                {
                    // Notify (immediately) observers that we start updating the context data:
                    apInfrastructureStartsBeingBusyEvent infraStartsBusyEvent(apInfrastructureStartsBeingBusyEvent::AP_UPDATING_DEBUGGED_PROCESS_DATA);
                    apEventsHandler::instance().handleDebugEvent(infraStartsBusyEvent);

                    // If there is no thread that has the queried context as "current thread":
                    if (threadId == OS_NO_THREAD_ID)
                    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                        {
                            // Get the Spy connecting socket:
                            osSocket& spyConnectionSocket = gaSpiesAPISocket();

                            // Update the context snapshot data (by temporary making the API thread "current
                            // render context" to be the input context:
                            spyConnectionSocket << GA_FID_gaUpdateContextDataSnapshot;
                            spyConnectionSocket << (gtInt32)contextId;
                            spyConnectionSocket >> retVal;
                        }
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
                        {
                            // Yaki 3/9/2008:
                            // This functionality is not supported on Linux, since glXMakeCurrent inputs
                            // both a drawable and a render context and we don't know which drawable to use.
                            // In Mac there is a similar problem, only the current drawable (or draw mode - eg
                            // drawing to fullscreen / window / offscreen buffer) is set separately from the call
                            // to CGLSetCurrentContext in a set of similar functions.
                            // (On windows a drawable and a context have a 1 to 1 relationship, therefore we can
                            //  use wglMakeCurrent which inputs a render context).
                            retVal = false;
                        }
#else
                        {
#error Unsupported platform
                        }
#endif
                    }
                    else
                    {
                        // We found a thread that has the queried context as its "current render thread":
                        pdProcessDebugger& theProcessDebugger = pdProcessDebugger::instance();

                        if (theProcessDebugger.canMakeThreadExecuteFunction(threadId))
                        {
                            // Get gaUpdateCurrentThreadRenderContextDataSnapshotStub address:
                            osProcedureAddress64 gaUpdateCurrentThreadRenderContextDataSnapshotStubAddr = gaBeforeDirectAPIFunctionExecution(GA_FID_gaUpdateCurrentThreadRenderContextDataSnapshot);
                            GT_IF_WITH_ASSERT(gaUpdateCurrentThreadRenderContextDataSnapshotStubAddr != OS_NULL_PROCEDURE_ADDRESS_64)
                            {
                                // Make the "current thread" execute gaUpdateCurrentThreadRenderContextDataSnapshotStub:
                                rc = theProcessDebugger.makeThreadExecuteFunction(threadId, gaUpdateCurrentThreadRenderContextDataSnapshotStubAddr);

                                if (rc)
                                {
                                    // Read the result (written by gaUpdateCurrentThreadRenderContextDataSnapshotStub):
                                    osSocket& spyConnectionSocket = gaSpiesAPISocket();
                                    spyConnectionSocket >> retVal;
                                }
                            }
                        }
                        else
                        {
#ifdef _SUPPORT_REMOTE_EXECUTION
                            // We cannot make the threads execute functions directly - send the request to the API (which will make the thread
                            // execute the function using gsSpyBreakpointImplementation:
                            // Get the Spy connecting socket:
                            osSocket& spyConnectionSocket = gaSpiesAPISocket();

                            // Send the command ID:
                            spyConnectionSocket << (gtInt32)GA_FID_gaMakeThreadUpdateContextDataSnapshot;

                            // Send the thread ID:
                            spyConnectionSocket << (gtUInt64)threadId;

                            // Receive success value:
                            spyConnectionSocket >> retVal;
#else
                            // We shouldn't get here on these builds:
                            GT_ASSERT(false);
#endif
                        }
                    }

                    // Notify observers that we ended updating the context data:
                    apInfrastructureEndsBeingBusyEvent infraEndsBusyEvent;
                    apEventsHandler::instance().handleDebugEvent(infraEndsBusyEvent);
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetSpyPerformanceCountersValues
// Description: Queries the current values of the spy performance counters.
// Arguments:   pValuesArray - An array of valuesArraySize that will receive the performance counter values.
//                             If there are more values than the array size (example, when a new render
//                             context is created), the function will fill only the first valuesArraySize values.
//              valuesArraySize - The size of the pValuesArray vector.
//              contextsAmount - Will get the current amount of render contexts, created by the debugged application.
//                               This value can be used to resize the input pValuesArray vector in the next calls to
//                               this function.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        24/7/2005
// ---------------------------------------------------------------------------
bool gaGetSpyPerformanceCountersValues(double*& pValuesArray, int& valuesArraySize)
{
    bool retVal = false;

    // Verify the the connection to the OpenGL32.dll spy is active:
    if (gaIsAPIConnectionActive(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetSpyPerformanceCountersValues;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive the amount of values:
            gtInt32 valuesAmount = 0;
            spyConnectionSocket >> valuesAmount;

            if ((valuesAmount != valuesArraySize) || (pValuesArray == NULL))
            {
                valuesArraySize = valuesAmount;
                delete[] pValuesArray;

                // Reallocate pValuesArray if needed and update valuesArraySize if needed.
                pValuesArray = new double[valuesArraySize];
            }

            for (int i = 0; i < valuesAmount; i++)
            {
                spyConnectionSocket >> pValuesArray[i];
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetSpyPerformanceCountersValues
// Description: Queries the current values of the remote OS performance counters.
// Arguments:   pValuesArray - An array of valuesArraySize that will receive the performance counter values.
//                             If there are more values than the array size (example, when a new render
//                             context is created), the function will fill only the first valuesArraySize values.
//              valuesArraySize - The size of the pValuesArray vector.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        23/11/2009
// ---------------------------------------------------------------------------
bool gaGetRemoteOSPerformanceCountersValues(double*& pValuesArray, int& valuesArraySize)
{
    bool retVal = false;

    // Verify the the connection to the OpenGL32.dll spy is active:
    if (gaIsAPIConnectionActive(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetRemoteOSPerformanceCountersValues;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive the amount of values:
            gtInt32 valuesAmount = 0;
            spyConnectionSocket >> valuesAmount;

            if ((valuesAmount != valuesArraySize) || (pValuesArray == NULL))
            {
                valuesArraySize = valuesAmount;
                delete[] pValuesArray;

                // Reallocate pValuesArray if needed and update valuesArraySize if needed.
                pValuesArray = new double[valuesArraySize];
            }

            for (int i = 0; i < valuesAmount; i++)
            {
                spyConnectionSocket >> pValuesArray[i];
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaAddSupportediPhonePerformanceCounter
// Description: Adds support for a given iPhone performance counter.
// Arguments:   counterIndex - The counter's index in pValuesArray of gaGetiPhonePerformanceCountersValues.
//              counterName - The counter's name.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        28/10/2010
// ---------------------------------------------------------------------------
bool gaAddSupportediPhonePerformanceCounter(int counterIndex, const gtString& counterName)
{
    bool retVal = false;

    // Verify the the connection to the OpenGL32.dll spy is active:
    if (gaIsAPIConnectionActive(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaAddSupportediPhonePerformanceCounter;

        // Send arguments:
        spyConnectionSocket << (gtInt32)counterIndex;
        spyConnectionSocket << counterName;

        // Receive success value:
        spyConnectionSocket >> retVal;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetiPhonePerformanceCountersValues
// Description: Initializes the OpenGL ES Server's side of the iPhone performance
//              counters reading facilities.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        28/10/2010
// ---------------------------------------------------------------------------
bool gaInitializeiPhonePerformanceCountersReader()
{
    bool retVal = false;

    // Verify the the connection to the OpenGL32.dll spy is active:
    if (gaIsAPIConnectionActive(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaInitializeiPhonePerformanceCountersReader;

        // Receive success value:
        spyConnectionSocket >> retVal;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetiPhonePerformanceCountersValues
// Description: Queries the current values of the active iPhone performance counters.
// Arguments:   pValuesArray - An array of valuesArraySize that will receive the performance counter values.
//                             If there are more values than the array size,
//                             the function will fill only the first valuesArraySize values.
//              valuesArraySize - The size of the pValuesArray vector.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/1/2010
// ---------------------------------------------------------------------------
bool gaGetiPhonePerformanceCountersValues(double*& pValuesArray, int& valuesArraySize)
{
    bool retVal = false;

    // Verify the the connection to the OpenGL32.dll spy is active:
    if (gaIsAPIConnectionActive(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetiPhonePerformanceCountersValues;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive the amount of values:
            gtInt32 valuesAmount = 0;
            spyConnectionSocket >> valuesAmount;

            if ((valuesAmount != valuesArraySize) || (pValuesArray == NULL))
            {
                valuesArraySize = valuesAmount;
                delete pValuesArray;

                // Reallocate pValuesArray if needed and update valuesArraySize if needed.
                pValuesArray = new double[valuesArraySize];
            }

            for (int i = 0; i < valuesAmount; i++)
            {
                // TO_DO: Read me in one chunk!!!
                spyConnectionSocket >> pValuesArray[i];
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetATIPerformanceCountersValues
// Description: Queries the current values of the active ATI performance counters.
// Arguments:   pValuesArray - An array of valuesArraySize that will receive the performance counter values.
//                             If there are more values than the array size (example, when a new render
//                             context is created), the function will fill only the first valuesArraySize values.
//              valuesArraySize - The size of the pValuesArray vector.
//              contextsAmount - Will get the current amount of render contexts, created by the debugged application.
//                               This value can be used to resize the input pValuesArray vector in the next calls to
//                               this function.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/03/2008
// ---------------------------------------------------------------------------
bool gaGetATIPerformanceCountersValues(double*& pValuesArray, int& valuesArraySize)
{
    bool retVal = false;

    // Verify the the connection to the OpenGL32.dll spy is active:
    if (gaIsAPIConnectionActive(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetATIPerformanceCountersValues;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive the amount of values:
            gtInt32 valuesAmount = 0;
            spyConnectionSocket >> valuesAmount;

            if (valuesAmount > 0)
            {
                if ((valuesAmount != valuesArraySize) || (pValuesArray == NULL))
                {
                    valuesArraySize = valuesAmount;
                    delete[] pValuesArray;

                    // Reallocate pValuesArray if needed and update valuesArraySize if needed.
                    pValuesArray = new double[valuesArraySize];

                }

                for (int i = 0; i < valuesAmount; i++)
                {
                    // TO_DO: Read me in one chunk!!!
                    spyConnectionSocket >> pValuesArray[i];
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaActivateATIPerformanceCounters
// Description: Activates a list of counters
// Arguments: gtVector<apCounterActivationInfo> counterIDsVec
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/1/2010
// ---------------------------------------------------------------------------
bool gaActivateATIPerformanceCounters(const gtVector<apCounterActivationInfo>& counterIDsVec)
{
    bool retVal = false;

    // Verify the the connection to the OpenGL32.dll spy is active:
    if (gaIsAPIConnectionActive(AP_OPENGL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaActivateATIPerformanceCounters;

        // Get the counters amount:
        gtInt32 amountOfCounters = (gtInt32)counterIDsVec.size();

        // Send the counters amount:
        spyConnectionSocket << (gtInt32)amountOfCounters;

        // Send the counter activation infos:
        for (int i = 0 ; i < amountOfCounters; i++)
        {
            // Send the current activation info:
            apCounterActivationInfo activationInfo = counterIDsVec[i];
            activationInfo.writeSelfIntoChannel(spyConnectionSocket);
        }

        // Receive success value:
        spyConnectionSocket >> retVal;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaCreateEventForwardingTCPConnection
// Description: Tells the spy and process debugger to create a TCP socket connection
//              to forward events from the spy to CodeXL.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        29/10/2009
// ---------------------------------------------------------------------------
bool gaCreateEventForwardingTCPConnection(const osPortAddress& portAddress)
{
    bool retVal = false;

    // Uri, 17/9/13 - this check currently requires the spies API thread to be defined in the
    // process debugger. However, the initializations of this thread's id is done after this
    // function is called. Compare also gaCreateEventForwardingPipeConnection(), where this is not
    // a dependency.
    // Verify the spy connection is active:
    // if (gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION))
    {
        // If the process debugger succeeded in creating the pipe, tell the spy to do the same:
        // Get the Spy connection socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the command ID:
        spyConnectionSocket << (gtInt32)GA_FID_gaCreateEventForwardingTCPConnection;

        // Send the connection parameters:
        spyConnectionSocket << portAddress.hostName();
        spyConnectionSocket << (gtUInt16)portAddress.portNumber();

        // Read the success value:
        spyConnectionSocket >> retVal;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaCreateEventForwardingPipeConnection
// Description: Creates the spies side of the events forwarding pipe connection.
// Arguments: eventsPipeName - the spies events pipe name.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        16/12/2009
// ---------------------------------------------------------------------------
bool gaCreateEventForwardingPipeConnection(const gtString& eventsPipeName)
{
    bool retVal = false;

    // If the process debugger succeeded in creating the pipe, tell the spy to do the same:
    // Get the Spy connection socket:
    osSocket& spyConnectionSocket = gaSpiesAPISocket();

    // Send the command ID:
    spyConnectionSocket << (gtInt32)GA_FID_gaCreateEventForwardingPipeConnection;

    // Send the connection parameters:
    spyConnectionSocket << eventsPipeName;

    // Read the success value:
    spyConnectionSocket >> retVal;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaUpdateOpenCLContextDataSnapshot
// Description: Updates OpenCL context data snapshot
// Arguments: int contextId
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/11/2009
// ---------------------------------------------------------------------------
bool gaUpdateOpenCLContextDataSnapshot(int contextId)
{
    bool retVal = false;

    // Update functions should not be used while the debugged process is being
    // Terminated. Note that this assumes that the Persistent Data Manager gets
    // the first notification of debugged process termination
    bool isDuringProcessTermination = gaPersistentDataManager::instance().isDuringDebuggedProcessTermination();

    if (!isDuringProcessTermination)
    {
        // Verify that the API is active and suspended:
        if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
        {
            // If this is a real context:
            if (0 <= contextId)
            {
                // Get the Spy connecting socket:
                osSocket& spyConnectionSocket = gaSpiesAPISocket();

                // Update the context snapshot data (by temporary making the API thread "current
                // render context" to be the input context:
                spyConnectionSocket << GA_FID_gaUpdateOpenCLContextDataSnapshot;
                spyConnectionSocket << (gtInt32)contextId;
                spyConnectionSocket >> retVal;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetAMDOpenCLPerformanceCountersValues
// Description: Queries the current values of the active AMD OpenCL performance counters.
// Arguments:   pValuesArray - An array of valuesArraySize that will receive the performance counter values.
//                             If there are more values than the array size (example, when a new render
//                             context is created), the function will fill only the first valuesArraySize values.
//              valuesArraySize - The size of the pValuesArray vector.
//              contextsAmount - Will get the current amount of render contexts, created by the debugged application.
//                               This value can be used to resize the input pValuesArray vector in the next calls to
//                               this function.
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/2/2010
// ---------------------------------------------------------------------------
bool gaGetAMDOpenCLPerformanceCountersValues(double*& pValuesArray, int& valuesArraySize)
{
    bool retVal = false;

    // Verify the the connection to the OpenCL.dll spy is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetAMDOpenCLPerformanceCountersValues;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive the amount of values:
            gtInt32 valuesAmount = 0;
            spyConnectionSocket >> valuesAmount;

            if (valuesAmount > 0)
            {
                if ((valuesAmount != valuesArraySize) || (pValuesArray == NULL))
                {
                    valuesArraySize = valuesAmount;
                    delete[] pValuesArray;

                    // Reallocate pValuesArray if needed and update valuesArraySize if needed.
                    pValuesArray = new double[valuesArraySize];

                }

                for (int i = 0; i < valuesAmount; i++)
                {
                    // TO_DO: Read me in one chunk!!!
                    spyConnectionSocket >> pValuesArray[i];
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaActivateAMDOpenCLPerformanceCounters
// Description: Activates a list of counters
// Arguments: gtVector<apCounterActivationInfo> counterIDsVec
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/2/2010
// ---------------------------------------------------------------------------
bool gaActivateAMDOpenCLPerformanceCounters(const gtVector<apCounterActivationInfo>& counterIDsVec)
{
    bool retVal = false;

    // Verify the the connection to the OpenCL.dll spy is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaActivateAMDOpenCLPerformanceCounters;

        // Get the counters amount:
        gtInt32 amountOfCounters = (gtInt32)counterIDsVec.size();

        // Send the counters amount:
        spyConnectionSocket << (gtInt32)amountOfCounters;

        // Send the counter activation infos:
        for (int i = 0 ; i < amountOfCounters; i++)
        {
            // Send the current activation info:
            apCounterActivationInfo activationInfo = counterIDsVec[i];
            activationInfo.writeSelfIntoChannel(spyConnectionSocket);
        }

        // Receive success value:
        spyConnectionSocket >> retVal;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLQueuePerformanceCountersValues
// Description: Queries the current values of the OpenCL queues performance counters.
// Arguments:   pValuesArray - An array of valuesArraySize that will receive the performance counter values.
//                             If there are more values than the array size (example, when a new render
//                             context is created), the function will fill only the first valuesArraySize values.
//              valuesArraySize - The size of the pValuesArray vector.
//              contextsAmount - Will get the current amount of render contexts, created by the debugged application.
//                               This value can be used to resize the input pValuesArray vector in the next calls to
//                               this function.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/3/2010
// ---------------------------------------------------------------------------
bool gaGetOpenCLQueuePerformanceCountersValues(double*& pValuesArray, int& valuesArraySize)
{
    bool retVal = false;

    // Verify the the connection to the OpenCL.dll spy is active:
    if (gaIsAPIConnectionActive(AP_OPENCL_API_CONNECTION))
    {
        // Get the Spy connecting socket:
        osSocket& spyConnectionSocket = gaSpiesAPISocket();

        // Send the function Id:
        spyConnectionSocket << (gtInt32)GA_FID_gaGetOpenCLQueuePerformanceCountersValues;

        // Receive success value:
        spyConnectionSocket >> retVal;

        if (retVal)
        {
            // Receive the amount of values:
            gtInt32 valuesAmount = 0;
            spyConnectionSocket >> valuesAmount;

            if ((valuesAmount != valuesArraySize) || (pValuesArray == NULL))
            {
                valuesArraySize = valuesAmount;
                delete[] pValuesArray;

                // Reallocate pValuesArray if needed and update valuesArraySize if needed.
                pValuesArray = new double[valuesArraySize];
            }

            for (int i = 0; i < valuesAmount; i++)
            {
                spyConnectionSocket >> pValuesArray[i];
            }
        }
    }

    return retVal;
}

