//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acImageItem.h
///
//==================================================================================

//------------------------------ acImageItem.h ------------------------------

#ifndef __ACIMAGEITEM
#define __ACIMAGEITEM

// Qt:
#include <QBrush>
#include <QFont>
#include <QPixmap>


// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>

// Local:
#include <AMDTApplicationComponents/Include/acImageDataProxy.h>
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

// If thumbSize equals to this value, image is not a thumbnail
#define AC_IMAGE_ITEM_NOT_THUMBNAIL -1

typedef enum
{
    AC_IMAGE_GRAYSCALE_FILTER   =   0x00000001,     // Grayscale filter:
    AC_IMAGE_INVERT_FILTER      =   0x00000002,     // Invert filter:
    AC_IMAGE_ZOOM               =   0x00000004,     // Zoom
    AC_IMAGE_ROTATE             =   0x00000008,     // Rotate
    AC_IMAGE_CHANNEL_RED        =   0x00000010,     // "Red" channel
    AC_IMAGE_CHANNEL_GREEN      =   0x00000020,     // "Green" channel
    AC_IMAGE_CHANNEL_BLUE       =   0x00000040,     // "Blue" channel
    AC_IMAGE_CHANNEL_ALPHA      =   0x00000080,     // "Alpha" channel

} acImageItemAction;

typedef int acImageItemID;

// ----------------------------------------------------------------------------------
// Class Name:          AC_API acImageItem
// General Description: This class represent an image item added to the images manager
//                      object
// Author:              Sigal Algranaty
// Creation Date:       10/6/2012
// ----------------------------------------------------------------------------------
class AC_API acImageItem
{
public:
    // Constructor:
    acImageItem(acImageItemID imageItemID, unsigned int imageActionsEnable, int thumbSize);

    // Destructor:
    ~acImageItem();

    acImageItemID itemId() const { return m_itemID; };
    void setItemId(acImageItemID canvasItemId) { m_itemID = canvasItemId; };

    QImage* asQImage();

    // Set the image data proxy loader service
    bool setImageDataProxy(acImageDataProxy* pImageDataProxy, bool loadOnDemand = false);

    // Load the image from an existing QImage object
    bool loadFromQImage(QImage* dib) { return replaceBitmap(dib, false /* show image in full size */); };

    // Replace currently displayed image with a new image
    bool replaceBitmap(QImage* dib, bool keepCurrentDimensions = true);

    // Update the image (renders the image)
    bool updateDisplayedImage();

    // Forces a thumbnail item to load it's image preview
    bool forceThumbnailLoad();

    // Set image filters properties
    void setImageZoomLevel(int zoomLevel);
    bool rotateImageByAngle(int rotationAngle);

    void setImageItemEnabledActions(unsigned int actions);
    void setImageItemActions(unsigned int actions);
    void applyChannelsFilter(QRgb& color) const;

    // Image channels public query functions:
    bool isActionChecked(acImageItemAction action);

    // General image public query functions:
    QSize getUnZoomedCanvasItemSize();
    QSize originalImageSize() { return m_originalImageSize; };
    QSize zoomedImageSize() { return m_zoomedImageSize;};
    bool getPixelColour(QPoint pixelPosition, QRgb& pixelColour);
    QPoint zoomedPositionToRealPosition(QPoint physicalPosition);
    bool isLoaded() { return m_isImageLoaded; };
    acImageDataProxy* imageProxy() { return m_pImageDataProxy; };
    int zoomLevel() {return m_zoomLevel;};

    // Generates an image snapshot of what currently seen on screen (WYSIWYG) in QImage format
    QImage* generateImageSnapshot();

    // Image bounding rectangle (actual and available):
    void setImageBoundingRect(const QRect& imageRect) { m_imageBoundingRect = imageRect;}
    QRect imageBoundingRect() const { return m_imageBoundingRect;}

    void setImageAvailableBoundingRect(const QRect& imageRect) { m_imageAvailableBoundingRect = imageRect;}
    QRect imageAvailableBoundingRect() const { return m_imageAvailableBoundingRect;}

    // Text:
    bool isDisplayingText() const {return m_isDisplayingText;};
    const gtString& displayedTextHeadline() const {return m_displayedTextHeadline;};
    const gtString& displayedTextMessage() const {return m_displayedTextMessage;};
    const QColor& displayedTextColor() const {return m_displayedTextColor;};

    // Empty items:
    bool isEmpty() const {return m_isEmpty;}
    void setIsEmpty(bool isEmpty = true) {m_isEmpty = isEmpty;}

    void setDisplayedText(const gtString& message, const gtString& heading = L"", const QColor& = Qt::black);

    // Item labels:
    void setLabels(const gtString& topLabel, const gtString& bottomLabel) {_topLabel = topLabel; _bottomLabel = bottomLabel;};
    const gtString& topLabel() const {return _topLabel;}
    const gtString& bottomLabel() const {return _bottomLabel;}

    // Item tooltip:
    const gtString& toolTipText() const {return _toolTipText;}
    void setToolTipText(const gtString& toolTipText) {_toolTipText = toolTipText;}

    // Item data:
    void* itemData() const {return _pItemData;}
    void setItemData(void* pItemData) {_pItemData = pItemData;}

private:

    // Load data from image proxy
    bool loadDataFromImageProxy();

    // Load an image / thumbnail file
    bool loadImage(bool keepCurrentDimensions = false);

    // Optimized image rendering functions (used for optimized drawing)
    void updateDrawingRegion(QRegion updateRegion);

    // Internal inquiry
    bool isThumbnail() { return m_thumbSize != AC_IMAGE_ITEM_NOT_THUMBNAIL; };

    void releaseImageMemory(QImage*& pImage);

private:

    // The bitmap that will be displayed to the user
    QImage* m_pOriginalImageFromProxy;
    QImage* m_pCurrentDisplayedImage;

    // Item ID:
    acImageItemID m_itemID;

    // The required thumbnail size (if image is thumbnail)
    int m_thumbSize;

    // This flag indicates if the image was loaded already or not (in thumbnail view)
    bool m_isImageLoaded;

    // This is the brush used to paint the image background
    QBrush m_backgroundBrush;

    // True iff we currently display text:
    bool m_isDisplayingText;
    gtString m_displayedTextHeadline;
    gtString m_displayedTextMessage;
    QColor m_displayedTextColor;

    // True iff the item is empty (for cubemap dummy images):
    bool m_isEmpty;

    // The rectangle containing the image:
    QRect m_imageBoundingRect;
    QRect m_imageAvailableBoundingRect;

    // The image data proxy which gives us the image data
    acImageDataProxy* m_pImageDataProxy;

    // General image properties
    bool m_shouldUpdateImage;

    // Current image size:
    QSize m_zoomedImageSize;

    // Original image size
    QSize m_originalImageSize;

    // Image zoom level:
    int m_zoomLevel;

    // Image rotation angle:
    int m_rotationAngle;

    // Is image item enabled?
    unsigned int _imageActionEnabled;

    // Is image item checked?
    unsigned int _imageActionChecked;

    // Item labels:
    gtString _topLabel;
    gtString _bottomLabel;
    gtString _toolTipText;

    // Item data:
    void* _pItemData;

};

#endif  // __ACIMAGEITEM
