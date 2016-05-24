//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suAPIFunctionsStubs.cpp
///
//==================================================================================

//------------------------------ suAPIFunctionsStubs.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osRawMemoryBuffer.h>
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTAPIClasses/Include/apAPIFunctionId.h>
#include <AMDTAPIClasses/Include/apApiFunctionsInitializationData.h>
#include <AMDTAPIClasses/Include/apContextID.h>
#include <AMDTAPIClasses/Include/apDetectedErrorParameters.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTAPIClasses/Include/apBasicParameters.h>

// Local:
#include <src/suAPIFunctionsImplementations.h>
#include <src/suAPIFunctionsStubs.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>
#include <AMDTServerUtilities/Include/suSWMRInstance.h>

// iPhone on-device only items:
#ifdef _GR_IPHONE_DEVICE_BUILD
    #include <AMDTServerUtilities/Include/suGlobalVariables.h>
    #include <AMDTServerUtilities/Include/suSpyBreakpointImplementation.h>
#endif


// ---------------------------------------------------------------------------
// Name:        suRegisterAPIStubFunctions
// Description: Registers spies utilities module stub functions.
// Author:      Yaki Tebeka
// Date:        30/11/2009
// ---------------------------------------------------------------------------
void suRegisterAPIStubFunctions()
{
    suRegisterAPIFunctionStub(GA_FID_gaIntializeAPI, &gaIntializeAPIStub);
    suRegisterAPIFunctionStub(GA_FID_gaIntializeAPIEnded, &gaIntializeAPIEndedStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAPIThreadId, &gaGetAPIThreadIdStub);
    suRegisterAPIFunctionStub(GA_FID_gaTerminateDebuggedProcess, &gaTerminateDebuggedProcessStub);
    suRegisterAPIFunctionStub(GA_FID_gaSuspendDebuggedProcess, &gaSuspendDebuggedProcessStub);
    suRegisterAPIFunctionStub(GA_FID_gaResumeDebuggedProcess, &gaResumeDebuggedProcessStub);
    suRegisterAPIFunctionStub(GA_FID_gaMakeThreadGetCallStack, &gaMakeThreadGetCallStackStub);
    suRegisterAPIFunctionStub(GA_FID_gaSuspendThreads, &gaSuspendThreadsStub);
    suRegisterAPIFunctionStub(GA_FID_gaResumeThreads, &gaResumeThreadsStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetDebuggedProcessExecutionMode, &gaSetDebuggedProcessExecutionModeStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetBreakpoint, &gaSetBreakpointStub);
    suRegisterAPIFunctionStub(GA_FID_gaRemoveBreakpoint, &gaRemoveBreakpointStub);
    suRegisterAPIFunctionStub(GA_FID_gaRemoveAllBreakpoints, &gaRemoveAllBreakpointsStub);
    suRegisterAPIFunctionStub(GA_FID_gaBreakOnNextMonitoredFunctionCall, &gaBreakOnNextMonitoredFunctionCallStub);
    suRegisterAPIFunctionStub(GA_FID_gaBreakOnNextDrawFunctionCall, &gaBreakOnNextDrawFunctionCallStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetDetectedErrorParameters, &gaGetDetectedErrorParametersStub);
    suRegisterAPIFunctionStub(GA_FID_gaBreakOnNextFrame, &gaBreakOnNextFrameStub);
    suRegisterAPIFunctionStub(GA_FID_gaBreakInMonitoredFunctionCall, &gaBreakInMonitoredFunctionCallStub);
    suRegisterAPIFunctionStub(GA_FID_gaClearAllStepFlags, &gaClearAllStepFlagsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetBreakReason, &gaGetBreakReasonStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetBreakpointTriggeringContextId, &gaGetBreakpointTriggeringContextIdStub);
    suRegisterAPIFunctionStub(GA_FID_gaCreateEventForwardingTCPConnection, &gaCreateEventForwardingTCPConnectionStub);
    suRegisterAPIFunctionStub(GA_FID_gaCreateEventForwardingPipeConnection, &gaCreateEventForwardingPipeConnectionStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfRegisteredAllocatedObjects, &gaGetAmountOfRegisteredAllocatedObjectsStub);
    suRegisterAPIFunctionStub(GA_FID_gaCollectAllocatedObjectsCreationCallsStacks, &gaCollectAllocatedObjectsCreationCallsStacksStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAllocatedObjectCreationStack, &gaGetAllocatedObjectCreationStackStub);
    suRegisterAPIFunctionStub(GA_FID_gaReadFile, &gaReadFileStub);
    suRegisterAPIFunctionStub(GA_FID_gaWriteFile, &gaWriteFileStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetSlowMotionDelay, &gaSetSlowMotionDelayStub);
    suRegisterAPIFunctionStub(GA_FID_gaStartMonitoredFunctionsCallsLogFileRecording, &gaStartMonitoredFunctionsCallsLogFileRecordingStub);
    suRegisterAPIFunctionStub(GA_FID_gaStopMonitoredFunctionsCallsLogFileRecording, &gaStopMonitoredFunctionsCallsLogFileRecordingStub);
    suRegisterAPIFunctionStub(GA_FID_gaIsMonitoredFunctionsCallsLogFileRecordingActive, &gaIsMonitoredFunctionsCallsLogFileRecordingActiveStub);
    suRegisterAPIFunctionStub(GA_FID_gaEnableImagesDataLogging, &gaEnableImagesDataLoggingStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetFloatParametersDisplayPrecision, &gaSetFloatParametersDisplayPrecisionStub);
    suRegisterAPIFunctionStub(GA_FID_gaFlushAfterEachMonitoredFunctionCall, &gaFlushLogFileAfterEachFunctionCallStub);
    suRegisterAPIFunctionStub(GA_FID_gaLockDriverThreads, &gaLockDriverThreadsStub);
    suRegisterAPIFunctionStub(GA_FID_gaUnlockDriverThreads, &gaUnLockDriverThreadsStub);
}


// ---------------------------------------------------------------------------
// Name:        gaIntializeAPIStub
// Description: Stub for gsInitializeAPI
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
void gaIntializeAPIStub(osSocket& apiSocket)
{
    // Read the API initialization data:
    gtAutoPtr<apApiFunctionsInitializationData> aptrInitData;
    bool rc1 = osReadTransferableObjectFromChannel<apApiFunctionsInitializationData>(apiSocket, aptrInitData);

    // Call the function implementation:
    bool rc2 = gaIntializeAPIImpl(*aptrInitData);

    // Return the API initialization status:
    bool rc = rc1 && rc2;
    apiSocket << rc;
}


// ---------------------------------------------------------------------------
// Name:        gaIntializeAPIEndedStub
// Description: Stub for gaIntializeAPIEnded
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
void gaIntializeAPIEndedStub(osSocket& apiSocket)
{
    (void)(apiSocket); // unused
    // Nothing to be done.
}


// ---------------------------------------------------------------------------
// Name:        gaGetAPIThreadIdStub
// Description: Stub for gaGetAPIThreadId
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
void gaGetAPIThreadIdStub(osSocket& apiSocket)
{
    // Get the API thread id:
    osThreadId apiThreadId = gaGetAPIThreadIdImpl();

    // Return the API thread id:
    apiSocket << (gtUInt64)apiThreadId;
}


// ---------------------------------------------------------------------------
// Name:        gaTerminateDebuggedProcessStub
// Description: Stub for gaTerminateDebuggedProcess
// Author:      Yaki Tebeka
// Date:        25/10/2004
// ---------------------------------------------------------------------------
void gaTerminateDebuggedProcessStub(osSocket& apiSocket)
{
    // Call the pre-termination part:
    gaBeforeTerminateDebuggedProcessImpl();

    // Return true - yep, we will terminate this application:
    apiSocket << true;

    // Close the API connecting socket:
    apiSocket.close();

    // Terminate this process:
    gaTerminateDebuggedProcessImpl();
}

// ---------------------------------------------------------------------------
// Name:        gaSuspendDebuggedProcessStub
// Description: Stub function for gaSuspendDebuggedProcess
// Author:      Yaki Tebeka
// Date:        3/10/2004
// ---------------------------------------------------------------------------
void gaSuspendDebuggedProcessStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    retVal = gaSuspendDebuggedProcessImpl();

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaResumeDebuggedProcessStub
// Description: Stub function for gaResumeDebuggedProcess
// Author:      Yaki Tebeka
// Date:        17/1/2007
// ---------------------------------------------------------------------------
void gaResumeDebuggedProcessStub(osSocket& apiSocket)
{
    (void)(apiSocket); // unused
    bool retVal = false;

    // Call the function implementation:
    retVal = gaResumeDebuggedProcessImpl();
    GT_ASSERT(retVal);

    // Notice - we do not return value here, since in some cases
    // (like when doing "single step"), the debugged process might
    // throw a breakpoint exception before we manage to return a value.
    // this makes the debugger process freeze.
}

// ---------------------------------------------------------------------------
// Name:        gaMakeThreadGetCallStackStub
// Description: "Direct" Stub function of gaGetThreadCallStack, for the
//              case where we have no debugger.
// Author:      Uri Shomroni
// Date:        19/11/2009
// ---------------------------------------------------------------------------
void gaMakeThreadGetCallStackStub(osSocket& apiSocket)
{
    gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    apiSocket >> threadIdAsUInt64;

#ifdef _GR_IPHONE_DEVICE_BUILD
    bool retVal = su_stat_spyBreakpointsImplemenation.makeThreadExecuteFunction((osThreadId)threadIdAsUInt64, (osProcedureAddress)&gaGetCurrentThreadCallStackInternalStub);

    // The internal stub returns the retVal as well as the calls stack, so we
    // only assert the value here:
    GT_ASSERT(retVal);
#else
    // This function should not be called if we do not have the spy breakpoints implementation:
    GT_ASSERT(false);

    // Call the internal stub to clear the function parameters from the API connection pipe:
    gaGetCurrentThreadCallStackInternalStub();
#endif
}

/// ---------------------------------------------------------------------------
/// \name:        gaSuspendThreadsStub
/// \brief        Stub function for gaSuspendThreadsImpl
/// \param apiSocket a reference to IPC instance
///
/// \author:      Vadim
/// \date:        17/12/2015
/// ---------------------------------------------------------------------------
void gaSuspendThreadsStub(osSocket& apiSocket)
{
    bool retVal = false;

    std::vector<osThreadId> thrds;
    size_t count;

    apiSocket >> count;

    for (size_t i = 0; i < count; i++)
    {
        osThreadId thrd;
        apiSocket >> thrd;
        thrds.push_back(thrd);
    }

    retVal = gaSuspendThreadsImpl(thrds);

    apiSocket << retVal;
}

/// ---------------------------------------------------------------------------
/// \name:        gaResumeThreadsStub
/// \brief        Stub function for gaSuspendThreadsImpl
///
/// \author:      Vadim
/// \date:        17/12/2015
// ---------------------------------------------------------------------------
void gaResumeThreadsStub(osSocket& apiSocket)
{
    bool retVal = false;

    retVal = gaResumeThreadsImpl();

    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetCurrentThreadCallStackInternalStub
// Description: The actual stub of gaGetThreadCallStack, used by the direct
//              stub function gaMakeThreadGetCallStackStub
// Author:      Uri Shomroni
// Date:        19/11/2009
// ---------------------------------------------------------------------------
bool gaGetCurrentThreadCallStackInternalStub()
{
    bool retVal = false;

    // Get the API socket:
    osSocket* pApiSocket = suSpiesAPISocket();
    GT_IF_WITH_ASSERT(pApiSocket != NULL)
    {
        osCallStack threadCallStack;

        bool hideSpyFunctions = false;
        *pApiSocket >> hideSpyFunctions;

        retVal = gaGetCurrentThreadCallStackImpl(threadCallStack, hideSpyFunctions);

        *pApiSocket << retVal;

        if (retVal)
        {
            threadCallStack.writeSelfIntoChannel(*pApiSocket);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetDebuggedProcessExecutionModeStub
// Description: Stub function for gaSetDeubggedProcessExecutionMode
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        29/6/2008
// ---------------------------------------------------------------------------
void gaSetDebuggedProcessExecutionModeStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Get the execution mode:
    gtInt32 executionModeInt32 = AP_DEBUGGING_MODE;
    apiSocket >> executionModeInt32;
    apExecutionMode executionMode = (apExecutionMode)executionModeInt32;

    // Call the function implementation:
    retVal = gaSetDebuggedProcessExecutionModeImpl(executionMode);

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetBreakpointStub
// Description: Stub for gaSetBreakpoint
// Author:      Yaki Tebeka
// Date:        14/6/2004
// ---------------------------------------------------------------------------
void gaSetBreakpointStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Read the breakpoint from the apiSocket:
    gtAutoPtr<osTransferableObject> aptrReadTransferableObj;
    apiSocket >> aptrReadTransferableObj;
    apBreakPoint* pBreakPoint = (apBreakPoint*)(aptrReadTransferableObj.releasePointedObjectOwnership());
    GT_IF_WITH_ASSERT(pBreakPoint != NULL)
    {
        // Call the function implementation:
        retVal = gaSetBreakpointImpl(*pBreakPoint);

        // Clean up:
        delete pBreakPoint;
        pBreakPoint = NULL;
    }

    // Return the return value:
    apiSocket << retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaRemoveBreakpointStub
// Description: Stub for gaRemoveBreakpoint
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        16/6/2008
// ---------------------------------------------------------------------------
void gaRemoveBreakpointStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Read the breakpoint from the apiSocket:
    gtAutoPtr<osTransferableObject> aptrReadTransferableObj;
    apiSocket >> aptrReadTransferableObj;
    apBreakPoint* pBreakPoint = (apBreakPoint*)(aptrReadTransferableObj.releasePointedObjectOwnership());

    if (pBreakPoint)
    {
        // Call the function implementation:
        retVal = gaRemoveBreakpointImpl(*pBreakPoint);

        // Clean up:
        delete pBreakPoint;
        pBreakPoint = NULL;
    }

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaRemoveAllBreakpointsStub
// Description: Stub for gaRemoveAllBreakpoints
// Author:      Yaki Tebeka
// Date:        14/6/2004
// ---------------------------------------------------------------------------
void gaRemoveAllBreakpointsStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    retVal = gaRemoveAllBreakpointsImpl();

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaBreakOnNextMonitoredFunctionCallStub
// Description: Stub for gaBreakOnNextMonitoredFunctionCall
// Author:      Yaki Tebeka
// Date:        15/6/2004
// ---------------------------------------------------------------------------
void gaBreakOnNextMonitoredFunctionCallStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    retVal = gaBreakOnNextMonitoredFunctionCallImpl();

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaBreakOnNextDrawFunctionCallStub
// Description: Stub for gaBreakOnNextDrawFunctionCall
// Author:      Avi Shapira
// Date:        25/5/2006
// ---------------------------------------------------------------------------
void gaBreakOnNextDrawFunctionCallStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    retVal = gaBreakOnNextDrawFunctionCallImpl();

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetDetectedErrorParametersStub
// Description: Stub function for gaGetDetectedErrorParameters
// Author:      Yaki Tebeka
// Date:        8/10/2007
// ---------------------------------------------------------------------------
void gaGetDetectedErrorParametersStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    apDetectedErrorParameters detectedErrorParameters;
    retVal = gaGetDetectedErrorParametersImpl(detectedErrorParameters);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Write output parameters:
        bool rc1 = detectedErrorParameters.writeSelfIntoChannel(apiSocket);
        GT_ASSERT(rc1);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaBreakOnNextFrameStub
// Description: Stub for gaBreakOnNextFrame
// Author:      Sigal Algranaty
// Date:        8/4/2010
// ---------------------------------------------------------------------------
void gaBreakOnNextFrameStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    retVal = gaBreakOnNextFrameImpl();

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaBreakInMonitoredFunctionCallStub
// Description: Stub for gaBreakPointInMonitoredFunctionCall
// Author:      Uri Shomroni
// Date:        27/10/2010
// ---------------------------------------------------------------------------
void gaBreakInMonitoredFunctionCallStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    bool replacedWithStepOver = false;
    retVal = gaBreakInMonitoredFunctionCallImpl(replacedWithStepOver);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the changed value:
        apiSocket << replacedWithStepOver;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaClearAllStepFlagsStub
// Description: Stub for gaClearAllStepFlags
// Author:      Uri Shomroni
// Date:        29/2/2016
// ---------------------------------------------------------------------------
void gaClearAllStepFlagsStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    retVal = gaClearAllStepFlagsImpl();

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetBreakReasonStub
// Description: Stub function for gaGetBreakReason
// Author:      Yaki Tebeka
// Date:        20/6/2004
// ---------------------------------------------------------------------------
void gaGetBreakReasonStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    apBreakReason breakReason = AP_FOREIGN_BREAK_HIT;
    retVal = gaGetBreakReasonImpl(breakReason);

    // Return the return value:
    apiSocket << retVal;

    // Return the break reason:
    gtInt32 breakReasonAsInt32 = (gtInt32)breakReason;
    apiSocket << breakReasonAsInt32;
}

// ---------------------------------------------------------------------------
// Name:        gaGetBreakpointTriggeringContextIdStub
// Description: Stub function for gaGetBreakpointTriggeringContextId
// Author:      Sigal Algranaty
// Date:        25/11/2009
// ---------------------------------------------------------------------------
void gaGetBreakpointTriggeringContextIdStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    apContextID contextId;
    retVal = gaGetBreakpointTriggeringContextIdImpl(contextId);
    apiSocket << retVal;

    if (retVal)
    {
        contextId.writeSelfIntoChannel(apiSocket);
    }
}


// ---------------------------------------------------------------------------
// Name:        gaCreateEventForwardingTCPConnectionStub
// Description: Stub function for gaCreateEventForwardingTCPConnection
// Author:      Uri Shomroni
// Date:        29/10/2009
// ---------------------------------------------------------------------------
void gaCreateEventForwardingTCPConnectionStub(osSocket& apiSocket)
{
    // Read the parameters:
    gtString hostName;
    apiSocket >> hostName;

    gtUInt16 portNumberAsUInt16 = 0;
    apiSocket >> portNumberAsUInt16;

    // Call the function implementation:
    osPortAddress portAddress(hostName, (unsigned short)portNumberAsUInt16);
    bool retVal = gaCreateEventForwardingTCPConnectionImpl(portAddress);

    // Write the return value:
    apiSocket << retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaCreateEventForwardingPipeConnectionStub
// Description: Stub function for gaCreateEventForwardingPipeConnection
// Author:      Yaki Tebeka
// Date:        16/12/2009
// ---------------------------------------------------------------------------
void gaCreateEventForwardingPipeConnectionStub(osSocket& apiSocket)
{
    // Read the parameters:
    gtString eventsPipeName;
    apiSocket >> eventsPipeName;

    // Call the function implementation:
    bool retVal = gaCreateEventForwardingPipeConnectionImpl(eventsPipeName);

    // Write the return value:
    apiSocket << retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfRegisteredAllocatedObjectsStub
// Description: Stub for gaGetAmountOfRegisteredAllocatedObjects
// ---------------------------------------------------------------------------
void gaGetAmountOfRegisteredAllocatedObjectsStub(osSocket& apiSocket)
{
    unsigned int amountOfAllocatedObjects = 0;
    bool retVal = gaGetAmountOfRegisteredAllocatedObjectsImpl(amountOfAllocatedObjects);

    // Send the RetVal:
    apiSocket << retVal;

    if (retVal)
    {
        // Send the result:
        apiSocket << (gtUInt32)amountOfAllocatedObjects;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetAllocatedObjectCreationStackStub
// Description: Stub for gaGetAllocatedObjectCreationStack()
// ---------------------------------------------------------------------------
void gaGetAllocatedObjectCreationStackStub(osSocket& apiSocket)
{
    // Get the object Id:
    gtInt32 allocatedObjectIdAsInt32 = -1;
    apiSocket >> allocatedObjectIdAsInt32;

    // Get the return value:
    osCallStack callsStack;
    bool retVal = gaGetAllocatedObjectCreationStackImpl((int)allocatedObjectIdAsInt32, callsStack);

    apiSocket << retVal;

    if (retVal)
    {
        callsStack.writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaCollectAllocatedObjectsCreationCallsStacksStub
// Description: Stub function for gaCollectAllocatedObjectsCreationCallsStacks
// Author:      Uri Shomroni
// Date:        3/2/2009
// ---------------------------------------------------------------------------
void gaCollectAllocatedObjectsCreationCallsStacksStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    bool collectCreationStacks = true;
    apiSocket >> collectCreationStacks;

    // Call the function implementation:
    bool retVal = gaCollectAllocatedObjectsCreationCallsStacksImpl(collectCreationStacks);

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaReadFileStub
// Description: Stub function for gaReadFile
// Author:      Uri Shomroni
// Date:        5/11/2009
// ---------------------------------------------------------------------------
void gaReadFileStub(osSocket& apiSocket)
{
    osFilePath filePath;
    osRawMemoryBuffer memoryBuffer;
    bool retVal = false;

    // Read the file path:
    bool rcPath = filePath.readSelfFromChannel(apiSocket);

    GT_IF_WITH_ASSERT(rcPath)
    {
        retVal = gaReadFileImpl(filePath, memoryBuffer);
    }

    // Write the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Write the file contents:
        bool rcBuffer = memoryBuffer.writeSelfIntoChannel(apiSocket);
        GT_ASSERT(rcBuffer);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaWriteFileStub
// Description: Stub function for gaWriteFile
// Author:      Uri Shomroni
// Date:        5/11/2009
// ---------------------------------------------------------------------------
void gaWriteFileStub(osSocket& apiSocket)
{
    osFilePath filePath;
    osRawMemoryBuffer memoryBuffer;
    bool retVal = false;

    bool rcPath = filePath.readSelfFromChannel(apiSocket);
    GT_ASSERT(rcPath);

    bool rcBuffer = memoryBuffer.readSelfFromChannel(apiSocket);
    GT_ASSERT(rcBuffer);

    // Only attempt this if we got everything properly:
    if (rcPath && rcBuffer)
    {
        retVal = gaWriteFileImpl(filePath, memoryBuffer);
    }

    // Write the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetSlowMotionDelayStub
// Description: Stub function for gaSetSlowMotionDelay.
// Author:      Yaki Tebeka
// Date:        9/11/2004
// ---------------------------------------------------------------------------
void gaSetSlowMotionDelayStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Get the delay time:
    gtInt32 delayTimeUnitsAsInt32 = 0;
    apiSocket >> delayTimeUnitsAsInt32;

    // Call the function implementation:
    retVal = gaSetSlowMotionDelayImpl((int)delayTimeUnitsAsInt32);

    // Return the return value:
    apiSocket << retVal;
}



// ---------------------------------------------------------------------------
// Name:        gaStartMonitoredFunctionsCallsLogFileRecordingStub
// Description: Stub function for gaStartMonitoredFunctionsLogFileRecording
// Author:      Yaki Tebeka
// Date:        17/8/2004
// ---------------------------------------------------------------------------
void gaStartMonitoredFunctionsCallsLogFileRecordingStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    retVal = gaStartMonitoredFunctionsCallsLogFileRecordingImpl();

    // Return the return value:
    apiSocket << retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaStopMonitoredFunctionsCallsLogFileRecordingStub
// Description: Stub function for gaStopMonitoredFunctionsLogFileRecording
// Author:      Yaki Tebeka
// Date:        17/8/2004
// ---------------------------------------------------------------------------
void gaStopMonitoredFunctionsCallsLogFileRecordingStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    retVal = gaStopMonitoredFunctionsCallsLogFileRecordingImpl();

    // Return the return value:
    apiSocket << retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaIsMonitoredFunctionsCallsLogFileRecordingActiveStub
// Description: Stub function for gaIsMonitoredFunctionsCallsLogFileRecordingActive
// Author:      Yaki Tebeka
// Date:        19/8/2004
// ---------------------------------------------------------------------------
void gaIsMonitoredFunctionsCallsLogFileRecordingActiveStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    bool isActive = false;
    retVal = gaIsMonitoredFunctionsCallsLogFileRecordingActiveImpl(isActive);

    // Return the return value:
    apiSocket << retVal;

    // Return output parameters:
    apiSocket << isActive;
}



// ---------------------------------------------------------------------------
// Name:        gaEnableImagesDataLoggingStub
// Description: Stub function for gaEnableTexturesImageDataLogging
// Author:      Yaki Tebeka
// Date:        15/3/2005
// ---------------------------------------------------------------------------
void gaEnableImagesDataLoggingStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    bool isImagesDataLogged = false;
    apiSocket >> isImagesDataLogged;

    // Call the function implementation:
    bool retVal = gaEnableImagesDataLoggingImpl(isImagesDataLogged);

    // Return the return value:
    apiSocket << retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaSetFloatParametersDisplayPrecisionStub
// Description: Sets floating parameters display precision.
// Author:      Yaki Tebeka
// Date:        24/9/2007
// ---------------------------------------------------------------------------
void gaSetFloatParametersDisplayPrecisionStub(osSocket& apiSocket)
{
    bool retVal = true;

    // Get the float parameters display precision:
    gtUInt32 maxSignificatDigitsAmountAsUInt32 = 8;
    apiSocket >> maxSignificatDigitsAmountAsUInt32;

    // Set the infrastructure float parameters display precision:
    apSetFloatParamsDisplayPrecision((unsigned int)maxSignificatDigitsAmountAsUInt32);

    // Return success value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaFlushLogFileAfterEachFunctionCallStub
// Description: Stub function for gaFlushLogFileAfterEachFunctionCall
// Author:      Avi Shapira
// Date:        7/4/2005
// ---------------------------------------------------------------------------
void gaFlushLogFileAfterEachFunctionCallStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Get the "Flush The Log File After Each OpenGL Function Call" state status:
    bool flushAfterEachFunctionCall = false;
    apiSocket >> flushAfterEachFunctionCall;

    // Call the function implementation:
    retVal = gaFlushLogFileAfterEachFunctionCallImpl(flushAfterEachFunctionCall);

    // Return success value:
    apiSocket << retVal;
}

/////////////////////////////////////////////////////////////////////////////
// \brief	Lock driver threads before process suspesion
// \param       apiSocket a spy socket instance. Not use at now		
// \author      Vadim Entov
// \date        11/05/2016
void gaLockDriverThreadsStub(osSocket& apiSocket)
{
    bool retVal = true;

#ifdef SU_USE_SINGLE_WRITE_MULTIPLE_READ_SYNC
    suSWMRInstance::UniqueLock();
#endif

    apiSocket << retVal;
}

/////////////////////////////////////////////////////////////////////////////
// \brief	UnLock driver threads before process resume
// \param       apiSocket a spy socket instance. Not use at now		
// \author      Vadim Entov
// \date        11/05/2016
void gaUnLockDriverThreadsStub(osSocket& apiSocket)
{
    bool retVal = true;

#ifdef SU_USE_SINGLE_WRITE_MULTIPLE_READ_SYNC
    suSWMRInstance::UniqueUnLock();
#endif

    apiSocket << retVal;
}


