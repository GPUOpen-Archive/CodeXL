//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLTextureMiplevelData.cpp
///
//==================================================================================

// -----------------------------   apGLTextureMiplevelData.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>

// Local:
#include <AMDTAPIClasses/Include/apGLTextureMiplevelData.h>

// ---------------------------------------------------------------------------
// Name:        apGLTextureMiplevelData::apGLTextureMiplevelData
// Description: Constructor
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        19/11/2008
// ---------------------------------------------------------------------------
apGLTextureMiplevelData::apGLTextureMiplevelData(): _textureName(0), _textureType(AP_UNKNOWN_TEXTURE_TYPE),
    _fboName(0), _requestedInternalPixelFormat(GL_NONE), _texelDataFormat(OA_TEXEL_FORMAT_UNKNOWN), _dataType(OA_UNKNOWN_DATA_TYPE),
    _width(0), _height(0), _depth(0), _borderWidth(0), _minLevel(0), _maxLevel(1000), _pbufferName(-1),
    _pbufferStaticBuffer(AP_DISPLAY_BUFFER_UNKNOWN), _bufferName(0), _bufferInternalFormat(GL_NONE),
    _openCLImageIndex(-1), _openCLImageName(-1), _openCLSpyID(-1)
{

}
// ---------------------------------------------------------------------------
// Name:        apGLTextureMiplevelData::readSelfFromChannel
// Description:
// Arguments: osChannel& ipcChannel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        19/11/2008
// Note: The complement to this function is apGLTexture::writeThumbnailDataToChannel
// ---------------------------------------------------------------------------
bool apGLTextureMiplevelData::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the texture attributes into the channel:
    gtUInt32 paramAsUInt32 = 0;
    ipcChannel >> paramAsUInt32;
    _textureName = (GLuint)paramAsUInt32;

    gtInt32 paramAsInt32 = 0;
    ipcChannel >> paramAsInt32;
    _textureType = (apTextureType)paramAsInt32;

    ipcChannel >> paramAsUInt32;
    _fboName = (GLuint)paramAsUInt32;

    ipcChannel >> paramAsInt32;
    _requestedInternalPixelFormat = (GLenum)paramAsInt32;

    ipcChannel >> paramAsUInt32;
    _texelDataFormat = (oaTexelDataFormat)paramAsUInt32;

    ipcChannel >> paramAsUInt32;
    _dataType = (oaDataType)paramAsUInt32;

    // Read texture dimensions:
    gtUInt64 paramAsUInt64 = 0;
    ipcChannel >> paramAsUInt64;
    _width = (GLsizei)paramAsUInt64;

    ipcChannel >> paramAsUInt64;
    _height = (GLsizei)paramAsUInt64;

    ipcChannel >> paramAsUInt64;
    _depth = (GLsizei)paramAsUInt64;

    ipcChannel >> paramAsUInt64;
    _borderWidth = (GLint)paramAsUInt64;

    // Read amount of mip levels:
    ipcChannel >> paramAsUInt32;
    _minLevel = (GLuint)paramAsUInt32;

    ipcChannel >> paramAsUInt32;
    _maxLevel = (GLuint)paramAsUInt32;

    // Read pbuffer details:
    ipcChannel >> paramAsInt32;
    _pbufferName = (int)paramAsInt32;

    ipcChannel >> paramAsInt32;
    _pbufferStaticBuffer = (apDisplayBuffer)paramAsInt32;

    // Read texture buffer details:
    ipcChannel >> paramAsUInt32;
    _bufferName = (GLuint)paramAsUInt32;

    ipcChannel >> paramAsInt32;
    _bufferInternalFormat = (GLenum)paramAsInt32;

    ipcChannel >> paramAsInt32;
    _openCLImageIndex = paramAsInt32;

    ipcChannel >> paramAsInt32;
    _openCLImageName = paramAsInt32;

    ipcChannel >> paramAsInt32;
    _openCLSpyID = paramAsInt32;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMiplevelData::~apGLTextureMiplevelData
// Description: Destructor
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        19/11/2008
// ---------------------------------------------------------------------------
apGLTextureMiplevelData::~apGLTextureMiplevelData()
{

}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMiplevelData::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        19/11/2008
// ---------------------------------------------------------------------------
osTransferableObjectType apGLTextureMiplevelData::type() const
{
    return OS_TOBJ_ID_GL_TEXTURE_THUMBNAIL_DATA;
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureMiplevelData::amountOfTextureDataFiles
// Description:
//   Returns the amount of texture data file paths according to the texture type.
//   Notice that some of the paths may be empty.
// Author:  AMD Developer Tools Team
// Date:        19/11/2008
// ---------------------------------------------------------------------------
int apGLTextureMiplevelData::amountOfTextureDataFiles() const
{
    int retVal = 1;

    if (_textureType == AP_CUBE_MAP_TEXTURE)
    {
        retVal = 6;
    }

    return retVal;
};


// ---------------------------------------------------------------------------
// Name:        apGLTextureMiplevelData::getDimensions
// Description: Get the texture thumbnail data dimensions
// Arguments: GLsizei& width
//            GLsizei& height
//            GLsizei& depth
//            GLsizei& borderWidth
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        20/11/2008
// ---------------------------------------------------------------------------
void apGLTextureMiplevelData::getDimensions(GLsizei& width, GLsizei& height, GLsizei& depth, GLsizei& borderWidth)const
{
    width = _width;
    height = _height;
    depth = _depth;
    borderWidth = _borderWidth;
}



// ---------------------------------------------------------------------------
// Name:        apGLTextureMiplevelData::writeSelfIntoChannel
// Description: Thus function should not be called.
// Arguments: osChannel& ipcChannel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/11/2008
// ---------------------------------------------------------------------------
bool apGLTextureMiplevelData::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    (void)(ipcChannel); // unused
    GT_ASSERT(false);
    return false;
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureMiplevelData::getPBufferName
// Description: Returns the id of the pbuffer attached with CGLTexImagePBuffer
// Author:  AMD Developer Tools Team
// Date:        22/12/2009
// ---------------------------------------------------------------------------
int apGLTextureMiplevelData::getPBufferName() const
{
    return _pbufferName;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::setPBufferName
// Description: Sets the id of the pbuffer attached with CGLTexImagePBuffer
// Author:  AMD Developer Tools Team
// Date:        22/12/2009
// ---------------------------------------------------------------------------
void apGLTextureMiplevelData::setPBufferName(int pbuffer)
{
    _pbufferName = pbuffer;
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMiplevelData::getPBufferStaticBuffer
// Description: Returns the source static buffer (which buffer is used to fill
//              the texture) - according to the CGL spec, should only be
//              GL_FRONT or GL_BACK.
// Author:  AMD Developer Tools Team
// Date:        22/12/2009
// ---------------------------------------------------------------------------
apDisplayBuffer apGLTextureMiplevelData::getPBufferStaticBuffer() const
{
    return _pbufferStaticBuffer;
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMiplevelData::setPBufferStaticBuffer
// Description:
// Arguments: apDisplayBuffer staticBuffer
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        22/12/2009
// ---------------------------------------------------------------------------
void apGLTextureMiplevelData::setPBufferStaticBuffer(apDisplayBuffer staticBuffer)
{
    _pbufferStaticBuffer = staticBuffer;
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureData::apGLTextureData
// Description: Constructor
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        16/5/2010
// ---------------------------------------------------------------------------
apGLTextureData::apGLTextureData(): _textureName(0), _textureType(AP_UNKNOWN_TEXTURE_TYPE), _minLevel(0), _maxLevel(1000),
    _openCLImageIndex(-1), _openCLImageName(-1), _openCLSpyID(-1)
{

}

// ---------------------------------------------------------------------------
// Name:        apGLTextureData::readSelfFromChannel
// Description:
// Arguments: osChannel& ipcChannel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        16/5/2010
// Note: The complement to this function is apGLTexture::writeTextureDataToChannel
// ---------------------------------------------------------------------------
bool apGLTextureData::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the texture attributes into the channel:
    gtUInt32 textureNameAsUInt32 = 0;
    ipcChannel >> textureNameAsUInt32;
    _textureName = (GLuint)textureNameAsUInt32;

    gtInt32 varAsInt32 = 0;
    ipcChannel >> varAsInt32;
    _textureType = (apTextureType)varAsInt32;

    // Read amount of mip levels:
    gtUInt32 minLevelAsUInt32 = 0;
    ipcChannel >> minLevelAsUInt32;
    _minLevel = (GLuint)minLevelAsUInt32;
    gtUInt32 maxLevelAsUInt32 = 0;
    ipcChannel >> maxLevelAsUInt32;
    _maxLevel = (GLuint)maxLevelAsUInt32;

    // Read the OpenCL share information:
    ipcChannel >> varAsInt32;
    _openCLImageIndex = varAsInt32;

    ipcChannel >> varAsInt32;
    _openCLImageName = varAsInt32;

    ipcChannel >> varAsInt32;
    _openCLSpyID = varAsInt32;


    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureData::~apGLTextureData
// Description: Destructor
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        16/5/2010
// ---------------------------------------------------------------------------
apGLTextureData::~apGLTextureData()
{

}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMiplevelData::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        16/5/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apGLTextureData::type() const
{
    return OS_TOBJ_ID_GL_TEXTURE_DATA;
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMiplevelData::writeSelfIntoChannel
// Description: Thus function should not be called.
// Arguments: osChannel& ipcChannel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        16/5/2010
// ---------------------------------------------------------------------------
bool apGLTextureData::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    (void)(ipcChannel); // unused
    GT_ASSERT(false);
    return false;
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureMemoryData::apGLTextureMemoryData
// Description: Constructor
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        24/5/2010
// ---------------------------------------------------------------------------
apGLTextureMemoryData::apGLTextureMemoryData():
    _textureName(0), _textureType(AP_UNKNOWN_TEXTURE_TYPE), _minLevel(0), _maxLevel(1000),
    _width(0), _height(0), _depth(0), _memorySize(0), _isMemoryEstimated(false), _isMemoryCalculated(false),
    _bufferInternalFormat(GL_NONE), _usedInternalPixelFormat(GL_NONE), _requestedInternalPixelFormat(GL_NONE),
    _mipmapType(AP_MIPMAP_NONE), _openCLImageIndex(-1), _openCLImageName(-1), _openCLSpyID(-1)
{

}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMemoryData::readSelfFromChannel
// Description:
// Arguments: osChannel& ipcChannel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        24/5/2010
// Note: The complement to this function is apGLTexture::writeTextureMemoryDataToChannel
// ---------------------------------------------------------------------------
bool apGLTextureMemoryData::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the texture attributes into the channel:
    gtUInt32 textureNameAsUInt32 = 0;
    ipcChannel >> textureNameAsUInt32;
    _textureName = (GLuint)textureNameAsUInt32;

    gtInt32 textureTypeAsInt32 = 0;
    ipcChannel >> textureTypeAsInt32;
    _textureType = (apTextureType)textureTypeAsInt32;

    // Read amount of mip levels:
    gtUInt32 minLevelAsUInt32 = 0;
    ipcChannel >> minLevelAsUInt32;
    _minLevel = (GLuint)minLevelAsUInt32;
    gtUInt32 maxLevelAsUInt32 = 0;
    ipcChannel >> maxLevelAsUInt32;
    _maxLevel = (GLuint)maxLevelAsUInt32;

    gtUInt64 paramAsUInt64 = 0;
    ipcChannel >> paramAsUInt64;
    _width = (GLsizei)paramAsUInt64;

    ipcChannel >> paramAsUInt64;
    _height = (GLsizei)paramAsUInt64;

    ipcChannel >> paramAsUInt64;
    _depth = (GLsizei)paramAsUInt64;

    ipcChannel >> paramAsUInt64;
    _memorySize = (GLsizei)paramAsUInt64;

    ipcChannel >> _isMemoryEstimated;
    ipcChannel >> _isMemoryCalculated;

    gtInt32 paramAsInt32 = 0;
    ipcChannel >> paramAsInt32;
    _bufferInternalFormat = (GLenum)paramAsInt32;

    ipcChannel >> paramAsInt32;
    _usedInternalPixelFormat = (GLenum)paramAsInt32;

    ipcChannel >> paramAsInt32;
    _requestedInternalPixelFormat = (GLenum)paramAsInt32;

    ipcChannel >> paramAsInt32;
    _mipmapType = (apTextureMipMapType)paramAsInt32;

    ipcChannel >> paramAsInt32;
    _openCLImageIndex = paramAsInt32;

    ipcChannel >> paramAsInt32;
    _openCLImageName = paramAsInt32;

    ipcChannel >> paramAsInt32;
    _openCLSpyID = paramAsInt32;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMemoryData::~apGLTextureMemoryData
// Description: Destructor
// Return Val:
// Author:  AMD Developer Tools Team
// Date:        24/5/2010
// ---------------------------------------------------------------------------
apGLTextureMemoryData::~apGLTextureMemoryData()
{

}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMemoryData::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        24/5/2010
// ---------------------------------------------------------------------------
osTransferableObjectType apGLTextureMemoryData::type() const
{
    return OS_TOBJ_ID_GL_TEXTURE_MEMORY_DATA;
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMemoryData::writeSelfIntoChannel
// Description: Thus function should not be called.
// Arguments:   osChannel& ipcChannel
// Return Val:  bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        24/5/2010
// ---------------------------------------------------------------------------
bool apGLTextureMemoryData::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    (void)(ipcChannel); // unused
    GT_ASSERT(false);
    return false;
}



// ---------------------------------------------------------------------------
// Name:        apGLTextureMemoryData::getDimensions
// Description:
// Arguments:   GLsizei& width
//              GLsizei& height
//              GLsizei& depth
// Return Val:  void
// Author:  AMD Developer Tools Team
// Date:        24/5/2010
// ---------------------------------------------------------------------------
void apGLTextureMemoryData::getDimensions(GLsizei& width, GLsizei& height, GLsizei& depth) const
{
    width = _width;
    height = _height;
    depth = _depth;
}



// ---------------------------------------------------------------------------
// Name:        apGLTextureMemoryData::getMemorySize
// Description:
// Arguments:   gtSize_t& memorySize
//              bool& isMemorySizeEstimated
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        24/5/2010
// ---------------------------------------------------------------------------
bool apGLTextureMemoryData::getMemorySize(gtSize_t& memorySize, bool& isMemorySizeEstimated) const
{
    memorySize = _memorySize;
    isMemorySizeEstimated = _isMemoryEstimated;
    return _isMemoryCalculated;
}

