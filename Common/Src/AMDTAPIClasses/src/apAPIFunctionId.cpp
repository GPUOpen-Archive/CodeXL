//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apAPIFunctionId.cpp
///
//==================================================================================

//------------------------------ apAPIFunctionId.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/apStringConstants.h>
#include <AMDTAPIClasses/Include/apAPIFunctionId.h>


// ---------------------------------------------------------------------------
// Name:        apAPIFunctionIdToString
// Description: Translates apAPIFunctionId to a human readable string.
// Arguments: functionId - The input apAPIFunctionId.
//            functionIdAsString - The output human readable string.
// Author:  AMD Developer Tools Team
// Date:        21/12/2009
// ---------------------------------------------------------------------------
void apAPIFunctionIdToString(apAPIFunctionId functionId, gtString& functionIdAsString)
{
    functionIdAsString.makeEmpty();

    switch (functionId)
    {
        case GA_FID_gaIntializeAPI:
            functionIdAsString = L"gaIntializeAPI";
            break;

        case GA_FID_gaIntializeAPIEnded:
            functionIdAsString = L"gaIntializeAPIEnded";
            break;

        case GA_FID_gaOpenGLServerInitializationEnded:
            functionIdAsString = L"gaOpenGLServerInitializationEnded";
            break;

        case GA_FID_gaOpenCLServerInitializationEnded:
            functionIdAsString = L"gaOpenCLServerInitializationEnded";
            break;

        case GA_FID_gaGetAPIThreadId:
            functionIdAsString = L"gaGetAPIThreadId";
            break;

        case GA_FID_gaBeforeDirectAPIFunctionExecution:
            functionIdAsString = L"gaBeforeDirectAPIFunctionExecution";
            break;

        case GA_FID_gaBeforeDirectAPIFunctionExecutionDuringBreak:
            functionIdAsString = L"gaBeforeDirectAPIFunctionExecutionDuringBreak";
            break;

        case GA_FID_gaTerminateDebuggedProcess:
            functionIdAsString = L"gaTerminateDebuggedProcess";
            break;

        case GA_FID_gaSuspendDebuggedProcess:
            functionIdAsString = L"gaSuspendDebuggedProcess";
            break;

        case GA_FID_gaResumeDebuggedProcess:
            functionIdAsString = L"gaResumeDebuggedProcess";
            break;

        case GA_FID_gaMakeThreadGetCallStack:
            functionIdAsString = L"gaMakeThreadGetCallStack";
            break;

        case GA_FID_gaGetThreadCurrentRenderContext:
            functionIdAsString = L"gaGetThreadCurrentRenderContext";
            break;

        case GA_FID_gaGetBreakpointTriggeringContextId:
            functionIdAsString = L"gaGetBreakpointTriggeringContextId";
            break;

        case GA_FID_gaGetAmountOfRenderContexts:
            functionIdAsString = L"gaGetAmountOfRenderContexts";
            break;

        case GA_FID_gaGetRenderContextDetails:
            functionIdAsString = L"gaGetRenderContextDetails";
            break;

        case GA_FID_gaGetRenderContextGraphicsDetails:
            functionIdAsString = L"gaGetRenderContextGraphicsDetails";
            break;

        case GA_FID_gaGetOpenCLContextDetails:
            functionIdAsString = L"gaGetOpenCLContextDetails";
            break;

        case GA_FID_gaGetDefaultOpenGLStateVariableValue:
            functionIdAsString = L"gaGetDefaultOpenGLStateVariableValue";
            break;

        case GA_FID_gaGetOpenGLStateVariableValue:
            functionIdAsString = L"gaGetOpenGLStateVariableValue";
            break;

        case GA_FID_gaEnableImagesDataLogging:
            functionIdAsString = L"gaEnableImagesDataLogging";
            break;

        case GA_FID_gaCollectAllocatedObjectsCreationCallsStacks:
            functionIdAsString = L"gaCollectAllocatedObjectsCreationCallsStacks";
            break;

        case GA_FID_gaGetAmountOfTextureUnits:
            functionIdAsString = L"gaGetAmountOfTextureUnits";
            break;

        case GA_FID_gaGetActiveTextureUnit:
            functionIdAsString = L"gaGetActiveTextureUnit";
            break;

        case GA_FID_gaGetTextureUnitName:
            functionIdAsString = L"gaGetTextureUnitName";
            break;

        case GA_FID_gaGetEnabledTexturingMode:
            functionIdAsString = L"gaGetEnabledTexturingMode";
            break;

        case GA_FID_gaGetAmountOfStaticBuffersObjects:
            functionIdAsString = L"gaGetAmountOfStaticBuffersObjects";
            break;

        case GA_FID_gaGetAmountOfPBuffersObjects:
            functionIdAsString = L"gaGetAmountOfPBuffersObjects";
            break;

        case GA_FID_gaGetAmountOfSyncObjects:
            functionIdAsString = L"gaGetAmountOfSyncObjects";
            break;

        case GA_FID_gaGetAmountOfFBOs:
            functionIdAsString = L"gaGetAmountOfFBOs";
            break;

        case GA_FID_gaGetFBOName:
            functionIdAsString = L"gaGetFBOName";
            break;

        case GA_FID_gaGetFBODetails:
            functionIdAsString = L"gaGetFBODetails";
            break;

        case GA_FID_gaGetActiveFBO:
            functionIdAsString = L"gaGetActiveFBO";
            break;

        case GA_FID_gaGetAmountOfVBOs:
            functionIdAsString = L"gaGetAmountOfVBOs";
            break;

        case GA_FID_gaGetVBODetails:
            functionIdAsString = L"gaGetVBODetails";
            break;

        case GA_FID_gaGetVBOAttachment:
            functionIdAsString = L"gaGetVBOAttachment";
            break;

        case GA_FID_gaGetVBOName:
            functionIdAsString = L"gaGetVBOName";
            break;

        case GA_FID_gaSetCurrentThreadVBODisplayProperties:
            functionIdAsString = L"gaSetCurrentThreadVBODisplayProperties";
            break;

        case GA_FID_gaSetVBODisplayProperties:
            functionIdAsString = L"gaSetVBODisplayProperties";
            break;

        case GA_FID_gaGetAmountOfTextureObjects:
            functionIdAsString = L"gaGetAmountOfTextureObjects";
            break;

        case GA_FID_gaGetBoundTexture:
            functionIdAsString = L"gaGetBoundTexture";
            break;

        case GA_FID_gaGetTextureObjectName:
            functionIdAsString = L"gaGetTextureObjectName";
            break;

        case GA_FID_gaGetTextureObjectType:
            functionIdAsString = L"gaGetTextureObjectType";
            break;

        case GA_FID_gaGetTextureObjectDetails:
            functionIdAsString = L"gaGetTextureObjectDetails";
            break;

        case GA_FID_gaGetTextureMiplevelDataFilePath:
            functionIdAsString = L"gaGetTextureMiplevelDataFilePath";
            break;

        case GA_FID_gaGetTextureThumbnailDataObjectDetails:
            functionIdAsString = L"gaGetTextureThumbnailDataObjectDetails";
            break;

        case GA_FID_gaGetTextureDataObjectDetails:
            functionIdAsString = L"gaGetTextureDataObjectDetails";
            break;

        case GA_FID_gaGetTextureMemoryDataObjectDetails:
            functionIdAsString = L"gaGetTextureMemoryDataObjectDetails";
            break;

        case GA_FID_gaGetRenderBufferObjectName:
            functionIdAsString = L"gaGetRenderBufferObjectName";
            break;

        case GA_FID_gaGetAmountOfRenderBufferObjects:
            functionIdAsString = L"gaGetAmountOfRenderBufferObjects";
            break;

        case GA_FID_gaGetRenderBufferObjectDetails:
            functionIdAsString = L"gaGetRenderBufferObjectDetails";
            break;

        case GA_FID_gaGetAmountOfPipelineObjects:
            functionIdAsString = L"gaGetAmountOfPipelineObjects";
            break;

        case GA_FID_gaGetPipelineObjectDetails:
            functionIdAsString = L"gaGetPipelineObjectDetails";
            break;

        case GA_FID_gaGetPipelineObjectName:
            functionIdAsString = L"gaGetPiplineNameStub";
            break;

        case GA_FID_gaGetAmountOfSamplerObjects:
            functionIdAsString = L"gaGetAmountOfSamplerObjects";
            break;

        case GA_FID_gaGetSamplerObjectDetails:
            functionIdAsString = L"gaGetSamplerObjectDetails";
            break;

        case GA_FID_gaGetSamplerObjectName:
            functionIdAsString = L"gaGetSamplerObjectName";
            break;

        case GA_FID_gaMarkAllTextureImagesAsUpdated:
            functionIdAsString = L"gaMarkAllTextureImagesAsUpdated";
            break;

        case GA_FID_gaUpdateTextureRawData:
            functionIdAsString = L"gaUpdateTextureRawData";
            break;

        case GA_FID_gaUpdateTextureParameters:
            functionIdAsString = L"gaUpdateTextureParameters";
            break;

        case GA_FID_gaIsTextureImageDirty:
            functionIdAsString = L"gaIsTextureImageDirty";
            break;

        case GA_FID_gaGetStaticBufferObjectDetails:
            functionIdAsString = L"gaGetStaticBufferObjectDetails";
            break;

        case GA_FID_gaGetStaticBufferType:
            functionIdAsString = L"gaGetStaticBufferType";
            break;

        case GA_FID_gaUpdateStaticBufferRawData:
            functionIdAsString = L"gaUpdateStaticBufferRawData";
            break;

        case GA_FID_gaUpdateStaticBuffersDimensions:
            functionIdAsString = L"gaUpdateStaticBuffersDimensions";
            break;

        case GA_FID_gaUpdateRenderBufferRawData:
            functionIdAsString = L"gaUpdateRenderBufferRawData";
            break;

        case GA_FID_gaUpdateVBORawData:
            functionIdAsString = L"gaUpdateVBORawData";
            break;

        case GA_FID_gaUpdatePBufferStaticBufferRawData:
            functionIdAsString = L"gaUpdatePBufferStaticBufferRawData";
            break;

        case GA_FID_gaUpdateCurrentThreadStaticBufferRawData:
            functionIdAsString = L"gaUpdateCurrentThreadStaticBufferRawData";
            break;

        case GA_FID_gaUpdateCurrentThreadStaticBuffersDimensions:
            functionIdAsString = L"gaUpdateCurrentThreadStaticBuffersDimensions";
            break;

        case GA_FID_gaUpdateCurrentThreadPBuffersDimensions:
            functionIdAsString = L"gaUpdateCurrentThreadPBuffersDimensions";
            break;

        case GA_FID_gaUpdateCurrentThreadPBufferStaticBufferRawData:
            functionIdAsString = L"gaUpdateCurrentThreadPBufferStaticBufferRawData";
            break;

        case GA_FID_gaUpdateCurrentThreadTextureRawData:
            functionIdAsString = L"gaUpdateCurrentThreadTextureRawData";
            break;

        case GA_FID_gaUpdateCurrentThreadTextureParameters:
            functionIdAsString = L"gaUpdateCurrentThreadTextureParameters";
            break;

        case GA_FID_gaUpdateCurrentThreadRenderBufferRawData:
            functionIdAsString = L"gaUpdateCurrentThreadRenderBufferRawData";
            break;

        case GA_FID_gaUpdateCurrentThreadVBORawData:
            functionIdAsString = L"gaUpdateCurrentThreadVBORawData";
            break;

        case GA_FID_gaGetAmountOfPBufferContentObjects:
            functionIdAsString = L"gaGetAmountOfPBufferContentObjects";
            break;

        case GA_FID_gaGetPBufferStaticBufferType:
            functionIdAsString = L"gaGetPBufferStaticBufferType";
            break;

        case GA_FID_gaGetPBufferStaticBufferObjectDetails:
            functionIdAsString = L"gaGetPBufferStaticBufferObjectDetails";
            break;

        case GA_FID_gaGetPBufferObjectDetails:
            functionIdAsString = L"gaGetPBufferObjectDetails";
            break;

        case GA_FID_gaUpdatePBuffersDimensions:
            functionIdAsString = L"gaUpdatePBuffersDimensions";
            break;

        case GA_FID_gaGetSyncObjectDetails:
            functionIdAsString = L"gaGetSyncObjectDetails";
            break;

        case GA_FID_gaGetAmountOfProgramObjects:
            functionIdAsString = L"gaGetAmountOfProgramObjects";
            break;

        case GA_FID_gaGetActiveProgramObjectName:
            functionIdAsString = L"gaGetActiveProgramObjectName";
            break;

        case GA_FID_gaGetProgramObjectName:
            functionIdAsString = L"gaGetProgramObjectName";
            break;

        case GA_FID_gaGetProgramObjectDetails:
            functionIdAsString = L"gaGetProgramObjectDetails";
            break;

        case GA_FID_gaGetProgramActiveUniforms:
            functionIdAsString = L"gaGetProgramActiveUniforms";
            break;

        case GA_FID_gaLinkProgramObject:
            functionIdAsString = L"gaLinkProgramObject";
            break;

        case GA_FID_gaLinkCurrentThreadProgramObject:
            functionIdAsString = L"gaLinkCurrentThreadProgramObject";
            break;

        case GA_FID_gaValidateProgramObject:
            functionIdAsString = L"gaValidateProgramObject";
            break;

        case GA_FID_gaValidateCurrentThreadProgramObject:
            functionIdAsString = L"gaValidateCurrentThreadProgramObject";
            break;

        case GA_FID_gaGetAmountOfShaderObjects:
            functionIdAsString = L"gaGetAmountOfShaderObjects";
            break;

        case GA_FID_gaGetShaderObjectName:
            functionIdAsString = L"gaGetShaderObjectName";
            break;

        case GA_FID_gaGetShaderObjectDetails:
            functionIdAsString = L"gaGetShaderObjectDetails";
            break;

        case GA_FID_gaMarkShaderObjectSourceCodeAsForced:
            functionIdAsString = L"gaMarkShaderObjectSourceCodeAsForced";
            break;

        case GA_FID_gaSetShaderObjectSourceCode:
            functionIdAsString = L"gaSetShaderObjectSourceCode";
            break;

        case GA_FID_gaSetCurrentThreadShaderObjectSourceCode:
            functionIdAsString = L"gaSetCurrentThreadShaderObjectSourceCode";
            break;

        case GA_FID_gaCompileShaderObject:
            functionIdAsString = L"gaCompileShaderObject";
            break;

        case GA_FID_gaCompileCurrentThreadShaderObject:
            functionIdAsString = L"gaCompileCurrentThreadShaderObject";
            break;

        case GA_FID_gaGetAmountOfDisplayLists:
            functionIdAsString = L"gaGetAmountOfDisplayLists";
            break;

        case GA_FID_gaGetDisplayListObjectName:
            functionIdAsString = L"gaGetDisplayListObjectName";
            break;

        case GA_FID_gaGetDisplayListObjectDetails:
            functionIdAsString = L"gaGetDisplayListObjectDetails";
            break;

        case GA_FID_gaGetAmountOfCurrentFrameFunctionCalls:
            functionIdAsString = L"gaGetAmountOfCurrentFrameFunctionCalls";
            break;

        case GA_FID_gaGetCurrentFrameFunctionCall:
            functionIdAsString = L"gaGetCurrentFrameFunctionCall";
            break;

        case GA_FID_gaGetCurrentFrameFunctionCallDeprecationDetails:
            functionIdAsString = L"gaGetCurrentFrameFunctionCallDeprecationDetails";
            break;

        case GA_FID_gaGetLastFunctionCall:
            functionIdAsString = L"gaGetLastFunctionCall";
            break;

        case GA_FID_gaFindCurrentFrameFunctionCall:
            functionIdAsString = L"gaFindCurrentFrameFunctionCall";
            break;

        case GA_FID_gaGetCurrentStatistics:
            functionIdAsString = L"gaGetCurrentStatistics";
            break;

        case GA_FID_gaGetCurrentOpenCLStatistics:
            functionIdAsString = L"gaGetCurrentOpenCLStatistics";
            break;

        case GA_FID_gaClearFunctionCallsStatistics:
            functionIdAsString = L"gaClearFunctionCallsStatistics";
            break;

        case GA_FID_gaClearOpenCLFunctionCallsStatistics:
            functionIdAsString = L"gaClearOpenCLFunctionCallsStatistics";
            break;

        case GA_FID_gaIsInOpenGLBeginEndBlock:
            functionIdAsString = L"gaIsInOpenGLBeginEndBlock";
            break;

        case GA_FID_gaGetRenderPrimitivesStatistics:
            functionIdAsString = L"gaGetRenderPrimitivesStatistics";
            break;

        case GA_FID_gaGetAmountOfOpenCLFunctionCalls:
            functionIdAsString = L"gaGetAmountOfOpenCLFunctionCalls";
            break;

        case GA_FID_gaGetOpenCLFunctionCall:
            functionIdAsString = L"gaGetOpenCLFunctionCall";
            break;

        case GA_FID_gaGetLastOpenCLFunctionCall:
            functionIdAsString = L"gaGetLastOpenCLFunctionCall";
            break;

        case GA_FID_gaFindOpenCLFunctionCall:
            functionIdAsString = L"gaFindOpenCLFunctionCall";
            break;

        case GA_FID_gaGetOpenCLHandleObjectDetails:
            functionIdAsString = L"gaGetOpenCLHandleObjectDetails";
            break;

        case GA_FID_gaGetAmountOfOpenCLContexts:
            functionIdAsString = L"gaGetAmountOfOpenCLContexts";
            break;

        case GA_FID_gaGetAmountOfOpenCLProgramObjects:
            functionIdAsString = L"gaGetAmountOfOpenCLProgramObjects";
            break;

        case GA_FID_gaGetOpenCLProgramObjectDetails:
            functionIdAsString = L"gaGetOpenCLProgramObjectDetails";
            break;

        case GA_FID_gaSetOpenCLProgramCode:
            functionIdAsString = L"gaSetOpenCLProgramCode";
            break;

        case GA_FID_gaBuildOpenCLProgram:
            functionIdAsString = L"gaBuildOpenCLProgram";
            break;

        case GA_FID_gaGetOpenCLProgramHandleFromSourceFilePath:
            functionIdAsString = L"gaGetOpenCLProgramHandleFromSourceFilePath";
            break;

        case GA_FID_gaGetOpenCLKernelObjectDetails:
            functionIdAsString = L"gaGetOpenCLKernelObjectDetails";
            break;

        case GA_FID_gaGetKernelDebuggingLocation:
            functionIdAsString = L"gaGetKernelDebuggingLocation";
            break;

        case GA_FID_gaGetCurrentlyDebuggedKernelDetails:
            functionIdAsString = L"gaGetCurrentlyDebuggedKernelDetails";
            break;

        case GA_FID_gaGetCurrentlyDebuggedKernelCallStack:
            functionIdAsString = L"gaGetCurrentlyDebuggedKernelCallStack";
            break;

        case GA_FID_gaSetKernelDebuggingCommand:
            functionIdAsString = L"gaSetKernelDebuggingCommand";
            break;

        case GA_FID_gaSetKernelSteppingWorkItem:
            functionIdAsString = L"gaSetKernelSteppingWorkItem";
            break;

        case GA_FID_gaIsWorkItemValid:
            functionIdAsString = L"gaIsWorkItemValid";
            break;

        case GA_FID_gaGetFirstValidWorkItem:
            functionIdAsString = L"gaGetFirstValidWorkItem";
            break;

        case GA_FID_gaCanGetKernelVariableValue:
            functionIdAsString = L"gaCanGetKernelVariableValue";
            break;

        case GA_FID_gaGetKernelDebuggingExpressionValue:
            functionIdAsString = L"gaGetKernelDebuggingExpressionValue";
            break;

        case GA_FID_gaGetKernelDebuggingAvailableVariables:
            functionIdAsString = L"gaGetKernelDebuggingAvailableVariables";
            break;

        case GA_FID_gaGetKernelDebuggingAmountOfActiveWavefronts:
            functionIdAsString = L"gaGetKernelDebuggingAmountOfActiveWavefronts";
            break;

        case GA_FID_gaGetKernelDebuggingActiveWavefrontID:
            functionIdAsString = L"gaGetKernelDebuggingActiveWavefrontID";
            break;

        case GA_FID_gaGetKernelDebuggingWavefrontIndex:
            functionIdAsString = L"gaGetKernelDebuggingWavefrontIndex";
            break;

        case GA_FID_gaUpdateKernelVariableValueRawData:
            functionIdAsString = L"gaUpdateKernelVariableValueRawData";
            break;

        case GA_FID_gaGetKernelSourceCodeBreakpointResolution:
            functionIdAsString = L"gaGetKernelSourceCodeBreakpointResolution";
            break;

        case GA_FID_gaSetKernelDebuggingEnable:
            functionIdAsString = L"gaSetKernelDebuggingEnable";
            break;

        case GA_FID_gaSetMultipleKernelDebugDispatchMode:
            functionIdAsString = L"gaSetMultipleKernelDebugDispatchMode";
            break;

        case GA_FID_gaGetOpenCLDeviceObjectDetails:
            functionIdAsString = L"gaGetOpenCLDeviceObjectDetails";
            break;

        case GA_FID_gaGetOpenCLPlatformAPIID:
            functionIdAsString = L"gaGetOpenCLPlatformAPIID";
            break;

        case GA_FID_gaGetAmountOfOpenCLBufferObjects:
            functionIdAsString = L"gaGetAmountOfOpenCLBufferObjects";
            break;

        case GA_FID_gaGetOpenCLBufferObjectDetails:
            functionIdAsString = L"gaGetOpenCLBufferObjectDetails";
            break;

        case GA_FID_gaGetOpenCLSubBufferObjectDetails:
            functionIdAsString = L"gaGetOpenCLSubBufferObjectDetails";
            break;

        case GA_FID_gaUpdateOpenCLBufferRawData:
            functionIdAsString = L"gaUpdateOpenCLBufferRawData";
            break;

        case GA_FID_gaUpdateOpenCLSubBufferRawData:
            functionIdAsString = L"gaUpdateOpenCLSubBufferRawData";
            break;

        case GA_FID_gaSetCLBufferDisplayProperties:
            functionIdAsString = L"gaSetCLBufferDisplayProperties";
            break;

        case GA_FID_gaSetCLSubBufferDisplayProperties:
            functionIdAsString = L"gaSetCLSubBufferDisplayProperties";
            break;

        case GA_FID_gaUpdateOpenCLImageRawData:
            functionIdAsString = L"gaUpdateOpenCLImageRawData";
            break;

        case GA_FID_gaGetAmountOfOpenCLImageObjects:
            functionIdAsString = L"gaGetAmountOfOpenCLImageObjects";
            break;

        case GA_FID_gaGetOpenCLImageObjectDetails:
            functionIdAsString = L"gaGetOpenCLImageObjectDetails";
            break;

        case GA_FID_gaGetAmountOfOpenCLPipeObjects:
            functionIdAsString = L"gaGetAmountOfOpenCLPipeObjects";
            break;

        case GA_FID_gaGetOpenCLPipeObjectDetails:
            functionIdAsString = L"gaGetOpenCLPipeObjectDetails";
            break;

        case GA_FID_gaGetAmountOfCommandQueues:
            functionIdAsString = L"gaGetAmountOfCommandQueues";
            break;

        case GA_FID_gaGetCommandQueueDetails:
            functionIdAsString = L"gaGetCommandQueueDetails";
            break;

        case GA_FID_gaGetAmountOfCommandsInQueue:
            functionIdAsString = L"gaGetAmountOfCommandsInQueue";
            break;

        case GA_FID_gaGetAmountOfEventsInQueue:
            functionIdAsString = L"gaGetAmountOfEventsInQueue";
            break;

        case GA_FID_gaGetEnqueuedCommandDetails:
            functionIdAsString = L"gaGetEnqueuedCommandDetails";
            break;

        case GA_FID_gaGetAmountOfOpenCLSamplers:
            functionIdAsString = L"gaGetAmountOfOpenCLSamplers";
            break;

        case GA_FID_gaGetOpenCLSamplerObjectDetails:
            functionIdAsString = L"gaGetOpenCLSamplerObjectDetails";
            break;

        case GA_FID_gaGetAmountOfOpenCLEvents:
            functionIdAsString = L"gaGetAmountOfOpenCLEvents";
            break;

        case GA_FID_gaGetOpenCLEventObjectDetails:
            functionIdAsString = L"gaGetOpenCLEventObjectDetails";
            break;

        case GA_FID_gaGetAmountOfRegisteredAllocatedObjects:
            functionIdAsString = L"gaGetAmountOfRegisteredAllocatedObjects";
            break;

        case GA_FID_gaGetAllocatedObjectCreationStack:
            functionIdAsString = L"gaGetAllocatedObjectCreationStack";
            break;

        case GA_FID_gaFindStringMarker:
            functionIdAsString = L"gaFindStringMarker";
            break;

        case GA_FID_gaSetBreakpoint:
            functionIdAsString = L"gaSetBreakpoint";
            break;

        case GA_FID_gaRemoveBreakpoint:
            functionIdAsString = L"gaRemoveBreakpoint";
            break;

        case GA_FID_gaRemoveAllBreakpoints:
            functionIdAsString = L"gaRemoveAllBreakpoints";
            break;

        case GA_FID_gaGetDetectedErrorParameters:
            functionIdAsString = L"gaGetDetectedErrorParameters";
            break;

        case GA_FID_gaBreakOnNextMonitoredFunctionCall:
            functionIdAsString = L"gaBreakOnNextMonitoredFunctionCall";
            break;

        case GA_FID_gaBreakOnNextDrawFunctionCall:
            functionIdAsString = L"gaBreakOnNextDrawFunctionCall";
            break;

        case GA_FID_gaBreakOnNextFrame:
            functionIdAsString = L"gaBreakOnNextFrame";
            break;

        case GA_FID_gaBreakInMonitoredFunctionCall:
            functionIdAsString = L"gaBreakInMonitoredFunctionCall";
            break;

        case GA_FID_gaClearAllStepFlags:
            functionIdAsString = L"gaClearAllStepFlags";
            break;

        case GA_FID_gaGetBreakReason:
            functionIdAsString = L"gaGetBreakReason";
            break;

        case GA_FID_gaStartMonitoredFunctionsCallsLogFileRecording:
            functionIdAsString = L"gaStartMonitoredFunctionsCallsLogFileRecording";
            break;

        case GA_FID_gaStopMonitoredFunctionsCallsLogFileRecording:
            functionIdAsString = L"gaStopMonitoredFunctionsCallsLogFileRecording";
            break;

        case GA_FID_gaIsMonitoredFunctionsCallsLogFileRecordingActive:
            functionIdAsString = L"gaIsMonitoredFunctionsCallsLogFileRecordingActive";
            break;

        case GA_FID_gaFlushAfterEachMonitoredFunctionCall:
            functionIdAsString = L"gaFlushAfterEachMonitoredFunctionCall";
            break;

        case GA_FID_gaGetContextLogFilePath:
            functionIdAsString = L"gaGetRenderContextLogFilePath";
            break;

        case GA_FID_gaGetCLContextLogFilePath:
            functionIdAsString = L"gaGetCLContextLogFilePath";
            break;

        case GA_FID_gaGetCurrentOpenGLError:
            functionIdAsString = L"gaGetCurrentOpenGLError";
            break;

        case GA_FID_gaSetSlowMotionDelay:
            functionIdAsString = L"gaSetSlowMotionDelay";
            break;

        case GA_FID_gaForceOpenGLFlush:
            functionIdAsString = L"gaForceOpenGLFlush";
            break;

        case GA_FID_gaSetInteractiveBreakMode:
            functionIdAsString = L"gaSetInteractiveBreakMode";
            break;

        case GA_FID_gaSetDebuggedProcessExecutionMode:
            functionIdAsString = L"gaSetDeubggedProcessExecutionMode";
            break;

        case GA_FID_gaForceOpenGLPolygonRasterMode:
            functionIdAsString = L"gaForceOpenGLPolygonRasterMode";
            break;

        case GA_FID_gaCancelOpenGLPolygonRasterModeForcing:
            functionIdAsString = L"gaCancelOpenGLPolygonRasterModeForcing";
            break;

        case GA_FID_gaSetOpenGLNullDriver:
            functionIdAsString = L"gaSetOpenGLNullDriver";
            break;

        case GA_FID_gaSetOpenGLForceStub:
            functionIdAsString = L"gaSetOpenGLForceStub";
            break;

        case GA_FID_gaSetOpenCLOperationExecution:
            functionIdAsString = L"gaSetOpenCLOperationExecution";
            break;

        case GA_FID_gaSetFloatParametersDisplayPrecision:
            functionIdAsString = L"gaSetFloatParametersDisplayPrecision";
            break;

        case GA_FID_gaEnableGLDebugOutputLogging:
            functionIdAsString = L"gaEnableGLDebugOutputLogging";
            break;

        case GA_FID_gaSetGLDebugOutputKindMask:
            functionIdAsString = L"gaSetGLDebugOutputKindMask";
            break;

        case GA_FID_gaSetGLDebugOutputSeverityEnabled:
            functionIdAsString = L"gaSetGLDebugOutputSeverityEnabled";
            break;

        case GA_FID_gaDoesDebugForcedContextExist:
            functionIdAsString = L"gaDoesDebugForcedContextExist";
            break;

        case GA_FID_gaGetSpyPerformanceCountersValues:
            functionIdAsString = L"gaGetSpyPerformanceCountersValues";
            break;

        case GA_FID_gaGetRemoteOSPerformanceCountersValues:
            functionIdAsString = L"gaGetRemoteOSPerformanceCountersValues";
            break;

        case GA_FID_gaAddSupportediPhonePerformanceCounter:
            functionIdAsString = L"gaAddSupportediPhonePerformanceCounter";
            break;

        case GA_FID_gaInitializeiPhonePerformanceCountersReader:
            functionIdAsString = L"gaInitializeiPhonePerformanceCountersReader";
            break;

        case GA_FID_gaGetiPhonePerformanceCountersValues:
            functionIdAsString = L"gaGetiPhonePerformanceCountersValues";
            break;

        case GA_FID_gaGetATIPerformanceCountersValues:
            functionIdAsString = L"gaGetATIPerformanceCountersValues";
            break;

        case GA_FID_gaActivateATIPerformanceCounters:
            functionIdAsString = L"gaActivateATIPerformanceCounters";
            break;

        case GA_FID_gaGetAMDOpenCLPerformanceCountersValues:
            functionIdAsString = L"gaGetAMDOpenCLPerformanceCountersValues";
            break;

        case GA_FID_gaGetOpenCLQueuePerformanceCountersValues:
            functionIdAsString = L"gaGetOpenCLQueuePerformanceCountersValues";
            break;

        case GA_FID_gaActivateAMDOpenCLPerformanceCounters:
            functionIdAsString = L"gaActivateAMDOpenCLPerformanceCounters";
            break;

        case GA_FID_gaCreateEventForwardingTCPConnection:
            functionIdAsString = L"gaCreateEventForwardingTCPConnection";
            break;

        case GA_FID_gaCreateEventForwardingPipeConnection:
            functionIdAsString = L"gaCreateEventForwardingPipeConnection";
            break;

        case GA_FID_gaReadFile:
            functionIdAsString = L"gaReadFile";
            break;

        case GA_FID_gaWriteFile:
            functionIdAsString = L"gaWriteFile";
            break;

        case GA_FID_gaUpdateContextDataSnapshot:
            functionIdAsString = L"gaUpdateContextDataSnapshot";
            break;

        case GA_FID_gaUpdateOpenCLContextDataSnapshot:
            functionIdAsString = L"gaUpdateOpenCLContextDataSnapshot";
            break;

        case GA_FID_gaUpdateCurrentThreadRenderContextDataSnapshot:
            functionIdAsString = L"gaUpdateCurrentThreadRenderContextDataSnapshot";
            break;

        case GA_FID_gaMakeThreadUpdateContextDataSnapshot:
            functionIdAsString = L"gaMakeThreadUpdateContextDataSnapshot";
            break;

        case GA_FID_gaMakeThreadUpdateStaticBufferRawData:
            functionIdAsString = L"gaMakeThreadUpdateStaticBufferRawData";
            break;

        case GA_FID_gaMakeThreadUpdateStaticBuffersDimensions:
            functionIdAsString = L"gaMakeThreadUpdateStaticBuffersDimensions";
            break;

        case GA_FID_gaMakeThreadUpdatePBuffersDimensions:
            functionIdAsString = L"gaMakeThreadUpdatePBuffersDimensions";
            break;

        case GA_FID_gaMakeThreadUpdatePBufferStaticBufferRawData:
            functionIdAsString = L"gaMakeThreadUpdatePBufferStaticBufferRawData";
            break;

        case GA_FID_gaMakeThreadUpdateRenderBufferRawData:
            functionIdAsString = L"gaMakeThreadUpdateRenderBufferRawData";
            break;

        case GA_FID_gaMakeThreadUpdateVBORawData:
            functionIdAsString = L"gaMakeThreadUpdateVBORawData";
            break;

        case GA_FID_gaMakeThreadSetVBODisplayProperties:
            functionIdAsString = L"gaMakeThreadSetVBODisplayProperties";
            break;

        case GA_FID_gaMakeThreadUpdateTextureRawData:
            functionIdAsString = L"gaMakeThreadUpdateTextureRawData";
            break;

        case GA_FID_gaMakeThreadUpdateTextureParameters:
            functionIdAsString = L"gaMakeThreadUpdateTextureParameters";
            break;

        case GA_FID_gaMakeThreadLinkProgramObject:
            functionIdAsString = L"gaMakeThreadLinkProgramObject";
            break;

        case GA_FID_gaMakeThreadValidateProgramObject:
            functionIdAsString = L"gaMakeThreadValidateProgramObject";
            break;

        case GA_FID_gaMakeThreadSetShaderObjectSourceCode:
            functionIdAsString = L"gaMakeThreadSetShaderObjectSourceCode";
            break;

        case GA_FID_gaMakeThreadCompileShaderObject:
            functionIdAsString = L"gaMakeThreadCompileShaderObject";
            break;

        case GA_FID_gaSetKernelSourceFilePath:
            functionIdAsString = L"gaSetKernelSourceFilePath";
            break;

        case GA_FID_gaIsInHSAKernelBreakpoint:
            functionIdAsString = L"gaIsInHSAKernelBreakpoint";
            break;

        case GA_FID_gaHSAGetCurrentLine:
            functionIdAsString = L"gaHSAGetCurrentLine";
            break;

        case GA_FID_gaHSAGetSourceFilePath:
            functionIdAsString = L"gaHSAGetSourceFilePath";
            break;

        case GA_FID_gaHSASetNextDebuggingCommand:
            functionIdAsString = L"gaHSASetNextDebuggingCommand";
            break;

        case GA_FID_gaHSASetBreakpoint:
            functionIdAsString = L"gaHSASetBreakpoint";
            break;

        case GA_FID_gaHSAListVariables:
            functionIdAsString = L"gaHSAListVariables";
            break;

        case GA_FID_gaHSAGetExpressionValue:
            functionIdAsString = L"gaHSAGetExpressionValue";
            break;

        case GA_FID_gaHSAListWorkItems:
            functionIdAsString = L"gaHSAListWorkItems";
            break;

        case GA_FID_gaHSASetActiveWorkItemIndex:
            functionIdAsString = L"gaHSASetActiveWorkItemIndex";
            break;

        case GA_FID_gaHSAGetWorkDims:
            functionIdAsString = L"gaHSAGetWorkDims";
            break;

        default:
        {
            // We encountered an unknown function id:
            functionIdAsString = AP_APIFunctionNo;
            functionIdAsString.appendFormattedString(L" %u", functionId);

            gtString errorString = AP_STR_UnknownAPIFunctionIdIsUsed;
            errorString += functionIdAsString;
            GT_ASSERT_EX(false, errorString.asCharArray());
        }
    }
}

