//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsAPIFunctionsStubs.h
///
//==================================================================================

//------------------------------ gsAPIFunctionsStubs.h ------------------------------

#ifndef __GSAPIFUNCTIONSSTUBS_H
#define __GSAPIFUNCTIONSSTUBS_H

// Forward decelerations:
class osSocket;

// API package:
void gsRegisterAPIStubFunctions();
void gsHandleAPIInitializationCalls();
void gaOpenGLServerInitializationEndedStub(osSocket& apiSocket);
void gaBeforeDirectAPIFunctionExecutionStub(osSocket& apiSocket);
void gaBeforeDirectAPIFunctionExecutionDuringBreakStub(osSocket& apiSocket);

// Render contexts:
void gaGetAmoutOfRenderContextsStub(osSocket& apiSocket);
void gaGetRenderContextDetailsStub(osSocket& apiSocket);
void gaGetRenderContextGraphicsDetailsStub(osSocket& apiSocket);
void gaGetThreadCurrentRenderContextStub(osSocket& apiSocket);
void gaUpdateContextDataSnapshotStub(osSocket& apiSocket);
void gaUpdateCurrentThreadRenderContextDataSnapshotStub();
void gaMakeThreadUpdateContextDataSnapshotStub(osSocket& apiSocket);

// State variables:
void gaGetDefaultOpenGLStateVariableValueStub(osSocket& apiSocket);
void gaGetOpenGLStateVariableValueStub(osSocket& apiSocket);

// Textures:
void gaGetAmountOfTextureUnitsStub(osSocket& apiSocket);
void gaGetActiveTextureUnitStub(osSocket& apiSocket);
void gaGetTextureUnitNameStub(osSocket& apiSocket);
void gaGetEnabledTexturingModeStub(osSocket& apiSocket);
void gaGetAmountOfTextureObjectsStub(osSocket& apiSocket);
void gaGetAmountOfTexBufferObjectsStub(osSocket& apiSocket);
void gaGetBoundTextureStub(osSocket& apiSocket);
void gaGetTextureObjectNameStub(osSocket& apiSocket);
void gaGetTextureObjectTypeStub(osSocket& apiSocket);
void gaGetTextureObjectDetailsStub(osSocket& apiSocket);
void gaGetTextureMiplevelDataFilePathStub(osSocket& apiSocket);
void gaGetTextureThumbnailDataObjectDetailsStub(osSocket& apiSocket);
void gaGetTextureDataObjectDetailsStub(osSocket& apiSocket);
void gaGetTextureMemoryDataObjectDetails(osSocket& apiSocket);
void gaMarkAllTextureImagesAsUpdatedStub(osSocket& apiSocket);
void gaUpdateTextureRawDataStub(osSocket& apiSocket);
void gaUpdateTextureParametersStub(osSocket& apiSocket);
void gaIsTextureImageDirtyStub(osSocket& apiSocket);
void gaUpdateCurrentThreadTextureRawDataStub();
void gaUpdateCurrentThreadTextureParametersStub();
void gaMakeThreadUpdateTextureRawDataStub(osSocket& apiSocket);
bool gaUpdateCurrentThreadTextureRawDataInternalStub();
void gaMakeThreadUpdateTextureParametersStub(osSocket& apiSocket);
bool gaUpdateCurrentThreadTextureParametersInternalStub();

// Render buffers:
void gaGetAmountOfRenderBufferObjectsStub(osSocket& apiSocket);
void gaGetRenderBufferObjectNameStub(osSocket& apiSocket);
void gaGetRenderBufferObjectDetailsStub(osSocket& apiSocket);
void gaUpdateRenderBufferRawDataStub(osSocket& apiSocket);
void gaUpdateCurrentThreadRenderBufferRawDataStub();
void gaMakeThreadUpdateRenderBufferRawDataStub(osSocket& apiSocket);
bool gaUpdateCurrentThreadRenderBufferRawDataInternalStub();

// Program pipelines:
void gaGetAmountOfPiplineObjectsStub(osSocket& apiSocket);
void gaGetPiplineObjectDetailsStub(osSocket& apiSocket);
void gaGetPiplineNameStub(osSocket& apiSocket);

// Samplers:
void gaGetAmountOfSamplerObjectsStub(osSocket& apiSocket);
void gaGetSamplerObjectDetailsStub(osSocket& apiSocket);
void gaGetSamplerNameStub(osSocket& apiSocket);

// FBOs
void gaGetAmountOfFBOsStub(osSocket& apiSocket);
void gaGetFBODetailsStub(osSocket& apiSocket);
void gaGetFBONameStub(osSocket& apiSocket);
void gaGetActiveFBOStub(osSocket& apiSocket);

// VBOs
void gaGetAmountOfVBOsStub(osSocket& apiSocket);
void gaGetVBODetailsStub(osSocket& apiSocket);
void gaGetVBOAttachmentStub(osSocket& apiSocket);
void gaGetVBONameStub(osSocket& apiSocket);
void gaUpdateCurrentThreadVBORawDataStub();
void gaUpdateVBORawDataStub(osSocket& apiSocket);
void gaMakeThreadUpdateVBORawDataStub(osSocket& apiSocket);
bool gaUpdateCurrentThreadVBORawDataInternalStub();
void gaSetVBODisplayPropertiesStub(osSocket& apiSocket);
void gaSetCurrentThreadVBODisplayPropertiesStub();
void gaMakeThreadSetVBODisplayPropertiesStub(osSocket& apiSocket);
bool gaSetCurrentThreadVBODisplayPropertiesInternalStub();

// Static Buffers:
void gaGetAmountOfStaticBuffersObjectsStub(osSocket& apiSocket);
void gaGetStaticBufferObjectDetailsStub(osSocket& apiSocket);
void gaGetStaticBufferTypeStub(osSocket& apiSocket);
void gaUpdateCurrentThreadStaticBufferRawDataStub();
void gaUpdateCurrentThreadStaticBuffersDimensionsStub();
void gaUpdateStaticBufferRawDataStub(osSocket& apiSocket);
void gaUpdateStaticBuffersDimensionsStub(osSocket& apiSocket);
void gaMakeThreadUpdateStaticBufferRawDataStub(osSocket& apiSocket);
bool gaUpdateCurrentThreadStaticBufferRawDataInternalStub();
void gaMakeThreadUpdateStaticBuffersDimensionsStub(osSocket& apiSocket);

// PBuffers:
void gaGetAmountOfPBuffersObjectsStub(osSocket& apiSocket);
void gaGetPBufferObjectDetailsStub(osSocket& apiSocket);
void gaGetAmountOfPBufferContentObjectsStub(osSocket& apiSocket);
void gaGetPBufferStaticBufferTypeStub(osSocket& apiSocket);
void gaGetPBufferStaticBufferObjectDetailsStub(osSocket& apiSocket);
void gaUpdatePBufferStaticBufferRawDataStub(osSocket& apiSocket);
void gaUpdateCurrentThreadPBufferStaticBufferRawDataStub();
void gaMakeThreadUpdatePBufferStaticBufferRawDataStub(osSocket& apiSocket);
bool gaUpdateCurrentThreadPBufferStaticBufferRawDataInternalStub();
void gaUpdatePBuffersDimensionsStub(osSocket& apiSocket);
void gaUpdateCurrentThreadPBuffersDimensionsStub();
void gaMakeThreadUpdatePBuffersDimensionsStub(osSocket& apiSocket);

// Sync Objects:
void gaGetAmountOfSyncObjectsStub(osSocket& apiSocket);
void gaGetSyncObjectDetailsStub(osSocket& apiSocket);

// Programs and Shaders:
void gaGetAmountOfProgramObjectsStub(osSocket& apiSocket);
void gaGetActiveProgramObjectNameStub(osSocket& apiSocket);
void gaGetProgramObjectNameStub(osSocket& apiSocket);
void gaGetProgramObjectDetailsStub(osSocket& apiSocket);
void gaGetProgramActiveUniformsStub(osSocket& apiSocket);
void gaLinkProgramObjectStub(osSocket& apiSocket);
void gaLinkCurrentThreadProgramObjectStub();
void gaMakeThreadLinkProgramObjectStub(osSocket& apiSocket);
bool gaLinkCurrentThreadProgramObjectInternalStub();
void gaValidateProgramObjectStub(osSocket& apiSocket);
void gaValidateCurrentThreadProgramObjectStub();
void gaMakeThreadValidateProgramObjectStub(osSocket& apiSocket);
bool gaValidateCurrentThreadProgramObjectInternalStub();

void gaGetAmountOfShaderObjectsStub(osSocket& apiSocket);
void gaGetShaderObjectNameStub(osSocket& apiSocket);
void gaGetShaderObjectDetailsStub(osSocket& apiSocket);
void gaMarkShaderObjectSourceCodeAsForcedStub(osSocket& apiSocket);
void gaSetShaderObjectSourceCodeStub(osSocket& apiSocket);
void gaSetCurrentThreadShaderObjectSourceCodeStub();
void gaMakeThreadSetShaderObjectSourceCodeStub(osSocket& apiSocket);
bool gaSetCurrentThreadShaderObjectSourceCodeInternalStub();
void gaCompileShaderObjectStub(osSocket& apiSocket);
void gaCompileCurrentThreadShaderObjectStub();
void gaMakeThreadCompileShaderObjectStub(osSocket& apiSocket);
bool gaCompileCurrentThreadShaderObjectInternalStub();

// OpenGL Debug output:
void gaEnableGLDebugOutputLoggingStub(osSocket& apiSocket);
void gaSetGLDebugOutputSeverityEnabledStub(osSocket& apiSocket);
void gaSetGLDebugOutputKindMaskStub(osSocket& apiSocket);
void gaDoesDebugForcedContextExistStub(osSocket& apiSocket);

// Display Lists:
void gaGetAmountOfDisplayListsStub(osSocket& apiSocket);
void gaGetDisplayListObjectNameStub(osSocket& apiSocket);
void gaGetDisplayListObjectDetailsStub(osSocket& apiSocket);

// Current frame function calls:
void gaGetAmountOfCurrentFrameFunctionCallsStub(osSocket& apiSocket);
void gaGetCurrentFrameFunctionCallStub(osSocket& apiSocket);
void gaGetCurrentFrameFunctionCallDeprecationDetailsStub(osSocket& apiSocket);
void gaGetLastFunctionCallStub(osSocket& apiSocket);
void gaFindCurrentFrameFunctionCallStub(osSocket& apiSocket);
void gaGetCurrentStatisticsStub(osSocket& apiSocket);
void gaClearFunctionCallsStatisticsStub(osSocket& apiSocket);
void gaIsInOpenGLBeginEndBlockStub(osSocket& apiSocket);
void gaGetRenderPrimitivesStatisticsStub(osSocket& apiSocket);

// String markers:
void gaFindStringMarkerStub(osSocket& apiSocket);

// Breakpoints:
bool gaBreakOnOpenGLErrorsStub(osSocket& apiSocket);
void gaGetCurrentOpenGLErrorStub(osSocket& apiSocket);

// Log file recording:
void gaGetContextLogFilePathStub(osSocket& apiSocket);

// Force OpenGL flush:
void gaForceOpenGLFlushStub(osSocket& apiSocket);

// "Interactive break" mode:
void gaSetInteractiveBreakModeStub(osSocket& apiSocket);

// Forces OpenGL render modes:
void gaForceOpenGLPolygonRasterModeStub(osSocket& apiSocket);
void gaCancelOpenGLPolygonRasterModeForcingStub(osSocket& apiSocket);

// Null OpenGL Driver:
void gaSetOpenGLNullDriverStub(osSocket& apiSocket);

// force Stub:
void gaSetOpenGLForceStubStub(osSocket& apiSocket);

// Spy performance counters:
void gaGetSpyPerformanceCountersValuesStub(osSocket& apiSocket);

// "Remote" OS performance Counters:
void gaGetRemoteOSPerformanceCountersValuesStub(osSocket& apiSocket);

// iPhone performance counters:
#ifdef _GR_IPHONE_DEVICE_BUILD
    void gaAddSupportediPhonePerformanceCounterStub(osSocket& apiSocket);
    void gaInitializeiPhonePerformanceCountersReaderStub(osSocket& apiSocket);
    void gaGetiPhonePerformanceCountersValuesStub(osSocket& apiSocket);
#endif

// ATI performance counters:
void gaGetATIPerformanceCountersValuesStub(osSocket& apiSocket);
void gaActivateATIPerformanceCountersStub(osSocket& apiSocket);

#endif //__GSAPIFUNCTIONSSTUBS_H
