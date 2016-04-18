//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsTextureSerializer.h
///
//==================================================================================

//------------------------------ gsTextureSerializer.h ------------------------------

#ifndef __GSTEXTURESERIALIZER
#define __GSTEXTURESERIALIZER

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTAPIClasses/Include/apTextureType.h>
#include <AMDTAPIClasses/Include/apFileType.h>

// Local:
#include <src/gsImageWriter.h>
#include <src/gsGLTexture.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsTextureSerializer : public gsImageWriter
// General Description: Stores an input texture into a file.
// Author:               Yaki Tebeka
// Creation Date:        27/12/2004
// ----------------------------------------------------------------------------------
class gsTextureSerializer : public gsImageWriter
{
public:
    gsTextureSerializer(GLenum bindTarget, GLint mipMapLevel = 0);
    virtual ~gsTextureSerializer();

    bool saveRawDataToFile(const gsGLTexture* pTextureObj, const osFilePath& filePath);

private:
    // Describes the format in which the texture data is retrieved from the
    // graphic board:
    enum gsRetrievedDataFormat
    {
        GS_RGBA_TEX_IMAGE,      // The retrieved texture data fills the RGBA components.
        GS_ALPHA_TEX_IMAGE,     // The retrieved texture data fills only the alpha components.
        GS_GRAY_SCALE_TEX_IMAGE // The retrieved texture data is a gray-scale image that fills
        // the Red (and maybe also alpha) component.
    };

    void getTextureParameters();
    bool copyOpenGLTextureToBitmap(void* pBitmap, const gsGLTexture* pTextureObj, oaTexelDataFormat dataFormat = OA_TEXEL_FORMAT_BGRA, oaDataType componentDataType = OA_UNSIGNED_BYTE);
    bool allocateOpenGL3DImageCopy();
    gsRetrievedDataFormat retrievedTextureDataFormat();
    gtByte* mirrorFlipTexture(const gtByte* pRawData, oaTexelDataFormat dataFormat, oaDataType componentDataType);
    bool readTextureViaFBO(GLenum pixelFormat, GLenum texelsType, void* pBitmap);
    bool readTextureAttachedToFBO(GLenum pixelFormat, GLenum texelsType, void* pBitmap, const apGLTexture* pTextureObj);

    // Do not allow to use my default constructor:
    gsTextureSerializer();

private:
    // The bind target who's bounded texture will be serialized:
    GLuint _bindTarget;

    // The OpenGL Name of the texture:
    GLuint _textureName;

    // The Mip map LOD level that will be serialized:
    GLint _mipMapLevel;

    // The format in which the texture is stored on the Graphic board:
    GLint _internalFormat;

    // The texture type:
    apTextureType _textureType;

    // A copy of the OpenGL 3D image:
    GLubyte* _pOpenGL3DImage;

    // The OpenGL name of an FBO used to read textures in the iPhone implementation:
    bool _isGL_OES_framebuffer_objectSupported;
    bool _isOpenGLES20Supported;
    GLuint _helperFBOName;

    // Function pointers for framebuffer extension:
    PFNGLCHECKFRAMEBUFFERSTATUSPROC _glCheckFramebufferStatus;
    PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC _glCheckFramebufferStatusExt;

};


#endif  // __GSTEXTURESERIALIZER
