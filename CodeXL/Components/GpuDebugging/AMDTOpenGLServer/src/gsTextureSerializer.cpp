//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsTextureSerializer.cpp
///
//==================================================================================

//------------------------------ gsTextureSerializer.cpp ------------------------------

// Standard C:
#include <string.h>
#include <stdlib.h>

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSAPIWrappers/Include/oaDataType.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>
#include <AMDTOSAPIWrappers/Include/oaRawFileSeralizer.h>
#include <AMDTAPIClasses/Include/apOpenGLParameters.h>

// Spies Utilities:
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsWrappersCommon.h>
#include <src/gsTextureSerializer.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsRenderContextMonitor.h>
#include <src/gsTexturesMonitor.h>



// ---------------------------------------------------------------------------
// Name:        gsTextureSerializer::gsTextureSerializer
// Description: Constructor
// Arguments:   bindTarget - The bind target who's bounded texture will be serialized.
//              mipMapLevel - The Mip map LOD level that will be serialized.
// Author:      Yaki Tebeka
// Date:        27/12/2004
// ---------------------------------------------------------------------------
gsTextureSerializer::gsTextureSerializer(GLenum bindTarget, GLint mipMapLevel)
    : _bindTarget(bindTarget), _textureName(0), _mipMapLevel(mipMapLevel), _internalFormat(0),
      _textureType(AP_UNKNOWN_TEXTURE_TYPE), _pOpenGL3DImage(NULL),
      _isGL_OES_framebuffer_objectSupported(false), _isOpenGLES20Supported(false), _helperFBOName(0),
      _glCheckFramebufferStatus(NULL), _glCheckFramebufferStatusExt(NULL)
{
#ifdef _GR_IPHONE_BUILD
    // Check the OpenGL ES version:
    gsRenderContextMonitor* pCurrentThreadContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();
    GT_IF_WITH_ASSERT(pCurrentThreadContextMonitor != NULL)
    {
        int oglesMajorVersion = 0;
        int oglesMinorVersion = 0;
        pCurrentThreadContextMonitor->getOpenGLVersion(oglesMajorVersion, oglesMinorVersion);
        _isOpenGLES20Supported = (oglesMajorVersion >= 2);
    }

    // To check if we support this extension, we make sure we have all the function pointers:
    _isGL_OES_framebuffer_objectSupported = ((gs_stat_realFunctionPointers.glGenFramebuffersOES != NULL) &&
                                             (gs_stat_realFunctionPointers.glDeleteFramebuffersOES != NULL) &&
                                             (gs_stat_realFunctionPointers.glFramebufferTexture2DOES != NULL) &&
                                             (gs_stat_realFunctionPointers.glCheckFramebufferStatusOES != NULL) &&
                                             (gs_stat_realFunctionPointers.glBindFramebufferOES != NULL));
#endif

    // Initialize the function pointers used for framebuffer texture object read:
    _glCheckFramebufferStatusExt = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)gsGetSystemsOGLModuleProcAddress("glCheckFramebufferStatusExt");
    _glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)gsGetSystemsOGLModuleProcAddress("glCheckFramebufferStatus");

}


// ---------------------------------------------------------------------------
// Name:        gsTextureSerializer::~gsTextureSerializer
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        23/8/2006
// ---------------------------------------------------------------------------
gsTextureSerializer::~gsTextureSerializer()
{
#ifdef _GR_IPHONE_BUILD

    // Clean up the "helper" FBO:
    if (_helperFBOName != 0)
    {
        if (_isOpenGLES20Supported)
        {
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDeleteFramebuffers);
            gs_stat_realFunctionPointers.glDeleteFramebuffers(1, &_helperFBOName);
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDeleteFramebuffers);
        }
        else if (_isGL_OES_framebuffer_objectSupported)
        {
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDeleteFramebuffersOES);
            gs_stat_realFunctionPointers.glDeleteFramebuffersOES(1, &_helperFBOName);
            SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glDeleteFramebuffersOES);
        }

        _helperFBOName = 0;
    }

#endif
}

// ---------------------------------------------------------------------------
// Name:        gsTextureSerializer::saveRawDataToFile
// Description: Saves the texture raw data into a file.
// Arguments:   filePath - The path of the file into which the texture raw
//              data will be saved.
// Return Val:  bool - Success / failure.
// Author:      Eran Zinman
// Date:        3/12/2007
// ---------------------------------------------------------------------------
bool gsTextureSerializer::saveRawDataToFile(const gsGLTexture* pTextureObj, const osFilePath& filePath)
{
    bool retVal = false;
    GT_IF_WITH_ASSERT(pTextureObj != NULL)
    {
        // Note the texture name:
        _textureName = pTextureObj->textureName();

        // Get the texture parameters:
        getTextureParameters();

        // Get the texture type:
        _textureType = apTextureBindTargetToTextureType(_bindTarget);

        // Get texture format and texel type:
        GLenum texelsType = pTextureObj->texelsType();
        GLenum pixelFormat = pTextureObj->pixelFormat();

#ifdef _GR_IPHONE_BUILD
        // On the iPhone, we can only read using GL_UNSIGNED_BYTE and GL_RGBA or the combination
        // specified by the GL_IMPLEMENTATION_COLOR_READ_*_OES variables (even mixing the type from
        // one and the format from the other is not allowed. We check if the set we have is the allowed
        // one, else we set to the UByte / RGBA one:
        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);
        GLint allowedFormat = GL_RGBA;
        gs_stat_realFunctionPointers.glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES, &allowedFormat);
        GLint allowedType = GL_UNSIGNED_INT;
        gs_stat_realFunctionPointers.glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE_OES, &allowedType);
        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetIntegerv);

        if ((pixelFormat != (GLenum)allowedFormat) || (texelsType != (GLenum)allowedType))
        {
            pixelFormat = GL_RGBA;
            texelsType = GL_UNSIGNED_BYTE;
        }

#endif

        // Convert GL data format and data type into the equivalent OsDataFormat and oaDataType
        oaDataType componentDataType;
        oaTexelDataFormat dataFormat;

        // Check if we are supporting this data type and data format
        if ((!oaGLEnumToDataType(texelsType, componentDataType)) || (!oaGLEnumToTexelDataFormat(pixelFormat, dataFormat)))
        {
            // If data format or data type is not supported, use default data type and format
            componentDataType = OA_UNSIGNED_BYTE;
            dataFormat = OA_TEXEL_FORMAT_RGBA;
        }

        // If the data was input in one of the reverse formats, mark the dataFormat as its reverse.
        if ((componentDataType == OA_UNSIGNED_BYTE_2_3_3_REV) || (componentDataType == OA_UNSIGNED_SHORT_5_6_5_REV) ||
            (componentDataType == OA_UNSIGNED_SHORT_4_4_4_4_REV) || (componentDataType == OA_UNSIGNED_SHORT_1_5_5_5_REV) ||
            (componentDataType == OA_UNSIGNED_INT_8_8_8_8_REV) || (componentDataType == OA_UNSIGNED_INT_2_10_10_10_REV))
        {
            switch (dataFormat)
            {
                case OA_TEXEL_FORMAT_RGB:
                    dataFormat = OA_TEXEL_FORMAT_RGB_REVERSED;
                    break;

                case OA_TEXEL_FORMAT_BGR:
                    dataFormat = OA_TEXEL_FORMAT_BGR_REVERSED;
                    break;

                case OA_TEXEL_FORMAT_RGBA:
                    dataFormat = OA_TEXEL_FORMAT_RGBA_REVERSED;
                    break;

                case OA_TEXEL_FORMAT_BGRA:
                    dataFormat = OA_TEXEL_FORMAT_BGRA_REVERSED;
                    break;

                default:
                    GT_ASSERT_EX(false, L"Used reverse component data type with the wrong texel format");
                    break;
            }
        }

        // Get texture dimensions:
        GLsizei width, height, depth;
        getImageSize(width, height, depth);

        // Sometimes the texture sizes are not available (especially for non-0 mip levels)
        // in this case we want to fail the function:
        GT_IF_WITH_ASSERT((width > 0) || (height > 0) || (depth > 0))
        {
            // The amount of pages is 1 in case of 2d/1d textures, and depth in case of 3d textures:
            gtSize_t amountOfPages = 1;

            if (depth > 1)
            {
                amountOfPages = depth;
            }

            // For 1D texture array - the height is used as amount of pages:
            if (_textureType == AP_1D_ARRAY_TEXTURE)
            {
                amountOfPages = height;
            }

            // Calculate raw data pixel size
            int rawDataPixelSize = oaCalculatePixelUnitByteSize(dataFormat, componentDataType);
            GT_IF_WITH_ASSERT(rawDataPixelSize != -1)
            {
                // Calculate the raw data container size
                size_t bufferSize = width * height * rawDataPixelSize * amountOfPages;

                // Allocate the raw data container, oaRawFileSeralizer will take control of the pointer
                gtByte* pGLTextureRawData = (gtByte*)malloc(bufferSize);


                // Copy the OpenGL texture to bitmap:
                bool rc1 = copyOpenGLTextureToBitmap((void*)pGLTextureRawData, pTextureObj, dataFormat, componentDataType);
                GT_IF_WITH_ASSERT(rc1)
                {
                    // Send progress event:
                    suSendSpyProgressEvent();

                    // This is the data that will be used in the raw file seralizer
                    gtByte* pRawData = pGLTextureRawData;

                    // If texture type is cube map texture, we need to "mirror-flip" it's contents
                    if ((AP_CUBE_MAP_TEXTURE == _textureType) || (AP_CUBE_MAP_ARRAY_TEXTURE == _textureType))
                    {
                        pRawData = mirrorFlipTexture(pGLTextureRawData, dataFormat, componentDataType);
                        free(pGLTextureRawData);
                    }

                    GT_IF_WITH_ASSERT(pRawData != NULL)
                    {
                        // Set the raw data file parameters, into the oaRawFileSeralizer class
                        oaRawFileSeralizer rawFileSeralizer;

                        rawFileSeralizer.setRawData(pRawData); /* Raw file serializer will release this pointer */
                        rawFileSeralizer.setRawDataDimensions(width, height);
                        rawFileSeralizer.setRawDataFormat(dataFormat, componentDataType);
                        rawFileSeralizer.setAmountOfPages((int)amountOfPages);

                        // Write data to raw file:
                        retVal = rawFileSeralizer.saveToFile(filePath);

                        // Send progress event:
                        suSendSpyProgressEvent();
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTextureSerializer::getTextureParameters
// Description: Retrieves the input bind target bounded texture parameters.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        2/1/2005
// ---------------------------------------------------------------------------
void gsTextureSerializer::getTextureParameters()
{
    // Get the texture width, height and border sizes:
    GLint width = 0;
    GLint height = 0;
    GLint depth = 0;
    GLint borderSize = 0;
#ifdef _GR_IPHONE_BUILD
    // OpenGL ES does not support texture level parameters or 3D textures
    // GL_TEXTURE_[WIDTH|HEIGHT] are not supported pnames for glGetTexParameteriv on the iPhone:
    gsRenderContextMonitor* pRCMon = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();
    GT_IF_WITH_ASSERT(pRCMon != NULL)
    {
        gsTexturesMonitor* pTexMon = pRCMon->texturesMonitor();
        GT_IF_WITH_ASSERT(pTexMon != NULL)
        {
            const gsGLTexture* pTexture = pTexMon->getTextureObjectDetails(_textureName);
            GT_IF_WITH_ASSERT(pTexture != NULL)
            {
                pTexture->getDimensions(width, height, depth, borderSize, _mipMapLevel);
            }
        }
    }
#else
    // Check for OpenGL errors:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
    GLenum formerOpenGLError = gs_stat_realFunctionPointers.glGetError();
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

    GT_ASSERT(formerOpenGLError == GL_NO_ERROR);

    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetTexLevelParameteriv);
    gs_stat_realFunctionPointers.glGetTexLevelParameteriv(_bindTarget, _mipMapLevel, GL_TEXTURE_WIDTH, &width);

    if (_bindTarget != GL_TEXTURE_1D)
    {
        // 1D textures have no height:
        gs_stat_realFunctionPointers.glGetTexLevelParameteriv(_bindTarget, _mipMapLevel, GL_TEXTURE_HEIGHT, &height);
    }

    gs_stat_realFunctionPointers.glGetTexLevelParameteriv(_bindTarget, _mipMapLevel, GL_TEXTURE_BORDER, &borderSize);

    if ((_bindTarget == GL_TEXTURE_3D) || (_bindTarget == GL_TEXTURE_2D_ARRAY))
    {
        // Get the texture depth:
        gs_stat_realFunctionPointers.glGetTexLevelParameteriv(_bindTarget, _mipMapLevel, GL_TEXTURE_DEPTH_EXT, &depth);
    }

#endif

    // Check for OpenGL errors:
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
    GLenum openGLError = gs_stat_realFunctionPointers.glGetError();
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

    // Check if we got dimensions successfully from texture parameters. If not, get the dimensions from the
    // texture object (the requested texture dimensions):

    bool gotDimensions = ((openGLError == GL_NO_ERROR) && ((width > 0) || (height > 0) || (depth > 0)));

    // If there was an OpenGL error - fail the function:
    if (!gotDimensions)
    {
        // OpenGL ES does not support texture level parameters or 3D textures
        // GL_TEXTURE_[WIDTH|HEIGHT] are not supported pnames for glGetTexParameteriv on the iPhone:
        gsRenderContextMonitor* pRCMon = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();
        GT_IF_WITH_ASSERT(pRCMon != NULL)
        {
            gsTexturesMonitor* pTexMon = pRCMon->texturesMonitor();
            GT_IF_WITH_ASSERT(pTexMon != NULL)
            {
                const gsGLTexture* pTexture = pTexMon->getTextureObjectDetails(_textureName);
                GT_IF_WITH_ASSERT(pTexture != NULL)
                {
                    pTexture->getDimensions(width, height, depth, borderSize, _mipMapLevel);
                }
            }
        }
    }

    // For 1D textures, the height is 0, but the image height is 1:
    if (_bindTarget == GL_TEXTURE_1D)
    {
        GT_ASSERT(height == 0);
        height = 1;
    }

    // Store the texture size:
    setImageSize(width, height, depth);

#ifdef _GR_IPHONE_BUILD
    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetTexParameteriv);
    gs_stat_realFunctionPointers.glGetTexParameteriv(_bindTarget, GL_TEXTURE_INTERNAL_FORMAT, &_internalFormat);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetTexParameteriv);
#else
    // Get the format in which the graphic board stores the texture:
    gs_stat_realFunctionPointers.glGetTexLevelParameteriv(_bindTarget, _mipMapLevel, GL_TEXTURE_INTERNAL_FORMAT, &_internalFormat);
    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetTexLevelParameteriv);
#endif

}



// ---------------------------------------------------------------------------
// Name:        gsTextureSerializer::mirrorFlipTexture
// Description: Mirror flips the texture raw data
// Arguments:   pBitmap - The bitmap that we need to mirror flip
//              dataFormat - The format of the raw data (for example OS_BGRA).
//              componentDataType - The data type of the raw data
//              (for example OA_UNSIGNED_BYTE)
// Return Val:  The mirror flipped raw data
// Author:      Eran Zinman
// Date:        28/12/2007
// ---------------------------------------------------------------------------
gtByte* gsTextureSerializer::mirrorFlipTexture(const gtByte* pRawData, oaTexelDataFormat dataFormat, oaDataType componentDataType)
{
    gtByte* pDest = NULL;

    // Sanity check:
    GT_IF_WITH_ASSERT(pRawData != NULL)
    {
        // Calculate raw data pixel size
        int rawDataPixelSize = oaCalculatePixelUnitByteSize(dataFormat, componentDataType);
        GT_IF_WITH_ASSERT(rawDataPixelSize != -1)
        {
            // Retrieve image size
            GLsizei width = 0;
            GLsizei height = 0;
            GLsizei depth = 0;
            getImageSize(width, height, depth);

            // We only handle one page texture and cube map arrays here
            bool validDepth = (0 == depth);
            GLsizei usedDepth = 1;

            if (GL_TEXTURE_CUBE_MAP_ARRAY == _bindTarget)
            {
                validDepth = ((0 < depth) && (0 == (depth % 6)));
                usedDepth = depth;
            }

            GT_IF_WITH_ASSERT(validDepth)
            {
                // Calculate the raw data container size
                size_t slicePitch = width * height * rawDataPixelSize;
                size_t bufferSize = slicePitch * usedDepth;

                // Allocate output raw data buffer
                pDest = (gtByte*)malloc(bufferSize);
                GT_IF_WITH_ASSERT(pDest != NULL)
                {
                    for (int z = 0; z < usedDepth; ++z)
                    {
                        // Calculate input read position and output read position
                        const gtByte* pReadPos = pRawData + (z * slicePitch);
                        gtByte* pWritePos = pDest + ((z + 1) * slicePitch) - rawDataPixelSize;

                        for (int y = 0; y < height; y++)
                        {
                            for (int x = 0; x < width; x++)
                            {
                                // Copy data from source to destination
                                memcpy(pWritePos, pReadPos, rawDataPixelSize);

                                // Advance to next pixel
                                pReadPos += rawDataPixelSize;
                                pWritePos -= rawDataPixelSize;
                            }
                        }
                    }
                }
            }
        }
    }

    return pDest;
}

// ---------------------------------------------------------------------------
// Name:        gsTextureSerializer::readTextureViaFBO
// Description: Uses a framebuffer object to read a texture. This is done as a
//              replacement for glGetTexImage, which isn't supported on OpenGL ES
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        25/5/2009
// ---------------------------------------------------------------------------
bool gsTextureSerializer::readTextureViaFBO(GLenum pixelFormat, GLenum texelsType, void* pBitmap)
{
    bool retVal = false;

    // This method is only needed on the iPhone:
#ifdef _GR_IPHONE_BUILD
    // Sanity check:
    GT_IF_WITH_ASSERT(pBitmap != NULL)
    {
        // In OpenGL ES 1.1, We currently only support 2D texture level 0 reading this way.
        // This is due to limitations imposed by glFramebufferTexture2DOES().
        // In OpenGL ES 2.0, we also allow reading cube map textures' level 0 texels.
        bool isSupportedBindTarget = (_bindTarget == GL_TEXTURE_2D);

        if (_isOpenGLES20Supported)
        {
            isSupportedBindTarget = isSupportedBindTarget || ((_bindTarget == GL_TEXTURE_CUBE_MAP_POSITIVE_X) || (_bindTarget == GL_TEXTURE_CUBE_MAP_NEGATIVE_X) || (_bindTarget == GL_TEXTURE_CUBE_MAP_POSITIVE_Y) ||
                                                              (_bindTarget == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y) || (_bindTarget == GL_TEXTURE_CUBE_MAP_POSITIVE_Z) || (_bindTarget == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z));
        }

        if (isSupportedBindTarget && (_mipMapLevel == 0))
        {
            if (_isOpenGLES20Supported)
            {
                if (_helperFBOName == 0)
                {
                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGenFramebuffers);
                    gs_stat_realFunctionPointers.glGenFramebuffers(1, &_helperFBOName);
                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGenFramebuffers);
                }

                GT_IF_WITH_ASSERT(_helperFBOName != 0)
                {
                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindFramebuffer);
                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glFramebufferTexture2D);
                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCheckFramebufferStatus);

                    GLuint currentBoundFBOName = 0;
                    gsRenderContextMonitor* pCurrentThreadContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();
                    GT_IF_WITH_ASSERT(pCurrentThreadContextMonitor != NULL)
                    {
                        currentBoundFBOName = pCurrentThreadContextMonitor->getActiveReadFboName();
                    }

                    // Bind the framebuffer:
                    gs_stat_realFunctionPointers.glBindFramebuffer(GL_FRAMEBUFFER, _helperFBOName);

                    // Connect the framebuffer to the texture:
                    gs_stat_realFunctionPointers.glFramebufferTexture2DOES(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, _bindTarget, _textureName, _mipMapLevel);

                    // Make sure this completed successfully:
                    GLuint status = gs_stat_realFunctionPointers.glCheckFramebufferStatus(GL_FRAMEBUFFER);

                    if (status == GL_FRAMEBUFFER_COMPLETE)
                    {
                        // Get the image size:
                        GLsizei width = 0, height = 0, depth = 0;
                        getImageSize(width, height, depth);
                        GT_ASSERT(depth == 0);

                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glReadPixels);
                        gs_stat_realFunctionPointers.glReadPixels(0, 0, width, height, pixelFormat, texelsType, pBitmap);
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glReadPixels);

                        retVal = true;
                    }
                    else
                    {
                        gtString errMsg;
                        apGLenumValueToString(status, errMsg);
                        errMsg.prepend(L"Got an error while creating framebuffer: ");
                        GT_ASSERT_EX(status == GL_FRAMEBUFFER_COMPLETE, errMsg.asCharArray())
                    }

                    // restore / unbind the framebuffer:
                    gs_stat_realFunctionPointers.glBindFramebuffer(GL_FRAMEBUFFER, currentBoundFBOName);

                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCheckFramebufferStatus);
                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glFramebufferTexture2D);
                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebuffer);
                }
            }
            else if (_isGL_OES_framebuffer_objectSupported)
            {
                // OpenGL ES 1.1, we use OES_framebuffer_object (we make sure we have all the function pointers):
                if (_helperFBOName == 0)
                {
                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGenFramebuffersOES);
                    gs_stat_realFunctionPointers.glGenFramebuffersOES(1, &_helperFBOName);
                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGenFramebuffersOES);
                }

                GT_IF_WITH_ASSERT(_helperFBOName != 0)
                {
                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferOES);
                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glFramebufferTexture2DOES);
                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glCheckFramebufferStatusOES);

                    GLuint currentBoundFBOName = 0;
                    gsRenderContextMonitor* pCurrentThreadContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();
                    GT_IF_WITH_ASSERT(pCurrentThreadContextMonitor != NULL)
                    {
                        currentBoundFBOName = pCurrentThreadContextMonitor->getActiveFboName();
                    }

                    // Bind the framebuffer:
                    gs_stat_realFunctionPointers.glBindFramebufferOES(GL_FRAMEBUFFER_OES, _helperFBOName);

                    // Connect the framebuffer to the texture:
                    gs_stat_realFunctionPointers.glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, _bindTarget, _textureName, _mipMapLevel);

                    // Make sure this completed successfully:
                    GLuint status = gs_stat_realFunctionPointers.glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);

                    if (status == GL_FRAMEBUFFER_COMPLETE_OES)
                    {
                        // Get the image size:
                        GLsizei width = 0, height = 0, depth = 0;
                        getImageSize(width, height, depth);
                        GT_ASSERT(depth == 0);

                        SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glReadPixels);
                        gs_stat_realFunctionPointers.glReadPixels(0, 0, width, height, pixelFormat, texelsType, pBitmap);
                        SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glReadPixels);

                        retVal = true;
                    }
                    else // status != GL_FRAMEBUFFER_COMPLETE_OES
                    {
                        // Report the incomplete status to the log on Debug level:
                        if (osDebugLog::instance().loggedSeverity() >= OS_DEBUG_LOG_DEBUG)
                        {
                            gtString errMsg;
                            apGLenumValueToString(status, errMsg);
                            errMsg.prepend(L"Got an error while creating framebuffer: ");
                            OS_OUTPUT_DEBUG_LOG(errMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);
                        }
                    }

                    // restore / unbind the framebuffer:
                    gs_stat_realFunctionPointers.glBindFramebufferOES(GL_FRAMEBUFFER_OES, currentBoundFBOName);

                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glCheckFramebufferStatusOES);
                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glFramebufferTexture2DOES);
                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glBindFramebufferOES);
                }
            }
        }
    }
#else
    // Resolve the compiler warning for the Linux variant
    (void)(pixelFormat);
    (void)(texelsType);
    (void)(pBitmap);
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsTextureSerializer::readTextureAttachedToFBO
// Description: Read a texture via FBO. This is done instead of the usual mechanism
//              of texture object extraction
// Arguments: GLenum pixelFormat
//            GLenum texelsType
//            void* pBitmap
//            apGLTexture* pTextureObj
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        30/8/2009
// ---------------------------------------------------------------------------
bool gsTextureSerializer::readTextureAttachedToFBO(GLenum pixelFormat, GLenum texelsType, void* pBitmap, const apGLTexture* pTextureObj)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((pBitmap != NULL) && (pTextureObj != NULL))
    {
        // Get the texture FBO name:
        GLuint fboName = pTextureObj->getFBOName();

        if (_mipMapLevel == 0)
        {
            GT_IF_WITH_ASSERT(fboName != 0)
            {
                gsRenderContextMonitor* pCurrentThreadContextMonitor = gs_stat_openGLMonitorInstance.currentThreadRenderContextMonitor();
                GT_IF_WITH_ASSERT(pCurrentThreadContextMonitor != NULL)
                {
                    // Get the FBO monitor:
                    const gsFBOMonitor* pFBOMonitor = pCurrentThreadContextMonitor->fboMonitor();
                    GT_IF_WITH_ASSERT(pFBOMonitor != NULL)
                    {
                        // Get the texture attachment from the texture object:
                        GLenum fboAttachmentPoint = GL_NONE;
                        apGLFBO* pFBO = pFBOMonitor->getFBODetails(fboName);
                        GT_IF_WITH_ASSERT(pFBO != NULL)
                        {
                            // Get the texture object attachment:
                            bool isTextureBound = false;
                            gtList<apFBOBindObject*> bindedObjects = pFBO->getBindedObjects();
                            gtList<apFBOBindObject*>::const_iterator iter;
                            gtList<apFBOBindObject*>::const_iterator iterEnd = bindedObjects.end();

                            for (iter = bindedObjects.begin(); iter != iterEnd; iter++)
                            {
                                apFBOBindObject* pCurrent = *iter;

                                if (pCurrent != NULL)
                                {
                                    bool isTextureAttachment = false;
                                    bool rc = apGLFBO::isTextureAttachmentTarget(pCurrent->_attachmentTarget, isTextureAttachment);
                                    GT_IF_WITH_ASSERT(rc)
                                    {
                                        if (isTextureAttachment)
                                        {
                                            if (pCurrent->_name == pTextureObj->textureName())
                                            {
                                                fboAttachmentPoint = pCurrent->_attachmentPoint;
                                                isTextureBound = true;
                                            }
                                        }
                                    }
                                }
                            }

                            GT_IF_WITH_ASSERT(isTextureBound)
                            {
                                // Make sure this completed successfully:
                                GLuint status = GL_NONE;

                                if (_glCheckFramebufferStatus != NULL)
                                {
                                    status = _glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
                                }
                                else if (_glCheckFramebufferStatusExt != NULL)
                                {
                                    status = _glCheckFramebufferStatusExt(GL_FRAMEBUFFER_EXT);
                                }

                                if (status == GL_FRAMEBUFFER_COMPLETE)
                                {
                                    // Get the image size:
                                    GLsizei width = 0, height = 0, depth = 0;
                                    getImageSize(width, height, depth);
                                    GT_ASSERT(depth == 0);

                                    setOpenGLReadBufferParameter(fboAttachmentPoint);

                                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glReadPixels);
                                    gs_stat_realFunctionPointers.glReadPixels(0, 0, width, height, pixelFormat, texelsType, pBitmap);
                                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glReadPixels);

                                    restoreOpenGLReadBufferParameter();

                                    retVal = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsTextureSerializer::copyOpenGLTextureToBitmap
// Description: Copies the input OpenGL bind target texture to a bitmap.
// Arguments:   pBitmap - The bitmap that will get the OpenGL texture.
//              const gsGLTexture* pTextureObj - the texture object
//              dataFormat - The format to copy the data (for example OS_BGRA).
//              componentDataType - The data type (for example OA_UNSIGNED_BYTE)
// Return Val:  bool - Success / Failure
// Author:      Yaki Tebeka
// Date:        16/1/2005
// ---------------------------------------------------------------------------
bool gsTextureSerializer::copyOpenGLTextureToBitmap(void* pBitmap, const gsGLTexture* pTextureObj, oaTexelDataFormat dataFormat, oaDataType componentDataType)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(pBitmap != NULL)
    {
        // Get texture format and texel type in OpenGL format:
        GLenum texelsType = GL_NONE;
        GLenum pixelFormat = GL_NONE;

        texelsType = oaDataTypeToGLEnum(componentDataType);
        GT_IF_WITH_ASSERT(texelsType != GL_NONE)
        {
            pixelFormat = oaTexelDataFormatToGLEnum(dataFormat);
            GT_IF_WITH_ASSERT(pixelFormat != GL_NONE)
            {
                retVal = true;

                // Set the OpenGL pixel "pack" parameters:
                setOpenGLPixelPackParameters(1);

                // Clear previous openGL errors:
                SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetError);
                GLenum oglError = gs_stat_realFunctionPointers.glGetError();

                // Get the texture out of OpenGL:
#ifdef _GR_IPHONE_BUILD
                retVal = readTextureViaFBO(pixelFormat, texelsType, pBitmap);
#else

                if (pTextureObj->isTextureBoundToActiveFBO())
                {
                    // If the texture is bound to the active FBO, read its content from the
                    // FBO and not using glGetTexImage:
                    readTextureAttachedToFBO(pixelFormat, texelsType, pBitmap, pTextureObj);
                }
                else
                {
                    SU_BEFORE_EXECUTING_REAL_FUNCTION(ap_glGetTexImage);
                    gs_stat_realFunctionPointers.glGetTexImage(_bindTarget, _mipMapLevel, pixelFormat, texelsType, pBitmap);
                    SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetTexImage);
                }

#endif

                // Check if we generated errors:
                oglError = gs_stat_realFunctionPointers.glGetError();
                SU_AFTER_EXECUTING_REAL_FUNCTION(ap_glGetError);

                if (oglError != GL_NO_ERROR)
                {
                    retVal = false;

                    gtString errString;
                    apGLenumValueToString(oglError, errString);
                    errString.prepend(L"Copy texture data (glGetTexImage) error: ");
                    GT_ASSERT_EX(false, errString.asCharArray());
                }

                // Restore the OpenGL pixel "pack" parameters:
                restoreOpenGLPixelPackParameters();
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTextureSerializer::allocateOpenGL3DImageCopy
// Description: Allocates space to hold a copy of the OpenGL 3D texture image.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        16/1/2005
// ---------------------------------------------------------------------------
bool gsTextureSerializer::allocateOpenGL3DImageCopy()
{
    bool retVal = false;

    // Calculate the amount of required bytes:
    GLsizei width = 0;
    GLsizei height = 0;
    GLsizei depth = 0;
    getImageSize(width, height, depth);
    long imageSize = width * height * depth * GS_BYTES_PER_PIXEL;

    // Allocate the image copy bytes:
    _pOpenGL3DImage = new GLubyte[imageSize];

    if (_pOpenGL3DImage)
    {
        retVal = true;
    }

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gsTextureSerializer::retrievedTextureDataFormat
// Description: Calculates and retrieved texture data format.
// Author:      Yaki Tebeka
// Date:        30/1/2005
// Implementation Notes:
//   We consider single and double component internal texture formats as
//   gray-scale images.
// ---------------------------------------------------------------------------
gsTextureSerializer::gsRetrievedDataFormat gsTextureSerializer::retrievedTextureDataFormat()
{
    gsRetrievedDataFormat retVal = GS_RGBA_TEX_IMAGE;

    switch (_internalFormat)
    {
        // Internal formats that fills only the alpha component:
        case GL_ALPHA:
        case GL_ALPHA4:
        case GL_ALPHA8:
        case GL_ALPHA12:
        case GL_ALPHA16:
        {
            retVal = GS_ALPHA_TEX_IMAGE;
            break;
        };

        // Internal formats that fill the Red (and maybe also alpha) component:
        case GL_LUMINANCE:
        case GL_LUMINANCE4:
        case GL_LUMINANCE8:
        case GL_LUMINANCE12:
        case GL_LUMINANCE16:
        case GL_LUMINANCE_ALPHA:
        case GL_LUMINANCE4_ALPHA4:
        case GL_LUMINANCE6_ALPHA2:
        case GL_LUMINANCE8_ALPHA8:
        case GL_LUMINANCE12_ALPHA4:
        case GL_LUMINANCE12_ALPHA12:
        case GL_LUMINANCE16_ALPHA16:
        case GL_INTENSITY:
        case GL_INTENSITY4:
        case GL_INTENSITY8:
        case GL_INTENSITY12:
        case GL_INTENSITY16:
        {
            retVal = GS_GRAY_SCALE_TEX_IMAGE;
            break;
        };

        // All other formats are considered RGBA formats:
        default:
        {
            retVal = GS_RGBA_TEX_IMAGE;
            break;
        }
    }

    return retVal;
}

