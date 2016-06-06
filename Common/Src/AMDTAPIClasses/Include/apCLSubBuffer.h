//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLSubBuffer.h
///
//==================================================================================

//------------------------------ apCLSubBuffer.h ------------------------------

#ifndef __APCLSUBBUFFER_H
#define __APCLSUBBUFFER_H


// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>

// Local:
#include <AMDTAPIClasses/Include/apCLMemObject.h>


// ----------------------------------------------------------------------------------
// Class Name:           AP_API apCLSubBuffer : public apAllocatedObject
// General Description:  Represents an OpenCL buffer.
// Author:  AMD Developer Tools Team
// Creation Date:        26/10/2010
// ----------------------------------------------------------------------------------
class AP_API apCLSubBuffer : public apCLMemObject
{
public:
    apCLSubBuffer();
    apCLSubBuffer(gtInt32 bufferName, gtInt32 subBufferName);
    virtual ~apCLSubBuffer();

    // Overrides osTransferableObject:
    virtual osTransferableObjectType type() const;
    virtual bool writeSelfIntoChannel(osChannel& ipcChannel) const;
    virtual bool readSelfFromChannel(osChannel& ipcChannel);

    // Buffer and sub buffer name:
    gtInt32 bufferName() const {return _bufferName;};
    gtInt32 subBufferName() const {return _subBufferName;};

    // Buffer create type:
    void setSubBufferCreateType(cl_buffer_create_type createType) {_subBufferCreateType = createType;}
    cl_buffer_create_type subBufferCreateType() const {return _subBufferCreateType;}

    // Buffer region:
    void setBufferRegion(cl_buffer_region bufferRegion) {_bufferRegion.origin = bufferRegion.origin; _bufferRegion.size = bufferRegion.size;}
    cl_buffer_region bufferRegion() const {return _bufferRegion;}

    // Buffer display properties:
    void setSubBufferDisplayProperties(oaTexelDataFormat displayFormat, int offset, gtSize_t stride);
    void getSubBufferDisplayProperties(oaTexelDataFormat& displayFormat, int& offset, gtSize_t& stride) const;
    oaTexelDataFormat displayFormat() const {return _displayFormat;};

    // Get & Set the sub buffer file path that contains the buffer content:
    void setSubBufferFilePath(const osFilePath& filePath) { _subBufferFile = filePath; };
    void getSubBufferFilePath(osFilePath& filePath) const { filePath = _subBufferFile; };

    // Dirty flag:
    void markAsDirty(bool isDirty) {_isDirty = isDirty;};
    bool isDirty() const {return _isDirty;};

private:

    // Buffer name:
    gtInt32 _bufferName;

    // Sub Buffer name:
    gtInt32 _subBufferName;

    // Buffer create type:
    cl_buffer_create_type _subBufferCreateType;

    // Buffer create region:
    cl_buffer_region _bufferRegion;

    // File path to the buffer data content:
    osFilePath _subBufferFile;

    // On if the buffer was changed since the last update:
    bool _isDirty;

    // User defined buffer displayed format:
    oaTexelDataFormat _displayFormat;

    // User defined buffer offset:
    int _offset;

    // User defined buffer stride (amount of bytes separating between each buffer chunk):
    gtSize_t _stride;

};

#endif //__APCLSUBBUFFER_H


