//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apPBuffer.cpp
///
//==================================================================================

//------------------------------ apPBuffer.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apDefinitions.h>
#include <AMDTAPIClasses/Include/apPBuffer.h>


// ---------------------------------------------------------------------------
// Name:        apPBuffer::apPBuffer
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        24/8/2007
// ---------------------------------------------------------------------------
apPBuffer::apPBuffer()
    : apAllocatedObject(), _pbufferRenderContextSpyId(-1), _width(0), _height(0), _pbufferHandler(0), _pbufferhDC(0),
      _isDeleted(false), _bindTarget(AP_UNDEFINED_PBUFFER), _internalFormat(AP_UNDEFINED_FORMAT_PBUFFER), _cubemapFace(GL_NONE), _maxMipmapLevel(-1), _mipmapLevel(-1)
{
}

// ---------------------------------------------------------------------------
// Name:        apPBuffer::apPBuffer
// Description: Constructor
// Author:  AMD Developer Tools Team
// Date:        24/8/2007
// ---------------------------------------------------------------------------
apPBuffer::apPBuffer(const oaPBufferHandle& pbufferHandler)
    : apAllocatedObject(), _pbufferRenderContextSpyId(-1), _width(0), _height(0), _pbufferHandler(pbufferHandler), _pbufferhDC(0),
      _isDeleted(false), _bindTarget(AP_UNDEFINED_PBUFFER), _internalFormat(AP_UNDEFINED_FORMAT_PBUFFER), _cubemapFace(GL_NONE), _maxMipmapLevel(-1), _mipmapLevel(-1)
{
}

// ---------------------------------------------------------------------------
// Name:        apPBuffer::~apPBuffer
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        24/8/2007
// ---------------------------------------------------------------------------
apPBuffer::~apPBuffer()
{

}

// ---------------------------------------------------------------------------
// Name:        apPBuffer::apPBuffer
// Description: Copy constructor
// Arguments:   other - The other PBuffer class from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        27/8/2007
// ---------------------------------------------------------------------------
apPBuffer::apPBuffer(const apPBuffer& other)
{
    apPBuffer::operator=(other);
}


// ---------------------------------------------------------------------------
// Name:        apPBuffer::operator=
// Description: Assignment operator
// Arguments:   other - The other object from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        27/8/2007
// ---------------------------------------------------------------------------
apPBuffer& apPBuffer::operator=(const apPBuffer& other)
{
    // Copy the apPBuffer object details
    _width = other._width;
    _height = other._height;
    _pbufferHandler = other._pbufferHandler;
    _isDeleted = other._isDeleted;
    _pbufferRenderContextSpyId = other._pbufferRenderContextSpyId;
    _pbufferhDC = other._pbufferhDC;
    _bindTarget = other._bindTarget;
    _internalFormat = other._internalFormat;
    _cubemapFace = other._cubemapFace;
    _maxMipmapLevel = other._maxMipmapLevel;
    _mipmapLevel = other._mipmapLevel;

    setAllocatedObjectId(other.getAllocatedObjectId(), true);

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        apPBuffer::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        27/8/2007
// ---------------------------------------------------------------------------
osTransferableObjectType apPBuffer::type() const
{
    return OS_TOBJ_ID_GL_PBUFFER;
}


// ---------------------------------------------------------------------------
// Name:        apPBuffer::writeSelfIntoChannel
// Description: Writes this class into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/8/2007
// ---------------------------------------------------------------------------
bool apPBuffer::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the PBuffer attributes into the channel:
    ipcChannel << (gtInt32)_width;
    ipcChannel << (gtInt32)_height;
    ipcChannel << (gtUInt64)_pbufferHandler;
    ipcChannel << _isDeleted;
    ipcChannel << (gtInt32)_pbufferRenderContextSpyId;
    ipcChannel << (gtUInt64)_pbufferhDC;
    ipcChannel << (gtInt32)_bindTarget;
    ipcChannel << (gtInt32)_internalFormat;
    ipcChannel << (gtInt32)_cubemapFace;
    ipcChannel << (gtInt32)_maxMipmapLevel;
    ipcChannel << (gtInt32)_mipmapLevel;

    apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apPBuffer::readSelfFromChannel
// Description: Read self from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        27/8/2007
// ---------------------------------------------------------------------------
bool apPBuffer::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the PBuffer attributes into the channel:
    gtInt32 widthAsInt32 = 0;
    ipcChannel >> widthAsInt32;
    _width = (GLint)widthAsInt32;
    gtInt32 heightAsInt32 = 0;
    ipcChannel >> heightAsInt32;
    _height = (GLint)heightAsInt32;
    gtUInt64 pbufferHandlerAsUInt64 = 0;
    ipcChannel >> pbufferHandlerAsUInt64;
    _pbufferHandler = (oaPBufferHandle)pbufferHandlerAsUInt64;
    ipcChannel >> _isDeleted;
    gtInt32 pbufferRenderContextSpyIdAsInt32 = 0;
    ipcChannel >> pbufferRenderContextSpyIdAsInt32;
    _pbufferRenderContextSpyId = (int)pbufferRenderContextSpyIdAsInt32;
    gtUInt64 pbufferhDCAsUInt64 = 0;
    ipcChannel >> pbufferhDCAsUInt64;
    _pbufferhDC = (oaDeviceContextHandle)pbufferhDCAsUInt64;
    gtInt32 bindTargetAsUInt = AP_UNDEFINED_PBUFFER;
    ipcChannel >> bindTargetAsUInt;
    _bindTarget = (pbufferBindTarget)bindTargetAsUInt;
    gtInt32 internalFormatAsUInt = AP_UNDEFINED_FORMAT_PBUFFER;
    ipcChannel >> internalFormatAsUInt;
    _internalFormat = (pbufferInternalFormat)internalFormatAsUInt;
    gtInt32 cubemapFaceAsInt32 = 0;
    ipcChannel >> cubemapFaceAsInt32;
    _cubemapFace = (GLenum)cubemapFaceAsInt32;
    gtInt32 maxMipmapLevelAsInt32 = 0;
    ipcChannel >> maxMipmapLevelAsInt32;
    _maxMipmapLevel = (GLint)maxMipmapLevelAsInt32;
    gtInt32 mipmapLevelAsInt32 = 0;
    ipcChannel >> mipmapLevelAsInt32;
    _mipmapLevel = (GLint)mipmapLevelAsInt32;

    apAllocatedObject::readSelfFromChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apPBuffer::setDimensions
// Description: Set the PBuffer width and height
// Arguments:   pbufferWidth, pbufferHeight - The pbuffer width and height
// Return Val:  bool - Success / Failure
// Author:  AMD Developer Tools Team
// Date:        24/8/2007
// ---------------------------------------------------------------------------
bool apPBuffer::setDimensions(GLint pbufferWidth, GLint pbufferHeight)
{
    bool retVal = false;

    // Check that we have reasonable values:
    GT_IF_WITH_ASSERT(pbufferWidth >= 0 && pbufferHeight >= 0)
    {
        // Set the PBuffer width and height
        _width = pbufferWidth;
        _height = pbufferHeight;

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apPBuffer::releasePBufferHDC
// Description: Release the PBuffer device context
// Arguments:   pbufferhDC - The PBuffer hDC to release
// Author:  AMD Developer Tools Team
// Date:        24/8/2007
// ---------------------------------------------------------------------------
void apPBuffer::releasePBufferHDC(oaDeviceContextHandle pbufferhDC)
{
    // Sanity check:
    GT_ASSERT(pbufferhDC == _pbufferhDC)

    // This function is called after openGL has confirmed the hDC was released ok.
    // Therefore, always release the hDC, even if we are holding a different one
    _pbufferhDC = NULL;
}


