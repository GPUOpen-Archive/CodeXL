//=============================================================================
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief Functions to convert DXGI enums into strings.
//=============================================================================
#include "dxgi.h"
#include "AMDTBaseTools/Include/gtASCIIString.h"

//-----------------------------------------------------------------------------
/// Stringify_DXGI_MODE_SCANLINE_ORDER
//-----------------------------------------------------------------------------
gtASCIIString Stringify_DXGI_MODE_SCANLINE_ORDER(DXGI_MODE_SCANLINE_ORDER var)
{
    switch (var)
    {
        case DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED: return "DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED";

        case DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE: return "DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE";

        case DXGI_MODE_SCANLINE_ORDER_UPPER_FIELD_FIRST: return "DXGI_MODE_SCANLINE_ORDER_UPPER_FIELD_FIRST";

        case DXGI_MODE_SCANLINE_ORDER_LOWER_FIELD_FIRST: return "DXGI_MODE_SCANLINE_ORDER_LOWER_FIELD_FIRST";

        default: return "not found";
    }
}

//-----------------------------------------------------------------------------
/// Stringify_DXGI_MODE_SCALING
//-----------------------------------------------------------------------------
gtASCIIString Stringify_DXGI_MODE_SCALING(DXGI_MODE_SCALING var)
{
    switch (var)
    {
        case DXGI_MODE_SCALING_UNSPECIFIED: return "DXGI_MODE_SCALING_UNSPECIFIED";

        case DXGI_MODE_SCALING_CENTERED: return "DXGI_MODE_SCALING_CENTERED";

        case DXGI_MODE_SCALING_STRETCHED: return "DXGI_MODE_SCALING_STRETCHED";

        default: return "not found";
    }
}

//-----------------------------------------------------------------------------
/// Stringify_DXGI_MODE_ROTATION
//-----------------------------------------------------------------------------
gtASCIIString Stringify_DXGI_MODE_ROTATION(DXGI_MODE_ROTATION var)
{
    switch (var)
    {
        case DXGI_MODE_ROTATION_UNSPECIFIED: return "DXGI_MODE_ROTATION_UNSPECIFIED";

        case DXGI_MODE_ROTATION_IDENTITY: return "DXGI_MODE_ROTATION_IDENTITY";

        case DXGI_MODE_ROTATION_ROTATE90: return "DXGI_MODE_ROTATION_ROTATE90";

        case DXGI_MODE_ROTATION_ROTATE180: return "DXGI_MODE_ROTATION_ROTATE180";

        case DXGI_MODE_ROTATION_ROTATE270: return "DXGI_MODE_ROTATION_ROTATE270";

        default: return "not found";
    }
}

//-----------------------------------------------------------------------------
/// Stringify_DXGI_SWAP_EFFECT
//-----------------------------------------------------------------------------
gtASCIIString Stringify_DXGI_SWAP_EFFECT(DXGI_SWAP_EFFECT var)
{
    switch (var)
    {
        case DXGI_SWAP_EFFECT_DISCARD: return "DXGI_SWAP_EFFECT_DISCARD";

        case DXGI_SWAP_EFFECT_SEQUENTIAL: return "DXGI_SWAP_EFFECT_SEQUENTIAL";

        case DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL: return "DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL";

        case DXGI_SWAP_EFFECT_FLIP_DISCARD: return "DXGI_SWAP_EFFECT_FLIP_DISCARD";

        default: return "not found";
    }
}