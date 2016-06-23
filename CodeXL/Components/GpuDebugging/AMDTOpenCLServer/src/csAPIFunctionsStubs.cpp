//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csAPIFunctionsStubs.cpp
///
//==================================================================================

//------------------------------ csAPIFunctionsStubs.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAPIClasses/Include/apAPIFunctionId.h>
#include <AMDTAPIClasses/Include/apCounterID.h>
#include <AMDTAPIClasses/Include/apCLBuffer.h>
#include <AMDTAPIClasses/Include/apCLSubBuffer.h>
#include <AMDTAPIClasses/Include/apCLCommandQueue.h>
#include <AMDTAPIClasses/Include/apStatistics.h>
#include <AMDTAPIClasses/Include/apCLContext.h>
#include <AMDTAPIClasses/Include/apCLDevice.h>
#include <AMDTAPIClasses/Include/apCLEvent.h>
#include <AMDTAPIClasses/Include/apCLKernel.h>
#include <AMDTAPIClasses/Include/apCLObjectID.h>
#include <AMDTAPIClasses/Include/apCLPipe.h>
#include <AMDTAPIClasses/Include/apCLProgram.h>
#include <AMDTAPIClasses/Include/apCLSampler.h>
#include <AMDTAPIClasses/Include/apCLImage.h>
#include <AMDTAPIClasses/Include/apExpression.h>
#include <AMDTAPIClasses/Include/apKernelDebuggingCommand.h>
#include <AMDTAPIClasses/Include/apKernelSourceCodeBreakpoint.h>
#include <AMDTServerUtilities/Include/suAPIMainLoop.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>

// Local:
#include <src/csAPIFunctionsImplementations.h>
#include <src/csAPIFunctionsStubs.h>
#include <src/csStringConstants.h>


// ---------------------------------------------------------------------------
// Name:        csRegisterAPIStubFunctions
// Description: Registers OpenCL Server module stub functions.
// Author:      Yaki Tebeka
// Date:        1/12/2009
// ---------------------------------------------------------------------------
void csRegisterAPIStubFunctions()
{
    suRegisterAPIFunctionStub(GA_FID_gaOpenCLServerInitializationEnded, &gaOpenCLServerInitializationEndedStub);
    suRegisterAPIFunctionStub(GA_FID_gaUpdateOpenCLContextDataSnapshot, &gaUpdateOpenCLContextDataSnapshotStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfOpenCLFunctionCalls, &gaGetAmountOfOpenCLFunctionCallsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetOpenCLFunctionCall, &gaGetOpenCLFunctionCallStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetLastOpenCLFunctionCall, &gaGetLastOpenCLFunctionCallStub);
    suRegisterAPIFunctionStub(GA_FID_gaFindOpenCLFunctionCall, &gaFindOpenCLFunctionCallStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetOpenCLHandleObjectDetails, &gaGetOpenCLHandleObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfOpenCLContexts, &gaGetAmountOfOpenCLContextsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfOpenCLProgramObjects, &gaGetAmountOfOpenCLProgramObjectsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfOpenCLBufferObjects, &gaGetAmountOfOpenCLBufferObjectsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfOpenCLImageObjects, &gaGetAmountOfOpenCLImageObjectsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfOpenCLPipeObjects, &gaGetAmountOfOpenCLPipeObjectsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetOpenCLProgramObjectDetails, &gaGetOpenCLProgramObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetOpenCLProgramCode, &gaSetOpenCLProgramCodeStub);
    suRegisterAPIFunctionStub(GA_FID_gaBuildOpenCLProgram, &gaBuildOpenCLProgramStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetOpenCLProgramHandleFromSourceFilePath, &gaGetOpenCLProgramHandleFromSourceFilePathStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetOpenCLKernelObjectDetails, &gaGetOpenCLKernelObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetKernelDebuggingLocation, &gaGetKernelDebuggingLocationStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetCurrentlyDebuggedKernelDetails, &gaGetCurrentlyDebuggedKernelDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetCurrentlyDebuggedKernelCallStack, &gaGetCurrentlyDebuggedKernelCallStackStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetKernelDebuggingCommand, &gaSetKernelDebuggingCommandStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetKernelSteppingWorkItem, &gaSetKernelSteppingWorkItemStub);
    suRegisterAPIFunctionStub(GA_FID_gaIsWorkItemValid, &gaIsWorkItemValidStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetFirstValidWorkItem, &gaGetFirstValidWorkItemStub);
    suRegisterAPIFunctionStub(GA_FID_gaCanGetKernelVariableValue, &gaCanGetKernelVariableValueStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetKernelDebuggingExpressionValue, &gaGetKernelDebuggingExpressionValueStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetKernelDebuggingAvailableVariables, &gaGetKernelDebuggingAvailableVariablesStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetKernelDebuggingAmountOfActiveWavefronts, &gaGetKernelDebuggingAmountOfActiveWavefrontsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetKernelDebuggingActiveWavefrontID, &gaGetKernelDebuggingActiveWavefrontIDStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetKernelDebuggingWavefrontIndex, &gaGetKernelDebuggingWavefrontIndexStub);
    suRegisterAPIFunctionStub(GA_FID_gaUpdateKernelVariableValueRawData, &gaUpdateKernelVariableValueRawDataStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetKernelSourceCodeBreakpointResolution, &gaGetKernelSourceCodeBreakpointResolutionStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetKernelDebuggingEnable, &gaSetKernelDebuggingEnableStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetMultipleKernelDebugDispatchMode, &gaSetMultipleKernelDebugDispatchModeStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetOpenCLDeviceObjectDetails, &gaGetOpenCLDeviceObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetOpenCLPlatformAPIID, &gaGetOpenCLPlatformAPIIDStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetOpenCLBufferObjectDetails, &gaGetOpenCLBufferObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetOpenCLSubBufferObjectDetails, &gaGetOpenCLSubBufferObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetOpenCLImageObjectDetails, &gaGetOpenCLImageObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetOpenCLPipeObjectDetails, &gaGetOpenCLPipeObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaUpdateOpenCLBufferRawData, &gaUpdateOpenCLBufferRawDataStub);
    suRegisterAPIFunctionStub(GA_FID_gaUpdateOpenCLSubBufferRawData, &gaUpdateOpenCLSubBufferRawDataStub);
    suRegisterAPIFunctionStub(GA_FID_gaUpdateOpenCLImageRawData, &gaUpdateOpenCLImageRawDataStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfCommandQueues, &gaGetAmountOfCommandQueuesStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetCommandQueueDetails, &gaGetCommandQueueDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfCommandsInQueue, &gaGetAmountOfCommandsInQueueStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfEventsInQueue, &gaGetAmountOfEventsInQueueStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetEnqueuedCommandDetails, &gaGetEnqueuedCommandDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfOpenCLSamplers, &gaGetAmountOfOpenCLSamplersStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetOpenCLSamplerObjectDetails, &gaGetOpenCLSamplerObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfOpenCLEvents, &gaGetAmountOfOpenCLEventsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetOpenCLEventObjectDetails, &gaGetOpenCLEventObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetOpenCLContextDetails, &gaGetOpenCLContextDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetCLBufferDisplayProperties, &gaSetCLBufferDisplayPropertiesStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetCLSubBufferDisplayProperties, &gaSetCLSubBufferDisplayPropertiesStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAMDOpenCLPerformanceCountersValues, &gaGetAMDOpenCLPerformanceCountersValuesStub);
    suRegisterAPIFunctionStub(GA_FID_gaActivateAMDOpenCLPerformanceCounters, &gaActivateAMDOpenCLPerformanceCountersStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetOpenCLQueuePerformanceCountersValues, &gaGetOpenCLQueuePerformanceCountersValuesStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetCLContextLogFilePath, &gaGetCLContextLogFilePathStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetCurrentOpenCLStatistics, &gaGetCurrentOpenCLStatisticsStub);
    suRegisterAPIFunctionStub(GA_FID_gaClearOpenCLFunctionCallsStatistics, &gaClearOpenCLFunctionCallsStatisticsStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetOpenCLOperationExecution, &gaSetOpenCLOperationExecutionStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetKernelSourceFilePath, &gaSetKernelSourceFilePathStub);
}


// ---------------------------------------------------------------------------
// Name:        csHandleAPIInitializationCalls
// Description: Handles the API functions that initialize the OpenCL Server.
// Author:      Yaki Tebeka
// Date:        11/1/2010
// ---------------------------------------------------------------------------
void csHandleAPIInitializationCalls()
{
    OS_OUTPUT_DEBUG_LOG(CS_STR_DebugLog_startedHandlingAPIInitCalls, OS_DEBUG_LOG_DEBUG);

    // If the API thread didn't start running yet, the main thread should handle the OpenCL Server's API initialization calls:
    bool shouldMainThreadHandleAPIInitializationCalls = suMainThreadStarsInitializingAPIConnection(AP_OPENCL_API_CONNECTION);

    if (shouldMainThreadHandleAPIInitializationCalls)
    {
        // Get the API socket:
        osSocket* pAPISocket = suSpiesAPISocket();
        GT_IF_WITH_ASSERT(pAPISocket != NULL)
        {
            // Handle the initialization calls using the main thread:
            suAPIMainLoop(*pAPISocket);
        }

        // Tell the API thread that the API calls initialization handling ended:
        suMainThreadEndedInitializingAPIConnection(AP_OPENCL_API_CONNECTION);
    }
    else
    {
        // The API thread is running and listening to API calls, therefore, the API thread will handle the OpenCL API initialization calls.
        // All we need to do is to wait until the OpenCL initialization calls are done:
        OS_OUTPUT_DEBUG_LOG(CS_STR_DebugLog_MainThreadWaitsForAPIThreadToHandleOCLAPIInitialization, OS_DEBUG_LOG_DEBUG);

        while (!(suIsAPIConnectionInitialized(AP_OPENCL_API_CONNECTION)))
        {
            osSleep(20);
        }

        OS_OUTPUT_DEBUG_LOG(CS_STR_DebugLog_MainThreadFinishedWaitingForAPIThreadToHandleOCLAPIInitialization, OS_DEBUG_LOG_DEBUG);
    }

    OS_OUTPUT_DEBUG_LOG(CS_STR_DebugLog_endedHandlingAPIInitCalls, OS_DEBUG_LOG_DEBUG);
}


// ---------------------------------------------------------------------------
// Name:        gaOpenCLServerInitializationEndedStub
// Description: Stub function for gaOpenCLServerInitializationEnded
// Author:      Yaki Tebeka
// Date:        28/3/2010
// ---------------------------------------------------------------------------
void gaOpenCLServerInitializationEndedStub(osSocket& apiSocket)
{
    (void)(apiSocket); // unused
    // Nothing to be done.
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfOpenCLFunctionCallsStub
// Description: Stub function for gaGetAmountOfOpenCLFunctionCalls
// Author:      Sigal Algranaty
// Date:        12/11/2009
// ---------------------------------------------------------------------------
void gaGetAmountOfOpenCLFunctionCallsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    int amountOfFunctionCalls = 0;
    bool rc = gaGetAmountOfOpenCLFunctionCallsImpl((int)contextIdAsInt32, amountOfFunctionCalls);

    // Return the return value:
    apiSocket << rc;

    if (rc)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtInt32)amountOfFunctionCalls;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLFunctionCallStub
// Description: Stub function for gaGetOpenCLFunctionCall
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        12/11/2009
// ---------------------------------------------------------------------------
void gaGetOpenCLFunctionCallStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 callIndexAsInt32 = -1;
    apiSocket >> callIndexAsInt32;

    // Call the function implementation:
    gtAutoPtr<apFunctionCall> aptrFunctionCall;
    bool rc = gaGetOpenCLFunctionCallImpl((int)contextIdAsInt32, (int)callIndexAsInt32, aptrFunctionCall);

    // Return the return value:
    apiSocket << rc;

    if (rc)
    {
        // Return the arguments that should be returned:
        osTransferableObject* pFunctionCallAsTransferableObj = aptrFunctionCall.pointedObject();
        apiSocket << *pFunctionCallAsTransferableObj;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetLastOpenCLFunctionCallStub
// Description: Stub function for gaGetLastOpenCLFunctionCall
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        12/11/2009
// ---------------------------------------------------------------------------
void gaGetLastOpenCLFunctionCallStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    gtAutoPtr<apFunctionCall> aptrFunctionCall;
    bool retVal = gaGetLastOpenCLFunctionCallImpl((int)contextIdAsInt32, aptrFunctionCall);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        osTransferableObject* pFunctionCallAsTransferableObj = aptrFunctionCall.pointedObject();
        apiSocket << *pFunctionCallAsTransferableObj;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaFindOpenCLFunctionCallStub
// Description: Stub function for gaFindOpenCLFunctionCall
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        12/11/2009
// ---------------------------------------------------------------------------
void gaFindOpenCLFunctionCallStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 searchDirectionAsInt32 = AP_SEARCH_INDICES_DOWN;
    apiSocket >> searchDirectionAsInt32;
    apSearchDirection searchDirection = (apSearchDirection)searchDirectionAsInt32;

    gtInt32 searchStartIndexAsInt32 = 0;
    apiSocket >> searchStartIndexAsInt32;

    // Get the search string:
    gtString searchedString;
    apiSocket >> searchedString;

    // Get case sensitive:
    bool isCaseSensitiveSearch = false;
    apiSocket >> isCaseSensitiveSearch;

    // Call the function implementation:
    int foundIndex = -1;
    bool rc = gaFindOpenCLFunctionCallImpl((int)contextIdAsInt32, searchDirection, (int)searchStartIndexAsInt32, searchedString, isCaseSensitiveSearch, foundIndex);

    // Send the success value:
    apiSocket << rc;

    if (rc)
    {
        // Send the found index:
        apiSocket << (gtInt32)foundIndex;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLHandleObjectDetailsStub
// Description: Stub function for gaGetOpenCLHandleObjectDetails
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        8/12/2009
// ---------------------------------------------------------------------------
void gaGetOpenCLHandleObjectDetailsStub(osSocket& apiSocket)
{
    gtUInt64 handleAsUint64 = 0;
    apiSocket >> handleAsUint64;

    // Call the function implementation:
    const apCLObjectID* pCLOjbectIDDetails = NULL;
    bool rc = gaGetOpenCLHandleObjectDetailsImpl((oaCLHandle)handleAsUint64, pCLOjbectIDDetails);

    if (pCLOjbectIDDetails == NULL)
    {
        rc = false;
    }

    // Send the success value:
    apiSocket << rc;

    if (rc)
    {
        // Send the found index:
        pCLOjbectIDDetails->writeSelfIntoChannel(apiSocket);
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfOpenCLContextsStub
// Description: Stub function for gaGetAmountOfOpenCLContexts
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/11/2009
// ---------------------------------------------------------------------------
void gaGetAmountOfOpenCLContextsStub(osSocket& apiSocket)
{
    // Call the function implementation:
    int amountOfOpenCLContexts = -1;
    bool rc = gaGetAmountOfOpenCLContextsImpl(amountOfOpenCLContexts);

    // Send the success value:
    apiSocket << rc;

    if (rc)
    {
        // Send the contexts amount:
        apiSocket << (gtInt32)amountOfOpenCLContexts;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaUpdateOpenCLContextDataSnapshotStub
// Description: Stub function for gaUpdateOpenCLContextDataSnapshot
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        24/11/2009
// ---------------------------------------------------------------------------
void gaUpdateOpenCLContextDataSnapshotStub(osSocket& apiSocket)
{
    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(CS_STR_DebugLog_UpdateOpenCLContextDataSnapshotStart, OS_DEBUG_LOG_DEBUG);

    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    bool retVal = gaUpdateOpenCLContextDataSnapshotImpl((int)contextIdAsInt32);

    // Return the return value:
    apiSocket << retVal;

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(CS_STR_DebugLog_UpdateOpenCLContextDataSnapshotEnd, OS_DEBUG_LOG_DEBUG);
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfOpenCLProgramObjectsStub
// Description: Stub function for gaGetAmountOfOpenCLProgramObjects
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        17/11/2009
// ---------------------------------------------------------------------------
void gaGetAmountOfOpenCLProgramObjectsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    int amountOfPrograms = -1;
    bool rc = gaGetAmountOfOpenCLProgramObjectsImpl((int)contextIdAsInt32, amountOfPrograms);

    // Send the success value:
    apiSocket << rc;

    if (rc)
    {
        // Send the programs amount:
        apiSocket << (gtInt32)amountOfPrograms;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfOpenCLProgramObjectsStub
// Description: Stub function for gaGetAmountOfOpenCLProgramObjects
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        17/11/2009
// ---------------------------------------------------------------------------
void gaGetOpenCLProgramObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 programIdAsInt32 = -1;
    apiSocket >> programIdAsInt32;

    // Call the function implementation:
    const apCLProgram* pCLProgramDetails = NULL;
    bool retVal = gaGetOpenCLProgramObjectDetailsImpl((int)contextIdAsInt32, (int)programIdAsInt32, pCLProgramDetails);

    // Return the return value:
    retVal = retVal && (pCLProgramDetails != NULL);
    apiSocket << retVal;

    if (retVal)
    {
        pCLProgramDetails->writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaSetOpenCLProgramCodeStub
// Description: Stub function for gaSetOpenCLProgramCode
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
void gaSetOpenCLProgramCodeStub(osSocket& apiSocket)
{
    // Read the program handle:
    gtUInt64 programHandleAsUInt64 = (gtUInt64)OA_CL_NULL_HANDLE;
    apiSocket >> programHandleAsUInt64;
    oaCLProgramHandle programHandle = (oaCLProgramHandle)programHandleAsUInt64;

    // Read the source path:
    osFilePath newSourcePath;
    newSourcePath.readSelfFromChannel(apiSocket);

    // Call the function implementation:
    bool retVal = gaSetOpenCLProgramCodeImpl(programHandle, newSourcePath);

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaBuildOpenCLProgramStub
// Description: Stub function for gaBuildOpenCLProgram
// Author:      Uri Shomroni
// Date:        18/1/2010
// ---------------------------------------------------------------------------
void gaBuildOpenCLProgramStub(osSocket& apiSocket)
{
    // Read the program handle:
    gtUInt64 programHandleAsUInt64 = (gtUInt64)OA_CL_NULL_HANDLE;
    apiSocket >> programHandleAsUInt64;
    oaCLProgramHandle programHandle = (oaCLProgramHandle)programHandleAsUInt64;

    // Call the function implementation:
    apCLProgram* pFailedProgramData = NULL;
    bool retVal = gaBuildOpenCLProgramImpl(programHandle, pFailedProgramData);

    // Return the return value:
    apiSocket << retVal;

    // Return whether or not we have the failure data:
    bool isFailedProgramDataValid = (pFailedProgramData != NULL);
    apiSocket << isFailedProgramDataValid;

    // If we have the data, write it, then dispose of it:
    if (isFailedProgramDataValid)
    {
        pFailedProgramData->writeSelfIntoChannel(apiSocket);
        delete pFailedProgramData;
        pFailedProgramData = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLProgramHandleFromSourceFilePathStub
// Description: Stub function for gaGetOpenCLProgramHandleFromSourceFilePath
// Author:      Uri Shomroni
// Date:        9/11/2010
// ---------------------------------------------------------------------------
void gaGetOpenCLProgramHandleFromSourceFilePathStub(osSocket& apiSocket)
{
    // Read the source file path:
    osFilePath sourceFilePath;
    sourceFilePath.readSelfFromChannel(apiSocket);

    osFilePath newTempSourceFilePath;

    // Call the function implementation:
    oaCLProgramHandle programHandle = OA_CL_NULL_HANDLE;
    bool retVal = gaGetOpenCLProgramHandleFromSourceFilePathImpl(sourceFilePath, newTempSourceFilePath, programHandle);

    // Return the return value:
    apiSocket << retVal;

    // Return the handle:
    apiSocket << (gtUInt64)programHandle;

    // Write the temporary source code file path into the channel:
    newTempSourceFilePath.writeSelfIntoChannel(apiSocket);
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLKernelObjectDetailsStub
// Description: Stub function for gaGetOpenCLKernelObjectDetails
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        22/11/2009
// ---------------------------------------------------------------------------
void gaGetOpenCLKernelObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtUInt64 kernelHandleAsUint64 = 0;
    apiSocket >> kernelHandleAsUint64;

    // Call the function implementation:
    oaCLKernelHandle kernelCLHandle = (oaCLKernelHandle)kernelHandleAsUint64;
    const apCLKernel* pCLKernelDetails = NULL;
    bool retVal = gaGetOpenCLKernelObjectDetailsImpl(kernelCLHandle, pCLKernelDetails);

    // Return the return value:
    retVal = retVal && (pCLKernelDetails != NULL);
    apiSocket << retVal;

    if (retVal)
    {
        pCLKernelDetails->writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetKernelDebuggingLocationStub
// Description: Stub function for gaGetKernelDebuggingLocation
// Author:      Uri Shomroni
// Date:        9/11/2010
// ---------------------------------------------------------------------------
void gaGetKernelDebuggingLocationStub(osSocket& apiSocket)
{
    // Call the function implementation:
    oaCLProgramHandle debuggedProgramHandle = OA_CL_NULL_HANDLE;
    int currentLineNumber = -1;
    bool retVal = gaGetKernelDebuggingLocationImpl(debuggedProgramHandle, currentLineNumber);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        apiSocket << (gtUInt64)debuggedProgramHandle;
        apiSocket << (gtInt32)currentLineNumber;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetCurrentlyDebuggedKernelDetailsStub
// Description: Stub function for gaGetCurrentlyDebuggedKernelDetails
// Author:      Uri Shomroni
// Date:        19/12/2010
// ---------------------------------------------------------------------------
void gaGetCurrentlyDebuggedKernelDetailsStub(osSocket& apiSocket)
{
    // Call the function implementation:
    const apCLKernel* pKernelDetails;
    bool retVal = gaGetCurrentlyDebuggedKernelDetailsImpl(pKernelDetails);
    retVal = retVal & (pKernelDetails != NULL);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Write the kernel details:
        pKernelDetails->writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetCurrentlyDebuggedKernelCallStackStub
// Description: Stub function for gaGetCurrentlyDebuggedKernelCallStack
// Author:      Uri Shomroni
// Date:        2/11/2010
// ---------------------------------------------------------------------------
void gaGetCurrentlyDebuggedKernelCallStackStub(osSocket& apiSocket)
{
    // Get the parameters:
    gtInt32 coordinateXAsInt32 = 0;
    apiSocket >> coordinateXAsInt32;
    gtInt32 coordinateYAsInt32 = 0;
    apiSocket >> coordinateYAsInt32;
    gtInt32 coordinateZAsInt32 = 0;
    apiSocket >> coordinateZAsInt32;

    int coordinate[3] = {(int)coordinateXAsInt32, (int)coordinateYAsInt32, (int)coordinateZAsInt32};

    // Call the function implementation:
    osCallStack kernelStack;
    bool retVal = gaGetCurrentlyDebuggedKernelCallStackImpl(coordinate, kernelStack);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Write the call stack:
        kernelStack.writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaSetKernelDebuggingCommandStub
// Description: Stub function for gaSetKernelDebuggingCommand
// Author:      Uri Shomroni
// Date:        2/11/2010
// ---------------------------------------------------------------------------
void gaSetKernelDebuggingCommandStub(osSocket& apiSocket)
{
    // Get the parameter:
    gtUInt32 commandAsUInt32 = 0;
    apiSocket >> commandAsUInt32;

    // Call the function implementation:
    bool retVal = gaSetKernelDebuggingCommandImpl((apKernelDebuggingCommand)commandAsUInt32);

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetKernelSteppingWorkItemStub
// Description: Stub function for gaSetKernelSteppingWorkItem
// Author:      Uri Shomroni
// Date:        16/2/2011
// ---------------------------------------------------------------------------
void gaSetKernelSteppingWorkItemStub(osSocket& apiSocket)
{
    // Get the parameters:
    gtInt32 coordinateXAsInt32 = 0;
    apiSocket >> coordinateXAsInt32;
    gtInt32 coordinateYAsInt32 = 0;
    apiSocket >> coordinateYAsInt32;
    gtInt32 coordinateZAsInt32 = 0;
    apiSocket >> coordinateZAsInt32;

    // Call the function implementation:
    int coordinate[3] = {(int)coordinateXAsInt32, (int)coordinateYAsInt32, (int)coordinateZAsInt32};
    bool retVal = gaSetKernelSteppingWorkItemImpl(coordinate);

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaIsWorkItemValidStub
// Description: Stub function for gaIsWorkItemValid
// Author:      Uri Shomroni
// Date:        14/11/2011
// ---------------------------------------------------------------------------
void gaIsWorkItemValidStub(osSocket& apiSocket)
{
    // Get the parameters:
    gtInt32 coordinateXAsInt32 = 0;
    apiSocket >> coordinateXAsInt32;
    gtInt32 coordinateYAsInt32 = 0;
    apiSocket >> coordinateYAsInt32;
    gtInt32 coordinateZAsInt32 = 0;
    apiSocket >> coordinateZAsInt32;

    // Call the function implementation:
    int coordinate[3] = {(int)coordinateXAsInt32, (int)coordinateYAsInt32, (int)coordinateZAsInt32};
    bool retVal = gaIsWorkItemValidImpl(coordinate);

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetFirstValidWorkItemStub
// Description: Stub function for gaGetFirstValidWorkItem
// Author:      Uri Shomroni
// Date:        14/11/2011
// ---------------------------------------------------------------------------
void gaGetFirstValidWorkItemStub(osSocket& apiSocket)
{
    // Get the parameters:
    gtInt32 wavefrontIndexAsInt32 = 0;
    apiSocket >> wavefrontIndexAsInt32;

    // Call the function implementation:
    int coordinate[3] = { -1, -1, -1};
    bool retVal = gaGetFirstValidWorkItemImpl((int)wavefrontIndexAsInt32, coordinate);

    // Return the return value:
    apiSocket << retVal;

    // Return the parameters:
    if (retVal)
    {
        apiSocket << (gtInt32)coordinate[0];
        apiSocket << (gtInt32)coordinate[1];
        apiSocket << (gtInt32)coordinate[2];
    }
}

// ---------------------------------------------------------------------------
// Name:        gaCanGetKernelVariableValueStub
// Description: Stub function for gaCanGetKernelVariableValue
// Author:      Uri Shomroni
// Date:        25/1/2011
// ---------------------------------------------------------------------------
void gaCanGetKernelVariableValueStub(osSocket& apiSocket)
{
    // Get the parameters:
    gtString variableName;
    apiSocket >> variableName;
    gtInt32 coordinateXAsInt32 = 0;
    apiSocket >> coordinateXAsInt32;
    gtInt32 coordinateYAsInt32 = 0;
    apiSocket >> coordinateYAsInt32;
    gtInt32 coordinateZAsInt32 = 0;
    apiSocket >> coordinateZAsInt32;

    int coordinate[3] = {(int)coordinateXAsInt32, (int)coordinateYAsInt32, (int)coordinateZAsInt32};

    // Call the function implementation:
    gtString variableValue;
    bool retVal = gaCanGetKernelVariableValueImpl(variableName, coordinate);

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetKernelDebuggingExpressionValueStub
// Description: Stub function for gaGetKernelDebuggingExpressionValue
// Author:      Uri Shomroni
// Date:        23/1/2011
// ---------------------------------------------------------------------------
void gaGetKernelDebuggingExpressionValueStub(osSocket& apiSocket)
{
    // Get the parameters:
    gtString variableName;
    apiSocket >> variableName;

    gtInt32 workItemXAsInt32 = -1;
    apiSocket >> workItemXAsInt32;
    gtInt32 workItemYAsInt32 = -1;
    apiSocket >> workItemYAsInt32;
    gtInt32 workItemZAsInt32 = -1;
    apiSocket >> workItemZAsInt32;
    int workItemCoordinates[3] = {(int)workItemXAsInt32, (int)workItemYAsInt32, (int)workItemZAsInt32};

    // Get the evaluation depth:
    gtInt32 evalDepth = 0;
    apiSocket >> evalDepth;

    // Call the function implementation:
    apExpression variableValue;
    bool retVal = gaGetKernelDebuggingExpressionValueImpl(variableName, workItemCoordinates, (int)evalDepth, variableValue);

    // Return the return value:
    apiSocket << retVal;

    // Return the variable Value:
    if (retVal)
    {
        retVal = variableValue.writeSelfIntoChannel(apiSocket);
        GT_ASSERT(retVal);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetKernelDebuggingAvailableVariablesStub
// Description: Stub function for gaGetKernelDebuggingAvailableVariables
// Author:      Uri Shomroni
// Date:        21/2/2011
// ---------------------------------------------------------------------------
void gaGetKernelDebuggingAvailableVariablesStub(osSocket& apiSocket)
{
    // Get the parameters:
    gtInt32 coordinateXAsInt32 = 0;
    apiSocket >> coordinateXAsInt32;
    gtInt32 coordinateYAsInt32 = 0;
    apiSocket >> coordinateYAsInt32;
    gtInt32 coordinateZAsInt32 = 0;
    apiSocket >> coordinateZAsInt32;
    gtInt32 evalDepthAsInt32 = 0;
    apiSocket >> evalDepthAsInt32;
    bool getLeaves = false;
    apiSocket >> getLeaves;
    gtInt32 stackFrameDepthAsInt32 = -1;
    apiSocket >> stackFrameDepthAsInt32;
    bool onlyNames = false;
    apiSocket >> onlyNames;

    int coordinate[3] = {(int)coordinateXAsInt32, (int)coordinateYAsInt32, (int)coordinateZAsInt32};

    // Call the function implementation:
    gtVector<apExpression> variables;
    bool retVal = gaGetKernelDebuggingAvailableVariablesImpl(coordinate, variables, (int)evalDepthAsInt32, getLeaves, (int)stackFrameDepthAsInt32, onlyNames);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Write the amount of variables:
        gtUInt32 amountOfVariables = (gtUInt32)variables.size();
        apiSocket << amountOfVariables;

        // Write the variable names:
        for (gtUInt32 i = 0; i < amountOfVariables; i++)
        {
            bool rcVar = variables[i].writeSelfIntoChannel(apiSocket);
            GT_ASSERT(rcVar);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetKernelDebuggingAmountOfActiveWavefrontsStub
// Description: Stub function for gaGetKernelDebuggingAmountOfActiveWavefronts
// Author:      Uri Shomroni
// Date:        4/4/2013
// ---------------------------------------------------------------------------
void gaGetKernelDebuggingAmountOfActiveWavefrontsStub(osSocket& apiSocket)
{
    // Call the function implementation:
    int amountOfWavefronts = -2;
    bool retVal = gaGetKernelDebuggingAmountOfActiveWavefrontsImpl(amountOfWavefronts);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Write the wavefront index:
        apiSocket << (gtInt32)amountOfWavefronts;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetKernelDebuggingActiveWavefrontIDStub
// Description: Stub function for gaGetKernelDebuggingActiveWavefrontID
// Author:      Uri Shomroni
// Date:        30/10/2013
// ---------------------------------------------------------------------------
void gaGetKernelDebuggingActiveWavefrontIDStub(osSocket& apiSocket)
{
    // Read the parameters:
    gtInt32 wavefrontIndexAsInt32 = -2;
    apiSocket >> wavefrontIndexAsInt32;

    int wavefrontIndex = (int)wavefrontIndexAsInt32;

    // Call the function implementation:
    int wavefrontId = -2;
    bool retVal = gaGetKernelDebuggingActiveWavefrontIDImpl(wavefrontIndex, wavefrontId);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Write the wavefront index:
        apiSocket << (gtInt32)wavefrontId;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetKernelDebuggingWavefrontIndexStub
// Description: Stub function for gaGetKernelDebuggingWavefrontIndex
// Author:      Uri Shomroni
// Date:        24/3/2013
// ---------------------------------------------------------------------------
void gaGetKernelDebuggingWavefrontIndexStub(osSocket& apiSocket)
{
    // Get the parameters:
    gtInt32 coordinateXAsInt32 = 0;
    apiSocket >> coordinateXAsInt32;
    gtInt32 coordinateYAsInt32 = 0;
    apiSocket >> coordinateYAsInt32;
    gtInt32 coordinateZAsInt32 = 0;
    apiSocket >> coordinateZAsInt32;

    int coordinate[3] = {(int)coordinateXAsInt32, (int)coordinateYAsInt32, (int)coordinateZAsInt32};

    // Call the function implementation:
    int wavefrontIndex = -2;
    bool retVal = gaGetKernelDebuggingWavefrontIndexImpl(coordinate, wavefrontIndex);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Write the wavefront index:
        apiSocket << (gtInt32)wavefrontIndex;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateKernelVariableValueRawDataStub
// Description: Stub function for gaUpdateKernelVariableValueRawData
// Author:      Uri Shomroni
// Date:        3/3/2011
// ---------------------------------------------------------------------------
void gaUpdateKernelVariableValueRawDataStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtString variableName;
    apiSocket >> variableName;

    // Call the function implementation:
    osFilePath variableRawDataFilePath;
    bool variableTypeSupported;
    bool retVal = gaUpdateKernelVariableValueRawDataImpl(variableName, variableTypeSupported, variableRawDataFilePath);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Write the file path:
        apiSocket << variableTypeSupported;
        variableRawDataFilePath.writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetKernelSourceCodeBreakpointResolutionStub
// Description: Stub function for gaGetKernelSourceCodeBreakpointResolution
// Author:      Uri Shomroni
// Date:        5/5/2011
// ---------------------------------------------------------------------------
void gaGetKernelSourceCodeBreakpointResolutionStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtUInt64 programHandleAsUInt64 = (gtUInt64)OA_CL_NULL_HANDLE;
    apiSocket >> programHandleAsUInt64;
    oaCLProgramHandle programHandle = (oaCLProgramHandle)programHandleAsUInt64;
    gtInt32 requestedLineNumberAsInt32 = -1;
    apiSocket >> requestedLineNumberAsInt32;
    int requestedLineNumber = (int)requestedLineNumberAsInt32;

    // Call the function implementation:
    int resolvedLineNumber = -1;
    bool retVal = gaGetKernelSourceCodeBreakpointResolutionImpl(programHandle, requestedLineNumber, resolvedLineNumber);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Write the file path:
        apiSocket << (gtInt32)resolvedLineNumber;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaSetKernelDebuggingEnableStub
// Description: stub for gaSetKernelDebuggingEnable
// Author:      Gilad Yarnitzky
// Date:        6/11/2011
// ---------------------------------------------------------------------------
void gaSetKernelDebuggingEnableStub(osSocket& apiSocket)
{
    bool kernelEnabled = false;
    apiSocket >> kernelEnabled;

    // Call the function implementation:
    bool retVal = gaSetKernelDebuggingEnableImpl(kernelEnabled);

    // Send success value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetMultipleKernelDebugDispatchModeStub
// Description: stub for gaSetMultipleKernelDebugDispatchMode
// Author:      Uri Shomroni
// Date:        5/7/2015
// ---------------------------------------------------------------------------
void gaSetMultipleKernelDebugDispatchModeStub(osSocket& apiSocket)
{
    gtInt32 modeAsInt32 = -1;
    apiSocket >> modeAsInt32;

    apMultipleKernelDebuggingDispatchMode mode = (apMultipleKernelDebuggingDispatchMode)modeAsInt32;

    // Call the function implementation:
    bool retVal = gaSetMultipleKernelDebugDispatchModeImpl(mode);

    // Send success value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLDeviceObjectDetailsStub
// Description: Stub function for gaGetOpenCLDeviceObjectDetails
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        29/11/2009
// ---------------------------------------------------------------------------
void gaGetOpenCLDeviceObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 deviceId = 0;
    apiSocket >> deviceId;

    // Call the function implementation:
    const apCLDevice* pCLDeviceDetails = NULL;
    bool retVal = gaGetOpenCLDeviceObjectDetailsImpl((int)deviceId, pCLDeviceDetails);

    // Return the return value:
    retVal = retVal && (pCLDeviceDetails != NULL);
    apiSocket << retVal;

    if (retVal)
    {
        pCLDeviceDetails->writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLPlatformAPIIDStub
// Description: Stub function for gaGetOpenCLPlatformName
// Arguments:   osSocket& apiSocket
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        11/4/2010
// ---------------------------------------------------------------------------
void gaGetOpenCLPlatformAPIIDStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtUInt64 platformId = 0;
    apiSocket >> platformId;

    // Call the function implementation:
    int platformName = 0;
    bool retVal = gaGetOpenCLPlatformAPIIDImpl((oaCLPlatformID)platformId, platformName);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        apiSocket << (gtInt32)platformName;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfOpenCLBufferObjectsStub
// Description: Stub function for gaGetAmountOfOpenCLBufferObjects
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        19/11/2009
// ---------------------------------------------------------------------------
void gaGetAmountOfOpenCLBufferObjectsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    int amountOfBuffers = -1;
    bool rc = gaGetAmountOfOpenCLBufferObjectsImpl((int)contextIdAsInt32, amountOfBuffers);

    // Send the success value:
    apiSocket << rc;

    if (rc)
    {
        // Send the programs amount:
        apiSocket << (gtInt32)amountOfBuffers;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaUpdateOpenCLBufferRawDataStub
// Description: Stub function for gaUpdateOpenCLBufferRawData
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
void gaUpdateOpenCLBufferRawDataStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Read the arguments from the apiSocket:
    gtInt64 contextId = -1;
    apiSocket >> contextId;

    // Get amount of buffers:
    gtInt64 amountOfBuffers = 0;
    apiSocket >> amountOfBuffers;

    // Create the buffers vector:
    gtVector<int> bufferIndices;

    for (int i = 0; i < amountOfBuffers; i++)
    {
        // Get buffer name:
        gtInt32 bufferIndex = -1;
        apiSocket >> bufferIndex;

        // Add it to the buffers vector:
        bufferIndices.push_back((int)bufferIndex);
    }

    // Call the real function:
    retVal = gaUpdateOpenCLBufferRawDataImpl((int)contextId, bufferIndices);

    // Write the
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateOpenCLSubBufferRawDataStub
// Description: Stub function for gaUpdateOpenCLBufferRawData
// Arguments:   osSocket& apiSocket
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        11/1/2011
// ---------------------------------------------------------------------------
void gaUpdateOpenCLSubBufferRawDataStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Read the arguments from the apiSocket:
    gtInt64 contextId = -1;
    apiSocket >> contextId;

    // Get amount of sub-buffers:
    gtInt64 amountOfSubBuffers = 0;
    apiSocket >> amountOfSubBuffers;

    // Create the buffers vector:
    gtVector<int> subBufferIndices;

    for (int i = 0; i < amountOfSubBuffers; i++)
    {
        // Get sub-buffer name:
        gtInt32 subBufferIndex = -1;
        apiSocket >> subBufferIndex;

        // Add it to the buffers vector:
        subBufferIndices.push_back((int)subBufferIndex);
    }

    // Call the real function:
    retVal = gaUpdateOpenCLSubBufferRawDataImpl((int)contextId, subBufferIndices);

    // Write the
    apiSocket << retVal;
}



// ---------------------------------------------------------------------------
// Name:        gaUpdateOpenCLImageRawDataStub
// Description: Stub function for gaUpdateOpenCLTextureRawData
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
void gaUpdateOpenCLImageRawDataStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Read the arguments from the apiSocket:
    gtInt64 contextId = -1;
    apiSocket >> contextId;

    // Get amount of images:
    gtInt64 amountOfImages = 0;
    apiSocket >> amountOfImages;

    // Create the images vector:
    gtVector<int> imageIndices;

    for (int i = 0; i < amountOfImages; i++)
    {
        // Get image index:
        gtInt32 imageIndex = -1;
        apiSocket >> imageIndex;

        // Add it to the images vector:
        imageIndices.push_back((int)imageIndex);
    }

    // Call the real function:
    retVal = gaUpdateOpenCLImageRawDataImpl((int)contextId, imageIndices);

    // Write the
    apiSocket << retVal;

}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLBufferObjectDetailsStub
// Description: Stub function for gaGetOpenCLBufferObjectDetails
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        19/11/2009
// ---------------------------------------------------------------------------
void gaGetOpenCLBufferObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 bufferIdAsInt32 = -1;
    apiSocket >> bufferIdAsInt32;

    // Call the function implementation:
    const apCLBuffer* pCLBufferDetails = NULL;
    bool retVal = gaGetOpenCLBufferObjectDetailsImpl((int)contextIdAsInt32, (int)bufferIdAsInt32, pCLBufferDetails);

    // Return the return value:
    retVal = retVal && (pCLBufferDetails != NULL);
    apiSocket << retVal;

    if (retVal)
    {
        pCLBufferDetails->writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLSubBufferObjectDetailsStub
// Description: Stub function for gaGetOpenCLSubBufferObjectDetails
// Arguments:   osSocket& apiSocket
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        26/10/2010
// ---------------------------------------------------------------------------
void gaGetOpenCLSubBufferObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 subBufferNameAsInt32 = -1;
    apiSocket >> subBufferNameAsInt32;

    // Call the function implementation:
    const apCLSubBuffer* pCLSubBufferDetails = NULL;
    bool retVal = gaGetOpenCLSubBufferObjectDetailsImpl((int)contextIdAsInt32, (int)subBufferNameAsInt32, pCLSubBufferDetails);

    // Return the return value:
    retVal = retVal && (pCLSubBufferDetails != NULL);
    apiSocket << retVal;

    if (retVal)
    {
        pCLSubBufferDetails->writeSelfIntoChannel(apiSocket);
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfOpenCLImageObjectsStub
// Description: Stub function for gaGetAmountOfOpenCLImageObjects
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        1/12/2009
// ---------------------------------------------------------------------------
void gaGetAmountOfOpenCLImageObjectsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    int amountOfImages = -1;
    bool rc = gaGetAmountOfOpenCLImageObjectsImpl((int)contextIdAsInt32, amountOfImages);

    // Send the success value:
    apiSocket << rc;

    if (rc)
    {
        // Send the images amount:
        apiSocket << (gtInt32)amountOfImages;
    }
}
// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLImageObjectDetailsStub
// Description: Stub function for gaGetOpenCLImageObjectDetails
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        1/12/2009
// ---------------------------------------------------------------------------
void gaGetOpenCLImageObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 imageIdAsInt32 = -1;
    apiSocket >> imageIdAsInt32;

    // Call the function implementation:
    const apCLImage* pCLImageDetails = NULL;
    bool retVal = gaGetOpenCLImageObjectDetailsImpl((int)contextIdAsInt32, (int)imageIdAsInt32, pCLImageDetails);

    // Return the return value:
    retVal = retVal && (pCLImageDetails != NULL);
    apiSocket << retVal;

    if (retVal)
    {
        pCLImageDetails->writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfOpenCLPipeObjectsStub
// Description: Stub function for gaGetAmountOfOpenCLPipeObjects
// Author:      Uri Shomroni
// Date:        1/10/2014
// ---------------------------------------------------------------------------
void gaGetAmountOfOpenCLPipeObjectsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    int amountOfPipes = -1;
    bool rc = gaGetAmountOfOpenCLPipeObjectsImpl((int)contextIdAsInt32, amountOfPipes);

    // Send the success value:
    apiSocket << rc;

    if (rc)
    {
        // Send the programs amount:
        apiSocket << (gtInt32)amountOfPipes;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLPipeObjectDetailsStub
// Description: Stub function for gaGetOpenCLPipeObjectDetails
// Author:      Uri Shomroni
// Date:        1/10/2014
// ---------------------------------------------------------------------------
void gaGetOpenCLPipeObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 pipeIdAsInt32 = -1;
    apiSocket >> pipeIdAsInt32;

    // Call the function implementation:
    const apCLPipe* pCLPipeDetails = NULL;
    bool retVal = gaGetOpenCLPipeObjectDetailsImpl((int)contextIdAsInt32, (int)pipeIdAsInt32, pCLPipeDetails);

    // Return the return value:
    retVal = retVal && (pCLPipeDetails != NULL);
    apiSocket << retVal;

    if (retVal)
    {
        pCLPipeDetails->writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfCommandQueuesStub
// Description: Stub function for
// Author:      Uri Shomroni
// Date:        2/12/2009
// ---------------------------------------------------------------------------
void gaGetAmountOfCommandQueuesStub(osSocket& apiSocket)
{
    // Read the function arguments:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;
    int contextId = (int)contextIdAsInt32;

    int amountOfQueues = -1;
    bool retVal = gaGetAmountOfCommandQueuesImpl(contextId, amountOfQueues);

    apiSocket << retVal;

    if (retVal)
    {
        apiSocket << (gtInt32)amountOfQueues;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetCommandQueueDetailsStub
// Description: Stub function for gaGetCommandQueueDetails
// Author:      Uri Shomroni
// Date:        2/12/2009
// ---------------------------------------------------------------------------
void gaGetCommandQueueDetailsStub(osSocket& apiSocket)
{
    // Read the function arguments:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;
    int contextId = (int)contextIdAsInt32;

    gtInt32 queueIndexAsInt32 = -1;
    apiSocket >> queueIndexAsInt32;
    int queueIndex = (int)queueIndexAsInt32;

    const apCLCommandQueue* pCLCommandQueue = NULL;
    bool retVal = gaGetCommandQueueDetailsImpl(contextId, queueIndex, pCLCommandQueue);

    retVal = retVal && (pCLCommandQueue != NULL);

    apiSocket << retVal;

    if (retVal)
    {
        pCLCommandQueue->writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfCommandsInQueueStub
// Description: Stub function for gaGetAmountOfCommandsInQueue
// Author:      Uri Shomroni
// Date:        2/12/2009
// ---------------------------------------------------------------------------
void gaGetAmountOfCommandsInQueueStub(osSocket& apiSocket)
{
    // Read the function arguments:
    gtUInt64 queueHandleAsUInt64 = 0;
    apiSocket >> queueHandleAsUInt64;
    oaCLCommandQueueHandle queueHandle = (oaCLCommandQueueHandle)queueHandleAsUInt64;

    int amountOfCommands = 1;
    bool retVal = gaGetAmountOfCommandsInQueueImpl(queueHandle, amountOfCommands);

    apiSocket << retVal;

    if (retVal)
    {
        apiSocket << (gtInt32)amountOfCommands;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfEventsInQueueStub
// Description: Stub function for gaGetAmountOfEventsInQueue
// Author:      Sigal Algranaty
// Date:        17/2/2010
// ---------------------------------------------------------------------------
void gaGetAmountOfEventsInQueueStub(osSocket& apiSocket)
{
    // Read the function arguments:
    gtUInt64 queueHandleAsUInt64 = 0;
    apiSocket >> queueHandleAsUInt64;
    oaCLCommandQueueHandle queueHandle = (oaCLCommandQueueHandle)queueHandleAsUInt64;

    int amountOfEvents = 1;
    bool retVal = gaGetAmountOfEventsInQueueImpl(queueHandle, amountOfEvents);

    apiSocket << retVal;

    if (retVal)
    {
        apiSocket << (gtInt32)amountOfEvents;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetEnqueuedCommandDetailsStub
// Description: Stub function for gaGetEnqueuedCommandDetails
// Author:      Uri Shomroni
// Date:        2/12/2009
// ---------------------------------------------------------------------------
void gaGetEnqueuedCommandDetailsStub(osSocket& apiSocket)
{
    // Read the function arguments:
    gtUInt64 queueHandleAsUInt64 = 0;
    apiSocket >> queueHandleAsUInt64;
    oaCLCommandQueueHandle queueHandle = (oaCLCommandQueueHandle)queueHandleAsUInt64;

    gtInt32 commandIndexAsInt32 = -1;
    apiSocket >> commandIndexAsInt32;
    int commandIndex = (int)commandIndexAsInt32;

    const apCLEnqueuedCommand* pCommand = NULL;
    bool retVal = gaGetEnqueuedCommandDetailsImpl(queueHandle, commandIndex, pCommand);

    retVal = retVal && (pCommand != NULL);

    apiSocket << retVal;

    if (retVal)
    {
        // Since apCLEnqueuedCommand is an abstract class, we write it as an osTransferableObject:
        apiSocket << (osTransferableObject&)(*pCommand);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfOpenCLSamplersStub
// Description: Stub function for gaGetAmountOfOpenCLSamplers
// Author:      Uri Shomroni
// Date:        29/3/2010
// ---------------------------------------------------------------------------
void gaGetAmountOfOpenCLSamplersStub(osSocket& apiSocket)
{
    // Read the function arguments:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;
    int contextId = (int)contextIdAsInt32;

    // Call the function implementation:
    int amountOfSamplers = -1;
    bool retVal = gaGetAmountOfOpenCLSamplersImpl(contextId, amountOfSamplers);

    // Write the success value:
    apiSocket << retVal;

    if (retVal)
    {
        // Write the return value:
        apiSocket << (gtInt32)amountOfSamplers;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLSamplerObjectDetailsStub
// Description: Stub function for gaGetOpenCLSamplerObjectDetails
// Author:      Uri Shomroni
// Date:        29/3/2010
// ---------------------------------------------------------------------------
void gaGetOpenCLSamplerObjectDetailsStub(osSocket& apiSocket)
{
    // Read the function arguments:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;
    int contextId = (int)contextIdAsInt32;

    gtInt32 samplerIndexAsInt32 = -1;
    apiSocket >> samplerIndexAsInt32;
    int samplerIndex = (int)samplerIndexAsInt32;

    // Call the function implementation:
    const apCLSampler* pSamplerDetails = NULL;
    bool retVal = gaGetOpenCLSamplerObjectDetailsImpl(contextId, samplerIndex, pSamplerDetails);

    retVal = retVal && (pSamplerDetails != NULL);

    // Write the success value:
    apiSocket << retVal;

    if (retVal)
    {
        pSamplerDetails->writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfOpenCLEventsStub
// Description: Stub function for gaGetAmountOfOpenCLEvents
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
void gaGetAmountOfOpenCLEventsStub(osSocket& apiSocket)
{
    // Read the function arguments:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;
    int contextId = (int)contextIdAsInt32;

    // Call the function implementation:
    int amountOfEvents = -1;
    bool retVal = gaGetAmountOfOpenCLEventsImpl(contextId, amountOfEvents);

    // Write the success value:
    apiSocket << retVal;

    if (retVal)
    {
        // Write the return value:
        apiSocket << (gtInt32)amountOfEvents;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLEventObjectDetailsStub
// Description: Stub function for gaGetOpenCLEventObjectDetails
// Author:      Uri Shomroni
// Date:        22/8/2013
// ---------------------------------------------------------------------------
void gaGetOpenCLEventObjectDetailsStub(osSocket& apiSocket)
{
    // Read the function arguments:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;
    int contextId = (int)contextIdAsInt32;

    gtInt32 eventIndexAsInt32 = -1;
    apiSocket >> eventIndexAsInt32;
    int eventIndex = (int)eventIndexAsInt32;

    // Call the function implementation:
    const apCLEvent* pEventDetails = NULL;
    bool retVal = gaGetOpenCLEventObjectDetailsImpl(contextId, eventIndex, pEventDetails);

    retVal = retVal && (pEventDetails != NULL);

    // Write the success value:
    apiSocket << retVal;

    if (retVal)
    {
        pEventDetails->writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLContextDetailsStub
// Description: Stub function for gaGetOpenCLContextDetails
// Author:      Sigal Algranaty
// Date:        7/11/2009
// ---------------------------------------------------------------------------
void gaGetOpenCLContextDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the API socket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    apCLContext contextInfo;
    bool rc = gaGetOpenCLContextDetailsImpl((int)contextIdAsInt32, contextInfo);

    // Return the return value:
    apiSocket << rc;

    if (rc)
    {
        contextInfo.writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaSetCLBufferDisplayPropertiesStub
// Description: Stub function for gaSetCLBufferDisplayProperties
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        8/12/2009
// ---------------------------------------------------------------------------
void gaSetCLBufferDisplayPropertiesStub(osSocket& apiSocket)
{
    // Get the context id:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Get buffer name:
    gtInt32 bufferIdAsInt32 = 0;
    apiSocket >> bufferIdAsInt32;

    // Get buffer display format:
    gtInt32 bufferDisplayFormatAsInt32 = OA_TEXEL_FORMAT_UNKNOWN;
    apiSocket >> bufferDisplayFormatAsInt32;
    oaTexelDataFormat bufferDisplayFormat = (oaTexelDataFormat)bufferDisplayFormatAsInt32;

    // Get the buffer offsetAsUInt64:
    gtUInt64 offsetAsUInt64 = 0;
    apiSocket >> offsetAsUInt64;

    // Get the buffer strideAsUInt64:
    gtUInt64 strideAsUInt64 = 0;
    apiSocket >> strideAsUInt64;

    // Call the real function:
    bool retVal = gaSetCLBufferDisplayPropertiesImpl((int)contextIdAsInt32, (int)bufferIdAsInt32, bufferDisplayFormat, (int)offsetAsUInt64, (GLsizei)strideAsUInt64);

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetCLSubBufferDisplayPropertiesStub
// Description: Stub function for gaSetCLSubBufferDisplayProperties
// Arguments:   osSocket& apiSocket
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        11/1/2011
// ---------------------------------------------------------------------------
void gaSetCLSubBufferDisplayPropertiesStub(osSocket& apiSocket)
{
    // Get the context id:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Get buffer name:
    gtInt32 subBufferIdAsInt32 = 0;
    apiSocket >> subBufferIdAsInt32;

    // Get buffer display format:
    gtInt32 bufferDisplayFormatAsInt32 = OA_TEXEL_FORMAT_UNKNOWN;
    apiSocket >> bufferDisplayFormatAsInt32;
    oaTexelDataFormat bufferDisplayFormat = (oaTexelDataFormat)bufferDisplayFormatAsInt32;

    // Get the buffer offsetAsUInt64:
    gtUInt64 offsetAsUInt64 = 0;
    apiSocket >> offsetAsUInt64;

    // Get the buffer strideAsUInt64:
    gtUInt64 strideAsUInt64 = 0;
    apiSocket >> strideAsUInt64;

    // Call the real function:
    bool retVal = gaSetCLSubBufferDisplayPropertiesImpl((int)contextIdAsInt32, (int)subBufferIdAsInt32 , bufferDisplayFormat, (int)offsetAsUInt64, (GLsizei)strideAsUInt64);

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAMDOpenCLPerformanceCountersValuesStub
// Description: Stub function for gaGetAMDOpenCLPerformanceCountersValues
// Author:      Sigal Algranaty
// Date:        25/2/2010
// ---------------------------------------------------------------------------
void gaGetAMDOpenCLPerformanceCountersValuesStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    int amountOfValues = 0;
    const double* pValuesArray = NULL;

    // Get all the counters values in a single array of values
    retVal = gaGetAMDOpenCLPerformanceCountersValuesImpl(pValuesArray, amountOfValues);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Send the amount of values to be sent
        apiSocket << (gtInt32)amountOfValues;

        // Return the values:
        // TO_DO: Write me in one chunk!!!
        for (int i = 0; i < amountOfValues; i++)
        {
            apiSocket << pValuesArray[i];
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gaActivateAMDOpenCLPerformanceCountersStub
// Description: Stub function for gaActivateAMDOpenCLPerformanceCounters
// Author:      Sigal Algranaty
// Date:        02/03/2010
// ---------------------------------------------------------------------------
void gaActivateAMDOpenCLPerformanceCountersStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Read the amount of counters:
    gtInt32 amountOfCounters = 0;
    apiSocket >> amountOfCounters;

    gtVector<apCounterActivationInfo> countersActivationVector;

    // Read each of the counters info;
    for (int i = 0; i < amountOfCounters; i++)
    {
        apCounterActivationInfo activationInfo;
        bool rc = activationInfo.readSelfFromChannel(apiSocket);
        GT_ASSERT(rc);

        // Add this activation info to the vector:
        countersActivationVector.push_back(activationInfo);
    }

    // Get all the counters values in a single array of values
    retVal = gaActivateAMDOpenCLPerformanceCountersImpl(countersActivationVector);

    // Return the return value:
    apiSocket << retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetOpenCLQueuePerformanceCountersValuesStub
// Description: Stub function for gaGetOpenCLQueuePerformanceCountersValues
// Author:      Sigal Algranaty
// Date:        8/3/2010
// ---------------------------------------------------------------------------
void gaGetOpenCLQueuePerformanceCountersValuesStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    int amountOfValues = 0;
    const double* pValuesArray = NULL;

    // Get all the counters values in a single array of values
    retVal = gaGetOpenCLQueuePerformanceCountersValuesImpl(pValuesArray, amountOfValues);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Send the amount of values to be sent
        apiSocket << (gtInt32)amountOfValues;

        // Return the values:
        // TO_DO: Write me in one chunk!!!
        for (int i = 0; i < amountOfValues; i++)
        {
            apiSocket << pValuesArray[i];
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetCLContextLogFilePathStub
// Description: Stub function for gaGetCLContextLogFilePath
// Author:      Sigal Algranaty
// Date:        23/3/2010
// ---------------------------------------------------------------------------
void gaGetCLContextLogFilePathStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Get the input arguments:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    bool logFileExists = false;
    osFilePath logFilePath;
    retVal = gaGetCLContextLogFilePathImpl((int)contextIdAsInt32, logFileExists, logFilePath);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return output parameters:
        apiSocket << logFileExists;

        if (logFileExists)
        {
            const gtString& logFileAsString = logFilePath.asString();
            apiSocket << logFileAsString;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetCurrentOpenCLStatisticsStub
// Description: Stub for gaGetCurrentOpenCLStatistics()
// Arguments:   osSocket& apiSocket
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        25/3/2010
// ---------------------------------------------------------------------------
void gaGetCurrentOpenCLStatisticsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Allocate a new statistics object:
    apStatistics* pStatistics = new apStatistics();


    // Call the function implementation:
    bool retVal = gaGetCurrentOpenCLStatisticsImpl((int)contextIdAsInt32, pStatistics);

    // Send success value:
    apiSocket << retVal;

    GT_IF_WITH_ASSERT(retVal)
    {
        // Write the statistics object into the channel:
        pStatistics->writeSelfIntoChannel(apiSocket);

        // Delete the allocated statistics object:
        delete pStatistics;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaClearOpenCLFunctionCallsStatisticsStub
// Description: A stub function for gaClearOpenCLFunctionCallsStatistics
// Arguments:   osSocket& apiSocket
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        28/3/2010
// ---------------------------------------------------------------------------
void gaClearOpenCLFunctionCallsStatisticsStub(osSocket& apiSocket)
{
    // Call the function implementation:
    bool retVal = gaClearOpenCLFunctionCallsStatisticsImpl();

    // Send success value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetOpenCLOperationExecutionStub
// Description: A stub function for gaSetOpenCLOperationExecution
// Arguments:   osSocket& apiSocket
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        9/5/2010
// ---------------------------------------------------------------------------
void gaSetOpenCLOperationExecutionStub(osSocket& apiSocket)
{
    // Read the parameters from socket:
    gtInt32 executionTypeAsInt32 = 0;
    apiSocket >> executionTypeAsInt32;
    apOpenCLExecutionType openCLExecutionType = (apOpenCLExecutionType)executionTypeAsInt32;

    bool isExecutionOn =  false;
    apiSocket >> isExecutionOn;

    // Call the function implementation:
    bool retVal = gaSetOpenCLOperationExecutionImpl(openCLExecutionType, isExecutionOn);

    // Send success value:
    apiSocket << retVal;

}

// ---------------------------------------------------------------------------
// Name:        gaSetKernelSourceFilePathStub
// Description: A stub function for gaSetKernelSourceFilePath
// Author:      Gilad Yarnitzky
// Date:        20/4/2011
// ---------------------------------------------------------------------------
void gaSetKernelSourceFilePathStub(osSocket& apiSocket)
{
    gtVector<osFilePath> clProgramFilePaths;
    gtInt32 numPrograms = 0;

    // Get number of programs:
    apiSocket >> numPrograms;

    // Get the program file paths:
    for (gtInt32 nProgram = 0 ; nProgram < numPrograms; nProgram++)
    {
        gtString currentProgramPathAsString;
        apiSocket >> currentProgramPathAsString;

        // Convert back to osFilePath:
        osFilePath currentProgramPath(currentProgramPathAsString);

        // Add to vector:
        clProgramFilePaths.push_back(currentProgramPath);
    }

    // Call the function implementation:
    bool retVal = gaSetKernelSourceFilePathImpl(clProgramFilePaths);

    // Send success value:
    apiSocket << retVal;
}


