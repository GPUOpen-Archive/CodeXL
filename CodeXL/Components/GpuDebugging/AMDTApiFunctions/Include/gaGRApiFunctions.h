//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gaGRApiFunctions.h
///
//==================================================================================

//------------------------------ gaGRApiFunctions.h ------------------------------

#ifndef __GAGRAPIFUNCTIONS
#define __GAGRAPIFUNCTIONS

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>

// Removing artifact code related to on-device debugging:
// #define _SUPPORT_REMOTE_EXECUTION

// Forward deceleration:
struct apCounterID;
class apCounterInfo;
struct apGLTextureMipLevelID;
struct apGPUInfo;
class apBreakPoint;
class apCLBuffer;
class apCLSubBuffer;
class apCLCommandQueue;
class apCLContext;
class apCLDevice;
class apCLEnqueuedCommand;
class apCLEvent;
class apCLKernel;
class apCLPipe;
class apCLProgram;
class apCLSampler;
class apCLImage;
class apContextID;
class apFunctionCall;
class apFunctionDeprecation;
class apGLDisplayList;
class apGLFBO;
class apGLItemsCollection;
class apGLProgram;
class apGLPipeline;
class apGLSampler;
class apGLRenderBuffer;
class apGLRenderContextGraphicsInfo;
class apGLRenderContextInfo;
class apGLShaderObject;
class apGLSync;
class apGLTexture;
class apGLTextureMiplevelData;
class apGLTextureMemoryData;
class apGLTextureData;
class apGLVBO;
class apKernelSourceCodeBreakpoint;
class apPBuffer;
class apParameter;
class apDebugProjectSettings;
class apRenderPrimitivesStatistics;
class apStaticBuffer;
class apStatistics;
class osCallStack;
class osFilePath;
class osRawMemoryBuffer;
class apOpenCLQueueDeletedEvent;
class apOpenCLQueueCreatedEvent;
class apCLObjectID;
struct apCounterID;
struct apCounterActivationInfo;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    enum oaTexelDataFormat;
    enum osTransferableObjectType;
    enum apAPIConnectionType;
    enum apContextType;
    enum apCounterType;
    enum apGLDebugOutputSeverity;
    enum apKernelDebuggingCommand;
    enum apMultipleKernelDebuggingDispatchMode;
    enum apMonitoredFunctionId;
    enum apOpenCLExecutionType;
    enum apOpenGLForcedModeType;
    enum apAPIVersion;
    enum apSearchDirection;
    enum apTextureType;
#else // AMDT_BUILD_TARGET != AMDT_WINDOWS_OS
    // GCC does not allow forward declaration of enum types:
    #include <AMDTOSWrappers/Include/osTransferableObjectType.h>
    #include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>
    #include <AMDTAPIClasses/Include/apAPIConnectionType.h>
    #include <AMDTAPIClasses/Include/apContextID.h>
    #include <AMDTAPIClasses/Include/apCounterType.h>
    #include <AMDTAPIClasses/Include/apGLDebugOutput.h>
    #include <AMDTAPIClasses/Include/apKernelDebuggingCommand.h>
    #include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
    #include <AMDTAPIClasses/Include/apApplicationModesEventsType.h>
    #include <AMDTAPIClasses/Include/apAPIVersion.h>
    #include <AMDTAPIClasses/Include/apSearchDirection.h>
    #include <AMDTAPIClasses/Include/apTextureType.h>
#endif

// Infra:
#include <AMDTBaseTools/Include/gtAutoPtr.h>

#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTAPIClasses/Include/apAPIFunctionId.h>
#include <AMDTAPIClasses/Include/apDisplayBuffer.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTAPIClasses/Include/apExpression.h>
#include <AMDTAPIClasses/Include/apGenericBreakpoint.h>
#include <AMDTAPIClasses/Include/apRasterMode.h>

// Local:
#include <AMDTApiFunctions/Include/apAPIFunctionsDLLBuild.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// Uri, 28/9/10: IMPORTANT NOTE:
// Whenever an API function is added, it should be implemented as follows:
// 1. Add, in this header file, a VIRTUAL function to the gaGRApiFunctions
//      class.
// 2. Add, in the same place in the function list, a global function with
//      the GA_API qualifier. This function MUST have the same name, return
//      type and parameters as the function added in (1)
// 3. In this file's .cpp counterpart, add the actual function body, in the
//      same place in the function list, as the member function of the
//      gaGRApiFunctions class.
// 4. In the .cpp, add the GA_CONNECT_API_FUNCTION_WRAPPER_TO_GRAPIFUNCTIONS
//      macro for the function in the same place in the function list
//
// All 4 of these places must also be updated when an API function is changed
//  or compilation errors will ensue.
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------------
// Class Name:           GA_API gaGRApiFunctions
// General Description: A class encompassing all the API functions, allowing them to
//                      be overridden by subclasses if we wish to use different
//                      implementations of these functions in some contexts.
// Author:               Uri Shomroni
// Creation Date:        27/9/2010
// ----------------------------------------------------------------------------------
class GA_API gaGRApiFunctions
{
public:
    gaGRApiFunctions();
    virtual ~gaGRApiFunctions();

    static gaGRApiFunctions& instance();
    static void registerInstance(gaGRApiFunctions& myInstance);

    // API package:
    virtual bool gaIntializeAPIPackage(bool shouldInitPerfCounters);
    virtual bool gaIsAPIConnectionActive(apAPIConnectionType apiType);
    virtual bool gaIsAPIConnectionActiveAndDebuggedProcessSuspended(apAPIConnectionType apiType);

    // Debugged process:
    virtual bool gaDebuggedExecutableExists(const apDebugProjectSettings& processCreationData);
    virtual bool gaSystemLibrariesExists(const apDebugProjectSettings& processCreationData, gtString& missingSystemLibraries);
    virtual bool gaLaunchDebuggedProcess(const apDebugProjectSettings& processCreationData, bool launchSuspended);
    virtual bool gaContinueDebuggedProcessFromSuspendedCreation();
    virtual bool gaDebuggedProcessExists();
    virtual bool gaTerminateDebuggedProcess();
    virtual bool gaSuspendDebuggedProcess();
    virtual bool gaResumeDebuggedProcess();
    virtual bool gaLockDriverThreads();
    virtual bool gaUnLockDriverThreads();
    virtual bool gaSuspendThreads(const std::vector<osThreadId>& thrds);
    virtual bool gaResumeThreads();
    virtual bool gaIsDebuggedProcessSuspended();
    virtual bool gaIsDuringDebuggedProcessTermination();
    virtual bool gaIsDebugging64BitApplication();
    virtual bool gaGetCurrentDebugSessionLogFilesSubDirectory(osFilePath& logFilesPath);

    // Debugged process threads:
    virtual bool gaGetAmountOfDebuggedProcessThreads(int& threadsAmount);
    virtual bool gaGetThreadId(int threadIndex, osThreadId& threadId);
    virtual bool gaGetBreakpointTriggeringThreadIndex(int& threadIndex);
    virtual bool gaGetThreadCurrentRenderContext(const osThreadId& threadId, int& contextId);
    virtual bool gaGetBreakpointTriggeringContextId(apContextID& contextId);
    virtual bool gaGetRenderContextCurrentThread(int contextId, osThreadId& threadId);
    virtual bool gaGetThreadCallStack(const osThreadId& threadId, osCallStack& callStack, bool hideSpyDLLsFunctions = true, bool fetchRemoteSourceFiles = true);
    virtual bool gaCanGetHostVariables();
    virtual bool gaCanGetHostDebugging();
    virtual bool gaHostDebuggerStepIn(osThreadId threadId);
    virtual bool gaHostDebuggerStepOut(osThreadId threadId);
    virtual bool gaHostDebuggerStepOver(osThreadId threadId);
    virtual bool gaGetThreadLocals(const osThreadId& threadId, int frameIndex, int evalDepth, gtVector<apExpression>& o_locals, bool onlyNames);
    virtual bool gaGetThreadExpressionValue(const osThreadId& threadId, int frameIndex, const gtString& expressionText, int evalDepth, apExpression& o_exp);
    virtual bool gaIsHostBreakPoint();
    virtual bool gaSetHostBreakpoint(const osFilePath& filePath, int lineNumber);
    virtual bool gaGetHostBreakpointLocation(osFilePath& bpFile, int& bpLine);

    // Render contexts:
    virtual bool gaGetAmountOfRenderContexts(int& contextsAmount);
    virtual bool gaGetRenderContextDetails(int contextId, apGLRenderContextInfo& renderContextInfo);
    virtual bool gaGetRenderContextGraphicDetails(int contextId, apGLRenderContextGraphicsInfo& renderContextGraphicsInfo);
    virtual bool gaGetRenderContextLivingShareListsHolder(int contextID, int& livingContextID);

    // OpenCL contexts:
    virtual bool gaGetOpenCLContextDetails(int contextId, apCLContext& renderContextInfo);

    // OpenCL & OpenGL contexts:
    virtual bool gaWasContextDeleted(const apContextID& contextId);

    // Monitored state variables:
    virtual bool gaGetAmountOfOpenGLStateVariables(int& amountOfStateVariables);
    virtual bool gaGetOpenGLStateVariableName(int stateVariableId, gtString& stateVariableName);
    virtual bool gaGetOpenGLStateVariableGlobalType(int stateVariableId, unsigned int& stateVariableGlobalType);
    virtual bool gaGetOpenGLStateVariableId(const gtString& stateVariableName, int& stateVariableId);

    // Monitored functions:
    virtual bool gaGetAmountOfMonitoredFunctions(int& amountOfFunctions);
    virtual bool gaGetMonitoredFunctionName(apMonitoredFunctionId functionId, gtString& functionName);
    virtual bool gaGetMonitoredFunctionId(const gtString& functionName, apMonitoredFunctionId& functionId);
    virtual bool gaGetMonitoredFunctionType(apMonitoredFunctionId functionId, unsigned int& functionType);
    virtual bool gaGetMonitoredFunctionAPIType(apMonitoredFunctionId functionId, unsigned int& functionAPIType);
    virtual bool gaGetMonitoredFunctionDeprecationVersion(apMonitoredFunctionId functionId, apAPIVersion& deprectedAtVersion, apAPIVersion& removedAtVersion);

    // State variable values:
    virtual bool gaGetDefaultOpenGLStateVariableValue(int contextId, int stateVariableId, gtAutoPtr<apParameter>& aptrStateVariableValue);
    virtual bool gaGetOpenGLStateVariableValue(int contextId, int stateVariableId, gtAutoPtr<apParameter>& aptrStateVariableValue);

    // Textures:
    virtual bool gaEnableImagesDataLogging(bool isImagesDataLogged);
    virtual bool gaIsImagesDataLogged(bool& isImagesDataLogged);

    virtual bool gaGetAmountOfTextureUnits(int contextId, int& amountOfTextureUnits);
    virtual bool gaGetActiveTextureUnit(int contextId, int& activeTextureUnitId);
    virtual bool gaGetTextureUnitName(int contextId, int textureUnitId, GLenum& textureUnitName);
    virtual bool gaGetEnabledTexturingMode(int contextId, int textureUnitId, bool& isTexturingEnabled, apTextureType& enabledTexturingMode);
    virtual bool gaGetBoundTexture(int contextId, int textureUnitId, apTextureType bindTarget, GLuint& textureName);

    virtual bool gaGetAmountOfTextureObjects(int contextId, int& amountOfTextures);
    virtual bool gaGetTextureObjectName(int contextId, int textureId, GLuint& textureName);
    virtual bool gaGetTextureObjectType(int contextId, int textureId, apTextureType& textureType);
    virtual bool gaGetTextureObjectDetails(int contextId, GLuint textureName, apGLTexture& textureDetails);
    virtual bool gaGetTextureObjectThumbnailData(int contextId, GLuint textureName, apGLTextureMiplevelData& textureThumbDetails);
    virtual bool gaGetTextureDataObjectDetails(int contextId, GLuint textureName, apGLTextureData& textureData);
    virtual bool gaGetTextureMemoryDataObjectDetails(int contextId, GLuint textureName, apGLTextureMemoryData& textureData);
    virtual bool gaUpdateTextureRawData(int contextId, const gtVector<apGLTextureMipLevelID>& texturesVector);
    virtual bool gaGetTextureMiplevelDataFilePath(int contextId, apGLTextureMipLevelID textureMiplevelId, int faceIndex, osFilePath& filePath);
    virtual bool gaUpdateTextureParameters(int contextId, const gtVector<apGLTextureMipLevelID>& texturesVector, bool shouldUpdateOnlyMemoryParams);
    virtual bool gaIsTextureImageDirty(int contextId, apGLTextureMipLevelID textureMiplevelId, bool& dirtyImageExists, bool& dirtyRawDataExists);
    virtual bool gaMarkAllTextureImagesAsUpdated(int contextId, int textureId);

    // Render buffers:
    virtual bool gaGetAmountOfRenderBufferObjects(int contextId, int& amountOfRenderBuffers);
    virtual bool gaGetRenderBufferObjectName(int contextId, int renderBufferId, GLuint& renderBufferName);
    virtual bool gaGetRenderBufferObjectDetails(int contextId, GLuint renderBufferName, apGLRenderBuffer& renderBufferDetails);
    virtual bool gaUpdateRenderBufferRawData(int contextId, const gtVector<GLuint>& renderBuffersVector);

    // Program Pipelines:
    virtual bool gaGetAmountOfPipelineObjects(int contextId, int& amountOfProgramPipelines);
    virtual bool gaGetPipelineObjectDetails(int contextId, GLuint pipelineName, apGLPipeline& programPipelineDetails);
    virtual bool gaGetPipelineObjectName(int contextId, int pipelineIndex, GLuint& pipelineName);

    // OpenGL Samplers:
    virtual bool gaGetAmountOfSamplerObjects(int contextId, int& amountOfSamplers);
    virtual bool gaGetSamplerObjectDetails(int contextId, GLuint samplerName, apGLSampler& samplerDetails);
    virtual bool gaGetSamplerObjectName(int contextId, int samplerIndex, GLuint& samplerName);

    // FBOs
    virtual bool gaGetAmountOfFBOs(int contextId, int& amountOfFBOs);
    virtual bool gaGetFBOName(int contextId, int fboId, GLuint& fboName);
    virtual bool gaGetFBODetails(int contextId, GLuint fboName, apGLFBO& fboDetails);
    virtual bool gaGetActiveFBO(int contextId, GLuint& fboName);

    // VBOs
    virtual bool gaGetAmountOfVBOs(int contextId, int& amountOfVBOs);
    virtual bool gaGetVBOName(int contextId, int vboId, GLuint& vboName);
    virtual bool gaGetVBODetails(int contextId, GLuint vboName, apGLVBO& vboDetails);
    virtual bool gaGetVBOAttachment(int contextId, GLuint vboName, GLenum& vboLastTarget, gtVector<GLenum>& vboCurrentTargets);
    virtual bool gaUpdateVBORawData(int contextId, const gtVector<GLuint>& vboNamesVector);
    virtual bool gaSetVBODisplayProperties(int contextId, GLuint vboName, oaTexelDataFormat displayFormat, int offset, GLsizei stride);

    // Static Buffers:
    virtual bool gaGetAmountOfStaticBuffersObjects(int contextId, int& amountOfStaticBuffers);
    virtual bool gaGetStaticBufferType(int contextId, int staticBufferId, apDisplayBuffer& bufferType);
    virtual bool gaGetStaticBufferObjectDetails(int contextId, apDisplayBuffer bufferType, apStaticBuffer& staticBufferDetails);
    virtual bool gaUpdateStaticBufferRawData(int contextId, apDisplayBuffer bufferType);
    virtual bool gaUpdateStaticBuffersDimension(int contextId);

    // PBuffers:
    virtual bool gaGetAmountOfPBuffersObjects(int& amountOfPBuffers);
    virtual bool gaGetPBufferObjectDetails(int pbufferID, apPBuffer& pbufferDetails);
    virtual bool gaGetAmountOfPBufferContentObjects(int pbufferID, int& amountOfPBufferContentObjects);
    virtual bool gaGetPBufferStaticBufferType(int pbufferID, int staticBufferIter, apDisplayBuffer& bufferType);
    virtual bool gaGetPBufferStaticBufferObjectDetails(int pbufferID, apDisplayBuffer bufferType, apStaticBuffer& staticBufferDetails);
    virtual bool gaUpdatePBufferStaticBufferRawData(int pbufferContextId, int pbufferID, apDisplayBuffer bufferType);
    virtual bool gaUpdatePBuffersDimension(int contextId);

    // Sync objects:
    virtual bool gaGetAmountOfSyncObjects(int& amountOfSyncObjects);
    virtual bool gaGetSyncObjectDetails(int syncId, apGLSync& syncObjectDetails);

    // Programs and Shaders:
    virtual bool gaGetAmountOfProgramObjects(int contextId, int& amountOfPrograms);
    virtual bool gaGetProgramObjectName(int contextId, int programId, GLuint& programName);
    virtual bool gaGetActiveProgramObjectName(int contextId, GLuint& activeProgramName);
    virtual bool gaGetProgramObjectDetails(int contextId, GLuint programName, apGLProgram& programDetails);
    virtual bool gaGetProgramActiveUniforms(int contextId, GLuint programName, apGLItemsCollection& activeUniforms);
    virtual bool gaLinkProgramObject(int contextId, GLuint programName, bool& wasLinkSuccessful, gtString& linkLog);
    virtual bool gaValidateProgramObject(int contextId, GLuint programName, bool& wasValidationSuccessful, gtString& validationLog);

    virtual bool gaGetAmountOfShaderObjects(int contextId, int& amountOfShaders);
    virtual bool gaGetShaderObjectName(int contextId, int shaderId, GLuint& shaderName);
    virtual bool gaGetShaderObjectDetails(int contextId, GLuint shaderName, gtAutoPtr<apGLShaderObject>& aptrShaderDetails);
    virtual bool gaMarkShaderObjectSourceCodeAsForced(int contextId, GLuint shaderName, bool isSourceCodeForced = true);
    virtual bool gaSetShaderObjectSourceCode(int contextId, GLuint shaderName, const osFilePath& sourceCodeFile);
    virtual bool gaCompileShaderObject(int contextId, GLuint shaderName, bool& wasCompilationSuccessful, gtString& compilationLog);

    // Display Lists:
    virtual bool gaGetAmountOfDisplayLists(int contextID, int& amountOfDisplayLists);
    virtual bool gaGetDisplayListObjectName(int contextID, int displayListIndex, GLuint& displayListName);
    virtual bool gaGetDisplayListObjectDetails(int contextID, GLuint displayListName, apGLDisplayList& pDisplayListDetails);

    // OpenGL / OpenCL Function calls:
    virtual bool gaGetAmountOfCurrentFrameFunctionCalls(const apContextID& contextID, int& amountOfFunctionCalls);
    virtual bool gaGetCurrentFrameFunctionCall(const apContextID& contextID, int callIndex, gtAutoPtr<apFunctionCall>& aptrFunctionCall);
    virtual bool gaGetLastFunctionCall(const apContextID& contextID, gtAutoPtr<apFunctionCall>& aptrFunctionCall);
    virtual bool gaFindCurrentFrameFunctionCall(const apContextID& contextID, apSearchDirection searchDirection, int searchStartIndex, const gtString& searchedString, bool isCaseSensitiveSearch, int& foundIndex);
    virtual bool gaClearFunctionCallsStatistics();
    virtual bool gaGetCurrentStatistics(const apContextID& contextID, apStatistics* pStatistics);

    // OpenGL function calls:
    virtual bool gaGetCurrentFrameFunctionCallDeprecationDetails(int contextId, int callIndex, apFunctionDeprecation& functionDeprecationDetails);
    virtual bool gaIsInOpenGLBeginEndBlock(int contextId);
    virtual bool gaGetRenderPrimitivesStatistics(int contextId, apRenderPrimitivesStatistics& renderPrimitivesStatistics);

    // OpenCL handles:
    virtual bool gaGetOpenCLHandleObjectDetails(oaCLHandle openCLHandlePtr, apCLObjectID& clObjectIdDetails);

    // OpenCL Contexts:
    virtual bool gaGetAmountOfOpenCLContexts(int& amountOfContexts);

    // OpenCL programs:
    virtual bool gaGetAmountOfOpenCLProgramObjects(int contextId, int& amountOfPrograms);
    virtual bool gaGetOpenCLProgramObjectDetails(int contextId, int programIndex, apCLProgram& programDetails);
    virtual bool gaSetOpenCLProgramCode(oaCLProgramHandle programHandle, const osFilePath& newSourcePath);
    virtual bool gaBuildOpenCLProgram(oaCLProgramHandle programHandle, apCLProgram*& pFailedProgramData);
    virtual bool gaGetOpenCLProgramHandleFromSourceFilePath(const osFilePath& sourceFilePath, osFilePath& newTempSourceFilePath, oaCLProgramHandle& programHandle);
    virtual bool gaSetKernelSourceFilePath(gtVector<osFilePath>& programsFilePath);

    // OpenCL kernels:
    virtual bool gaGetOpenCLKernelObjectDetails(int contextId, oaCLKernelHandle kernelHandle, apCLKernel& kernelDetails);

    // OpenCL kernel debugging:
    virtual bool gaGetKernelDebuggingLocation(oaCLProgramHandle& debuggedProgramHandle, int& currentLineNumber);
    virtual bool gaGetCurrentlyDebuggedKernelDetails(apCLKernel& kernelDetails);
    virtual bool gaGetCurrentlyDebuggedKernelCallStack(osCallStack& kernelStack);
    virtual bool gaSetKernelDebuggingCommand(apKernelDebuggingCommand command);
    virtual bool gaGetKernelDebuggingGlobalWorkOffset(int coordinate, int& dimension);
    virtual bool gaGetKernelDebuggingGlobalWorkOffset(int& xOffset, int& yOffset, int& zOffset);
    virtual bool gaGetKernelDebuggingGlobalWorkSize(int coordinate, int& dimension);
    virtual bool gaGetKernelDebuggingGlobalWorkSize(int& xDimension, int& yDimension, int& zDimension);
    virtual bool gaGetKernelDebuggingLocalWorkSize(int coordinate, int& dimension);
    virtual bool gaGetKernelDebuggingLocalWorkSize(int& xDimension, int& yDimension, int& zDimension, int& amountOfDimensions);
    virtual bool gaSetKernelDebuggingCurrentWorkItemCoordinate(int coordinate, int value);
    virtual bool gaGetKernelDebuggingCurrentWorkItemCoordinate(int coordinate, int& value);
    virtual bool gaGetKernelDebuggingCurrentWorkItem(int& xValue, int& yValue, int& zValue);
    virtual bool gaIsInKernelDebugging();
    virtual bool gaSetKernelSteppingWorkItem(const int coordinate[3]);
    virtual bool gaUpdateKernelSteppingWorkItemToCurrentCoordinate();
    virtual bool gaIsWorkItemValid(const int coordinate[3]);
    virtual bool gaGetFirstValidWorkItem(int wavefrontIndex, int coordinate[3]);
    virtual bool gaCanGetKernelVariableValue(const gtString& variableName);
    virtual bool gaGetKernelDebuggingExpressionValue(const gtString& expressionString, const int workItem[3], int evalDepth, apExpression& o_exp);
    virtual bool gaGetKernelDebuggingAvailableVariables(int evalDepth, gtVector<apExpression>& o_locals, bool onlyLeaves, int stackFrameDepth, bool onlyNames);
    virtual bool gaGetKernelDebuggingAmountOfActiveWavefronts(int& amountOfWavefronts);
    virtual bool gaGetKernelDebuggingActiveWavefrontID(int wavefrontIndex, int& wavefrontId);
    virtual bool gaGetKernelDebuggingWavefrontIndex(const int coordinate[3], int& wavefrontIndex);
    virtual bool gaUpdateKernelVariableValueRawData(const gtString& variableName, bool& variableTypeSupported, osFilePath& variableRawData);
    virtual bool gaGetKernelSourceCodeBreakpointResolution(oaCLProgramHandle programHandle, int requestedLineNumber, int& resolvedLineNumber);
    virtual bool gaSetKernelDebuggingEnable(bool kernelEnable);
    virtual bool gaSetMultipleKernelDebugDispatchMode(apMultipleKernelDebuggingDispatchMode mode);

    // OpenCL device:
    virtual bool gaGetOpenCLDeviceObjectDetails(int deviceId, apCLDevice& deviceDetails);

    // OpenCL platforms:
    virtual bool gaGetOpenCLPlatformAPIID(gtUInt64 platformId, int& platformName);

    // OpenCL buffers:
    virtual bool gaGetAmountOfOpenCLBufferObjects(int contextId, int& amountOfBuffers);
    virtual bool gaGetOpenCLBufferObjectDetails(int contextId, int bufferIndex, apCLBuffer& bufferDetails);
    virtual bool gaUpdateOpenCLBufferRawData(int contextId, const gtVector<int>& bufferIdsVector);
    virtual bool gaSetCLBufferDisplayProperties(int contextId, int bufferId, oaTexelDataFormat displayFormat, int displayOffset, gtSize_t displayStride);

    // OpenCL Sub Buffers:
    virtual bool gaGetOpenCLSubBufferObjectDetails(int contextId, int subBufferName, apCLSubBuffer& subBufferDetails);
    virtual bool gaUpdateOpenCLSubBufferRawData(int contextId, const gtVector<int>& bufferIdsVector);
    virtual bool gaSetCLSubBufferDisplayProperties(int contextId, int subBufferId, oaTexelDataFormat displayFormat, int displayOffset, gtSize_t displayStride);

    // OpenCL images:
    virtual bool gaGetAmountOfOpenCLImageObjects(int contextId, int& amountOfTextures);
    virtual bool gaGetOpenCLImageObjectDetails(int contextId, int textureIndex, apCLImage& textureDetails);
    virtual bool gaUpdateOpenCLImageRawData(int contextId, const gtVector<int>& textureIdsVector);

    // OpenCL pipes:
    virtual bool gaGetAmountOfOpenCLPipeObjects(int contextId, int& amountOfPipes);
    virtual bool gaGetOpenCLPipeObjectDetails(int contextId, int pipeIndex, apCLPipe& pipeDetails);

    // OpenCL command queues:
    virtual bool gaGetAmountOfCommandQueues(int contextId, int& amountOfQueues);
    virtual bool gaGetCommandQueueDetails(int contextId, int queueIndex, apCLCommandQueue& commandQueueDetails);
    virtual bool gaGetAmountOfCommandsInQueue(oaCLCommandQueueHandle hQueue, int& amountOfCommands);
    virtual bool gaGetAmountOfEventsInQueue(oaCLCommandQueueHandle hQueue, int& amountOfEvents);
    virtual bool gaGetEnqueuedCommandDetails(oaCLCommandQueueHandle hQueue, int commandIndex, gtAutoPtr<apCLEnqueuedCommand>& aptrCommand);

    // OpenCL samplers:
    virtual bool gaGetAmountOfOpenCLSamplers(int contextId, int& amountOfSamplers);
    virtual bool gaGetOpenCLSamplerObjectDetails(int contextId, int samplerIndex, apCLSampler& samplerDetails);

    // OpenCL events:
    virtual bool gaGetAmountOfOpenCLEvents(int contextId, int& amountOfEvents);
    virtual bool gaGetOpenCLEventObjectDetails(int contextId, int eventIndex, apCLEvent& eventDetails);

    // Allocated Objects
    virtual bool gaGetAmountOfRegisteredAllocatedObjects(unsigned int& numberOfObjects);
    virtual bool gaGetAllocatedObjectCreationStack(int allocatedObjectId, osCallStack& callStack);
    virtual bool gaCollectAllocatedObjectsCreationCallsStacks(bool collectCreationStacks);
    virtual bool gaGetCollectAllocatedObjectCreationCallsStacks(bool& collectCreationStacks);

    // String markers:
    virtual bool gaFindStringMarker(int contextId, apSearchDirection searchDirection, int searchStartIndex, int& foundIndex);

    // Breakpoints:
    virtual bool gaSetBreakpoint(const apBreakPoint& breakpoint);
    virtual bool gaRemoveBreakpoint(int breakpointId);
    virtual bool gaRemoveGenericBreakpoint(apGenericBreakpointType breakpointType);
    virtual bool gaGetAmountOfBreakpoints(int& amountOfBreakpoints);
    virtual bool gaGetBreakpoint(int breakPointId, gtAutoPtr<apBreakPoint>& aptrBreakpoint);
    virtual bool gaGetBreakpointIndex(const apBreakPoint& breakpoint, int& breakpointId);
    virtual bool gaRemoveAllBreakpoints();
    virtual bool gaRemoveAllBreakpointsByType(osTransferableObjectType breakpointType);
    virtual bool gaGetEnableAllBreakpointsStatus(bool& isEnableAllBreakpointsChecked, bool& isEnableAllBreakpointsEnabled);
    virtual bool gaSetBreakpointHitCount(int breakpointIndex, int hitCount);
    virtual bool gaSetKernelBreakpointProgramHandle(int breakpointIndex, oaCLProgramHandle programHandle);
    virtual bool gaRemoveAllBreakpointsByState(const apBreakPoint::State state);
    virtual bool gaTemporarilyDisableAllBreakpoints();
    virtual bool gaEnableAllBreakpointsByState(const apBreakPoint::State state);


    virtual bool gaBreakOnNextMonitoredFunctionCall();
    virtual bool gaBreakOnNextDrawFunctionCall();
    virtual bool gaBreakOnNextFrame();
    virtual bool gaBreakInMonitoredFunctionCall();
    virtual bool gaClearAllStepFlags();
    virtual bool gaGetGenericBreakpointStatus(apGenericBreakpointType breakpointType, bool& doesExist, bool& isEnabled);

    // Log file recording:
    virtual bool gaDeleteLogFilesWhenDebuggedProcessTerminates(bool deleteLogFiles);
    virtual bool gaStartMonitoredFunctionsCallsLogFileRecording();
    virtual bool gaStopMonitoredFunctionsCallsLogFileRecording();
    virtual bool gaIsMonitoredFunctionsCallsLogFileRecordingActive(bool& isActive);
    virtual bool gaGetContextLogFilePath(apContextID contextID, bool& logFileExists, osFilePath& filePath);
    virtual bool gaFlushLogFileAfterEachFunctionCall(bool flushAfterEachFunctionCall);
    virtual bool gaIsLogFileFlushedAfterEachFunctionCall(bool& isLogFileFlushedAfterEachFunctionCall);
    virtual bool gaWasOpenGLDataRecordedInDebugSession();
    virtual bool gaResetRecordingWasDoneFlag(bool isEnabled);

    // Slow motion:
    virtual bool gaSetSlowMotionDelay(int delayTimeUnits);
    virtual bool gaGetSlowMotionDelay(int& delayTimeUnits);

    // Force OpenGL flush:
    virtual bool gaForceOpenGLFlush(bool isOpenGLFlushForced);
    virtual bool gaIsOpenGLFlushForced(bool& isOpenGLFlushForced);

    // "Interactive" break mode:
    virtual bool gaSetInteractiveBreakMode(bool isInteractiveBreakOn);
    virtual bool gaIsInteractiveBreakOn();

    virtual bool gaSetDebuggedProcessExecutionMode(apExecutionMode executionMode);
    virtual bool gaGetDebuggedProcessExecutionMode(apExecutionMode& executionMode);

    // Forces OpenGL render modes:
    virtual bool gaForceOpenGLPolygonRasterMode(apRasterMode rasterMode);
    virtual bool gaCancelOpenGLPolygonRasterModeForcing();
    virtual bool gaIsOpenGLPolygonRasterModeForced(bool& isForced);
    virtual bool gaGetForceOpenGLPolygonRasterMode(apRasterMode& rasterMode);

    // Turn off graphic pipeline stages:
    virtual bool gaSwitchToNULLOpenGLImplementation(bool isNULLOpenGLImplOn);
    virtual bool gaIsUnderNULLOpenGLImplementation(bool& isUnderNULLOpenGLImplementation);

    // OpenGL Force stub operations:
    virtual bool gaForceOpenGLStub(apOpenGLForcedModeType openGLStubType, bool isStubForced);
    virtual bool gaIsOpenGLStubForced(apOpenGLForcedModeType openGLStubType, bool& isStubForced);

    // OpenCL cancel operations:
    virtual bool gaIsOpenCLExecutionOn(apOpenCLExecutionType executionType, bool& isExecutionOn);
    virtual bool gaSetOpenCLExecution(apOpenCLExecutionType executionType, bool isExecutionOn);

    // Performance counters:
    virtual bool gaGetAmountOfPerformanceCounters(int& countersAmount);
    virtual bool gaGetAmountOfPerformanceCountersByType(apCounterType counterType, int& countersAmount);
    virtual bool gaAddOSCounter(const apCounterInfo& newCounterInfo, int& newCounterIndex, bool& isNewCounter);
    virtual bool gaRemoveOSCounter(int counterIndex);

    virtual const apCounterInfo* gaGetPerformanceCounterInfo(int counterIndex);
    virtual bool gaGetPerformanceCounterLocalIndex(int counterIndex, int& counterLocalIndex);
    virtual bool gaGetPerformanceCounterType(int counterIndex, apCounterType& counterType);
    virtual bool gaGetPerformanceCounterIndex(const apCounterID& counterId, int& counterIndex);
    virtual bool gaActivatePerformanceCounter(const apCounterActivationInfo& counterActivationInfo);
    virtual bool gaUpdatePerformanceCountersValues();
    virtual double gaGetPerformanceCounterValue(const apCounterID& counterId);

    // OpenCL queues:
    virtual void gaOnQueueCreatedEvent(const apOpenCLQueueCreatedEvent& eve);
    virtual void gaOnQueueDeletedEvent(const apOpenCLQueueDeletedEvent& eve);

    // OpenGL debug output messages:
    virtual bool gaIsGLDebugOutputSupported();
    virtual bool gaEnableGLDebugOutputLogging(bool isGLDebugOutputLoggingEnabled);
    virtual bool gaGetGLDebugOutputLoggingEnabledStatus(bool& isGLDebugOutputLoggingEnabled);
    virtual bool gaGetGLDebugOutputSeverityEnabled(apGLDebugOutputSeverity severity, bool& enabled);
    virtual bool gaSetGLDebugOutputSeverityEnabled(apGLDebugOutputSeverity severity, bool enabled);
    virtual bool gaSetGLDebugOutputKindMask(const gtUInt64& debugReportMask);
    virtual bool gaGetGLDebugOutputKindMask(gtUInt64& debugReportMask);
    virtual bool gaDoesDebugForcedContextExist(bool isDebugContext, bool& isDebugContextForced);

    // GPUs:
    virtual bool gaGetAmountOfLocalMachineGPUs(int& GPUsAmount);
    virtual bool gaGetLocalMachineGPUInformation(int GPUIndex, apGPUInfo& GPUInfo);

    // Sending files through the API pipe:
    virtual bool gaReadFile(const osFilePath& remoteFilePath, osRawMemoryBuffer& memoryBuffer);
    virtual bool gaWriteFile(const osFilePath& remoteFilePath, const osRawMemoryBuffer& memoryBuffer);

    // Sending files with the process debugger:
    virtual bool gaRemoteToLocalFile(osFilePath& io_filePath, bool useCache);

    // AID function for multiple APIs functions:
    virtual apAPIFunctionId gaFindMultipleAPIsFunctionID(apAPIFunctionId originalFunctionId, apContextType contextType, apAPIConnectionType& apiConnectionType);

    // Deleted context:
    virtual bool gaMarkContextAsDeleted(const apContextID& deletedContextID);

    // VS information:
    virtual bool gaSetHexDisplayMode(bool hexMode);
    virtual bool gaIsHexDisplayMode();

    // Extra crash Information:
    virtual bool gaGetCrashReportAdditionalInformation(bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce);

    // HSA debugging:
    virtual bool gaIsInHSAKernelDebugging();
    virtual bool gaIsInHSAKernelBreakpoint();
    virtual bool gaHSAGetCurrentLine(gtUInt64& line, gtUInt64& addr);
    virtual bool gaHSAGetSourceFilePath(osFilePath& srcPath, gtString& kernelName);
    virtual bool gaHSAGetCallStack(osCallStack& stack);
    virtual bool gaHSASetNextDebuggingCommand(apKernelDebuggingCommand cmd);
    virtual bool gaHSASetBreakpoint(const gtString& kernelName, const gtUInt64& line);
    virtual bool gaHSAListVariables(int evalDepth, gtVector<apExpression>& o_locals);
    virtual bool gaHSAGetExpressionValue(const gtString& expStr, int evalDepth, apExpression& o_exp);
    virtual bool gaHSAListWorkItems(gtVector<gtUInt32>& o_gidLidWgid);
    virtual bool gaHSASetActiveWorkItemIndex(gtUInt32 wiIndex);
    virtual bool gaHSAGetWorkDims(gtUByte& dims);

protected:
    // Allow subclasses to query whether an instance was already registered:
    static bool wasInstanceRegistered() {return (_pMySingleInstance != NULL);};

private:
    static void deleteInstance();

private:
    friend class gaSingletonsDelete;
    friend class vspSingletonsDelete;

private:
    static gaGRApiFunctions* _pMySingleInstance;
};


//////////////////////////////////////////////////////////////////////////
// Global functions start here
//////////////////////////////////////////////////////////////////////////
// API package:
GA_API bool gaIntializeAPIPackage(bool shouldInitPerfCounters);
GA_API bool gaIsAPIConnectionActive(apAPIConnectionType apiType);
GA_API bool gaIsAPIConnectionActiveAndDebuggedProcessSuspended(apAPIConnectionType apiType);

// Debugged process:
GA_API bool gaDebuggedExecutableExists(const apDebugProjectSettings& processCreationData);
GA_API bool gaSystemLibrariesExists(const apDebugProjectSettings& processCreationData, gtString& missingSystemLibraries);
GA_API bool gaLaunchDebuggedProcess(const apDebugProjectSettings& processCreationData, bool launchSuspended);
GA_API bool gaContinueDebuggedProcessFromSuspendedCreation();
GA_API bool gaDebuggedProcessExists();
GA_API bool gaTerminateDebuggedProcess();
GA_API bool gaSuspendDebuggedProcess();
GA_API bool gaResumeDebuggedProcess();
GA_API bool gaLockDriverThreads();
GA_API bool gaUnLockDriverThreads();
GA_API bool gaSuspendThreads(const std::vector<osThreadId>& thrds);
GA_API bool gaResumeThreads();
GA_API bool gaIsDebuggedProcessSuspended();
GA_API bool gaIsDuringDebuggedProcessTermination();
GA_API bool gaIsDebugging64BitApplication();
GA_API bool gaGetCurrentDebugSessionLogFilesSubDirectory(osFilePath& logFilesPath);

// Debugged process threads:
GA_API bool gaGetAmountOfDebuggedProcessThreads(int& threadsAmount);
GA_API bool gaGetThreadId(int threadIndex, osThreadId& threadId);
GA_API bool gaGetBreakpointTriggeringThreadIndex(int& threadIndex);
GA_API bool gaGetThreadCurrentRenderContext(const osThreadId& threadId, int& contextId);
GA_API bool gaGetBreakpointTriggeringContextId(apContextID& contextId);
GA_API bool gaGetRenderContextCurrentThread(int contextId, osThreadId& threadId);
GA_API bool gaGetThreadCallStack(const osThreadId& threadId, osCallStack& callStack, bool hideSpyDLLsFunctions = true, bool fetchRemoteSourceFiles = true);
GA_API bool gaCanGetHostVariables();
GA_API bool gaCanGetHostDebugging();
GA_API bool gaHostDebuggerStepIn(osThreadId threadId);
GA_API bool gaHostDebuggerStepOut(osThreadId threadId);
GA_API bool gaHostDebuggerStepOver(osThreadId threadId);
GA_API bool gaGetThreadLocals(const osThreadId& threadId, int frameIndex, int evalDepth, gtVector<apExpression>& o_locals, bool onlyNames = false);
GA_API bool gaGetThreadExpressionValue(const osThreadId& threadId, int frameIndex, const gtString& expressionText, int evalDepth, apExpression& o_exp);
GA_API bool gaIsHostBreakPoint();
GA_API bool gaSetHostBreakpoint(const osFilePath& filePath, int lineNumber);
GA_API bool gaGetHostBreakpointLocation(osFilePath& bpFile, int& bpLine);

// Render contexts:
GA_API bool gaGetAmountOfRenderContexts(int& contextsAmount);
GA_API bool gaGetRenderContextDetails(int contextId, apGLRenderContextInfo& renderContextInfo);
GA_API bool gaGetRenderContextGraphicDetails(int contextId, apGLRenderContextGraphicsInfo& renderContextGraphicsInfo);
GA_API bool gaGetRenderContextLivingShareListsHolder(int contextID, int& livingContextID);

// OpenCL contexts:
GA_API bool gaGetOpenCLContextDetails(int contextId, apCLContext& renderContextInfo);

// OpenCL & OpenGL contexts:
GA_API bool gaWasContextDeleted(const apContextID& contextId);

// Monitored state variables:
GA_API bool gaGetAmountOfOpenGLStateVariables(int& amountOfStateVariables);
GA_API bool gaGetOpenGLStateVariableName(int stateVariableId, gtString& stateVariableName);
GA_API bool gaGetOpenGLStateVariableGlobalType(int stateVariableId, unsigned int& stateVariableGlobalType);
GA_API bool gaGetOpenGLStateVariableId(const gtString& stateVariableName, int& stateVariableId);

// Monitored functions:
GA_API bool gaGetAmountOfMonitoredFunctions(int& amountOfFunctions);
GA_API bool gaGetMonitoredFunctionName(apMonitoredFunctionId functionId, gtString& functionName);
GA_API bool gaGetMonitoredFunctionId(const gtString& functionName, apMonitoredFunctionId& functionId);
GA_API bool gaGetMonitoredFunctionType(apMonitoredFunctionId functionId, unsigned int& functionType);
GA_API bool gaGetMonitoredFunctionAPIType(apMonitoredFunctionId functionId, unsigned int& functionAPIType);
GA_API bool gaGetMonitoredFunctionDeprecationVersion(apMonitoredFunctionId functionId, apAPIVersion& deprectedAtVersion, apAPIVersion& removedAtVersion);

// State variable values:
GA_API bool gaGetDefaultOpenGLStateVariableValue(int contextId, int stateVariableId, gtAutoPtr<apParameter>& aptrStateVariableValue);
GA_API bool gaGetOpenGLStateVariableValue(int contextId, int stateVariableId, gtAutoPtr<apParameter>& aptrStateVariableValue);

// Textures:
GA_API bool gaEnableImagesDataLogging(bool isTexturesImageDataLogged);
GA_API bool gaIsImagesDataLogged(bool& isTexturesImageDataLogged);

GA_API bool gaGetAmountOfTextureUnits(int contextId, int& amountOfTextureUnits);
GA_API bool gaGetActiveTextureUnit(int contextId, int& activeTextureUnitId);
GA_API bool gaGetTextureUnitName(int contextId, int textureUnitId, GLenum& textureUnitName);
GA_API bool gaGetEnabledTexturingMode(int contextId, int textureUnitId, bool& isTexturingEnabled, apTextureType& enabledTexturingMode);
GA_API bool gaGetBoundTexture(int contextId, int textureUnitId, apTextureType bindTarget, GLuint& textureName);

GA_API bool gaGetAmountOfTextureObjects(int contextId, int& amountOfTextures);
GA_API bool gaGetTextureObjectName(int contextId, int textureId, GLuint& textureName);
GA_API bool gaGetTextureObjectType(int contextId, int textureId, apTextureType& textureType);
GA_API bool gaGetTextureObjectDetails(int contextId, GLuint textureName, apGLTexture& textureDetails);
GA_API bool gaGetTextureObjectThumbnailData(int contextId, GLuint textureName, apGLTextureMiplevelData& textureThumbDetails);
GA_API bool gaGetTextureDataObjectDetails(int contextId, GLuint textureName, apGLTextureData& textureData);
GA_API bool gaGetTextureMemoryDataObjectDetails(int contextId, GLuint textureName, apGLTextureMemoryData& textureData);
GA_API bool gaUpdateTextureRawData(int contextId, const gtVector<apGLTextureMipLevelID>& texturesVector);
GA_API bool gaGetTextureMiplevelDataFilePath(int contextId, apGLTextureMipLevelID textureMiplevelId, int faceIndex, osFilePath& filePath);
GA_API bool gaUpdateTextureParameters(int contextId, const gtVector<apGLTextureMipLevelID>& texturesVector, bool shouldUpdateOnlyMemoryParams);
GA_API bool gaIsTextureImageDirty(int contextId, apGLTextureMipLevelID textureMiplevelId, bool& dirtyImageExists, bool& dirtyRawDataExists);
GA_API bool gaMarkAllTextureImagesAsUpdated(int contextId, int textureId);

// Render buffers:
GA_API bool gaGetAmountOfRenderBufferObjects(int contextId, int& amountOfRenderBuffers);
GA_API bool gaGetRenderBufferObjectName(int contextId, int renderBufferId, GLuint& renderBufferName);
GA_API bool gaGetRenderBufferObjectDetails(int contextId, GLuint renderBufferName, apGLRenderBuffer& renderBufferDetails);
GA_API bool gaUpdateRenderBufferRawData(int contextId, const gtVector<GLuint>& renderBuffersVector);

// Program pipelines:
GA_API bool gaGetAmountOfPipelineObjects(int contextId, int& amountOfProgramPipelines);
GA_API bool gaGetPipelineObjectName(int contextId, int pipelineIndex, GLuint& pipelineName);
GA_API bool gaGetPipelineObjectDetails(int contextId, GLuint pipelineName, apGLPipeline& programPipelineDetails);

// OpenGL samplers:
GA_API bool gaGetAmountOfSamplerObjects(int contextId, int& amountOfSamplers);
GA_API bool gaGetSamplerObjectDetails(int contextId, GLuint samplerName, apGLSampler& samplerDetails);
GA_API bool gaGetSamplerObjectName(int contextId, int samplerIndex, GLuint& samplerName);

// FBOs
GA_API bool gaGetAmountOfFBOs(int contextId, int& amountOfFBOs);
GA_API bool gaGetFBOName(int contextId, int fboId, GLuint& fboName);
GA_API bool gaGetFBODetails(int contextId, GLuint fboName, apGLFBO& fboDetails);
GA_API bool gaGetActiveFBO(int contextId, GLuint& fboName);

// VBOs
GA_API bool gaGetAmountOfVBOs(int contextId, int& amountOfVBOs);
GA_API bool gaGetVBOName(int contextId, int vboId, GLuint& vboName);
GA_API bool gaGetVBODetails(int contextId, GLuint vboName, apGLVBO& vboDetails);
GA_API bool gaGetVBOAttachment(int contextId, GLuint vboName, GLenum& vboLastTarget, gtVector<GLenum>& vboCurrentTargets);
GA_API bool gaUpdateVBORawData(int contextId, const gtVector<GLuint>& vboNamesVector);
GA_API bool gaSetVBODisplayProperties(int contextId, GLuint vboName, oaTexelDataFormat displayFormat, int offset, GLsizei stride);

// Static Buffers:
GA_API bool gaGetAmountOfStaticBuffersObjects(int contextId, int& amountOfStaticBuffers);
GA_API bool gaGetStaticBufferType(int contextId, int staticBufferId, apDisplayBuffer& bufferType);
GA_API bool gaGetStaticBufferObjectDetails(int contextId, apDisplayBuffer bufferType, apStaticBuffer& staticBufferDetails);
GA_API bool gaUpdateStaticBufferRawData(int contextId, apDisplayBuffer bufferType);
GA_API bool gaUpdateStaticBuffersDimension(int contextId);

// PBuffers:
GA_API bool gaGetAmountOfPBuffersObjects(int& amountOfPBuffers);
GA_API bool gaGetPBufferObjectDetails(int pbufferID, apPBuffer& pbufferDetails);
GA_API bool gaGetAmountOfPBufferContentObjects(int pbufferID, int& amountOfPBufferContentObjects);
GA_API bool gaGetPBufferStaticBufferType(int pbufferID, int staticBufferIter, apDisplayBuffer& bufferType);
GA_API bool gaGetPBufferStaticBufferObjectDetails(int pbufferID, apDisplayBuffer bufferType, apStaticBuffer& staticBufferDetails);
GA_API bool gaUpdatePBufferStaticBufferRawData(int pbufferContextId, int pbufferID, apDisplayBuffer bufferType);
GA_API bool gaUpdatePBuffersDimension(int contextId);

// Sync objects:
GA_API bool gaGetAmountOfSyncObjects(int& amountOfSyncObjects);
GA_API bool gaGetSyncObjectDetails(int syncId, apGLSync& syncObjectDetails);

// Programs and Shaders:
GA_API bool gaGetAmountOfProgramObjects(int contextId, int& amountOfPrograms);
GA_API bool gaGetProgramObjectName(int contextId, int programId, GLuint& programName);
GA_API bool gaGetActiveProgramObjectName(int contextId, GLuint& activeProgramName);
GA_API bool gaGetProgramObjectDetails(int contextId, GLuint programName, apGLProgram& programDetails);
GA_API bool gaGetProgramActiveUniforms(int contextId, GLuint programName, apGLItemsCollection& activeUniforms);
GA_API bool gaLinkProgramObject(int contextId, GLuint programName, bool& wasLinkSuccessful, gtString& linkLog);
GA_API bool gaValidateProgramObject(int contextId, GLuint programName, bool& wasValidationSuccessful, gtString& validationLog);

GA_API bool gaGetAmountOfShaderObjects(int contextId, int& amountOfShaders);
GA_API bool gaGetShaderObjectName(int contextId, int shaderId, GLuint& shaderName);
GA_API bool gaGetShaderObjectDetails(int contextId, GLuint shaderName, gtAutoPtr<apGLShaderObject>& aptrShaderDetails);
GA_API bool gaMarkShaderObjectSourceCodeAsForced(int contextId, GLuint shaderName, bool isSourceCodeForced = true);
GA_API bool gaSetShaderObjectSourceCode(int contextId, GLuint shaderName, const osFilePath& sourceCodeFile);
GA_API bool gaCompileShaderObject(int contextId, GLuint shaderName, bool& wasCompilationSuccessful, gtString& compilationLog);

// Display Lists:
GA_API bool gaGetAmountOfDisplayLists(int contextID, int& amountOfDisplayLists);
GA_API bool gaGetDisplayListObjectName(int contextID, int displayListIndex, GLuint& displayListName);
GA_API bool gaGetDisplayListObjectDetails(int contextID, GLuint displayListName, apGLDisplayList& pDisplayListDetails);

// OpenGL / OpenCL Function calls:
GA_API bool gaGetAmountOfCurrentFrameFunctionCalls(const apContextID& contextID, int& amountOfFunctionCalls);
GA_API bool gaGetCurrentFrameFunctionCall(const apContextID& contextID, int callIndex, gtAutoPtr<apFunctionCall>& aptrFunctionCall);
GA_API bool gaGetLastFunctionCall(const apContextID& contextID, gtAutoPtr<apFunctionCall>& aptrFunctionCall);
GA_API bool gaFindCurrentFrameFunctionCall(const apContextID& contextID, apSearchDirection searchDirection, int searchStartIndex, const gtString& searchedString, bool isCaseSensitiveSearch, int& foundIndex);
GA_API bool gaClearFunctionCallsStatistics();
GA_API bool gaGetCurrentStatistics(const apContextID& contextID, apStatistics* pStatistics);

// OpenGL function calls:
GA_API bool gaGetCurrentFrameFunctionCallDeprecationDetails(int contextId, int callIndex, apFunctionDeprecation& functionDeprecationDetails);
GA_API bool gaIsInOpenGLBeginEndBlock(int contextId);
GA_API bool gaGetRenderPrimitivesStatistics(int contextId, apRenderPrimitivesStatistics& renderPrimitivesStatistics);

// OpenCL handles:
GA_API bool gaGetOpenCLHandleObjectDetails(oaCLHandle openCLHandlePtr, apCLObjectID& clObjectIdDetails);

// OpenGL Contexts:
GA_API bool gaGetAmountOfOpenCLContexts(int& amountOfContexts);

// OpenCL programs:
GA_API bool gaGetAmountOfOpenCLProgramObjects(int contextId, int& amountOfPrograms);
GA_API bool gaGetOpenCLProgramObjectDetails(int contextId, int programIndex, apCLProgram& programDetails);
GA_API bool gaSetOpenCLProgramCode(oaCLProgramHandle programHandle, const osFilePath& newSourcePath);
GA_API bool gaBuildOpenCLProgram(oaCLProgramHandle programHandle, apCLProgram*& pFailedProgramData);
GA_API bool gaGetOpenCLProgramHandleFromSourceFilePath(const osFilePath& sourceFilePath, osFilePath& newTempSourceFilePath, oaCLProgramHandle& programHandle);
GA_API bool gaSetKernelSourceFilePath(gtVector<osFilePath>& programsFilePath);
GA_API bool gaCodeLocationFromKernelSourceBreakpoint(const apKernelSourceCodeBreakpoint& breakpoint, osFilePath& sourceFilePath, int& lineNum);

// OpenCL kernels:
GA_API bool gaGetOpenCLKernelObjectDetails(int contextId, oaCLKernelHandle kernelHandle, apCLKernel& kernelDetails);

// OpenCL kernel debugging:
GA_API bool gaGetKernelDebuggingLocation(oaCLProgramHandle& debuggedProgramHandle, int& currentLineNumber);
GA_API bool gaGetCurrentlyDebuggedKernelDetails(apCLKernel& kernelDetails);
GA_API bool gaGetCurrentlyDebuggedKernelCallStack(osCallStack& kernelStack);
GA_API bool gaSetKernelDebuggingCommand(apKernelDebuggingCommand command);
GA_API bool gaGetKernelDebuggingGlobalWorkOffset(int coordinate, int& dimension);
GA_API bool gaGetKernelDebuggingGlobalWorkOffset(int& xOffset, int& yOffset, int& zOffset);
GA_API bool gaGetKernelDebuggingGlobalWorkSize(int coordinate, int& dimension);
GA_API bool gaGetKernelDebuggingGlobalWorkSize(int& xDimension, int& yDimension, int& zDimension);
GA_API bool gaGetKernelDebuggingLocalWorkSize(int coordinate, int& dimension);
GA_API bool gaGetKernelDebuggingLocalWorkSize(int& xDimension, int& yDimension, int& zDimension, int& amountOfDimensions);
GA_API bool gaSetKernelDebuggingCurrentWorkItemCoordinate(int coordinate, int value);
GA_API bool gaGetKernelDebuggingCurrentWorkItemCoordinate(int coordinate, int& value);
GA_API bool gaGetKernelDebuggingCurrentWorkItem(int& xValue, int& yValue, int& zValue);
GA_API bool gaIsInKernelDebugging();
GA_API bool gaSetKernelSteppingWorkItem(const int coordinate[3]);
GA_API bool gaUpdateKernelSteppingWorkItemToCurrentCoordinate();
GA_API bool gaIsWorkItemValid(const int coordinate[3]);
GA_API bool gaGetFirstValidWorkItem(int wavefrontIndex, int coordinate[3]);
GA_API bool gaCanGetKernelVariableValue(const gtString& variableName);
GA_API bool gaGetKernelDebuggingExpressionValue(const gtString& expressionString, const int workItem[3], int evalDepth, apExpression& o_exp);
GA_API bool gaGetKernelDebuggingAvailableVariables(int evalDepth, gtVector<apExpression>& o_locals, bool getLeaves = false, int stackFrameDepth = -1, bool onlyNames = false);
GA_API bool gaGetKernelDebuggingAmountOfActiveWavefronts(int& amountOfWavefronts);
GA_API bool gaGetKernelDebuggingActiveWavefrontID(int wavefrontIndex, int& wavefrontId);
GA_API bool gaGetKernelDebuggingWavefrontIndex(const int coordinate[3], int& wavefrontIndex);
GA_API bool gaUpdateKernelVariableValueRawData(const gtString& variableName, bool& variableTypeSupported, osFilePath& variableRawData);
GA_API bool gaGetKernelSourceCodeBreakpointResolution(oaCLProgramHandle programHandle, int requestedLineNumber, int& resolvedLineNumber);
GA_API bool gaSetKernelDebuggingEnable(bool kernelEnable);
GA_API bool gaSetMultipleKernelDebugDispatchMode(apMultipleKernelDebuggingDispatchMode mode);

// OpenCL device:
GA_API bool gaGetOpenCLDeviceObjectDetails(int deviceId, apCLDevice& deviceDetails);

// OpenCL platforms:
GA_API bool gaGetOpenCLPlatformAPIID(gtUInt64 platformId, int& platformName);

// OpenCL buffers:
GA_API bool gaGetAmountOfOpenCLBufferObjects(int contextId, int& amountOfBuffers);
GA_API bool gaGetOpenCLBufferObjectDetails(int contextId, int bufferIndex, apCLBuffer& bufferDetails);
GA_API bool gaUpdateOpenCLBufferRawData(int contextId, const gtVector<int>& bufferIdsVector);
GA_API bool gaSetCLBufferDisplayProperties(int contextId, int bufferId, oaTexelDataFormat displayFormat, int displayOffset, gtSize_t displayStride);

// OpenCL Sub-Buffers:
GA_API bool gaUpdateOpenCLSubBufferRawData(int contextId, const gtVector<int>& bufferIdsVector);
GA_API bool gaSetCLSubBufferDisplayProperties(int contextId, int subBufferId, oaTexelDataFormat displayFormat, int displayOffset, gtSize_t displayStride);
GA_API bool gaGetOpenCLSubBufferObjectDetails(int contextId, int subBufferName, apCLSubBuffer& subBufferDetails);

// OpenCL images:
GA_API bool gaGetAmountOfOpenCLImageObjects(int contextId, int& amountOfImages);
GA_API bool gaGetOpenCLImageObjectDetails(int contextId, int imageIndex, apCLImage& imageDetails);
GA_API bool gaUpdateOpenCLImageRawData(int contextId, const gtVector<int>& imageIdsVector);

// OpenCL pipes:
GA_API bool gaGetAmountOfOpenCLPipeObjects(int contextId, int& amountOfPipes);
GA_API bool gaGetOpenCLPipeObjectDetails(int contextId, int pipeIndex, apCLPipe& pipeDetails);

// OpenCL command queues:
GA_API bool gaGetAmountOfCommandQueues(int contextId, int& amountOfQueues);
GA_API bool gaGetCommandQueueDetails(int contextId, int queueIndex, apCLCommandQueue& commandQueueDetails);
GA_API bool gaGetAmountOfCommandsInQueue(oaCLCommandQueueHandle hQueue, int& amountOfCommands);
GA_API bool gaGetAmountOfEventsInQueue(oaCLCommandQueueHandle hQueue, int& amountOfEvents);
GA_API bool gaGetEnqueuedCommandDetails(oaCLCommandQueueHandle hQueue, int commandIndex, gtAutoPtr<apCLEnqueuedCommand>& aptrCommand);

// OpenCL samplers:
GA_API bool gaGetAmountOfOpenCLSamplers(int contextId, int& amountOfSamplers);
GA_API bool gaGetOpenCLSamplerObjectDetails(int contextId, int samplerIndex, apCLSampler& samplerDetails);

// OpenCL events:
GA_API bool gaGetAmountOfOpenCLEvents(int contextId, int& amountOfEvents);
GA_API bool gaGetOpenCLEventObjectDetails(int contextId, int eventIndex, apCLEvent& eventDetails);

// Allocated Objects
GA_API bool gaGetAmountOfRegisteredAllocatedObjects(unsigned int& numberOfObjects);
GA_API bool gaGetAllocatedObjectCreationStack(int allocatedObjectId, osCallStack& callStack);
GA_API bool gaCollectAllocatedObjectsCreationCallsStacks(bool collectCreationStacks);
GA_API bool gaGetCollectAllocatedObjectCreationCallsStacks(bool& collectCreationStacks);

// String markers:
GA_API bool gaFindStringMarker(int contextId, apSearchDirection searchDirection, int searchStartIndex, int& foundIndex);

// Breakpoints:
GA_API bool gaSetBreakpoint(const apBreakPoint& breakpoint);
GA_API bool gaRemoveBreakpoint(int breakpointId);
GA_API bool gaRemoveGenericBreakpoint(apGenericBreakpointType breakpointType);
GA_API bool gaGetAmountOfBreakpoints(int& amountOfBreakpoints);
GA_API bool gaGetBreakpoint(int breakPointId, gtAutoPtr<apBreakPoint>& aptrBreakpoint);
GA_API bool gaGetBreakpointIndex(const apBreakPoint& Breakpoint, int& breakpointId);
GA_API bool gaRemoveAllBreakpoints();
GA_API bool gaRemoveAllBreakpointsByType(osTransferableObjectType breakpointType);
GA_API bool gaGetEnableAllBreakpointsStatus(bool& isEnableAllBreakpointsChecked, bool& isEnableAllBreakpointsEnabled);
GA_API bool gaBreakOnNextMonitoredFunctionCall();
GA_API bool gaBreakOnNextDrawFunctionCall();
GA_API bool gaBreakOnNextFrame();
GA_API bool gaBreakInMonitoredFunctionCall();
GA_API bool gaClearAllStepFlags();
GA_API bool gaGetGenericBreakpointStatus(apGenericBreakpointType breakpointType, bool& doesExist, bool& isEnabled);
GA_API bool gaSetBreakpointHitCount(int breakpointIndex, int hitCount);
GA_API bool gaSetKernelBreakpointProgramHandle(int breakpointIndex, oaCLProgramHandle programHandle);
GA_API bool gaRemoveAllBreakpointsByState(const apBreakPoint::State state);
GA_API bool gaTemporarilyDisableAllBreakpoints();
GA_API bool gaEnableAllBreakpointsByState(const apBreakPoint::State state);

// Log file recording:
GA_API bool gaDeleteLogFilesWhenDebuggedProcessTerminates(bool deleteLogFiles);
GA_API bool gaStartMonitoredFunctionsCallsLogFileRecording();
GA_API bool gaStopMonitoredFunctionsCallsLogFileRecording();
GA_API bool gaIsMonitoredFunctionsCallsLogFileRecordingActive(bool& isActive);
GA_API bool gaGetContextLogFilePath(apContextID contextID, bool& logFileExists, osFilePath& filePath);
GA_API bool gaFlushLogFileAfterEachFunctionCall(bool flushAfterEachFunctionCall);
GA_API bool gaIsLogFileFlushedAfterEachFunctionCall(bool& isLogFileFlushedAfterEachFunctionCall);
GA_API bool gaWasOpenGLDataRecordedInDebugSession();
GA_API bool gaResetRecordingWasDoneFlag(bool isEnabled);

// Slow motion:
GA_API bool gaSetSlowMotionDelay(int delayTimeUnits);
GA_API bool gaGetSlowMotionDelay(int& delayTimeUnits);

// Force OpenGL flush:
GA_API bool gaForceOpenGLFlush(bool isOpenGLFlushForced);
GA_API bool gaIsOpenGLFlushForced(bool& isOpenGLFlushForced);

// "Interactive" break mode:
GA_API bool gaSetInteractiveBreakMode(bool isInteractiveBreakOn);
GA_API bool gaIsInteractiveBreakOn();

GA_API bool gaSetDebuggedProcessExecutionMode(apExecutionMode executionMode);
GA_API bool gaGetDebuggedProcessExecutionMode(apExecutionMode& executionMode);

// Forces OpenGL render modes:
GA_API bool gaForceOpenGLPolygonRasterMode(apRasterMode rasterMode);
GA_API bool gaCancelOpenGLPolygonRasterModeForcing();
GA_API bool gaIsOpenGLPolygonRasterModeForced(bool& isForced);
GA_API bool gaGetForceOpenGLPolygonRasterMode(apRasterMode& rasterMode);

// Turn off graphic pipeline stages:
GA_API bool gaSwitchToNULLOpenGLImplementation(bool isNULLOpenGLImplOn);
GA_API bool gaIsUnderNULLOpenGLImplementation(bool& isUnderNULLOpenGLImplementation);

// OpenGL Force stub operations:
GA_API bool gaForceOpenGLStub(apOpenGLForcedModeType openGLStubType, bool isStubForced);
GA_API bool gaIsOpenGLStubForced(apOpenGLForcedModeType openGLStubType, bool& isStubForced);

// OpenCL cancel operations:
GA_API bool gaIsOpenCLExecutionOn(apOpenCLExecutionType executionType, bool& isExecutionOn);
GA_API bool gaSetOpenCLExecution(apOpenCLExecutionType executionType, bool isExecutionOn);

// Performance counters:
GA_API bool gaGetAmountOfPerformanceCounters(int& countersAmount);
GA_API bool gaGetAmountOfPerformanceCountersByType(apCounterType counterType, int& countersAmount);
GA_API bool gaAddOSCounter(const apCounterInfo& newCounterInfo, int& newCounterIndex, bool& isNewCounter);
GA_API bool gaRemoveOSCounter(int counterIndex);

GA_API const apCounterInfo* gaGetPerformanceCounterInfo(int counterIndex);
GA_API bool gaGetPerformanceCounterLocalIndex(int counterIndex, int& counterLocalIndex);
GA_API bool gaGetPerformanceCounterType(int counterIndex, apCounterType& counterType);
GA_API bool gaGetPerformanceCounterIndex(const apCounterID& counterId, int& counterIndex);
GA_API bool gaActivatePerformanceCounter(const apCounterActivationInfo& counterActivationInfo);
GA_API bool gaUpdatePerformanceCountersValues();
GA_API double gaGetPerformanceCounterValue(const apCounterID& counterId);

// OpenCL queues:
GA_API void gaOnQueueCreatedEvent(const apOpenCLQueueCreatedEvent& eve);
GA_API void gaOnQueueDeletedEvent(const apOpenCLQueueDeletedEvent& eve);

// OpenGL debug output messages:
GA_API bool gaIsGLDebugOutputSupported();
GA_API bool gaEnableGLDebugOutputLogging(bool isGLDebugOutputLoggingEnabled);
GA_API bool gaGetGLDebugOutputLoggingEnabledStatus(bool& isGLDebugOutputLoggingEnabled);
GA_API bool gaGetGLDebugOutputSeverityEnabled(apGLDebugOutputSeverity severity, bool& enabled);
GA_API bool gaSetGLDebugOutputSeverityEnabled(apGLDebugOutputSeverity severity, bool enabled);
GA_API bool gaSetGLDebugOutputKindMask(const gtUInt64& debugReportMask);
GA_API bool gaGetGLDebugOutputKindMask(gtUInt64& debugReportMask);
GA_API bool gaDoesDebugForcedContextExist(bool isDebugContext, bool& isDebugContextForced);

// GPUs:
GA_API bool gaGetAmountOfLocalMachineGPUs(int& GPUsAmount);
GA_API bool gaGetLocalMachineGPUInformation(int GPUIndex, apGPUInfo& GPUInfo);

// Sending files through the API pipe:
GA_API bool gaReadFile(const osFilePath& remoteFilePath, osRawMemoryBuffer& memoryBuffer);
GA_API bool gaWriteFile(const osFilePath& remoteFilePath, const osRawMemoryBuffer& memoryBuffer);

// Sending files with the process debugger:
GA_API bool gaRemoteToLocalFile(osFilePath& io_filePath, bool useCache);
GA_API bool gaRemoteToLocalCallStack(osCallStack& io_callStack);

// AID function for multiple APIs functions:
GA_API apAPIFunctionId gaFindMultipleAPIsFunctionID(apAPIFunctionId originalFunctionId, apContextType contextType, apAPIConnectionType& apiConnectionType);

// Deleted context:
GA_API bool gaMarkContextAsDeleted(const apContextID& deletedContextID);

// VS information:
GA_API bool gaSetHexDisplayMode(bool hexMode);
GA_API bool gaIsHexDisplayMode();

// Extra crash Information:
GA_API bool gaGetCrashReportAdditionalInformation(bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce);

// HSA debugging:
GA_API bool gaIsInHSAKernelDebugging();
GA_API bool gaIsInHSAKernelBreakpoint();
GA_API bool gaHSAGetCurrentLine(gtUInt64& line, gtUInt64& addr);
GA_API bool gaHSAGetSourceFilePath(osFilePath& srcPath, gtString& kernelName);
GA_API bool gaHSAGetCallStack(osCallStack& stack);
GA_API bool gaHSASetNextDebuggingCommand(apKernelDebuggingCommand cmd);
GA_API bool gaHSASetBreakpoint(const gtString& kernelName, const gtUInt64& line);
GA_API bool gaHSAListVariables(int evalDepth, gtVector<apExpression>& o_locals);
GA_API bool gaHSAGetExpressionValue(const gtString& expStr, int evalDepth, apExpression& o_exp);
GA_API bool gaHSAListWorkItems(gtVector<gtUInt32>& o_gidLidWgid);
GA_API bool gaHSASetActiveWorkItemIndex(gtUInt32 wiIndex);
GA_API bool gaHSAGetWorkDims(gtUByte& dims);

#endif  // __GAGRAPIFUNCTIONS
