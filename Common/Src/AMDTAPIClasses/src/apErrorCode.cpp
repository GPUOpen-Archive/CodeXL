//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apErrorCode.cpp
///
//==================================================================================

//------------------------------ apErrorCode.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/apErrorCode.h>


// ---------------------------------------------------------------------------
// Name:        apDetectedErrorCodeToString
// Description: Translates apErrorCode to a error code string.
// Arguments:   apErrorCode - The input error code.
//              errorString - The output error string.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        23/3/2005
// ---------------------------------------------------------------------------
bool apDetectedErrorCodeToString(apErrorCode errorCode, gtString& errorString)
{
    bool retVal = true;
    errorString.makeEmpty();

    switch (errorCode)
    {
        case AP_FOREIGN_CONTEXT_EXTENSION_FUNC_CALL_ERROR:
            errorString = L"AP_FOREIGN_CONTEXT_EXTENSION_FUNC_CALL_ERROR";
            break;

        case AP_NON_SUPPORTED_TEXTURE_UNIT_ERROR:
            errorString = L"AP_NON_SUPPORTED_TEXTURE_UNIT_ERROR";
            break;

        case AP_OBJECT_NAME_DOES_NOT_EXIST_ERROR:
            errorString = L"AP_OBJECT_NAME_DOES_NOT_EXIST_ERROR";
            break;

        case AP_SHADER_ALREADY_ATTACHED_ERROR:
            errorString = L"AP_SHADER_ALREADY_ATTACHED_ERROR";
            break;

        case AP_SHADER_COMPILATION_FAILED_ERROR:
            errorString = L"AP_SHADER_COMPILATION_FAILED_ERROR";
            break;

        case AP_PROGRAM_LINK_FAILED_ERROR:
            errorString = L"AP_PROGRAM_LINK_FAILED_ERROR";
            break;

        case AP_MAX_LOGGED_FUNCTIONS_EXCEEDED_ERROR:
            errorString = L"AP_MAX_LOGGED_FUNCTIONS_EXCEEDED_ERROR";
            break;

        case AP_RENDER_CONTEXT_DELETION_FAILED_ERROR:
            errorString = L"AP_RENDER_CONTEXT_DELETION_FAILED_ERROR";
            break;

        case AP_USING_SOFTWARE_RENDERER_ERROR:
            errorString = L"AP_USING_SOFTWARE_RENDERER_ERROR";
            break;

        case AP_QUEUE_EVENTS_OVERFLOW:
            errorString = L"AP_QUEUE_EVENTS_OVERFLOW";
            break;

        default:
            // Unknown file type:
            errorString = L"UNKNOWN_ERROR_CODE";
            retVal = false;
            break;
    }

    return retVal;
}

