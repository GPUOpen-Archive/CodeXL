//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsRenderContextExtensionsData.cpp
///
//==================================================================================

//------------------------------ gsRenderContextExtensionsData.cpp ------------------------------

// Standard C:
#include <string.h>

// Local:
#include <src/gsRenderContextExtensionsData.h>


// ---------------------------------------------------------------------------
// Name:        gsRenderContextExtensionsData::gsRenderContextExtensionsData
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        13/6/2006
// ---------------------------------------------------------------------------
gsRenderContextExtensionsData::gsRenderContextExtensionsData()
{
    // Initialize _extensionFunctionsRealImpl with NULLs:
    memset(&_extensionFunctionsRealImpl, 0, sizeof(gsMonitoredFunctionPointers));

    // Initialize the _isExtensionSupported array items to contain false:
    for (int i = 0; i < AP_AMOUNT_OF_SUPPORTED_OGL_EXTENSIONS; i++)
    {
        _isExtensionSupported[i] = false;
    }
}


// ---------------------------------------------------------------------------
// Name:        gsRenderContextExtensionsData::~gsRenderContextExtensionsData
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        13/6/2006
// ---------------------------------------------------------------------------
gsRenderContextExtensionsData::~gsRenderContextExtensionsData()
{
}
