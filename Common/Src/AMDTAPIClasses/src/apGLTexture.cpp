//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLTexture.cpp
///
//==================================================================================

// -----------------------------   apGLTexture.cpp ------------------------------
#include <math.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>

// Local:
#include <AMDTAPIClasses/Include/apStringConstants.h>
#include <AMDTAPIClasses/Include/apParameters.h>
#include <AMDTAPIClasses/Include/ap2DRectangle.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>



// ---------------------------------------------------------------------------
// Name:        apGLTexture::apGLTexture
// Description: Constructor
// Arguments:   textureName - The OpenGL texture name.
// Author:  AMD Developer Tools Team
// Date:        19/12/2004
// ---------------------------------------------------------------------------
apGLTexture::apGLTexture(GLuint textureName)
    : apAllocatedObject(), _textureName(textureName), _textureType(AP_UNKNOWN_TEXTURE_TYPE),
      _pCropRectangle(NULL), _fboName(0), _pbufferName(-1), _pbufferStaticBuffer(AP_DISPLAY_BUFFER_UNKNOWN),
      _bufferName(0), _bufferInternalFormat(GL_NONE), _isOpenGLESTexture(false),
      _openCLImageIndex(-1), _openCLImageName(-1), _openCLSpyID(-1)
{
    // Add mip-level 0 to the texture:
    // TO_DO: When supporting mip map levels, we need to create level 0 only when it is
    //        really created (on glTexImageXX)
    apGLTextureMipLevel* pNewTextureMipLevel = new apGLTextureMipLevel;


    _textureMipLevels.push_back(pNewTextureMipLevel);
}


// ---------------------------------------------------------------------------
// Name:        apGLTexture::gsGLTexture
// Description: Copy constructor
// Arguments: other - The other texture class from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        2/7/2006
// ---------------------------------------------------------------------------
apGLTexture::apGLTexture(const apGLTexture& other)
    : _pCropRectangle(NULL), _fboName(0)
{
    apGLTexture::operator=(other);
}


// ---------------------------------------------------------------------------
// Name:        apGLTexture::~apGLTexture
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        25/12/2004
// ---------------------------------------------------------------------------
apGLTexture::~apGLTexture()
{
    // Delete allocated data:
    clear();
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::operator=
// Description: Assignment operator
// Arguments:   other - The other object from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        30/12/2004
// ---------------------------------------------------------------------------
apGLTexture& apGLTexture::operator=(const apGLTexture& other)
{
    _textureName = other._textureName;
    _textureType = other._textureType;

    setAllocatedObjectId(other.getAllocatedObjectId(), true);

    // Copy texture parameters object:
    _textureParameters = other._textureParameters;

    // Clear allocated values:
    clear();

    // Copy crop rectangle parameter
    ap2DRectangle* pCropRectangle = NULL;

    if (other._pCropRectangle)
    {
        pCropRectangle = (ap2DRectangle*)(other._pCropRectangle->clone());
    }

    _pCropRectangle = pCropRectangle;

    _fboName = other._fboName;
    _pbufferName = other._pbufferName;
    _pbufferStaticBuffer = other._pbufferStaticBuffer;
    _isOpenGLESTexture = other._isOpenGLESTexture;

    // Delete current mip levels:
    _textureMipLevels.deleteElementsAndClear();

    // Copy the texture mip levels:
    int amountOfMipLevels = (int)other._textureMipLevels.size();

    for (int i = 0; i < amountOfMipLevels; i++)
    {
        apGLTextureMipLevel* pNewTextureMipLevel = NULL;
        apGLTextureMipLevel* pOtherTextureMipLevel = other._textureMipLevels[i];

        if (pOtherTextureMipLevel != NULL)
        {
            pNewTextureMipLevel = new apGLTextureMipLevel(*pOtherTextureMipLevel);
        }

        _textureMipLevels.push_back(pNewTextureMipLevel);
    }

    _openCLImageIndex = other._openCLImageIndex;
    _openCLImageName = other._openCLImageName;
    _openCLSpyID = other._openCLSpyID;

    return *this;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::getTextureImageFilePath
// Description: Generates texture image file path
// Arguments:   fileType - File type to save the image in
//              filePath - Output texture image file path
//              fileIndex - Texture bind target index
//              int mipLevel - the requested mipmap level
// Return Val:  bool - success / failure.
// Author:  AMD Developer Tools Team
// Date:        30/1/2008
// ---------------------------------------------------------------------------
bool apGLTexture::getTextureImageFilePath(apFileType fileType, const osFilePath*& pFilePath, int fileIndex, int mipLevel)
{
    (void)(mipLevel); // unused
    bool retVal = false;

    // Convert file type to extension
    gtString fileExtension;
    bool rc1 = apFileTypeToFileExtensionString(fileType, fileExtension);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Get texture raw data file path
        bool rc2 = getTextureDataFilePath(pFilePath, fileIndex);
        GT_IF_WITH_ASSERT(rc2 && (nullptr != pFilePath))
        {
            // Set the file extension to the image file path
            gtString outFileExt;
            pFilePath->getFileExtension(outFileExt);
            GT_ASSERT(fileExtension == outFileExt);
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::addDefaultGLTextureParameters
// Description: Adds the OpenGL default texture parameters.
//              int mipLevel - the requested mipmap level
// Return Val: bool - success / failure.
// Author:  AMD Developer Tools Team
// Date:        25/12/2004
// ---------------------------------------------------------------------------
void apGLTexture::addDefaultGLTextureParameters()
{
    GT_ASSERT(!_isOpenGLESTexture);

    // Add texture default values:
    // NOTICE: this check is not a sanity check, we can get to this functions
    // few times, and we want the default parameters to be added only once.
    if (_textureParameters.amountOfTextureParameters() == 0)
    {
        if ((_textureType != AP_2D_TEXTURE_MULTISAMPLE_ARRAY) && (_textureType != AP_2D_TEXTURE_MULTISAMPLE))
        {
            _textureParameters.addDefaultGLTextureParameters();
        }
    }

    // Add default parameters for each of the texture mip levels:
    int amountOfMipLevels = (int)_textureMipLevels.size();

    for (int i = 0; i < amountOfMipLevels; i++)
    {
        // Get the current texture level:
        apGLTextureMipLevel* pTextureLevel = getTextureMipLevel(i);

        if (pTextureLevel != NULL)
        {
            pTextureLevel->addDefaultGLTextureLevelsParameters(_textureType);
        }
    }

}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::addDefaultGLESTextureParameters
// Description: Adds the OpenGL ES default texture parameters to texture mip level 0.
//              TODO: implement for all levels
// Return Val: bool - success / failure.
// Date:        25/12/2004
// ---------------------------------------------------------------------------
void apGLTexture::addDefaultGLESTextureParameters()
{
    GT_ASSERT(_isOpenGLESTexture);

    if (_textureParameters.amountOfTextureParameters() == 0)
    {
        _textureParameters.addDefaultGLESTextureParameters();
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::setTextureFormats
// Description: Sets the texture data formats.
// Arguments: bindTarget - the texture bind target.
//            internalPixelFormat - The number of color components in the texture.
//            pixelFormat - The format of the pixel data.
//            textureType - The type of the texture.
//              int mipLevel - the requested mipmap level
// Author:  AMD Developer Tools Team
// Date:        3/7/2006
// ---------------------------------------------------------------------------
void apGLTexture::setTextureFormats(GLenum bindTarget, GLint internalPixelFormat, GLenum pixelFormat, GLenum texelsType, int mipLevel)
{
    _textureType = apTextureBindTargetToTextureType(bindTarget);
    apGLTextureMipLevel* pTextureLevel = getTextureMipLevel(mipLevel);
    GT_IF_WITH_ASSERT(pTextureLevel != NULL)
    {
        pTextureLevel->setTextureFormats(internalPixelFormat, pixelFormat, texelsType);
    }

}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::setTextureType
// Description: Sets the texture's type and makes sure all its faces have the
//              right default parameter values
// Author:  AMD Developer Tools Team
// Date:        25/11/2008
// ---------------------------------------------------------------------------
void apGLTexture::setTextureType(apTextureType texType)
{
    // Set the type:
    _textureType = texType;

    // Make sure the texture default parameters are set for all relevant texture faces:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    // The GRAPIClasses dll is used for both OpenGL and OpenGL ES implementations:
    if (_isOpenGLESTexture)
    {
        // Add OpenGL ES default texture parameters:
        addDefaultGLESTextureParameters();
    }
    else
    {
        // Add OpenGL default texture parameters:
        addDefaultGLTextureParameters();
    }

#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
#ifdef _GR_IPHONE_BUILD
    // Add OpenGL ES default texture parameters:
    addDefaultGLESTextureParameters();
#else
    // Add OpenGL default texture parameters:
    addDefaultGLTextureParameters();
#endif
#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT)
    // OpenGL ES is not supported on Linux, add OpenGL default texture parameters:
    addDefaultGLTextureParameters();
#else
#error Unknown Linux Variant!
#endif
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::getTextureDataFilePath
// Description: Return a texture level 0 file path
// Arguments: osFilePath& filePath&
//            int fileIndex
//              int mipLevel - the requested mipmap level
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        6/10/2008
// ---------------------------------------------------------------------------
bool apGLTexture::getTextureDataFilePath(const osFilePath*& pFilePath, int fileIndex, int mipLevel) const
{
    bool retVal = false;
    const apGLTextureMipLevel* pTextureLevel = getTextureMipLevel(mipLevel);
    GT_IF_WITH_ASSERT(pTextureLevel != NULL)
    {
        pFilePath = pTextureLevel->getTextureDataFilePath(fileIndex);
        GT_IF_WITH_ASSERT(pFilePath != NULL)
        {
            retVal = true;
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTexture::updateTextureDataFile
// Description: Updates the texture data file path.
// Arguments:   bindTarget - the texture bind target.
//              textureDataFilePath - The path of a file that contains the texture data.
// Author:  AMD Developer Tools Team
// Date:        20/1/2005
// ---------------------------------------------------------------------------
void apGLTexture::updateTextureDataFile(GLenum bindTarget, const osFilePath& textureDataFilePath, int mipLevel)
{
    apGLTextureMipLevel* pTextureLevel = getTextureMipLevel(mipLevel);
    GT_IF_WITH_ASSERT(pTextureLevel != NULL)
    {
        pTextureLevel->updateTextureDataFile(bindTarget, textureDataFilePath);
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::setCropRectangle
// Description: Sets the texture crop rectangle.
// Arguments: rectangle - The texture's new crop rectangle.
// Author:  AMD Developer Tools Team
// Date:        10/4/2006
// ---------------------------------------------------------------------------
void apGLTexture::setCropRectangle(const ap2DRectangle& rectangle)
{
    // If we don't have a crop rectangle:
    if (!_pCropRectangle)
    {
        _pCropRectangle = new ap2DRectangle(rectangle);
        GT_ASSERT(_pCropRectangle);
    }
    else
    {
        *_pCropRectangle = rectangle;
    }
}


// ---------------------------------------------------------------------------
// Name:        apGLTexture::getCropRectangle
// Description: Retrieves the texture's crop rectangle.
// Author:  AMD Developer Tools Team
// Date:        10/4/2006
// ---------------------------------------------------------------------------
bool apGLTexture::getCropRectangle(ap2DRectangle& rectangle) const
{
    bool retVal = false;

    // Crop rectangles are currently supported only for 2D textures:
    GT_IF_WITH_ASSERT(_textureType == AP_2D_TEXTURE)
    {
        retVal = true;

        // If we have a defined crop rectangle:
        if (_pCropRectangle)
        {
            rectangle = *_pCropRectangle;
        }
        else
        {
            GLsizei width, height, depth, borderWidth;
            retVal = getDimensions(width, height, depth, borderWidth);
            GT_ASSERT(retVal);
            // We don't have a defined crop rectangle - return the entire texture size:
            rectangle._xPos = 0;
            rectangle._yPos = 0;
            rectangle._width = float(width);
            rectangle._height = float(height);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::amountOfTextureDataFiles
// Description:
//   Returns the amount of texture data file paths according to the texture type.
//   Notice that some of the paths may be empty.
// Author:  AMD Developer Tools Team
// Date:        3/1/2005
// ---------------------------------------------------------------------------
int apGLTexture::amountOfTextureDataFiles() const
{
    int retVal = 1;

    if (_textureType == AP_CUBE_MAP_TEXTURE)
    {
        retVal = 6;
    }

    return retVal;
};


// ---------------------------------------------------------------------------
// Name:        apGLTexture::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        19/12/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLTexture::type() const
{
    return OS_TOBJ_ID_GL_TEXTURE;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::writeSelfIntoChannel
// Description: Writes this class into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        19/12/2004
// ---------------------------------------------------------------------------
bool apGLTexture::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the texture attributes into the channel:
    ipcChannel << (gtUInt32)_textureName;
    ipcChannel << (gtInt32)_textureType;

    // Write the texture's crop rectangle:
    if (!_pCropRectangle)
    {
        ipcChannel << false;
    }
    else
    {
        ipcChannel << true;
        _pCropRectangle->writeSelfIntoChannel(ipcChannel);
    }

    ipcChannel << (gtUInt32)_fboName;
    ipcChannel << (gtInt32)_pbufferName;
    ipcChannel << (gtInt32)_pbufferStaticBuffer;
    ipcChannel << _isOpenGLESTexture;

    // Write the texture parameters:
    _textureParameters.writeSelfIntoChannel(ipcChannel);

    // Write the amount of mipmap levels to the channel:
    gtUInt64 amountOfTextureMipLevels = (gtUInt64)_textureMipLevels.size();
    ipcChannel << amountOfTextureMipLevels;

    // Write the texture mip levels:
    for (unsigned int i = 0; i < amountOfTextureMipLevels; i++)
    {
        // Get the i'th level texture mipmap:
        apGLTextureMipLevel* pTextureMipLevel = _textureMipLevels[i];

        bool isLevelPresent = (pTextureMipLevel != NULL);
        ipcChannel << isLevelPresent;

        if (isLevelPresent)
        {
            // Write the texture mip level to the channel:
            ipcChannel << (gtUInt32)i;
            bool rc = pTextureMipLevel->writeSelfIntoChannel(ipcChannel);
            retVal = retVal && rc;
        }
    }

    // Write the texture buffer attributes:
    ipcChannel << (gtUInt32)_bufferName;
    ipcChannel << (gtInt32)_bufferInternalFormat;

    // Write the OpenCL share information:
    ipcChannel << (gtInt32)_openCLImageIndex;
    ipcChannel << (gtInt32)_openCLImageName;
    ipcChannel << (gtInt32)_openCLSpyID;

    apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::writeThumbnailDataToChannel
// Description: Writes the texture thumbnail data into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        19/11/2008
// Note: The complement to this function is apGLTextureMiplevelData::readSelfFromChannel
// ---------------------------------------------------------------------------
bool apGLTexture::writeThumbnailDataToChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the texture attributes into the channel:
    ipcChannel << (gtUInt32)_textureName;
    ipcChannel << (gtInt32)_textureType;
    ipcChannel << (gtUInt32)_fboName;

    // Get the first texture mip-level:
    GLsizei width = 0, height = 0, depth = 0, borderWidth = 0;
    const osFilePath* textureMipLevelDataFilesPaths[6];
    const apGLTextureMipLevel* pTextureMipLevel = getTextureMipLevel(0);
    GT_IF_WITH_ASSERT(pTextureMipLevel != NULL)
    {
        // Write the requested internal pixel format:
        GLenum requestedInternalFormat = requestedInternalPixelFormat();
        ipcChannel << (gtInt32)requestedInternalFormat;

        // Write the texel format:
        oaTexelDataFormat dataFormat = OA_TEXEL_FORMAT_UNKNOWN;
        oaGLEnumToTexelDataFormat(pTextureMipLevel->getPixelFormat(), dataFormat);

        // Get the data type:
        oaDataType dataType = OA_UNKNOWN_DATA_TYPE;
        oaGLEnumToDataType(pTextureMipLevel->getTexelsType(), dataType);
        ipcChannel << (gtUInt32)dataFormat;
        ipcChannel << (gtUInt32)dataType;

        // Get the texture mip-level dimensions:
        pTextureMipLevel->getDimensions(width, height, depth, borderWidth);

        // Write the texture file paths:
        for (int i = 0; i < 6; i++)
        {
            textureMipLevelDataFilesPaths[i] = pTextureMipLevel->getTextureDataFilePath(i);
            retVal = retVal && (textureMipLevelDataFilesPaths[i] != NULL);
        }
    }

    // Write the texture mip-level attributes to the channel:
    ipcChannel << (gtUInt64)width;
    ipcChannel << (gtUInt64)height;
    ipcChannel << (gtUInt64)depth;
    ipcChannel << (gtUInt64)borderWidth;

    // Write the texture mip levels:
    GLuint minLevel = 0;
    GLuint maxLevel = 1000;
    bool rc = getTextureMinMaxLevels(minLevel, maxLevel);
    GT_ASSERT(rc);
    ipcChannel << (gtUInt32)minLevel;
    ipcChannel << (gtUInt32)maxLevel;

    // Write the pbuffer:
    ipcChannel << (gtInt32)_pbufferName;
    ipcChannel << (gtInt32)_pbufferStaticBuffer;

    // Write the texture buffer attributes:
    ipcChannel << (gtUInt32)_bufferName;
    ipcChannel << (gtInt32)_bufferInternalFormat;

    // Write the OpenCL share information:
    ipcChannel << (gtInt32)_openCLImageIndex;
    ipcChannel << (gtInt32)_openCLImageName;
    ipcChannel << (gtInt32)_openCLSpyID;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTexture::writeTextureDataToChannel
// Description: Writes the texture data into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        16/5/2010
// Note: The complement to this function is apGLTextureData::readSelfFromChannel
// ---------------------------------------------------------------------------
bool apGLTexture::writeTextureDataToChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the texture attributes into the channel:
    ipcChannel << (gtUInt32)_textureName;
    ipcChannel << (gtInt32)_textureType;

    // Write the texture mip levels:
    GLuint minLevel = 0;
    GLuint maxLevel = 1000;
    bool rc = getTextureMinMaxLevels(minLevel, maxLevel);
    GT_ASSERT(rc);
    ipcChannel << (gtUInt32)minLevel;
    ipcChannel << (gtUInt32)maxLevel;

    // Write the OpenCL share information:
    ipcChannel << (gtInt32)_openCLImageIndex;
    ipcChannel << (gtInt32)_openCLImageName;
    ipcChannel << (gtInt32)_openCLSpyID;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTexture::writeTextureMemoryDataToChannel
// Description: Writes the texture memory data into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        16/5/2010
// Note: The complement to this function is apGLTextureMemoryData::readSelfFromChannel
// ---------------------------------------------------------------------------
bool apGLTexture::writeTextureMemoryDataToChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the texture attributes into the channel:
    ipcChannel << (gtUInt32)_textureName;
    ipcChannel << (gtInt32)_textureType;

    // Write the texture mip levels:
    GLuint minLevel = 0;
    GLuint maxLevel = 1000;
    bool rc = getTextureMinMaxLevels(minLevel, maxLevel);
    GT_ASSERT(rc);
    ipcChannel << (gtUInt32)minLevel;
    ipcChannel << (gtUInt32)maxLevel;

    // Get dimensions:
    GLsizei width = 0, height = 0, depth = 0, borderWidth = 0;
    rc = getDimensions(width, height, depth, borderWidth);
    GT_ASSERT(rc);

    ipcChannel << (gtUInt64)width;
    ipcChannel << (gtUInt64)height;
    ipcChannel << (gtUInt64)depth;

    // Calculate the memory size:
    gtSize_t memorySize = 0;
    bool isMemoryEstimated = true;
    bool isMemoryCalculated = getMemorySize(memorySize, isMemoryEstimated);

    ipcChannel << (gtUInt64)memorySize;
    ipcChannel << isMemoryEstimated;
    ipcChannel << isMemoryCalculated;

    GLenum usedInternalFormat = usedInternalPixelFormat();
    GLenum requestedInternalFormat = requestedInternalPixelFormat();

    ipcChannel << (gtInt32)_bufferInternalFormat;
    ipcChannel << (gtInt32)usedInternalFormat;
    ipcChannel << (gtInt32)requestedInternalFormat;

    // Get the mipmap type:
    apTextureMipMapType mipmapType = getTextureMipmapType();
    ipcChannel << (gtInt32)mipmapType;

    // Write the OpenCL share information:
    ipcChannel << (gtInt32)_openCLImageIndex;
    ipcChannel << (gtInt32)_openCLImageName;
    ipcChannel << (gtInt32)_openCLSpyID;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTexture::getTextureThumbnailData
// Description: Return a texture thumbnail data
// Arguments: apGLTextureMiplevelData& textureThumbData
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        19/11/2008
// ---------------------------------------------------------------------------
bool apGLTexture::getTextureThumbnailData(apGLTextureMiplevelData& textureThumbData) const
{
    bool retVal = false;

    // Set the texture attributes into the channel:
    textureThumbData._textureName = _textureName;
    textureThumbData._textureType = _textureType;

    textureThumbData._fboName = _fboName;

    // Get the first texture mip-level:
    GLsizei width = 0, height = 0, depth = 0, borderWidth = 0;
    const osFilePath* textureMipLevelDataFilesPaths[6];
    const apGLTextureMipLevel* pTextureMipLevel = getTextureMipLevel(0);
    GT_IF_WITH_ASSERT(pTextureMipLevel != NULL)
    {
        retVal = true;
        // Get the texture mip-level dimensions:
        pTextureMipLevel->getDimensions(width, height, depth, borderWidth);

        // Write the texture file paths:
        for (int i = 0; i < 6; i++)
        {
            textureMipLevelDataFilesPaths[i] = pTextureMipLevel->getTextureDataFilePath(i);
            retVal = retVal && textureMipLevelDataFilesPaths[i] != NULL;
        }
    }

    // Set the texture thumbnails dimensions:
    textureThumbData._width = width;
    textureThumbData._height = height;
    textureThumbData._depth = depth;
    textureThumbData._borderWidth = borderWidth;

    // Set the texture amount of mip levels:
    textureThumbData._minLevel = 0;
    textureThumbData._maxLevel = 1000;
    bool rc = getTextureMinMaxLevels(textureThumbData._minLevel, textureThumbData._maxLevel);
    GT_ASSERT(rc);

    // Set the texture buffer attributes:
    textureThumbData._bufferInternalFormat = _bufferInternalFormat;
    textureThumbData._bufferName = _bufferName;

    // Set the OpenCL share information:
    textureThumbData._openCLImageIndex = _openCLImageIndex;
    textureThumbData._openCLImageName = _openCLImageName;
    textureThumbData._openCLSpyID = _openCLSpyID;

    return true;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::readSelfFromChannel
// Description: Read self from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/12/2004
// ---------------------------------------------------------------------------
bool apGLTexture::readSelfFromChannel(osChannel& ipcChannel)
{
    clear();

    bool retVal = true;

    // Read the texture attributes into the channel:
    gtUInt32 varAsUInt32 = 0;
    ipcChannel >> varAsUInt32;
    _textureName = (GLuint)varAsUInt32;

    gtInt32 varAsInt32 = 0;
    ipcChannel >> varAsInt32;
    _textureType = (apTextureType)varAsInt32;

    // Read the texture's crop rectangle:
    bool cropRectangleExists = false;
    ipcChannel >> cropRectangleExists;

    if (cropRectangleExists)
    {
        _pCropRectangle = new ap2DRectangle;
        GT_IF_WITH_ASSERT(_pCropRectangle != NULL)
        {
            _pCropRectangle->readSelfFromChannel(ipcChannel);
        }
    }

    ipcChannel >> varAsUInt32;
    _fboName = (GLuint)varAsUInt32;

    ipcChannel >> varAsInt32;
    _pbufferName = (int)varAsInt32;

    ipcChannel >> varAsInt32;
    _pbufferStaticBuffer = (apDisplayBuffer)varAsInt32;
    ipcChannel >> _isOpenGLESTexture;

    // Read the texture parameters:
    _textureParameters.readSelfFromChannel(ipcChannel);

    gtUInt64 textureMipMapLevels = 0;
    ipcChannel >> textureMipMapLevels;

    // Read the texture mip levels:
    for (gtUInt64 i = 0; i < textureMipMapLevels; i++)
    {
        bool isLevelPresent = false;
        ipcChannel >> isLevelPresent;

        if (isLevelPresent)
        {
            // Read the current mip level:
            gtUInt32 textureLevel;
            ipcChannel >> textureLevel;

            apGLTextureMipLevel* pTextureMipLevel = new apGLTextureMipLevel;

            bool rc = pTextureMipLevel->readSelfFromChannel(ipcChannel);

            GT_IF_WITH_ASSERT(rc)
            {
                // Push nulls if the texture mip levels vector has holes:
                gtUInt64 currentNumberOfLevels = (gtUInt64)_textureMipLevels.size();

                if (textureLevel >= currentNumberOfLevels)
                {
                    for (gtUInt64 j = currentNumberOfLevels; j <= textureLevel; j++)
                    {
                        _textureMipLevels.push_back(NULL);
                    }
                }
            }
            _textureMipLevels[textureLevel] = pTextureMipLevel;
        }
    }

    // Write the texture buffer attributes:
    ipcChannel >> varAsUInt32;
    _bufferName = (GLuint)varAsUInt32;

    ipcChannel >> varAsInt32;
    _bufferInternalFormat = (GLenum)varAsInt32;

    ipcChannel >> varAsInt32;
    _openCLImageIndex = varAsInt32;

    ipcChannel >> varAsInt32;
    _openCLImageName = varAsInt32;

    ipcChannel >> varAsInt32;
    _openCLSpyID = varAsInt32;

    apAllocatedObject::readSelfFromChannel(ipcChannel);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTexture::clear
// Description: Clears (deletes) allocated data.
// Author:  AMD Developer Tools Team
// Date:        10/4/2006
// ---------------------------------------------------------------------------
void apGLTexture::clear()
{
    // Delete the texture's crop rectangle (if any):
    delete _pCropRectangle;
    _pCropRectangle = NULL;

    // Delete the texture mip level elements:
    _textureMipLevels.deleteElementsAndClear();
}


// ---------------------------------------------------------------------------
// Name:        apGLTexture::getTextureMipLevel
// Description: Returns the mipLevel'th level texture object
// Return Val: apGLTextureMipLevel*
// Arguments: int mipLevel - the requested texture mipmap level
// Author:  AMD Developer Tools Team
// Date:        5/10/2008
// ---------------------------------------------------------------------------
apGLTextureMipLevel* apGLTexture::getTextureMipLevel(int mipLevel)
{
    apGLTextureMipLevel* retVal = NULL;
    bool bLevelExist = (size_t)mipLevel < _textureMipLevels.size();

    if (bLevelExist)
    {
        retVal = _textureMipLevels[mipLevel];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::getTextureLogicMipLevel
// Description: Returns the mipLevel'th not NULL level texture object
// Return Val: apGLTextureMipLevel*
// Arguments: int mipLevel - the requested texture mipmap level
// Author:  AMD Developer Tools Team
// Date:        5/10/2008
// ---------------------------------------------------------------------------
apGLTextureMipLevel* apGLTexture::getTextureLogicMipLevel(int mipLevel)
{
    apGLTextureMipLevel* retVal = NULL;
    int mipLevelIndex = 0;

    // Search for the none NULL miplevel'th mip level:
    int amountOfMipLevels = (int)_textureMipLevels.size();

    for (int i = 0; i < amountOfMipLevels; i++)
    {
        if (_textureMipLevels[i] != NULL)
        {
            if (mipLevelIndex == mipLevel)
            {
                retVal = _textureMipLevels[i];
                break;
            }

            mipLevelIndex++;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTexture::getTextureMipLevel
// Description: Returns a class representing a queried texture's mip-map level.
// Arguments: mipLevel - The queries texture mip map level.
// Return Val: const apGLTextureMipLevel* - Will get the queried texture's mip-map level
//                                          or NULL if it does not exist.
// Author:  AMD Developer Tools Team
// Date:        3/11/2008
// ---------------------------------------------------------------------------
const apGLTextureMipLevel* apGLTexture::getTextureMipLevel(int mipLevel) const
{
    const apGLTextureMipLevel* retVal = NULL;

    // If the mip-map level exist:
    size_t amountOfMipLevels = _textureMipLevels.size();
    GT_IF_WITH_ASSERT((size_t)mipLevel < amountOfMipLevels)
    {
        retVal = _textureMipLevels[mipLevel];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::setFBOName
// Description: Sets the texture fbo name
// Arguments: GLuint fboName
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        5/10/2008
// ---------------------------------------------------------------------------
void apGLTexture::setFBOName(GLuint fboName)
{
    _fboName = fboName;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::getPBufferName
// Description: Returns the id of the pbuffer attached with CGLTexImagePBuffer
// Author:  AMD Developer Tools Team
// Date:        15/3/2009
// ---------------------------------------------------------------------------
int apGLTexture::getPBufferName() const
{
    return _pbufferName;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::setPBufferName
// Description: Sets the id of the pbuffer attached with CGLTexImagePBuffer
// Author:  AMD Developer Tools Team
// Date:        15/3/2009
// ---------------------------------------------------------------------------
void apGLTexture::setPBufferName(int pbuffer)
{
    _pbufferName = pbuffer;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::getPBufferStaticBuffer
// Description: Returns the source static buffer (which buffer is used to fill
//              the texture) - according to the CGL spec, should only be
//              GL_FRONT or GL_BACK.
// Author:  AMD Developer Tools Team
// Date:        15/3/2009
// ---------------------------------------------------------------------------
apDisplayBuffer apGLTexture::getPBufferStaticBuffer() const
{
    return _pbufferStaticBuffer;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::setPBufferStaticBuffer
// Description:
// Arguments: apDisplayBuffer staticBuffer
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        15/3/2009
// ---------------------------------------------------------------------------
void apGLTexture::setPBufferStaticBuffer(apDisplayBuffer staticBuffer)
{
    _pbufferStaticBuffer = staticBuffer;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::getFBOName
// Description: Returns fbo name (TODO: support all levels):
// Return Val: int
// Author:  AMD Developer Tools Team
// Date:        5/10/2008
// ---------------------------------------------------------------------------
GLuint apGLTexture::getFBOName()const
{
    return _fboName;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::setDimensions
// Description: Set a texture mip level dimensions.
//              TODO:implement for all levels
// Arguments: GLsizei width
//            GLsizei height
//            GLsizei depth
//            GLsizei borderWidth
//              int mipLevel - the requested mipmap level
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        5/10/2008
// ---------------------------------------------------------------------------
bool apGLTexture::setDimensions(GLsizei width, GLsizei height, GLsizei depth, GLsizei borderWidth, int mipLevel)
{
    bool retVal = false;
    apGLTextureMipLevel* pTextureLevel = getTextureMipLevel(mipLevel);
    GT_IF_WITH_ASSERT(pTextureLevel != NULL)
    {
        pTextureLevel->setDimensions(width, height, depth, borderWidth);
        retVal = true;
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::getDimensions
// Description: Gets a texture mip level getDimensions.
//              TODO:implement for all levels
// Arguments: GLsizei& width
//            GLsizei& height
//            GLsizei& depth
//            GLsizei& borderWidth
//            int mipLevel - the requested mipmap level
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        5/10/2008
// ---------------------------------------------------------------------------
bool apGLTexture::getDimensions(GLsizei& width, GLsizei& height, GLsizei& depth, GLsizei& borderWidth, int mipLevel)const
{
    bool retVal = false;
    const apGLTextureMipLevel* pTextureLevel = getTextureMipLevel(mipLevel);
    GT_IF_WITH_ASSERT(pTextureLevel != NULL)
    {
        pTextureLevel->getDimensions(width, height, depth, borderWidth);
        retVal = true;
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::texelsType
// Description: Gets a texture mip level texel type.
//              int mipLevel - the requested mipmap level
// Return Val: GLenum
// Author:  AMD Developer Tools Team
// Date:        5/10/2008
// ---------------------------------------------------------------------------
GLenum apGLTexture::texelsType(int mipLevel) const
{
    GLenum retVal = 0;
    const apGLTextureMipLevel* pTextureLevel = getTextureMipLevel(mipLevel);
    GT_IF_WITH_ASSERT(pTextureLevel != NULL)
    {
        retVal = pTextureLevel->getTexelsType();
    }
    return retVal;

}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::pixelFormat
// Description: Gets a texture mip level pixel format.
//              int mipLevel - the requested mipmap level
// Return Val: GLenum
// Author:  AMD Developer Tools Team
// Date:        5/10/2008
// ---------------------------------------------------------------------------
GLenum apGLTexture::pixelFormat(int mipLevel) const
{
    GLenum retVal = 0;
    const apGLTextureMipLevel* pTextureLevel = getTextureMipLevel(mipLevel);
    GT_IF_WITH_ASSERT(pTextureLevel != NULL)
    {
        retVal = pTextureLevel->getPixelFormat();
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::requestedInternalPixelFormat
// Description: Gets a texture requested internal pixel format.
//              int mipLevel - the requested mipmap level
// Return Val: GLint
// Author:  AMD Developer Tools Team
// Date:        5/10/2008
// ---------------------------------------------------------------------------
GLint apGLTexture::requestedInternalPixelFormat(int mipLevel) const
{
    GLint retVal = 0;
    const apGLTextureMipLevel* pTextureLevel = getTextureMipLevel(mipLevel);
    GT_IF_WITH_ASSERT(pTextureLevel != NULL)
    {
        retVal = pTextureLevel->getRequestedInternalPixelFormat();
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::usedInternalPixelFormat
// Description: Get the used internal pixel format for the texture mip level
// Author:  AMD Developer Tools Team
// Date:        9/11/2008
// ---------------------------------------------------------------------------
GLenum apGLTexture::usedInternalPixelFormat(int mipLevel) const
{
    GLenum retVal = 0;

    if (_textureType != AP_UNKNOWN_TEXTURE_TYPE)
    {
        // Get the texture level parameters:
        const apGLTextureParams* pTextureLevelParams = textureLevelParameters(mipLevel);
        GT_IF_WITH_ASSERT(pTextureLevelParams != NULL)
        {
            // Get the internal format parameter value:
            bool rc = pTextureLevelParams->getTextureEnumParameterValue(GL_TEXTURE_INTERNAL_FORMAT, retVal);

            if (!rc)
            {
                OS_OUTPUT_DEBUG_LOG(AP_STR_couldNotDetermineTextureInternalFormat, OS_DEBUG_LOG_DEBUG);
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::getMemorySize
// Description: Calculates the texture mipmap levels size
// Arguments: gtSize_t& memorySize - texture memory size
//            bool& isMemoryEstimated - is one (or more) of the textures mip-levels memory size is estimated
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        6/10/2008
// ---------------------------------------------------------------------------
bool apGLTexture::getMemorySize(gtSize_t& memorySize, bool& isMemoryEstimated) const
{
    bool retVal = true;
    isMemoryEstimated = false;
    memorySize = 0;

    // Unknown texture type - memory size is 0 (there are no parameters):
    if (_textureType != AP_UNKNOWN_TEXTURE_TYPE)
    {
        // Summarize each of the texture mipmap level sizes:
        int amountOfMipLevels = (int)_textureMipLevels.size();

        for (int i = 0; i < amountOfMipLevels; i++)
        {
            const apGLTextureMipLevel* pTextureMipmapLevel = _textureMipLevels[i];

            if (pTextureMipmapLevel != NULL)
            {
                gtSize_t currentMemerySize = 0;
                bool isMipLevelMemoryEstimated = false;
                bool rc1  = pTextureMipmapLevel->getMemorySize(currentMemerySize, isMipLevelMemoryEstimated, textureType());
                GT_IF_WITH_ASSERT(rc1)
                {
                    memorySize += currentMemerySize;
                    isMemoryEstimated = isMemoryEstimated || isMipLevelMemoryEstimated;
                }
                retVal = retVal && rc1;
            }
        }

        // Multiply the size by 6 for cube maps:
        if (_textureType == AP_CUBE_MAP_TEXTURE && isMemoryEstimated)
        {
            memorySize *= 6;
        }

    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTexture::getAutoGeneratedMiplevelMemorySize
// Description: Calculate texture auto generated mip-levels size.
//              Notice:
//              rectangle texture - auto generation of mip-levels is not allowed
//              non-power of-two texture dimensions are calculated, even though it is
//              supported only when extension - texture_non_power_of_two - is supported
// Arguments: gtSize_t& autoGeneratedMiplevelsSize
//              bool& isEstimated - is texture auto generated size estimated (when dimensions are
//              not extracted out of the texture base level parameters)
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        24/11/2008
// ---------------------------------------------------------------------------
bool apGLTexture::getAutoGeneratedMiplevelMemorySize(gtSize_t& autoGeneratedMiplevelsSize, bool& isEstimated)const
{
    bool retVal = false;
    autoGeneratedMiplevelsSize = 0;

    isEstimated = false;

    // Check auto generation levels for supported texture types:
    if (_textureType != AP_TEXTURE_RECTANGLE)
    {
        // Get the texture mip-level 0:
        const apGLTextureMipLevel* pTextureLevel = getTextureMipLevel(0);

        // Get the texture level parameters:
        const apGLTextureParams* pMiplevelParams = textureLevelParameters(0);

        GT_IF_WITH_ASSERT(pTextureLevel != NULL && pMiplevelParams != NULL)
        {
            // Get the texture base level dimensions:
            GLsizei width = 0, height = 0, depth = 0, borderSize;
            bool areDimensionsEstimated = false;
            bool rc1 = pTextureLevel->getDimensions(width, height, depth, borderSize, areDimensionsEstimated);
            GT_IF_WITH_ASSERT(rc1)
            {
                GLuint baseLevel = 0, maxLevel = 0;
                bool isMiplevelsNumEstimated = false;
                bool rc2 = calcNumberOfAutoGeneratedMiplevels(baseLevel, maxLevel, isMiplevelsNumEstimated);
                GT_IF_WITH_ASSERT(rc2)
                {
                    isEstimated = areDimensionsEstimated && isMiplevelsNumEstimated;
                    // Get the texture pixel size:
                    bool isPixelSizeEstimated = false;
                    GLuint pixelSize = 0;
                    bool rc3 = pTextureLevel->getPixelSize(pixelSize, isPixelSizeEstimated, _textureType);
                    GT_IF_WITH_ASSERT(rc3)
                    {
                        isEstimated = isEstimated || isPixelSizeEstimated;

                        // Convert the dimensions to float for calculations:
                        float w = (float)width;
                        float h = (float)height;
                        float d = (float)depth;

                        for (unsigned int i = baseLevel; i < maxLevel; i++)
                        {
                            // Get this level dimensions:
                            w = std::max(1.0F, (float)floor(w / 2));
                            h = std::max(1.0F, (float)floor(h / 2));
                            d = std::max(1.0F, (float)floor(d / 2));

                            // Calculate the total number of pixels:
                            float numOfPixels = w * h * d;

                            // Calculated added levels size:
                            gtSize_t currentLevelSize = (gtSize_t)numOfPixels * pixelSize;
                            autoGeneratedMiplevelsSize += currentLevelSize;
                        }

                        retVal = true;
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::getEstimatedMemorySize
// Description: Get the texture mipmap levels estimated size
// Arguments: gtSize_t& memorySize - texture memory size
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        6/10/2008
// ---------------------------------------------------------------------------
bool apGLTexture::getEstimatedMemorySize(gtSize_t& estimatedMemorySize) const
{
    bool retVal = true;
    estimatedMemorySize = 0;
    // Initialize first and last texture faces (for iterating the faces in case of cube map):
    apGLTextureMipLevel::apTextureFaceIndex firstFaceIndex = apGLTextureMipLevel::AP_SINGLE_TEXTURE_FACE_INDEX;
    apGLTextureMipLevel::apTextureFaceIndex lastFaceIndex = apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_NEGATIVE_X_FACE_INDEX;

    if (textureType() == AP_CUBE_MAP_TEXTURE)
    {
        firstFaceIndex = apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_POSITIVE_X_FACE_INDEX;
        lastFaceIndex = apGLTextureMipLevel::AP_MAX_AMOUNT_OF_TEXTURE_FACES;
    }

    // Summarize each of the texture mipmap level sizes:
    int amountOfMipLevels = (int)_textureMipLevels.size();

    for (int i = 0; i < amountOfMipLevels; i++)
    {
        apGLTextureMipLevel* pTextureMipmapLevel = _textureMipLevels[i];

        if (pTextureMipmapLevel != NULL)
        {
            for (int faceIndex = firstFaceIndex; faceIndex < lastFaceIndex; faceIndex++)
            {
                gtSize_t currentMemerySize = 0;
                apGLTextureMipLevel::apTextureFaceIndex textureFaceIndex = (apGLTextureMipLevel::apTextureFaceIndex)faceIndex;
                retVal  = retVal && pTextureMipmapLevel->getEstimatedMemorySize(currentMemerySize, textureFaceIndex);
                estimatedMemorySize += currentMemerySize;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::addTextureMipLevel
// Description:
// Arguments: GLint level- the texture mipmap level
//            GLint internalformat - the mipmap level internal format
//            GLsizei width, height, depth, border - the mipmap level dimensions
//            GLenum format - the mipmap level external format
//            GLenum type - the mipmap level pixel format
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        7/10/2008
// ---------------------------------------------------------------------------
bool apGLTexture::addTextureMipLevel(GLint level, GLint internalformat,
                                     GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type)
{
    bool retVal = true;

    // Get or create the new mipmap level object:
    apGLTextureMipLevel* pNewTextureMipLevel = getTextureMipLevel(level);

    if (pNewTextureMipLevel == NULL)
    {
        pNewTextureMipLevel = new apGLTextureMipLevel;

    }

    // Set the mipmap level dimensions:
    pNewTextureMipLevel->setDimensions(width, height, depth, border);

    // Set the mipmap level formats:
    pNewTextureMipLevel->setTextureFormats(internalformat, format, type);

    // Add default texture level parameters to the texture mipmap level:
    pNewTextureMipLevel->addDefaultGLTextureLevelsParameters(_textureType);

    // Fill the texture mipmaps vector holes (the vector contain "holes" in all the integer places
    // that are not 2 exponents):
    for (int i = (int)_textureMipLevels.size(); i <= level; i++)
    {
        _textureMipLevels.push_back(NULL);
    }

    // Put the new created texture in the texture mipmap levels vector:
    _textureMipLevels[level] = pNewTextureMipLevel;

    // Add the new mipmap level to the texture levels vector:
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTexture::amountOfTextureMipLevels
// Description: Returns the "real" - not nulls mipmap levels
// Arguments:   int& minLevel
//              int& maxLevel
// Return Val: bool Success/Failure
// Author:  AMD Developer Tools Team
// Date:        7/10/2008
// ---------------------------------------------------------------------------
bool apGLTexture::getTextureMinMaxLevels(GLuint& minLevel, GLuint& maxLevel) const
{
    bool retVal = false;

    // Get the texture mipmap type:
    apTextureMipMapType mipmapType = getTextureMipmapType();

    // For manual mipmap, count the levels inserted manualy:
    if (mipmapType == AP_MIPMAP_NONE_MANUAL)
    {
        minLevel = 0;
        maxLevel = (int)_textureMipLevels.size() - 1;
        retVal = true;
    }
    // Calculate the auto generated number of mipmap levels:
    else if (mipmapType == AP_MIPMAP_AUTO_GENERATE)
    {
        bool isEstimated = false;
        retVal = calcNumberOfAutoGeneratedMiplevels(minLevel, maxLevel, isEstimated);
    }
    else
    {
        // No levels:
        minLevel = 0;
        maxLevel = 0;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTexture::calcNumberOfAutoGeneratedMiplevels
// Description: Calculate the number of auto generated mip levels
// Arguments:
//            GLint& baseLevel - the texture base mip level
//            GLint& maxLevel - the texture max mip level
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        14/1/2009
// ---------------------------------------------------------------------------
bool apGLTexture::calcNumberOfAutoGeneratedMiplevels(GLuint& baseLevel, GLuint& maxLevel, bool& isEstimated)const
{
    (void)(isEstimated); // unused
    bool retVal = false;

    // Check auto generation levels for supported texture types:
    if (_textureType != AP_TEXTURE_RECTANGLE)
    {
        // Get the texture mip-level 0:
        const apGLTextureMipLevel* pTextureLevel = getTextureMipLevel(0);

        // Get the texture level parameters:
        const apGLTextureParams* pMiplevelParams = textureLevelParameters(0);
        GT_IF_WITH_ASSERT(pTextureLevel != NULL && pMiplevelParams != NULL)
        {
            // Get the texture base level dimensions:
            GLsizei width = 0, height = 0, depth = 0, borderSize;
            bool areDimensionsEstimated = false;
            bool rc1 = pTextureLevel->getDimensions(width, height, depth, borderSize, areDimensionsEstimated);
            GT_IF_WITH_ASSERT(rc1)
            {
                GLsizei minDimension = width;

                // Get the smallest dimension:
                if ((_textureType != AP_1D_TEXTURE) && (_textureType != AP_BUFFER_TEXTURE))
                {
                    minDimension = std::min(width, height);
                }

                if ((_textureType == AP_3D_TEXTURE) || (_textureType == AP_2D_ARRAY_TEXTURE))
                {
                    minDimension = std::min(minDimension, depth);
                }

                // Compute number of levels:
                baseLevel = 0;
                maxLevel = (GLuint)floor(logf((float)minDimension) / logf(2));

                // Get the user input max and base levels:
                GLuint userMaxLevel = maxLevel;
                GLuint userBaseLevel = baseLevel;
                bool rc2 = getMipmapBaseMaxLevels(userBaseLevel, userMaxLevel);
                // This function always fails on the iPhone, since the parameters needed for it
                // are not supported by OpenGL ES
                GT_IF_WITH_ASSERT(rc2)
                {
                    // The texture base and max level as parameters should be intersected with the possible
                    // base and max levels, as computed from the texture dimensions:
                    baseLevel = (GLuint)std::max(baseLevel, userBaseLevel);
                    maxLevel = (GLuint)std::min(maxLevel, userMaxLevel);
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::getTextureMipmapType
// Description: Returns the texture mipmap type
// Return Val: apTextureMipMapType
// Author:  AMD Developer Tools Team
// Date:        7/10/2008
// ---------------------------------------------------------------------------
apTextureMipMapType apGLTexture::getTextureMipmapType()const
{
    apTextureMipMapType retVal = AP_MIPMAP_NONE;

    // Unknown texture types - means that the texture parameters do not exist:
    if (_textureType != AP_UNKNOWN_TEXTURE_TYPE)
    {
        // Search for a mipmap auto generation parameter:
        const apGLTextureMipLevel* pTextureMipmapLevel = getTextureMipLevel(0);
        GT_IF_WITH_ASSERT(pTextureMipmapLevel != NULL)
        {
            // Get mipmap generation parameter index:
            GLboolean isAutomaticMipMap = false;
            bool rc = _textureParameters.getTextureBoolenParameterValue(GL_GENERATE_MIPMAP, isAutomaticMipMap);

            if (!rc)
            {
                retVal = AP_MIPMAP_NONE;
            }

            if (isAutomaticMipMap == GL_TRUE)
            {
                retVal = AP_MIPMAP_AUTO_GENERATE;
            }
        }

        // If type is still none - check the number of texture levels:
        if (retVal == AP_MIPMAP_NONE)
        {
            if (_textureMipLevels.size() > 1)
            {
                retVal = AP_MIPMAP_NONE_MANUAL;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::getTextureMipLevelAllocatedObjectId
// Description: Gets the allocated Object Id of a mip level. -2 if the level doesn't exist,
//              -1 if it exists but has no Id.
// Author:  AMD Developer Tools Team
// Date:        22/10/2008
// ---------------------------------------------------------------------------
int apGLTexture::getTextureMipLevelAllocatedObjectId(GLint level)
{
    int retVal = -2;

    apGLTextureMipLevel* mipLevel = getTextureMipLevel(level);

    if (mipLevel != NULL)
    {
        retVal = mipLevel->getAllocatedObjectId();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::setTextureMipLevelAllocatedObjectId
// Description: Sets the allocated object Id of a mip level.
// Author:  AMD Developer Tools Team
// Date:        22/10/2008
// ---------------------------------------------------------------------------
void apGLTexture::setTextureMipLevelAllocatedObjectId(GLint level, int allocatedId)
{
    apGLTextureMipLevel* mipLevel = getTextureMipLevel(level);

    GT_IF_WITH_ASSERT(mipLevel != NULL)
    {
        // This will notify us if we allocated the same mip level twice:
        mipLevel->setAllocatedObjectId(allocatedId);
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::textureLevelParameters
// Description: Return a texture level parameters
// Arguments: int level - the texture mip level
//            apGLTextureMipLevel::apTextureFaceIndex texturefaceIndex - the texture face index (cube map face)
// Return Val: const apGLTextureParams*
// Author:  AMD Developer Tools Team
// Date:        10/11/2008
// ---------------------------------------------------------------------------
apGLTextureParams* apGLTexture::textureLevelParameters(int level, apGLTextureMipLevel::apTextureFaceIndex texturefaceIndex)
{
    apGLTextureParams*  retVal = NULL;
    apGLTextureMipLevel* mipLevel = getTextureMipLevel(level);

    if (mipLevel != NULL)
    {
        retVal = &(mipLevel->_textureLevelParameters[texturefaceIndex]);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::textureLevelParameters
// Description: Return a texture level parameters
// Arguments: int level - the texture mip level
//            apGLTextureMipLevel::apTextureFaceIndex texturefaceIndex - the texture face index (cube map face)
// Return Val: const apGLTextureParams*
// Author:  AMD Developer Tools Team
// Date:        10/11/2008
// ---------------------------------------------------------------------------
const apGLTextureParams* apGLTexture::textureLevelParameters(int level, apGLTextureMipLevel::apTextureFaceIndex texturefaceIndex)const
{
    const apGLTextureParams*  retVal = NULL;
    const apGLTextureMipLevel* mipLevel = getTextureMipLevel(level);

    if (mipLevel != NULL)
    {
        retVal = &(mipLevel->_textureLevelParameters[texturefaceIndex]);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::getCompressionRate
// Description: Return a compression rate if the texture is compressed (or one of its mip level is compressed):
// Arguments: float& compressRate - the rate, with one digit after the point
// Return Val: bool  - Success / failure - if the texture has no compressed mip levels - failure
// Author:  AMD Developer Tools Team
// Date:        17/11/2008
// ---------------------------------------------------------------------------
bool apGLTexture::getCompressionRate(float& compressRate) const
{
    bool retVal = false;

    // Initialize first and last texture faces (for iterating the faces in case of cube map):
    apGLTextureMipLevel::apTextureFaceIndex firstFaceIndex = apGLTextureMipLevel::AP_SINGLE_TEXTURE_FACE_INDEX;
    apGLTextureMipLevel::apTextureFaceIndex lastFaceIndex = apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_NEGATIVE_X_FACE_INDEX;

    if (textureType() == AP_CUBE_MAP_TEXTURE)
    {
        firstFaceIndex = apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_POSITIVE_X_FACE_INDEX;
        lastFaceIndex = apGLTextureMipLevel::AP_MAX_AMOUNT_OF_TEXTURE_FACES;
    }

    // Initialize compression components to false:
    bool doesTextureHaveCompressedComponents = false;

    // Check if one the texture levels is compressed:
    int amountOfMipLevels = (int)_textureMipLevels.size();

    for (int i = 0; i < amountOfMipLevels; i++)
    {
        apGLTextureMipLevel* pTextureLevel = _textureMipLevels[i];

        if (pTextureLevel != NULL)
        {
            for (int faceIndex = (int)firstFaceIndex; faceIndex < (int)lastFaceIndex; faceIndex++)
            {
                // Check if this face is compressed:
                bool isFaceCompressed = false;
                apGLTextureMipLevel::apTextureFaceIndex textureFaceIndex = (apGLTextureMipLevel::apTextureFaceIndex)faceIndex;
                pTextureLevel->isTextureMipLevelCompressed(isFaceCompressed, textureFaceIndex);
                doesTextureHaveCompressedComponents = doesTextureHaveCompressedComponents || isFaceCompressed;
            }
        }
    }

    // If the texture is compressed, calculate the estimated texture size:
    if (doesTextureHaveCompressedComponents)
    {
        // Get the memory size:
        gtSize_t memorySize = 0;
        gtSize_t estimatedMemorySize = 0;
        bool isSizeEstimated = false;
        bool rc = getMemorySize(memorySize, isSizeEstimated);

        // If the size is not estimated, calculate the estimated size:
        if (!isSizeEstimated && rc)
        {
            retVal = true;

            // Calculate and summarize the compressed texture mip levels estimated size:
            amountOfMipLevels = (int)_textureMipLevels.size();

            for (int i = 0; i < amountOfMipLevels; i++)
            {
                apGLTextureMipLevel* pTextureLevel = _textureMipLevels[i];

                if (pTextureLevel != NULL)
                {
                    for (int faceIndex = firstFaceIndex; faceIndex < (int)lastFaceIndex; faceIndex++)
                    {
                        // Check if this face is compressed:
                        bool isFaceCompressed = false;
                        gtSize_t faceMemorySize;
                        apGLTextureMipLevel::apTextureFaceIndex textureFaceIndex = (apGLTextureMipLevel::apTextureFaceIndex)faceIndex;
                        pTextureLevel->isTextureMipLevelCompressed(isFaceCompressed, textureFaceIndex);

                        if (isFaceCompressed)
                        {
                            // For compressed face, get the estimated size:
                            rc = pTextureLevel->getEstimatedMemorySize(faceMemorySize, textureFaceIndex);
                            retVal = retVal && rc;
                        }
                        else
                        {
                            // Get the size for non compressed faces:
                            pTextureLevel->getMemorySize(faceMemorySize, isSizeEstimated, textureFaceIndex);
                        }

                        estimatedMemorySize += faceMemorySize;
                    }
                }
            }
        }

        // Calculate compression rate:
        compressRate = (float)estimatedMemorySize / (float)memorySize ;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        apGLTexture::getMipmapBaseMaxLevels
// Description: Get the user base and max levels from texture parameters
// Arguments: GLuint& baseLevel
//            GLuint& maxLevel
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        15/1/2009
// ---------------------------------------------------------------------------
bool apGLTexture::getMipmapBaseMaxLevels(GLuint& baseLevel, GLuint& maxLevel)const
{
    bool retVal = _isOpenGLESTexture;

    if (!retVal)
    {
        // Get the base and max texture levels:
        GLint maxLevelFromParams = 0;
        GLint baseLevelFromParams = 0;
        bool rc1 = _textureParameters.getTextureIntParameterValue(GL_TEXTURE_BASE_LEVEL, baseLevelFromParams);
        bool rc2 = _textureParameters.getTextureIntParameterValue(GL_TEXTURE_MAX_LEVEL, maxLevelFromParams);
        retVal = rc1 && rc2;

        if (retVal)
        {
            maxLevel = (GLuint)maxLevelFromParams;
            baseLevel = (GLuint)baseLevelFromParams;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::markTextureAsDirty
// Description: Mark one of the mip levels (or all of them) with dirty flags
// Arguments: GLenum bindTarget
//            int level
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        15/1/2009
// ---------------------------------------------------------------------------
void apGLTexture::markTextureAsDirty(GLenum bindTarget, int level)
{
    // Check if the image mip levels are auto generated:
    apTextureMipMapType mipmapType = getTextureMipmapType();

    // For manual mipmap, mark only the relevant level:
    if ((mipmapType == AP_MIPMAP_NONE_MANUAL) || (mipmapType == AP_MIPMAP_NONE))
    {
        apGLTextureMipLevel* pTextureLevel = getTextureMipLevel(level);

        // If the texture was generated in "force stub textures" mode, then it will have no levels:
        if (pTextureLevel != NULL)
        {
            pTextureLevel->markTextureAsDirty(bindTarget);
        }
    }
    // Mark all levels as dirty for auto generated levels:
    else if (mipmapType == AP_MIPMAP_AUTO_GENERATE)
    {
        markTextureAsDirty(bindTarget);
    }
}


// ---------------------------------------------------------------------------
// Name:        apGLTexture::markTextureAsDirty
// Description: Mark all the mip levels with dirty flags
// Arguments: GLenum bindTarget
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        15/1/2009
// ---------------------------------------------------------------------------
void apGLTexture::markTextureAsDirty(GLenum bindTarget)
{
    // Mark all the mip levels as dirty:
    int amountOfMipLevels = (int)_textureMipLevels.size();

    for (int i = 0; i < amountOfMipLevels; i++)
    {
        apGLTextureMipLevel* pTextureLevel = _textureMipLevels[i];

        if (pTextureLevel != NULL)
        {
            pTextureLevel->markTextureAsDirty(bindTarget);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::setBufferInternalFormat
// Description: Set texture buffer internal format
// Arguments: GLenum internalformat
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        3/8/2009
// ---------------------------------------------------------------------------
void apGLTexture::setBufferInternalFormat(GLenum internalformat)
{
    _bufferInternalFormat = internalformat;
}



// ---------------------------------------------------------------------------
// Name:        apGLTexture::textureBufferFormatToDataType
// Description: Transltate a texture buffer data format to data type (according
//              to texture buffer extension spec)
// Arguments: GLenum openglDataFormat
//            oaDataType& dataType
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        5/8/2009
// ---------------------------------------------------------------------------
bool apGLTexture::textureBufferFormatToDataType(GLenum openglDataFormat, oaDataType& dataType)
{
    bool retVal = true;

    switch (openglDataFormat)
    {
        case GL_ALPHA8:
        case GL_ALPHA8UI_EXT:
        case GL_LUMINANCE8:
        case GL_LUMINANCE8UI_EXT:
        case GL_LUMINANCE8_ALPHA8:
        case GL_LUMINANCE_ALPHA8UI_EXT:
        case GL_INTENSITY8:
        case GL_INTENSITY8UI_EXT:
        case GL_RGBA8:
        case GL_RGBA8UI_EXT:
        {
            dataType = OA_UNSIGNED_BYTE;
            break;
        }


        case GL_ALPHA16:
        case GL_ALPHA16UI_EXT:
        case GL_LUMINANCE16:
        case GL_LUMINANCE16UI_EXT:
        case GL_LUMINANCE16_ALPHA16:
        case GL_LUMINANCE_ALPHA16UI_EXT:
        case GL_INTENSITY16:
        case GL_INTENSITY16UI_EXT:
        case GL_RGBA16:
        case GL_RGBA16UI_EXT:
        {
            dataType = OA_UNSIGNED_SHORT;
            break;
        }

        case GL_ALPHA16F_ARB:
        case GL_LUMINANCE16F_ARB:
        case GL_LUMINANCE_ALPHA16F_ARB:
        case GL_INTENSITY16F_ARB:
        case GL_RGBA16F_ARB:
        {
            // TO_DO: OpenGL3.1 support half data type format
            GT_ASSERT_EX(false, L"Unsupported texture buffer data type: half");
            retVal = false;
            break;
        }

        case GL_ALPHA32F_ARB:
        case GL_LUMINANCE32F_ARB:
        case GL_LUMINANCE_ALPHA32F_ARB:
        case GL_INTENSITY32F_ARB:
        case GL_RGBA32F_ARB:
        {
            dataType = OA_FLOAT;
            break;
        }

        case GL_ALPHA8I_EXT:
        case GL_LUMINANCE8I_EXT:
        case GL_LUMINANCE_ALPHA8I_EXT:
        case GL_INTENSITY8I_EXT:
        case GL_RGBA8I_EXT:
        {
            dataType = OA_BYTE;
            break;
        }

        case GL_ALPHA16I_EXT:
        case GL_LUMINANCE16I_EXT:
        case GL_LUMINANCE_ALPHA16I_EXT:
        case GL_INTENSITY16I_EXT:
        case GL_RGBA16I_EXT:
        {
            dataType = OA_SHORT;
            break;
        }

        case GL_ALPHA32I_EXT:
        case GL_LUMINANCE32I_EXT:
        case GL_LUMINANCE_ALPHA32I_EXT:
        case GL_INTENSITY32I_EXT:
        case GL_RGBA32I_EXT:
        {
            dataType = OA_INT;
            break;
        }


        case GL_ALPHA32UI_EXT:
        case GL_LUMINANCE32UI_EXT:
        case GL_LUMINANCE_ALPHA32UI_EXT:
        case GL_INTENSITY32UI_EXT:
        case GL_RGBA32UI_EXT:
        {
            dataType = OA_UNSIGNED_INT;
            break;
        }

        default:
        {
            dataType = OA_UNKNOWN_DATA_TYPE;
            GT_ASSERT_EX(false, L"Unsupported texture buffer data type");
            retVal = false;
        }

    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        apGLTexture::markAllTextureImagesAsUpdated
// Description:
// Arguments: int level
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        30/8/2009
// ---------------------------------------------------------------------------
void apGLTexture::markAllTextureImagesAsUpdated(int level)
{
    apGLTextureMipLevel* pTexMiplevel = getTextureMipLevel(level);

    if (pTexMiplevel != NULL)
    {
        pTexMiplevel->markAllTextureImagesAsUpdated();
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::dirtyTextureImageExists
// Description:
// Arguments: int level
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        30/8/2009
// ---------------------------------------------------------------------------
bool apGLTexture::dirtyTextureImageExists(int level) const
{
    bool retVal = false;
    const apGLTextureMipLevel* pTexMiplevel = getTextureMipLevel(level);

    if (pTexMiplevel != NULL)
    {
        retVal = pTexMiplevel->dirtyTextureImageExists();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTexture::markAllTextureRawDataAsUpdated
// Description:
// Arguments: int level
// Return Val: void
// Author:  AMD Developer Tools Team
// Date:        30/8/2009
// ---------------------------------------------------------------------------
void apGLTexture::markAllTextureRawDataAsUpdated(int level)
{
    apGLTextureMipLevel* pTexMiplevel = getTextureMipLevel(level);

    if (pTexMiplevel != NULL)
    {
        pTexMiplevel->markAllTextureRawDataAsUpdated();
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::dirtyTextureRawDataExists
// Description:
// Arguments: int level
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        30/8/2009
// ---------------------------------------------------------------------------
bool apGLTexture::dirtyTextureRawDataExists(int level) const
{
    bool retVal = false;
    const apGLTextureMipLevel* pTexMiplevel = getTextureMipLevel(level);

    if (pTexMiplevel != NULL)
    {
        retVal = pTexMiplevel->dirtyTextureRawDataExists();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTexture::shareTextureMiplevelWithCLImage
// Description: OpenCL interoperability: Share a texture miplevel with an OpenCL image
// Arguments:   int clImageIndex - the OpenCL image id
//              int clSpyID - the OpenCL spy ID
//              int textureMipLevel - the shared
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        22/7/2010
// ---------------------------------------------------------------------------
bool apGLTexture::shareTextureMiplevelWithCLImage(int clImageIndex, int clImageName, int clSpyID, int textureMipLevel)
{
    bool retVal = false;
    apGLTextureMipLevel* pTextureMiplevel = getTextureMipLevel(textureMipLevel);
    GT_IF_WITH_ASSERT(pTextureMiplevel != NULL)
    {
        // Set the OpenCL image:
        pTextureMiplevel->setSharedCLImage(clImageIndex, clImageName, clSpyID);
        retVal = true;
    }

    // If this is the miplevel 0, set it as the texture sharing:
    if (textureMipLevel == 0)
    {
        _openCLImageIndex = clImageIndex;
        _openCLImageName = clImageName;
        _openCLSpyID = clSpyID;
    }

    return retVal;
}

