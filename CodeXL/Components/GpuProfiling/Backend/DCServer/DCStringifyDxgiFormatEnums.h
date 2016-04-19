//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief  This file contains DirectCompute Utilities to stringify DC enums
//==============================================================================

#ifndef _DC_STRINGIFY_DXGI_FORMAT_ENUMS_H_
#define _DC_STRINGIFY_DXGI_FORMAT_ENUMS_H_

#include <string>
#include "dxgiformat.h"

/// \addtogroup DCServer
// @{

namespace DCUtils
{
std::string Stringify_DXGI_FORMAT(DXGI_FORMAT var);
}
// @}

#endif // _DC_STRINGIFY_DXGI_FORMAT_ENUMS_H_
