//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLDebugOutput.cpp
///
//==================================================================================

//------------------------------ apGLDebugOutput.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/Events/apGLDebugOutput.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>

// ---------------------------------------------------------------------------
// Name:        apGLDebugOutputCategoryMaskAsString
// Description: Translates debug output category mask to a string
// Arguments:   apGLDebugOutputCategoryMask categoryMask
//              gtString& categoryAsString
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        7/6/2010
// ---------------------------------------------------------------------------
bool apGLDebugOutputCategoryMaskAsString(apGLDebugOutputCategoryMask categoryMask, gtString& categoryAsString)
{
    bool retVal = true;

    categoryAsString.makeEmpty();

    switch (categoryMask)
    {
        case AP_GL_DEBUG_OUTPUT_API_ERROR_CATEGORY:
            categoryAsString = AP_STR_OpenGLDebugOutputAPIError;
            break;

        case AP_GL_DEBUG_OUTPUT_WINDOW_SYSTEM_CATEGORY:
            categoryAsString = AP_STR_OpenGLDebugOutputWindowSystem;
            break;

        case AP_GL_DEBUG_OUTPUT_DEPRECATION_CATEGORY:
            categoryAsString = AP_STR_OpenGLDebugOutputDeprecation;
            break;

        case AP_GL_DEBUG_OUTPUT_UNDEFINED_BEHAVIOR_CATEGORY:
            categoryAsString = AP_STR_OpenGLDebugOutputUndefinedBehavior;
            break;

        case AP_GL_DEBUG_OUTPUT_PERFORMANCE_CATEGORY:
            categoryAsString = AP_STR_OpenGLDebugOutputPerformance;
            break;

        case AP_GL_DEBUG_OUTPUT_SHADER_COMPILER_CATEGORY:
            categoryAsString = AP_STR_OpenGLDebugOutputShaderCompiler;
            break;

        case AP_GL_DEBUG_OUTPUT_APPLICATION_CATEGORY:
            categoryAsString = AP_STR_OpenGLDebugOutputApplication;
            break;

        case AP_GL_DEBUG_OUTPUT_OTHER_CATEGORY:
            categoryAsString = AP_STR_OpenGLDebugOutputOther;
            break;

        default:
            categoryAsString = AP_STR_Unknown;
            retVal = false;
            break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLDebugOutputCategotyMaskFromEnum
// Description: Translate OpenGL category enumeration to our enumeration
// Arguments:   GLenum glCategory
//              apDebugOutputCategory& category
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        10/6/2010
// ---------------------------------------------------------------------------
bool apGLDebugOutputCategotyMaskFromEnum(GLenum glCategory, apGLDebugOutputCategoryMask& category)
{
    bool retVal = true;

    switch (glCategory)
    {
        case GL_DEBUG_CATEGORY_API_ERROR_AMD:
            category = AP_GL_DEBUG_OUTPUT_API_ERROR_CATEGORY;
            break;

        case GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD:
            category = AP_GL_DEBUG_OUTPUT_WINDOW_SYSTEM_CATEGORY;
            break;

        case GL_DEBUG_CATEGORY_DEPRECATION_AMD:
            category = AP_GL_DEBUG_OUTPUT_DEPRECATION_CATEGORY;
            break;

        case GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD:
            category = AP_GL_DEBUG_OUTPUT_UNDEFINED_BEHAVIOR_CATEGORY;
            break;

        case GL_DEBUG_CATEGORY_PERFORMANCE_AMD:
            category = AP_GL_DEBUG_OUTPUT_PERFORMANCE_CATEGORY;
            break;

        case GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD:
            category = AP_GL_DEBUG_OUTPUT_SHADER_COMPILER_CATEGORY;
            break;

        case GL_DEBUG_CATEGORY_APPLICATION_AMD:
            category = AP_GL_DEBUG_OUTPUT_APPLICATION_CATEGORY;
            break;

        case GL_DEBUG_CATEGORY_OTHER_AMD:
            category = AP_GL_DEBUG_OUTPUT_OTHER_CATEGORY;
            break;

        default:
            category = AP_GL_DEBUG_OUTPUT_UNKNOWN_CATEGORY;
            retVal = false;
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLDebugOutputSeverityAsString
// Description: Translate OpenGL debug output severity enumeration to a string
// Arguments:   apGLDebugOutputSeverity severity
//              gtString& severityAsString
// Return Val:  bool AP_API  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        13/6/2010
// ---------------------------------------------------------------------------
bool AP_API apGLDebugOutputSeverityAsString(apGLDebugOutputSeverity severity, gtString& severityAsString)
{
    bool retVal = true;

    severityAsString.makeEmpty();

    switch (severity)
    {
        case AP_GL_DEBUG_OUTPUT_SEVERITY_LOW:
            severityAsString = AP_STR_Low;
            break;

        case AP_GL_DEBUG_OUTPUT_SEVERITY_MEDIUM:
            severityAsString = AP_STR_Medium;
            break;

        case AP_GL_DEBUG_OUTPUT_SEVERITY_HIGH:
            severityAsString = AP_STR_High;
            break;

        default:
            severityAsString = AP_STR_Unknown;
            retVal = false;
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLDebugOutputSeverityFromEnum
// Description: Translate OpenGL severity enumeration to our enumeration
// Arguments:   GLenum glSeverity
//              apGLDebugOutputSeverity& severity
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        10/6/2010
// ---------------------------------------------------------------------------
bool apGLDebugOutputSeverityFromEnum(GLenum glSeverity, apGLDebugOutputSeverity& severity)
{
    bool retVal = true;

    switch (glSeverity)
    {
        case GL_DEBUG_SEVERITY_HIGH_AMD:
            severity = AP_GL_DEBUG_OUTPUT_SEVERITY_HIGH;
            break;

        case GL_DEBUG_SEVERITY_MEDIUM_AMD:
            severity = AP_GL_DEBUG_OUTPUT_SEVERITY_MEDIUM;
            break;

        case GL_DEBUG_SEVERITY_LOW_AMD:
            severity = AP_GL_DEBUG_OUTPUT_SEVERITY_LOW;
            break;

        default:
            severity = AP_GL_DEBUG_OUTPUT_SEVERITY_UNKNOWN;
            retVal = false;
            break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLDebugOutputSeverityFromString
// Description: Severity from string
// Arguments:   const gtString& severityAsStr
// Return Val:  apGLDebugOutputSeverity
// Author:  AMD Developer Tools Team
// Date:        13/6/2010
// ---------------------------------------------------------------------------
apGLDebugOutputSeverity apGLDebugOutputSeverityFromString(const gtString& severityAsStr)
{
    apGLDebugOutputSeverity retVal = AP_GL_DEBUG_OUTPUT_SEVERITY_UNKNOWN;

    if (severityAsStr == AP_STR_High)
    {
        retVal = AP_GL_DEBUG_OUTPUT_SEVERITY_HIGH;
    }
    else if (severityAsStr == AP_STR_Medium)
    {
        retVal = AP_GL_DEBUG_OUTPUT_SEVERITY_MEDIUM;
    }
    else if (severityAsStr == AP_STR_Low)
    {
        retVal = AP_GL_DEBUG_OUTPUT_SEVERITY_LOW;
    }

    return retVal;
}

