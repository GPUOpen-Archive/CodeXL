//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLBuffer.cpp
///
//==================================================================================

//------------------------------ apCLBuffer.cpp ------------------------------

// Local:
#include <AMDTAPIClasses/Include/apCLBuffer.h>


// ---------------------------------------------------------------------------
// Name:        apCLBuffer::apCLBuffer
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        18/11/2009
// ---------------------------------------------------------------------------
apCLBuffer::apCLBuffer(gtInt32 bufferName):
    apCLMemObject(), _bufferName(bufferName), _bufferSize(0), _bufferFile(L""), _isDirty(true),
    _displayFormat(OA_TEXEL_FORMAT_FLOAT4), _offset(0), _stride(0), _openGLBufferName(0), _openGLSpyID(0)
{
}


// ---------------------------------------------------------------------------
// Name:        apCLBuffer::~apCLBuffer
// Description: Destructor.
// Author:  AMD Developer Tools Team
// Date:        18/11/2009
// ---------------------------------------------------------------------------
apCLBuffer::~apCLBuffer()
{
}

// ---------------------------------------------------------------------------
// Name:        apCLBuffer::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        18/11/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLBuffer::type() const
{
    return OS_TOBJ_ID_CL_BUFFER;
}


// ---------------------------------------------------------------------------
// Name:        apCLBuffer::writeSelfIntoChannel
// Description: Writes this object into an IPC Channel.
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        18/11/2009
// ---------------------------------------------------------------------------
bool apCLBuffer::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = false;

    // Write the buffer id:
    ipcChannel << _bufferName;

    // Write the buffer size:
    ipcChannel << (gtUInt64)_bufferSize;

    // Write the amount of sub buffers:
    ipcChannel << (gtUInt32)_subBufferIndices.size();

    // Write the sub buffers indices:
    for (int i = 0; i < (int)_subBufferIndices.size(); i++)
    {
        ipcChannel << (gtInt32)_subBufferIndices[i];
    }

    // Write buffer file path into channel:
    _bufferFile.writeSelfIntoChannel(ipcChannel);

    // Write the buffer display format:
    ipcChannel << (gtInt32)_displayFormat;

    // Write the buffer offset:
    ipcChannel << (gtUInt64)_offset;

    // Write the buffer stride:
    ipcChannel << (gtUInt64)_stride;

    // Write the OpenGL buffer name:
    ipcChannel << (gtUInt32)_openGLBufferName;

    // Write the OpenGL context spy id:
    ipcChannel << (gtInt32)_openGLSpyID;

    // Write the mem object Info:
    retVal = apCLMemObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLBuffer::readSelfFromChannel
// Description: Reads this object from an IPC Channel.
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        18/11/2009
// ---------------------------------------------------------------------------
bool apCLBuffer::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = false;

    // Read the buffer id:
    ipcChannel >> _bufferName;

    // Read the buffer size:
    gtUInt64 uInt64Param = 0;
    ipcChannel >> uInt64Param;
    _bufferSize = (gtSize_t)uInt64Param;

    // Read the amount of sub buffers:
    gtUInt32 amountOfSubBuffers;
    ipcChannel >> amountOfSubBuffers;

    // Write the sub buffers indices:
    for (gtUInt32 i = 0; i < amountOfSubBuffers; i++)
    {
        gtInt32 subBufferIndex = 0;
        ipcChannel >> subBufferIndex;
        _subBufferIndices.push_back(subBufferIndex);
    }


    // Read buffer file path from channel:
    _bufferFile.readSelfFromChannel(ipcChannel);

    gtInt32 int32Param = 0;
    ipcChannel >> int32Param;
    _displayFormat = (oaTexelDataFormat)int32Param;

    ipcChannel >> uInt64Param;
    _offset = (int)uInt64Param;

    ipcChannel >> uInt64Param;
    _stride = (gtSize_t)uInt64Param;

    // Read the buffer OpenGL name:
    gtUInt32 uInt32Param = 0;
    ipcChannel >> uInt32Param;
    _openGLBufferName = ((GLuint)uInt32Param);

    // Read the buffer OpenGL spy ID:
    ipcChannel >> int32Param;
    _openGLSpyID = (int)int32Param;

    // Read the allocated object Info:
    retVal = apCLMemObject::readSelfFromChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLBuffer::setBufferDisplayProperties
// Description: Sets the buffer display properties
// Arguments: oaTexelDataFormat displayFormat
//            int offset
//            GLsizei stride
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        8/12/2009
// ---------------------------------------------------------------------------
void apCLBuffer::setBufferDisplayProperties(oaTexelDataFormat displayFormat, int offset, gtSize_t stride)
{
    _displayFormat = displayFormat;
    _offset = offset;
    _stride = stride;
}

// ---------------------------------------------------------------------------
// Name:        apCLBuffer::getBufferDisplayProperties
// Description: Get the buffer display properties
// Arguments: oaTexelDataFormat& displayFormat
//            unsigned long& offset
//            gtSize_t& stride
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        8/12/2009
// ---------------------------------------------------------------------------
void apCLBuffer::getBufferDisplayProperties(oaTexelDataFormat& displayFormat, int& offset, gtSize_t& stride) const
{
    displayFormat = _displayFormat;
    offset = _offset;
    stride = _stride;
}

