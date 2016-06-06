//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file apCLImage.cpp
///
//==================================================================================

//------------------------------ apCLImage.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <AMDTAPIClasses/Include/apCLImage.h>
#include <AMDTAPIClasses/Include/apTextureType.h>


// ---------------------------------------------------------------------------
// Name:        apCLImage::apCLImage
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        1/12/2009
// ---------------------------------------------------------------------------
apCLImage::apCLImage(gtInt32 textureName):
    apCLMemObject(), _imageName(textureName), _imageType(AP_UNKNOWN_TEXTURE_TYPE), _clDataFormat(0), _clDataType(0),
    _contextHandle(OA_CL_NULL_HANDLE), _width(0), _height(0), _depth(0), _buffer(OA_CL_NULL_HANDLE), _imageFilePath(L""), _isDirty(true),
    _openGLRenderBufferName(0), _openGLTextureName(0), _openGLMiplevel(0), _openGLTarget(0), _openGLSpyId(0)
{
}


// ---------------------------------------------------------------------------
// Name:        apCLImage::apCLImage
// Description: Constructor.
// Author:  AMD Developer Tools Team
// Date:        1/12/2009
// ---------------------------------------------------------------------------
apCLImage::apCLImage(int imageName, apTextureType imageType, gtSize_t width, gtSize_t height, gtSize_t depth):
    apCLMemObject(), _imageName(imageName), _imageType(imageType), _clDataFormat(0), _clDataType(0),
    _contextHandle(OA_CL_NULL_HANDLE), _width(width), _height(height), _depth(depth), _buffer(OA_CL_NULL_HANDLE), _imageFilePath(L""), _isDirty(true),
    _openGLRenderBufferName(0), _openGLTextureName(0), _openGLMiplevel(0), _openGLTarget(0), _openGLSpyId(0)
{
}


// ---------------------------------------------------------------------------
// Name:        apCLImage::~apCLImage
// Description: Destructor.
// Author:  AMD Developer Tools Team
// Date:        1/12/2009
// ---------------------------------------------------------------------------
apCLImage::~apCLImage()
{
}

// ---------------------------------------------------------------------------
// Name:        apCLImage::type
// Description: Returns my transferable object type.
// Author:  AMD Developer Tools Team
// Date:        1/12/2009
// ---------------------------------------------------------------------------
osTransferableObjectType apCLImage::type() const
{
    return OS_TOBJ_ID_CL_IMAGE;
}


// ---------------------------------------------------------------------------
// Name:        apCLImage::writeSelfIntoChannel
// Description: Writes this object into an IPC Channel.
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        1/12/2009
// ---------------------------------------------------------------------------
bool apCLImage::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = false;
    // Write the image type:
    ipcChannel << _imageName;
    ipcChannel << (gtInt32)_imageType;
    ipcChannel << (gtInt64)_clDataFormat;
    ipcChannel << (gtInt64)_clDataType;

    // Write the context handle:
    ipcChannel << (gtUInt64)_contextHandle;

    // Write the image dimensions:
    ipcChannel << (gtUInt64)_width;
    ipcChannel << (gtUInt64)_height;
    ipcChannel << (gtUInt64)_depth;

    // Wirte the buffer handle:
    ipcChannel << (gtUInt64)_buffer;

    // Write the texture GL details:
    ipcChannel << (gtUInt32)_openGLRenderBufferName;
    ipcChannel << (gtUInt32)_openGLTextureName;
    ipcChannel << (gtInt32)_openGLMiplevel;
    ipcChannel << (gtUInt32)_openGLTarget;

    ipcChannel << (gtInt32)_openGLSpyId;

    // Write the file path:
    retVal = _imageFilePath.writeSelfIntoChannel(ipcChannel);

    // Write the mem object Info:
    retVal = apCLMemObject::writeSelfIntoChannel(ipcChannel) && retVal;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        apCLImage::readSelfFromChannel
// Description: Reads this object from an IPC Channel.
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        1/12/2009
// ---------------------------------------------------------------------------
bool apCLImage::readSelfFromChannel(osChannel& ipcChannel)
{
    bool retVal = false;

    // Read the image name:
    ipcChannel >> _imageName;

    // Read the image type:
    gtInt32 int32Var = 0;
    ipcChannel >> int32Var;
    _imageType = (apTextureType)int32Var;
    gtInt64 int64Var = 0;

    // Read the data type & format:
    ipcChannel >> int64Var;
    _clDataFormat = (cl_uint)int64Var;
    ipcChannel >> int64Var;
    _clDataType = (cl_uint)int64Var;

    // Read the context handle:
    gtUInt64 uint64Var = 0;
    ipcChannel >> uint64Var;
    _contextHandle = (oaCLContextHandle)uint64Var;

    // Read dimensions:
    ipcChannel >> uint64Var;
    _width = (gtSize_t)uint64Var;

    ipcChannel >> uint64Var;
    _height = (gtSize_t)uint64Var;

    ipcChannel >> uint64Var;
    _depth = (gtSize_t)uint64Var;

    gtUInt64 bufferAsUInt64 = 0;
    ipcChannel >> bufferAsUInt64;
    _buffer = (oaCLMemHandle)bufferAsUInt64;

    // Read the texture GL details:
    gtUInt32 uint32Var = 0;

    ipcChannel >> uint32Var;
    _openGLRenderBufferName = (GLuint)uint32Var;

    ipcChannel >> uint32Var;
    _openGLTextureName = (GLuint)uint32Var;

    ipcChannel >> int32Var;
    _openGLMiplevel = (GLint)int32Var;

    ipcChannel >> uint32Var;
    _openGLTarget = (GLenum)uint32Var;

    ipcChannel >> int32Var;
    _openGLSpyId = (int)int32Var;

    // Read the buffer flags:
    retVal = _imageFilePath.readSelfFromChannel(ipcChannel);

    // Read the mem object Info:
    retVal = apCLMemObject::readSelfFromChannel(ipcChannel) && retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLImage::getMemorySize
// Description: Calculate a image memory size
// Arguments: gtSize_t& memorySize
// Return Val: bool  - Success / failure.
// Author:  AMD Developer Tools Team
// Date:        6/12/2009
// ---------------------------------------------------------------------------
bool apCLImage::getMemorySize(gtSize_t& memorySize) const
{
    bool retVal = false;

    // Get the image dimensions:
    gtSize_t width = 0, height = 0, depth = 0;
    getDimensions(width, height, depth);

    // Get the data format and type in our structures:
    oaTexelDataFormat dataFormat = OA_TEXEL_FORMAT_UNKNOWN;
    oaDataType dataType = OA_UNKNOWN_DATA_TYPE;

    // Get the data format as oaTexelDataFormat:
    bool rcTexelFormat = oaCLImageFormatToTexelFormat(_clDataFormat, dataFormat);

    // Get the data type as oaDataType:
    bool rcDataType = oaCLImageDataTypeToOSDataType(_clDataType, dataType);
    GT_IF_WITH_ASSERT(rcDataType && rcTexelFormat)
    {
        // Get the image pixel size:
        int pixelSize = oaCalculatePixelUnitByteSize(dataFormat, dataType);

        // Calculate the image number of pixels:
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
        memorySize = (gtSize_t)numberOfPixels * pixelSize * 8;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apCLImage::pixelSize
// Description: Return the image pixel size
// Return Val:  int
// Author:  AMD Developer Tools Team
// Date:        6/5/2010
// ---------------------------------------------------------------------------
int apCLImage::pixelSize() const
{
    int retVal = 0;

    // Get the data format and type in our structures:
    oaTexelDataFormat dataFormat = OA_TEXEL_FORMAT_UNKNOWN;
    oaDataType dataType = OA_UNKNOWN_DATA_TYPE;

    // Get the data format as oaTexelDataFormat:
    bool rcTexelFormat = oaCLImageFormatToTexelFormat(_clDataFormat, dataFormat);

    // Get the data type as oaDataType:
    bool rcDataType = oaCLImageDataTypeToOSDataType(_clDataType, dataType);
    GT_IF_WITH_ASSERT(rcDataType && rcTexelFormat)
    {
        // Calculate the pixel size according to the data format and data type:
        retVal = oaCalculatePixelUnitByteSize(dataFormat, dataType);
    }
    return retVal;

}


// ---------------------------------------------------------------------------
// Name:        apCLImage::textureTypeFromMemObjectType
// Description: Convert an open cl memory object type to a texture type
// Arguments:   cl_mem_object_type image_type
// Return Val:  apTextureType
// Author:  AMD Developer Tools Team
// Date:        16/1/2012
// ---------------------------------------------------------------------------
bool apCLImage::textureTypeFromMemObjectType(cl_mem_object_type image_type, apTextureType& textureType)
{
    bool retVal = false;
    textureType = AP_UNKNOWN_TEXTURE_TYPE;

    switch (image_type)
    {

        case CL_MEM_OBJECT_IMAGE1D:
            retVal = true;
            textureType = AP_1D_TEXTURE;
            break;

        case CL_MEM_OBJECT_IMAGE2D:
            retVal = true;
            textureType = AP_2D_TEXTURE;
            break;

        case CL_MEM_OBJECT_IMAGE3D:
            retVal = true;
            textureType = AP_3D_TEXTURE;
            break;

        case CL_MEM_OBJECT_IMAGE1D_ARRAY:
            retVal = true;
            textureType = AP_1D_ARRAY_TEXTURE;
            break;

        case CL_MEM_OBJECT_IMAGE2D_ARRAY:
            retVal = true;
            textureType = AP_2D_ARRAY_TEXTURE;
            break;

        case CL_MEM_OBJECT_IMAGE1D_BUFFER:
            textureType = AP_BUFFER_TEXTURE;
            retVal = true;
            break;

        case CL_MEM_OBJECT_BUFFER:
        case CL_MEM_OBJECT_PIPE:
            retVal = false;
            break;

        default:
            retVal = false;
            break;
    }

    return retVal;
}

