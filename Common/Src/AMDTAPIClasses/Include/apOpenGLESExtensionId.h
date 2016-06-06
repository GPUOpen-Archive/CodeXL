//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apOpenGLESExtensionId.h
///
//==================================================================================

//------------------------------ apOpenGLESExtensionId.h ------------------------------

#ifndef __APOPENGLESEXTENSIONID_H
#define __APOPENGLESEXTENSIONID_H

// Infra:
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>


// OpenGL ES extensions ids:
enum apOpenGLESExtensionId
{
    AP_OES_draw_texture,
    AP_AMOUNT_OF_SUPPORTED_OGLES_EXTENSIONS
};


AP_API bool apOpenGLESExtensionsIdToString(apOpenGLESExtensionId extensionId, gtString& extensionString);


#endif //__APOPENGLESEXTENSIONID_H

