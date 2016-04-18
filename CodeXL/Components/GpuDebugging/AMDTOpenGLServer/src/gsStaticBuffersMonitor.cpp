//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsStaticBuffersMonitor.cpp
///
//==================================================================================

//------------------------------ gsStaticBuffersMonitor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

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
#include <src/gsStaticBuffersMonitor.h>
#include <src/gsBufferSerializer.h>

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    typedef void (* PFNGLBINDFRAMEBUFFEROESPROC)(GLenum, GLuint);
#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)
    typedef void (* PFNGLBINDFRAMEBUFFEROESPROC)(GLenum, GLuint);
    #define GL_FRAMEBUFFER_OES 0x8D40
#endif

// ---------------------------------------------------------------------------
// Name:        gsStaticBuffersMonitor::gsStaticBuffersMonitor
// Description: Constructor
// Author:      Eran Zinman
// Date:        24/8/2007
// ---------------------------------------------------------------------------
gsStaticBuffersMonitor::gsStaticBuffersMonitor(int spyContextId)
    : _spyContextId(spyContextId), _pRenderContextPixelFormat(NULL), _activeReadBuffer(GL_NONE), _areFramebufferSizeQueriesSupported(true)
{

}

// ---------------------------------------------------------------------------
// Name:        gsStaticBuffersMonitor::~gsStaticBuffersMonitor
// Description: Destructor
// Author:      Eran Zinman
// Date:        24/8/2007
// ---------------------------------------------------------------------------
gsStaticBuffersMonitor::~gsStaticBuffersMonitor()
{
    // Clear all the static buffers from the static buffer vector
    clearAllBuffers();
}

// ---------------------------------------------------------------------------
// Name:        gsStaticBuffersMonitor::::onContextDeletion
// Description: Is called before the render context is deleted.
// Author:      Eran Zinman
// Date:        25/8/2007
// ---------------------------------------------------------------------------
void gsStaticBuffersMonitor::onContextDeletion()
{
    // Clear all the static buffers from the static buffer vector
    clearAllBuffers();
}

// ---------------------------------------------------------------------------
// Name:        gsStaticBuffersMonitor::onFirstTimeContextMadeCurrent
// Description: Called the first time a context was made current
// Author:      Uri Shomroni
// Date:        5/1/2010
// ---------------------------------------------------------------------------
void gsStaticBuffersMonitor::onFirstTimeContextMadeCurrent(const int contextOGLVersion[2])
{
    if ((contextOGLVersion[0] > 3) || ((contextOGLVersion[0] == 3) && (contextOGLVersion[1] > 0)))
    {
        // OpenGL 3.1 and higher do not support framebuffer size queries:
        _areFramebufferSizeQueriesSupported = false;
    }
    else
    {
        _areFramebufferSizeQueriesSupported = true;
    }
}

// ---------------------------------------------------------------------------
// Name:        gsStaticBuffersMonitor::amountOfStaticBuffers
// Description: Returns amount of the static buffers in the current render context
// Return Val:  Amount of static buffer
// Author:      Eran Zinman
// Date:        25/8/2007
// ---------------------------------------------------------------------------
int gsStaticBuffersMonitor::amountOfStaticBuffers() const
{
    // Get amount of static buffers
    int staticBuffersAmount = (int)_staticBuffers.size();

    return staticBuffersAmount;
}

// ---------------------------------------------------------------------------
// Name:        gsStaticBuffersMonitor::clearContextDataSnapshot
// Description: Clears the context data snapshot.
// Author:      Eran Zinman
// Date:        25/8/2007
// ---------------------------------------------------------------------------
void gsStaticBuffersMonitor::clearContextDataSnapshot()
{
    // Clear all the static buffers from the static buffer vector
    clearAllBuffers();
}

// ---------------------------------------------------------------------------
// Name:        gsStaticBuffersMonitor::clearAllBuffers
// Description: Clear all the static buffers from the static buffer vector
// Author:      Eran Zinman
// Date:        26/1/2008
// ---------------------------------------------------------------------------
void gsStaticBuffersMonitor::clearAllBuffers()
{
    // Delete and clear the static buffers vector
    _staticBuffers.deleteElementsAndClear();
}

// ---------------------------------------------------------------------------
// Name:        gsStaticBuffersMonitor::updateBufferData
// Description: Update the raw data of a given static buffer.
// Arguments: staticBufferId - The API id of the buffer to be updated.
// Return Val:  bool - Success / Failure
// Author:      Yaki Tebeka
// Date:        16/10/2007
// ---------------------------------------------------------------------------
bool gsStaticBuffersMonitor::updateBufferRawData(apDisplayBuffer bufferType)
{
    bool retVal = false;

    // Get the queried buffer:
    apStaticBuffer* pStaticBuffer = getStaticBufferObjectDetails(bufferType);
    GT_IF_WITH_ASSERT(pStaticBuffer != NULL)
    {
        // Update its data:
        retVal = updateBufferRawData(*pStaticBuffer);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStaticBuffersMonitor::generateBufferFilePath
// Description: Generates the buffer file path
// Arguments:   bufferType - The buffer type
//              bufferFilePath - The output buffer file path.
// Return Val:  bool - Filename was generated successfully (true), otherwise (false)
// Author:      Eran Zinman
// Date:        7/8/2007
// ---------------------------------------------------------------------------
bool gsStaticBuffersMonitor::generateBufferFilePath(apDisplayBuffer bufferType, osFilePath& bufferFilePath) const
{
    bool retVal = false;

    // Convert the buffer type into a string describing the buffer
    gtString bufferNameCode;
    bool rc = apGetBufferNameCode(bufferType, bufferNameCode);
    GT_IF_WITH_ASSERT(rc)
    {
        // Build the buffer log file name:
        gtString logFileName;
        logFileName.appendFormattedString(GS_STR_staticBufferFilePath, _spyContextId, bufferNameCode.asCharArray());

        // Set the log file path:
        bufferFilePath = suCurrentSessionLogFilesDirectory();

        // Set the file name
        bufferFilePath.setFileName(logFileName);
        bufferFilePath.setFileExtension(SU_STR_rawFileExtension);
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::updateStaticBuffer
// Description: Update an individual static buffer
// Arguments:   bufferType - The buffer type
// Author:      Eran Zinman
// Date:        28/8/2007
// ---------------------------------------------------------------------------
void gsStaticBuffersMonitor::updateStaticBuffer(apDisplayBuffer bufferType)
{
    // Add the static buffer to the buffers vector
    apStaticBuffer* pStaticBuffer = new apStaticBuffer;
    GT_IF_WITH_ASSERT(pStaticBuffer != NULL)
    {
        // Store the buffers type:
        pStaticBuffer->setBufferType(bufferType);

        // Generate the buffer file name:
        osFilePath bufferFilePath;
        bool rc1 = generateBufferFilePath(bufferType, bufferFilePath);
        GT_IF_WITH_ASSERT(rc1)
        {
            // Set the buffer file name
            pStaticBuffer->setBufferFilePath(bufferFilePath);

            // Gives this object its Render context's ID so we will know to ask for its stack:
            const gsOpenGLMonitor& theOpenGLMonitor = gsOpenGLMonitor::instance();
            int allocatedID = -1;
            const gsRenderContextMonitor* pRCMon = theOpenGLMonitor.renderContextMonitor(_spyContextId);
            GT_IF_WITH_ASSERT(pRCMon != NULL)
            {
                allocatedID = pRCMon->allocatedObjectId();
            }

            pStaticBuffer->setAllocatedObjectId(allocatedID);

            // Add the static buffer to the static buffers vector
            _staticBuffers.push_back(pStaticBuffer);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsStaticBuffersMonitor::updateBufferRawData
// Description: Updates a given buffer's raw data.
// Arguments:   staticBuffer - An object representing the buffer.
// Return Val:  bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        16/10/2007
// ---------------------------------------------------------------------------
bool gsStaticBuffersMonitor::updateBufferRawData(apStaticBuffer& staticBuffer)
{
    bool retVal = false;

    // Get the static buffer type:
    apDisplayBuffer staticBufferType = staticBuffer.bufferType();

    // Update buffer only if it exist:
    if (isBufferExists(staticBufferType))
    {
        // Get buffer output file path
        osFilePath bufferFilePath;
        staticBuffer.getBufferFilePath(bufferFilePath);

        // Check if the application is double buffered:
        bool isDoubleBuffer = false;

        if (_pRenderContextPixelFormat != NULL)
        {
            isDoubleBuffer = _pRenderContextPixelFormat->isDoubleBuffered();
        }

        // Save the buffer content to a file:
        gsBufferSerializer bufferSerializer;
        bool rc2 = bufferSerializer.saveBufferToFile(staticBuffer, bufferFilePath, isDoubleBuffer);
        GT_IF_WITH_ASSERT(rc2)
        {
            // Set the static buffer file path
            staticBuffer.setBufferFilePath(bufferFilePath);

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

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStaticBuffersMonitor::changeActiveFBOInAPI
// Description: Manually changes the active FBO in the OpenGL API without changing
//              this class's or gsRenderContextMonitor's members.
// Author:      Uri Shomroni
// Date:        12/5/2010
// ---------------------------------------------------------------------------
void gsStaticBuffersMonitor::changeActiveFBOInAPI(GLuint fboToBind) const
{
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    PFNGLBINDFRAMEBUFFERPROC pglBindFrameBuffer = gs_stat_realFunctionPointers.glBindFramebuffer;
    PFNGLBINDFRAMEBUFFEREXTPROC pglBindFrameBufferEXT = gs_stat_realFunctionPointers.glBindFramebufferEXT;
    PFNGLBINDFRAMEBUFFEROESPROC pglBindFrameBufferOES = gs_stat_realFunctionPointers.glBindFramebufferOES;
#else
    PFNGLBINDFRAMEBUFFERPROC pglBindFrameBuffer = NULL;
    PFNGLBINDFRAMEBUFFEREXTPROC pglBindFrameBufferEXT = NULL;
    PFNGLBINDFRAMEBUFFEROESPROC pglBindFrameBufferOES = NULL;

    gsMonitoredFunctionPointers* pCurrentContextRealFunctions = gs_stat_extensionsManager.extensionsRealImplementationPointers(_spyContextId);
    GT_IF_WITH_ASSERT(pCurrentContextRealFunctions != NULL)
    {
        pglBindFrameBuffer = pCurrentContextRealFunctions->glBindFramebuffer;
        pglBindFrameBufferEXT = pCurrentContextRealFunctions->glBindFramebufferEXT;
        // glBindFrameBufferOES only exists on the Mac implementation of OpenGL ES
    }
#endif

    if (pglBindFrameBuffer != NULL)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindFramebuffer);
        pglBindFrameBuffer(GL_READ_FRAMEBUFFER, fboToBind);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebuffer);
    }
    else if (pglBindFrameBufferEXT != NULL)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferEXT);
        pglBindFrameBufferEXT(GL_FRAMEBUFFER_EXT, fboToBind);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferEXT);
    }
    else if (pglBindFrameBufferOES != NULL)
    {
        // ap_glBindFramebufferOES is only defined in Mac:
#ifdef ap_glBindFramebufferOES
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferOES);
        pglBindFrameBufferOES(GL_FRAMEBUFFER_OES, fboToBind);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferOES);
#endif
    }
}

// ---------------------------------------------------------------------------
// Name:        gsStaticBuffersMonitor::getStaticBufferObjectDetails
// Description: Return the apStaticBuffer object linked with the bufferType
// Arguments:   bufferType - The buffer type to return it's details
// Return Val:  Pointer to the matching apStaticBuffer object
// Author:      Eran Zinman
// Date:        27/8/2008
// ---------------------------------------------------------------------------
apStaticBuffer* gsStaticBuffersMonitor::getStaticBufferObjectDetails(apDisplayBuffer bufferType) const
{
    apStaticBuffer* pStaticBuffer = NULL;

    // Get static buffers vector size
    int amountOfIndicies = (int)_staticBuffers.size();

    for (int i = 0; i < amountOfIndicies; i++)
    {
        if (_staticBuffers[i])
        {
            // Is this the buffer we were looking for?
            if (_staticBuffers[i]->bufferType() == bufferType)
            {
                // Item found, break the loop
                pStaticBuffer = _staticBuffers[i];
                break;
            }
        }
    }

    return pStaticBuffer;
}


// ---------------------------------------------------------------------------
// Name:        gsStaticBuffersMonitor::getStaticBufferObjectDetails
// Description: Return the apStaticBuffer object linked with the bufferType
// Arguments:   bufferID - The buffer ID (ID is from the buffers vector) to
//              return the details about
// Return Val:  Pointer to the matching apStaticBuffer object
// Author:      Eran Zinman
// Date:        27/8/2008
// ---------------------------------------------------------------------------
apStaticBuffer* gsStaticBuffersMonitor::getStaticBufferObjectDetails(int bufferID) const
{
    apStaticBuffer* pStaticBuffer = NULL;

    // Get static buffers vector size
    int amountOfIndicies = (int)_staticBuffers.size();

    // Range check:
    if ((bufferID >= 0) && (bufferID < amountOfIndicies))
    {
        pStaticBuffer = _staticBuffers[bufferID];
    }

    return pStaticBuffer;
}

// ---------------------------------------------------------------------------
// Name:        gsStaticBuffersMonitor::updateAllStaticBuffers
// Description: Updates all the static buffers in the current context
// Author:      Eran Zinman
// Date:        27/8/2008
// ---------------------------------------------------------------------------
void gsStaticBuffersMonitor::updateAllStaticBuffers(bool canQueryContext)
{
    // Clear all the static buffers from the static buffer vector:
    clearAllBuffers();

    gtVector<apDisplayBuffer> availableStaticBuffers;

    // Get a list of static buffers according to the pixel format:
    if (_pRenderContextPixelFormat != NULL)
    {
        // When we have pixel format information - derive the available buffers from the pixel format:
        if (_pRenderContextPixelFormat->amountOfZBufferBits() > 0)
        {
            availableStaticBuffers.push_back(AP_DEPTH_BUFFER);
        }

        if (_pRenderContextPixelFormat->amountOfStencilBufferBits() > 0)
        {
            availableStaticBuffers.push_back(AP_STENCIL_BUFFER);
        }

        if ((_pRenderContextPixelFormat->amountOfColorBits() > 0) && (!_pRenderContextPixelFormat->isDoubleBuffered()))
        {
            // Single buffer:
            availableStaticBuffers.push_back(AP_FRONT_BUFFER);
        }
        else if ((_pRenderContextPixelFormat->amountOfColorBits() > 0) && (_pRenderContextPixelFormat->isDoubleBuffered()))
        {
            // Double buffer:
            availableStaticBuffers.push_back(AP_BACK_BUFFER);
            availableStaticBuffers.push_back(AP_FRONT_BUFFER);
        }
    }
    else
    {
        // No pixel format information - default buffers:
        availableStaticBuffers.push_back(AP_DEPTH_BUFFER);
        availableStaticBuffers.push_back(AP_STENCIL_BUFFER);
        availableStaticBuffers.push_back(AP_BACK_BUFFER);
        availableStaticBuffers.push_back(AP_FRONT_BUFFER);
    }

    // Update buffers according to pixel format:
    for (int i = 0; i < (int)availableStaticBuffers.size(); i++)
    {
        updateStaticBuffer(availableStaticBuffers[i]);
    }

    if (canQueryContext)
    {
        // If a framebuffer object is bound, unbind it before attempting to get the AUX. buffers amount:
        GLuint formerlyActiveFBOName = 0;
        const gsRenderContextMonitor* pRCMon = gs_stat_openGLMonitorInstance.renderContextMonitor(_spyContextId);
        GT_IF_WITH_ASSERT(pRCMon != NULL)
        {
            formerlyActiveFBOName = pRCMon->getActiveReadFboName();
        }

        if (formerlyActiveFBOName != 0)
        {
            // Unbind the FBO:
            changeActiveFBOInAPI(0);
        }

        // Update all "Auxiliary" buffers [1..4]:
        // Clear previous openGL errors:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
        GLenum oglError = gs_stat_realFunctionPointers.glGetError();

        // Get number of auxiliary buffers
        GLint auxiliaryBuffersNum = 0;
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
        gs_stat_realFunctionPointers.glGetIntegerv(GL_AUX_BUFFERS, &auxiliaryBuffersNum);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);

        // Check that we didn't generate any errors
        oglError = gs_stat_realFunctionPointers.glGetError();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

        if (formerlyActiveFBOName != 0)
        {
            // Unbind the FBO:
            changeActiveFBOInAPI(formerlyActiveFBOName);
        }

        if (oglError == GL_NO_ERROR)
        {
            // We don't know how to handle more than 4 auxiliary buffers:
            bool rc1 = ((auxiliaryBuffersNum >= 0) && (auxiliaryBuffersNum <= 4));
            GT_IF_WITH_ASSERT(rc1)
            {
                // Loop through all the auxiliary buffers:
                for (int i = 0; i < auxiliaryBuffersNum; i++)
                {
                    // Get buffer enumeration id
                    apDisplayBuffer bufferType = (apDisplayBuffer)((int)AP_AUX0_BUFFER + i);

                    // If the buffer exist:
                    if (isBufferExists(bufferType))
                    {
                        // Update buffer:
                        updateStaticBuffer(bufferType);
                    }
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gsStaticBuffersMonitor::updateContextDataSnapshot
// Description: Updates the context data snapshot.
// Author:      Eran Zinman
// Date:        27/8/2008
// ---------------------------------------------------------------------------
void gsStaticBuffersMonitor::updateContextDataSnapshot(bool canQueryContext)
{
    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateBuffersDataStarted, OS_DEBUG_LOG_DEBUG);

    // Update all of the context static buffer:
    updateAllStaticBuffers(canQueryContext);

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_UpdateBuffersDataEnded, OS_DEBUG_LOG_DEBUG);
}

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::storeCurrentlyActiveBuffer
// Description: Store the currently active buffer
// Author:      Eran Zinman
// Date:        28/8/2007
// ---------------------------------------------------------------------------
void gsStaticBuffersMonitor::storeCurrentlyActiveBuffer()
{
#ifdef _GR_IPHONE_BUILD
    // TO_DO iPhone
#else
    GT_IF_WITH_ASSERT(_activeReadBuffer == GL_NONE)
    {
        // Store the current "Read_Buffer" parameters
        GLint activeReadBufferAsGLInt = GL_NONE;
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
        gs_stat_realFunctionPointers.glGetIntegerv(GL_READ_BUFFER, &activeReadBufferAsGLInt);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
        _activeReadBuffer = (GLenum)activeReadBufferAsGLInt;
    }
#endif
}

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::restoreCurrentlyActiveBuffer
// Description: Restore the currently active buffer
// Author:      Eran Zinman
// Date:        28/8/2007
// ---------------------------------------------------------------------------
void gsStaticBuffersMonitor::restoreCurrentlyActiveBuffer()
{
#ifdef _GR_IPHONE_BUILD
    // TO_DO iPhone
#else
    GT_IF_WITH_ASSERT(_activeReadBuffer != GL_NONE)
    {
        // Restore the current "Read_Buffer" parameters
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glReadBuffer);
        gs_stat_realFunctionPointers.glReadBuffer(_activeReadBuffer);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glReadBuffer);

        _activeReadBuffer = GL_NONE;
    }
#endif
}

// ---------------------------------------------------------------------------
// Name:        gsStaticBuffersMonitor::isInteractiveBreakOn
// Description: Checks if interactive break mode is enabled.
// Return Val:  bool - True (Active), False (Not Active)
// Author:      Eran Zinman
// Date:        26/8/2007
// ---------------------------------------------------------------------------
bool gsStaticBuffersMonitor::isInteractiveBreakOn() const
{
    const gsOpenGLMonitor& theOpenGLMonitor = gsOpenGLMonitor::instance();
    return theOpenGLMonitor.isInteractiveBreakOn();
}

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::setBufferAsActive
// Description: Sets a color index buffer as the active buffer
// Arguments:   bufferType - The buffer type
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        28/8/2007
// ---------------------------------------------------------------------------
bool gsStaticBuffersMonitor::setBufferAsActive(apDisplayBuffer bufferType)
{
    bool retVal = true;

    // Get the buffer type in openGL format. If we got an GL_NONE result,
    // the buffer is not a color index buffer, so we don't need to make it active
    GLenum oglBufferType = apColorIndexBufferTypeToGLEnum(bufferType);

    if (oglBufferType != GL_NONE)
    {
        // Check if the application is double buffered:
        bool isDoubleBuffered = false;

        if (_pRenderContextPixelFormat != NULL)
        {
            isDoubleBuffered = _pRenderContextPixelFormat->isDoubleBuffered();
        }

        // If interactive mode is enabled, We need to switch between the "Back Buffer" and the "Front Buffer"
        // (only on doubled buffer applications):
        if (isInteractiveBreakOn() && isDoubleBuffered)
        {
            if (oglBufferType == GL_FRONT)
            {
                oglBufferType = GL_BACK;
            }
            else if (oglBufferType == GL_BACK)
            {
                oglBufferType = GL_FRONT;
            }
        }

        // Clear previous openGL errors:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
        GLenum previousOGLError = gs_stat_realFunctionPointers.glGetError();
        GT_ASSERT(previousOGLError == GL_NO_ERROR);

#ifdef _GR_IPHONE_BUILD
        // TO_DO iPhone
#else
        // Set the buffer as active
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glReadBuffer);
        gs_stat_realFunctionPointers.glReadBuffer(oglBufferType);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glReadBuffer);
#endif

        // Check if we generated errors:
        GLenum oglError = gs_stat_realFunctionPointers.glGetError();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

        if (oglError != GL_NO_ERROR)
        {
            retVal = false;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::isStencilBufferExists
// Description: Checks if a specific buffer exists or not
// Arguments:   bufferType - The buffer type
// Return Val:  bool  - True (Exists), otherwise False.
// Author:      Eran Zinman
// Date:        20/8/2007
// ---------------------------------------------------------------------------
bool gsStaticBuffersMonitor::isBufferExists(apDisplayBuffer bufferType)
{
    bool bufferExists = false;

    // Save the currently active buffer:
    storeCurrentlyActiveBuffer();

    bool rcSet = setBufferAsActive(bufferType);

    // Check if we can set the buffer to be the active buffer (if exists)
    if (rcSet)
    {
        // Depth and Stencil buffers, always exist. Let's check their bit depth
        if (_areFramebufferSizeQueriesSupported && (bufferType == AP_STENCIL_BUFFER || bufferType == AP_DEPTH_BUFFER))
        {
            // Set the buffer number of bits query command.
            GLenum bufferBitsQuery = GL_COLOR_INDEX;

            switch (bufferType)
            {
                case AP_STENCIL_BUFFER:
                    bufferBitsQuery = GL_STENCIL_BITS;
                    break;

                case AP_DEPTH_BUFFER:
                    bufferBitsQuery = GL_DEPTH_BITS;
                    break;

                default:
                {
                    bufferExists = false;
                    GT_ASSERT_EX(false, L"Unsupported buffer type");
                }
                break;
            }

            // Get the buffer number of bits per pixel
            GLint bufferBits = 0;
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
            gs_stat_realFunctionPointers.glGetIntegerv(bufferBitsQuery, &bufferBits);
            SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);

            // If bit depth is largest than 0, then the buffer exist
            if (bufferBits > 0)
            {
                bufferExists = true;
            }
        }
        else
        {
            // For other buffer (other than depth, stencil), if they pass the "SetBufferActive" test, they exist
            bufferExists = true;
        }
    }

    // Restore the currently active buffer:
    restoreCurrentlyActiveBuffer();

    return bufferExists;
}


// ---------------------------------------------------------------------------
// Name:        gsStaticBuffersMonitor::getCurrentThreadHDCSize
// Description: Calculates the current thread HDC size.
// Arguments:   bufferWidth - Output buffer width
//              bufferHeight - Output buffer height
// Return Val:  bool  - Success / failure.
// Author:      Eran Zinman
// Date:        15/8/2007
// ---------------------------------------------------------------------------
bool gsStaticBuffersMonitor::getCurrentThreadHDCSize(int& hdcWidth, int& hdcHeight)
{
    bool retVal = false;

    // Create a 4 integers array to hold (x, y, width, height) parameters
    GLint viewPort[4] = { 0 };

    // Get the view port width and height
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
    gs_stat_realFunctionPointers.glGetIntegerv(GL_VIEWPORT, viewPort);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT) && (AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE))
    // Uri, 17/9/09: Mac 64-bit has a problem where glGetIntergerv takes a GLint (= long = 8 bytes) pointer as a parameter, but the output is
    // as int (= 4 bytes). This causes "x" to contain x + (y << 32), "y" to contain width + (height << 32), and "width" and "height" to contain
    // garbage data. So - we cast the pointer to an int* to overcome this:
    GLsizei width = ((int*)viewPort)[2];    // Width
    GLsizei height = ((int*)viewPort)[3];   // Height
#else
    // Retrieve view port width and height
    //GLint x = viewPort[0];        // X coordinate (not used)
    //GLint y = viewPort[1];        // Y coordinate (not used)
    GLsizei width = viewPort[2];  // Width
    GLsizei height = viewPort[3]; // Height
#endif

    // Calculate the hdc width and height
    hdcWidth = width;
    hdcHeight = height;

    // Did we get reasonable values?
    GT_ASSERT(0 < hdcWidth);
    GT_ASSERT(0 < hdcHeight);

    if ((0 < hdcWidth) && (0 < hdcHeight))
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsStaticBuffersMonitor::updateStaticBuffersDimensions
// Description: Updates all existing static buffer with the current thread HDC size
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        19/10/2008
// ---------------------------------------------------------------------------
bool gsStaticBuffersMonitor::updateStaticBuffersDimensions()
{
    bool retVal = true;

    apStaticBuffer* pStaticBuffer = NULL;

    // Get static buffers vector size
    int amountOfIndices  = (int)_staticBuffers.size();

    if (amountOfIndices > 0)
    {
        // Get the size of the hDC which is currently linked with current graphic context
        int hdcWidth = 0;
        int hdcHeight = 0;
        retVal = getCurrentThreadHDCSize(hdcWidth, hdcHeight);

        // Iterate the static buffers and update the dimensions:
        for (int i = 0; i < amountOfIndices ; i++)
        {
            pStaticBuffer = _staticBuffers[i];

            if (pStaticBuffer != NULL)
            {
                // Set the current static buffer's dimensions:
                pStaticBuffer->setBufferDimensions(hdcWidth, hdcHeight);

                // Get buffer data format and data format type:
                oaTexelDataFormat bufferDataFormat = OA_TEXEL_FORMAT_UNKNOWN;
                oaDataType componentDataType;
                bool rcType = getBufferDataFormatAndDataType(pStaticBuffer->bufferType(), bufferDataFormat, componentDataType);
                GT_IF_WITH_ASSERT(rcType)
                {
                    pStaticBuffer->setBufferDataFormat(bufferDataFormat, componentDataType);
                }

                retVal = retVal && rcType;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsStaticBuffersMonitor::getBufferDataFormatAndDataType
// Description: For a given buffer type, return the buffer data format
//              and component data type
// Arguments:   bufferType - The buffer type to get the formats from
//              bufferDataFormat - Output buffer data format
//              componentDataType - Out buffer component data type
// Return Val:  Success / Failure
// Author:      Eran Zinman
// Date:        11/8/2007
// ---------------------------------------------------------------------------
bool gsStaticBuffersMonitor::getBufferDataFormatAndDataType(apDisplayBuffer bufferType, oaTexelDataFormat& bufferDataFormat, oaDataType& componentDataType)
{
    bool retVal = true;

    bufferDataFormat = OA_TEXEL_FORMAT_UNKNOWN;

    switch (bufferType)
    {
        case AP_DEPTH_BUFFER:
        {
            // The format and data type in which we retrieve the depth buffer data:
            bufferDataFormat = OA_TEXEL_FORMAT_DEPTH;
            componentDataType = OA_FLOAT;
        }
        break;

        case AP_STENCIL_BUFFER:
        {
            // The format and data type in which we retrieve the stencil buffer data:
            bufferDataFormat = OA_TEXEL_FORMAT_STENCIL;
            componentDataType = OA_UNSIGNED_BYTE;
        }
        break;

        case AP_BACK_BUFFER:
        case AP_FRONT_BUFFER:
        case AP_AUX0_BUFFER:
        case AP_AUX1_BUFFER:
        case AP_AUX2_BUFFER:
        case AP_AUX3_BUFFER:
        {
            // The format and data type in which we retrieve the front buffer data:
            bufferDataFormat = OA_TEXEL_FORMAT_RGB;
            componentDataType = OA_UNSIGNED_BYTE;
        }
        break;

        case AP_COLOR_ATTACHMENT0_EXT:
        case AP_COLOR_ATTACHMENT1_EXT:
        case AP_COLOR_ATTACHMENT2_EXT:
        case AP_COLOR_ATTACHMENT3_EXT:
        case AP_COLOR_ATTACHMENT4_EXT:
        case AP_COLOR_ATTACHMENT5_EXT:
        case AP_COLOR_ATTACHMENT6_EXT:
        case AP_COLOR_ATTACHMENT7_EXT:
        case AP_COLOR_ATTACHMENT8_EXT:
        case AP_COLOR_ATTACHMENT9_EXT:
        case AP_COLOR_ATTACHMENT10_EXT:
        case AP_COLOR_ATTACHMENT11_EXT:
        case AP_COLOR_ATTACHMENT12_EXT:
        case AP_COLOR_ATTACHMENT13_EXT:
        case AP_COLOR_ATTACHMENT14_EXT:
        case AP_COLOR_ATTACHMENT15_EXT:
        {
            // The format and data type in which we retrieve the front buffer data:
            bufferDataFormat = OA_TEXEL_FORMAT_RGB;
            componentDataType = OA_UNSIGNED_BYTE;
        }
        break;

        case AP_DEPTH_ATTACHMENT_EXT:
        {
            // The format and data type in which we retrieve the depth buffer data:
            bufferDataFormat = OA_TEXEL_FORMAT_DEPTH_EXT;
            //bufferDataFormat = GL_DEPTH_ATTACHMENT_EXT;
            componentDataType = OA_FLOAT;
        }
        break;

        case AP_STENCIL_ATTACHMENT_EXT:
        {
            // The format and data type in which we retrieve the stencil buffer data:
            bufferDataFormat = OA_TEXEL_FORMAT_STENCIL;
            componentDataType = OA_UNSIGNED_BYTE;
        }
        break;

        default:
        {
            GT_ASSERT_EX(false, L"Buffer type is not supported!");
            retVal = false;
        }
        break;
    }

#ifdef _GR_IPHONE_BUILD
    {
        // On the iPhone, the implementation only allows certain format and type values:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
        GLint allowedFormat = GL_RGBA;
        gs_stat_realFunctionPointers.glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES, &allowedFormat);
        GLint allowedType = GL_UNSIGNED_INT;
        gs_stat_realFunctionPointers.glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE_OES, &allowedType);
        bool rcFormat = oaGLEnumToTexelDataFormat((GLenum)allowedFormat, bufferDataFormat);
        bool rcType = oaGLEnumToDataType((GLenum)allowedType, componentDataType);
        GT_ASSERT(rcFormat && rcType);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
    }
#endif

    return retVal;
}
