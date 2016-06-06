//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLDebugOutput.h
///
//==================================================================================

//------------------------------ apGLDebugOutput.h ------------------------------

#ifndef __APGLDEBUGOUTPUT_H
#define __APGLDEBUGOUTPUT_H

// Infra:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTBaseTools/Include/gtString.h>

// Forward declarations:
template <class TransferableObjectType> class osTransferableObjectCreator;

// Local:
#include <AMDTAPIClasses/Include/Events/apEvent.h>

// ----------------------------------------------------------------------------------
// Enumeration:         apGLDebugOutputSource
// General Description: Is used to describe debug output source (ARB / KHR / 4.3 version).
// Author:  AMD Developer Tools Team
// Creation Date:        26/6/2014
// ----------------------------------------------------------------------------------
enum apGLDebugOutputSource
{
    AP_GL_DEBUG_OUTPUT_SOURCE_API,
    AP_GL_DEBUG_OUTPUT_SOURCE_WINDOW_SYSTEM,
    AP_GL_DEBUG_OUTPUT_SOURCE_SHADER_COMPILER,
    AP_GL_DEBUG_OUTPUT_SOURCE_THIRD_PARTY,
    AP_GL_DEBUG_OUTPUT_SOURCE_APPLICATION,
    AP_GL_DEBUG_OUTPUT_SOURCE_OTHER,
    AP_NUMBER_OF_DEBUG_OUTPUT_SOURCES
};

// ----------------------------------------------------------------------------------
// Enumeration:         apGLDebugOutputType
// General Description: Is used to describe debug output type (ARB / KHR / 4.3 version).
// Author:  AMD Developer Tools Team
// Creation Date:        26/6/2014
// ----------------------------------------------------------------------------------
enum apGLDebugOutputType
{
    AP_GL_DEBUG_OUTPUT_TYPE_ERROR,
    AP_GL_DEBUG_OUTPUT_TYPE_DEPRECATED_BEHAVIOR,
    AP_GL_DEBUG_OUTPUT_TYPE_UNDEFINED_BEHAVIOR,
    AP_GL_DEBUG_OUTPUT_TYPE_PORTABILITY,
    AP_GL_DEBUG_OUTPUT_TYPE_PERFORMANCE,
    AP_GL_DEBUG_OUTPUT_TYPE_OTHER,
    AP_GL_ARB_DEBUG_OUTPUT_LAST_TYPE = AP_GL_DEBUG_OUTPUT_TYPE_OTHER,
    AP_GL_DEBUG_OUTPUT_TYPE_MARKER,
    AP_NUMBER_OF_USER_VISIBLE_DEBUG_OUTPUT_TYPES = AP_GL_DEBUG_OUTPUT_TYPE_MARKER,
    // The last two types are to be used in the spy only:
    AP_GL_DEBUG_OUTPUT_TYPE_PUSH_GROUP,
    AP_GL_DEBUG_OUTPUT_TYPE_POP_GROUP,
    AP_NUMBER_OF_DEBUG_OUTPUT_TYPES
};

// ----------------------------------------------------------------------------------
// Enumeration:         apGLDebugOutputCategory
// General Description: Is used to describe debug output category (AMD version).
// Author:  AMD Developer Tools Team
// Creation Date:        26/6/2014
// ----------------------------------------------------------------------------------
enum apGLDebugOutputCategory
{
    AP_DEBUG_OUTPUT_CATEGORY_API_ERROR,
    AP_DEBUG_OUTPUT_CATEGORY_WINDOW_SYSTEM,
    AP_DEBUG_OUTPUT_CATEGORY_DEPRECATION,
    AP_DEBUG_OUTPUT_CATEGORY_UNDEFINED_BEHAVIOR,
    AP_DEBUG_OUTPUT_CATEGORY_PERFORMANCE,
    AP_DEBUG_OUTPUT_CATEGORY_SHADER_COMPILER,
    AP_DEBUG_OUTPUT_CATEGORY_APPLICATION,
    AP_DEBUG_OUTPUT_CATEGORY_OTHER,
    AP_NUMBER_OF_DEBUG_OUTPUT_CATEGORIES
};


// ----------------------------------------------------------------------------------
// Enumeration:         apGLDebugOutputSeverity
// General Description: Is used to describe debug output severity.
// Author:  AMD Developer Tools Team
// Creation Date:        13/6/2010
// ----------------------------------------------------------------------------------
enum apGLDebugOutputSeverity
{
    AP_GL_DEBUG_OUTPUT_SEVERITY_LOW,            // Low
    AP_GL_DEBUG_OUTPUT_SEVERITY_MEDIUM,         // Medium
    AP_GL_DEBUG_OUTPUT_SEVERITY_HIGH,           // High
    AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES        // Unknown
};


// Debug output utilities:
AP_API const bool& apGLDebugOutputKindFromFlagArray(const bool* aFlags, apGLDebugOutputSource source, apGLDebugOutputType type);
AP_API bool& apGLDebugOutputKindFromMutableFlagArray(bool* aFlags, apGLDebugOutputSource source, apGLDebugOutputType type);

GLenum AP_API apDebugOutputSeverityToGL43Enum(apGLDebugOutputSeverity severity);
GLenum AP_API apDebugOutputSourceToGL43Enum(apGLDebugOutputSource source);
GLenum AP_API apDebugOutputTypeToGL43Enum(apGLDebugOutputType type);
apGLDebugOutputSeverity AP_API apDebugOutputSeverityFromGL43Enum(GLenum severity);
apGLDebugOutputSource AP_API apDebugOutputSourceFromGL43Enum(GLenum source);
apGLDebugOutputType AP_API apDebugOutputTypeFromGL43Enum(GLenum type);

GLenum AP_API apDebugOutputSeverityToGLARBEnum(apGLDebugOutputSeverity severity);
GLenum AP_API apDebugOutputSourceToGLARBEnum(apGLDebugOutputSource source);
GLenum AP_API apDebugOutputTypeToGLARBEnum(apGLDebugOutputType type);
apGLDebugOutputSeverity AP_API apDebugOutputSeverityFromGLARBEnum(GLenum severity);
apGLDebugOutputSource AP_API apDebugOutputSourceFromGLARBEnum(GLenum source);
apGLDebugOutputType AP_API apDebugOutputTypeFromGLARBEnum(GLenum type);

GLenum AP_API apDebugOutputSeverityToGLAMDEnum(apGLDebugOutputSeverity severity);
GLenum AP_API apDebugOutputCategoryToGLAMDEnum(apGLDebugOutputCategory category);
apGLDebugOutputSeverity AP_API apDebugOutputSeverityFromGLAMDEnum(GLenum severity);
apGLDebugOutputCategory AP_API apDebugOutputCategoryFromGLAMDEnum(GLenum category);

bool AP_API apIsDebugOutputCategoryFlagged(const bool* aFlags, apGLDebugOutputCategory category);

bool AP_API apGLDebugOutputSourceAsString(apGLDebugOutputSource source, gtString& sourceAsString);
bool AP_API apGLDebugOutputTypeAsString(apGLDebugOutputType type, gtString& sourceAsString, bool shortString = false);

bool AP_API apGLDebugOutputSeverityAsString(apGLDebugOutputSeverity severity, gtString& severityAsString);
apGLDebugOutputSeverity AP_API apGLDebugOutputSeverityFromString(const gtString& severityAsStr);
void AP_API apGLDebugOutputSeveritiesAsString(const bool severities[AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES], gtString& severitiesString);
bool AP_API apGLDebugOutputSeveritiesFromString(const gtString& severitiesString, bool severities[AP_NUMBER_OF_DEBUG_OUTPUT_SEVERITIES]);

#endif //__APGLDEBUGOUTPUT_H

