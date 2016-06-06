//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLDebugOutput.cpp
///
//==================================================================================

//------------------------------ apGLDebugOutput.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/apGLDebugOutput.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osChannelOperators.h>


// ---------------------------------------------------------------------------
// Name:        apDebugOutputSeverityToGL43Enum
// Description: Helper function to "dereference" arrays of flags:
// Author:  AMD Developer Tools Team
// Date:        26/6/2014
// ---------------------------------------------------------------------------
const bool& apGLDebugOutputKindFromFlagArray(const bool* aFlags, apGLDebugOutputSource source, apGLDebugOutputType type)
{
    return aFlags[type + (AP_NUMBER_OF_DEBUG_OUTPUT_TYPES * source)];
}

// ---------------------------------------------------------------------------
// Name:        apGLDebugOutputKindFromMutableFlagArray
// Description: Helper function to "dereference" arrays of flags:
// Author:  AMD Developer Tools Team
// Date:        26/6/2014
// ---------------------------------------------------------------------------
bool& apGLDebugOutputKindFromMutableFlagArray(bool* aFlags, apGLDebugOutputSource source, apGLDebugOutputType type)
{
    return aFlags[type + (AP_NUMBER_OF_DEBUG_OUTPUT_TYPES * source)];
}

// ---------------------------------------------------------------------------
// Name:        apDebugOutputSeverityToGL43Enum
// Description: Translates debug output severity to a GLenum (KHR / 4.3 version)
// Author:  AMD Developer Tools Team
// Date:        26/6/2014
// ---------------------------------------------------------------------------
GLenum apDebugOutputSeverityToGL43Enum(apGLDebugOutputSeverity severity)
{
    GLenum retVal = GL_NONE;

    switch (severity)
    {
        case AP_GL_DEBUG_OUTPUT_SEVERITY_LOW:
            retVal = GL_DEBUG_SEVERITY_LOW;
            break;

        case AP_GL_DEBUG_OUTPUT_SEVERITY_MEDIUM:
            retVal = GL_DEBUG_SEVERITY_MEDIUM;
            break;

        case AP_GL_DEBUG_OUTPUT_SEVERITY_HIGH:
            retVal = GL_DEBUG_SEVERITY_HIGH;
            break;

        case AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES:
        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebugOutputSourceToGL43Enum
// Description: Translates debug output source to a GLenum (KHR / 4.3 version)
// Author:  AMD Developer Tools Team
// Date:        26/6/2014
// ---------------------------------------------------------------------------
GLenum apDebugOutputSourceToGL43Enum(apGLDebugOutputSource source)
{
    GLenum retVal = GL_NONE;

    switch (source)
    {
        case AP_GL_DEBUG_OUTPUT_SOURCE_API:
            retVal = GL_DEBUG_SOURCE_API;
            break;

        case AP_GL_DEBUG_OUTPUT_SOURCE_WINDOW_SYSTEM:
            retVal = GL_DEBUG_SOURCE_WINDOW_SYSTEM;
            break;

        case AP_GL_DEBUG_OUTPUT_SOURCE_SHADER_COMPILER:
            retVal = GL_DEBUG_SOURCE_SHADER_COMPILER;
            break;

        case AP_GL_DEBUG_OUTPUT_SOURCE_THIRD_PARTY:
            retVal = GL_DEBUG_SOURCE_THIRD_PARTY;
            break;

        case AP_GL_DEBUG_OUTPUT_SOURCE_APPLICATION:
            retVal = GL_DEBUG_SOURCE_APPLICATION;
            break;

        case AP_GL_DEBUG_OUTPUT_SOURCE_OTHER:
            retVal = GL_DEBUG_SOURCE_OTHER;
            break;

        case AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES:
        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebugOutputTypeToGL43Enum
// Description: Translates debug output type to a GLenum (KHR / 4.3 version)
// Author:  AMD Developer Tools Team
// Date:        26/6/2014
// ---------------------------------------------------------------------------
GLenum apDebugOutputTypeToGL43Enum(apGLDebugOutputType type)
{
    GLenum retVal = GL_NONE;

    switch (type)
    {
        case AP_GL_DEBUG_OUTPUT_TYPE_ERROR:
            retVal = GL_DEBUG_TYPE_ERROR;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_DEPRECATED_BEHAVIOR:
            retVal = GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_UNDEFINED_BEHAVIOR:
            retVal = GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_PORTABILITY:
            retVal = GL_DEBUG_TYPE_PORTABILITY;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_PERFORMANCE:
            retVal = GL_DEBUG_TYPE_PERFORMANCE;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_OTHER:
            retVal = GL_DEBUG_TYPE_OTHER;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_MARKER:
            retVal = GL_DEBUG_TYPE_MARKER;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_PUSH_GROUP:
            retVal = GL_DEBUG_TYPE_PUSH_GROUP;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_POP_GROUP:
            retVal = GL_DEBUG_TYPE_POP_GROUP;
            break;

        case AP_NUMBER_OF_DEBUG_OUTPUT_TYPES:
        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebugOutputSeverityFromGL43Enum
// Description: Translates debug output severity from a GLenum (KHR / 4.3 version)
// Author:  AMD Developer Tools Team
// Date:        30/6/2014
// ---------------------------------------------------------------------------
apGLDebugOutputSeverity apDebugOutputSeverityFromGL43Enum(GLenum severity)
{
    apGLDebugOutputSeverity retVal = AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES;

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_LOW:
            retVal = AP_GL_DEBUG_OUTPUT_SEVERITY_LOW;
            break;

        case GL_DEBUG_SEVERITY_MEDIUM:
            retVal = AP_GL_DEBUG_OUTPUT_SEVERITY_MEDIUM;
            break;

        case GL_DEBUG_SEVERITY_HIGH:
            retVal = AP_GL_DEBUG_OUTPUT_SEVERITY_HIGH;
            break;

        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebugOutputSourceFromGL43Enum
// Description: Translates debug output source from a GLenum (KHR / 4.3 version)
// Author:  AMD Developer Tools Team
// Date:        30/6/2014
// ---------------------------------------------------------------------------
apGLDebugOutputSource apDebugOutputSourceFromGL43Enum(GLenum source)
{
    apGLDebugOutputSource retVal = AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:
            retVal = AP_GL_DEBUG_OUTPUT_SOURCE_API;
            break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
            retVal = AP_GL_DEBUG_OUTPUT_SOURCE_WINDOW_SYSTEM;
            break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER:
            retVal = AP_GL_DEBUG_OUTPUT_SOURCE_SHADER_COMPILER;
            break;

        case GL_DEBUG_SOURCE_THIRD_PARTY:
            retVal = AP_GL_DEBUG_OUTPUT_SOURCE_THIRD_PARTY;
            break;

        case GL_DEBUG_SOURCE_APPLICATION:
            retVal = AP_GL_DEBUG_OUTPUT_SOURCE_APPLICATION;
            break;

        case GL_DEBUG_SOURCE_OTHER:
            retVal = AP_GL_DEBUG_OUTPUT_SOURCE_OTHER;
            break;

        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebugOutputTypeFromGL43Enum
// Description: Translates debug output type from a GLenum (KHR / 4.3 version)
// Author:  AMD Developer Tools Team
// Date:        26/6/2014
// ---------------------------------------------------------------------------
apGLDebugOutputType apDebugOutputTypeFromGL43Enum(GLenum type)
{
    apGLDebugOutputType retVal = AP_NUMBER_OF_DEBUG_OUTPUT_TYPES;

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:
            retVal = AP_GL_DEBUG_OUTPUT_TYPE_ERROR;
            break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
            retVal = AP_GL_DEBUG_OUTPUT_TYPE_DEPRECATED_BEHAVIOR;
            break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
            retVal = AP_GL_DEBUG_OUTPUT_TYPE_UNDEFINED_BEHAVIOR;
            break;

        case GL_DEBUG_TYPE_PORTABILITY:
            retVal = AP_GL_DEBUG_OUTPUT_TYPE_PORTABILITY;
            break;

        case GL_DEBUG_TYPE_PERFORMANCE:
            retVal = AP_GL_DEBUG_OUTPUT_TYPE_PERFORMANCE;
            break;

        case GL_DEBUG_TYPE_OTHER:
            retVal = AP_GL_DEBUG_OUTPUT_TYPE_OTHER;
            break;

        case GL_DEBUG_TYPE_MARKER:
            retVal = AP_GL_DEBUG_OUTPUT_TYPE_MARKER;
            break;

        case GL_DEBUG_TYPE_PUSH_GROUP:
            retVal = AP_GL_DEBUG_OUTPUT_TYPE_PUSH_GROUP;
            break;

        case GL_DEBUG_TYPE_POP_GROUP:
            retVal = AP_GL_DEBUG_OUTPUT_TYPE_POP_GROUP;
            break;

        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebugOutputSeverityToGLARBEnum
// Description: Translates debug output severity to a GLenum (ARB version)
// Author:  AMD Developer Tools Team
// Date:        26/6/2014
// ---------------------------------------------------------------------------
GLenum apDebugOutputSeverityToGLARBEnum(apGLDebugOutputSeverity severity)
{
    GLenum retVal = GL_NONE;

    switch (severity)
    {
        case AP_GL_DEBUG_OUTPUT_SEVERITY_LOW:
            retVal = GL_DEBUG_SEVERITY_LOW_ARB;
            break;

        case AP_GL_DEBUG_OUTPUT_SEVERITY_MEDIUM:
            retVal = GL_DEBUG_SEVERITY_MEDIUM_ARB;
            break;

        case AP_GL_DEBUG_OUTPUT_SEVERITY_HIGH:
            retVal = GL_DEBUG_SEVERITY_HIGH_ARB;
            break;

        case AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES:
        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebugOutputSourceToGLARBEnum
// Description: Translates debug output source to a GLenum (ARB version)
// Author:  AMD Developer Tools Team
// Date:        26/6/2014
// ---------------------------------------------------------------------------
GLenum apDebugOutputSourceToGLARBEnum(apGLDebugOutputSource source)
{
    GLenum retVal = GL_NONE;

    switch (source)
    {
        case AP_GL_DEBUG_OUTPUT_SOURCE_API:
            retVal = GL_DEBUG_SOURCE_API_ARB;
            break;

        case AP_GL_DEBUG_OUTPUT_SOURCE_WINDOW_SYSTEM:
            retVal = GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB;
            break;

        case AP_GL_DEBUG_OUTPUT_SOURCE_SHADER_COMPILER:
            retVal = GL_DEBUG_SOURCE_SHADER_COMPILER_ARB;
            break;

        case AP_GL_DEBUG_OUTPUT_SOURCE_THIRD_PARTY:
            retVal = GL_DEBUG_SOURCE_THIRD_PARTY_ARB;
            break;

        case AP_GL_DEBUG_OUTPUT_SOURCE_APPLICATION:
            retVal = GL_DEBUG_SOURCE_APPLICATION_ARB;
            break;

        case AP_GL_DEBUG_OUTPUT_SOURCE_OTHER:
            retVal = GL_DEBUG_SOURCE_OTHER_ARB;
            break;

        case AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES:
        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebugOutputTypeToGLARBEnum
// Description: Translates debug output type to a GLenum (ARB version)
// Author:  AMD Developer Tools Team
// Date:        26/6/2014
// ---------------------------------------------------------------------------
GLenum apDebugOutputTypeToGLARBEnum(apGLDebugOutputType type)
{
    GLenum retVal = GL_NONE;

    switch (type)
    {
        case AP_GL_DEBUG_OUTPUT_TYPE_ERROR:
            retVal = GL_DEBUG_TYPE_ERROR_ARB;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_DEPRECATED_BEHAVIOR:
            retVal = GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_UNDEFINED_BEHAVIOR:
            retVal = GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_PORTABILITY:
            retVal = GL_DEBUG_TYPE_PORTABILITY_ARB;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_PERFORMANCE:
            retVal = GL_DEBUG_TYPE_PERFORMANCE_ARB;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_OTHER:
            retVal = GL_DEBUG_TYPE_OTHER_ARB;
            break;

        case AP_NUMBER_OF_DEBUG_OUTPUT_TYPES:
        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebugOutputSeverityFromGLARBEnum
// Description: Translates debug output severity from a GLenum (ARB version)
// Author:  AMD Developer Tools Team
// Date:        30/6/2014
// ---------------------------------------------------------------------------
apGLDebugOutputSeverity apDebugOutputSeverityFromGLARBEnum(GLenum severity)
{
    apGLDebugOutputSeverity retVal = AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES;

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_LOW_ARB:
            retVal = AP_GL_DEBUG_OUTPUT_SEVERITY_LOW;
            break;

        case GL_DEBUG_SEVERITY_MEDIUM_ARB:
            retVal = AP_GL_DEBUG_OUTPUT_SEVERITY_MEDIUM;
            break;

        case GL_DEBUG_SEVERITY_HIGH_ARB:
            retVal = AP_GL_DEBUG_OUTPUT_SEVERITY_HIGH;
            break;

        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebugOutputSourceFromGLARBEnum
// Description: Translates debug output source from a GLenum (ARB version)
// Author:  AMD Developer Tools Team
// Date:        30/6/2014
// ---------------------------------------------------------------------------
apGLDebugOutputSource apDebugOutputSourceFromGLARBEnum(GLenum source)
{
    apGLDebugOutputSource retVal = AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API_ARB:
            retVal = AP_GL_DEBUG_OUTPUT_SOURCE_API;
            break;

        case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
            retVal = AP_GL_DEBUG_OUTPUT_SOURCE_WINDOW_SYSTEM;
            break;

        case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
            retVal = AP_GL_DEBUG_OUTPUT_SOURCE_SHADER_COMPILER;
            break;

        case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
            retVal = AP_GL_DEBUG_OUTPUT_SOURCE_THIRD_PARTY;
            break;

        case GL_DEBUG_SOURCE_APPLICATION_ARB:
            retVal = AP_GL_DEBUG_OUTPUT_SOURCE_APPLICATION;
            break;

        case GL_DEBUG_SOURCE_OTHER_ARB:
            retVal = AP_GL_DEBUG_OUTPUT_SOURCE_OTHER;
            break;

        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebugOutputTypeFromGLARBEnum
// Description: Translates debug output type from a GLenum (ARB version)
// Author:  AMD Developer Tools Team
// Date:        26/6/2014
// ---------------------------------------------------------------------------
apGLDebugOutputType apDebugOutputTypeFromGLARBEnum(GLenum type)
{
    apGLDebugOutputType retVal = AP_NUMBER_OF_DEBUG_OUTPUT_TYPES;

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR_ARB:
            retVal = AP_GL_DEBUG_OUTPUT_TYPE_ERROR;
            break;

        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
            retVal = AP_GL_DEBUG_OUTPUT_TYPE_DEPRECATED_BEHAVIOR;
            break;

        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
            retVal = AP_GL_DEBUG_OUTPUT_TYPE_UNDEFINED_BEHAVIOR;
            break;

        case GL_DEBUG_TYPE_PORTABILITY_ARB:
            retVal = AP_GL_DEBUG_OUTPUT_TYPE_PORTABILITY;
            break;

        case GL_DEBUG_TYPE_PERFORMANCE_ARB:
            retVal = AP_GL_DEBUG_OUTPUT_TYPE_PERFORMANCE;
            break;

        case GL_DEBUG_TYPE_OTHER_ARB:
            retVal = AP_GL_DEBUG_OUTPUT_TYPE_OTHER;
            break;

        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebugOutputSeverityToGLAMDEnum
// Description: Translates debug output severity to a GLenum (AMD version)
// Author:  AMD Developer Tools Team
// Date:        26/6/2014
// ---------------------------------------------------------------------------
GLenum apDebugOutputSeverityToGLAMDEnum(apGLDebugOutputSeverity severity)
{
    GLenum retVal = GL_NONE;

    switch (severity)
    {
        case AP_GL_DEBUG_OUTPUT_SEVERITY_LOW:
            retVal = GL_DEBUG_SEVERITY_LOW_AMD;
            break;

        case AP_GL_DEBUG_OUTPUT_SEVERITY_MEDIUM:
            retVal = GL_DEBUG_SEVERITY_MEDIUM_AMD;
            break;

        case AP_GL_DEBUG_OUTPUT_SEVERITY_HIGH:
            retVal = GL_DEBUG_SEVERITY_HIGH_AMD;
            break;

        case AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES:
        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebugOutputCategoryToGLAMDEnum
// Description: Translates debug output category to a GLenum (AMD version)
// Author:  AMD Developer Tools Team
// Date:        26/6/2014
// ---------------------------------------------------------------------------
GLenum apDebugOutputCategoryToGLAMDEnum(apGLDebugOutputCategory category)
{
    GLenum retVal = GL_NONE;

    switch (category)
    {
        case AP_DEBUG_OUTPUT_CATEGORY_API_ERROR:
            retVal = GL_DEBUG_CATEGORY_API_ERROR_AMD;
            break;

        case AP_DEBUG_OUTPUT_CATEGORY_WINDOW_SYSTEM:
            retVal = GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD;
            break;

        case AP_DEBUG_OUTPUT_CATEGORY_DEPRECATION:
            retVal = GL_DEBUG_CATEGORY_DEPRECATION_AMD;
            break;

        case AP_DEBUG_OUTPUT_CATEGORY_UNDEFINED_BEHAVIOR:
            retVal = GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD;
            break;

        case AP_DEBUG_OUTPUT_CATEGORY_PERFORMANCE:
            retVal = GL_DEBUG_CATEGORY_PERFORMANCE_AMD;
            break;

        case AP_DEBUG_OUTPUT_CATEGORY_SHADER_COMPILER:
            retVal = GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD;
            break;

        case AP_DEBUG_OUTPUT_CATEGORY_APPLICATION:
            retVal = GL_DEBUG_CATEGORY_APPLICATION_AMD;
            break;

        case AP_DEBUG_OUTPUT_CATEGORY_OTHER:
            retVal = GL_DEBUG_CATEGORY_OTHER_AMD;
            break;

        case AP_NUMBER_OF_DEBUG_OUTPUT_CATEGORIES:
        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebugOutputSeverityFromGLAMDEnum
// Description: Translates debug output severity from a GLenum (AMD version)
// Author:  AMD Developer Tools Team
// Date:        30/6/2014
// ---------------------------------------------------------------------------
apGLDebugOutputSeverity apDebugOutputSeverityFromGLAMDEnum(GLenum severity)
{
    apGLDebugOutputSeverity retVal = AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES;

    switch (severity)
    {
        case GL_DEBUG_SEVERITY_LOW_AMD:
            retVal = AP_GL_DEBUG_OUTPUT_SEVERITY_LOW;
            break;

        case GL_DEBUG_SEVERITY_MEDIUM_AMD:
            retVal = AP_GL_DEBUG_OUTPUT_SEVERITY_MEDIUM;
            break;

        case GL_DEBUG_SEVERITY_HIGH_AMD:
            retVal = AP_GL_DEBUG_OUTPUT_SEVERITY_HIGH;
            break;

        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebugOutputCategoryFromGLAMDEnum
// Description: Translates debug output category from a GLenum (AMD version)
// Author:  AMD Developer Tools Team
// Date:        26/6/2014
// ---------------------------------------------------------------------------
apGLDebugOutputCategory apDebugOutputCategoryFromGLAMDEnum(GLenum category)
{
    apGLDebugOutputCategory retVal = AP_NUMBER_OF_DEBUG_OUTPUT_CATEGORIES;

    switch (category)
    {
        case GL_DEBUG_CATEGORY_API_ERROR_AMD:
            retVal = AP_DEBUG_OUTPUT_CATEGORY_API_ERROR;
            break;

        case GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD:
            retVal = AP_DEBUG_OUTPUT_CATEGORY_WINDOW_SYSTEM;
            break;

        case GL_DEBUG_CATEGORY_DEPRECATION_AMD:
            retVal = AP_DEBUG_OUTPUT_CATEGORY_DEPRECATION;
            break;

        case GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD:
            retVal = AP_DEBUG_OUTPUT_CATEGORY_UNDEFINED_BEHAVIOR;
            break;

        case GL_DEBUG_CATEGORY_PERFORMANCE_AMD:
            retVal = AP_DEBUG_OUTPUT_CATEGORY_PERFORMANCE;
            break;

        case GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD:
            retVal = AP_DEBUG_OUTPUT_CATEGORY_SHADER_COMPILER;
            break;

        case GL_DEBUG_CATEGORY_APPLICATION_AMD:
            retVal = AP_DEBUG_OUTPUT_CATEGORY_APPLICATION;
            break;

        case GL_DEBUG_CATEGORY_OTHER_AMD:
            retVal = AP_DEBUG_OUTPUT_CATEGORY_OTHER;
            break;

        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apDebugOutputCategoryToGLAMDEnum
// Description: Translates debug output category to a combination
//              of source + type and checks if any of those flags are raised
// Author:  AMD Developer Tools Team
// Date:        26/6/2014
// ---------------------------------------------------------------------------
bool apIsDebugOutputCategoryFlagged(const bool* aFlags, apGLDebugOutputCategory category)
{
    bool retVal = false;

    apGLDebugOutputSource checkSource = AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES;
    apGLDebugOutputType checkType = AP_NUMBER_OF_DEBUG_OUTPUT_TYPES;

    switch (category)
    {
        case AP_DEBUG_OUTPUT_CATEGORY_API_ERROR:
            retVal = apGLDebugOutputKindFromFlagArray(aFlags, AP_GL_DEBUG_OUTPUT_SOURCE_API, AP_GL_DEBUG_OUTPUT_TYPE_ERROR);
            break;

        case AP_DEBUG_OUTPUT_CATEGORY_WINDOW_SYSTEM:
            checkSource = AP_GL_DEBUG_OUTPUT_SOURCE_WINDOW_SYSTEM;
            break;

        case AP_DEBUG_OUTPUT_CATEGORY_DEPRECATION:
            checkType = AP_GL_DEBUG_OUTPUT_TYPE_DEPRECATED_BEHAVIOR;
            break;

        case AP_DEBUG_OUTPUT_CATEGORY_UNDEFINED_BEHAVIOR:
            checkType = AP_GL_DEBUG_OUTPUT_TYPE_UNDEFINED_BEHAVIOR;
            break;

        case AP_DEBUG_OUTPUT_CATEGORY_PERFORMANCE:
            checkType = AP_GL_DEBUG_OUTPUT_TYPE_PERFORMANCE;
            break;

        case AP_DEBUG_OUTPUT_CATEGORY_SHADER_COMPILER:
            checkSource = AP_GL_DEBUG_OUTPUT_SOURCE_SHADER_COMPILER;
            break;

        case AP_DEBUG_OUTPUT_CATEGORY_APPLICATION:
            checkSource = AP_GL_DEBUG_OUTPUT_SOURCE_APPLICATION;
            break;

        case AP_DEBUG_OUTPUT_CATEGORY_OTHER:
            checkSource = AP_GL_DEBUG_OUTPUT_SOURCE_OTHER;
            checkType = AP_GL_DEBUG_OUTPUT_TYPE_OTHER;
            break;

        case AP_NUMBER_OF_DEBUG_OUTPUT_CATEGORIES:
        default:
            // Unexpected value!
            GT_ASSERT(false);
            break;
    }

    if (!retVal)
    {
        // If specified, check all types for a source:
        if (AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES != checkSource)
        {
            for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_TYPES; i++)
            {
                if (apGLDebugOutputKindFromFlagArray(aFlags, checkSource, (apGLDebugOutputType)i))
                {
                    retVal = true;
                    break;
                }
            }
        }

        if (!retVal)
        {
            // If specified, check all sources for a type:
            if (AP_NUMBER_OF_DEBUG_OUTPUT_TYPES != checkType)
            {
                for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES; i++)
                {
                    if (apGLDebugOutputKindFromFlagArray(aFlags, (apGLDebugOutputSource)i, checkType))
                    {
                        retVal = true;
                        break;
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLDebugOutputSourceAsString
// Description: Translate OpenGL debug output source enumeration to a string
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/6/2014
// ---------------------------------------------------------------------------
bool AP_API apGLDebugOutputSourceAsString(apGLDebugOutputSource source, gtString& sourceAsString)
{
    bool retVal = true;

    switch (source)
    {
        case AP_GL_DEBUG_OUTPUT_SOURCE_API:
            sourceAsString = AP_STR_OpenGLDebugOutputSourceAPI;
            break;

        case AP_GL_DEBUG_OUTPUT_SOURCE_WINDOW_SYSTEM:
            sourceAsString = AP_STR_OpenGLDebugOutputSourceWindowSystem;
            break;

        case AP_GL_DEBUG_OUTPUT_SOURCE_SHADER_COMPILER:
            sourceAsString = AP_STR_OpenGLDebugOutputSourceShaderCompiler;
            break;

        case AP_GL_DEBUG_OUTPUT_SOURCE_THIRD_PARTY:
            sourceAsString = AP_STR_OpenGLDebugOutputSourceThirdParty;
            break;

        case AP_GL_DEBUG_OUTPUT_SOURCE_APPLICATION:
            sourceAsString = AP_STR_OpenGLDebugOutputSourceApplication;
            break;

        case AP_GL_DEBUG_OUTPUT_SOURCE_OTHER:
            sourceAsString = AP_STR_OpenGLDebugOutputSourceOther;
            break;

        case AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES:
        default:
            sourceAsString = AP_STR_Unknown;
            GT_ASSERT(false);
            retVal = false;
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLDebugOutputTypeAsString
// Description: Translate OpenGL debug output type enumeration to a string
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/6/2014
// ---------------------------------------------------------------------------
bool AP_API apGLDebugOutputTypeAsString(apGLDebugOutputType type, gtString& typeAsString, bool shortString)
{
    bool retVal = true;

    switch (type)
    {
        case AP_GL_DEBUG_OUTPUT_TYPE_ERROR:
            typeAsString = AP_STR_OpenGLDebugOutputTypeError;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_DEPRECATED_BEHAVIOR:
            typeAsString = shortString ? AP_STR_OpenGLDebugOutputTypeDeprecatedBehaviorShort : AP_STR_OpenGLDebugOutputTypeDeprecatedBehavior;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_UNDEFINED_BEHAVIOR:
            typeAsString = shortString ? AP_STR_OpenGLDebugOutputTypeUndefinedBehaviorShort : AP_STR_OpenGLDebugOutputTypeUndefinedBehavior;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_PORTABILITY:
            typeAsString = AP_STR_OpenGLDebugOutputTypePortability;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_PERFORMANCE:
            typeAsString = AP_STR_OpenGLDebugOutputTypePerformance;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_OTHER:
            typeAsString = AP_STR_OpenGLDebugOutputTypeOther;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_MARKER:
            typeAsString = AP_STR_OpenGLDebugOutputTypeMarker;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_PUSH_GROUP:
            typeAsString = AP_STR_OpenGLDebugOutputTypePushGroup;
            break;

        case AP_GL_DEBUG_OUTPUT_TYPE_POP_GROUP:
            typeAsString = AP_STR_OpenGLDebugOutputTypePopGroup;
            break;

        case AP_NUMBER_OF_DEBUG_OUTPUT_TYPES:
        default:
            typeAsString = AP_STR_Unknown;
            retVal = false;
            GT_ASSERT(false);
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
// Name:        apGLDebugOutputSeverityFromString
// Description: Severity from string
// Arguments:   const gtString& severityAsStr
// Return Val:  apGLDebugOutputSeverity
// Author:  AMD Developer Tools Team
// Date:        13/6/2010
// ---------------------------------------------------------------------------
apGLDebugOutputSeverity apGLDebugOutputSeverityFromString(const gtString& severityAsStr)
{
    apGLDebugOutputSeverity retVal = AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES;

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

// ---------------------------------------------------------------------------
// Name:        apGLDebugOutputSeveritiesAsString
// Description: Severity list to string
// Author:  AMD Developer Tools Team
// Date:        29/6/2014
// ---------------------------------------------------------------------------
void apGLDebugOutputSeveritiesAsString(const bool severities[AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES], gtString& severitiesString)
{
    bool firstActive = true;

    // Initialize the output string:
    severitiesString.makeEmpty();

    // Check each severity:
    for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES; i++)
    {
        // If it is enabled,
        if (severities[i])
        {
            // Translate it to a string
            gtString currentSeverityAsString;
            bool rcNm = apGLDebugOutputSeverityAsString((apGLDebugOutputSeverity)i, currentSeverityAsString);
            GT_IF_WITH_ASSERT(rcNm)
            {
                // And add it to the output string:
                if (firstActive)
                {
                    firstActive = false;
                }
                else
                {
                    severitiesString.append(',');
                }

                severitiesString.append(currentSeverityAsString);
            }
        }
    }

    // If none were found, set the NA string:
    if (firstActive)
    {
        severitiesString = AP_STR_NotAvailable;
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLDebugOutputSeveritiesFromString
// Description: Severity list from string
// Author:  AMD Developer Tools Team
// Date:        29/6/2014
// ---------------------------------------------------------------------------
bool apGLDebugOutputSeveritiesFromString(const gtString& severitiesString, bool severities[AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES])
{
    bool retVal = true;

    // Initialize the output array:
    for (int i = 0; i < AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES; i++)
    {
        severities[i] = false;
    }

    // The N/A string is considered a success:
    if (AP_STR_NotAvailable != severitiesString)
    {
        // Tokenize the string
        gtStringTokenizer strTokenizer(severitiesString, L", ");
        gtString currentToken;

        while (strTokenizer.getNextToken(currentToken))
        {
            if (!currentToken.isEmpty())
            {
                // Translate the current token to a severity:
                apGLDebugOutputSeverity sev = apGLDebugOutputSeverityFromString(currentToken);

                if (AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES != sev)
                {
                    severities[sev] = true;
                }
                else
                {
                    // Unexpected value!
                    retVal = false;
                }
            }
        }
    }


    return retVal;
}


