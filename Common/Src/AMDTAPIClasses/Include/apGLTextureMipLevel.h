//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLTextureMipLevel.h
///
//==================================================================================

//------------------------------ apGLTextureMipLevel.h ------------------------------

#ifndef __APGLTEXTUREMIPLEVEL
#define __APGLTEXTUREMIPLEVEL

// Pre decelerations:
class apParameter;
class apOpenGLParameter;
struct ap2DRectangle;

// OpenGL
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apAllocatedObject.h>
#include <AMDTAPIClasses/Include/apTextureType.h>
#include <AMDTAPIClasses/Include/apGLTextureParams.h>
#include <AMDTAPIClasses/Include/apFileType.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLTextureMipLevel : public osTransferableObject
// General Description:
//   Represents an OpenGL texture.mipmap level. Holds the texture level data and parameters.
// Author:  AMD Developer Tools Team
// Creation Date:        5/10/2008
// ----------------------------------------------------------------------------------
class AP_API apGLTextureMipLevel : public apAllocatedObject
{
    friend class apGLTexture;
public:
    // Self functions:
    apGLTextureMipLevel();
    apGLTextureMipLevel(const apGLTextureMipLevel& other);
    virtual ~apGLTextureMipLevel();
    apGLTextureMipLevel& operator=(const apGLTextureMipLevel& other);

    // Defines the index of texture data files in the texture data files array:
    enum apTextureFaceIndex
    {
        AP_SINGLE_TEXTURE_FACE_INDEX = 0,
        AP_TEXTURE_CUBE_MAP_POSITIVE_X_FACE_INDEX = AP_SINGLE_TEXTURE_FACE_INDEX,
        AP_TEXTURE_CUBE_MAP_NEGATIVE_X_FACE_INDEX,
        AP_TEXTURE_CUBE_MAP_POSITIVE_Y_PANE_INDEX,
        AP_TEXTURE_CUBE_MAP_NEGATIVE_Y_PANE_INDEX,
        AP_TEXTURE_CUBE_MAP_POSITIVE_Z_PANE_INDEX,
        AP_TEXTURE_CUBE_MAP_NEGATIVE_Z_FACE_INDEX,
        AP_MAX_AMOUNT_OF_TEXTURE_FACES
    };

    static apTextureFaceIndex nextfaceIndex(apTextureFaceIndex faceIndex, apTextureType textureType);
    void addDefaultGLTextureLevelsParameters(apTextureType textureType);

    void updateTextureDataFile(GLenum bindTarget, const osFilePath& textureDataFilePath);

    const osFilePath* getTextureDataFilePath(int faceIndex = AP_SINGLE_TEXTURE_FACE_INDEX) const;
    void setDimensions(GLsizei width, GLsizei height, GLsizei depth, GLsizei borderWidth) { _requestedWidth = width; _requestedHeight = height; _requestedDepth = depth; _requestedBorderWidth = borderWidth; };
    bool getDimensions(GLsizei& width, GLsizei& height, GLsizei& depth, GLsizei& borderSize, apTextureFaceIndex faceIndex = AP_SINGLE_TEXTURE_FACE_INDEX)const;

    void setTextureFormats(GLint internalPixelFormat, GLenum pixelFormat, GLenum texelsType);
    GLenum getTexelsType() const { return _texelsType; };
    GLenum getPixelFormat() const { return _pixelFormat; };
    GLint getRequestedInternalPixelFormat() const { return _requestedInternalPixelFormat; };
    GLuint getUsedInternalPixelFormat(apTextureFaceIndex faceIndex = AP_SINGLE_TEXTURE_FACE_INDEX) const;
    bool getUsedInternalPixelFormat(GLuint& internalFormat, apTextureFaceIndex faceIndex = AP_SINGLE_TEXTURE_FACE_INDEX) const;

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Memory size:
    bool getMemorySize(gtSize_t& memorySize, bool& isMemoryEstimated, apTextureFaceIndex texturefaceIndex) const;
    bool getEstimatedMemorySize(gtSize_t& memorySize, apTextureFaceIndex faceIndex) const ;
    bool getMemorySize(gtSize_t& memorySize, bool& isMemoryEstimated, apTextureType textureType) const;

    static GLenum textureFaceIndexToBindTarget(apTextureFaceIndex faceIndex, GLenum originalBindTarget) ;

    // Dirty texture images:
    const gtVector<GLenum>& dirtyTextureImages() const { return _dirtyTextureImages; };
    const gtVector<GLenum>& getDirtyTextureData() const { return _dirtyRawData; };

    // OpenCL interoperability:
    void setSharedCLImage(int openCLImageName, int openCLImageIndex, int openCLSpyID) { _openCLImageName = openCLImageName; _openCLImageIndex = openCLImageIndex; _openCLSpyID = openCLSpyID;}

protected:
    // Contains the "dirty" texture images snapshot (as bind targets) of this texture mip-level.
    // (A texture image is considered "dirty" if it was changed in OpenGL, but its image snapshot,
    //  was not updated yet)
    // Notice:
    //   Texture images are images saved for the HTML log file browsing. They are --NOT-- thumbnails or images
    //   that are displayed in the texture viewer!!
    gtVector<GLenum> _dirtyTextureImages;

    // Contains the "dirty" raw data (as bind targets) of this texture mip-level.
    // (A texture raw data is considered "dirty" if it was changed in OpenGL, but its raw data snapshot,
    //  held by gsGLTexture::_textureDataFilesPaths, was not updated yet)
    gtVector<GLenum> _dirtyRawData;

private:
    bool getPixelSize(GLuint& pixelSize, bool& isEstimated, apTextureType textureType) const ;
    bool getPixelSize(GLuint& pixelSize, bool& isEstimated, apTextureFaceIndex faceIndex) const ;
    bool getDimensions(GLsizei& width, GLsizei& height, GLsizei& depth, GLsizei& borderSize, bool& areDimensionsEstimated, apTextureFaceIndex faceIndex = AP_SINGLE_TEXTURE_FACE_INDEX)const;

    int bindTargetToTextureFaceIndex(GLenum bindTarget) const;
    bool isTextureMipLevelCompressed(bool& isCompressed, apTextureFaceIndex faceIndex = AP_SINGLE_TEXTURE_FACE_INDEX) const;

    void markTextureAsDirty(GLenum bindTarget);


    // Dirty texture image preview mechanism
    void markAllTextureImagesAsUpdated() { _dirtyTextureImages.clear(); };
    bool dirtyTextureImageExists() const { return (0 < _dirtyTextureImages.size()); };

    void markAllTextureRawDataAsUpdated() { _dirtyRawData.clear(); };
    bool dirtyTextureRawDataExists() const { return (0 < _dirtyRawData.size()); };

private:
    // The texture mip level requested width, height and depth (as was asked on mip level generation):
    GLsizei _requestedWidth;
    GLsizei _requestedHeight;
    GLsizei _requestedDepth;

    // The width of the texture border:
    GLint _requestedBorderWidth;

    // The format of the input texture data.
    // I.E: How are the input texels arranged (GL_RGB, GL_RGBA, etc).
    GLenum _pixelFormat;

    // The type of the input texture texels (GL_BYTE, GL_SHORT, etc).
    GLenum _texelsType;

    // A request for a format that will be used to stored the texture data
    // on the graphics server:
    GLint _requestedInternalPixelFormat;

    // A list of texture level parameters (list for each texture pane):
    apGLTextureParams _textureLevelParameters[AP_MAX_AMOUNT_OF_TEXTURE_FACES];

    // A list of paths of a files that contains the texture data:
    osFilePath _textureDataFilesPaths[AP_MAX_AMOUNT_OF_TEXTURE_FACES];

    // OpenCL interoperability:
    int _openCLImageIndex;
    int _openCLImageName;
    int _openCLSpyID;
};


#endif  // __APGLTEXTURE

