//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acImageManager.h
///
//==================================================================================

//------------------------------ acImageManager.h ------------------------------

#ifndef __ACIMAGEMANAGER
#define __ACIMAGEMANAGER

// Qt:
#include <QAbstractTableModel>
#include <QTableView>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtAutoPtr.h>

#include <AMDTAPIClasses/Include/ap2DRectangle.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>
#include <AMDTAPIClasses/Include/apDefaultTextureNames.h>

// Local:
#include <AMDTApplicationComponents/Include/acImageItem.h>
#include <AMDTApplicationComponents/Include/acImageDataProxy.h>
#include <AMDTApplicationComponents/Include/acImageManagerDefinitions.h>
#include <AMDTApplicationComponents/Include/acImageManagerModel.h>
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

// need to undef Bool after all includes so the moc will compile in Linux
#undef Bool

#define AC_IMAGE_MANAGER_ID_NONE -1


// Thumbnail size in pixels (size * size)
#define AC_IMAGES_MANAGER_THUMBNAIL_SIZE 128
#define AC_IMAGES_MANAGER_THUMBNAIL_MARGIN 5
#define AC_IMAGE_MANAGER_MARGIN 30
#define AC_IMAGE_MANAGER_SCROLLBAR_MARGIN 50

// Available manager layout modes:
enum acImagesManagerMode
{
    AC_MANAGER_MODE_STANDARD_ITEM,          // Standard object (2D, 3D, Rectangle textures, Buffers, etc..)
    AC_MANAGER_MODE_TEXT_ITEM,              // Text object
    AC_MANAGER_MODE_THUMBNAIL_VIEW,         // Set manager state to show thumbnails of textures / buffers
    AC_MANAGER_MODE_CUBEMAP_TEXTURE         // Cube map texture view (show layout as open cube)
};

// Enumerates the available manager tools:
enum acImageManagerToolType
{
    AC_IMAGE_MANAGER_TOOL_STANDARD,      // Standard Tool   (Standard mouse pointer)
    AC_IMAGE_MANAGER_TOOL_PAN            // Pan Tool        (Allows scrolling by dragging the mouse)
};

class acHeaderView;
// ----------------------------------------------------------------------------------
// Class Name:          AC_API acImageManager : public QTableView
// General Description: This class is a Qt implementation of an image thumbnail view
//
// Author:              Sigal Algranaty
// Creation Date:       10/6/2012
// ----------------------------------------------------------------------------------
class AC_API acImageManager : public QTableView
{
    Q_OBJECT

public:
    friend class acImageManagerModel;
    friend class acImageItemDelegate;
    // Constructor:
    acImageManager(QWidget* pParent, acImagesManagerMode managerMode);

    // Destructor:
    ~acImageManager();

    acImageItem* getItem(acImageItemID imageId);
    const acImageItem* getItem(acImageItemID imageId) const ;
    acImageItem* getItem(const QModelIndex& index);

    int itemsAmount() const;
    acImageItemID addItem(gtAutoPtr<acImageItem>& aptrCanvasItem);

    // Get / Set manager mode:
    bool setManagerMode(acImagesManagerMode managerMode);
    acImagesManagerMode managerMode() { return m_managerMode; };

    // Clears all the objects in the textures and buffers manager
    void clearAllObjects();

    // Return a pointer to the item data
    void* getItemData(acImageItemID canvasID);

    // Load an object into the manager:
    bool addThumbnailItem(acImageDataProxy* pImageDataProxy, unsigned int itemEnabledActions, void* pItemData, const gtString& topLabel, const gtString& bottomLabel);

    // Sets the currently displayed object heading:
    void setObjectHeading(gtString& objectHeading);

    // Set a tooltip to an object in the manager
    virtual bool setObjectToolTip(acImageItemID canvasID, const gtString& toolTipText);

    // Image actions:
    static bool getItemActionsByFormat(oaTexelDataFormat dataFormat, unsigned int& enabledActions);

    // Write a text message in the image manager:
    void writeTextMessage(const gtString& headline, const gtString& message = L"", const QColor& titleColor = Qt::black);

    void calculateImagesLayout();

    bool getImageVisibleRect(QRect& rect);

    QRect itemVisibleRect(const QModelIndex& index);

    bool setItemHeading(const gtString& itemHeading);

    // Allows the manager's parent to activate the manager tools
    void setActiveTool(acImageManagerToolType tool);
    acImageManagerToolType activeTool() {return m_currentTool;};
    void resetActiveTool();

    // Add items in manager, and set their properties
    acImageItemID addThumbnailIntoManager(acImageDataProxy* pImageDataProxy, unsigned int imageActionsEnable, int thumbSize);
    acImageItemID addImageObject(QImage* pImage, void* pItemData, const gtString& objectName, QPoint matrixPos, unsigned int imagePossibleActionsMask);

    int amountOfImagesInManager() { return m_imageItems.size(); };

    // Image manipulations functions
    void setFilterForAllImages(unsigned int filter, double value = 0.0, bool repaint = true);
    bool replaceImageBitmap(acImageItemID imageId, QImage* pNewImage);
    void setFirstTimeBestFitZoom(int nZoomLevel);

    // Get Current scroll position
    void getScrollPosition(int& xCoord, int& yCoord);

    // Blocking wheel event to enable image zoom using Ctrl+Wheel
    bool eventFilter(QObject*, QEvent* pEvt);

public slots:

    void forceImagesRepaint();

protected:

    // Re-implemented events:
    void mouseMoveEvent(QMouseEvent* pMouseEvent);
    void mousePressEvent(QMouseEvent* pMouseEvent);
    void mouseReleaseEvent(QMouseEvent* pMouseEvent);
    void mouseDoubleClickEvent(QMouseEvent* pMouseEvent);

    acImageItemID modelIndexToImageIndex(const QModelIndex& index);

    virtual void resizeEvent(QResizeEvent* pResizeEvent);

    void enterEvent(QEvent* pMouseEvent);
    void leaveEvent(QEvent* pMouseEvent);

    void addImageProxyForDeletion(acImageDataProxy* pImageDataProxy);
    void updateCellDimensions();
    void wheelEvent(QWheelEvent* event);

    void FixScrollbarsAppearance();


signals:

    // Pixel position changed signal:
    void pixelPositionChanged(acImageItemID imageItemID, const QPoint& posOnImage, bool mouseLeftDown, bool mouseDoubleClick);

    // First time best fit zoom level change
    void zoomChanged(int);

private:

    // View and Layout functions
    void onImageStatusChanged(const QModelIndex& itemIndex, const QPoint& mousePos, bool mouseLeftDown);
    void sendPixelInformation(const QModelIndex& mouseHoverItem, const QPoint& mousePos);
    void panImageItem(const QModelIndex& mouseHoverItem, const QPoint& mousePos, bool isLeftButtonDown);

    acImageItemID allocateNewItemIndex();
    acImageItemID loadCubemapImageItem(QImage* pImage, const QPoint& matrixPos, unsigned int imagePossibleActionsMask);
    void prepareCubemapLayout();

    void updateData();

protected:

    // The current manager mode
    acImagesManagerMode m_managerMode;

    // Contains the canvas items:
    gtPtrVector<acImageItem*> m_imageItems;

    // Contain the current image actions that should be performed:
    unsigned int m_currentImageActionsChecked;

private:

    // My data model:
    acImageManagerModel* m_pImagesModel;

    // Currently used tool
    acImageManagerToolType m_currentTool;

    // Vetical header:
    acHeaderView* m_pVerticalHeader;

    // Last mouse position
    QPoint m_lastMousePosition;

    // Contain the image visible rect:
    QRect m_imageVisibleRect;

    // Contain the rectangle for the zoomed image:
    QRect m_imageActualRect;

    QSize m_previousViewSize;
    bool m_isImageVisibleRectCalculated;

    // Current action taking place
    acImageManagerToolType m_currentAction;

    // Vector for the image proxies (to be deleted when the manager is deleted):
    gtPtrVector<acImageDataProxy*> m_imageProxiesVector;

    // The object heading canvas ID:
    gtString m_itemHeading;

    // Amount of displayed items:
    QSize m_averageItemSize;

    bool m_processingSizeChanged;
    bool m_afterSizeChanged;


};


#endif  // __ACIMAGEMANAGER
