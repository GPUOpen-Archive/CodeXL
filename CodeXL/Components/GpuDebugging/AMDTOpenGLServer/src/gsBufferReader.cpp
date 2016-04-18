//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsBufferReader.cpp
///
//==================================================================================

//------------------------------ gsBufferReader.cpp ------------------------------

// Standard C:
#include <stdlib.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apGLenumParameter.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsWrappersCommon.h>
#include <src/gsBufferReader.h>
#include <src/gsExtensionsManager.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsOpenGLMonitor.h>

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    typedef void (* PFNGLBINDFRAMEBUFFEROESPROC)(GLenum, GLuint);
#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)
    typedef void (* PFNGLBINDFRAMEBUFFEROESPROC)(GLenum, GLuint);
    #define GL_FRAMEBUFFER_OES 0x8D40
#endif

// ---------------------------------------------------------------------------
// Name:        gsBufferReader::gsBufferReader
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        5/7/2006
// ---------------------------------------------------------------------------
gsBufferReader::gsBufferReader(): suBufferReader(),
    _readBuffer(AP_BACK_BUFFER)
{
}


// ---------------------------------------------------------------------------
// Name:        gsBufferReader::~gsBufferReader
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        5/7/2006
// ---------------------------------------------------------------------------
gsBufferReader::~gsBufferReader()
{
}

// ---------------------------------------------------------------------------
// Name:        gsBufferReader::unbindFBO
// Description: Unbinds the FBO so we can read the static buffers. If the
//              extension is not supported, we needn't do anything here.
// Author:      Uri Shomroni
// Date:        28/6/2009
// ---------------------------------------------------------------------------
void gsBufferReader::unbindFBO()
{
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    PFNGLBINDFRAMEBUFFERPROC pglBindFrameBuffer = gs_stat_realFunctionPointers.glBindFramebuffer;
    PFNGLBINDFRAMEBUFFEREXTPROC pglBindFrameBufferEXT = gs_stat_realFunctionPointers.glBindFramebufferEXT;
    PFNGLBINDFRAMEBUFFEROESPROC pglBindFrameBufferOES = gs_stat_realFunctionPointers.glBindFramebufferOES;
#else
    PFNGLBINDFRAMEBUFFERPROC pglBindFrameBuffer = NULL;
    PFNGLBINDFRAMEBUFFEREXTPROC pglBindFrameBufferEXT = NULL;
    PFNGLBINDFRAMEBUFFEROESPROC pglBindFrameBufferOES = NULL;

    gsMonitoredFunctionPointers* pCurrentContextRealFunctions = gs_stat_extensionsManager.currentRenderContextExtensionsRealImplPointers();
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
        pglBindFrameBuffer(GL_READ_FRAMEBUFFER, 0);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebuffer);
    }
    else if (pglBindFrameBuffer != NULL)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferEXT);
        pglBindFrameBufferEXT(GL_FRAMEBUFFER_EXT, 0);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferEXT);
    }
    else if (pglBindFrameBufferOES != NULL)
    {
        // ap_glBindFramebufferOES is only defined in Mac:
#ifdef ap_glBindFramebufferOES
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferOES);
        pglBindFrameBufferOES(GL_FRAMEBUFFER_OES, 0);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferOES);
#endif
    }
}

// ---------------------------------------------------------------------------
// Name:        gsBufferReader::restoreFBO
// Description: Restores the previously bound FBO after reading the static buffer
// Author:      Uri Shomroni
// Date:        28/6/2009
// ---------------------------------------------------------------------------
void gsBufferReader::restoreFBO()
{
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    PFNGLBINDFRAMEBUFFERPROC pglBindFrameBuffer = gs_stat_realFunctionPointers.glBindFramebuffer;
    PFNGLBINDFRAMEBUFFEREXTPROC pglBindFrameBufferEXT = gs_stat_realFunctionPointers.glBindFramebufferEXT;
    PFNGLBINDFRAMEBUFFEROESPROC pglBindFrameBufferOES = gs_stat_realFunctionPointers.glBindFramebufferOES;
#else
    PFNGLBINDFRAMEBUFFERPROC pglBindFrameBuffer = NULL;
    PFNGLBINDFRAMEBUFFEREXTPROC pglBindFrameBufferEXT = NULL;
    PFNGLBINDFRAMEBUFFEROESPROC pglBindFrameBufferOES = NULL;

    gsMonitoredFunctionPointers* pCurrentContextRealFunctions = gs_stat_extensionsManager.currentRenderContextExtensionsRealImplPointers();
    GT_IF_WITH_ASSERT(pCurrentContextRealFunctions != NULL)
    {
        pglBindFrameBuffer = pCurrentContextRealFunctions->glBindFramebuffer;
        pglBindFrameBufferEXT = pCurrentContextRealFunctions->glBindFramebufferEXT;
        // glBindFrameBufferOES only exists on the Mac implementation of OpenGL ES
    }
#endif

    GLuint activeFBOName = 0;
    gsRenderContextMonitor* pCurrentThreadRCMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();
    GT_IF_WITH_ASSERT(pCurrentThreadRCMonitor != NULL)
    {
        activeFBOName = pCurrentThreadRCMonitor->getActiveReadFboName();
    }

    if (pglBindFrameBuffer != NULL)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindFramebuffer);
        pglBindFrameBuffer(GL_READ_FRAMEBUFFER, activeFBOName);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebuffer);
    }
    else if (pglBindFrameBufferEXT != NULL)
    {
        // TO_DO: Should we handle non-OpenGL 3.0+ with GL_framebuffer_object and GL_framebuffer_blit (where GL_READ_FRAMEBUFFER_EXT is available?)
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferEXT);
        pglBindFrameBufferEXT(GL_FRAMEBUFFER_EXT, activeFBOName);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferEXT);
    }
    else if (pglBindFrameBufferOES != NULL)
    {
        // ap_glBindFramebufferOES is only defined in Mac:
#ifdef ap_glBindFramebufferOES
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferOES);
        pglBindFrameBufferOES(GL_FRAMEBUFFER_OES, activeFBOName);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferOES);
#endif
    }
}

// ---------------------------------------------------------------------------
// Name:        gsTexturesMonitor::isInteractiveBreakOn
// Description: Checks is interactive break mode is enabled.
// Return Val:  bool - True (Active), False (Not Active)
// Author:      Eran Zinman
// Date:        20/8/2007
// ---------------------------------------------------------------------------
bool gsBufferReader::isInteractiveBreakOn() const
{
    // Check if interactive break is on
    const gsOpenGLMonitor& theOpenGLMonitor = gsOpenGLMonitor::instance();
    bool isOn = theOpenGLMonitor.isInteractiveBreakOn();

    return isOn;
}

// ---------------------------------------------------------------------------
// Name:        gsBufferReader::readVBOContent
// Description: Reads a given VBO's data.
// Arguments: GLenum target - the VBO target to read from
//            int offset - The offset from which we want to read the buffer content from
//            unsibufferSize - requested size of data to read
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/4/2009
// ---------------------------------------------------------------------------
#if !((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    bool gsBufferReader::readVBOContent(GLenum target, GLintptr offset, GLsizeiptr bufferSize, PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData)
#else
    bool gsBufferReader::readVBOContent(GLenum target, GLintptr offset, GLsizeiptr bufferSize)
#endif
{
    bool retVal = false;

    // Release previous operation allocated buffer data (if exists):
    releaseBufferData();

    // Allocate the buffer:
    _pReadBufferData = (gtByte*)malloc(bufferSize);
    GT_IF_WITH_ASSERT(_pReadBufferData != NULL)
    {
        GLsizeiptr size = bufferSize;
        GLintptr offsetAsIntPtr(offset);

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
#ifdef _GR_IPHONE_BUILD
        // TO_DO iPhone
#else
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetBufferSubData);
        gs_stat_realFunctionPointers.glGetBufferSubData(target, offsetAsIntPtr, size, _pReadBufferData);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetBufferSubData);
#endif // _GR_IPHONE_BUILD
#else
        glGetBufferSubData(target, offsetAsIntPtr, size, _pReadBufferData);
#endif

        // Check if we generated any errors:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
        GLenum oglError = gs_stat_realFunctionPointers.glGetError();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

        if (oglError == GL_NO_ERROR)
        {
            // No OpenGL error:
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsBufferReader::setActiveReadBuffer
// Description: Sets the active read buffer (if needed)
// Return Val:  bool - Was buffer was set active successfully or not?
// Author:      Eran Zinman
// Date:        18/8/2007
// ---------------------------------------------------------------------------
bool gsBufferReader::setActiveReadBuffer(bool& wasModeChanged)
{
    bool retVal = false;
    wasModeChanged = false;

    // Depth and Stencil buffers always exists, therefore we don't need to set them as active
    if ((_readBuffer == AP_DEPTH_BUFFER) || (_readBuffer == AP_STENCIL_BUFFER) || (_readBuffer == AP_DEPTH_ATTACHMENT_EXT))
    {
        retVal = true;
    }
    else
    {
        // Get the buffer type in openGL format
        GLenum oglBufferType = apColorIndexBufferTypeToGLEnum(_readBuffer);
        GT_IF_WITH_ASSERT(oglBufferType != GL_NONE)
        {
            retVal = true;

            // If interactive mode is enabled, We need to switch back the "Back" and "Front" buffers
            if (isInteractiveBreakOn() && _isDoubleBuffer)
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

            retVal = setOpenGLReadBufferParameter(oglBufferType);
            wasModeChanged = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsBufferReader::readBufferContentFromAPI
// Description: Reads the OpenGL buffers data into _pReadBufferData.
// Return Val:  bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/7/2006
// ---------------------------------------------------------------------------
bool gsBufferReader::readBufferContentFromAPI()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pReadBufferData != NULL)
    {
        // Translate the buffer's data format and component data type to OpenGL types:
        GLenum bufferGLDataFormat = oaTexelDataFormatToGLEnum(_bufferDataFormat);
        GLenum componentGLDataType = oaDataTypeToGLEnum(_componentDataType);
        bool rc1 = ((bufferGLDataFormat != GL_NONE) && (componentGLDataType != GL_NONE));
        GT_IF_WITH_ASSERT(rc1)
        {
            // Set OpenGL pixel pack parameters to match our memory packing:
            setOpenGLPixelPackParameters(1);

            // Clear out any bound FBO:
            unbindFBO();

            // Set the active buffer for reading (if needed)
            bool wasModeChanged = false;
            bool rc2 = setActiveReadBuffer(wasModeChanged);
            GT_IF_WITH_ASSERT(rc2)
            {
                retVal = true;

                // Clear previous OpenGL errors:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
                GLenum oglError = gs_stat_realFunctionPointers.glGetError();
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

                // glReadPixels does not accept GL_DEPTH_COMPONENT##, so we have to convert it to the supported version:
                if ((bufferGLDataFormat == GL_DEPTH_COMPONENT16) || (bufferGLDataFormat == GL_DEPTH_COMPONENT24) || (bufferGLDataFormat == GL_DEPTH_COMPONENT32))
                {
                    bufferGLDataFormat = GL_DEPTH_COMPONENT;
                }

                // Read the buffer content from (0, 0) to (_bufferWidth, _bufferHeight):
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glReadPixels);
                gs_stat_realFunctionPointers.glReadPixels(0, 0, _bufferWidth, _bufferHeight, bufferGLDataFormat, componentGLDataType, _pReadBufferData);
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glReadPixels);

                // Check if we generated any errors:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
                oglError = gs_stat_realFunctionPointers.glGetError();
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

                if (oglError != GL_NO_ERROR)
                {
                    // We generated an OpenGL error:
                    gtString oglEnumAsString;
                    apGLenumValueToString(oglError, oglEnumAsString);

                    // Show an assert message
                    gtString errorMessage = GS_STR_oglError;
                    errorMessage.appendFormattedString(L": %ls", oglEnumAsString.asCharArray());
                    GT_ASSERT_EX(false, errorMessage.asCharArray());

                    retVal = false;
                }
            }

            // If we changed the GL_READ_BUFFER value:
            if (wasModeChanged)
            {
                // Restore the read buffer:
                bool rcReadRest = restoreOpenGLReadBufferParameter();
                GT_ASSERT(rcReadRest);
            }

            // Restore OpenGL pixel pack parameters to their original status:
            restoreOpenGLPixelPackParameters();
        }
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gsBufferReader::readBufferContent
// Description: Reads a given buffer's data.
// Arguments:   readBuffer - The buffer to be read.
//              bufferWidth, bufferHeight - The buffer dimensions
//              bufferDataFormat - The buffer's data format.
//              componentDataType - The type of a single data component.
//              isDoubleBuffer - is the application double buffered
// Return Val:  bool  - Success / failure.
// Author:      Eran Zinman
// Date:        23/1/2008
// ---------------------------------------------------------------------------
bool gsBufferReader::readBufferContent(apDisplayBuffer readBuffer, int bufferWidth, int bufferHeight, oaTexelDataFormat bufferDataFormat, oaDataType componentDataType, bool isDoubleBuffer)
{
    bool retVal = false;

    _readBuffer = readBuffer;
    _isDoubleBuffer = isDoubleBuffer;

    // Call the base class implementation:
    retVal = suBufferReader::readBufferContent(bufferWidth, bufferHeight, 1, 0, bufferDataFormat, componentDataType);

    return retVal;
}
