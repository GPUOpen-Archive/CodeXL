//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apStaticBuffer.h
///
//==================================================================================

//------------------------------ apStaticBuffer.h ------------------------------

#ifndef __APSTATICBUFFER
#define __APSTATICBUFFER

// OpenGL:
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSAPIWrappers/Include/oaDataType.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>

// Local:
#include <AMDTAPIClasses/Include/apDisplayBuffer.h>
#include <AMDTAPIClasses/Include/apAllocatedObject.h>

// ----------------------------------------------------------------------------------
// Class Name:           apStaticBuffer : public osTransferableObject
// General Description:  Represents an OpenGL static buffer object
//
// Author:  AMD Developer Tools Team
// Creation Date:        25/08/2007
// ----------------------------------------------------------------------------------
class AP_API apStaticBuffer : public apAllocatedObject
{
public:
    // Constructor:
    apStaticBuffer();

    // Destructor:
    virtual ~apStaticBuffer();

    // Self functions:
    apStaticBuffer(const apStaticBuffer& other);
    apStaticBuffer& operator=(const apStaticBuffer& other);

    // Memory size:
    bool calculateMemorySize(gtSize_t& memorySize);

public:
    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

public:
    // Get buffer type
    apDisplayBuffer bufferType() const { return _bufferType; };

    // Get buffer format:
    oaTexelDataFormat bufferFormat() const {return _bufferDataFormat;}

    // Get buffer data type:
    oaDataType bufferDataType() const {return _bufferDataType;}

    // Retrieves the buffer file path
    void getBufferFilePath(osFilePath& filePath) const { filePath = _bufferFile; };

    // Retrieves the buffer dimensions:
    void getBufferDimensions(GLsizei& bufferWidth, GLsizei& bufferHeight) const;
    GLsizei width() const {return _bufferWidth;}
    GLsizei height() const {return _bufferHeight;}

    // Retrieves the buffer data format and data type
    void getBufferFormat(oaTexelDataFormat& bufferDataFormat, oaDataType& bufferDataType) const;

public:
    // Set the buffer type
    void setBufferType(apDisplayBuffer bufferType) { _bufferType = bufferType; };

    // Set the buffer file path that contains the buffer content
    void setBufferFilePath(const osFilePath& filePath) { _bufferFile = filePath; };

    // Set the buffer dimensions
    void setBufferDimensions(int bufferWidth, int bufferHeight) { _bufferWidth = bufferWidth; _bufferHeight = bufferHeight; };

    // Set the buffer data format and data type
    void setBufferDataFormat(oaTexelDataFormat bufferDataFormat, oaDataType bufferDataType) { _bufferDataFormat = bufferDataFormat; _bufferDataType = bufferDataType; };

private:
    // Buffer type
    apDisplayBuffer _bufferType;

    // Buffer data format:
    oaTexelDataFormat _bufferDataFormat;

    // Data type:
    oaDataType _bufferDataType;

    // File path to the buffer data content
    osFilePath _bufferFile;

    // The PBuffer width and height:
    GLsizei _bufferWidth, _bufferHeight;
};


#endif  // __APSTATICBUFFER
