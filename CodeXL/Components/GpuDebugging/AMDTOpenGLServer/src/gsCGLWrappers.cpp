//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsCGLWrappers.cpp
///
//==================================================================================

//------------------------------ gsCGLWrappers.cpp------------------------------

// --------------------------------------------------------
// File:
// This file contains a wrapper function for CGL "base" functions
// (CGL functions that are exported from the system's OpenGL module (ex: libGL))
// --------------------------------------------------------

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/apDefinitions.h>
#include <AMDTAPIClasses/Include/apFrameTerminators.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsStringConstants.h>
#include <src/gsWrappersCommon.h>

CGLError CGLChoosePixelFormat(const CGLPixelFormatAttribute* attribs, CGLPixelFormatObj* pix, GLint* npix)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLChoosePixelFormat);

    // CGLChoosePixelFormat internally calls CGLSetCurrentContext:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_CGLSetCurrentContext);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLChoosePixelFormat, 3, OS_TOBJ_ID_POINTER_PARAMETER, attribs, OS_TOBJ_ID_POINTER_PARAMETER, pix, OS_TOBJ_ID_GL_P_INT_PARAMETER, npix);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLChoosePixelFormat(attribs, pix, npix);

    // Restore the internal function code:
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_CGLSetCurrentContext);

    SU_END_FUNCTION_WRAPPER(ap_CGLChoosePixelFormat);

    return retVal;
}

CGLError CGLDestroyPixelFormat(CGLPixelFormatObj pix)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLDestroyPixelFormat);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLDestroyPixelFormat, 1, OS_TOBJ_ID_POINTER_PARAMETER, pix);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLDestroyPixelFormat(pix);

    SU_END_FUNCTION_WRAPPER(ap_CGLDestroyPixelFormat);

    return retVal;
}

CGLError CGLDescribePixelFormat(CGLPixelFormatObj pix, GLint pix_num, CGLPixelFormatAttribute attrib, GLint* value)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLDescribePixelFormat);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLDescribePixelFormat, 4, OS_TOBJ_ID_POINTER_PARAMETER, pix, OS_TOBJ_ID_GL_INT_PARAMETER, pix_num, OS_TOBJ_ID_CGL_PIXEL_FORMAT_ATTRIBUTE_PARAMETER, attrib, OS_TOBJ_ID_GL_P_INT_PARAMETER, value);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLDescribePixelFormat(pix, pix_num, attrib, value);

    SU_END_FUNCTION_WRAPPER(ap_CGLDescribePixelFormat);

    return retVal;
}

CGLError CGLCreateContext(CGLPixelFormatObj pix, CGLContextObj share, CGLContextObj* ctx)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLCreateContext);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLCreateContext, 3, OS_TOBJ_ID_POINTER_PARAMETER, pix, OS_TOBJ_ID_POINTER_PARAMETER, share, OS_TOBJ_ID_POINTER_PARAMETER, ctx);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLCreateContext(pix, share, ctx);

    // If the context creation succeeded:
    if (retVal == kCGLNoError)
    {
        GT_IF_WITH_ASSERT(ctx != NULL)
        {
            // Register the created context in our OpenGL monitor:
            gs_stat_openGLMonitorInstance.onContextCreation(NULL, *ctx, ap_CGLCreateContext);

            // mark the context's VisualID:
            int renderContextID = gs_stat_openGLMonitorInstance.renderContextSpyId(*ctx);
            gs_stat_openGLMonitorInstance.renderContextMonitor(renderContextID)->setPixelFormatId(pix);

            // If we share another context's list, register this fact:
            if (share != NULL)
            {
                gs_stat_openGLMonitorInstance.onShareLists(share, *ctx);
            }
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_CGLCreateContext);

    return retVal;
}

CGLError CGLCopyContext(CGLContextObj src, CGLContextObj dst, GLbitfield mask)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLCopyContext);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLCopyContext, 3, OS_TOBJ_ID_POINTER_PARAMETER, src, OS_TOBJ_ID_POINTER_PARAMETER, dst, OS_TOBJ_ID_GL_BITFIELD_PARAMETER, mask);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLCopyContext(src, dst, mask);

    SU_END_FUNCTION_WRAPPER(ap_CGLCopyContext);

    return retVal;
}

CGLError CGLDestroyContext(CGLContextObj ctx)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLDestroyContext);

    // CGLDestroyContext internally calls CGLClearDrawable and CGLGetCurrentContext:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_CGLClearDrawable);
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_CGLGetCurrentContext);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLDestroyContext, 1, OS_TOBJ_ID_POINTER_PARAMETER, ctx);

    // Mark that the context was deleted:
    gs_stat_openGLMonitorInstance.beforeContextDeletion(ctx);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLDestroyContext(ctx);

    // Mark that the context was deleted:
    gs_stat_openGLMonitorInstance.afterContextDeletion(ctx);

    // Restore the internal function code
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_CGLClearDrawable);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_CGLGetCurrentContext);

    SU_END_FUNCTION_WRAPPER(ap_CGLDestroyContext);

    return retVal;
}

CGLContextObj CGLGetCurrentContext()
{
    SU_START_FUNCTION_WRAPPER(ap_CGLGetCurrentContext);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLGetCurrentContext , 0);

    // Call the real function:
    CGLContextObj retVal = gs_stat_realFunctionPointers.CGLGetCurrentContext();

    SU_END_FUNCTION_WRAPPER(ap_CGLGetCurrentContext);

    return retVal;
}

CGLError CGLSetCurrentContext(CGLContextObj ctx)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLSetCurrentContext);

    // CGLSetCurrentContext internally calls glGetProgramivARB, glGetHandleARB and CGLGetParameter:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramivARB);
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetHandleARB);
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_CGLGetParameter);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLSetCurrentContext, 1, OS_TOBJ_ID_POINTER_PARAMETER, ctx);

    // If we are defined as a frame terminator:
    unsigned int frameTerminatorsMask = suFrameTerminatorsMask();

    if (frameTerminatorsMask & AP_MAKE_CURRENT_TERMINATOR)
    {
        // Terminate the current frame:
        gs_stat_openGLMonitorInstance.onFrameTerminatorCall();
    }

    // Notify the OpenGL monitor that the thread current render context is going to change:
    gs_stat_openGLMonitorInstance.beforeContextMadeCurrent(ctx);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLSetCurrentContext(ctx);

    if (retVal == kCGLNoError)
    {
        // Update the OpenGL monitor active context:
        gs_stat_openGLMonitorInstance.onContextMadeCurrent(NULL, NULL, NULL, ctx);

        // Uri, 12/3/09: It seems from various internet discussion and some sample code (which works)
        // That CGLSetPBuffer is equivalent to functions such as CGLSetFullScreen and CGLSetOffscreen,
        // And not to CGLSetCurrentContext. Thus, making a different context current (or making no
        // no context at all current) does not change a PBuffer's attachment to the context. Therefore,
        // unlike Linux and Windows, we do not inform the PBuffers monitor of this change.
    }

    // Restore the internal function code:
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramivARB);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetHandleARB);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_CGLGetParameter);

    SU_END_FUNCTION_WRAPPER(ap_CGLSetCurrentContext);

    return retVal;
}

CGLError CGLEnable(CGLContextObj ctx, CGLContextEnable pname)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLEnable);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLEnable, 2, OS_TOBJ_ID_POINTER_PARAMETER, ctx, OS_TOBJ_ID_CGL_CONTEXT_ENABLE_PARAMETER, pname);

    CGLError retVal = kCGLNoError;

    if (pname != kCGLCEMPEngine)
    {
        // Call the real function:
        retVal = gs_stat_realFunctionPointers.CGLEnable(ctx, pname);
    }
    else // pname == kCGLCEMPEngine
    {
        // This enable activates the multi-threaded OpenGL engine. This engine
        // has threads wait for a worker thread, so calling glGetXXXX inside a
        // makeThreadExecuteFunction call causes the Spy and CodeXL to hang.
        // So, we ignore it and notify the user of the override:
        OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_overridingCallToCGLEnablekCGLCEMPEngine, OS_DEBUG_LOG_INFO);
        osOutputDebugString(GS_STR_tryingToEnableCGLCEMPEngine);
    }

    SU_END_FUNCTION_WRAPPER(ap_CGLEnable);

    return retVal;
}

CGLError CGLDisable(CGLContextObj ctx, CGLContextEnable pname)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLDisable);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLDisable, 2, OS_TOBJ_ID_POINTER_PARAMETER, ctx, OS_TOBJ_ID_CGL_CONTEXT_ENABLE_PARAMETER, pname);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLDisable(ctx, pname);

    SU_END_FUNCTION_WRAPPER(ap_CGLDisable);

    return retVal;
}

CGLError CGLIsEnabled(CGLContextObj ctx, CGLContextEnable pname, GLint* enable)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLIsEnabled);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLIsEnabled, 3, OS_TOBJ_ID_POINTER_PARAMETER, ctx, OS_TOBJ_ID_CGL_CONTEXT_ENABLE_PARAMETER, pname, OS_TOBJ_ID_CGL_CONTEXT_ENABLE_PARAMETER, enable);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLIsEnabled(ctx, pname, enable);

    SU_END_FUNCTION_WRAPPER(ap_CGLIsEnabled);

    return retVal;
}

CGLError CGLSetParameter(CGLContextObj ctx, CGLContextParameter pname, const GLint* params)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLSetParameter);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLSetParameter, 3, OS_TOBJ_ID_POINTER_PARAMETER, ctx, OS_TOBJ_ID_CGL_CONTEXT_PARAMETER_PARAMETER, pname, OS_TOBJ_ID_GL_P_INT_PARAMETER, params);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLSetParameter(ctx, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_CGLSetParameter);

    return retVal;
}

CGLError CGLGetParameter(CGLContextObj ctx, CGLContextParameter pname, GLint* params)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLGetParameter);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLGetParameter, 3, OS_TOBJ_ID_POINTER_PARAMETER, ctx, OS_TOBJ_ID_CGL_CONTEXT_PARAMETER_PARAMETER, pname, OS_TOBJ_ID_GL_P_INT_PARAMETER, params);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLGetParameter(ctx, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_CGLGetParameter);

    return retVal;
}

CGLError CGLLockContext(CGLContextObj ctx)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLLockContext);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLLockContext, 1, OS_TOBJ_ID_POINTER_PARAMETER, ctx);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLLockContext(ctx);

    SU_END_FUNCTION_WRAPPER(ap_CGLLockContext);

    return retVal;
}

CGLError CGLUnlockContext(CGLContextObj ctx)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLUnlockContext);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLUnlockContext, 1, OS_TOBJ_ID_POINTER_PARAMETER, ctx);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLUnlockContext(ctx);

    SU_END_FUNCTION_WRAPPER(ap_CGLUnlockContext);

    return retVal;
}

CGLError CGLSetOffScreen(CGLContextObj ctx, GLsizei width, GLsizei height, GLint rowbytes, void* baseaddr)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLSetOffScreen);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLSetOffScreen, 5, OS_TOBJ_ID_POINTER_PARAMETER, ctx, OS_TOBJ_ID_GL_SIZEI_PARAMETER, width, OS_TOBJ_ID_GL_SIZEI_PARAMETER, height, OS_TOBJ_ID_GL_INT_PARAMETER, rowbytes, OS_TOBJ_ID_POINTER_PARAMETER, baseaddr);

    // Yaki + Uri, 8/3/09: We currently do not support offscreen rendering, as it is a little-used
    // method which came before there were PBuffers (which do the same thing, only better). Note that
    // offscreen rendering works with *NON*-hardware accelerated contexts, and if we decide to support
    // it, should be treated roughly the same as a PBuffer.

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLSetOffScreen(ctx, width, height, rowbytes, baseaddr);

    SU_END_FUNCTION_WRAPPER(ap_CGLSetOffScreen);

    return retVal;
}

CGLError CGLGetOffScreen(CGLContextObj ctx, GLsizei* width, GLsizei* height, GLint* rowbytes, void** baseaddr)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLGetOffScreen);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLGetOffScreen, 5, OS_TOBJ_ID_POINTER_PARAMETER, ctx, OS_TOBJ_ID_GL_P_SIZEI_PARAMETER, width, OS_TOBJ_ID_GL_P_SIZEI_PARAMETER, height, OS_TOBJ_ID_GL_P_INT_PARAMETER, rowbytes, OS_TOBJ_ID_POINTER_PARAMETER, baseaddr);

    // Yaki + Uri, 8/3/09: We currently do not support offscreen rendering, as it is a little-used
    // method which came before there were PBuffers (which do the same thing, only better). Note that
    // offscreen rendering works with *NON*-hardware accelerated contexts, and if we decide to support
    // it, should be treated roughly the same as a PBuffer.

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLGetOffScreen(ctx, width, height, rowbytes, baseaddr);

    SU_END_FUNCTION_WRAPPER(ap_CGLGetOffScreen);

    return retVal;
}

CGLError CGLSetFullScreen(CGLContextObj ctx)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLSetFullScreen);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLSetFullScreen, 1, OS_TOBJ_ID_POINTER_PARAMETER, ctx);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLSetFullScreen(ctx);

    SU_END_FUNCTION_WRAPPER(ap_CGLSetFullScreen);

    return retVal;
}

CGLError CGLClearDrawable(CGLContextObj ctx)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLClearDrawable);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLClearDrawable, 1, OS_TOBJ_ID_POINTER_PARAMETER, ctx);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLClearDrawable(ctx);

    SU_END_FUNCTION_WRAPPER(ap_CGLClearDrawable);

    return retVal;
}

CGLError CGLFlushDrawable(CGLContextObj ctx)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLFlushDrawable);

    // CGLFlushDrawable internally calls glGetProgramivARB, glGetHandleARB and CGLGetParameter:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_CGLGetParameter);
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramivARB);
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetHandleARB);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLFlushDrawable, 1, OS_TOBJ_ID_POINTER_PARAMETER, ctx);

    // If we are defined as a frame terminator:
    unsigned int frameTerminatorsMask = suFrameTerminatorsMask();

    if (frameTerminatorsMask & AP_SWAP_BUFFERS_TERMINATOR)
    {
        // Terminate the current frame:
        gs_stat_openGLMonitorInstance.onFrameTerminatorCall();
    }

    // Will get true iff we are in "Force front draw buffer" mode:
    bool isFrontDrawBuffForced = false;

    // Get the current thread render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        // Check if front draw buffer is forced:
        gsForcedModesManager& forcedModesMgr = pCurrentThreadRenderContextMonitor->forcedModesManager();
        isFrontDrawBuffForced = forcedModesMgr.isStubForced(AP_OPENGL_FORCED_FRONT_DRAW_BUFFER_MODE);
    }

    CGLError retVal = kCGLNoError;

    // If the OpenGL front draw buffer is forced:
    if (isFrontDrawBuffForced)
    {
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glFlush);
        // Since we are rendering into the front buffer - instead of calling CGLFlushDrawable
        // we will call glFlush.
        // Note that glFlush doesn't return a value, so our wrapper just returns a success value.
        gs_stat_realFunctionPointers.glFlush();
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glFlush);
    }
    else
    {
        // Call the real function:
        retVal = gs_stat_realFunctionPointers.CGLFlushDrawable(ctx);
    }

    // Restore the internal function code:
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramivARB);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetHandleARB);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_CGLGetParameter);

    SU_END_FUNCTION_WRAPPER(ap_CGLFlushDrawable);

    return retVal;
}

CGLError CGLCreatePBuffer(GLsizei width, GLsizei height, GLenum target, GLenum internalFormat, GLint max_level, CGLPBufferObj* pbuffer)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLCreatePBuffer);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLCreatePBuffer, 6, OS_TOBJ_ID_GL_SIZEI_PARAMETER, width, OS_TOBJ_ID_GL_SIZEI_PARAMETER, height, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, internalFormat, OS_TOBJ_ID_GL_INT_PARAMETER, max_level, OS_TOBJ_ID_POINTER_PARAMETER, pbuffer);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLCreatePBuffer(width, height, target, internalFormat, max_level, pbuffer);

    if ((retVal == kCGLNoError) && (pbuffer != NULL))
    {
        // Get the PBuffer monitor:
        gsPBuffersMonitor& pbuffersMonitor = gs_stat_openGLMonitorInstance.pbuffersMonitor();

        // Log the created PBuffer:
        pbuffersMonitor.onPBufferCreation(*pbuffer, 0, 0, (int)width, (int)height, NULL, target, internalFormat, max_level);
    }

    SU_END_FUNCTION_WRAPPER(ap_CGLCreatePBuffer);

    return retVal;
}

CGLError CGLDescribePBuffer(CGLPBufferObj obj, GLsizei* width, GLsizei* height, GLenum* target, GLenum* internalFormat, GLint* mipmap)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLDescribePBuffer);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLDescribePBuffer, 6, OS_TOBJ_ID_POINTER_PARAMETER, obj, OS_TOBJ_ID_GL_P_SIZEI_PARAMETER, width, OS_TOBJ_ID_GL_P_SIZEI_PARAMETER, height, OS_TOBJ_ID_GL_P_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_P_ENUM_PARAMETER, internalFormat, OS_TOBJ_ID_GL_P_INT_PARAMETER, mipmap);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLDescribePBuffer(obj, width, height, target, internalFormat, mipmap);

    SU_END_FUNCTION_WRAPPER(ap_CGLDescribePBuffer);

    return retVal;
}

CGLError CGLDestroyPBuffer(CGLPBufferObj pbuffer)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLDestroyPBuffer);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLDestroyPBuffer, 1, OS_TOBJ_ID_POINTER_PARAMETER, pbuffer);

    // Get the PBuffer monitor
    gsPBuffersMonitor& pbuffersMonitor = gs_stat_openGLMonitorInstance.pbuffersMonitor();

    // Delete the PBuffer from the PBuffer logger:
    pbuffersMonitor.onPBufferDeletion(pbuffer);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLDestroyPBuffer(pbuffer);

    SU_END_FUNCTION_WRAPPER(ap_CGLDestroyPBuffer);

    return retVal;
}

CGLError CGLGetPBuffer(CGLContextObj ctx, CGLPBufferObj* pbuffer, GLenum* face, GLint* level, GLint* screen)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLGetPBuffer);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLGetPBuffer, 5, OS_TOBJ_ID_POINTER_PARAMETER, ctx, OS_TOBJ_ID_POINTER_PARAMETER, pbuffer, OS_TOBJ_ID_GL_P_ENUM_PARAMETER, face, OS_TOBJ_ID_GL_P_INT_PARAMETER, level, OS_TOBJ_ID_GL_P_INT_PARAMETER, screen);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLGetPBuffer(ctx, pbuffer, face, level, screen);

    SU_END_FUNCTION_WRAPPER(ap_CGLGetPBuffer);

    return retVal;
}


CGLError CGLSetPBuffer(CGLContextObj ctx, CGLPBufferObj pbuffer, GLenum face, GLint level, GLint screen)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLSetPBuffer);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLSetPBuffer, 5, OS_TOBJ_ID_POINTER_PARAMETER, ctx, OS_TOBJ_ID_POINTER_PARAMETER, pbuffer, OS_TOBJ_ID_GL_ENUM_PARAMETER, face, OS_TOBJ_ID_GL_INT_PARAMETER, level, OS_TOBJ_ID_GL_INT_PARAMETER, screen);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLSetPBuffer(ctx, pbuffer, face, level, screen);

    if (retVal == kCGLNoError)
    {
        // Update the OpenGL monitor active context:
        gs_stat_openGLMonitorInstance.onContextMadeCurrent(NULL, NULL, NULL, ctx);

        // Update the PBuffers monitor:
        gsPBuffersMonitor& pbufferMtr = gs_stat_openGLMonitorInstance.pbuffersMonitor();
        pbufferMtr.onCGLSetPBuffer(ctx, pbuffer, face, level);
    }

    SU_END_FUNCTION_WRAPPER(ap_CGLSetPBuffer);

    return retVal;
}

CGLError CGLTexImagePBuffer(CGLContextObj ctx, CGLPBufferObj pbuffer, GLenum source)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLTexImagePBuffer);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLTexImagePBuffer, 3, OS_TOBJ_ID_POINTER_PARAMETER, ctx, OS_TOBJ_ID_POINTER_PARAMETER, pbuffer, OS_TOBJ_ID_GL_ENUM_PARAMETER, source);

    CGLError retVal = kCGLNoError;
    // Check if stub textures are forced:
    bool areStubTexForced = false;
    int contextSpyId = gs_stat_openGLMonitorInstance.renderContextSpyId(ctx);
    GT_IF_WITH_ASSERT(contextSpyId != AP_NULL_CONTEXT_ID)
    {
        gsRenderContextMonitor* pRenderContextMonitor = gs_stat_openGLMonitorInstance.renderContextMonitor(contextSpyId);

        if (pRenderContextMonitor != NULL)
        {
            areStubTexForced = pRenderContextMonitor->forcedModesManager().isStubForced(AP_OPENGL_FORCED_STUB_TEXTURES_MODE);
        }

        // If we are not in "Force stub textures" mode:
        if (!areStubTexForced)
        {
            // Call the real function:
            retVal = gs_stat_realFunctionPointers.CGLTexImagePBuffer(ctx, pbuffer, source);

            // Log the changes to the texture:
            // Get the PBuffer properties:
            const gsPBuffersMonitor& pbuffersMonitor = gs_stat_openGLMonitorInstance.pbuffersMonitor();
            pbuffersMonitor.onCGLTexImagePBuffer(contextSpyId, pbuffer, source);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_CGLTexImagePBuffer);

    return retVal;
}

const char* CGLErrorString(CGLError error)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLErrorString);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLErrorString, 1, OS_TOBJ_ID_INT_PARAMETER, error);

    // Call the real function:
    const char* retVal = gs_stat_realFunctionPointers.CGLErrorString(error);

    SU_END_FUNCTION_WRAPPER(ap_CGLErrorString);

    return retVal;
}

CGLError CGLSetOption(CGLGlobalOption pname, GLint param)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLSetOption);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLSetOption, 2, OS_TOBJ_ID_CGL_GLOBAL_OPTION_PARAMETER, pname, OS_TOBJ_ID_INT_PARAMETER, param);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLSetOption(pname, param);

    SU_END_FUNCTION_WRAPPER(ap_CGLSetOption);

    return retVal;
}
CGLError CGLGetOption(CGLGlobalOption pname, GLint* param)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLGetOption);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLGetOption, 2, OS_TOBJ_ID_CGL_GLOBAL_OPTION_PARAMETER, pname, OS_TOBJ_ID_GL_P_INT_PARAMETER, param);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLGetOption(pname, param);

    SU_END_FUNCTION_WRAPPER(ap_CGLGetOption);

    return retVal;
}
void CGLGetVersion(GLint* majorvers, GLint* minorvers)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLGetVersion);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLGetVersion, 2, OS_TOBJ_ID_GL_P_INT_PARAMETER, majorvers, OS_TOBJ_ID_GL_P_INT_PARAMETER, minorvers);

    // Call the real function:
    gs_stat_realFunctionPointers.CGLGetVersion(majorvers, minorvers);

    SU_END_FUNCTION_WRAPPER(ap_CGLGetVersion);
}

CGLError CGLDescribeRenderer(CGLRendererInfoObj rend, GLint rend_num, CGLRendererProperty prop, GLint* value)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLDescribeRenderer);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLDescribeRenderer, 4, OS_TOBJ_ID_POINTER_PARAMETER, rend, OS_TOBJ_ID_GL_INT_PARAMETER, rend_num, OS_TOBJ_ID_CGL_RENDERER_PROPERTY_PARAMETER, prop, OS_TOBJ_ID_GL_P_INT_PARAMETER, value);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLDescribeRenderer(rend, rend_num, prop, value);

    SU_END_FUNCTION_WRAPPER(ap_CGLDescribeRenderer);

    return retVal;
}

CGLError CGLDestroyRendererInfo(CGLRendererInfoObj rend)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLDestroyRendererInfo);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLDestroyRendererInfo, 1, OS_TOBJ_ID_POINTER_PARAMETER, rend);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLDestroyRendererInfo(rend);

    SU_END_FUNCTION_WRAPPER(ap_CGLDestroyRendererInfo);

    return retVal;
}

CGLError CGLQueryRendererInfo(GLuint display_mask, CGLRendererInfoObj* rend, GLint* nrend)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLQueryRendererInfo);

    // CGLQueryRendererInfo internally calls CGLSetCurrentContext:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_CGLSetCurrentContext);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLQueryRendererInfo, 3, OS_TOBJ_ID_GL_UINT_PARAMETER, display_mask, OS_TOBJ_ID_POINTER_PARAMETER, rend, OS_TOBJ_ID_GL_P_INT_PARAMETER, nrend);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLQueryRendererInfo(display_mask, rend, nrend);

    // Restore the internal function code:
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_CGLSetCurrentContext);

    SU_END_FUNCTION_WRAPPER(ap_CGLQueryRendererInfo);

    return retVal;
}

CGLError CGLSetVirtualScreen(CGLContextObj ctx, GLint screen)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLSetVirtualScreen);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLSetVirtualScreen, 2, OS_TOBJ_ID_POINTER_PARAMETER, ctx, OS_TOBJ_ID_GL_INT_PARAMETER, screen);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLSetVirtualScreen(ctx, screen);

    SU_END_FUNCTION_WRAPPER(ap_CGLSetVirtualScreen);

    return retVal;
}

CGLError CGLGetVirtualScreen(CGLContextObj ctx, GLint* screen)
{
    SU_START_FUNCTION_WRAPPER(ap_CGLGetVirtualScreen);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_CGLGetVirtualScreen, 2, OS_TOBJ_ID_POINTER_PARAMETER, ctx, OS_TOBJ_ID_GL_P_INT_PARAMETER, screen);

    // Call the real function:
    CGLError retVal = gs_stat_realFunctionPointers.CGLGetVirtualScreen(ctx, screen);

    SU_END_FUNCTION_WRAPPER(ap_CGLGetVirtualScreen);

    return retVal;
}
