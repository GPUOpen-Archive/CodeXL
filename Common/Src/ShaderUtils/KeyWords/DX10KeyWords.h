//=====================================================================
// Copyright 2010-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DX10KeyWords.h
///
//=====================================================================

//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/KeyWords/DX10KeyWords.h#4 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================

#pragma once

#ifndef DX10KEYWORDS_H
#define DX10KEYWORDS_H

#include <Windows.h>
#include "KeyWords.h"
#include "D3D10ShaderObject.h"

/// ShaderUtils is a set of shader utility functions.

namespace ShaderUtils
{
/// Get the keywords list for a DX10 assembly shader
/// \param[out] keyWords The keywords list.
/// \param[in] shader The shader object.
/// \return True if successful, otherwise false.
bool GetDX10AsmKeyWords(ShaderUtils::KeyWordsList& keyWords, const D3D10ShaderObject::CD3D10ShaderObject& shader);

/// Get the keywords list for DX10 HLSL shaders
/// \param[out] keyWords The keywords list.
/// \param[in] shader The shader object.
/// \return True if successful, otherwise false.
bool GetDX10HLSLKeyWords(ShaderUtils::KeyWordsList& keyWords, const D3D10ShaderObject::CD3D10ShaderObject& shader);
}; // ShaderUtils

#endif // DX10KEYWORDS_H
