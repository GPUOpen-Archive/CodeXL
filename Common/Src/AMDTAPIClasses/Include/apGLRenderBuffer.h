//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLRenderBuffer.h
///
//==================================================================================

//------------------------------ apGLRenderBuffer.h ------------------------------

#ifndef __APGLRENDERBUFFER
#define __APGLRENDERBUFFER

// OpenGL
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>
#include <AMDTOSAPIWrappers/Include/oaDataType.h>


// Local:
#include <AMDTAPIClasses/Include/apAPIClassesDLLBuild.h>
#include <AMDTAPIClasses/Include/apAllocatedObject.h>
#include <AMDTAPIClasses/Include/apDisplayBuffer.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apGLRenderBuffer : public osTransferableObject
// General Description:
//   Represents an OpenGL render buffer object. Holds the render buffer data and parameters.
//
// Author:  AMD Developer Tools Team
// Creation Date:        26/5/2008
// ----------------------------------------------------------------------------------
class AP_API apGLRenderBuffer : public apAllocatedObject
{
public:

    // Self functions:
    apGLRenderBuffer(GLuint bufferName);
    apGLRenderBuffer(const apGLRenderBuffer& other);
    virtual ~apGLRenderBuffer();
    apGLRenderBuffer& operator=(const apGLRenderBuffer& other);

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    void markAsBoundToActiveFBO(bool isBound);

    GLuint renderBufferName()const {return _bufferName;};
    void markAsDirty(bool isDirty) {_isDirty = isDirty;};
    bool isDirty() const {return _isDirty;};

    // Get & Set the buffer dimensions
    void getBufferDimensions(GLint& bufferWidth, GLint& bufferHeight) const;
    void setBufferDimensions(int bufferWidth, int bufferHeight) { _width = bufferWidth; _height = bufferHeight; };


    // Get & Set the buffer data format
    oaTexelDataFormat bufferFormat() const { return _bufferDataFormat; } ;
    oaDataType bufferDataType() const { return _bufferDataType ;} ;
    void setBufferDataFormat(oaTexelDataFormat bufferDataFormat) { _bufferDataFormat = bufferDataFormat; };
    void setBufferDataType(oaDataType bufferDataType) { _bufferDataType = bufferDataType; };

    // Get & Set the buffer display type
    apDisplayBuffer getBufferType() const { return _bufferType; };
    void setBufferType(apDisplayBuffer bufferType);

    // Get & Set the buffer FBO name
    GLuint getFBOName()const {return _fboName;} ;
    void setFBOName(GLuint fboName) { _fboName = fboName; };


    // Get & Set the buffer file path that contains the buffer content
    void setBufferFilePath(const osFilePath& filePath) { _bufferFile = filePath; };
    void getBufferFilePath(osFilePath& filePath) const { filePath = _bufferFile; };

    // Memory size:
    bool calculateMemorySize(gtSize_t& memorySize);

    // Buffer Name:
    GLuint bufferName() {return _bufferName;};

    // OpenCL interoperability:
    void shareBufferWithCLImage(int clImageIndex, int clImageName, int clSpyID) {_openCLImageIndex = clImageIndex; _openCLImageName = clImageName; _openCLSpyID = clSpyID;};
    void getCLImageDetails(int& clImageIndex, int& openCLImageName, int& clSpyID) const {openCLImageName = _openCLImageName; clImageIndex = _openCLImageIndex; clSpyID = _openCLSpyID;}
    int openCLImageIndex() const {return _openCLImageIndex;}
    int openCLImageName() const {return _openCLImageName;}

private:

    bool setDataTypeByBufferType();

    // The OpenGL render buffer name:
    GLuint _bufferName;

    // The render buffer width, height and depth:
    GLsizei _width;
    GLsizei _height;

    // Buffer data format:
    oaTexelDataFormat _bufferDataFormat;

    // Data type:
    oaDataType _bufferDataType;

    // This flag is on if the render buffer is bounded to the active FBO
    // (this means that the render buffer may be rendered into):
    bool _isBoundToActiveFBO;

    // On if the render buffer was changed since the last update:
    bool _isDirty;

    // Buffer type:
    apDisplayBuffer _bufferType;

    // FBO name:
    GLuint _fboName;

    // File path to the buffer data content:
    osFilePath _bufferFile;

    // Contain the OpenCL image id shared with the buffer:
    int _openCLImageIndex;

    // Contain the OpenCL image name shared with the buffer:
    int _openCLImageName;

    // Contain the OpenCL spy ID:
    int _openCLSpyID;

};


#endif  // __APGLRENDERBUFFER
