//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains DirectCompute Utilities to stringify DC enums
//==============================================================================

#ifndef _DC_STRINGIFY_DXGI_TYPE_ENUMS_H
#define _DC_STRINGIFY_DXGI_TYPE_ENUMS_H

#include <string>
#include "dxgi.h"

/// \addtogroup DCServer
// @{

namespace DCUtils
{

std::string Stringify_DXGI_MODE_SCANLINE_ORDER(DXGI_MODE_SCANLINE_ORDER var);
std::string Stringify_DXGI_MODE_SCALING(DXGI_MODE_SCALING var);
std::string Stringify_DXGI_MODE_ROTATION(DXGI_MODE_ROTATION var);
std::string Stringify_DXGI_SWAP_EFFECT(DXGI_SWAP_EFFECT var);

}
// @}

#endif // _DC_STRINGIFY_DXGI_TYPE_ENUMS_H
