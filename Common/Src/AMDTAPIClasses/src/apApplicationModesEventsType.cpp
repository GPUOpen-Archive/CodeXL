//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apApplicationModesEventsType.cpp
///
//==================================================================================

//------------------------------ apApplicationModesEventsType.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTAPIClasses/Include/apApplicationModesEventsType.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>


// ---------------------------------------------------------------------------
// Name:        apOpenGLForcedModeTypeAsString
// Description: Translate an OpenGL forced mode type to a string
// Arguments:   apOpenGLForcedModeType forceModeType
// Return Val:  gtString
// Author:  AMD Developer Tools Team
// Date:        9/5/2010
// ---------------------------------------------------------------------------
gtString apOpenGLForcedModeTypeAsString(apOpenGLForcedModeType forceModeType)
{
    gtString retVal;

    switch (forceModeType)
    {
        case AP_OPENGL_FORCED_STUB_UNKNOWN:
        {
            retVal = AP_STR_Unknown;
            break;
        }

        case AP_OPENGL_FORCED_FRONT_DRAW_BUFFER_MODE:
        {
            retVal = AP_STR_OpenGLStubDrawBufferMode;
            break;
        }

        case AP_OPENGL_FORCED_POLYGON_RASTER_MODE:
        {
            retVal = AP_STR_OpenGLStubPolygonRasterMode;
            break;
        }

        case AP_OPENGL_FORCED_STUB_TEXTURES_MODE:
        {
            retVal = AP_STR_OpenGLStubTexturesMode;
            break;
        }

        case AP_OPENGL_FORCED_SINGLE_PIXEL_VIEW_PORT:
        {
            retVal = AP_STR_OpenGLStubSingleViewPortMode;
            break;
        }

        case AP_OPENGL_FORCED_STUB_FRAGMENT_SHADERS_MODE:
        {
            retVal = AP_STR_OpenGLStubFragmentShadersMode;
            break;
        }

        case AP_OPENGL_FORCED_STUB_GEOMETRY_SHADERS_MODE:
        {
            retVal = AP_STR_OpenGLStubGeometryShadersMode;
            break;
        }

        case AP_OPENGL_FORCED_NO_LIGHTS_MODE:
        {
            retVal = AP_STR_OpenGLStubNoLightsMode;
            break;
        }

        case AP_OPENGL_DEBUG_OUTPUT_PARAMETER_CHANGED:
        {
            retVal = AP_STR_Unknown;
            GT_ASSERT_EX(false, L"Should not get here");
            break;
        }


        default:
        {
            retVal = AP_STR_Unknown;
            GT_ASSERT_EX(false, L"Unknown OpenGL stub type");
            break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apOpenCLExecutionTypeAsString
// Description: Translate an OpenCL operation to a string
// Arguments:   apOpenCLExecutionType openCLExecutionType
// Return Val:  gtString
// Author:  AMD Developer Tools Team
// Date:        9/5/2010
// ---------------------------------------------------------------------------
gtString apOpenCLExecutionTypeAsString(apOpenCLExecutionType openCLExecutionType)
{
    gtString retVal;

    switch (openCLExecutionType)
    {
        case AP_OPENCL_KERNEL_EXECUTION:
        {
            retVal = AP_STR_OpenCLKernelExecution;
            break;
        }

        case AP_OPENCL_READ_OPERATION_EXECUTION:
        {
            retVal = AP_STR_OpenCLReadOperationsExecution;
            break;
        }


        case AP_OPENCL_WRITE_OPERATION_EXECUTION:
        {
            retVal = AP_STR_OpenCLWriteOperationsExecution;
            break;
        }

        case AP_OPENCL_COPY_OPERATION_EXECUTION:
        {
            retVal = AP_STR_OpenCLCopyOperationsExecution;
            break;
        }

        default:
        {
            retVal = AP_STR_Unknown;
            GT_ASSERT_EX(false, L"Unknown OpenCL operation");
            break;
        }
    }

    return retVal;
}

