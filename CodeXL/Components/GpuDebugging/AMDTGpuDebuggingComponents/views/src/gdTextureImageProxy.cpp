//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdTextureImageProxy.cpp
///
//==================================================================================

//------------------------------ gdTextureImageProxy.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>




// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osStopWatch.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>
#include <AMDTAPIClasses/Include/apCLImage.h>
#include <AMDTAPIClasses/Include/apGLPixelInternalFormatParameter.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acRawFileHandler.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdTextureImageProxy.h>



// ---------------------------------------------------------------------------
// Name:        gdTextureImageProxy::gdTextureImageProxy
// Description: Constructor
// Arguments:   gdDebugApplicationTreeData* pTextureTreeItemData
//              isInGLBeginEndBlock - are we in a begin - end block
//              imageW - the image width
//              imageH - the image height
// Author:      Sigal Algranaty
// Date:        10/4/2011
// ---------------------------------------------------------------------------
gdTextureImageProxy::gdTextureImageProxy(afApplicationTreeItemData* pTextureTreeItemData, bool isInGLBeginEndBlock, int imageW, int imageH, bool releaseItemDataMemory) :
    _pTextureTreeItemData(pTextureTreeItemData), _releaseItemDataMemory(releaseItemDataMemory), _isInGLBeginEndBlock(isInGLBeginEndBlock),
    _imageW(imageW), _imageH(imageH), _optimizedMiplevel(-1), _calculatedLoadedImageSize(-1)
{

}

// ---------------------------------------------------------------------------
// Name:        gdTextureImageProxy::~gdTextureImageProxy
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        10/4/2011
// ---------------------------------------------------------------------------
gdTextureImageProxy::~gdTextureImageProxy()
{
    if (_releaseItemDataMemory)
    {
        if (_pTextureTreeItemData != NULL)
        {
            delete _pTextureTreeItemData;
            _pTextureTreeItemData = NULL;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdTextureImageProxy::loadImage
// Description: Generates a texture image preview
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/7/2012
// ---------------------------------------------------------------------------
bool gdTextureImageProxy::loadImage()
{
    bool retVal = false;
    // First, clear the loaded image if such an image exists
    releaseLoadedImage();

    // Sanity check:
    GT_IF_WITH_ASSERT(_pTextureTreeItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pTextureTreeItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Generate a texture preview only if we not in a glBegin - glEnd block
            if (!_isInGLBeginEndBlock)
            {
                // Try to generate the texture image
                bool rc1 = generateTextureImage();

                if (!rc1)
                {
                    // We failed to generate the texture image. Now we got two options:
                    // 1. If texture type is unknown, we notify the user texture type is unknown.
                    // 2. There was a problem writing / reading / converting the texture. Show texture not available message

                    // This flag indicates if texture type is unknown
                    bool isTextureTypeUnknown = false;

                    // Get the selected texture details:
                    bool rc2 = gaGetTextureObjectThumbnailData(pGDData->_contextId._contextId, pGDData->_textureMiplevelID._textureName, _textureThumbnailDetails);

                    if (rc2)
                    {
                        // Get texture type
                        apTextureType textureType = _textureThumbnailDetails.textureType();

                        // Is this an unknown texture?
                        isTextureTypeUnknown = (textureType == AP_UNKNOWN_TEXTURE_TYPE);
                    }

                    // Get the context type:
                    bool isOpenGLContext = (pGDData->_contextId.isOpenGLContext());
                    GT_ASSERT(pGDData->_contextId.isValid() && (!pGDData->_contextId.isDefault()));

                    // Check if the texture format is supported for OpenGL ES projects:
                    bool isSupportedFormat = true;
#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)

                    if (apDoesProjectTypeSupportOpenGLES(gdGDebuggerGlobalVariablesManager::instance().CodeXLProjectType()))
                    {
                        // Get the buffer format:
                        oaTexelDataFormat dataFormat = _textureThumbnailDetails.texelDataFormat();

                        // Check if the buffer format is supported for OpenGLES (RGB | RGBA):
                        if ((dataFormat != OA_TEXEL_FORMAT_RGB) && (dataFormat != OA_TEXEL_FORMAT_RGBA))
                        {
                            isSupportedFormat = false;
                        }
                    }

#endif

                    if (isTextureTypeUnknown || !isSupportedFormat)
                    {
                        // If this is an unknown texture, generate an unknown texture message:
                        m_pLoadedQImage = createMessageImage(isOpenGLContext ? GD_STR_ImageProxyTextureTypeUnknown : GD_STR_ImageProxyImageTypeUnknown);
                    }
                    else
                    {
                        // Generate an "Texture data is not available at this time." message:
                        gtString msg;

                        if (isOpenGLContext)
                        {
                            msg.appendFormattedString(GD_STR_ImageProxyObjectNAMessage, GD_STR_ImageProxyTexture);
                        }
                        else
                        {
                            msg.appendFormattedString(GD_STR_ImageProxyObjectNAMessage, GD_STR_ImageProxyImage);
                        }

                        m_pLoadedQImage = createMessageImage(msg);
                    }
                }
            }
        }
        else
        {
            // We are in a glBegin - glEnd block. generate the appropriate message:
            gtString msg;
            msg.appendFormattedString(GD_STR_ImageProxyGLBeginEndMessage, GD_STR_ImageProxyTexture);
            m_pLoadedQImage = createMessageImage(msg);
        }
    }

    // Return true if we got a valid image
    retVal = (m_pLoadedQImage != NULL);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdTextureImageProxy::calculateOptimizedMiplevel
// Description: Calculate the optimized texture mip level for the requested
//              image size
// Arguments: int& optimizedMipLevel
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/1/2009
// ---------------------------------------------------------------------------
bool gdTextureImageProxy::calculateOptimizedMiplevel()
{
    bool retVal = false;
    _optimizedMiplevel = 0;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pTextureTreeItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pTextureTreeItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

            // TO_DO iPhone: Uri, 14/6/09 - we currently can't get mipmap levels beyond 0 in the iPhone
            // spy, so we just use level 0 here:
            if (apDoesProjectTypeSupportOpenGLES(gdGDebuggerGlobalVariablesManager::instance().CodeXLProjectType()))
            {
                retVal = true;
            }
            else
#endif
            {
                // Get the texture mip level data:
                bool rc = gaGetTextureObjectThumbnailData(pGDData->_contextId._contextId, pGDData->_textureMiplevelID._textureName, _textureThumbnailDetails);
                GT_IF_WITH_ASSERT(rc)
                {
                    retVal = true;

                    // Check if the texture has mip levels:
                    if (_textureThumbnailDetails.maxLevel() > _textureThumbnailDetails.minLevel())
                    {
                        // Browse the levels, from the smallest sized to the biggest, and search for the best:
                        for (int i = _textureThumbnailDetails.maxLevel(); i >= _textureThumbnailDetails.minLevel(); i--)
                        {
                            // Get the level size division:
                            int levelExp = (int)pow(2.0f, (float)i);
                            GLsizei textureW = 0, textureH = 0, textureD = 0, borderSize = 0;
                            _textureThumbnailDetails.getDimensions(textureW, textureH, textureD, borderSize);
                            int miplevelW = textureW / levelExp;
                            int miplevelH = textureH / levelExp;

                            // Check if this mip level's size is enough:
                            if ((miplevelH >= _imageH) && (miplevelW >= _imageH))
                            {
                                _optimizedMiplevel = i;
                                retVal = true;
                                break;
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
// Name:        gdTextureImageProxy::generateTextureImage
// Description: Generates a texture image according to the texture
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool gdTextureImageProxy::generateTextureImage()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pTextureTreeItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pTextureTreeItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            if (pGDData->_contextId.isOpenCLContext())
            {
                // Generate an OpenCL texture image:
                retVal = generateCachedCLImageImage();
            }
            else if (pGDData->_contextId.isOpenGLContext())
            {
                // Generate an OpenGL texture image:
                retVal = generateCachedGLTextureImage();
            }
            else // pGDData->_contextId.isDefault()
            {
                // Texture image generation should not be called for NULL context:
                GT_ASSERT_EX(false, L"should not get here");
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdTextureImageProxy::generateCachedGLTextureImage
// Description: Generates a cached thumbnail image.
//              1. If the texture has mip levels, optimize using the mip levels
//                 mechanism.
//              2. After the thumbnail is produced, save it to the disk.
//              3. If the image is not dirty, load the cached copy of the thumbnail.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/7/2010
// ---------------------------------------------------------------------------
bool gdTextureImageProxy::generateCachedGLTextureImage()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pTextureTreeItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pTextureTreeItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            bool isInteropObject = (_pTextureTreeItemData->m_itemType == AF_TREE_ITEM_CL_IMAGE) && (pGDData->_objectOpenGLName > 0);
            isInteropObject = ((_pTextureTreeItemData->m_itemType == AF_TREE_ITEM_GL_TEXTURE) && (pGDData->_objectOpenCLName > 0)) || isInteropObject;

            if (!isInteropObject)
            {
                // Check if the image is dirty:
                // Find out the smallest mip level needed to fit the requested size (if the image has mip levels):
                bool rcOptimizeMipLevel = calculateOptimizedMiplevel();
                GT_ASSERT(rcOptimizeMipLevel);


                // Get texture main raw data file path:
                osFilePath textureRawDataFile;
                apGLTextureMipLevelID mipLevelId = pGDData->_textureMiplevelID;
                mipLevelId._textureMipLevel = _optimizedMiplevel;

                bool rcGetFilePath = gaGetTextureMiplevelDataFilePath(pGDData->_contextId._contextId, mipLevelId, 0, textureRawDataFile);

                // YuriR: if file path is invalid set Mip Level to 0 and try again
                if (!textureRawDataFile.exists())
                {
                    retVal = updateTextureRawData();
                    rcGetFilePath = gaGetTextureMiplevelDataFilePath(pGDData->_contextId._contextId, mipLevelId, 0, textureRawDataFile);
                }
                else
                {
                    retVal = true;
                }

                if ((_optimizedMiplevel != 0) && ((!retVal) || (!textureRawDataFile.exists())))
                {
                    _optimizedMiplevel = 0;
                    mipLevelId._textureMipLevel = _optimizedMiplevel;
                    retVal = updateTextureRawData();
                    rcGetFilePath = gaGetTextureMiplevelDataFilePath(pGDData->_contextId._contextId, mipLevelId, 0, textureRawDataFile);
                }

                GT_IF_WITH_ASSERT(rcGetFilePath && retVal && textureRawDataFile.exists())
                {
                    // Localize the path as needed:
                    gaRemoteToLocalFile(textureRawDataFile, false);

                    // Try to load the cached file:
                    retVal = loadCachedThumbnailFile(textureRawDataFile);

                    // If the textures raw data is updated:
                    if (!retVal)
                    {
                        // Load the original file:
                        retVal = loadRawDataFile(textureRawDataFile);
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdTextureImageProxy::generateCachedCLImageImage
// Description: Generates an OpenCL texture image
// Return Val:  bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/12/2009
// ---------------------------------------------------------------------------
bool gdTextureImageProxy::generateCachedCLImageImage()
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pTextureTreeItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pTextureTreeItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // TO_DO: implement dirty mechanism for OpenCL textures
            bool dirtyImageExists = true;
            // bool rcDirtyCheck = gaIsOpenCLTextureImageDirty(pGDData->_contextId, (int)_textureMiplevelID._textureName, dirtyImageExists, dirtyRawDataExists);
            // GT_ASSERT(rcDirtyCheck);

            // Update texture data if necessary:
            gtVector <int> textures;
            textures.push_back(pGDData->_objectOpenCLIndex);
            bool isTextureDataUpdated = gaUpdateOpenCLImageRawData(pGDData->_contextId._contextId, textures);
            GT_IF_WITH_ASSERT(isTextureDataUpdated)
            {
                // Get texture main raw data file path
                osFilePath textureRawDataFile;
                apCLImage textureDetails;
                bool rcGetTexture = gaGetOpenCLImageObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, textureDetails);
                GT_IF_WITH_ASSERT(rcGetTexture)
                {
                    // Get the texture file path:
                    textureDetails.imageFilePath(textureRawDataFile);

                    // Localize the path as needed:
                    gaRemoteToLocalFile(textureRawDataFile, false);

                    // Try to load the cached file:
                    if (!dirtyImageExists)
                    {
                        retVal = loadCachedThumbnailFile(textureRawDataFile);
                    }

                    // If the textures raw data is updated:
                    if (!retVal)
                    {
                        // Load the original file:
                        retVal = loadRawDataFile(textureRawDataFile);
                    }
                }
            }
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdTextureImageProxy::updateTextureRawData
// Description: Update the texture raw data (either with the optimized mip
//              level or with 0 level
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/7/2010
// ---------------------------------------------------------------------------
bool gdTextureImageProxy::updateTextureRawData()
{
    bool retVal = false;
    // Sanity check:
    GT_IF_WITH_ASSERT(_pTextureTreeItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pTextureTreeItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Update and extract the texture raw data to disk:
            gtVector<apGLTextureMipLevelID> texturesVector;
            apGLTextureMipLevelID mipLevelId = pGDData->_textureMiplevelID;
            mipLevelId._textureMipLevel = _optimizedMiplevel;

            texturesVector.push_back(pGDData->_textureMiplevelID);
            // Check if the requested mip level contain dirty data:
            bool dirtyImageExists = true, dirtyRawDataExists = true;
            bool rcDirtyCheck = gaIsTextureImageDirty(pGDData->_contextId._contextId, mipLevelId, dirtyImageExists, dirtyRawDataExists);
            GT_ASSERT(rcDirtyCheck);

            if (dirtyImageExists || dirtyRawDataExists)
            {
                retVal = gaUpdateTextureRawData(pGDData->_contextId._contextId, texturesVector);
            }
            else
            {
                retVal = true;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdTextureImageProxy::loadRawDataFile
// Description: Loads a texture thumbnail file.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/7/2010
// ---------------------------------------------------------------------------
bool gdTextureImageProxy::loadRawDataFile(const osFilePath& textureFilePath)
{
    bool retVal = false;

    // Load the texture raw data:
    acRawFileHandler rawFileHandler;

    // Get the texture file page:
    bool rc5 = rawFileHandler.loadFromFile(textureFilePath);

    if (rc5)
    {
        // If raw data was loaded successfully:
        if (rawFileHandler.isOk())
        {
            if (_textureThumbnailDetails.textureType() == AP_BUFFER_TEXTURE)
            {
                // Set the raw file handler data format:
                bool rc3 = rawFileHandler.setDataFormatAndAdaptSize(_textureThumbnailDetails.bufferInternalFormat());
                GT_ASSERT(rc3);
            }

            // Set middle page to be active page
            rawFileHandler.setMiddlePageAsActivePage();

            // Convert the raw data to QImage object:
            m_pLoadedQImage = rawFileHandler.convertToQImage();

            // Return true if we got a valid image
            retVal = (m_pLoadedQImage != NULL);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdTextureImageProxy::loadCachedThumbnailFile
// Description: Loads a texture thumbnail file from cache, if it exists.
// Arguments:   const osFilePath& textureRawDataFile - the texture raw data file path
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/7/2010
// ---------------------------------------------------------------------------
bool gdTextureImageProxy::loadCachedThumbnailFile(const osFilePath& textureRawDataFilePath)
{
    bool retVal = false;

    // Create temporary file path:
    osFilePath cachedFilePath = textureRawDataFilePath;

    // Get the original file name:
    gtString origFileName;
    cachedFilePath.getFileName(origFileName);

    // The thumbnail file is saved in the same folder, with the same name,
    // with postfix _thumb:
    origFileName += GD_STR_TexturesAndBuffersLoggingThumbPostfix;
    cachedFilePath.setFileName(origFileName);
    cachedFilePath.setFileExtension(acQStringToGTString(GD_STR_TexturesAndBuffersLoggingFormatBmp));

    if (cachedFilePath.exists())
    {
        // Get the file path as string:
        gtString localFilePathStr = cachedFilePath.asString();

        // Convert the raw data to QImage object:
        m_pLoadedQImage = new QImage(acGTStringToQString(localFilePathStr));


        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdTextureImageProxy::cacheThumbnail
// Description: If the thumbnail is big, save the image to the disk, in order
//              to optimize the textures load next time.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        29/7/2010
// ---------------------------------------------------------------------------
bool gdTextureImageProxy::cacheThumbnail()
{
    bool retVal = false;

    // Sanity Check:
    GT_IF_WITH_ASSERT((m_pLoadedQImage != NULL) && (_pTextureTreeItemData != NULL))
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pTextureTreeItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Save a thumbnail only for non text messages:
            if (!m_isTextMessage)
            {
                bool rcGetFilePath = false;
                osFilePath textureRawDataFile;

                if (pGDData->_contextId.isOpenGLContext())
                {
                    // Get the texture raw data file path:
                    rcGetFilePath = gaGetTextureMiplevelDataFilePath(pGDData->_contextId._contextId, pGDData->_textureMiplevelID, 0, textureRawDataFile);
                }
                else
                {
                    // Get texture main raw data file path
                    apCLImage textureDetails;
                    rcGetFilePath = gaGetOpenCLImageObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, textureDetails);

                    if (rcGetFilePath)
                    {
                        // Get the texture file path:
                        textureDetails.imageFilePath(textureRawDataFile);
                    }
                }

                if (rcGetFilePath)
                {
                    // Localize the path as needed:
                    gaRemoteToLocalFile(textureRawDataFile, false);

                    // Get the original file name:
                    gtString origFileName;
                    textureRawDataFile.getFileName(origFileName);
                    origFileName += GD_STR_TexturesAndBuffersLoggingThumbPostfix;
                    textureRawDataFile.setFileName(origFileName);
                    textureRawDataFile.setFileExtension(acQStringToGTString(GD_STR_TexturesAndBuffersLoggingFormatBmp));

                    // Get the file path as string:
                    gtString filePathStr = textureRawDataFile.asString();

                    // Save the bitmap:
                    retVal = m_pLoadedQImage->save(acGTStringToQString(filePathStr));
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdTextureImageProxy::calculateLoadedImageSize
// Description: Calculate the loaded image size, if not calculated yet
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        1/8/2010
// ---------------------------------------------------------------------------
int gdTextureImageProxy::calculateLoadedImageSize()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pTextureTreeItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pTextureTreeItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            if (_calculatedLoadedImageSize < 0)
            {
                // Default:
                _calculatedLoadedImageSize = _imageW * _imageH * 4;

                if (pGDData->_contextId.isOpenCLContext())
                {
                    // Calculate the OpenCL texture:
                    _calculatedLoadedImageSize = calculateOpenCLLoadedImageSize();
                }
                else if (pGDData->_contextId.isOpenGLContext())
                {
                    // Calculate the OpenGL texture:
                    _calculatedLoadedImageSize = calculateOpenGLLoadedImageSize();
                }
            }
        }
    }

    return _calculatedLoadedImageSize;
}

// ---------------------------------------------------------------------------
// Name:        gdTextureImageProxy::calculateOpenGLLoadedImageSize
// Description: Get the loaded item size
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/8/2010
// ---------------------------------------------------------------------------
int gdTextureImageProxy::calculateOpenGLLoadedImageSize()
{
    int retVal = 0;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pTextureTreeItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pTextureTreeItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Get the selected texture details:
            bool rc = gaGetTextureObjectThumbnailData(pGDData->_contextId._contextId, pGDData->_textureMiplevelID._textureName, _textureThumbnailDetails);
            GT_IF_WITH_ASSERT(rc)
            {
                // Get the texture texel type:
                oaTexelDataFormat dataFormat = _textureThumbnailDetails.texelDataFormat();
                oaDataType dataType = _textureThumbnailDetails.dataType();

                // Get the texture pixel size:
                int pixelSize = oaCalculatePixelUnitByteSize(dataFormat, dataType);
                GT_IF_WITH_ASSERT(pixelSize > 0)
                {
                    // Get the texture size:
                    GLsizei textureW, textureH, textureD, textureBorderSize;
                    _textureThumbnailDetails.getDimensions(textureW, textureH, textureD, textureBorderSize);
                    retVal = textureW * textureH * pixelSize;

                    if (m_isTextMessage)
                    {
                        retVal = _imageH * _imageW * 4;
                    }
                    else
                    {
                        // Check if the requested mip level contain dirty data:
                        bool dirtyImageExists = true, dirtyRawDataExists = true;
                        bool rcDirtyCheck = gaIsTextureImageDirty(pGDData->_contextId._contextId, pGDData->_textureMiplevelID, dirtyImageExists, dirtyRawDataExists);
                        GT_ASSERT(rcDirtyCheck);

                        // Get the texture size:
                        if (dirtyRawDataExists)
                        {
                            // Get the texture mip level data:
                            rc = gaGetTextureObjectThumbnailData(pGDData->_contextId._contextId, pGDData->_textureMiplevelID._textureName, _textureThumbnailDetails);
                            GT_IF_WITH_ASSERT(rc)
                            {
                                // Check if the texture has mip levels:
                                if (_textureThumbnailDetails.maxLevel() > _textureThumbnailDetails.minLevel())
                                {
                                    // Browse the levels, from the smallest sized to the biggest, and search for the best:
                                    for (int i = _textureThumbnailDetails.maxLevel(); i >= _textureThumbnailDetails.minLevel(); i--)
                                    {
                                        // Get the level size division:
                                        int levelExp = (int)pow(2.0f, (float)i);
                                        GLsizei texW = 0, texH = 0, textureDepth = 0, borderSize = 0;
                                        _textureThumbnailDetails.getDimensions(texW, texH, textureDepth, borderSize);
                                        int miplevelW = texW / levelExp;
                                        int miplevelH = texH / levelExp;

                                        // Check if this mip level's size is enough:
                                        if ((miplevelH >= _imageH) && (miplevelW >= _imageH))
                                        {
                                            _optimizedMiplevel = i;
                                            retVal = miplevelW * miplevelH * pixelSize;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            // Get texture main raw data file path:
                            osFilePath textureRawDataFile;
                            bool rcGetFilePath = gaGetTextureMiplevelDataFilePath(pGDData->_contextId._contextId, pGDData->_textureMiplevelID, 0, textureRawDataFile);
                            GT_IF_WITH_ASSERT(rcGetFilePath)
                            {
                                // Localize the path as needed:
                                gaRemoteToLocalFile(textureRawDataFile, false);

                                // Get the original file name:
                                gtString origFileName;
                                textureRawDataFile.getFileName(origFileName);
                                origFileName += GD_STR_TexturesAndBuffersLoggingThumbPostfix;
                                textureRawDataFile.setFileName(origFileName);
                                textureRawDataFile.setFileExtension(acQStringToGTString(GD_STR_TexturesAndBuffersLoggingFormatBmp));

                                // Check if the file exists:
                                if (textureRawDataFile.exists())
                                {
                                    retVal = _imageH * _imageW * pixelSize;
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
// Name:        gdTextureImageProxy::calculateOpenCLLoadedImageSize
// Description: Get the loaded item size
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/8/2010
// ---------------------------------------------------------------------------
int gdTextureImageProxy::calculateOpenCLLoadedImageSize()
{
    int retVal = 0;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pTextureTreeItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pTextureTreeItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Get texture main raw data file path
            osFilePath textureRawDataFile;
            apCLImage textureDetails;
            bool rcGetTexture = gaGetOpenCLImageObjectDetails(pGDData->_contextId._contextId, pGDData->_objectOpenCLIndex, textureDetails);
            GT_IF_WITH_ASSERT(rcGetTexture)
            {
                if (m_isTextMessage)
                {
                    retVal = _imageH * _imageW * 4;
                }
                else
                {
                    // Get the texture texel type:
                    cl_uint clDataFormat = textureDetails.dataFormat();
                    cl_uint clDataType = textureDetails.dataType();

                    // Translates cl image format to its equivalent oaTexelDataFormat:
                    oaTexelDataFormat dataFormat;
                    bool rc1 = oaCLImageFormatToTexelFormat(clDataFormat, dataFormat);
                    GT_ASSERT(rc1);

                    oaDataType dataType;
                    bool rc2 = oaCLImageDataTypeToOSDataType(clDataType, dataType);
                    GT_ASSERT(rc2);

                    // Get the texture pixel size:
                    int pixelSize = oaCalculatePixelUnitByteSize(dataFormat, dataType);

                    // Get the texture size:
                    size_t textureW, textureH, textureD;
                    textureDetails.getDimensions(textureW, textureH, textureD);
                    retVal = textureW * textureH * pixelSize;

                    // Get the texture file path:
                    osFilePath texRawDataFile;
                    textureDetails.imageFilePath(textureRawDataFile);

                    // Localize the path as needed:
                    gaRemoteToLocalFile(texRawDataFile, false);

                    // Get the original file name:
                    gtString origFileName;
                    texRawDataFile.getFileName(origFileName);
                    origFileName += GD_STR_TexturesAndBuffersLoggingThumbPostfix;
                    texRawDataFile.setFileName(origFileName);
                    texRawDataFile.setFileExtension(acQStringToGTString(GD_STR_TexturesAndBuffersLoggingFormatBmp));

                    // Check if the file exists:
                    if (textureRawDataFile.exists())
                    {
                        retVal = _imageH * _imageW * pixelSize;
                    }
                }
            }
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdTextureImageProxy::tooltipText
// Description: Builds the texture tooltip text
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        23/8/2010
// ---------------------------------------------------------------------------
void gdTextureImageProxy::buildTooltipText()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pTextureTreeItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pTextureTreeItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            // Build the texture tooltip:
            m_tooltipText.makeEmpty();

            // Build the texture name (ignore CL interoperability):
            gdHTMLProperties htmlBuilder;
            htmlBuilder.getGLTextureName(pGDData->_textureMiplevelID, -1, -1, m_tooltipText, false);

            // Get the texture min / max levels:
            int minLevel = _textureThumbnailDetails.minLevel();
            int maxLevel = _textureThumbnailDetails.maxLevel();

            bool resetValues = false;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();
            apProjectType CodeXLProjectType = globalVarsManager.CodeXLProjectType();

            // The iPhone OpenGL ES implementation doesn't give us texture level information, so we suffice in the
            // auto generated value (min = 0 max = log2(width)).
            resetValues = resetValues && (!apDoesProjectTypeSupportOpenGLES(CodeXLProjectType));
#endif

            if (resetValues)
            {
                minLevel = 0;
                maxLevel = 1000;
            }

            // Calculate the amount if texture levels:
            int amountOfTextureLevels = maxLevel - minLevel + 1;

            if (amountOfTextureLevels > 1)
            {
                m_tooltipText.appendFormattedString(GD_STR_ObjectTooltipMiplevels, amountOfTextureLevels);
            }
            else
            {
                m_tooltipText.appendFormattedString(GD_STR_ObjectTooltipMiplevel, amountOfTextureLevels);
            }

            m_tooltipText.append(AF_STR_NewLine);

            // Build the requested internal pixel format string:
            gtString strTextureRequestedInternalFormat;
            GLenum textureRequestedInternalFormat = _textureThumbnailDetails.requestedInternalPixelFormat();
            apGLPixelInternalFormatParameter internalRequestedPixelFormat;
            internalRequestedPixelFormat.setValueFromInt(textureRequestedInternalFormat);
            internalRequestedPixelFormat.valueAsString(strTextureRequestedInternalFormat);

            // Add the pixel format:
            m_tooltipText.appendFormattedString(GD_STR_ObjectTooltipRequestedInternalFormat, strTextureRequestedInternalFormat.asCharArray());
            m_tooltipText.append(AF_STR_NewLine);

            // Add the texel data format to the tooltip:
            if (_textureThumbnailDetails.texelDataFormat() != OA_TEXEL_FORMAT_UNKNOWN)
            {
                gtString texelFormatStr;
                oaGetTexelDataFormatName(_textureThumbnailDetails.texelDataFormat(), texelFormatStr);
                m_tooltipText.appendFormattedString(GD_STR_ObjectTooltipTexelFormat, texelFormatStr.asCharArray());
                m_tooltipText.append(AF_STR_NewLine);
            }

            // Add the pixel data type to the tooltip:
            if (_textureThumbnailDetails.dataType() != OA_UNKNOWN_DATA_TYPE)
            {
                gtString dataTypeStr;
                GLenum glDataType = oaDataTypeToGLEnum(_textureThumbnailDetails.dataType());
                apGLenumParameter dataTypeParam(glDataType);
                dataTypeParam.valueAsString(dataTypeStr);
                m_tooltipText.appendFormattedString(GD_STR_ObjectTooltipDataType, dataTypeStr.asCharArray());
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdTextureImageProxy::getDebugString
// Description: Used for debugging
// Return Val:  gtString
// Author:      Sigal Algranaty
// Date:        10/4/2011
// ---------------------------------------------------------------------------
gtString gdTextureImageProxy::getDebugString()
{
    gtString dbg;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pTextureTreeItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pTextureTreeItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != NULL)
        {
            dbg.appendFormattedString(L" %d ", pGDData->_textureMiplevelID._textureName);
        }
    }
    return dbg;
}




