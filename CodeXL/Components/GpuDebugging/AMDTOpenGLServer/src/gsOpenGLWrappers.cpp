//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsOpenGLWrappers.cpp
///
//==================================================================================

//------------------------------ gsOpenGLWrappers.cpp ------------------------------


// --------------------------------------------------------
// File:
// This file contains a wrapper function for the "base"
// OpenGL functions (functions that are exported from the systems OpenGL module)
// --------------------------------------------------------

// OpenGL:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osTransferableObjectType.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
#include <AMDTAPIClasses/Include/apFrameTerminators.h>
#include <AMDTAPIClasses/Include/apGLenumParameter.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsVertexArrayDrawer.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsWrappersCommon.h>
#include <src/gsExtensionsManager.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsWrappersCommon.h>

// If we are building an OpenGL ES implementation DLL:
#ifdef OS_OGL_ES_IMPLEMENTATION_DLL_BUILD

    // OpenGL ES major and minor version:
    #define OGL_ES_MAJOR_VERSION 1
    #define OGL_ES_MINOR_VERSION 1

#endif

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))

unsigned int APIENTRY _loader_get_dispatch_table_size(void)
{
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_loaderGetDispatchTableSize, 0);

    return gs_stat_realFunctionPointers._loader_get_dispatch_table_size();
#else
    return 0;
#endif
}

int APIENTRY _loader_get_proc_offset(const char* name)
{
    (void)name;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_loaderGetProcOffset, 0);

    return gs_stat_realFunctionPointers._loader_get_proc_offset(name);
#else
    return 0;
#endif
}

int APIENTRY _loader_add_dispatch(const char* const* names, const char* signature)
{
    (void)names;
    (void)signature;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_loaderAddDispatch, 0);

    return gs_stat_realFunctionPointers._loader_add_dispatch(names, signature);
#else
    return 0;
#endif
}

void APIENTRY _loader_set_dispatch(const void* dispTable)
{
    (void)dispTable;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_loaderSetDispatch, 0);

    gs_stat_realFunctionPointers._loader_set_dispatch(dispTable);
#endif
}

#endif

// --------------------------------------------------------
//             OpenGL Wrapper functions
// --------------------------------------------------------

void APIENTRY glAccum(GLenum op, GLfloat value)
{
    SU_START_FUNCTION_WRAPPER(ap_glAccum);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glAccum, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, op, OS_TOBJ_ID_GL_FLOAT_PARAMETER, value);

    // Call the real function:
    gs_stat_realFunctionPointers.glAccum(op, value);

    SU_END_FUNCTION_WRAPPER(ap_glAccum);
}


void APIENTRY glAlphaFunc(GLenum func, GLclampf ref)
{
    SU_START_FUNCTION_WRAPPER(ap_glAlphaFunc);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glAlphaFunc, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, func, OS_TOBJ_ID_GL_CLAMPF_PARAMETER, ref);

    // Call the real function:
    gs_stat_realFunctionPointers.glAlphaFunc(func, ref);

    SU_END_FUNCTION_WRAPPER(ap_glAlphaFunc);
}


GLboolean APIENTRY glAreTexturesResident(GLsizei n, const GLuint* textures, GLboolean* residences)
{
    GLboolean retVal = false;

    SU_START_FUNCTION_WRAPPER(ap_glAreTexturesResident);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glAreTexturesResident, 3, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_GL_P_UINT_PARAMETER, textures, OS_TOBJ_ID_NOT_AVAILABLE_PARAMETER);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.glAreTexturesResident(n, textures, residences);

    SU_END_FUNCTION_WRAPPER(ap_glAreTexturesResident);

    return retVal;
}


void APIENTRY glArrayElement(GLint i)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glArrayElement);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glArrayElement, 1, OS_TOBJ_ID_GL_INT_PARAMETER, i);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onArrayElement(i);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glArrayElement(i);

    SU_END_FUNCTION_WRAPPER(ap_glArrayElement);
}


void APIENTRY glBegin(GLenum mode)
{
    SU_START_FUNCTION_WRAPPER(ap_glBegin);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glBegin, 1, OS_TOBJ_ID_GL_PRIMITIVE_TYPE_PARAMETER, mode);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onBegin(mode);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glBegin(mode);

    SU_END_FUNCTION_WRAPPER(ap_glBegin);
}


void APIENTRY glBindTexture(GLenum target, GLuint texture)
{
    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_FUNCTION_WRAPPER(ap_glBindTexture);

    bool areStubTexForced = false;
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (!inNestedFunction)
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glBindTexture, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_UINT_PARAMETER, texture, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &texture);

        // Check if stub textures are forced:
        if (pCurrentThreadRenderContextMonitor)
        {
            areStubTexForced = pCurrentThreadRenderContextMonitor->forcedModesManager().isStubForced(AP_OPENGL_FORCED_STUB_TEXTURES_MODE);
        }
    }

    // If we are not in "Force stub textures" mode:
    if (!areStubTexForced)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glBindTexture(target, texture);
    }
    else
    {
        // Force the stub texture to be the bind texture:
        GLuint forcedTxName = gs_stat_openGLMonitorInstance.forcedStubTextureName(target);
        gs_stat_realFunctionPointers.glBindTexture(target, forcedTxName);
    }

    if (!inNestedFunction)
    {
        // Mark that a texture was bound:
        if (pCurrentThreadRenderContextMonitor)
        {
            pCurrentThreadRenderContextMonitor->onTextureTargetBind(target, texture);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_glBindTexture);
}


void APIENTRY glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte* bitmap)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glBitmap);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glBitmap, 7, OS_TOBJ_ID_GL_SIZEI_PARAMETER, width, OS_TOBJ_ID_GL_SIZEI_PARAMETER, height, OS_TOBJ_ID_GL_FLOAT_PARAMETER, xorig, OS_TOBJ_ID_GL_FLOAT_PARAMETER, yorig, OS_TOBJ_ID_GL_FLOAT_PARAMETER, xmove, OS_TOBJ_ID_GL_FLOAT_PARAMETER, ymove, OS_TOBJ_ID_GL_P_UBYTE_PARAMETER, bitmap);

    // Call the real function:
    gs_stat_realFunctionPointers.glBitmap(width, height, xorig, yorig, xmove, ymove, bitmap);

    SU_END_FUNCTION_WRAPPER(ap_glBitmap);
}


void APIENTRY glBlendFunc(GLenum sfactor, GLenum dfactor)
{
    SU_START_FUNCTION_WRAPPER(ap_glBlendFunc);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glBlendFunc, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, sfactor, OS_TOBJ_ID_GL_ENUM_PARAMETER, dfactor);

    // Call the real function:
    gs_stat_realFunctionPointers.glBlendFunc(sfactor, dfactor);

    SU_END_FUNCTION_WRAPPER(ap_glBlendFunc);
}


void APIENTRY glCallList(GLuint list)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glCallList);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glCallList, 1, OS_TOBJ_ID_GL_UINT_PARAMETER, list);

    // Log the display list end:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Call display lists from render primitives monitor:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticesLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();
        renderPrimitivesStatisticesLogger.onCallDisplayList(list);

        gsDisplayListMonitor* pDisplayListMonitor = pCurrentThreadRenderContextMonitor->displayListsMonitor();
        GT_IF_WITH_ASSERT(pDisplayListMonitor != NULL)
        {
            pDisplayListMonitor->callDisplayList(list);
        }
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glCallList(list);

    SU_END_FUNCTION_WRAPPER(ap_glCallList);
}


void APIENTRY glCallLists(GLsizei n, GLenum type, const GLvoid* lists)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glCallLists);

    // Log the call to this function:

    // If there are up to gs_stat_maxLoggedArraySize lists - we will log the lists names:
    if (n <= GLsizei(gs_stat_maxLoggedArraySize))
    {
        switch (type)
        {
            case GL_BYTE:
                gs_stat_openGLMonitorInstance.addFunctionCall(ap_glCallLists, 3, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_BYTE_PARAMETER, n, lists);
                break;

            case GL_UNSIGNED_BYTE:
                gs_stat_openGLMonitorInstance.addFunctionCall(ap_glCallLists, 3, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_UBYTE_PARAMETER, n, lists);
                break;

            case GL_SHORT:
                gs_stat_openGLMonitorInstance.addFunctionCall(ap_glCallLists, 3, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_SHORT_PARAMETER, n, lists);
                break;

            case GL_UNSIGNED_SHORT:
                gs_stat_openGLMonitorInstance.addFunctionCall(ap_glCallLists, 3, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_USHORT_PARAMETER, n, lists);
                break;

            case GL_INT:
                gs_stat_openGLMonitorInstance.addFunctionCall(ap_glCallLists, 3, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, n, lists);
                break;

            case GL_UNSIGNED_INT:
                gs_stat_openGLMonitorInstance.addFunctionCall(ap_glCallLists, 3, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_UINT_PARAMETER, n, lists);
                break;

            default:
                // Unknown type - display the lists argument value:
                gs_stat_openGLMonitorInstance.addFunctionCall(ap_glCallLists, 3, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_GL_P_VOID_PARAMETER, lists);
                break;
        }
    }
    else
    {
        // There are more than gs_stat_maxLoggedArraySize list names - log only the lists names array pointer value:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glCallLists, 3, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_GL_P_VOID_PARAMETER, lists);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glCallLists(n, type, lists);

    // Log the display list end:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Call display lists from render primitives monitor:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticesLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();
        renderPrimitivesStatisticesLogger.onCallDisplayLists(n, type, lists);

        gsDisplayListMonitor* pDisplayListMonitor = pCurrentThreadRenderContextMonitor->displayListsMonitor();
        GT_IF_WITH_ASSERT(pDisplayListMonitor != NULL)
        {
            pDisplayListMonitor->callDisplayLists(n, type, lists);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_glCallLists);
}


void APIENTRY glClear(GLbitfield mask)
{
    SU_START_FUNCTION_WRAPPER(ap_glClear);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glClear, 1, OS_TOBJ_ID_GL_CLEAR_BITFIELD_PARAMETER, mask);

    // If we are defines as a frame terminator:
    unsigned int frameTerminatorsMask = suFrameTerminatorsMask();

    if (frameTerminatorsMask & AP_GL_CLEAR_TERMINATOR)
    {
        // Terminate the current frame:
        gs_stat_openGLMonitorInstance.onFrameTerminatorCall();
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glClear(mask);

    SU_END_FUNCTION_WRAPPER(ap_glClear);
}


void APIENTRY glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    SU_START_FUNCTION_WRAPPER(ap_glClearAccum);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glClearAccum, 4, OS_TOBJ_ID_GL_FLOAT_PARAMETER, red, OS_TOBJ_ID_GL_FLOAT_PARAMETER, green, OS_TOBJ_ID_GL_FLOAT_PARAMETER, blue, OS_TOBJ_ID_GL_FLOAT_PARAMETER, alpha);

    // Call the real function:
    gs_stat_realFunctionPointers.glClearAccum(red, green, blue, alpha);

    SU_END_FUNCTION_WRAPPER(ap_glClearAccum);
}


void APIENTRY glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    SU_START_FUNCTION_WRAPPER(ap_glClearColor);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glClearColor, 4, OS_TOBJ_ID_GL_CLAMPF_PARAMETER, red, OS_TOBJ_ID_GL_CLAMPF_PARAMETER, green, OS_TOBJ_ID_GL_CLAMPF_PARAMETER, blue, OS_TOBJ_ID_GL_CLAMPF_PARAMETER, alpha);

    // Call the real function:
    gs_stat_realFunctionPointers.glClearColor(red, green, blue, alpha);

    SU_END_FUNCTION_WRAPPER(ap_glClearColor);
}


void APIENTRY glClearDepth(GLclampd depth)
{
    SU_START_FUNCTION_WRAPPER(ap_glClearDepth);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glClearDepth, 1, OS_TOBJ_ID_GL_CLAMPD_PARAMETER, depth);

    // Call the real function:
    gs_stat_realFunctionPointers.glClearDepth(depth);

    SU_END_FUNCTION_WRAPPER(ap_glClearDepth);
}


void APIENTRY glClearIndex(GLfloat c)
{
    SU_START_FUNCTION_WRAPPER(ap_glClearIndex);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glClearIndex, 1, OS_TOBJ_ID_GL_FLOAT_PARAMETER, c);

    // Call the real function:
    gs_stat_realFunctionPointers.glClearIndex(c);

    SU_END_FUNCTION_WRAPPER(ap_glClearIndex);
}


void APIENTRY glClearStencil(GLint s)
{
    SU_START_FUNCTION_WRAPPER(ap_glClearStencil);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glClearStencil, 1, OS_TOBJ_ID_GL_INT_PARAMETER, s);

    // Call the real function:
    gs_stat_realFunctionPointers.glClearStencil(s);

    SU_END_FUNCTION_WRAPPER(ap_glClearStencil);
}


void APIENTRY glClipPlane(GLenum plane, const GLdouble* equation)
{
    SU_START_FUNCTION_WRAPPER(ap_glClipPlane);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glClipPlane, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, plane, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 4, equation);

    // Call the real function:
    gs_stat_realFunctionPointers.glClipPlane(plane, equation);

    SU_END_FUNCTION_WRAPPER(ap_glClipPlane);
}


void APIENTRY glColor3b(GLbyte red, GLbyte green, GLbyte blue)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor3b);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor3b, 3, OS_TOBJ_ID_GL_BYTE_PARAMETER, red, OS_TOBJ_ID_GL_BYTE_PARAMETER, green, OS_TOBJ_ID_GL_BYTE_PARAMETER, blue);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor3b(red, green, blue);

    SU_END_FUNCTION_WRAPPER(ap_glColor3b);
}


void APIENTRY glColor3bv(const GLbyte* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor3bv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor3bv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_BYTE_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor3bv(v);

    SU_END_FUNCTION_WRAPPER(ap_glColor3bv);
}


void APIENTRY glColor3d(GLdouble red, GLdouble green, GLdouble blue)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor3d);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor3d, 3, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, red, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, green, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, blue);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor3d(red, green, blue);

    SU_END_FUNCTION_WRAPPER(ap_glColor3d);
}


void APIENTRY glColor3dv(const GLdouble* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor3dv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor3dv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor3dv(v);

    SU_END_FUNCTION_WRAPPER(ap_glColor3dv);
}


void APIENTRY glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor3f);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor3f, 3, OS_TOBJ_ID_GL_FLOAT_PARAMETER, red, OS_TOBJ_ID_GL_FLOAT_PARAMETER, green, OS_TOBJ_ID_GL_FLOAT_PARAMETER, blue);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor3f(red, green, blue);

    SU_END_FUNCTION_WRAPPER(ap_glColor3f);
}


void APIENTRY glColor3fv(const GLfloat* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor3fv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor3fv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor3fv(v);

    SU_END_FUNCTION_WRAPPER(ap_glColor3fv);
}


void APIENTRY glColor3i(GLint red, GLint green, GLint blue)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor3i);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor3i, 3, OS_TOBJ_ID_GL_INT_PARAMETER, red, OS_TOBJ_ID_GL_INT_PARAMETER, green, OS_TOBJ_ID_GL_INT_PARAMETER, blue);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor3i(red, green, blue);

    SU_END_FUNCTION_WRAPPER(ap_glColor3i);
}


void APIENTRY glColor3iv(const GLint* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor3iv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor3iv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor3iv(v);

    SU_END_FUNCTION_WRAPPER(ap_glColor3iv);
}


void APIENTRY glColor3s(GLshort red, GLshort green, GLshort blue)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor3s);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor3s, 3, OS_TOBJ_ID_GL_SHORT_PARAMETER, red, OS_TOBJ_ID_GL_SHORT_PARAMETER, green, OS_TOBJ_ID_GL_SHORT_PARAMETER, blue);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor3s(red, green, blue);

    SU_END_FUNCTION_WRAPPER(ap_glColor3s);
}


void APIENTRY glColor3sv(const GLshort* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor3sv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor3sv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_SHORT_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor3sv(v);

    SU_END_FUNCTION_WRAPPER(ap_glColor3sv);
}


void APIENTRY glColor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor3ub);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor3ub, 3, OS_TOBJ_ID_GL_UBYTE_PARAMETER, red, OS_TOBJ_ID_GL_UBYTE_PARAMETER, green, OS_TOBJ_ID_GL_UBYTE_PARAMETER, blue);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor3ub(red, green, blue);

    SU_END_FUNCTION_WRAPPER(ap_glColor3ub);
}


void APIENTRY glColor3ubv(const GLubyte* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor3ubv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor3ubv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_UBYTE_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor3ubv(v);

    SU_END_FUNCTION_WRAPPER(ap_glColor3ubv);
}


void APIENTRY glColor3ui(GLuint red, GLuint green, GLuint blue)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor3ui);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor3ui, 3, OS_TOBJ_ID_GL_UINT_PARAMETER, red, OS_TOBJ_ID_GL_UINT_PARAMETER, green, OS_TOBJ_ID_GL_UINT_PARAMETER, blue);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor3ui(red, green, blue);

    SU_END_FUNCTION_WRAPPER(ap_glColor3ui);
}


void APIENTRY glColor3uiv(const GLuint* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor3uiv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor3uiv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_UINT_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor3uiv(v);

    SU_END_FUNCTION_WRAPPER(ap_glColor3uiv);
}


void APIENTRY glColor3us(GLushort red, GLushort green, GLushort blue)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor3us);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor3us, 3, OS_TOBJ_ID_GL_USHORT_PARAMETER, red, OS_TOBJ_ID_GL_USHORT_PARAMETER, green, OS_TOBJ_ID_GL_USHORT_PARAMETER, blue);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor3us(red, green, blue);

    SU_END_FUNCTION_WRAPPER(ap_glColor3us);
}


void APIENTRY glColor3usv(const GLushort* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor3usv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor3usv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_USHORT_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor3usv(v);

    SU_END_FUNCTION_WRAPPER(ap_glColor3usv);
}


void APIENTRY glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor4b);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor4b, 4, OS_TOBJ_ID_GL_BYTE_PARAMETER, red, OS_TOBJ_ID_GL_BYTE_PARAMETER, green, OS_TOBJ_ID_GL_BYTE_PARAMETER, blue, OS_TOBJ_ID_GL_BYTE_PARAMETER, alpha);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor4b(red, green, blue, alpha);

    SU_END_FUNCTION_WRAPPER(ap_glColor4b);
}


void APIENTRY glColor4bv(const GLbyte* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor4bv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor4bv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_BYTE_PARAMETER, 4, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor4bv(v);

    SU_END_FUNCTION_WRAPPER(ap_glColor4bv);
}


void APIENTRY glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor4d);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor4d, 4, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, red, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, green, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, blue, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, alpha);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor4d(red, green, blue, alpha);

    SU_END_FUNCTION_WRAPPER(ap_glColor4d);
}


void APIENTRY glColor4dv(const GLdouble* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor4dv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor4dv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 4, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor4dv(v);

    SU_END_FUNCTION_WRAPPER(ap_glColor4dv);
}


void APIENTRY glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor4f);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor4f, 4, OS_TOBJ_ID_GL_FLOAT_PARAMETER, red, OS_TOBJ_ID_GL_FLOAT_PARAMETER, green, OS_TOBJ_ID_GL_FLOAT_PARAMETER, blue, OS_TOBJ_ID_GL_FLOAT_PARAMETER, alpha);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor4f(red, green, blue, alpha);

    SU_END_FUNCTION_WRAPPER(ap_glColor4f);
}


void APIENTRY glColor4fv(const GLfloat* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor4fv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor4fv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 4, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor4fv(v);

    SU_END_FUNCTION_WRAPPER(ap_glColor4fv);
}


void APIENTRY glColor4i(GLint red, GLint green, GLint blue, GLint alpha)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor4i);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor4i, 4, OS_TOBJ_ID_GL_INT_PARAMETER, red, OS_TOBJ_ID_GL_INT_PARAMETER, green, OS_TOBJ_ID_GL_INT_PARAMETER, blue, OS_TOBJ_ID_GL_INT_PARAMETER, alpha);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor4i(red, green, blue, alpha);

    SU_END_FUNCTION_WRAPPER(ap_glColor4i);
}


void APIENTRY glColor4iv(const GLint* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor4iv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor4iv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 4, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor4iv(v);

    SU_END_FUNCTION_WRAPPER(ap_glColor4iv);
}


void APIENTRY glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor4s);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor4s, 4, OS_TOBJ_ID_GL_SHORT_PARAMETER, red, OS_TOBJ_ID_GL_SHORT_PARAMETER, green, OS_TOBJ_ID_GL_SHORT_PARAMETER, blue, OS_TOBJ_ID_GL_SHORT_PARAMETER, alpha);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor4s(red, green, blue, alpha);

    SU_END_FUNCTION_WRAPPER(ap_glColor4s);
}


void APIENTRY glColor4sv(const GLshort* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor4sv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor4sv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_SHORT_PARAMETER, 4, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor4sv(v);

    SU_END_FUNCTION_WRAPPER(ap_glColor4sv);
}


void APIENTRY glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor4ub);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor4ub, 4, OS_TOBJ_ID_GL_UBYTE_PARAMETER, red, OS_TOBJ_ID_GL_UBYTE_PARAMETER, green, OS_TOBJ_ID_GL_UBYTE_PARAMETER, blue, OS_TOBJ_ID_GL_UBYTE_PARAMETER, alpha);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor4ub(red, green, blue, alpha);

    SU_END_FUNCTION_WRAPPER(ap_glColor4ub);
}


void APIENTRY glColor4ubv(const GLubyte* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor4ubv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor4ubv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_UBYTE_PARAMETER, 4, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor4ubv(v);

    SU_END_FUNCTION_WRAPPER(ap_glColor4ubv);
}


void APIENTRY glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor4ui);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor4ui, 4, OS_TOBJ_ID_GL_UINT_PARAMETER, red, OS_TOBJ_ID_GL_UINT_PARAMETER, green, OS_TOBJ_ID_GL_UINT_PARAMETER, blue, OS_TOBJ_ID_GL_UINT_PARAMETER, alpha);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor4ui(red, green, blue, alpha);

    SU_END_FUNCTION_WRAPPER(ap_glColor4ui);
}


void APIENTRY glColor4uiv(const GLuint* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor4uiv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor4uiv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_UINT_PARAMETER, 4, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor4uiv(v);

    SU_END_FUNCTION_WRAPPER(ap_glColor4uiv);
}


void APIENTRY glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor4us);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor4us, 4, OS_TOBJ_ID_GL_USHORT_PARAMETER, red, OS_TOBJ_ID_GL_USHORT_PARAMETER, green, OS_TOBJ_ID_GL_USHORT_PARAMETER, blue, OS_TOBJ_ID_GL_USHORT_PARAMETER, alpha);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor4us(red, green, blue, alpha);

    SU_END_FUNCTION_WRAPPER(ap_glColor4us);
}


void APIENTRY glColor4usv(const GLushort* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor4usv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor4usv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_USHORT_PARAMETER, 4, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glColor4usv(v);

    SU_END_FUNCTION_WRAPPER(ap_glColor4usv);
}


void APIENTRY glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    SU_START_FUNCTION_WRAPPER(ap_glColorMask);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColorMask, 4, OS_TOBJ_ID_GL_BOOL_PARAMETER, red, OS_TOBJ_ID_GL_BOOL_PARAMETER, green, OS_TOBJ_ID_GL_BOOL_PARAMETER, blue, OS_TOBJ_ID_GL_BOOL_PARAMETER, alpha);

    // Call the real function:
    gs_stat_realFunctionPointers.glColorMask(red, green, blue, alpha);

    SU_END_FUNCTION_WRAPPER(ap_glColorMask);
}


void APIENTRY glColorMaterial(GLenum face, GLenum mode)
{
    SU_START_FUNCTION_WRAPPER(ap_glColorMaterial);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColorMaterial, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, face, OS_TOBJ_ID_GL_ENUM_PARAMETER, mode);

    // Call the real function:
    gs_stat_realFunctionPointers.glColorMaterial(face, mode);

    SU_END_FUNCTION_WRAPPER(ap_glColorMaterial);
}


void APIENTRY glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    SU_START_FUNCTION_WRAPPER(ap_glColorPointer);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColorPointer, 4, OS_TOBJ_ID_GL_INT_PARAMETER, size, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_GL_SIZEI_PARAMETER, stride, OS_TOBJ_ID_GL_P_VOID_PARAMETER, pointer);

    // If we are building the an OpenGL ES DLL:
#ifdef OS_OGL_ES_IMPLEMENTATION_DLL_BUILD

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        // Update the current vertex array data:
        gsVertexArrayData& curVertexArrayData = pCurrentThreadRenderContextMonitor->currentVertexArrayData();
        gsArrayPointer& colorsArrayData = curVertexArrayData._colorsArray;
        colorsArrayData._numOfCoordinates = size;
        colorsArrayData._dataType = type;
        colorsArrayData._stride = stride;
        colorsArrayData._pArrayRawData = pointer;
    }

    // If the data type is supported by OpenGL:
    if (type != GL_FIXED)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glColorPointer(size, type, stride, pointer);
    }

#else

    // None ES DLL - Just call the real function:
    gs_stat_realFunctionPointers.glColorPointer(size, type, stride, pointer);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glColorPointer);
}


void APIENTRY glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glCopyPixels);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glCopyPixels, 5, OS_TOBJ_ID_GL_INT_PARAMETER, x, OS_TOBJ_ID_GL_INT_PARAMETER, y, OS_TOBJ_ID_GL_SIZEI_PARAMETER, width, OS_TOBJ_ID_GL_SIZEI_PARAMETER, height, OS_TOBJ_ID_GL_ENUM_PARAMETER, type);

    // Call the real function:
    gs_stat_realFunctionPointers.glCopyPixels(x, y, width, height, type);

    SU_END_FUNCTION_WRAPPER(ap_glCopyPixels);
}


void APIENTRY glCopyTexImage1D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLint border)
{
    SU_START_FUNCTION_WRAPPER(ap_glCopyTexImage1D);

    // Check if stub textures are forced:
    bool areStubTexForced = false;
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        areStubTexForced = pCurrentThreadRenderContextMonitor->forcedModesManager().isStubForced(AP_OPENGL_FORCED_STUB_TEXTURES_MODE);
    }

    // If we are not in "Force stub textures" mode:
    if (!areStubTexForced)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glCopyTexImage1D(target, level, internalFormat, x, y, width, border);

        // Log the loaded texture:
        if (pCurrentThreadRenderContextMonitor)
        {
            bool rcTex = pCurrentThreadRenderContextMonitor->onTextureImageLoaded(target, level, internalFormat, width, 0, 0, border, GL_GREMEDY_COPIED_FROM_BUFFER, GL_GREMEDY_COPIED_FROM_BUFFER);
            GT_ASSERT(rcTex);
        }
    }

    // Log the call to this function:
    GLuint boundTextureName = gs_stat_openGLMonitorInstance.boundTexture(target);
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glCopyTexImage1D, 8, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_INT_PARAMETER, level, OS_TOBJ_ID_GL_ENUM_PARAMETER, internalFormat, OS_TOBJ_ID_GL_INT_PARAMETER, x, OS_TOBJ_ID_GL_INT_PARAMETER, y, OS_TOBJ_ID_GL_SIZEI_PARAMETER, width, OS_TOBJ_ID_GL_INT_PARAMETER, border, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);

    SU_END_FUNCTION_WRAPPER(ap_glCopyTexImage1D);
}


void APIENTRY glCopyTexImage2D(GLenum target, GLint level, GLenum internalFormat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    SU_START_FUNCTION_WRAPPER(ap_glCopyTexImage2D);

    // Check if stub textures are forced:
    bool areStubTexForced = false;
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        areStubTexForced = pCurrentThreadRenderContextMonitor->forcedModesManager().isStubForced(AP_OPENGL_FORCED_STUB_TEXTURES_MODE);
    }

    // If we are not in "Force stub textures" mode:
    if (!areStubTexForced)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glCopyTexImage2D(target, level, internalFormat, x, y, width, height, border);

        // Log the loaded texture:
        if (pCurrentThreadRenderContextMonitor)
        {
            bool rcTex = pCurrentThreadRenderContextMonitor->onTextureImageLoaded(target, level, internalFormat, width, height, 0, border, GL_GREMEDY_COPIED_FROM_BUFFER, GL_GREMEDY_COPIED_FROM_BUFFER);
            GT_ASSERT(rcTex);
        }
    }

    // Log the call to this function:
    GLuint boundTextureName = gs_stat_openGLMonitorInstance.boundTexture(target);
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glCopyTexImage2D, 9, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_INT_PARAMETER, level, OS_TOBJ_ID_GL_ENUM_PARAMETER, internalFormat, OS_TOBJ_ID_GL_INT_PARAMETER, x, OS_TOBJ_ID_GL_INT_PARAMETER, y, OS_TOBJ_ID_GL_SIZEI_PARAMETER, width, OS_TOBJ_ID_GL_SIZEI_PARAMETER, height, OS_TOBJ_ID_GL_INT_PARAMETER, border, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);

    SU_END_FUNCTION_WRAPPER(ap_glCopyTexImage2D);
}


void APIENTRY glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)
{
    SU_START_FUNCTION_WRAPPER(ap_glCopyTexSubImage1D);

    // Check if stub textures are forced:
    bool areStubTexForced = false;
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        areStubTexForced = pCurrentThreadRenderContextMonitor->forcedModesManager().isStubForced(AP_OPENGL_FORCED_STUB_TEXTURES_MODE);
    }

    // If we are not in "Force stub textures" mode:
    if (!areStubTexForced)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glCopyTexSubImage1D(target, level, xoffset, x, y, width);

        // Log the loaded sub-texture:
        if (pCurrentThreadRenderContextMonitor)
        {
            bool rcSubTex = pCurrentThreadRenderContextMonitor->onTextureSubImageLoaded(target, level);
            GT_ASSERT(rcSubTex);
        }
    }

    // Log the call to this function:
    GLuint boundTextureName = gs_stat_openGLMonitorInstance.boundTexture(target);
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glCopyTexSubImage1D, 7, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_INT_PARAMETER, level, OS_TOBJ_ID_GL_INT_PARAMETER, xoffset, OS_TOBJ_ID_GL_INT_PARAMETER, x, OS_TOBJ_ID_GL_INT_PARAMETER, y, OS_TOBJ_ID_GL_SIZEI_PARAMETER, width, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);

    SU_END_FUNCTION_WRAPPER(ap_glCopyTexSubImage1D);
}


void APIENTRY glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    SU_START_FUNCTION_WRAPPER(ap_glCopyTexSubImage2D);

    // Check if stub textures are forced:
    bool areStubTexForced = false;
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        areStubTexForced = pCurrentThreadRenderContextMonitor->forcedModesManager().isStubForced(AP_OPENGL_FORCED_STUB_TEXTURES_MODE);
    }

    // If we are not in "Force stub textures" mode:
    if (!areStubTexForced)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glCopyTexSubImage2D(target, level, xoffset, yoffset, x, y, width, height);

        // Log the loaded sub-texture:
        if (pCurrentThreadRenderContextMonitor)
        {
            bool rcSubTex = pCurrentThreadRenderContextMonitor->onTextureSubImageLoaded(target, level);
            GT_ASSERT(rcSubTex);
        }
    }

    // Log the call to this function:
    GLuint boundTextureName = gs_stat_openGLMonitorInstance.boundTexture(target);
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glCopyTexSubImage2D, 9, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_INT_PARAMETER, level, OS_TOBJ_ID_GL_INT_PARAMETER, xoffset, OS_TOBJ_ID_GL_INT_PARAMETER, yoffset, OS_TOBJ_ID_GL_INT_PARAMETER, x, OS_TOBJ_ID_GL_INT_PARAMETER, y, OS_TOBJ_ID_GL_SIZEI_PARAMETER, width, OS_TOBJ_ID_GL_SIZEI_PARAMETER, height, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);

    SU_END_FUNCTION_WRAPPER(ap_glCopyTexSubImage2D);
}


void APIENTRY glCullFace(GLenum mode)
{
    SU_START_FUNCTION_WRAPPER(ap_glCullFace);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glCullFace, 1, OS_TOBJ_ID_GL_ENUM_PARAMETER, mode);

    // Call the real function:
    gs_stat_realFunctionPointers.glCullFace(mode);

    SU_END_FUNCTION_WRAPPER(ap_glCullFace);
}


void APIENTRY glDeleteLists(GLuint list, GLsizei range)
{
    SU_START_FUNCTION_WRAPPER(ap_glDeleteLists);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDeleteLists, 2, OS_TOBJ_ID_GL_UINT_PARAMETER, list, OS_TOBJ_ID_GL_SIZEI_PARAMETER, range);

    // Call the real function:
    gs_stat_realFunctionPointers.glDeleteLists(list, range);

    // Log the display lists' deletion:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        gsDisplayListMonitor* displayListMonitor = pCurrentThreadRenderContextMonitor->displayListsMonitor();
        GT_IF_WITH_ASSERT(displayListMonitor != NULL)
        {
            displayListMonitor->removeDisplayLists(list, range);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_glDeleteLists);
}


void APIENTRY glDeleteTextures(GLsizei n, const GLuint* textures)
{
    SU_START_FUNCTION_WRAPPER(ap_glDeleteTextures);

    // Log the call to this function:

    // If there are gs_stat_maxLoggedArraySize or less textures - we will log the textures array:
    if (n <= GLsizei(gs_stat_maxLoggedArraySize))
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDeleteTextures, 3, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_UINT_PARAMETER, n, textures, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, n, textures);
    }
    else
    {
        // There are more than gs_stat_maxLoggedArraySize textures - we will log the array pointer:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDeleteTextures, 3, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_GL_P_UINT_PARAMETER, textures, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, gs_stat_maxLoggedArraySize, textures);
    }

    // Log the textures deletion:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        gsTexturesMonitor* texturesMon = pCurrentThreadRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(texturesMon != NULL)
        {
            texturesMon->onTextureObjectsDeletion(n, textures);
        }
    }

    // When this function is called when there is not current context, it sometimes crushes:
    if (gs_stat_openGLMonitorInstance.currentThreadRenderContextSpyId() > 0)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glDeleteTextures(n, textures);
    }

    SU_END_FUNCTION_WRAPPER(ap_glDeleteTextures);
}


void APIENTRY glDepthFunc(GLenum func)
{
    SU_START_FUNCTION_WRAPPER(ap_glDepthFunc);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDepthFunc, 1, OS_TOBJ_ID_GL_ENUM_PARAMETER, func);

    // Call the real function:
    gs_stat_realFunctionPointers.glDepthFunc(func);

    SU_END_FUNCTION_WRAPPER(ap_glDepthFunc);
}


void APIENTRY glDepthMask(GLboolean flag)
{
    SU_START_FUNCTION_WRAPPER(ap_glDepthMask);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDepthMask, 1, OS_TOBJ_ID_GL_BOOL_PARAMETER, flag);

    // Call the real function:
    gs_stat_realFunctionPointers.glDepthMask(flag);

    SU_END_FUNCTION_WRAPPER(ap_glDepthMask);
}


void APIENTRY glDepthRange(GLclampd zNear, GLclampd zFar)
{
    SU_START_FUNCTION_WRAPPER(ap_glDepthRange);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDepthRange, 2, OS_TOBJ_ID_GL_CLAMPD_PARAMETER, zNear, OS_TOBJ_ID_GL_CLAMPD_PARAMETER, zFar);

    // Call the real function:
    gs_stat_realFunctionPointers.glDepthRange(zNear, zFar);

    SU_END_FUNCTION_WRAPPER(ap_glDepthRange);
}


void APIENTRY glDisable(GLenum cap)
{
    SU_START_FUNCTION_WRAPPER(ap_glDisable);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDisable, 1, OS_TOBJ_ID_GL_ENUM_PARAMETER, cap);

    bool shouldDisable = true;

    // If a light was turned off:
    if ((GL_LIGHT0 <= cap) && (cap <= GL_LIGHT7))
    {
        gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

        if (pCurrentThreadRenderContextMonitor)
        {
            // Mark the turned on light:
            pCurrentThreadRenderContextMonitor->lightsMonitor().onLightTurnedOff(cap);

            // If we are in "force no lights" mode:
            if (pCurrentThreadRenderContextMonitor->forcedModesManager().isStubForced(AP_OPENGL_FORCED_NO_LIGHTS_MODE))
            {
                shouldDisable = false;
            }
        }
    }

    if (shouldDisable)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glDisable(cap);
    }

    SU_END_FUNCTION_WRAPPER(ap_glDisable);
}


void APIENTRY glDisableClientState(GLenum array)
{
    SU_START_FUNCTION_WRAPPER(ap_glDisableClientState);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDisableClientState, 1, OS_TOBJ_ID_GL_ENUM_PARAMETER, array);

    if ((array == GL_PRIMITIVE_RESTART) || (array == GL_PRIMITIVE_RESTART_NV))
    {
        // Get the current render context monitor:
        gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

        if (pCurrentThreadRenderContextMonitor != NULL)
        {
            // Get the render primitives statistics logger:
            gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

            // Log this draw primitives function call:
            renderPrimitivesStatisticsLogger.enablePrimitiveRestart(false);
        }
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glDisableClientState(array);

    // If we are building the an OpenGL ES DLL:
#ifdef OS_OGL_ES_IMPLEMENTATION_DLL_BUILD

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        // Update the current vertex array data:
        gsVertexArrayData& curVertexArrayData = pCurrentThreadRenderContextMonitor->currentVertexArrayData();

        switch (array)
        {
            case GL_VERTEX_ARRAY:
                curVertexArrayData._isVerticesArrayEnabled = false;
                break;

            case GL_NORMAL_ARRAY:
                curVertexArrayData._isNormalsArrayEnabled = false;
                break;

            case GL_TEXTURE_COORD_ARRAY:
                curVertexArrayData._isTextureCoorinatesArrayEnabled = false;
                break;

            case GL_COLOR_ARRAY:
                curVertexArrayData._isColorsArrayEnabled = false;
                break;
        }
    }

#endif // OpenGL ES DLL build


    SU_END_FUNCTION_WRAPPER(ap_glDisableClientState);
}


void APIENTRY glDrawArrays(GLenum mode, GLint first, GLsizei count)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glDrawArrays);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDrawArrays, 3, OS_TOBJ_ID_GL_PRIMITIVE_TYPE_PARAMETER, mode, OS_TOBJ_ID_GL_INT_PARAMETER, first, OS_TOBJ_ID_GL_SIZEI_PARAMETER, count);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onDrawArrays(first, count, mode);
    }

    // If we are building the an OpenGL ES DLL:
#ifdef OS_OGL_ES_IMPLEMENTATION_DLL_BUILD

    if (pCurrentThreadRenderContextMonitor)
    {
        // Get the current vertex array data:
        const gsVertexArrayData& curVertexArrayData = pCurrentThreadRenderContextMonitor->currentVertexArrayData();

        // Draw the vertex array, performing required data migrations:
        gsVertexArrayDrawer& vertexArrayDrawer = pCurrentThreadRenderContextMonitor->vertexArrayDrawer();
        vertexArrayDrawer.drawArrays(mode, first, count);
    }

#elif defined _GR_IPHONE_BUILD
    // In the iPhone simulator, we create the effect of glRasterMode(GL_POINTS) by changing the parameter here:
    bool shouldForcePoints = false;

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        const gsForcedModesManager& forcedModesManager = pCurrentThreadRenderContextMonitor->forcedModesManager();
        shouldForcePoints = (forcedModesManager.isStubForced(AP_OPENGL_FORCED_POLYGON_RASTER_MODE) && (forcedModesManager.forcedPolygonRasterMode() == AP_RASTER_POINTS));
    }

    if (shouldForcePoints)
    {
        // Use GL_POINTS
        gs_stat_realFunctionPointers.glDrawArrays(GL_POINTS, first, count);
    }
    else
    {
        // Use the user's choice
        gs_stat_realFunctionPointers.glDrawArrays(mode, first, count);
    }

#else
    // Non-OpenGL ES DLL - Just call the real function:
    gs_stat_realFunctionPointers.glDrawArrays(mode, first, count);
#endif

    SU_END_FUNCTION_WRAPPER(ap_glDrawArrays);
}


void APIENTRY glDrawBuffer(GLenum mode)
{
    SU_START_FUNCTION_WRAPPER(ap_glDrawBuffer);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDrawBuffer, 1, OS_TOBJ_ID_GL_ENUM_PARAMETER, mode);

    bool isFrontDrawBuffForced = false;

    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        // Log the "real" draw buffer:
        gsForcedModesManager& forcedModesMgr = pCurrentThreadRenderContextMonitor->forcedModesManager();
        forcedModesMgr.onSetDrawBuffersCall(mode);

        // Check if front draw buffer is forced:
        isFrontDrawBuffForced = forcedModesMgr.isStubForced(AP_OPENGL_FORCED_FRONT_DRAW_BUFFER_MODE);
    }

    // If the OpenGL front draw buffer is not forced:
    if (!isFrontDrawBuffForced)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glDrawBuffer(mode);
    }

    SU_END_FUNCTION_WRAPPER(ap_glDrawBuffer);
}


void APIENTRY glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glDrawElements);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDrawElements, 4, OS_TOBJ_ID_GL_PRIMITIVE_TYPE_PARAMETER, mode, OS_TOBJ_ID_GL_SIZEI_PARAMETER, count, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_GL_P_VOID_PARAMETER, indices);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onDrawElements(count, mode, type);
    }

    // If we are building the an OpenGL ES DLL:
#ifdef OS_OGL_ES_IMPLEMENTATION_DLL_BUILD

    if (pCurrentThreadRenderContextMonitor)
    {
        // Get the current vertex array data:
        const gsVertexArrayData& curVertexArrayData = pCurrentThreadRenderContextMonitor->currentVertexArrayData();

        // Draw the vertex array, performing required data migrations:
        gsVertexArrayDrawer& vertexArrayDrawer = pCurrentThreadRenderContextMonitor->vertexArrayDrawer();
        vertexArrayDrawer.drawElements(mode, count, type, indices);
    }

#elif defined _GR_IPHONE_BUILD
    // In the iPhone simulator, we create the effect of glRasterMode(GL_POINTS) by changing the parameter here:
    bool shouldForcePoints = false;

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        const gsForcedModesManager& forcedModesManager = pCurrentThreadRenderContextMonitor->forcedModesManager();
        shouldForcePoints = (forcedModesManager.isStubForced(AP_OPENGL_FORCED_POLYGON_RASTER_MODE) && (forcedModesManager.forcedPolygonRasterMode() == AP_RASTER_POINTS));
    }

    if (shouldForcePoints)
    {
        // Use GL_POINTS
        gs_stat_realFunctionPointers.glDrawElements(GL_POINTS, count, type, indices);
    }
    else
    {
        // Use the user's choice
        gs_stat_realFunctionPointers.glDrawElements(mode, count, type, indices);
    }

#else
    // Non-OpenGL ES DLL - Just call the real function:
    gs_stat_realFunctionPointers.glDrawElements(mode, count, type, indices);
#endif

    SU_END_FUNCTION_WRAPPER(ap_glDrawElements);
}


void APIENTRY glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glDrawPixels);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDrawPixels, 5, OS_TOBJ_ID_GL_SIZEI_PARAMETER, width, OS_TOBJ_ID_GL_SIZEI_PARAMETER, height, OS_TOBJ_ID_GL_ENUM_PARAMETER, format, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_GL_P_VOID_PARAMETER, pixels);

    // Call the real function:
    gs_stat_realFunctionPointers.glDrawPixels(width, height, format, type, pixels);

    SU_END_FUNCTION_WRAPPER(ap_glDrawPixels);
}


void APIENTRY glEdgeFlag(GLboolean flag)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glEdgeFlag);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glEdgeFlag, 1, OS_TOBJ_ID_GL_BOOL_PARAMETER, flag);

    // Call the real function:
    gs_stat_realFunctionPointers.glEdgeFlag(flag);

    SU_END_FUNCTION_WRAPPER(ap_glEdgeFlag);
}


void APIENTRY glEdgeFlagPointer(GLsizei stride, const GLvoid* pointer)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glEdgeFlagPointer);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glEdgeFlagPointer, 2, OS_TOBJ_ID_GL_SIZEI_PARAMETER, stride, OS_TOBJ_ID_GL_P_VOID_PARAMETER, pointer);

    // Call the real function:
    gs_stat_realFunctionPointers.glEdgeFlagPointer(stride, pointer);

    SU_END_FUNCTION_WRAPPER(ap_glEdgeFlagPointer);
}


void APIENTRY glEdgeFlagv(const GLboolean* flag)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glEdgeFlagv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glEdgeFlagv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_BOOL_PARAMETER, 1, flag);

    // Call the real function:
    gs_stat_realFunctionPointers.glEdgeFlagv(flag);

    SU_END_FUNCTION_WRAPPER(ap_glEdgeFlagv);
}


void APIENTRY glEnable(GLenum cap)
{
    SU_START_FUNCTION_WRAPPER(ap_glEnable);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glEnable, 1, OS_TOBJ_ID_GL_ENUM_PARAMETER, cap);

    bool shouldEnable = true;

    // If a light was turned on:
    if ((GL_LIGHT0 <= cap) && (cap <= GL_LIGHT7))
    {
        gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

        if (pCurrentThreadRenderContextMonitor)
        {
            // Mark the turned on light:
            pCurrentThreadRenderContextMonitor->lightsMonitor().onLightTurnedOn(cap);

            // If we are in "force no lights" mode:
            if (pCurrentThreadRenderContextMonitor->forcedModesManager().isStubForced(AP_OPENGL_FORCED_NO_LIGHTS_MODE))
            {
                shouldEnable = false;
            }
        }
    }

    if (shouldEnable)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glEnable(cap);
    }

    SU_END_FUNCTION_WRAPPER(ap_glEnable);
}


void APIENTRY glEnableClientState(GLenum array)
{
    SU_START_FUNCTION_WRAPPER(ap_glEnableClientState);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glEnableClientState, 1, OS_TOBJ_ID_GL_ENUM_PARAMETER, array);

    if ((array == GL_PRIMITIVE_RESTART) || (array == GL_PRIMITIVE_RESTART_NV))
    {
        // Get the current render context monitor:
        gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

        if (pCurrentThreadRenderContextMonitor != NULL)
        {
            // Get the render primitives statistics logger:
            gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

            // Log this draw primitives function call:
            renderPrimitivesStatisticsLogger.enablePrimitiveRestart(true);
        }
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glEnableClientState(array);

    // If we are building the an OpenGL ES DLL:
#ifdef OS_OGL_ES_IMPLEMENTATION_DLL_BUILD

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        // Update the current vertex array data:
        gsVertexArrayData& curVertexArrayData = pCurrentThreadRenderContextMonitor->currentVertexArrayData();

        switch (array)
        {
            case GL_VERTEX_ARRAY:
                curVertexArrayData._isVerticesArrayEnabled = true;
                break;

            case GL_NORMAL_ARRAY:
                curVertexArrayData._isNormalsArrayEnabled = true;
                break;

            case GL_TEXTURE_COORD_ARRAY:
                curVertexArrayData._isTextureCoorinatesArrayEnabled = true;
                break;

            case GL_COLOR_ARRAY:
                curVertexArrayData._isColorsArrayEnabled = true;
                break;
        }
    }

#endif // OpenGL ES DLL build


    SU_END_FUNCTION_WRAPPER(ap_glEnableClientState);
}


void APIENTRY glEnd(void)
{
    SU_START_FUNCTION_WRAPPER(ap_glEnd);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glEnd, 0);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onEnd();
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glEnd();

    SU_END_FUNCTION_WRAPPER(ap_glEnd);
}


void APIENTRY glEndList(void)
{
    SU_START_FUNCTION_WRAPPER(ap_glEndList);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glEndList, 0);

    // Log the display list end:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        gsDisplayListMonitor* pDisplayListMonitor = pCurrentThreadRenderContextMonitor->displayListsMonitor();
        GT_IF_WITH_ASSERT(pDisplayListMonitor != NULL)
        {
            // Notify the display lists monitor that the list creation has ended:
            pDisplayListMonitor->endDisplayList();
        }
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glEndList();

    SU_END_FUNCTION_WRAPPER(ap_glEndList);
}


void APIENTRY glEvalCoord1d(GLdouble u)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glEvalCoord1d);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glEvalCoord1d, 1, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, u);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onEvalCoord(sizeof(GLdouble));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glEvalCoord1d(u);

    SU_END_FUNCTION_WRAPPER(ap_glEvalCoord1d);
}


void APIENTRY glEvalCoord1dv(const GLdouble* u)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glEvalCoord1dv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glEvalCoord1dv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 1, u);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onEvalCoord(sizeof(GLdouble));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glEvalCoord1dv(u);

    SU_END_FUNCTION_WRAPPER(ap_glEvalCoord1dv);
}


void APIENTRY glEvalCoord1f(GLfloat u)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glEvalCoord1f);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glEvalCoord1f, 1, OS_TOBJ_ID_GL_FLOAT_PARAMETER, u);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onEvalCoord(sizeof(float));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glEvalCoord1f(u);

    SU_END_FUNCTION_WRAPPER(ap_glEvalCoord1f);
}


void APIENTRY glEvalCoord1fv(const GLfloat* u)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glEvalCoord1fv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glEvalCoord1fv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 1, u);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onEvalCoord(sizeof(float));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glEvalCoord1fv(u);

    SU_END_FUNCTION_WRAPPER(ap_glEvalCoord1fv);
}


void APIENTRY glEvalCoord2d(GLdouble u, GLdouble v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glEvalCoord2d);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glEvalCoord2d, 2, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, u, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, v);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onEvalCoord(sizeof(double));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glEvalCoord2d(u, v);

    SU_END_FUNCTION_WRAPPER(ap_glEvalCoord2d);
}


void APIENTRY glEvalCoord2dv(const GLdouble* u)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glEvalCoord2dv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glEvalCoord2dv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 2, u);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onEvalCoord(sizeof(GLdouble));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glEvalCoord2dv(u);

    SU_END_FUNCTION_WRAPPER(ap_glEvalCoord2dv);
}


void APIENTRY glEvalCoord2f(GLfloat u, GLfloat v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glEvalCoord2f);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glEvalCoord2f, 2, OS_TOBJ_ID_GL_FLOAT_PARAMETER, u, OS_TOBJ_ID_GL_FLOAT_PARAMETER, v);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onEvalCoord(sizeof(GLfloat));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glEvalCoord2f(u, v);

    SU_END_FUNCTION_WRAPPER(ap_glEvalCoord2f);
}


void APIENTRY glEvalCoord2fv(const GLfloat* u)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glEvalCoord2fv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glEvalCoord2fv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 2, u);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onEvalCoord(sizeof(GLfloat));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glEvalCoord2fv(u);

    SU_END_FUNCTION_WRAPPER(ap_glEvalCoord2fv);
}


void APIENTRY glEvalMesh1(GLenum mode, GLint i1, GLint i2)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glEvalMesh1);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glEvalMesh1, 3, OS_TOBJ_ID_GL_PRIMITIVE_TYPE_PARAMETER, mode, OS_TOBJ_ID_GL_INT_PARAMETER, i1, OS_TOBJ_ID_GL_INT_PARAMETER, i2);

    // Call the real function:
    gs_stat_realFunctionPointers.glEvalMesh1(mode, i1, i2);

    SU_END_FUNCTION_WRAPPER(ap_glEvalMesh1);
}


void APIENTRY glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glEvalMesh2);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glEvalMesh2, 5, OS_TOBJ_ID_GL_PRIMITIVE_TYPE_PARAMETER, mode, OS_TOBJ_ID_GL_INT_PARAMETER, i1, OS_TOBJ_ID_GL_INT_PARAMETER, i2, OS_TOBJ_ID_GL_INT_PARAMETER, j1, OS_TOBJ_ID_GL_INT_PARAMETER, j2);

    // Call the real function:
    gs_stat_realFunctionPointers.glEvalMesh2(mode, i1, i2, j1, j2);

    SU_END_FUNCTION_WRAPPER(ap_glEvalMesh2);
}


void APIENTRY glEvalPoint1(GLint i)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glEvalPoint1);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glEvalPoint1, 1, OS_TOBJ_ID_GL_INT_PARAMETER, i);

    // Call the real function:
    gs_stat_realFunctionPointers.glEvalPoint1(i);

    SU_END_FUNCTION_WRAPPER(ap_glEvalPoint1);
}


void APIENTRY glEvalPoint2(GLint i, GLint j)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glEvalPoint2);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glEvalPoint2, 2, OS_TOBJ_ID_GL_INT_PARAMETER, i, OS_TOBJ_ID_GL_INT_PARAMETER, j);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onEvalPoint(sizeof(GLint));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glEvalPoint2(i, j);

    SU_END_FUNCTION_WRAPPER(ap_glEvalPoint2);
}


void APIENTRY glFeedbackBuffer(GLsizei size, GLenum type, GLfloat* buffer)
{
    SU_START_FUNCTION_WRAPPER(ap_glFeedbackBuffer);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glFeedbackBuffer, 3, OS_TOBJ_ID_GL_SIZEI_PARAMETER, size, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_GL_P_FLOAT_PARAMETER, buffer);

    // Call the real function:
    gs_stat_realFunctionPointers.glFeedbackBuffer(size, type, buffer);

    SU_END_FUNCTION_WRAPPER(ap_glFeedbackBuffer);
}


void APIENTRY glFinish(void)
{
    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_FUNCTION_WRAPPER(ap_glFinish);

    if (!inNestedFunction)
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glFinish, 0);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glFinish();

    if (!inNestedFunction)
    {
        // If we are defines as a frame terminator:
        unsigned int frameTerminatorsMask = suFrameTerminatorsMask();

        if (frameTerminatorsMask & AP_GL_FINISH_TERMINATOR)
        {
            // Terminate the current frame:
            gs_stat_openGLMonitorInstance.onFrameTerminatorCall();
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_glFinish);
}


void APIENTRY glFlush(void)
{
    SU_START_FUNCTION_WRAPPER(ap_glFlush);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glFlush, 0);

    // Call the real function:
    gs_stat_realFunctionPointers.glFlush();

    // If we are defines as a frame terminator:
    unsigned int frameTerminatorsMask = suFrameTerminatorsMask();

    if (frameTerminatorsMask & AP_GL_FLUSH_TERMINATOR)
    {
        // Terminate the current frame:
        gs_stat_openGLMonitorInstance.onFrameTerminatorCall();
    }

    SU_END_FUNCTION_WRAPPER(ap_glFlush);
}


void APIENTRY glFogf(GLenum pname, GLfloat param)
{
    SU_START_FUNCTION_WRAPPER(ap_glFogf);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glFogf, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_FLOAT_PARAMETER, param);

    // Call the real function:
    gs_stat_realFunctionPointers.glFogf(pname, param);

    SU_END_FUNCTION_WRAPPER(ap_glFogf);
}


void APIENTRY glFogfv(GLenum pname, const GLfloat* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glFogfv);

    // Log the call to this function:

    int amountOfArrayElements = 1;

    if (pname == GL_FOG_COLOR)
    {
        amountOfArrayElements = 4;
    }

    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glFogfv, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, amountOfArrayElements, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glFogfv(pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glFogfv);
}


void APIENTRY glFogi(GLenum pname, GLint param)
{
    SU_START_FUNCTION_WRAPPER(ap_glFogi);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glFogi, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_INT_PARAMETER, param);

    // Call the real function:
    gs_stat_realFunctionPointers.glFogi(pname, param);

    SU_END_FUNCTION_WRAPPER(ap_glFogi);
}


void APIENTRY glFogiv(GLenum pname, const GLint* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glFogiv);

    // Log the call to this function:

    int amountOfArrayElements = 1;

    if (pname == GL_FOG_COLOR)
    {
        amountOfArrayElements = 4;
    }

    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glFogiv, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, amountOfArrayElements, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glFogiv(pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glFogiv);
}


void APIENTRY glFrontFace(GLenum mode)
{
    SU_START_FUNCTION_WRAPPER(ap_glFrontFace);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glFrontFace, 1, OS_TOBJ_ID_GL_ENUM_PARAMETER, mode);

    // Call the real function:
    gs_stat_realFunctionPointers.glFrontFace(mode);

    SU_END_FUNCTION_WRAPPER(ap_glFrontFace);
}


void APIENTRY glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    SU_START_FUNCTION_WRAPPER(ap_glFrustum);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glFrustum, 6, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, left, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, right, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, bottom, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, top, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, zNear, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, zFar);

    // Call the real function:
    gs_stat_realFunctionPointers.glFrustum(left, right, bottom, top, zNear, zFar);

    SU_END_FUNCTION_WRAPPER(ap_glFrustum);
}


GLuint APIENTRY glGenLists(GLsizei range)
{
    GLuint retVal = 0;

    SU_START_FUNCTION_WRAPPER(ap_glGenLists);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGenLists, 1, OS_TOBJ_ID_GL_SIZEI_PARAMETER, range);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.glGenLists(range);

    if (retVal > 0)
    {
        // The function succeeded in generating displays lists named n, n+1, n+2, ... , n+range-1:
        gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

        if (pCurrentThreadRenderContextMonitor)
        {
            gsDisplayListMonitor* displayListMonitor = pCurrentThreadRenderContextMonitor->displayListsMonitor();
            GT_IF_WITH_ASSERT(displayListMonitor != NULL)
            {
                displayListMonitor->addNewDisplayLists(retVal, range);
            }
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_glGenLists);

    return retVal;
}


void APIENTRY glGenTextures(GLsizei n, GLuint* textures)
{
    SU_START_FUNCTION_WRAPPER(ap_glGenTextures);

    // Log the call to this function:

    // If there are gs_stat_maxLoggedArraySize or less textures - we will log the textures array:
    if (n <= GLsizei(gs_stat_maxLoggedArraySize))
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGenTextures, 2, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_UINT_PARAMETER, n, textures);
    }
    else
    {
        // There are more than gs_stat_maxLoggedArraySize textures - we will log the array pointer:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGenTextures, 2, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_GL_P_UINT_PARAMETER, textures);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glGenTextures(n, textures);

    // Log the textures generation:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        gsTexturesMonitor* texturesMon = pCurrentThreadRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(texturesMon != NULL)
        {
            texturesMon->onTextureObjectsGeneration(n, textures);
        }
    }


    SU_END_FUNCTION_WRAPPER(ap_glGenTextures);
}


void APIENTRY glGetBooleanv(GLenum pname, GLboolean* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetBooleanv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetBooleanv, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_BOOL_PARAMETER, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetBooleanv(pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetBooleanv);
}


void APIENTRY glGetClipPlane(GLenum plane, GLdouble* equation)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetClipPlane);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetClipPlane, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, plane, OS_TOBJ_ID_GL_P_DOUBLE_PARAMETER, equation);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetClipPlane(plane, equation);

    SU_END_FUNCTION_WRAPPER(ap_glGetClipPlane);
}


void APIENTRY glGetDoublev(GLenum pname, GLdouble* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetDoublev);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetDoublev, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_DOUBLE_PARAMETER, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetDoublev(pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetDoublev);
}


GLenum APIENTRY glGetError(void)
{
    GLenum retVal = GL_NO_ERROR;

    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_FUNCTION_WRAPPER(ap_glGetError);

    if (!inNestedFunction)
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetError, 0);
    }

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.glGetError();

    SU_END_FUNCTION_WRAPPER(ap_glGetError);

    return retVal;
}


void APIENTRY glGetFloatv(GLenum pname, GLfloat* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetFloatv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetFloatv, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FLOAT_PARAMETER, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetFloatv(pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetFloatv);
}


void APIENTRY glGetIntegerv(GLenum pname, GLint* params)
{
    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_FUNCTION_WRAPPER(ap_glGetIntegerv);

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
#ifndef _GR_IPHONE_BUILD
    // The apple (CGL) implementation of glGetIntegerv internally calls glGetProgramivARB, glGetHandleARB and CGLGetParameter:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetProgramivARB);
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetHandleARB);
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_CGLGetParameter);
#endif // _GR_IPHONE_BUILD
#endif // ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))


    if (!inNestedFunction)
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetIntegerv, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_INT_PARAMETER, params);
    }

    if ((pname == GL_NUM_EXTENSIONS) || inNestedFunction)
    {
        // Get the current thread render context monitor:
        gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();
        GT_IF_WITH_ASSERT(pCurrentThreadRenderContextMonitor != NULL)
        {
            // Return the spy extensions string:
            int renderContextId = pCurrentThreadRenderContextMonitor->spyId();

            // Get the value from the extensions manager (including our extensions):
            GLint retVal = (GLint)(gs_stat_extensionsManager.getNumberOfSpyExtensionStrings(renderContextId));
            GT_IF_WITH_ASSERT(params != NULL)
            {
                *params = retVal;
            }
        }
    }
    else // pname != GL_NUM_EXTENSIONS && !inNestedFunction
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glGetIntegerv(pname, params);
    }

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
#ifndef _GR_IPHONE_BUILD
    // Restore the internal function code:
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetProgramivARB);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetHandleARB);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_CGLGetParameter);
#endif // _GR_IPHONE_BUILD
#endif // ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    SU_END_FUNCTION_WRAPPER(ap_glGetIntegerv);
}


void APIENTRY glGetLightfv(GLenum light, GLenum pname, GLfloat* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetLightfv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetLightfv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, light, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FLOAT_PARAMETER, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetLightfv(light, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetLightfv);
}


void APIENTRY glGetLightiv(GLenum light, GLenum pname, GLint* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetLightiv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetLightiv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, light, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_INT_PARAMETER, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetLightiv(light, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetLightiv);
}


void APIENTRY glGetMapdv(GLenum target, GLenum query, GLdouble* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetMapdv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetMapdv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, query, OS_TOBJ_ID_GL_P_DOUBLE_PARAMETER, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetMapdv(target, query, v);

    SU_END_FUNCTION_WRAPPER(ap_glGetMapdv);
}


void APIENTRY glGetMapfv(GLenum target, GLenum query, GLfloat* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetMapfv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetMapfv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, query, OS_TOBJ_ID_GL_P_FLOAT_PARAMETER, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetMapfv(target, query, v);

    SU_END_FUNCTION_WRAPPER(ap_glGetMapfv);
}


void APIENTRY glGetMapiv(GLenum target, GLenum query, GLint* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetMapiv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetMapiv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, query, OS_TOBJ_ID_GL_P_INT_PARAMETER, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetMapiv(target, query, v);

    SU_END_FUNCTION_WRAPPER(ap_glGetMapiv);
}


void APIENTRY glGetMaterialfv(GLenum face, GLenum pname, GLfloat* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetMaterialfv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetMaterialfv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, face, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FLOAT_PARAMETER, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetMaterialfv(face, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetMaterialfv);
}


void APIENTRY glGetMaterialiv(GLenum face, GLenum pname, GLint* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetMaterialiv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetMaterialiv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, face, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_INT_PARAMETER, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetMaterialiv(face, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetMaterialiv);
}


void APIENTRY glGetPixelMapfv(GLenum map, GLfloat* values)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetPixelMapfv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetPixelMapfv, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, map, OS_TOBJ_ID_GL_P_FLOAT_PARAMETER, values);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetPixelMapfv(map, values);

    SU_END_FUNCTION_WRAPPER(ap_glGetPixelMapfv);
}


void APIENTRY glGetPixelMapuiv(GLenum map, GLuint* values)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetPixelMapuiv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetPixelMapuiv, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, map, OS_TOBJ_ID_GL_P_UINT_PARAMETER, values);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetPixelMapuiv(map, values);

    SU_END_FUNCTION_WRAPPER(ap_glGetPixelMapuiv);
}


void APIENTRY glGetPixelMapusv(GLenum map, GLushort* values)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetPixelMapusv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetPixelMapusv, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, map, OS_TOBJ_ID_GL_P_USHORT_PARAMETER, values);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetPixelMapusv(map, values);

    SU_END_FUNCTION_WRAPPER(ap_glGetPixelMapusv);
}


void APIENTRY glGetPointerv(GLenum pname, GLvoid** params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetPointerv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetPointerv, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_PP_VOID_PARAMETER, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetPointerv(pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetPointerv);
}


void APIENTRY glGetPolygonStipple(GLubyte* mask)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetPolygonStipple);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetPolygonStipple, 1, OS_TOBJ_ID_GL_P_UBYTE_PARAMETER, mask);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetPolygonStipple(mask);

    SU_END_FUNCTION_WRAPPER(ap_glGetPolygonStipple);
}


const GLubyte* APIENTRY glGetString(GLenum name)
{
    const GLubyte* retVal = NULL;

    // If we should not log the glGetString call:
    bool areInitFunctionsLogged = gsAreInitializationFunctionsLogged();

    if (!areInitFunctionsLogged)
    {
        // Call the real function:
        retVal = gs_stat_realFunctionPointers.glGetString(name);
    }
    else
    {
        SU_START_FUNCTION_WRAPPER(ap_glGetString);

        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetString, 1, OS_TOBJ_ID_GL_ENUM_PARAMETER, name);

        // If we were asked about the extensions string:
        if (name == GL_EXTENSIONS)
        {
            // Get the current thread render context monitor:
            gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();
            GT_IF_WITH_ASSERT(pCurrentThreadRenderContextMonitor != NULL)
            {
                // Return the spy extensions string:
                int renderContextId = pCurrentThreadRenderContextMonitor->spyId();
                retVal = gs_stat_extensionsManager.getSpyUnifiedExtensionsString(renderContextId);
            }
        }
        else
        {
            //#if defined (_GR_OPENGL32) || defined (_GR_OPENGL_MODULE) || defined (_GR_OPENGLES_IPHONE)
#ifdef _AMDT_OPENGLSERVER_EXPORTS
            // If this is the regular spy or the iPhone Spy:
            // Call the real function implementation:
            retVal = gs_stat_realFunctionPointers.glGetString(name);

#elif defined OS_OGL_ES_IMPLEMENTATION_DLL_BUILD
            // If this is an OpenGL ES implementation DLL:

            if (name == GL_VENDOR)
            {
                // Return Graphic Remedy as the vendor:
                static wchar_t stat_vendorName[] = STRCOMPANYNAME;
                // TO_DO: Unicode - remove the translation!
                gtString vendorNameStr(stat_vendorName);
                retVal = (const GLubyte*)vendorNameStr.asASCIICharArray();
            }
            else if (name == GL_VERSION)
            {
                // Calculate the version string only once:
                static gtASCIIString stat_OGLESVersionString;

                if (stat_OGLESVersionString.isEmpty())
                {
                    // Add the OpenGL ES implementation type string:
#ifdef _GR_OPENGLES_COMMON
                    stat_OGLESVersionString = "OpenGL ES-CM ";
#endif

#ifdef _GR_OPENGLES_COMMON_LITE
                    stat_OGLESVersionString = "OpenGL ES-CL ";
#endif

                    // Add the OpenGL ES major and minor versions:
                    stat_OGLESVersionString.appendFormattedString("%d.%d", OGL_ES_MAJOR_VERSION, OGL_ES_MINOR_VERSION);
                }

                retVal = (const GLubyte*)(stat_OGLESVersionString.asCharArray());
            }
            else
            {
                // Call the real function implementation:
                retVal = gs_stat_realFunctionPointers.glGetString(name);
            }

#else
#error Unknown spy module!
#endif
        }

        SU_END_FUNCTION_WRAPPER(ap_glGetString);
    }

    GT_ASSERT(retVal != NULL);

    return retVal;
}


void APIENTRY glGetTexEnvfv(GLenum target, GLenum pname, GLfloat* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetTexEnvfv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetTexEnvfv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FLOAT_PARAMETER, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetTexEnvfv(target, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetTexEnvfv);
}


void APIENTRY glGetTexEnviv(GLenum target, GLenum pname, GLint* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetTexEnviv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetTexEnviv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_INT_PARAMETER, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetTexEnviv(target, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetTexEnviv);
}


void APIENTRY glGetTexGendv(GLenum coord, GLenum pname, GLdouble* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetTexGendv);

    // Log the call to this function:

    // Get the texture coordinates as strings:
    const char* coordAsString = gsTextureCoordinateString(coord);

    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetTexGendv, 3, OS_TOBJ_ID_STRING_PARAMETER, coordAsString, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_DOUBLE_PARAMETER, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetTexGendv(coord, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetTexGendv);
}


void APIENTRY glGetTexGenfv(GLenum coord, GLenum pname, GLfloat* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetTexGenfv);

    // Log the call to this function:

    // Get the texture coordinates as strings:
    const char* coordAsString = gsTextureCoordinateString(coord);

    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetTexGenfv, 3, OS_TOBJ_ID_STRING_PARAMETER, coordAsString, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FLOAT_PARAMETER, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetTexGenfv(coord, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetTexGenfv);
}


void APIENTRY glGetTexGeniv(GLenum coord, GLenum pname, GLint* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetTexGeniv);

    // Log the call to this function:

    // Get the texture coordinates as strings:
    const char* coordAsString = gsTextureCoordinateString(coord);

    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetTexGeniv, 3, OS_TOBJ_ID_STRING_PARAMETER, coordAsString, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_INT_PARAMETER, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetTexGeniv(coord, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetTexGeniv);
}


void APIENTRY glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid* pixels)
{
    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_FUNCTION_WRAPPER(ap_glGetTexImage);

    if (!inNestedFunction)
    {
        // Get the input bind target bounded texture name:
        GLuint boundTextureName = gs_stat_openGLMonitorInstance.boundTexture(target);

        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetTexImage, 6, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_INT_PARAMETER, level, OS_TOBJ_ID_GL_ENUM_PARAMETER, format, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_GL_P_VOID_PARAMETER, pixels, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glGetTexImage(target, level, format, type, pixels);

    SU_END_FUNCTION_WRAPPER(ap_glGetTexImage);
}


void APIENTRY glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetTexLevelParameterfv);

    // Get the input bind target bounded texture name:
    GLuint boundTextureName = gs_stat_openGLMonitorInstance.boundTexture(target);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetTexLevelParameterfv, 5, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_INT_PARAMETER, level, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FLOAT_PARAMETER, params, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetTexLevelParameterfv(target, level, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetTexLevelParameterfv);
}


void APIENTRY glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint* params)
{
    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_FUNCTION_WRAPPER(ap_glGetTexLevelParameteriv);

    if (!inNestedFunction)
    {
        // Get the input bind target bounded texture name:
        GLuint boundTextureName = gs_stat_openGLMonitorInstance.boundTexture(target);

        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetTexLevelParameteriv, 5, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_INT_PARAMETER, level, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_INT_PARAMETER, params, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glGetTexLevelParameteriv(target, level, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetTexLevelParameteriv);
}


void APIENTRY glGetTexParameterfv(GLenum target, GLenum pname, GLfloat* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetTexParameterfv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetTexParameterfv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FLOAT_PARAMETER, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetTexParameterfv(target, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetTexParameterfv);
}


void APIENTRY glGetTexParameteriv(GLenum target, GLenum pname, GLint* params)
{
    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_FUNCTION_WRAPPER(ap_glGetTexParameteriv);

    if (!inNestedFunction)
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetTexParameteriv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_INT_PARAMETER, params);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glGetTexParameteriv(target, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glGetTexParameteriv);
}


void APIENTRY glHint(GLenum target, GLenum mode)
{
    SU_START_FUNCTION_WRAPPER(ap_glHint);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glHint, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, mode);

    // Call the real function:
    gs_stat_realFunctionPointers.glHint(target, mode);

    SU_END_FUNCTION_WRAPPER(ap_glHint);
}


void APIENTRY glIndexMask(GLuint mask)
{
    SU_START_FUNCTION_WRAPPER(ap_glIndexMask);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glIndexMask, 1, OS_TOBJ_ID_GL_UINT_PARAMETER, mask);

    // Call the real function:
    gs_stat_realFunctionPointers.glIndexMask(mask);

    SU_END_FUNCTION_WRAPPER(ap_glIndexMask);
}


void APIENTRY glIndexPointer(GLenum type, GLsizei stride, const GLvoid* pointer)
{
    SU_START_FUNCTION_WRAPPER(ap_glIndexPointer);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glIndexPointer, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_GL_SIZEI_PARAMETER, stride, OS_TOBJ_ID_GL_P_VOID_PARAMETER, pointer);

    // Call the real function:
    gs_stat_realFunctionPointers.glIndexPointer(type, stride, pointer);

    SU_END_FUNCTION_WRAPPER(ap_glIndexPointer);
}


void APIENTRY glIndexd(GLdouble c)
{
    SU_START_FUNCTION_WRAPPER(ap_glIndexd);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glIndexd, 1, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, c);

    // Call the real function:
    gs_stat_realFunctionPointers.glIndexd(c);

    SU_END_FUNCTION_WRAPPER(ap_glIndexd);
}


void APIENTRY glIndexdv(const GLdouble* c)
{
    SU_START_FUNCTION_WRAPPER(ap_glIndexdv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glIndexdv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 1, c);

    // Call the real function:
    gs_stat_realFunctionPointers.glIndexdv(c);

    SU_END_FUNCTION_WRAPPER(ap_glIndexdv);
}


void APIENTRY glIndexf(GLfloat c)
{
    SU_START_FUNCTION_WRAPPER(ap_glIndexf);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glIndexf, 1, OS_TOBJ_ID_GL_FLOAT_PARAMETER, c);

    // Call the real function:
    gs_stat_realFunctionPointers.glIndexf(c);

    SU_END_FUNCTION_WRAPPER(ap_glIndexf);
}


void APIENTRY glIndexfv(const GLfloat* c)
{
    SU_START_FUNCTION_WRAPPER(ap_glIndexfv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glIndexfv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 1, c);

    // Call the real function:
    gs_stat_realFunctionPointers.glIndexfv(c);

    SU_END_FUNCTION_WRAPPER(ap_glIndexfv);
}


void APIENTRY glIndexi(GLint c)
{
    SU_START_FUNCTION_WRAPPER(ap_glIndexi);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glIndexi, 1, OS_TOBJ_ID_GL_INT_PARAMETER, c);

    // Call the real function:
    gs_stat_realFunctionPointers.glIndexi(c);

    SU_END_FUNCTION_WRAPPER(ap_glIndexi);
}


void APIENTRY glIndexiv(const GLint* c)
{
    SU_START_FUNCTION_WRAPPER(ap_glIndexiv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glIndexiv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 1, c);

    // Call the real function:
    gs_stat_realFunctionPointers.glIndexiv(c);

    SU_END_FUNCTION_WRAPPER(ap_glIndexiv);
}


void APIENTRY glIndexs(GLshort c)
{
    SU_START_FUNCTION_WRAPPER(ap_glIndexs);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glIndexs, 1, OS_TOBJ_ID_GL_SHORT_PARAMETER, c);

    // Call the real function:
    gs_stat_realFunctionPointers.glIndexs(c);

    SU_END_FUNCTION_WRAPPER(ap_glIndexs);
}


void APIENTRY glIndexsv(const GLshort* c)
{
    SU_START_FUNCTION_WRAPPER(ap_glIndexsv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glIndexsv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_SHORT_PARAMETER, 1, c);

    // Call the real function:
    gs_stat_realFunctionPointers.glIndexsv(c);

    SU_END_FUNCTION_WRAPPER(ap_glIndexsv);
}


void APIENTRY glIndexub(GLubyte c)
{
    SU_START_FUNCTION_WRAPPER(ap_glIndexub);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glIndexub, 1, OS_TOBJ_ID_GL_UBYTE_PARAMETER, c);

    // Call the real function:
    gs_stat_realFunctionPointers.glIndexub(c);

    SU_END_FUNCTION_WRAPPER(ap_glIndexub);
}


void APIENTRY glIndexubv(const GLubyte* c)
{
    SU_START_FUNCTION_WRAPPER(ap_glIndexubv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glIndexubv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_UBYTE_PARAMETER, 1, c);

    // Call the real function:
    gs_stat_realFunctionPointers.glIndexubv(c);

    SU_END_FUNCTION_WRAPPER(ap_glIndexubv);
}


void APIENTRY glInitNames(void)
{
    SU_START_FUNCTION_WRAPPER(ap_glInitNames);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glInitNames, 0);

    // Call the real function:
    gs_stat_realFunctionPointers.glInitNames();

    SU_END_FUNCTION_WRAPPER(ap_glInitNames);
}


void APIENTRY glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid* pointer)
{
    SU_START_FUNCTION_WRAPPER(ap_glInterleavedArrays);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glInterleavedArrays, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, format, OS_TOBJ_ID_GL_SIZEI_PARAMETER, stride, OS_TOBJ_ID_GL_P_VOID_PARAMETER, pointer);

    // Call the real function:
    gs_stat_realFunctionPointers.glInterleavedArrays(format, stride, pointer);

    SU_END_FUNCTION_WRAPPER(ap_glInterleavedArrays);
}


GLboolean APIENTRY glIsEnabled(GLenum cap)
{
    GLboolean retVal = GL_FALSE;

    SU_START_FUNCTION_WRAPPER(ap_glIsEnabled);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glIsEnabled, 1, OS_TOBJ_ID_GL_ENUM_PARAMETER, cap);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.glIsEnabled(cap);

    SU_END_FUNCTION_WRAPPER(ap_glIsEnabled);

    return retVal;
}


GLboolean APIENTRY glIsList(GLuint list)
{
    GLboolean retVal = false;

    SU_START_FUNCTION_WRAPPER(ap_glIsList);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glIsList, 1, OS_TOBJ_ID_GL_UINT_PARAMETER, list);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.glIsList(list);

    SU_END_FUNCTION_WRAPPER(ap_glIsList);

    return retVal;
}


GLboolean APIENTRY glIsTexture(GLuint texture)
{
    GLboolean retVal = false;

    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_FUNCTION_WRAPPER(ap_glIsTexture);

    if (!inNestedFunction)
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glIsTexture, 1, OS_TOBJ_ID_GL_UINT_PARAMETER, texture);
    }

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.glIsTexture(texture);

    SU_END_FUNCTION_WRAPPER(ap_glIsTexture);

    return retVal;
}


void APIENTRY glLightModelf(GLenum pname, GLfloat param)
{
    SU_START_FUNCTION_WRAPPER(ap_glLightModelf);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLightModelf, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_FLOAT_PARAMETER, param);

    // Call the real function:
    gs_stat_realFunctionPointers.glLightModelf(pname, param);

    SU_END_FUNCTION_WRAPPER(ap_glLightModelf);
}


void APIENTRY glLightModelfv(GLenum pname, const GLfloat* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glLightModelfv);

    // Log the call to this function:

    int amountOfArrayElements = 1;

    if (amountOfArrayElements == GL_LIGHT_MODEL_AMBIENT)
    {
        amountOfArrayElements = 4;
    }

    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLightModelfv, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, amountOfArrayElements, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glLightModelfv(pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glLightModelfv);
}


void APIENTRY glLightModeli(GLenum pname, GLint param)
{
    SU_START_FUNCTION_WRAPPER(ap_glLightModeli);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLightModeli, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_INT_PARAMETER, param);

    // Call the real function:
    gs_stat_realFunctionPointers.glLightModeli(pname, param);

    SU_END_FUNCTION_WRAPPER(ap_glLightModeli);
}


void APIENTRY glLightModeliv(GLenum pname, const GLint* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glLightModeliv);

    // Log the call to this function:

    int amountOfArrayElements = 1;

    if (amountOfArrayElements == GL_LIGHT_MODEL_AMBIENT)
    {
        amountOfArrayElements = 4;
    }

    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLightModeliv, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, amountOfArrayElements, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glLightModeliv(pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glLightModeliv);
}


void APIENTRY glLightf(GLenum light, GLenum pname, GLfloat param)
{
    SU_START_FUNCTION_WRAPPER(ap_glLightf);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLightf, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, light, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_FLOAT_PARAMETER, param);

    // Call the real function:
    gs_stat_realFunctionPointers.glLightf(light, pname, param);

    SU_END_FUNCTION_WRAPPER(ap_glLightf);
}


void APIENTRY glLightfv(GLenum light, GLenum pname, const GLfloat* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glLightfv);

    // Log the call to this function:

    int amountOfVectorItems = 1;

    if ((pname == GL_AMBIENT) || (pname == GL_DIFFUSE) || (pname == GL_SPECULAR) ||
        (pname == GL_POSITION))
    {
        amountOfVectorItems = 4;
    }
    else if (pname == GL_SPOT_DIRECTION)
    {
        amountOfVectorItems = 3;
    }

    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLightfv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, light, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, amountOfVectorItems, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glLightfv(light, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glLightfv);
}


void APIENTRY glLighti(GLenum light, GLenum pname, GLint param)
{
    SU_START_FUNCTION_WRAPPER(ap_glLighti);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLighti, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, light, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_INT_PARAMETER, param);

    // Call the real function:
    gs_stat_realFunctionPointers.glLighti(light, pname, param);

    SU_END_FUNCTION_WRAPPER(ap_glLighti);
}


void APIENTRY glLightiv(GLenum light, GLenum pname, const GLint* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glLightiv);

    // Log the call to this function:

    int amountOfVectorItems = 1;

    if ((pname == GL_AMBIENT) || (pname == GL_DIFFUSE) || (pname == GL_SPECULAR) ||
        (pname == GL_POSITION))
    {
        amountOfVectorItems = 4;
    }
    else if (pname == GL_SPOT_DIRECTION)
    {
        amountOfVectorItems = 3;
    }

    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLightfv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, light, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, amountOfVectorItems, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glLightiv(light, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glLightiv);
}


void APIENTRY glLineStipple(GLint factor, GLushort pattern)
{
    SU_START_FUNCTION_WRAPPER(ap_glLineStipple);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLineStipple, 2, OS_TOBJ_ID_GL_INT_PARAMETER, factor, OS_TOBJ_ID_GL_USHORT_PARAMETER, pattern);

    // Call the real function:
    gs_stat_realFunctionPointers.glLineStipple(factor, pattern);

    SU_END_FUNCTION_WRAPPER(ap_glLineStipple);
}


void APIENTRY glLineWidth(GLfloat width)
{
    SU_START_FUNCTION_WRAPPER(ap_glLineWidth);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLineWidth, 1, OS_TOBJ_ID_GL_FLOAT_PARAMETER, width);

    // Call the real function:
    gs_stat_realFunctionPointers.glLineWidth(width);

    SU_END_FUNCTION_WRAPPER(ap_glLineWidth);
}


void APIENTRY glListBase(GLuint base)
{
    SU_START_FUNCTION_WRAPPER(ap_glListBase);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glListBase, 1, OS_TOBJ_ID_GL_UINT_PARAMETER, base);

    // Call the real function:
    gs_stat_realFunctionPointers.glListBase(base);

    SU_END_FUNCTION_WRAPPER(ap_glListBase);
}


void APIENTRY glLoadIdentity(void)
{
    SU_START_FUNCTION_WRAPPER(ap_glLoadIdentity);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLoadIdentity, 0);

    // Call the real function:
    gs_stat_realFunctionPointers.glLoadIdentity();

    SU_END_FUNCTION_WRAPPER(ap_glLoadIdentity);
}


void APIENTRY glLoadMatrixd(const GLdouble* m)
{
    SU_START_FUNCTION_WRAPPER(ap_glLoadMatrixd);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLoadMatrixd, 1, OS_TOBJ_ID_MATRIX_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 4, 4, m);

    // Call the real function:
    gs_stat_realFunctionPointers.glLoadMatrixd(m);

    SU_END_FUNCTION_WRAPPER(ap_glLoadMatrixd);
}


void APIENTRY glLoadMatrixf(const GLfloat* m)
{
    SU_START_FUNCTION_WRAPPER(ap_glLoadMatrixf);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLoadMatrixf, 1, OS_TOBJ_ID_MATRIX_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 4, 4, m);

    // Call the real function:
    gs_stat_realFunctionPointers.glLoadMatrixf(m);

    SU_END_FUNCTION_WRAPPER(ap_glLoadMatrixf);
}


void APIENTRY glLoadName(GLuint name)
{
    SU_START_FUNCTION_WRAPPER(ap_glLoadName);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLoadName, 1, OS_TOBJ_ID_GL_UINT_PARAMETER, name);

    // Call the real function:
    gs_stat_realFunctionPointers.glLoadName(name);

    SU_END_FUNCTION_WRAPPER(ap_glLoadName);
}


void APIENTRY glLogicOp(GLenum opcode)
{
    SU_START_FUNCTION_WRAPPER(ap_glLogicOp);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLogicOp, 1, OS_TOBJ_ID_GL_ENUM_PARAMETER, opcode);

    // Call the real function:
    gs_stat_realFunctionPointers.glLogicOp(opcode);

    SU_END_FUNCTION_WRAPPER(ap_glLogicOp);
}


void APIENTRY glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble* points)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glMap1d);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMap1d, 6, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, u1, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, u2, OS_TOBJ_ID_GL_INT_PARAMETER, stride, OS_TOBJ_ID_GL_INT_PARAMETER, order, OS_TOBJ_ID_GL_P_DOUBLE_PARAMETER, points);

    // Call the real function:
    gs_stat_realFunctionPointers.glMap1d(target, u1, u2, stride, order, points);

    SU_END_FUNCTION_WRAPPER(ap_glMap1d);
}


void APIENTRY glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat* points)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glMap1f);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMap1f, 6, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_FLOAT_PARAMETER, u1, OS_TOBJ_ID_GL_FLOAT_PARAMETER, u2, OS_TOBJ_ID_GL_INT_PARAMETER, stride, OS_TOBJ_ID_GL_INT_PARAMETER, order, OS_TOBJ_ID_GL_P_FLOAT_PARAMETER, points);

    // Call the real function:
    gs_stat_realFunctionPointers.glMap1f(target, u1, u2, stride, order, points);

    SU_END_FUNCTION_WRAPPER(ap_glMap1f);
}


void APIENTRY glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble* points)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glMap2d);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMap2d, 10, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, u1, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, u2, OS_TOBJ_ID_GL_INT_PARAMETER, ustride, OS_TOBJ_ID_GL_INT_PARAMETER, uorder, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, v1, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, v2, OS_TOBJ_ID_GL_INT_PARAMETER, vstride, OS_TOBJ_ID_GL_INT_PARAMETER, vorder, OS_TOBJ_ID_GL_P_DOUBLE_PARAMETER, points);

    // Call the real function:
    gs_stat_realFunctionPointers.glMap2d(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);

    SU_END_FUNCTION_WRAPPER(ap_glMap2d);
}


void APIENTRY glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat* points)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glMap2f);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMap2f, 10, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_FLOAT_PARAMETER, u1, OS_TOBJ_ID_GL_FLOAT_PARAMETER, u2, OS_TOBJ_ID_GL_INT_PARAMETER, ustride, OS_TOBJ_ID_GL_INT_PARAMETER, uorder, OS_TOBJ_ID_GL_FLOAT_PARAMETER, v1, OS_TOBJ_ID_GL_FLOAT_PARAMETER, v2, OS_TOBJ_ID_GL_INT_PARAMETER, vstride, OS_TOBJ_ID_GL_INT_PARAMETER, vorder, OS_TOBJ_ID_GL_P_FLOAT_PARAMETER, points);

    // Call the real function:
    gs_stat_realFunctionPointers.glMap2f(target, u1, u2, ustride, uorder, v1, v2, vstride, vorder, points);

    SU_END_FUNCTION_WRAPPER(ap_glMap2f);
}


void APIENTRY glMapGrid1d(GLint un, GLdouble u1, GLdouble u2)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glMapGrid1d);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMapGrid1d, 3, OS_TOBJ_ID_GL_INT_PARAMETER, un, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, u1, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, u2);

    // Call the real function:
    gs_stat_realFunctionPointers.glMapGrid1d(un, u1, u2);

    SU_END_FUNCTION_WRAPPER(ap_glMapGrid1d);
}


void APIENTRY glMapGrid1f(GLint un, GLfloat u1, GLfloat u2)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glMapGrid1f);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMapGrid1f, 3, OS_TOBJ_ID_GL_INT_PARAMETER, un, OS_TOBJ_ID_GL_FLOAT_PARAMETER, u1, OS_TOBJ_ID_GL_FLOAT_PARAMETER, u2);

    // Call the real function:
    gs_stat_realFunctionPointers.glMapGrid1f(un, u1, u2);

    SU_END_FUNCTION_WRAPPER(ap_glMapGrid1f);
}


void APIENTRY glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glMapGrid2d);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMapGrid2d, 6, OS_TOBJ_ID_GL_INT_PARAMETER, un, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, u1, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, u2, OS_TOBJ_ID_GL_INT_PARAMETER, vn, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, v1, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, v2);

    // Call the real function:
    gs_stat_realFunctionPointers.glMapGrid2d(un, u1, u2, vn, v1, v2);

    SU_END_FUNCTION_WRAPPER(ap_glMapGrid2d);
}


void APIENTRY glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glMapGrid2f);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMapGrid2f, 6, OS_TOBJ_ID_GL_INT_PARAMETER, un, OS_TOBJ_ID_GL_FLOAT_PARAMETER, u1, OS_TOBJ_ID_GL_FLOAT_PARAMETER, u2, OS_TOBJ_ID_GL_INT_PARAMETER, vn, OS_TOBJ_ID_GL_FLOAT_PARAMETER, v1, OS_TOBJ_ID_GL_FLOAT_PARAMETER, v2);

    // Call the real function:
    gs_stat_realFunctionPointers.glMapGrid2f(un, u1, u2, vn, v1, v2);

    SU_END_FUNCTION_WRAPPER(ap_glMapGrid2f);
}


void APIENTRY glMaterialf(GLenum face, GLenum pname, GLfloat param)
{
    SU_START_FUNCTION_WRAPPER(ap_glMaterialf);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMaterialf, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, face, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_FLOAT_PARAMETER, param);

    // Call the real function:
    gs_stat_realFunctionPointers.glMaterialf(face, pname, param);

    SU_END_FUNCTION_WRAPPER(ap_glMaterialf);
}


void APIENTRY glMaterialfv(GLenum face, GLenum pname, const GLfloat* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glMaterialfv);

    // Log the call to this function:

    int amountOfArguments = 1;

    if ((pname == GL_AMBIENT) || (pname == GL_DIFFUSE) || (pname == GL_SPECULAR) ||
        (pname == GL_EMISSION) || (pname == GL_AMBIENT_AND_DIFFUSE))
    {
        amountOfArguments = 4;
    }
    else if (pname == GL_COLOR_INDEXES)
    {
        amountOfArguments = 3;
    }

    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMaterialfv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, face, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, amountOfArguments, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glMaterialfv(face, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glMaterialfv);
}


void APIENTRY glMateriali(GLenum face, GLenum pname, GLint param)
{
    SU_START_FUNCTION_WRAPPER(ap_glMateriali);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMateriali, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, face, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_INT_PARAMETER, param);

    // Call the real function:
    gs_stat_realFunctionPointers.glMateriali(face, pname, param);

    SU_END_FUNCTION_WRAPPER(ap_glMateriali);
}


void APIENTRY glMaterialiv(GLenum face, GLenum pname, const GLint* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glMaterialiv);

    // Log the call to this function:
    int amountOfArguments = 1;

    if ((pname == GL_AMBIENT) || (pname == GL_DIFFUSE) || (pname == GL_SPECULAR) ||
        (pname == GL_EMISSION) || (pname == GL_AMBIENT_AND_DIFFUSE))
    {
        amountOfArguments = 4;
    }
    else if (pname == GL_COLOR_INDEXES)
    {
        amountOfArguments = 3;
    }

    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMaterialfv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, face, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, amountOfArguments, params);

    // Call the real function:
    gs_stat_realFunctionPointers.glMaterialiv(face, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glMaterialiv);
}


void APIENTRY glMatrixMode(GLenum mode)
{
    SU_START_FUNCTION_WRAPPER(ap_glMatrixMode);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMatrixMode, 1, OS_TOBJ_ID_GL_ENUM_PARAMETER, mode);

    // Call the real function:
    gs_stat_realFunctionPointers.glMatrixMode(mode);

    SU_END_FUNCTION_WRAPPER(ap_glMatrixMode);
}


void APIENTRY glMultMatrixd(const GLdouble* m)
{
    SU_START_FUNCTION_WRAPPER(ap_glMultMatrixd);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMultMatrixd, 1, OS_TOBJ_ID_MATRIX_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 4, 4, m);

    // Call the real function:
    gs_stat_realFunctionPointers.glMultMatrixd(m);

    SU_END_FUNCTION_WRAPPER(ap_glMultMatrixd);
}


void APIENTRY glMultMatrixf(const GLfloat* m)
{
    SU_START_FUNCTION_WRAPPER(ap_glMultMatrixf);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMultMatrixf, 1, OS_TOBJ_ID_MATRIX_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 4, 4, m);

    // Call the real function:
    gs_stat_realFunctionPointers.glMultMatrixf(m);

    SU_END_FUNCTION_WRAPPER(ap_glMultMatrixf);
}


void APIENTRY glNewList(GLuint list, GLenum mode)
{
    SU_START_FUNCTION_WRAPPER(ap_glNewList);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glNewList, 2, OS_TOBJ_ID_GL_UINT_PARAMETER, list, OS_TOBJ_ID_GL_ENUM_PARAMETER, mode);

    // Log the Display list generation or binding:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        gsDisplayListMonitor* displayListMonitor = pCurrentThreadRenderContextMonitor->displayListsMonitor();
        GT_IF_WITH_ASSERT(displayListMonitor != NULL)
        {
            displayListMonitor->onNewList(list, mode);
        }
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glNewList(list, mode);

    SU_END_FUNCTION_WRAPPER(ap_glNewList);
}


void APIENTRY glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glNormal3b);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glNormal3b, 3, OS_TOBJ_ID_GL_BYTE_PARAMETER, nx, OS_TOBJ_ID_GL_BYTE_PARAMETER, ny, OS_TOBJ_ID_GL_BYTE_PARAMETER, nz);

    // Call the real function:
    gs_stat_realFunctionPointers.glNormal3b(nx, ny, nz);

    SU_END_FUNCTION_WRAPPER(ap_glNormal3b);
}


void APIENTRY glNormal3bv(const GLbyte* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glNormal3bv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glNormal3bv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_BYTE_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glNormal3bv(v);

    SU_END_FUNCTION_WRAPPER(ap_glNormal3bv);
}


void APIENTRY glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glNormal3d);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glNormal3d, 3, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, nx, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, ny, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, nz);

    // Call the real function:
    gs_stat_realFunctionPointers.glNormal3d(nx, ny, nz);

    SU_END_FUNCTION_WRAPPER(ap_glNormal3d);
}


void APIENTRY glNormal3dv(const GLdouble* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glNormal3dv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glNormal3dv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glNormal3dv(v);

    SU_END_FUNCTION_WRAPPER(ap_glNormal3dv);
}


void APIENTRY glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glNormal3f);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glNormal3f, 3, OS_TOBJ_ID_GL_FLOAT_PARAMETER, nx, OS_TOBJ_ID_GL_FLOAT_PARAMETER, ny, OS_TOBJ_ID_GL_FLOAT_PARAMETER, nz);

    // Call the real function:
    gs_stat_realFunctionPointers.glNormal3f(nx, ny, nz);

    SU_END_FUNCTION_WRAPPER(ap_glNormal3f);
}


void APIENTRY glNormal3fv(const GLfloat* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glNormal3fv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glNormal3fv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glNormal3fv(v);

    SU_END_FUNCTION_WRAPPER(ap_glNormal3fv);
}


void APIENTRY glNormal3i(GLint nx, GLint ny, GLint nz)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glNormal3i);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glNormal3i, 3, OS_TOBJ_ID_GL_INT_PARAMETER, nx, OS_TOBJ_ID_GL_INT_PARAMETER, ny, OS_TOBJ_ID_GL_INT_PARAMETER, nz);

    // Call the real function:
    gs_stat_realFunctionPointers.glNormal3i(nx, ny, nz);

    SU_END_FUNCTION_WRAPPER(ap_glNormal3i);
}


void APIENTRY glNormal3iv(const GLint* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glNormal3iv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glNormal3iv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glNormal3iv(v);

    SU_END_FUNCTION_WRAPPER(ap_glNormal3iv);
}


void APIENTRY glNormal3s(GLshort nx, GLshort ny, GLshort nz)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glNormal3s);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glNormal3s, 3, OS_TOBJ_ID_GL_SHORT_PARAMETER, nx, OS_TOBJ_ID_GL_SHORT_PARAMETER, ny, OS_TOBJ_ID_GL_SHORT_PARAMETER, nz);

    // Call the real function:
    gs_stat_realFunctionPointers.glNormal3s(nx, ny, nz);

    SU_END_FUNCTION_WRAPPER(ap_glNormal3s);
}


void APIENTRY glNormal3sv(const GLshort* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glNormal3sv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glNormal3sv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_SHORT_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glNormal3sv(v);

    SU_END_FUNCTION_WRAPPER(ap_glNormal3sv);
}


void APIENTRY glNormalPointer(GLenum type, GLsizei stride, const GLvoid* pointer)
{
    SU_START_FUNCTION_WRAPPER(ap_glNormalPointer);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glNormalPointer, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_GL_SIZEI_PARAMETER, stride, OS_TOBJ_ID_GL_P_VOID_PARAMETER, pointer);

    // If we are building the an OpenGL ES DLL:
#ifdef OS_OGL_ES_IMPLEMENTATION_DLL_BUILD

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        // Update the current vertex array data:
        gsVertexArrayData& curVertexArrayData = pCurrentThreadRenderContextMonitor->currentVertexArrayData();
        gsArrayPointer& normalsArrayData = curVertexArrayData._normalsArray;
        normalsArrayData._numOfCoordinates = 3;
        normalsArrayData._dataType = type;
        normalsArrayData._stride = stride;
        normalsArrayData._pArrayRawData = pointer;
    }

    // If the data type is supported by OpenGL:
    if (type != GL_FIXED)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glNormalPointer(type, stride, pointer);
    }

#else

    // None ES DLL - Just call the real function:
    gs_stat_realFunctionPointers.glNormalPointer(type, stride, pointer);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glNormalPointer);
}


void APIENTRY glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)
{
    SU_START_FUNCTION_WRAPPER(ap_glOrtho);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glOrtho, 6, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, left, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, right, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, bottom, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, top, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, zNear, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, zFar);

    // Call the real function:
    gs_stat_realFunctionPointers.glOrtho(left, right, bottom, top, zNear, zFar);

    SU_END_FUNCTION_WRAPPER(ap_glOrtho);
}


void APIENTRY glPassThrough(GLfloat token)
{
    SU_START_FUNCTION_WRAPPER(ap_glPassThrough);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPassThrough, 1, OS_TOBJ_ID_GL_FLOAT_PARAMETER, token);

    // Call the real function:
    gs_stat_realFunctionPointers.glPassThrough(token);

    SU_END_FUNCTION_WRAPPER(ap_glPassThrough);
}


void APIENTRY glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat* values)
{
    SU_START_FUNCTION_WRAPPER(ap_glPixelMapfv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPixelMapfv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, map, OS_TOBJ_ID_GL_SIZEI_PARAMETER, mapsize, OS_TOBJ_ID_GL_P_FLOAT_PARAMETER, values);

    // Call the real function:
    gs_stat_realFunctionPointers.glPixelMapfv(map, mapsize, values);

    SU_END_FUNCTION_WRAPPER(ap_glPixelMapfv);
}


void APIENTRY glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint* values)
{
    SU_START_FUNCTION_WRAPPER(ap_glPixelMapuiv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPixelMapuiv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, map, OS_TOBJ_ID_GL_SIZEI_PARAMETER, mapsize, OS_TOBJ_ID_GL_P_UINT_PARAMETER, values);

    // Call the real function:
    gs_stat_realFunctionPointers.glPixelMapuiv(map, mapsize, values);

    SU_END_FUNCTION_WRAPPER(ap_glPixelMapuiv);
}


void APIENTRY glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort* values)
{
    SU_START_FUNCTION_WRAPPER(ap_glPixelMapusv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPixelMapusv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, map, OS_TOBJ_ID_GL_SIZEI_PARAMETER, mapsize, OS_TOBJ_ID_GL_P_USHORT_PARAMETER, values);

    // Call the real function:
    gs_stat_realFunctionPointers.glPixelMapusv(map, mapsize, values);

    SU_END_FUNCTION_WRAPPER(ap_glPixelMapusv);
}


void APIENTRY glPixelStoref(GLenum pname, GLfloat param)
{
    SU_START_FUNCTION_WRAPPER(ap_glPixelStoref);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPixelStoref, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_FLOAT_PARAMETER, param);

    // Call the real function:
    gs_stat_realFunctionPointers.glPixelStoref(pname, param);

    SU_END_FUNCTION_WRAPPER(ap_glPixelStoref);
}


void APIENTRY glPixelStorei(GLenum pname, GLint param)
{
    SU_START_FUNCTION_WRAPPER(ap_glPixelStorei);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPixelStorei, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_INT_PARAMETER, param);

    // Call the real function:
    gs_stat_realFunctionPointers.glPixelStorei(pname, param);

    SU_END_FUNCTION_WRAPPER(ap_glPixelStorei);
}


void APIENTRY glPixelTransferf(GLenum pname, GLfloat param)
{
    SU_START_FUNCTION_WRAPPER(ap_glPixelTransferf);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPixelTransferf, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_FLOAT_PARAMETER, param);

    // Call the real function:
    gs_stat_realFunctionPointers.glPixelTransferf(pname, param);

    SU_END_FUNCTION_WRAPPER(ap_glPixelTransferf);
}


void APIENTRY glPixelTransferi(GLenum pname, GLint param)
{
    SU_START_FUNCTION_WRAPPER(ap_glPixelTransferi);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPixelTransferi, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_INT_PARAMETER, param);

    // Call the real function:
    gs_stat_realFunctionPointers.glPixelTransferi(pname, param);

    SU_END_FUNCTION_WRAPPER(ap_glPixelTransferi);
}


void APIENTRY glPixelZoom(GLfloat xfactor, GLfloat yfactor)
{
    SU_START_FUNCTION_WRAPPER(ap_glPixelZoom);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPixelZoom, 2, OS_TOBJ_ID_GL_FLOAT_PARAMETER, xfactor, OS_TOBJ_ID_GL_FLOAT_PARAMETER, yfactor);

    // Call the real function:
    gs_stat_realFunctionPointers.glPixelZoom(xfactor, yfactor);

    SU_END_FUNCTION_WRAPPER(ap_glPixelZoom);
}


void APIENTRY glPointSize(GLfloat size)
{
    SU_START_FUNCTION_WRAPPER(ap_glPointSize);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPointSize, 1, OS_TOBJ_ID_GL_FLOAT_PARAMETER, size);

    // Call the real function:
    gs_stat_realFunctionPointers.glPointSize(size);

    SU_END_FUNCTION_WRAPPER(ap_glPointSize);
}


void APIENTRY glPolygonMode(GLenum face, GLenum mode)
{
    SU_START_FUNCTION_WRAPPER(ap_glPolygonMode);

    bool isRasterModeForced = false;
    apRasterMode forcedRasterMode = AP_RASTER_LINES;

    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        // Log the "real" polygon raster mode:
        gsForcedModesManager& forcedModesMgr = pCurrentThreadRenderContextMonitor->forcedModesManager();
        forcedModesMgr.onPolygonRasterModeSet(face, mode);

        // Get the forced raster mode (if any):
        isRasterModeForced = forcedModesMgr.isStubForced(AP_OPENGL_FORCED_POLYGON_RASTER_MODE);

        if (isRasterModeForced)
        {
            forcedRasterMode = forcedModesMgr.forcedPolygonRasterMode();
        }
    }

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPolygonMode, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, face, OS_TOBJ_ID_GL_ENUM_PARAMETER, mode);

    // If the OpenGL raster mode is not forced:
    if (!isRasterModeForced)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glPolygonMode(face, mode);
    }
    else
    {
        // Set the forced raster mode:
        GLenum forcedRasterModeAsGLenum = apRasterModeToGLenum(forcedRasterMode);
        gs_stat_realFunctionPointers.glPolygonMode(face, forcedRasterModeAsGLenum);
    }

    SU_END_FUNCTION_WRAPPER(ap_glPolygonMode);
}


void APIENTRY glPolygonOffset(GLfloat factor, GLfloat units)
{
    SU_START_FUNCTION_WRAPPER(ap_glPolygonOffset);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPolygonOffset, 2, OS_TOBJ_ID_GL_FLOAT_PARAMETER, factor, OS_TOBJ_ID_GL_FLOAT_PARAMETER, units);

    // Call the real function:
    gs_stat_realFunctionPointers.glPolygonOffset(factor, units);

    SU_END_FUNCTION_WRAPPER(ap_glPolygonOffset);
}


void APIENTRY glPolygonStipple(const GLubyte* mask)
{
    SU_START_FUNCTION_WRAPPER(ap_glPolygonStipple);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPolygonStipple, 1, OS_TOBJ_ID_GL_P_UBYTE_PARAMETER, mask);

    // Call the real function:
    gs_stat_realFunctionPointers.glPolygonStipple(mask);

    SU_END_FUNCTION_WRAPPER(ap_glPolygonStipple);
}


void APIENTRY glPopAttrib(void)
{
    SU_START_FUNCTION_WRAPPER(ap_glPopAttrib);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPopAttrib, 0);

    // Call the real function:
    gs_stat_realFunctionPointers.glPopAttrib();

    // Get this thread render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        // Get its forced modes manager:
        gsForcedModesManager& forcedModesMgr = pCurrentThreadRenderContextMonitor->forcedModesManager();

        // Notify it that pop attrib was called:
        forcedModesMgr.onPopAttribCalled();
    }

    SU_END_FUNCTION_WRAPPER(ap_glPopAttrib);
}


void APIENTRY glPopClientAttrib(void)
{
    SU_START_FUNCTION_WRAPPER(ap_glPopClientAttrib);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPopClientAttrib, 0);

    // Call the real function:
    gs_stat_realFunctionPointers.glPopClientAttrib();

    SU_END_FUNCTION_WRAPPER(ap_glPopClientAttrib);
}


void APIENTRY glPopMatrix(void)
{
    SU_START_FUNCTION_WRAPPER(ap_glPopMatrix);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPopMatrix, 0);

    // Call the real function:
    gs_stat_realFunctionPointers.glPopMatrix();

    SU_END_FUNCTION_WRAPPER(ap_glPopMatrix);
}


void APIENTRY glPopName(void)
{
    SU_START_FUNCTION_WRAPPER(ap_glPopName);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPopName, 0);

    // Call the real function:
    gs_stat_realFunctionPointers.glPopName();

    SU_END_FUNCTION_WRAPPER(ap_glPopName);
}


void APIENTRY glPrioritizeTextures(GLsizei n, const GLuint* textures, const GLclampf* priorities)
{
    SU_START_FUNCTION_WRAPPER(ap_glPrioritizeTextures);

    // Log the call to this function:

    // If there are gs_stat_maxLoggedArraySize or less textures - we will log the textures and priorities array:
    if (n <= GLsizei(gs_stat_maxLoggedArraySize))
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPrioritizeTextures, 3, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_UINT_PARAMETER, n, textures, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_CLAMPF_PARAMETER, n, priorities);
    }
    else
    {
        // Log only the textures and priorities array pointers:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPrioritizeTextures, 3, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_GL_P_UINT_PARAMETER, textures, OS_TOBJ_ID_GL_P_CLAMPF_PARAMETER, priorities);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glPrioritizeTextures(n, textures, priorities);

    SU_END_FUNCTION_WRAPPER(ap_glPrioritizeTextures);
}


void APIENTRY glPushAttrib(GLbitfield mask)
{
    SU_START_FUNCTION_WRAPPER(ap_glPushAttrib);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPushAttrib, 1, OS_TOBJ_ID_GL_BITFIELD_PARAMETER, mask);

    // Call the real function:
    gs_stat_realFunctionPointers.glPushAttrib(mask);

    // Get this thread render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        // Get its forced modes manager:
        gsForcedModesManager& forcedModesMgr = pCurrentThreadRenderContextMonitor->forcedModesManager();

        // Notify it that push attrib was called:
        forcedModesMgr.onPushAttribCalled(mask);
    }

    SU_END_FUNCTION_WRAPPER(ap_glPushAttrib);
}


void APIENTRY glPushClientAttrib(GLbitfield mask)
{
    SU_START_FUNCTION_WRAPPER(ap_glPushClientAttrib);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPushClientAttrib, 1, OS_TOBJ_ID_GL_BITFIELD_PARAMETER, mask);

    // Call the real function:
    gs_stat_realFunctionPointers.glPushClientAttrib(mask);

    SU_END_FUNCTION_WRAPPER(ap_glPushClientAttrib);
}


void APIENTRY glPushMatrix(void)
{
    SU_START_FUNCTION_WRAPPER(ap_glPushMatrix);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPushMatrix, 0);

    // Call the real function:
    gs_stat_realFunctionPointers.glPushMatrix();

    SU_END_FUNCTION_WRAPPER(ap_glPushMatrix);
}


void APIENTRY glPushName(GLuint name)
{
    SU_START_FUNCTION_WRAPPER(ap_glPushName);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPushName, 1, OS_TOBJ_ID_GL_UINT_PARAMETER, name);

    // Call the real function:
    gs_stat_realFunctionPointers.glPushName(name);

    SU_END_FUNCTION_WRAPPER(ap_glPushName);
}


void APIENTRY glRasterPos2d(GLdouble x, GLdouble y)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos2d);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos2d, 2, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, x, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, y);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos2d(x, y);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos2d);
}


void APIENTRY glRasterPos2dv(const GLdouble* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos2dv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos2dv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 2, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos2dv(v);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos2dv);
}


void APIENTRY glRasterPos2f(GLfloat x, GLfloat y)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos2f);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos2f, 2, OS_TOBJ_ID_GL_FLOAT_PARAMETER, x, OS_TOBJ_ID_GL_FLOAT_PARAMETER, y);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos2f(x, y);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos2f);
}


void APIENTRY glRasterPos2fv(const GLfloat* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos2fv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos2fv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 2, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos2fv(v);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos2fv);
}


void APIENTRY glRasterPos2i(GLint x, GLint y)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos2i);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos2i, 2, OS_TOBJ_ID_GL_INT_PARAMETER, x, OS_TOBJ_ID_GL_INT_PARAMETER, y);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos2i(x, y);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos2i);
}


void APIENTRY glRasterPos2iv(const GLint* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos2iv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos2iv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 2, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos2iv(v);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos2iv);
}


void APIENTRY glRasterPos2s(GLshort x, GLshort y)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos2s);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos2s, 2, OS_TOBJ_ID_GL_SHORT_PARAMETER, x, OS_TOBJ_ID_GL_SHORT_PARAMETER, y);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos2s(x, y);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos2s);
}


void APIENTRY glRasterPos2sv(const GLshort* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos2sv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos2sv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_SHORT_PARAMETER, 2, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos2sv(v);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos2sv);
}


void APIENTRY glRasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos3d);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos3d, 3, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, x, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, y, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, z);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos3d(x, y, z);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos3d);
}


void APIENTRY glRasterPos3dv(const GLdouble* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos3dv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos3dv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos3dv(v);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos3dv);
}


void APIENTRY glRasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos3f);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos3f, 3, OS_TOBJ_ID_GL_FLOAT_PARAMETER, x, OS_TOBJ_ID_GL_FLOAT_PARAMETER, y, OS_TOBJ_ID_GL_FLOAT_PARAMETER, z);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos3f(x, y, z);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos3f);
}


void APIENTRY glRasterPos3fv(const GLfloat* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos3fv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos3fv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos3fv(v);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos3fv);
}


void APIENTRY glRasterPos3i(GLint x, GLint y, GLint z)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos3i);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos3i, 3, OS_TOBJ_ID_GL_INT_PARAMETER, x, OS_TOBJ_ID_GL_INT_PARAMETER, y, OS_TOBJ_ID_GL_INT_PARAMETER, z);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos3i(x, y, z);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos3i);
}


void APIENTRY glRasterPos3iv(const GLint* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos3iv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos3iv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos3iv(v);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos3iv);
}


void APIENTRY glRasterPos3s(GLshort x, GLshort y, GLshort z)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos3s);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos3s, 3, OS_TOBJ_ID_GL_SHORT_PARAMETER, x, OS_TOBJ_ID_GL_SHORT_PARAMETER, y, OS_TOBJ_ID_GL_SHORT_PARAMETER, z);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos3s(x, y, z);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos3s);
}


void APIENTRY glRasterPos3sv(const GLshort* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos3sv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos3sv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_SHORT_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos3sv(v);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos3sv);
}


void APIENTRY glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos4d);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos4d, 4, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, x, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, y, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, z, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, w);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos4d(x, y, z, w);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos4d);
}


void APIENTRY glRasterPos4dv(const GLdouble* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos4dv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos4dv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 4, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos4dv(v);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos4dv);
}


void APIENTRY glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos4f);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos4f, 4, OS_TOBJ_ID_GL_FLOAT_PARAMETER, x, OS_TOBJ_ID_GL_FLOAT_PARAMETER, y, OS_TOBJ_ID_GL_FLOAT_PARAMETER, z, OS_TOBJ_ID_GL_FLOAT_PARAMETER, w);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos4f(x, y, z, w);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos4f);
}


void APIENTRY glRasterPos4fv(const GLfloat* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos4fv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos4fv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 4, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos4fv(v);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos4fv);
}


void APIENTRY glRasterPos4i(GLint x, GLint y, GLint z, GLint w)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos4i);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos4i, 4, OS_TOBJ_ID_GL_INT_PARAMETER, x, OS_TOBJ_ID_GL_INT_PARAMETER, y, OS_TOBJ_ID_GL_INT_PARAMETER, z, OS_TOBJ_ID_GL_INT_PARAMETER, w);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos4i(x, y, z, w);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos4i);
}


void APIENTRY glRasterPos4iv(const GLint* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos4iv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos4iv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 4, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos4iv(v);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos4iv);
}


void APIENTRY glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos4s);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos4s, 4, OS_TOBJ_ID_GL_SHORT_PARAMETER, x, OS_TOBJ_ID_GL_SHORT_PARAMETER, y, OS_TOBJ_ID_GL_SHORT_PARAMETER, z, OS_TOBJ_ID_GL_SHORT_PARAMETER, w);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos4s(x, y, z, w);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos4s);
}


void APIENTRY glRasterPos4sv(const GLshort* v)
{
    SU_START_FUNCTION_WRAPPER(ap_glRasterPos4sv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRasterPos4sv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_SHORT_PARAMETER, 4, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glRasterPos4sv(v);

    SU_END_FUNCTION_WRAPPER(ap_glRasterPos4sv);
}


void APIENTRY glReadBuffer(GLenum mode)
{
    SU_START_FUNCTION_WRAPPER(ap_glReadBuffer);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glReadBuffer, 1, OS_TOBJ_ID_GL_ENUM_PARAMETER, mode);

    // Call the real function:
    gs_stat_realFunctionPointers.glReadBuffer(mode);

    SU_END_FUNCTION_WRAPPER(ap_glReadBuffer);
}


void APIENTRY glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glReadPixels);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glReadPixels, 7, OS_TOBJ_ID_GL_INT_PARAMETER, x, OS_TOBJ_ID_GL_INT_PARAMETER, y, OS_TOBJ_ID_GL_SIZEI_PARAMETER, width, OS_TOBJ_ID_GL_SIZEI_PARAMETER, height, OS_TOBJ_ID_GL_ENUM_PARAMETER, format, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_GL_P_VOID_PARAMETER, pixels);

    // Call the real function:
    gs_stat_realFunctionPointers.glReadPixels(x, y, width, height, format, type, pixels);

    SU_END_FUNCTION_WRAPPER(ap_glReadPixels);
}


void APIENTRY glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glRectd);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRectd, 4, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, x1, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, y1, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, x2, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, y2);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onRect(sizeof(GLdouble));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glRectd(x1, y1, x2, y2);

    SU_END_FUNCTION_WRAPPER(ap_glRectd);
}


void APIENTRY glRectdv(const GLdouble* v1, const GLdouble* v2)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glRectdv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRectdv, 2, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 2, v1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 2, v2);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onRect(sizeof(GLdouble));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glRectdv(v1, v2);

    SU_END_FUNCTION_WRAPPER(ap_glRectdv);
}


void APIENTRY glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glRectf);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRectf, 4, OS_TOBJ_ID_GL_FLOAT_PARAMETER, x1, OS_TOBJ_ID_GL_FLOAT_PARAMETER, y1, OS_TOBJ_ID_GL_FLOAT_PARAMETER, x2, OS_TOBJ_ID_GL_FLOAT_PARAMETER, y2);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onRect(sizeof(GLfloat));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glRectf(x1, y1, x2, y2);

    SU_END_FUNCTION_WRAPPER(ap_glRectf);
}


void APIENTRY glRectfv(const GLfloat* v1, const GLfloat* v2)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glRectfv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRectfv, 2, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 2, v1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 2, v2);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onRect(sizeof(GLfloat));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glRectfv(v1, v2);

    SU_END_FUNCTION_WRAPPER(ap_glRectfv);
}


void APIENTRY glRecti(GLint x1, GLint y1, GLint x2, GLint y2)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glRecti);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRecti, 4, OS_TOBJ_ID_GL_INT_PARAMETER, x1, OS_TOBJ_ID_GL_INT_PARAMETER, y1, OS_TOBJ_ID_GL_INT_PARAMETER, x2, OS_TOBJ_ID_GL_INT_PARAMETER, y2);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onRect(sizeof(GLint));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glRecti(x1, y1, x2, y2);

    SU_END_FUNCTION_WRAPPER(ap_glRecti);
}


void APIENTRY glRectiv(const GLint* v1, const GLint* v2)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glRectiv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRectiv, 2, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 2, v1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 2, v2);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onRect(sizeof(GLint));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glRectiv(v1, v2);

    SU_END_FUNCTION_WRAPPER(ap_glRectiv);
}


void APIENTRY glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glRects);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRects, 4, OS_TOBJ_ID_GL_SHORT_PARAMETER, x1, OS_TOBJ_ID_GL_SHORT_PARAMETER, y1, OS_TOBJ_ID_GL_SHORT_PARAMETER, x2, OS_TOBJ_ID_GL_SHORT_PARAMETER, y2);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onRect(sizeof(GLshort));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glRects(x1, y1, x2, y2);

    SU_END_FUNCTION_WRAPPER(ap_glRects);
}


void APIENTRY glRectsv(const GLshort* v1, const GLshort* v2)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glRectsv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRectsv, 2, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_SHORT_PARAMETER, 2, v1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_SHORT_PARAMETER, 2, v2);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onRect(sizeof(GLshort));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glRectsv(v1, v2);

    SU_END_FUNCTION_WRAPPER(ap_glRectsv);
}


GLint APIENTRY glRenderMode(GLenum mode)
{
    GLint retVal = -1;

    SU_START_FUNCTION_WRAPPER(ap_glRenderMode);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRenderMode, 1, OS_TOBJ_ID_GL_ENUM_PARAMETER, mode);

    // Call the real function:
    retVal = gs_stat_realFunctionPointers.glRenderMode(mode);

    SU_END_FUNCTION_WRAPPER(ap_glRenderMode);

    return retVal;
}


void APIENTRY glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    SU_START_FUNCTION_WRAPPER(ap_glRotated);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRotated, 4, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, angle, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, x, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, y, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, z);

    // Call the real function:
    gs_stat_realFunctionPointers.glRotated(angle, x, y, z);

    SU_END_FUNCTION_WRAPPER(ap_glRotated);
}


void APIENTRY glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z)
{
    SU_START_FUNCTION_WRAPPER(ap_glRotatef);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRotatef, 4, OS_TOBJ_ID_GL_FLOAT_PARAMETER, angle, OS_TOBJ_ID_GL_FLOAT_PARAMETER, x, OS_TOBJ_ID_GL_FLOAT_PARAMETER, y, OS_TOBJ_ID_GL_FLOAT_PARAMETER, z);

    // Call the real function:
    gs_stat_realFunctionPointers.glRotatef(angle, x, y, z);

    SU_END_FUNCTION_WRAPPER(ap_glRotatef);
}


void APIENTRY glScaled(GLdouble x, GLdouble y, GLdouble z)
{
    SU_START_FUNCTION_WRAPPER(ap_glScaled);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glScaled, 3, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, x, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, y, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, z);

    // Call the real function:
    gs_stat_realFunctionPointers.glScaled(x, y, z);

    SU_END_FUNCTION_WRAPPER(ap_glScaled);
}


void APIENTRY glScalef(GLfloat x, GLfloat y, GLfloat z)
{
    SU_START_FUNCTION_WRAPPER(ap_glScalef);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glScalef, 3, OS_TOBJ_ID_GL_FLOAT_PARAMETER, x, OS_TOBJ_ID_GL_FLOAT_PARAMETER, y, OS_TOBJ_ID_GL_FLOAT_PARAMETER, z);

    // Call the real function:
    gs_stat_realFunctionPointers.glScalef(x, y, z);

    SU_END_FUNCTION_WRAPPER(ap_glScalef);
}


void APIENTRY glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    SU_START_FUNCTION_WRAPPER(ap_glScissor);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glScissor, 4, OS_TOBJ_ID_GL_INT_PARAMETER, x, OS_TOBJ_ID_GL_INT_PARAMETER, y, OS_TOBJ_ID_GL_SIZEI_PARAMETER, width, OS_TOBJ_ID_GL_SIZEI_PARAMETER, height);

    // Call the real function:
    gs_stat_realFunctionPointers.glScissor(x, y, width, height);

    SU_END_FUNCTION_WRAPPER(ap_glScissor);
}


void APIENTRY glSelectBuffer(GLsizei size, GLuint* buffer)
{
    SU_START_FUNCTION_WRAPPER(ap_glSelectBuffer);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glSelectBuffer, 2, OS_TOBJ_ID_GL_SIZEI_PARAMETER, size, OS_TOBJ_ID_GL_P_UINT_PARAMETER, buffer);

    // Call the real function:
    gs_stat_realFunctionPointers.glSelectBuffer(size, buffer);

    SU_END_FUNCTION_WRAPPER(ap_glSelectBuffer);
}


void APIENTRY glShadeModel(GLenum mode)
{
    SU_START_FUNCTION_WRAPPER(ap_glShadeModel);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glShadeModel, 1, OS_TOBJ_ID_GL_ENUM_PARAMETER, mode);

    // Call the real function:
    gs_stat_realFunctionPointers.glShadeModel(mode);

    SU_END_FUNCTION_WRAPPER(ap_glShadeModel);
}


void APIENTRY glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
    SU_START_FUNCTION_WRAPPER(ap_glStencilFunc);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glStencilFunc, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, func, OS_TOBJ_ID_GL_INT_PARAMETER, ref, OS_TOBJ_ID_GL_UINT_PARAMETER, mask);

    // Call the real function:
    gs_stat_realFunctionPointers.glStencilFunc(func, ref, mask);

    SU_END_FUNCTION_WRAPPER(ap_glStencilFunc);
}


void APIENTRY glStencilMask(GLuint mask)
{
    SU_START_FUNCTION_WRAPPER(ap_glStencilMask);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glStencilMask, 1, OS_TOBJ_ID_GL_UINT_PARAMETER, mask);

    // Call the real function:
    gs_stat_realFunctionPointers.glStencilMask(mask);

    SU_END_FUNCTION_WRAPPER(ap_glStencilMask);
}


void APIENTRY glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
    SU_START_FUNCTION_WRAPPER(ap_glStencilOp);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glStencilOp, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, fail, OS_TOBJ_ID_GL_ENUM_PARAMETER, zfail, OS_TOBJ_ID_GL_ENUM_PARAMETER, zpass);

    // Call the real function:
    gs_stat_realFunctionPointers.glStencilOp(fail, zfail, zpass);

    SU_END_FUNCTION_WRAPPER(ap_glStencilOp);
}


void APIENTRY glTexCoord1d(GLdouble s)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord1d);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord1d, 1, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, s);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord1d(s);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord1d);
}


void APIENTRY glTexCoord1dv(const GLdouble* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord1dv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord1dv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 1, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord1dv(v);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord1dv);
}


void APIENTRY glTexCoord1f(GLfloat s)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord1f);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord1f, 1, OS_TOBJ_ID_GL_FLOAT_PARAMETER, s);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord1f(s);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord1f);
}


void APIENTRY glTexCoord1fv(const GLfloat* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord1fv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord1fv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 1, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord1fv(v);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord1fv);
}


void APIENTRY glTexCoord1i(GLint s)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord1i);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord1i, 1, OS_TOBJ_ID_GL_INT_PARAMETER, s);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord1i(s);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord1i);
}


void APIENTRY glTexCoord1iv(const GLint* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord1iv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord1iv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 1, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord1iv(v);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord1iv);
}


void APIENTRY glTexCoord1s(GLshort s)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord1s);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord1s, 1, OS_TOBJ_ID_GL_SHORT_PARAMETER, s);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord1s(s);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord1s);
}


void APIENTRY glTexCoord1sv(const GLshort* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord1sv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord1sv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_SHORT_PARAMETER, 1, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord1sv(v);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord1sv);
}


void APIENTRY glTexCoord2d(GLdouble s, GLdouble t)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord2d);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord2d, 2, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, s, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, t);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord2d(s, t);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord2d);
}


void APIENTRY glTexCoord2dv(const GLdouble* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord2dv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord2dv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 2, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord2dv(v);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord2dv);
}


void APIENTRY glTexCoord2f(GLfloat s, GLfloat t)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord2f);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord2f, 2, OS_TOBJ_ID_GL_FLOAT_PARAMETER, s, OS_TOBJ_ID_GL_FLOAT_PARAMETER, t);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord2f(s, t);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord2f);
}


void APIENTRY glTexCoord2fv(const GLfloat* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord2fv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord2fv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 2, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord2fv(v);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord2fv);
}


void APIENTRY glTexCoord2i(GLint s, GLint t)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord2i);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord2i, 2, OS_TOBJ_ID_GL_INT_PARAMETER, s, OS_TOBJ_ID_GL_INT_PARAMETER, t);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord2i(s, t);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord2i);
}


void APIENTRY glTexCoord2iv(const GLint* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord2iv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord2iv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 2, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord2iv(v);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord2iv);
}


void APIENTRY glTexCoord2s(GLshort s, GLshort t)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord2s);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord2s, 2, OS_TOBJ_ID_GL_SHORT_PARAMETER, s, OS_TOBJ_ID_GL_SHORT_PARAMETER, t);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord2s(s, t);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord2s);
}


void APIENTRY glTexCoord2sv(const GLshort* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord2sv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord2sv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_SHORT_PARAMETER, 2, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord2sv(v);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord2sv);
}


void APIENTRY glTexCoord3d(GLdouble s, GLdouble t, GLdouble r)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord3d);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord3d, 3, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, s, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, t, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, r);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord3d(s, t, r);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord3d);
}


void APIENTRY glTexCoord3dv(const GLdouble* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord3dv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord3dv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord3dv(v);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord3dv);
}


void APIENTRY glTexCoord3f(GLfloat s, GLfloat t, GLfloat r)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord3f);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord3f, 3, OS_TOBJ_ID_GL_FLOAT_PARAMETER, s, OS_TOBJ_ID_GL_FLOAT_PARAMETER, t, OS_TOBJ_ID_GL_FLOAT_PARAMETER, r);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord3f(s, t, r);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord3f);
}


void APIENTRY glTexCoord3fv(const GLfloat* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord3fv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord3fv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord3fv(v);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord3fv);
}


void APIENTRY glTexCoord3i(GLint s, GLint t, GLint r)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord3i);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord3i, 3, OS_TOBJ_ID_GL_INT_PARAMETER, s, OS_TOBJ_ID_GL_INT_PARAMETER, t, OS_TOBJ_ID_GL_INT_PARAMETER, r);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord3i(s, t, r);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord3i);
}


void APIENTRY glTexCoord3iv(const GLint* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord3iv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord3iv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord3iv(v);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord3iv);
}


void APIENTRY glTexCoord3s(GLshort s, GLshort t, GLshort r)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord3s);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord3s, 3, OS_TOBJ_ID_GL_SHORT_PARAMETER, s, OS_TOBJ_ID_GL_SHORT_PARAMETER, t, OS_TOBJ_ID_GL_SHORT_PARAMETER, r);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord3s(s, t, r);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord3s);
}


void APIENTRY glTexCoord3sv(const GLshort* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord3sv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord3sv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_SHORT_PARAMETER, 3, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord3sv(v);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord3sv);
}


void APIENTRY glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord4d);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord4d, 4, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, s, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, t, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, r, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, q);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord4d(s, t, r, q);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord4d);
}


void APIENTRY glTexCoord4dv(const GLdouble* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord4dv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord4dv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 4, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord4dv(v);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord4dv);
}


void APIENTRY glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord4f);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord4f, 4, OS_TOBJ_ID_GL_FLOAT_PARAMETER, s, OS_TOBJ_ID_GL_FLOAT_PARAMETER, t, OS_TOBJ_ID_GL_FLOAT_PARAMETER, r, OS_TOBJ_ID_GL_FLOAT_PARAMETER, q);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord4f(s, t, r, q);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord4f);
}


void APIENTRY glTexCoord4fv(const GLfloat* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord4fv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord4fv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 4, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord4fv(v);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord4fv);
}


void APIENTRY glTexCoord4i(GLint s, GLint t, GLint r, GLint q)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord4i);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord4i, 4, OS_TOBJ_ID_GL_INT_PARAMETER, s, OS_TOBJ_ID_GL_INT_PARAMETER, t, OS_TOBJ_ID_GL_INT_PARAMETER, r, OS_TOBJ_ID_GL_INT_PARAMETER, q);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord4i(s, t, r, q);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord4i);
}


void APIENTRY glTexCoord4iv(const GLint* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord4iv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord4iv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 4, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord4iv(v);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord4iv);
}


void APIENTRY glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord4s);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord4s, 4, OS_TOBJ_ID_GL_SHORT_PARAMETER, s, OS_TOBJ_ID_GL_SHORT_PARAMETER, t, OS_TOBJ_ID_GL_SHORT_PARAMETER, r, OS_TOBJ_ID_GL_SHORT_PARAMETER, q);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord4s(s, t, r, q);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord4s);
}


void APIENTRY glTexCoord4sv(const GLshort* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexCoord4sv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoord4sv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_SHORT_PARAMETER, 4, v);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexCoord4sv(v);

    SU_END_FUNCTION_WRAPPER(ap_glTexCoord4sv);
}


void APIENTRY glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    SU_START_FUNCTION_WRAPPER(ap_glTexCoordPointer);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexCoordPointer, 4, OS_TOBJ_ID_GL_INT_PARAMETER, size, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_GL_SIZEI_PARAMETER, stride, OS_TOBJ_ID_GL_P_VOID_PARAMETER, pointer);

    // If we are building the an OpenGL ES DLL:
#ifdef OS_OGL_ES_IMPLEMENTATION_DLL_BUILD

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        // Update the current vertex array data:
        gsVertexArrayData& curVertexArrayData = pCurrentThreadRenderContextMonitor->currentVertexArrayData();
        gsArrayPointer& textureCoordinatesArray = curVertexArrayData._textureCoordinatesArray;
        textureCoordinatesArray._numOfCoordinates = size;
        textureCoordinatesArray._dataType = type;
        textureCoordinatesArray._stride = stride;
        textureCoordinatesArray._pArrayRawData = pointer;
    }

    // If the data type is not supported by OpenGL:
    if ((type == GL_FIXED) || (type == GL_BYTE))
    {
        // We will set the texture coordinates pointer just before the render call that uses it.
    }
    else
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glTexCoordPointer(size, type, stride, pointer);
    }

#else

    // None ES DLL - Just call the real function:
    gs_stat_realFunctionPointers.glTexCoordPointer(size, type, stride, pointer);

#endif


    SU_END_FUNCTION_WRAPPER(ap_glTexCoordPointer);
}


void APIENTRY glTexEnvf(GLenum target, GLenum pname, GLfloat param)
{
    SU_START_FUNCTION_WRAPPER(ap_glTexEnvf);

    if (pname == GL_TEXTURE_ENV_MODE)
    {
        // In this case the parameter is not a floating point value, but rather an enumrator:
        GLenum paramAsEnum = (GLenum)param;
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexEnvf, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_ENUM_PARAMETER, paramAsEnum);
    }
    else
    {
        // Log the call to this function:
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexEnvf, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_FLOAT_PARAMETER, param);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glTexEnvf(target, pname, param);

    SU_END_FUNCTION_WRAPPER(ap_glTexEnvf);
}


void APIENTRY glTexEnvfv(GLenum target, GLenum pname, const GLfloat* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glTexEnvfv);

    // Log the call to this function:

    if (pname == GL_TEXTURE_ENV_MODE)
    {
        GLenum paramAsEnum = (GLenum) * params;
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexEnvfv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_ENUM_PARAMETER, paramAsEnum);
    }
    else if (pname == GL_TEXTURE_ENV_COLOR)
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexEnvfv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 4, params);
    }
    else
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexEnvfv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FLOAT_PARAMETER, params);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glTexEnvfv(target, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glTexEnvfv);
}


void APIENTRY glTexEnvi(GLenum target, GLenum pname, GLint param)
{
    SU_START_FUNCTION_WRAPPER(ap_glTexEnvi);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexEnvi, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_INT_PARAMETER, param);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexEnvi(target, pname, param);

    SU_END_FUNCTION_WRAPPER(ap_glTexEnvi);
}


void APIENTRY glTexEnviv(GLenum target, GLenum pname, const GLint* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glTexEnviv);

    // Log the call to this function:

    if (pname == GL_TEXTURE_ENV_MODE)
    {
        GLenum paramAsEnum = (GLenum) * params;
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexEnvfv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_ENUM_PARAMETER, paramAsEnum);
    }
    else if (pname == GL_TEXTURE_ENV_COLOR)
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexEnvfv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 4, params);
    }
    else
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexEnvfv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_INT_PARAMETER, params);
    }


    // Call the real function:
    gs_stat_realFunctionPointers.glTexEnviv(target, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glTexEnviv);
}


void APIENTRY glTexGend(GLenum coord, GLenum pname, GLdouble param)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexGend);

    // Get the texture coordinates as strings:
    const char* coordAsString = gsTextureCoordinateString(coord);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexGend, 3, OS_TOBJ_ID_STRING_PARAMETER, coordAsString, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, param);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexGend(coord, pname, param);

    SU_END_FUNCTION_WRAPPER(ap_glTexGend);
}


void APIENTRY glTexGendv(GLenum coord, GLenum pname, const GLdouble* params)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexGendv);

    // Log the call to this function:

    // Get the texture coordinates as strings:
    const char* coordAsString = gsTextureCoordinateString(coord);

    if (pname == GL_TEXTURE_GEN_MODE)
    {
        GLenum paramAsEnum = (GLenum) * params;
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexGendv, 3, OS_TOBJ_ID_STRING_PARAMETER, coordAsString, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_ENUM_PARAMETER, paramAsEnum);
    }
    else if ((pname == GL_OBJECT_PLANE) || (pname == GL_EYE_PLANE))
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexGendv, 3, OS_TOBJ_ID_STRING_PARAMETER, coordAsString, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 4, params);
    }
    else
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexGendv, 3, OS_TOBJ_ID_STRING_PARAMETER, coordAsString, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_DOUBLE_PARAMETER, params);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glTexGendv(coord, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glTexGendv);
}


void APIENTRY glTexGenf(GLenum coord, GLenum pname, GLfloat param)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexGenf);

    // Log the call to this function:

    // Get the texture coordinates as strings:
    const char* coordAsString = gsTextureCoordinateString(coord);

    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexGenf, 3, OS_TOBJ_ID_STRING_PARAMETER, coordAsString, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_FLOAT_PARAMETER, param);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexGenf(coord, pname, param);

    SU_END_FUNCTION_WRAPPER(ap_glTexGenf);
}


void APIENTRY glTexGenfv(GLenum coord, GLenum pname, const GLfloat* params)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexGenfv);

    // Log the call to this function:

    // Get the texture coordinates as strings:
    const char* coordAsString = gsTextureCoordinateString(coord);

    if (pname == GL_TEXTURE_GEN_MODE)
    {
        GLenum paramAsEnum = (GLenum) * params;
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexGenfv, 3, OS_TOBJ_ID_STRING_PARAMETER, coordAsString, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_ENUM_PARAMETER, paramAsEnum);
    }
    else if ((pname == GL_OBJECT_PLANE) || (pname == GL_EYE_PLANE))
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexGenfv, 3, OS_TOBJ_ID_STRING_PARAMETER, coordAsString, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 4, params);
    }
    else
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexGenfv, 3, OS_TOBJ_ID_STRING_PARAMETER, coordAsString, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FLOAT_PARAMETER, params);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glTexGenfv(coord, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glTexGenfv);
}


void APIENTRY glTexGeni(GLenum coord, GLenum pname, GLint param)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexGeni);

    // Log the call to this function:

    // Get the texture coordinates as strings:
    const char* coordAsString = gsTextureCoordinateString(coord);

    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexGeni, 3, OS_TOBJ_ID_STRING_PARAMETER, coordAsString, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_INT_PARAMETER, param);

    // Call the real function:
    gs_stat_realFunctionPointers.glTexGeni(coord, pname, param);

    SU_END_FUNCTION_WRAPPER(ap_glTexGeni);
}


void APIENTRY glTexGeniv(GLenum coord, GLenum pname, const GLint* params)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glTexGeniv);

    // Log the call to this function:

    // Get the texture coordinates as strings:
    const char* coordAsString = gsTextureCoordinateString(coord);

    if (pname == GL_TEXTURE_GEN_MODE)
    {
        GLenum paramAsEnum = (GLenum) * params;
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexGeniv, 3, OS_TOBJ_ID_STRING_PARAMETER, coordAsString, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_ENUM_PARAMETER, paramAsEnum);
    }
    else if ((pname == GL_OBJECT_PLANE) || (pname == GL_EYE_PLANE))
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexGeniv, 3, OS_TOBJ_ID_STRING_PARAMETER, coordAsString, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 4, params);
    }
    else
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexGeniv, 3, OS_TOBJ_ID_STRING_PARAMETER, coordAsString, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_INT_PARAMETER, params);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glTexGeniv(coord, pname, params);

    SU_END_FUNCTION_WRAPPER(ap_glTexGeniv);
}

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    void APIENTRY glTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
#else
    void APIENTRY glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
#endif
{
    SU_START_FUNCTION_WRAPPER(ap_glTexImage1D);

    // Log the call to this function:
    GLuint boundTextureName = gs_stat_openGLMonitorInstance.boundTexture(target);
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexImage1D, 9, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_INT_PARAMETER, level, OS_TOBJ_ID_GL_PIXEL_INTERNAL_FORMAT_PARAMETER, internalformat, OS_TOBJ_ID_GL_SIZEI_PARAMETER, width, OS_TOBJ_ID_GL_INT_PARAMETER, border, OS_TOBJ_ID_GL_ENUM_PARAMETER, format, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_GL_P_VOID_PARAMETER, pixels, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);


    // Check if stub textures are forced:
    bool areStubTexForced = false;
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        areStubTexForced = pCurrentThreadRenderContextMonitor->forcedModesManager().isStubForced(AP_OPENGL_FORCED_STUB_TEXTURES_MODE);
    }

    // If we are not in "Force stub textures" mode:
    if (!areStubTexForced)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glTexImage1D(target, level, internalformat, width, border, format, type, pixels);

        // Log the loaded texture:
        if (pCurrentThreadRenderContextMonitor)
        {
            bool rcTex = pCurrentThreadRenderContextMonitor->onTextureImageLoaded(target, level, internalformat, width, 0, 0, border, format, type);
            GT_ASSERT(rcTex);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_glTexImage1D);
}

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT) && (!defined(_GR_IPHONE_BUILD)))
    void APIENTRY glTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
#else
    void APIENTRY glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
#endif
{
    bool inNestedFunction = su_stat_interoperabilityHelper.isInNestedFunction();
    SU_START_FUNCTION_WRAPPER(ap_glTexImage2D);

    bool areStubTexForced = false;
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (!inNestedFunction)
    {
        // Log the call to this function:
        GLuint boundTextureName = gs_stat_openGLMonitorInstance.boundTexture(target);
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexImage2D, 10, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_INT_PARAMETER, level, OS_TOBJ_ID_GL_PIXEL_INTERNAL_FORMAT_PARAMETER, internalformat, OS_TOBJ_ID_GL_SIZEI_PARAMETER, width, OS_TOBJ_ID_GL_SIZEI_PARAMETER, height, OS_TOBJ_ID_GL_INT_PARAMETER, border, OS_TOBJ_ID_GL_ENUM_PARAMETER, format, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_GL_P_VOID_PARAMETER, pixels, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);

        // Check if stub textures are forced:
        if (pCurrentThreadRenderContextMonitor)
        {
            areStubTexForced = pCurrentThreadRenderContextMonitor->forcedModesManager().isStubForced(AP_OPENGL_FORCED_STUB_TEXTURES_MODE);
        }
    }

    // If we are not in "Force stub textures" mode:
    if (!areStubTexForced)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels);

        // Log the loaded texture:
        if ((pCurrentThreadRenderContextMonitor != NULL) && (!inNestedFunction))
        {
            bool rcTex = pCurrentThreadRenderContextMonitor->onTextureImageLoaded(target, level, internalformat, width, height, 0, border, format, type);
            GT_ASSERT(rcTex);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_glTexImage2D);
}


void APIENTRY glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    SU_START_FUNCTION_WRAPPER(ap_glTexParameterf);

    // Get the input bind target bounded texture name:
    GLuint boundTextureName = gs_stat_openGLMonitorInstance.boundTexture(target);

    // Log the call to this function:
    if ((pname == GL_TEXTURE_MIN_FILTER) || (pname == GL_TEXTURE_MAG_FILTER) ||
        (pname == GL_TEXTURE_COMPARE_MODE) || (pname == GL_TEXTURE_COMPARE_FUNC) || (pname == GL_DEPTH_TEXTURE_MODE) ||
        (pname == GL_TEXTURE_WRAP_S) || (pname == GL_TEXTURE_WRAP_T) || (pname == GL_TEXTURE_WRAP_R))
    {
        GLenum paramAsEnum = (GLenum)param;
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexParameterf, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_ENUM_PARAMETER, paramAsEnum, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);
    }
    else
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexParameterf, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_FLOAT_PARAMETER, param, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);
    }

    // Check if stub textures are forced:
    bool areStubTexForced = false;
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        areStubTexForced = pCurrentThreadRenderContextMonitor->forcedModesManager().isStubForced(AP_OPENGL_FORCED_STUB_TEXTURES_MODE);
    }

    // If we are not in "Force stub textures" mode:
    if (!areStubTexForced)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glTexParameterf(target, pname, param);
    }

    // Log the new parameter value:
    if (pCurrentThreadRenderContextMonitor)
    {
        gsTexturesMonitor* texturesMon = pCurrentThreadRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(texturesMon != NULL)
        {
            bool rcTexParamf = pCurrentThreadRenderContextMonitor->onTextureFloatParameterChanged(target, pname, &param);
            GT_ASSERT(rcTexParamf);
        }

        // Log the mipmap auto generation:
        if ((pname == GL_GENERATE_MIPMAP) || (pname == GL_TEXTURE_BASE_LEVEL) || (pname == GL_TEXTURE_MAX_LEVEL))
        {
            // Get the param value:
            GLfloat value = param;
            bool rcMipmapGeneration = pCurrentThreadRenderContextMonitor->onMipmapGenerateParamSet(target, pname, value);
            GT_ASSERT(rcMipmapGeneration);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_glTexParameterf);
}


void APIENTRY glTexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glTexParameterfv);

    // Get the input bind target bounded texture name:
    GLuint boundTextureName = gs_stat_openGLMonitorInstance.boundTexture(target);

    // Log the call to this function:

    if ((pname == GL_TEXTURE_MIN_FILTER) || (pname == GL_TEXTURE_MAG_FILTER) ||
        (pname == GL_TEXTURE_COMPARE_MODE) || (pname == GL_TEXTURE_COMPARE_FUNC) || (pname == GL_DEPTH_TEXTURE_MODE) ||
        (pname == GL_TEXTURE_WRAP_S) || (pname == GL_TEXTURE_WRAP_T) || (pname == GL_TEXTURE_WRAP_R))
    {
        GLenum paramAsEnum = (GLenum) * params;
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexParameterfv, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_ENUM_PARAMETER, paramAsEnum, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);
    }
    else if ((pname == GL_TEXTURE_BORDER_COLOR) || (pname == GL_TEXTURE_CROP_RECT_OES))
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexParameterfv, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 4, params, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);
    }
    else if (pname == GL_TEXTURE_PRIORITY)
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexParameterfv, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_FLOAT_PARAMETER, *params, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);
    }
    else
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexParameterfv, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FLOAT_PARAMETER, params, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);
    }

    // Check if stub textures are forced:
    bool areStubTexForced = false;
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        areStubTexForced = pCurrentThreadRenderContextMonitor->forcedModesManager().isStubForced(AP_OPENGL_FORCED_STUB_TEXTURES_MODE);
    }

    // If we are not in "Force stub textures" mode:
    if (!areStubTexForced)
    {
        // If the parameter is the crop parameter of the OES_draw_texture extension:
        if (pname == GL_TEXTURE_CROP_RECT_OES)
        {
            // Set the texture crop rectangle:
            if (pCurrentThreadRenderContextMonitor)
            {
                gsTexturesMonitor* texturesMon = pCurrentThreadRenderContextMonitor->texturesMonitor();
                GT_IF_WITH_ASSERT(texturesMon != NULL)
                {
                    bool rcTexRect = pCurrentThreadRenderContextMonitor->setTextureCropRectangle(target, params);
                    GT_ASSERT(rcTexRect);
                }
            }
        }
        else
        {
            // Call the real function:
            gs_stat_realFunctionPointers.glTexParameterfv(target, pname, params);
        }
    }

    // Log the new parameter value:
    if (pCurrentThreadRenderContextMonitor)
    {
        gsTexturesMonitor* texturesMon = pCurrentThreadRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(texturesMon != NULL)
        {
            bool rcTexParamf = pCurrentThreadRenderContextMonitor->onTextureFloatParameterChanged(target, pname, params);
            GT_ASSERT(rcTexParamf);
        }

        // Log the mipmap auto generation:
        if ((pname == GL_GENERATE_MIPMAP) || (pname == GL_TEXTURE_BASE_LEVEL) || (pname == GL_TEXTURE_MAX_LEVEL))
        {
            // Get the param value:
            GLfloat value = *params;
            bool rcMipmapGeneration = pCurrentThreadRenderContextMonitor->onMipmapGenerateParamSet(target, pname, value);
            GT_ASSERT(rcMipmapGeneration);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_glTexParameterfv);
}


void APIENTRY glTexParameteri(GLenum target, GLenum pname, GLint param)
{
    SU_START_FUNCTION_WRAPPER(ap_glTexParameteri);

    // Get the input bind target bounded texture name:
    GLuint boundTextureName = gs_stat_openGLMonitorInstance.boundTexture(target);

    // Log the call to this function:
    if ((pname == GL_TEXTURE_MIN_FILTER) || (pname == GL_TEXTURE_MAG_FILTER) ||
        (pname == GL_TEXTURE_COMPARE_MODE) || (pname == GL_TEXTURE_COMPARE_FUNC) || (pname == GL_DEPTH_TEXTURE_MODE) ||
        (pname == GL_TEXTURE_WRAP_S) || (pname == GL_TEXTURE_WRAP_T) || (pname == GL_TEXTURE_WRAP_R))
    {
        GLenum paramAsEnum = (GLenum)param;
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexParameteri, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_ENUM_PARAMETER, paramAsEnum, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);
    }
    else
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexParameteri, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_INT_PARAMETER, param, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);
    }

    // Check if stub textures are forced:
    bool areStubTexForced = false;
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        areStubTexForced = pCurrentThreadRenderContextMonitor->forcedModesManager().isStubForced(AP_OPENGL_FORCED_STUB_TEXTURES_MODE);
    }

    // If we are not in "Force stub textures" mode:
    if (!areStubTexForced)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glTexParameteri(target, pname, param);
    }

    // Log the new parameter value:
    if (pCurrentThreadRenderContextMonitor)
    {
        gsTexturesMonitor* texturesMon = pCurrentThreadRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(texturesMon != NULL)
        {
            bool rcTexParami = pCurrentThreadRenderContextMonitor->onTextureIntParameterChanged(target, pname, &param);
            GT_ASSERT(rcTexParami);
        }

        // Log the mipmap auto generation:
        if ((pname == GL_GENERATE_MIPMAP) || (pname == GL_TEXTURE_BASE_LEVEL) || (pname == GL_TEXTURE_MAX_LEVEL))
        {
            // Get the param value:
            GLint value = param;
            bool rcMipmapGeneration = pCurrentThreadRenderContextMonitor->onMipmapGenerateParamSet(target, pname, (GLfloat)value);
            GT_ASSERT(rcMipmapGeneration);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_glTexParameteri);
}


void APIENTRY glTexParameteriv(GLenum target, GLenum pname, const GLint* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glTexParameteriv);

    // Get the input bind target bounded texture name:
    GLuint boundTextureName = gs_stat_openGLMonitorInstance.boundTexture(target);

    // Log the call to this function:

    if ((pname == GL_TEXTURE_MIN_FILTER) || (pname == GL_TEXTURE_MAG_FILTER) ||
        (pname == GL_TEXTURE_COMPARE_MODE) || (pname == GL_TEXTURE_COMPARE_FUNC) || (pname == GL_DEPTH_TEXTURE_MODE) ||
        (pname == GL_TEXTURE_WRAP_S) || (pname == GL_TEXTURE_WRAP_T) || (pname == GL_TEXTURE_WRAP_R))
    {
        GLenum paramAsEnum = (GLenum) * params;
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexParameteriv, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_ENUM_PARAMETER, paramAsEnum, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);
    }
    else if ((pname == GL_TEXTURE_BORDER_COLOR) || (pname == GL_TEXTURE_CROP_RECT_OES))
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexParameteriv, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 4, params, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);
    }
    else if (pname == GL_TEXTURE_PRIORITY)
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexParameteriv, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_INT_PARAMETER, *params, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);
    }
    else
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexParameteriv, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_INT_PARAMETER, params, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);
    }

    // Check if stub textures are forced:
    bool areStubTexForced = false;
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        areStubTexForced = pCurrentThreadRenderContextMonitor->forcedModesManager().isStubForced(AP_OPENGL_FORCED_STUB_TEXTURES_MODE);
    }

    // If we are not in "Force stub textures" mode:
    if (!areStubTexForced)
    {
        // If the parameter is the crop parameter of the OES_draw_texture extension:
        if (pname == GL_TEXTURE_CROP_RECT_OES)
        {
            // Set the texture crop rectangle:
            if (pCurrentThreadRenderContextMonitor)
            {
                GLfloat paramAsFloat[4];
                paramAsFloat[0] = GLfloat(params[0]);
                paramAsFloat[1] = GLfloat(params[1]);
                paramAsFloat[2] = GLfloat(params[2]);
                paramAsFloat[3] = GLfloat(params[3]);

                gsTexturesMonitor* texturesMon = pCurrentThreadRenderContextMonitor->texturesMonitor();
                GT_IF_WITH_ASSERT(texturesMon != NULL)
                {
                    bool rcTexRect = pCurrentThreadRenderContextMonitor->setTextureCropRectangle(target, paramAsFloat);
                    GT_ASSERT(rcTexRect);
                }
            }
        }
        else
        {
            // Call the real function:
            gs_stat_realFunctionPointers.glTexParameteriv(target, pname, params);
        }
    }

    // Log the new parameter value:
    if (pCurrentThreadRenderContextMonitor)
    {
        gsTexturesMonitor* texturesMon = pCurrentThreadRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(texturesMon != NULL)
        {
            bool rcTexParami = pCurrentThreadRenderContextMonitor->onTextureIntParameterChanged(target, pname, params);
            GT_ASSERT(rcTexParami);
        }

        // Log the mipmap auto generation:
        if ((pname == GL_GENERATE_MIPMAP) || (pname == GL_TEXTURE_BASE_LEVEL) || (pname == GL_TEXTURE_MAX_LEVEL))
        {
            // Get the param value:
            GLint value = *params;
            bool rcMipmapGeneration = pCurrentThreadRenderContextMonitor->onMipmapGenerateParamSet(target, pname, (GLfloat)value);
            GT_ASSERT(rcMipmapGeneration);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_glTexParameteriv);
}


void APIENTRY glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid* pixels)
{
    SU_START_FUNCTION_WRAPPER(ap_glTexSubImage1D);

    // Check if stub textures are forced:
    bool areStubTexForced = false;
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        areStubTexForced = pCurrentThreadRenderContextMonitor->forcedModesManager().isStubForced(AP_OPENGL_FORCED_STUB_TEXTURES_MODE);
    }

    // If we are not in "Force stub textures" mode:
    if (!areStubTexForced)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glTexSubImage1D(target, level, xoffset, width, format, type, pixels);

        // Log the loaded sub-texture:
        if (pCurrentThreadRenderContextMonitor)
        {
            bool rcSubTex = pCurrentThreadRenderContextMonitor->onTextureSubImageLoaded(target, level);
            GT_ASSERT(rcSubTex);
        }
    }

    // Log the call to this function:
    GLuint boundTextureName = gs_stat_openGLMonitorInstance.boundTexture(target);
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexSubImage1D, 8, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_INT_PARAMETER, level, OS_TOBJ_ID_GL_INT_PARAMETER, xoffset, OS_TOBJ_ID_GL_SIZEI_PARAMETER, width, OS_TOBJ_ID_GL_ENUM_PARAMETER, format, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_GL_P_VOID_PARAMETER, pixels, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);

    SU_END_FUNCTION_WRAPPER(ap_glTexSubImage1D);
}


void APIENTRY glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
    SU_START_FUNCTION_WRAPPER(ap_glTexSubImage2D);

    // Check if stub textures are forced:
    bool areStubTexForced = false;
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        areStubTexForced = pCurrentThreadRenderContextMonitor->forcedModesManager().isStubForced(AP_OPENGL_FORCED_STUB_TEXTURES_MODE);
    }

    // If we are not in "Force stub textures" mode:
    if (!areStubTexForced)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels);

        // Log the loaded sub-texture:
        pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

        if (pCurrentThreadRenderContextMonitor)
        {
            bool rcSubTex = pCurrentThreadRenderContextMonitor->onTextureSubImageLoaded(target, level);
            GT_ASSERT(rcSubTex);
        }
    }

    // Log the call to this function:
    GLuint boundTextureName = gs_stat_openGLMonitorInstance.boundTexture(target);
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexSubImage2D, 10, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_INT_PARAMETER, level, OS_TOBJ_ID_GL_INT_PARAMETER, xoffset, OS_TOBJ_ID_GL_INT_PARAMETER, yoffset, OS_TOBJ_ID_GL_SIZEI_PARAMETER, width, OS_TOBJ_ID_GL_SIZEI_PARAMETER, height, OS_TOBJ_ID_GL_ENUM_PARAMETER, format, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_GL_P_VOID_PARAMETER, pixels, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);

    SU_END_FUNCTION_WRAPPER(ap_glTexSubImage2D);
}


void APIENTRY glTranslated(GLdouble x, GLdouble y, GLdouble z)
{
    SU_START_FUNCTION_WRAPPER(ap_glTranslated);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTranslated, 3, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, x, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, y, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, z);

    // Call the real function:
    gs_stat_realFunctionPointers.glTranslated(x, y, z);

    SU_END_FUNCTION_WRAPPER(ap_glTranslated);
}


void APIENTRY glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
    SU_START_FUNCTION_WRAPPER(ap_glTranslatef);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTranslatef, 3, OS_TOBJ_ID_GL_FLOAT_PARAMETER, x, OS_TOBJ_ID_GL_FLOAT_PARAMETER, y, OS_TOBJ_ID_GL_FLOAT_PARAMETER, z);

    // Call the real function:
    gs_stat_realFunctionPointers.glTranslatef(x, y, z);

    SU_END_FUNCTION_WRAPPER(ap_glTranslatef);
}


void APIENTRY glVertex2d(GLdouble x, GLdouble y)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex2d);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex2d, 2, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, x, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, y);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(2 * sizeof(GLdouble));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex2d(x, y);

    SU_END_FUNCTION_WRAPPER(ap_glVertex2d);
}


void APIENTRY glVertex2dv(const GLdouble* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex2dv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex2dv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 2, v);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(2 * sizeof(GLdouble));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex2dv(v);

    SU_END_FUNCTION_WRAPPER(ap_glVertex2dv);
}


void APIENTRY glVertex2f(GLfloat x, GLfloat y)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex2f);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex2f, 2, OS_TOBJ_ID_GL_FLOAT_PARAMETER, x, OS_TOBJ_ID_GL_FLOAT_PARAMETER, y);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(2 * sizeof(GLfloat));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex2f(x, y);

    SU_END_FUNCTION_WRAPPER(ap_glVertex2f);
}


void APIENTRY glVertex2fv(const GLfloat* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex2fv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex2fv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 2, v);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(2 * sizeof(GLfloat));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex2fv(v);

    SU_END_FUNCTION_WRAPPER(ap_glVertex2fv);
}


void APIENTRY glVertex2i(GLint x, GLint y)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex2i);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex2i, 2, OS_TOBJ_ID_GL_INT_PARAMETER, x, OS_TOBJ_ID_GL_INT_PARAMETER, y);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(2 * sizeof(GLint));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex2i(x, y);

    SU_END_FUNCTION_WRAPPER(ap_glVertex2i);
}


void APIENTRY glVertex2iv(const GLint* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex2iv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex2iv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 2, v);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(2 * sizeof(GLint));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex2iv(v);

    SU_END_FUNCTION_WRAPPER(ap_glVertex2iv);
}


void APIENTRY glVertex2s(GLshort x, GLshort y)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex2s);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex2s, 2, OS_TOBJ_ID_GL_SHORT_PARAMETER, x, OS_TOBJ_ID_GL_SHORT_PARAMETER, y);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(2 * sizeof(GLshort));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex2s(x, y);

    SU_END_FUNCTION_WRAPPER(ap_glVertex2s);
}


void APIENTRY glVertex2sv(const GLshort* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex2sv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex2sv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_SHORT_PARAMETER, 2, v);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(2 * sizeof(GLshort));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex2sv(v);

    SU_END_FUNCTION_WRAPPER(ap_glVertex2sv);
}


void APIENTRY glVertex3d(GLdouble x, GLdouble y, GLdouble z)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex3d);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex3d, 3, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, x, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, y, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, z);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(3 * sizeof(GLdouble));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex3d(x, y, z);

    SU_END_FUNCTION_WRAPPER(ap_glVertex3d);
}


void APIENTRY glVertex3dv(const GLdouble* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex3dv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex3dv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 3, v);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(3 * sizeof(GLdouble));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex3dv(v);

    SU_END_FUNCTION_WRAPPER(ap_glVertex3dv);
}


void APIENTRY glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex3f);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex3f, 3, OS_TOBJ_ID_GL_FLOAT_PARAMETER, x, OS_TOBJ_ID_GL_FLOAT_PARAMETER, y, OS_TOBJ_ID_GL_FLOAT_PARAMETER, z);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(3 * sizeof(GLfloat));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex3f(x, y, z);

    SU_END_FUNCTION_WRAPPER(ap_glVertex3f);
}


void APIENTRY glVertex3fv(const GLfloat* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex3fv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex3fv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 3, v);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(3 * sizeof(GLfloat));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex3fv(v);

    SU_END_FUNCTION_WRAPPER(ap_glVertex3fv);
}


void APIENTRY glVertex3i(GLint x, GLint y, GLint z)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex3i);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex3i, 3, OS_TOBJ_ID_GL_INT_PARAMETER, x, OS_TOBJ_ID_GL_INT_PARAMETER, y, OS_TOBJ_ID_GL_INT_PARAMETER, z);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(3 * sizeof(GLint));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex3i(x, y, z);

    SU_END_FUNCTION_WRAPPER(ap_glVertex3i);
}


void APIENTRY glVertex3iv(const GLint* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex3iv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex3iv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 3, v);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(3 * sizeof(GLint));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex3iv(v);

    SU_END_FUNCTION_WRAPPER(ap_glVertex3iv);
}


void APIENTRY glVertex3s(GLshort x, GLshort y, GLshort z)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex3s);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex3s, 3, OS_TOBJ_ID_GL_SHORT_PARAMETER, x, OS_TOBJ_ID_GL_SHORT_PARAMETER, y, OS_TOBJ_ID_GL_SHORT_PARAMETER, z);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(3 * sizeof(GLshort));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex3s(x, y, z);

    SU_END_FUNCTION_WRAPPER(ap_glVertex3s);
}


void APIENTRY glVertex3sv(const GLshort* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex3sv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex3sv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_SHORT_PARAMETER, 3, v);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(3 * sizeof(GLshort));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex3sv(v);

    SU_END_FUNCTION_WRAPPER(ap_glVertex3sv);
}


void APIENTRY glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex4d);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex4d, 4, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, x, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, y, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, z, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, w);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(4 * sizeof(GLdouble));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex4d(x, y, z, w);

    SU_END_FUNCTION_WRAPPER(ap_glVertex4d);
}


void APIENTRY glVertex4dv(const GLdouble* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex4dv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex4dv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_DOUBLE_PARAMETER, 4, v);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(4 * sizeof(GLdouble));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex4dv(v);

    SU_END_FUNCTION_WRAPPER(ap_glVertex4dv);
}


void APIENTRY glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex4f);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex4f, 4, OS_TOBJ_ID_GL_FLOAT_PARAMETER, x, OS_TOBJ_ID_GL_FLOAT_PARAMETER, y, OS_TOBJ_ID_GL_FLOAT_PARAMETER, z, OS_TOBJ_ID_GL_FLOAT_PARAMETER, w);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(4 * sizeof(GLfloat));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex4f(x, y, z, w);

    SU_END_FUNCTION_WRAPPER(ap_glVertex4f);
}


void APIENTRY glVertex4fv(const GLfloat* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex4fv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex4fv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 4, v);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(4 * sizeof(GLfloat));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex4fv(v);

    SU_END_FUNCTION_WRAPPER(ap_glVertex4fv);
}


void APIENTRY glVertex4i(GLint x, GLint y, GLint z, GLint w)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex4i);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex4i, 4, OS_TOBJ_ID_GL_INT_PARAMETER, x, OS_TOBJ_ID_GL_INT_PARAMETER, y, OS_TOBJ_ID_GL_INT_PARAMETER, z, OS_TOBJ_ID_GL_INT_PARAMETER, w);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(4 * sizeof(GLint));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex4i(x, y, z, w);

    SU_END_FUNCTION_WRAPPER(ap_glVertex4i);
}


void APIENTRY glVertex4iv(const GLint* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex4iv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex4iv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_INT_PARAMETER, 4, v);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(4 * sizeof(GLint));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex4iv(v);

    SU_END_FUNCTION_WRAPPER(ap_glVertex4iv);
}


void APIENTRY glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex4s);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex4s, 4, OS_TOBJ_ID_GL_SHORT_PARAMETER, x, OS_TOBJ_ID_GL_SHORT_PARAMETER, y, OS_TOBJ_ID_GL_SHORT_PARAMETER, z, OS_TOBJ_ID_GL_SHORT_PARAMETER, w);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertex(4 * sizeof(GLshort));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glVertex4s(x, y, z, w);

    SU_END_FUNCTION_WRAPPER(ap_glVertex4s);
}


void APIENTRY glVertex4sv(const GLshort* v)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glVertex4sv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertex4sv, 1, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_SHORT_PARAMETER, 4, v);



    // Call the real function:
    gs_stat_realFunctionPointers.glVertex4sv(v);

    SU_END_FUNCTION_WRAPPER(ap_glVertex4sv);
}


void APIENTRY glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    SU_START_FUNCTION_WRAPPER(ap_glVertexPointer);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glVertexPointer, 4, OS_TOBJ_ID_GL_INT_PARAMETER, size, OS_TOBJ_ID_GL_ENUM_PARAMETER, type, OS_TOBJ_ID_GL_SIZEI_PARAMETER, stride, OS_TOBJ_ID_GL_P_VOID_PARAMETER, pointer);

    // Get the current render context monitor:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor != NULL)
    {
        // Get the render primitives statistics logger:
        gsRenderPrimitivesStatisticsLogger& renderPrimitivesStatisticsLogger = pCurrentThreadRenderContextMonitor->renderPrimitivesStatisticsLogger();

        // Log this draw primitives function call:
        renderPrimitivesStatisticsLogger.onVertexPointer(size, type);
    }

    // If we are building the an OpenGL ES DLL:
#ifdef OS_OGL_ES_IMPLEMENTATION_DLL_BUILD

    if (pCurrentThreadRenderContextMonitor)
    {
        // Update the current vertex array data:
        gsVertexArrayData& curVertexArrayData = pCurrentThreadRenderContextMonitor->currentVertexArrayData();
        gsArrayPointer& verticesArrayData = curVertexArrayData._verticesArray;
        verticesArrayData._numOfCoordinates = size;
        verticesArrayData._dataType = type;
        verticesArrayData._stride = stride;
        verticesArrayData._pArrayRawData = pointer;
    }

    // If the data type is not supported by OpenGL:
    if ((type == GL_FIXED) || (type == GL_BYTE))
    {
        // We will set the vertex pointer just before the render call that uses it.
    }
    else
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glVertexPointer(size, type, stride, pointer);
    }

#else

    // None ES DLL - Just call the real function:
    gs_stat_realFunctionPointers.glVertexPointer(size, type, stride, pointer);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glVertexPointer);
}


void APIENTRY glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    SU_START_FUNCTION_WRAPPER(ap_glViewport);

    // Will get true iff single view port mode is forced:
    bool isSinglePixViewPortForced = false;

    // Will get true iff this function is called when an OpenGL context is current.
    // (See case 2672 where some Linux distributions crash when calling glViewport when
    //  there is no active render context)
    bool isUnderRenderContext = false;

    // Get the current thread's current render context:
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        // If we are under a render context:
        int currentContextId = pCurrentThreadRenderContextMonitor->spyId();

        if (0 < currentContextId)
        {
            isUnderRenderContext = true;
        }

        // Log the "real" view port:
        gsForcedModesManager& forcedModesMgr = pCurrentThreadRenderContextMonitor->forcedModesManager();
        forcedModesMgr.onViewPortSet(x, y, width, height);

        // Check if a single pixel view port is forced:
        isSinglePixViewPortForced = forcedModesMgr.isStubForced(AP_OPENGL_FORCED_SINGLE_PIXEL_VIEW_PORT);
    }

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glViewport, 4, OS_TOBJ_ID_GL_INT_PARAMETER, x, OS_TOBJ_ID_GL_INT_PARAMETER, y, OS_TOBJ_ID_GL_SIZEI_PARAMETER, width, OS_TOBJ_ID_GL_SIZEI_PARAMETER, height);

    // If a single pixel view port is not forced:
    if (!isSinglePixViewPortForced)
    {
        // Some Linux distributions crash when calling glViewport when there is no active
        // render context (see case 2672)
        if (isUnderRenderContext)
        {
            // Call the real function:
            gs_stat_realFunctionPointers.glViewport(x, y, width, height);
        }
    }
    else
    {
        // Set the forced one pixel view-port:
        gs_stat_realFunctionPointers.glViewport(x, y, 1, 1);
    }

    SU_END_FUNCTION_WRAPPER(ap_glViewport);
}

