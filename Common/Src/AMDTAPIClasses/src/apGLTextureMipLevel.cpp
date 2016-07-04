//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apGLTextureMipLevel.cpp
///
//==================================================================================

// -----------------------------   apGLTextureMipLevel.cpp ------------------------------

//

#include <math.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSAPIWrappers/Include/oaOpenGLIncludes.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTAPIClasses/Include/apStringConstants.h>
#include <AMDTAPIClasses/Include/apParameters.h>
#include <AMDTAPIClasses/Include/apInternalFormat.h>
#include <AMDTAPIClasses/Include/ap2DRectangle.h>
#include <AMDTAPIClasses/Include/apGLTextureMipLevel.h>



// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::apGLTextureMipLevel
// Description: Constructor
// Arguments:   textureName - The OpenGL texture name.
// Author:  AMD Developer Tools Team
// Date:        19/12/2004
// ---------------------------------------------------------------------------
apGLTextureMipLevel::apGLTextureMipLevel()
    : apAllocatedObject(), _requestedWidth(0), _requestedHeight(0), _requestedDepth(0), _requestedBorderWidth(0), _pixelFormat(0), _texelsType(0), _requestedInternalPixelFormat(0),
      _openCLImageIndex(-1), _openCLImageName(-1), _openCLSpyID(-1)
{
}



// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::gsGLTexture
// Description: Copy constructor
// Arguments: other - The other texture class from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        2/7/2006
// ---------------------------------------------------------------------------
apGLTextureMipLevel::apGLTextureMipLevel(const apGLTextureMipLevel& other)
{
    apGLTextureMipLevel::operator=(other);

    // Copy self data:
    _dirtyRawData = other.getDirtyTextureData();

}


// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::~apGLTextureMipLevel
// Description: Destructor
// Author:  AMD Developer Tools Team
// Date:        25/12/2004
// ---------------------------------------------------------------------------
apGLTextureMipLevel::~apGLTextureMipLevel()
{
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::operator=
// Description: Assignment operator
// Arguments:   other - The other object from which I am copied.
// Author:  AMD Developer Tools Team
// Date:        30/12/2004
// ---------------------------------------------------------------------------
apGLTextureMipLevel& apGLTextureMipLevel::operator=(const apGLTextureMipLevel& other)
{
    _requestedWidth = other._requestedWidth;
    _requestedHeight = other._requestedHeight;
    _requestedDepth = other._requestedDepth;
    _requestedBorderWidth = other._requestedBorderWidth;
    _pixelFormat = other._pixelFormat;
    _texelsType = other._texelsType;
    _requestedInternalPixelFormat = other._requestedInternalPixelFormat;

    setAllocatedObjectId(other.getAllocatedObjectId(), true);

    for (int i = 0; i < AP_MAX_AMOUNT_OF_TEXTURE_FACES; i++)
    {
        _textureLevelParameters[i] = other._textureLevelParameters[i];
    }


    // Copy the texture data files paths:
    for (int i = 0; i < AP_MAX_AMOUNT_OF_TEXTURE_FACES; i++)
    {
        _textureDataFilesPaths[i] = other._textureDataFilesPaths[i];
    }

    // Copy texture images dirty vector
    _dirtyTextureImages = other.dirtyTextureImages();

    // Copy CL interoperability:
    _openCLImageIndex = other._openCLImageIndex;
    _openCLImageName = other._openCLImageName;
    _openCLSpyID = other._openCLSpyID;

    return *this;
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::getTextureDataFilePath
// Description: Returns the texture mip level file path
// Arguments: int faceIndex = AP_SINGLE_TEXTURE_FACE_INDEX
// Return Val: const osFilePath* - NULL for non exist texture file path
// Author:  AMD Developer Tools Team
// Date:        6/10/2008
// ---------------------------------------------------------------------------
const osFilePath* apGLTextureMipLevel::getTextureDataFilePath(int faceIndex) const
{
    const osFilePath* retVal = NULL;

    if ((faceIndex >= 0) && (faceIndex < AP_MAX_AMOUNT_OF_TEXTURE_FACES))
    {
        retVal = &_textureDataFilesPaths[faceIndex];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::addDefaultGLTextureLevelsParameters
// Description: Adds the OpenGL default texture level parameters.
// Author:  AMD Developer Tools Team
// Date:        28/10/2008
// ---------------------------------------------------------------------------
void apGLTextureMipLevel::addDefaultGLTextureLevelsParameters(apTextureType textureType)
{
    // Initialize the number of texture panes according to texture type:
    int numOfPanes = 1;

    if (textureType == AP_CUBE_MAP_TEXTURE)
    {
        numOfPanes = AP_MAX_AMOUNT_OF_TEXTURE_FACES;
    }

    // Add default parameters for each pane:
    // NOTICE: This function can be called several times, since it is called both for texture generation,
    // and for texture image load.
    // If default parameters already exist, we do nothing
    for (int i = 0; i < numOfPanes; i++)
    {
        int amountOfParameters = _textureLevelParameters[i].amountOfTextureParameters();

        // If the face parameters is empty, add it:
        if (amountOfParameters == 0)
        {
            _textureLevelParameters[i].addDefaultGLTextureLevelParameters();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::setTextureFormats
// Description: Sets the texture data formats.
// Arguments: bindTarget - the texture bind target.
//            internalPixelFormat - The number of color components in the texture.
//            pixelFormat - The format of the pixel data.
//            textureType - The type of the texture.
// Author:  AMD Developer Tools Team
// Date:        3/7/2006
// ---------------------------------------------------------------------------
void apGLTextureMipLevel::setTextureFormats(GLint internalPixelFormat, GLenum pixelFormat, GLenum texelsType)
{
    _requestedInternalPixelFormat = internalPixelFormat;
    _pixelFormat = pixelFormat;
    _texelsType = texelsType;
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::updateTextureDataFile
// Description: Updates the texture data file path.
// Arguments:   bindTarget - the texture bind target.
//              textureDataFilePath - The path of a file that contains the texture data.
// Author:  AMD Developer Tools Team
// Date:        20/1/2005
// ---------------------------------------------------------------------------
void apGLTextureMipLevel::updateTextureDataFile(GLenum bindTarget, const osFilePath& textureDataFilePath)
{
    int dataFileIndex = bindTargetToTextureFaceIndex(bindTarget);
    _textureDataFilesPaths[dataFileIndex] = textureDataFilePath;
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        19/12/2004
// ---------------------------------------------------------------------------
osTransferableObjectType apGLTextureMipLevel::type() const
{
    return OS_TOBJ_ID_GL_TEXTURE;
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::writeSelfIntoChannel
// Description: Writes this class into a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        19/12/2004
// ---------------------------------------------------------------------------
bool apGLTextureMipLevel::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    // Write the texture mip level attributes into the channel:
    ipcChannel << (gtInt32)_texelsType;
    ipcChannel << (gtUInt64)_requestedWidth;
    ipcChannel << (gtUInt64)_requestedHeight;
    ipcChannel << (gtUInt64)_requestedDepth;
    ipcChannel << (gtInt32)_requestedBorderWidth;
    ipcChannel << (gtInt32)_pixelFormat;
    ipcChannel << (gtInt32)_requestedInternalPixelFormat;

    apAllocatedObject::writeSelfIntoChannel(ipcChannel);

    // Write the texture file paths:
    for (int i = 0; i < AP_MAX_AMOUNT_OF_TEXTURE_FACES; i++)
    {
        bool rc = _textureDataFilesPaths[i].writeSelfIntoChannel(ipcChannel);
        retVal = retVal && rc;
    }

    for (int i = 0; i < AP_MAX_AMOUNT_OF_TEXTURE_FACES; i++)
    {
        _textureLevelParameters[i].writeSelfIntoChannel(ipcChannel);
    }

    // Copy CL interoperability:
    ipcChannel << (gtInt32)_openCLImageIndex;
    ipcChannel << (gtInt32)_openCLImageName;
    ipcChannel << (gtInt32)_openCLSpyID;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::readSelfFromChannel
// Description: Read self from a channel.
// Return Val:  bool - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        20/12/2004
// ---------------------------------------------------------------------------
bool apGLTextureMipLevel::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = true;

    // Read the texture mip level attributes into the channel:
    gtInt32 paramAsInt32 = 0;
    ipcChannel >> paramAsInt32;
    _texelsType = (GLenum)paramAsInt32;

    gtUInt64 paramAsUint64 = 0;
    ipcChannel >> paramAsUint64;
    _requestedWidth = (GLsizei)paramAsUint64;

    ipcChannel >> paramAsUint64;
    _requestedHeight = (GLsizei)paramAsUint64;

    ipcChannel >> paramAsUint64;
    _requestedDepth = (GLsizei)paramAsUint64;

    ipcChannel >> paramAsInt32;
    _requestedBorderWidth = (GLint)paramAsInt32;

    ipcChannel >> paramAsInt32;
    _pixelFormat = (GLenum)paramAsInt32;

    ipcChannel >> paramAsInt32;
    _requestedInternalPixelFormat = (GLint)paramAsInt32;

    apAllocatedObject::readSelfFromChannel(ipcChannel);

    // Read the texture file name path:
    for (int i = 0; i < AP_MAX_AMOUNT_OF_TEXTURE_FACES; i++)
    {
        bool rc = _textureDataFilesPaths[i].readSelfFromChannel(ipcChannel);
        retVal = retVal && rc;
    }

    // Read the texture file name path:
    for (int i = 0; i < AP_MAX_AMOUNT_OF_TEXTURE_FACES; i++)
    {
        bool rc = _textureLevelParameters[i].readSelfFromChannel(ipcChannel);
        retVal = retVal && rc;
    }

    ipcChannel >> paramAsInt32;
    _openCLImageIndex = paramAsInt32;

    ipcChannel >> paramAsInt32;
    _openCLImageName = paramAsInt32;

    ipcChannel >> paramAsInt32;
    _openCLSpyID = paramAsInt32;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::bindTargetToTextureFaceIndex
// Description: Maps an OpenGL bind target to a texture data file index
//              (in the _textureDataFilesPaths array)
// Author:  AMD Developer Tools Team
// Date:        3/1/2005
// ---------------------------------------------------------------------------
int apGLTextureMipLevel::bindTargetToTextureFaceIndex(GLenum bindTarget) const
{
    int retVal = 0;

    switch (bindTarget)
    {
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            retVal = AP_TEXTURE_CUBE_MAP_POSITIVE_X_FACE_INDEX;
            break;

        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            retVal = AP_TEXTURE_CUBE_MAP_NEGATIVE_X_FACE_INDEX;
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            retVal = AP_TEXTURE_CUBE_MAP_POSITIVE_Y_PANE_INDEX;
            break;

        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            retVal = AP_TEXTURE_CUBE_MAP_NEGATIVE_Y_PANE_INDEX;
            break;

        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            retVal = AP_TEXTURE_CUBE_MAP_POSITIVE_Z_PANE_INDEX;
            break;

        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            retVal = AP_TEXTURE_CUBE_MAP_NEGATIVE_Z_FACE_INDEX;
            break;

        default:
            retVal = AP_SINGLE_TEXTURE_FACE_INDEX;
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::texturefaceIndexToBindTarget
// Description: Convert a bindTarget (the texture bind target) to a texture pane bind target
//              according to texture pane index
// Arguments: apTextureFaceIndex faceIndex
//            apTextureFaceIndex faceIndex
//            GLenum originalBindTarget
// Return Val: GLenum
// Author:  AMD Developer Tools Team
// Date:        5/11/2008
// ---------------------------------------------------------------------------
GLenum apGLTextureMipLevel::textureFaceIndexToBindTarget(apTextureFaceIndex faceIndex, GLenum originalBindTarget)
{
    // For texture with single pane index, keep the original bind target:
    GLenum retVal = originalBindTarget;

    // For cube map, map apTextureFaceIndex to OpenGL bind target:
    if (originalBindTarget == GL_TEXTURE_CUBE_MAP)
    {
        switch (faceIndex)
        {
            case AP_TEXTURE_CUBE_MAP_POSITIVE_X_FACE_INDEX:
                retVal = GL_TEXTURE_CUBE_MAP_POSITIVE_X;
                break;

            case AP_TEXTURE_CUBE_MAP_NEGATIVE_X_FACE_INDEX:
                retVal = GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
                break;

            case AP_TEXTURE_CUBE_MAP_POSITIVE_Y_PANE_INDEX:
                retVal = GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
                break;

            case AP_TEXTURE_CUBE_MAP_NEGATIVE_Y_PANE_INDEX:
                retVal = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
                break;

            case AP_TEXTURE_CUBE_MAP_POSITIVE_Z_PANE_INDEX:
                retVal = GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
                break;

            case AP_TEXTURE_CUBE_MAP_NEGATIVE_Z_FACE_INDEX:
                retVal = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
                break;

            default:
            {
                retVal = 0;
                GT_ASSERT_EX(false, L"Unsupported texture pane");
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::getUsedInternalPixelFormat
// Description: Return internal format (used)
// Arguments: GLuint& internalFormat
//             apTextureFaceIndex faceIndex - texture pane index
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/10/2008
// ---------------------------------------------------------------------------
bool apGLTextureMipLevel::getUsedInternalPixelFormat(GLuint& internalFormat, apTextureFaceIndex faceIndex) const
{
    bool retVal = false;

    // Get the internal format parameter:
    GLenum paramValue = 0;
    retVal = _textureLevelParameters[faceIndex].getTextureEnumParameterValue(GL_TEXTURE_INTERNAL_FORMAT, internalFormat);

    if (retVal)
    {
        internalFormat = (GLuint)paramValue;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::isTextureMipLevelCompressed
// Description: Check if the texture mip level internal format is a compressed one
// Arguments: bool& isCompressed
//             apTextureFaceIndex faceIndex - texture pane index
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        29/10/2008
// ---------------------------------------------------------------------------
bool apGLTextureMipLevel::isTextureMipLevelCompressed(bool& isCompressed, apTextureFaceIndex faceIndex) const
{
    bool retVal = false;

    // Check if the texture is compressed:
    GLboolean glIsCompressed = GL_FALSE;
    retVal = _textureLevelParameters[faceIndex].getTextureBoolenParameterValue(GL_TEXTURE_COMPRESSED, glIsCompressed);

    if (retVal)
    {
        if (glIsCompressed == GL_TRUE)
        {
            isCompressed = true;
        }
        else
        {
            isCompressed = false;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::getMemorySize
// Description: Memory size according to texture type
// Arguments: gtSize_t& memorySize
//            bool& isMemoryEstimated
//            apTextureType textureType (needed for face number)
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        5/11/2008
// ---------------------------------------------------------------------------
bool apGLTextureMipLevel::getMemorySize(gtSize_t& memorySize, bool& isMemoryEstimated, apTextureType textureType) const
{
    bool retVal = true;

    // Initialize memory estimation:
    isMemoryEstimated = false;

    // Check number of texture faces:
    apGLTextureMipLevel::apTextureFaceIndex firstFaceIndex = apGLTextureMipLevel::AP_SINGLE_TEXTURE_FACE_INDEX;
    apGLTextureMipLevel::apTextureFaceIndex lastFaceIndex = apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_NEGATIVE_X_FACE_INDEX;

    if (textureType == AP_CUBE_MAP_TEXTURE)
    {
        firstFaceIndex = apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_POSITIVE_X_FACE_INDEX;
        lastFaceIndex = apGLTextureMipLevel::AP_MAX_AMOUNT_OF_TEXTURE_FACES;
    }

    // Initialize memory size:
    memorySize = 0;

    // Summarize memory size for each texture face:
    for (int i = firstFaceIndex; i < lastFaceIndex; i++)
    {
        gtSize_t faceMemorySize = 0;
        bool faceMemoryEstimated = true;
        apGLTextureMipLevel::apTextureFaceIndex textureFace = (apGLTextureMipLevel::apTextureFaceIndex)i;
        bool rc1 = getMemorySize(faceMemorySize, faceMemoryEstimated, textureFace);

        if (rc1)
        {
            memorySize += faceMemorySize;
            isMemoryEstimated = isMemoryEstimated || faceMemoryEstimated;
        }

        retVal = retVal && rc1;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::getMemorySize
// Description: Calculate the texture mipmap level according to the internal pixel format (bits)
// Arguments: gtSize_t& memorySize
//             apTextureFaceIndex faceIndex - texture pane index
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        17/11/2008
// ---------------------------------------------------------------------------
bool apGLTextureMipLevel::getEstimatedMemorySize(gtSize_t& memorySize, apTextureFaceIndex faceIndex) const
{
    bool retVal = false;
    memorySize = 0;

    // Get the pixel size:
    GLuint totalPixelSizeInBitsAsUint = 0;
    bool isEstimated = false;
    bool rc = getPixelSize(totalPixelSizeInBitsAsUint, isEstimated, faceIndex);

    GT_IF_WITH_ASSERT(rc)
    {
        // Calculate the texture number of pixels:
        float numberOfPixels = 1;
        numberOfPixels *= _requestedWidth;

        if (_requestedHeight > 0)
        {
            numberOfPixels *= _requestedHeight;
        }

        if (_requestedDepth > 0)
        {
            numberOfPixels *= _requestedDepth;
        }

        // Memory size is actually number of pixel * pixel size in bits:
        memorySize = (GLuint)numberOfPixels * totalPixelSizeInBitsAsUint;
        retVal = true;
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::getPixelSize
// Description: Calculate total pixel size for texture mip-level. The texture pixel size
//              is summarized for each texture level face.
// Arguments: GLuint& pixelSize
//            bool& isEstimated
//            apTextureType textureType
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        24/11/2008
// ---------------------------------------------------------------------------
bool apGLTextureMipLevel::getPixelSize(GLuint& pixelSize, bool& isEstimated, apTextureType textureType) const
{
    bool retVal = true;
    pixelSize = 0;

    // Check number of texture panes:
    apGLTextureMipLevel::apTextureFaceIndex firstFaceIndex = apGLTextureMipLevel::AP_SINGLE_TEXTURE_FACE_INDEX;
    apGLTextureMipLevel::apTextureFaceIndex lastFaceIndex = apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_NEGATIVE_X_FACE_INDEX;

    if (textureType == AP_CUBE_MAP_TEXTURE)
    {
        firstFaceIndex = apGLTextureMipLevel::AP_TEXTURE_CUBE_MAP_POSITIVE_X_FACE_INDEX;
        lastFaceIndex = apGLTextureMipLevel::AP_MAX_AMOUNT_OF_TEXTURE_FACES;
    }

    // Summarize pixel size for each texture face:
    for (int textureFace = firstFaceIndex; textureFace < lastFaceIndex; textureFace++)
    {
        // Convert int to texture face index:
        apGLTextureMipLevel::apTextureFaceIndex faceIndex = (apGLTextureMipLevel::apTextureFaceIndex)textureFace;
        // Get the texture face pixel size:
        GLuint currentFacePixelSize = 0;
        bool rc = getPixelSize(currentFacePixelSize, isEstimated, faceIndex);
        GT_IF_WITH_ASSERT(rc)
        {
            pixelSize += currentFacePixelSize;
        }
        retVal = retVal && rc;
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::getPixelSize
// Description: Return the texture mip-level pixel size
// Arguments: GLuint& pixelSize
//            bool& isEstimated - is estimated (based on requested pixel format or extracted from texture parameters)
//            apTextureFaceIndex faceIndex
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        24/11/2008
// ---------------------------------------------------------------------------
bool apGLTextureMipLevel::getPixelSize(GLuint& pixelSize, bool& isEstimated, apTextureFaceIndex faceIndex) const
{
    bool retVal = false;
    // Try to get texture channels size parameters values:
    float pixelSizeInBits = 0;
    float totalPixelSizeInBits = 0;
    bool rc1 = _textureLevelParameters[faceIndex].getTextureFloatParameterValue(GL_TEXTURE_RED_SIZE, pixelSizeInBits);
    totalPixelSizeInBits += pixelSizeInBits;
    bool rc2 = _textureLevelParameters[faceIndex].getTextureFloatParameterValue(GL_TEXTURE_GREEN_SIZE, pixelSizeInBits);
    totalPixelSizeInBits += pixelSizeInBits;
    bool rc3 = _textureLevelParameters[faceIndex].getTextureFloatParameterValue(GL_TEXTURE_BLUE_SIZE, pixelSizeInBits);
    totalPixelSizeInBits += pixelSizeInBits;
    bool rc4 = _textureLevelParameters[faceIndex].getTextureFloatParameterValue(GL_TEXTURE_ALPHA_SIZE, pixelSizeInBits);
    totalPixelSizeInBits += pixelSizeInBits;
    bool rc5 = _textureLevelParameters[faceIndex].getTextureFloatParameterValue(GL_TEXTURE_LUMINANCE_SIZE, pixelSizeInBits);
    totalPixelSizeInBits += pixelSizeInBits;
    bool rc6 = _textureLevelParameters[faceIndex].getTextureFloatParameterValue(GL_TEXTURE_INTENSITY_SIZE, pixelSizeInBits);
    totalPixelSizeInBits += pixelSizeInBits;
    bool rc7 = _textureLevelParameters[faceIndex].getTextureFloatParameterValue(GL_TEXTURE_DEPTH_SIZE, pixelSizeInBits);
    totalPixelSizeInBits += pixelSizeInBits;

    // The following texture size parameters are not always supported, so we do not fail the function if one of it
    // cannot be updated:

    // Get GL_TEXTURE_SHARED_SIZE:
    bool rc8 = _textureLevelParameters[faceIndex].getTextureFloatParameterValue(GL_TEXTURE_SHARED_SIZE, pixelSizeInBits);

    if (rc8)
    {
        totalPixelSizeInBits += pixelSizeInBits;
    }

    // Get GL_TEXTURE_STENCIL_SIZE:
    rc8 = _textureLevelParameters[faceIndex].getTextureFloatParameterValue(GL_TEXTURE_STENCIL_SIZE, pixelSizeInBits);

    if (rc8)
    {
        totalPixelSizeInBits += pixelSizeInBits;
    }

    // Get GL_TEXTURE_HI_SIZE_NV:
    rc8 = _textureLevelParameters[faceIndex].getTextureFloatParameterValue(GL_TEXTURE_HI_SIZE_NV, pixelSizeInBits);

    if (rc8)
    {
        totalPixelSizeInBits += pixelSizeInBits;
    }

    // Get GL_TEXTURE_LO_SIZE_NV:
    rc8 = _textureLevelParameters[faceIndex].getTextureFloatParameterValue(GL_TEXTURE_LO_SIZE_NV, pixelSizeInBits);

    if (rc8)
    {
        totalPixelSizeInBits += pixelSizeInBits;
    }

    // Get GL_TEXTURE_DS_SIZE_NV:
    rc8 = _textureLevelParameters[faceIndex].getTextureFloatParameterValue(GL_TEXTURE_DS_SIZE_NV, pixelSizeInBits);

    if (rc8)
    {
        totalPixelSizeInBits += pixelSizeInBits;
    }

    // Get GL_TEXTURE_DT_SIZE_NV:
    rc8 = _textureLevelParameters[faceIndex].getTextureFloatParameterValue(GL_TEXTURE_DT_SIZE_NV, pixelSizeInBits);

    if (rc8)
    {
        totalPixelSizeInBits += pixelSizeInBits;
    }

    // Get GL_TEXTURE_MAG_SIZE_NV:
    rc8 = _textureLevelParameters[faceIndex].getTextureFloatParameterValue(GL_TEXTURE_MAG_SIZE_NV, pixelSizeInBits);

    if (rc8)
    {
        totalPixelSizeInBits += pixelSizeInBits;
    }

    retVal = rc1 && rc2 && rc3 && rc4 && rc5 && rc6 && rc7;

    pixelSize = (GLuint)ceil(totalPixelSizeInBits);

    if (!retVal)
    {
        // Estimate pixel size from internal format:
        retVal = apGetPixelSizeInBitsByInternalFormat(_requestedInternalPixelFormat, pixelSize);
        GT_ASSERT(retVal);
        isEstimated = true;
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::getMemorySize
// Description: Calculate the texture mipmap level according to the internal pixel
//              format (bits)
// Arguments: gtSize_t& memorySize
//              bool& isMemoryEstimated - when texture level parameters are not available
//              for some reason, we estimate the size
//             apTextureFaceIndex faceIndex - texture pane index
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        6/10/2008
// ---------------------------------------------------------------------------
bool apGLTextureMipLevel::getMemorySize(gtSize_t& memorySize, bool& isMemoryEstimated, apTextureFaceIndex faceIndex) const
{
    bool retVal = true;
    memorySize = 0;

    // Check if the texture is compressed:
    bool isCompressed = false;
    isMemoryEstimated = false;
    retVal = isTextureMipLevelCompressed(isCompressed, faceIndex);

    if (!retVal)
    {
        // When texture parameters are not updated, we assume that the texture is not compressed,
        // and we don't want to fail the function:
        isCompressed = false;
        isMemoryEstimated = true;
        retVal = true;

        OS_OUTPUT_DEBUG_LOG(AP_STR_couldNotDetermineMipLevelCompressionStatus, OS_DEBUG_LOG_DEBUG);
    }

    if (isCompressed)
    {
        // Get texture compression size:
        float memorySizeAsFloat = 0;
        retVal = retVal && _textureLevelParameters[faceIndex].getTextureFloatParameterValue(GL_TEXTURE_COMPRESSED_IMAGE_SIZE, memorySizeAsFloat);
        memorySize = (gtSize_t)memorySizeAsFloat;

        // Compressed image size is given in bytes, and this function calculated the memory size in bits:
        memorySize *= GT_BITS_PER_BYTE;
    }
    else
    {
        // Get the texture level pixel size:
        GLuint totalPixelSizeInBitsAsUint = 0;
        bool rc = getPixelSize(totalPixelSizeInBitsAsUint, isMemoryEstimated, faceIndex);
        retVal = retVal && rc;
        GT_IF_WITH_ASSERT(retVal)
        {
            // Get the texture mip-level dimensions:
            GLsizei width = 0, height = 0, depth = 0, borderSize = 0;
            bool areDimensionsEstimated = false;
            retVal = retVal && getDimensions(width, height, depth, borderSize, areDimensionsEstimated, faceIndex);
            isMemoryEstimated = isMemoryEstimated || areDimensionsEstimated;

            // Calculate the texture number of pixels:
            float numberOfPixels = 1;
            numberOfPixels *= width;

            if (height > 0)
            {
                numberOfPixels *= height;
            }

            if (depth > 0)
            {
                numberOfPixels *= depth;
            }

            // Memory size is actually number of pixel * pixel size in bits:
            memorySize = (GLuint)numberOfPixels * totalPixelSizeInBitsAsUint;
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::getDimensions
// Description: Get the texture dimension (from parameters or requested)
// Arguments: GLsizei& width
//            GLsizei& height
//            GLsizei& depth
//            GLsizei& borderSize
//            bool& areDimensionsEstimated - the dimension are extracted from level parameters
//            or it is the requested dimensions
//            apTextureFaceIndex faceIndex
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        24/11/2008
// ---------------------------------------------------------------------------
bool apGLTextureMipLevel::getDimensions(GLsizei& width, GLsizei& height, GLsizei& depth, GLsizei& borderSize, bool& areDimensionsEstimated, apTextureFaceIndex faceIndex)const
{
    bool retVal = false;
    areDimensionsEstimated = false;
    float w, h, d, b;
    bool rc1 = _textureLevelParameters[faceIndex].getTextureFloatParameterValue(GL_TEXTURE_WIDTH, w);

    if (rc1)
    {
        width = (GLsizei)w;
    }

    bool rc2 = _textureLevelParameters[faceIndex].getTextureFloatParameterValue(GL_TEXTURE_HEIGHT, h);

    if (rc2)
    {
        height = (GLsizei)h;
    }

    bool rc3 = _textureLevelParameters[faceIndex].getTextureFloatParameterValue(GL_TEXTURE_DEPTH, d);

    if (rc3)
    {
        depth = (GLsizei)d;
    }

    bool rc4 = _textureLevelParameters[faceIndex].getTextureFloatParameterValue(GL_TEXTURE_BORDER, b);

    if (rc4)
    {
        borderSize = (GLsizei)b;
    }
    else
    {
        // Texture border is not supported for OpenGL 3.1 and higher core contexts:
        borderSize = 0;
        rc4 = true;
    }

    if (!rc1 || !rc2 || !rc3 || !rc4)
    {
        // Get estimated dimensions:
        width = _requestedWidth;
        height = _requestedHeight;
        depth = _requestedDepth;
        borderSize = _requestedBorderWidth;
        areDimensionsEstimated = true;
        retVal = true;
    }
    else
    {
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::getDimensions
// Description: Get the texture dimension (from parameters or requested)
// Arguments: GLsizei& width
//            GLsizei& height
//            GLsizei& depth
//            GLsizei& borderSize
//            bool& areDimensionsEstimated - the dimension are extracted from level parameters
//            or it is the requested dimensions
//            apTextureFaceIndex faceIndex
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        24/11/2008
// ---------------------------------------------------------------------------
bool apGLTextureMipLevel::getDimensions(GLsizei& width, GLsizei& height, GLsizei& depth, GLsizei& borderSize, apTextureFaceIndex faceIndex)const
{
    bool retVal = false;
    bool areDimensionsEstimated = false;
    retVal = getDimensions(width, height, depth, borderSize, areDimensionsEstimated, faceIndex);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::markTextureImageAsDirty
// Description: Marks one of this texture object raw data and image preview
//              as dirty.
// Arguments:   bindTarget - A bind target that identifies the dirty image.
// Author:  AMD Developer Tools Team
// Date:        2/7/2006
// ---------------------------------------------------------------------------
void apGLTextureMipLevel::markTextureAsDirty(GLenum bindTarget)
{
    // Check if the input bind target is already marked as dirty:
    bool inputBindTargetAlreadyMarkedAsDirty = false;
    gtVector<GLenum>::const_iterator iter = _dirtyRawData.begin();
    gtVector<GLenum>::const_iterator endIter = _dirtyRawData.end();

    while (iter != endIter)
    {
        if (*iter == bindTarget)
        {
            inputBindTargetAlreadyMarkedAsDirty = true;
            break;
        }

        iter++;
    }

    // If the bind target is not currently marked as dirty:
    if (!inputBindTargetAlreadyMarkedAsDirty)
    {
        // Mark it as dirty in both raw data and images dirty texture vector:
        _dirtyRawData.push_back(bindTarget);
        _dirtyTextureImages.push_back(bindTarget);
    }
}


// ---------------------------------------------------------------------------
// Name:        apGLTextureMipLevel::getUsedInternalPixelFormat
// Description: Return the used internal format of the texture (this data is extracted
//              from the mip level parameters
// Arguments:  apTextureFaceIndex faceIndex - texture face index
// Return Val: GLuint
// Author:  AMD Developer Tools Team
// Date:        2/11/2008
// ---------------------------------------------------------------------------
GLuint apGLTextureMipLevel::getUsedInternalPixelFormat(apTextureFaceIndex faceIndex) const
{
    GLuint retVal = 0;

#ifdef _GR_IPHONE_BUILD
    // Uri, 9/5/2010: On the iPhone (OpenGL ES), we cannot query the used pixel format. However, this information is
    // used for initializing mip levels, so we cannot simply let this be 0. We return the requested pixel format, as
    // it is the format used in OpenGL ES (OpenGL ES does not have the specific size pixel formats (e.g. GL_RGBA16),
    // so it should be the same anyway):
    retVal = _requestedInternalPixelFormat;
#else
    // Get the internal format of the texture object from the texture mipmap level parameters:
    bool rc = _textureLevelParameters[faceIndex].getTextureEnumParameterValue(GL_TEXTURE_INTERNAL_FORMAT, retVal);

    if (!rc)
    {
        retVal = 0;
        OS_OUTPUT_DEBUG_LOG(AP_STR_couldNotUpdateTextureRawData, OS_DEBUG_LOG_DEBUG);
    }

#endif

    return retVal;
}
