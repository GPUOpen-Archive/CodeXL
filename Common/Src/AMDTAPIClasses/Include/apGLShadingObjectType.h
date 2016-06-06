//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLShadingObjectType.h
///
//==================================================================================

//------------------------------ apGLShadingObjectType.h ------------------------------

#ifndef __APGLSHADINGOBJECTTYPE_H
#define __APGLSHADINGOBJECTTYPE_H

// ----------------------------------------------------------------------------------
// Class Name:          apGLShadingObjectType
// General Description: Signifies whether a program or shader was created with the
//                      ARB_shader_objects extension or with the OpenGL 2.0+ core
//                      functions.
// Author:  AMD Developer Tools Team
// Creation Date:       4/2/2010
// ----------------------------------------------------------------------------------
enum apGLShadingObjectType
{
    AP_GL_ARB_SHADER_OBJECTS_EXTENSION_SHADING_OBJECT,
    // A shader object created using the GL_ARB_shader_objects extension.
    // (Using glCreateShaderObjectARB).
    // A program created using the GL_ARB_shader_objects extension.
    // (Using glCreateProgramObjectARB).
    AP_GL_2_0_SHADING_OBJECT,
    // A shader object created using OpenGL 2.0 standard
    // (Using glCreateShader).
    // A program created using OpenGL 2.0 standard
    // (Using glCreateProgram).
};

#endif //__APGLSHADINGOBJECTTYPE_H

