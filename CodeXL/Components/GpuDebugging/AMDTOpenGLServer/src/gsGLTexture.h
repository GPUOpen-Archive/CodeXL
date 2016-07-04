//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsGLTexture.h
///
//==================================================================================

//------------------------------ gsGLTexture.h ------------------------------

#ifndef __GSGLTEXTURE_H
#define __GSGLTEXTURE_H

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>


// ----------------------------------------------------------------------------------
// Class Name:           gsGLTexture : public apGLTexture
// General Description:
//   Represents an OpenGL texture.
//   Adds spy related data to apGLTexture.
//
// Author:               Yaki Tebeka
// Creation Date:        29/6/2006
// ----------------------------------------------------------------------------------
class gsGLTexture : public apGLTexture
{
public:
    gsGLTexture(GLuint textureName = 0);
    gsGLTexture(const gsGLTexture& other);
    virtual ~gsGLTexture();
    gsGLTexture& operator=(const gsGLTexture& other);

    // Is the texture as bounded to the active FBO:
    void markTextureAsBoundToActiveFBO(bool bounded) {_isBoundToActiveFBO = bounded;};
    bool isTextureBoundToActiveFBO() const { return _isBoundToActiveFBO; };

    // Updates the OpenGL parameters for each of the texture mip levels:
    bool updateTextureParameters(bool shouldUpdateOnlyMemoryParams, bool isOpenGL31CoreContext);
    void markAllParametersAsUpdated(bool isUpdated);
    void setNewMipLevelsAllocIds();

    // Mipmap levels auto generation:
    void setMipmapAutoGeneration(bool shouldAutoGenerateMipmap) {_shouldAutoGenerateMipmap = shouldAutoGenerateMipmap;};
    virtual bool generateAutoMipmapLevels();
    void setMipmapBaseLevel(GLuint baseLevel) {_mipmapBaseLevel = baseLevel;}
    void setMipmapMaxLevel(GLuint maxLevel) {_mipmapMaxLevel = maxLevel;}
    virtual apTextureMipMapType getTextureMipmapType() const;
    virtual bool getMipmapBaseMaxLevels(GLuint& baseLevel, GLuint& maxLevel)const;

    // Dirty textures:
    virtual bool dirtyTextureRawDataExists(int level) const;
    virtual bool dirtyTextureImageExists(int level) const;

private:

    // Private parameters update functions:
    bool updateTextureParameters(GLenum bindTarget, bool shouldUpdateOnlyMemoryParams, bool isOpenGL31CoreContext);
    bool updateTextureMipLevelsParameters(GLenum bindTarget, bool shouldUpdateOnlyMemoryParams, bool isOpenGL31CoreContext);
    bool shouldFailOnTextureParameterUpdate(const apGLTextureParams& textureParams, GLenum paramName) const;
    bool isDeprecatedTexParam(GLenum paraName) const;
    bool isMemoryParameter(GLenum parameterName);
    bool isTextureCompressed(const apGLTextureParams& textureParams, bool& isCompressed) const;

private:
    // This flag is on if the texture is bounded to the active FBO:
    // (this means that the texture may be rendered into)
    bool _isBoundToActiveFBO;

    // Should the texture levels be auto generated:
    bool _shouldAutoGenerateMipmap;

    // Base level for mipmap generation:
    GLuint _mipmapBaseLevel;

    // Max level for mipmap generation:
    GLuint _mipmapMaxLevel;
};

#endif //__GSGLTEXTURE_H

