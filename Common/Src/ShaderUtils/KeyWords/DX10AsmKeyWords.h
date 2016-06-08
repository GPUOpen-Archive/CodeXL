//=====================================================================
// Copyright 2009-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DX10AsmKeyWords.h
///
//=====================================================================

//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/KeyWords/DX10AsmKeyWords.h#4 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================

#pragma once

#ifndef DX10ASMKEYWORDS_H
#define DX10ASMKEYWORDS_H

#include "DX10KeyWords.h"

/// ShaderUtils is a set of shader utility functions.

namespace ShaderUtils
{
/// Generate a keyword list containing all the assembly instruction keywords for the specified shader.
/// \param[out] keyWords   The list of generated keywords.
/// \param[in]  shader     The shader for which to generate keywords.
/// \return     True if successful, otherwise false.
bool GetDX10InstKeyWords(ShaderUtils::KeyWordsList& keyWords, const D3D10ShaderObject::CD3D10ShaderObject& shader);

/// Generate a keyword list containing all the assembly register keywords for the specified shader.
/// \param[out] keyWords   The list of generated keywords.
/// \param[in]  shader     The shader for which to generate keywords.
/// \return     True if successful, otherwise false.
bool GetDX10RegKeyWords(ShaderUtils::KeyWordsList& keyWords, const D3D10ShaderObject::CD3D10ShaderObject& shader);
}; // ShaderUtils

#endif // DX10ASMKEYWORDS_H
