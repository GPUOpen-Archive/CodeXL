//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsRenderContextMonitor.h
///
//==================================================================================

//------------------------------ gsRenderContextMonitor.h ------------------------------

#ifndef __GSRENDERCONTEXTMONITOR
#define __GSRENDERCONTEXTMONITOR

// Forward decelerations:
class oaPixelFormat;
class apGLRenderContextGraphicsInfo;

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTAPIClasses/Include/apOpenGLExtensionsId.h>
#include <AMDTAPIClasses/Include/apGLRenderContextGraphicsInfo.h>
#include <AMDTAPIClasses/Include/apGLRenderContextInfo.h>

// Spies utilities:
#include <AMDTServerUtilities/Include/suContextMonitor.h>

// Local:
#include <src/gsRenderPrimitivesStatisticsLogger.h>
#include <src/gsCallsHistoryLogger.h>
#include <src/gsProgramsAndShadersMonitor.h>
#include <src/gsLightsMonitor.h>
#include <src/gsStateVariablesSnapshot.h>
#include <src/gsTexturesMonitor.h>
#include <src/gsRenderBuffersMonitor.h>
#include <src/gsPipelineMonitor.h>
#include <src/gsSamplersMonitor.h>
#include <src/gsFBOMonitor.h>
#include <src/gsVBOMonitor.h>
#include <src/gsDisplayListMonitor.h>
#include <src/gsStaticBuffersMonitor.h>
#include <src/gsForcedModesManager.h>
#include <src/gsGLDebugOutputManager.h>
#include <src/gsRenderContextPerformanceCountersManager.h>
#include <src/gsVertexArrayData.h>
#include <src/gsVertexArrayDrawer.h>
#include <src/gsAnalyzeModeExecutor.h>

// ATI Counters are supported on Windows only:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <src/gsATIRenderContextPerformanceCountersReader.h>
#endif


// ----------------------------------------------------------------------------------
// Class Name:           gsRenderContextMonitor
// General Description:
//   Monitors an OpenGL render context:
//   - OS device context and render context handles,
//   - The used pixel format,
//   - Texture objects,
//   - Programs and Shaders,
//   - Lights,
//   - etc
//
// Author:               Yaki Tebeka
// Creation Date:        17/9/2004
// ----------------------------------------------------------------------------------
class gsRenderContextMonitor : public suContextMonitor
{
public:
    gsRenderContextMonitor(int contextSpyId, oaDeviceContextHandle deviceContextOSHandle, oaOpenGLRenderContextHandle renderContextOSHandle,
                           bool shouldMonitorPerformanceCounters, apMonitoredFunctionId creationFunc, const int* attribList, bool isDebugFlagForced);
    ~gsRenderContextMonitor();

    // suContextMonitor overrides:
    virtual void onFrameTerminatorCall();
    virtual void addFunctionCall(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionDeprecationStatus functionDeprecationStatus);
    virtual void onMonitoredFunctionCall();
    virtual bool updateContextDataSnapshot(bool sendEvents = false);
    virtual void beforeUpdatingContextDataSnapshot();
    virtual void afterUpdatingContextDataSnapshot();
    virtual void afterMonitoredFunctionExecutionActions(apMonitoredFunctionId calledFunctionId);

    // Events:
    void onContextDeletion();
    void beforeContextDeletion() const;
    void onContextMadeCurrent(oaDeviceContextHandle deviceContextOSHandle);
    void onContextRemovedFromBeingCurrent();
    bool onActiveTextureUnitChanged(GLenum activeTextureUnit);
    void onTextureTargetBind(GLenum target, GLuint textureName);
    void onTextureTargetBindToUnit(GLenum textureUnit, GLenum target, GLuint textureName);
    void onTextureBindToUnit(GLenum textureUnit, GLuint textureName);
    bool onMultiTextureTargetBind(GLenum texunit, GLenum target, GLuint textureName);
    bool onTextureImageLoaded(GLenum bindTarget, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type);
    bool onNamedTextureImageLoaded(GLuint texture, GLenum bindTarget, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type);
    bool onTextureStorageSet(GLenum bindTarget, GLsizei levels, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth);
    bool onNamedTextureStorageSet(GLuint texture, GLsizei levels, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth);
    bool onNamedTextureStorageSet(GLuint texture, GLenum bindTarget, GLsizei levels, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth);
    bool onNamedTextureImageLoaded(GLuint texture, GLint level, GLenum format);
    bool onMultiTextureImageLoaded(GLenum texUnit, GLenum bindTarget, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type);
    void onTextureMipmapGenerate(GLenum bindTarget);
    void onNamedTextureMipmapGenerate(GLuint texture);
    void onMultiTextureMipmapGenerate(GLenum texunit, GLenum bindTarget);
    void onTextureBuffer(GLenum target, GLenum internalformat, GLuint buffer);
    void onTextureBuffer(GLuint texture, GLenum target, GLenum internalformat, GLuint buffer);
    void onMultiTextureBuffer(GLenum texunit, GLenum target, GLenum internalformat, GLuint buffer);
    bool onTextureSubImageLoaded(GLenum bindTarget, GLint level);
    bool onTextureSubImageLoaded(GLuint texture, GLenum bindTarget, GLint level);
    bool onNamedTextureSubImageLoaded(GLuint texture, GLint level);
    bool onMultiTextureSubImageLoaded(GLenum texunit, GLenum bindTarget, GLint level);
    bool onTextureIntParameterChanged(GLenum target, GLenum pname, const GLint* pParam);
    bool onNamedTextureIntParameterChanged(GLuint texture, GLenum pname, const GLint* pParam);
    bool onMultiTextureIntParameterChanged(GLuint texture, GLenum target, GLenum pname, const GLint* pParam);
    bool onNamedTextureUIntParameterChanged(GLuint texture, GLenum pname, const GLuint* pParam);
    bool onMipmapGenerateParamSet(GLenum target, GLenum pname, GLfloat paramValueAsInt);
    bool onNamedMipmapGenerateParamSet(GLuint texture, GLenum pname, GLfloat paramValueAsInt);
    bool onMultiTextureMipmapGenerateParamSet(GLenum texunit, GLenum target, GLenum pname, GLfloat paramValueAsInt);
    bool onMultiTextureUIntParameterChanged(GLenum texunit, GLenum target, GLenum pname, const GLuint* pParam);
    bool onTextureFloatParameterChanged(GLenum target, GLenum pname, const GLfloat* pParam);
    bool onTextureLevelFloatParameterChanged(GLenum target, GLint level, GLenum pname, const GLfloat* pParam);
    bool onNamedTextureFloatParameterChanged(GLuint texture, GLenum pname, const GLfloat* pParam);
    bool onNamedTextureLevelFloatParameterChanged(GLuint texture, GLint level, GLenum pname, const GLfloat* pParam);
    bool onProgramUsed(GLuint programName);
    void onBreakOnOpenGLErrorModeChanged(bool breakOnGLErrors) { _shouldTestForOpenGLErrorsAtNextFunctionCall = breakOnGLErrors; };
    bool isOpenGLErrorCheckNeededOnModeTurnedOn() const { return _shouldTestForOpenGLErrorsAtNextFunctionCall; };
    void checkForOpenGLErrorAfterModeChange();
    bool afterProgramRestoredFromStubFS(GLuint programName);
    int openCLSharedContextID() const { return m_contextInfo.openCLSpyID(); };
    void setOpenCLSharedContextID(int clID) { m_contextInfo.setOpenCLSpyID(clID); };

    // OpenGL ES Emulator functions:
    bool setTextureCropRectangle(GLenum target, const GLfloat* pRectangleAsFloatArray);
    bool setTextureCropRectangle(GLuint texture, GLenum target, const GLfloat* pRectangleAsFloatArray);

    // Context Info:
    const apGLRenderContextInfo& RenderContextInfo() const { return m_contextInfo; };

    // Spy Id and OS Handles:
    int spyId() const { return m_contextInfo.spyID(); };
    oaDeviceContextHandle deviceContextOSHandle() const { return _deviceContextOSHandle; };
    oaOpenGLRenderContextHandle renderContextOSHandle() const { return _renderContextOSHandle; };
    bool wasDeleted() const;

    // Allocated object Id:
    int allocatedObjectId() const { return m_contextInfo.getAllocatedObjectId(); };

    // Pixel format:
    void setPixelFormatId(oaPixelFormatId OSPixelFormatId);
    oaPixelFormatId pixelFormatId() const { return _pixelFormatId; };
    const oaPixelFormat* pixelFormatDescription() const { return _pPixelFormatDescription; };
    void initializePixelFormatFromOpenGLES();

    // Render primitives statistics logger:
    const gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger() const { return _renderPrimitivesStatisticsLogger; };
    gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger() { return _renderPrimitivesStatisticsLogger; };

    // Textures:
    const gsTexturesMonitor* texturesMonitor() const { return _pTexturesMonitor; };
    gsTexturesMonitor* texturesMonitor() { return _pTexturesMonitor; };
    void setTexturesMonitor(gsTexturesMonitor* texturesMonitor) {_pTexturesMonitor = texturesMonitor; };

    // Texture units:
    int amountOfTextureUnits() const { return (int)_textureUnitMonitors.size(); };
    GLenum textureUnitIndexToName(int textureUnitIndex) const;
    int textureUnitNameToIndex(GLenum textureUnitName) const;
    int activeTextureUnitIndex() const { return _activeTextureUnitIndex; };

    // Render Buffers:
    const gsRenderBuffersMonitor* renderBuffersMonitor() const { return _pRenderBuffersMonitor; };
    gsRenderBuffersMonitor* renderBuffersMonitor() { return _pRenderBuffersMonitor; };
    void setRenderBuffersMonitor(gsRenderBuffersMonitor* rendBufMon) { _pRenderBuffersMonitor = rendBufMon; };
    bool activateRenderBuffer(GLuint bufferName);
    GLuint getActiveRenderBufferObject();
    bool setActiveRenderBufferObjectParameters(GLenum internalformat, GLsizei width, GLsizei height);
    bool setRenderBufferObjectParameters(GLuint buffer, GLenum internalformat, GLsizei width, GLsizei height);

    // Program pipelines:
    gsPipelineMonitor& pipelinesMonitor() { return _pipelineMonitor; };
    const gsPipelineMonitor& pipelinesMonitor() const { return _pipelineMonitor; };

    // Samplers:
    gsSamplersMonitor& samplersMonitor() { return _samplersMonitor; };
    const gsSamplersMonitor& samplersMonitor() const { return _samplersMonitor; };

    // Frame buffer objects:
    const gsFBOMonitor* fboMonitor() const { return _pFBOMonitor; };
    gsFBOMonitor* fboMonitor() { return _pFBOMonitor; };
    void setFBOMonitor(gsFBOMonitor* fboMon) { _pFBOMonitor = fboMon; };
    GLuint getActiveReadFboName() const { return m_activeReadFBOName; };
    GLuint getActiveDrawFboName() const { return m_activeDrawFBOName; };
    // GLuint getActiveFboName() const;
    bool activateFBO(GLuint frameBuffer, GLenum target);
    bool updateFBOActiveState(bool isActive, GLuint framebuffer);
    GLuint getActiveFBOForTarget(GLenum fboTarget) const;
    bool bindObjectToTheActiveFBO(GLenum fboTarget, GLenum attachmentPoint, GLenum attachmentTarget, GLuint objectName, GLuint textureLayer);
    bool bindTextureToTheActiveFBO(GLenum fboTarget, GLenum bindTarget, GLenum attachmentPoint, GLuint textureName, GLuint textureLayer);
    bool bindTextureToFBO(GLuint framebuffer, GLenum attachmentPoint, GLuint textureName, GLuint textureLayer);
    bool bindTextureLayerToTheActiveFBO(GLenum fboTarget, GLenum bindTarget, GLenum attachmentPoint, GLuint textureName, GLuint textureLayer);
    bool bindObjectToFBO(GLuint framebuffer, GLenum attachmentPoint, GLenum attachmentTarget, GLuint objectName, GLuint textureLayer);

    // Vertex buffer objects:
    const gsVBOMonitor* vboMonitor() const { return _pVBOMonitor; };
    gsVBOMonitor* vboMonitor() { return _pVBOMonitor; };

    // Display lists:
    const gsDisplayListMonitor* displayListsMonitor() const { return _pDisplayListMonitor; };
    gsDisplayListMonitor* displayListsMonitor() { return _pDisplayListMonitor; };

    // Buffers monitor:
    const gsStaticBuffersMonitor& buffersMonitor() const { return _buffersMonitor; };
    gsStaticBuffersMonitor& buffersMonitor() { return _buffersMonitor; };

    // Program and Shader objects:
    const gsProgramsAndShadersMonitor* programsAndShadersMonitor() const { return _pProgramsAndShadersMonitor; };
    gsProgramsAndShadersMonitor* programsAndShadersMonitor() { return _pProgramsAndShadersMonitor; };
    void setProgramsAndShadersMonitor(gsProgramsAndShadersMonitor* programsAndShadersMonitor) {_pProgramsAndShadersMonitor = programsAndShadersMonitor; };
    GLuint activeProgramName() const { return _activeProgramName; };

    // Lights:
    const gsLightsMonitor& lightsMonitor() const { return _lightsMonitor; };
    gsLightsMonitor& lightsMonitor() { return _lightsMonitor; };

    // State variables:
    const gsStateVariablesSnapshot& getStateVariablesDefaultValues() const { return _stateVariablesDefaultValues; };
    const gsStateVariablesSnapshot& getStateVariablesSnapshot() const { return _stateVariablesSnapshot; };
    void setStateVariableUpdateMode(bool shouldUpdate) { _shouldUpdateStateVariables = shouldUpdate; };

    // Forced modes:
    const gsForcedModesManager& forcedModesManager() const { return _forcedModesManager; };
    gsForcedModesManager& forcedModesManager() { return _forcedModesManager; };

    // Debug output:
    const gsGLDebugOutputManager& debugOutputManager() const { return m_debugOutputManager; };
    gsGLDebugOutputManager& debugOutputManager() { return m_debugOutputManager; };

    // Performance counters:
    const gsRenderContextPerformanceCountersManager& performanceCountersManager() const { return _performanceCountersManager; };
    gsRenderContextPerformanceCountersManager& performanceCountersManager() { return _performanceCountersManager; };

    // ATI Performance counters:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA
    const gsATIRenderContextPerformanceCountersReader& atiPerformanceCountersManager() const { return _atiPerformanceCountersReader; };
    gsATIRenderContextPerformanceCountersReader& atiPerformanceCountersManager() { return _atiPerformanceCountersReader; };
#endif
#endif

    // Vertex arrays:
    const gsVertexArrayData& currentVertexArrayData() const { return _currentVertexArrayData; };
    gsVertexArrayData& currentVertexArrayData() { return _currentVertexArrayData; };
    gsVertexArrayDrawer& vertexArrayDrawer() { return _vertexArrayDrawer; };

    // Texture Units:
    bool getEnabledTexturingMode(int textureUnitIndex, bool& isTexturingEnabled, apTextureType& enabledTexturingMode) const;
    GLuint bindTextureName(int textureUnitIndex, apTextureType bindTarget) const;
    void applyForcedStubTextureObjects();
    void cancelForcedStubTextureObjects();
    gsGLTexture* getBoundTexture(int textureUnitIndex, GLenum bindTarget);
    GLuint createDefaultTextureObject(int textureUnitIndex, apTextureType textureType);
    void markTextureParametersAsNonUpdated();

    // Misc:
    bool getGLObjectType(GLenum objectName, osTransferableObjectType& objType);
    bool isInOpenGLBeginEndBlock() const;
    void getOpenGLVersion(int& majorNumber, int& minorNumber) const;
    bool isComaptibilityContext() const { return _isCompatibiltyContext; };
    bool isOpenGLVersionOrNewerCoreContext(int maj = 3, int min = 1) const;
    bool isBackwardsCompatible() const { return _isForwardCompatibleContext; };
    bool isDebugContext() const { return _isDebugContext; };
    bool isDebugContextFlagForced() const { return _isDebugContextFlagForced; };
    bool flushContentToDrawSurface();
    int getObjectSharingContextID() const { return m_contextInfo.sharingContextID(); };
    void setObjectSharingContext(int objectSharingContext, gsRenderContextMonitor* otherRC, bool deleteSubMonitors = false);
    bool isSharingLists() const {return (0 < getObjectSharingContextID()); };
    const apGLRenderContextGraphicsInfo& GraphicsInfo() const { return m_contextGraphicsInfo; };

    void startStateVariablesAnalyzeLogging(int calledFunctionId);

    // Extension functions to use in state variable reader. These function pointers are managed here
    // for performance:
    PFNGLGETINTEGER64VPROC myGLGetInteger64v() const { return _glGetInteger64v; };

    // Extension function pointer:
    PFNGLGETINTEGERUI64VNVPROC myGLGetIntegerui64vNV() const { return _glGetIntegerui64vNV; };

private:
    void parseAttribList(const int* attribList);
    void logCreatedContextParameters();
    void updateOpenGLVersion();
    void updateVendorType();
    void checkForSoftwareRenderer();
    void constructGraphicsInfo();
    void updateSharedContextsList();
    int amountOfGLSupportedTextureUnits() const;
    bool disablelVerticalSync() const;
    int getIntChannelSizeParameterDefaultValue(apOpenGLStateVariableId channelStateVarId) const;

    // Private implementations for direct state access extension:
    bool onTextureImageLoaded(gsGLTexture* pTextureObject, GLenum bindTarget, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type);
    bool onMipmapGenerateParamSet(gsGLTexture* pTexParamObj , GLenum pname, GLfloat paramValueAsFloat);
    void onTextureBuffer(gsGLTexture* pTextureObject, GLenum internalformat, GLuint buffer);
    bool bindObjectToFBO(apGLFBO* pFBO, GLenum attachmentPoint, GLenum attachmentTarget, GLuint objectName, GLuint textureLayer);

    // Do not allow the use of the default, copy or move constructors, or operator=:
    gsRenderContextMonitor() = delete;
    gsRenderContextMonitor& operator=(const gsRenderContextMonitor&) = delete;
    gsRenderContextMonitor(const gsRenderContextMonitor&) = delete;
    gsRenderContextMonitor& operator=(gsRenderContextMonitor&&) = delete;
    gsRenderContextMonitor(gsRenderContextMonitor&&) = delete;

private:
    // Context information (includes context Id, allocated object Id, and sharing information (GL+CL):
    // - The internal id of a context this contexts shares lists with, or -1 if it doesn't
    // - The context's allocated Object ID:
    // - OpenCL shared context id:
    apGLRenderContextInfo m_contextInfo;

    // Context graphics information:
    apGLRenderContextGraphicsInfo m_contextGraphicsInfo;

    // The render context's "draw surface" native OS handle:
    // (The surface into which OpenGL draws using this render context)
    oaDeviceContextHandle _deviceContextOSHandle;

    // The render context OS handle:
    oaOpenGLRenderContextHandle _renderContextOSHandle;

    // The render context pixel format:
    oaPixelFormatId _pixelFormatId;

    // Contains the pixel format description:
    oaPixelFormat* _pPixelFormatDescription;

    // Contains this context's OpenGL vendor string:
    gtString _openGLVendorString;

    // Contains this context's OpenGL renderer string:
    gtString _openGLRendererString;

    // Contains this context's OpenGL renderer version string:
    gtString m_openGLVersionString;

    // Contains this context's OpenGL shading language version string:
    gtString _openGLShadingLangVersionString;

    // Contains true iff this is the first time that this render context is made the
    // current context:
    bool _isFirstMakeCurrentOfThisContext;

    // The render primitives statistics logger:
    gsRenderPrimitivesStatisticsLogger _renderPrimitivesStatisticsLogger;

    // Contains the state variables default values:
    // (Updated when the context becomes current for the first time)
    gsStateVariablesSnapshot _stateVariablesDefaultValues;

    // A snapshot of the render context state variables values:
    // (Updated only when needed)
    gsStateVariablesSnapshot _stateVariablesSnapshot;
    bool _shouldUpdateStateVariables;

    // An object that handles everything related to analyze mode:
    gsAnalyzeModeExecutor _analyzeModeExecutor;

    // Monitors textures:
    gsTexturesMonitor* _pTexturesMonitor;

    // Holds the active texture unit index (in the _textureUnitMonitors vector):
    int _activeTextureUnitIndex;

    // Holds texture units monitors:
    gtPtrVector<gsTextureUnitMonitor*> _textureUnitMonitors;

    // Contains the amount of multi texture texture units:
    // (GL_MAX_TEXTURE_UNITS  of GL_ARB_multitexture extension):
    GLint _maxTextureUnits;

    // Monitors render buffers:
    gsRenderBuffersMonitor* _pRenderBuffersMonitor;

    // Program pipelines monitor:
    gsPipelineMonitor _pipelineMonitor;

    // Samplers monitor:
    gsSamplersMonitor _samplersMonitor;

    // The OpenGL name of the currently bounded render buffer object:
    GLuint _activeRenderBuffer;

    // Monitors FBOs:
    gsFBOMonitor* _pFBOMonitor;

    // Monitors VBOs:
    gsVBOMonitor* _pVBOMonitor;

    // Display List monitor:
    gsDisplayListMonitor* _pDisplayListMonitor;

    // Contain the active frame buffer object names:
    GLuint m_activeReadFBOName;
    GLuint m_activeDrawFBOName;

    // Monitors buffers:
    gsStaticBuffersMonitor _buffersMonitor;

    // Monitors program and shader objects:
    gsProgramsAndShadersMonitor* _pProgramsAndShadersMonitor;

    // Contains the active program name (or 0 if there is no active program):
    GLuint _activeProgramName;

    // Monitors lights:
    gsLightsMonitor _lightsMonitor;

    // Managed forced modes:
    gsForcedModesManager _forcedModesManager;

    // Debug output manager:
    gsGLDebugOutputManager m_debugOutputManager;

    // Contain true iff performance counters should be monitored:
    bool _shouldInitializePerformanceCounters;

    // The performance counters manager:
    gsRenderContextPerformanceCountersManager _performanceCountersManager;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA
    // The ATI performance counters manager:
    gsATIRenderContextPerformanceCountersReader _atiPerformanceCountersReader;
#endif
#endif

    // Holds the current vertex array data:
    gsVertexArrayData _currentVertexArrayData;

    // Draws vertex arrays, performs data migrations if needed:
    gsVertexArrayDrawer _vertexArrayDrawer;

    // Contains OpenGL major and minor version:
    int _oglVersion[2];

    // True iff this is a 3.1+ context with the GL_ARB_compatibilty extension or
    // a 3.2+ context created with the compatibility profile:
    bool _isCompatibiltyContext;
    bool _isCompatibiliyProfile;
    bool _isForwardCompatibleContext;

    // Debug context information:
    bool _isDebugContext;
    bool _isDebugContextFlagForced;

    // A vector of GPU handles for which this context has affinity:
    gtVector<intptr_t> _gpuAffinities;

    // Contain the vendor type:
    oaVendorType _vendorType;

    // Contains true if we need to check for OpenGL errors that existed before turning on "Break on OpenGL Errors" mode:
    bool _shouldTestForOpenGLErrorsAtNextFunctionCall;

    // Extension function pointer:
    PFNGLGETINTEGER64VPROC _glGetInteger64v;

    // Extension function pointer:
    PFNGLGETINTEGERUI64VNVPROC _glGetIntegerui64vNV;
};


#endif  // __GSRENDERCONTEXTMONITOR
