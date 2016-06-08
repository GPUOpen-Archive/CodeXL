//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLTextureMiplevelData.h
///
//==================================================================================

//------------------------------ apGLTextureMiplevelData.h ------------------------------

#ifndef __APGLTEXTUREMIPLEVELDATA
#define __APGLTEXTUREMIPLEVELDATA

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>
#include <AMDTAPIClasses/Include/apDisplayBuffer.h>

// Local:
#include <AMDTAPIClasses/Include/apTextureType.h>

class apGLTexture;

class AP_API apGLTextureData : public osTransferableObject
{
    friend class apGLTexture;
public:
    apGLTextureData();
    virtual ~apGLTextureData();
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;

    // Get the transferable object type:
    virtual osTransferableObjectType type()const;

    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    int minLevel()const {return _minLevel;};
    int maxLevel()const {return _maxLevel;};

    apTextureType textureType()const {return _textureType;};
    GLuint textureName()const {return _textureName;};

    // OpenCL interoperability:
    void getCLDetails(int& openCLImageIndex, int& openCLImageName, int& openCLSpyID) const { openCLImageIndex = _openCLImageIndex; openCLImageName = _openCLImageName; openCLSpyID = _openCLSpyID;}
    int openCLImageIndex() const {return _openCLImageIndex;}
    int openCLImageName() const {return _openCLImageName;}

private:

    apGLTextureData(const apGLTextureData&) {};
    bool operator=(const apGLTextureData&) { return false; };

    // The OpenGL texture name:
    GLuint _textureName;

    // The texture type:
    apTextureType _textureType;

    // Amount of texture mip levels:
    GLuint _minLevel;
    GLuint _maxLevel;

    // OpenCL interoperability:
    int _openCLImageIndex;
    int _openCLImageName;
    int _openCLSpyID;
};

class AP_API apGLTextureMemoryData : public osTransferableObject
{
    friend class apGLTexture;
public:
    apGLTextureMemoryData();
    virtual ~apGLTextureMemoryData();
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;

    // Get the transferable object type:
    virtual osTransferableObjectType type()const;

    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Mip levels:
    int minLevel()const {return _minLevel;};
    int maxLevel()const {return _maxLevel;};

    // Texture type:
    apTextureType textureType()const {return _textureType;};

    // Texture name:
    GLuint textureName()const {return _textureName;};

    // Texture dimensions:
    void getDimensions(GLsizei& width, GLsizei& height, GLsizei& depth) const;

    // Memory size:
    bool getMemorySize(gtSize_t& memorySize, bool& isMemorySizeEstimated) const;

    // Texture internal format:
    GLenum bufferInternalFormat() const {return _bufferInternalFormat;};
    GLenum usedInternalPixelFormat() const {return _usedInternalPixelFormat;};
    GLenum requestedInternalPixelFormat() const {return _requestedInternalPixelFormat;}

    // Texture mipmap type:
    apTextureMipMapType getTextureMipmapType() const {return _mipmapType;};

    // OpenCL interoperability:
    void getCLDetails(int& openCLImageIndex, int& openCLImageName, int& openCLSpyID) const { openCLImageIndex = _openCLImageIndex; openCLImageName = _openCLImageName; openCLSpyID = _openCLSpyID;}
    int openCLImageIndex() const {return _openCLImageIndex;}
    int openCLImageName() const {return _openCLImageName;}


private:

    apGLTextureMemoryData(const apGLTextureMemoryData&) {};
    bool operator=(const apGLTextureMemoryData&) { return false; };

    // The OpenGL texture name:
    GLuint _textureName;

    // The texture type:
    apTextureType _textureType;

    // Amount of texture mip levels:
    GLuint _minLevel;
    GLuint _maxLevel;

    // The texture mip level requested width, height and depth (as was asked on mip level generation):
    GLsizei _width;
    GLsizei _height;
    GLsizei _depth;

    // Memory size:
    gtSize_t _memorySize;
    bool _isMemoryEstimated;
    bool _isMemoryCalculated;

    // Internal format:
    GLenum _bufferInternalFormat;
    GLenum _usedInternalPixelFormat;
    GLenum _requestedInternalPixelFormat;

    // Texture mipmap type:
    apTextureMipMapType _mipmapType;

    // OpenCL interoperability:
    int _openCLImageIndex;
    int _openCLImageName;
    int _openCLSpyID;
};


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLTextureMiplevelData : public osTransferableObject
// General Description: Is used for texture thumbnail data write. When we only want to
//                      display a texture thumbnail, we use this class to pass the information
// Author:  AMD Developer Tools Team
// Creation Date:        20/11/2008
// ----------------------------------------------------------------------------------
class AP_API apGLTextureMiplevelData : public osTransferableObject
{
    friend class apGLTexture;
public:
    apGLTextureMiplevelData();
    virtual ~apGLTextureMiplevelData();
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;

    // Get the transferable object type:
    virtual osTransferableObjectType type()const;

    virtual bool readSelfFromChannel(osChannel& ipcChannel);
    int amountOfTextureDataFiles() const;

    int minLevel()const {return _minLevel;};
    int maxLevel()const {return _maxLevel;};

    apTextureType textureType()const {return _textureType;};
    GLuint textureName()const {return _textureName;};

    // Internal pixel format:
    GLenum requestedInternalPixelFormat() const { return _requestedInternalPixelFormat;};

    // Texel format:
    oaTexelDataFormat texelDataFormat() const { return _texelDataFormat;};
    oaDataType dataType() const { return _dataType;};

    GLuint fboName()const {return _fboName;};
    void getDimensions(GLsizei& width, GLsizei& height, GLsizei& depth, GLsizei& borderSize)const;

    // Get and set CGL pbuffer information:
    int getPBufferName() const;
    void setPBufferName(int pbuffer);
    apDisplayBuffer getPBufferStaticBuffer() const;
    void setPBufferStaticBuffer(apDisplayBuffer staticBuffer);

    GLuint bufferName()const {return _bufferName;}
    GLenum bufferInternalFormat()const {return _bufferInternalFormat;}

    // OpenCL interoperability:
    void getCLDetails(int& openCLImageIndex, int& openCLImageName, int& openCLSpyID) const { openCLImageIndex = _openCLImageIndex; openCLImageName = _openCLImageName; openCLSpyID = _openCLSpyID;}
    int openCLImageIndex() const {return _openCLImageIndex;}
    int openCLImageName() const {return _openCLImageName;}

private:

    apGLTextureMiplevelData(const apGLTextureMiplevelData&) {};
    bool operator=(const apGLTextureMiplevelData&) { return false; };

    // The OpenGL texture name:
    GLuint _textureName;

    // The texture type:
    apTextureType _textureType;

    // Texture FBO name:
    GLuint _fboName;

    // Contain the requested internal pixel format as GLEnum:
    GLenum _requestedInternalPixelFormat;

    // Texture data format (for mip level 0):
    oaTexelDataFormat _texelDataFormat;
    oaDataType _dataType;

    // The texture mip level requested width, height and depth (as was asked on mip level generation):
    GLsizei _width;
    GLsizei _height;
    GLsizei _depth;

    // The width of the texture border:
    GLint _borderWidth;

    // Amount of texture mip levels:
    GLuint _minLevel;
    GLuint _maxLevel;

    // PBuffers (CGL):
    int _pbufferName;
    apDisplayBuffer _pbufferStaticBuffer;

    // Texture buffer:
    // For texture buffer - the buffer attached to the texture:
    GLuint _bufferName;
    GLenum _bufferInternalFormat;

    // OpenCL interoperability:
    int _openCLImageIndex;
    int _openCLImageName;
    int _openCLSpyID;
};

#endif  // __APGLTEXTUREMIPLEVELDATA



