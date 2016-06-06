//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apTextureType.cpp
///
//==================================================================================

//------------------------------ apTextureType.cpp ------------------------------

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTAPIClasses/Include/apTextureType.h>
#include <AMDTAPIClasses/Include/apStringConstants.h>


// ---------------------------------------------------------------------------
// Name:        apIsProxyTextureBindTarget
// Description: Inputs a bind target and returns true iff this is a
//              texture proxy bind target.
// Author:  AMD Developer Tools Team
// Date:        29/11/2005
// ---------------------------------------------------------------------------
AP_API bool apIsProxyTextureBindTarget(GLenum bindTarget)
{
    bool retVal = false;

    // If this is texture proxy:
    if ((bindTarget == GL_PROXY_TEXTURE_1D) || (bindTarget == GL_PROXY_TEXTURE_2D) ||
        (bindTarget == GL_PROXY_TEXTURE_3D) || (bindTarget == GL_PROXY_TEXTURE_CUBE_MAP) ||
        (bindTarget == GL_PROXY_TEXTURE_RECTANGLE_ARB) || (bindTarget == GL_PROXY_TEXTURE_1D_ARRAY) || (bindTarget == GL_PROXY_TEXTURE_2D_ARRAY))
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apTextureBindTargetToTextureType
// Description: Translates OpenGL bind target to apTextureType.
// Author:  AMD Developer Tools Team
// Date:        3/1/2005
// ---------------------------------------------------------------------------
apTextureType apTextureBindTargetToTextureType(GLenum bindTarget)
{
    apTextureType retVal = AP_UNKNOWN_TEXTURE_TYPE;

    switch (bindTarget)
    {
        case GL_TEXTURE_1D:
            retVal = AP_1D_TEXTURE;
            break;

        case GL_TEXTURE_2D:
            retVal = AP_2D_TEXTURE;
            break;

        case GL_TEXTURE_3D:
            retVal = AP_3D_TEXTURE;
            break;

        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            retVal = AP_CUBE_MAP_TEXTURE;
            break;

        case GL_TEXTURE_CUBE_MAP_ARRAY:
            retVal = AP_CUBE_MAP_ARRAY_TEXTURE;
            break;

        case GL_TEXTURE_RECTANGLE:
            retVal = AP_TEXTURE_RECTANGLE;
            break;

        case GL_TEXTURE_1D_ARRAY:
            retVal = AP_1D_ARRAY_TEXTURE;
            break;

        case GL_TEXTURE_2D_ARRAY:
            retVal = AP_2D_ARRAY_TEXTURE;
            break;

        case GL_TEXTURE_2D_MULTISAMPLE:
            retVal = AP_2D_TEXTURE_MULTISAMPLE;
            break;

        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
            retVal = AP_2D_TEXTURE_MULTISAMPLE_ARRAY;
            break;

        case GL_TEXTURE_BUFFER:
            retVal = AP_BUFFER_TEXTURE;
            break;
    }

    if (retVal == AP_UNKNOWN_TEXTURE_TYPE)
    {
        gtString logMgs = L"Warning: apTextureBindTargetToTextureType is translating unknown texture type: GLenum = ";
        logMgs.appendFormattedString(L"0x%X", bindTarget);

        OS_OUTPUT_DEBUG_LOG(logMgs.asCharArray(), OS_DEBUG_LOG_DEBUG);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apTextureTypeToTextureBindTarget
// Description:
//   Translates apTextureType to OpenGL bind target.
//   The function return GL_NONE on failure.
// Author:  AMD Developer Tools Team
// Date:        21/4/2005
// ---------------------------------------------------------------------------
GLenum apTextureTypeToTextureBindTarget(apTextureType textureType)
{
    GLenum retVal = GL_NONE;

    switch (textureType)
    {
        case AP_1D_TEXTURE:
            retVal = GL_TEXTURE_1D;
            break;

        case AP_2D_TEXTURE:
            retVal = GL_TEXTURE_2D;
            break;

        case AP_3D_TEXTURE:
            retVal = GL_TEXTURE_3D;
            break;

        case AP_CUBE_MAP_TEXTURE:
            retVal = GL_TEXTURE_CUBE_MAP;
            break;

        case AP_CUBE_MAP_ARRAY_TEXTURE:
            retVal = GL_TEXTURE_CUBE_MAP_ARRAY;
            break;

        case AP_TEXTURE_RECTANGLE:
            retVal = GL_TEXTURE_RECTANGLE_ARB;
            break;

        case AP_1D_ARRAY_TEXTURE:
            retVal = GL_TEXTURE_1D_ARRAY;
            break;

        case AP_2D_ARRAY_TEXTURE:
            retVal = GL_TEXTURE_2D_ARRAY;
            break;

        case AP_2D_TEXTURE_MULTISAMPLE:
            retVal = GL_TEXTURE_2D_MULTISAMPLE;
            break;

        case AP_2D_TEXTURE_MULTISAMPLE_ARRAY:
            retVal = GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
            break;

        case AP_BUFFER_TEXTURE:
            retVal = GL_TEXTURE_BUFFER;
            break;

        case AP_UNKNOWN_TEXTURE_TYPE:
            retVal = GL_NONE;
            break;

        default:
            // Invalid input:
            GT_ASSERT(0);
            break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apTextureTypeAsString
// Description: Convert the texture type to string.
// Author:  AMD Developer Tools Team
// Date:        4/1/2005
// ---------------------------------------------------------------------------
void apTextureTypeAsString(apTextureType type, gtString& string)
{
    gtString textureTypeString = L"Unknown Type";

    switch (type)
    {
        case AP_1D_TEXTURE:
            textureTypeString = AP_STR_1DTexture;
            break;

        case AP_2D_TEXTURE:
            textureTypeString = AP_STR_2DTexture;
            break;

        case AP_3D_TEXTURE:
            textureTypeString = AP_STR_3DTexture;
            break;

        case AP_CUBE_MAP_TEXTURE:
            textureTypeString = AP_STR_CubeMapTexture;
            break;

        case AP_CUBE_MAP_ARRAY_TEXTURE:
            textureTypeString = AP_STR_CubeMapArrayTexture;
            break;

        case AP_TEXTURE_RECTANGLE:
            textureTypeString = AP_STR_RectangleTexture;
            break;

        case AP_1D_ARRAY_TEXTURE:
            textureTypeString = AP_STR_1DArrayTexture;
            break;

        case AP_2D_ARRAY_TEXTURE:
            textureTypeString = AP_STR_2DArrayTexture;
            break;

        case AP_2D_TEXTURE_MULTISAMPLE:
            textureTypeString = AP_STR_2DMultiSampleTexture;
            break;

        case AP_2D_TEXTURE_MULTISAMPLE_ARRAY:
            textureTypeString = AP_STR_2DMultiSampleArrayTexture;
            break;

        case AP_BUFFER_TEXTURE:
            textureTypeString = AP_STR_BufferTexture;
            break;

        case AP_UNKNOWN_TEXTURE_TYPE:
            textureTypeString = AP_STR_Unknown;
            break;

        default:
            textureTypeString = AP_STR_NATexture;
    }

    string = textureTypeString;
}

