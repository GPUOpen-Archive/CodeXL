//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acImageDataProxy.cpp
///
//==================================================================================

//------------------------------ acImageDataProxy.cpp ------------------------------

// Qt:
#include <QPainter>
#include <QStaticText>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>

// Local:
#include <AMDTApplicationComponents/Include/acImageDataProxy.h>
#include <AMDTApplicationComponents/Include/acImageItem.h>
#include <AMDTApplicationComponents/Include/acImageManagerDefinitions.h>
#include <AMDTApplicationComponents/Include/res/icons/image_background_pattern.xpm>



// ------------------------------------------ acImageDataProxy ------------------------------------------ //


#define AC_MIN_TEXTURE_CACHE_SIZE (128*128*32)



// ---------------------------------------------------------------------------
// Name:        acImageDataProxy::acImageDataProxy
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        29/7/2012
// ---------------------------------------------------------------------------
acImageDataProxy::acImageDataProxy() : m_pLoadedQImage(NULL), m_shouldReleaseImageData(false), m_isTextMessage(false), m_tooltipText(L"")
{

}


// ---------------------------------------------------------------------------
// Name:        acImageDataProxy::createMessageImage
// Description: Generates a "message" image which consist of a white
//              background a the message written in red.
// Arguments:   message - Message to write on image bitmap
//              imageWidth, imageHeight - Output image dimensions
// Return Val:  QImage image if successful, NULL otherwise
// Author:      Eran Zinman
// Date:        24/1/2008
// ---------------------------------------------------------------------------
QImage* acImageDataProxy::createMessageImage(const gtString& message, int imageWidth, int imageHeight)
{
    // Define the rect where text will be written (which is the image size minus extra 2 pixels)
    QRect labelRect(2, 2, imageWidth - 2, imageHeight - 2);

    // Create the message bitmap
    QImage* pRetVal = new QImage(QSize(imageWidth, imageHeight), QImage::Format_ARGB32);


    QPainter painter(pRetVal);
    QFont font = painter.font();
    font.setPointSize(10);
    font.setBold(true);
    painter.setFont(font);

    QRect imageRect(0, 0, imageWidth, imageHeight);
    painter.fillRect(imageRect, Qt::white);

    QTextOption option;
    option.setAlignment(Qt::AlignCenter);

    QColor color = QColor::fromRgb(255, 0, 0);
    color.setAlpha(255);
    painter.setPen(color);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.drawText(labelRect, message.asASCIICharArray(), option);

    // Mark this image data as a text message:
    m_isTextMessage = true;

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageDataProxy::releaseLoadedImage
// Description: Releases a loaded image proxy
// Author:      Yaki Tebeka
// Date:        14/1/2008
// ---------------------------------------------------------------------------
void acImageDataProxy::releaseLoadedImage()
{
    /* if (m_pLoadedQImage != NULL)
     {
         if (m_shouldReleaseImageData)
         {
             uchar* pImageData = m_pLoadedQImage->bits();
             if (pImageData != NULL)
             {
                 delete [] pImageData;
             }

             delete m_pLoadedQImage;
             m_pLoadedQImage = NULL;
         }
     }*/
}

// ---------------------------------------------------------------------------
// Name:        acImageDataProxy::createThumnbailImage
// Description: Create a thumbnail.
// Arguments:   int thumbWidth
//              int thumbHeight
// Return Val:  QImage*
// Author:      Sigal Algranaty
// Date:        29/7/2010
// ---------------------------------------------------------------------------
QImage* acImageDataProxy::createThumnbailImage(int thumbWidth, int thumbHeight, bool withBG)
{
    // If the loaded image is initialized:
    bool isImageLoaded = true;

    if (m_pLoadedQImage == NULL)
    {
        isImageLoaded = loadImage();
    }

    // Only if the image is already loaded:
    GT_IF_WITH_ASSERT(isImageLoaded && (m_pLoadedQImage != NULL))
    {
        // Check if the image should be cached:
        bool shouldCache = shouldImageBeCached();

        // Create the scaled image:
        QImage zoomedImage = m_pLoadedQImage->scaledToWidth(thumbWidth - 2);

        // Get the scaled image data:
        uchar* pZoomedImageData = zoomedImage.bits();
        GT_IF_WITH_ASSERT(pZoomedImageData != NULL)
        {
            int zoomedImageSize = zoomedImage.byteCount();
            uchar* pData = new uchar[zoomedImageSize];


            // Copy the data to the new pointer:
            memcpy(pData, pZoomedImageData, zoomedImageSize);

            // Create the displayed image with the zoomed dimensions:
            QImage* pThumbnailImage = new QImage(pData, zoomedImage.width(), zoomedImage.height(), zoomedImage.width() * 4, QImage::Format_ARGB32);


            // Unload original image
            releaseLoadedImage();

            if (withBG)
            {
                // Create the new image (leave 2 pixels for border):
                m_pLoadedQImage = new QImage(QSize(thumbWidth, thumbHeight), QImage::Format_ARGB32);


                // Define a painter for the image:
                QPainter painter(m_pLoadedQImage);

                // Get the system default background color:
                QPixmap bgPixmap(image_background_pattern);
                QBrush bgBrush(Qt::white, bgPixmap);


                QRect imageBGRect(QPoint(0, 0), m_pLoadedQImage->size());
                painter.fillRect(imageBGRect, Qt::white);
                painter.fillRect(imageBGRect, bgBrush);

                // Draw the image:
                painter.drawImage(imageBGRect, *pThumbnailImage);
                m_shouldReleaseImageData = false;
            }
            else
            {
                // Set the free image bitmap to be the thumbnail bitmap
                m_pLoadedQImage = pThumbnailImage;
                m_shouldReleaseImageData = true;
            }

            if (shouldCache)
            {
                // Save thumbnail file if the image is big:
                bool rcCacheThumb = cacheThumbnail();
                GT_ASSERT(rcCacheThumb);
            }
        }
    }

    return m_pLoadedQImage;
}

// ---------------------------------------------------------------------------
// Name:        acImageDataProxy::shouldImageBeCached
// Description: Checks if the image size is big enough for cache.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        29/7/2010
// ---------------------------------------------------------------------------
bool acImageDataProxy::shouldImageBeCached()
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(m_pLoadedQImage != NULL)
    {
        // Calculate the image size:
        gtUInt64 imageSize = m_pLoadedQImage->byteCount();

        if (imageSize > AC_MIN_TEXTURE_CACHE_SIZE)
        {
            retVal = true;
        }
    }
    return retVal;
}
