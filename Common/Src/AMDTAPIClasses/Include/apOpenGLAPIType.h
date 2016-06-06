//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenGLAPIType.h
///
//==================================================================================

//------------------------------ apOpenGLAPIType.h ------------------------------

#ifndef __APOPENGLAPITYPE
#define __APOPENGLAPITYPE

// Describes a state variable type:
// (A function type is a bitwise collection of the below values)
typedef enum
{
    AP_OPENGL_STATE_VAR =       0x00000001,     // OpenGL state variable.
    AP_OPENGL_ES_1_STATE_VAR =  0x00000002,     // OpenGL ES 1.1 state variable.
    AP_OPENGL_ES_2_STATE_VAR =  0x00000004,     // OpenGL ES 2.0 state variable.
    AP_CGL_STATE_VAR =          0x00000008,     // CGL state variable.
    AP_WGL_STATE_VAR =          0x00000010,     // WGL state variable.
} apOpenGLAPIType;


#endif  // __APOPENGLAPITYPE
