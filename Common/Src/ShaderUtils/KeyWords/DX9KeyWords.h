//=====================================================================
// Copyright 2009-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DX9KeyWords.h
///
//=====================================================================

//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/KeyWords/DX9KeyWords.h#4 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================

#pragma once

#ifndef DX9KEYWORDS_H
#define DX9KEYWORDS_H

#include <Windows.h>
#include "KeyWords.h"
#include "D3D9ShaderUtils.h"

/// ShaderUtils is a set of shader utility functions.

namespace ShaderUtils
{
/// Get the keywords list for a DX9 assembly shader
/// \param[out] keyWords The keywords list.
/// \param[in] pdwShader Pointer to the shader.
/// \return True if successful, otherwise false.
bool GetDX9AsmKeyWords(ShaderUtils::KeyWordsList& keyWords, const DWORD* pdwShader);

/// Get the keywords list for DX9 assembly shaders
/// \param[out] keyWords The keywords list.
/// \param[in] shaderType The type shader.
/// \param[in] wShaderModelMajor The shader model major version number.
/// \param[in] wShaderModelMinor The shader model minor version number.
/// \return True if successful, otherwise false.
bool GetDX9AsmKeyWords(ShaderUtils::KeyWordsList& keyWords, D3D9ShaderUtils::DX9ShaderType shaderType, WORD wShaderModelMajor, WORD wShaderModelMinor);

/// Get the keywords list for DX9 HLSL shaders
/// \param[out] keyWords The keywords list.
/// \param[in] shaderType The type shader.
/// \param[in] wShaderModelMajor The shader model major version number.
/// \param[in] wShaderModelMinor The shader model minor version number.
/// \return True if successful, otherwise false.
bool GetDX9HLSLKeyWords(ShaderUtils::KeyWordsList& keyWords, D3D9ShaderUtils::DX9ShaderType shaderType, WORD wShaderModelMajor, WORD wShaderModelMinor);
}; // ShaderUtils

#endif // DX9KEYWORDS_H
