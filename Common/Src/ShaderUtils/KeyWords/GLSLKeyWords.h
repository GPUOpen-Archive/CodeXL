//=====================================================================
// Copyright 2009-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file GLSLKeyWords.h
///
//=====================================================================
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/KeyWords/GLSLKeyWords.h#4 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================

#pragma once

#ifndef GLSLKEYWORDS_H
#define GLSLKEYWORDS_H

#include "KeyWords.h"

//#pragma message(__FILE__  "TODO: Improve GetGLSLKeyWords to handle different GLSL versions")
/// \todo Improve GetGLSLKeyWords to handle different GLSL versions

/// ShaderUtils is a set of shader utility functions.

namespace ShaderUtils
{
/// Get the keywords list for GLSL shaders
/// \param[out] keyWords The keywords list.
/// \return True if successful, otherwise false.
bool GetGLSLKeyWords(ShaderUtils::KeyWordsList& keyWords);
}; // ShaderUtils

#endif // GLSLKEYWORDS_H
