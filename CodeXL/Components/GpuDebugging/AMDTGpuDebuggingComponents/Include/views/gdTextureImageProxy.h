//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdTextureImageProxy.h
///
//==================================================================================

//------------------------------ gdTextureImageProxy.h ------------------------------

#ifndef __GDTEXTUREIMAGEPROXY
#define __GDTEXTUREIMAGEPROXY

// Infra:
#include <AMDTApplicationComponents/Include/acImageDataProxy.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>


// ----------------------------------------------------------------------------------
// Class Name:           gdTextureImageProxy
// General Description:  An image proxy to retrieve a texture image
// Author:               Eran Zinman
// Creation Date:        31/12/2007
// ----------------------------------------------------------------------------------
class gdTextureImageProxy : public acImageDataProxy
{
public:
    // Constructor:
    gdTextureImageProxy(afApplicationTreeItemData* pTextureTreeItemData, bool isInGLBeginEndBlock, int imageW, int imageH, bool releaseItemDataMemory = false);

    // Destructor:
    virtual ~gdTextureImageProxy();

    // TO_DO: thumbnails optimization: Needed for debugging. Leaving until virtual list is done.
    virtual gtString getDebugString();

public:
    // Image generation function:
    virtual bool loadImage();
    virtual bool cacheThumbnail();
    virtual int calculateLoadedImageSize();

    // Tooltip:
    virtual void buildTooltipText();

    // Find the optimized mip level:
    int getOptimizedMiplevel() {return _optimizedMiplevel;};

protected:

    // Generates the texture image:
    bool loadRawDataFile(const osFilePath& textureFilePath);
    bool loadCachedThumbnailFile(const osFilePath& textureFilePath);

    bool updateTextureRawData();
    bool generateTextureImage();
    bool generateCachedGLTextureImage();
    bool generateCachedCLImageImage();

    // Find the optimized mip level:
    bool calculateOptimizedMiplevel();

    int calculateOpenGLLoadedImageSize();
    int calculateOpenCLLoadedImageSize();

protected:

    // Tree item data for the displayed texture:
    afApplicationTreeItemData* _pTextureTreeItemData;

    // Contain true iff _pTextureTreeItemData memory should be release by me:
    bool _releaseItemDataMemory;

    // The texture object details:
    apGLTextureMiplevelData _textureThumbnailDetails;

    // Are we in GL begin end block:
    bool _isInGLBeginEndBlock;

    // The image requested thumbnail size:
    int _imageW;
    int _imageH;

    int _optimizedMiplevel;

    // Calculate loaded image size:
    int _calculatedLoadedImageSize;
};


#endif  // __GDTEXTUREIMAGEPROXY

