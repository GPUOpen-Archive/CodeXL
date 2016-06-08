//=====================================================================
// Copyright 2009-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file KeyWords.h
///
//=====================================================================
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/KeyWords/KeyWords.h#4 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================

#pragma once

#ifndef KEYWORDS_H
#define KEYWORDS_H

#include <string>
#include <vector>

/// ShaderUtils is a set of shader utility functions.

namespace ShaderUtils
{
/// The type of keywords.
typedef enum
{
    KW_All,           ///< All keywords.
    KW_Op,            ///< Regular floating-point operation keywords.
    KW_IntOp,         ///< Integer operation keywords.
    KW_DoubleOp,      ///< Double floating-point operation keywords.
    KW_TextureOp,     ///< Texture operation keywords.
    KW_UnsupportedOp, ///< Unsupported operation keywords. These are for instructions not yet supported by ShaderDebugger.
    KW_Function,      ///< Function name keywords.
    KW_Reg,           ///< Register name keywords.
    KW_Type,          ///< Type name keywords.
    KW_Semantics,     ///< Semantic name keywords.
    KW_Variable,      ///< Variable name keywords.
    KW_Reserved,      ///< Reserved word keywords.
} KeyWordType;

/// Type definition of a keywords list member.
typedef struct
{
    KeyWordType type;          ///< The type of keywords.
    std::string strKeywords;   ///< The string containing the keywords.
} KeyWords;

/// Type definition of a list of keywords.
typedef std::vector<KeyWords> KeyWordsList;

}; // ShaderUtils


#endif // KEYWORDS_H
