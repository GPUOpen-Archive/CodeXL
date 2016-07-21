//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsAPIFunctionsImplementations.h
///
//==================================================================================

//------------------------------ gsAPIFunctionsImplementations.h ------------------------------

#ifndef __GSAPIFUNCTIONSIMPLEMENTATIONS_H
#define __GSAPIFUNCTIONSIMPLEMENTATIONS_H

// Forward declaration:
class osFilePath;
class osPortAddress;
class apGLDisplayList;
class apGLItemsCollection;
class apGLProgram;
class apGLRenderBuffer;
class apGLRenderContextGraphicsInfo;
class apGLRenderContextInfo;
class apGLTexture;
class apGLPipeline;
class apGLSampler;
struct apGLTextureMipLevelID;
class apGLVBO;
class gsRenderContextMonitor;
class apRenderPrimitivesStatistics;
class apStatistics;

// Infra:
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTAPIClasses/Include/apAPIFunctionId.h>
#include <AMDTAPIClasses/Include/apContextID.h>
#include <AMDTAPIClasses/Include/apCounterID.h>
#include <AMDTAPIClasses/Include/apFunctionCall.h>
#include <AMDTAPIClasses/Include/apFunctionCallStatistics.h>
#include <AMDTAPIClasses/Include/apGLFBO.h>
#include <AMDTAPIClasses/Include/apGLShaderObject.h>
#include <AMDTAPIClasses/Include/apGLSync.h>
#include <AMDTAPIClasses/Include/apPBuffer.h>
#include <AMDTAPIClasses/Include/apRasterMode.h>
#include <AMDTAPIClasses/Include/apSearchDirection.h>
#include <AMDTAPIClasses/Include/apStaticBuffer.h>
#include <AMDTAPIClasses/Include/apTextureType.h>
#include <AMDTAPIClasses/Include/apApplicationModesEventsType.h>
#include <AMDTAPIClasses/Include/apGLDebugOutput.h>

// Utility:
bool gsMakeRenderContextCurrent(int renderContextId);

// API package:
osProcedureAddress gaBeforeDirectAPIFunctionExecutionImpl(apAPIFunctionId functionToBeCalled);

// Debugged process threads:
bool gaUpdateContextDataSnapshotImpl(int contextId);
bool gaGetThreadCurrentRenderContextImpl(const osThreadId& threadId, int& contextId);

// OpenGL render contexts:
bool gaGetAmoutOfRenderContextsImpl(int& contextsAmount);
bool gaGetRenderContextDetailsImpl(int contextId, const apGLRenderContextInfo*& pRenderContextInfo);
bool gaGetRenderContextGraphicsDetailsImpl(int contextId, const apGLRenderContextGraphicsInfo*& pRenderContextGraphicsInfo);
bool gaUpdateCurrentThreadRenderContextDataSnapshotImpl();

// OpenGL state variables:
bool gaGetDefaultOpenGLStateVariableValueImpl(int contextId, int stateVariableId, const apParameter*& pDefaultStateVariableValue);
bool gaGetOpenGLStateVariableValueImpl(int contextId, int stateVariableId, const apParameter*& pStateVariableValue);

// Textures:
bool gaGetAmountOfTextureUnitsImpl(int contextId, int& amountOfTextureUnits);
bool gaGetActiveTextureUnitImpl(int contextId, int& activeTextureUnitId);
bool gaGetTextureUnitNameImpl(int contextId, int textureUnitId, GLenum& textureUnitName);
bool gaGetEnabledTexturingModeImpl(int contextId,  int textureUnitId, bool& isTexturingEnabled, apTextureType& enabledTexturingMode);
bool gaGetAmountOfTextureObjectsImpl(int contextId, int& amountOfTextures);
bool gaGetAmountOfTexBufferObjectsImpl(int contextId, int& amountOfTextures);
bool gaGetBoundTextureImpl(int contextId, int textureUnitId, apTextureType bindTarget, GLuint& textureName);
bool gaGetTextureObjectNameImpl(int contextId, int textureId, GLuint& textureName);
bool gaGetTextureObjectTypeImpl(int contextId, int textureId, apTextureType& textureType);
bool gaGetTextureObjectDetailsImpl(int contextId, GLuint textureName, const apGLTexture*& prTextureDetails);
bool gaGetTextureMiplevelDataFilePathImpl(int contextId, apGLTextureMipLevelID miplevelId, int faceIndex, const osFilePath*& pFilePath);
bool gaMarkAllTextureImagesAsUpdatedImpl(int contextId, int textureId);
bool gaUpdateTextureRawDataImpl(int contextId, const gtVector<apGLTextureMipLevelID>& texturesVector);
bool gaUpdateCurrentThreadTextureRawDataImpl(const gtVector<apGLTextureMipLevelID>& texturesVector);
bool gaIsTextureImageDirtyImpl(int contextId, apGLTextureMipLevelID textureMiplevelId, bool& dirtyImageExists, bool& dirtyRawDataExists);
bool gaUpdateTextureParametersImpl(int contextId, const gtVector<apGLTextureMipLevelID>& texturesVector, bool shouldUpdateOnlyMemoryParams);
bool gaUpdateTextureParametersImpl(gsRenderContextMonitor* pRenderContextMonitor, const gtVector<apGLTextureMipLevelID>& texturesVector, bool shouldUpdateOnlyMemoryParams);
bool gaUpdateCurrentThreadTextureParametersImpl(const gtVector<apGLTextureMipLevelID>& textureVector, bool shouldUpdateOnlyMemoryParams);

// Render Buffers:
bool gaGetRenderBufferObjectNameImpl(int contextId, int renderBufferId, GLuint& renderBufferName);
bool gaGetAmountOfRenderBufferObjectsImpl(int contextId, int& amountOfRenderBuffers);
bool gaGetRenderBufferObjectDetailsImpl(int contextId, GLuint renderBufferName, const apGLRenderBuffer*& prRenderBufferDetails);
bool gaUpdateRenderBufferRawDataImpl(int contextId, const gtVector<GLuint>& renderBuffersVector);
bool gaUpdateCurrentThreadRenderBufferRawDataImpl(const gtVector<GLuint>& renderBuffersVector);

// Program pipelines:
bool gaGetAmountOfPipelineObjectsImpl(int contextId, int& amountOfProgramPipelines);
bool gaGetPipelineObjectDetailsImpl(int contextId, GLuint pipelineName, const apGLPipeline*& pPipelineDetails);
bool gaGetPipelineObjectNameImpl(int contextId, int pipelineIndex, GLuint& pipelineName);

// Samplers:
bool gaGetAmountOfSamplerObjectsImpl(int contextId, int& amountOfSamplers);
bool gaGetSamplerObjectDetailsImpl(int contextId, GLuint samplerName, const apGLSampler*& pSamplerDetails);
bool gaGetSamplerObjectNameImpl(int contextId, int samplerIndex, GLuint& samplerName);

// FBOs:
bool gaGetAmountOfFBOsImpl(int contextId, int& amountOfFBOs);
bool gaGetFBONameImpl(int contextId, int fboId, GLuint& fboName);
bool gaGetFBODetailsImpl(int contextId, GLuint fboName, const apGLFBO*& prFboDetails);
bool gaGetActiveFBOImpl(int contextId, GLuint& fboName);

// VBOs:
bool gaGetAmountOfVBOsImpl(int contextId, int& amountOfVBOs);
bool gaGetVBONameImpl(int contextId, int vboId, GLuint& vboName);
bool gaGetVBODetailsImpl(int contextId, GLuint vboName, const apGLVBO*& prVboDetails);
bool gaGetVBOAttachmentImpl(int contextId, GLuint vboName, GLenum& vboLastTarget, gtVector<GLenum>& vboCurrentTargets);
bool gaUpdateVBORawDataImpl(int contextId, const gtVector<GLuint>& vboNamesVector);
bool gaUpdateCurrentThreadVBORawDataImpl(const gtVector<GLuint>& vboNamesVector);
bool gaSetVBODisplayPropertiesImpl(int contextId, GLuint vboName, oaTexelDataFormat bufferDisplayFormat, int offset, GLsizei stride);
bool gaSetCurrentThreadVBODisplayPropertiesImpl(GLuint vboName, oaTexelDataFormat bufferDisplayFormat, int offset, GLsizei stride);


// Static Buffers:
bool gaGetAmountOfStaticBuffersObjectsImpl(int contextId, int& amountOfStaticBuffers);
bool gaUpdateCurrentThreadStaticBuffersDimensionsImpl();
bool gaGetStaticBufferObjectDetailsImpl(int contextId, apDisplayBuffer bufferType, const apStaticBuffer*& prStaticBuffer);
bool gaGetStaticBufferTypeImpl(int contextId, int bufferId, apDisplayBuffer& bufferType);
bool gaUpdateCurrentThreadStaticBufferRawDataImpl(apDisplayBuffer bufferType);
bool gaUpdateStaticBufferRawDataImpl(int contextId, apDisplayBuffer bufferType);
bool gaUpdateStaticBuffersDimensionsImpl(int contextId);

// PBuffers:
bool gaGetAmountOfPBuffersObjectsImpl(int& amountOfTextures);
bool gaGetPBufferObjectDetailsImpl(int pbufferId, const apPBuffer*& prPBufferDetails);
bool gaGetAmountOfPBufferStaticBuffersObjectsImpl(int pbufferId, int& amountOfStaticBuffers);
bool gaGetPBufferStaticBufferTypeImpl(int pbufferId, int staticBufferIter, apDisplayBuffer& bufferType);
bool gaGetPBufferStaticBufferObjectDetailsImpl(int pbufferId, apDisplayBuffer bufferType, const apStaticBuffer*& prStaticBuffer);
bool gaUpdateCurrentThreadPBufferStaticBufferRawDataImpl(int pbufferId, apDisplayBuffer bufferType);
bool gaUpdatePBufferStaticBufferRawDataImpl(int pbufferId, apDisplayBuffer bufferType);
bool gaUpdateCurrentThreadPBuffersDimensionsImpl();
bool gaUpdatePBuffersDimensionsImpl(int contextId);

// Sync Objects:
bool gaGetAmountOfSyncObjectsImpl(int& amountOfSyncObjects);
bool gaGetSyncObjectDetailsImpl(int syncId, const apGLSync*& pSyncDetails);

// Programs and Shaders:
bool gaGetAmountOfProgramObjectsImpl(int contextId, int& amountOfPrograms);
bool gaGetActiveProgramObjectNameImpl(int contextId, GLuint& activeProgramName);
bool gaGetProgramObjectNameImpl(int contextId, int programId, GLuint& programName);
bool gaGetProgramObjectDetailsImpl(int contextId, GLuint programName, const apGLProgram*& prProgramDetails);
bool gaGetProgramActiveUniformsImpl(int contextId, GLuint programName, const apGLItemsCollection*& activeUniforms);
bool gaLinkProgramObjectImpl(int contextId, GLuint programName, bool& wasLinkSuccessful, gtString& linkLog);
bool gaLinkCurrentThreadProgramObjectImpl(GLuint programName, bool& wasLinkSuccessful, gtString& linkLog);
bool gaValidateProgramObjectImpl(int contextId, GLuint programName, bool& wasValidationSuccessful, gtString& validationLog);
bool gaValidateCurrentThreadProgramObjectImpl(GLuint programName, bool& wasValidationSuccessful, gtString& validationLog);

bool gaGetAmountOfShaderObjectsImpl(int contextId, int& amountOfShaders);
bool gaGetShaderObjectNameImpl(int contextId, int shaderId, GLuint& shaderName);
bool gaGetShaderObjectDetailsImpl(int contextId, GLuint shaderName, gtAutoPtr<apGLShaderObject>& aptrShaderDetails);
bool gaMarkShaderObjectSourceCodeAsForcedImpl(int contextId, GLuint shaderName, bool isSourceCodeForced);
bool gaSetShaderObjectSourceCodeImpl(int contextId, GLuint shaderName, const osFilePath& sourceCodeFile);
bool gaSetCurrentThreadShaderObjectSourceCodeImpl(GLuint shaderName, const osFilePath& sourceCodeFile);
bool gaCompileShaderObjectImpl(int contextId, GLuint shaderName, bool& wasCompilationSuccessful, gtString& compilationLog);
bool gaCompileCurrentThreadShaderObjectImpl(GLuint shaderName, bool& wasCompilationSuccessful, gtString& compilationLog);

// Display Lists:
bool gaGetAmountOfDisplayListsImpl(int contextId, int& amountOfDisplayLists);
bool gaGetDisplayListObjectNameImpl(int contextID, int displayListIndex, GLuint& displayListName);
bool gaGetDisplayListObjectDetailsImpl(int contextId, GLuint displayListName, const apGLDisplayList*& prDisplayListDetails);

// Monitored function calls logging:
bool gaGetAmountOfCurrentFrameFunctionCallsImpl(int contextId, int& amountOfFunctionCalls);
bool gaGetCurrentFrameFunctionCallImpl(int contextId, int callIndex, gtAutoPtr<apFunctionCall>& aptrFunctionCall);
bool gaGetCurrentFrameFunctionCallDeprecationDetailsImpl(int contextId, int callIndex, apFunctionDeprecation& functionCallDeprecation);
bool gaGetLastFunctionCallImpl(int contextId, gtAutoPtr<apFunctionCall>& aptrFunctionCall);
bool gaFindCurrentFrameFunctionCallImpl(int contextId, apSearchDirection searchDirection, int searchStartIndex, const gtString& searchedString, bool isCaseSensitiveSearch, int& foundIndex);
bool gaGetCurrentStatisticsImpl(int contextId, apStatistics* pStatistics);
bool gaClearFunctionCallsStatisticsImpl();
bool gaIsInOpenGLBeginEndBlockImpl(int contextId);
bool gaGetRenderPrimitivesStatisticsImpl(int contextId, const apRenderPrimitivesStatistics*& pRenderPrimitivesStatistics);

// String markers:
bool gaFindStringMarkerImpl(int contextId, apSearchDirection searchDirection, int searchStartIndex, int& foundIndex);

// Breakpoints:
bool gaGetCurrentOpenGLErrorImpl(GLenum& openGLError);

// Log file recording:
bool gaGetContextLogFilePathImpl(int contextId, bool& logFileExists, const osFilePath*& filePath);

// Force OpenGL flush:
bool gaForceOpenGLFlushImpl(bool isOpenGLFlushForced);

// "Interactive break" mode:
bool gaSetInteractiveBreakModeImpl(bool isInteractiveBreakOn);

// Stub Mode:
bool gaSetOpenGLForceStubImpl(apOpenGLForcedModeType stubType, bool isStubForced);

// Forces OpenGL render modes:
bool gaForceOpenGLPolygonRasterModeImpl(apRasterMode rasterMode);
bool gaCancelOpenGLPolygonRasterModeForcingImpl();

// Null OpenGL Driver:
bool gaSetOpenGLNullDriverImpl(bool isNULLOpenGLImplOn);

// Spy performance counters:
bool gaGetSpyPerformanceCountersValuesImpl(const double*& pValuesArray, int& amountOfValues);

// "Remote" OS performace counters:
bool gaGetRemoteOSPerformanceCountersValuesImpl(const double*& pValuesArray, int& amountOfValues);

// iPhone performance counters:
#ifdef _GR_IPHONE_DEVICE_BUILD
    bool gaAddSupportediPhonePerformanceCounterImpl(int counterIndex, const gtString& counterName);
    bool gaInitializeiPhonePerformanceCountersReaderImpl();
    bool gaGetiPhonePerformanceCountersValuesImpl(const double*& pValuesArray, int& amountOfValues);
#endif

// ATI performance counters:
bool gaGetATIPerformanceCountersValuesImpl(const double*& pValuesArray, int& amountOfValues);
bool gaActivateATIPerformanceCountersImpl(const gtVector<apCounterActivationInfo>& countersActivationInfosVec);

// OpenGL Debug output:
bool gaEnableGLDebugOutputLoggingImpl(bool glDebugOutputIntegrationEnabled);
bool gaSetGLDebugOutputSeverityEnabledImpl(apGLDebugOutputSeverity severity, bool enabled);
bool gaSetGLDebugOutputKindMaskImpl(const gtUInt64& debugOutputCategoryMask);
bool gaDoesDebugForcedContextExistImpl(bool isDebugOn, bool& isDebugContextExist);

#endif //__GSAPIFUNCTIONSIMPLEMENTATIONS_H
