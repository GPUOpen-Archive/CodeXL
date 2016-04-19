//==============================================================================
// Copyright (c) 2013-2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  Interface used to track the currently active core objects
//==============================================================================

#include "IMonitor.h"

#define stringify( name ) # name ///< helper macro

//
//  Converts a CREATION_TYPE enum element into a string
//
const char* CreationTypeToString(CREATION_TYPE type)
{
    static const char* CreationTypeString[] = { stringify(DX9_DEVICE), stringify(DX10_DEVICE), stringify(DX11_DEVICE), stringify(DXGI_FACTORY), stringify(DX11_SWAPCHAIN), stringify(D3D11_DEVICE_CONTEXT), stringify(OGL_CONTEXT) };
    return CreationTypeString[ type ];
}

