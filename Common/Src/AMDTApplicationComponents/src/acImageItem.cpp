//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acImageItem.cpp
///
//==================================================================================

//------------------------------ acImageItem.cpp ------------------------------

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>

// Local:
#include <AMDTApplicationComponents/Include/acImageItem.h>
#include <inc/acStringConstants.h>

// Defines the image border colour
#define AC_IMAGE_ITEM_IMAGE_BORDER_COLOR Qt::black;


// ---------------------------------------------------------------------------
// Name:        acImageItem::acImageItem
// Description: Constructor. Creates an image holder (empty), and resets
//              the image settings
// Arguments:   itemId - The item id
//              position - The item position (in pixels)
//              thumbSize - Indicates if image is thumbnail or not (if it is
//              it will be in the size of thumbSize)
// Author:      Eran Zinman
// Date:        19/5/2007
// ---------------------------------------------------------------------------
acImageItem::acImageItem(acImageItemID imageItemID, unsigned int imageActionsEnable, int thumbSize) :
    m_pOriginalImageFromProxy(NULL), m_pCurrentDisplayedImage(NULL), m_itemID(imageItemID), m_thumbSize(thumbSize),
    m_isImageLoaded(false), m_backgroundBrush(Qt::white), m_isDisplayingText(false), m_displayedTextHeadline(L""), m_displayedTextMessage(L""),
    m_displayedTextColor(Qt::black), m_isEmpty(false), m_imageBoundingRect(0, 0, 0, 0), m_imageAvailableBoundingRect(0, 0, 0, 0),
    m_pImageDataProxy(NULL), m_shouldUpdateImage(false), m_zoomedImageSize(0, 0), m_originalImageSize(0, 0), m_zoomLevel(100),
    m_rotationAngle(0), _imageActionEnabled(imageActionsEnable), _imageActionChecked(0),
    _topLabel(L""), _bottomLabel(L""), _toolTipText(L""), _pItemData(NULL)
{
    // Initialize the channels enable:
    _imageActionEnabled = imageActionsEnable;

    // Initialize the actions init state:
    _imageActionChecked = AC_IMAGE_CHANNEL_RED | AC_IMAGE_CHANNEL_GREEN | AC_IMAGE_CHANNEL_BLUE | AC_IMAGE_CHANNEL_ALPHA;
}

// ---------------------------------------------------------------------------
// Name:        acImageItem::~acImageItem
// Description: Destructor
// Author:      Eran Zinman
// Date:        19/5/2007
// ---------------------------------------------------------------------------
acImageItem::~acImageItem()
{
    // Release the memory:
    releaseImageMemory(m_pOriginalImageFromProxy);
    releaseImageMemory(m_pCurrentDisplayedImage);
}

// ---------------------------------------------------------------------------
// Name:        acImageItem::isActionChecked
// Description: Return True / False is the image action is applied.
// Arguments:   acImageItemAction action
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/4/2010
// ---------------------------------------------------------------------------
bool acImageItem::isActionChecked(acImageItemAction action)
{
    bool retVal = false;

    if (_imageActionChecked & action)
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageItem::loadDataFromImageProxy
// Description: Loads the data from the image proxy
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        19/5/2007
// ---------------------------------------------------------------------------
bool acImageItem::loadDataFromImageProxy()
{
    bool retVal = false;

    // Sanity check:
    if (m_pImageDataProxy)
    {
        // Flag the image was loaded
        m_isImageLoaded = true;

        // If original bitmap exists - unload it
        if (m_pOriginalImageFromProxy != NULL)
        {
            if (m_pImageDataProxy != NULL)
            {
                if (m_pImageDataProxy->getImage() == m_pOriginalImageFromProxy)
                {
                    m_pImageDataProxy->releaseLoadedImageOwnership();
                }
            }

            releaseImageMemory(m_pOriginalImageFromProxy);
        }

        // Initialize the free image object
        bool rc1 = loadImage();
        GT_IF_WITH_ASSERT(rc1)
        {
            // Update the image with the new bitmap
            retVal = updateDisplayedImage();
        }

    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageItem::generateImageSnapshot
// Description: Generates an image snapshot of what currently seen on
//              screen (WYSIWYG) in QImage format
// Return Val:  QImage bitmap of the currently seen image if successful,
//              NULL if saving failed
// Author:      Eran Zinman
// Date:        5/1/2008
// ---------------------------------------------------------------------------
QImage* acImageItem::generateImageSnapshot()
{
    QImage* pRetVal = NULL;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pCurrentDisplayedImage != NULL)
    {
        // Generate a copy of the image:
        pRetVal = new QImage(*m_pCurrentDisplayedImage);

    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageItem::setImageDataProxy
// Description: Sets the image data proxy which gives us the image data
// Arguments:   pImageDataProxy - The image data proxy service
//              loadOnDemand - Should we load the file now, or only
//              when needed?
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        19/5/2007
// ---------------------------------------------------------------------------
bool acImageItem::setImageDataProxy(acImageDataProxy* pImageDataProxy, bool loadOnDemand)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pImageDataProxy != NULL)
    {
        retVal = true;

        // Save the image proxy data
        m_pImageDataProxy = pImageDataProxy;

        // Should we load the item now?
        if (!loadOnDemand)
        {
            retVal = loadDataFromImageProxy();
            GT_IF_WITH_ASSERT(retVal)
            {
                // Flag internally that image was loaded successfully
                m_isImageLoaded = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageItem::replaceBitmap
// Description: Replaces the bitmap with a new bitmap
// Arguments:   dib - The new bitmap
//              keepCurrentDimensions - Should we keep current dimensions?
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        9/11/2007
// ---------------------------------------------------------------------------
bool acImageItem::replaceBitmap(QImage* pImage, bool keepCurrentDimensions)
{
    bool retVal = false;

    // If new image is valid
    GT_IF_WITH_ASSERT(pImage != NULL)
    {
        // Unload original bitmap (if exists)
        if (m_pOriginalImageFromProxy != NULL)
        {
            if (m_pImageDataProxy != NULL)
            {
                if (m_pOriginalImageFromProxy == m_pImageDataProxy->getImage())
                {
                    m_pImageDataProxy->releaseLoadedImageOwnership();
                }
            }

            releaseImageMemory(m_pOriginalImageFromProxy);
            releaseImageMemory(m_pCurrentDisplayedImage);
        }

        // Set the bitmap to be the new bitmap
        m_pOriginalImageFromProxy = pImage;
        m_originalImageSize = m_pOriginalImageFromProxy->size();

        // Initialize the free image object
        bool rc1 = loadImage(keepCurrentDimensions);
        GT_IF_WITH_ASSERT(rc1)
        {
            // Update the displayed image
            bool rc2 = updateDisplayedImage();
            GT_IF_WITH_ASSERT(rc2)
            {
                // Update the rotation:
                int oldRotationAngle = m_rotationAngle;
                m_rotationAngle = 0;
                bool rc3 = true;

                if (oldRotationAngle != 0)
                {
                    rc3 = rotateImageByAngle(oldRotationAngle);
                }

                GT_IF_WITH_ASSERT(rc3)
                {
                    // Flag that image is loaded
                    m_isImageLoaded = true;

                    retVal = true;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageItem::loadImage
// Description: Init the currently displayed image object
// Arguments:   bool keepCurrentDimensions
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/7/2012
// ---------------------------------------------------------------------------
bool acImageItem::loadImage(bool keepCurrentDimensions)
{
    bool retVal = false;

    if (isThumbnail())
    {
        // Should get here only once and have proxy:
        GT_IF_WITH_ASSERT((m_pOriginalImageFromProxy == NULL) && (m_pImageDataProxy != NULL))
        {
            // Load the new image
            bool rc1 = m_pImageDataProxy->loadImage();

            if (rc1)
            {
                // Get the image as thumbnail:
                m_pOriginalImageFromProxy = m_pImageDataProxy->createThumnbailImage(m_thumbSize, m_thumbSize);
                m_originalImageSize = m_pOriginalImageFromProxy->size();

                // Leave this part out of the previous GT_IF_WITH_ASSERT block, since we want the
                // image item tooltip also for unloaded images:
                m_pImageDataProxy->buildTooltipText();
            }

            if (m_pImageDataProxy->isTextMessage())
            {
                // Reset image actions for text images:
                _imageActionEnabled = 0;
            }
        }
    }

    GT_IF_WITH_ASSERT(m_pOriginalImageFromProxy != NULL)
    {
        // Set the original image size attributes
        m_originalImageSize = m_pOriginalImageFromProxy->size();

        // Unless we should keep our current dimensions, show 100%
        if (!keepCurrentDimensions)
        {
            m_zoomedImageSize = m_originalImageSize;
        }

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageItem::applyChannelsFilter
// Description: Apply channel filters on specific channel, since this function
//              is called on every pixel, make it inline
// Arguments:   channelValue - Process the channels according to the active filters
// Author:      Eran Zinman
// Date:        16/6/2007
// ---------------------------------------------------------------------------
void acImageItem::applyChannelsFilter(QRgb& color) const
{
    // If grayscale enabled
    if ((_imageActionChecked & AC_IMAGE_GRAYSCALE_FILTER) && (_imageActionEnabled & AC_IMAGE_GRAYSCALE_FILTER))
    {
        // Calculate colors average
        int averageValue = (qRed(color) + qGreen(color) + qBlue(color)) / 3;

        if (!(_imageActionChecked & AC_IMAGE_CHANNEL_ALPHA))
        {
            // Set full alpha value:
            color = qRgba(averageValue, averageValue, averageValue, 255);
        }
        else
        {
            // Set the average values to all channels
            color = qRgba(averageValue, averageValue, averageValue, qAlpha(color));
        }
    }

    // If invert enabled
    else if ((_imageActionChecked & AC_IMAGE_INVERT_FILTER) && (_imageActionEnabled & AC_IMAGE_INVERT_FILTER))
    {
        // Set the invert values to all channels:
        color = qRgb(255 - qRed(color), 255 - qGreen(color), 255 - qBlue(color));

        // If "Alpha" channel is inactive
        if (!(_imageActionChecked & AC_IMAGE_CHANNEL_ALPHA))
        {
            // Set full alpha value
            color = qRgba(255 - qRed(color), 255 - qGreen(color), 255 - qBlue(color), 255);
        }
    }

    // Normal filter, check if channels are Enabled / Disabled
    else
    {
        // If "Red" channel is inactive
        if ((!(_imageActionChecked & AC_IMAGE_CHANNEL_RED)) && (_imageActionEnabled & AC_IMAGE_CHANNEL_RED))
        {
            // Set "Red" channel to be zero:
            color = qRgba(0, qGreen(color), qBlue(color), qAlpha(color));
        }

        // If "Green" channel is inactive
        if ((!(_imageActionChecked & AC_IMAGE_CHANNEL_GREEN)) && (_imageActionEnabled & AC_IMAGE_CHANNEL_GREEN))
        {
            // Set "Green" channel to be zero:
            color = qRgba(qRed(color), 0, qBlue(color), qAlpha(color));

        }

        // If "Blue" channel is inactive
        if ((!(_imageActionChecked & AC_IMAGE_CHANNEL_BLUE)) && (_imageActionEnabled & AC_IMAGE_CHANNEL_BLUE))
        {
            // Set "Blue" channel to be zero:
            color = qRgba(qRed(color), qGreen(color), 0, qAlpha(color));
        }

        // If "Alpha" channel is inactive
        if ((!(_imageActionChecked & AC_IMAGE_CHANNEL_ALPHA)) && (_imageActionEnabled & AC_IMAGE_CHANNEL_ALPHA))
        {
            // Set full alpha value:
            color = qRgba(qRed(color), qGreen(color), qBlue(color), 255);
        }
    }
}



// ---------------------------------------------------------------------------
// Name:        acImageItem::updateDisplayedImage
// Description: Create an image with the currently displayed parameters
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/7/2012
// ---------------------------------------------------------------------------
bool acImageItem::updateDisplayedImage()
{
    bool retVal = false;

    releaseImageMemory(m_pCurrentDisplayedImage);

    GT_IF_WITH_ASSERT(m_pOriginalImageFromProxy != NULL)
    {
        QImage originalImage = m_pOriginalImageFromProxy->scaled(m_originalImageSize);
        QTransform tRotate;

        if (m_rotationAngle != 0)
        {
            tRotate.rotate(m_rotationAngle);
        }

        QImage RotatedImage = originalImage.transformed(tRotate);
        int nZoomLevel = zoomLevel();
        QImage TransformedImage = RotatedImage;

        if (nZoomLevel != 100)
        {
            QSize qZoomed(double(RotatedImage.width() * double(nZoomLevel) / 100.0), double(RotatedImage.height()*double(nZoomLevel) / 100.0));
            TransformedImage = RotatedImage.scaled(qZoomed);
        }

        // Get the data for transformed image:
        const uchar* pTransformedImageBits = TransformedImage.bits();
        GT_IF_WITH_ASSERT(pTransformedImageBits != NULL)
        {
            int nTransformedImageSize = TransformedImage.byteCount();
            uchar* pData = new uchar[nTransformedImageSize];
            // Copy the data to the new pointer:
            memcpy(pData, pTransformedImageBits, nTransformedImageSize);
            // Create the displayed image with the zoomed dimensions:
            m_pCurrentDisplayedImage = new QImage(pData, TransformedImage.width(), TransformedImage.height(), TransformedImage.width() * 4, QImage::Format_ARGB32);
            // Get the created image width:
            int imageWidth = m_pCurrentDisplayedImage->width();
            int imageHeight = m_pCurrentDisplayedImage->height();

            // Iterate the image pixels and apply filters:
            for (int y = 0; y < imageHeight; y++)
            {
                // Get the current line:
                QRgb* pLineData = (QRgb*)m_pCurrentDisplayedImage->scanLine(y);

                for (int x = 0; x < imageWidth; x++)
                {
                    QRgb currentPixel = pLineData[x];
                    // Process the pixel data according to the active filters
                    applyChannelsFilter(currentPixel);
                    m_pCurrentDisplayedImage->setPixel(QPoint(x, y), currentPixel);
                }
            }
        }
        retVal = true;

        if (NULL != m_pCurrentDisplayedImage)
        {
            m_zoomedImageSize = m_pCurrentDisplayedImage->size();
        }

    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageItem::setImageRotationAngle
// Description: Sets the image rotation angle
// Arguments:   rotateAngle - The angle to rotate the image
// Author:      Eran Zinman
// Date:        17/7/2007
// ---------------------------------------------------------------------------
bool acImageItem::rotateImageByAngle(int rotationAngle)
{
    bool bRet = false;

    if (0 != rotationAngle)
    {
        // Save the new rotation angle
        m_rotationAngle += rotationAngle;
        // Stay in the 360deg circle
        m_rotationAngle %= 360;

        GT_IF_WITH_ASSERT(m_pOriginalImageFromProxy != NULL)
        {
            // Perform a the rotation on the original image:
            QTransform rotation;
            rotation.rotate(m_rotationAngle);
            QImage rotatedImage = m_pOriginalImageFromProxy->transformed(rotation);
            // Get the data for the zoomed image:
            const uchar* pRotatedImageBits = rotatedImage.bits();
            GT_IF_WITH_ASSERT(pRotatedImageBits != NULL)
            {
                releaseImageMemory(m_pCurrentDisplayedImage);
                int rotatedImageSize = rotatedImage.byteCount();
                uchar* pData = new uchar[rotatedImageSize];
                // Copy the data to the new pointer:
                memcpy(pData, pRotatedImageBits, rotatedImageSize);
                // Create the displayed image with the zoomed dimensions:
                m_pCurrentDisplayedImage = new QImage(pData, rotatedImage.width(), rotatedImage.height(), rotatedImage.width() * 4, QImage::Format_ARGB32);
                // Update the properties
                m_originalImageSize = m_pOriginalImageFromProxy->size();
            }
            // Recalculate the zoomed image size
            setImageZoomLevel(m_zoomLevel);
            m_shouldUpdateImage = true; // force update to use current zoom after rotation
            bRet = true;
        }
    }
    else // 0 == rotationAngle
    {
        bRet = true;
    }

    return bRet;
}

// ---------------------------------------------------------------------------
// Name:        acImageItem::setImageZoomLevel
// Description: Sets the image zoom level
// Arguments:   zoomLevel - The image zoom level
// Author:      Eran Zinman
// Date:        30/6/2007
// ---------------------------------------------------------------------------
void acImageItem::setImageZoomLevel(int zoomLevel)
{
    if ((m_zoomLevel != zoomLevel) && (zoomLevel > 0))
    {
        // Next time image is required, update the image:
        m_shouldUpdateImage = true;

        // Save the new zoom level
        m_zoomLevel = zoomLevel;

        // Get original image width and height (zoom is relative to original image)
        int imageWidth = m_originalImageSize.width();
        int imageHeight = m_originalImageSize.height();

        // Set width and height multiplier
        double sizeMultiplier = (double) zoomLevel / (double) 100.0;

        // Determine the image new width and height
        int imageNewWidth = int((double) imageWidth * sizeMultiplier);
        int imageNewHeight = int((double) imageHeight * sizeMultiplier);

        // Make sure new width and height are at least 1
        imageNewWidth = max(1, imageNewWidth);
        imageNewHeight = max(1, imageNewHeight);

        // Change the zoomed image width and height
        m_zoomedImageSize = QSize(imageNewWidth, imageNewHeight);
    }
}


/// -----------------------------------------------------------------------------------------------
/// \brief Name:        setImageItemEnabledActions
/// \brief Description: Sets the image enabled actions
/// \param[in]          actions
/// \return void
/// -----------------------------------------------------------------------------------------------
void acImageItem::setImageItemEnabledActions(unsigned int actions)
{
    _imageActionEnabled = actions;
    _imageActionChecked = _imageActionChecked & _imageActionEnabled;
}

// ---------------------------------------------------------------------------
// Name:        acImageItem::setImageItemActions
// Description: Set the image actions
// Arguments:   unsigned int filter - an integer containing the actions to perform with mask
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/4/2010
// ---------------------------------------------------------------------------
void acImageItem::setImageItemActions(unsigned int actions)
{
    if (_imageActionChecked != actions)
    {
        m_shouldUpdateImage = true;
    }

    // Set the filter:
    _imageActionChecked = actions;
}

// ---------------------------------------------------------------------------
// Name:        acImageItem::forceThumbnailLoad
// Description: Forces a thumbnail item to load it's image preview
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        1/1/2008
// ---------------------------------------------------------------------------
bool acImageItem::forceThumbnailLoad()
{
    bool retVal = false;

    // Sanity check:
    if (m_pImageDataProxy != NULL)
    {
        retVal = loadDataFromImageProxy();

        // Add the item tooltip:
        m_pImageDataProxy->buildTooltipText();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageItem::zoomedPositionToRealPosition
// Description: Transform image screen position to image position
// Arguments:   physicalPosition - Screen image position
// Return Val:  Position on the image
// Author:      Eran Zinman
// Date:        27/5/2007
// ---------------------------------------------------------------------------
QPoint acImageItem::zoomedPositionToRealPosition(QPoint physicalPosition)
{
    // Get (x, y) position on the screen
    int xPos = physicalPosition.x();
    int yPos = physicalPosition.y();

    // In case we are zooming, transform the (x, y) position on the screen to image (x, y) position
    if (m_originalImageSize != m_zoomedImageSize)
    {
        // Get the multiplier
        float multiplier = 100.0 / (float) m_zoomLevel;

        // Transform screen (x, y) to image (x, y)
        xPos = int((float) physicalPosition.x() * multiplier);
        yPos = int((float) physicalPosition.y() * multiplier);
    }

    // This is the image position
    QPoint imagePosition(xPos, yPos);
    return imagePosition;
}

// ---------------------------------------------------------------------------
// Name:        acImageItem::getPixelColour
// Description: Return the pixel colour at the desired location
// Arguments:   pixelPosition - The position of the pixel on the screen,
//              pixelColour - The pixel colour
// Return Val:  Success / Failure
// Author:      Eran Zinman
// Date:        20/5/2007
// ---------------------------------------------------------------------------
bool acImageItem::getPixelColour(QPoint pixelPosition, QRgb& pixelColour)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(m_pCurrentDisplayedImage != NULL)
    {
        // Get the pixel at the requested point:
        pixelColour = m_pCurrentDisplayedImage->pixel(pixelPosition);

        // If we got the pixel color ok:
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acCanvasItem::getUnZoomedCanvasItemSize
// Description: Returns the original canvas item size (before it was zoomed)
// Author:      Eran Zinman
// Date:        8/7/2007
// ---------------------------------------------------------------------------
QSize acImageItem::getUnZoomedCanvasItemSize()
{
    QSize unzoomedCanvasItemSize(0, 0);

    // Get original image size
    unzoomedCanvasItemSize = m_originalImageSize;

    // Add border size to the calculation
    unzoomedCanvasItemSize.setWidth(unzoomedCanvasItemSize.width() + 2);
    unzoomedCanvasItemSize.setHeight(unzoomedCanvasItemSize.height() + 2);

    // Return the un-zoomed canvas item size
    return unzoomedCanvasItemSize;
}

// ---------------------------------------------------------------------------
// Name:        acImageItem::asQImage
// Description: Load to QImage
// Return Val:  QImage*
// Author:      Sigal Algranaty
// Date:        21/6/2012
// ---------------------------------------------------------------------------
QImage* acImageItem::asQImage()
{
    QImage* pRetVal = m_pCurrentDisplayedImage;
    bool loadFirstTime = (pRetVal == NULL);

    // Check if the image should be updated:
    if (loadFirstTime)
    {
        forceThumbnailLoad();
        pRetVal = m_pCurrentDisplayedImage;

        // Image is updated:
        m_shouldUpdateImage = false;
    }
    else if (m_shouldUpdateImage)
    {
        // Update the bitmap:
        updateDisplayedImage();

        // Return the new created bitmap:
        pRetVal = m_pCurrentDisplayedImage;

        // Image is updated:
        m_shouldUpdateImage = false;
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageItem::setDisplayedText
// Description: Set displayed text parameters
// Arguments:    const gtString& message
//              const gtString& heading
//              const QColor
// Author:      Sigal Algranaty
// Date:        21/6/2012
// ---------------------------------------------------------------------------
void acImageItem::setDisplayedText(const gtString& message, const gtString& heading, const QColor& color)
{
    GT_UNREFERENCED_PARAMETER(color);

    m_isDisplayingText = true;
    m_displayedTextMessage = message;
    m_displayedTextHeadline = heading;
}


// ---------------------------------------------------------------------------
// Name:        acImageItem::releaseImageMemory
// Description: Deletes an image item memory
// Arguments:   QImage* pImage
// Author:      Sigal Algranaty
// Date:        29/7/2012
// ---------------------------------------------------------------------------
void acImageItem::releaseImageMemory(QImage*& pImage)
{
    if (pImage != NULL)
    {
        uchar* pImageData = pImage->bits();

        if (pImageData != NULL)
        {
            delete [] pImageData;
        }

        delete pImage;
        pImage = NULL;
    }
}
