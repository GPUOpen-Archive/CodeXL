//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acImageDataProxy.h
///
//==================================================================================

//------------------------------ acImageDataProxy.h ------------------------------

#ifndef __ACIMAGEDATAPROXY
#define __ACIMAGEDATAPROXY

// Qt:
#include <QImage>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

// Defines the default message texture width and height
#define GD_IMAGE_PROXY_DEFAULT_MESSAGE_TEXTURE_SIZE 120

// ----------------------------------------------------------------------------------
// Class Name:           acImageDataProxy
// General Description:  This is a base class for generating image data.
//                       This class is being used by acImageItem to retrieve
//                       image content without relying on the image source.
//
//                       The output image with bill in QImage format,
//                       with an internal format of 32Bit RGBA.
// Author:               Eran Zinman
// Creation Date:        31/12/2007
// ----------------------------------------------------------------------------------
class AC_API acImageDataProxy
{
public:
    // Constructor:
    acImageDataProxy();

    // Destructor
    virtual ~acImageDataProxy() {};

public:
    // Image generation function
    virtual bool loadImage() = 0;
    virtual QImage* getImage() const { return m_pLoadedQImage; };
    virtual bool cacheThumbnail() {return true;};
    virtual int calculateLoadedImageSize() {return 0;};
    virtual bool shouldImageBeCached();

    // Object tooltip:
    const gtString& tooltipText() {return m_tooltipText;};
    virtual void buildTooltipText() = 0;

    QImage* createThumnbailImage(int thumbWidth, int thumbHeight, bool withBG = false);
    virtual void releaseLoadedImage();
    void releaseLoadedImageOwnership() {m_pLoadedQImage = NULL;};

    bool isTextMessage() const {return m_isTextMessage;}

    // TO_DO: thumbnails optimization: For debugging. Remove me once debugging is done.
    virtual gtString getDebugString() {return L"";};

protected:
    // Generates a "message" image
    QImage* createMessageImage(const gtString& message, int imageWidth = GD_IMAGE_PROXY_DEFAULT_MESSAGE_TEXTURE_SIZE, int imageHeight = GD_IMAGE_PROXY_DEFAULT_MESSAGE_TEXTURE_SIZE);


protected:
    // Contains the loaded image:
    QImage* m_pLoadedQImage;

    // Contain true iff the image data should be released:
    bool m_shouldReleaseImageData;

    // Contain true iff the image is a text message:
    bool m_isTextMessage;

    // Contain the image item tooltip text:
    gtString m_tooltipText;



};

#endif  // __ACIMAGEDATAPROXY
