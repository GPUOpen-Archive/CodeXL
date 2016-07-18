//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsAPIFunctionsStubs.cpp
///
//==================================================================================

//------------------------------ gsAPIFunctionsStubs.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osRawMemoryStream.h>
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAPIClasses/Include/apBreakReason.h>
#include <AMDTAPIClasses/Include/apCounterID.h>
#include <AMDTAPIClasses/Include/apGLFBO.h>
#include <AMDTAPIClasses/Include/apGLDisplayList.h>
#include <AMDTAPIClasses/Include/apGLItemsCollection.h>
#include <AMDTAPIClasses/Include/apGLProgram.h>
#include <AMDTAPIClasses/Include/apGLRenderBuffer.h>
#include <AMDTAPIClasses/Include/apGLRenderContextGraphicsInfo.h>
#include <AMDTAPIClasses/Include/apGLRenderContextInfo.h>
#include <AMDTAPIClasses/Include/apGLSync.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>
#include <AMDTAPIClasses/Include/apGLPipeline.h>
#include <AMDTAPIClasses/Include/apGLSampler.h>
#include <AMDTAPIClasses/Include/apGLVBO.h>
#include <AMDTAPIClasses/Include/apRenderPrimitivesStatistics.h>
#include <AMDTAPIClasses/Include/apSearchDirection.h>
#include <AMDTAPIClasses/Include/apStatistics.h>
#include <AMDTServerUtilities/Include/suAPIMainLoop.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>

// Local:
#include <src/gsAPIFunctionsImplementations.h>
#include <src/gsAPIFunctionsStubs.h>
#include <src/gsStringConstants.h>

// iPhone on-device only items:
#ifdef _GR_IPHONE_DEVICE_BUILD
    #include <AMDTServerUtilities/Include/suGlobalVariables.h>
    #include <AMDTServerUtilities/Include/suSpyBreakpointImplementation.h>
#endif


// ----------------------------------- Aid functions -----------------------------------


// ---------------------------------------------------------------------------
// Name:        gsRegisterAPIStubFunctions
// Description: Registers OpenGL Server module stub functions.
// Author:      Yaki Tebeka
// Date:        30/11/2009
// ---------------------------------------------------------------------------
void gsRegisterAPIStubFunctions()
{
    suRegisterAPIFunctionStub(GA_FID_gaOpenGLServerInitializationEnded, &gaOpenGLServerInitializationEndedStub);
    suRegisterAPIFunctionStub(GA_FID_gaBeforeDirectAPIFunctionExecution, &gaBeforeDirectAPIFunctionExecutionStub);
    suRegisterAPIFunctionStub(GA_FID_gaBeforeDirectAPIFunctionExecutionDuringBreak, &gaBeforeDirectAPIFunctionExecutionDuringBreakStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfRenderContexts, &gaGetAmoutOfRenderContextsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetRenderContextDetails, &gaGetRenderContextDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetRenderContextGraphicsDetails, &gaGetRenderContextGraphicsDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetThreadCurrentRenderContext, &gaGetThreadCurrentRenderContextStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetDefaultOpenGLStateVariableValue, &gaGetDefaultOpenGLStateVariableValueStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetOpenGLStateVariableValue, &gaGetOpenGLStateVariableValueStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfTextureUnits, &gaGetAmountOfTextureUnitsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetActiveTextureUnit, &gaGetActiveTextureUnitStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetTextureUnitName, &gaGetTextureUnitNameStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetEnabledTexturingMode, &gaGetEnabledTexturingModeStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfTextureObjects, &gaGetAmountOfTextureObjectsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfRenderBufferObjects, &gaGetAmountOfRenderBufferObjectsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfFBOs, &gaGetAmountOfFBOsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfPipelineObjects, &gaGetAmountOfPiplineObjectsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetPipelineObjectDetails, &gaGetPiplineObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetPipelineObjectName, &gaGetPiplineNameStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfSamplerObjects, &gaGetAmountOfSamplerObjectsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetSamplerObjectDetails, &gaGetSamplerObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetSamplerObjectName, &gaGetSamplerNameStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetFBODetails, &gaGetFBODetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetActiveFBO, &gaGetActiveFBOStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetFBOName, &gaGetFBONameStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfVBOs, &gaGetAmountOfVBOsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetVBODetails, &gaGetVBODetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetVBOAttachment, &gaGetVBOAttachmentStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetVBOName, &gaGetVBONameStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetVBODisplayProperties, &gaSetVBODisplayPropertiesStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetRenderBufferObjectName, &gaGetRenderBufferObjectNameStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfPBuffersObjects, &gaGetAmountOfPBuffersObjectsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfSyncObjects, &gaGetAmountOfSyncObjectsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfStaticBuffersObjects, &gaGetAmountOfStaticBuffersObjectsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetBoundTexture, &gaGetBoundTextureStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetTextureObjectName, &gaGetTextureObjectNameStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetTextureObjectType, &gaGetTextureObjectTypeStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetTextureObjectDetails, &gaGetTextureObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetTextureMiplevelDataFilePath, &gaGetTextureMiplevelDataFilePathStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetTextureThumbnailDataObjectDetails, &gaGetTextureThumbnailDataObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetTextureDataObjectDetails, &gaGetTextureDataObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetTextureMemoryDataObjectDetails, &gaGetTextureMemoryDataObjectDetails);
    suRegisterAPIFunctionStub(GA_FID_gaGetRenderBufferObjectDetails, &gaGetRenderBufferObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaMarkAllTextureImagesAsUpdated, &gaMarkAllTextureImagesAsUpdatedStub);
    suRegisterAPIFunctionStub(GA_FID_gaUpdateTextureRawData, &gaUpdateTextureRawDataStub);
    suRegisterAPIFunctionStub(GA_FID_gaUpdateTextureParameters, &gaUpdateTextureParametersStub);
    suRegisterAPIFunctionStub(GA_FID_gaIsTextureImageDirty, &gaIsTextureImageDirtyStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetStaticBufferObjectDetails, &gaGetStaticBufferObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetStaticBufferType, &gaGetStaticBufferTypeStub);
    suRegisterAPIFunctionStub(GA_FID_gaUpdateStaticBuffersDimensions, &gaUpdateStaticBuffersDimensionsStub);
    suRegisterAPIFunctionStub(GA_FID_gaUpdateStaticBufferRawData, &gaUpdateStaticBufferRawDataStub);
    suRegisterAPIFunctionStub(GA_FID_gaUpdatePBufferStaticBufferRawData, &gaUpdatePBufferStaticBufferRawDataStub);
    suRegisterAPIFunctionStub(GA_FID_gaUpdateRenderBufferRawData, &gaUpdateRenderBufferRawDataStub);
    suRegisterAPIFunctionStub(GA_FID_gaUpdateVBORawData, &gaUpdateVBORawDataStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfPBufferContentObjects, &gaGetAmountOfPBufferContentObjectsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetPBufferStaticBufferType, &gaGetPBufferStaticBufferTypeStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetPBufferStaticBufferObjectDetails, &gaGetPBufferStaticBufferObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetPBufferObjectDetails, &gaGetPBufferObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetSyncObjectDetails, &gaGetSyncObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaUpdatePBuffersDimensions, &gaUpdatePBuffersDimensionsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfProgramObjects, &gaGetAmountOfProgramObjectsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetActiveProgramObjectName, &gaGetActiveProgramObjectNameStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetProgramObjectName, &gaGetProgramObjectNameStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetProgramObjectDetails, &gaGetProgramObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetProgramActiveUniforms, &gaGetProgramActiveUniformsStub);
    suRegisterAPIFunctionStub(GA_FID_gaLinkProgramObject, &gaLinkProgramObjectStub);
    suRegisterAPIFunctionStub(GA_FID_gaValidateProgramObject, &gaValidateProgramObjectStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfShaderObjects, &gaGetAmountOfShaderObjectsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetShaderObjectName, &gaGetShaderObjectNameStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetShaderObjectDetails, &gaGetShaderObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaMarkShaderObjectSourceCodeAsForced, &gaMarkShaderObjectSourceCodeAsForcedStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetShaderObjectSourceCode, &gaSetShaderObjectSourceCodeStub);
    suRegisterAPIFunctionStub(GA_FID_gaCompileShaderObject, &gaCompileShaderObjectStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfCurrentFrameFunctionCalls, &gaGetAmountOfCurrentFrameFunctionCallsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetAmountOfDisplayLists, &gaGetAmountOfDisplayListsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetDisplayListObjectName, &gaGetDisplayListObjectNameStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetDisplayListObjectDetails, &gaGetDisplayListObjectDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetCurrentFrameFunctionCall, &gaGetCurrentFrameFunctionCallStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetCurrentFrameFunctionCallDeprecationDetails, &gaGetCurrentFrameFunctionCallDeprecationDetailsStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetLastFunctionCall, &gaGetLastFunctionCallStub);
    suRegisterAPIFunctionStub(GA_FID_gaFindCurrentFrameFunctionCall, &gaFindCurrentFrameFunctionCallStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetCurrentStatistics, &gaGetCurrentStatisticsStub);
    suRegisterAPIFunctionStub(GA_FID_gaClearFunctionCallsStatistics, &gaClearFunctionCallsStatisticsStub);
    suRegisterAPIFunctionStub(GA_FID_gaIsInOpenGLBeginEndBlock, &gaIsInOpenGLBeginEndBlockStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetRenderPrimitivesStatistics, &gaGetRenderPrimitivesStatisticsStub);
    suRegisterAPIFunctionStub(GA_FID_gaFindStringMarker, &gaFindStringMarkerStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetContextLogFilePath, &gaGetContextLogFilePathStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetCurrentOpenGLError, &gaGetCurrentOpenGLErrorStub);
    suRegisterAPIFunctionStub(GA_FID_gaForceOpenGLFlush, &gaForceOpenGLFlushStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetInteractiveBreakMode, &gaSetInteractiveBreakModeStub);
    suRegisterAPIFunctionStub(GA_FID_gaForceOpenGLPolygonRasterMode, &gaForceOpenGLPolygonRasterModeStub);
    suRegisterAPIFunctionStub(GA_FID_gaCancelOpenGLPolygonRasterModeForcing, &gaCancelOpenGLPolygonRasterModeForcingStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetOpenGLNullDriver, &gaSetOpenGLNullDriverStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetOpenGLForceStub, &gaSetOpenGLForceStubStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetSpyPerformanceCountersValues, &gaGetSpyPerformanceCountersValuesStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetRemoteOSPerformanceCountersValues, &gaGetRemoteOSPerformanceCountersValuesStub);

#ifdef _GR_IPHONE_DEVICE_BUILD
    suRegisterAPIFunctionStub(GA_FID_gaAddSupportediPhonePerformanceCounter, &gaAddSupportediPhonePerformanceCounterStub);
    suRegisterAPIFunctionStub(GA_FID_gaInitializeiPhonePerformanceCountersReader, &gaInitializeiPhonePerformanceCountersReaderStub);
    suRegisterAPIFunctionStub(GA_FID_gaGetiPhonePerformanceCountersValues, &gaGetiPhonePerformanceCountersValuesStub);
#endif

    suRegisterAPIFunctionStub(GA_FID_gaGetATIPerformanceCountersValues, &gaGetATIPerformanceCountersValuesStub);
    suRegisterAPIFunctionStub(GA_FID_gaActivateATIPerformanceCounters, &gaActivateATIPerformanceCountersStub);
    suRegisterAPIFunctionStub(GA_FID_gaUpdateContextDataSnapshot, &gaUpdateContextDataSnapshotStub);
    suRegisterAPIFunctionStub(GA_FID_gaMakeThreadUpdateContextDataSnapshot, &gaMakeThreadUpdateContextDataSnapshotStub);
    suRegisterAPIFunctionStub(GA_FID_gaMakeThreadUpdateStaticBufferRawData, &gaMakeThreadUpdateStaticBufferRawDataStub);
    suRegisterAPIFunctionStub(GA_FID_gaMakeThreadUpdateStaticBuffersDimensions, &gaMakeThreadUpdateStaticBuffersDimensionsStub);
    suRegisterAPIFunctionStub(GA_FID_gaMakeThreadUpdatePBuffersDimensions, &gaMakeThreadUpdatePBuffersDimensionsStub);
    suRegisterAPIFunctionStub(GA_FID_gaMakeThreadUpdatePBufferStaticBufferRawData, &gaMakeThreadUpdatePBufferStaticBufferRawDataStub);
    suRegisterAPIFunctionStub(GA_FID_gaMakeThreadUpdateRenderBufferRawData, &gaMakeThreadUpdateRenderBufferRawDataStub);
    suRegisterAPIFunctionStub(GA_FID_gaMakeThreadUpdateVBORawData, &gaMakeThreadUpdateVBORawDataStub);
    suRegisterAPIFunctionStub(GA_FID_gaMakeThreadSetVBODisplayProperties, &gaMakeThreadSetVBODisplayPropertiesStub);
    suRegisterAPIFunctionStub(GA_FID_gaMakeThreadUpdateTextureRawData, &gaMakeThreadUpdateTextureRawDataStub);
    suRegisterAPIFunctionStub(GA_FID_gaMakeThreadUpdateTextureParameters, &gaMakeThreadUpdateTextureParametersStub);
    suRegisterAPIFunctionStub(GA_FID_gaMakeThreadLinkProgramObject, &gaMakeThreadLinkProgramObjectStub);
    suRegisterAPIFunctionStub(GA_FID_gaMakeThreadValidateProgramObject, &gaMakeThreadValidateProgramObjectStub);
    suRegisterAPIFunctionStub(GA_FID_gaMakeThreadSetShaderObjectSourceCode, &gaMakeThreadSetShaderObjectSourceCodeStub);
    suRegisterAPIFunctionStub(GA_FID_gaMakeThreadCompileShaderObject, &gaMakeThreadCompileShaderObjectStub);
    suRegisterAPIFunctionStub(GA_FID_gaEnableGLDebugOutputLogging, &gaEnableGLDebugOutputLoggingStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetGLDebugOutputKindMask, &gaSetGLDebugOutputKindMaskStub);
    suRegisterAPIFunctionStub(GA_FID_gaSetGLDebugOutputSeverityEnabled, &gaSetGLDebugOutputSeverityEnabledStub);
    suRegisterAPIFunctionStub(GA_FID_gaDoesDebugForcedContextExist, &gaDoesDebugForcedContextExistStub);

}


// ---------------------------------------------------------------------------
// Name:        gsHandleAPIInitializationCalls
// Description: Handles the API functions that initialize the OpenGL Server.
// Author:      Yaki Tebeka
// Date:        21/12/2009
// ---------------------------------------------------------------------------
void gsHandleAPIInitializationCalls()
{
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_startedHandlingAPIInitCalls, OS_DEBUG_LOG_DEBUG);

    // If the API thread didn't start running yet, the main thread should handle the OpenGL Server's API initialization calls:
    bool shouldMainThreadHandleAPIInitializationCalls = suMainThreadStarsInitializingAPIConnection(AP_OPENGL_API_CONNECTION);

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
        suMainThreadEndedInitializingAPIConnection(AP_OPENGL_API_CONNECTION);
    }
    else
    {
        // The API thread is running and listening to API calls, therefore, the API thread will handle the OpenGL API initialization calls.
        // All we need to do is to wait until the OpenGL initialization calls are done:
        OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_MainThreadWaitsForAPIThreadToHandleOGLAPIInitialization, OS_DEBUG_LOG_DEBUG);

        while (!(suIsAPIConnectionInitialized(AP_OPENGL_API_CONNECTION)))
        {
            osSleep(20);
        }

        OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_MainThreadFinishedWaitingForAPIThreadToHandleOGLAPIInitialization, OS_DEBUG_LOG_DEBUG);
    }

    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_endedHandlingAPIInitCalls, OS_DEBUG_LOG_DEBUG);
}


// ----------------------------------- Stub functions -----------------------------------



// ---------------------------------------------------------------------------
// Name:        gaOpenGLServerInitializationEndedStub
// Description: Stub function for gaOpenGLServerInitializationEnded
// Author:      Yaki Tebeka
// Date:        27/12/2009
// ---------------------------------------------------------------------------
void gaOpenGLServerInitializationEndedStub(osSocket& apiSocket)
{
    (void)(apiSocket); // unused
    // Nothing to be done.
}


// ---------------------------------------------------------------------------
// Name:        gaBeforeDirectAPIFunctionExecutionStub
// Description: Stub for gaBeforeDirectAPIFunctionExecution
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Yaki Tebeka
// Date:        20/11/2005
// ---------------------------------------------------------------------------
void gaBeforeDirectAPIFunctionExecutionStub(osSocket& apiSocket)
{
    // Debug printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_gaBeforeDirectAPIFunctionExecutionStarted, OS_DEBUG_LOG_DEBUG);

    // Read input arguments:
    gtInt32 functionIdAsInt32 = 0;
    apiSocket >> functionIdAsInt32;
    apAPIFunctionId apiFunctionId = (apAPIFunctionId)functionIdAsInt32;

    // Call the function implementation:
    osProcedureAddress retVal = gaBeforeDirectAPIFunctionExecutionImpl(apiFunctionId);

    if (NULL != retVal)
    {
        // Mark that we are starting direct function execution:
        suBeforeDirectFunctionExecution();

        if (!suIsDuringDirectFunctionExecution())
        {
            // Report failure:
            retVal = NULL;
        }
    }

    apiSocket << (gtUInt64)retVal;

    // Debug printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_gaBeforeDirectAPIFunctionExecutionEnded, OS_DEBUG_LOG_DEBUG);
}

// ---------------------------------------------------------------------------
// Name:        gaBeforeDirectAPIFunctionExecutionDuringBreakStub
// Description: Stub for gaBeforeDirectAPIFunctionExecutionDuringBreak
// Author:      Uri Shomroni
// Date:        12/2/2010
// ---------------------------------------------------------------------------
void gaBeforeDirectAPIFunctionExecutionDuringBreakStub(osSocket& apiSocket)
{
    // Debug printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_gaBeforeDirectAPIFunctionExecutionDuringBreakStubStarted, OS_DEBUG_LOG_DEBUG);

    // Read input arguments:
    gtInt32 functionIdAsInt32 = 0;
    apiSocket >> functionIdAsInt32;
    apAPIFunctionId apiFunctionId = (apAPIFunctionId)functionIdAsInt32;

    // Call the function implementation:
    osProcedureAddress funcAddress = gaBeforeDirectAPIFunctionExecutionImpl(apiFunctionId);

    // Set this address to the breakpoints manager:
    su_stat_theBreakpointsManager.setFunctionToBeExecutedDuringBreak(funcAddress);

    // Return the success value of this function:
    bool retVal = (funcAddress != NULL);

    if (retVal)
    {
        // Mark that we are starting direct function execution:
        suBeforeDirectFunctionExecution();
        retVal = suIsDuringDirectFunctionExecution();
    }

    apiSocket << retVal;

    // Debug printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_gaBeforeDirectAPIFunctionExecutionDuringBreakEnded, OS_DEBUG_LOG_DEBUG);
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmoutOfRenderContextsStub
// Description: Stub for gaGetAmoutOfRenderContexts
// Author:      Yaki Tebeka
// Date:        12/5/2004
// ---------------------------------------------------------------------------
void gaGetAmoutOfRenderContextsStub(osSocket& apiSocket)
{
    // Call the function implementation:
    int contextsAmount = 0;
    bool rc = gaGetAmoutOfRenderContextsImpl(contextsAmount);

    // Return the return value:
    apiSocket << rc;

    GT_IF_WITH_ASSERT(rc)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtInt32)contextsAmount;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetRenderContextDetailsStub
// Description: Stub function for gaRenderContextInfo
// Author:      Uri Shomroni
// Date:        11/6/2008
// ---------------------------------------------------------------------------
void gaGetRenderContextDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the API socket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    apGLRenderContextInfo renderContextInfo;
    bool rc = gaGetRenderContextDetailsImpl((int)contextIdAsInt32, renderContextInfo);

    // Return the return value:
    apiSocket << rc;

    if (rc)
    {
        renderContextInfo.writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetRenderContextGraphicsDetailsStub
// Description: Stub function for gaRenderContextGraphicsInfo
// Author:      Uri Shomroni
// Date:        19/3/2009
// ---------------------------------------------------------------------------
void gaGetRenderContextGraphicsDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the API socket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    apGLRenderContextGraphicsInfo renderContextGraphicsInfo;
    bool rc = gaGetRenderContextGraphicsDetailsImpl((int)contextIdAsInt32, renderContextGraphicsInfo);

    // Return the return value:
    apiSocket << rc;

    if (rc)
    {
        renderContextGraphicsInfo.writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetThreadCurrentRenderContextStub
// Description: Stub for gaGetThreadCurrentRenderContext
// Author:      Yaki Tebeka
// Date:        12/5/2004
// ---------------------------------------------------------------------------
void gaGetThreadCurrentRenderContextStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    apiSocket >> threadIdAsUInt64;
    osThreadId threadId = (osThreadId)threadIdAsUInt64;

    // Call the function implementation:
    int contextId = 0;
    bool rc = gaGetThreadCurrentRenderContextImpl(threadId, contextId);

    // Return the return value:
    apiSocket << rc;

    if (rc)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtInt32)contextId;
    }

    // End the direct API call:
    //  suAfterDirectFunctionExecution();
}


// ---------------------------------------------------------------------------
// Name:        gaUpdateContextDataSnapshotStub
// Description: Stub function for gaUpdateContextDataSnapshot
// Author:      Yaki Tebeka
// Date:        17/10/2005
// ---------------------------------------------------------------------------
void gaUpdateContextDataSnapshotStub(osSocket& apiSocket)
{
    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateContextDataSnapshotStart, OS_DEBUG_LOG_DEBUG);

    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    bool retVal = gaUpdateContextDataSnapshotImpl((int)contextIdAsInt32);

    // Return the return value:
    apiSocket << retVal;

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateContextDataSnapshotEnd, OS_DEBUG_LOG_DEBUG);
}


// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadRenderContextDataSnapshotStub
// Description:
//   Direct stub function for gaUpdateContextDataSnapshot()
//
//   Updates the context data of the "current render context" of the
//   thread that executes this function.
//
// Author:      Yaki Tebeka
// Date:        11/5/2005
// ---------------------------------------------------------------------------
void gaUpdateCurrentThreadRenderContextDataSnapshotStub()
{
    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateCurrentThreadCtxSnapshotStart, OS_DEBUG_LOG_DEBUG);

    // Call the real function:
    bool retVal = gaUpdateCurrentThreadRenderContextDataSnapshotImpl();

    // Get the API socket:
    osSocket* pApiSocket = suSpiesAPISocket();
    GT_IF_WITH_ASSERT(pApiSocket != NULL)
    {
        // Return the return value:
        *pApiSocket << retVal;
    }

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateCurrentThreadCtxSnapshotEnd, OS_DEBUG_LOG_DEBUG);

    // End the direct API call:
    suAfterDirectFunctionExecution();
}

// ---------------------------------------------------------------------------
// Name:        gaMakeThreadUpdateContextDataSnapshotStub
// Description: "Direct" Stub function of gaUpdateContextDataSnapshot, for the
//              case where we have no debugger.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
void gaMakeThreadUpdateContextDataSnapshotStub(osSocket& apiSocket)
{
    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateContextDataSnapshotStart, OS_DEBUG_LOG_DEBUG);

    gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    apiSocket >> threadIdAsUInt64;

#ifdef _GR_IPHONE_DEVICE_BUILD
    bool retVal = su_stat_spyBreakpointsImplemenation.makeThreadExecuteFunction((osThreadId)threadIdAsUInt64, (osProcedureAddress)&gaUpdateCurrentThreadRenderContextDataSnapshotImpl);
#else
    // This function should not be called if we do not have the spy breakpoints implementation:
    bool retVal = false;
    GT_ASSERT(false);
#endif

    apiSocket << retVal;

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateContextDataSnapshotEnd, OS_DEBUG_LOG_DEBUG);
}

// ---------------------------------------------------------------------------
// Name:        gaGetDefaultOpenGLStateVariableValueStub
// Description: Stub function for gaGetDefaultOpenGLStateVariableValue()
// Author:      Yaki Tebeka
// Date:        1/5/2007
// ---------------------------------------------------------------------------
void gaGetDefaultOpenGLStateVariableValueStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 stateVariableIdAsInt32 = 0;
    apiSocket >> stateVariableIdAsInt32;

    // Call the function implementation:
    const apParameter* pDefaultStateVariableValue = NULL;
    bool retVal = gaGetDefaultOpenGLStateVariableValueImpl((int)contextIdAsInt32, (int)stateVariableIdAsInt32, pDefaultStateVariableValue);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << *pDefaultStateVariableValue;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetOpenGLStateVariableValueStub
// Description: Stub function for gaGetOpenGLStateVariableValue()
// Author:      Yaki Tebeka
// Date:        12/7/2004
// ---------------------------------------------------------------------------
void gaGetOpenGLStateVariableValueStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 stateVariableIdAsInt32 = 0;
    apiSocket >> stateVariableIdAsInt32;

    // Call the function implementation:
    const apParameter* pStateVariableValue = NULL;
    bool retVal = gaGetOpenGLStateVariableValueImpl((int)contextIdAsInt32, (int)stateVariableIdAsInt32, pStateVariableValue);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << *pStateVariableValue;
    }
}



// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfTextureUnitsStub
// Description: Stub function for gaGetAmountOfTextureUnits
// Author:      Yaki Tebeka
// Date:        19/4/2005
// ---------------------------------------------------------------------------
void gaGetAmountOfTextureUnitsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    int amountOfTextureUnits = 1;
    bool retVal = gaGetAmountOfTextureUnitsImpl((int)contextIdAsInt32, amountOfTextureUnits);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return output arguments:
        apiSocket << (gtInt32)amountOfTextureUnits;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetActiveTextureUnitStub
// Description: Stub function for gaGetActiveTextureUnit
// Author:      Yaki Tebeka
// Date:        19/4/2005
// ---------------------------------------------------------------------------
void gaGetActiveTextureUnitStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    int activeTextureUnitId = 0;
    bool retVal = gaGetActiveTextureUnitImpl((int)contextIdAsInt32, activeTextureUnitId);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return output arguments:
        apiSocket << (gtInt32)activeTextureUnitId;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetTextureUnitNameStub
// Description: Stub function for gaGetTextureUnitName
// Author:      Yaki Tebeka
// Date:        19/4/2005
// ---------------------------------------------------------------------------
void gaGetTextureUnitNameStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 textureUnitIdAsInt32 = 0;
    apiSocket >> textureUnitIdAsInt32;

    // Call the function implementation:
    GLenum textureUnitName = GL_TEXTURE0;
    bool retVal = gaGetTextureUnitNameImpl((int)contextIdAsInt32, (int)textureUnitIdAsInt32, textureUnitName);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return output arguments:
        apiSocket << (gtInt32)textureUnitName;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetEnabledTexturingModeStub
// Description: Stub function for gaGetEnabledTexturingMode()
// Author:      Yaki Tebeka
// Date:        30/12/2004
// ---------------------------------------------------------------------------
void gaGetEnabledTexturingModeStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 textureUnitIdAsInt32 = 0;
    apiSocket >> textureUnitIdAsInt32;

    // Call the function implementation:
    bool isTexturingEnabled = false;
    apTextureType enabledTexturingMode = AP_2D_TEXTURE;
    bool retVal = gaGetEnabledTexturingModeImpl((int)contextIdAsInt32, (int)textureUnitIdAsInt32, isTexturingEnabled, enabledTexturingMode);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << isTexturingEnabled;
        apiSocket << (gtInt32)enabledTexturingMode;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfTextureObjectsStub
// Description: Stub function for gaGetAmountOfContextTextures()
// Author:      Yaki Tebeka
// Date:        30/12/2004
// ---------------------------------------------------------------------------
void gaGetAmountOfTextureObjectsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    int amountOfTextures = 0;
    bool retVal = gaGetAmountOfTextureObjectsImpl((int)contextIdAsInt32, amountOfTextures);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtInt32)amountOfTextures;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfFBOsStub
// Description: Stub function for gaGetAmountOfFBOs()
// Author:      Sigal ALgranaty
// Date:        4/6/2008
// ---------------------------------------------------------------------------
void gaGetAmountOfFBOsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    int amountOfFbos = 0;
    bool retVal = gaGetAmountOfFBOsImpl((int)contextIdAsInt32, amountOfFbos);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtInt32)amountOfFbos;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfRenderBufferObjectsStub
// Description: Stub function for gaGetAmountOfRenderBufferObjects()
// Author:      Sigal ALgranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
void gaGetAmountOfRenderBufferObjectsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    int amountOfRenderBuffers = 0;
    bool retVal = gaGetAmountOfRenderBufferObjectsImpl((int)contextIdAsInt32, amountOfRenderBuffers);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtInt32)amountOfRenderBuffers;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfPBuffersObjectsStub
// Description: Stub function for gaGetAmountOfContextPBuffers()
// Author:      Eran Zinman
// Date:        26/08/2007
// ---------------------------------------------------------------------------
void gaGetAmountOfPBuffersObjectsStub(osSocket& apiSocket)
{
    // Call the function implementation:
    int amountOfPBuffers = 0;
    bool retVal = gaGetAmountOfPBuffersObjectsImpl(amountOfPBuffers);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtInt32)amountOfPBuffers;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfSyncObjectsStub
// Description: Stub function for gaGetAmountOfSyncObjects()
// Author:      Sigal Algranaty
// Date:        28/10/2009
// ---------------------------------------------------------------------------
void gaGetAmountOfSyncObjectsStub(osSocket& apiSocket)
{
    // Call the function implementation:
    int amountOfSyncObjects = 0;
    bool retVal = gaGetAmountOfSyncObjectsImpl(amountOfSyncObjects);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtInt32)amountOfSyncObjects;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfStaticBuffersObjectsStub
// Description: Stub function for gaGetAmountOfContextStaticBuffers()
// Author:      Eran Zinman
// Date:        26/08/2007
// ---------------------------------------------------------------------------
void gaGetAmountOfStaticBuffersObjectsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    int amountOfStaticBuffers = 0;
    bool retVal = gaGetAmountOfStaticBuffersObjectsImpl((int)contextIdAsInt32, amountOfStaticBuffers);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtInt32)amountOfStaticBuffers;
    }

    // End the direct API call:
    //  suAfterDirectFunctionExecution();
}


// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadTextureRawDataStub
// Description:
//  Direct stub function for gaUpdateCurrentThreadTextureRawData.
//  (This function is executed directly using the makeThreadExecuteFunction mechanism.
//
// Author:      Eran Zinman
// Date:        1/12/2007
// ---------------------------------------------------------------------------
void gaUpdateCurrentThreadTextureRawDataStub()
{
    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_TextureRawDataStarted, OS_DEBUG_LOG_DEBUG);

    // Call the internal stub to read the parameters and call the real function
    bool retVal = gaUpdateCurrentThreadTextureRawDataInternalStub();

    // Get the API socket:
    osSocket* pApiSocket = suSpiesAPISocket();
    GT_IF_WITH_ASSERT(pApiSocket != NULL)
    {
        // Return the return value:
        *pApiSocket << retVal;
    }

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_TextureRawDataEnded, OS_DEBUG_LOG_DEBUG);

    // End the direct API call:
    suAfterDirectFunctionExecution();
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadTextureParametersStub
// Description:
//  Direct stub function for gaUpdateCurrentThreadTextureParameters.
//  (This function is executed directly using the makeThreadExecuteFunction mechanism.
//
// Author:      Sigal Algranaty
// Date:        28/10/2008
// ---------------------------------------------------------------------------
void gaUpdateCurrentThreadTextureParametersStub()
{
    // Call the internal stub to read the parameters and call the real function
    bool retVal = gaUpdateCurrentThreadTextureParametersInternalStub();

    // Get the API socket:
    osSocket* pApiSocket = suSpiesAPISocket();
    GT_IF_WITH_ASSERT(pApiSocket != NULL)
    {
        // Return the return value:
        *pApiSocket << retVal;
    }

    // End the direct API call:
    suAfterDirectFunctionExecution();
}

// ---------------------------------------------------------------------------
// Name:        gaMakeThreadUpdateTextureRawDataStub
// Description: "Direct" Stub function of gaUpdateTextureRawData, for the
//              case where we have no debugger.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
void gaMakeThreadUpdateTextureRawDataStub(osSocket& apiSocket)
{
    gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    apiSocket >> threadIdAsUInt64;

#ifdef _GR_IPHONE_DEVICE_BUILD
    bool retVal = su_stat_spyBreakpointsImplemenation.makeThreadExecuteFunction((osThreadId)threadIdAsUInt64, (osProcedureAddress)&gaUpdateCurrentThreadTextureRawDataInternalStub);
#else
    // This function should not be called if we do not have the spy breakpoints implementation:
    bool retVal = false;
    GT_ASSERT(false);

    // Call the internal stub to clear the function parameters from the API connection pipe:
    gaUpdateCurrentThreadTextureRawDataInternalStub();
#endif

    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadTextureRawDataInternalStub
// Description: The actual stub of gaUpdateTextureRawData, used by the direct
//              stub functions gaMakeThreadUpdateTextureRawDataStub and
//              gaUpdateCurrentThreadTextureRawDataStub
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
bool gaUpdateCurrentThreadTextureRawDataInternalStub()
{
    bool retVal = false;

    // Get the API socket:
    osSocket* pApiSocket = suSpiesAPISocket();
    GT_IF_WITH_ASSERT(pApiSocket != NULL)
    {
        // Get amount of textures (textures vector size)
        gtInt64 amountOfTextures = 0;
        *pApiSocket >> amountOfTextures;

        // Create the textures vector
        gtVector<apGLTextureMipLevelID> texturesVector;

        for (int i = 0; i < amountOfTextures; i++)
        {
            // Get texture id
            gtUInt32 textureNameAsUInt32 = 0;
            *pApiSocket >> textureNameAsUInt32;
            gtInt32 textureMipLevelAsInt32 = 0;
            *pApiSocket >> textureMipLevelAsInt32;
            apGLTextureMipLevelID textureId;
            textureId._textureName = (GLuint)textureNameAsUInt32;
            textureId._textureMipLevel = (int)textureMipLevelAsInt32;

            // Add it to the textures vector
            texturesVector.push_back(textureId);
        }

        // Call the real function:
        retVal = gaUpdateCurrentThreadTextureRawDataImpl(texturesVector);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaMakeThreadUpdateTextureParametersStub
// Description: "Direct" Stub function of gaUpdateTextureParameters, for the
//              case where we have no debugger.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
void gaMakeThreadUpdateTextureParametersStub(osSocket& apiSocket)
{
    gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    apiSocket >> threadIdAsUInt64;

#ifdef _GR_IPHONE_DEVICE_BUILD
    bool retVal = su_stat_spyBreakpointsImplemenation.makeThreadExecuteFunction((osThreadId)threadIdAsUInt64, (osProcedureAddress)&gaUpdateCurrentThreadTextureParametersInternalStub);
#else
    // This function should not be called if we do not have the spy breakpoints implementation:
    bool retVal = false;
    GT_ASSERT(false);

    // Call the internal stub to clear the function parameters from the API connection pipe:
    gaUpdateCurrentThreadTextureParametersInternalStub();
#endif

    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadTextureParametersInternalStub
// Description: The actual stub of gaUpdateTextureParameters, used by the direct
//              stub functions gaMakeThreadUpdateTextureParametersStub and
//              gaUpdateCurrentThreadTextureParametersStub
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
bool gaUpdateCurrentThreadTextureParametersInternalStub()
{
    bool retVal = false;

    // Get the API socket:
    osSocket* pApiSocket = suSpiesAPISocket();
    GT_IF_WITH_ASSERT(pApiSocket != NULL)
    {
        // Get the update only memory parameters flag:
        bool shouldUpdateOnlyMemoryParams = false;
        *pApiSocket >> shouldUpdateOnlyMemoryParams;

        // Get amount of textures (textures vector size)
        gtInt64 amountOfTextures = 0;
        *pApiSocket >> amountOfTextures;

        // Create the textures vector
        gtVector<apGLTextureMipLevelID> texturesVector;

        for (int i = 0; i < amountOfTextures; i++)
        {
            // Read the texture id from the socket:
            gtUInt32 textureNameAsUInt32 = 0;
            *pApiSocket >> textureNameAsUInt32;
            gtInt32 textureMipLevelAsInt32 = 0;
            *pApiSocket >> textureMipLevelAsInt32;
            apGLTextureMipLevelID textureId;
            textureId._textureName = (GLuint)textureNameAsUInt32;
            textureId._textureMipLevel = (int)textureMipLevelAsInt32;

            // Add it to the textures vector
            texturesVector.push_back(textureId);
        }

        // Call the real function:
        retVal = gaUpdateCurrentThreadTextureParametersImpl(texturesVector, shouldUpdateOnlyMemoryParams);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateRenderBufferRawDataStub
// Description: Stub function for gaUpdateRenderBufferRawData.
// Author:      Yaki Tebeka
// Date:        30/11/2008
// ---------------------------------------------------------------------------
void gaUpdateRenderBufferRawDataStub(osSocket& apiSocket)
{
    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateRenderBufferDataStarted, OS_DEBUG_LOG_DEBUG);

    // Get the context id:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Get amount of textures (textures vector size)
    gtInt64 amountOfRenderBuffers = 0;
    apiSocket >> amountOfRenderBuffers;

    // Create the render buffers vector
    gtVector<GLuint> renderBuffersVector;

    for (int i = 0; i < amountOfRenderBuffers; i++)
    {
        // Get texture id
        gtUInt32 renderBufferIdAsUInt32 = 0;
        apiSocket >> renderBufferIdAsUInt32;

        // Add it to the textures vector
        renderBuffersVector.push_back((GLuint)renderBufferIdAsUInt32);
    }

    // Call the real function:
    bool retVal = gaUpdateRenderBufferRawDataImpl((int)contextIdAsInt32, renderBuffersVector);

    // Return the return value:
    apiSocket << retVal;

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateRenderBufferDataEnded, OS_DEBUG_LOG_DEBUG);
}



// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadRenderBufferRawDataStub
// Description: Direct stub function for gaUpdateCurrentThreadRenderBufferRawData.
//              (This function is executed directly using the makeThreadExecuteFunction mechanism.
// Return Val: void
// Author:      Sigal Algranaty
// Date:        27/5/2008
// ---------------------------------------------------------------------------
void gaUpdateCurrentThreadRenderBufferRawDataStub()
{
    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateRenderBufferDataStarted, OS_DEBUG_LOG_DEBUG);

    // Call the internal stub to read the parameters and call the real function
    bool retVal = gaUpdateCurrentThreadRenderBufferRawDataInternalStub();

    // Get the API socket:
    osSocket* pApiSocket = suSpiesAPISocket();
    GT_IF_WITH_ASSERT(pApiSocket != NULL)
    {
        // Return the return value:
        *pApiSocket << retVal;
    }

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateRenderBufferDataEnded, OS_DEBUG_LOG_DEBUG);

    // End the direct API call:
    suAfterDirectFunctionExecution();
}

// ---------------------------------------------------------------------------
// Name:        gaMakeThreadUpdateRenderBufferRawDataStub
// Description: "Direct" Stub function of gaUpdateRenderBufferRawData, for the
//              case where we have no debugger.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
void gaMakeThreadUpdateRenderBufferRawDataStub(osSocket& apiSocket)
{
    gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    apiSocket >> threadIdAsUInt64;

#ifdef _GR_IPHONE_DEVICE_BUILD
    bool retVal = su_stat_spyBreakpointsImplemenation.makeThreadExecuteFunction((osThreadId)threadIdAsUInt64, (osProcedureAddress)&gaUpdateCurrentThreadRenderBufferRawDataInternalStub);
#else
    // This function should not be called if we do not have the spy breakpoints implementation:
    bool retVal = false;
    GT_ASSERT(false);

    // Call the internal stub to clear the function parameters from the API connection pipe:
    gaUpdateCurrentThreadRenderBufferRawDataInternalStub();
#endif

    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadRenderBufferRawDataInternalStub
// Description: The actual stub of gaUpdateRenderBufferRawData, used by the direct
//              stub functions gaMakeThreadUpdateRenderBufferRawDataStub and
//              gaUpdateCurrentThreadRenderBufferRawDataStub
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
bool gaUpdateCurrentThreadRenderBufferRawDataInternalStub()
{
    bool retVal = false;

    // Get the API socket:
    osSocket* pApiSocket = suSpiesAPISocket();
    GT_IF_WITH_ASSERT(pApiSocket != NULL)
    {
        // Get amount of textures (textures vector size)
        gtInt64 amountOfRenderBuffers = 0;
        *pApiSocket >> amountOfRenderBuffers;

        // Create the render buffers vector
        gtVector<GLuint> renderBuffersVector;

        for (int i = 0; i < amountOfRenderBuffers; i++)
        {
            // Get texture id
            gtUInt32 renderBufferIdAsUInt32 = 0;
            *pApiSocket >> renderBufferIdAsUInt32;

            // Add it to the textures vector
            renderBuffersVector.push_back((GLuint)renderBufferIdAsUInt32);
        }

        // Call the real function:
        retVal = gaUpdateCurrentThreadRenderBufferRawDataImpl(renderBuffersVector);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateVBORawDataStub
// Description: Stub function for gaUpdateVBORawData.
// Author:      Sigal Algranaty
// Date:        6/4/2009
// ---------------------------------------------------------------------------
void gaUpdateVBORawDataStub(osSocket& apiSocket)
{
    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateVBODataStarted, OS_DEBUG_LOG_DEBUG);

    // Get the context id:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Get amount of VBOs (VBO vector size):
    gtInt64 amountOfVBOs = 0;
    apiSocket >> amountOfVBOs;

    // Create the render buffers vector
    gtVector<GLuint> vboNamesVector;

    for (int i = 0; i < amountOfVBOs; i++)
    {
        // Get VBO name:
        gtUInt32 vboName = 0;
        apiSocket >> vboName;

        // Add it to the textures vector
        vboNamesVector.push_back((GLuint)vboName);
    }

    // Call the real function:
    bool retVal = gaUpdateVBORawDataImpl((int)contextIdAsInt32, vboNamesVector);

    // Return the return value:
    apiSocket << retVal;

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateVBODataEnded, OS_DEBUG_LOG_DEBUG);
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadVBORawDataStub
// Description: Direct stub function for gaUpdateCurrentThreadVBORawData.
//              (This function is executed directly using the makeThreadExecuteFunction mechanism.
// Return Val: void
// Author:      Sigal Algranaty
// Date:        6/4/2009
// ---------------------------------------------------------------------------
void gaUpdateCurrentThreadVBORawDataStub()
{
    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateVBODataStarted, OS_DEBUG_LOG_DEBUG);

    // Call the internal stub to read the parameters and call the real function
    bool retVal = gaUpdateCurrentThreadVBORawDataInternalStub();

    // Get the API socket:
    osSocket* pApiSocket = suSpiesAPISocket();
    GT_IF_WITH_ASSERT(pApiSocket != NULL)
    {
        // Return the return value:
        *pApiSocket << retVal;
    }

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateVBODataEnded, OS_DEBUG_LOG_DEBUG);

    // End the direct API call:
    suAfterDirectFunctionExecution();
}

// ---------------------------------------------------------------------------
// Name:        gaMakeThreadUpdateVBORawDataStub
// Description: "Direct" Stub function of gaUpdateVBORawData, for the
//              case where we have no debugger.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
void gaMakeThreadUpdateVBORawDataStub(osSocket& apiSocket)
{
    gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    apiSocket >> threadIdAsUInt64;

#ifdef _GR_IPHONE_DEVICE_BUILD
    bool retVal = su_stat_spyBreakpointsImplemenation.makeThreadExecuteFunction((osThreadId)threadIdAsUInt64, (osProcedureAddress)&gaUpdateCurrentThreadVBORawDataInternalStub);
#else
    // This function should not be called if we do not have the spy breakpoints implementation:
    bool retVal = false;
    GT_ASSERT(false);

    // Call the internal stub to clear the function parameters from the API connection pipe:
    gaUpdateCurrentThreadVBORawDataInternalStub();
#endif

    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadVBORawDataInternalStub
// Description: The actual stub of gaUpdateVBORawData, used by the direct
//              stub functions gaMakeThreadUpdateVBORawDataStub and
//              gaUpdateCurrentThreadVBORawDataStub
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
bool gaUpdateCurrentThreadVBORawDataInternalStub()
{
    bool retVal = false;

    // Get the API socket:
    osSocket* pApiSocket = suSpiesAPISocket();
    GT_IF_WITH_ASSERT(pApiSocket != NULL)
    {
        // Get amount of vbos (VBOs vector size)
        gtInt64 amountOfVBOs = 0;
        *pApiSocket >> amountOfVBOs;

        // Create the VBOs vector:
        gtVector<GLuint> vboNamesVector;

        for (int i = 0; i < amountOfVBOs; i++)
        {
            // Get VBO name:
            gtUInt32 vboNameAsUInt32 = 0;
            *pApiSocket >> vboNameAsUInt32;

            // Add it to the textures vector
            vboNamesVector.push_back((GLuint)vboNameAsUInt32);
        }

        // Call the real function:
        retVal = gaUpdateCurrentThreadVBORawDataImpl(vboNamesVector);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetVBODisplayPropertiesStub
// Description: Stub function for gaSetVBODisplayProperties.
// Author:      Sigal Algranaty
// Date:        26/4/2009
// ---------------------------------------------------------------------------
void gaSetVBODisplayPropertiesStub(osSocket& apiSocket)
{
    // Get the context id:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Get VBO name:
    gtUInt32 vboNameAsUInt32 = 0;
    apiSocket >> vboNameAsUInt32;

    // Get VBO display format:
    gtInt32 bufferDisplayFormatAsInt32 = OA_TEXEL_FORMAT_UNKNOWN;
    apiSocket >> bufferDisplayFormatAsInt32;
    oaTexelDataFormat bufferDisplayFormat = (oaTexelDataFormat)bufferDisplayFormatAsInt32;

    // Get the VBO offsetAsUInt64:
    gtUInt64 offsetAsUInt64 = 0;
    apiSocket >> offsetAsUInt64;

    // Get the VBO strideAsUInt64:
    gtUInt64 strideAsUInt64 = 0;
    apiSocket >> strideAsUInt64;

    // Call the real function:
    bool retVal = gaSetVBODisplayPropertiesImpl((int)contextIdAsInt32, (GLuint)vboNameAsUInt32, bufferDisplayFormat, (int)offsetAsUInt64, (GLsizei)strideAsUInt64);

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetCurrentThreadVBODisplayPropertiesStub
// Description: Direct stub function for gaSetCurrentThreadVBODisplayProperties.
//              (This function is executed directly using the makeThreadExecuteFunction mechanism).
// Return Val: void
// Author:      Sigal Algranaty
// Date:        26/4/2009
// ---------------------------------------------------------------------------
void gaSetCurrentThreadVBODisplayPropertiesStub()
{
    // Call the internal stub to read the parameters and call the real function
    bool retVal = gaSetCurrentThreadVBODisplayPropertiesInternalStub();

    // Get the API socket:
    osSocket* pApiSocket = suSpiesAPISocket();
    GT_IF_WITH_ASSERT(pApiSocket != NULL)
    {
        // Return the return value:
        *pApiSocket << retVal;
    }

    // End the direct API call:
    suAfterDirectFunctionExecution();
}

// ---------------------------------------------------------------------------
// Name:        gaMakeThreadSetVBODisplayPropertiesStub
// Description: "Direct" Stub function of gaSetVBODisplayProperties, for the
//              case where we have no debugger.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
void gaMakeThreadSetVBODisplayPropertiesStub(osSocket& apiSocket)
{
    gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    apiSocket >> threadIdAsUInt64;

#ifdef _GR_IPHONE_DEVICE_BUILD
    bool retVal = su_stat_spyBreakpointsImplemenation.makeThreadExecuteFunction((osThreadId)threadIdAsUInt64, (osProcedureAddress)&gaSetCurrentThreadVBODisplayPropertiesInternalStub);
#else
    // This function should not be called if we do not have the spy breakpoints implementation:
    bool retVal = false;
    GT_ASSERT(false);

    // Call the internal stub to clear the function parameters from the API connection pipe:
    gaSetCurrentThreadVBODisplayPropertiesInternalStub();
#endif

    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetCurrentThreadVBODisplayPropertiesInternalStub
// Description: The actual stub of gaSetVBODisplayProperties, used by the direct
//              stub functions gaMakeThreadSetVBODisplayPropertiesStub and
//              gaSetCurrentThreadVBODisplayPropertiesStub
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
bool gaSetCurrentThreadVBODisplayPropertiesInternalStub()
{
    bool retVal = false;

    // Get the API socket:
    osSocket* pApiSocket = suSpiesAPISocket();
    GT_IF_WITH_ASSERT(pApiSocket != NULL)
    {
        // Get VBO name:
        gtUInt32 vboNameAsUInt32 = 0;
        *pApiSocket >> vboNameAsUInt32;

        // Get VBO display format:
        gtInt32 bufferDisplayFormatAsInt32 = OA_TEXEL_FORMAT_UNKNOWN;
        *pApiSocket >> bufferDisplayFormatAsInt32;
        oaTexelDataFormat bufferDisplayFormat = (oaTexelDataFormat)bufferDisplayFormatAsInt32;

        // Get the VBO offset:
        gtUInt64 offsetAsUInt64 = 0;
        *pApiSocket >> offsetAsUInt64;

        // Get the VBO stride:
        gtUInt64 strideAsUInt64 = 0;
        *pApiSocket >> strideAsUInt64;

        // Call the real function:
        retVal = gaSetCurrentThreadVBODisplayPropertiesImpl((GLuint)vboNameAsUInt32, bufferDisplayFormat, (int)offsetAsUInt64, (GLsizei)strideAsUInt64);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadStaticBufferRawDataStub
// Description:
//  Direct stub function for gaUpdateCurrentThreadStaticBufferRawData.
//  (This function is executed directly using the makeThreadExecuteFunction mechanism.
//
// Author:      Yaki Tebeka
// Date:        16/10/2007
// ---------------------------------------------------------------------------
void gaUpdateCurrentThreadStaticBufferRawDataStub()
{
    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_StaticBufferRawDataStarted, OS_DEBUG_LOG_DEBUG);

    // Call the internal stub to read the parameters and call the real function
    bool retVal = gaUpdateCurrentThreadStaticBufferRawDataInternalStub();

    // Get the API socket:
    osSocket* pApiSocket = suSpiesAPISocket();
    GT_IF_WITH_ASSERT(pApiSocket != NULL)
    {
        // Return the return value:
        *pApiSocket << retVal;
    }

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_StaticBufferRawDataEnded, OS_DEBUG_LOG_DEBUG);

    // End the direct API call:
    suAfterDirectFunctionExecution();
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadStaticBuffersDimensionsStub
// Description:
//  Direct stub function for gaUpdateCurrentThreadStaticBuffersDimensions.
//  (This function is executed directly using the makeThreadExecuteFunction mechanism.
//
// Author:      Sigal Algranaty
// Date:        19/10/2008
// ---------------------------------------------------------------------------
void gaUpdateCurrentThreadStaticBuffersDimensionsStub()
{
    // Get the API socket:
    osSocket* pApiSocket = suSpiesAPISocket();
    GT_IF_WITH_ASSERT(pApiSocket != NULL)
    {
        // Call the real function:
        bool retVal = gaUpdateCurrentThreadStaticBuffersDimensionsImpl();

        // Return the return value:
        *pApiSocket << retVal;
    }

    // End the direct API call:
    suAfterDirectFunctionExecution();
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateStaticBufferRawDataStub
// Description:
//  Direct stub function for gaUpdateStaticBufferRawDataStub.
//
// Author:      Eran Zinman
// Date:        23/10/2007
// ---------------------------------------------------------------------------
void gaUpdateStaticBufferRawDataStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Get buffer type (as int) and convert it to apDisplayBuffer type
    gtInt32 bufferTypeAsInt32 = 0;
    apiSocket >> bufferTypeAsInt32;
    apDisplayBuffer bufferType = (apDisplayBuffer)bufferTypeAsInt32;

    // Call the function implementation:
    bool retVal = gaUpdateStaticBufferRawDataImpl((int)contextIdAsInt32, bufferType);

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateStaticBuffersDimensionsStub
// Description:
//  Direct stub function for gaUpdateStaticBuffersDimensions.
//
// Author:      Sigal Algranaty
// Date:        19/10/2008
// ---------------------------------------------------------------------------
void gaUpdateStaticBuffersDimensionsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    bool retVal = gaUpdateStaticBuffersDimensionsImpl((int)contextIdAsInt32);

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaMakeThreadUpdateStaticBufferRawDataStub
// Description: "Direct" Stub function of gaUpdateStaticBufferRawData, for the
//              case where we have no debugger.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
void gaMakeThreadUpdateStaticBufferRawDataStub(osSocket& apiSocket)
{
    gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    apiSocket >> threadIdAsUInt64;

#ifdef _GR_IPHONE_DEVICE_BUILD
    bool retVal = su_stat_spyBreakpointsImplemenation.makeThreadExecuteFunction((osThreadId)threadIdAsUInt64, (osProcedureAddress)&gaUpdateCurrentThreadStaticBufferRawDataInternalStub);
#else
    // This function should not be called if we do not have the spy breakpoints implementation:
    bool retVal = false;
    GT_ASSERT(false);

    // Call the internal stub to clear the function parameters from the API connection pipe:
    gaUpdateCurrentThreadStaticBufferRawDataInternalStub();
#endif

    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadStaticBufferRawDataInternalStub
// Description: The actual stub of gaUpdateStaticBufferRawData, used by the direct
//              stub functions gaMakeThreadUpdateStaticBufferRawDataStub and
//              gaUpdateCurrentThreadStaticBufferRawDataStub
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
bool gaUpdateCurrentThreadStaticBufferRawDataInternalStub()
{
    bool retVal = false;

    // Get the API socket:
    osSocket* pApiSocket = suSpiesAPISocket();
    GT_IF_WITH_ASSERT(pApiSocket != NULL)
    {
        // Get the static buffer type:
        gtInt32 bufferTypeAsInt32 = AP_DISPLAY_BUFFER_UNKNOWN;
        *pApiSocket >> bufferTypeAsInt32;
        apDisplayBuffer bufferType = (apDisplayBuffer)bufferTypeAsInt32;

        // Call the real function:
        retVal = gaUpdateCurrentThreadStaticBufferRawDataImpl(bufferType);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaMakeThreadUpdateStaticBuffersDimensionsStub
// Description: "Direct" Stub function of gaUpdateStaticBuffersDimensions, for the
//              case where we have no debugger.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
void gaMakeThreadUpdateStaticBuffersDimensionsStub(osSocket& apiSocket)
{
    gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    apiSocket >> threadIdAsUInt64;

#ifdef _GR_IPHONE_DEVICE_BUILD
    bool retVal = su_stat_spyBreakpointsImplemenation.makeThreadExecuteFunction((osThreadId)threadIdAsUInt64, (osProcedureAddress)&gaUpdateCurrentThreadStaticBuffersDimensionsStub);
#else
    // This function should not be called if we do not have the spy breakpoints implementation:
    bool retVal = false;
    GT_ASSERT(false);
#endif

    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdatePBuffersDimensionsStub
// Description:
//  Direct stub function for gaUpdateStaticBuffersDimensions.
//
// Author:      Sigal Algranaty
// Date:        19/10/2008
// ---------------------------------------------------------------------------
void gaUpdatePBuffersDimensionsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    bool retVal = gaUpdatePBuffersDimensionsImpl((int)contextIdAsInt32);

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadPBuffersDimensionsStub
// Description:
//  Direct stub function for gaUpdateCurrentThreadPBuffersDimensions.
//  (This function is executed directly using the makeThreadExecuteFunction mechanism.
//
// Author:      Sigal Algranaty
// Date:        19/10/2008
// ---------------------------------------------------------------------------
void gaUpdateCurrentThreadPBuffersDimensionsStub()
{
    // Get the API socket:
    osSocket* pApiSocket = suSpiesAPISocket();
    GT_IF_WITH_ASSERT(pApiSocket != NULL)
    {
        // Call the real function:
        bool retVal = gaUpdateCurrentThreadPBuffersDimensionsImpl();

        // Return the return value:
        *pApiSocket << retVal;
    }

    // End the direct API call:
    suAfterDirectFunctionExecution();
}

// ---------------------------------------------------------------------------
// Name:        gaMakeThreadUpdatePBuffersDimensionsStub
// Description: "Direct" Stub function of gaUpdatePBuffersDimensionsStub, for the
//              case where we have no debugger.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
void gaMakeThreadUpdatePBuffersDimensionsStub(osSocket& apiSocket)
{
    gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    apiSocket >> threadIdAsUInt64;

#ifdef _GR_IPHONE_DEVICE_BUILD
    bool retVal = su_stat_spyBreakpointsImplemenation.makeThreadExecuteFunction((osThreadId)threadIdAsUInt64, (osProcedureAddress)&gaUpdateCurrentThreadPBuffersDimensionsImpl);
#else
    // This function should not be called if we do not have the spy breakpoints implementation:
    bool retVal = false;
    GT_ASSERT(false);
#endif

    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdatePBufferStaticBufferRawDataStub
// Description:
//  Stub function for gaUpdatePBufferStaticBufferRawData.
//
// Author:      Eran Zinman
// Date:        25/1/2008
// ---------------------------------------------------------------------------
void gaUpdatePBufferStaticBufferRawDataStub(osSocket& apiSocket)
{
    // Get the PBuffer id:
    gtInt32 pbufferIDAsInt32 = 0;
    apiSocket >> pbufferIDAsInt32;

    // Get buffer type (as int) and convert it to apDisplayBuffer type
    gtInt32 bufferTypeAsInt32 = AP_DISPLAY_BUFFER_UNKNOWN;
    apiSocket >> bufferTypeAsInt32;
    apDisplayBuffer bufferType = (apDisplayBuffer)bufferTypeAsInt32;

    // Call the function implementation:
    bool retVal = gaUpdatePBufferStaticBufferRawDataImpl((int)pbufferIDAsInt32, bufferType);

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadPBufferStaticBufferRawDataStub
// Description:
//  Direct stub function for gaUpdateCurrentThreadPBufferStaticBufferRawData.
//
// Author:      Eran Zinman
// Date:        19/1/2008
// ---------------------------------------------------------------------------
void gaUpdateCurrentThreadPBufferStaticBufferRawDataStub()
{
    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_PBufferRawDataStarted, OS_DEBUG_LOG_DEBUG);

    // Call the internal stub to read the parameters and call the real function
    bool retVal = gaUpdateCurrentThreadPBufferStaticBufferRawDataInternalStub();

    // Get the API socket:
    osSocket* pApiSocket = suSpiesAPISocket();
    GT_IF_WITH_ASSERT(pApiSocket != NULL)
    {
        // Return the return value:
        *pApiSocket << retVal;
    }

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_PBufferRawDataEnded, OS_DEBUG_LOG_DEBUG);

    // End the direct API call:
    suAfterDirectFunctionExecution();
}

// ---------------------------------------------------------------------------
// Name:        gaMakeThreadUpdatePBufferStaticBufferRawDataStub
// Description: "Direct" Stub function of gaUpdateStaticBuffersDimensions, for the
//              case where we have no debugger.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
void gaMakeThreadUpdatePBufferStaticBufferRawDataStub(osSocket& apiSocket)
{
    gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    apiSocket >> threadIdAsUInt64;

#ifdef _GR_IPHONE_DEVICE_BUILD
    bool retVal = su_stat_spyBreakpointsImplemenation.makeThreadExecuteFunction((osThreadId)threadIdAsUInt64, (osProcedureAddress)&gaUpdateCurrentThreadPBufferStaticBufferRawDataInternalStub);
#else
    // This function should not be called if we do not have the spy breakpoints implementation:
    bool retVal = false;
    GT_ASSERT(false);

    // Call the internal stub to clear the function parameters from the API connection pipe:
    gaUpdateCurrentThreadPBufferStaticBufferRawDataInternalStub();
#endif

    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateCurrentThreadPBufferStaticBufferRawDataInternalStub
// Description: The actual stub of gaUpdatePBufferStaticBufferRawData, used by the direct
//              stub functions gaMakeThreadUpdatePBufferStaticBufferRawDataStub and
//              gaUpdateCurrentThreadPBufferStaticBufferRawDataStub
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
bool gaUpdateCurrentThreadPBufferStaticBufferRawDataInternalStub()
{
    bool retVal = false;

    // Get the API socket:
    osSocket* pApiSocket = suSpiesAPISocket();
    GT_IF_WITH_ASSERT(pApiSocket != NULL)
    {
        // Get PBuffer id
        gtInt32 pbufferIDAsInt32 = 0;
        *pApiSocket >> pbufferIDAsInt32;

        // Get the static buffer type:
        gtInt32 bufferTypeAsInt32 = AP_DISPLAY_BUFFER_UNKNOWN;
        *pApiSocket >> bufferTypeAsInt32;
        apDisplayBuffer bufferType = (apDisplayBuffer)bufferTypeAsInt32;

        // Call the real function:
        retVal = gaUpdateCurrentThreadPBufferStaticBufferRawDataImpl((int)pbufferIDAsInt32, bufferType);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetBoundTextureStub
// Description: Stub function for gaGetBoundTexture()
// Author:      Yaki Tebeka
// Date:        30/12/2004
// ---------------------------------------------------------------------------
void gaGetBoundTextureStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 textureUnitIdAsInt32 = 0;
    apiSocket >> textureUnitIdAsInt32;

    gtInt32 bindTargetAsInt32 = AP_2D_TEXTURE;
    apiSocket >> bindTargetAsInt32;
    apTextureType bindTarget = (apTextureType)bindTargetAsInt32;

    // Call the function implementation:
    GLuint textureName = 0;
    bool retVal = gaGetBoundTextureImpl((int)contextIdAsInt32, (int)textureUnitIdAsInt32, bindTarget, textureName);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtUInt32)textureName;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetFBONameStub
// Description: Stub function for gaGetFBOName()
// Author:      Sigal Algranaty
// Date:        4/6/2008
// ---------------------------------------------------------------------------
void gaGetFBONameStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 fboIdAsInt32 = 0;
    apiSocket >> fboIdAsInt32;

    // Call the function implementation:
    GLuint fboName = 0;
    bool retVal = gaGetFBONameImpl((int)contextIdAsInt32, (int)fboIdAsInt32, fboName);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtUInt32)fboName;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetRenderBufferObjectNameStub
// Description: Stub function for gaGetRenderBufferObjectName()
// Author:      Sigal ALgranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
void gaGetRenderBufferObjectNameStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 renderBufferIdAsInt32 = 0;
    apiSocket >> renderBufferIdAsInt32;

    // Call the function implementation:
    GLuint renderBufferName = 0;
    bool retVal = gaGetRenderBufferObjectNameImpl((int)contextIdAsInt32, (int)renderBufferIdAsInt32, renderBufferName);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtUInt32)renderBufferName;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetTextureObjectNameStub
// Description: Stub function for gaGetTextureName()
// Author:      Yaki Tebeka
// Date:        30/12/2004
// ---------------------------------------------------------------------------
void gaGetTextureObjectNameStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 textureIdAsInt32 = 0;
    apiSocket >> textureIdAsInt32;

    // Call the function implementation:
    GLuint textureName = 0;
    bool retVal = gaGetTextureObjectNameImpl((int)contextIdAsInt32, (int)textureIdAsInt32, textureName);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtUInt32)textureName;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetTextureObjectTypeStub
// Description: Stub function for gaGetTextureName()
// Author:      Sigal Algranaty
// Date:        2/8/2009
// ---------------------------------------------------------------------------
void gaGetTextureObjectTypeStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 textureIdAsInt32 = 0;
    apiSocket >> textureIdAsInt32;

    // Call the function implementation:
    apTextureType textureType;
    bool retVal = gaGetTextureObjectTypeImpl((int)contextIdAsInt32, (int)textureIdAsInt32, textureType);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtInt32)textureType;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetStaticBufferObjectDetailsStub
// Description: Stub function for gaGetStaticBufferDetails()
// Author:      Eran Zinman
// Date:        27/07/2007
// ---------------------------------------------------------------------------
void gaGetStaticBufferObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 bufferTypeAsInt32;
    apiSocket >> bufferTypeAsInt32;
    apDisplayBuffer bufferType = (apDisplayBuffer)bufferTypeAsInt32;

    // Call the function implementation:
    const apStaticBuffer* pStaticBufferDetails = NULL;
    bool retVal = gaGetStaticBufferObjectDetailsImpl((int)contextIdAsInt32, bufferType, pStaticBufferDetails);

    // Return the return value:
    apiSocket << retVal;

    if (retVal && (pStaticBufferDetails != NULL))
    {
        pStaticBufferDetails->writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetStaticBufferObjectDetailsStub
// Description: Stub function for gaGetStaticBufferType()
// Author:      Eran Zinman
// Date:        25/11/2007
// ---------------------------------------------------------------------------
void gaGetStaticBufferTypeStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 bufferIdAsInt32 = 0;
    apiSocket >> bufferIdAsInt32;

    // Call the function implementation:
    apDisplayBuffer bufferType;
    bool retVal = gaGetStaticBufferTypeImpl((int)contextIdAsInt32, (int)bufferIdAsInt32, bufferType);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        apiSocket << (gtInt32)bufferType;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfPBufferContentObjectsStub
// Description: Stub function for gaGetAmountOfPBufferStaticBuffers()
// Author:      Eran Zinman
// Date:        26/08/2007
// ---------------------------------------------------------------------------
void gaGetAmountOfPBufferContentObjectsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 pbufferIdAsInt32 = 0;
    apiSocket >> pbufferIdAsInt32;

    // Call the function implementation:
    int amountOfStaticBuffers = 0;
    bool retVal = gaGetAmountOfPBufferStaticBuffersObjectsImpl((int)pbufferIdAsInt32, amountOfStaticBuffers);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtInt32)amountOfStaticBuffers;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetPBufferStaticBufferTypeStub
// Description: Stub function for gaGetPBufferStaticBufferType()
// Author:      Eran Zinman
// Date:        2/1/2008
// ---------------------------------------------------------------------------
void gaGetPBufferStaticBufferTypeStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 pbufferIdAsInt32 = 0;
    apiSocket >> pbufferIdAsInt32;

    gtInt32 staticBufferIterAsInt32 = 0;
    apiSocket >> staticBufferIterAsInt32;

    // Call the function implementation:
    apDisplayBuffer bufferType;
    bool retVal = gaGetPBufferStaticBufferTypeImpl((int)pbufferIdAsInt32, (int)staticBufferIterAsInt32, bufferType);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        gtInt32 bufferTypeAsInt32 = (gtInt32)bufferType;
        apiSocket << bufferTypeAsInt32;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetPBufferStaticBufferObjectDetailsStub
// Description: Stub function for gaGetPBufferStaticBufferDetails()
// Author:      Eran Zinman
// Date:        27/07/2007
// ---------------------------------------------------------------------------
void gaGetPBufferStaticBufferObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 pbufferIdAsInt32 = 0;
    apiSocket >> pbufferIdAsInt32;

    gtInt32 bufferTypeAsInt32 = AP_DISPLAY_BUFFER_UNKNOWN;
    apiSocket >> bufferTypeAsInt32;

    // Convert to apDisplayBuffer
    apDisplayBuffer bufferType = (apDisplayBuffer)bufferTypeAsInt32;

    // Call the function implementation:
    const apStaticBuffer* pStaticBufferDetails = NULL;
    bool retVal = gaGetPBufferStaticBufferObjectDetailsImpl((int)pbufferIdAsInt32, bufferType, pStaticBufferDetails);

    // Return the return value:
    apiSocket << retVal;

    GT_IF_WITH_ASSERT(retVal && (pStaticBufferDetails != NULL))
    {
        pStaticBufferDetails->writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetPBufferObjectDetailsStub
// Description: Stub function for gaGetPBufferDetails()
// Author:      Eran Zinman
// Date:        27/07/2007
// ---------------------------------------------------------------------------
void gaGetPBufferObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 pbufferIdAsInt32 = 0;
    apiSocket >> pbufferIdAsInt32;

    // Call the function implementation:
    const apPBuffer* pPBufferDetails = NULL;
    bool retVal = gaGetPBufferObjectDetailsImpl((int)pbufferIdAsInt32, pPBufferDetails);

    // Return the return value:
    apiSocket << retVal;

    if (retVal && (pPBufferDetails != NULL))
    {
        pPBufferDetails->writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetSyncObjectDetailsStub
// Description: Stub function for gaGetSyncObjectDetails()
// Author:      Sigal Algranaty
// Date:        28/10/2009
// ---------------------------------------------------------------------------
void gaGetSyncObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 syncIdAsInt32 = 0;
    apiSocket >> syncIdAsInt32;

    // Call the function implementation:
    const apGLSync* pSyncDetails = NULL;
    bool retVal = gaGetSyncObjectDetailsImpl((int)syncIdAsInt32, pSyncDetails);

    retVal = retVal && (pSyncDetails != NULL);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        pSyncDetails->writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetTextureMiplevelDataFilePathStub
// Description: Stub function for gaGetTextureMiplevelDataFilePath()
// Author:      Sigal Algranaty
// Date:        13/1/2009
// ---------------------------------------------------------------------------
void gaGetTextureMiplevelDataFilePathStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 textureNameAsUInt32 = 0;
    apiSocket >> textureNameAsUInt32;
    gtInt32 textureMipLevelAsInt32 = 0;
    apiSocket >> textureMipLevelAsInt32;
    apGLTextureMipLevelID miplevelId;
    miplevelId._textureName = (GLuint)textureNameAsUInt32;
    miplevelId._textureMipLevel = (int)textureMipLevelAsInt32;
    gtInt32 faceIndexAsInt32 = 0;
    apiSocket >> faceIndexAsInt32;

    // Call the function implementation:
    osFilePath filePath;
    bool retVal = gaGetTextureMiplevelDataFilePathImpl((int)contextIdAsInt32, miplevelId, (int)faceIndexAsInt32, filePath);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        filePath.writeSelfIntoChannel(apiSocket);
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetTextureObjectDetailsStub
// Description: Stub function for gaGetTextureDetails()
// Author:      Yaki Tebeka
// Date:        30/12/2004
// ---------------------------------------------------------------------------
void gaGetTextureObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 textureNameAsUint32 = 0;
    apiSocket >> textureNameAsUint32;

    // Call the function implementation:
    const apGLTexture* pTextureDetails = NULL;
    bool retVal = gaGetTextureObjectDetailsImpl((int)contextIdAsInt32, (GLuint)textureNameAsUint32, pTextureDetails);

    // Return the return value:
    apiSocket << retVal;

    if (retVal && (pTextureDetails != NULL))
    {
        pTextureDetails->writeSelfIntoChannel(apiSocket);
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetTextureThumbnailDataObjectDetailsStub
// Description: Stub function for gaGetTextureThumbnailDataObjectDetails()
// Author:      Sigal Algranaty
// Date:        19/11/2008
// ---------------------------------------------------------------------------
void gaGetTextureThumbnailDataObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 textureNameAsUInt32 = 0;
    apiSocket >> textureNameAsUInt32;

    // Call the function implementation:
    const apGLTexture* pTextureDetails = NULL;
    bool retVal = gaGetTextureObjectDetailsImpl((int)contextIdAsInt32, (GLuint)textureNameAsUInt32, pTextureDetails);

    // Return the return value:
    apiSocket << retVal;

    if (retVal && (pTextureDetails != NULL))
    {
        pTextureDetails->writeThumbnailDataToChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetTextureDataObjectDetailsStub
// Description: Stub function for gaGetTextureDataObjectDetails()
// Author:      Sigal Algranaty
// Date:        16/5/2010
// ---------------------------------------------------------------------------
void gaGetTextureDataObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 textureNameAsUInt32 = 0;
    apiSocket >> textureNameAsUInt32;

    // Call the function implementation:
    const apGLTexture* pTextureDetails = NULL;
    bool retVal = gaGetTextureObjectDetailsImpl((int)contextIdAsInt32, (GLuint)textureNameAsUInt32, pTextureDetails);

    // Return the return value:
    apiSocket << retVal;

    if (retVal && (pTextureDetails != NULL))
    {
        pTextureDetails->writeTextureDataToChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetTextureMemoryDataObjectDetailsStub
// Description: Stub function for gaGetTextureMemoryDataObjectDetails()
// Author:      Sigal Algranaty
// Date:        24/5/2010
// ---------------------------------------------------------------------------
void gaGetTextureMemoryDataObjectDetails(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 textureNameAsUInt32 = 0;
    apiSocket >> textureNameAsUInt32;

    // Call the function implementation:
    const apGLTexture* pTextureDetails = NULL;
    bool retVal = gaGetTextureObjectDetailsImpl((int)contextIdAsInt32, (GLuint)textureNameAsUInt32, pTextureDetails);

    // Return the return value:
    apiSocket << retVal;

    if (retVal && (pTextureDetails != NULL))
    {
        pTextureDetails->writeTextureMemoryDataToChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetRenderBufferObjectDetailsStub
// Description: Stub function for gaGetRenderBufferObjectDetails()
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
void gaGetRenderBufferObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 renderBufferNameAsUInt32 = 0;
    apiSocket >> renderBufferNameAsUInt32;

    // Call the function implementation:
    const apGLRenderBuffer* pRenderBufferDetails = NULL;
    bool retVal = gaGetRenderBufferObjectDetailsImpl((int)contextIdAsInt32, (GLuint)renderBufferNameAsUInt32, pRenderBufferDetails);

    // Return the return value:
    apiSocket << retVal;

    if (retVal && (pRenderBufferDetails != NULL))
    {
        pRenderBufferDetails->writeSelfIntoChannel(apiSocket);
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetActiveFBOStub
// Description: Stub function for gaGetActiveFBO()
// Author:      Sigal Algranaty
// Date:        11/6/2008
// ---------------------------------------------------------------------------
void gaGetActiveFBOStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    GLuint fboName = 0;
    bool retVal = gaGetActiveFBOImpl((int)contextIdAsInt32, fboName);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        apiSocket << (gtUInt32)fboName;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetFBODetailsStub
// Description: Stub function for gaGetFBODetails()
// Author:      Sigal Algranaty
// Date:        04/6/2008
// ---------------------------------------------------------------------------
void gaGetFBODetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 fboNameAsUInt32 = 0;
    apiSocket >> fboNameAsUInt32;

    // Call the function implementation:
    const apGLFBO* pFboDetails = NULL;
    bool retVal = gaGetFBODetailsImpl((int)contextIdAsInt32, (GLuint)fboNameAsUInt32, pFboDetails);

    // Return the return value:

    apiSocket << retVal;

    if (retVal && (pFboDetails != NULL))
    {
        pFboDetails->writeSelfIntoChannel(apiSocket);
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfVBOsStub
// Description: Stub function for gaGetAmountOfVBOs()
// Author:      Sigal Algranaty
// Date:        22/10/2008
// ---------------------------------------------------------------------------
void gaGetAmountOfVBOsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    int amountOfVbos = 0;
    bool retVal = gaGetAmountOfVBOsImpl((int)contextIdAsInt32, amountOfVbos);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtInt32)amountOfVbos;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetVBODetailsStub
// Description: Stub function for gaGetVBODetails()
// Author:      Sigal Algranaty
// Date:        22/10/2008
// ---------------------------------------------------------------------------
void gaGetVBODetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 vboNameAsUInt32;
    apiSocket >> vboNameAsUInt32;

    // Call the function implementation:
    const apGLVBO* pVboDetails = NULL;
    bool retVal = gaGetVBODetailsImpl((int)contextIdAsInt32, (GLuint)vboNameAsUInt32, pVboDetails);

    retVal = retVal && (nullptr != pVboDetails);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        pVboDetails->writeSelfIntoChannel(apiSocket);
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetVBODetailsStub
// Description: Stub function for gaGetVBODetails()
// Author:      Sigal Algranaty
// Date:        22/10/2008
// ---------------------------------------------------------------------------
void gaGetVBOAttachmentStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 vboName = 0;
    apiSocket >> vboName;

    // Call the function implementation:
    GLenum vboLastTarget = 0;
    gtVector<GLenum> vboCurrentTargets;
    bool retVal = gaGetVBOAttachmentImpl((int)contextIdAsInt32, (GLuint)vboName, vboLastTarget, vboCurrentTargets);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        apiSocket << (gtUInt32)vboLastTarget;

        size_t currentTargetCount = vboCurrentTargets.size();
        apiSocket << (gtUInt32)currentTargetCount;

        for (size_t i = 0; currentTargetCount > i; ++i)
        {
            apiSocket << (gtUInt32)vboCurrentTargets[i];
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetVBONameStub
// Description: Stub function for gaGetVBOName()
// Author:      Sigal Algranaty
// Date:        22/10/2008
// ---------------------------------------------------------------------------
void gaGetVBONameStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 vboIdAsInt32 = 0;
    apiSocket >> vboIdAsInt32;

    // Call the function implementation:
    GLuint vboName = 0;
    bool retVal = gaGetVBONameImpl((int)contextIdAsInt32, (int)vboIdAsInt32, vboName);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtUInt32)vboName;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaMarkAllTextureImagesAsUpdatedStub
// Description: Stub function for gaMarkAllTextureImagesAsUpdated()
// Author:      Eran Zinman
// Date:        5/2/2008
// ---------------------------------------------------------------------------
void gaMarkAllTextureImagesAsUpdatedStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 textureIdAsInt32 = 0;
    apiSocket >> textureIdAsInt32;

    // Call the function implementation:
    bool retVal = gaMarkAllTextureImagesAsUpdatedImpl((int)contextIdAsInt32, (int)textureIdAsInt32);

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaIsTextureImagesDirtyStub
// Description: Stub function for gaIsTextureImagesDirty()
// Author:      Eran Zinman
// Date:        6/2/2008
// ---------------------------------------------------------------------------
void gaIsTextureImageDirtyStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Get texture Id
    gtUInt32 textureNameAsUInt32 = 0;
    apiSocket >> textureNameAsUInt32;
    gtInt32 textureMipLevelAsInt32 = 0;
    apiSocket >> textureMipLevelAsInt32;
    apGLTextureMipLevelID textureMiplevelId;
    textureMiplevelId._textureName = (GLuint)textureNameAsUInt32;
    textureMiplevelId._textureMipLevel = (int)textureMipLevelAsInt32;

    // Check if texture images are flagged as dirty or not
    bool isDirtyImages = true, dirtyRawDataExists = true;
    bool retVal = gaIsTextureImageDirtyImpl((int)contextIdAsInt32, textureMiplevelId, isDirtyImages, dirtyRawDataExists);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return output arguments:
        apiSocket << isDirtyImages;
        apiSocket << dirtyRawDataExists;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateTextureRawDataStub
// Description: Stub function for gaUpdateTextureRawData()
// Author:      Eran Zinman
// Date:        1/12/2007
// ---------------------------------------------------------------------------
void gaUpdateTextureRawDataStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Get amount of textures (textures vector size)
    gtInt64 amountOfTextures = 0;
    apiSocket >> amountOfTextures;

    // Create the textures vector
    gtVector<apGLTextureMipLevelID> texturesVector;

    for (int i = 0; i < amountOfTextures; i++)
    {
        // Get texture id
        gtUInt32 textureNameAsUInt32 = 0;
        apiSocket >> textureNameAsUInt32;
        gtInt32 textureMipLevelAsInt32 = 0;
        apiSocket >> textureMipLevelAsInt32;
        apGLTextureMipLevelID textureId;
        textureId._textureName = (GLuint)textureNameAsUInt32;
        textureId._textureMipLevel = (int)textureMipLevelAsInt32;

        // Add it to the textures vector
        texturesVector.push_back(textureId);
    }

    // Call the function implementation:
    bool retVal = gaUpdateTextureRawDataImpl((int)contextIdAsInt32, texturesVector);

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaUpdateTextureParametersStub
// Description: Stub function for gaUpdateTextureParameters()
// Author:      Sigal Algranaty
// Date:        28/10/2008
// ---------------------------------------------------------------------------
void gaUpdateTextureParametersStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Get the update memory parameters only flag:
    bool shouldUpdateOnlyMemoryParams = false;
    apiSocket >> shouldUpdateOnlyMemoryParams;

    // Get amount of textures (textures vector size)
    gtInt64 amountOfTextures = 0;
    apiSocket >> amountOfTextures;

    // Create the textures vector
    gtVector<apGLTextureMipLevelID> texturesVector;

    for (int i = 0; i < amountOfTextures; i++)
    {
        // Get texture id:
        gtUInt32 textureNameAsUInt32 = 0;
        apiSocket >> textureNameAsUInt32;

        gtInt32 textureMipLevelAsInt32 = 0;
        apiSocket >> textureMipLevelAsInt32;
        apGLTextureMipLevelID textureId;

        textureId._textureName = (GLuint)textureNameAsUInt32;
        textureId._textureMipLevel = (int)textureMipLevelAsInt32;

        // Add it to the textures vector
        texturesVector.push_back(textureId);
    }

    // Call the function implementation:
    bool retVal = gaUpdateTextureParametersImpl((int)contextIdAsInt32, texturesVector, shouldUpdateOnlyMemoryParams);

    // Return the return value:
    apiSocket << retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfProgramObjectsStub
// Description: Stub function for gaGetAmountOfProgramObjects
// Author:      Yaki Tebeka
// Date:        27/4/2005
// ---------------------------------------------------------------------------
void gaGetAmountOfProgramObjectsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    int amountOfPrograms = 0;
    bool retVal = gaGetAmountOfProgramObjectsImpl((int)contextIdAsInt32, amountOfPrograms);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return output arguments:
        apiSocket << (gtInt32)amountOfPrograms;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetActiveProgramObjectNameStub
// Description: Stub function for gaGetActiveProgramObjectName
// Author:      Yaki Tebeka
// Date:        7/6/2005
// ---------------------------------------------------------------------------
void gaGetActiveProgramObjectNameStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    GLuint activeProgramName = 0;
    bool retVal = gaGetActiveProgramObjectNameImpl((int)contextIdAsInt32, activeProgramName);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return output arguments:
        apiSocket << (gtUInt32)activeProgramName;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetProgramObjectNameStub
// Description: Stub function for gaGetProgramObjectName
// Author:      Yaki Tebeka
// Date:        27/4/2005
// ---------------------------------------------------------------------------
void gaGetProgramObjectNameStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 programIdAsInt32 = 0;
    apiSocket >> programIdAsInt32;

    // Call the function implementation:
    GLuint programName = 0;
    bool retVal = gaGetProgramObjectNameImpl((int)contextIdAsInt32, (int)programIdAsInt32, programName);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return output arguments:
        apiSocket << (gtUInt32)programName;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetProgramObjectDetailsStub
// Description: Stub function for gaGetProgramObjectDetails
// Author:      Yaki Tebeka
// Date:        27/4/2005
// ---------------------------------------------------------------------------
void gaGetProgramObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 programNameAsUInt32 = 0;
    apiSocket >> programNameAsUInt32;

    // Call the function implementation:
    const apGLProgram* pProgramDetails = NULL;
    bool retVal = gaGetProgramObjectDetailsImpl((int)contextIdAsInt32, (GLuint)programNameAsUInt32, pProgramDetails);

    // Return the return value:
    apiSocket << retVal;

    if (retVal && (pProgramDetails != NULL))
    {
        // Return output arguments:
        pProgramDetails->writeSelfIntoChannel(apiSocket);
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetProgramActiveUniformsStub
// Description: Stub function for gaGetProgramActiveUniforms
// Author:      Yaki Tebeka
// Date:        30/5/2005
// ---------------------------------------------------------------------------
void gaGetProgramActiveUniformsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 programNameAsUInt32 = 0;
    apiSocket >> programNameAsUInt32;

    // Call the function implementation:
    const apGLItemsCollection* activeUniforms = nullptr;
    bool retVal = gaGetProgramActiveUniformsImpl((int)contextIdAsInt32, (GLuint)programNameAsUInt32, activeUniforms);
    retVal = retVal && (nullptr != activeUniforms);


    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return output arguments:
        activeUniforms->writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaLinkProgramObjectStub
// Description: Stub function for gaLinkProgramObject
// Author:      Yaki Tebeka
// Date:        15/11/2005
// ---------------------------------------------------------------------------
void gaLinkProgramObjectStub(osSocket& apiSocket)
{
    // Receive input arguments:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 programNameAsUInt32 = 0;
    apiSocket >> programNameAsUInt32;

    bool wasLinkSuccessful = false;
    gtString linkLog;
    bool retVal = gaLinkProgramObjectImpl((int)contextIdAsInt32, (GLuint)programNameAsUInt32, wasLinkSuccessful, linkLog);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return output parameters:
        apiSocket << wasLinkSuccessful;
        apiSocket << linkLog;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaLinkCurrentThreadProgramObjectStub
// Description: Direct stub function for gaLinkCurrentThreadProgramObject
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
void gaLinkCurrentThreadProgramObjectStub()
{
    // Call the internal stub to read the parameters and call the real function
    bool retVal = gaLinkCurrentThreadProgramObjectInternalStub();

    // The internal stub returns the retVal as well as the link log, so we
    // only assert the value here:
    GT_ASSERT(retVal);

    // End the direct API call:
    suAfterDirectFunctionExecution();
}

// ---------------------------------------------------------------------------
// Name:        gaMakeThreadLinkProgramObjectStub
// Description: "Direct" Stub function of gaLinkProgramObject, for the
//              case where we have no debugger.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
void gaMakeThreadLinkProgramObjectStub(osSocket& apiSocket)
{
    gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    apiSocket >> threadIdAsUInt64;

#ifdef _GR_IPHONE_DEVICE_BUILD
    bool retVal = su_stat_spyBreakpointsImplemenation.makeThreadExecuteFunction((osThreadId)threadIdAsUInt64, (osProcedureAddress)&gaLinkCurrentThreadProgramObjectInternalStub);

    // The internal stub returns the retVal as well as the link log, so we
    // only assert the value here:
    GT_ASSERT(retVal);
#else
    // This function should not be called if we do not have the spy breakpoints implementation:
    GT_ASSERT(false);

    // Call the internal stub to clear the function parameters from the API connection pipe:
    gaLinkCurrentThreadProgramObjectInternalStub();
#endif
}

// ---------------------------------------------------------------------------
// Name:        gaLinkCurrentThreadProgramObjectInternalStub
// Description: The actual stub of gaLinkProgramObject, used by the direct
//              stub functions gaMakeThreadLinkProgramObjectStub and
//              gaLinkCurrentThreadProgramObjectStub
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
bool gaLinkCurrentThreadProgramObjectInternalStub()
{
    bool retVal = false;

    // Get the API socket:
    osSocket* pAPISocket = suSpiesAPISocket();

    if (pAPISocket)
    {
        // Receive input arguments:
        gtUInt32 programNameAsUInt32 = 0;
        *pAPISocket >> programNameAsUInt32;

        bool wasLinkSuccessful = false;
        gtString linkLog;
        retVal = gaLinkCurrentThreadProgramObjectImpl((GLuint)programNameAsUInt32, wasLinkSuccessful, linkLog);

        // Return the return value:
        *pAPISocket << retVal;

        if (retVal)
        {
            // Return output parameters:
            *pAPISocket << wasLinkSuccessful;
            *pAPISocket << linkLog;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaValidateProgramObjectStub
// Description: Stub function for gaValidateProgramObject
// Author:      Yaki Tebeka
// Date:        15/11/2005
// ---------------------------------------------------------------------------
void gaValidateProgramObjectStub(osSocket& apiSocket)
{
    // Receive input arguments:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 programNameAsUInt32 = 0;
    apiSocket >> programNameAsUInt32;

    bool wasValidationSuccessful = false;
    gtString validationLog;
    bool retVal = gaValidateProgramObjectImpl((int)contextIdAsInt32, (GLuint)programNameAsUInt32, wasValidationSuccessful, validationLog);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return output parameters:
        apiSocket << wasValidationSuccessful;
        apiSocket << validationLog;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaValidateCurrentThreadProgramObjectStub
// Description: Direct stub function for gaValidateCurrentThreadProgramObject
// Author:      Uri Shomroni
// Date:        4/11/2009
// ---------------------------------------------------------------------------
void gaValidateCurrentThreadProgramObjectStub()
{
    // Call the internal stub to read the parameters and call the real function
    bool retVal = gaValidateCurrentThreadProgramObjectInternalStub();

    // The internal stub returns the retVal as well as the validation log, so we
    // only assert the value here:
    GT_ASSERT(retVal);

    // End the direct API call:
    suAfterDirectFunctionExecution();
}

// ---------------------------------------------------------------------------
// Name:        gaMakeThreadValidateProgramObjectStub
// Description: "Direct" Stub function of gaValidateProgramObject, for the
//              case where we have no debugger.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
void gaMakeThreadValidateProgramObjectStub(osSocket& apiSocket)
{
    gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    apiSocket >> threadIdAsUInt64;

#ifdef _GR_IPHONE_DEVICE_BUILD
    bool retVal = su_stat_spyBreakpointsImplemenation.makeThreadExecuteFunction((osThreadId)threadIdAsUInt64, (osProcedureAddress)&gaValidateCurrentThreadProgramObjectInternalStub);

    // The internal stub returns the retVal as well as the validation log, so we
    // only assert the value here:
    GT_ASSERT(retVal);
#else
    // This function should not be called if we do not have the spy breakpoints implementation:
    GT_ASSERT(false);

    // Call the internal stub to clear the function parameters from the API connection pipe:
    gaValidateCurrentThreadProgramObjectInternalStub();
#endif
}

// ---------------------------------------------------------------------------
// Name:        gaValidateCurrentThreadProgramObjectInternalStub
// Description: The actual stub of gaValidateProgramObject, used by the direct
//              stub functions gaMakeThreadValidateProgramObjectStub and
//              gaValidateCurrentThreadProgramObjectStub
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
bool gaValidateCurrentThreadProgramObjectInternalStub()
{
    bool retVal = false;

    // Get the API socket:
    osSocket* pAPISocket = suSpiesAPISocket();

    if (pAPISocket)
    {
        // Receive input arguments:
        gtUInt32 programNameAsUInt32 = 0;
        *pAPISocket >> programNameAsUInt32;

        bool wasValidationSuccessful = false;
        gtString validationLog;
        retVal = gaValidateCurrentThreadProgramObjectImpl((GLuint)programNameAsUInt32, wasValidationSuccessful, validationLog);

        // Return the return value:
        *pAPISocket << retVal;

        if (retVal)
        {
            // Return output parameters:
            *pAPISocket << wasValidationSuccessful;
            *pAPISocket << validationLog;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfShaderObjectsStub
// Description: Stub function for gaGetAmountOfShaderObjects
// Author:      Yaki Tebeka
// Date:        27/4/2005
// ---------------------------------------------------------------------------
void gaGetAmountOfShaderObjectsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    int amountOfShaders = 0;
    bool retVal = gaGetAmountOfShaderObjectsImpl((int)contextIdAsInt32, amountOfShaders);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return output arguments:
        apiSocket << (gtInt32)amountOfShaders;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetShaderObjectNameStub
// Description: Stub function for gaGetShaderObjectName
// Author:      Yaki Tebeka
// Date:        27/4/2005
// ---------------------------------------------------------------------------
void gaGetShaderObjectNameStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 shaderIdAsInt32 = 0;
    apiSocket >> shaderIdAsInt32;

    // Call the function implementation:
    GLuint shaderName = 0;
    bool retVal = gaGetShaderObjectNameImpl((int)contextIdAsInt32, (int)shaderIdAsInt32, shaderName);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return output arguments:
        apiSocket << (gtUInt32)shaderName;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetShaderObjectDetailsStub
// Description: Stub function for gaGetShaderObjectDetails
// Author:      Yaki Tebeka
// Date:        27/4/2005
// ---------------------------------------------------------------------------
void gaGetShaderObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 shaderNameAsUInt32 = 0;
    apiSocket >> shaderNameAsUInt32;

    // Call the function implementation:
    gtAutoPtr<apGLShaderObject> aptrShaderDetails;
    bool retVal = gaGetShaderObjectDetailsImpl((int)contextIdAsInt32, (GLuint)shaderNameAsUInt32, aptrShaderDetails);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return output arguments:
        osTransferableObject* pShaderObjDetails = aptrShaderDetails.pointedObject();
        apiSocket << *pShaderObjDetails;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaMarkShaderObjectSourceCodeAsForcedStub
// Description: Stub function for gaMarkShaderObjectSourceCodeAsForced
// Author:      Yaki Tebeka
// Date:        15/11/2005
// ---------------------------------------------------------------------------
void gaMarkShaderObjectSourceCodeAsForcedStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 shaderNameAsUInt32 = 0;
    apiSocket >> shaderNameAsUInt32;

    bool isSourceCodeForced = false;
    apiSocket >> isSourceCodeForced;

    // Call the function implementation:
    bool retVal = gaMarkShaderObjectSourceCodeAsForcedImpl((int)contextIdAsInt32, (GLuint)shaderNameAsUInt32, isSourceCodeForced);

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetShaderObjectSourceCodeStub
// Description: Stub function for gaSetShaderObjectSourceCode
// Author:      Yaki Tebeka
// Date:        20/11/2005
// ---------------------------------------------------------------------------
void gaSetShaderObjectSourceCodeStub(osSocket& apiSocket)
{
    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_SetShaderSourceCodeStarted, OS_DEBUG_LOG_DEBUG);

    // Receive input arguments:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 shaderNameAsUInt32 = 0;
    apiSocket >> shaderNameAsUInt32;

    osFilePath sourceCodeFilePath;
    sourceCodeFilePath.readSelfFromChannel(apiSocket);

    // Call the function implementation:
    bool retVal = gaSetShaderObjectSourceCodeImpl((int)contextIdAsInt32, (GLuint)shaderNameAsUInt32, sourceCodeFilePath);

    // Return the return value:
    apiSocket << retVal;

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_SetShaderSourceCodeEnded, OS_DEBUG_LOG_DEBUG);
}

// ---------------------------------------------------------------------------
// Name:        gaSetCurrentThreadShaderObjectSourceCodeStub
// Description: Direct stub function for gaSetCurrentThreadShaderObjectSourceCode
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
void gaSetCurrentThreadShaderObjectSourceCodeStub()
{
    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_SetShaderSourceCodeStarted, OS_DEBUG_LOG_DEBUG);

    // Call the internal stub to read the parameters and call the real function
    bool retVal = gaSetCurrentThreadShaderObjectSourceCodeInternalStub();
    // Get the API socket:
    osSocket* pAPISocket = suSpiesAPISocket();

    if (pAPISocket)
    {
        // Return the return value:
        *pAPISocket << retVal;
    }

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_SetShaderSourceCodeEnded, OS_DEBUG_LOG_DEBUG);

    // End the direct API call:
    suAfterDirectFunctionExecution();
}

// ---------------------------------------------------------------------------
// Name:        gaMakeThreadSetShaderObjectSourceCodeStub
// Description: "Direct" Stub function of gaSetShaderObjectSourceCode, for the
//              case where we have no debugger.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
void gaMakeThreadSetShaderObjectSourceCodeStub(osSocket& apiSocket)
{
    gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    apiSocket >> threadIdAsUInt64;

#ifdef _GR_IPHONE_DEVICE_BUILD
    bool retVal = su_stat_spyBreakpointsImplemenation.makeThreadExecuteFunction((osThreadId)threadIdAsUInt64, (osProcedureAddress)&gaSetCurrentThreadShaderObjectSourceCodeInternalStub);
#else
    // This function should not be called if we do not have the spy breakpoints implementation:
    bool retVal = false;
    GT_ASSERT(false);

    // Call the internal stub to clear the function parameters from the API connection pipe:
    gaSetCurrentThreadShaderObjectSourceCodeInternalStub();
#endif

    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetCurrentThreadShaderObjectSourceCodeInternalStub
// Description: The actual stub of gaSetShaderObjectSourceCode, used by the direct
//              stub functions gaMakeThreadSetShaderObjectSourceCodeStub and
//              gaSetCurrentThreadShaderObjectSourceCodeStub
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
bool gaSetCurrentThreadShaderObjectSourceCodeInternalStub()
{
    bool retVal = false;

    // Get the API socket:
    osSocket* pAPISocket = suSpiesAPISocket();

    if (pAPISocket)
    {
        // Receive input arguments:
        gtUInt32 shaderNameAsUInt32 = 0;
        *pAPISocket >> shaderNameAsUInt32;

        osFilePath sourceCodeFilePath;
        sourceCodeFilePath.readSelfFromChannel(*pAPISocket);

        // Call the function implementation:
        retVal = gaSetCurrentThreadShaderObjectSourceCodeImpl((GLuint)shaderNameAsUInt32, sourceCodeFilePath);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaCompileShaderObjectStub
// Description: Stub function for gaCompileShaderObject
// Author:      Yaki Tebeka
// Date:        15/11/2005
// ---------------------------------------------------------------------------
void gaCompileShaderObjectStub(osSocket& apiSocket)
{
    // Receive input arguments:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 shaderNameAsUInt32 = 0;
    apiSocket >> shaderNameAsUInt32;

    bool wasCompilationSuccessful = false;
    gtString compilationLog;
    bool retVal = gaCompileShaderObjectImpl((int)contextIdAsInt32, (GLuint)shaderNameAsUInt32, wasCompilationSuccessful, compilationLog);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return output parameters:
        apiSocket << wasCompilationSuccessful;
        apiSocket << compilationLog;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaCompileCurrentThreadShaderObjectStub
// Description: Direct stub function for gaCompileCurrentThreadShaderObject
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
void gaCompileCurrentThreadShaderObjectStub()
{
    // Call the internal stub to read the parameters and call the real function
    bool retVal = gaCompileCurrentThreadShaderObjectInternalStub();

    // The internal stub returns the retVal as well as the compilation log, so we
    // only assert the value here:
    GT_ASSERT(retVal);

    // End the direct API call:
    suAfterDirectFunctionExecution();
}

// ---------------------------------------------------------------------------
// Name:        gaMakeThreadCompileShaderObjectStub
// Description: "Direct" Stub function of gaCompileShaderObject, for the
//              case where we have no debugger.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
void gaMakeThreadCompileShaderObjectStub(osSocket& apiSocket)
{
    gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    apiSocket >> threadIdAsUInt64;

#ifdef _GR_IPHONE_DEVICE_BUILD
    bool retVal = su_stat_spyBreakpointsImplemenation.makeThreadExecuteFunction((osThreadId)threadIdAsUInt64, (osProcedureAddress)&gaCompileCurrentThreadShaderObjectInternalStub);

    // The internal stub returns the retVal as well as the validation log, so we
    // only assert the value here:
    GT_ASSERT(retVal);
#else
    // This function should not be called if we do not have the spy breakpoints implementation:
    GT_ASSERT(false);

    // Call the internal stub to clear the function parameters from the API connection pipe:
    gaCompileCurrentThreadShaderObjectInternalStub();
#endif
}

// ---------------------------------------------------------------------------
// Name:        gaCompileCurrentThreadShaderObjectInternalStub
// Description: The actual stub of gaCompileShaderObject, used by the direct
//              stub functions gaMakeThreadCompileShaderObjectStub and
//              gaCompileCurrentThreadShaderObjectStub
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        3/11/2009
// ---------------------------------------------------------------------------
bool gaCompileCurrentThreadShaderObjectInternalStub()
{
    bool retVal = false;

    // Get the API socket:
    osSocket* pAPISocket = suSpiesAPISocket();

    if (pAPISocket)
    {
        // Receive input arguments:
        gtUInt32 shaderNameAsUInt32 = 0;
        *pAPISocket >> shaderNameAsUInt32;

        bool wasCompilationSuccessful = false;
        gtString compilationLog;
        retVal = gaCompileCurrentThreadShaderObjectImpl((GLuint)shaderNameAsUInt32, wasCompilationSuccessful, compilationLog);

        // Return the return value:
        *pAPISocket << retVal;

        if (retVal)
        {
            // Return output parameters:
            *pAPISocket << wasCompilationSuccessful;
            *pAPISocket << compilationLog;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfDisplayListsStub
// Description: Stub function for gaGetAmountOfDisplayLists
// Return Val: void
// Author:      Sigal Algranaty
// Date:        24/9/2008
// ---------------------------------------------------------------------------
void gaGetAmountOfDisplayListsStub(osSocket& apiSocket)
{
    // Receive input arguments:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    int amountOfDisplayLists = 0;

    bool retVal = gaGetAmountOfDisplayListsImpl((int)contextIdAsInt32, amountOfDisplayLists);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return output parameters:
        apiSocket << (gtInt32)amountOfDisplayLists;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetDisplayListObjectNameStub
// Description: Stub function for gaGetDisplayListObjectName
// Author:      Uri Shomroni
// Date:        17/11/2008
// ---------------------------------------------------------------------------
void gaGetDisplayListObjectNameStub(osSocket& apiSocket)
{
    // Get parameters:
    gtInt32 contextIDAsInt32 = -1;
    apiSocket >> contextIDAsInt32;
    gtInt32 displayListIndexAsInt32 = -1;
    apiSocket >> displayListIndexAsInt32;

    GLuint displayListName = 0;
    bool retVal = gaGetDisplayListObjectNameImpl((int)contextIDAsInt32, (int)displayListIndexAsInt32, displayListName);

    // Receive success value:
    apiSocket << retVal;

    if (retVal)
    {
        // Receive name:
        apiSocket << (gtUInt32)displayListName;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetDisplayListObjectDetailsStub
// Description: Stub function for gaGetDisplayListObjectDetails()
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        24/9/2008
// ---------------------------------------------------------------------------
void gaGetDisplayListObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 displayListNameAsUInt32 = 0;
    apiSocket >> displayListNameAsUInt32;

    // Call the function implementation:
    const apGLDisplayList* pDisplayListDetails = NULL;
    bool retVal = gaGetDisplayListObjectDetailsImpl((int)contextIdAsInt32, (GLuint)displayListNameAsUInt32, pDisplayListDetails);

    // Return the return value:
    apiSocket << retVal;

    if (retVal && (pDisplayListDetails != NULL))
    {
        pDisplayListDetails->writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfCurrentFrameFunctionCallsStub
// Description: Stub for gaGetAmountOfCurrentFrameFunctionCalls()
// Author:      Yaki Tebeka
// Date:        24/4/2004
// ---------------------------------------------------------------------------
void gaGetAmountOfCurrentFrameFunctionCallsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    int amountOfFunctionCalls = 0;
    bool rc = gaGetAmountOfCurrentFrameFunctionCallsImpl((int)contextIdAsInt32, amountOfFunctionCalls);

    // Return the return value:
    apiSocket << rc;

    if (rc)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtInt32)amountOfFunctionCalls;
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetCurrentFrameFunctionCallStub
// Description: Stub for gaGetAmountOfCurrentFrameFunctionCalls()
// Author:      Yaki Tebeka
// Date:        26/4/2004
// ---------------------------------------------------------------------------
void gaGetCurrentFrameFunctionCallStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 callIndexAsInt32 = -1;
    apiSocket >> callIndexAsInt32;

    // Call the function implementation:
    gtAutoPtr<apFunctionCall> aptrFunctionCall;
    bool rc = gaGetCurrentFrameFunctionCallImpl((int)contextIdAsInt32, (int)callIndexAsInt32, aptrFunctionCall);

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
// Name:        gaGetCurrentFrameFunctionCallDeprecationDetailsStub
// Description: Stub for gaGetCurrentFrameFunctionCallDeprecationDetails()
// Author:      Sigal Algranaty
// Date:        4/3/2009
// ---------------------------------------------------------------------------
void gaGetCurrentFrameFunctionCallDeprecationDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 callIndexAsInt32 = -1;
    apiSocket >> callIndexAsInt32;

    // Call the function implementation:
    apFunctionDeprecation functionCallDeprecation;
    bool rc = gaGetCurrentFrameFunctionCallDeprecationDetailsImpl((int)contextIdAsInt32, (int)callIndexAsInt32, functionCallDeprecation);

    // Return the return value:
    apiSocket << rc;

    if (rc)
    {
        // Return the arguments that should be returned:
        functionCallDeprecation.writeSelfIntoChannel(apiSocket);
    }
}


// ---------------------------------------------------------------------------
// Name:        gaGetLastFunctionCallStub
// Description: Stub function for gaGetLastFunctionCall
// Author:      Yaki Tebeka
// Date:        21/6/2005
// ---------------------------------------------------------------------------
void gaGetLastFunctionCallStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    gtAutoPtr<apFunctionCall> aptrFunctionCall;
    bool retVal = gaGetLastFunctionCallImpl((int)contextIdAsInt32, aptrFunctionCall);

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
// Name:        gaFindCurrentFrameFunctionCallStub
// Description: Stub for gaFindCurrentFrameFunctionCall()
// Author:      Yaki Tebeka
// Date:        13/12/2004
// ---------------------------------------------------------------------------
void gaFindCurrentFrameFunctionCallStub(osSocket& apiSocket)
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
    bool rc = gaFindCurrentFrameFunctionCallImpl((int)contextIdAsInt32, searchDirection, (int)searchStartIndexAsInt32,
                                                 searchedString, isCaseSensitiveSearch, foundIndex);

    // Send the success value:
    apiSocket << rc;

    if (rc)
    {
        // Send the found index:
        apiSocket << (gtInt32)foundIndex;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetCurrentStatisticsStub
// Description: Stub for gaGetCurrentStatisticsStub()
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        17/7/2008
// ---------------------------------------------------------------------------
void gaGetCurrentStatisticsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Allocate a new statistics object:
    apStatistics* pStatistics = new apStatistics();


    // Call the function implementation:
    bool retVal = gaGetCurrentStatisticsImpl((int)contextIdAsInt32, pStatistics);

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
// Name:        gaGetRenderPrimitivesStatisticsStub
// Description: Stub for gaGetRenderPrimitivesStatistics()
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gaGetRenderPrimitivesStatisticsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    apRenderPrimitivesStatistics renderPrimitivesStatistics;

    // Call the function implementation:
    bool retVal = gaGetRenderPrimitivesStatisticsImpl((int)contextIdAsInt32, renderPrimitivesStatistics);

    // Send success value:
    apiSocket << retVal;

    GT_IF_WITH_ASSERT(retVal)
    {
        // Write the statistics object into the channel:
        renderPrimitivesStatistics.writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaClearFunctionCallsStatisticsStub
// Description: A stub function for gaClearFunctionCallsStatisticsImpl
// Arguments: osSocket& apiSocket
// Return Val: void
// Author:      Sigal Algranaty
// Date:        17/7/2008
// ---------------------------------------------------------------------------
void gaClearFunctionCallsStatisticsStub(osSocket& apiSocket)
{
    // Call the function implementation:
    bool retVal = gaClearFunctionCallsStatisticsImpl();

    // Send success value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaIsInOpenGLBeginEndBlockStub
// Description: Stub for gaIsInOpenGLBeginEndBlockStub()
// ---------------------------------------------------------------------------
void gaIsInOpenGLBeginEndBlockStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    bool retVal = gaIsInOpenGLBeginEndBlockImpl((int)contextIdAsInt32);

    // Send success value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaFindStringMarkerStub
// Description: Stub for gaFindCurrentFrameFunctionCall()
// ---------------------------------------------------------------------------
void gaFindStringMarkerStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 searchDirectionAsInt32 = 0;
    apiSocket >> searchDirectionAsInt32;
    apSearchDirection searchDirection = (apSearchDirection)searchDirectionAsInt32;

    gtInt32 searchStartIndexAsInt32 = 0;
    apiSocket >> searchStartIndexAsInt32;

    // Call the function implementation:
    int foundIndex = -1;
    bool rc = gaFindStringMarkerImpl((int)contextIdAsInt32, searchDirection, (int)searchStartIndexAsInt32, foundIndex);

    // Send the success value:
    apiSocket << rc;

    if (rc)
    {
        // Send the found index:
        apiSocket << (gtInt32)foundIndex;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetCurrentOpenGLErrorStub
// Description: Stub function for gaGetCurrentOpenGLError
// Author:      Yaki Tebeka
// Date:        25/8/2004
// ---------------------------------------------------------------------------
void gaGetCurrentOpenGLErrorStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    GLenum openGLError = GL_NO_ERROR;
    retVal = gaGetCurrentOpenGLErrorImpl(openGLError);

    // Return the return value:
    apiSocket << retVal;

    // Return the OpenGL error reason:
    apiSocket << (gtInt32)openGLError;
}

// ---------------------------------------------------------------------------
// Name:        gaGetContextLogFilePathStub
// Description: Stub function for gaGetRenderContextLogFilePath
// Author:      Yaki Tebeka
// Date:        23/3/2005
// ---------------------------------------------------------------------------
void gaGetContextLogFilePathStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Get the input arguments:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    bool logFileExists = false;
    osFilePath logFilePath;
    retVal = gaGetContextLogFilePathImpl((int)contextIdAsInt32, logFileExists, logFilePath);

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
// Name:        gaForceOpenGLFlushStub
// Description: Stub function for gaForceOpenGLFlushStub
// Author:      Yaki Tebeka
// Date:        11/11/2004
// ---------------------------------------------------------------------------
void gaForceOpenGLFlushStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Get the "Force OpenGL flush" state status:
    bool isOpenGLFlushForced = false;
    apiSocket >> isOpenGLFlushForced;

    // Call the function implementation:
    retVal = gaForceOpenGLFlushImpl(isOpenGLFlushForced);

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetInteractiveBreakModeStub
// Description: Stub function for gaSetInteractiveBreakMode.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        23/7/2006
// ---------------------------------------------------------------------------
void gaSetInteractiveBreakModeStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Get the "Interactive break" mode status:
    bool isInteractiveBreakOn = false;
    apiSocket >> isInteractiveBreakOn;

    // Call the function implementation:
    retVal = gaSetInteractiveBreakModeImpl(isInteractiveBreakOn);

    // Return the return value:
    apiSocket << retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaForceOpenGLPolygonRasterModeStub
// Description: Stub function for gaForceOpenGLPolygonRasterModeStub
// Author:      Yaki Tebeka
// Date:        11/11/2004
// ---------------------------------------------------------------------------
void gaForceOpenGLPolygonRasterModeStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Get the forces polygon raster mode:
    gtInt32 rasterModeAsInt32 = AP_RASTER_FILL;
    apiSocket >> rasterModeAsInt32;
    apRasterMode rasterMode = (apRasterMode)rasterModeAsInt32;

    // Call the function implementation:
    retVal = gaForceOpenGLPolygonRasterModeImpl(rasterMode);

    // Return the return value:
    apiSocket << retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaCancelOpenGLPolygonRasterModeForcingStub
// Description: Stub function for gaCancelOpenGLPolygonRasterModeForcingStub
// Author:      Yaki Tebeka
// Date:        11/11/2004
// ---------------------------------------------------------------------------
void gaCancelOpenGLPolygonRasterModeForcingStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    retVal = gaCancelOpenGLPolygonRasterModeForcingImpl();

    // Return the return value:
    apiSocket << retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaSetOpenGLNullDriverStub
// Description: Stub function for gaSetOpenGLNullDriver
// Author:      Avi Shapira
// Date:        28/2/2005
// ---------------------------------------------------------------------------
void gaSetOpenGLNullDriverStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Get the "NULL OpenGL Impl" state status:
    bool isNULLOpenGLImplOn = false;
    apiSocket >> isNULLOpenGLImplOn;

    // Call the function implementation:
    retVal = gaSetOpenGLNullDriverImpl(isNULLOpenGLImplOn);

    // Return success value:
    apiSocket << retVal;
}
// ---------------------------------------------------------------------------
// Name:        gaSetOpenGLForceStubStub
// Description: Stub function for gaSetOpenGLForce
// Author:      Avi Shapira
// Date:        28/2/2005
// ---------------------------------------------------------------------------
void gaSetOpenGLForceStubStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Get the stub type:
    apOpenGLForcedModeType stubType = AP_OPENGL_AMOUNT_OF_FORCED_STUBS;
    gtInt32 stubTypeAsint = 0;
    apiSocket >> stubTypeAsint;
    stubType = (apOpenGLForcedModeType)stubTypeAsint;

    // Get the "Is Stub Forced" status:
    bool isStubForced = false;
    apiSocket >> isStubForced;

    // Call the function implementation:
    retVal = gaSetOpenGLForceStubImpl(stubType, isStubForced);

    // Return success value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetSpyPerformanceCountersValuesStub
// Description: Stub function for gaGetSpyPerformanceCountersValues
// Author:      Yaki Tebeka
// Date:        24/7/2005
// ---------------------------------------------------------------------------
void gaGetSpyPerformanceCountersValuesStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    int amountOfValues = 0;
    const double* pValuesArray = NULL;

    retVal = gaGetSpyPerformanceCountersValuesImpl(pValuesArray, amountOfValues);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the amount of values:
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
// Name:        gaGetRemoteOSPerformanceCountersValuesStub
// Description: Stub function for gaGetRemoteOSPerformanceCountersValues
// Author:      Uri Shomroni
// Date:        23/11/2009
// ---------------------------------------------------------------------------
void gaGetRemoteOSPerformanceCountersValuesStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    int amountOfValues = 0;
    const double* pValuesArray = NULL;

    retVal = gaGetRemoteOSPerformanceCountersValuesImpl(pValuesArray, amountOfValues);

    // To for better performance, we write this in one chunk:
    osRawMemoryStream memoryStream;

    // Return the return value:
    memoryStream << retVal;

    if (retVal)
    {
        // Return the amount of values:
        memoryStream << (gtInt32)amountOfValues;

        // Return the values:
        for (int i = 0; i < amountOfValues; i++)
        {
            memoryStream << pValuesArray[i];
        }
    }

    // Write the memory stream:
    apiSocket << memoryStream;
}

// The below functions are supported on the iPhone device only:
#ifdef _GR_IPHONE_DEVICE_BUILD

// ---------------------------------------------------------------------------
// Name:        gaAddSupportediPhonePerformanceCounterStub
// Description: Stub function for gaAddSupportediPhonePerformanceCounter
// Author:      Yaki Tebeka
// Date:        28/2/2010
// ---------------------------------------------------------------------------
void gaAddSupportediPhonePerformanceCounterStub(osSocket& apiSocket)
{
    // Get the function arguments:
    gtInt32 counterIndex = 0;
    apiSocket >> counterIndex;

    gtString counterName;
    apiSocket >> counterName;

    // Call the function implementation:
    bool retVal = gaAddSupportediPhonePerformanceCounterImpl(int(counterIndex), counterName);

    // Return success value:
    apiSocket << retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaInitializeiPhonePerformanceCountersReaderStub
// Description: Stub function for gaInitializeiPhonePerformanceCountersReader
// Author:      Yaki Tebeka
// Date:        28/2/2010
// ---------------------------------------------------------------------------
void gaInitializeiPhonePerformanceCountersReaderStub(osSocket& apiSocket)
{
    // Call the function implementation:
    bool retVal = gaInitializeiPhonePerformanceCountersReaderImpl();

    // Return success value:
    apiSocket << retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetiPhonePerformanceCountersValuesStub
// Description: Stub function for gaGetiPhonePerformanceCountersValues
// Author:      Sigal Algranaty
// Date:        13/1/2010
// ---------------------------------------------------------------------------
void gaGetiPhonePerformanceCountersValuesStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    int amountOfValues = 0;
    const double* pValuesArray = NULL;

    // Get all the counters values in a single array of values
    retVal = gaGetiPhonePerformanceCountersValuesImpl(pValuesArray, amountOfValues);
    retVal = retVal && ((pValuesArray != NULL) || (amountOfValues == 0));

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

#endif // _GR_IPHONE_DEVICE_BUILD


// ---------------------------------------------------------------------------
// Name:        gaGetATIPerformanceCountersValuesStub
// Description: Stub function for gaGetATIPerformanceCountersValues
// Author:      Sigal Algranaty
// Date:        23/03/2008
// ---------------------------------------------------------------------------
void gaGetATIPerformanceCountersValuesStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Call the function implementation:
    int amountOfValues = 0;
    const double* pValuesArray = NULL;

    // Get all the counters values in a single array of values
    retVal = gaGetATIPerformanceCountersValuesImpl(pValuesArray, amountOfValues);

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
// Name:        gaActivateATIPerformanceCountersStub
// Description: Stub function for gaActivateATIPerformanceCounter
// Author:      Sigal Algranaty
// Date:        27/01/2010
// ---------------------------------------------------------------------------
void gaActivateATIPerformanceCountersStub(osSocket& apiSocket)
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
    retVal = gaActivateATIPerformanceCountersImpl(countersActivationVector);

    // Return the return value:
    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaEnableGLDebugOutputLoggingStub
// Description: Stub function for gaEnableGLDebugOutputIntegration
// Arguments:   osSocket& apiSocket
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        9/6/2010
// ---------------------------------------------------------------------------
void gaEnableGLDebugOutputLoggingStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Read the flag:
    bool glDebugOutputIntegrationEnabled = false;
    apiSocket >> glDebugOutputIntegrationEnabled;

    // Call the function implementation:
    retVal = gaEnableGLDebugOutputLoggingImpl(glDebugOutputIntegrationEnabled);

    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetGLDebugOutputSeverityEnabledStub
// Description: Stub function for gaSetGLDebugOutputSeverityEnabled
// Arguments:   osSocket& apiSocket
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        13/6/2010
// ---------------------------------------------------------------------------
void gaSetGLDebugOutputSeverityEnabledStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Read the flag:
    gtUInt32 severityAsInt32 = 0;
    apiSocket >> severityAsInt32;
    apGLDebugOutputSeverity severity = (apGLDebugOutputSeverity)severityAsInt32;

    bool enabled = false;
    apiSocket >> enabled;

    // Call the function implementation:
    retVal = gaSetGLDebugOutputSeverityEnabledImpl(severity, enabled);

    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaDoesDebugForcedContextExistStub
// Description: Stub function for gaDoesDebugForcedContextExist
// Arguments:   osSocket& apiSocket
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        7/7/2010
// ---------------------------------------------------------------------------
void gaDoesDebugForcedContextExistStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Read the flag:
    bool isDebugOn = false;
    apiSocket >> isDebugOn;

    // Call the function implementation:
    bool isDebugContextExist;
    retVal = gaDoesDebugForcedContextExistImpl(isDebugOn, isDebugContextExist);

    apiSocket << retVal;

    if (retVal)
    {
        // Write the results:
        apiSocket << isDebugContextExist;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaSetGLDebugOutputKindMaskStub
// Description: Stub function for gaSetGLDebugOutputKindMask
// Arguments:   osSocket& apiSocket
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        9/6/2010
// ---------------------------------------------------------------------------
void gaSetGLDebugOutputKindMaskStub(osSocket& apiSocket)
{
    bool retVal = false;

    // Read the flag:
    gtUInt64 debugOutputCategoryMask = 0;
    apiSocket >> debugOutputCategoryMask;

    // Call the function implementation:
    retVal = gaSetGLDebugOutputKindMaskImpl(debugOutputCategoryMask);

    apiSocket << retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfPiplineObjectsStub
// Description: Stub function for gaGetAmountOfPiplineObjects
// Arguments:   osSocket& apiSocket
// Return Val:  void
// Author:      Amit Ben-Moshe
// Date:        22/6/2014
// ---------------------------------------------------------------------------
void gaGetAmountOfPiplineObjectsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    int amountOfPipelines = 0;
    bool retVal = gaGetAmountOfPipelineObjectsImpl((int)contextIdAsInt32, amountOfPipelines);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtInt32)amountOfPipelines;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetPiplineObjectDetailsStub
// Description: Stub function for gaGetPiplineObjectDetails
// Arguments:   osSocket& apiSocket
// Return Val:  void
// Author:      Amit Ben-Moshe
// Date:        22/6/2014
// ---------------------------------------------------------------------------
void gaGetPiplineObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 pipelineNameAsUInt32 = 0;
    apiSocket >> pipelineNameAsUInt32;

    // Call the function implementation:
    apGLPipeline pipelineBuffer;
    bool retVal = gaGetPipelineObjectDetailsImpl((int)contextIdAsInt32, (GLuint)pipelineNameAsUInt32, pipelineBuffer);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        pipelineBuffer.writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetPiplineObjectDetailsStub
// Description: Stub function for gaGetPipelineName
// Arguments:   osSocket& apiSocket
// Return Val:  void
// Author:      Amit Ben-Moshe
// Date:        22/6/2014
// ---------------------------------------------------------------------------
void gaGetPiplineNameStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 pipelineIndexAsInt32 = 0;
    apiSocket >> pipelineIndexAsInt32;

    // Call the function implementation:
    GLuint pipelineName = 0;
    bool retVal = gaGetPipelineObjectNameImpl((int)contextIdAsInt32, (int)pipelineIndexAsInt32, pipelineName);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtUInt32)pipelineName;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfSamplerObjectsStub
// Description: Stub function for gaGetAmountOfSamplerObjects
// Arguments:   osSocket& apiSocket
// Return Val:  void
// Author:      Amit Ben-Moshe
// Date:        22/6/2014
// ---------------------------------------------------------------------------
void gaGetAmountOfSamplerObjectsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    // Call the function implementation:
    int amountOfSamplers = 0;
    bool retVal = gaGetAmountOfSamplerObjectsImpl((int)contextIdAsInt32, amountOfSamplers);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtInt32)amountOfSamplers;
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetSamplerObjectDetailsStub
// Description: Stub function for gaGetSamplerObjectDetails
// Arguments:   osSocket& apiSocket
// Return Val:  void
// Author:      Amit Ben-Moshe
// Date:        22/6/2014
// ---------------------------------------------------------------------------
void gaGetSamplerObjectDetailsStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtUInt32 samplerNameAsUInt32 = 0;
    apiSocket >> samplerNameAsUInt32;

    // Call the function implementation:
    apGLSampler samplerBuffer;
    bool retVal = gaGetSamplerObjectDetailsImpl((int)contextIdAsInt32, (GLuint)samplerNameAsUInt32, samplerBuffer);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        samplerBuffer.writeSelfIntoChannel(apiSocket);
    }
}

// ---------------------------------------------------------------------------
// Name:        gaGetSamplerNameStub
// Description: Stub function for gaGetSamplerName
// Arguments:   osSocket& apiSocket
// Return Val:  void
// Author:      Amit Ben-Moshe
// Date:        22/6/2014
// ---------------------------------------------------------------------------
void gaGetSamplerNameStub(osSocket& apiSocket)
{
    // Read the arguments from the apiSocket:
    gtInt32 contextIdAsInt32 = -1;
    apiSocket >> contextIdAsInt32;

    gtInt32 samplerIndexAsInt32 = 0;
    apiSocket >> samplerIndexAsInt32;

    // Call the function implementation:
    GLuint samplerName = 0;
    bool retVal = gaGetSamplerObjectNameImpl((int)contextIdAsInt32, (int)samplerIndexAsInt32, samplerName);

    // Return the return value:
    apiSocket << retVal;

    if (retVal)
    {
        // Return the arguments that should be returned:
        apiSocket << (gtUInt32)samplerName;
    }
}


