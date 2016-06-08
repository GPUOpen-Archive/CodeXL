//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apStaticBuffer.cpp
///
//==================================================================================

//------------------------------ apStaticBuffer.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apStaticBuffer.h>


// ---------------------------------------------------------------------------
// Name:        apStaticBuffer::apStaticBuffer
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        24/8/2007
// ---------------------------------------------------------------------------
apStaticBuffer::apStaticBuffer()
    : apAllocatedObject(), _bufferType(AP_FRONT_BUFFER), _bufferDataFormat(OA_TEXEL_FORMAT_UNKNOWN),
      _bufferDataType(OA_UNSIGNED_BYTE), _bufferWidth(-1), _bufferHeight(-1)
{
}

// ---------------------------------------------------------------------------
// Name:        apStaticBuffer::~apStaticBuffer
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        24/8/2007
// ---------------------------------------------------------------------------
apStaticBuffer::~apStaticBuffer()
{

}

// ---------------------------------------------------------------------------
// Name:        apStaticBuffer::apStaticBuffer
// Description: Copy constructor
// Arguments:   other - The other static buffer class from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        27/8/2007
// ---------------------------------------------------------------------------
apStaticBuffer::apStaticBuffer(const apStaticBuffer& other)
{
    apStaticBuffer::operator=(other);
}


// ---------------------------------------------------------------------------
// Name:        apStaticBuffer::operator=
// Description: Assignment operator
// Arguments:   other - The other object from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        27/8/2007
// ---------------------------------------------------------------------------
apStaticBuffer& apStaticBuffer::operator=(const apStaticBuffer& other)
{
    // Copy the apStaticBuffer object
    _bufferType = other._bufferType;
    _bufferFile = other._bufferFile;
    _bufferWidth = other._bufferWidth;
    _bufferHeight = other._bufferHeight;
    _bufferDataType = other._bufferDataType;
    _bufferDataFormat = other._bufferDataFormat;

    setAllocatedObjectId(other.getAllocatedObjectId(), true);

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        apStaticBuffer::getBufferDimensions(
// Description: Retrieves the buffer dimensions
// Arguments:   bufferWidth - Output buffer width
//              bufferHeight - Output buffer height
// Author:  AMD Developer Tools Team
// Date:        24/8/2007
// ---------------------------------------------------------------------------
void apStaticBuffer::getBufferDimensions(GLsizei& bufferWidth, GLsizei& bufferHeight) const
{
    bufferWidth = _bufferWidth;
    bufferHeight = _bufferHeight;
}

// ---------------------------------------------------------------------------
// Name:        apStaticBuffer::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        27/8/2007
// ---------------------------------------------------------------------------
osTransferableObjectType apStaticBuffer::type() const
{
    return OS_TOBJ_ID_GL_STATIC_BUFFER;
}


// ---------------------------------------------------------------------------
// Name:        apStaticBuffer::writeSelfIntoChannel
// Description: Writes this class into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/8/2007
// ---------------------------------------------------------------------------
bool apStaticBuffer::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the static buffer attributes into the channel:
    ipcChannel << (gtInt32)_bufferType;
    ipcChannel << (gtUInt64)_bufferWidth;
    ipcChannel << (gtUInt64)_bufferHeight;
    ipcChannel << (gtInt32)_bufferDataFormat;
    ipcChannel << (gtInt32)_bufferDataType;

    // Write buffer file path into channel
    _bufferFile.writeSelfIntoChannel(ipcChannel);

    apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apStaticBuffer::readSelfFromChannel
// Description: Read self from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/8/2007
// ---------------------------------------------------------------------------
bool apStaticBuffer::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the static buffer attributes from the channel:
    gtInt32 bufferTypeAsInt32 = 0;
    ipcChannel >> bufferTypeAsInt32;
    _bufferType = (apDisplayBuffer)bufferTypeAsInt32;
    gtUInt64 bufferWidthAsUInt64 = 0;
    ipcChannel >> bufferWidthAsUInt64;
    _bufferWidth = (GLsizei)bufferWidthAsUInt64;
    gtUInt64 bufferHeightAsUInt64 = 0;
    ipcChannel >> bufferHeightAsUInt64;
    _bufferHeight = (GLsizei)bufferHeightAsUInt64;
    gtInt32 bufferDataFormatAsInt32 = 0;
    ipcChannel >> bufferDataFormatAsInt32;
    _bufferDataFormat = (oaTexelDataFormat)bufferDataFormatAsInt32;
    gtInt32 bufferDataTypeAsInt32 = 0;
    ipcChannel >> bufferDataTypeAsInt32;
    _bufferDataType = (oaDataType)bufferDataTypeAsInt32;

    // Read buffer file path from channel
    _bufferFile.readSelfFromChannel(ipcChannel);

    apAllocatedObject::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apStaticBuffer::getBufferFormat
// Description: Retrieves the buffer data format and data type
// Arguments:   bufferDataFormat - Output buffer data format
//              bufferDataType - Output buffer data type
// Author:  AMD Developer Tools Team
// Date:        3/9/2007
// ---------------------------------------------------------------------------
void apStaticBuffer::getBufferFormat(oaTexelDataFormat& bufferDataFormat, oaDataType& bufferDataType) const
{
    bufferDataFormat = _bufferDataFormat;
    bufferDataType = _bufferDataType;
}

// ---------------------------------------------------------------------------
// Name:        apStaticBuffer::calculateMemorySize
// Description: Calculates the static buffer's memory size
// Arguments: gtSize_t& memorySize
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/10/2008
// ---------------------------------------------------------------------------
bool apStaticBuffer::calculateMemorySize(gtSize_t& memorySize)
{
    bool retVal = false;
    memorySize = 0;
    int pixelSizeInBits = 0;

    // Calculate the pixel unit byte size:
    int pixelSizeInBytes = oaCalculatePixelUnitByteSize(_bufferDataFormat, _bufferDataType);

    if (pixelSizeInBytes > 0)
    {
        retVal = true;
        // Convert to bits:
        pixelSizeInBits = pixelSizeInBytes * GT_BITS_PER_BYTE;

        // Calculate the texture number of pixels:
        GLuint numberOfPixels = 1;

        if ((_bufferWidth > 0) && (_bufferHeight > 0))
        {
            numberOfPixels *= _bufferWidth;
            numberOfPixels *= _bufferHeight;
        }
        else
        {
            numberOfPixels = 0;
        }

        // Multiply the number of pixels with the size needed for each pixel:
        memorySize = numberOfPixels * pixelSizeInBits;

    }


    return retVal;
}
