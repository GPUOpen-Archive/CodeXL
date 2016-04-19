//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsOpenGLESWrappers.cpp
///
//==================================================================================

//------------------------------ gsOpenGLESWrappers.cpp ------------------------------

// --------------------------------------------------------
// File:
// This file contains wrapper functions for the unique OpenGL ES functions.
// --------------------------------------------------------

// OpenGL:
#include <AMDTOSWrappers/osOpenGLIncludes.h>

// Local:
#include <inc/gsStringConstants.h>
#include <inc/gsMonitoredFunctionPointers.h>
#include <inc/gsWrappersCommon.h>
#include <inc/gsExtensionsManager.h>
#include <inc/gsOpenGLMonitor.h>


// --------------------------------------------------------
//             OpenGL ES Wrapper functions
// --------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// OpenGL ES
//////////////////////////////////////////////////////////////////////////
void GL_APIENTRY glAlphaFuncx(GLenum func, GLclampx ref)
{
    SU_START_FUNCTION_WRAPPER(ap_glAlphaFuncx);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glAlphaFuncx, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, func, OS_TOBJ_ID_GL_CLAMPX_PARAMETER, ref);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLclampf refAsGLclumpf = (GLclampf)apGLclampxParameter::clampxToFloat(ref);

    // Call the real function:
    gs_stat_realFunctionPointers.glAlphaFunc(func, refAsGLclumpf);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glAlphaFuncx(func, ref);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glAlphaFuncx);
}

void GL_APIENTRY glClearColorx(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha)
{
    SU_START_FUNCTION_WRAPPER(ap_glClearColorx);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glClearColorx, 4, OS_TOBJ_ID_GL_CLAMPX_PARAMETER, red, OS_TOBJ_ID_GL_CLAMPX_PARAMETER, green, OS_TOBJ_ID_GL_CLAMPX_PARAMETER, blue, OS_TOBJ_ID_GL_CLAMPX_PARAMETER, alpha);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLclampf redAsGLclumpf = (GLclampf)apGLclampxParameter::clampxToFloat(red);
    GLclampf greenAsGLclumpf = (GLclampf)apGLclampxParameter::clampxToFloat(green);
    GLclampf blueAsGLclumpf = (GLclampf)apGLclampxParameter::clampxToFloat(blue);
    GLclampf alphaAsGLclumpf = (GLclampf)apGLclampxParameter::clampxToFloat(alpha);

    // Call the real function:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glClearColor);
    gs_stat_realFunctionPointers.glClearColor(redAsGLclumpf, greenAsGLclumpf, blueAsGLclumpf, alphaAsGLclumpf);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glClearColor);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    GT_ASSERT((void*)(gs_stat_realFunctionPointers.glClearColorx) == (((void**)(&gs_stat_realFunctionPointers))[ap_glClearColorx]));
    gs_stat_realFunctionPointers.glClearColorx(red, green, blue, alpha);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glClearColorx);
}

void GL_APIENTRY glClearDepthf(GLclampf depth)
{
    SU_START_FUNCTION_WRAPPER(ap_glClearDepthf);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glClearDepthf, 1, OS_TOBJ_ID_GL_CLAMPF_PARAMETER, depth);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLclampd depthAsGLclumpd = GLclampd(depth);

    // Call the real function:
    gs_stat_realFunctionPointers.glClearDepth(depthAsGLclumpd);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glClearDepthf(depth);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glClearDepthf);
}

void GL_APIENTRY glClearDepthx(GLclampx depth)
{
    SU_START_FUNCTION_WRAPPER(ap_glClearDepthx);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glClearDepthx, 1, OS_TOBJ_ID_GL_CLAMPX_PARAMETER, depth);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLclampd depthAsGLclumpd = GLclampd(apGLclampxParameter::clampxToFloat(depth));

    // Call the real function:
    gs_stat_realFunctionPointers.glClearDepth(depthAsGLclumpd);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glClearDepthx(depth);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glClearDepthx);
}

void GL_APIENTRY glClipPlanef(GLenum plane, const GLfloat* equation)
{
    SU_START_FUNCTION_WRAPPER(ap_glClipPlanef);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glClipPlanef, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, plane, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FLOAT_PARAMETER, 4, equation);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLdouble equationAsDouble[4];

    for (int i = 0; i < 4; i++)
    {
        equationAsDouble[i] = GLdouble(equation[i]);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glClipPlane(plane, equationAsDouble);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glClipPlanef(plane, equation);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glClipPlanef);
}

void GL_APIENTRY glClipPlanex(GLenum plane, const GLfixed* equation)
{
    SU_START_FUNCTION_WRAPPER(ap_glClipPlanex);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glClipPlanex, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, plane, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FIXED_PARAMETER, 4, equation);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLdouble equationAsDouble[4];

    for (int i = 0; i < 4; i++)
    {
        equationAsDouble[i] = GLdouble(apGLfixedParameter::fixedToFloat(equation[i]));
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glClipPlane(plane, equationAsDouble);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glClipPlanex(plane, equation);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glClipPlanex);
}

void GL_APIENTRY glColor4x(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
    SU_START_FUNCTION_WRAPPER(ap_glColor4x);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glColor4x, 4, OS_TOBJ_ID_GL_FIXED_PARAMETER, red, OS_TOBJ_ID_GL_FIXED_PARAMETER, green, OS_TOBJ_ID_GL_FIXED_PARAMETER, blue, OS_TOBJ_ID_GL_FIXED_PARAMETER, alpha);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLfloat redAsGLfloat = GLfloat(apGLfixedParameter::fixedToFloat(red));
    GLfloat greenAsGLfloat = GLfloat(apGLfixedParameter::fixedToFloat(green));
    GLfloat blueAsGLfloat = GLfloat(apGLfixedParameter::fixedToFloat(blue));
    GLfloat alphaAsGLfloat = GLfloat(apGLfixedParameter::fixedToFloat(alpha));

    // Call the real function:
    gs_stat_realFunctionPointers.glColor4f(redAsGLfloat, greenAsGLfloat, blueAsGLfloat, alphaAsGLfloat);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glColor4x(red, green, blue, alpha);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glColor4x);
}

void GL_APIENTRY glDepthRangef(GLclampf zNear, GLclampf zFar)
{
    SU_START_FUNCTION_WRAPPER(ap_glDepthRangef);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDepthRangef, 2, OS_TOBJ_ID_GL_CLAMPF_PARAMETER, zNear, OS_TOBJ_ID_GL_CLAMPF_PARAMETER, zFar);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLclampd zNearAsGLclumpd = GLclampd(zNear);
    GLclampd zFarAsGLclumpd = GLclampd(zFar);

    // Call the real function:
    gs_stat_realFunctionPointers.glDepthRange(zNearAsGLclumpd, zFarAsGLclumpd);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glDepthRangef(zNear, zFar);

#endif


    SU_END_FUNCTION_WRAPPER(ap_glDepthRangef);
}

void GL_APIENTRY glDepthRangex(GLclampx zNear, GLclampx zFar)
{
    SU_START_FUNCTION_WRAPPER(ap_glDepthRangex);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glDepthRangex, 2, OS_TOBJ_ID_GL_CLAMPX_PARAMETER, zNear, OS_TOBJ_ID_GL_CLAMPX_PARAMETER, zFar);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLclampd zNearAsGLclumpd = GLclampd(apGLclampxParameter::clampxToFloat(zNear));
    GLclampd zFarAsGLclumpd = GLclampd(apGLclampxParameter::clampxToFloat(zFar));

    // Call the real function:
    gs_stat_realFunctionPointers.glDepthRange(zNearAsGLclumpd, zFarAsGLclumpd);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glDepthRangex(zNear, zFar);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glDepthRangex);
}

void GL_APIENTRY glFogx(GLenum pname, GLfixed param)
{
    SU_START_FUNCTION_WRAPPER(ap_glFogx);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glFogx, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_FIXED_PARAMETER, param);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Enum parameters:
    if (pname == GL_FOG_MODE)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glFogi(pname, param);
    }
    else
    {
        // Fixed parameter: translate fixed to float:
        GLfloat paramAsGLfloat = GLfloat(apGLfixedParameter::fixedToFloat(param));

        // Call the real function:
        gs_stat_realFunctionPointers.glFogf(pname, paramAsGLfloat);
    }

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glFogx(pname, param);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glFogx);
}


void GL_APIENTRY glFogxv(GLenum pname, const GLfixed* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glFogxv);

    // Calculate the amount of elements in the params array:
    int amountOfArrayElements = 1;

    if (pname == GL_FOG_COLOR)
    {
        amountOfArrayElements = 4;
    }

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glFogxv, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FIXED_PARAMETER, amountOfArrayElements, params);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // If this is an enum parameter:
    if (pname == GL_FOG_MODE)
    {
        // Translate OpenGL ES parameters to OpenGL parameters:
        GLint paramsAsInt[1];
        paramsAsInt[0] = (GLint)apGLfixedParameter::fixedToInt(params[0]);

        // Call the real function:
        gs_stat_realFunctionPointers.glFogiv(pname, paramsAsInt);
    }
    else
    {
        // Translate OpenGL ES parameters to OpenGL parameters:
        GLfloat paramsAsFloat[4];

        for (int i = 0; i < amountOfArrayElements; i++)
        {
            paramsAsFloat[i] = (GLfloat)apGLfixedParameter::fixedToFloat(params[i]);
        }

        // Call the real function:
        gs_stat_realFunctionPointers.glFogfv(pname, paramsAsFloat);
    }

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glFogxv(pname, params);

#endif
    SU_END_FUNCTION_WRAPPER(ap_glFogxv);
}

void GL_APIENTRY glFrustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
    SU_START_FUNCTION_WRAPPER(ap_glFrustumf);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glFrustumf, 6, OS_TOBJ_ID_GL_FLOAT_PARAMETER, left, OS_TOBJ_ID_GL_FLOAT_PARAMETER, right, OS_TOBJ_ID_GL_FLOAT_PARAMETER, bottom, OS_TOBJ_ID_GL_FLOAT_PARAMETER, top, OS_TOBJ_ID_GL_FLOAT_PARAMETER, zNear, OS_TOBJ_ID_GL_FLOAT_PARAMETER, zFar);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLdouble leftAsDouble = GLdouble(left);
    GLdouble rightAsDouble = GLdouble(right);
    GLdouble bottomAsDouble = GLdouble(bottom);
    GLdouble topAsDouble = GLdouble(top);
    GLdouble zNearAsDouble = GLdouble(zNear);
    GLdouble zFarAsDouble = GLdouble(zFar);

    // Call the real function:
    gs_stat_realFunctionPointers.glFrustum(leftAsDouble, rightAsDouble, bottomAsDouble, topAsDouble, zNearAsDouble, zFarAsDouble);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glFrustumf(left, right, bottom, top, zNear, zFar);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glFrustumf);
}

void GL_APIENTRY glFrustumx(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
    SU_START_FUNCTION_WRAPPER(ap_glFrustumx);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glFrustumx, 6, OS_TOBJ_ID_GL_FIXED_PARAMETER, left, OS_TOBJ_ID_GL_FIXED_PARAMETER, right, OS_TOBJ_ID_GL_FIXED_PARAMETER, bottom, OS_TOBJ_ID_GL_FIXED_PARAMETER, top, OS_TOBJ_ID_GL_FIXED_PARAMETER, zNear, OS_TOBJ_ID_GL_FIXED_PARAMETER, zFar);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLdouble leftAsDouble = GLdouble(apGLfixedParameter::fixedToFloat(left));
    GLdouble rightAsDouble = GLdouble(apGLfixedParameter::fixedToFloat(right));
    GLdouble bottomAsDouble = GLdouble(apGLfixedParameter::fixedToFloat(bottom));
    GLdouble topAsDouble = GLdouble(apGLfixedParameter::fixedToFloat(top));
    GLdouble zNearAsDouble = GLdouble(apGLfixedParameter::fixedToFloat(zNear));
    GLdouble zFarAsDouble = GLdouble(apGLfixedParameter::fixedToFloat(zFar));

    // Call the real function:
    gs_stat_realFunctionPointers.glFrustum(leftAsDouble, rightAsDouble, bottomAsDouble, topAsDouble, zNearAsDouble, zFarAsDouble);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glFrustumx(left, right, bottom, top, zNear, zFar);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glFrustumx);
}

void GL_APIENTRY glGetClipPlanef(GLenum pname, GLfloat eqn[4])
{
    SU_START_FUNCTION_WRAPPER(ap_glGetClipPlanef);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetClipPlanef, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FLOAT_PARAMETER, eqn);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Call the real function:
    GLdouble equationAsDouble[4];
    gs_stat_realFunctionPointers.glGetClipPlane(pname, equationAsDouble);

    // Translate OpenGL ES parameters to OpenGL parameters:
    for (int i = 0; i < 4; i++)
    {
        eqn[i] = (GLfloat)(equationAsDouble[i]);
    }


#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glGetClipPlanef(pname, eqn);

#endif


    SU_END_FUNCTION_WRAPPER(ap_glGetClipPlanef);
}

void GL_APIENTRY glGetClipPlanex(GLenum pname, GLfixed eqn[4])
{
    SU_START_FUNCTION_WRAPPER(ap_glGetClipPlanex);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetClipPlanex, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FIXED_PARAMETER, eqn);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Call the real function:
    GLdouble equationAsDouble[4];
    gs_stat_realFunctionPointers.glGetClipPlane(pname, equationAsDouble);

    // Translate OpenGL ES parameters to OpenGL parameters:
    for (int i = 0; i < 4; i++)
    {
        eqn[i] = GLfixed(apGLfixedParameter::floatToFixed((GLfloat)(equationAsDouble[i])));
    }

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glGetClipPlanex(pname, eqn);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glGetClipPlanex);
}

void GL_APIENTRY glGetFixedv(GLenum pname, GLfixed* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetFixedv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetFixedv, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FIXED_PARAMETER, params);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // TO_DO: We have to handle: GL_MODELVIEW_MATRIX_FLOAT_AS_INT_BITS and GL_PROJECTION_MATRIX_FLOAT_AS_INT_BITS
    //        (Read and see what it means)

    // If we were asked for a parameter that is actually an OpenGL enumerator value:
    if ((pname == GL_ALPHA_TEST_FUNC) || (pname == GL_BLEND_DST) || (pname == GL_BLEND_SRC) ||
        (pname == GL_COLOR_ARRAY_TYPE) || (pname == GL_CULL_FACE) || (pname == GL_DEPTH_FUNC) ||
        (pname == GL_FOG_HINT) || (pname == GL_FOG_MODE) || (pname == GL_FRONT_FACE) ||
        (pname == GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES) || (pname == GL_IMPLEMENTATION_COLOR_READ_TYPE_OES) ||
        (pname == GL_LINE_SMOOTH_HINT) || (pname == GL_LOGIC_OP_MODE) || (pname == GL_MATRIX_INDEX_ARRAY_TYPE_OES) ||
        (pname == GL_MATRIX_MODE) || (pname == GL_NORMAL_ARRAY_TYPE) || (pname == GL_PERSPECTIVE_CORRECTION_HINT) ||
        (pname == GL_POINT_SIZE_ARRAY_TYPE_OES) || (pname == GL_POINT_SMOOTH_HINT) || (pname == GL_SHADE_MODEL) ||
        (pname == GL_STENCIL_FAIL) || (pname == GL_STENCIL_FUNC) || (pname == GL_STENCIL_PASS_DEPTH_FAIL) ||
        (pname == GL_STENCIL_PASS_DEPTH_PASS) || (pname == GL_STENCIL_VALUE_MASK) || (pname == GL_STENCIL_WRITEMASK) ||
        (pname == GL_TEXTURE_COORD_ARRAY_TYPE) || (pname == GL_VERTEX_ARRAY_TYPE) || (pname == GL_WEIGHT_ARRAY_TYPE_OES))
    {
        // Call the real function:
        GLint paramAsInt = 0;
        gs_stat_realFunctionPointers.glGetIntegerv(pname, &paramAsInt);

        // Output the queried parameter:
        *params = paramAsInt;
    }
    else
    {
        // Allocate space for the maximal returned size (which is a matrix):
        GLfloat paramsAsFloat[16];

        // Mark all space as "untouched" (using a "pseudo random" number):
        static GLfloat untouckedFloat = float(INT_MAX - 1274);

        for (int i = 0; i < 16; i++)
        {
            paramsAsFloat[i] = untouckedFloat;
        }

        // Call the real function:
        gs_stat_realFunctionPointers.glGetFloatv(pname, paramsAsFloat);

        // Translate OpenGL ES parameters to OpenGL parameters and output "touched" values:
        for (int i = 0; i < 16; i++)
        {
            // If the value was "touched":
            if (paramsAsFloat[i] != untouckedFloat)
            {
                params[i] = GLfixed(apGLfixedParameter::floatToFixed(paramsAsFloat[i]));
            }
            else
            {
                // Exit the loop:
                break;
            }
        }
    }

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glGetFixedv(pname, params);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glGetFixedv);
}

void GL_APIENTRY glGetLightxv(GLenum light, GLenum pname, GLfixed* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetLightxv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetLightxv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, light, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FIXED_PARAMETER, params);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Call the real function:
    GLfloat paramsAsFloat[4];
    gs_stat_realFunctionPointers.glGetLightfv(light, pname, paramsAsFloat);

    // Translate OpenGL parameters to OpenGL ES parameters and output them:
    int amountOfArrayElements = 1;

    if ((pname == GL_AMBIENT) || (pname == GL_DIFFUSE) || (pname == GL_SPECULAR) || (pname == GL_EMISSION))
    {
        amountOfArrayElements = 4;
    }
    else if (pname == GL_SPOT_DIRECTION)
    {
        amountOfArrayElements = 3;
    }

    for (int i = 0; i < amountOfArrayElements; i++)
    {
        params[i] = (GLfixed)apGLfixedParameter::floatToFixed(paramsAsFloat[i]);
    }

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glGetLightxv(light, pname, params);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glGetLightxv);
}

void GL_APIENTRY glGetMaterialxv(GLenum face, GLenum pname, GLfixed* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetMaterialxv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetMaterialxv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, face, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FIXED_PARAMETER, params);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Call the real function:
    GLfloat paramsAsFloat[4];
    gs_stat_realFunctionPointers.glGetMaterialfv(face, pname, paramsAsFloat);

    // Translate OpenGL parameters to OpenGL ES parameters and output them:
    int amountOfArrayElements = 1;

    if ((pname == GL_AMBIENT) || (pname == GL_DIFFUSE) || (pname == GL_SPECULAR) || (pname == GL_EMISSION))
    {
        amountOfArrayElements = 4;
    }

    for (int i = 0; i < amountOfArrayElements; i++)
    {
        params[i] = (GLfixed)apGLfixedParameter::floatToFixed(paramsAsFloat[i]);
    }

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glGetMaterialxv(face, pname, params);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glGetMaterialxv);
}

void GL_APIENTRY glGetTexEnvxv(GLenum target, GLenum pname, GLfixed* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetTexEnvxv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetTexEnvxv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FIXED_PARAMETER, params);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // If we need to output an enumerator:
    if (pname == GL_TEXTURE_ENV_MODE)
    {
        // Call the real function:
        GLint paramAsInt = 0;
        gs_stat_realFunctionPointers.glGetTexEnviv(target, pname, &paramAsInt);

        // Output the queried parameter:
        *params = paramAsInt;
    }
    else
    {
        // We need to output float parameters as fixed points:

        // Call the real function:
        GLfloat paramsAsFloat[4];
        gs_stat_realFunctionPointers.glGetTexEnvfv(target, pname, paramsAsFloat);

        // Translate OpenGL parameters to OpenGL ES parameters and output them:
        int amountOfArrayElements = 1;

        if (pname == GL_TEXTURE_ENV_COLOR)
        {
            amountOfArrayElements = 4;
        }

        for (int i = 0; i < amountOfArrayElements; i++)
        {
            params[i] = (GLfixed)apGLfixedParameter::floatToFixed(paramsAsFloat[i]);
        }
    }

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glGetTexEnvxv(target, pname, params);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glGetTexEnvxv);
}

void GL_APIENTRY glGetTexParameterxv(GLenum target, GLenum pname, GLfixed* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetTexParameterxv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetTexParameterxv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FIXED_PARAMETER, params);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // If we need to output an enumerator:
    if ((pname == GL_TEXTURE_MIN_FILTER) || (pname == GL_TEXTURE_MAG_FILTER) ||
        (pname == GL_TEXTURE_COMPARE_MODE) || (pname == GL_TEXTURE_COMPARE_FUNC) || (pname == GL_DEPTH_TEXTURE_MODE) ||
        (pname == GL_TEXTURE_WRAP_S) || (pname == GL_TEXTURE_WRAP_T) || (pname == GL_TEXTURE_WRAP_R))
    {
        // Call the real function:
        GLint paramAsInt = 0;
        gs_stat_realFunctionPointers.glGetTexParameteriv(target, pname, &paramAsInt);

        // Output the queried parameter:
        *params = paramAsInt;
    }
    else
    {
        // Call the real function:
        GLfloat paramsAsFloat[4];
        gs_stat_realFunctionPointers.glGetTexParameterfv(target, pname, paramsAsFloat);

        // Translate OpenGL parameters to OpenGL ES parameters and output them:
        int amountOfArrayElements = 1;

        if (pname == GL_TEXTURE_BORDER_COLOR)
        {
            amountOfArrayElements = 4;
        }

        for (int i = 0; i < amountOfArrayElements; i++)
        {
            params[i] = (GLfixed)apGLfixedParameter::floatToFixed(paramsAsFloat[i]);
        }
    }

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glGetTexParameterxv(target, pname, params);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glGetTexParameterxv);
}

void GL_APIENTRY glLightModelx(GLenum pname, GLfixed param)
{
    SU_START_FUNCTION_WRAPPER(ap_glLightModelx);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLightModelx, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_FIXED_PARAMETER, param);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLfloat paramAsGLfloat = GLfloat(apGLfixedParameter::fixedToFloat(param));

    // Call the real function:
    gs_stat_realFunctionPointers.glLightModelf(pname, paramAsGLfloat);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glLightModelx(pname, param);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glLightModelx);
}


void GL_APIENTRY glLightModelxv(GLenum pname, const GLfixed* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glLightModelxv);

    // Log the call to this function:

    GLfloat paramsAsFloat[4];
    int amountOfArrayElements = 1;

    if (pname == GL_LIGHT_MODEL_AMBIENT)
    {
        amountOfArrayElements = 4;

        // Translate OpenGL ES parameters to OpenGL parameters:
        for (int i = 0; i < 4; i++)
        {
            paramsAsFloat[i] = (GLfloat)apGLfixedParameter::fixedToFloat(params[i]);
        }
    }
    else
    {
        // Translate OpenGL ES parameters to OpenGL parameters:
        paramsAsFloat[0] = (GLfloat)apGLfixedParameter::fixedToFloat(*params);
    }

    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLightModelxv, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FIXED_PARAMETER, amountOfArrayElements, params);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    // Call the real function:
    gs_stat_realFunctionPointers.glLightModelfv(pname, paramsAsFloat);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glLightModelxv(pname, params);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glLightModelxv);
}

void GL_APIENTRY glLightx(GLenum light, GLenum pname, GLfixed param)
{
    SU_START_FUNCTION_WRAPPER(ap_glLightx);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLightx, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, light, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_FIXED_PARAMETER, param);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLfloat paramAsGLfloat = GLfloat(apGLfixedParameter::fixedToFloat(param));

    // Call the real function:
    gs_stat_realFunctionPointers.glLightf(light, pname, paramAsGLfloat);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glLightx(light, pname, param);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glLightx);
}


void GL_APIENTRY glLightxv(GLenum light, GLenum pname, const GLfixed* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glLightxv);

    // Log the call to this function:

    GLfloat paramsAsFloat[4];
    int amountOfVectorItems = 1;

    if ((pname == GL_AMBIENT) || (pname == GL_DIFFUSE) || (pname == GL_SPECULAR) ||
        (pname == GL_POSITION))
    {
        amountOfVectorItems = 4;

        // Translate OpenGL ES parameters to OpenGL parameters:
        for (int i = 0; i < 4; i++)
        {
            paramsAsFloat[i] = (GLfloat)apGLfixedParameter::fixedToFloat(params[i]);
        }
    }
    else if (pname == GL_SPOT_DIRECTION)
    {
        amountOfVectorItems = 3;

        // Translate OpenGL ES parameters to OpenGL parameters:
        for (int i = 0; i < 3; i++)
        {
            paramsAsFloat[i] = (GLfloat)apGLfixedParameter::fixedToFloat(params[i]);
        }
    }

    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLightxv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, light, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FIXED_PARAMETER, amountOfVectorItems, params);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    // Call the real function:
    gs_stat_realFunctionPointers.glLightfv(light, pname, paramsAsFloat);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glLightxv(light, pname, params);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glLightxv);
}

void GL_APIENTRY glLineWidthx(GLfixed width)
{
    SU_START_FUNCTION_WRAPPER(ap_glLineWidthx);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLineWidthx, 1, OS_TOBJ_ID_GL_FIXED_PARAMETER, width);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLfloat widthAsFloat = (GLfloat)apGLfixedParameter::fixedToFloat(width);

    // Call the real function:
    gs_stat_realFunctionPointers.glLineWidth(widthAsFloat);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glLineWidthx(width);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glLineWidthx);
}

void GL_APIENTRY glLoadMatrixx(const GLfixed* m)
{
    SU_START_FUNCTION_WRAPPER(ap_glLoadMatrixx);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glLoadMatrixx, 1, OS_TOBJ_ID_MATRIX_PARAMETER, OS_TOBJ_ID_GL_FIXED_PARAMETER, 4, 4, m);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLfloat mAsFloat[16];

    for (int i = 0; i < 16; i++)
    {
        mAsFloat[i] = apGLfixedParameter::fixedToFloat(m[i]);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glLoadMatrixf(mAsFloat);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glLoadMatrixx(m);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glLoadMatrixx);
}

void GL_APIENTRY glMaterialx(GLenum face, GLenum pname, GLfixed param)
{
    SU_START_FUNCTION_WRAPPER(ap_glMaterialx);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMaterialx, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, face, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_FIXED_PARAMETER, param);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLfloat paramAsFloat = (GLfloat)apGLfixedParameter::fixedToFloat(param);

    // Call the real function:
    gs_stat_realFunctionPointers.glMaterialf(face, pname, paramAsFloat);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glMaterialx(face, pname, param);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glMaterialx);
}


void GL_APIENTRY glMaterialxv(GLenum face, GLenum pname, const GLfixed* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glMaterialxv);

    // Log the call to this function:
    GLfloat paramsAsFloat[4];
    int amountOfArguments = 1;

    if ((pname == GL_AMBIENT) || (pname == GL_DIFFUSE) || (pname == GL_SPECULAR) ||
        (pname == GL_EMISSION) || (pname == GL_AMBIENT_AND_DIFFUSE))
    {
        amountOfArguments = 4;

        // Translate OpenGL ES parameters to OpenGL parameters:
        for (int i = 0; i < 4; i++)
        {
            paramsAsFloat[i] = (GLfloat)apGLfixedParameter::fixedToFloat(params[i]);
        }
    }
    else if (pname == GL_COLOR_INDEXES)
    {
        amountOfArguments = 3;

        // Translate OpenGL ES parameters to OpenGL parameters:
        for (int i = 0; i < 3; i++)
        {
            paramsAsFloat[i] = (GLfloat)apGLfixedParameter::fixedToFloat(params[i]);
        }
    }

    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMaterialxv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, face, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FIXED_PARAMETER, amountOfArguments, params);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    // Call the real function:
    gs_stat_realFunctionPointers.glMaterialfv(face, pname, paramsAsFloat);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glMaterialxv(face, pname, params);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glMaterialxv);
}

void GL_APIENTRY glMultMatrixx(const GLfixed* m)
{
    SU_START_FUNCTION_WRAPPER(ap_glMultMatrixx);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMultMatrixx, 1, OS_TOBJ_ID_MATRIX_PARAMETER, OS_TOBJ_ID_GL_FIXED_PARAMETER, 4, 4, m);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLfloat mAsFloat[16];

    for (int i = 0; i < 16; i++)
    {
        mAsFloat[i] = apGLfixedParameter::fixedToFloat(m[i]);
    }

    // Call the real function:
    gs_stat_realFunctionPointers.glMultMatrixf(mAsFloat);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glMultMatrixx(m);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glMultMatrixx);
}

void GL_APIENTRY glMultiTexCoord4x(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glMultiTexCoord4x);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glMultiTexCoord4x, 5, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_FIXED_PARAMETER, s, OS_TOBJ_ID_GL_FIXED_PARAMETER, t, OS_TOBJ_ID_GL_FIXED_PARAMETER, r, OS_TOBJ_ID_GL_FIXED_PARAMETER, q);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLfloat sAsFloat = (GLfloat)apGLfixedParameter::fixedToFloat(s);
    GLfloat tAsFloat = (GLfloat)apGLfixedParameter::fixedToFloat(t);
    GLfloat rAsFloat = (GLfloat)apGLfixedParameter::fixedToFloat(r);
    GLfloat qAsFloat = (GLfloat)apGLfixedParameter::fixedToFloat(q);

    // Mac OpenGL has no extensions, glMultiTexCoord4f is a base function there:
    // Get the real function pointer (its an extension function in OpenGL):
    static PFNGLMULTITEXCOORD4FPROC glMultiTexCoord4f = NULL;

    if (glMultiTexCoord4f == NULL)
    {
        glMultiTexCoord4f = (PFNGLMULTITEXCOORD4FPROC)(gs_stat_realFunctionPointers.wglGetProcAddress("glMultiTexCoord4f"));
    }

    GT_IF_WITH_ASSERT(glMultiTexCoord4f != NULL)
    {
        // Call the real function:
        glMultiTexCoord4f(target, sAsFloat, tAsFloat, rAsFloat, qAsFloat);
    }

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glMultiTexCoord4x(target, s, t, r, q);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glMultiTexCoord4x);
}

void GL_APIENTRY glNormal3x(GLfixed nx, GLfixed ny, GLfixed nz)
{
    SU_START_DRAW_FUNCTION_WRAPPER(ap_glNormal3x);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glNormal3x, 3, OS_TOBJ_ID_GL_FIXED_PARAMETER, nx, OS_TOBJ_ID_GL_FIXED_PARAMETER, ny, OS_TOBJ_ID_GL_FIXED_PARAMETER, nz);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLfloat nxAsFloat = (GLfloat)apGLfixedParameter::fixedToFloat(nx);
    GLfloat nyAsFloat = (GLfloat)apGLfixedParameter::fixedToFloat(ny);
    GLfloat nzAsFloat = (GLfloat)apGLfixedParameter::fixedToFloat(nz);

    // Call the real function:
    gs_stat_realFunctionPointers.glNormal3f(nxAsFloat, nyAsFloat, nzAsFloat);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glNormal3x(nx, ny, nz);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glNormal3x);
}

void GL_APIENTRY glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar)
{
    SU_START_FUNCTION_WRAPPER(ap_glOrthof);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glOrthof, 6, OS_TOBJ_ID_GL_FLOAT_PARAMETER, left, OS_TOBJ_ID_GL_FLOAT_PARAMETER, right, OS_TOBJ_ID_GL_FLOAT_PARAMETER, bottom, OS_TOBJ_ID_GL_FLOAT_PARAMETER, top, OS_TOBJ_ID_GL_FLOAT_PARAMETER, zNear, OS_TOBJ_ID_GL_FLOAT_PARAMETER, zFar);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLdouble leftAsDouble = GLdouble(left);
    GLdouble rightAsDouble = GLdouble(right);
    GLdouble bottomAsDouble = GLdouble(bottom);
    GLdouble topAsDouble = GLdouble(top);
    GLdouble zNearAsDouble = GLdouble(zNear);
    GLdouble zFarAsDouble = GLdouble(zFar);

    // Call the real function:
    gs_stat_realFunctionPointers.glOrtho(leftAsDouble, rightAsDouble, bottomAsDouble, topAsDouble, zNearAsDouble, zFarAsDouble);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glOrthof(left, right, bottom, top, zNear, zFar);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glOrthof);
}

void GL_APIENTRY glOrthox(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar)
{
    SU_START_FUNCTION_WRAPPER(ap_glOrthox);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glOrthox, 6, OS_TOBJ_ID_GL_FIXED_PARAMETER, left, OS_TOBJ_ID_GL_FIXED_PARAMETER, right, OS_TOBJ_ID_GL_FIXED_PARAMETER, bottom, OS_TOBJ_ID_GL_FIXED_PARAMETER, top, OS_TOBJ_ID_GL_FIXED_PARAMETER, zNear, OS_TOBJ_ID_GL_FIXED_PARAMETER, zFar);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLdouble leftAsDouble = GLdouble(apGLfixedParameter::fixedToFloat(left));
    GLdouble rightAsDouble = GLdouble(apGLfixedParameter::fixedToFloat(right));
    GLdouble bottomAsDouble = GLdouble(apGLfixedParameter::fixedToFloat(bottom));
    GLdouble topAsDouble = GLdouble(apGLfixedParameter::fixedToFloat(top));
    GLdouble zNearAsDouble = GLdouble(apGLfixedParameter::fixedToFloat(zNear));
    GLdouble zFarAsDouble = GLdouble(apGLfixedParameter::fixedToFloat(zFar));

    // Call the real function:
    gs_stat_realFunctionPointers.glOrtho(leftAsDouble, rightAsDouble, bottomAsDouble, topAsDouble, zNearAsDouble, zFarAsDouble);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glOrthox(left, right, bottom, top, zNear, zFar);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glOrthox);
}

void GL_APIENTRY glPointParameterx(GLenum pname, GLfixed param)
{
    SU_START_FUNCTION_WRAPPER(ap_glPointParameterx);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPointParameterx, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_FIXED_PARAMETER, param);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLfloat paramAsFloat = GLfloat(apGLfixedParameter::fixedToFloat(param));

    // Get the real function pointer (its an extension function in OpenGL):
    static PFNGLPOINTPARAMETERFPROC glPointParameterfPtr = NULL;

    if (glPointParameterfPtr == NULL)
    {
        glPointParameterfPtr = (PFNGLPOINTPARAMETERFPROC)(gs_stat_realFunctionPointers.wglGetProcAddress("glPointParameterf"));
    }

    GT_IF_WITH_ASSERT(glPointParameterfPtr != NULL)
    {
        // Call the real function:
        glPointParameterfPtr(pname, paramAsFloat);
    }

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glPointParameterx(pname, param);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glPointParameterx);
}

void GL_APIENTRY glPointParameterxv(GLenum pname, const GLfixed* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glPointParameterxv);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPointParameterxv, 2, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FIXED_PARAMETER, params);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLfloat paramsAsFloat[3];
    int amountOfArrayElements = 1;

    if (pname == GL_POINT_DISTANCE_ATTENUATION)
    {
        amountOfArrayElements = 3;
    }

    for (int i = 0; i < amountOfArrayElements; i++)
    {
        paramsAsFloat[i] = (GLfloat)apGLfixedParameter::fixedToFloat(params[i]);
    }


    // Get the real function pointer (its an extension function in OpenGL):
    static PFNGLPOINTPARAMETERFVPROC glPointParameterfvPtr = NULL;

    if (glPointParameterfvPtr == NULL)
    {
        glPointParameterfvPtr = (PFNGLPOINTPARAMETERFVPROC)(gs_stat_realFunctionPointers.wglGetProcAddress("glPointParameterfv"));
    }

    GT_IF_WITH_ASSERT(glPointParameterfvPtr != NULL)
    {
        // Call the real function:
        glPointParameterfvPtr(pname, paramsAsFloat);
    }

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glPointParameterxv(pname, params);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glPointParameterxv);
}

void GL_APIENTRY glPointSizex(GLfixed size)
{
    SU_START_FUNCTION_WRAPPER(ap_glPointSizex);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPointSizex, 1, OS_TOBJ_ID_GL_FIXED_PARAMETER, size);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLfloat sizeAsFloat = GLfloat(apGLfixedParameter::fixedToFloat(size));

    // Call the real function:
    gs_stat_realFunctionPointers.glPointSize(sizeAsFloat);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glPointSizex(size);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glPointSizex);
}

void GL_APIENTRY glPolygonOffsetx(GLfixed factor, GLfixed units)
{
    SU_START_FUNCTION_WRAPPER(ap_glPolygonOffsetx);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glPolygonOffsetx, 2, OS_TOBJ_ID_GL_FIXED_PARAMETER, factor, OS_TOBJ_ID_GL_FIXED_PARAMETER, units);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLfloat factorAsFloat = GLfloat(apGLfixedParameter::fixedToFloat(factor));
    GLfloat unitsAsFloat = GLfloat(apGLfixedParameter::fixedToFloat(units));

    // Call the real function:
    gs_stat_realFunctionPointers.glPolygonOffset(factorAsFloat, unitsAsFloat);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glPolygonOffsetx(factor, units);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glPolygonOffsetx);
}

void GL_APIENTRY glRotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z)
{
    SU_START_FUNCTION_WRAPPER(ap_glRotatex);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glRotatex, 4, OS_TOBJ_ID_GL_FIXED_PARAMETER, angle, OS_TOBJ_ID_GL_FIXED_PARAMETER, x, OS_TOBJ_ID_GL_FIXED_PARAMETER, y, OS_TOBJ_ID_GL_FIXED_PARAMETER, z);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLfloat angleAsFloat = GLfloat(apGLfixedParameter::fixedToFloat(angle));
    GLfloat xAsFloat = GLfloat(apGLfixedParameter::fixedToFloat(x));
    GLfloat yAsFloat = GLfloat(apGLfixedParameter::fixedToFloat(y));
    GLfloat zAsFloat = GLfloat(apGLfixedParameter::fixedToFloat(z));

    // Call the real function:
    gs_stat_realFunctionPointers.glRotatef(angleAsFloat, xAsFloat, yAsFloat, zAsFloat);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glRotatex(angle, x, y, z);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glRotatex);
}

void GL_APIENTRY glSampleCoveragex(GLclampx value, GLboolean invert)
{
    SU_START_FUNCTION_WRAPPER(ap_glSampleCoveragex);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glSampleCoveragex, 2, OS_TOBJ_ID_GL_CLAMPX_PARAMETER, value, OS_TOBJ_ID_GL_BOOL_PARAMETER, invert);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLclampf valueAsGLclampf = (GLclampf)apGLclampxParameter::clampxToFloat(value);

    // Get the real function pointer (its an extension function in OpenGL):
    static PFNGLSAMPLECOVERAGEPROC glSampleCoverage = NULL;

    if (glSampleCoverage == NULL)
    {
        glSampleCoverage = (PFNGLSAMPLECOVERAGEPROC)(gs_stat_realFunctionPointers.wglGetProcAddress("glSampleCoverage"));
    }

    GT_IF_WITH_ASSERT(glSampleCoverage != NULL)
    {
        // Call the real function:
        glSampleCoverage(valueAsGLclampf, invert);
    }

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glSampleCoveragex(value, invert);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glSampleCoveragex);
}

void GL_APIENTRY glScalex(GLfixed x, GLfixed y, GLfixed z)
{
    SU_START_FUNCTION_WRAPPER(ap_glScalex);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glScalex, 3, OS_TOBJ_ID_GL_FIXED_PARAMETER, x, OS_TOBJ_ID_GL_FIXED_PARAMETER, y, OS_TOBJ_ID_GL_FIXED_PARAMETER, z);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLfloat xAsFloat = GLfloat(apGLfixedParameter::fixedToFloat(x));
    GLfloat yAsFloat = GLfloat(apGLfixedParameter::fixedToFloat(y));
    GLfloat zAsFloat = GLfloat(apGLfixedParameter::fixedToFloat(z));

    // Call the real function:
    gs_stat_realFunctionPointers.glScalef(xAsFloat, yAsFloat, zAsFloat);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glScalex(x, y, z);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glScalex);
}

void GL_APIENTRY glTexEnvx(GLenum target, GLenum pname, GLfixed param)
{
    SU_START_FUNCTION_WRAPPER(ap_glTexEnvx);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexEnvx, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_FIXED_PARAMETER, param);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // If param is an enumerator:
    if ((target == GL_TEXTURE_ENV) && (pname == GL_TEXTURE_ENV_MODE))
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glTexEnvi(target, pname, param);
    }
    else
    {
        // param is a float represented as a fixed parameter:

        // Translate OpenGL ES parameters to OpenGL parameters:
        GLfloat paramAsFloat = GLfloat(apGLfixedParameter::fixedToFloat(param));

        // Call the real function:
        gs_stat_realFunctionPointers.glTexEnvf(target, pname, paramAsFloat);

    }

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glTexEnvx(target, pname, param);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glTexEnvx);
}


void GL_APIENTRY glTexEnvxv(GLenum target, GLenum pname, const GLfixed* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glTexEnvxv);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // If param is an enumerator:
    if ((target == GL_TEXTURE_ENV) && (pname == GL_TEXTURE_ENV_MODE))
    {
        GLint paramsAsInt[1];
        paramsAsInt[0] = (GLint)apGLfixedParameter::fixedToInt(params[0]);

        // Call the real function:
        gs_stat_realFunctionPointers.glTexEnviv(target, pname, paramsAsInt);
    }
    else
    {
        // Translate OpenGL ES parameters to OpenGL parameters:
        GLfloat paramsAsFloat[4];

        if (pname == GL_TEXTURE_ENV_COLOR)
        {
            for (int i = 0; i < 4; i++)
            {
                paramsAsFloat[i] = (GLfloat)apGLfixedParameter::fixedToFloat(params[i]);
            }
        }
        else
        {
            paramsAsFloat[0] = (GLfloat)apGLfixedParameter::fixedToFloat(*params);
        }

        // Log the call to this function:
        if (pname == GL_TEXTURE_ENV_MODE)
        {
            GLenum paramAsEnum = (GLenum)(paramsAsFloat[0]);

            gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexEnvxv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_ENUM_PARAMETER, paramAsEnum);
        }
        else if (pname == GL_TEXTURE_ENV_COLOR)
        {
            gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexEnvxv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FIXED_PARAMETER, 4, params);
        }
        else
        {
            gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexEnvxv, 3, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FIXED_PARAMETER, params);
        }

        // Call the real function:
        gs_stat_realFunctionPointers.glTexEnvfv(target, pname, paramsAsFloat);
    }

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glTexEnvxv(target, pname, params);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glTexEnvxv);
}

void GL_APIENTRY glTexParameterx(GLenum target, GLenum pname, GLfixed param)
{
    SU_START_FUNCTION_WRAPPER(ap_glTexParameterx);

    // Check if stub textures are forced:
    bool areStubTexForced = false;
    gsRenderContextMonitor* pCurrentThreadRenderContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();

    if (pCurrentThreadRenderContextMonitor)
    {
        areStubTexForced = pCurrentThreadRenderContextMonitor->forcedModesManager().isStubForced(AP_OPENGL_FORCED_STUB_TEXTURES_MODE);
    }

    // Get the input bind target bounded texture name:
    GLuint boundTextureName = gs_stat_openGLMonitorInstance.boundTexture(target);

    // Translate the input parameter to a GLenum:
    // (Currently, in OpenGL ES, param represents only enumerator values (and not float values):
    GLenum paramAsEnum = (GLenum)param;

    // Log the call to this function:
    if ((pname == GL_TEXTURE_MIN_FILTER) || (pname == GL_TEXTURE_MAG_FILTER) ||
        (pname == GL_TEXTURE_COMPARE_MODE) || (pname == GL_TEXTURE_COMPARE_FUNC) || (pname == GL_DEPTH_TEXTURE_MODE) ||
        (pname == GL_TEXTURE_WRAP_S) || (pname == GL_TEXTURE_WRAP_T) || (pname == GL_TEXTURE_WRAP_R))
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexParameterx, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_ENUM_PARAMETER, paramAsEnum, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);
    }
    else
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexParameterx, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_FIXED_PARAMETER, param, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);
    }

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // If we are not in "Force stub textures" mode:
    if (!areStubTexForced)
    {
        // Call the real function:
        gs_stat_realFunctionPointers.glTexParameteri(target, pname, paramAsEnum);
    }

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    if (!areStubTexForced)
    {

        // Call the real function:
        gs_stat_realFunctionPointers.glTexParameterx(target, pname, param);
    }

#endif

    // Log the new parameter value:
    if (pCurrentThreadRenderContextMonitor)
    {
        gsTexturesMonitor* texturesMon = pCurrentThreadRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(texturesMon != NULL)
        {
            bool rcTexParami = pCurrentThreadRenderContextMonitor->onTextureIntParameterChanged(target, pname, (const GLint*)(&paramAsEnum));
            GT_ASSERT(rcTexParami);
        }

        // Log the mipmap auto generation:
        if (pname == GL_GENERATE_MIPMAP)
        {
            // Get the param value:
            GLint valueAsInt = (GLint)param;
            bool rcMipmapGeneration = pCurrentThreadRenderContextMonitor->onMipmapGenerateParamSet(target, pname, (GLfloat)valueAsInt);
            GT_ASSERT(rcMipmapGeneration);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_glTexParameterx);
}


void GL_APIENTRY glTexParameterxv(GLenum target, GLenum pname, const GLfixed* params)
{
    SU_START_FUNCTION_WRAPPER(ap_glTexParameterxv);

    // Get the input bind target bounded texture name:
    GLuint boundTextureName = gs_stat_openGLMonitorInstance.boundTexture(target);

    // Translate the input parameter to a GLenum:
    // (Currently, in OpenGL ES, param represents only enumerator values (and not float values):
    GLenum paramAsEnum = (GLenum)(params[0]);

    // Log the call to this function:
    if ((pname == GL_TEXTURE_MIN_FILTER) || (pname == GL_TEXTURE_MAG_FILTER) ||
        (pname == GL_TEXTURE_COMPARE_MODE) || (pname == GL_TEXTURE_COMPARE_FUNC) || (pname == GL_DEPTH_TEXTURE_MODE) ||
        (pname == GL_TEXTURE_WRAP_S) || (pname == GL_TEXTURE_WRAP_T) || (pname == GL_TEXTURE_WRAP_R))
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexParameterxv, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_ENUM_PARAMETER, paramAsEnum, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);
    }
    else if (pname == GL_TEXTURE_CROP_RECT_OES)
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexParameteriv, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_VECTOR_PARAMETER, OS_TOBJ_ID_GL_FIXED_PARAMETER, 4, params, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);
    }
    else
    {
        gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTexParameterxv, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, target, OS_TOBJ_ID_GL_ENUM_PARAMETER, pname, OS_TOBJ_ID_GL_P_FIXED_PARAMETER, params, OS_TOBJ_ID_ASSOCIATED_TEXTURE_NAMES_PSEUDO_PARAMETER, 1, &boundTextureName);
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

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

        // If the parameter is the crop parameter of the OES_draw_texture extension:
        if (pname == GL_TEXTURE_CROP_RECT_OES)
        {
            GLfloat paramAsFloat[4];
            paramAsFloat[0] = apGLfixedParameter::fixedToFloat(params[0]);
            paramAsFloat[1] = apGLfixedParameter::fixedToFloat(params[1]);
            paramAsFloat[2] = apGLfixedParameter::fixedToFloat(params[2]);
            paramAsFloat[3] = apGLfixedParameter::fixedToFloat(params[3]);

            // Set the texture crop rectangle:
            if (pCurrentThreadRenderContextMonitor)
            {
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
            gs_stat_realFunctionPointers.glTexParameteri(target, pname, paramAsEnum);
        }

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

        // Call the real function:
        gs_stat_realFunctionPointers.glTexParameterxv(target, pname, params);

#endif
    }

    // Log the new parameter valueAsInt:
    if (pCurrentThreadRenderContextMonitor)
    {
        gsTexturesMonitor* texturesMon = pCurrentThreadRenderContextMonitor->texturesMonitor();
        GT_IF_WITH_ASSERT(texturesMon != NULL)
        {
            bool rcTexParami = pCurrentThreadRenderContextMonitor->onTextureIntParameterChanged(target, pname, (const GLint*)(&paramAsEnum));
            GT_ASSERT(rcTexParami);
        }

        // Log the mipmap auto generation:
        if (pname == GL_GENERATE_MIPMAP)
        {
            // Get the param value:
            GLint valueAsInt = (GLint) * params;
            bool rcMipmapGeneration = pCurrentThreadRenderContextMonitor->onMipmapGenerateParamSet(target, pname, (GLfloat)valueAsInt);
            GT_ASSERT(rcMipmapGeneration);
        }
    }

    SU_END_FUNCTION_WRAPPER(ap_glTexParameterxv);
}

void GL_APIENTRY glTranslatex(GLfixed x, GLfixed y, GLfixed z)
{
    SU_START_FUNCTION_WRAPPER(ap_glTranslatex);

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glTranslatex, 3, OS_TOBJ_ID_GL_FIXED_PARAMETER, x, OS_TOBJ_ID_GL_FIXED_PARAMETER, y, OS_TOBJ_ID_GL_FIXED_PARAMETER, z);

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    // Translate OpenGL ES parameters to OpenGL parameters:
    GLfloat xAsFloat = GLfloat(apGLfixedParameter::fixedToFloat(x));
    GLfloat yAsFloat = GLfloat(apGLfixedParameter::fixedToFloat(y));
    GLfloat zAsFloat = GLfloat(apGLfixedParameter::fixedToFloat(z));

    // Call the real function:
    gs_stat_realFunctionPointers.glTranslatef(xAsFloat, yAsFloat, zAsFloat);

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

    // Call the real function:
    gs_stat_realFunctionPointers.glTranslatex(x, y, z);

#endif

    SU_END_FUNCTION_WRAPPER(ap_glTranslatex);
}

//////////////////////////////////////////////////////////////////////////
// OpenGL ES 2.0
//////////////////////////////////////////////////////////////////////////
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
void GL_APIENTRY glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
    SU_START_FUNCTION_WRAPPER(ap_glGetShaderPrecisionFormat)

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glGetShaderPrecisionFormat, 4, OS_TOBJ_ID_GL_ENUM_PARAMETER, shadertype, OS_TOBJ_ID_GL_ENUM_PARAMETER, precisiontype, OS_TOBJ_ID_GL_P_INT_PARAMETER, range, OS_TOBJ_ID_GL_P_INT_PARAMETER, precision);

    // Call the real function:
    gs_stat_realFunctionPointers.glGetShaderPrecisionFormat(shadertype, precisiontype, range, precision);

    SU_END_FUNCTION_WRAPPER(ap_glGetShaderPrecisionFormat)
}

void GL_APIENTRY glReleaseShaderCompiler(void)
{
    SU_START_FUNCTION_WRAPPER(ap_glReleaseShaderCompiler)

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glReleaseShaderCompiler, 0);

    // Call the real function:
    gs_stat_realFunctionPointers.glReleaseShaderCompiler();

    SU_END_FUNCTION_WRAPPER(ap_glReleaseShaderCompiler)
}

void GL_APIENTRY glShaderBinary(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length)
{
    SU_START_FUNCTION_WRAPPER(ap_glShaderBinary)

    // Log the call to this function:
    gs_stat_openGLMonitorInstance.addFunctionCall(ap_glShaderBinary, 5, OS_TOBJ_ID_GL_SIZEI_PARAMETER, n, OS_TOBJ_ID_GL_P_UINT_PARAMETER, shaders, OS_TOBJ_ID_GL_ENUM_PARAMETER, binaryformat, OS_TOBJ_ID_GL_P_VOID_PARAMETER, binary, OS_TOBJ_ID_GL_SIZEI_PARAMETER, length);

    // Call the real function:
    gs_stat_realFunctionPointers.glShaderBinary(n, shaders, binaryformat, binary, length);

    SU_END_FUNCTION_WRAPPER(ap_glShaderBinary)
}

#endif



/////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// GL_
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// GL_
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// GL_
//////////////////////////////////////////////////////////////////////////

