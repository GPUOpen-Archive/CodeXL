//=====================================================================
// Copyright 2009-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file
///
//=====================================================================
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/KeyWords/ARBProgramKeyWords.h#4 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================


#pragma once

#ifndef ARBPROGRAMKEYWORDS_H
#define ARBPROGRAMKEYWORDS_H

#include "KeyWords.h"

/// ShaderUtils is a set of shader utility functions.

namespace ShaderUtils
{
/// Get the keywords list for ARB FP shaders
/// \param[out] keyWords The keywords list.
/// \return True if successful, otherwise false.
bool GetARBFPKeyWords(ShaderUtils::KeyWordsList& keyWords);

/// Get the keywords list for ARB VP shaders
/// \param[out] keyWords The keywords list.
/// \return True if successful, otherwise false.
bool GetARBVPKeyWords(ShaderUtils::KeyWordsList& keyWords);
}; // ShaderUtils

#endif // ARBPROGRAMKEYWORDS_H
