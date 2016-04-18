//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsRenderContextMonitor.cpp
///
//==================================================================================

//------------------------------ gsRenderContextMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSAPIWrappers/Include/oaPixelFormat.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTAPIClasses/Include/apInternalFormat.h>
#include <AMDTAPIClasses/Include/apDefaultTextureNames.h>
#include <AMDTAPIClasses/Include/apDefinitions.h>
#include <AMDTAPIClasses/Include/ap2DRectangle.h>
#include <AMDTAPIClasses/Include/apGLRenderContextInfo.h>
#include <AMDTAPIClasses/Include/apGLRenderContextGraphicsInfo.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
#include <AMDTAPIClasses/Include/Events/apSpyProgressEvent.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>

// Local:
#include <src/gsDeprecationAnalyzer.h>
#include <src/gsDisplayListMonitor.h>
#include <src/gsExtensionsManager.h>
#include <src/gsGLDebugOutputManager.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsRenderContextMonitor.h>
#include <src/gsStringConstants.h>
#include <src/gsWrappersCommon.h>
#ifdef _GR_IPHONE_BUILD
    #include <src/gsEAGLWrappers.h>
#endif


// Limit how many attributes we "read" before deciding the array is in a bad format:
#define GS_MAX_READ_CONTEXT_ATTRIBUTES 1024

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::gsRenderContextMonitor
// Description: Default constructor.
// Arguments:
//   contextSpyId - The spy id for this render context.
//   deviceContextOSHandle - The OS handle of the device context that holds the
//                           render context (casted into void*).
//   renderContextOSHandle - The OS handle of the OpenGL render context (casted into void*)
//
// Author:      Yaki Tebeka
// Date:        17/9/2004
// ---------------------------------------------------------------------------
gsRenderContextMonitor::gsRenderContextMonitor(int contextSpyId,
                                               oaDeviceContextHandle deviceContextOSHandle,
                                               oaOpenGLRenderContextHandle renderContextOSHandle,
                                               bool shouldMonitorPerformanceCounters,
                                               apMonitoredFunctionId creationFunc,
                                               const int* attribList,
                                               bool isDebugFlagForced)
    : suContextMonitor(apContextID(AP_OPENGL_CONTEXT, contextSpyId)),
      _objectSharingContext(-1),
      _allocatedObjectId(-1),
      _deviceContextOSHandle(deviceContextOSHandle),
      _renderContextOSHandle(renderContextOSHandle),
      _pixelFormatId(OA_NO_PIXEL_FORMAT_ID),
      _pPixelFormatDescription(NULL),
      _isFirstMakeCurrentOfThisContext(true),
      _shouldUpdateStateVariables(true),
      _pTexturesMonitor(NULL),
      _activeTextureUnitIndex(0),
      _maxTextureUnits(1),
      _pRenderBuffersMonitor(NULL),
      _activeRenderBuffer(0),
      _pFBOMonitor(NULL),
      _pVBOMonitor(NULL),
      _pDisplayListMonitor(NULL),
      m_activeReadFBOName(0),
      m_activeDrawFBOName(0),
      _buffersMonitor(contextSpyId),
      _pProgramsAndShadersMonitor(NULL),
      _activeProgramName(0),
      m_debugOutputManager(contextSpyId, _forcedModesManager),
      _shouldInitializePerformanceCounters(shouldMonitorPerformanceCounters),
      _vertexArrayDrawer(_currentVertexArrayData),
      _isCompatibiltyContext(false),
      _isCompatibiliyProfile(true),
      _isForwardCompatibleContext(false),
      _isDebugContext(false),
      _isDebugContextFlagForced(isDebugFlagForced),
      _vendorType(OA_VENDOR_UNKNOWN),
      _shouldTestForOpenGLErrorsAtNextFunctionCall(false),
      _openCLSharedContextID(0),
      _glGetInteger64v(NULL),
      _glGetIntegerui64vNV(NULL)
{
    // Parse our attrib list:
    parseAttribList(attribList);

    // Create the textures monitor:
    _pTexturesMonitor = new gsTexturesMonitor(contextSpyId);

    // Create the programs and shaders monitor:
    _pProgramsAndShadersMonitor = new gsProgramsAndShadersMonitor(contextSpyId);

    // Create the RBO monitor:
    _pRenderBuffersMonitor = new gsRenderBuffersMonitor(contextSpyId);

    // Create the FBO monitor:
    _pFBOMonitor = new gsFBOMonitor;

    _pVBOMonitor = new gsVBOMonitor(contextSpyId);

    _pDisplayListMonitor = new gsDisplayListMonitor(this);

    if (_shouldInitializePerformanceCounters)
    {
        // Notify members that need a back pointer to me:
        _performanceCountersManager.setMonitoredRenderContext(*this);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA
        {
            _atiPerformanceCountersReader.setMonitoredRenderContext(*this);
        }
#endif
#endif
    }

    // Initialize the supported OpenGL version to 1.1:
    _oglVersion[0] = 1;
    _oglVersion[1] = 1;

    // Set _analyzeModeExecutor render context monitor:
    bool rc = _analyzeModeExecutor.initialize(this);
    GT_ASSERT_EX(rc, L"Problem with analyze mode executor initialization");

    // If this is a real RC and not the one we use to represent context-less function calls
    if (contextSpyId > 0)
    {
        // Register in the allocated objects monitor:
        apGLRenderContextInfo dummyRCInfo;
        su_stat_theAllocatedObjectsMonitor.registerAllocatedObject(dummyRCInfo);
        _allocatedObjectId = dummyRCInfo.getAllocatedObjectId();
    }

    // Set the render primitive logger pointer to me:
    _renderPrimitivesStatisticsLogger.setRenderContextMonitor(this);

    // Initialize the calls history logger:
    gtVector<gtString> attribStrings;

    if (nullptr != attribList)
    {
        const int* pCurrentAttrib = attribList;

        while (0 != *pCurrentAttrib)
        {
            gtString attr;
            apGLenumValueToString((GLenum)(*(pCurrentAttrib++)), attr);
            attr.appendFormattedString(L" = %d", *(pCurrentAttrib++));
            attribStrings.push_back(attr);
        }
    }

    _pCallsHistoryLogger = new gsCallsHistoryLogger(contextSpyId, creationFunc, (nullptr == attribList) ? nullptr : &attribStrings);
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::~gsRenderContextMonitor
// Description: Destructor.
// Author:      Yaki Tebeka
// Date:        25/1/2005
// ---------------------------------------------------------------------------
gsRenderContextMonitor::~gsRenderContextMonitor()
{
    // Delete the submonitors (textures, programs, ... ), but do this only on the "real" monitor:
    if (!isSharingLists())
    {
        // Delete all the pointed members if we haven't yet:
        if (_pTexturesMonitor != NULL)
        {
            delete _pTexturesMonitor;
            _pTexturesMonitor = NULL;
        }

        if (_pProgramsAndShadersMonitor != NULL)
        {
            delete _pProgramsAndShadersMonitor;
            _pProgramsAndShadersMonitor = NULL;
        }

        if (_pRenderBuffersMonitor != NULL)
        {
            delete _pRenderBuffersMonitor;
            _pRenderBuffersMonitor = NULL;
        }

        if (_pFBOMonitor != NULL)
        {
            delete _pFBOMonitor;
            _pFBOMonitor = NULL;
        }

        if (_pVBOMonitor != NULL)
        {
            delete _pVBOMonitor;
            _pVBOMonitor = NULL;
        }

        if (_pDisplayListMonitor != NULL)
        {
            delete _pDisplayListMonitor;
            _pDisplayListMonitor = NULL;
        }

        int amountOfRCs = gs_stat_openGLMonitorInstance.amountOfContexts();

        // Make sure everyone pointing at us is notified (Note: the RCs vector is zero-based, but RC 0 is a fake one
        // we use to mark events which don't belong to any RC):
        for (int i = 1; i < amountOfRCs; i++)
        {
            gsRenderContextMonitor* currentRC = gs_stat_openGLMonitorInstance.renderContextMonitor(i);

            if (currentRC != NULL)
            {
                if (currentRC->getObjectSharingContextID() == _contextID._contextId)
                {
                    currentRC->setTexturesMonitor(NULL);
                    currentRC->setProgramsAndShadersMonitor(NULL);
                    currentRC->setRenderBuffersMonitor(NULL);
                    currentRC->setFBOMonitor(NULL);
                }
            }
        }
    }

    if (_pPixelFormatDescription != NULL)
    {
        delete _pPixelFormatDescription;
    }

    // Clear texture units monitors:
    _textureUnitMonitors.deleteElementsAndClear();
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onContextDeletion
// Description: Is called when my monitored context is deleted from the OS.
// Author:      Yaki Tebeka
// Date:        17/10/2005
// Implementation notes:
//   We would like to keep the context history, therefore, we don't delete this
//   class instance and only clear the OS handles.
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::onContextDeletion()
{
    // Notify members about the context deletion:
    gsCallsHistoryLogger* pCallsHistoryLogger = (gsCallsHistoryLogger*)callsHistoryLogger();
    GT_IF_WITH_ASSERT(pCallsHistoryLogger != NULL)
    {
        pCallsHistoryLogger->onContextDeletion();
    }
    _pTexturesMonitor->onContextDeletion();
    _pRenderBuffersMonitor->onContextDeletion();
    _buffersMonitor.onContextDeletion();

    // Mark the context as deleted. Note that while the context resource might be deleted later
    // (as is the case in glX), the handle is invalidated immediately in all implementations (wgl, glX, CGL):
    _deviceContextOSHandle = NULL;
    _renderContextOSHandle = NULL;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::beforeContextDeletion
// Description: Is called before my monitored context is deleted from the OS.
// Return Val: void
// Author:      Sigal Algranaty
// Date:        30/11/2008
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::beforeContextDeletion() const
{
    // Perform textures monitor before context deletion actions:
    if (_pTexturesMonitor != NULL)
    {
        _pTexturesMonitor->beforeContextDeletion();
    }
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onContextMadeCurrent
// Description: Is called when the context becomes the current context of the
//              calling thread.
// Arguments: deviceContextOSHandle - Native OS handle of the device context of the
//                                    draw surface (the surface into which OpenGL draws).
// Author:      Yaki Tebeka
// Date:        1/3/2005
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::onContextMadeCurrent(oaDeviceContextHandle deviceContextOSHandle)
{
    // Log the new device context:
    _deviceContextOSHandle = deviceContextOSHandle;

    // If this is the first time in which the context is made current:
    if (_isFirstMakeCurrentOfThisContext)
    {
        // Mark that the first make current was done:
        _isFirstMakeCurrentOfThisContext = false;

        // If my monitored context is not the NULL context:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        // CGL doesn't need a device context for render context creation (and doesn't really have a
        // term which is parallel to a device context). Furthermore, CGL pixel formats are objects that
        // can be released, thus there is a possibility the user will call CGLGetPixelFormat ->
        // CGLCreateRenderContext -> CGLReleasePixelFormat ->CGLSetCurrentContext, which would mean
        // by the time we get here, the pixel format "id" we have is invalid.
        // For both these reasons, this procedure is done in Mac when we get the pixel format.

        // Set the pixel format for the static buffers monitor:
        _buffersMonitor.setRenderContextPixelFormat(_pPixelFormatDescription);
#else
        // Initialize the context's pixel format:
        GT_IF_WITH_ASSERT(_deviceContextOSHandle != NULL)
        {
            // Log the pixel format description:
            _pPixelFormatDescription = new oaPixelFormat(_deviceContextOSHandle, _pixelFormatId);
            GT_IF_WITH_ASSERT(_pPixelFormatDescription != NULL)
            {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
                // Our Linux implementation og oaPixelFormat uses glXGetConfig for initialize():
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glXGetConfig);
#endif

                bool rcInit = _pPixelFormatDescription->initialize();
                GT_ASSERT(rcInit);

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
                // Our Linux implementation og oaPixelFormat uses glXGetConfig for initialize():
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glXGetConfig);
#endif

                // Set the pixel format for the static buffers monitor:
                _buffersMonitor.setRenderContextPixelFormat(_pPixelFormatDescription);

                // Check if this context is rendered using a software renderer:
                checkForSoftwareRenderer();
            }
        }
#endif

        // Disable calls history logging, since GLExpert and maybe others retrieves
        // OpenGL extension function pointers using OpenGL extensions mechanism.
        suCallsHistoryLogger* pCallsHistoryLogger = (gsCallsHistoryLogger*)callsHistoryLogger();
        bool storedLoggingMode = false;
        GT_IF_WITH_ASSERT(pCallsHistoryLogger != NULL)
        {
            storedLoggingMode = pCallsHistoryLogger->isLoggingEnabled();
            pCallsHistoryLogger->enableLogging(false);
        }

        // Update the supported OpenGL extension:
        updateOpenGLVersion();

        // Log the created context parameters:
        logCreatedContextParameters();

        // Update the operating system vendor type:
        updateVendorType();

        // Notify the extensions manager about the context creation:
        gsExtensionsManager& theExtensionsMgr = gsExtensionsManager::instance();
        theExtensionsMgr.onFirstTimeContextMadeCurrent(_contextID._contextId, _oglVersion, _isCompatibiltyContext);

        // Check for the compatibility extension:
        if (theExtensionsMgr.isExtensionSupported(_contextID._contextId, AP_GL_ARB_compatibility))
        {
            _isCompatibiltyContext = true;
        }

        // Disable Vertical Sync for this render context:
        disablelVerticalSync();

        // Notify members about the context made current for the first time:
        beforeUpdatingContextDataSnapshot();
        _stateVariablesDefaultValues.onFirstTimeContextMadeCurrent(*this);
        _stateVariablesDefaultValues.updateContextDataSnapshot();
        _stateVariablesSnapshot.onFirstTimeContextMadeCurrent(*this);
        _pProgramsAndShadersMonitor->onFirstTimeContextMadeCurrent();
        _forcedModesManager.onFirstTimeContextMadeCurrent(*this);
        m_debugOutputManager.onFirstTimeContextMadeCurrent();
        _analyzeModeExecutor.onFirstTimeContextMadeCurrent();
        _pTexturesMonitor->onFirstTimeContextMadeCurrent();
        _pRenderBuffersMonitor->onFirstTimeContextMadeCurrent();
        _samplersMonitor.onFirstTimeContextMadeCurrent();

        // Light state was deprecated in OpenGL 3.0 and removed in 3.1:
        if (_isCompatibiltyContext ||
            (3 > _oglVersion[0]) ||
            ((3 == _oglVersion[0]) && (1 > _oglVersion[1])))
        {
            _lightsMonitor.onFirstTimeContextMadeCurrent();
        }

        if (_shouldInitializePerformanceCounters)
        {

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA

            if (_vendorType == OS_VENDOR_ATI)
            {
                _atiPerformanceCountersReader.onFirstTimeContextMadeCurrent();
            }

#endif
#endif
        }

        _buffersMonitor.onFirstTimeContextMadeCurrent(_oglVersion);
#ifdef _GR_IPHONE_BUILD
        // EAGL does not have pixel formats, so we need to get some of the details manually from OpenGL ES:
        initializePixelFormatFromOpenGLES();
#endif
        afterUpdatingContextDataSnapshot();

        // If multi textures are supported:
        if (_pTexturesMonitor != NULL)
        {
            if (_pTexturesMonitor->areMultiTexturesSupported())
            {
                // Get the value of GL_MAX_TEXTURE_UNITS / GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
#ifdef _GR_IPHONE_BUILD
                // In OpenGL ES 2.0, GL_MAX_TEXTURE_UNITS no longer exists.
                GLenum pname = _pTexturesMonitor->maxTextureUnitsSymbolicName();
                gs_stat_realFunctionPointers.glGetIntegerv(pname, &_maxTextureUnits);
#else
                gs_stat_realFunctionPointers.glGetIntegerv(GL_MAX_TEXTURE_UNITS, &_maxTextureUnits);
#endif
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
            }
        }

        // Allocate texture units monitors:
        int amountOfTextureUnits = amountOfGLSupportedTextureUnits();

        for (int i = 0; i < amountOfTextureUnits; i++)
        {
            // Allocate the current texture unit:
            int textureUnitName = GL_TEXTURE0 + i;
            gsTextureUnitMonitor* pCurTextureUnitMtr = new gsTextureUnitMonitor(textureUnitName);
            _textureUnitMonitors.push_back(pCurTextureUnitMtr);

            if (pCurTextureUnitMtr)
            {
                // Notify it on the texture made current event:
                pCurTextureUnitMtr->onFirstTimeContextMadeCurrent();
            }
            else
            {
                GT_ASSERT(0);
            }
        }

        if (_pVBOMonitor != NULL)
        {
            _pVBOMonitor->onFirstTimeContextMadeCurrent();
        }

        // Restore the calls history logger logging mode:
        if (pCallsHistoryLogger != NULL)
        {
            pCallsHistoryLogger->enableLogging(storedLoggingMode);
        }

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
        {
            // GPU affinities are currently supported only on Windows:
            if (gs_stat_extensionsManager.isExtensionSupported(_contextID._contextId, AP_WGL_NV_gpu_affinity))
            {
                BOOL resultCode = FALSE;

                gsMonitoredFunctionPointers* pContextRealFunctions = gs_stat_extensionsManager.currentRenderContextExtensionsRealImplPointers();
                GT_IF_WITH_ASSERT(pContextRealFunctions != NULL)
                {
                    // Verify that we have the function pointer:
                    if (pContextRealFunctions->wglEnumGpusFromAffinityDCNV == NULL)
                    {
                        gs_stat_extensionsManager.getExtensionPointerFromSystem(ap_wglEnumGpusFromAffinityDCNV);
                    }

                    // The extension is supported, no reason for there not to be a pointer to the function:
                    GT_IF_WITH_ASSERT(pContextRealFunctions->wglEnumGpusFromAffinityDCNV != NULL)
                    {
                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_wglEnumGpusFromAffinityDCNV);
                        bool goOn = true;
                        UINT gpuIndex = 0;
                        HGPUNV hGPU = NULL;

                        while (goOn)
                        {
                            resultCode = pContextRealFunctions->wglEnumGpusFromAffinityDCNV(_deviceContextOSHandle, gpuIndex, &hGPU);

                            if (resultCode == TRUE)
                            {
                                GT_IF_WITH_ASSERT(hGPU != NULL)
                                {
                                    // Note this GPU handle:
                                    _gpuAffinities.push_back((intptr_t)hGPU);
                                }

                                gpuIndex++;
                            }
                            else
                            {
                                // We need to know what caused this failure:
                                DWORD errCode = GetLastError();

                                if (errCode == ERROR_INVALID_HANDLE)
                                {
                                    // This is either a "regular" (not affinity) DC, or a bad DC handle (eg NULL, which would mean this is an offscreen context):
                                    // We push in a 0 to signify this is not an error:
                                    _gpuAffinities.push_back(0);
                                }
                                else if (errCode == ERROR_SUCCESS)
                                {
                                    // There was no error, meaning that the index was simply too large (ie we reached the last GPU supported by this DC):
                                    if (_gpuAffinities.size() == 0)
                                    {
                                        // We didn't find any GPUs, it appears that some NVidia driver versions do this by error when the DC is a "regular" DC:
                                        _gpuAffinities.push_back(0);
                                    }
                                }
                                else
                                {
                                    // There is a different problem, stop anyway:
                                    gtString errMsg;
                                    errMsg.appendFormattedString(L"Error determining GPU affinities for Context %d. Error code is %lu", _contextID._contextId, errCode);
                                    GT_ASSERT_EX(false, errMsg.asCharArray());
                                }

                                goOn = false;
                            }
                        }

                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_wglEnumGpusFromAffinityDCNV);
                    }
                }
            }
        }
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    }

    // Get a pointer to the glGetInteger64v function:
    _glGetInteger64v = (PFNGLGETINTEGER64VPROC)gsGetSystemsOGLModuleProcAddress("glGetInteger64v");

    // Get a pointer to the glGetIntegerui64vNV function:
    _glGetIntegerui64vNV = (PFNGLGETINTEGERUI64VNVPROC)gsGetSystemsOGLModuleProcAddress("glGetIntegerui64vNV");
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onContextRemovedFromBeingCurrent
// Description: Is called when the context is removed from being the calling
//              thread current context.
// Return Val: void
// Author:      Yaki Tebeka
// Date:        3/11/2005
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::onContextRemovedFromBeingCurrent()
{

}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onFrameTerminatorCall
// Description: Is called when a frame terminator is called (wglSwapBuffers, etc)
// Author:      Yaki Tebeka
// Date:        13/12/2005
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::onFrameTerminatorCall()
{
    // Notify relevant members:
    _renderPrimitivesStatisticsLogger.onFrameTerminatorCall();
    _performanceCountersManager.onFrameTerminatorCall();

    if (_shouldInitializePerformanceCounters)
    {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
#ifdef OA_DEBUGGER_USE_AMD_GPA
        {
            // Notify the ATI performance counters manager:
            if (_vendorType == OS_VENDOR_ATI)
            {
                _atiPerformanceCountersReader.onFrameTerminatorCall();
            }
        }
#endif
#endif
    }

    _forcedModesManager.onFrameTerminatorCall();

    // Some monitors are not relevant in Profile mode:
    apExecutionMode currentExecMode = suDebuggedProcessExecutionMode();

    if (currentExecMode != AP_PROFILING_MODE)
    {
        _pProgramsAndShadersMonitor->onFrameTerminatorCall();
    }

    // Call the base class implementation:
    suContextMonitor::onFrameTerminatorCall();
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onActiveTextureUnitChanged
// Description: Is called when the active texture unit changes.
// Arguments:   activeTextureUnit - The new active texture unit.
// Return Val:  bool - Success / failure. Failure occur when the input texture
//                     unit is not supported by the current context.
// Author:      Yaki Tebeka
// Date:        18/4/2005
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onActiveTextureUnitChanged(GLenum activeTextureUnit)
{
    // TO_DO: We should also have "onActiveClientTextureUnitChanged" - for glClientActiveTextureARB, etc.

    bool retVal = false;

    // Get the active texture unit index:
    int activeTexUnitIndex = textureUnitNameToIndex(activeTextureUnit);

    // Check texture unit index range:
    if ((0 <= activeTexUnitIndex) && (activeTexUnitIndex < amountOfTextureUnits()))
    {
        _activeTextureUnitIndex = activeTexUnitIndex;
        retVal = true;
    }
    else
    {
        // The user uses an invalid texture unit. Trigger an assertion failure:
        gtString errMessage = L"Error - Using an invalid texture unit: ";
        errMessage.appendFormattedString(L"0x%X while the amount of texture units is: %d",
                                         activeTextureUnit, amountOfTextureUnits());
        GT_ASSERT_EX(false, errMessage.asCharArray());
    }

    return retVal;
}

// TO_DO: GL_EXT_direct_state_access ask yaki if this function implementation is fine
// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onMultiTextureTargetBind
// Description: Handle the call to glBindMultiTextureEXT, which sets the active
//              texture, and bind a textre target.
// Arguments: GLenum texunit
//            GLenum target
//            GLuint textureName
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/9/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onMultiTextureTargetBind(GLenum texunit, GLenum target, GLuint textureName)
{
    bool retVal = false;

    // Get the active texture unit index:
    int activeTexUnitIndex = textureUnitNameToIndex(texunit);

    // Check texture unit index range:
    if ((0 <= activeTexUnitIndex) && (activeTexUnitIndex < amountOfTextureUnits()))
    {
        _activeTextureUnitIndex = activeTexUnitIndex;

        onTextureTargetBind(target, textureName);
        retVal = true;
    }
    else
    {
        // The user uses an invalid texture unit. Trigger an assertion failure:
        gtString errMessage = L"Error - Using an invalid texture unit: ";
        errMessage.appendFormattedString(L"0x%X while the amount of texture units is: %d", texunit, amountOfTextureUnits());
        GT_ASSERT_EX(false, errMessage.asCharArray());
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::activateRenderBuffer
// Description: Sets the currently active render buffer object
// Arguments: GLuint bufferName - the render buffer name
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/6/2008
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::activateRenderBuffer(GLuint bufferName)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRenderBuffersMonitor != NULL)
    {
        // If this is a bind command:
        if (bufferName == 0)
        {
            // Note that the renderbuffer was unbound:
            _activeRenderBuffer = bufferName;
            retVal = true;
        }
        else // bufferName != 0
        {
            // Get the render buffer object according to the name:
            apGLRenderBuffer* pRenderBufferObj = _pRenderBuffersMonitor->getRenderBufferObjectDetails(bufferName);

            if (pRenderBufferObj == NULL)
            {
                // If the object was not generated yet, make sure it is now generated:
                _pRenderBuffersMonitor->onRenderBufferObjectsGeneration(1, &bufferName);

                // Get the generated buffer pointer:
                pRenderBufferObj = _pRenderBuffersMonitor->getRenderBufferObjectDetails(bufferName);
            }

            if (pRenderBufferObj != NULL)
            {
                // Found the render buffer object:
                _activeRenderBuffer = bufferName;
                retVal = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::getActiveRenderBufferObject
// Description: Returns the currently active render buffer object
// Return Val: GLuint
// Author:      Sigal Algranaty
// Date:        1/6/2008
// ---------------------------------------------------------------------------
GLuint gsRenderContextMonitor::getActiveRenderBufferObject()
{
    return _activeRenderBuffer;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::setActiveRenderBufferObjectParameters
// Description: Sets the active render buffer parameters
// Arguments: GLenum internalformat
//            GLsizei width
//            GLsizei height
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/6/2008
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::setActiveRenderBufferObjectParameters(GLenum internalformat, GLsizei width, GLsizei height)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRenderBuffersMonitor != NULL)
    {
        // Get the render buffer object according to the name:
        apGLRenderBuffer* pRenderBufferObj = _pRenderBuffersMonitor->getRenderBufferObjectDetails(_activeRenderBuffer);
        GT_IF_WITH_ASSERT(pRenderBufferObj != NULL)
        {
            // Set the buffer dimensions:
            pRenderBufferObj->setBufferDimensions(width, height);

            // Translate the GL enum to oaTexelDataFormat:
            oaTexelDataFormat bufferDataFormat = OA_TEXEL_FORMAT_UNKNOWN;
            bool rc = oaGLEnumToTexelDataFormat(internalformat, bufferDataFormat);
            GT_IF_WITH_ASSERT(rc)
            {
                pRenderBufferObj->setBufferDataFormat(bufferDataFormat);
                retVal = true;
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::setRenderBufferObjectParameters
// Description: Sets a named render buffer parameters
// Arguments: GLuint buffer - buffer name
//            GLenum internalformat
//            GLsizei width
//            GLsizei height
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        16/9/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::setRenderBufferObjectParameters(GLuint buffer, GLenum internalformat, GLsizei width, GLsizei height)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pRenderBuffersMonitor != NULL)
    {
        // Get the render buffer object according to the name:
        apGLRenderBuffer* pRenderBufferObj = _pRenderBuffersMonitor->getRenderBufferObjectDetails(buffer);

        if (pRenderBufferObj != NULL)
        {
            // Set the buffer dimensions:
            pRenderBufferObj->setBufferDimensions(width, height);

            // Translate the GL enum to oaTexelDataFormat:
            oaTexelDataFormat bufferDataFormat = OA_TEXEL_FORMAT_UNKNOWN;
            bool rc = oaGLEnumToTexelDataFormat(internalformat, bufferDataFormat);
            GT_IF_WITH_ASSERT(rc)
            {
                pRenderBufferObj->setBufferDataFormat(bufferDataFormat);
                retVal = true;
            }
        }
        else
        {
            // TO_DO: GL_EXT_direct_state_access
            // Add detected error
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureTargetBind
// Description: Is called when a texture object is bound to a texture target.
// Arguments:   target - The bind target.
//              textureName - The name of the bound texture.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        23/12/2004
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::onTextureTargetBind(GLenum target, GLuint textureName)
{
    // Get the texture type:
    apTextureType textureType = apTextureBindTargetToTextureType(target);

    // If a default texture is bound:
    if (textureName == 0)
    {
        // Get the default texture name:
        GLenum  textureUnitName = textureUnitIndexToName(_activeTextureUnitIndex);
        GLuint defaultTextureName = 0;
        bool rc = apGetDefaultTextureName(textureUnitName , textureType, defaultTextureName);

        if (rc)
        {
            // Try to get the texture object monitor index:
            int monitorObjIndex = _pTexturesMonitor->textureObjMonitorIndex(defaultTextureName);

            // If this default texture has a representing object:
            // (I.E: It was used by the debugged program)
            if (monitorObjIndex != -1)
            {
                // Use the default texture object "name" instead of 0:
                textureName = defaultTextureName;
            }
        }
    }
    else
    {
        // Check if this texture object exists, and if it doesn't, add it to the texture monitor:
        apGLTexture* pTextureObject = _pTexturesMonitor->getTextureObjectDetails(textureName);

        if (pTextureObject == NULL)
        {
            GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
            {
                _pTexturesMonitor->onTextureObjectsGeneration(1, &textureName);
            }
        }

        // Set the texture type:
        GT_IF_WITH_ASSERT(pTextureObject != NULL)
        {
            pTextureObject->setTextureType(textureType);
        }
    }

    // Log the texture bind:
    _textureUnitMonitors[_activeTextureUnitIndex]->onTextureTargetBind(target, textureName);
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureTargetBindToUnit
// Description: Is called when a texture object is bound to a texture target on a unit (direct state access).
// Arguments:   textureUnit - The texture unit name
//              target - The bind target.
//              textureName - The name of the bound texture.
// Return Val: bool - Success / failure.
// Author:      Uri Shomroni
// Date:        21/6/2015
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::onTextureTargetBindToUnit(GLenum textureUnit, GLenum target, GLuint textureName)
{
    // Get the texture unit index:
    int unitIndex = textureUnitNameToIndex(textureUnit);

    GT_IF_WITH_ASSERT((0 <= unitIndex) && (amountOfTextureUnits() > unitIndex))
    {
        gsTextureUnitMonitor* pTexUnitMonitor = _textureUnitMonitors[unitIndex];

        // Setting the NULL texture target to GL_NONE signifies unbinding all textures:
        if ((0 == textureName) && (GL_NONE == target))
        {
            pTexUnitMonitor->unbindAllTextures();
        }
        else
        {
            pTexUnitMonitor->onTextureTargetBind(target, textureName);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureTargetBindToUnit
// Description: Is called when a texture object is bound to a unit (direct state access).
//              Texture object has to have a target already defined.
// Arguments:   textureUnit - The texture unit name
//              textureName - The name of the bound texture.
// Return Val: bool - Success / failure.
// Author:      Uri Shomroni
// Date:        21/6/2015
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::onTextureBindToUnit(GLenum textureUnit, GLuint textureName)
{
    // If the texture name is 0, unbind all textures:
    if (0 == textureName)
    {
        onTextureTargetBindToUnit(textureUnit, GL_NONE, textureName);
    }
    else
    {
        // Get the object:
        gsGLTexture* pTextureObject = _pTexturesMonitor->getTextureObjectDetails(textureName);

        if (nullptr != pTextureObject)
        {
            // Get the target:
            GLenum target = apTextureTypeToTextureBindTarget(pTextureObject->textureType());

            // Note the binding:
            onTextureTargetBindToUnit(textureUnit, target, textureName);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureMipmapGenerate
// Description: Is called after mipmap auto generation is called in OpenGL
// Arguments: GLenum bindTarget
// Return Val: void
// Author:      Sigal Algranaty
// Date:        16/9/2009
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::onTextureMipmapGenerate(GLenum bindTarget)
{
    // Ignore texture proxies:
    bool isProxyTexture = apIsProxyTextureBindTarget(bindTarget);

    if (!isProxyTexture)
    {
        GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
        {
            // Get the object that represents the bound texture:
            gsGLTexture* pTextureObj = getBoundTexture(_activeTextureUnitIndex, bindTarget);
            GT_IF_WITH_ASSERT(pTextureObj != NULL)
            {
                _pTexturesMonitor->onTextureMipmapGenerate(pTextureObj);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureMipmapGenerate
// Description: Is called after mipmap auto generation is called in OpenGL (direct access version)
// Arguments:   GLuint texture - the texture id
//              GLenum bindTarget
// Return Val: void
// Author:      Sigal Algranaty
// Date:        18/1/2009
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::onNamedTextureMipmapGenerate(GLuint texture)
{
    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
    {
        // Get the object that represents the bound texture:
        gsGLTexture* pTextureObj = _pTexturesMonitor->getTextureObjectDetails(texture);
        GT_IF_WITH_ASSERT(pTextureObj != NULL)
        {
            _pTexturesMonitor->onTextureMipmapGenerate(pTextureObj);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onMultiTextureMipmapGenerate
// Description: Is called after mipmap auto generation is called in OpenGL (direct access version)
// Arguments:   GLenum texunit - the texture unit
//              GLenum bindTarget
// Return Val: void
// Author:      Sigal Algranaty
// Date:        18/1/2009
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::onMultiTextureMipmapGenerate(GLenum texunit, GLenum bindTarget)
{
    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
    {
        // Get the requested texture unit index:
        int textureUnitIndex = textureUnitNameToIndex(texunit);

        // Check texture unit index range:
        if ((0 <= textureUnitIndex) && (textureUnitIndex < amountOfTextureUnits()))
        {
            // Get the bound texture object:
            gsGLTexture* pTextureObject = getBoundTexture(textureUnitIndex, bindTarget);

            if (pTextureObject != NULL)
            {
                _pTexturesMonitor->onTextureMipmapGenerate(pTextureObject);
            }
            else
            {
                // TO_DO: GL_EXT_direct_state_access
                // Add detected error
            }
        }
    }
}



// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureImageLoaded
// Description: Is called after texture data is loaded into OpenGL.
// Arguments:   bindTarget - The target texture bind.
//              level - The loaded LOD (level-of-detail) level number.
//              internalformat - A request for a format that will be used to stored
//                               the texture data on the graphics server.
//              width, height, depth - The texture dimensions.
//              border - The texture border width.
//              format - The format of the input texture data.
//                       I.E: How are the input texels arranged (GL_RGB, GL_RGBA, etc).
//              type - The type of the input texture texels (GL_BYTE, GL_SHORT, etc).
// Return Val: bool  - Success / failure. We fail when there is no textures monitor
//                     or the target has no bound texture.
// Author:      Yaki Tebeka
// Date:        25/12/2004
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onTextureImageLoaded(GLenum bindTarget, GLint level, GLint internalformat,
                                                  GLsizei width, GLsizei height, GLsizei depth, GLint border,
                                                  GLenum format, GLenum type)
{
    bool retVal = false;

    // Ignore texture proxies:
    bool isProxyTexture = apIsProxyTextureBindTarget(bindTarget);

    if (!isProxyTexture)
    {
        GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
        {
            // Get the object that represents the bound texture:
            gsGLTexture* pTextureObj = getBoundTexture(_activeTextureUnitIndex, bindTarget);
            GT_IF_WITH_ASSERT(pTextureObj != NULL)
            {
                retVal = onNamedTextureImageLoaded(pTextureObj->textureName(), bindTarget, level, internalformat, width, height, depth, border, format, type);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureImageLoaded
// Description: Is called after texture data is loaded into OpenGL.
// Arguments:   texture - the texture id
//              bindTarget - The target texture bind.
//              level - The loaded LOD (level-of-detail) level number.
//              internalformat - A request for a format that will be used to stored
//                               the texture data on the graphics server.
//              width, height, depth - The texture dimensions.
//              border - The texture border width.
//              format - The format of the input texture data.
//                       I.E: How are the input texels arranged (GL_RGB, GL_RGBA, etc).
//              type - The type of the input texture texels (GL_BYTE, GL_SHORT, etc).
// Return Val: bool  - Success / failure. We fail when there is no textures monitor
//             or the target has no bound texture.
// Author:      Sigal Algranaty
// Date:        13/9/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onNamedTextureImageLoaded(GLuint texture, GLenum bindTarget, GLint level, GLint internalformat,
                                                       GLsizei width, GLsizei height, GLsizei depth, GLint border,
                                                       GLenum format, GLenum type)
{
    bool retVal = false;

    // Sanity check:
    if (_pTexturesMonitor != NULL)
    {
        // Get the texture monitor object that represents the bind texture name:
        gsGLTexture* pTextureObject = _pTexturesMonitor->getTextureObjectDetails(texture);
        GT_IF_WITH_ASSERT(pTextureObject != NULL)
        {
            retVal = onTextureImageLoaded(pTextureObject, bindTarget, level, internalformat, width, height, depth, border, format, type);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onNamedTextureImageLoaded
// Description: Is called after texture data is loaded into OpenGL for an existing texture.
// Arguments:   texture - the texture id
//              level - The loaded LOD (level-of-detail) level number.
//              format - The format of the input texture data.
//                       I.E: How are the input texels arranged (GL_RGB, GL_RGBA, etc).
// Return Val: bool  - Success / failure. We fail when there is no textures monitor
//             or the target has no bound texture.
// Author:      Uri Shomroni
// Date:        21/6/2015
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onNamedTextureImageLoaded(GLuint texture, GLint level, GLenum format)
{
    bool retVal = false;

    // Sanity check:
    if (_pTexturesMonitor != NULL)
    {
        // Get the texture monitor object that represents the bind texture name:
        gsGLTexture* pTextureObject = _pTexturesMonitor->getTextureObjectDetails(texture);
        GT_IF_WITH_ASSERT(pTextureObject != NULL)
        {
            // Texture has to already have been bound to a target:
            GLenum target = apTextureTypeToTextureBindTarget(pTextureObject->textureType());

            if (GL_NONE != target)
            {
                GLsizei width = 0;
                GLsizei height = 0;
                GLsizei depth = 0;
                GLsizei border = 0;
                pTextureObject->getDimensions(width, height, depth, border, level);

                retVal = onNamedTextureImageLoaded(texture, target, level, pTextureObject->requestedInternalPixelFormat(), width, height, depth, border, format, GL_NONE);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureStorageSet
// Description: Called when texture storage is reserved.
//              This makes the storage immutable
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        22/7/2015
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onTextureStorageSet(GLenum bindTarget, GLsizei levels, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    bool retVal = false;

    // Ignore texture proxies:
    bool isProxyTexture = apIsProxyTextureBindTarget(bindTarget);

    if (!isProxyTexture)
    {
        // Get the object that represents the bound texture:
        gsGLTexture* pTextureObj = getBoundTexture(_activeTextureUnitIndex, bindTarget);

        if (nullptr != pTextureObj)
        {
            retVal = onNamedTextureStorageSet(pTextureObj->textureName(), bindTarget, levels, internalformat, width, height, depth);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onNamedTextureStorageSet
// Description: Called when a texture storage is reserved using DSA.
//              This makes the storage immutable.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        22/7/2015
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onNamedTextureStorageSet(GLuint texture, GLsizei levels, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(nullptr != _pTexturesMonitor)
    {
        // Get the object that represents the bound texture:
        gsGLTexture* pTextureObj = _pTexturesMonitor->getTextureObjectDetails(texture);

        if (nullptr != pTextureObj)
        {
            GLenum bindTarget = apTextureTypeToTextureBindTarget(pTextureObj->textureType());
            retVal = onNamedTextureStorageSet(texture, bindTarget, levels, internalformat, width, height, depth);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onNamedTextureStorageSet
// Description: Helper function called by the other two on*TextureStorageSet
//              Uses onNamedTextureImageLoaded to apply the changes internally
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        22/7/2015
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onNamedTextureStorageSet(GLuint texture, GLenum bindTarget, GLsizei levels, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    bool retVal = (levels > 0);

    GLsizei currWidth = width;
    GLsizei currHeight = height;
    GLsizei currDepth = depth;
    static const GLenum cubeFaces[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z };
    bool isCube = (GL_TEXTURE_CUBE_MAP == bindTarget);
    const GLenum* pFirstTarget = (isCube ? cubeFaces : &bindTarget);
    int numTargets = (isCube ? 6 : 1);

    for (GLsizei i = 0; i < levels; ++i)
    {
        for (int j = 0; j < numTargets; ++j)
        {
            onNamedTextureImageLoaded(texture, pFirstTarget[j], i, internalformat, currWidth, currHeight, currDepth, 0, GL_NONE, GL_NONE);
        }

        // We want to preserve values of 0 and 1:
        if (currWidth > 1)
        {
            currWidth /= 2;
        }

        if (currHeight > 1)
        {
            currHeight /= 2;
        }

        if (currDepth > 1)
        {
            currDepth /= 2;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onMultiTextureImageLoaded
// Description: Is called after texture data is loaded into OpenGL for a texture unit.
//              The function get the texture unit and texture bind target
// Arguments: GLenum texUnit
//            GLenum bindTarget
//            GLint level
//            GLint internalformat
//            GLsizei width
//            GLsizei height
//            GLsizei depth
//            GLint border
//            GLenum format
//            GLenum type
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/9/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onMultiTextureImageLoaded(GLenum texUnit, GLenum bindTarget, GLint level, GLint internalformat,
                                                       GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type)
{
    bool retVal = true;

    // Get the requested texture unit index:
    int activeTexUnitIndex = textureUnitNameToIndex(texUnit);

    // Check texture unit index range:
    if ((0 <= activeTexUnitIndex) && (activeTexUnitIndex < amountOfTextureUnits()))
    {
        // Get the bound texture object:
        gsGLTexture* pTextureObject = getBoundTexture(activeTexUnitIndex, bindTarget);

        if (pTextureObject != NULL)
        {
            // If the appropriate mip-map level does not exist:
            apGLTextureMipLevel* pTextureMipLevel = pTextureObject->getTextureMipLevel(level);

            if (pTextureMipLevel == NULL)
            {
                // Add the texture mip-map level:
                bool rc1 = pTextureObject->addTextureMipLevel(level, internalformat, width, height, depth, border, format, type);
                GT_IF_WITH_ASSERT(rc1)
                {
                    pTextureMipLevel = pTextureObject->getTextureMipLevel(level);
                }
            }

            // Mark the texture image as dirty:
            pTextureObject->markTextureAsDirty(bindTarget, level);

            GT_IF_WITH_ASSERT(pTextureMipLevel != NULL)
            {
                // If this is mip-map level 0:
                if (level == 0)
                {
                    // Store the texture formats:
                    pTextureObject->setTextureFormats(bindTarget, internalformat, format, type);
                    _pTexturesMonitor->onTextureImageLoaded(pTextureObject, width, height, depth, border, format, type);
                }
            }

            // Make sure all the texture's mip levels are registered in the allocated objects manager:
            pTextureObject->setNewMipLevelsAllocIds();
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureImageLoaded
// Description: Is called after texture data is loaded into OpenGL. The function already
//              get the texture object pointer, and is called from the texture image load
//              functions (both from the direct access function and the none direct access functions)
// Arguments: gsGLTexture* pTextureObject
//            GLenum bindTarget
//            GLint level
//            GLint internalformat
//            GLsizei width
//            GLsizei height
//            GLsizei depth
//            GLint border
//            GLenum format
//            GLenum type
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/9/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onTextureImageLoaded(gsGLTexture* pTextureObject, GLenum bindTarget, GLint level, GLint internalformat,
                                                  GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type)
{
    bool retVal = true;

    // If the appropriate mip-map level does not exist:
    apGLTextureMipLevel* pTextureMipLevel = pTextureObject->getTextureMipLevel(level);

    if (pTextureMipLevel == NULL)
    {
        // Add the texture mip-map level:
        bool rc1 = pTextureObject->addTextureMipLevel(level, internalformat, width, height, depth, border, format, type);
        GT_IF_WITH_ASSERT(rc1)
        {
            pTextureMipLevel = pTextureObject->getTextureMipLevel(level);
        }
    }

    // Mark the texture image as dirty:
    pTextureObject->markTextureAsDirty(bindTarget, level);

    GT_IF_WITH_ASSERT(pTextureMipLevel != NULL)
    {
        // If this is mip-map level 0:
        if (level == 0)
        {
            // Store the texture formats:
            pTextureObject->setTextureFormats(bindTarget, internalformat, format, type);

            // Handle texture image load operation in texture monitor:
            _pTexturesMonitor->onTextureImageLoaded(pTextureObject, width, height, depth, border, format, type);
        }
    }

    // Make sure all the texture's mip levels are registered in the allocated objects manager:
    pTextureObject->setNewMipLevelsAllocIds();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureBuffer
// Description: Is called when a texture buffer is attached.
// Arguments:   bindTarget - The target texture bind (should be GL_TEXTURE_BUFFER
//              otherwise - it is a detected error)
//              internalformat - The buffer internal format
//              buffer - the OpenGL buffer name
// Author:      Sigal Algranaty
// Date:        3/8/2009
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::onTextureBuffer(GLenum target, GLenum internalformat, GLuint buffer)
{
    // Ignore texture proxies:
    bool isBufferTexture = (target == GL_TEXTURE_BUFFER);

    if (isBufferTexture)
    {
        GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
        {
            // Get the object that represents the bound texture:
            gsGLTexture* pTextureObj = getBoundTexture(_activeTextureUnitIndex, target);
            GT_IF_WITH_ASSERT(pTextureObj != NULL)
            {
                // Set the texture buffer:
                pTextureObj->setBufferName(buffer);

                // Mark the texture as dirty:
                pTextureObj->markTextureAsDirty(target);

                // Call the actual handling of texture buffer:
                onTextureBuffer(pTextureObj, internalformat, buffer);
            }
        }
    }
    else
    {
        // TO_DO: OpenGL3.1 add detected error
    }
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureBuffer
// Description:
// Arguments: GLuint texture
//            GLenum target
//            GLenum internalformat
//            GLuint buffer
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/9/2009
void gsRenderContextMonitor::onTextureBuffer(GLuint texture, GLenum target, GLenum internalformat, GLuint buffer)
{
    // Ignore texture proxies:
    bool isBufferTexture = (target == GL_TEXTURE_BUFFER);

    if (isBufferTexture)
    {
        GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
        {
            // Get the object that represents the texture:
            gsGLTexture* pTextureObj = _pTexturesMonitor->getTextureObjectDetails(texture);
            GT_IF_WITH_ASSERT(pTextureObj != NULL)
            {
                // Mark the texture as dirty:
                pTextureObj->markTextureAsDirty(target);

                onTextureBuffer(pTextureObj, internalformat, buffer);
            }
        }
    }
    else
    {
        // TO_DO: OpenGL3.1 add detected error
    }
}



// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onMultiTextureBuffer
// Description: Perform texture buffer operation for multi texture
// Arguments: GLenum texunit
//            GLenum target
//            GLenum internalformat
//            GLuint buffer
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/9/2009
void gsRenderContextMonitor::onMultiTextureBuffer(GLenum texunit, GLenum target, GLenum internalformat, GLuint buffer)
{
    // Ignore texture proxies:
    bool isBufferTexture = (target == GL_TEXTURE_BUFFER);

    if (isBufferTexture)
    {
        GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
        {
            // Get the requested texture unit index:
            int texUnitIndex = textureUnitNameToIndex(texunit);

            // Check texture unit index range:
            if ((0 <= texUnitIndex) && (texUnitIndex < amountOfTextureUnits()))
            {
                // Get the bound texture object:
                gsGLTexture* pTextureObject = getBoundTexture(texUnitIndex, target);

                if (pTextureObject != NULL)
                {
                    // Mark the texture as dirty:
                    pTextureObject->markTextureAsDirty(target);

                    onTextureBuffer(pTextureObject, internalformat, buffer);
                }
            }
        }
    }
    else
    {
        // TO_DO: OpenGL3.1 add detected error
    }
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureBuffer
// Description: Handles texture buffer operation on a texture object
// Arguments: gsGLTexture* pTextureObject
//            GLenum internalformat
//            GLuint buffer
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/9/2009
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::onTextureBuffer(gsGLTexture* pTextureObject, GLenum internalformat, GLuint buffer)
{
    GT_IF_WITH_ASSERT(pTextureObject != NULL)
    {
        // Set the texture internal format:
        pTextureObject->setBufferInternalFormat(internalformat);

        if (buffer != 0)
        {
            // Get the VBO object for this buffer:
            apGLVBO* pVBO = _pVBOMonitor->getVBODetails(buffer);

            if (pVBO != NULL)
            {
                // Get the relevant VBO format for this texture buffer internal format:
                oaTexelDataFormat vboFormat = OA_TEXEL_FORMAT_UNKNOWN;
                bool rc = apGetVBOFormatFromTextureBufferFormat(internalformat, vboFormat);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Set the VBO format according to the texture buffer format:
                    pVBO->setBufferDisplayFormat(vboFormat);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureSubImageLoaded
// Description: Is called after sub texture data is loaded into OpenGL.
// Arguments:   bindTarget - The target texture bind.
//              level - The loaded LOD (level-of-detail) level number.
// Return Val: bool  - Success / failure. We fail when there is no textures monitor
//                     or the target has no bound texture.
// Author:      Yaki Tebeka
// Date:        20/1/2005
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onTextureSubImageLoaded(GLenum bindTarget, GLint level)
{
    bool retVal = false;

    // If we have a textures monitor:
    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
    {
        // Look for the param object that represents the texture that is bind to the input
        // bind target:
        gsGLTexture* pTextureObj = getBoundTexture(_activeTextureUnitIndex, bindTarget);

        if (pTextureObj != NULL)
        {
            retVal = true;
            pTextureObj->markTextureAsDirty(bindTarget, level);
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureSubImageLoaded
// Description: Is called after sub texture data is loaded into OpenGL. (direct state access)
// Arguments:   texture - the texture OpenGL name
//              bindTarget - The target texture bind.
//              level - The loaded LOD (level-of-detail) level number.
// Return Val: bool  - Success / failure. We fail when there is no textures monitor
//                     or the target has no bound texture.
// Author:      Sigal Algranaty
// Date:        13/9/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onTextureSubImageLoaded(GLuint texture, GLenum bindTarget, GLint level)
{
    bool retVal = false;

    // If we have a textures monitor:
    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
    {
        // Look for the param object that represents the texture that is bind to the input
        // bind target:
        gsGLTexture* pTextureObj = _pTexturesMonitor->getTextureObjectDetails(texture);

        if (pTextureObj != NULL)
        {
            retVal = true;
            pTextureObj->markTextureAsDirty(bindTarget, level);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onNamedTextureSubImageLoaded
// Description: Is called after sub texture data is loaded into OpenGL. (direct state access)
// Arguments:   texture - the texture OpenGL name
//              level - The loaded LOD (level-of-detail) level number.
// Return Val: bool  - Success / failure. We fail when there is no textures monitor
//                     or the target has no bound texture.
// Author:      Uri Shomroni
// Date:        21/6/2015
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onNamedTextureSubImageLoaded(GLuint texture, GLint level)
{
    bool retVal = false;

    // If we have a textures monitor:
    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
    {
        // Look for the param object that represents the texture that is bind to the input
        // bind target:
        gsGLTexture* pTextureObj = _pTexturesMonitor->getTextureObjectDetails(texture);

        if (pTextureObj != NULL)
        {
            // Target has to already be defined:
            GLenum bindTarget = apTextureTypeToTextureBindTarget(pTextureObj->textureType());

            if (GL_NONE != bindTarget)
            {
                retVal = true;
                pTextureObj->markTextureAsDirty(bindTarget, level);
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onMultiTextureSubImageLoaded
// Description: Is called after sub multi texture data is loaded into OpenGL. (direct state access)
// Arguments: GLenum texunit
//            GLenum bindTarget
//            GLint level
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/9/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onMultiTextureSubImageLoaded(GLenum texunit, GLenum bindTarget, GLint level)
{
    bool retVal = false;

    // If we have a textures monitor:
    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
    {
        // Get the requested texture unit index:
        int activeTexUnitIndex = textureUnitNameToIndex(texunit);

        // Check texture unit index range:
        if ((0 <= activeTexUnitIndex) && (activeTexUnitIndex < amountOfTextureUnits()))
        {
            // Get the bound texture object:
            apGLTexture* pTextureObject = getBoundTexture(activeTexUnitIndex, bindTarget);

            if (pTextureObject != NULL)
            {
                retVal = true;
                pTextureObject->markTextureAsDirty(bindTarget, level);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureIntParameterChanged
// Description: Is called when an int texture parameter is changed.
// Arguments:   target - The target texture bind.
//              pname - The OpenGL name of the changed parameter.
//              pParam - Pointer to the new parameter value.
// Return Val: bool  - Success / failure. We fail when there is no textures monitor
//                     or the target has no bound texture.
// Author:      Yaki Tebeka
// Date:        20/1/2005
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onTextureIntParameterChanged(GLenum target, GLenum pname, const GLint* pParam)
{
    bool retVal = false;

    // If the we have a textures monitor:
    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
    {
        // Get the param object that represents the bind texture:
        gsGLTexture* pTexParamObj = getBoundTexture(_activeTextureUnitIndex, target);

        if (pTexParamObj)
        {
            // Update the texture's parameter:
            apGLTextureParams& params = pTexParamObj->textureParameters();
            params.setTextureParameterIntValue(pname, pParam);

            retVal = true;
        }
        else
        {
            // An error occur:
            // Yaki - I removed the below assert, since it made NVSG stuck.
            // gtAssert(0);
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureUIntParameterChanged
// Description: Is called when an uint texture parameter is changed.
// Arguments:   target - The target texture bind.
//              pname - The OpenGL name of the changed parameter.
//              pParam - Pointer to the new parameter value.
// Return Val: bool  - Success / failure. We fail when there is no textures monitor
//                     or the target has no bound texture.
// Author:      Yaki Tebeka
// Date:        15/9/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onNamedTextureUIntParameterChanged(GLuint texture, GLenum pname, const GLuint* pParam)
{
    bool retVal = false;

    // If the we have a textures monitor:
    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
    {
        // Get the param object that represents the bind texture:
        gsGLTexture* pTexParamObj = _pTexturesMonitor->getTextureObjectDetails(texture);

        if (pTexParamObj)
        {
            // Update the texture's parameter:
            apGLTextureParams& params = pTexParamObj->textureParameters();
            params.setTextureParameterUIntValue(pname, pParam);

            retVal = true;
        }
        else
        {
            // An error occur:
            // Yaki - I removed the below assert, since it made NVSG stuck.
            // gtAssert(0);
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureIntParameterChanged
// Description: Is called when an int texture parameter is changed
//              (on direct state access extension functions)
// Arguments: GLuint texture
//            GLenum target
//            GLenum pname
//            const GLint* pParam
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/9/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onNamedTextureIntParameterChanged(GLuint texture, GLenum pname, const GLint* pParam)
{
    bool retVal = false;

    // If the we have a textures monitor:
    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
    {
        // Get the param object that represents the bind texture:
        gsGLTexture* pTexParamObj = _pTexturesMonitor->getTextureObjectDetails(texture);

        if (pTexParamObj)
        {
            // Update the texture's parameter:
            apGLTextureParams& params = pTexParamObj->textureParameters();
            params.setTextureParameterIntValue(pname, pParam);

            retVal = true;
        }
        else
        {
            // An error occur:
            // Yaki - I removed the below assert, since it made NVSG stuck.
            // gtAssert(0);
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onMultiTextureIntParameterChanged
// Description: Is called when an int texture parameter is changed
//              for multi texture unit.
// Arguments: GLenum texunit
//            GLenum target
//            GLenum pname
//            const GLint* pParam
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/9/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onMultiTextureIntParameterChanged(GLenum texunit, GLenum target, GLenum pname, const GLint* pParam)
{
    bool retVal = false;

    // If the we have a textures monitor:
    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
    {
        // Get the requested texture unit index:
        int activeTexUnitIndex = textureUnitNameToIndex(texunit);

        // Check texture unit index range:
        if ((0 <= activeTexUnitIndex) && (activeTexUnitIndex < amountOfTextureUnits()))
        {
            // Get the bound texture object:
            gsGLTexture* pTextureObject = getBoundTexture(activeTexUnitIndex, target);

            if (pTextureObject)
            {
                // Update the texture's parameter:
                apGLTextureParams& params = pTextureObject->textureParameters();
                params.setTextureParameterIntValue(pname, pParam);

                retVal = true;
            }
            else
            {
                // An error occur:
                // Yaki - I removed the below assert, since it made NVSG stuck.
                // gtAssert(0);
            }
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onMultiTextureUIntParameterChanged
// Description: Is called when an uint texture parameter is changed
//              for multi texture unit.
// Arguments: GLenum texunit
//            GLenum target
//            GLenum pname
//            const GLint* pParam
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/9/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onMultiTextureUIntParameterChanged(GLenum texunit, GLenum target, GLenum pname, const GLuint* pParam)
{
    bool retVal = false;

    // If the we have a textures monitor:
    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
    {
        // Get the requested texture unit index:
        int activeTexUnitIndex = textureUnitNameToIndex(texunit);

        // Check texture unit index range:
        if ((0 <= activeTexUnitIndex) && (activeTexUnitIndex < amountOfTextureUnits()))
        {
            // Get the bound texture object:
            gsGLTexture* pTextureObject = getBoundTexture(activeTexUnitIndex, target);

            if (pTextureObject)
            {
                // Update the texture's parameter:
                apGLTextureParams& params = pTextureObject->textureParameters();
                params.setTextureParameterUIntValue(pname, pParam);

                retVal = true;
            }
            else
            {
                // An error occur:
                // Yaki - I removed the below assert, since it made NVSG stuck.
                // gtAssert(0);
            }
        }
    }
    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureFloatParameterChanged
// Description: Is called when a float texture parameter is changed.
// Arguments:   target - The target texture bind.
//              pname - The OpenGL name of the changed parameter.
//              pParam - Pointer to the new parameter value.
// Return Val: bool  - Success / failure. We fail when there is no textures monitor
//                     or the target has no bound texture.
// Author:      Yaki Tebeka
// Date:        20/1/2005
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onTextureFloatParameterChanged(GLenum target, GLenum pname, const GLfloat* pParam)
{
    bool retVal = false;

    // If the we have a textures monitor:
    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
    {
        // Get the param object that represents the bind texture:
        gsGLTexture* pTexParamObj = getBoundTexture(_activeTextureUnitIndex, target);

        if (pTexParamObj)
        {
            apGLTextureParams& params = pTexParamObj->textureParameters();
            params.setTextureParameterFloatValue(pname, pParam);

            retVal = true;
        }
        else
        {
            // An error occur:
            // Yaki - I removed the below assert, since it made NVSG stuck.
            // gtAssert(0);
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureFloatParameterChanged
// Description: Is called when a float texture parameter is changed.
// Arguments:   target - The target texture bind.
//              level - the texture level
//              pname - The OpenGL name of the changed parameter.
//              pParam - Pointer to the new parameter value.
// Return Val: bool  - Success / failure. We fail when there is no textures monitor
//                     or the target has no bound texture.
// Author:      Yaki Tebeka
// Date:        20/1/2005
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onTextureLevelFloatParameterChanged(GLenum target, GLint level, GLenum pname, const GLfloat* pParam)
{
    bool retVal = false;

    // If the we have a textures monitor:
    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
    {
        // Get the param object that represents the bind texture:
        gsGLTexture* pTexParamObj = getBoundTexture(_activeTextureUnitIndex, target);

        if (pTexParamObj)
        {
            apGLTextureParams* pTextureLevelParams = pTexParamObj->textureLevelParameters(level);

            if (pTextureLevelParams != NULL)
            {
                pTextureLevelParams->setTextureParameterFloatValue(pname, pParam);
                retVal = true;
            }
        }
        else
        {
            // An error occur:
            // Yaki - I removed the below assert, since it made NVSG stuck.
            // gtAssert(0);
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onNamedTextureFloatParameterChanged
// Description: Is called when a float texture parameter is changed by direct state access.
// Arguments:   texture - The texture name.
//              pname - The OpenGL name of the changed parameter.
//              pParam - Pointer to the new parameter value.
// Return Val: bool  - Success / failure. We fail when there is no textures monitor
//                     or the target has no bound texture.
// Author:      Uri Shomroni
// Date:        21/06/2015
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onNamedTextureFloatParameterChanged(GLuint texture, GLenum pname, const GLfloat* pParam)
{
    bool retVal = false;

    // If the we have a textures monitor:
    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
    {
        // Get the param object that represents the bind texture:
        gsGLTexture* pTexParamObj = _pTexturesMonitor->getTextureObjectDetails(texture);

        if (pTexParamObj)
        {
            apGLTextureParams& params = pTexParamObj->textureParameters();
            params.setTextureParameterFloatValue(pname, pParam);

            retVal = true;
        }
        else
        {
            // An error occur:
            // Yaki - I removed the below assert, since it made NVSG stuck.
            // gtAssert(0);
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onTextureFloatParameterChanged
// Description: Is called when a float texture parameter is changed by direct state access.
// Arguments:   texture - The texture name.
//              level - the texture level
//              pname - The OpenGL name of the changed parameter.
//              pParam - Pointer to the new parameter value.
// Return Val: bool  - Success / failure. We fail when there is no textures monitor
//                     or the target has no bound texture.
// Author:      Uri Shomroni
// Date:        21/06/2015
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onNamedTextureLevelFloatParameterChanged(GLuint texture, GLint level, GLenum pname, const GLfloat* pParam)
{
    bool retVal = false;

    // If the we have a textures monitor:
    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
    {
        // Get the param object that represents the bind texture:
        gsGLTexture* pTexParamObj = _pTexturesMonitor->getTextureObjectDetails(texture);

        if (pTexParamObj)
        {
            apGLTextureParams* pTextureLevelParams = pTexParamObj->textureLevelParameters(level);

            if (pTextureLevelParams != NULL)
            {
                pTextureLevelParams->setTextureParameterFloatValue(pname, pParam);
                retVal = true;
            }
        }
        else
        {
            // An error occur:
            // Yaki - I removed the below assert, since it made NVSG stuck.
            // gtAssert(0);
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onProgramUsed
// Description: Is called when a program is used (becomes the active program).
// Arguments:   programName - The name of the used program, or 0 to mark that
//                            no program is used (the OpenGL fixed functionality will be used).
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        6/4/2005
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onProgramUsed(GLuint programName)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pProgramsAndShadersMonitor != NULL)
    {
        // Mark that the program as the active program:
        _activeProgramName = programName;

        // If this is the NULL program (means that no program is active)
        if (programName == 0)
        {
            retVal = true;
        }
        else
        {
            // Get the object that represents the program:
            gsGLProgram* pProgramObj = _pProgramsAndShadersMonitor->getProgramObject(programName);

            if (pProgramObj)
            {
                retVal = true;

                // Mark that the program was used in the current frame:
                pProgramObj->setWasUsedInCurrentFrame(true);

                // If the program was restored from stub FS and we are not in stub FS forced mode:
                if ((!_pProgramsAndShadersMonitor->areStubFragmentShadersForced()) && pProgramObj->wasProgramRestoredFromStubFS())
                {
                    retVal = afterProgramRestoredFromStubFS(programName);

                    // Clear the "restored from stub FS" flag:
                    pProgramObj->setProgramRestoredFromStubFS(false);
                }
            }
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::checkForOpenGLErrorAfterModeChange
// Description: Checks for OpenGL Errors on the first function call after "Break
//              on OpenGL Errors" mode was turned on and reports if any previous
//              errors were found. This function assumes the context is already
//              the current context.
// Author:      Uri Shomroni
// Date:        25/5/2010
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::checkForOpenGLErrorAfterModeChange()
{
    // If OpenGL recorded an error before we turned on the "Break on OpenGL errors" mode:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
    GLenum openGLError = gs_stat_realFunctionPointers.glGetError();
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

    if (openGLError != GL_NO_ERROR)
    {
        // Output an appropriate warning message:
        osOutputDebugString(GS_STR_OpenGLErrorBeforeErrorsTest);
    }

    // Mark that we don't need to test until the next time the mode is switched:
    _shouldTestForOpenGLErrorsAtNextFunctionCall = false;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::afterProgramRestoredFromStubFS
// Description:
//   Executes actions that should be performed after a program was restored
//   from forces stub fragment shader mode.
//   Actions include:
//   - Update the programs logged active uniforms.
//   - Restore the active uniforms values to their stored values.
//
//   Notice that this function should only be called when the input program is
//   the active program.
//
// Arguments: programName - The program that was restored from forces stub
//                          fragment shader mode.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        29/11/2005
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::afterProgramRestoredFromStubFS(GLuint programName)
{
    bool retVal = false;

    // We can only act on the active program:
    // (We can only set uniform values of the active program)
    GT_IF_WITH_ASSERT(_activeProgramName == programName)
    {
        GT_IF_WITH_ASSERT(_pProgramsAndShadersMonitor != NULL)
        {
            // if we are not in any other force mode:
            if (!(_pProgramsAndShadersMonitor->areStubFragmentShadersForced() || _pProgramsAndShadersMonitor->areStubGeometryShadersForced()))

            {
                gsActiveUniformsMonitor& programsActiveUniforms = _pProgramsAndShadersMonitor->programsActiveUniformsMgr();
                // Copy aside the programs uniforms values:
                const apGLItemsCollection* programsCurrentUniformsValues = nullptr;
                bool rc1 = programsActiveUniforms.getProgramActiveUniforms(programName, programsCurrentUniformsValues);

                // Update the programs available active uniforms:
                bool rc2 = programsActiveUniforms.onProgramLinked(programName, true);

                bool rc3 = false;

                if (rc1 && (nullptr != programsCurrentUniformsValues))
                {
                    bool isProgramActive = (programName == _activeProgramName);

                    if (!isProgramActive)
                    {
                        // If this program isn't the active program, we need to make it active:
                        isProgramActive = _pProgramsAndShadersMonitor->activateProgramForUpdate(programName);
                    }

                    GT_IF_WITH_ASSERT(isProgramActive)
                    {
                        // Restore the program uniform values:
                        rc3 = programsActiveUniforms.restoreProgramUniformValues(programName, *programsCurrentUniformsValues);

                        if (programName != _activeProgramName)
                        {
                            // Restore the active program
                            bool rcRest = _pProgramsAndShadersMonitor->restoreActiveProgram(_activeProgramName);
                            GT_ASSERT(rcRest);
                        }
                    }
                }

                retVal = rc1 && rc2 && rc3;
            }
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::activateFBO
// Description: Given a frame buffer object OpenGL name, the function marks this
//              FBO as active. If the frame buffer name is 0, it means that the
//              current active FBO should be deactivated
// Arguments: GLuint frameBuffer
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/5/2008
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::activateFBO(GLuint frameBuffer, GLenum target)
{
    bool retVal = false;

    bool updateRead = ((GL_READ_FRAMEBUFFER == target) || (GL_FRAMEBUFFER == target));
    bool updateDraw = ((GL_DRAW_FRAMEBUFFER == target) || (GL_FRAMEBUFFER == target));

    if (frameBuffer == 0)
    {
        retVal = true;

        if (updateRead && (0 != m_activeReadFBOName))
        {
            // If there is an active FBO, deactivate it:
            retVal = updateFBOActiveState(false, m_activeReadFBOName) && retVal;
            m_activeReadFBOName = 0;
        }

        if (updateDraw && (0 != m_activeDrawFBOName))
        {
            // If there is an active FBO, deactivate it:
            retVal = updateFBOActiveState(false, m_activeDrawFBOName) && retVal;
            m_activeDrawFBOName = 0;
        }
    }
    else
    {
        if (updateRead)
        {
            m_activeReadFBOName = frameBuffer;
        }

        if (updateDraw)
        {
            m_activeDrawFBOName = frameBuffer;
        }

        retVal = updateFBOActiveState(true, frameBuffer);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::activateFBO
// Description: Deactivates an FBO object.
//              * unbind all the textures
//              * mark the texture as unbinded
//              * mark the images as dirty (since the FBO was active,and the
//                textures were bounded to it and therefore drawn into)
// Arguments: bool isActive - should the FBO be marked as active/inactive
//            GLuint framebuffer - the frame buffer object name
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/5/2008
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::updateFBOActiveState(bool isActive, GLuint framebuffer)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pFBOMonitor != NULL)
    {
        // Get the currently active FBO:
        const apGLFBO* pCurrentlyActiveFBO = _pFBOMonitor->getFBODetails(framebuffer);

        if (pCurrentlyActiveFBO == NULL)
        {
            // Add the FBO:
            _pFBOMonitor->onGenFramebuffers(&framebuffer, 1);

            // Get the frame buffer monitor object after it was added:
            pCurrentlyActiveFBO = _pFBOMonitor->getFBODetails(framebuffer);
        }

        GT_IF_WITH_ASSERT(pCurrentlyActiveFBO != NULL)
        {
            // For each of the binded texture, mark as unbounded, and as dirty (texture was drawn to
            // while the FBO was active):
            gtList<apFBOBindObject*> bindedObjects;
            bindedObjects = pCurrentlyActiveFBO->getBindedObjects();
            gtList<apFBOBindObject*>::iterator iter = bindedObjects.begin();
            gtList<apFBOBindObject*>::iterator iterEnd = bindedObjects.end();

            // TO_DO: Uri, 20/10/2015 - consider only doing this for the draw FBO:
            for (; iter != iterEnd; iter++)
            {
                apFBOBindObject* pObject = *iter;

                if (pObject != NULL)
                {
                    bool isTextureAttachment = false;
                    bool rc = apGLFBO::isTextureAttachmentTarget(pObject->_attachmentTarget, isTextureAttachment);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        if (isTextureAttachment)
                        {
                            GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
                            {
                                gsGLTexture* pTexture = _pTexturesMonitor->getTextureObjectDetails(pObject->_name);
                                GT_IF_WITH_ASSERT(pTexture != NULL)
                                {
                                    pTexture->markTextureAsBoundToActiveFBO(isActive);

                                    if (!isActive)
                                    {
                                        //pTexture->setFBOName(0);
                                        // Get the textures type:
                                        apTextureType textureType = pTexture->textureType();

                                        // Mark all texture faces as dirty:
                                        if (textureType != AP_CUBE_MAP_TEXTURE)
                                        {
                                            GLenum bindTarget = apTextureTypeToTextureBindTarget(textureType);
                                            pTexture->markTextureAsDirty(bindTarget);
                                        }
                                        else
                                        {
                                            pTexture->markTextureAsDirty(GL_TEXTURE_CUBE_MAP_POSITIVE_X);
                                            pTexture->markTextureAsDirty(GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
                                            pTexture->markTextureAsDirty(GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
                                            pTexture->markTextureAsDirty(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
                                            pTexture->markTextureAsDirty(GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
                                            pTexture->markTextureAsDirty(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            // Get the render buffers monitor and mark the render buffer as bound
                            GT_IF_WITH_ASSERT(_pRenderBuffersMonitor != NULL)
                            {
                                apGLRenderBuffer* pRenderBuffer = _pRenderBuffersMonitor->getRenderBufferObjectDetails(pObject->_name);
                                GT_IF_WITH_ASSERT(pRenderBuffer != NULL)
                                {
                                    pRenderBuffer->markAsBoundToActiveFBO(isActive);

                                    if (!isActive)
                                    {
                                        pRenderBuffer->markAsDirty(true);
                                        //pRenderBuffer->setFBOName(0);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            retVal = true;
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::getActiveFBOForTarget
// Description: Gets the FBO currently bound to the requested target.
// Arguments:   GLuint fboTarget - the FBO target. Should be GL_[READ_|DRAW_|]FRAMEBUFFER.
// Return Val:  The attached FBO. In case of ambiguity, the read framebuffer is returned and an assertion is raised.
// Author:      Uri Shomroni
// Date:        20/10/2015
// ---------------------------------------------------------------------------
GLuint gsRenderContextMonitor::getActiveFBOForTarget(GLenum fboTarget) const
{
    GLuint retVal = 0;

    switch (fboTarget)
    {
        case GL_FRAMEBUFFER:
            retVal = m_activeReadFBOName;
            GT_ASSERT(m_activeReadFBOName == m_activeDrawFBOName);

            if ((m_activeReadFBOName != m_activeDrawFBOName) && (0 == retVal))
            {
                retVal = m_activeDrawFBOName;
            }

            break;

        case GL_READ_FRAMEBUFFER:
            retVal = m_activeReadFBOName;
            break;

        case GL_DRAW_FRAMEBUFFER:
            retVal = m_activeDrawFBOName;
            break;

        default:
            // Unexpected target!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::bindObjectToFBO
// Description: Binds an objects to a named FBO
// Arguments: GLuint framebuffer - the FBO name
//            GLenum attachmentPoint - the FBO attachment point (color/depth/etc')
//            GLenum attachmentTarget - the object type (texture/render buffer)
//            GLuint objectName - the object OpenGL name
//            GLuint textureLayer - the attached texture layer
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/5/2008
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::bindObjectToFBO(GLuint framebuffer, GLenum attachmentPoint, GLenum attachmentTarget, GLuint objectName, GLuint textureLayer)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pFBOMonitor != NULL)
    {
        apGLFBO* pFBO = _pFBOMonitor->getFBODetails(framebuffer);

        if (pFBO != NULL)
        {
            retVal = bindObjectToFBO(pFBO, attachmentPoint, attachmentTarget, objectName, textureLayer);
        }
        else
        {
            // TO_DO: GL_EXT_direct_state_access
            // Add detected error
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::bindObjectToFBO
// Description: Binds an objects to an FBO. The function already get the FBO
//              object pointer
// Arguments: apGLFBO* pFBO
//            GLenum attachmentPoint
//            GLenum attachmentTarget
//            GLuint objectName
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        16/9/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::bindObjectToFBO(apGLFBO* pFBO, GLenum attachmentPoint, GLenum attachmentTarget, GLuint objectName, GLuint textureLayer)
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(pFBO != NULL)
    {
        // Get the currently bound object with the same attachment point:
        apFBOBindObject currentlyBoundObject;
        currentlyBoundObject._attachmentPoint = attachmentPoint;
        currentlyBoundObject._textureLayer = textureLayer;
        bool rc = pFBO->getCurrentlyBoundObject(currentlyBoundObject);

        if (rc)
        {
            // The FBO has a texture object bound to the same attachment point, we should unbind it:
            bool isTextureAttachment = false;
            rc = apGLFBO::isTextureAttachmentTarget(currentlyBoundObject._attachmentTarget, isTextureAttachment);
            GT_IF_WITH_ASSERT(rc)
            {
                if (isTextureAttachment)
                {
                    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
                    {
                        // Get the texture object details:
                        gsGLTexture* pTexture = _pTexturesMonitor->getTextureObjectDetails(currentlyBoundObject._name);
                        GT_IF_WITH_ASSERT(pTexture != NULL)
                        {
                            pTexture->setFBOName(0);
                        }
                    }
                }
                // The FBO has a render buffer object bound to the same attachment point, we should unbind it:
                else if (currentlyBoundObject._attachmentTarget == GL_RENDERBUFFER_EXT)
                {
                    GT_IF_WITH_ASSERT(_pRenderBuffersMonitor != NULL)
                    {
                        // Get the render buffer object:
                        apGLRenderBuffer* pRenderBuffer = _pRenderBuffersMonitor->getRenderBufferObjectDetails(currentlyBoundObject._name);
                        GT_IF_WITH_ASSERT(pRenderBuffer != NULL)
                        {
                            pRenderBuffer->setFBOName(0);
                        }
                    }
                }
            }
        }

        // Bind the object to the FBO
        pFBO->bindObject(attachmentPoint, attachmentTarget, objectName, textureLayer);

        // If objectName is a real object - the name is different  than 0
        // objectName == 0 means that the attachment should be detached
        if (objectName != 0)
        {
            // If the binded object is a texture object, we should mark it as bound to FBO:
            // The FBO has a texture object bound to the same attachment point, we should unbind it:
            bool isTextureAttachment = false;
            rc = apGLFBO::isTextureAttachmentTarget(attachmentTarget, isTextureAttachment);
            GT_IF_WITH_ASSERT(rc)
            {
                GLuint fboName = pFBO->getFBOName();
                bool isActiveFBO = ((m_activeDrawFBOName == fboName) && (m_activeReadFBOName == fboName) && (0 != fboName));

                if (isTextureAttachment)
                {
                    // Get the texture monitor and mark the texture as binded
                    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
                    {
                        gsGLTexture* pTexture = _pTexturesMonitor->getTextureObjectDetails(objectName);
                        GT_IF_WITH_ASSERT(pTexture != NULL)
                        {
                            // Mark the texture as bound to FBO, and set the FBO name:
                            pTexture->markTextureAsBoundToActiveFBO(isActiveFBO);

                            if (isActiveFBO)
                            {
                                pTexture->markTextureAsDirty(attachmentTarget);
                            }

                            pTexture->setFBOName(fboName);
                        }
                    }
                }
                // If the binded object is a render buffer object, we should mark it as bound to FBO:
                else
                {
                    // Get the render buffer monitor and mark the RBO as bound
                    GT_IF_WITH_ASSERT(_pRenderBuffersMonitor != NULL)
                    {
                        apGLRenderBuffer* pRenderBuffer = _pRenderBuffersMonitor->getRenderBufferObjectDetails(objectName);
                        GT_IF_WITH_ASSERT(pRenderBuffer != NULL)
                        {
                            apDisplayBuffer displayBufferType = apGLEnumToColorIndexBufferType(attachmentPoint);
                            pRenderBuffer->markAsBoundToActiveFBO(isActiveFBO);
                            pRenderBuffer->setBufferType(displayBufferType);

                            pRenderBuffer->setFBOName(fboName);
                        }
                    }
                }
            }
        }

        retVal = true;
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::bindObjectToTheActiveFBO
// Description: Binds an objects to an FBO
// Arguments: GLenum attachmentPoint - the FBO attachment point (color/depth/etc')
//            GLenum attachmentTarget - the object type (texture/render buffer)
//            GLuint objectName - the object OpenGL name
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/5/2008
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::bindObjectToTheActiveFBO(GLenum fboTarget, GLenum attachmentPoint, GLenum attachmentTarget, GLuint objectName, GLuint textureLayer)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pFBOMonitor != NULL)
    {
        GLuint fboName = getActiveFBOForTarget(fboTarget);
        apGLFBO* pFBO = _pFBOMonitor->getFBODetails(fboName);
        GT_IF_WITH_ASSERT(pFBO != NULL)
        {
            retVal = bindObjectToFBO(pFBO, attachmentPoint, attachmentTarget, objectName, textureLayer);
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::bindTextureToTheActiveFBO
// Description: Binds a texture to an FBO
// Arguments: GLenum attachmentPoint - the FBO attachment point (color/depth/etc')
//            GLuint textureName - the object OpenGL name
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        3/11/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::bindTextureToTheActiveFBO(GLenum fboTarget, GLenum bindTarget, GLenum attachmentPoint, GLuint textureName, GLuint textureLayer)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((_pFBOMonitor != NULL) && (_pTexturesMonitor != NULL))
    {
        // Get the active FBO details:
        GLuint fboName = getActiveFBOForTarget(fboTarget);
        apGLFBO* pFBO = _pFBOMonitor->getFBODetails(fboName);

        if ((0 != textureName) && (GL_NONE == bindTarget))
        {
            // Get the bound texture details:
            apGLTexture* pTextureObject = _pTexturesMonitor->getTextureObjectDetails(textureName);

            GT_IF_WITH_ASSERT(pTextureObject != NULL)
            {
                // Get the texture target from the texture object:
                apTextureType textureType = pTextureObject->textureType();

                // Translate the texture type to a bind target:
                bindTarget = apTextureTypeToTextureBindTarget(textureType);

                if ((GL_TEXTURE_CUBE_MAP == bindTarget) || (GL_TEXTURE_CUBE_MAP_ARRAY == bindTarget))
                {
                    GT_IF_WITH_ASSERT((textureLayer == GL_TEXTURE_CUBE_MAP_POSITIVE_X) ||
                                      (textureLayer == GL_TEXTURE_CUBE_MAP_NEGATIVE_X) ||
                                      (textureLayer == GL_TEXTURE_CUBE_MAP_POSITIVE_Y) ||
                                      (textureLayer == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y) ||
                                      (textureLayer == GL_TEXTURE_CUBE_MAP_POSITIVE_Z) ||
                                      (textureLayer == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z))
                    {
                        bindTarget = textureLayer;
                    }

                    textureLayer = 0;
                }
            }
        }

        // Bind the object to the FBO:
        GT_IF_WITH_ASSERT((pFBO != NULL) && (bindTarget != GL_NONE))
        {
            retVal = bindObjectToFBO(pFBO, attachmentPoint, bindTarget, textureName, textureLayer);
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::bindTextureToFBO
// Description: Binds a texture to a named FBO
// Arguments: GLenum attachmentPoint - the FBO attachment point (color/depth/etc')
//            GLuint textureName - the object OpenGL name
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/11/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::bindTextureToFBO(GLuint framebuffer, GLenum attachmentPoint, GLuint textureName, GLuint textureLayer)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((_pFBOMonitor != NULL) && (_pTexturesMonitor != NULL))
    {
        // Get the active FBO details:
        apGLFBO* pFBO = _pFBOMonitor->getFBODetails(framebuffer);

        // Get the bound texture details:
        apGLTexture* pTextureObject = _pTexturesMonitor->getTextureObjectDetails(textureName);

        GT_IF_WITH_ASSERT(pTextureObject != NULL)
        {
            // Get the texture target from the texture object:
            apTextureType textureType = pTextureObject->textureType();

            // Translate the texture type to a bind target:
            GLenum bindTarget = apTextureTypeToTextureBindTarget(textureType);
            GT_IF_WITH_ASSERT((pFBO != NULL) && (bindTarget != GL_NONE))
            {
                retVal = bindObjectToFBO(pFBO, attachmentPoint, bindTarget, textureName, textureLayer);
            }
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::bindTextureLayerToTheActiveFBO
// Description: Binds a texture layer to the active FBO
// Arguments:   GLenum bindTarget - the texture bind target
// Arguments:   GLenum attachmentPoint - the texture attachment point
//              GLuint textureName - the texture name (0 to clear the FBO)
//              GLuint textureLayer - the texture layer (for 3d bindings)
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        3/11/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::bindTextureLayerToTheActiveFBO(GLenum fboTarget, GLenum bindTarget, GLenum attachmentPoint, GLuint textureName, GLuint textureLayer)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((_pFBOMonitor != NULL) && (_pTexturesMonitor != NULL))
    {
        // Get the active FBO details:
        GLuint fboName = getActiveFBOForTarget(fboTarget);
        apGLFBO* pFBO = _pFBOMonitor->getFBODetails(fboName);

        if ((0 != textureName) && (GL_NONE == bindTarget))
        {
            // Get the bound texture details:
            apGLTexture* pTextureObject = _pTexturesMonitor->getTextureObjectDetails(textureName);

            GT_IF_WITH_ASSERT(pTextureObject != NULL)
            {
                // Get the texture target from the texture object:
                apTextureType textureType = pTextureObject->textureType();

                // Translate the texture type to a bind target:
                bindTarget = apTextureTypeToTextureBindTarget(textureType);
            }
        }

        // Bind the object to the FBO:
        GT_IF_WITH_ASSERT((pFBO != NULL) && (bindTarget != GL_NONE))
        {
            retVal = bindObjectToFBO(pFBO, attachmentPoint, bindTarget, textureName, textureLayer);
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::setTextureCropRectangle
// Description:
//   Sets the crop parameter (OES_draw_texture extension) of a texture.
//
// Arguments: target - The target texture bind.
//            pRectangleAsFloatArray - The crop rectangle, as 4 floats array:
//                                     (x, y, width, height)
//
// Return Val: bool  - Success / failure. We fail when there is no textures monitor
//                     or the target has no bound texture.
// Author:      Yaki Tebeka
// Date:        10/4/2006
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::setTextureCropRectangle(GLenum target, const GLfloat* pRectangleAsFloatArray)
{
    bool retVal = false;

    // If the we have a textures monitor:
    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
    {
        // Crop rectangle is supported only for 2D textures:
        if (target != GL_TEXTURE_2D)
        {
            // TO_DO: Raise an OpenGL / detected error here !!!
            GT_ASSERT(0);
        }
        else
        {
            // Get the param object that represents the bind texture:
            gsGLTexture* pTexParamObj = getBoundTexture(_activeTextureUnitIndex, target);
            GT_IF_WITH_ASSERT(pTexParamObj != NULL)
            {
                retVal = true;
                ap2DRectangle rectangle(pRectangleAsFloatArray);
                pTexParamObj->setCropRectangle(rectangle);
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::textureUnitIndexToName
// Description: Inputs a texture unit index and outputs its OpenGL name (GL_TEXTUREi).
// Arguments:   textureUnitIndex - The input unit index.
// Return Val:  GLenum - The output name, or GL_FALSE if the index is out of range.
// Author:      Yaki Tebeka
// Date:        19/4/2005
// ---------------------------------------------------------------------------
GLenum gsRenderContextMonitor::textureUnitIndexToName(int textureUnitIndex) const
{
    GLenum retVal = GL_FALSE;

    // Index range check:
    int amountOfTexUnits = amountOfTextureUnits();

    if ((0 <= textureUnitIndex) && (textureUnitIndex < amountOfTexUnits))
    {
        // Get the queried unit enabled texture mode:
        retVal = _textureUnitMonitors[textureUnitIndex]->textureUnitName();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::textureUnitNameToIndex
// Description: Translates OpenGL texture unit name to _textureUnits vector
//              index.
// Arguments:   textureUnitName - The input OpenGL texture unit name.
// Return Val:  int - The output _textureUnits vector index, or -1 in case of
//                    invalid input.
// Author:      Yaki Tebeka
// Date:        18/4/2005
// ---------------------------------------------------------------------------
int gsRenderContextMonitor::textureUnitNameToIndex(GLenum textureUnitName) const
{
    int retVal = -1;

    // Verify that the input is in the right range:
    if ((GL_TEXTURE0 <= textureUnitName) && (textureUnitName <= GL_TEXTURE31))
    {
        // Translate to 0 based index:
        retVal = textureUnitName - GL_TEXTURE0;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::getEnabledTexturingMode
// Description: Returns the currently enabled texturing mode.
// Arguments:   textureUnitIndex - The queried texture unit index.
//              isTexturingEnabled - Will get true iff texturing is enabled.
//              enabledTexturingMode - Will get the enabled texturing mode.
// Author:      Yaki Tebeka
// Date:        30/12/2004
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::getEnabledTexturingMode(int textureUnitIndex, bool& isTexturingEnabled, apTextureType& enabledTexturingMode) const
{
    bool retVal = false;

    // Sanity check:
    int amountOfTexUnits = amountOfTextureUnits();

    if ((0 <= textureUnitIndex) && (textureUnitIndex < amountOfTexUnits))
    {
        // Get the queried unit enabled texture mode:
        _textureUnitMonitors[textureUnitIndex]->getEnabledTexturingMode(isTexturingEnabled, enabledTexturingMode);
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::bindTextureName
// Description: Inputs a bind target and outputs the OpenGL name of the texture
//              that is bind to this target.
// Arguments:   textureUnitIndex - The index of the queried texture unit.
//              target - The queried bind target.
//              textureName - The output texture name.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        23/12/2004
// Implementation notes:
//   The currently supported bound targets are: GL_TEXTURE_1D, GL_TEXTURE_2D,
//   GL_TEXTURE_3D, GL_TEXTURE_CUBE_MAP and GL_TEXTURE_RECTANGLE
// ---------------------------------------------------------------------------
GLuint gsRenderContextMonitor::bindTextureName(int textureUnitIndex, apTextureType bindTarget) const
{
    GLuint retVal = 0;

    // Sanity check:
    int amountOfTexUnits = amountOfTextureUnits();

    if ((0 <= textureUnitIndex) && (textureUnitIndex < amountOfTexUnits))
    {
        // Get the queried unit bind texture name:
        retVal = _textureUnitMonitors[textureUnitIndex]->bindTextureName(bindTarget);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::applyForcedStubTextureObjects
// Description: Applies the "Forces stub textures" mode.
// Author:      Yaki Tebeka
// Date:        2/3/2005
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::applyForcedStubTextureObjects()
{
    if (_pTexturesMonitor != NULL)
    {
        // Iterate the GL_ARB_multitexture extension texture units:
        for (int i = 0; i < _maxTextureUnits; i++)
        {
            gsTextureUnitMonitor* currentTexUnitMtr = _textureUnitMonitors[i];

            // Use the texture monitor to apply the stub textures:
            _pTexturesMonitor->applyForcedStubTextureObjectsToTexUnitMtr(currentTexUnitMtr);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::cancelForcedStubTextureObjects
// Description: Cancels the "Forces stub textures" mode.
// Author:      Yaki Tebeka
// Date:        2/3/2005
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::cancelForcedStubTextureObjects()
{
    if (_pTexturesMonitor)
    {
        // Iterate the GL_ARB_multitexture extension texture units:
        for (int i = 0; i < _maxTextureUnits; i++)
        {
            // Replace currently bound textures with the stub textures:
            _textureUnitMonitors[i]->cancelForcedStubTextureObjects();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::getBoundTexture
// Description: Inputs a texture unit and a bind target and outputs the gsGLTexture object
//              that represents the texture that is bound this target (or NULL, if
//              the object is not found).
// Author:      Yaki Tebeka
// Date:        22/1/2005
// ---------------------------------------------------------------------------
gsGLTexture* gsRenderContextMonitor::getBoundTexture(int textureUnitIndex, GLenum bindTarget)
{
    gsGLTexture* retVal = NULL;

    // Get the texture that is bind to the input bind target:
    apTextureType textureType = apTextureBindTargetToTextureType(bindTarget);
    GLuint bindTexName = bindTextureName(textureUnitIndex, textureType);

    // If the bind texture is a bind target default texture object:
    // (The bound texture object name is 0):
    if (bindTexName == 0)
    {
        // Create an object that will represent the bind target default texture object:
        bindTexName = createDefaultTextureObject(_activeTextureUnitIndex, textureType);

        // Make it the bound texture object:
        _textureUnitMonitors[_activeTextureUnitIndex]->onTextureTargetBind(bindTarget, bindTexName);
    }

    // Verify that we have a bind texture name:
    if (bindTexName != 0)
    {
        // Look for bind texture object monitor:
        int monitorObjIndex = _pTexturesMonitor->textureObjMonitorIndex(bindTexName);

        if (monitorObjIndex == -1)
        {
            // We didn't find a texture monitor object that represents this texture name.
            // This means that the application uses texture names without creating them first,
            // which is legal in OpenGL: A quote from glBindTexture documentation:
            // "glBindTexture binds the texture named texture to the specified target. If the
            //  name does not exist, it is created."

            // Generate the texture object wrapper now:
            _pTexturesMonitor->onTextureObjectsGeneration(1, &bindTexName);

            // Get the texture monitor index.
            monitorObjIndex = _pTexturesMonitor->textureObjMonitorIndex(bindTexName);
        }

        // Sanity check:
        if (monitorObjIndex != -1)
        {
            // Get the texture monitor object that represents the bind texture name:
            retVal = _pTexturesMonitor->getTextureObjectDetails(bindTexName);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::createDefaultTextureObject
// Description: Creates an object that will represent a bind target default texture object.
//              (For more details, see the "What are default texture names?" comment
//               at apDefaultTextureNames.h).
// Arguments:   textureUnitIndex - The index of the texture unit in which the bind target resides.
//              textureType - The bind target type.
// Return Val:  GLuint - The created default texture object name, or 0 in case of failure.
// Author:      Yaki Tebeka
// Date:        20/4/2005
// ---------------------------------------------------------------------------
GLuint gsRenderContextMonitor::createDefaultTextureObject(int textureUnitIndex, apTextureType textureType)
{
    GLuint retVal = 0;

    // Sanity check:
    if (textureType != AP_UNKNOWN_TEXTURE_TYPE)
    {
        // Get the texture unit OpenGL name:
        GLenum textureUnitName = textureUnitIndexToName(textureUnitIndex);

        // Get the default texture object "name":
        GLuint textureObjectName = 0;
        bool rc = apGetDefaultTextureName(textureUnitName, textureType, textureObjectName);

        if (rc)
        {
            // Create an object that will represent this default texture object:
            _pTexturesMonitor->onTextureObjectsGeneration(1, &textureObjectName);
            retVal = textureObjectName;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::markTextureParametersAsNonUpdated
// Description: Marks the current context textures as not updated
// Return Val: void
// Author:      Sigal Algranaty
// Date:        30/11/2008
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::markTextureParametersAsNonUpdated()
{
    if (_pTexturesMonitor != NULL)
    {
        _pTexturesMonitor->markTextureParametersAsNonUpdated();
    }
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::wasDeleted
// Description: Returns true iff my monitored context exist in the OS.
// Author:      Yaki Tebeka
// Date:        17/10/2005
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::wasDeleted() const
{
    return (_renderContextOSHandle == NULL);
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::setPixelFormatId
// Description: Sets the OS pixel format id of this render context.
// Arguments: OSPixelFormatId - The input OS id for this pixel format.
// Author:      Yaki Tebeka
// Date:        23/7/2006
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::setPixelFormatId(oaPixelFormatId OSPixelFormatId)
{
    // Log the pixel format id:
    _pixelFormatId = OSPixelFormatId;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    // There is no way a render context could change its pixel format in CGL:
    GT_IF_WITH_ASSERT(_pPixelFormatDescription == NULL)
    {
        // Log the pixel format description:
        _pPixelFormatDescription = new oaPixelFormat(_deviceContextOSHandle, _pixelFormatId);
        GT_IF_WITH_ASSERT(_pPixelFormatDescription != NULL)
        {
            // Our Mac implementation of oaPixelFormat uses CGLDescribePixelFormat for initialize():
#ifdef _GR_IPHONE_BUILD
            // TO_DO iPhone
#else
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_CGLDescribePixelFormat);
            bool rcInit = _pPixelFormatDescription->initialize();
            GT_ASSERT(rcInit);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_CGLDescribePixelFormat);
#endif

            // Check if this context is rendered using a software renderer:
            checkForSoftwareRenderer();
        }
    }
#endif
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::initializePixelFormatFromOpenGLES
// Description: Makes the EAGL pixel format object initialize from OpenGL ES,
//              as the details are not available to us before the context has
//              been made current.
// Author:      Uri Shomroni
// Date:        23/5/2010
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::initializePixelFormatFromOpenGLES()
{
    // This function should not be called if we already have the pixel format information:
    GT_IF_WITH_ASSERT(_pPixelFormatDescription == NULL)
    {
        // Log the pixel format description:
        _pPixelFormatDescription = new oaPixelFormat(_deviceContextOSHandle, _pixelFormatId);

        // Get the "default" values for the GL_xxx_BITS variables, which should be
        // updated when the context is made current for the first time. Since these
        // values cannot change for the context, their (constant) values describe
        // the pixel format. However note that bound objects (such as framebuffers) can
        // change the values at runtime, so it would not be enough to query them when needed -
        // we need to query them at creation time.
        int rBits = getIntChannelSizeParameterDefaultValue(apGL_RED_BITS);
        int gBits = getIntChannelSizeParameterDefaultValue(apGL_GREEN_BITS);
        int bBits = getIntChannelSizeParameterDefaultValue(apGL_BLUE_BITS);
        int aBits = getIntChannelSizeParameterDefaultValue(apGL_ALPHA_BITS);
        int depthBits = getIntChannelSizeParameterDefaultValue(apGL_DEPTH_BITS);
        int stencilBits = getIntChannelSizeParameterDefaultValue(apGL_STENCIL_BITS);

        // Use these values to initialize the pixel format:
        _pPixelFormatDescription->initializeGLESPixelFormatWithChannelValues(rBits, gBits, bBits, aBits, depthBits, stencilBits);
    }
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::getGLObjectType
// Description:
//   Inputs the OpenGL name of an object that resides in this context
//   and returns its type as an osTransferableObjectType.
//   The currently supported types are:
//   - OS_TOBJ_ID_GL_*_SHADER
//   - OS_TOBJ_ID_GL_PROGRAM
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/5/2005
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::getGLObjectType(GLenum objectName, osTransferableObjectType& objType)
{
    bool retVal = false;
    objType = OS_AMOUNT_OF_TRANSFERABLE_OBJECT_TYPES;

    // Look for program and shader types:
    if (_pProgramsAndShadersMonitor->isShaderObject(objectName))
    {
        // Get the shader type:
        objType = _pProgramsAndShadersMonitor->shaderObjectType(objectName);
        retVal = (OS_AMOUNT_OF_TRANSFERABLE_OBJECT_TYPES != objType);
    }
    else if (_pProgramsAndShadersMonitor->isProgramObject(objectName))
    {
        // This is a program:
        objType = OS_TOBJ_ID_GL_PROGRAM;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::updateContextDataSnapshot
// Description: Updates the snapshot of context data that should be available
//              when the context thread is froze.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        17/1/2005
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::updateContextDataSnapshot(bool sendEvents)
{
    bool retVal = false;

    // Temporarily disable breaking on GL messages, since these might be generated by us:
    bool wasReportingDebugMessages = _forcedModesManager.isDebugOutputLoggingEnabled();
    _forcedModesManager.setGLDebugOutputLoggingEnabled(false);

    if (wasReportingDebugMessages)
    {
        m_debugOutputManager.applyDebugOutputSettingsToContext();
    }

    // If this context is not in a glBegin-glEnd block:
    if (!isInOpenGLBeginEndBlock())
    {

        // Update clients that the context data update is about to begin:
        beforeUpdatingContextDataSnapshot();

        if (sendEvents)
        {
            // Send a progress event to client:
            suSendSpyProgressEvent(AP_AFTER_RENDER_CONTEXT_UPDATE);
        }

        // Update textures data snapshot:
        if (_pTexturesMonitor != NULL)
        {
            // Output debug log printout:
            OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateTextureDataStarted, OS_DEBUG_LOG_DEBUG);

            // Update the texture units data:
            int amountOfTexUnits = amountOfTextureUnits();

            for (int i = 0; i < amountOfTexUnits; i++)
            {
                _textureUnitMonitors[i]->updateContextDataSnapshot(_contextID._contextId, _oglVersion);
            }

            if (sendEvents)
            {
                // Send a progress event to client:
                suSendSpyProgressEvent(AP_AFTER_TEXTURE_UNIT_MONITORS_UPDATE);
            }

            // Output debug log printout:
            OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateTextureDataEnded, OS_DEBUG_LOG_DEBUG);
        }

        // Update render buffers data snapshot:
        _pRenderBuffersMonitor->updateContextDataSnapshot();

        // Update the samplers data snapshot:
        _samplersMonitor.updateContextDataSnapshot();

        if (sendEvents)
        {
            // Send a progress event to client:
            suSendSpyProgressEvent(AP_AFTER_RENDER_BUFFERS_MONITOR_UPDATE);
        }

        // Update static buffers data snapshot:
        _buffersMonitor.updateContextDataSnapshot(_shouldUpdateStateVariables);

        if (sendEvents)
        {
            // Send a progress event to client:
            suSendSpyProgressEvent(AP_AFTER_STATIC_BUFFERS_MONITOR_UPDATE);
        }

        // State variable updating is time-consuming and can cause instabilities.
        // Avoid it when performing other activities such as checking for memory leaks.
        bool rc1 = true;

        if (_shouldUpdateStateVariables)
        {
            // Update state variables data snapshot:
            rc1 = _stateVariablesSnapshot.updateContextDataSnapshot();

            if (sendEvents)
            {
                // Send a progress event to client:
                suSendSpyProgressEvent(AP_AFTER_STATE_VARIABLES_UPDATE);
            }
        }

        // Update programs and shaders context data snapshot:
        bool rc2 = _pProgramsAndShadersMonitor->updateContextDataSnapshot();

        if (sendEvents)
        {
            // Send a progress event to client:
            suSendSpyProgressEvent(AP_AFTER_PROGRAM_AND_SHADERS_UPDATE);
        }

        // Update clients that the context data update was ended:
        afterUpdatingContextDataSnapshot();

        retVal = rc1 && rc2;
    }
    else // isInOpenGLBeginEndBlock()
    {
        // We are in a glBegin-glEnd block - clear the context data snapshots:

        if (sendEvents)
        {
            // Send a progress event to client:
            suSendSpyProgressEvent(AP_AFTER_RENDER_CONTEXT_UPDATE);
        }

        // Output debug log printout:
        OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdatingWhileInBeginEndBlock, OS_DEBUG_LOG_DEBUG);

        // Clears texture units enabled texturing modes:
        int amountOfTexUnits = amountOfTextureUnits();

        for (int i = 0; i < amountOfTexUnits; i++)
        {
            _textureUnitMonitors[i]->clearContextDataSnapshot();
        }

        if (sendEvents)
        {
            // Send a progress event to client:
            suSendSpyProgressEvent(AP_AFTER_TEXTURE_UNIT_MONITORS_UPDATE);
        }

        // Clear render buffers data snapshot:
        _pRenderBuffersMonitor->clearContextDataSnapshot();

        if (sendEvents)
        {
            // Send a progress event to client:
            suSendSpyProgressEvent(AP_AFTER_RENDER_BUFFERS_MONITOR_UPDATE);
        }

        // Clear buffers data snapshot:
        _buffersMonitor.clearContextDataSnapshot();

        if (sendEvents)
        {
            // Send a progress event to client:
            suSendSpyProgressEvent(AP_AFTER_STATIC_BUFFERS_MONITOR_UPDATE);
        }

        // Clear state variables data snapshot:
        _stateVariablesSnapshot.clearContextDataSnapshot();

        if (sendEvents)
        {
            // Send a progress event to client:
            suSendSpyProgressEvent(AP_AFTER_STATE_VARIABLES_UPDATE);
        }

        // Clear programs and shaders context data snapshot:
        _pProgramsAndShadersMonitor->clearContextDataSnapshot();

        if (sendEvents)
        {
            // Send a progress event to client:
            suSendSpyProgressEvent(AP_AFTER_PROGRAM_AND_SHADERS_UPDATE);
        }

        retVal = true;
    }

    // Restore the debug messages:
    _forcedModesManager.setGLDebugOutputLoggingEnabled(wasReportingDebugMessages);

    if (wasReportingDebugMessages)
    {
        m_debugOutputManager.applyDebugOutputSettingsToContext();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::isInOpenGLBeginEndBlock
// Description:
//  Returns true iff this render context is in a glBegin-glEnd block.
//
//  Notice: This function should only be called between the monitored function logging
//          and the real function execution.
//
// Author:      Yaki Tebeka
// Date:        26/7/2004
// Implementation Notes:
//   We are called between the monitored function logging and the real function execution.
//   This means that we should consider both the context logger and the currently called
//   function.
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::isInOpenGLBeginEndBlock() const
{
    bool retVal = true;

    gsCallsHistoryLogger* pCallsHistoryLogger = (gsCallsHistoryLogger*)callsHistoryLogger();
    GT_IF_WITH_ASSERT(pCallsHistoryLogger != NULL)
    {
        // Get the last logged function id (in the input context):
        int lastLoggedFunctionId = pCallsHistoryLogger->lastCalledFunctionId();

        // If the input context logger is in a glBegin - glEnd block:
        bool isLoggerDuringOpenGLBeginEndBlock = pCallsHistoryLogger->isInOpenGLBeginEndBlock();

        if (isLoggerDuringOpenGLBeginEndBlock)
        {
            // The calls logger logs the function before it is executed.
            // I.E: if glBegin is the last logged function - it was not executed yet:
            if (lastLoggedFunctionId == ap_glBegin)
            {
                retVal = false;
            }
        }
        else
        {
            // The calls logger logs the function before it is executed.
            // I.E: if glEnd is the last logged function - it was not executed yet:
            if (lastLoggedFunctionId != ap_glEnd)
            {
                retVal = false;

            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::getOpenGLVersion
// Description: Retrieves the OpenGL version supported by my monitored
//              render context.
// Author:      Yaki Tebeka
// Date:        22/2/2006
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::getOpenGLVersion(int& majorNumber, int& minorNumber) const
{
    majorNumber = _oglVersion[0];
    minorNumber = _oglVersion[1];
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::flushContentToDrawSurface
// Description:
//   Flushed the render context content to the surface on which it is drawn
//   (Usually a screen).
//   I.E:
//   - For single buffered contexts - calls glFlush.
//   - For double buffered contexts - calls SwapBuffers.
//
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        23/7/2006
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::flushContentToDrawSurface()
{
    bool retVal = false;

    bool isDoubleBuff = false;

#ifdef _GR_IPHONE_BUILD
    // iPhone contexts are always double-buffered:
    isDoubleBuff = true;
#else
    // Get the render context pixel format description:
    GT_IF_WITH_ASSERT(_pPixelFormatDescription != NULL)
    {
        // Check if the context is double-buffered:
        isDoubleBuff = _pPixelFormatDescription->isDoubleBuffered();
    }
#endif

    if (isDoubleBuff && !suIsDuringDebuggedProcessTermination())
    {
        // If the render context is double buffered, swap the buffers:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        // Disable "wglSwapBuffers" logging:
        gsEnableInitializationFunctionsLogging(false);

        // Swap its buffers:
        BOOL rc = ::SwapBuffers(_deviceContextOSHandle);

        if (rc == TRUE)
        {
            retVal = true;
        }

        // Enable "wglSwapBuffers" logging:
        gsEnableInitializationFunctionsLogging(true);
#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)
        // If there is a current drawable:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glXGetCurrentDrawable);
        GLXDrawable currentDrawable = gs_stat_realFunctionPointers.glXGetCurrentDrawable();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glXGetCurrentDrawable);

        if (currentDrawable != None)
        {
            // Swap its buffers:
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glXSwapBuffers);
            gs_stat_realFunctionPointers.glXSwapBuffers(_deviceContextOSHandle, currentDrawable);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glXSwapBuffers);
            retVal = true;
        }

#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
#ifdef _GR_IPHONE_BUILD
        retVal = gsSwapEAGLContextBuffers(_renderContextOSHandle);
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_CGLFlushDrawable);
        gs_stat_realFunctionPointers.CGLFlushDrawable(_renderContextOSHandle);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_CGLFlushDrawable);

        retVal = true;
#endif
#else
#error Unknown build configuration!
#endif
    }
    else
    {
        // This is a single buffered context - simply flush its context:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glFlush);
        gs_stat_realFunctionPointers.glFlush();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glFlush);
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::setObjectSharingContext
// Description:
// Arguments: objectSharingContext - the internal numeral ID of the other RC
//            otherRC - a pointer to our RC wrapper, for copying relevant information
//            deleteSubMonitors - whether to actually delete the spy textures monitor
//              programs and shaders monitor and so on. This is needed when we share
//              contexts because only the called context should delete the sub monitors
//            This is supposed to share:
//              - display lists (essentially replaced by VBO/IBO)
//              - VBOs/IBOs
//              + programs + shaders
//              + textures
//              + FBOs
//              - PBOs
// Author:      Uri Shomroni
// Date:        12/6/2008
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::setObjectSharingContext(int objectSharingContext, gsRenderContextMonitor* otherRC, bool deleteSubMonitors)
{
    // If we are not pointing at ourselves, note who are pointing at:
    // The situation where we are called to point at ourselves is caused when the
    // user tried to connect multiple contexts more than once eg
    // 2->1, 3->1 and then 4->2 and 4->3.
    if (_contextID._contextId != objectSharingContext)
    {
        _objectSharingContext = objectSharingContext;
    }

    // Sanity check:
    GT_IF_WITH_ASSERT(otherRC != NULL)
    {
        GT_ASSERT_EX((otherRC->spyId() == objectSharingContext), L"Trying to use sharelists with a Render Context that does not match the given ID.")

        gsTexturesMonitor* otherTexMon = otherRC->texturesMonitor();
        gsProgramsAndShadersMonitor* otherProgAndShadMon = otherRC->programsAndShadersMonitor();
        gsRenderBuffersMonitor* otherRenBufMon = otherRC->renderBuffersMonitor();
        gsFBOMonitor* pOtherFBOMon = otherRC->fboMonitor();
        gsVBOMonitor* pOtherVBOMon = otherRC->vboMonitor();
        gsDisplayListMonitor* pOtherDispListMon = otherRC->displayListsMonitor();

        if (deleteSubMonitors)
        {
            // If the user is trying to share with a context we're already actually sharing, don't delete them
            // this Causes a crash because
            if ((_pTexturesMonitor != NULL) && (_pTexturesMonitor != otherTexMon))
            {
                delete _pTexturesMonitor;
            }

            if ((_pProgramsAndShadersMonitor != NULL) && (_pProgramsAndShadersMonitor != otherProgAndShadMon))
            {
                delete _pProgramsAndShadersMonitor;
            }

            if ((_pRenderBuffersMonitor != NULL) && (_pRenderBuffersMonitor != otherRenBufMon))
            {
                delete _pRenderBuffersMonitor;
            }

            if ((_pFBOMonitor != NULL) && (_pFBOMonitor != pOtherFBOMon))
            {
                delete _pFBOMonitor;
            }

            if ((_pVBOMonitor != NULL) && (_pVBOMonitor != pOtherVBOMon))
            {
                delete _pVBOMonitor;
            }

            if ((_pDisplayListMonitor != NULL) && (_pDisplayListMonitor != pOtherDispListMon))
            {
                delete _pDisplayListMonitor;
            }
        }

        _pTexturesMonitor = otherTexMon;
        _pProgramsAndShadersMonitor = otherProgAndShadMon;
        _pRenderBuffersMonitor = otherRenBufMon;
        _pFBOMonitor = pOtherFBOMon;
        _pVBOMonitor = pOtherVBOMon;
        _pDisplayListMonitor = pOtherDispListMon;
    }
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::constructGraphicsInfo
// Description: Constrcuts an apGLRenderContextGraphicsInfo to represent this
//              render context monitor.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        19/3/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::constructGraphicsInfo(apGLRenderContextGraphicsInfo& graphicsInfo) const
{
    bool retVal = false;

    // Set compatibility attributes:
    graphicsInfo.setComaptibilityContext(_isCompatibiltyContext);
    graphicsInfo.setForwardCompatible(_isForwardCompatibleContext);
    graphicsInfo.setDebugFlagDetails(_isDebugContext, _isDebugContextFlagForced);

    // Make sure we have a pixel format and that its info was initialized:
    GT_IF_WITH_ASSERT(_pPixelFormatDescription != NULL)
    {
        GT_IF_WITH_ASSERT(_pPixelFormatDescription->isInitialized())
        {
            apGLRenderContextGraphicsInfo::hardwareAcceleration acceleration = apGLRenderContextGraphicsInfo::AP_UNKNOWN_HARDWARE_ACCELERATED_CONTEXT;

            switch (_pPixelFormatDescription->hardwareSupport())
            {
                case oaPixelFormat::FULL_HARDWARE_ACCELERATION:
                    acceleration = apGLRenderContextGraphicsInfo::AP_FULL_HARDWARE_ACCELERATED_CONTEXT;
                    break;

                case oaPixelFormat::PARTIAL_HARDWARE_ACCELERATION:
                    acceleration = apGLRenderContextGraphicsInfo::AP_PARTIAL_HARDWARE_ACCELERATED_CONTEXT;
                    break;

                case oaPixelFormat::NO_HARDWARE_ACCELERATION:
                    acceleration = apGLRenderContextGraphicsInfo::AP_NOT_HARDWARE_ACCELERATED_CONTEXT;
                    break;

                default:
                    // Something's wrong
                    GT_ASSERT(false);
                    break;
            }

            int pixelFormatIndex = -1;
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)))
            // Linux and Windows pixel formats can be enumerated:
            pixelFormatIndex = (int)_pPixelFormatDescription->nativeId();
#endif

            bool stereo = _pPixelFormatDescription->isStereoscopic();
            bool supportsNative = _pPixelFormatDescription->supportsNativeRendering();
            graphicsInfo.setGeneralGraphicsInfo(pixelFormatIndex, _pPixelFormatDescription->isDoubleBuffered(), acceleration, stereo, supportsNative);
            oaPixelFormat::PixelType pixType = _pPixelFormatDescription->pixelType();

            if (pixType == oaPixelFormat::COLOR_INDEX)
            {
                // Indexed pixel format:
                graphicsInfo.setChannels(0, 0, 0, 0, _pPixelFormatDescription->amountOfColorBits(), _pPixelFormatDescription->amountOfZBufferBits(),
                                         _pPixelFormatDescription->amountOfStencilBufferBits(), _pPixelFormatDescription->amountOfAccumulationBufferBits());
            }
            else
            {
                // RGBA pixel format:
                GT_ASSERT(pixType == oaPixelFormat::RGBA);
                graphicsInfo.setChannels(_pPixelFormatDescription->amountOfRedBits(), _pPixelFormatDescription->amountOfGreenBits(), _pPixelFormatDescription->amountOfBlueBits(),
                                         _pPixelFormatDescription->amountOfAlphaBits(), 0, _pPixelFormatDescription->amountOfZBufferBits(), _pPixelFormatDescription->amountOfStencilBufferBits(), _pPixelFormatDescription->amountOfAccumulationBufferBits());
            }

            retVal = true;
        }
    }

    if (_objectSharingContext == -1)
    {
        // If this context isn't sharing lists, see by which contexts (if any) it is shared:
        int amountOfContexts = gs_stat_openGLMonitorInstance.amountOfContexts();

        for (int i = 1; i < amountOfContexts; i++)
        {
            const gsRenderContextMonitor* pCurrentContext = gs_stat_openGLMonitorInstance.renderContextMonitor(i);
            GT_IF_WITH_ASSERT(pCurrentContext != NULL)
            {
                // If this context shares us:
                if (pCurrentContext->getObjectSharingContextID() == _contextID._contextId)
                {
                    graphicsInfo.addSharingContext(i);
                }
            }
            else
            {
                // We failed to get the information:
                retVal = false;
            }
        }
    }

    graphicsInfo.setOpenGLVersion(_oglVersion[0], _oglVersion[1]);

    graphicsInfo.setShadingLanguageVersionString(_openGLShadingLangVersionString);

    int numberOfAffGPUs = (int)_gpuAffinities.size();

    for (int i = 0; i < numberOfAffGPUs; i++)
    {
        graphicsInfo.addGPUAffinity(_gpuAffinities[i]);
    }

    graphicsInfo.setRendererInformation(_openGLVendorString, _openGLRendererString, m_openGLVersionString);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::parseAttribList
// Description: Parses the attriblist parameter of [wgl|glX]CreateContextAttribsARB
// Author:      Uri Shomroni
// Date:        5/1/2010
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::parseAttribList(const int* attribList)
{
    // Only relevant if the attriblist is present:
    if (attribList != NULL)
    {
        //////////////////////////////////////////////////////////////////////////
        // Compatibility profiles and the forward-compatible flag:
        // Also see
        // http://www.opengl.org/registry/specs/ARB/wgl_create_context.txt
        // http://www.opengl.org/registry/specs/ARB/glx_create_context.txt
        // 1. If the compatibility profile is requested, the context is backwards-compatible.
        // 2. If a non-compatibility profile is requested, the context isn't backwards-compatible.
        // 3. If the forward-compatible flag is set and no profile is requested, the context isn't backwards-compatible.
        // 4. If the forward-compatible flag is not set (or no flags are given) and no profile is requested, the context is backwards-compatible.
        //
        // Note that profiles override the flag in this aspect no matter what the flag is.
        //////////////////////////////////////////////////////////////////////////
        bool wasProfileAttribRead = false;

        // The attriblist parameter is an array of form:
        // <name0, value0, name1, value1, ... nameN, valueN, 0>
        // for both wgl and glX. Note that value is always a single int
        // so that the index of "nameN" is always 2N and "valueN" is 2N+1:
        for (int i = 0; i < GS_MAX_READ_CONTEXT_ATTRIBUTES; i += 2)
        {
            // Get the current attribute's name:
            const int& name = attribList[i];

            if (name == 0)
            {
                break;
            }
            else
            {
                // Get the value:
                const int& val = attribList[i + 1];

                switch (name)
                {
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

                    // wglCreateContextAttribsARB attribs:
                    case WGL_CONTEXT_MAJOR_VERSION_ARB:
                        break;

                    case WGL_CONTEXT_MINOR_VERSION_ARB:
                        break;

                    case WGL_CONTEXT_LAYER_PLANE_ARB:
                        break;

                    case WGL_CONTEXT_FLAGS_ARB:
                    {
                        // WGL_CONTEXT_DEBUG_BIT_ARB

                        if ((val & WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB) != 0)
                        {
                            _isForwardCompatibleContext = true;

                            // See above
                            if (!wasProfileAttribRead)
                            {
                                _isCompatibiliyProfile = false;
                            }
                        }

                        if ((val & WGL_CONTEXT_DEBUG_BIT_ARB) != 0)
                        {
                            _isDebugContext = true;
                        }
                    }
                    break;

                    case WGL_CONTEXT_PROFILE_MASK_ARB:
                    {
                        // WGL_CONTEXT_CORE_PROFILE_BIT_ARB

                        // If this attrib isn't present, the value is true, since the compatibility
                        // profile is the default profile:
                        _isCompatibiliyProfile = ((val & WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB) != 0);
                        wasProfileAttribRead = true;
                    }
                    break;
#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)

                    // glXCreateContextAttribsARB attribs:
                    case GLX_CONTEXT_MAJOR_VERSION_ARB:
                        break;

                    case GLX_CONTEXT_MINOR_VERSION_ARB:
                        break;

                    case GLX_CONTEXT_FLAGS_ARB:
                    {
                        // GLX_CONTEXT_DEBUG_BIT_ARB

                        if ((val & GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB) != 0)
                        {
                            _isForwardCompatibleContext = true;

                            // See above
                            if (!wasProfileAttribRead)
                            {
                                _isCompatibiliyProfile = false;
                            }
                        }

                        if ((val & GLX_CONTEXT_DEBUG_BIT_ARB))
                        {
                            _isDebugContext = true;
                        }
                    }
                    break;

                    case GLX_CONTEXT_PROFILE_MASK_ARB:
                    {
                        // GLX_CONTEXT_CORE_PROFILE_BIT_ARB

                        // If this attrib isn't present, the value is true, since the compatibility
                        // profile is the default profile:
                        _isCompatibiliyProfile = ((val & GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB) != 0);
                        wasProfileAttribRead = true;
                    }
                    break;
#endif // AMDT_BUILD_TARGET

                    default:
                    {
                        // Unknown attribute, do nothing.
                    }
                    break;
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::logCreatedContextParameters
// Description: Writes the created context parameters into the OGL Server
//              debug log file.
// Author:      Yaki Tebeka
// Date:        3/8/2007
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::logCreatedContextParameters()
{
    // Make sure we didn't have OpenGL errors before:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
    GLenum previousError = gs_stat_realFunctionPointers.glGetError();
    GT_ASSERT(previousError == GL_NO_ERROR);

    // Get the render context parameters:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetString);
    _openGLVendorString.fromASCIIString((const char*)gs_stat_realFunctionPointers.glGetString(GL_VENDOR));
    _openGLRendererString.fromASCIIString((const char*)gs_stat_realFunctionPointers.glGetString(GL_RENDERER));

    _openGLShadingLangVersionString = L"NA";
    const char* pOpenGLShadingLangVersionString = (const char*)gs_stat_realFunctionPointers.glGetString(GL_SHADING_LANGUAGE_VERSION);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetString);

    // The call the glGetString(GL_SHADING_LANGUAGE_VERSION) creates an OpenGL error if shaders are not supported.
    // Clear that error:
    GLenum err = gs_stat_realFunctionPointers.glGetError();
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

    if ((pOpenGLShadingLangVersionString != NULL) && (err == GL_NO_ERROR))
    {
        _openGLShadingLangVersionString.fromASCIIString(pOpenGLShadingLangVersionString);
    }

    int OGLMajorVersion = 0;
    int OGLMinorVersion = 0;
    getOpenGLVersion(OGLMajorVersion, OGLMinorVersion);

    // Build a string that will be sent to the log file:
    gtString logFileString;
    logFileString.appendFormattedString(GS_STR_renderContextIsCurrentForFirstTime, _contextID._contextId);
    logFileString.append(L": ");
    logFileString.append(GS_STR_OGLVendor);
    logFileString.append(L": ");
    logFileString.append(_openGLVendorString);
    logFileString.append(L", ");
    logFileString.append(GS_STR_OGLRenderer);
    logFileString.append(L": ");
    logFileString.append(_openGLRendererString);
    logFileString.append(L", ");
    logFileString.appendFormattedString(L"OpenGL version: %d.%d", OGLMajorVersion, OGLMinorVersion);

    if (_isCompatibiltyContext) { logFileString.append(L" (Compatibility Context)"); }

    logFileString.append(L", ");
    logFileString.append(GS_STR_OGLShadingLangVersion);
    logFileString.append(L": ");
    logFileString.append(_openGLShadingLangVersionString);

    // Output the string to the log file:
    OS_OUTPUT_DEBUG_LOG(logFileString.asCharArray(), OS_DEBUG_LOG_INFO);

    // TO_DO: Check if we want to log also GLX parameters - see glXQueryServerString,
    // glXGetClientString, glXQueryVersion, etc.
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::updateOpenGLVersion
// Description: Updated the OpenGL version that this render context supports.
// Author:      Yaki Tebeka
// Date:        22/2/2006
//
// Implementation notes:
//   The GL_VERSION string begins with a version number.
//   The version number uses one of these forms:
//   - major_number.minor_number
//   - major_number.minor_number.release_number
//   Vendor-specific information may follow the version number. Its format
//   depends on the implementation, but a space always separates the version number
//   and the vendor-specific information.
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::updateOpenGLVersion()
{
    /*
    // Uri, 10/6/09: This returns the EAGL version and not the OpenGL ES version (like calling glXQueryVersion).
    // in EAGL these versions differ (EAGL 1.0 = OGLES 1.1), so we use glGetString even though it is slightly more
    // complicated.
    #ifdef _GR_IPHONE_BUILD
        // glGetString(GL_VERSION) does not work in OpenGL ES (EAGL), use EAGLGetVersion instead:
        unsigned int minor = 0, major = 0;
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_EAGLGetVersion);
        gs_stat_realFunctionPointers.EAGLGetVersion(&major, &minor);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_EAGLGetVersion);
        GT_ASSERT(major > 0);
        _oglVersion[0] = (int)major;
        _oglVersion[1] = (int)minor;
    #endif // _GR_IPHONE_BUILD
    */

    // Get the OpenGL extensions string for the
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetString);
    const char* pOGLVersion = (const char*)gs_stat_realFunctionPointers.glGetString(GL_VERSION);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetString);

    m_openGLVersionString.fromASCIIString(pOGLVersion);

    GT_IF_WITH_ASSERT(!m_openGLVersionString.isEmpty())
    {
        // Look for the first dot (that separates the major and minor version strings):
        int firstDotPos = m_openGLVersionString.find('.');
        GT_IF_WITH_ASSERT(firstDotPos != -1)
        {
            int versionStringStart = 0;
#ifdef _GR_IPHONE_BUILD
            // OpenGL ES 2.0 defines a different string for GL_VERSION, in the format "OpenGL ES #.# <vendor specific information>".
            // This is also (wrongly) implemented in their iPhone OS 3.0 version of OpenGL ES 1.1. Thus, we need to search for spaces before
            // and after the first dot, and not just after:
            int spaceBeforeDot = m_openGLVersionString.reverseFind(' ', firstDotPos);
            GT_IF_WITH_ASSERT(spaceBeforeDot < firstDotPos)
            {
                // Note that if spaceBeforeDot is -1, this is set back to 0:
                versionStringStart = spaceBeforeDot + 1;
            }
#endif
            // Get the major version string:
            gtString majorVersionString;
            m_openGLVersionString.getSubString(versionStringStart, (firstDotPos - 1), majorVersionString);

            // Convert it into an int:
            bool rc1 = majorVersionString.toIntNumber(_oglVersion[0]);
            GT_ASSERT(rc1);

            GT_IF_WITH_ASSERT((firstDotPos + 1) < m_openGLVersionString.length())
            {
                // Get the minor version terminator position:
                // (this terminator can either be '.' , ' ' or NULL)
                int secondDotPos = m_openGLVersionString.find('.', (firstDotPos + 1));

                if (secondDotPos == -1)
                {
                    secondDotPos = m_openGLVersionString.length();
                }

                int spacePos = m_openGLVersionString.find(' ', (firstDotPos + 1));

                if (spacePos == -1)
                {
                    spacePos = m_openGLVersionString.length();
                }

                int minorVerTerminatorPos = secondDotPos;

                if (spacePos < secondDotPos)
                {
                    minorVerTerminatorPos = spacePos;
                }

                // Get the minor version string:
                gtString minorVersionString;
                m_openGLVersionString.getSubString((firstDotPos + 1), (minorVerTerminatorPos - 1),
                                                   minorVersionString);

                // Convert it into an int:
                bool rc2 = minorVersionString.toIntNumber(_oglVersion[1]);
                GT_ASSERT(rc2);
            }
        }
    }

    // If the version is 3.2, OpenGL profiles are supported:
    if ((_oglVersion[0] > 3) || ((_oglVersion[0] == 3) && (_oglVersion[1] >= 2)))
    {
        // If this context is of the compatibility profile, it is a compatibility context
        if (_isCompatibiliyProfile)
        {
            _isCompatibiltyContext = true;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::checkForSoftwareRenderer
// Description:
//  Checks if this render context is rendered using a software renderer.
//  If it is - generates a software renderer detected error.
//
// Author:      Yaki Tebeka
// Date:        2/10/2007
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::checkForSoftwareRenderer()
{
    // Will get true iff this render context is rendered using a software renderer:
    bool isSoftwareRenderer = false;

    GT_IF_WITH_ASSERT(_pPixelFormatDescription != NULL)
    {
        // Get the hardware acceleration support:
        oaPixelFormat::HardwareSupport hardwareSupportMode = _pPixelFormatDescription->hardwareSupport();

        if (hardwareSupportMode != oaPixelFormat::FULL_HARDWARE_ACCELERATION)
        {
            isSoftwareRenderer = true;
        }
    }

    // If this render context is rendered using a software renderer - trigger a detected error:
    if (isSoftwareRenderer)
    {
        gtString errorDescription;
        errorDescription.appendFormattedString(GS_STR_usingSoftwareRenderer, _contextID._contextId, _openGLVendorString.asCharArray(), _openGLRendererString.asCharArray());

        gsOpenGLMonitor& theOGLMtr = gsOpenGLMonitor::instance();
        theOGLMtr.reportDetectedError(AP_USING_SOFTWARE_RENDERER_ERROR, errorDescription);
    }
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::beforeUpdatingContextDataSnapshot
// Description: Is called before a context data snapshot is updated.
//              Notifies relevant clients about this event.
// Author:      Yaki Tebeka
// Date:        9/3/2006
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::beforeUpdatingContextDataSnapshot()
{
    // Call the base context monitor before update:
    suContextMonitor::beforeUpdatingContextDataSnapshot();
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::afterUpdatingContextDataSnapshot
// Description: Is called after a context data snapshot is updated.
//              Notifies relevant clients about this event.
// Author:      Yaki Tebeka
// Date:        9/3/2006
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::afterUpdatingContextDataSnapshot()
{
    // Call the base context monitor event:
    suContextMonitor::afterUpdatingContextDataSnapshot();
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::amountOfGLSupportedTextureUnits
// Description: Returns the amount of texture units that my render
//              context supports.
// Author:      Yaki Tebeka
// Date:        18/4/2005
// Implementation notes
//   TO_DO:
//   The use of this function is wrong, since there are few max texture units:
//   GL_MAX_TEXTURE_UNITS, GL_MAX_TEXTURE_IMAGE_UNITS_ARB, GL_MAX_TEXTURE_COORDS_ARB
//   Until we fix that - we return the maximal amount that OpenGL glext.h supports: 32.
//   The fix should replace this function with few get functions for the different texture
//   limits. Notice that part of the fix is already implemented - see updateTexturesHardwareLimits().
//   For more details, see NVIDIA FAQ: http://developer.nvidia.com/object/General_FAQ.html#t6
//
//   Below is that FAQ content:
//   =========================
//   The value of GL_MAX_TEXTURE_UNITS is 4 for GeForce FX and GeForce 6 Series GPUs. Why is
//   that, since those GPUs have 16 texture units?
//
//   GL_MAX_TEXTURE_UNITS refers to the number of conventional texture units as designed by
//   the old GL_ARB_multitexture extension for which texture coordinate sets and texture image
//   units have to correspond. Now, it doesnt make sense to carry on with this design choice
//   when, as it is the case with the GeForce FX and GeForce 6800 families, the number of
//   texture coordinate sets and the number of texture image units differs, both of them are
//   more than 4, and you can arbitrarily use any texture coordinate to sample any texture
//   image. So, we have intentionally chosen not to further aggrandize conventional texturing
//   with more texture units and have programmers use the GL_ARB_fragment_program and
//   GL_NV_fragment_program extensions when they need to work with more than 4 texture units.
//   Fragment programs are far more general, flexible, efficient, and forward-looking.
//
//   The GL_MAX_TEXTURE_UNITS limit has thus been kept at 4 and the GL_ARB_fragment_program and
//   GL_NV_fragment_program extensions define two new limits:
//    * GL_MAX_TEXTURE_IMAGE_UNITS_ARB/GL_MAX_TEXTURE_IMAGE_UNITS_NV for the number of texture
//      image units, which is 16 for the GeForce FX and GeForce 6800 families,
//    * GL_MAX_TEXTURE_COORDS_ARB/ GL_MAX_TEXTURE_COORDS_NV for the number of texture
//      coordinate sets, which is 8 for the GeForce FX and GeForce 6800 families.
//
//    For GPUs that do not support GL_ARB_fragment_program and GL_NV_fragment_program, those
//    two limits are set equal to GL_MAX_TEXTURE_UNITS.
//
//    Its also important to understand which of those three limits apply to which
//    texture-related states:
//
//    * GL_MAX_TEXTURE_UNITS applies to:
//          o glEnable/glDisable(GL_TEXTURE_xxx)
//          o glTexEnv/glGetTexEnv(GL_TEXTURE_ENV_MODE, ...)
//          o glTexEnv/glGetTexEnviv(GL_TEXTURE_SHADER_NV, ...)
//          o glFinalCombinerInput()
//          o glCombinerInput()
//          o glCombinerOutput()
//    * GL_MAX_TEXTURE_IMAGE_UNITS_ARB/GL_MAX_TEXTURE_IMAGE_UNITS_NV applies to:
//          o glTexImageXXX(...)
//          o glGetTexImage(...)
//          o glTexSubImageXXX(...)
//          o glCopyTexImageXXX(...)
//          o glCopySubTexImageXXX(...)
//          o glTexParameter/glGetTexParameter(...)
//          o glColorTable/glGetColorTable(GL_TEXTURE_xxx/GL_PROXY_TEXTURE_xxx, ...)
//          o glCopyColorTable/glCopyColorSubTable(GL_TEXTURE_xxx/GL_PROXY_TEXTURE_xxx, ...)
//          o glGetColorTableParameter(GL_TEXTURE_xxx/GL_PROXY_TEXTURE_xxx, ...)
//          o glTexEnv/glGetTexEnv(GL_TEXTURE_FILTER_CONTROL_EXT, ...)
//          o glGetTextureLevelParameter(...)
//          o glBindTexture(...)
//    * GL_MAX_TEXTURE_COORDS_ARB/ GL_MAX_TEXTURE_COORDS_NV applies to:
//          o glEnable/glDisable(GL_TEXTURE_GEN_xxx)
//          o glTexGen(...)
//          o glMatrixMode(GL_TEXTURE)/glLoadIdentity/glLoadMatrix/glRotate/...
//          o glPointParameter(GL_POINT_SPRITE_R_MODE_NV, ...)
//          o glTexEnv/glGetTexEnv(GL_POINT_SPRITE_NV, ...)
//          o glClientActiveTexture(...)
//          o glMultTexCoord(...)
//
//  Incidentally, the classification above also tells you that all the calls corresponding
//  to GL_MAX_TEXTURE_UNITS are useless when using fragment programs and actually ignored by
//  the driver (so, making those calls has no adverse performance impact). For example,
//  you don't need to call glEnable/glDisable(GL_TEXTURE_xxx) since the texture target is
//  passed as a parameter to the TEX/TXP/etc instructions called inside the fragment program.
//  Making those calls will actually generate errors if the active texture index is above
//  GL_MAX_TEXTURE_UNITS. glBindProgramARB is the one doing all the work of state configuration
//  and texture enabling/disabling. One glBindProgramARB basically replaces a dozen or more of
//  glActiveTexture, glEnable/glDisable, and glTexEnv calls.
//
// ---------------------------------------------------------------------------
int gsRenderContextMonitor::amountOfGLSupportedTextureUnits() const
{
    int retVal = 32;

    /*
    // Query the amount of supported texture units:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
    gs_stat_realFunctionPointers.glGetIntegerv(GL_MAX_TEXTURE_UNITS, &retVal);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);

    // If multi textures are not supported:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
    GLenum curGLError = gs_stat_realFunctionPointers.glGetError();
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);
    if (curGLError != GL_NO_ERROR)
    {
    // Set the amount of available texture units to 1:
    retVal = 1;
    }

    */

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::disablelVerticalSync
// Description:
//   Disables the vertical sync for the current context.
//   In this mode, the graphic hardware does not wait for the screen to finish
//   it's refresh, enabling us to measure the "real" graphic hardware performance.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/2/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::disablelVerticalSync() const
{
    bool retVal = false;

    // On Windows:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        // If the WGL_EXT_swap_control extension is supported by this render context:
        gsExtensionsManager& theExtensionsMgr = gsExtensionsManager::instance();
        bool isWGL_EXT_swap_controlSupported = theExtensionsMgr.isExtensionSupported(_contextID._contextId, AP_WGL_EXT_swap_control);

        if (isWGL_EXT_swap_controlSupported)
        {
            // Retrieve a pointer to System's implementation of wglSwapIntervalEXT and wglGetSwapIntervalEXT:
            bool rc1 = theExtensionsMgr.getExtensionPointerFromSystem(ap_wglSwapIntervalEXT);
            bool rc2 = theExtensionsMgr.getExtensionPointerFromSystem(ap_wglGetSwapIntervalEXT);
            GT_IF_WITH_ASSERT(rc1 && rc2)
            {
                // Get the extension functions implementations for the current context:
                gsMonitoredFunctionPointers* pExtensionsRealImplementationPointers = theExtensionsMgr.extensionsRealImplementationPointers(_contextID._contextId);
                GT_IF_WITH_ASSERT(pExtensionsRealImplementationPointers != NULL)
                {
                    // Get a pointer to the wglSwapIntervalEXT function:
                    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)(pExtensionsRealImplementationPointers->wglSwapIntervalEXT);
                    GT_IF_WITH_ASSERT(wglSwapIntervalEXT != NULL)
                    {
                        // Cancel vertical sync for this render context:
                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_wglGetSwapIntervalEXT);
                        BOOL rc3 = wglSwapIntervalEXT(0);
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_wglGetSwapIntervalEXT);
                        GT_IF_WITH_ASSERT(rc3 == TRUE)
                        {
                            retVal = true;
                        }
                    }
                }
            }
        }
    }
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
    /// Uri, 8/3/09: //////////////////////////////////////////////////////////
    // On Linux, there are two extensions that have control over the swap rate
    // (effectively replacements for GL_EXT_swap_control): GLX_SGI_swap_control
    // and GLX_MESA_swap_control. Both of them have a glXSwapInterval### function
    // which takes an int and sets the vertical sync to be so that there is no
    // more than one buffers swap per than many screen redraws. For example,
    // glXSwapInterval###(5) on a 50hz monitor would cause a maximum of 10 fps.
    // Thus, we need to call the function with 0 to disable sync.
    //
    // However, glXSwapIntervalSGI(0) returns a GLX_BAD_VALUE (you can only SET
    // the vertical sync, not disable it), and it also seems that on machines
    // with this extension, the vertical sync is normally off. So, we can only
    // use the MESA extension here. On the other hand, the MESA extension seems
    // to be absent from glxext.h files dating at least back to 2005, so we
    // can't use it either. At any rate, it seems vertical sync is not normally
    // on in any of our Linux machines, so we just do nothing for this function
    // other than return true.
    //////////////////////////////////////////////////////////////////////////
    retVal = true;

#elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
    // This only works in CGL:
#ifndef _GR_IPHONE_BUILD
    // Change the context's kCGLCPSwapInterval parameter to 0 (turns off vertical sync):
    GLint newValue = 0;
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_CGLSetParameter);
    CGLError errCode = gs_stat_realFunctionPointers.CGLSetParameter(_renderContextOSHandle, kCGLCPSwapInterval, &newValue);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_CGLSetParameter);
    retVal = (errCode == kCGLNoError);
#endif
#else
#error Unknown Linux variant
#endif
#else
#error Unknown Build target!
#endif

    if (retVal)
    {
        // Output a debug log message:
        gtString dbgMsg;
        dbgMsg.appendFormattedString(GS_STR_disabled_vertical_sync_for_context, _contextID._contextId);
        OS_OUTPUT_DEBUG_LOG(dbgMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::getIntChannelSizeParameterDefaultValue
// Description: If channelStateVarId is an integer parameter (such as GL_xxx_BITS),
//              returns its default value from the default values snapshot.
// Author:      Uri Shomroni
// Date:        23/5/2010
// ---------------------------------------------------------------------------
int gsRenderContextMonitor::getIntChannelSizeParameterDefaultValue(apOpenGLStateVariableId channelStateVarId) const
{
    int retVal = -1;

    // Get the appropriate parameter:
    const apParameter* pVariableValue = NULL;
    bool rcVal = _stateVariablesDefaultValues.getStateVariableValue(channelStateVarId, pVariableValue);
    GT_IF_WITH_ASSERT(rcVal)
    {
        // Channel parameters are always GLint-s:
        GT_IF_WITH_ASSERT(pVariableValue->type() == OS_TOBJ_ID_GL_INT_PARAMETER)
        {
            // Get the value:
            const apGLintParameter* pVariableAsGLIntParameter = (const apGLintParameter*)pVariableValue;
            retVal = (int)pVariableAsGLIntParameter->value();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::addFunctionCall
// Description:
//  Add a function call.
//  * Handles analyze mode function logging
//  * Add the function call to the function call history logger
//
//  Notice - this function is NOT called when in profile mode
//  (However, onMonitoredFunctionCall is called in profile mode)
//
// Arguments: calledFuncitonId - The if of the called function.
//            argumentsAmount - Amount of arguments.
//            pArgumentList - functions arguments list in va list style.
//
// Author:      Sigal Algranaty
// Date:        6/8/2008
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::addFunctionCall(apMonitoredFunctionId calledFunctionId, int argumentsAmount, va_list& pArgumentList, apFunctionDeprecationStatus functionDeprecationStatus)
{
    // Check current execution mode:
    apExecutionMode executionMode = suDebuggedProcessExecutionMode();

    if (executionMode == AP_ANALYZE_MODE)
    {
        // Get the function deprecation status:
        gsDeprecationAnalyzer& deprecationAnalyzer = gsDeprecationAnalyzer::instance();
        deprecationAnalyzer.getFunctionCallDeprecationStatus(this, calledFunctionId, argumentsAmount, pArgumentList, functionDeprecationStatus);
    }

    // Call base class function:
    suContextMonitor::addFunctionCall(calledFunctionId, argumentsAmount, pArgumentList, functionDeprecationStatus);

    // If in "break on deprecated functions mode", break when relevant:
    if ((functionDeprecationStatus != AP_DEPRECATION_NONE) && (su_stat_theBreakpointsManager.breakOnGenericBreakpoint(AP_BREAK_ON_DEPRECATED_FUNCTION)))
    {
        gs_stat_openGLMonitorInstance.triggerBreakpointException(AP_DEPRECATED_FUNCTION_BREAKPOINT_HIT, GL_NO_ERROR, false);
    }

    // Yaki 31/8/2008:
    // Don't perform any action when updating context data snapshot.
    // Example: During context data snapshot update, we call NvAPI_OGL_ExpertModeSet, which calls our wglGetProcAddress,
    //          which calls gs_stat_openGLMonitorInstance.addFunctionCall which call us again, that calls GLExpert again.
    //          This causes a crash in GLExpert!
    if (!isDuringContextDataUpdate())
    {
        _analyzeModeExecutor.addFunctionCall(calledFunctionId, argumentsAmount, pArgumentList);
    }
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onMonitoredFunctionCall
// Description:
//  Is called when a monitored function is called by the debugged application.
//  This function is called also in profile mode.
//
// Author:      Yaki Tebeka
// Date:        7/9/2008
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::onMonitoredFunctionCall()
{
    if (_shouldInitializePerformanceCounters)
    {
        // Notify the performance counters manager:
        _performanceCountersManager.onMonitoredFunctionCall();
    }
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::afterMonitoredFunctionExecutionActions
// Description: Is called after a function call.
//              Handling redundant state changes operations with analyze mode executor.
// Arguments: int calledFunctionIndex
// Return Val: void
// Author:      Sigal Algranaty
// Date:        4/8/2008
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::afterMonitoredFunctionExecutionActions(apMonitoredFunctionId calledFunctionIndex)
{
    // Call the base class implementation:
    suContextMonitor::afterMonitoredFunctionExecutionActions(calledFunctionIndex);

    // Perform analyze mode related actions:
    _analyzeModeExecutor.afterMonitoredFunctionExecutionActions(calledFunctionIndex);
}

// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onMipmapGenerateParamSet
// Description: Logs mipmap generation for direct access functions
// Arguments: GLuint texture
//            GLenum target
//            GLenum pname
//            GLfloat paramValueAsFloat
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/9/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onNamedMipmapGenerateParamSet(GLuint texture, GLenum pname, GLfloat paramValueAsFloat)
{
    bool retVal = false;

    // If the we have a textures monitor:
    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
    {
        // Get the param object that represents the bind texture:
        gsGLTexture* pTexParamObj = _pTexturesMonitor->getTextureObjectDetails(texture);

        if (pTexParamObj)
        {
            retVal = onMipmapGenerateParamSet(pTexParamObj, pname, paramValueAsFloat);
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onMultiTextureMipmapGenerateParamSet
// Description: Logs mipmap generation for multi textures
// Arguments: GLenum texunit
//            GLenum target
//            GLenum pname
//            GLfloat paramValueAsInt
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/9/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onMultiTextureMipmapGenerateParamSet(GLenum texunit, GLenum target, GLenum pname, GLfloat paramValueAsInt)
{
    bool retVal = false;

    // If the we have a textures monitor:
    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
    {
        // Get the requested texture unit index:
        int activeTexUnitIndex = textureUnitNameToIndex(texunit);

        // Check texture unit index range:
        if ((0 <= activeTexUnitIndex) && (activeTexUnitIndex < amountOfTextureUnits()))
        {
            // Get the bound texture object:
            gsGLTexture* pTextureObject = getBoundTexture(activeTexUnitIndex, target);

            if (pTextureObject != NULL)
            {
                retVal = onMipmapGenerateParamSet(pTextureObject, pname, paramValueAsInt);
            }
        }
    }
    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onMipmapGenerateParamSet
// Description: Logs mipmap generation for target bound textures
// Arguments:   GLenum target - the texture target
//              GLenum pname - the parameter enumeration
//              GLfloat paramValueAsInt
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        14/1/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onMipmapGenerateParamSet(GLenum target, GLenum pname, GLfloat paramValueAsFloat)
{
    bool retVal = false;

    // If the we have a textures monitor:
    GT_IF_WITH_ASSERT(_pTexturesMonitor != NULL)
    {
        // Get the param object that represents the bind texture:
        gsGLTexture* pTexParamObj = getBoundTexture(_activeTextureUnitIndex, target);

        if (pTexParamObj)
        {
            retVal = onMipmapGenerateParamSet(pTexParamObj, pname, paramValueAsFloat);
        }
    }
    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::onMipmapGenerateParamSet
// Description: Logs mipmap generation to the input texture object
//              When mipmap generation is changed from 0 -> 1 - generates the automatic
//              mip levels
//              When mipmap generation is changed from 1 -> 0 - destroys the automatic
//              mip levels
// Arguments:   gsGLTexture* pTexParamObj - the texture object
//              GLenum pname - the parameter enumeration
//              GLfloat paramValueAsInt
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/9/2009
// ---------------------------------------------------------------------------
bool gsRenderContextMonitor::onMipmapGenerateParamSet(gsGLTexture* pTexParamObj , GLenum pname, GLfloat paramValueAsFloat)
{
    bool retVal = false;

    // If the we have a textures monitor:
    if (pTexParamObj != NULL)
    {
        switch (pname)
        {
            case GL_GENERATE_MIPMAP:
            {
                bool shouldGenerateMipmap = !(paramValueAsFloat == 0);

                // Set the mipmap auto generation flag:
                pTexParamObj->setMipmapAutoGeneration(shouldGenerateMipmap);

                // Generate the mipmap levels:
                retVal = pTexParamObj->generateAutoMipmapLevels();
                break;
            }

            case GL_TEXTURE_BASE_LEVEL:
            {
                // Convert the float value to gluint:
                GLuint baseLevel = (GLuint)paramValueAsFloat;
                pTexParamObj->setMipmapBaseLevel(baseLevel);
                retVal = true;
                break;
            }

            case GL_TEXTURE_MAX_LEVEL:
            {
                // Convert the float value to gluint:
                GLuint maxLevel = (GLuint)paramValueAsFloat;
                pTexParamObj->setMipmapMaxLevel(maxLevel);
                retVal = true;
                break;
            }

            default:
            {
                break;
            }
        }
    }
    else
    {
        // An error occur:
        // Yaki - I removed the below assert, since it made NVSG stuck.
        // gtAssert(0);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextMonitor::updateVendorType
// Description: Update OS vendor type
// Return Val: void
// Author:      Sigal Algranaty
// Date:        9/2/2010
// ---------------------------------------------------------------------------
void gsRenderContextMonitor::updateVendorType()
{
    gtString vendorTypeLowerCase = _openGLVendorString;
    vendorTypeLowerCase.toLowerCase();

    _vendorType = OA_VENDOR_UNKNOWN;

    // Find one of the renderer types within the vendor string:
    if (vendorTypeLowerCase.startsWith(L"ati"))
    {
        _vendorType = OA_VENDOR_ATI;
    }
    else if (vendorTypeLowerCase.startsWith(L"nvidia"))
    {
        _vendorType = OA_VENDOR_NVIDIA;
    }
    else if (vendorTypeLowerCase.startsWith(L"s3"))
    {
        _vendorType = OA_VENDOR_S3;
    }
    else if (vendorTypeLowerCase.startsWith(L"intel"))
    {
        _vendorType = OA_VENDOR_INTEL;
    }
    else if (vendorTypeLowerCase.startsWith(L"microsoft"))
    {
        _vendorType = OA_VENDOR_MICROSOFT;
    }
    else if (vendorTypeLowerCase.startsWith(L"mesa"))
    {
        _vendorType = OA_VENDOR_MESA;
    }
}

