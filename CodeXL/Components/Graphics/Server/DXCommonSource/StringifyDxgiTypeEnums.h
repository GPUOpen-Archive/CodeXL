//=============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Functions to convert DXGI enums into strings.
//=============================================================================
#ifndef STRINGIFYDXGITYPEENUMS_H
#define STRINGIFYDXGITYPEENUMS_H

#include "dxgi.h"
#include "AMDTBaseTools/Include/gtASCIIString.h"

gtASCIIString Stringify_DXGI_MODE_SCANLINE_ORDER(DXGI_MODE_SCANLINE_ORDER var);
gtASCIIString Stringify_DXGI_MODE_SCALING(DXGI_MODE_SCALING var);
gtASCIIString Stringify_DXGI_MODE_ROTATION(DXGI_MODE_ROTATION var);
gtASCIIString Stringify_DXGI_SWAP_EFFECT(DXGI_SWAP_EFFECT var);

#endif // STRINGIFYDXGITYPEENUMS_H