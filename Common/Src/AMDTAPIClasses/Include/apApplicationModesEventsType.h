//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apApplicationModesEventsType.h
///
//==================================================================================

//------------------------------ apApplicationModesEventsType.h ------------------------------

#ifndef __APAPPLICATIONMODESEVENTSTYPE
#define __APAPPLICATIONMODESEVENTSTYPE

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>

// Define the OpenGL force modes events:
enum apOpenGLForcedModeType
{
    AP_OPENGL_FORCED_STUB_UNKNOWN = -1,
    AP_OPENGL_FORCED_FRONT_DRAW_BUFFER_MODE = 0,
    AP_OPENGL_FORCED_POLYGON_RASTER_MODE,
    AP_OPENGL_FORCED_STUB_TEXTURES_MODE,
    AP_OPENGL_FORCED_SINGLE_PIXEL_VIEW_PORT,
    AP_OPENGL_FORCED_STUB_FRAGMENT_SHADERS_MODE,
    AP_OPENGL_FORCED_NO_LIGHTS_MODE,
    AP_OPENGL_FORCED_STUB_GEOMETRY_SHADERS_MODE,
    AP_OPENGL_DEBUG_OUTPUT_PARAMETER_CHANGED,
    AP_OPENGL_AMOUNT_OF_FORCED_STUBS
};

enum apOpenCLExecutionType
{
    AP_OPENCL_EXECUTION_UNKNOWN = -1,
    AP_OPENCL_KERNEL_EXECUTION = 0,
    AP_OPENCL_READ_OPERATION_EXECUTION,
    AP_OPENCL_WRITE_OPERATION_EXECUTION,
    AP_OPENCL_COPY_OPERATION_EXECUTION,
    AP_OPENCL_AMOUNT_OF_EXECUTIONS
};

// String translation functions:
AP_API gtString apOpenGLForcedModeTypeAsString(apOpenGLForcedModeType forceModeType);
AP_API gtString apOpenCLExecutionTypeAsString(apOpenCLExecutionType openCLExecutionType);

#endif  // __APAPPLICATIONMODESEVENTSTYPE
