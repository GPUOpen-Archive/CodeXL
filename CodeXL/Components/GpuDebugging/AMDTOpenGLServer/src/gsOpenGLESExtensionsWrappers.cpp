//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsOpenGLESExtensionsWrappers.cpp
///
//==================================================================================

//------------------------------ gsOpenGLESExtensionsWrappers.cpp ------------------------------

// ---------------------------------------------------------------------------
// File:
//  This file contains OpenGL ES extension functions implementations.
// ---------------------------------------------------------------------------

// OpenGL:
#include <AMDTOSWrappers/osOpenGLIncludes.h>

// Local:
#include <inc/gsStringConstants.h>
#include <inc/gsWrappersCommon.h>
#include <inc/gsExtensionsManager.h>
#include <inc/gsOpenGLMonitor.h>
#include <inc/gsOpenGLESAidFunctions.h>

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// TO_DO iPhone, Uri 26/5/09: All the mac (iPhone) implementations in this
// file are with gs_stat_realFunctionPointers instead of using
// SU_CALL_EXTENSION_FUNC[_WITH_RET_VAL], since that mechanism seems to fail
// here for some reason. While the iPhone is supposed to automatically
// support all these functions, we cannot presume all extension functions
// are supported all the time (or we should not mark them as extensions)
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// OES_draw_texture
//////////////////////////////////////////////////////////////////////////

void GL_APIENTRY glDrawTexsOES(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height)
{
    SU_START_FUNCTION_WRAPPER(ap_glDrawTexsOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDrawTexsOES, 5, OS_TOBJ_ID_GL_SHORT_PARAMETER, x, OS_TOBJ_ID_GL_SHORT_PARAMETER, y, OS_TOBJ_ID_GL_SHORT_PARAMETER, z, OS_TOBJ_ID_GL_SHORT_PARAMETER, width, OS_TOBJ_ID_GL_SHORT_PARAMETER, height);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Call the float function implementation:
    gsDrawTexfOES(GLfloat(x), GLfloat(y), GLfloat(z), GLfloat(width), GLfloat(height));

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glDrawTexsOES(x, y, z, width, height);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glDrawTexsOES);
}


void GL_APIENTRY glDrawTexiOES(GLint x, GLint y, GLint z, GLint width, GLint height)
{
    SU_START_FUNCTION_WRAPPER(ap_glDrawTexiOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDrawTexiOES, 5, OS_TOBJ_ID_GL_INT_PARAMETER, x, OS_TOBJ_ID_GL_INT_PARAMETER, y, OS_TOBJ_ID_GL_INT_PARAMETER, z, OS_TOBJ_ID_GL_INT_PARAMETER, width, OS_TOBJ_ID_GL_INT_PARAMETER, height);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Call the float function implementation:
    gsDrawTexfOES(GLfloat(x), GLfloat(y), GLfloat(z), GLfloat(width), GLfloat(height));

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glDrawTexiOES(x, y, z, width, height);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glDrawTexiOES);
}


void GL_APIENTRY glDrawTexxOES(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height)
{
    SU_START_FUNCTION_WRAPPER(ap_glDrawTexxOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDrawTexxOES, 5, OS_TOBJ_ID_GL_FIXED_PARAMETER, x, OS_TOBJ_ID_GL_FIXED_PARAMETER, y, OS_TOBJ_ID_GL_FIXED_PARAMETER, z, OS_TOBJ_ID_GL_FIXED_PARAMETER, width, OS_TOBJ_ID_GL_FIXED_PARAMETER, height);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate from fixed to float arguments:
    GLfloat xAsFloat = apGLfixedParameter::fixedToFloat(x);
    GLfloat yAsFloat = apGLfixedParameter::fixedToFloat(y);
    GLfloat zAsFloat = apGLfixedParameter::fixedToFloat(z);
    GLfloat widthAsFloat = apGLfixedParameter::fixedToFloat(width);
    GLfloat heightAsFloat = apGLfixedParameter::fixedToFloat(height);

    // Call the float function implementation:
    gsDrawTexfOES(xAsFloat, yAsFloat, zAsFloat, widthAsFloat, heightAsFloat);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glDrawTexxOES(x, y, z, width, height);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glDrawTexxOES);
}


void GL_APIENTRY glDrawTexsvOES(const GLshort* coords)
{
    SU_START_FUNCTION_WRAPPER(ap_glDrawTexsvOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDrawTexsvOES, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_SHORT_PARAMETER, 5, coords);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Call the float function implementation:
    gsDrawTexfOES(GLfloat(coords[0]), GLfloat(coords[1]), GLfloat(coords[2]), GLfloat(coords[3]), GLfloat(coords[4]));

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glDrawTexsvOES(coords);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glDrawTexsvOES);
}


void GL_APIENTRY glDrawTexivOES(const GLint* coords)
{
    SU_START_FUNCTION_WRAPPER(ap_glDrawTexivOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDrawTexivOES, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 5, coords);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Call the float function implementation:
    gsDrawTexfOES(GLfloat(coords[0]), GLfloat(coords[1]), GLfloat(coords[2]), GLfloat(coords[3]), GLfloat(coords[4]));

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glDrawTexivOES(coords);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glDrawTexivOES);
}


void GL_APIENTRY glDrawTexxvOES(const GLfixed* coords)
{
    SU_START_FUNCTION_WRAPPER(ap_glDrawTexxvOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDrawTexxvOES, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FIXED_PARAMETER, 5, coords);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate from fixed to float arguments:
    GLfloat xAsFloat = apGLfixedParameter::fixedToFloat(coords[0]);
    GLfloat yAsFloat = apGLfixedParameter::fixedToFloat(coords[1]);
    GLfloat zAsFloat = apGLfixedParameter::fixedToFloat(coords[2]);
    GLfloat widthAsFloat = apGLfixedParameter::fixedToFloat(coords[3]);
    GLfloat heightAsFloat = apGLfixedParameter::fixedToFloat(coords[4]);

    // Call the float function implementation:
    gsDrawTexfOES(xAsFloat, yAsFloat, zAsFloat, widthAsFloat, heightAsFloat);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glDrawTexxvOES(coords);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glDrawTexxvOES);
}


void GL_APIENTRY glDrawTexfOES(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height)
{
    SU_START_FUNCTION_WRAPPER(ap_glDrawTexfOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDrawTexfOES, 5, OS_TOBJ_ID_GL_FLOAT_PARAMETER, x, OS_TOBJ_ID_GL_FLOAT_PARAMETER, y, OS_TOBJ_ID_GL_FLOAT_PARAMETER, z, OS_TOBJ_ID_GL_FLOAT_PARAMETER, width, OS_TOBJ_ID_GL_FLOAT_PARAMETER, height);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Call the function implementation:
    gsDrawTexfOES(x, y, z, width, height);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glDrawTexfOES(x, y, z, width, height);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glDrawTexfOES);
}


void GL_APIENTRY glDrawTexfvOES(const GLfloat* coords)
{
    SU_START_FUNCTION_WRAPPER(ap_glDrawTexfvOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDrawTexfvOES, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 5, coords);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Call the function implementation:
    gsDrawTexfOES(coords[0], coords[1], coords[2], coords[3], coords[4]);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glDrawTexfvOES(coords);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glDrawTexfvOES);
}

// These functions are MAC only (iPhone extensions):
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

//////////////////////////////////////////////////////////////////////////
// OES_blend_subtract
//////////////////////////////////////////////////////////////////////////

void GL_APIENTRY glBlendEquationOES(GLenum mode)
{
    SU_START_FUNCTION_WRAPPER(ap_glBlendEquationOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glBlendEquationOES, 1, OS_TOBJ_ID_GL_ENUM_PARAMETER, mode);

    // Call the real function:
    gs_stat_realFunctionPointers.glBlendEquationOES(mode);

    SU_END_FUNCTION_WRAPPER(ap_glBlendEquationOES);
}

//////////////////////////////////////////////////////////////////////////
// OES_framebuffer_object
//////////////////////////////////////////////////////////////////////////

GLboolean GL_APIENTRY glIsRenderbufferOES(GLuint renderbuffer)
{
    GLboolean retVal = GL_FALSE;

    SU_START_FUNCTION_WRAPPER(ap_glIsRenderbufferOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glIsRenderbufferOES, 1, OS_TOBJ_ID_GL_UINT_PARAMETER, renderbuffer);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.glIsRenderbufferOES(renderbuffer);

    SU_END_FUNCTION_WRAPPER(ap_glIsRenderbufferOES);

    return retVal;
}

void GL_APIENTRY glBindRenderbufferOES(GLenum target, GLuint renderbuffer)
{
    SU_START_FUNCTION_WRAPPER(ap_glBindRenderbufferOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glBindRenderbufferOES, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_UINT_PARAMETER, renderbuffer);

    // Get the RenderContextMonitor
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        bool rcActRBO = pCurrentThreadRenderContextMonitor->activateRenderBuffer(renderbuffer);
        GT_ASSERT(rcActRBO);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glBindRenderbufferOES(target, renderbuffer);

    SU_END_FUNCTION_WRAPPER(ap_glBindRenderbufferOES);
}

void GL_APIENTRY glDeleteRenderbuffersOES(GLsizei n, const GLuint* renderbuffers)
{
    SU_START_FUNCTION_WRAPPER(ap_glDeleteRenderbuffersOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDeleteRenderbuffersOES, 2, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_GL_P_UINT_PARAMETER, renderbuffers);

    // Log the textures generation:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        gsRenderBuffersMonitor* renderBuffersMon = pCurrentThreadRenderContextMonitor->renderBuffersMonitor();
        GT_IF_WITH_ASSERT(renderBuffersMon != NULL)
        {
            renderBuffersMon->onRenderBufferObjectsDeletion(n, renderbuffers);
        }
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glDeleteRenderbuffersOES(n, renderbuffers);

    SU_END_FUNCTION_WRAPPER(ap_glDeleteRenderbuffersOES);
}

void GL_APIENTRY glGenRenderbuffersOES(GLsizei n, GLuint* renderbuffers)
{
    SU_START_FUNCTION_WRAPPER(ap_glGenRenderbuffersOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGenRenderbuffersOES, 2, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_GL_P_UINT_PARAMETER, renderbuffers);

    // Call the real function:
    gs_stat_realFunctionPointers.glGenRenderbuffersOES(n, renderbuffers);

    // Log the textures generation:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        gsRenderBuffersMonitor* renderBuffersMon = pCurrentThreadRenderContextMonitor->renderBuffersMonitor();
        GT_IF_WITH_ASSERT(renderBuffersMon != NULL)
        {
            renderBuffersMon->onRenderBufferObjectsGeneration(n, renderbuffers);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_glGenRenderbuffersOES);
}

void GL_APIENTRY glRenderbufferStorageOES(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    SU_START_FUNCTION_WRAPPER(ap_glRenderbufferStorageOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRenderbufferStorageOES, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, internalformat, OS_TOBJ_ID_GL_SIZEI_PARAMETER, width, OS_TOBJ_ID_GL_SIZEI_PARAMETER, height);

    // Log the textures generation:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        bool rcRBOParams = pCurrentThreadRenderContextMonitor->setActiveRenderBufferObjectParameters(internalformat, width, height);
        GT_ASSERT(rcRBOParams);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glRenderbufferStorageOES(target, internalformat, width, height);

    SU_END_FUNCTION_WRAPPER(ap_glRenderbufferStorageOES);
}

void GL_APIENTRY glGetRenderbufferParameterivOES(GLenum target, GLenum pname, GLint* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetRenderbufferParameterivOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetRenderbufferParameterivOES, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_INT_PARAMETER, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetRenderbufferParameterivOES(target, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetRenderbufferParameterivOES);
}

GLboolean GL_APIENTRY glIsFramebufferOES(GLuint framebuffer)
{
    GLboolean retVal = GL_FALSE;

    SU_START_FUNCTION_WRAPPER(ap_glIsFramebufferOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glIsFramebufferOES, 1, OS_TOBJ_ID_GL_UINT_PARAMETER, framebuffer);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.glIsFramebufferOES(framebuffer);

    SU_END_FUNCTION_WRAPPER(ap_glIsFramebufferOES);

    return retVal;
}

void GL_APIENTRY glBindFramebufferOES(GLenum target, GLuint framebuffer)
{
    SU_START_FUNCTION_WRAPPER(ap_glBindFramebufferOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glBindFramebufferOES, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_UINT_PARAMETER, framebuffer);

    // Get the RenderContextMonitor
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();
    GT_IF_WITH_ASSERT(pCurrentThreadRenderContextMonitor != NULL)
    {
        bool rcActFBO = pCurrentThreadRenderContextMonitor->activateFBO(framebuffer);
        GT_ASSERT(rcActFBO);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glBindFramebufferOES(target, framebuffer);

    SU_END_FUNCTION_WRAPPER(ap_glBindFramebufferOES);
}

void GL_APIENTRY glDeleteFramebuffersOES(GLsizei n, const GLuint* framebuffers)
{
    SU_START_FUNCTION_WRAPPER(ap_glDeleteFramebuffersOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDeleteFramebuffersOES, 2, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_GL_P_UINT_PARAMETER, framebuffers);

    // Get the RenderContextMonitor
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    // Delete each of the FBOs from the FBOMonitor:
    GT_IF_WITH_ASSERT(pCurrentThreadRenderContextMonitor != NULL)
    {
        gsFBOMonitor* fboMon = pCurrentThreadRenderContextMonitor->fboMonitor();
        GT_IF_WITH_ASSERT(fboMon != NULL)
        {
            for (int i = 0; i < n; i++)
            {
                // delete the FBOs from the FBOs monitor
                fboMon->removeFbo(framebuffers[i]);
            }
        }
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glDeleteFramebuffersOES(n, framebuffers);

    SU_END_FUNCTION_WRAPPER(ap_glDeleteFramebuffersOES);
}

void GL_APIENTRY glGenFramebuffersOES(GLsizei n, GLuint* framebuffers)
{
    SU_START_FUNCTION_WRAPPER(ap_glGenFramebuffersOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGenFramebuffersOES, 2, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_GL_P_UINT_PARAMETER, framebuffers);

    // Call the real function:
    gs_stat_realFunctionPointers.glGenFramebuffersOES(n, framebuffers);

    // Get the RenderContextMonitor
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    // Construct a new apFBO for each of the generated frame buffer objects:
    GT_IF_WITH_ASSERT(pCurrentThreadRenderContextMonitor != NULL)
    {
        gsFBOMonitor* fboMon = pCurrentThreadRenderContextMonitor->fboMonitor();
        GT_IF_WITH_ASSERT(fboMon != NULL)
        {
            for (int i = 0; i < n; i++)
            {
                // Add a new FBO to the FBOs monitor
                fboMon->constructNewFbo(framebuffers[i]);
            }
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_glGenFramebuffersOES);
}

GLenum GL_APIENTRY glCheckFramebufferStatusOES(GLenum target)
{
    SU_START_FUNCTION_WRAPPER(ap_glCheckFramebufferStatusOES);

    GLenum retVal = GL_FRAMEBUFFER_UNSUPPORTED_EXT;

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glCheckFramebufferStatusOES, 1, OS_TOBJ_ID_GL_ENUM_PARAMETER, target);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.glCheckFramebufferStatusOES(target);

    SU_END_FUNCTION_WRAPPER(ap_glCheckFramebufferStatusOES);

    return retVal;
}

void GL_APIENTRY glFramebufferRenderbufferOES(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    SU_START_FUNCTION_WRAPPER(ap_glFramebufferRenderbufferOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glFramebufferRenderbufferOES, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, attachment, OS_TOBJ_ID_GL_ENUM_PARAMETER, renderbuffertarget, OS_TOBJ_ID_GL_UINT_PARAMETER, renderbuffer);

    // Get the RenderContextMonitor
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    // Construct a new apFBO for each of the generated frame buffer objects:
    GT_IF_WITH_ASSERT(pCurrentThreadRenderContextMonitor != NULL)
    {
        // Bind the texture to the active FBO:
        bool rcBindFBO = pCurrentThreadRenderContextMonitor->bindObjectToTheActiveFBO(attachment, renderbuffertarget, renderbuffer, 0);
        GT_ASSERT(rcBindFBO);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glFramebufferRenderbufferOES(target, attachment, renderbuffertarget, renderbuffer);

    SU_END_FUNCTION_WRAPPER(ap_glFramebufferRenderbufferOES);
}

void GL_APIENTRY glFramebufferTexture2DOES(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    SU_START_FUNCTION_WRAPPER(ap_glFramebufferTexture2DOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glFramebufferTexture2DOES, 5, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, attachment, OS_TOBJ_ID_GL_ENUM_PARAMETER, textarget, OS_TOBJ_ID_GL_UINT_PARAMETER, texture, OS_TOBJ_ID_GL_INT_PARAMETER, level);

    // Get the RenderContextMonitor
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    // Construct a new apFBO for each of the generated frame buffer objects:
    GT_IF_WITH_ASSERT(pCurrentThreadRenderContextMonitor != NULL)
    {
        // Bind the texture to the active FBO:
        bool rcBindFBO = pCurrentThreadRenderContextMonitor->bindObjectToTheActiveFBO(attachment, textarget, texture, 0);
        GT_ASSERT(rcBindFBO);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glFramebufferTexture2DOES(target, attachment, textarget, texture, level);

    SU_END_FUNCTION_WRAPPER(ap_glFramebufferTexture2DOES);
}

void GL_APIENTRY glGetFramebufferAttachmentParameterivOES(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetFramebufferAttachmentParameterivOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetFramebufferAttachmentParameterivOES, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, attachment, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_INT_PARAMETER, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetFramebufferAttachmentParameterivOES(target, attachment, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetFramebufferAttachmentParameterivOES);
}

void GL_APIENTRY glGenerateMipmapOES(GLenum target)
{
    SU_START_FUNCTION_WRAPPER(ap_glGenerateMipmapOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGenerateMipmapOES, 1, OS_TOBJ_ID_GL_ENUM_PARAMETER, target);

    // Call the real function:
    gs_stat_realFunctionPointers.glGenerateMipmapOES(target);

    // Log the mipmap generation:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        pCurrentThreadRenderContextMonitor->onTextureMipmapGenerate(target);
    }

    SU_END_FUNCTION_WRAPPER(ap_glGenerateMipmapOES);
}

//////////////////////////////////////////////////////////////////////////
// OES_mapbuffer
//////////////////////////////////////////////////////////////////////////
void GL_APIENTRY glGetBufferPointervOES(GLenum target, GLenum pname, GLvoid** params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetBufferPointervOES);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetBufferPointervOES, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_PP_VOID_PARAMETER, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetBufferPointervOES(target, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetBufferPointervOES);
}

GLvoid* GL_APIENTRY glMapBufferOES(GLenum target, GLenum access)
{
    SU_START_FUNCTION_WRAPPER(ap_glMapBufferOES);

    GLvoid* retVal = NULL;

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMapBufferOES, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, access);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.glMapBufferOES(target, access);

    SU_END_FUNCTION_WRAPPER(ap_glMapBufferOES);

    return retVal;
}

GLboolean GL_APIENTRY glUnmapBufferOES(GLenum target)
{
    SU_START_FUNCTION_WRAPPER(ap_glUnmapBufferOES);

    GLboolean retVal = GL_FALSE;

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glUnmapBufferOES, 1, OS_TOBJ_ID_GL_ENUM_PARAMETER, target);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.glUnmapBufferOES(target);

    SU_END_FUNCTION_WRAPPER(ap_glUnmapBufferOES);

    return retVal;
}

//////////////////////////////////////////////////////////////////////////
// OES_matrix_palette
//////////////////////////////////////////////////////////////////////////
void GL_APIENTRY glCurrentPaletteMatrixOES(GLuint matrixpaletteindex)
{
    SU_START_FUNCTION_WRAPPER(ap_glCurrentPaletteMatrixOES)

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glCurrentPaletteMatrixOES, 1, OS_TOBJ_ID_GL_UINT_PARAMETER, matrixpaletteindex);

    // Call the real function:
    gs_stat_realFunctionPointers.glCurrentPaletteMatrixOES(matrixpaletteindex);

    SU_END_FUNCTION_WRAPPER(ap_glCurrentPaletteMatrixOES)
}

void GL_APIENTRY glLoadPaletteFromModelViewMatrixOES(void)
{
    SU_START_FUNCTION_WRAPPER(ap_glLoadPaletteFromModelViewMatrixOES)

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLoadPaletteFromModelViewMatrixOES, 0);

    // Call the real function:
    gs_stat_realFunctionPointers.glLoadPaletteFromModelViewMatrixOES();

    SU_END_FUNCTION_WRAPPER(ap_glLoadPaletteFromModelViewMatrixOES)

}

void GL_APIENTRY glMatrixIndexPointerOES(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    SU_START_FUNCTION_WRAPPER(ap_glMatrixIndexPointerOES)

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMatrixIndexPointerOES, 4, OS_TOBJ_ID_GL_INT_PARAMETER, size, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_GL_SIZEI_PARAMETER, stride, OS_TOBJ_ID_GL_P_VOID_PARAMETER, pointer);

    // Call the real function:
    gs_stat_realFunctionPointers.glMatrixIndexPointerOES(size, type, stride, pointer);

    SU_END_FUNCTION_WRAPPER(ap_glMatrixIndexPointerOES)
}

void GL_APIENTRY glWeightPointerOES(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    SU_START_FUNCTION_WRAPPER(ap_glWeightPointerOES)

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glWeightPointerOES, 4, OS_TOBJ_ID_GL_INT_PARAMETER, size, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_GL_SIZEI_PARAMETER, stride, OS_TOBJ_ID_GL_P_VOID_PARAMETER, pointer);

    // Call the real function:
    gs_stat_realFunctionPointers.glWeightPointerOES(size, type, stride, pointer);

    SU_END_FUNCTION_WRAPPER(ap_glWeightPointerOES)
}

//////////////////////////////////////////////////////////////////////////
// OES_point_size_array
//////////////////////////////////////////////////////////////////////////
void GL_APIENTRY glPointSizePointerOES(GLenum type, GLsizei stride, const GLvoid* pointer)
{
    SU_START_FUNCTION_WRAPPER(ap_glPointSizePointerOES)

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPointSizePointerOES, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_GL_SIZEI_PARAMETER, stride, OS_TOBJ_ID_GL_P_VOID_PARAMETER, pointer);

    // Call the real function:
    gs_stat_realFunctionPointers.glPointSizePointerOES(type, stride, pointer);

    SU_END_FUNCTION_WRAPPER(ap_glPointSizePointerOES)
}

#endif

