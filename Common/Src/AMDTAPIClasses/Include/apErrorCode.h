//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apErrorCode.h
///
//==================================================================================

//------------------------------ apErrorCode.h ------------------------------

#ifndef __APERRORCODE
#define __APERRORCODE

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>

// Lists error codes:
enum apErrorCode
{
    AP_UNKNOWN_DETECTED_ERROR,                      // Default value

    AP_FOREIGN_CONTEXT_EXTENSION_FUNC_CALL_ERROR,   // Calling an OpenGL extension function
    // in context A using a function pointer
    // the was retrieved from context B.

    AP_NON_SUPPORTED_TEXTURE_UNIT_ERROR,            // Using a non supported texture unit.

    AP_OBJECT_NAME_DOES_NOT_EXIST_ERROR,            // Using an OpenGL managed object name that
    // does not exist.

    AP_SHADER_ALREADY_ATTACHED_ERROR,               // Trying to attach a shader to a program
    // to which it is already attached.

    AP_SHADER_COMPILATION_FAILED_ERROR,             // A shader compilation operation failed.

    AP_PROGRAM_LINK_FAILED_ERROR,                   // A program link operation failed.

    AP_MAX_LOGGED_FUNCTIONS_EXCEEDED_ERROR,         // The maximal amount of logged functions
    // was exceeded.

    AP_RENDER_CONTEXT_DELETION_FAILED_ERROR,        // The deletion of a render context had failed.

    AP_USING_SOFTWARE_RENDERER_ERROR,               // A render context is using a software renderer.

    AP_QUEUE_EVENTS_OVERFLOW,                       // Too many events were assigned on a single OpenCL
    // command queue, stopping the logging of these events.
};


AP_API bool apDetectedErrorCodeToString(apErrorCode errorCode, gtString& errorString);


#endif  // __APERRORCODE
