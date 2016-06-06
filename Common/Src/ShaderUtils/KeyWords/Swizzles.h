//=====================================================================
// Copyright 2009-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file Swizzles.h
///
//=====================================================================

//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/KeyWords/Swizzles.h#4 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================

#pragma once

#ifndef SWIZZLES_H
#define SWIZZLES_H

#include "DX10KeyWords.h"

/// ShaderUtils is a set of shader utility functions.

namespace ShaderUtils
{
/// Generate a string containing all possible RGBA swizzles.
/// \return A string containing all possible RGBA swizzles.
const std::string& GetRGBASwizzles();

/// Generate a string containing all possible XYZW swizzles.
/// \return A string containing all possible XYZW swizzles.
const std::string& GetXYZWSwizzles();
}; // ShaderUtils

#endif // SWIZZLES_H
