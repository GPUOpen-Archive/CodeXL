//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apFunctionType.h
///
//==================================================================================

//------------------------------ apFunctionType.h ------------------------------

#ifndef __APFUNCTIONTYPE
#define __APFUNCTIONTYPE

// Describes a API type:
typedef enum
{
    AP_OPENGL_GENERIC_FUNC              =   0x00000001,     // Generic OpenGL function.
    AP_OPENGL_ES_GENERIC_FUNC           =   0x00000002,     // Generic OpenGLES function.
    AP_OPENGL_ES_1_MAC_GENERIC_FUNC     =   0x00000004,     // Mac OpenGL ES function.
    AP_OPENGL_ES_2_MAC_GENERIC_FUNC     =   0x00000008,     // Mac OpenGL ES 2.0 function
    AP_OPENGL_EXTENSION_FUNC            =   0x00000010,     // OpenGL extension function.
    AP_OPENGL_ES_EXTENSION_FUNC         =   0x00000020,     // OpenGLES extension function.
    AP_OPENGL_ES_MAC_EXTENSION_FUNC     =   0x00000040,     // Mac OpenGLES extension function.
    AP_OPENGL_GREMEDY_EXTENSION_FUNC    =   0x00000080,     // OpenGL GREMEDY extension function.
    AP_WGL_FUNC                         =   0x00000100,     // WGL function.
    AP_WGL_EXTENSION_FUNC               =   0x00000200,     // WGL extension function.
    AP_GLX_FUNC                         =   0x00000400,     // GLX function (Same enum as WGL extension function).
    AP_GLX_EXTENSION_FUNC               =   0x00000800,     // GLX extension function.
    AP_CGL_FUNC                         =   0x00001000,     // CGL function.
    AP_EGL_FUNC                         =   0x00002000,     // EGL function.
    AP_EGL_EXTENSION_FUNC               =   0x00004000,     // EGL extension function.

    AP_OPENGL_API_FUNC_MASK             =   0x00007FFF,     // All of the above.

    AP_OPENCL_GENERIC_FUNC              =   0x00008000,     // OpenCL generic function.
    AP_OPENCL_EXTENSION_FUNC            =   0x00010000,     // OpenCL extension function.
    AP_OPENCL_AMD_EXTENSION_FUNC        =   0x00020000,     // Graphic AMD OpenCL extension function.

    AP_OPENCL_API_FUNC_MASK             =   0x00038000,     // All of the above.

    AP_LAST_API_TYPE                    =   0x00040000

} apAPIType;

// Describes a function type:
// (A function type is a bitwise collection of the below values)
typedef enum
{
    AP_DRAW_FUNC                    =   0x00000001,     // (GL) Draw related function that has a visible effect (is also a AP_DRAW_FUNC).
    AP_RASTER_FUNC                  =   0x00000002,     // (GL) Raster related function.
    AP_GET_FUNC                     =   0x00000004,     // (GL/CL) Get function.
    AP_STATE_CHANGE_FUNC            =   0x00000008,     // (GL) State variables changing function.
    AP_PROGRAM_SHADER_FUNC          =   0x00000010,     // (GL) Program and Shaders operation function.
    AP_PROGRAM_KERNEL_FUNC          =   0x00000020,     // (CL) Program and Kernels operation function.
    AP_TEXTURE_FUNC                 =   0x00000040,     // (GL) Texture related function.
    AP_BUFFER_FUNC                  =   0x00000080,     // (GL) Buffer related function.
    AP_MATRIX_FUNC                  =   0x00000100,     // (GL) Matrix operation related function.
    AP_NAME_FUNC                    =   0x00000200,     // (GL) Name stack related function.
    AP_QUERY_FUNC                   =   0x00000400,     // (GL) Query related function.
    AP_BUFFER_IMAGE_FUNC            =   0x00000800,     // (CL) Buffer and Image related function.
    AP_DEPRECATED_FUNC              =   0x00001000,     // (GL/CL) Deprecated function.
    AP_QUEUE_FUNC                   =   0x00002000,     // (CL) queue related function.
    AP_SYNCHRONIZATION_FUNC         =   0x00004000,     // (GL/CL) Synchronization related function.
    AP_FEEDBACK_FUNC                =   0x00008000,     // (GL) Transform feedback related function.
    AP_VERTEX_ARRAY_FUNC            =   0x00010000,     // (GL) Transform feedback related function.
    AP_DEBUG_FUNC                   =   0x00020000,     // (GL/CL) Function related to debugging.
    AP_NULL_CONTEXT_FUNCTION        =   0x00040000,     // (GL/CL) Functions that can be run be no context.

    AP_LAST_FUNCTION_TYPE           =   0x00080000

} apFunctionType;


typedef enum
{
    AP_REDUNDANCY_UNKNOWN                       = 0,            // Function call redundancy status is not calculated
    AP_REDUNDANCY_REDUNDANT                     = 1,            // Function call redundancy calculated, and found redundant
    AP_REDUNDANCY_NOT_REDUNDANT                 = 2             // Function call redundancy calculated, and found not redundant
} apFunctionRedundancyStatus;


#endif  // __APFUNCTIONTYPE
