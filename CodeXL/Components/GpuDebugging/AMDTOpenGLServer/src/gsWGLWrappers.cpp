//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsWGLWrappers.cpp
///
//==================================================================================

//------------------------------ gsWGLWrappers.cpp ------------------------------

// --------------------------------------------------------
// File:
// This file contains a wrapper function for WGL "base" functions
// (WGL functions that are exported from the system's OpenGL module (ex: OpenGL32.dll)
// --------------------------------------------------------

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTAPIClasses/Include/apFrameTerminators.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsWrappersCommon.h>
#include <src/gsExtensionsManager.h>
#include <src/gsOpenGLMonitor.h>

int WINAPI wglChoosePixelFormat(HDC a, CONST PIXELFORMATDESCRIPTOR* b)
{
    int retVal = -1;

    SU_START_WGL_FUNCTION_WRAPPER(ap_wglChoosePixelFormat);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglChoosePixelFormat, 2, OS_TOBJ_ID_WIN32_HDC_PARAMETER, a, OS_TOBJ_ID_P_PIXELFORMATDESCRIPTOR_PARAMETER, b);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglChoosePixelFormat(a, b);

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglChoosePixelFormat);

    return retVal;
}


BOOL WINAPI wglCopyContext(HGLRC a, HGLRC b, UINT c)
{
    BOOL retVal = -1;

    SU_START_WGL_FUNCTION_WRAPPER(ap_wglCopyContext);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglCopyContext, 3, OS_TOBJ_ID_WIN32_HGLRC_PARAMETER, a, OS_TOBJ_ID_WIN32_HGLRC_PARAMETER, b, OS_TOBJ_ID_WIN32_UINT_PARAMETER, c);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglCopyContext(a, b, c);

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglCopyContext);

    return retVal;
}


HGLRC WINAPI wglCreateContext(HDC deviceContextHandle)
{
    HGLRC retVal = 0;

    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_WGL_FUNCTION_WRAPPER(ap_wglCreateContext);

    if (!inNestedFunction)
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglCreateContext, 1, OS_TOBJ_ID_WIN32_HDC_PARAMETER, deviceContextHandle);
    }

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglCreateContext(deviceContextHandle);

    // If the context creation succeeded:
    if ((retVal != NULL) && (!inNestedFunction))
    {
        // Register the created context in our OpenGL monitor:
        gs_stat_openGLMonitorInstance.onContextCreation(deviceContextHandle, retVal, ap_wglCreateContext);
    }

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglCreateContext);

    return retVal;
}


HGLRC WINAPI wglCreateLayerContext(HDC deviceContextHandle, int iLayerPlane)
{
    HGLRC retVal = 0;

    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_WGL_FUNCTION_WRAPPER(ap_wglCreateLayerContext);

    // Notice:
    // Sigal 5/7/2010
    // This function is called from wglCreateContextAttribsARB. Therefore, when we are in this function wrapper,
    // we want to avoid performing our operations, when the function call stack depth is > 1
    if (!inNestedFunction)
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglCreateLayerContext, 2, OS_TOBJ_ID_WIN32_HDC_PARAMETER, deviceContextHandle, OS_TOBJ_ID_INT_PARAMETER, iLayerPlane);
    }

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglCreateLayerContext(deviceContextHandle, iLayerPlane);

    // If the context creation succeeded:
    if ((retVal != NULL) && (!inNestedFunction))
    {
        // Register the created context in our OpenGL monitor:
        gs_stat_openGLMonitorInstance.onContextCreation(deviceContextHandle, retVal, ap_wglCreateLayerContext);
    }

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglCreateLayerContext);

    return retVal;
}


BOOL WINAPI wglDeleteContext(HGLRC a)
{
    BOOL retVal = FALSE;

    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_WGL_FUNCTION_WRAPPER(ap_wglDeleteContext);

    if (!inNestedFunction)
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglDeleteContext, 1, OS_TOBJ_ID_WIN32_HGLRC_PARAMETER, a);

        // Mark that the context was deleted:
        gs_stat_openGLMonitorInstance.beforeContextDeletion(a);
    }

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglDeleteContext(a);

    if (!inNestedFunction)
    {
        // If the context deletion failed:
        if (retVal != TRUE)
        {
            // Report a detected error:
            gtString errorDescription = GS_STR_renderContextDeletionFailed;
            apErrorCode errorCode = AP_RENDER_CONTEXT_DELETION_FAILED_ERROR;
            gs_stat_openGLMonitorInstance.reportDetectedError(errorCode, errorDescription, ap_wglDeleteContext);
        }
        else
        {
            // The context deletion succeeded:

            // Get the deleted context spy id:
            int deletedContextId = gs_stat_openGLMonitorInstance.renderContextSpyId(a);

            // Get this thread current context:
            gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

            if (pCurrentThreadRenderContextMonitor)
            {
                // Get this thread current context spy id:
                int currentContextId = pCurrentThreadRenderContextMonitor->spyId();

                // If the deleted render context is this thread's current render context:
                if (currentContextId == deletedContextId)
                {
                    // From MSDN documentation of wglDeleteContext:
                    // "If a rendering context is the calling thread's current context, the wglDeleteContext
                    //  function changes the rendering context to being not current before deleting it".
                    gs_stat_openGLMonitorInstance.onContextMadeCurrent(NULL, 0, 0, NULL);
                }
            }

            // Mark that the context was deleted:
            gs_stat_openGLMonitorInstance.afterContextDeletion(a);
        }
    }

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglDeleteContext);

    return retVal;
}


BOOL WINAPI wglDescribeLayerPlane(HDC a, int b, int c, UINT d, LPLAYERPLANEDESCRIPTOR e)
{
    BOOL retVal = FALSE;

    SU_START_WGL_FUNCTION_WRAPPER(ap_wglDescribeLayerPlane);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglDescribeLayerPlane, 5, OS_TOBJ_ID_WIN32_HDC_PARAMETER, a, OS_TOBJ_ID_INT_PARAMETER, b, OS_TOBJ_ID_INT_PARAMETER, c, OS_TOBJ_ID_WIN32_UINT_PARAMETER, d, OS_TOBJ_ID_LP_LAYERPLANEDESCRIPTOR_PARAMETER, e);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglDescribeLayerPlane(a, b, c, d, e);

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglDescribeLayerPlane);

    return retVal;
}


int WINAPI wglDescribePixelFormat(HDC a, int b, UINT c, LPPIXELFORMATDESCRIPTOR d)
{
    BOOL retVal = 0;

    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_WGL_FUNCTION_WRAPPER(ap_wglDescribePixelFormat);

    if (!inNestedFunction)
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglDescribePixelFormat, 4, OS_TOBJ_ID_WIN32_HDC_PARAMETER, a, OS_TOBJ_ID_INT_PARAMETER, b, OS_TOBJ_ID_WIN32_UINT_PARAMETER, c, OS_TOBJ_ID_LP_PIXELFORMATDESCRIPTOR_PARAMETER, d);
    }

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglDescribePixelFormat(a, b, c, d);

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglDescribePixelFormat);

    return retVal;
}


HGLRC WINAPI wglGetCurrentContext()
{
    HGLRC retVal = 0;

    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_WGL_FUNCTION_WRAPPER(ap_wglGetCurrentContext);

    if (!inNestedFunction)
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglGetCurrentContext, 0);
    }

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglGetCurrentContext();

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglGetCurrentContext);

    return retVal;
}


HDC WINAPI wglGetCurrentDC()
{
    HDC retVal = 0;

    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_WGL_FUNCTION_WRAPPER(ap_wglGetCurrentDC);

    if (!inNestedFunction)
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglGetCurrentDC, 0);
    }

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglGetCurrentDC();

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglGetCurrentDC);

    return retVal;
}


PROC WINAPI wglGetDefaultProcAddress(LPCSTR a)
{
    PROC retVal = NULL;

    SU_START_WGL_FUNCTION_WRAPPER(ap_wglGetDefaultProcAddress);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglGetDefaultProcAddress, 1, OS_TOBJ_ID_STRING_PARAMETER, a);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglGetDefaultProcAddress(a);

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglGetDefaultProcAddress);

    return retVal;
}


int WINAPI wglGetLayerPaletteEntries(HDC a, int b, int c, int d, COLORREF* e)
{
    int retVal = 0;

    SU_START_WGL_FUNCTION_WRAPPER(ap_wglGetLayerPaletteEntries);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglGetLayerPaletteEntries, 5, OS_TOBJ_ID_WIN32_HDC_PARAMETER, a, OS_TOBJ_ID_INT_PARAMETER, b, OS_TOBJ_ID_INT_PARAMETER, c, OS_TOBJ_ID_INT_PARAMETER, d, OS_TOBJ_ID_P_COLORREF_PARAMETER, e);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglGetLayerPaletteEntries(a, b, c, d, e);

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglGetLayerPaletteEntries);

    return retVal;
}


int WINAPI wglGetPixelFormat(HDC a)
{
    int retVal = 0;

    // Yaki 7/5/2007:
    // On Vista, this internal function seems to be called a lot of times by the OS, this
    // makes it appear a lot of times in the calls history log.
    // To resolve this problem, we don't log the call to this internal function.
    // TO_DO: In future, we should log the real API function: GetPixelFormat, that
    //        resides in gdi32.dll.

    // SU_START_WGL_FUNCTION_WRAPPER( ap_wglGetPixelFormat );

    // Log the call to this function:
    //gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglGetPixelFormat, 1, OS_TOBJ_ID_WIN32_HDC_PARAMETER, a);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglGetPixelFormat(a);

    // SU_END_WGL_FUNCTION_WRAPPER( ap_wglGetPixelFormat );

    return retVal;
}


PROC WINAPI wglGetProcAddress(LPCSTR a)
{
    PROC retVal = NULL;

    // If we should not log the wglGetProcAddress call:
    bool areInitFunctionsLogged = gsAreInitializationFunctionsLogged();

    if (!areInitFunctionsLogged)
    {
        // Call the real function:
        retVal = gs_stat_realFunctionPointers.wglGetProcAddress(a);
    }
    else
    {
        SU_START_WGL_FUNCTION_WRAPPER(ap_wglGetProcAddress);

        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglGetProcAddress, 1, OS_TOBJ_ID_STRING_PARAMETER, a);

        // Return the wrapper function address:
        retVal = (PROC)(gs_stat_extensionsManager.wrapperFunctionAddress(a));

        SU_END_WGL_FUNCTION_WRAPPER(ap_wglGetProcAddress);
    }

    return retVal;
}


BOOL WINAPI wglMakeCurrent(HDC a, HGLRC b)
{
    BOOL retVal = FALSE;

    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_WGL_FUNCTION_WRAPPER(ap_wglMakeCurrent);

    if (!inNestedFunction)
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglMakeCurrent, 2, OS_TOBJ_ID_WIN32_HDC_PARAMETER, a, OS_TOBJ_ID_WIN32_HGLRC_PARAMETER, b);

        // If we are defines as a frame terminator:
        unsigned int frameTerminatorsMask = suFrameTerminatorsMask();

        if (frameTerminatorsMask & AP_MAKE_CURRENT_TERMINATOR)
        {
            // Terminate the current frame:
            gs_stat_openGLMonitorInstance.onFrameTerminatorCall();
        }

        // Notify the OpenGL monitor that the thread current render context is going to change:
        gs_stat_openGLMonitorInstance.beforeContextMadeCurrent(b);
    }

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglMakeCurrent(a, b);

    if (!inNestedFunction)
    {
        if (retVal != FALSE)
        {
            // Update the OpenGL monitor active context:
            gs_stat_openGLMonitorInstance.onContextMadeCurrent(a, 0, 0, b);

            // Update the PBuffers monitor:
            gsPBuffersMonitor& pbufferMtr = gs_stat_openGLMonitorInstance.pbuffersMonitor();
            pbufferMtr.onWGLMakeCurrent(a, b);
        }
        else
        {
            // From MSDN documentation of wglMakeCurrent:
            // "If an error occurs, the wglMakeCurrent function makes the thread's
            //  current rendering context not current before returning".
            gs_stat_openGLMonitorInstance.onContextMadeCurrent(NULL, 0, 0, NULL);
        }
    }

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglMakeCurrent);

    return retVal;
}


BOOL WINAPI wglRealizeLayerPalette(HDC a, int b, BOOL c)
{
    BOOL retVal = FALSE;

    SU_START_WGL_FUNCTION_WRAPPER(ap_wglRealizeLayerPalette);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglRealizeLayerPalette, 3, OS_TOBJ_ID_WIN32_HDC_PARAMETER, a, OS_TOBJ_ID_INT_PARAMETER, b, OS_TOBJ_ID_WIN32_BOOL_PARAMETER, c);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglRealizeLayerPalette(a, b, c);

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglRealizeLayerPalette);

    return retVal;
}


int WINAPI wglSetLayerPaletteEntries(HDC a, int b, int c, int d, CONST COLORREF* e)
{
    int retVal = 0;

    SU_START_WGL_FUNCTION_WRAPPER(ap_wglSetLayerPaletteEntries);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglSetLayerPaletteEntries, 5, OS_TOBJ_ID_WIN32_HDC_PARAMETER, a, OS_TOBJ_ID_INT_PARAMETER, b, OS_TOBJ_ID_INT_PARAMETER, c, OS_TOBJ_ID_INT_PARAMETER, d, OS_TOBJ_ID_P_COLORREF_PARAMETER, e);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglSetLayerPaletteEntries(a, b, c, d, e);

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglSetLayerPaletteEntries);

    return retVal;
}


BOOL WINAPI wglSetPixelFormat(HDC a, int b, CONST PIXELFORMATDESCRIPTOR* c)
{
    BOOL retVal = FALSE;

    SU_START_WGL_FUNCTION_WRAPPER(ap_wglSetPixelFormat);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglSetPixelFormat, 3, OS_TOBJ_ID_WIN32_HDC_PARAMETER, a, OS_TOBJ_ID_INT_PARAMETER, b, OS_TOBJ_ID_P_PIXELFORMATDESCRIPTOR_PARAMETER, c);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglSetPixelFormat(a, b, c);

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglSetPixelFormat);

    return retVal;
}


BOOL WINAPI wglShareLists(HGLRC a, HGLRC b)
{
    BOOL retVal = FALSE;

    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_WGL_FUNCTION_WRAPPER(ap_wglShareLists);

    if (!inNestedFunction)
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglShareLists, 2, OS_TOBJ_ID_WIN32_HGLRC_PARAMETER, a, OS_TOBJ_ID_WIN32_HGLRC_PARAMETER, b);
    }

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglShareLists(a, b);

    if ((retVal == TRUE) && (!inNestedFunction))
    {
        // Register the sharing:
        gs_stat_openGLMonitorInstance.onShareLists(a, b);
    }

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglShareLists);

    return retVal;
}


BOOL WINAPI wglSwapBuffers(HDC a)
{
    BOOL retVal = FALSE;

    // If we should not log the wglSwapBuffers call:
    bool areInitFunctionsLogged = gsAreInitializationFunctionsLogged();

    if (!areInitFunctionsLogged)
    {
        // Call the real function:
        retVal = gs_stat_realFunctionPointers.wglSwapBuffers(a);
    }
    else
    {
        SU_START_WGL_FUNCTION_WRAPPER(ap_wglSwapBuffers);

        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglSwapBuffers, 1, OS_TOBJ_ID_WIN32_HDC_PARAMETER, a);

        // If we are defines as a frame terminator:
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

        // If the OpenGL front draw buffer is forced:
        if (isFrontDrawBuffForced)
        {
            // Since we are rendering into the front buffer - instead of calling wglSwapBuffers
            // we will call glFlush:
            gs_stat_realFunctionPointers.glFlush();
        }
        else
        {
            // Call the real function:
            retVal = gs_stat_realFunctionPointers.wglSwapBuffers(a);
        }

        SU_END_WGL_FUNCTION_WRAPPER(ap_wglSwapBuffers);
    }

    return retVal;
}


BOOL WINAPI wglSwapLayerBuffers(HDC hdc, UINT fuPlanes)
{
    BOOL retVal = FALSE;

    SU_START_WGL_FUNCTION_WRAPPER(ap_wglSwapLayerBuffers);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglSwapLayerBuffers, 2, OS_TOBJ_ID_WIN32_HDC_PARAMETER, hdc, OS_TOBJ_ID_WIN32_UINT_PARAMETER, fuPlanes);

    // Will get true iff we are in "Force front draw buffer" mode:
    bool isFrontDrawBuffForced = false;

    // If we are defines as a frame terminator:
    unsigned int frameTerminatorsMask = suFrameTerminatorsMask();

    if (frameTerminatorsMask & AP_SWAP_LAYER_BUFFERS_TERMINATOR)
    {
        // Terminate the current frame:
        gs_stat_openGLMonitorInstance.onFrameTerminatorCall();
    }

    // Get the current thread render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        // Check if front draw buffer is forced:
        gsForcedModesManager& forcedModesMgr = pCurrentThreadRenderContextMonitor->forcedModesManager();
        isFrontDrawBuffForced = forcedModesMgr.isStubForced(AP_OPENGL_FORCED_FRONT_DRAW_BUFFER_MODE);
    }

    // If the OpenGL front draw buffer is forced:
    if (isFrontDrawBuffForced)
    {
        // Since we are rendering into the front buffer - instead of calling wglSwapLayerBuffers
        // we will call glFlush:
        gs_stat_realFunctionPointers.glFlush();
    }
    else
    {
        // Call the real function:
        retVal = gs_stat_realFunctionPointers.wglSwapLayerBuffers(hdc, fuPlanes);
    }

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglSwapLayerBuffers);

    return retVal;
}


DWORD WINAPI wglSwapMultipleBuffers(UINT a, const WGLSWAP* b)
{
    DWORD retVal = 0xFFFFFFFFUL;

    SU_START_WGL_FUNCTION_WRAPPER(ap_wglSwapMultipleBuffers);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglSwapMultipleBuffers, 2, OS_TOBJ_ID_WIN32_UINT_PARAMETER, a, OS_TOBJ_ID_P_WGLSWAP_PARAMETER, b);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglSwapMultipleBuffers(a, b);

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglSwapMultipleBuffers);

    return retVal;
}


BOOL WINAPI wglUseFontBitmapsA(HDC a, DWORD b, DWORD c, DWORD d)
{
    BOOL retVal = FALSE;

    SU_START_WGL_FUNCTION_WRAPPER(ap_wglUseFontBitmapsA);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglUseFontBitmapsA, 4, OS_TOBJ_ID_WIN32_HDC_PARAMETER, a, OS_TOBJ_ID_WIN32_DWORD_PARAMETER, b, OS_TOBJ_ID_WIN32_DWORD_PARAMETER, c, OS_TOBJ_ID_WIN32_DWORD_PARAMETER, d);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglUseFontBitmapsA(a, b, c, d);

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglUseFontBitmapsA);

    return retVal;
}


BOOL WINAPI wglUseFontBitmapsW(HDC a, DWORD b, DWORD c, DWORD d)
{
    BOOL retVal = FALSE;

    SU_START_WGL_FUNCTION_WRAPPER(ap_wglUseFontBitmapsW);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglUseFontBitmapsW, 4, OS_TOBJ_ID_WIN32_HDC_PARAMETER, a, OS_TOBJ_ID_WIN32_DWORD_PARAMETER, b, OS_TOBJ_ID_WIN32_DWORD_PARAMETER, c, OS_TOBJ_ID_WIN32_DWORD_PARAMETER, d);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglUseFontBitmapsW(a, b, c, d);

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglUseFontBitmapsW);

    return retVal;
}


BOOL WINAPI wglUseFontOutlinesA(HDC a, DWORD b, DWORD c, DWORD d, FLOAT e, FLOAT f, int g, LPGLYPHMETRICSFLOAT h)
{
    BOOL retVal = FALSE;

    SU_START_WGL_FUNCTION_WRAPPER(ap_wglUseFontOutlinesA);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglUseFontOutlinesA, 8, OS_TOBJ_ID_WIN32_HDC_PARAMETER, a, OS_TOBJ_ID_WIN32_DWORD_PARAMETER, b, OS_TOBJ_ID_WIN32_DWORD_PARAMETER, c, OS_TOBJ_ID_WIN32_DWORD_PARAMETER, d, OS_TOBJ_ID_FLOAT_PARAMETER, e, OS_TOBJ_ID_FLOAT_PARAMETER, f, OS_TOBJ_ID_INT_PARAMETER, g, OS_TOBJ_ID_LP_GLYPHMETRICSFLOAT_PARAMETER, h);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglUseFontOutlinesA(a, b, c, d, e, f, g, h);

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglUseFontOutlinesA);

    return retVal;
}


BOOL WINAPI wglUseFontOutlinesW(HDC a, DWORD b, DWORD c, DWORD d, FLOAT e, FLOAT f, int g, LPGLYPHMETRICSFLOAT h)
{
    BOOL retVal = FALSE;

    SU_START_WGL_FUNCTION_WRAPPER(ap_wglUseFontOutlinesW);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_wglUseFontOutlinesW, 8, OS_TOBJ_ID_WIN32_HDC_PARAMETER, a, OS_TOBJ_ID_WIN32_DWORD_PARAMETER, b, OS_TOBJ_ID_WIN32_DWORD_PARAMETER, c, OS_TOBJ_ID_WIN32_DWORD_PARAMETER, d, OS_TOBJ_ID_FLOAT_PARAMETER, e, OS_TOBJ_ID_FLOAT_PARAMETER, f, OS_TOBJ_ID_INT_PARAMETER, g, OS_TOBJ_ID_LP_GLYPHMETRICSFLOAT_PARAMETER, h);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.wglUseFontOutlinesW(a, b, c, d, e, f, g, h);

    SU_END_WGL_FUNCTION_WRAPPER(ap_wglUseFontOutlinesW);

    return retVal;
}




