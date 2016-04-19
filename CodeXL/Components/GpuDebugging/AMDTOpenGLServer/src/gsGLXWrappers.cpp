//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsGLXWrappers.cpp
///
//==================================================================================

//------------------------------ gsGLXWrappers.cpp ------------------------------

// --------------------------------------------------------
// File:
// This file contains a wrapper function for GLX "base" functions
// (GLX functions that are exported from the system's OpenGL module (ex: libGL))
// --------------------------------------------------------

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTAPIClasses/Include/apDefinitions.h>
#include <AMDTAPIClasses/Include/apGLRenderContextGraphicsInfo.h>
#include <AMDTAPIClasses/Include/apFrameTerminators.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsWrappersCommon.h>
#include <src/gsExtensionsManager.h>
#include <src/gsOpenGLMonitor.h>

// Aid types definitions:
typedef void (*gsGLXextFuncPtr)();


// TO_DO: LNX: Implement the function logging and the special GLX parameters that are required.
// Example: // gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXCopyContext, 2, OS_TOBJ_ID_);

XVisualInfo* glXChooseVisual(Display* dpy, int screen, int* attribList)
{
    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_FUNCTION_WRAPPER(ap_glXChooseVisual);

    if (!inNestedFunction)
    {
        // Log the call to this function:
        // TO_DO: LNX - log the attribList and enums
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXChooseVisual, 3, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_INT_PARAMETER, screen, OS_TOBJ_ID_P_INT_PARAMETER, attribList);
    }

    // Call the real function:
    XVisualInfo* retVal = gs_stat_realFunctionPointers.glXChooseVisual(dpy, screen, attribList);

    SU_END_FUNCTION_WRAPPER(ap_glXChooseVisual);

    return retVal;
}


GLXContext glXCreateContext(Display* dpy, XVisualInfo* vis, GLXContext shareList, Bool direct)
{
    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_FUNCTION_WRAPPER(ap_glXCreateContext);

    if (!inNestedFunction)
    {
        // Log the call to this function:
        // TO_DO: LNX: Log the XVisualInfo parameters (see /usr/include/X11/Xutil.h for XVisualInfo definition).
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXCreateContext, 4, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_POINTER_PARAMETER, vis, OS_TOBJ_ID_POINTER_PARAMETER, shareList, OS_TOBJ_ID_X11_BOOL_PARAMETER, direct);
    }

    // Call the real function:
    GLXContext retVal = gs_stat_realFunctionPointers.glXCreateContext(dpy, vis, shareList, direct);

    // If the context creation succeeded:
    if ((retVal != NULL) && (!inNestedFunction))
    {
        // Register the created context in our OpenGL monitor:
        gs_stat_openGLMonitorInstance.onContextCreation(dpy, retVal, ap_glXCreateContext);

        // Mark the context's VisualID:
        int renderContextID = gs_stat_openGLMonitorInstance.renderContextSpyId(retVal);
        VisualID visId = vis->visualid;

        if (visId == 0)
        {
            visId = XVisualIDFromVisual(vis->visual);
        }

        if (renderContextID != AP_NULL_CONTEXT_ID)
        {
            gsRenderContextMonitor* pRenderContextMonitor = gs_stat_openGLMonitorInstance.renderContextMonitor(renderContextID);
            GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
            {
                pRenderContextMonitor->setPixelFormatId(visId);
            }
        }

        // If we share another context's list, register this fact:
        if (shareList != NULL)
        {
            gs_stat_openGLMonitorInstance.onShareLists(shareList, retVal);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_glXCreateContext);

    return retVal;
}


void glXDestroyContext(Display* dpy, GLXContext ctx)
{
    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_FUNCTION_WRAPPER(ap_glXDestroyContext);

    if (!inNestedFunction)
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXDestroyContext, 2, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_POINTER_PARAMETER, ctx);

        // Mark that the context was deleted:
        gs_stat_openGLMonitorInstance.beforeContextDeletion(ctx);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glXDestroyContext(dpy, ctx);

    if (!inNestedFunction)
    {
        // Mark that the context was deleted:
        gs_stat_openGLMonitorInstance.afterContextDeletion(ctx);
    }

    SU_END_FUNCTION_WRAPPER(ap_glXDestroyContext);
}


Bool glXMakeCurrent(Display* dpy, GLXDrawable drawable, GLXContext ctx)
{
    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_FUNCTION_WRAPPER(ap_glXMakeCurrent);

    if (!inNestedFunction)
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXMakeCurrent, 3, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_XORG_CARD32_PARAMETER, drawable, OS_TOBJ_ID_POINTER_PARAMETER, ctx);

        // If we are defines as a frame terminator:
        unsigned int frameTerminatorsMask = suFrameTerminatorsMask();

        if (frameTerminatorsMask & AP_MAKE_CURRENT_TERMINATOR)
        {
            // Terminate the current frame:
            gs_stat_openGLMonitorInstance.onFrameTerminatorCall();
        }

        // Notify the OpenGL monitor that the thread current render context is going to change:
        gs_stat_openGLMonitorInstance.beforeContextMadeCurrent(ctx);
    }

    // Call the real function:
    Bool retVal = gs_stat_realFunctionPointers.glXMakeCurrent(dpy, drawable, ctx);

    if ((retVal == True) && (!inNestedFunction))
    {
        // Update the OpenGL monitor active context:
        gs_stat_openGLMonitorInstance.onContextMadeCurrent(dpy, drawable, drawable, ctx);

        // Update the PBuffers monitor:
        gsPBuffersMonitor& pbufferMtr = gs_stat_openGLMonitorInstance.pbuffersMonitor();
        pbufferMtr.onGLXMakeCurrent(dpy, drawable, drawable, ctx);
    }

    SU_END_FUNCTION_WRAPPER(ap_glXMakeCurrent);

    return retVal;
}


void glXCopyContext(Display* dpy, GLXContext src, GLXContext dst, unsigned long mask)
{
    SU_START_FUNCTION_WRAPPER(ap_glXCopyContext);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXCopyContext, 4, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_POINTER_PARAMETER, src, OS_TOBJ_ID_POINTER_PARAMETER, dst, OS_TOBJ_ID_LONG_BITFIELD_PARAMETER, mask);

    // Call the real function:
    gs_stat_realFunctionPointers.glXCopyContext(dpy, src, dst, mask);

    SU_END_FUNCTION_WRAPPER(ap_glXCopyContext);
}


GLXPixmap glXCreateGLXPixmap(Display* dpy, XVisualInfo* vis, Pixmap pixmap)
{
    SU_START_FUNCTION_WRAPPER(ap_glXCreateGLXPixmap);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXCreateGLXPixmap, 3, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_POINTER_PARAMETER, vis, OS_TOBJ_ID_XID_PARAMETER, pixmap);

    // Call the real function:
    GLXPixmap retVal = gs_stat_realFunctionPointers.glXCreateGLXPixmap(dpy, vis, pixmap);

    SU_END_FUNCTION_WRAPPER(ap_glXCreateGLXPixmap);

    return retVal;
}


void glXDestroyGLXPixmap(Display* dpy, GLXPixmap pix)
{
    SU_START_FUNCTION_WRAPPER(ap_glXDestroyGLXPixmap);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXDestroyGLXPixmap, 2, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_XID_PARAMETER, pix);

    // Call the real function:
    gs_stat_realFunctionPointers.glXDestroyGLXPixmap(dpy, pix);

    SU_END_FUNCTION_WRAPPER(ap_glXDestroyGLXPixmap);
}


int glXGetConfig(Display* dpy, XVisualInfo* vis, int attrib, int* value)
{
    SU_START_FUNCTION_WRAPPER(ap_glXGetConfig);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXGetConfig, 4, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_POINTER_PARAMETER, vis, OS_TOBJ_ID_GLX_ENUM_PARAMETER, attrib, OS_TOBJ_ID_P_INT_PARAMETER, value);

    // Call the real function:
    int retVal = gs_stat_realFunctionPointers.glXGetConfig(dpy, vis, attrib, value);

    SU_END_FUNCTION_WRAPPER(ap_glXGetConfig);

    return retVal;
}


GLXContext glXGetCurrentContext(void)
{
    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_FUNCTION_WRAPPER(ap_glXGetCurrentContext);

    if (!inNestedFunction)
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXGetCurrentContext, 0);
    }

    // Call the real function:
    GLXContext retVal = gs_stat_realFunctionPointers.glXGetCurrentContext();

    SU_END_FUNCTION_WRAPPER(ap_glXGetCurrentContext);

    return retVal;
}


GLXDrawable glXGetCurrentDrawable(void)
{
    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_FUNCTION_WRAPPER(ap_glXGetCurrentDrawable);

    if (!inNestedFunction)
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXGetCurrentDrawable, 0);
    }

    // Call the real function:
    GLXDrawable retVal = gs_stat_realFunctionPointers.glXGetCurrentDrawable();

    SU_END_FUNCTION_WRAPPER(ap_glXGetCurrentDrawable);

    return retVal;
}


Bool glXIsDirect(Display* dpy, GLXContext ctx)
{
    SU_START_FUNCTION_WRAPPER(ap_glXIsDirect);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXIsDirect, 2, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_POINTER_PARAMETER, ctx);

    // Call the real function:
    Bool retVal = gs_stat_realFunctionPointers.glXIsDirect(dpy, ctx);

    SU_END_FUNCTION_WRAPPER(ap_glXIsDirect);

    return retVal;
}



Bool glXQueryExtension(Display* dpy, int* errorBase, int* eventBase)
{
    SU_START_FUNCTION_WRAPPER(ap_glXQueryExtension);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXQueryExtension, 3, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_P_INT_PARAMETER, errorBase, OS_TOBJ_ID_P_INT_PARAMETER, eventBase);

    // Call the real function:
    Bool retVal = gs_stat_realFunctionPointers.glXQueryExtension(dpy, errorBase, eventBase);

    SU_END_FUNCTION_WRAPPER(ap_glXQueryExtension);

    return retVal;
}


Bool glXQueryVersion(Display* dpy, int* major, int* minor)
{
    SU_START_FUNCTION_WRAPPER(ap_glXQueryVersion);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXQueryVersion, 3, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_P_INT_PARAMETER, major, OS_TOBJ_ID_P_INT_PARAMETER, minor);

    // Call the real function:
    Bool retVal = gs_stat_realFunctionPointers.glXQueryVersion(dpy, major, minor);

    SU_END_FUNCTION_WRAPPER(ap_glXQueryVersion);

    return retVal;
}


void glXSwapBuffers(Display* dpy, GLXDrawable drawable)
{
    SU_START_FUNCTION_WRAPPER(ap_glXSwapBuffers);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXSwapBuffers, 2, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_XID_PARAMETER, drawable);

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
        // Since we are rendering into the front buffer - instead of calling glXSwapBuffers
        // we will call glFlush:
        gs_stat_realFunctionPointers.glFlush();
    }
    else
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glXSwapBuffers(dpy, drawable);
    }

    SU_END_FUNCTION_WRAPPER(ap_glXSwapBuffers);
}


void glXUseXFont(Font font, int first, int count, int listBase)
{
    SU_START_FUNCTION_WRAPPER(ap_glXUseXFont);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXUseXFont, 4, OS_TOBJ_ID_XID_PARAMETER, font, OS_TOBJ_ID_INT_PARAMETER, first, OS_TOBJ_ID_INT_PARAMETER, count, OS_TOBJ_ID_INT_PARAMETER, listBase);

    // Call the real function:
    gs_stat_realFunctionPointers.glXUseXFont(font, first, count, listBase);

    SU_END_FUNCTION_WRAPPER(ap_glXUseXFont);
}


void glXWaitGL(void)
{
    SU_START_FUNCTION_WRAPPER(ap_glXWaitGL);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXWaitGL, 0);

    // Call the real function:
    gs_stat_realFunctionPointers.glXWaitGL();

    SU_END_FUNCTION_WRAPPER(ap_glXWaitGL);
}


void glXWaitX(void)
{
    SU_START_FUNCTION_WRAPPER(ap_glXWaitX);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXWaitX, 0);

    // Call the real function:
    gs_stat_realFunctionPointers.glXWaitX();

    SU_END_FUNCTION_WRAPPER(ap_glXWaitX);
}


const char* glXGetClientString(Display* dpy, int name)
{
    SU_START_FUNCTION_WRAPPER(ap_glXGetClientString);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXGetClientString, 2, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_INT_PARAMETER, name);

    // Call the real function:
    const char* retVal = gs_stat_realFunctionPointers.glXGetClientString(dpy, name);

    SU_END_FUNCTION_WRAPPER(ap_glXGetClientString);

    return retVal;
}


const char* glXQueryServerString(Display* dpy, int screen, int name)
{
    SU_START_FUNCTION_WRAPPER(ap_glXQueryServerString);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXQueryServerString, 3, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_INT_PARAMETER, screen, OS_TOBJ_ID_INT_PARAMETER, name);

    // Call the real function:
    const char* retVal = gs_stat_realFunctionPointers.glXQueryServerString(dpy, screen, name);

    SU_END_FUNCTION_WRAPPER(ap_glXQueryServerString);

    return retVal;
}


const char* glXQueryExtensionsString(Display* dpy, int screen)
{
    SU_START_FUNCTION_WRAPPER(ap_glXQueryExtensionsString);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXQueryExtensionsString, 2, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_INT_PARAMETER, screen);

    // Yaki 27/7/2007:
    // Notice: This function should return only GLX extensions related strings.
    //         Since we don't have (yet) gremedy extensions for GLX, we don't need
    //         to add items to the returned string.

    // Call the real function:
    const char* retVal = gs_stat_realFunctionPointers.glXQueryExtensionsString(dpy, screen);

    SU_END_FUNCTION_WRAPPER(ap_glXQueryExtensionsString);

    return retVal;
}


GLXFBConfig* glXGetFBConfigs(Display* dpy, int screen, int* nelements)
{
    SU_START_FUNCTION_WRAPPER(ap_glXGetFBConfigs);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXGetFBConfigs, 3, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_INT_PARAMETER, screen, OS_TOBJ_ID_INT_PARAMETER, nelements);

    // Call the real function:
    GLXFBConfig* retVal = gs_stat_realFunctionPointers.glXGetFBConfigs(dpy, screen, nelements);

    SU_END_FUNCTION_WRAPPER(ap_glXGetFBConfigs);

    return retVal;
}


GLXFBConfig* glXChooseFBConfig(Display* dpy, int screen, const int* attrib_list, int* nelements)
{
    SU_START_FUNCTION_WRAPPER(ap_glXChooseFBConfig);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXChooseFBConfig, 4, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_INT_PARAMETER, screen, OS_TOBJ_ID_P_INT_PARAMETER, attrib_list, OS_TOBJ_ID_P_INT_PARAMETER, nelements);

    // Call the real function:
    GLXFBConfig* retVal = gs_stat_realFunctionPointers.glXChooseFBConfig(dpy, screen, attrib_list, nelements);

    SU_END_FUNCTION_WRAPPER(ap_glXChooseFBConfig);

    return retVal;
}


int glXGetFBConfigAttrib(Display* dpy, GLXFBConfig config, int attribute, int* value)
{
    SU_START_FUNCTION_WRAPPER(ap_glXGetFBConfigAttrib);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXGetFBConfigAttrib, 4, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_POINTER_PARAMETER, config, OS_TOBJ_ID_GLX_ENUM_PARAMETER, attribute, OS_TOBJ_ID_P_INT_PARAMETER, value);

    // Call the real function:
    int retVal = gs_stat_realFunctionPointers.glXGetFBConfigAttrib(dpy, config, attribute, value);

    SU_END_FUNCTION_WRAPPER(ap_glXGetFBConfigAttrib);

    return retVal;
}


XVisualInfo* glXGetVisualFromFBConfig(Display* dpy, GLXFBConfig config)
{
    SU_START_FUNCTION_WRAPPER(ap_glXGetVisualFromFBConfig);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXGetVisualFromFBConfig, 2, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_POINTER_PARAMETER, config);

    // Call the real function:
    XVisualInfo* retVal = gs_stat_realFunctionPointers.glXGetVisualFromFBConfig(dpy, config);

    SU_END_FUNCTION_WRAPPER(ap_glXGetVisualFromFBConfig);

    return retVal;
}


GLXWindow glXCreateWindow(Display* dpy, GLXFBConfig config, Window win, const int* attrib_list)
{
    SU_START_FUNCTION_WRAPPER(ap_glXCreateWindow);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXCreateWindow, 4, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_POINTER_PARAMETER, config, OS_TOBJ_ID_XID_PARAMETER, win, OS_TOBJ_ID_P_INT_PARAMETER, attrib_list);

    // Call the real function:
    GLXWindow retVal = gs_stat_realFunctionPointers.glXCreateWindow(dpy, config, win, attrib_list);

    SU_END_FUNCTION_WRAPPER(ap_glXCreateWindow);

    return retVal;
}


void glXDestroyWindow(Display* dpy, GLXWindow win)
{
    SU_START_FUNCTION_WRAPPER(ap_glXDestroyWindow);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXDestroyWindow, 2, OS_TOBJ_ID_P_INT_PARAMETER, dpy, OS_TOBJ_ID_XID_PARAMETER, win);

    // Call the real function:
    gs_stat_realFunctionPointers.glXDestroyWindow(dpy, win);

    SU_END_FUNCTION_WRAPPER(ap_glXDestroyWindow);
}


GLXPixmap glXCreatePixmap(Display* dpy, GLXFBConfig config, Pixmap pixmap, const int* attrib_list)
{
    SU_START_FUNCTION_WRAPPER(ap_glXCreatePixmap);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXCreatePixmap, 4, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_POINTER_PARAMETER, config, OS_TOBJ_ID_XID_PARAMETER, pixmap, OS_TOBJ_ID_P_INT_PARAMETER, attrib_list);

    // Call the real function:
    GLXPixmap retVal = gs_stat_realFunctionPointers.glXCreatePixmap(dpy, config, pixmap, attrib_list);

    SU_END_FUNCTION_WRAPPER(ap_glXCreatePixmap);

    return retVal;
}


void glXDestroyPixmap(Display* dpy, GLXPixmap pixmap)
{
    SU_START_FUNCTION_WRAPPER(ap_glXDestroyPixmap);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXDestroyPixmap, 2, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_XID_PARAMETER, pixmap);

    // Call the real function:
    gs_stat_realFunctionPointers.glXDestroyPixmap(dpy, pixmap);

    SU_END_FUNCTION_WRAPPER(ap_glXDestroyPixmap);
}


GLXPbuffer glXCreatePbuffer(Display* dpy, GLXFBConfig config, const int* attrib_list)
{
    SU_START_FUNCTION_WRAPPER(ap_glXCreatePbuffer);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXCreatePbuffer, 3, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_POINTER_PARAMETER, config, OS_TOBJ_ID_P_INT_PARAMETER, attrib_list);

    // Call the real function:
    GLXPbuffer retVal = gs_stat_realFunctionPointers.glXCreatePbuffer(dpy, config, attrib_list);

    if (retVal != 0)
    {
        // Iterate the attributes list to get the pbuffer's width and height:
        int width = 0;
        int height = 0;

        const int* pCurrentAttrib = attrib_list;

        while ((pCurrentAttrib != NULL) && (*pCurrentAttrib != None))
        {
            // Get the current attribute:
            int currentAttrib = *pCurrentAttrib;

            if (currentAttrib == GLX_PBUFFER_WIDTH)
            {
                pCurrentAttrib++;
                width = *pCurrentAttrib;
            }
            else if (currentAttrib == GLX_PBUFFER_HEIGHT)
            {
                pCurrentAttrib++;
                height = *pCurrentAttrib;
            }

            // Advance to the next attribute:
            pCurrentAttrib++;
        }

        // Get the PBuffer monitor:
        gsPBuffersMonitor& pbuffersMonitor = gs_stat_openGLMonitorInstance.pbuffersMonitor();

        // Log the created PBuffer:
        pbuffersMonitor.onPBufferCreation(retVal, dpy, 0, width, height, attrib_list);
    }

    SU_END_FUNCTION_WRAPPER(ap_glXCreatePbuffer);

    return retVal;
}


void glXDestroyPbuffer(Display* dpy, GLXPbuffer pbuf)
{
    SU_START_FUNCTION_WRAPPER(ap_glXDestroyPbuffer);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXDestroyPbuffer, 2, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_XID_PARAMETER, pbuf);

    // Call the real function:
    gs_stat_realFunctionPointers.glXDestroyPbuffer(dpy, pbuf);

    // Get the PBuffer monitor
    gsPBuffersMonitor& pbuffersMonitor = gs_stat_openGLMonitorInstance.pbuffersMonitor();

    // Delete the PBuffer from the PBuffer monitor:
    pbuffersMonitor.onPBufferDeletion(pbuf);

    SU_END_FUNCTION_WRAPPER(ap_glXDestroyPbuffer);
}


void glXQueryDrawable(Display* dpy, GLXDrawable draw, int attribute, unsigned int* value)
{
    SU_START_FUNCTION_WRAPPER(ap_glXQueryDrawable);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXQueryDrawable, 4, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_XID_PARAMETER, draw, OS_TOBJ_ID_GLX_ENUM_PARAMETER, attribute, OS_TOBJ_ID_P_INT_PARAMETER, value);

    // Call the real function:
    gs_stat_realFunctionPointers.glXQueryDrawable(dpy, draw, attribute, value);

    SU_END_FUNCTION_WRAPPER(ap_glXQueryDrawable);
}


GLXContext glXCreateNewContext(Display* dpy, GLXFBConfig config, int render_type, GLXContext share_list, Bool direct)
{
    SU_START_FUNCTION_WRAPPER(ap_glXCreateNewContext);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXCreateNewContext, 5 , OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_POINTER_PARAMETER, config, OS_TOBJ_ID_INT_PARAMETER, render_type, OS_TOBJ_ID_POINTER_PARAMETER, share_list, OS_TOBJ_ID_X11_BOOL_PARAMETER, direct);

    // Call the real function:
    GLXContext retVal = gs_stat_realFunctionPointers.glXCreateNewContext(dpy, config, render_type, share_list, direct);

    // If the context creation succeeded:
    if (retVal != NULL)
    {
        // Register the created context in our OpenGL monitor:
        gs_stat_openGLMonitorInstance.onContextCreation(dpy, retVal, ap_glXCreateNewContext);

        // Get the XVisualInfo:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glXGetVisualFromFBConfig);
        XVisualInfo* vis = glXGetVisualFromFBConfig(dpy, config);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glXGetVisualFromFBConfig);

        // Mark the context's VisualID:
        int renderContextID = gs_stat_openGLMonitorInstance.renderContextSpyId(retVal);
        VisualID visId = vis->visualid;

        if (visId == 0)
        {
            visId = XVisualIDFromVisual(vis->visual);
        }

        // Free the temporary XVisualInfo:
        XFree(vis);

        if (visId != AP_NULL_CONTEXT_ID)
        {
            gsRenderContextMonitor* pRenderContextMonitor = gs_stat_openGLMonitorInstance.renderContextMonitor(renderContextID);
            GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
            {
                pRenderContextMonitor->setPixelFormatId(visId);
            }
        }

        // If we share another context's list, register this fact:
        if (share_list != NULL)
        {
            gs_stat_openGLMonitorInstance.onShareLists(share_list, retVal);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_glXCreateNewContext);

    return retVal;
}


Bool glXMakeContextCurrent(Display* display, GLXDrawable draw, GLXDrawable read, GLXContext ctx)
{
    SU_START_FUNCTION_WRAPPER(ap_glXMakeContextCurrent);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXMakeContextCurrent, 4, OS_TOBJ_ID_POINTER_PARAMETER, display, OS_TOBJ_ID_XID_PARAMETER, draw, OS_TOBJ_ID_XID_PARAMETER, read, OS_TOBJ_ID_POINTER_PARAMETER, ctx);

    // If we are defines as a frame terminator:
    unsigned int frameTerminatorsMask = suFrameTerminatorsMask();

    if (frameTerminatorsMask & AP_MAKE_CURRENT_TERMINATOR)
    {
        // Terminate the current frame:
        gs_stat_openGLMonitorInstance.onFrameTerminatorCall();
    }

    // Notify the OpenGL monitor that the thread current render context is going to change:
    gs_stat_openGLMonitorInstance.beforeContextMadeCurrent(ctx);

    // Call the real function:
    Bool retVal = gs_stat_realFunctionPointers.glXMakeContextCurrent(display, draw, read, ctx);

    if (retVal != False)
    {
        // Update the OpenGL monitor active context:
        gs_stat_openGLMonitorInstance.onContextMadeCurrent(display, draw, read, ctx);

        // Update the PBuffers monitor:
        gsPBuffersMonitor& pbufferMtr = gs_stat_openGLMonitorInstance.pbuffersMonitor();
        pbufferMtr.onGLXMakeCurrent(display, draw, read, ctx);
    }

    SU_END_FUNCTION_WRAPPER(ap_glXMakeContextCurrent);

    return retVal;
}


GLXDrawable glXGetCurrentReadDrawable(void)
{
    SU_START_FUNCTION_WRAPPER(ap_glXGetCurrentReadDrawable);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXGetCurrentReadDrawable, 0);

    // Call the real function:
    GLXDrawable retVal = gs_stat_realFunctionPointers.glXGetCurrentReadDrawable();

    SU_END_FUNCTION_WRAPPER(ap_glXGetCurrentReadDrawable);

    return retVal;
}


Display* glXGetCurrentDisplay(void)
{
    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_FUNCTION_WRAPPER(ap_glXGetCurrentDisplay);

    if (!inNestedFunction)
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXGetCurrentDisplay, 0);
    }

    // Call the real function:
    Display* retVal = gs_stat_realFunctionPointers.glXGetCurrentDisplay();

    SU_END_FUNCTION_WRAPPER(ap_glXGetCurrentDisplay);

    return retVal;
}


int glXQueryContext(Display* dpy, GLXContext ctx, int attribute, int* value)
{
    SU_START_FUNCTION_WRAPPER(ap_glXQueryContext);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXQueryContext, 4, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_POINTER_PARAMETER, ctx, OS_TOBJ_ID_GLX_ENUM_PARAMETER, attribute, OS_TOBJ_ID_P_INT_PARAMETER, value);

    // Call the real function:
    int retVal = gs_stat_realFunctionPointers.glXQueryContext(dpy, ctx, attribute, value);

    SU_END_FUNCTION_WRAPPER(ap_glXQueryContext);

    return retVal;
}


void glXSelectEvent(Display* dpy, GLXDrawable draw, unsigned long event_mask)
{
    SU_START_FUNCTION_WRAPPER(ap_glXSelectEvent);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXSelectEvent, 3, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_XID_PARAMETER, draw, OS_TOBJ_ID_LONG_BITFIELD_PARAMETER, event_mask);

    // Call the real function:
    gs_stat_realFunctionPointers.glXSelectEvent(dpy, draw, event_mask);

    SU_END_FUNCTION_WRAPPER(ap_glXSelectEvent);
}


void glXGetSelectedEvent(Display* dpy, GLXDrawable draw, unsigned long* event_mask)
{
    SU_START_FUNCTION_WRAPPER(ap_glXGetSelectedEvent);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXGetSelectedEvent, 3, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_XID_PARAMETER, draw, OS_TOBJ_ID_POINTER_PARAMETER, event_mask);

    // Call the real function:
    gs_stat_realFunctionPointers.glXGetSelectedEvent(dpy, draw, event_mask);

    SU_END_FUNCTION_WRAPPER(ap_glXGetSelectedEvent);
}


void (*glXGetProcAddress(const GLubyte* procname))(void)
{
    SU_START_FUNCTION_WRAPPER(ap_glXGetProcAddress);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXGetProcAddress, 1, OS_TOBJ_ID_STRING_PARAMETER, procname);

    // Return the wrapper function address:
    gtASCIIString functionName = (const char*)procname;
    gsGLXextFuncPtr retVal = (gsGLXextFuncPtr)(gs_stat_extensionsManager.wrapperFunctionAddress(functionName));

    SU_END_FUNCTION_WRAPPER(ap_glXGetProcAddress);

    return retVal;
}


// --------------------------------- GLX extension functions -------------------------------------------


void (*glXGetProcAddressARB(const GLubyte* procName))(void)
{
    SU_START_FUNCTION_WRAPPER(ap_glXGetProcAddressARB);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXGetProcAddressARB, 1, OS_TOBJ_ID_STRING_PARAMETER, procName);

    // Return the wrapper function address:
    gtASCIIString functionName = (const char*)procName;
    gsGLXextFuncPtr retVal = (gsGLXextFuncPtr)(gs_stat_extensionsManager.wrapperFunctionAddress(functionName));

    // If we failed, try calling the real function:
    if (NULL == retVal)
    {
        SU_CALL_EXTENSION_FUNC_WITH_RET_VAL(glXGetProcAddressARB, (procName), retVal);
    }

    SU_END_FUNCTION_WRAPPER(ap_glXGetProcAddressARB);

    return retVal;
}

GLXContext glXCreateContextAttribsARB(Display* dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int* attrib_list)
{
    SU_START_FUNCTION_WRAPPER(ap_glXCreateContextAttribsARB);

    // Get the current execution mode:
    apExecutionMode currentExeuctionMode = suDebuggedProcessExecutionMode();

    // Force the debug flag if necessary:
    const int* spyAttribList = attrib_list;
    bool isDebugFlagForced = false;
    bool createdAttribList = false;

    if (!suGetGlobalServerEnvironmentSettings().m_gsDontForceOpenGLDebugContexts)
    {
        apGLRenderContextGraphicsInfo::forceDebugContext(currentExeuctionMode, attrib_list, spyAttribList, isDebugFlagForced);
        createdAttribList = true;
    }

    // Count the number of attributes to display them in the calls history view:
    int numberOfAttribs = 0;

    if (NULL != attrib_list)
    {
        // Add the name and value of each attribute
        while (0 != attrib_list[numberOfAttribs])
        {
            numberOfAttribs += 2;
        }

        // Add the terminating 0:
        numberOfAttribs++;
    }

    // Log the call to this function:
    if (numberOfAttribs > 0)
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXCreateContextAttribsARB, 5, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_POINTER_PARAMETER, config, OS_TOBJ_ID_POINTER_PARAMETER, share_context, OS_TOBJ_ID_X11_BOOL_PARAMETER, direct, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_INT_PARAMETER, numberOfAttribs, attrib_list);
    }
    else
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXCreateContextAttribsARB, 5, OS_TOBJ_ID_POINTER_PARAMETER, dpy, OS_TOBJ_ID_POINTER_PARAMETER, config, OS_TOBJ_ID_POINTER_PARAMETER, share_context, OS_TOBJ_ID_X11_BOOL_PARAMETER, direct, OS_TOBJ_ID_P_INT_PARAMETER, attrib_list);
    }

    // Call the real function:
    GLXContext retVal = NULL;
    SU_CALL_EXTENSION_FUNC_WITH_RET_VAL(glXCreateContextAttribsARB, (dpy, config, share_context, direct, spyAttribList), retVal);

    // If the context creation succeeded:
    if (retVal != NULL)
    {
        // Register the created context in our OpenGL monitor:
        gs_stat_openGLMonitorInstance.onContextCreation(dpy, retVal, ap_glXCreateContextAttribsARB, spyAttribList, isDebugFlagForced);

        // Get the XVisualInfo:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glXGetVisualFromFBConfig);
        XVisualInfo* vis = glXGetVisualFromFBConfig(dpy, config);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glXGetVisualFromFBConfig);

        // Mark the context's VisualID:
        int renderContextID = gs_stat_openGLMonitorInstance.renderContextSpyId(retVal);
        VisualID visId = vis->visualid;

        if (visId == 0)
        {
            visId = XVisualIDFromVisual(vis->visual);
        }

        // Free the temporary XVisualInfo:
        XFree(vis);

        if (renderContextID != AP_NULL_CONTEXT_ID)
        {
            gsRenderContextMonitor* pRenderContextMonitor = gs_stat_openGLMonitorInstance.renderContextMonitor(renderContextID);
            GT_IF_WITH_ASSERT(pRenderContextMonitor != NULL)
            {
                pRenderContextMonitor->setPixelFormatId(visId);
            }
        }

        // If we share another context's list, register this fact:
        if (share_context != NULL)
        {
            gs_stat_openGLMonitorInstance.onShareLists(share_context, retVal);
        }
    }

    // Release the attribute list:
    if (createdAttribList)
    {
        apGLRenderContextGraphicsInfo::releaseAttribListCreatedForDebugContextForcing(spyAttribList);
    }

    SU_END_FUNCTION_WRAPPER(ap_glXCreateContextAttribsARB);

    return retVal;
}


/*

GLXContextID glXGetContextIDEXT(const GLXContext ctx)
{
    SU_START_FUNCTION_WRAPPER( ap_glXGetContextIDEXT );

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXGetContextIDEXT, 0);

    // Call the real function:
    gs_stat_realFunctionPointers.glXGetContextIDEXT(ctx);

    SU_END_FUNCTION_WRAPPER( ap_glXGetContextIDEXT );
}


GLXDrawable glXGetCurrentDrawableEXT(void)
{
    SU_START_FUNCTION_WRAPPER( ap_glXGetCurrentDrawableEXT );

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXGetCurrentDrawableEXT, 0);

    // Call the real function:
    gs_stat_realFunctionPointers.glXGetCurrentDrawableEXT();

    SU_END_FUNCTION_WRAPPER( ap_glXGetCurrentDrawableEXT );
}


GLXContext glXImportContextEXT(Display *dpy, GLXContextID contextID)
{
    SU_START_FUNCTION_WRAPPER( ap_glXImportContextEXT );

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXImportContextEXT, 0);

    // Call the real function:
    gs_stat_realFunctionPointers.glXImportContextEXT(dpy, contextID);

    SU_END_FUNCTION_WRAPPER( ap_glXImportContextEXT );
}


void glXFreeContextEXT(Display *dpy, GLXContext ctx)
{
    SU_START_FUNCTION_WRAPPER( ap_glXFreeContextEXT );

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXFreeContextEXT, 0);

    // Call the real function:
    gs_stat_realFunctionPointers.glXFreeContextEXT(dpy, ctx);

    SU_END_FUNCTION_WRAPPER( ap_glXFreeContextEXT );
}


int glXQueryContextInfoEXT(Display *dpy, GLXContext ctx, int attribute, int *value)
{
    SU_START_FUNCTION_WRAPPER( ap_glXQueryContextInfoEXT );

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glXQueryContextInfoEXT, 0);

    // Call the real function:
    gs_stat_realFunctionPointers.glXQueryContextInfoEXT(dpy, ctx, attribute, value);

    SU_END_FUNCTION_WRAPPER( ap_glXQueryContextInfoEXT );
}

*/
