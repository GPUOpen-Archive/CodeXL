//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLSubBuffer.cpp
///
//==================================================================================

//------------------------------ apCLSubBuffer.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/apCLSubBuffer.h>

// ---------------------------------------------------------------------------
// Name:        apCLSubBuffer::apCLSubBuffer
// Description: Constructor
// Arguments:   gtInt32 subBufferName
//              gtInt32 bufferName - the internal name of my buffer
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        26/10/2010
// ---------------------------------------------------------------------------
apCLSubBuffer::apCLSubBuffer():
    apCLMemObject(), _bufferName(0), _subBufferName(0), _subBufferCreateType(CL_BUFFER_CREATE_TYPE_REGION), _subBufferFile(L""), _isDirty(true),
    _displayFormat(OA_TEXEL_FORMAT_V3F), _offset(0), _stride(0)
{
    _bufferRegion.origin = 0;
    _bufferRegion.size = 0;
}
// ---------------------------------------------------------------------------
// Name:        apCLSubBuffer::apCLSubBuffer
// Description: Constructor
// Arguments:   gtInt32 subBufferName
//              gtInt32 bufferName - the internal name of my buffer
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        26/10/2010
// ---------------------------------------------------------------------------
apCLSubBuffer::apCLSubBuffer(gtInt32 bufferName, gtInt32 subBufferName):
    apCLMemObject(), _bufferName(bufferName), _subBufferName(subBufferName), _subBufferCreateType(CL_BUFFER_CREATE_TYPE_REGION), _subBufferFile(L""), _isDirty(true),
    _displayFormat(OA_TEXEL_FORMAT_V3F), _offset(0), _stride(0)
{
    _bufferRegion.origin = 0;
    _bufferRegion.size = 0;
}


// ---------------------------------------------------------------------------
// Name:        apCLSubBuffer::~apCLSubBuffer
// Description: Destructor.
// Author:  AMD Developer Tools Team
// Date:        26/10/2010
// ---------------------------------------------------------------------------
apCLSubBuffer::~apCLSubBuffer()
{
}

// ---------------------------------------------------------------------------
// Name:        apCLSubBuffer::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        26/10/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apCLSubBuffer::type() const
{
    return OS_TOBJ_ID_CL_SUB_BUFFER;
}


// ---------------------------------------------------------------------------
// Name:        apCLSubBuffer::writeSelfIntoChannel
// Description: Writes this object into an IPC Channel.
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        26/10/2010
// ---------------------------------------------------------------------------
bool apCLSubBuffer::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = false;

    // Write the buffer id:
    ipcChannel << _bufferName;

    // Write the sub buffer id:
    ipcChannel << _subBufferName;

    // Write the buffer create type:
    ipcChannel << (gtInt32)_subBufferCreateType;

    // Write the buffer region:
    ipcChannel << (gtUInt64)_bufferRegion.origin;
    ipcChannel << (gtUInt64)_bufferRegion.size;

    // Write buffer file path into channel:
    _subBufferFile.writeSelfIntoChannel(ipcChannel);

    // Write the buffer display format:
    ipcChannel << (gtInt32)_displayFormat;

    // Write the buffer offset:
    ipcChannel << (gtUInt64)_offset;

    // Write the buffer stride:
    ipcChannel << (gtUInt64)_stride;

    // Write the mem object Info:
    retVal = apCLMemObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLSubBuffer::readSelfFromChannel
// Description: Reads this object from an IPC Channel.
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        26/10/2010
// ---------------------------------------------------------------------------
bool apCLSubBuffer::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = false;

    // Read the buffer id:
    ipcChannel >> _bufferName;

    // Read the sub buffer id:
    ipcChannel >> _subBufferName;

    // Read the buffer create type:
    gtInt32 int32Param = 0;
    ipcChannel >> int32Param;
    _subBufferCreateType = (cl_buffer_create_type)int32Param;

    // Read the buffer region:
    gtUInt64 uInt64Param = 0;
    ipcChannel >> uInt64Param;
    _bufferRegion.origin = (size_t)uInt64Param;
    ipcChannel >> uInt64Param;
    _bufferRegion.size = (size_t)uInt64Param;

    // Read buffer file path from channel:
    _subBufferFile.readSelfFromChannel(ipcChannel);

    ipcChannel >> int32Param;
    _displayFormat = (oaTexelDataFormat)int32Param;

    ipcChannel >> uInt64Param;
    _offset = (int)uInt64Param;

    ipcChannel >> uInt64Param;
    _stride = (gtSize_t)uInt64Param;

    // Read the allocated object Info:
    retVal = apCLMemObject::readSelfFromChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLSubBuffer::setSubBufferDisplayProperties
// Description: Sets the buffer display properties
// Arguments:   oaTexelDataFormat displayFormat
//              int offset
//              GLsizei stride
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        26/10/2010
// ---------------------------------------------------------------------------
void apCLSubBuffer::setSubBufferDisplayProperties(oaTexelDataFormat displayFormat, int offset, gtSize_t stride)
{
    _displayFormat = displayFormat;
    _offset = offset;
    _stride = stride;
}

// ---------------------------------------------------------------------------
// Name:        apCLSubBuffer::getSubBufferDisplayProperties
// Description: Get the buffer display properties
// Arguments:   oaTexelDataFormat& displayFormat
//              unsigned long& offset
//              gtSize_t& stride
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        26/10/2010
// ---------------------------------------------------------------------------
void apCLSubBuffer::getSubBufferDisplayProperties(oaTexelDataFormat& displayFormat, int& offset, gtSize_t& stride) const
{
    displayFormat = _displayFormat;
    offset = _offset;
    stride = _stride;
}

