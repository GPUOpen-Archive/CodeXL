//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLBuffer.h
///
//==================================================================================

//------------------------------ apCLBuffer.h ------------------------------

#ifndef __APCLBUFFER_H
#define __APCLBUFFER_H

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>

// OpenCL:
#include <AMDTOSAPIWrappers/Include/oaOpenCLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apCLMemObject.h>
#include <AMDTAPIClasses/Include/apOpenCLParameters.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLBuffer : public apAllocatedObject
// General Description:
//   Represents an OpenCL buffer.
//
// Author:  AMD Developer Tools Team
// Creation Date:        18/11/2009
// ----------------------------------------------------------------------------------
class AP_API apCLBuffer : public apCLMemObject
{
public:
    apCLBuffer(gtInt32 bufferName = -1);
    virtual ~apCLBuffer();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Buffer id:
    gtInt32 bufferName() const {return _bufferName;};

    // Sub Buffers:
    void clearSubBuffers() { _subBufferIndices.clear();}
    const gtVector<int>& subBuffersIndices() const { return _subBufferIndices;}
    void addSubBufferIndex(int subBufferIndex) { _subBufferIndices.push_back(subBufferIndex);}

    // Buffer display properties:
    void setBufferDisplayProperties(oaTexelDataFormat displayFormat, int offset, gtSize_t stride);
    void getBufferDisplayProperties(oaTexelDataFormat& displayFormat, int& offset, gtSize_t& stride) const;
    oaTexelDataFormat displayFormat() const {return _displayFormat;};

    // Get & Set the buffer file path that contains the buffer content:
    void setBufferFilePath(const osFilePath& filePath) { _bufferFile = filePath; };
    void getBufferFilePath(osFilePath& filePath) const { filePath = _bufferFile; };

    // Dirty flag:
    void markAsDirty(bool isDirty) {_isDirty = isDirty;};
    bool isDirty() const {return _isDirty;};

    // Buffer size:
    gtSize_t bufferSize() const {return _bufferSize;}
    void setBufferSize(gtSize_t size) {_bufferSize = size;};

    // GL - CL interoperability:
    GLuint openGLBufferName() const {return _openGLBufferName;};
    void setGLBufferName(GLuint glBufferName) {_openGLBufferName = glBufferName;};
    int openGLSpyID() const { return _openGLSpyID;}
    void setOpenGLSpyID(int glSpyID) { _openGLSpyID = glSpyID;}

private:

    // Buffer name:
    gtInt32 _bufferName;

    // The buffer's size:
    gtSize_t _bufferSize;

    // Contain a vector of indices for sub buffers attached to this buffer object:
    gtVector<int> _subBufferIndices;

    // File path to the buffer data content:
    osFilePath _bufferFile;

    // On if the buffer was changed since the last update:
    bool _isDirty;

    // User defined buffer displayed format:
    oaTexelDataFormat _displayFormat;

    // User defined buffer offset:
    int _offset;

    // User defined buffer stride (amount of bytes separating between each buffer chunk):
    gtSize_t _stride;

    // When the buffer is created using OpenGL - OpenCL interoperability:
    // OpenGL buffer name:
    GLuint _openGLBufferName;

    // OpenGL context ID:
    GLuint _openGLSpyID;
};

#endif //__APCLBUFFER_H

