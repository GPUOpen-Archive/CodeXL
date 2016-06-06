//=====================================================================
// Copyright 2009-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DX9PSAsmKeyWords.h
///
//=====================================================================

//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/KeyWords/DX9PSAsmKeyWords.h#4 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================

#pragma once

#ifndef DX9PSASMKEYWORDS_H
#define DX9PSASMKEYWORDS_H

#include "DX9KeyWords.h"

/// ShaderUtils is a set of shader utility functions.

namespace ShaderUtils
{
/// Generate a string containing all DX9 Pixel Shader instruction keywords for the specified shader model.
/// \param[out] strKeyWords         The string of generated keywords.
/// \param[in]  wShaderModelMajor   The shader model version (major) for which to generate keywords.
/// \param[in]  wShaderModelMinor   The shader model version (minor) for which to generate keywords.
/// \param[in]  bPartialPrecision   Generate partial-precision keywords or regular full-precision keywords?
/// \return     True if successful, otherwise false.
bool GetDX9PSInstKeyWords(std::string& strKeyWords, WORD wShaderModelMajor, WORD wShaderModelMinor, bool bPartialPrecision);

/// Generate a string containing all DX9 Pixel Shader register keywords for the specified shader model.
/// \param[out] strKeyWords         The string of generated keywords.
/// \param[in]  wShaderModelMajor   The shader model version (major) for which to generate keywords.
/// \param[in]  wShaderModelMinor   The shader model version (minor) for which to generate keywords.
/// \return     True if successful, otherwise false.
bool GetDX9PSRegKeyWords(std::string& strKeyWords, WORD wShaderModelMajor, WORD wShaderModelMinor);
}; // ShaderUtils

#endif // DX9PSASMKEYWORDS_H
