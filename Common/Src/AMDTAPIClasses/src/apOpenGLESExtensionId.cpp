//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenGLESExtensionId.cpp
///
//==================================================================================

//------------------------------ apOpenGLESExtensionId.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/apOpenGLESExtensionId.h>


// ---------------------------------------------------------------------------
// Name:        apOpenGLESExtensionsIdToString
// Description: Translated OpenGL ES extension enum to string.
// Arguments: extensionId - The input OpenGL ES extension enum.
//            extensionString - The output string, containing the extension name.
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        9/4/2006
// ---------------------------------------------------------------------------
bool apOpenGLESExtensionsIdToString(apOpenGLESExtensionId extensionId, gtString& extensionString)
{
    bool retVal = true;

    switch (extensionId)
    {
        case AP_OES_draw_texture:
            extensionString = L"OES_draw_texture";
            break;

        default:
            // A non supported extension was queried:
            extensionString.makeEmpty();
            retVal = false;
            break;
    }

    return retVal;
}

