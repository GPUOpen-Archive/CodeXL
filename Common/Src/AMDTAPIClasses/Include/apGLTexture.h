//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLTexture.h
///
//==================================================================================

//------------------------------ apGLTexture.h ------------------------------

#ifndef __APGLTEXTURE
#define __APGLTEXTURE

// Pre decelerations:
class apParameter;
class apOpenGLParameter;
struct ap2DRectangle;

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaDataType.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apAllocatedObject.h>
#include <AMDTAPIClasses/Include/apDisplayBuffer.h>
#include <AMDTAPIClasses/Include/apFileType.h>
#include <AMDTAPIClasses/Include/apGLTextureMipLevel.h>
#include <AMDTAPIClasses/Include/apGLTextureMiplevelData.h>
#include <AMDTAPIClasses/Include/apGLTextureParams.h>
#include <AMDTAPIClasses/Include/apTextureType.h>



// ----------------------------------------------------------------------------------
// Struct:           apGLTextureMipLevelID
// General Description: Is used for identifying a texture mip level.
// Author:  AMD Developer Tools Team
// Creation Date:        12/1/2009
// ----------------------------------------------------------------------------------
struct apGLTextureMipLevelID
{
    // The texture name:
    GLuint _textureName;

    // Texture mip level:
    int _textureMipLevel;
};

// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLTexture : public osTransferableObject
// General Description:
//   Represents an OpenGL texture. Holds the texture data and parameters.
//
// Author:  AMD Developer Tools Team
// Creation Date:        19/12/2004
// ----------------------------------------------------------------------------------
class AP_API apGLTexture : public apAllocatedObject
{
public:
    apGLTexture(GLuint textureName = 0);
    apGLTexture(const apGLTexture& other);
    virtual ~apGLTexture();
    apGLTexture& operator=(const apGLTexture& other);

    // Texture name:
    GLuint textureName() const { return _textureName; };

    // Texture type;
    apTextureType textureType() const { return _textureType; };

    // Dimensions:
    bool setDimensions(GLsizei width, GLsizei height, GLsizei depth, GLsizei borderWidth, int mipLevel = 0);
    bool getDimensions(GLsizei& width, GLsizei& height, GLsizei& depth, GLsizei& borderWidth, int mipLevel = 0) const;

    // Crop rectangle:
    void setCropRectangle(const ap2DRectangle& rectangle);
    bool getCropRectangle(ap2DRectangle& rectangle) const;

    // Texture parameters:
    void addDefaultGLTextureParameters();
    void addDefaultGLESTextureParameters();

    void updateTextureDataFile(GLenum bindTarget, const osFilePath& textureDataFilePath, int mipLevel = 0);

    // Texture file paths:
    bool getTextureDataFilePath(const osFilePath*& pFilePath, int fileIndex = apGLTextureMipLevel::AP_SINGLE_TEXTURE_FACE_INDEX, int mipLevel = 0) const;
    bool getTextureImageFilePath(apFileType fileType, const osFilePath*& pFilePath, int fileIndex = apGLTextureMipLevel::AP_SINGLE_TEXTURE_FACE_INDEX, int mipLevel = 0);

    // Texel type, internal format and pixel format:
    GLenum texelsType(int mipLevel = 0) const ;
    GLenum pixelFormat(int mipLevel = 0) const ;
    GLint requestedInternalPixelFormat(int mipLevel = 0) const ;
    GLenum usedInternalPixelFormat(int mipLevel = 0) const ;
    void setTextureFormats(GLenum bindTarget, GLint internalPixelFormat, GLenum pixelFormat, GLenum texelsType, int mipLevel = 0);
    void setTextureType(apTextureType texType);

    int amountOfTextureDataFiles() const;

    // Get & Set the buffer FBO name
    GLuint getFBOName() const;
    void setFBOName(GLuint fboName);

    // Get and set CGL pbuffer information:
    int getPBufferName() const;
    void setPBufferName(int pbuffer);
    apDisplayBuffer getPBufferStaticBuffer() const;
    void setPBufferStaticBuffer(apDisplayBuffer staticBuffer);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    bool writeThumbnailDataToChannel(osChannel& ipcChannel) const;
    bool writeTextureDataToChannel(osChannel& ipcChannel) const;
    bool writeTextureMemoryDataToChannel(osChannel& ipcChannel) const;

    bool getTextureThumbnailData(apGLTextureMiplevelData& textureThumbData) const;

    bool addTextureMipLevel(GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type);
    bool getTextureMinMaxLevels(GLuint& minLevel, GLuint& maxLevel) const;
    virtual apTextureMipMapType getTextureMipmapType() const;

    // Handle Mip Levels' allocation status:
    int getTextureMipLevelAllocatedObjectId(GLint level);
    void setTextureMipLevelAllocatedObjectId(GLint level, int allocatedId);

    // Memory size:
    bool getMemorySize(gtSize_t& memorySize, bool& isMemoryEstimated) const;
    bool getEstimatedMemorySize(gtSize_t& memorySize) const;

    // Texture parameters:
    const apGLTextureParams& textureParameters()const {return _textureParameters;};
    apGLTextureParams& textureParameters() {return _textureParameters;};

    // Texture parameters:
    const apGLTextureParams* textureLevelParameters(int level = 0, apGLTextureMipLevel::apTextureFaceIndex texturefaceIndex = apGLTextureMipLevel::AP_SINGLE_TEXTURE_FACE_INDEX) const;
    apGLTextureParams* textureLevelParameters(int level = 0, apGLTextureMipLevel::apTextureFaceIndex texturefaceIndex = apGLTextureMipLevel::AP_SINGLE_TEXTURE_FACE_INDEX);

    // Texture buffer:
    void setBufferName(GLint buffer) {_bufferName = buffer;}
    GLuint bufferName() {return _bufferName;}

    // Texture buffer internal format:
    void setBufferInternalFormat(GLenum internalformat);
    GLenum bufferInternalFormat() {return _bufferInternalFormat;};

    // Return the object holds one of the texture mip level:
    apGLTextureMipLevel* getTextureMipLevel(int mipLevel);
    const apGLTextureMipLevel* getTextureMipLevel(int mipLevel) const;

    // Returns the object holds the miplevel'th (logic) mip level:
    apGLTextureMipLevel* getTextureLogicMipLevel(int mipLevel);

    // Compression rate:
    bool getCompressionRate(float& compressRate) const;

    // Texture dirty flag:
    void markTextureAsDirty(GLenum bindTarget);
    void markTextureAsDirty(GLenum bindTarget, int level);

    virtual void markAllTextureImagesAsUpdated(int level);
    virtual bool dirtyTextureImageExists(int level) const;

    virtual void markAllTextureRawDataAsUpdated(int level);
    virtual bool dirtyTextureRawDataExists(int level) const;

    // TO_DO: OpenGL3.1 should this function be here?
    static bool textureBufferFormatToDataType(GLenum openglDataFormat, oaDataType& dataType);

    // OpenCL interoperability:
    bool shareTextureMiplevelWithCLImage(int clImageIndex, int clImageName, int clSpyID, int textureMipLevel) ;
    void getCLImageDetails(int& clImageIndex, int& openCLImageName, int& clSpyID) const {openCLImageName = _openCLImageName; clImageIndex = _openCLImageIndex; clSpyID = _openCLSpyID;}

private:
    void clear();

protected:

    // Auto generated mip-levels:
    bool getAutoGeneratedMiplevelMemorySize(gtSize_t& autoGeneratedMiplevelsSize, bool& isEstimated)const;
    bool calcNumberOfAutoGeneratedMiplevels(GLuint& baseLevel, GLuint& maxLevel, bool& isEstimated)const;
    virtual bool getMipmapBaseMaxLevels(GLuint& baseLevel, GLuint& maxLevel) const;

private:
    // The OpenGL texture name:
    GLuint _textureName;

    // The texture type:
    apTextureType _textureType;

    // The texture crop rectangle (if any):
    // (See OES_draw_texture extension documentation about crop rectangles)
    ap2DRectangle* _pCropRectangle;

    // The name of the FBO to which this texture is bound:
    GLuint _fboName;

    // In CGL, PBuffers can work in a manner similar to FBOs:
    int _pbufferName;
    apDisplayBuffer _pbufferStaticBuffer;

    // For texture buffer - the buffer attached to the texture:
    GLuint _bufferName;
    GLenum _bufferInternalFormat;

protected:
    // OpenGL ES textures don't have several parameters which we use:
    bool _isOpenGLESTexture;

    // Texture OpenGL parameters:
    apGLTextureParams _textureParameters;

    // Vector contain the texture mipmap level descriptions:
    gtPtrVector<apGLTextureMipLevel*> _textureMipLevels;

    // Contain the OpenCL image id shared with the miplevel 0 of this texture:
    int _openCLImageIndex;

    // Contain the OpenCL image name shared with the miplevel 0 of this texture:
    int _openCLImageName;

    // Contain the OpenCL spy ID:
    int _openCLSpyID;

};
#endif  // __APGLTEXTURE
