//=====================================================================
// Copyright 2008-2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file D3D10Utils.h
///
//=====================================================================
//=====================================================================
// $Id: //devtools/main/Common/Src/ShaderUtils/DX10/D3D10Utils.h#4 $
//
// Last checkin:  $DateTime: 2016/04/18 06:01:26 $
// Last edited by: $Author:  AMD Developer Tools Team
//=====================================================================

#pragma once

#ifndef D3D10UTILS_H
#define D3D10UTILS_H

#include <string>
#include <d3d10_1.h>

/// D3D10Utils is a set of D3D10 utility functions.

namespace D3D10Utils
{
/// Save the texture as a DDS file.
/// \param[in] pTexture    The texture.
/// \param[in] strFilePath The filename path to save the texture to.
/// \return                True if a successful, otherwise false.
bool SaveTextureAsDDS(ID3D10Texture2D* pTexture, const std::string& strFilePath);

}; // D3D10Utils


#endif // D3D10OBJECT_H