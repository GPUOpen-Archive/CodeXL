//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsRenderContextExtensionsData.h
///
//==================================================================================

//------------------------------ gsRenderContextExtensionsData.h ------------------------------

#ifndef __GSRENDERCONTEXTEXTENSIONSDATA_H
#define __GSRENDERCONTEXTEXTENSIONSDATA_H

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTAPIClasses/Include/apOpenGLExtensionsId.h>

// Local:
#include <src/gsMonitoredFunctionPointers.h>

// ----------------------------------------------------------------------------------
// Struct Name:   gsRenderContextExtensionsData
// General Description:
//   Contains data related to a render context extensions support.
//
// Author:               Yaki Tebeka
// Creation Date:        13/6/2006
// ----------------------------------------------------------------------------------
struct gsRenderContextExtensionsData
{
    // Maps render context id to its extensions function "real" implementation:
    gsMonitoredFunctionPointers _extensionFunctionsRealImpl;

    // Contains the spy version of render context supported extensions unified string (OpenGL 3.0 and lower):
    gtASCIIString _spyExtensionsUnifiedString;

    // Contains the spy version of the render context supported extension strings (OpenGL 3.0 and higher):
    gtVector<gtASCIIString> _spyExtensionStrings;

    // Index i contains true iff extension i (in apOpenGLExtensionsId) is supported by this context:
    bool _isExtensionSupported[AP_AMOUNT_OF_SUPPORTED_OGL_EXTENSIONS];

public:
    gsRenderContextExtensionsData();
    ~gsRenderContextExtensionsData();
};


#endif //__GSRENDERCONTEXTEXTENSIONSDATA_H
