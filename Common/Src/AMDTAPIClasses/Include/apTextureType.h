//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apTextureType.h
///
//==================================================================================

//------------------------------ apTextureType.h ------------------------------

#ifndef __APTEXTURETYPE
#define __APTEXTURETYPE

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>

// Enumeration that lists texture types:
enum apTextureType
{
    AP_UNKNOWN_TEXTURE_TYPE,    // The texture type is unknown.
    AP_1D_TEXTURE,              // 1 dimensional texture.
    AP_2D_TEXTURE,              // 2 dimensional texture.
    AP_3D_TEXTURE,              // 3 dimensional texture.
    AP_CUBE_MAP_TEXTURE,        // A cube map texture.
    AP_CUBE_MAP_ARRAY_TEXTURE,  // A cube map array texture.
    AP_TEXTURE_RECTANGLE,       // A 2 dimensional texture rectangle.
    AP_1D_ARRAY_TEXTURE,        // A 1 dimensional texture array.
    AP_2D_ARRAY_TEXTURE,        // A 2 dimensional texture array.
    AP_2D_TEXTURE_MULTISAMPLE,  // A 2 dimensional multisample texture.
    AP_2D_TEXTURE_MULTISAMPLE_ARRAY,// A 2 dimensional multisample array texture.
    AP_BUFFER_TEXTURE,          // A texture buffer

    AP_AMOUNT_OF_TEXTURE_BIND_TARGETS = AP_BUFFER_TEXTURE
};

// Texture mipmap levels:
enum apTextureMipMapType
{
    AP_MIPMAP_NONE = 0,
    AP_MIPMAP_AUTO_GENERATE = 1,
    AP_MIPMAP_NONE_MANUAL = 2
};

// OpenGL types conversion utilities:
AP_API bool apIsProxyTextureBindTarget(GLenum bindTarget);
AP_API apTextureType apTextureBindTargetToTextureType(GLenum bindTarget);
AP_API GLenum apTextureTypeToTextureBindTarget(apTextureType textureType);
AP_API void apTextureTypeAsString(apTextureType type, gtString& string);



#endif  // __APTEXTURETYPE
