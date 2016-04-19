//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsRenderBuffersMonitor.cpp
///
//==================================================================================

//------------------------------ gsRenderBuffersMonitor.cpp ------------------------------

#include <math.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/apDefinitions.h>
#include <AMDTAPIClasses/Include/apDefaultTextureNames.h>
#include <AMDTAPIClasses/Include/ap2DRectangle.h>

// Spy Utilities:
#include <AMDTServerUtilities/Include/suStringConstants.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsWrappersCommon.h>
#include <src/gsExtensionsManager.h>
#include <src/gsRenderContextMonitor.h>
#include <src/gsTextureSerializer.h>
#include <src/gsRenderBuffersMonitor.h>
#include <src/gsBufferSerializer.h>

// ---------------------------------------------------------------------------
// Name:        gsRenderBuffersMonitor::gsRenderBuffersMonitor
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
gsRenderBuffersMonitor::gsRenderBuffersMonitor(int spyContextId)
    : _spyContextId(spyContextId),
      _amountOfRenderBufferObjects(0),
      m_isOpenGL3FBOSupported(false),
      m_isFrameBufferExtSupported(false),
      _dummyFBOName(0),
      m_glGenFramebuffers(nullptr),
      m_glBindFramebuffer(nullptr),
      m_glFramebufferRenderbuffer(nullptr),
      m_glDeleteFramebuffers(nullptr),
      m_glGenFramebuffersEXT(nullptr),
      m_glBindFramebufferEXT(nullptr),
      m_glFramebufferRenderbufferEXT(nullptr),
      m_glDeleteFramebuffersEXT(nullptr)
{
}

// ---------------------------------------------------------------------------
// Name:        gsRenderBuffersMonitor::~gsRenderBuffersMonitor
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
gsRenderBuffersMonitor::~gsRenderBuffersMonitor()
{
    // Delete the texture wrappers vector:
    int amountOfRenderBuffers = (int)_renderBuffers.size();

    for (int i = 0; i < amountOfRenderBuffers; i++)
    {
        delete _renderBuffers[i];
        _renderBuffers[i] = NULL;
    }

    _renderBuffers.clear();
}

// ---------------------------------------------------------------------------
// Name:        gsRenderBuffersMonitor::onFirstTimeContextMadeCurrent
// Description: Is called on the first time in which my context is made the
//              current context.
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
void gsRenderBuffersMonitor::onFirstTimeContextMadeCurrent()
{
#ifndef _GR_IPHONE_BUILD
    // Initalize OpenGL 3.0 function pointers:
    m_glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)gsGetSystemsOGLModuleProcAddress("glGenFramebuffers");
    m_glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)gsGetSystemsOGLModuleProcAddress("glBindFramebuffer");
    m_glFramebufferRenderbuffer = (PFNGLFRAMEBUFFERRENDERBUFFERPROC)gsGetSystemsOGLModuleProcAddress("glFramebufferRenderbuffer");
    m_glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)gsGetSystemsOGLModuleProcAddress("glDeleteFramebuffers");
    m_isOpenGL3FBOSupported = ((nullptr != m_glBindFramebuffer) && (nullptr != m_glGenFramebuffers) &&
                               (nullptr != m_glFramebufferRenderbuffer) && (nullptr != m_glDeleteFramebuffers));

    // Initialize GL_frame_buffer_EXT function pointers:
    m_glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)gsGetSystemsOGLModuleProcAddress("glGenFramebuffersEXT");
    m_glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)gsGetSystemsOGLModuleProcAddress("glBindFramebufferEXT");
    m_glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)gsGetSystemsOGLModuleProcAddress("glFramebufferRenderbufferEXT");
    m_glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)gsGetSystemsOGLModuleProcAddress("glDeleteFramebuffersEXT");
    // Verify that we managed to get all function pointers:
    m_isFrameBufferExtSupported = ((nullptr != m_glBindFramebufferEXT) && (nullptr != m_glGenFramebuffersEXT) &&
                                   (nullptr != m_glFramebufferRenderbufferEXT) && (nullptr != m_glDeleteFramebuffersEXT));

    // Also initialize these two pointers if they exist, for use in the gsBufferReader:
    gsGetSystemsOGLModuleProcAddress("glGenFramebuffers");
    gsGetSystemsOGLModuleProcAddress("glGenFramebuffersOES");
#else
    m_isFrameBufferExtSupported = ((gs_stat_realFunctionPointers.glBindFramebufferOES != NULL) &&
                                   (gs_stat_realFunctionPointers.glGenFramebuffersOES != NULL) &&
                                   (gs_stat_realFunctionPointers.glFramebufferRenderbufferOES != NULL) &&
                                   (gs_stat_realFunctionPointers.glDeleteFramebuffersOES != NULL));
#endif

    if (!(m_isOpenGL3FBOSupported || m_isFrameBufferExtSupported))
    {
        OS_OUTPUT_DEBUG_LOG(L"Frame buffer extension is not supported", OS_DEBUG_LOG_DEBUG);
    }
}

// ---------------------------------------------------------------------------
// Name:        gsRenderBuffersMonitor::onContextDeletion
// Description: Is called before the render context is deleted.
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
void gsRenderBuffersMonitor::onContextDeletion()
{
}

// ---------------------------------------------------------------------------
// Name:        gsRenderBuffersMonitor::onRenderBufferObjectsGeneration
// Description: Is called when render buffer objects are generated.
// Arguments:   amountOfGeneratedRenderBuffers - The amount of generated render buffer objects.
//              renderBufferNames - An array, containing the names of the generated render buffers
//                             objects.
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
void gsRenderBuffersMonitor::onRenderBufferObjectsGeneration(GLsizei amountOfGeneratedRenderBuffers, GLuint* renderBufferNames)
{
    // If the monitored context is NOT the NULL context:
    GT_IF_WITH_ASSERT((AP_NULL_CONTEXT_ID != _spyContextId) && (NULL != renderBufferNames) && (0 < amountOfGeneratedRenderBuffers))
    {
        gtVector<apAllocatedObject*> renderbuffersForAllocationMonitor;

        for (int i = 0; i < amountOfGeneratedRenderBuffers; i++)
        {
            // Get the name of the currently created render buffer object:
            GLuint newRenderBufferName = renderBufferNames[i];

            // Create a render buffer object monitor for the created texture:
            apGLRenderBuffer* pCurrentRBO = createRenderBufferObjectMonitor(newRenderBufferName);
            GT_IF_WITH_ASSERT(NULL != pCurrentRBO)
            {
                renderbuffersForAllocationMonitor.push_back(pCurrentRBO);
            }
        }

        // Register these objects in the allocated objects monitor:
        su_stat_theAllocatedObjectsMonitor.registerAllocatedObjects(renderbuffersForAllocationMonitor);
    }
}


// ---------------------------------------------------------------------------
// Name:        gsRenderBuffersMonitor::updateContextDataSnapshot
// Description: Updates the context data snapshot.
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
void gsRenderBuffersMonitor::updateContextDataSnapshot()
{
    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateRenderBufferDataStarted, OS_DEBUG_LOG_DEBUG);

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateRenderBufferDataEnded, OS_DEBUG_LOG_DEBUG);
}


// ---------------------------------------------------------------------------
// Name:        gsStateVariablesSnapshot::clearContextDataSnapshot
// Description:
// Author:      Sigal Algranaty
// Date:        3/6/2008
// ---------------------------------------------------------------------------
void gsRenderBuffersMonitor::clearContextDataSnapshot()
{
}

// ---------------------------------------------------------------------------
// Name:        gsRenderBuffersMonitor::getRenderBufferObjectDetails
// Description: Returns a queried render buffer object details.
// Arguments:   renderBufferName - The OpenGL name of the texture.
// Return Val:  const gsGLRenderBuffer* - Will get the render buffer details, or NULL if a
//                                   render buffer of this name does not exist.
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
apGLRenderBuffer* gsRenderBuffersMonitor::getRenderBufferObjectDetails(GLuint renderBufferName) const
{
    apGLRenderBuffer* retVal = NULL;

    // Get the id of the input texture object monitor:
    int monitorObjIndex = getRenderBufferObjMonitorIndex(renderBufferName);

    if (monitorObjIndex != -1)
    {
        // Return the render buffer object details:
        retVal = _renderBuffers[monitorObjIndex];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderBufferMonitor::getRenderBufferObjectName
// Description: Inputs a render buffer id (in this context) and returns its OpenGL name.
// Arguments:   contextId - The context in which the texture resides.
//              renderBufferObjIndex - The index of the texture in this context.
//              renderBufferName - The OpenGL name of the render buffer.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
bool gsRenderBuffersMonitor::getRenderBufferObjectName(int renderBufferObjIndex, GLuint& renderBufferName) const
{
    bool retVal = false;

    // Sanity test:
    int amountOfRenderBuffersIndices = (int)_renderBuffers.size();

    if ((0 <= renderBufferObjIndex) && (renderBufferObjIndex < amountOfRenderBuffersIndices))
    {
        // If there are no free indices:
        if (_freeRenderBufferIndices.empty())
        {
            // The queried texture index is the _textures array index:
            renderBufferName = _renderBuffers[renderBufferObjIndex]->renderBufferName();
            retVal = true;
        }
        else
        {
            // We have free indices - we need to count "allocated" indices:
            int amountOfAllocatedIndices = 0;
            int currentIndex = 0;

            while (currentIndex < amountOfRenderBuffersIndices)
            {
                // If our allocated indices count reached the queried id:
                if (amountOfAllocatedIndices == renderBufferObjIndex)
                {
                    // If the current texture index is allocated -
                    // we found the index that match the requested id:
                    if (_renderBuffers[currentIndex] != NULL)
                    {
                        // Output the texture name:
                        renderBufferName = _renderBuffers[currentIndex]->renderBufferName();
                        retVal = true;
                        break;
                    }
                }
                else
                {
                    // If we didn't reach the queried id yet.
                    // If the current index is allocated:
                    if (_renderBuffers[currentIndex] != NULL)
                    {
                        amountOfAllocatedIndices++;
                    }
                }

                currentIndex++;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::createRenderBufferObjectMonitor
// Description: Creates a render buffer object monitor, initializes it and registers
//              it in this class vectors.
// Arguments: renderBufferObjectName - The OpenGL name of the render buffer that the
//                                created render buffer monitor will monitor.
//
// Return Val: apGLRenderBuffer*  - The created render buffer monitor (Or NULL in case of failure).
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
apGLRenderBuffer* gsRenderBuffersMonitor::createRenderBufferObjectMonitor(GLuint renderBufferObjectName)
{
    apGLRenderBuffer* retVal = NULL;

    // Create the texture object monitor:
    apGLRenderBuffer* pNewRenderBufferMtr = new apGLRenderBuffer(renderBufferObjectName);
    GT_IF_WITH_ASSERT(pNewRenderBufferMtr != NULL)
    {
        // Add the render buffer object to the _renderBuffers vector:
        // ----------------------------------------------

        // Will contain the render buffer index in the _renderBuffers vector:
        int newRenderBufferIndex = 0;

        // If there is already a free index - use it:
        if (!(_freeRenderBufferIndices.empty()))
        {
            // Get the free index:
            newRenderBufferIndex = _freeRenderBufferIndices.back();
            _freeRenderBufferIndices.pop_back();

            // Put the texture object at this index:
            _renderBuffers[newRenderBufferIndex] = pNewRenderBufferMtr;
        }
        else
        {
            // There is currently no free index - we will allocate a new index:
            newRenderBufferIndex = (int)_renderBuffers.size();
            _renderBuffers.push_back(pNewRenderBufferMtr);
        }

        // Add the monitor index to the _renderBufferOpenGLNameToIndex map:
        _renderBufferOpenGLNameToIndex[renderBufferObjectName] = newRenderBufferIndex;

        // Increment the allocated textures amount:
        _amountOfRenderBufferObjects++;

        // Return the created texture object:
        retVal = pNewRenderBufferMtr;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderBuffersMonitor::getRenderBufferObjMonitorIndex
// Description: Inputs an OpenGL render buffer name and outputs its monitor's location (index)
//              in the _renderBuffers array, or -1 if it does not exist.
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
int gsRenderBuffersMonitor::getRenderBufferObjMonitorIndex(GLuint renderBufferName) const
{
    int retVal = -1;

    gtMap<GLuint, int>::const_iterator endIter = _renderBufferOpenGLNameToIndex.end();
    gtMap<GLuint, int>::const_iterator iter = _renderBufferOpenGLNameToIndex.find(renderBufferName);

    if (iter != endIter)
    {
        retVal = (*iter).second;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderBuffersMonitor::onRenderBufferObjectsDeletion
// Description: Is called when render buffer objects are deleted.
// Arguments:   amountOfDeletedTextures - The amount of deleted texture objects.
//              textureNames - An array, containing the names of the render buffer objects
//                             to be deleted.
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
void gsRenderBuffersMonitor::onRenderBufferObjectsDeletion(GLsizei amountOfDeletedRenderBuffers, const GLuint* renderBufferNames)
{
    // If the monitored context is NOT the NULL context:
    GT_IF_WITH_ASSERT(_spyContextId != AP_NULL_CONTEXT_ID)
    {
        for (int i = 0; i < amountOfDeletedRenderBuffers; i++)
        {
            // Delete the input texture param:
            GLuint currentRenderBufferName = renderBufferNames[i];

            // Ignore render buffer 0:
            if (currentRenderBufferName != 0)
            {
                // Get the texture object monitor index:
                int monitorObjIndex = getRenderBufferObjMonitorIndex(currentRenderBufferName);

                // If the user is trying to delete a non existing texture:
                if (monitorObjIndex == -1)
                {
                    // Display an error message to the user -
                    gtString errorMessage = GS_STR_deletingNonExistingRenderBuffer;
                    errorMessage.appendFormattedString(L" %u", currentRenderBufferName);
                    osOutputDebugString(errorMessage);
                }
                else
                {
                    // Get the object that represents the texture that is going to be deleted:
                    apGLRenderBuffer* pRenderBufferToBeDeleted = _renderBuffers[monitorObjIndex];
                    GT_IF_WITH_ASSERT(pRenderBufferToBeDeleted != NULL)
                    {
                        // Delete the render buffer param object:
                        delete pRenderBufferToBeDeleted;
                    }

                    // Remove it from the textures array and map:
                    _renderBuffers[monitorObjIndex] = NULL;
                    _renderBufferOpenGLNameToIndex[currentRenderBufferName] = -1;

                    // Decrement the allocated textures amount:
                    _amountOfRenderBufferObjects--;

                    // Store the empty _textures index:
                    _freeRenderBufferIndices.push_back(monitorObjIndex);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsRenderBuffersMonitor::generateRenderBufferFilePath
// Description: Generates a render buffer file path.
// Arguments:   bufferName - The render buffer OpenGL name.
//              bufferFilePath - The output texture file path.
// Author:      Sigal Algranaty
// Date:        27/5/2008
// ---------------------------------------------------------------------------
void gsRenderBuffersMonitor::generateRenderBufferFilePath(GLuint bufferName, osFilePath& bufferFilePath) const
{
    // Build the log file name:
    gtString logFileName;
    logFileName.appendFormattedString(GS_STR_renderBufferFilePath, _spyContextId, bufferName);

    // Set the log file path:
    bufferFilePath = suCurrentSessionLogFilesDirectory();
    bufferFilePath.setFileName(logFileName);

    // Set the log file extension:
    bufferFilePath.setFileExtension(SU_STR_rawFileExtension);
}

// ---------------------------------------------------------------------------
// Name:        gsRenderBuffersMonitor::updateRenderBufferRawData
// Description: Updates a given render buffer raw data file.
// Arguments:   pRenderBufferObject - The render buffer object to be updated.
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/5/2008
// ---------------------------------------------------------------------------
bool gsRenderBuffersMonitor::updateRenderBufferRawData(apGLRenderBuffer* pRenderBufferObject, GLuint currentlyActiveFBO)
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(pRenderBufferObject != NULL)
    {
        // Check if this render buffer is connected to an FBO object:
        bool rc1 = (pRenderBufferObject->getBufferType() != AP_DISPLAY_BUFFER_UNKNOWN);
        GT_IF_WITH_ASSERT(rc1)
        {
            bool rc2 = (pRenderBufferObject->getFBOName() != 0);
            GT_IF_WITH_ASSERT(rc2)
            {
                // If the render buffer is "dirty":
                if (pRenderBufferObject->isDirty())
                {
                    // If this is a GL-CL interop buffer, print a warning message to the log, since we might
                    // run into undefined behaviour if the buffer is acquired by OpenCL:
                    int clImageIndex = -1;
                    int clImageName = -1;
                    int clContextId = -1;
                    pRenderBufferObject->getCLImageDetails(clImageIndex, clImageName, clContextId);

                    if (clImageName > -1)
                    {
                        OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_aboutToReadCLObject, OS_DEBUG_LOG_INFO);
                    }

                    // Get the render buffer object name:
                    GLuint renderBufferName = pRenderBufferObject->renderBufferName();

                    // Will be used for the buffer file name:
                    GLuint renderBufferNameForFileName = renderBufferName;

                    // Make sure that the render buffer is bounded to the active FBO:
                    bool needToRestoreFBO = true;
                    rc1 = activateRenderBufferFBO(pRenderBufferObject, currentlyActiveFBO, needToRestoreFBO);

                    GT_IF_WITH_ASSERT(rc1)
                    {
                        // Generate the texture file name:
                        osFilePath bufferFilePath;
                        generateRenderBufferFilePath(renderBufferNameForFileName, bufferFilePath);

                        // Set the buffer file path:
                        pRenderBufferObject->setBufferFilePath(bufferFilePath);

                        // Save the buffer content to a file:
                        gsBufferSerializer bufferSerializer;
                        rc2 = bufferSerializer.saveBufferToFile(*pRenderBufferObject, bufferFilePath);
                        GT_IF_WITH_ASSERT(rc2)
                        {
                            retVal = true;
                        }

                        if (!retVal)
                        {
                            // Raise an assertion failure:
                            // TO_DO: Move me to string table:
                            gtString errorMessage = L"Failed to save buffer to a file: ";
                            errorMessage += bufferFilePath.asString();
                            GT_ASSERT_EX(false, errorMessage.asCharArray());
                        }
                    }

                    rc2 = restoreCurretlyActiveFBO(pRenderBufferObject, currentlyActiveFBO, needToRestoreFBO);
                    GT_ASSERT(rc2);
                    retVal = retVal && rc1 && rc2;
                }
            }
        }
        else
        {
            // The buffer is not connect to an FBO object:
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderBuffersMonitor::activateRenderBufferFBO
// Description: The function activates the render buffer FBO, and saves the currently active FBO, if there is
//              Check which FBO is currently active:
//              If the currently active FBO is the same FBO as the render buffer's FBO, do nothing.
//              If the currently active FBO is some other FBO, deactivate it and activate the render buffer FBO
// Arguments: apGLRenderBuffer* pRenderBufferObject - the render buffer
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/5/2008
// ---------------------------------------------------------------------------
bool gsRenderBuffersMonitor::activateRenderBufferFBO(apGLRenderBuffer* pRenderBufferObject, GLuint currentlyActiveFBO, bool& needToRestoreFBO)
{
    bool retVal = false;

    if (m_isOpenGL3FBOSupported || m_isFrameBufferExtSupported)
    {
        // If the render buffer doesn't have an FBO attached, create a dummy one, that later would be destroyed:
        GLuint renderBufferFBOName = pRenderBufferObject->getFBOName();

        if (renderBufferFBOName == 0)
        {
            // Create dummy FBO:
#ifdef _GR_IPHONE_BUILD
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGenFramebuffersOES);
            gs_stat_realFunctionPointers.glGenFramebuffersOES(1, &_dummyFBOName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGenFramebuffersOES);
#else

            if (m_isOpenGL3FBOSupported)
            {
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGenFramebuffers);
                m_glGenFramebuffers(1, &_dummyFBOName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGenFramebuffers);
            }
            else // m_isFrameBufferExtSupported
            {
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGenFramebuffersEXT);
                m_glGenFramebuffersEXT(1, &_dummyFBOName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGenFramebuffersEXT);
            }

#endif

            // Bind the created FBO:
#ifdef _GR_IPHONE_BUILD
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferOES);
            gs_stat_realFunctionPointers.glBindFramebufferOES(GL_FRAMEBUFFER_OES, _dummyFBOName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferOES);
#else

            if (m_isOpenGL3FBOSupported)
            {
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindFramebuffer);
                m_glBindFramebuffer(GL_READ_FRAMEBUFFER, _dummyFBOName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebuffer);
            }
            else // m_isFrameBufferExtSupported
            {
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferEXT);
                m_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _dummyFBOName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferEXT);
            }

#endif

            // Get the render buffer display type, and convert it to OpenGL enum:
            GLenum openGLBufferDisplayType = apColorIndexBufferTypeToGLEnum(pRenderBufferObject->getBufferType());

            // Attach the render buffer to the FBO:
            GLuint rboName = pRenderBufferObject->bufferName();
#ifdef _GR_IPHONE_BUILD
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glFramebufferRenderbufferOES);
            gs_stat_realFunctionPointers.glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, openGLBufferDisplayType, GL_RENDERBUFFER_OES, rboName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glFramebufferRenderbufferOES);
#else

            if (m_isOpenGL3FBOSupported)
            {
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glFramebufferRenderbuffer);
                m_glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, openGLBufferDisplayType, GL_RENDERBUFFER, rboName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glFramebufferRenderbuffer);
            }
            else // m_isFrameBufferExtSupported
            {
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glFramebufferRenderbufferEXT);
                m_glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, openGLBufferDisplayType, GL_RENDERBUFFER_EXT, rboName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glFramebufferRenderbufferEXT);
            }

#endif
            retVal = true;
        }

        // If the render buffer is already attached to the currently attached FBO, do nothing:
        else if (renderBufferFBOName == currentlyActiveFBO)
        {
            retVal = true;
            needToRestoreFBO = false;
        }

        // The Render buffer it attached to an FBO, but that FBO is not currently bound, so bind it:
        else // renderBufferFBOName != currentlyActiveFBO, 0
        {
            // Bind the render buffer FBO:
#ifdef _GR_IPHONE_BUILD
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferOES);
            gs_stat_realFunctionPointers.glBindFramebufferOES(GL_FRAMEBUFFER_OES, renderBufferFBOName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferOES);
#else

            if (m_isOpenGL3FBOSupported)
            {
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindFramebuffer);
                m_glBindFramebuffer(GL_READ_FRAMEBUFFER, renderBufferFBOName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebuffer);
            }
            else // m_isFrameBufferExtSupported
            {
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferEXT);
                m_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, renderBufferFBOName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferEXT);
            }

#endif
            retVal = true;
            needToRestoreFBO = true;
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gsRenderBuffersMonitor::restoreCurretlyActiveFBO
// Description: The function restores FBO according to what it was before the render buffer was saved to file
// Arguments: apGLRenderBuffer* pRenderBufferObject
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/5/2008
// ---------------------------------------------------------------------------
bool gsRenderBuffersMonitor::restoreCurretlyActiveFBO(apGLRenderBuffer* pRenderBufferObject, GLuint currentlyActiveFBO, bool needToRestoreFBO)
{
    (void)(pRenderBufferObject); // unused
    bool retVal = false;

    if (m_isOpenGL3FBOSupported || m_isFrameBufferExtSupported)
    {
        // If we need to restore the bound FBO (or no FBO was bound and we need to restore that state):
        if (needToRestoreFBO)
        {
            // Restore the active FBO / unbind the FBO:
#ifdef _GR_IPHONE_BUILD
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferOES);
            gs_stat_realFunctionPointers.glBindFramebufferOES(GL_FRAMEBUFFER_OES, currentlyActiveFBO);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferOES);
#else

            if (m_isOpenGL3FBOSupported)
            {
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindFramebuffer);
                m_glBindFramebuffer(GL_READ_FRAMEBUFFER, currentlyActiveFBO);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebuffer);
            }
            else // m_isFrameBufferExtSupported
            {
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferEXT);
                m_glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, currentlyActiveFBO);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferEXT);
            }

#endif
        }

        if (_dummyFBOName != 0)
        {
            // Delete the dummy FBO
#ifdef _GR_IPHONE_BUILD
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDeleteFramebuffersOES);
            gs_stat_realFunctionPointers.glDeleteFramebuffersOES(1, &_dummyFBOName);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDeleteFramebuffersOES);
#else

            if (m_isOpenGL3FBOSupported)
            {
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDeleteFramebuffers);
                m_glDeleteFramebuffers(1, &_dummyFBOName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDeleteFramebuffers);
            }
            else // m_isFrameBufferExtSupported
            {
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDeleteFramebuffersEXT);
                m_glDeleteFramebuffersEXT(1, &_dummyFBOName);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glDeleteFramebuffersEXT);
            }

#endif
        }

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsRenderBuffersMonitor::calculateBuffersMemorySize
// Description: Calculate the current existing buffers memory size
// Arguments:   gtUInt64& buffersMemorySize
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/7/2010
// ---------------------------------------------------------------------------
bool gsRenderBuffersMonitor::calculateBuffersMemorySize(gtUInt64& buffersMemorySize) const
{
    bool retVal = true;

    buffersMemorySize = 0;

    // Iterate the buffers:
    for (int i = 0; i < (int) _renderBuffers.size(); i++)
    {
        // Get the current render buffer object:
        apGLRenderBuffer* pRenderBuffer = _renderBuffers[i];

        if (pRenderBuffer != NULL)
        {

            // Get the calculated render buffer size:
            gtSize_t currentBufferMemorySize = 0;
            bool rc = pRenderBuffer->calculateMemorySize(currentBufferMemorySize);
            GT_IF_WITH_ASSERT(rc)
            {
                // Set the object size in KB (ceil the number for 0.1KB to turn to be 1KB):
                float val = (float)currentBufferMemorySize / (float)(1024 * 8);
                buffersMemorySize += (gtUInt32)ceil(val);
            }
        }
    }

    return retVal;
}
