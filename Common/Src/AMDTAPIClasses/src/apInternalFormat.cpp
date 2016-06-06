//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apInternalFormat.cpp
///
//==================================================================================

//------------------------------ apInternalFormat.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/apInternalFormat.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apGLPixelInternalFormatParameter.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>


// ---------------------------------------------------------------------------
// Name:        apInternalFormatValueAsString
// Description: Translates a GLenum parameter that we know it is an internal format
//              to a string. If the enum is a number, it returns the number as string,
//              otherwise it treats the enum as other glenum, and translate it to a string
// Arguments: GLenum internalFormat
// Return Val: gtString
// Author:  AMD Developer Tools Team
// Date:        12/10/2008
// ---------------------------------------------------------------------------
gtString apInternalFormatValueAsString(GLenum internalFormat)
{
    gtString retVal;

    apGLPixelInternalFormatParameter param(internalFormat);
    param.valueAsString(retVal);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGetPixelSizeInBitsByInternalFormat
// Description: Returns a pixel size in bits according to openGL enum that represents an internal format
// Arguments: GLenum internalFormat
//            GLuint& pixelSizeInBits
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/10/2008
// ---------------------------------------------------------------------------
bool apGetPixelSizeInBitsByInternalFormat(GLenum internalFormat, GLuint& pixelSizeInBits)
{
    bool retVal = true;

    switch (internalFormat)
    {
        case 0:
        {
            // Internal format was not set yet:
            pixelSizeInBits = 0;
        }
        break;

        case 1:
        case 2:
        case 3:
        case 4:
        {
            pixelSizeInBits = GT_BITS_PER_BYTE * internalFormat;
        }
        break;

        case GL_COLOR_INDEX1_EXT:
        {
            pixelSizeInBits = 1;
        }
        break;

        case GL_COLOR_INDEX2_EXT:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
        case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
#endif
            {
                pixelSizeInBits = 2;
            }
            break;

        case GL_ALPHA:
        case GL_DEPTH_COMPONENT:
        case GL_LUMINANCE:
        case GL_INTENSITY:
        case GL_SLUMINANCE:
        {
            pixelSizeInBits = GT_BITS_PER_BYTE;
        }
        break;

        case GL_ALPHA4:
        case GL_LUMINANCE4:
        case GL_INTENSITY4:
        case GL_COLOR_INDEX4_EXT:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
        case GL_PALETTE4_R5_G6_B5_OES:
        case GL_PALETTE4_RGBA4_OES:
        case GL_PALETTE4_RGB5_A1_OES:
        case GL_PALETTE4_RGB8_OES:
        case GL_PALETTE4_RGBA8_OES:
#endif
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
        case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
#endif
            {
                pixelSizeInBits = 4;
            }
            break;

        case GL_ALPHA8:
        case GL_LUMINANCE8:
        case GL_INTENSITY8:
        case GL_LUMINANCE4_ALPHA4:
        case GL_LUMINANCE6_ALPHA2:
        case GL_R3_G3_B2:
        case GL_RGBA2:
        case GL_SLUMINANCE8:
        case GL_COLOR_INDEX8_EXT:
        case GL_ALPHA8UI_EXT:
        case GL_ALPHA8I_EXT:
        case GL_INTENSITY8UI_EXT:
        case GL_LUMINANCE8UI_EXT:
        case GL_INTENSITY8I_EXT:
        case GL_LUMINANCE8I_EXT:
        case GL_R8I:
        case GL_R8UI:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
        case GL_PALETTE8_R5_G6_B5_OES:
        case GL_PALETTE8_RGBA4_OES:
        case GL_PALETTE8_RGB5_A1_OES:
        case GL_PALETTE8_RGB8_OES:
        case GL_PALETTE8_RGBA8_OES:
#endif
            {
                pixelSizeInBits = 8;
            }
            break;

        case GL_ALPHA12:
        case GL_LUMINANCE12:
        case GL_INTENSITY12:
        case GL_RGB4:
        case GL_COLOR_INDEX12_EXT:
        {
            pixelSizeInBits = 12;
        }
        break;

        case GL_ALPHA16:
        case GL_DEPTH_COMPONENT16:
        case GL_LUMINANCE16:
        case GL_INTENSITY16:
        case GL_LUMINANCE8_ALPHA8:
        case GL_LUMINANCE12_ALPHA4:
        case GL_RGBA4:
        case GL_RGB5_A1:
        case GL_SLUMINANCE8_ALPHA8:
        case GL_COLOR_INDEX16_EXT:
        case GL_ALPHA16F_ARB:
        case GL_INTENSITY16F_ARB:
        case GL_LUMINANCE16F_ARB:
        case GL_ALPHA16UI_EXT:
        case GL_INTENSITY16UI_EXT:
        case GL_LUMINANCE16UI_EXT:
        case GL_LUMINANCE_ALPHA8UI_EXT:
        case GL_ALPHA16I_EXT:
        case GL_INTENSITY16I_EXT:
        case GL_LUMINANCE16I_EXT:
        case GL_LUMINANCE_ALPHA8I_EXT:
        case GL_R16F:
        case GL_R16I:
        case GL_R16UI:
        case GL_RG8I:
        case GL_RG8UI:
        {
            pixelSizeInBits = 16;
        }
        break;

        case GL_COMPRESSED_ALPHA:
        case GL_COMPRESSED_LUMINANCE:
        case GL_COMPRESSED_LUMINANCE_ALPHA:
        case GL_COMPRESSED_INTENSITY:
        case GL_COMPRESSED_RGB:
        case GL_COMPRESSED_RGBA:
        case GL_COMPRESSED_RED_RGTC1:
        case GL_COMPRESSED_SIGNED_RED_RGTC1:
        case GL_COMPRESSED_RG_RGTC2:
        case GL_COMPRESSED_SIGNED_RG_RGTC2:
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_RGB_FXT1_3DFX:
        case GL_COMPRESSED_RGBA_FXT1_3DFX:
        case GL_COMPRESSED_SRGB_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_EXT:
        case GL_COMPRESSED_SLUMINANCE_EXT:
        case GL_COMPRESSED_SLUMINANCE_ALPHA_EXT:
        case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_RED:
        case GL_COMPRESSED_RG:
        {
            pixelSizeInBits = 0;
        }
        break;

        case GL_DEPTH_COMPONENT24:
        case GL_LUMINANCE12_ALPHA12:
        case GL_RGB8:
        case GL_SRGB8:
        case GL_RGB8UI_EXT:
        case GL_RGB8I_EXT:
        {
            pixelSizeInBits = 24;
        }
        break;

        case GL_DEPTH_COMPONENT32:
        case GL_LUMINANCE16_ALPHA16:
        case GL_RGB10_A2:
        case GL_RGBA8:
        case GL_SRGB8_ALPHA8:
        case GL_ALPHA32F_ARB:
        case GL_INTENSITY32F_ARB:
        case GL_LUMINANCE32F_ARB:
        case GL_LUMINANCE_ALPHA16F_ARB:
        case GL_RGBA8UI_EXT:
        case GL_ALPHA32UI_EXT:
        case GL_INTENSITY32UI_EXT:
        case GL_LUMINANCE32UI_EXT:
        case GL_LUMINANCE_ALPHA16UI_EXT:
        case GL_LUMINANCE_ALPHA16I_EXT:
        case GL_INTENSITY32I_EXT:
        case GL_LUMINANCE32I_EXT:
        case GL_ALPHA32I_EXT:
        case GL_RGBA8I_EXT:
        case GL_RGB9_E5_EXT:
        case GL_R32F:
        case GL_RG16F:
        case GL_DEPTH24_STENCIL8:
        case GL_R32I:
        case GL_R32UI:
        case GL_RG16I:
        case GL_RG16UI:
        {
            pixelSizeInBits = 32;
        }
        break;

        case GL_LUMINANCE_ALPHA:
        case GL_SLUMINANCE_ALPHA:
        {
            pixelSizeInBits = GT_BITS_PER_BYTE * 2;
        }
        break;

        case GL_RGB:
        case GL_SRGB:
        {
            pixelSizeInBits = GT_BITS_PER_BYTE * 3;
        }
        break;

        case GL_RGB5:
        {
            pixelSizeInBits  = 15;
        }
        break;

        case GL_RGB10:
        {
            pixelSizeInBits = 30;
        }
        break;

        case GL_RGB12:
        {
            pixelSizeInBits = 36;
        }
        break;

        case GL_RGB16:
        case GL_RGBA12:
        case GL_RGB_FLOAT16_ATI:
        case GL_RGB16UI_EXT:
        {
            pixelSizeInBits = 48;
        }
        break;

        case GL_RGBA:
        case GL_SRGB_ALPHA:
        {
            pixelSizeInBits = GT_BITS_PER_BYTE * 4;
        }
        break;

        case GL_RGBA16:
        case GL_RGBA16F:
        case GL_LUMINANCE_ALPHA32F_ARB:
        case GL_RGBA16UI_EXT:
        case GL_RGBA16I_EXT:
        case GL_LUMINANCE_ALPHA32UI_EXT:
        case GL_LUMINANCE_ALPHA32I_EXT:
        case GL_RGB16I_EXT:
        case GL_RG32F:
        case GL_RG32I:
        case GL_RG32UI:
        {
            pixelSizeInBits = 64;
        }
        break;

        case GL_RGB32F:
        case GL_RGB32UI_EXT:
        case GL_RGB32I_EXT:
        {
            pixelSizeInBits = 96;
        }
        break;

        case GL_RGBA32F_ARB:
        case GL_RGBA32UI_EXT:
        case GL_RGBA32I_EXT:
        {
            pixelSizeInBits = 128;
        }
        break;

        default:
        {
            // Pixel size is not set yet:
            pixelSizeInBits = 0;
            retVal = false;
        }
        break;
    }

    return retVal;
}
bool apGetChannelCountByInternalFormat(GLenum internalFormat, int& channels)
{
    bool retVal = true;

    switch (internalFormat)
    {
        // Undefined, compressed, etc.: no channels
        case 0:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
        case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
#endif
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
        case GL_PALETTE4_R5_G6_B5_OES:
        case GL_PALETTE4_RGBA4_OES:
        case GL_PALETTE4_RGB5_A1_OES:
        case GL_PALETTE4_RGB8_OES:
        case GL_PALETTE4_RGBA8_OES:
#endif
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
        case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
#endif
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
        case GL_PALETTE8_R5_G6_B5_OES:
        case GL_PALETTE8_RGBA4_OES:
        case GL_PALETTE8_RGB5_A1_OES:
        case GL_PALETTE8_RGB8_OES:
        case GL_PALETTE8_RGBA8_OES:
#endif
        case GL_COMPRESSED_ALPHA:
        case GL_COMPRESSED_LUMINANCE:
        case GL_COMPRESSED_LUMINANCE_ALPHA:
        case GL_COMPRESSED_INTENSITY:
        case GL_COMPRESSED_RGB:
        case GL_COMPRESSED_RGBA:
        case GL_COMPRESSED_RED_RGTC1:
        case GL_COMPRESSED_SIGNED_RED_RGTC1:
        case GL_COMPRESSED_RG_RGTC2:
        case GL_COMPRESSED_SIGNED_RG_RGTC2:
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_RGB_FXT1_3DFX:
        case GL_COMPRESSED_RGBA_FXT1_3DFX:
        case GL_COMPRESSED_SRGB:
        case GL_COMPRESSED_SRGB_ALPHA:
        case GL_COMPRESSED_SLUMINANCE:
        case GL_COMPRESSED_SLUMINANCE_ALPHA:
        case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_RED:
        case GL_COMPRESSED_RG:
        {
            // Internal format was not set yet:
            channels = 0;
        }
        break;

        // R, A, L, D, S, Index, etc.: 1 channel
        case 1:
        case GL_COLOR_INDEX1_EXT:
        case GL_COLOR_INDEX2_EXT:
        case GL_ALPHA:
        case GL_DEPTH_COMPONENT:
        case GL_LUMINANCE:
        case GL_INTENSITY:
        case GL_SLUMINANCE:
        case GL_ALPHA4:
        case GL_LUMINANCE4:
        case GL_INTENSITY4:
        case GL_COLOR_INDEX4_EXT:
        case GL_ALPHA8:
        case GL_LUMINANCE8:
        case GL_INTENSITY8:
        case GL_SLUMINANCE8:
        case GL_COLOR_INDEX8_EXT:
        case GL_ALPHA8UI_EXT:
        case GL_ALPHA8I_EXT:
        case GL_INTENSITY8UI_EXT:
        case GL_LUMINANCE8UI_EXT:
        case GL_INTENSITY8I_EXT:
        case GL_LUMINANCE8I_EXT:
        case GL_R8I:
        case GL_R8UI:
        case GL_ALPHA12:
        case GL_LUMINANCE12:
        case GL_INTENSITY12:
        case GL_COLOR_INDEX12_EXT:
        case GL_ALPHA16:
        case GL_DEPTH_COMPONENT16:
        case GL_LUMINANCE16:
        case GL_INTENSITY16:
        case GL_COLOR_INDEX16_EXT:
        case GL_ALPHA16F_ARB:
        case GL_INTENSITY16F_ARB:
        case GL_LUMINANCE16F_ARB:
        case GL_ALPHA16UI_EXT:
        case GL_INTENSITY16UI_EXT:
        case GL_LUMINANCE16UI_EXT:
        case GL_ALPHA16I_EXT:
        case GL_INTENSITY16I_EXT:
        case GL_LUMINANCE16I_EXT:
        case GL_R16F:
        case GL_R16I:
        case GL_R16UI:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
        case GL_ALPHA32F_ARB:
        case GL_INTENSITY32F_ARB:
        case GL_LUMINANCE32F_ARB:
        case GL_ALPHA32UI_EXT:
        case GL_INTENSITY32UI_EXT:
        case GL_LUMINANCE32UI_EXT:
        case GL_INTENSITY32I_EXT:
        case GL_LUMINANCE32I_EXT:
        case GL_ALPHA32I_EXT:
        case GL_R32F:
        case GL_R32I:
        case GL_R32UI:
        {
            // Internal format was not set yet:
            channels = 1;
        }
        break;

        // RG, RA, LA, Rx, sR, DS, etc.: 2 channels
        case 2:
        case GL_LUMINANCE4_ALPHA4:
        case GL_LUMINANCE6_ALPHA2:
        case GL_LUMINANCE8_ALPHA8:
        case GL_LUMINANCE12_ALPHA4:
        case GL_SLUMINANCE8_ALPHA8:
        case GL_LUMINANCE_ALPHA8UI_EXT:
        case GL_LUMINANCE_ALPHA8I_EXT:
        case GL_RG8I:
        case GL_RG8UI:
        case GL_LUMINANCE12_ALPHA12:
        case GL_LUMINANCE16_ALPHA16:
        case GL_LUMINANCE_ALPHA16F_ARB:
        case GL_LUMINANCE_ALPHA16UI_EXT:
        case GL_LUMINANCE_ALPHA16I_EXT:
        case GL_RG16F:
        case GL_DEPTH24_STENCIL8:
        case GL_RG16I:
        case GL_RG16UI:
        case GL_LUMINANCE_ALPHA:
        case GL_SLUMINANCE_ALPHA:
        case GL_LUMINANCE_ALPHA32F_ARB:
        case GL_LUMINANCE_ALPHA32UI_EXT:
        case GL_LUMINANCE_ALPHA32I_EXT:
        case GL_RG32F:
        case GL_RG32I:
        case GL_RG32UI:
        {
            // Internal format was not set yet:
            channels = 2;
        }
        break;

        // RGB, RGA, RGx, etc.: 3 channels
        case 3:
        case GL_R3_G3_B2:
        case GL_RGB4:
        case GL_RGB8:
        case GL_SRGB8:
        case GL_RGB8UI:
        case GL_RGB8I:
        case GL_RGB:
        case GL_SRGB:
        case GL_RGB5:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16:
        case GL_RGB16F:
        case GL_RGB16UI:
        case GL_RGB16I:
        case GL_RGB32F:
        case GL_RGB32UI:
        case GL_RGB32I:
        {
            // Internal format was not set yet:
            channels = 3;
        }
        break;

        // RGBA, RGBx, etc.: 4 channels
        case 4:
        case GL_RGBA2:
        case GL_RGBA4:
        case GL_RGB5_A1:
        case GL_RGB10_A2:
        case GL_RGBA8:
        case GL_SRGB8_ALPHA8:
        case GL_RGBA8UI:
        case GL_RGB9_E5:
        case GL_RGBA8I:
        case GL_RGBA12:
        case GL_RGBA:
        case GL_SRGB_ALPHA:
        case GL_RGBA16:
        case GL_RGBA16F:
        case GL_RGBA16UI:
        case GL_RGBA16I:
        case GL_RGBA32F:
        case GL_RGBA32UI:
        case GL_RGBA32I:
        {
            // Internal format was not set yet:
            channels = 4;
        }
        break;

        default:
        {
            // Pixel size is not set yet:
            channels = -1;
            retVal = false;
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGetIsInternalFormatCompressed
// Description: Return true if internalFormat is one of the compressed internal formats
// Arguments: GLenum internalFormat
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/10/2008
// ---------------------------------------------------------------------------
bool apGetIsInternalFormatCompressed(GLenum internalFormat)
{
    bool retVal = false;

    switch (internalFormat)
    {
        case GL_COMPRESSED_ALPHA:
        case GL_COMPRESSED_LUMINANCE:
        case GL_COMPRESSED_LUMINANCE_ALPHA:
        case GL_COMPRESSED_INTENSITY:
        case GL_COMPRESSED_RGB:
        case GL_COMPRESSED_RGBA:
        case GL_COMPRESSED_RED_RGTC1:
        case GL_COMPRESSED_SIGNED_RED_RGTC1:
        case GL_COMPRESSED_RG_RGTC2:
        case GL_COMPRESSED_SIGNED_RG_RGTC2:
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_RGB_FXT1_3DFX:
        case GL_COMPRESSED_RGBA_FXT1_3DFX:
        case GL_COMPRESSED_SRGB_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_EXT:
        case GL_COMPRESSED_SLUMINANCE_EXT:
        case GL_COMPRESSED_SLUMINANCE_ALPHA_EXT:
        case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
        case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
        case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))
        case GL_PALETTE4_R5_G6_B5_OES:
        case GL_PALETTE4_RGBA4_OES:
        case GL_PALETTE4_RGB5_A1_OES:
        case GL_PALETTE4_RGB8_OES:
        case GL_PALETTE4_RGBA8_OES:
        case GL_PALETTE8_R5_G6_B5_OES:
        case GL_PALETTE8_RGBA4_OES:
        case GL_PALETTE8_RGB5_A1_OES:
        case GL_PALETTE8_RGB8_OES:
        case GL_PALETTE8_RGBA8_OES:
#endif
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:
        case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG:
        case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:
        case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG:
#endif
            {
                retVal = true;
            }
            break;

        default:
        {
            retVal = false;
        }
        break;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        apGetVBOFormatFromTextureBufferFormat
// Description: Translate a texture buffer format to the VBO format
// Arguments: GLenum texBufferInternalFormat
//            oaTexelDataFormat& vboFormat
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        10/8/2009
// ---------------------------------------------------------------------------
bool apGetVBOFormatFromTextureBufferFormat(GLenum texBufferInternalFormat, oaTexelDataFormat& vboFormat)
{
    bool retVal = true;

    switch (texBufferInternalFormat)
    {
        case GL_ALPHA8:
        case GL_LUMINANCE8:
        case GL_INTENSITY8:
            vboFormat = OA_TEXEL_FORMAT_V1UB;
            break;

        case GL_ALPHA16:
        case GL_LUMINANCE16:
        case GL_ALPHA16UI_EXT:
        case GL_LUMINANCE16UI_EXT:
        case GL_INTENSITY16UI_EXT:
        case GL_INTENSITY16:
            vboFormat = OA_TEXEL_FORMAT_V1US;
            break;

        case GL_ALPHA16F_ARB:
        case GL_LUMINANCE16F_ARB:
        case GL_LUMINANCE_ALPHA16F_ARB:
        case GL_INTENSITY16F_ARB:
        case GL_RGBA16F_ARB:
        {
            // TO_DO: OpenGL3.1 support half data type format
            GT_ASSERT_EX(false, L"Unsupported texture buffer data type: half");
            retVal = false;
            break;
        }

        case GL_ALPHA32F_ARB:
        case GL_LUMINANCE32F_ARB:
        case GL_INTENSITY32F_ARB:
            vboFormat = OA_TEXEL_FORMAT_V1F;
            break;

        case GL_ALPHA8I_EXT:
        case GL_LUMINANCE8I_EXT:
        case GL_INTENSITY8I_EXT:
            vboFormat = OA_TEXEL_FORMAT_V1B;
            break;

        case GL_ALPHA16I_EXT:
        case GL_INTENSITY16I_EXT:
        case GL_LUMINANCE16I_EXT:
            vboFormat = OA_TEXEL_FORMAT_V1S;
            break;

        case GL_LUMINANCE_ALPHA16I_EXT:
            vboFormat = OA_TEXEL_FORMAT_V2S;
            break;


        case GL_ALPHA32I_EXT:
        case GL_LUMINANCE32I_EXT:
        case GL_INTENSITY32I_EXT:
            vboFormat = OA_TEXEL_FORMAT_V1I;
            break;

        case GL_ALPHA8UI_EXT:
        case GL_LUMINANCE8UI_EXT:
        case GL_INTENSITY8UI_EXT:
            vboFormat = OA_TEXEL_FORMAT_V1UB;
            break;

        case GL_ALPHA32UI_EXT:
        case GL_LUMINANCE32UI_EXT:
        case GL_INTENSITY32UI_EXT:
            vboFormat = OA_TEXEL_FORMAT_V1UI;
            break;

        case GL_LUMINANCE8_ALPHA8:
        case GL_LUMINANCE_ALPHA8UI_EXT:
            vboFormat = OA_TEXEL_FORMAT_V2UB;
            break;

        case GL_LUMINANCE16_ALPHA16:
        case GL_LUMINANCE_ALPHA16UI_EXT:
            vboFormat = OA_TEXEL_FORMAT_V2US;
            break;

        case GL_LUMINANCE_ALPHA32F_ARB:
            vboFormat = OA_TEXEL_FORMAT_V2F;
            break;

        case GL_LUMINANCE_ALPHA8I_EXT:
            vboFormat = OA_TEXEL_FORMAT_V2B;
            break;

        case GL_LUMINANCE_ALPHA32I_EXT:
            vboFormat = OA_TEXEL_FORMAT_V2I;
            break;

        case GL_LUMINANCE_ALPHA32UI_EXT:
            vboFormat = OA_TEXEL_FORMAT_V2UI;
            break;

        case GL_RGBA8:
        case GL_RGBA8UI_EXT:
            vboFormat = OA_TEXEL_FORMAT_V4UB;
            break;

        case GL_RGBA16:
        case GL_RGBA16UI_EXT:
            vboFormat = OA_TEXEL_FORMAT_V4US;
            break;

        case GL_RGBA32F_ARB:
            vboFormat = OA_TEXEL_FORMAT_V4F;
            break;

        case GL_RGBA8I_EXT:
            vboFormat = OA_TEXEL_FORMAT_V4UB;
            break;

        case GL_RGBA16I_EXT:
            vboFormat = OA_TEXEL_FORMAT_V4S;
            break;

        case GL_RGBA32I_EXT:
            vboFormat = OA_TEXEL_FORMAT_V4I;
            break;

        case GL_RGBA32UI_EXT:
            vboFormat = OA_TEXEL_FORMAT_V4UI;
            break;

        default:
        {
            GT_ASSERT_EX(false, L"Unsupported texture buffer data type");
            retVal = false;
            break;
        }

    }

    return retVal;
}

