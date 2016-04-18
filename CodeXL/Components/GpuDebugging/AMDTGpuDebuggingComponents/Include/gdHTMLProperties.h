//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdHTMLProperties.h
///
//==================================================================================

//------------------------------ gdHTMLProperties.h ------------------------------

#ifndef __GDHTMLPROPERTIES
#define __GDHTMLPROPERTIES

// Forward Declarations:
class gtString;
class apApiConnectionEndedEvent;
class apApiConnectionEstablishedEvent;
class apAssociatedTextureNamesPseudoParameter;
class apBreakpointHitEvent;
class apComputeContextCreatedEvent;
class apComputeContextDeletedEvent;
class apCLBuffer;
class apCLCommandQueue;
class apCLContext;
class apCLDevice;
class apCLEnqueuedCommand;
class apCLEvent;
class apCLKernel;
class apCLPipe;
class apCLProgram;
class apCLSampler;
class apCLSubBuffer;
class apCLImage;
class apDebuggedProcessDetectedErrorEvent;
class apDebuggedProcessOutputStringEvent;
class apExceptionEvent;
class apFunctionCall;
class apGDBErrorEvent;
class apGDBOutputStringEvent;
class apGLDebugOutputMessageEvent;
class apGLRenderBuffer;
class apGLPipeline;
class apGLSampler;
class apGLRenderContextGraphicsInfo;
class apGLRenderContextInfo;
class apGLSync;
class apGLTexture;
class apGLVBO;
class apMemoryLeakEvent;
class apSearchingForMemoryLeaksEvent;
class apModuleLoadedEvent;
class apModuleUnloadedEvent;
class apOpenCLErrorEvent;
class apOpenCLProgramCreatedEvent;
class apOpenCLProgramDeletedEvent;
class apOpenCLProgramBuildEvent;
class apOpenCLQueueCreatedEvent;
class apOpenCLQueueDeletedEvent;
class apOutputDebugStringEvent;
class apPBuffer;
class apDebugProjectSettings;
class apDebuggedProcessCreationFailureEvent;
class apRenderContextCreatedEvent;
class apRenderContextDeletedEvent;
class apStaticBuffer;
class apTechnologyMonitorFailureEvent;
class apThreadCreatedEvent;
class apThreadTerminatedEvent;
struct FIBITMAP;
class afProgressBarWrapper;
class gdBreakpointsItemData;


// Infra:
#include <AMDTAPIClasses/Include/apFunctionDeprecation.h>
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osTransferableObjectType.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTAPIClasses/Include/apFileType.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>
#include <AMDTAPIClasses/Include/apGLProgram.h>
#include <AMDTAPIClasses/Include/apGLDisplayList.h>
#include <AMDTAPIClasses/Include/apGLFBO.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afHTMLContent.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdShaderType.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStatisticsViewBase.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeData.h>


#define GD_HTML_PROPERTIES_VERTICES_TO_BATCHES_OPTIMIZED_RATE 0.5


// ----------------------------------------------------------------------------------
// Class Name:           gdHTMLProperties
// General Description: The class builds a properties window HTML strings for objects.
//                      It is used by viewers that contain a properties window
// Author:               Sigal Algranaty
// Creation Date:        15/10/2008
// ----------------------------------------------------------------------------------
class GD_API gdHTMLProperties
{
    enum gdHTMLPropertiesObjectType
    {
        AP_HTML_TEXTURE,
        AP_HTML_RENDER_BUFFER,
        AP_HTML_STATIC_BUFFER,
        AP_HTML_PBUFFER
    };

public:
    gdHTMLProperties();

    // Memory and Texture Viewer items:
    void buildDebuggedApplicationHTMLPropertiesString(afHTMLContent& htmlContent);
    void buildRenderContextHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent);
    void buildStaticBuffersListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects);
    void buildStaticBufferHTMLPropertiesString(const apContextID& contextID, const apStaticBuffer& staticBufferDetails, bool bAddThumbnail, afProgressBarWrapper* pProgressBar, afHTMLContent& htmlContent);
    void buildTexBufferHTMLPropertiesString(const apContextID& contextID, const apGLTextureMiplevelData& textureBufferDetails, afHTMLContent& htmlContent, bool bAddThumbnail, afProgressBarWrapper* pProgressBar);
    void buildRenderBuffersListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects);
    void buildRenderBufferHTMLPropertiesString(const apContextID& contextID, const apGLRenderBuffer& renderBufferDetails, bool bAddThumbnail, afProgressBarWrapper* pProgressBar, afHTMLContent& htmlContent);
    void buildPipelinesListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects);
    void buildPipelinesHTMLPropertiesString(const apContextID& contextID, const apGLPipeline& pipelineDetails, afHTMLContent& htmlContent);
    void buildOpenGlSamplersListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects);
    void buildOpenGlSamplersHTMLPropertiesString(const apGLSampler& samplerDetails, afHTMLContent& htmlContent);
    void buildTextureHTMLPropertiesString(const apGLTexture& textureDetails, const apContextID& contextID, apGLTextureMipLevelID textureID, afHTMLContent& htmlContent, bool bBuildParametersTable, bool bAddThumbnail, afProgressBarWrapper* pProgressBar);
    void buildTextureHTMLPropertiesString(const apContextID& contextID, apGLTextureMipLevelID textureID, afHTMLContent& htmlContent, bool bBuildParametersTable, bool bAddThumbnail, afProgressBarWrapper* pProgressBar);
    void buildTextureTypesHTMLPropertiesString(const apContextID& contextID, gtMap<apTextureType, int>& texturesTypesAmountMap, afHTMLContent& htmlContent);
    void buildTexturesListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent);
    void buildPBuffersListHTMLPropertiesString(afHTMLContent& htmlContent, int numberOfObjects);
    void buildPBufferHTMLPropertiesString(int pbufferID, const apPBuffer& pbufferDetails, afHTMLContent& htmlContent, bool isParent = true);
    void buildPBufferStaticBufferHTMLPropertiesString(const apStaticBuffer& staticBufferDetails, const apPBuffer& pBufferDetails, int bufferId, bool bAddThumbnail, afProgressBarWrapper* pProgressBar, afHTMLContent& htmlContent);
    void buildFunctionCallHTMLPropertiesString(apExecutionMode currentExecutionMode, apContextID functionContextId, int functionCallIndex, gtString& propertiesHTMLMessage, afProgressBarWrapper* pProgressBar);
    void buildFunctionCallHTMLPropertiesString(apExecutionMode currentExecutionMode, apContextID functionContextId, int functionCallIndex,  afIconType iconType, const gtString& functionNameWithArgsStr, gtString& propertiesHTMLMessage);
    void buildFunctionHTMLPropertiesString(const apContextID& contextID, const gtString& functionName, afIconType iconType, gtString& propertiesHTMLMessage);
    void buildTotalStatisticsPropertiesString(apContextID& functionContextID, const gdFuncCallsViewTypes& functionType, bool isTotalItem, gtString& propertiesHTMLMessage);
    void buildStateChangeFunctionHTMLPropertiesString(const gtString& functionName, afIconType iconType, gtString& propertiesHTMLMessage);
    void buildDeprecatedFunctionHTMLPropertiesString(int functionId, const gtString& functionName, apFunctionDeprecationStatus functionDeprecationStatus, apAPIVersion derecatedAtVersion, apAPIVersion removedAtVersion, gtString& propertiesHTMLMessage);
    void buildVBOsListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects);
    void buildVBOHTMLPropertiesString(const apContextID& contextID, const apGLVBO& vboDetails, GLenum& vboLastAttachment, gtVector<GLenum>& vboCurrentAttachments, afHTMLContent& htmlContent, bool showDetailsLink);
    void buildProgramsListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects);
    void buildProgramHTMLPropertiesString(int contextId, const apGLProgram& programDetails, bool showActiveUniforms, afHTMLContent& htmlContent);
    void buildShadersListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, gtMap<gdShaderType, int>& shaderTypesAmountsMap);
    void buildShaderHTMLPropertiesString(const apContextID& contextID, const apGLShaderObject& shader, bool isExpanded, afHTMLContent& htmlContent);
    void buildDisplayListsListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects);
    void buildDisplayListHTMLPropertiesString(const apContextID& contextID, const apGLDisplayList& dpyList, afHTMLContent& htmlContent);
    void buildFBOsListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects);
    void buildFBOHTMLPropertiesString(const apContextID& contextID, const apGLFBO& fbo, bool forMemoryViewer, afHTMLContent& htmlContent);
    void buildAllBuffersHTMLPropertiesString(const apContextID& contextID, int amountOfStaticBuffers, int amountOfRenderBuffers, int amountOfVBOs, int amountOfFBOs, int amountOfPBuffers, afHTMLContent& htmlContent);
    void buildBatchStatisticsPropertiesString(int minAmountOfVertices, int maxAmountOfVertices, float percentageOfVertices, float percentageOfBatches, afIconType iconType, gtString& propertiesHTMLMessage);
    void buildSyncObjectHTMLPropertiesString(const apGLSync& syncObjectDetails, afHTMLContent& htmlContent);
    void buildSyncListHTMLPropertiesString(int amountOfSyncObjects, afHTMLContent& htmlContent);
    void addOpenGLContextHeaderToHTMLContent(const apContextID& contextID, const apGLRenderContextInfo& contextInfo, const apGLRenderContextGraphicsInfo& contextGraphicsInfo, afHTMLContent& htmlContent);
    void buildOpenGLContextHTMLPropertiesString(const apContextID& contextID, const apGLRenderContextInfo& contextInfo, const apGLRenderContextGraphicsInfo& contextGraphicsInfo, bool isOpenGLESProject, afHTMLContent& htmlContent);
    const static gtVector<gtString>& cubemapTitles() {return m_sCubemapTitles;}

    // OpenCL objects:
    void buildOpenCLDeviceHTMLPropertiesString(const apCLDevice& deviceDetails, int numberOfQueuesOnDevice, bool showQueuesNumber, afHTMLContent& htmlContent);
    void buildOpenCLProgramHTMLPropertiesString(int contextId, const apCLProgram& programDetails , int programId, bool isExpanded, afHTMLContent& htmlContent);
    void buildOpenCLSamplerHTMLPropertiesString(int contextId, const apCLSampler& samplerDetails , afHTMLContent& htmlContent);
    void buildOpenCLEventHTMLPropertiesString(int contextId, int eventIndex, const apCLEvent& eventDetails , afHTMLContent& htmlContent);
    void buildOpenCLKernelHTMLPropertiesString(int contextId, const apCLKernel& kernelDetails , int kernelId, afHTMLContent& htmlContent);
    void buildOpenCLBuffersListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects);
    void buildOpenCLBufferHTMLPropertiesString(const apContextID& contextID, const apCLBuffer& bufferDetails, int bufferIndex, bool addBufferLink, afHTMLContent& htmlContent);
    void buildOpenCLSubBufferHTMLPropertiesString(const apContextID& contextID, const apCLSubBuffer& subBufferDetails, bool addSubBufferLink, afHTMLContent& htmlContent);
    void buildOpenCLImageHTMLPropertiesString(const apContextID& contextID, int textureIndex, bool addThumbnail, afHTMLContent& htmlContent, afProgressBarWrapper* pProgressBar);
    void buildOpenCLPipesListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects);
    void buildOpenCLPipeHTMLPropertiesString(const apContextID& contextID, const apCLPipe& pipeDetails, int pipeIndex, bool addPipeLink, afHTMLContent& htmlContent);
    void buildOpenCLCommandQueueHTMLPropertiesString(const apContextID& contextID, const apCLCommandQueue& commandQueueDetails, int queueIndex, bool addQueueLink, afHTMLContent& htmlContent);
    void buildOpenCLCommandQueuesListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects);
    void buildOpenCLEnqueuedCommandHTMLPropertiesString(const apCLEnqueuedCommand& commandDetails, gtString& commandName, afHTMLContent& htmlContent);
    void buildOpenCLProgramsListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects);
    void buildOpenCLSamplersListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects);
    void buildOpenCLEventsListHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent, int numberOfObjects);
    bool addDevicesToHTMLContent(const apCLContext& contextInfo, afHTMLContent& htmlContent);
    void buildOpenCLContextHTMLPropertiesString(const apContextID& contextID, const apCLContext& contextInfo, afHTMLContent& htmlContent);
    void buildOpenCLContextHTMLPropertiesString(const apContextID& contextID, afHTMLContent& htmlContent);

    // Events:
    void buildThreadCreatedEventPropertiesString(const apThreadCreatedEvent& threadCreatedEvent, afHTMLContent& htmlContent);
    void buildThreadTerminatedEventPropertiesString(const apThreadTerminatedEvent& threadTerminatedEvent, afHTMLContent& htmlContent);
    void buildProcessCreationFailureEventPropertiesString(const apDebugProjectSettings& processStartedData, const apDebuggedProcessCreationFailureEvent& processCreationFailureEvent, afHTMLContent& htmlContent);
    void buildExceptionPropertiesString(const apExceptionEvent& exceptionEvent, afHTMLContent& htmlContent);
    void buildOutputDebugStringEventString(const apOutputDebugStringEvent& outputDebugStringEvent, afHTMLContent& htmlContent);
    void buildDebuggedProcessOutputStringEventString(const apDebuggedProcessOutputStringEvent& debuggedProcessOutputEvent, afHTMLContent& htmlContent);
    void buildGDBOutputStringEventPropertiesString(const apGDBOutputStringEvent& outputGDBStringEvent, afHTMLContent& htmlContent);
    void buildGDBErrorPropertiesString(const apGDBErrorEvent& gdbErrorEvent, afHTMLContent& htmlContent);
    void buildErrorEventPropertiesString(const apDebuggedProcessDetectedErrorEvent& errorEvent, afHTMLContent& htmlContent);
    void buildDLLLoadPropertiesString(const apModuleLoadedEvent& moduleLoadEvent, afHTMLContent& htmlContent);
    void buildDLLUnloadPropertiesString(const apModuleUnloadedEvent& moduleUnloadEvent, afHTMLContent& htmlContent);
    void buildCallStackPropertiesString(const gtString& functionName, const osFilePath& sourceCodeFilePath, const osFilePath& sourceCodeModulePath, int sourceCodeFileLineNumber, osInstructionPointer& functionStartAddress, osInstructionPointer instructionCounterAddress, afHTMLContent& htmlContent);
    void buildDebugProcessEventsPropertiesString(afHTMLContent& htmlContent);
    void buildMultipleItemPropertiesString(const gtString& title, const gtString& itemName, afHTMLContent& htmlContent);
    void buildBreakpointPropertiesString(const gtString& funcName, const gtString& funcArgs, const apBreakpointHitEvent& breakpointEvent, afHTMLContent& htmlContent);
    void buildAPIConnectionEstablishedEventProperties(const apApiConnectionEstablishedEvent& apiConnectionEstablishedEvent, afHTMLContent& htmlContent);
    void buildAPIConnectionEndedEventProperties(const apApiConnectionEndedEvent& apiConnectionEndedEvent, afHTMLContent& htmlContent);
    void buildRenderContextCreatedEventProperties(const apRenderContextCreatedEvent& renderContextCreatedEvent, afHTMLContent& htmlContent);
    void buildRenderContextDeletedEventProperties(const apRenderContextDeletedEvent& renderContextDeletedEvent, afHTMLContent& htmlContent);
    void buildComputeContextCreatedEventProperties(const apComputeContextCreatedEvent& computeContextCreatedEvent, afHTMLContent& htmlContent);
    void buildComputeContextDeletedEventProperties(const apComputeContextDeletedEvent& computeContextDeletedEvent, afHTMLContent& htmlContent);
    void buildOpenCLQueueCreatedEventProperties(const apOpenCLQueueCreatedEvent& queueCreatedEvent, afHTMLContent& htmlContent);
    void buildOpenCLQueueDeletedEventProperties(const apOpenCLQueueDeletedEvent& queueDeletedEvent, afHTMLContent& htmlContent);
    void buildOpenCLProgramCreatedEventProperties(const apOpenCLProgramCreatedEvent& programCreatedEvent, afHTMLContent& htmlContent);
    void buildOpenCLProgramDeletedEventProperties(const apOpenCLProgramDeletedEvent& programDeletedEvent, afHTMLContent& htmlContent);
    void buildOpenCLProgramBuildEventProperties(const apOpenCLProgramBuildEvent& programBuildEvent, afHTMLContent& htmlContent);
    void buildTechnologyMonitorFailureEventProperties(const apTechnologyMonitorFailureEvent& monitorFailureEvent, afHTMLContent& htmlContent);
    void buildGLDebugOutputMessageEventProperties(const apGLDebugOutputMessageEvent& glDebugOutputMessageEvent, afHTMLContent& htmlContent);
    void buildCLErrorEventProperties(const gtString& funcName, const gtString& funcArgs, const apOpenCLErrorEvent& clErrorEvent, afHTMLContent& htmlContent);

    // Messages:
    void buildProcessRunResumedMessage(afHTMLContent& htmlContent);

    // State variables:
    void buildStateVariablePropertiesString(const gtString& variableName, const gtString& variableValue, afHTMLContent& htmlContent);

    // Warnings:
    void buildStartRecordingWarningPropertiesString(const gtString& logFilesDirectory, afHTMLContent& htmlContent);
    void buildOpenRecordingFilePropertiesString(const gtString& recordFilePath, afHTMLContent& htmlContent);
    void buildPerformanceWarningPropertiesString(apExecutionMode executionMode, afHTMLContent& htmlContent);

    // Simple HTML message:
    void buildSimpleHTMLMessage(const gtString& title, const gtString& content, gtString& propertiesHTMLMessage, bool shouldCreateHTMLDesignTable);

    // Events:
    void buildMemoryLeakEventHTMLPropertiesString(const apMemoryLeakEvent& eve, bool displayIcon, afHTMLContent& htmlContent);
    void buildSearchingForMemoryLeakEventHTMLPropertiesString(const apSearchingForMemoryLeaksEvent& eve, afHTMLContent& htmlContent);

    // Texture string utilities:
    void getCLImageName(const apCLImage& textureDetails, gtString& strTextureName, bool shortVersion = false);
    void getGLTextureName(apGLTextureMipLevelID textureID, int clImageName, int clSpyID, gtString& strTextureName, bool shortVersion = false);
    void getGLDefaultTextureName(apGLTextureMipLevelID textureID, gtString& strTextureName, bool shortVersion = false);

    // Shader source code viewer:
    void buildShaderCompilationHTMLStatus(int numberOfCompileSucceeded, int numberOfCompileFailed, bool wasLinked, bool wasLinkSuccessful, bool wasValidated, int wasValidationSuccessful, gtString& compilationSummaryStr);
    void buildOpenCLProgramBuildHTMLStatus(const afApplicationTreeItemData& compiledObjectData, bool wasBuildSuccessful, const apCLProgram& afterBuildProgramDetails, gtString& buildSummaryString);
    void buildProgramActiveUniformHTMLString(int contextId, GLuint programName, afHTMLContent& htmlContent);
    bool addActiveUniformsToHTMLContext(int contextId, GLuint programName, afHTMLContent& htmlContent);
    void parseOpenGLProgramOrShaderBuildLog(const afApplicationTreeItemData& compiledObjectData, const gtString& buildLog, gtString& htmlParsedBuildLog);
    void parseOpenCLProgramBuildLog(const afApplicationTreeItemData& compiledObjectData, const gtString& buildLog, gtString& htmlParsedBuildLog);
    void createShadersEditorOpenGLSourceCodeLocationLink(const afApplicationTreeItemData& compiledObjectData, int lineNumber, gtString& htmlLink);

    // Breakpoints & Watch:
    void buildBreakpointPropertiesString(gdBreakpointsItemData* pBreakpointItemData, afHTMLContent& htmlContent);
    void buildWatchVariablePropertiesString(const gtString& variableName, const gtString& variableValue, const gtString& variableType, afHTMLContent& htmlContent);
    void buildLocalVariablePropertiesString(const gtString& variableName, const gtString& variableType, const gtVector<gtString>& variableSubNames, const gtVector<gtString>& variableValues, afHTMLContent& htmlContent);

    // Empty string:
    const static gtString& emptyHTML() {return afHTMLContent::emptyHTML();}

    // Utilities for creation and parse of an HTML link:
    static bool htmlLinkToObjectDetails(const gtString& htmlLink, afApplicationTreeItemData& objectID, int& additionalParameter);
    static bool objectDataToHTMLLink(const afApplicationTreeItemData& objectData, int additionalParameter, gtString& htmlLink);
    static bool itemLinkToDisplayString(const gtString& itemLinkStr, gtString& itemDisplayString);
    static bool itemIDAsString(const afApplicationTreeItemData& objectID, gtString& itemDisplayString);


private:
    void getAdditionalDataParametersPropertiesViewMessage(apContextID contextID, const apFunctionCall& functionCall, gtString& propertiesViewMessage, afProgressBarWrapper* pProgressBar);
    void getAdditionalTextureThumbnailPropertiesViewMessage(apContextID contextID, const apFunctionCall& functionCall, gtString& textureThumbnail, afProgressBarWrapper* pProgressBar);
    void getAssociatedTexturePropertiesViewMessage(apContextID contextID, const apAssociatedTextureNamesPseudoParameter& associatedTextureNames, gtString& propertiesViewMessage, afProgressBarWrapper* pProgressBar);
    void getAssociatedProgramPropertiesViewMessage(const apAssociatedProgramNamePseudoParameter& associatedProgramName, gtString& propertiesViewMessage);
    void getAssociatedShaderPropertiesViewMessage(const apAssociatedShaderNamePseudoParameter& associatedShaderName, gtString& propertiesViewMessage);
    void getFunctionCallWarnings(const apContextID& contextID, int functionCallIndex, afIconType iconType, gtString& functionWarning1, gtString& functionWarning2, gtString& functionWarningTitle);

    // Texture properties aid functions:
    void getTextureProperties(const apGLTexture& textureDetails, int mipLevel, gtString& strTextureType, gtString& strTextureAmountOfMipLevels, gtString& strTextureDimensions, gtString& strTextureRequestedInternalFormat, gtString& strTextureUsedInternalFormat, bool& bInternalFormatsMatch, gtString& strTextureBorderWidth, gtString& strTextureCompressRate, gtString& strEstimatedMemorySize, gtString& strTexturePixelFormat, gtString& strTextureTexelsType, gtString& strTextureCLLink);
    void addTexturesParametersToHTMLContent(afHTMLContent& htmlContent, int mipLevel, const apGLTexture& textureDetails);
    void getCLImageProperties(const apContextID& contextID, int textureIndex, int& texturenName, gtString& textureNameStr, gtString& strTextureType, gtString& strTextureDimensions, gtString& strTexturePixelFormat, gtString& strTextureDataType, gtString& strTextureHandle, gtString& strTextureGLLink, gtString& destructorPfnNotifyStr, gtString& destructorUserDataStr);
    bool getTextureThumbnail(const apContextID& contextID, int textureIndex, int textureId, gtString& strTextureThumbnail, bool addThumbnail, afProgressBarWrapper* pProgressBar);
    bool doesInternalFormatMatchRequestedInternalFormat(GLint requested, GLenum used);

    // Thumbnail aid functions:
    bool generateObjectPreview(const apContextID& contextID, gdHTMLPropertiesObjectType objectType, GLuint objectOpenGLID, apDisplayBuffer bufferType, osFilePath& previewFile, int& imageWidth, int& imageHeight, afProgressBarWrapper* pProgressBar);
    bool generateRenderBufferPreviewFileName(const apGLRenderBuffer& renderBufferDetails, apFileType fileType, osFilePath& previewFileName);
    bool generateStaticBufferPreviewFileName(const apStaticBuffer& staticBufferDetails, apFileType fileType, osFilePath& previewFileName);
    bool generateTexturePreviewFileName(const apContextID& contextID, const apGLTextureMipLevelID mipLevelId, int faceIndex, apFileType fileType, osFilePath& previewFileName);

    void getImagesDir(osFilePath& imagesDir);


    // Shaders and Programs aid functions:
    void addProgramGeometryParametersToHTMLContent(afHTMLContent& htmlContent, const apGLProgram& programDetails);

    // Function calls:
    bool buildFunctionCallArguments(apContextID functionContextId, gtAutoPtr<apFunctionCall> aptrFunctionCall, const gtList<const apParameter*>& funcArguments, afProgressBarWrapper* pProgressBar, gtString& funcArgsStr, gtString& additionalParamsStr);

    // OpenCL enqueued commands aid functions:
    void addCommandSpecificParametersToHtmlContent(afHTMLContent& htmlContent, const apCLEnqueuedCommand& commandDetails);
    void addCommandEventsListAndEventParametersToHtmlContent(afHTMLContent& htmlContent, const apCLEnqueuedCommand& commandDetails);
    void memoryObjectsVectorToHTMLString(const gtVector<oaCLMemHandle>& memHandles, gtString& memHandlesAsString, bool allowBuffers, bool allowTextures);
    void memoryObjectHandleToHTMLString(oaCLMemHandle memHandle, gtString& memHandleAsString, bool allowBuffer, bool allowTexture);

    void addOpenGLInteropPostfix(const apCLImage& textureDetails, gtString& strTextureName, bool shortVersion);

    // Utilities:
    bool generateShaderHtmlLinkString(int contextId, GLuint programName, gdShaderType shaderType, gtString& linkStrBuffer);
private:
    // Deprecation:
    bool buildDeprecationDocumentationByFunctionId(int functionId, const gtString& functionName, apAPIVersion deprecatedAtVersion, apAPIVersion removedAtVersion, gtString& documentationString);
    bool buildDeprecationDocumentationByStatus(apFunctionDeprecationStatus functionDeprecationStatus, const gtString& functionName, int functionId, apAPIVersion deprecatedAtVersion, apAPIVersion removedAtVersion, gtString& documentationString);
    bool buildDeprecationAlternativeByStatus(apFunctionDeprecationStatus functionDeprecationStatus, const gtString& functionName, gtString& documentationString);
    bool buildDeprecationHeader(apFunctionDeprecationStatus functionCallDeprecationStatus, const gtString& functionName, apAPIVersion deprecatedAtVersion, apAPIVersion removedAtVersion, gtString& header);
    bool buildDeprecatedFunctionList(apFunctionDeprecationStatus deprecationStatus, gtString& functionsDeprecatedList);
    bool buildDeprecatedFunctionCallsList(apFunctionDeprecationStatus deprecationStatus, gtString& functionsDeprecatedCallsList);

    // Static members:
    void initializeStaticMembers();

    // Cube map titles strings:
    static gtVector<gtString> m_sCubemapTitles;
    static bool m_sAreStaticInitialized;
};


#endif  // __GDHTMLPROPERTIES
