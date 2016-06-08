//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLRenderBuffer.cpp
///
//==================================================================================

// -----------------------------   apGLRenderBuffer.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apParameters.h>
#include <AMDTAPIClasses/Include/apInternalFormat.h>
#include <AMDTAPIClasses/Include/ap2DRectangle.h>
#include <AMDTAPIClasses/Include/apGLRenderBuffer.h>



// ---------------------------------------------------------------------------
// Name:        apGLRenderBuffer::apGLRenderBuffer
// Description: Constructor
// Arguments:   bufferName - The OpenGL texture name.
// Author:  AMD Developer Tools Team
// Date:        26/5/2008
// ---------------------------------------------------------------------------
apGLRenderBuffer::apGLRenderBuffer(GLuint bufferName)
    :
    apAllocatedObject(),
    _bufferName(bufferName),
    _width(0),
    _height(0),
    _bufferDataFormat(OA_TEXEL_FORMAT_UNKNOWN),
    _bufferDataType(OA_UNKNOWN_DATA_TYPE),
    _isBoundToActiveFBO(false),
    _isDirty(true),
    _bufferType(AP_DISPLAY_BUFFER_UNKNOWN),
    _fboName(0),
    _bufferFile(L""),
    _openCLImageIndex(-1),
    _openCLImageName(-1),
    _openCLSpyID(-1)
{
}



// ---------------------------------------------------------------------------
// Name:        apGLRenderBuffer::apGLRenderBuffer
// Description: Copy constructor
// Arguments: other - The other texture class from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        26/5/2008
// ---------------------------------------------------------------------------
apGLRenderBuffer::apGLRenderBuffer(const apGLRenderBuffer& other)
{
    apGLRenderBuffer::operator=(other);
}


// ---------------------------------------------------------------------------
// Name:        apGLRenderBuffer::~apGLRenderBuffer
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        26/5/2008
// ---------------------------------------------------------------------------
apGLRenderBuffer::~apGLRenderBuffer()
{
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderBuffer::operator=
// Description: Assignment operator
// Arguments:   other - The other object from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        26/5/2008
// ---------------------------------------------------------------------------
apGLRenderBuffer& apGLRenderBuffer::operator=(const apGLRenderBuffer& other)
{
    _bufferName = other._bufferName;
    _bufferType = other._bufferType;
    _width = other._width;
    _bufferFile = other._bufferFile;
    _bufferDataFormat = other._bufferDataFormat;
    _bufferDataType = other._bufferDataType;
    _isBoundToActiveFBO = other._isBoundToActiveFBO;
    _isDirty = other._isDirty;
    _height = other._height;
    _fboName = other._fboName;

    _openCLImageIndex = other._openCLImageIndex;
    _openCLImageName = other._openCLImageName;
    _openCLSpyID = other._openCLSpyID;

    setAllocatedObjectId(other.getAllocatedObjectId(), true);

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        19/12/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLRenderBuffer::type() const
{
    return OS_TOBJ_ID_GL_RENDER_BUFFER;
}


// ---------------------------------------------------------------------------
// Name:        apGLRenderBuffer::markAsBoundToActiveFBO
// Description: Marks the render buffer as bound to the active FBO
// Arguments: bool isBound
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        11/6/2008
// ---------------------------------------------------------------------------
void apGLRenderBuffer::markAsBoundToActiveFBO(bool isBound)
{
    _isBoundToActiveFBO = isBound;
}
// ---------------------------------------------------------------------------
// Name:        apGLRenderBuffer::getBufferDimensions(
// Description: Retrieves the buffer dimensions
// Arguments:   bufferWidth - Output buffer width
//              bufferHeight - Output buffer height
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        28/5/2008
// ---------------------------------------------------------------------------
void apGLRenderBuffer::getBufferDimensions(GLint& bufferWidth, GLint& bufferHeight) const
{
    bufferWidth = _width;
    bufferHeight = _height;
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderBuffer::writeSelfIntoChannel
// Description: Writes this class into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        26/5/2008
// ---------------------------------------------------------------------------
bool apGLRenderBuffer::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the texture attributes into the channel:
    ipcChannel << (gtUInt32)_bufferName;
    ipcChannel << (gtInt32)_bufferType;
    ipcChannel << (gtUInt64)_width;
    ipcChannel << (gtUInt64)_height;
    ipcChannel << (gtInt32)_bufferDataFormat;
    ipcChannel << (gtInt32)_bufferDataType;
    ipcChannel << (gtUInt32)_fboName;

    // Write the OpenCL share information:
    ipcChannel << (gtInt32)_openCLImageIndex;
    ipcChannel << (gtInt32)_openCLImageName;
    ipcChannel << (gtInt32)_openCLSpyID;

    // Write buffer file path into channel
    _bufferFile.writeSelfIntoChannel(ipcChannel);

    apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderBuffer::readSelfFromChannel
// Description: Read self from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        26/5/2008
// ---------------------------------------------------------------------------
bool apGLRenderBuffer::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the render buffer attributes from the channel:
    gtUInt32 bufferNameAsUInt32 = 0;
    ipcChannel >> bufferNameAsUInt32;
    _bufferName = (GLuint)bufferNameAsUInt32;

    gtInt32 varAsInt32 = 0;
    ipcChannel >> varAsInt32;
    _bufferType = (apDisplayBuffer)varAsInt32;

    gtInt64 widthAsInt64 = 0;
    ipcChannel >> widthAsInt64;
    _width = (GLsizei)widthAsInt64;

    gtInt64 heightAsInt64 = 0;
    ipcChannel >> heightAsInt64;
    _height = (GLsizei)heightAsInt64;

    ipcChannel >> varAsInt32;
    _bufferDataFormat = (oaTexelDataFormat)varAsInt32;

    ipcChannel >> varAsInt32;
    _bufferDataType = (oaDataType)varAsInt32;

    gtUInt32 fboNameAsUInt32 = 0;
    ipcChannel >> fboNameAsUInt32;
    _fboName = (GLuint)fboNameAsUInt32;

    ipcChannel >> varAsInt32;
    _openCLImageIndex = varAsInt32;

    ipcChannel >> varAsInt32;
    _openCLImageName = varAsInt32;

    ipcChannel >> varAsInt32;
    _openCLSpyID = varAsInt32;

    // Read buffer file path from channel
    _bufferFile.readSelfFromChannel(ipcChannel);

    apAllocatedObject::readSelfFromChannel(ipcChannel);

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        apGLRenderBuffer::calculateMemorySize
// Description: Calculates the render buffer's memory size
// Arguments: gtSize_t& memorySize
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/10/2008
// ---------------------------------------------------------------------------
bool apGLRenderBuffer::calculateMemorySize(gtSize_t& memorySize)
{
    bool retVal = false;
    memorySize = 0;

    // Calculate the pixel unit byte size:
    int pixelSizeInBytes = oaCalculatePixelUnitByteSize(_bufferDataFormat, _bufferDataType);

    if (pixelSizeInBytes > 0)
    {
        retVal = true;

        // Convert to bits:
        int pixelSizeInBits = pixelSizeInBytes * GT_BITS_PER_BYTE;

        // Calculate the texture number of pixels:
        GLuint numberOfPixels = 1;
        numberOfPixels *= _width;
        numberOfPixels *= _height;

        // Multiply the number of pixels with the size needed for each pixel:
        memorySize = numberOfPixels * pixelSizeInBits;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLRenderBuffer::setBufferType
// Description: Sets the buffer type, and also find the data type according to
//              buffer type
// Arguments: apDisplayBuffer bufferType
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        12/10/2008
// ---------------------------------------------------------------------------
void apGLRenderBuffer::setBufferType(apDisplayBuffer bufferType)
{
    // Set the buffer type:
    _bufferType = bufferType;

    // Compute the buffer data type and format:
    bool rc = setDataTypeByBufferType();
    GT_ASSERT(rc);
}
// ---------------------------------------------------------------------------
// Name:        apGLRenderBuffer::setFormatAndDataTypeByBufferType
// Description: The function sets the buffer's format and data type according to the
//              buffer's display type.
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        12/10/2008
// ---------------------------------------------------------------------------
bool apGLRenderBuffer::setDataTypeByBufferType()
{
    bool retVal = true;

    switch (_bufferType)
    {
        case AP_DEPTH_BUFFER:
        {
            _bufferDataType = OA_FLOAT;
        }
        break;

        case AP_STENCIL_BUFFER:
        {
            _bufferDataType = OA_UNSIGNED_BYTE;
        }
        break;

        case AP_BACK_BUFFER:
        case AP_FRONT_BUFFER:
        case AP_AUX0_BUFFER:
        case AP_AUX1_BUFFER:
        case AP_AUX2_BUFFER:
        case AP_AUX3_BUFFER:
        {
            _bufferDataType = OA_UNSIGNED_BYTE;
        }
        break;

        case AP_COLOR_ATTACHMENT0_EXT:
        case AP_COLOR_ATTACHMENT1_EXT:
        case AP_COLOR_ATTACHMENT2_EXT:
        case AP_COLOR_ATTACHMENT3_EXT:
        case AP_COLOR_ATTACHMENT4_EXT:
        case AP_COLOR_ATTACHMENT5_EXT:
        case AP_COLOR_ATTACHMENT6_EXT:
        case AP_COLOR_ATTACHMENT7_EXT:
        case AP_COLOR_ATTACHMENT8_EXT:
        case AP_COLOR_ATTACHMENT9_EXT:
        case AP_COLOR_ATTACHMENT10_EXT:
        case AP_COLOR_ATTACHMENT11_EXT:
        case AP_COLOR_ATTACHMENT12_EXT:
        case AP_COLOR_ATTACHMENT13_EXT:
        case AP_COLOR_ATTACHMENT14_EXT:
        case AP_COLOR_ATTACHMENT15_EXT:
        {
            _bufferDataType = OA_UNSIGNED_BYTE;
        }
        break;

        case AP_DEPTH_ATTACHMENT_EXT:
        {
            _bufferDataType = OA_FLOAT;
        }
        break;

        case AP_STENCIL_ATTACHMENT_EXT:
        {
            _bufferDataType = OA_UNSIGNED_BYTE;
        }
        break;

        default:
        {
            GT_ASSERT_EX(false, L"Buffer type is not supported!");
            retVal = false;
        }
        break;
    }

    return retVal;
}
