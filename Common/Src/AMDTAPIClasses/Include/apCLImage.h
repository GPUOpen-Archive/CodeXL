//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLImage.h
///
//==================================================================================

//------------------------------ apCLImage.h ------------------------------

#ifndef __APCLIMAGE_H
#define __APCLIMAGE_H

// OpenCL:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSAPIWrappers/Include/oaDataType.h>
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>

// Local:
#include <AMDTAPIClasses/Include/apCLMemObject.h>
#include <AMDTAPIClasses/Include/apTextureType.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLImage : public apAllocatedObject
// General Description:
//   Represents an OpenCL buffer.
//
// Author:  AMD Developer Tools Team
// Creation Date:        1/12/2009
// ----------------------------------------------------------------------------------
class AP_API apCLImage : public apCLMemObject
{
public:
    apCLImage(gtInt32 imageName = -1);
    apCLImage(gtInt32 imageName, apTextureType imageType, gtSize_t width, gtSize_t height, gtSize_t depth);
    virtual ~apCLImage();

    // Image id:
    gtInt32 imageName() const {return _imageName;};

    // Memory size:
    bool getMemorySize(gtSize_t& memorySize) const;

    // Pixel Size
    int pixelSize() const;

    // Dimensions:
    void setDimensions(gtSize_t width, gtSize_t height, gtSize_t depth) {_width = width; _height = height; _depth = depth;};
    void getDimensions(gtSize_t& width, gtSize_t& height, gtSize_t& depth) const {width = _width; height = _height; depth = _depth;};

    // Get & Set the image file path that contains the buffer content:
    void setImageFilePath(const osFilePath& filePath) { _imageFilePath = filePath; };
    void imageFilePath(osFilePath& filePath) const { filePath = _imageFilePath; };

    // Dirty flag:
    void markAsDirty(bool isDirty) {_isDirty = isDirty;};
    bool isDirty() const {return _isDirty;};

    // Image type:
    apTextureType imageType()const {return _imageType;}

    // CL Buffer:
    void setCLBuffer(oaCLMemHandle buffer) {_buffer = buffer;}
    oaCLMemHandle buffer() const {return _buffer;}

    // Format & Type:
    void setFormatAndType(cl_uint dataFormat, cl_uint dataType) {_clDataFormat = dataFormat; _clDataType = dataType;};
    cl_uint dataFormat() const {return _clDataFormat;};
    cl_uint dataType() const {return _clDataType;};

    // GL - CL interoperability:
    void setGLTextureDetails(GLuint glTextureName, GLint glMipLevel, GLenum glTarget) {_openGLTextureName = glTextureName; _openGLMiplevel = glMipLevel; _openGLTarget = glTarget; };
    void setGLRenderBufferDetails(GLuint glRenderBufferName) {_openGLRenderBufferName = glRenderBufferName; };
    void setOpenGLSpyID(int glSpyId) {_openGLSpyId = glSpyId;}
    void getGLTextureDetails(GLuint& glTextureName, GLint& glMipLevel, GLenum& glTarget) const {glTextureName = _openGLTextureName; glMipLevel = _openGLMiplevel; glTarget = _openGLTarget;};
    GLuint openGLRenderBufferName() const { return _openGLRenderBufferName; };
    GLuint openGLTextureName() const { return _openGLTextureName; };
    int openGLSpyID() const {return _openGLSpyId;}

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);
    static bool textureTypeFromMemObjectType(cl_mem_object_type image_type, apTextureType& textureType);

protected:

    // Image name:
    gtInt32 _imageName;

    // Image type:
    apTextureType _imageType;

    // Image format:
    cl_uint _clDataFormat;

    // Image data type:
    cl_uint _clDataType;

    // Context handle:
    oaCLContextHandle _contextHandle;

    // Dimensions:
    gtSize_t _width;
    gtSize_t _height;
    gtSize_t _depth;

    // Buffer image:
    oaCLMemHandle _buffer;

    // File path to the texture data content:
    osFilePath _imageFilePath;

    // True iff the image was changed since the last update:
    bool _isDirty;

    // CL - GL interoperability:
    GLuint _openGLRenderBufferName;
    GLuint _openGLTextureName;
    GLint _openGLMiplevel;
    GLenum _openGLTarget;
    int _openGLSpyId;

};

#endif //__APCLIMAGE_H

