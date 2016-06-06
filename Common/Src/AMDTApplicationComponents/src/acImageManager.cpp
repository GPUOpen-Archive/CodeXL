//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acImageManager.cpp
///
//==================================================================================

//------------------------------ acImageManager.cpp ------------------------------

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSAPIWrappers/Include/oaTexelDataFormat.h>

// Local:
#include <AMDTApplicationComponents/Include/acCommandIDs.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acHeaderView.h>
#include <AMDTApplicationComponents/Include/acImageItemDelegate.h>
#include <AMDTApplicationComponents/Include/acImageManager.h>
#include <AMDTApplicationComponents/Include/acImageManagerModel.h>
#include <AMDTApplicationComponents/Include/acImageItem.h>
#include <inc/acStringConstants.h>


// This is a flag indicating mouse position is outside the image manager
#define AC_IMAGE_MANAGER_MOUSE_OUTSIDE_MANAGER QPoint(-1, -1)
// Maximum manager size in pixels (size * size)
#define AC_IMAGES_MANAGER_ITEM_LABEL_FONT_SIZE 8
#define AC_IMAGES_MANAGER_ITEM_HEADING_FONT_SIZE 10
#define AC_IMAGES_MANAGER_SCROLL_RATE 10

// TO_DO: thumbnails optimization: should be 1 or 2, making it zero to make the debug more clear:
#define AC_IMAGES_MANAGER_EXTRA_LINES_COUNT 0

// ---------------------------------------------------------------------------
// Name:        acImageManager::acImageManager
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        10/6/2012
// ---------------------------------------------------------------------------
acImageManager::acImageManager(QWidget* pParent, acImagesManagerMode managerMode)
    : QTableView(pParent),
      m_managerMode(managerMode),
      m_currentImageActionsChecked(0),
      m_pImagesModel(NULL),
      m_currentTool(AC_IMAGE_MANAGER_TOOL_STANDARD),
      m_pVerticalHeader(NULL),
      m_lastMousePosition(AC_IMAGE_MANAGER_MOUSE_OUTSIDE_MANAGER),
      m_imageVisibleRect(0, 0, 0, 0),
      m_imageActualRect(0, 0, 0, 0),
      m_previousViewSize(0, 0),
      m_isImageVisibleRectCalculated(false),
      m_currentAction(AC_IMAGE_MANAGER_TOOL_STANDARD),
      m_itemHeading(L""),
      m_averageItemSize(0, 0),
      m_processingSizeChanged(false),
      m_afterSizeChanged(false)
{
    setManagerMode(managerMode);

    setWindowTitle(tr("Image Viewer"));

    // Create new images model:
    m_pImagesModel = new acImageManagerModel(this);
    setModel(m_pImagesModel);

    // Do not show the grid:
    setShowGrid(false);

    // Get the system background color:
    QColor bgColor = acGetSystemDefaultBackgroundColor();

    // Set the manager background color:
    setAutoFillBackground(true);
    QPalette managerPalette = this->palette();
    managerPalette.setColor(this->backgroundRole(), bgColor);
    managerPalette.setColor(QPalette::Base, bgColor);
    managerPalette.setColor(QPalette::Text, Qt::black);

    setPalette(managerPalette);

    // Create an item delegate:
    acImageItemDelegate* pItemDelegate = new acImageItemDelegate(this);


    // Set the item delegate:
    setItemDelegate(pItemDelegate);

    // Create a new vertical header:
    m_pVerticalHeader = new acHeaderView(Qt::Vertical, NULL);

    setVerticalHeader(m_pVerticalHeader);

    verticalHeader()->hide();
    horizontalHeader()->hide();
    verticalHeader()->setContentsMargins(0, 0, 0, 0);
    horizontalHeader()->setContentsMargins(0, 0, 0, 0);
    horizontalHeader()->setAutoScrollMargin(false);
    verticalHeader()->setAutoScrollMargin(false);

    bool rcConnect = connect(m_pImagesModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(forceImagesRepaint()));
    GT_ASSERT(rcConnect);
    rcConnect = connect(m_pImagesModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), this, SLOT(forceImagesRepaint()));
    GT_ASSERT(rcConnect);

    // Make sure that images are repaint when scrolling:
    rcConnect = connect(horizontalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(forceImagesRepaint()));
    GT_ASSERT(rcConnect);

    rcConnect = connect(verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(forceImagesRepaint()));
    GT_ASSERT(rcConnect);

    // Enable mouse move events:
    setMouseTracking(true);
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::~acImageManager
// Description: Destructor.
// Author:      Eran Zinman
// Date:        22/5/2007
// ---------------------------------------------------------------------------
acImageManager::~acImageManager()
{
    // Delete all elements in the vectors:
    clearAllObjects();

    // Delete the vector that contains the textures items
    m_imageItems.deleteElementsAndClear();

}

// ---------------------------------------------------------------------------
// Name:        acImageManager::replaceImageBitmap
// Description: Replaces a given image item (given the canvasID) with a
//              new bitmap (dib)
// Arguments:   canvasID - The image item to replace it's bitmap
//              dib - The new bitmap
// Author:      Eran Zinman
// Date:        9/11/2007
// ---------------------------------------------------------------------------
bool acImageManager::replaceImageBitmap(acImageItemID imageId, QImage* pImage)
{
    bool retVal = false;

    // Get image item
    acImageItem* pImageItem = getItem(imageId);
    GT_IF_WITH_ASSERT(pImageItem != NULL)
    {
        // Replace the bitmap with the new bitmap
        retVal = pImageItem->replaceBitmap(pImage);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::enterEvent
// Description: Called when the mouse enters the manager window
// Arguments:   QEvent* pEvent
// Author:      Sigal Algranaty
// Date:        10/6/2012
// ---------------------------------------------------------------------------
void acImageManager::enterEvent(QEvent* pEvent)
{
    GT_UNREFERENCED_PARAMETER(pEvent);

    // Is any tool activated?
    switch (m_currentTool)
    {
        case AC_IMAGE_MANAGER_TOOL_PAN:
        {
            setCursor(QCursor(Qt::ClosedHandCursor));
        }
        break;

        default:
        {
            // Set standard cursor:
            setCursor(QCursor(Qt::ArrowCursor));
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::leaveEvent
// Description: Called when the mouse leaves the manager window
// Arguments:   QEvent* pEvent
// Author:      Sigal Algranaty
// Date:        10/6/2012
// ---------------------------------------------------------------------------
void acImageManager::leaveEvent(QEvent* pEvent)
{
    GT_UNREFERENCED_PARAMETER(pEvent);

    // Reset last mouse position
    m_lastMousePosition = AC_IMAGE_MANAGER_MOUSE_OUTSIDE_MANAGER;

    // Emit pixel position changed event:
    emit pixelPositionChanged(AC_IMAGE_MANAGER_ID_NONE, QPoint(-1, -1), false, false);

    // If we are currently panning
    if (m_currentAction == AC_IMAGE_MANAGER_TOOL_PAN)
    {
        // Release the mouse:
        releaseMouse();
    }
    else
    {
        setCursor(Qt::ArrowCursor);
        setToolTip("");

    }

    // Reset current action
    m_currentAction = AC_IMAGE_MANAGER_TOOL_STANDARD;
}

// ---------------------------------------------------------------------------
// Name:        acImageManager:::setFilterForAllImages
// Description: Sets a filters for all the images in the manager
// Arguments:   filter - The filter type to apply to the images (a bitwise of type acImageItemAction)
//              value - The value associated with the filter (some filters don't require this argument)
// Author:      Eran Zinman
// Date:        15/7/2007
// ---------------------------------------------------------------------------
void acImageManager::setFilterForAllImages(unsigned int filter, double value, bool repaint)
{
    if (managerMode() != AC_MANAGER_MODE_TEXT_ITEM)
    {
        // Set the current image actions for later use (on scroll):
        m_currentImageActionsChecked = filter;

        // Get amount of images in the manager
        int amountOfIndices = m_imageItems.size();

        if (amountOfIndices > 0)
        {
            // Loop through all the images in the manager
            for (int i = 0; i < amountOfIndices; i++)
            {
                // Get the canvas item
                acImageItem* pImageItem = (acImageItem*)getItem((acImageItemID)i);
                GT_IF_WITH_ASSERT(pImageItem != NULL)
                {
                    // This is a general boolean that is being used by many filters.
                    // It is set to true if value is different than 0, false otherwise
                    if ((filter & AC_IMAGE_GRAYSCALE_FILTER) ||
                        (filter & AC_IMAGE_INVERT_FILTER) ||
                        (filter & AC_IMAGE_CHANNEL_RED) ||
                        (filter & AC_IMAGE_CHANNEL_GREEN) ||
                        (filter & AC_IMAGE_CHANNEL_BLUE) ||
                        (filter & AC_IMAGE_CHANNEL_ALPHA) ||
                        (filter == 0))
                    {
                        // Set the image item action:
                        pImageItem->setImageItemActions(filter);
                    }

                    if (filter & AC_IMAGE_ZOOM)
                    {
                        // Scroll the window by half the difference of the sizes:
                        pImageItem->setImageZoomLevel(value);
                        updateGeometries();
                    }

                    if (filter & AC_IMAGE_ROTATE)
                    {
                        // Set the item rotation angle
                        pImageItem->rotateImageByAngle(int(value));
                    }
                }
            }

            if (repaint)
            {
                // Refresh the manager:
                forceImagesRepaint();
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        acImageManager:::setFirstTimeBestFitZoom
// Description: Sets first time best fit zoom level to all images and emits zoomChanged signal
// Arguments:   nZoomLevel - First time best fit zoom level
// Author:      Yuri Rshtunique
// Date:        19/07/2014
// ---------------------------------------------------------------------------

void acImageManager::setFirstTimeBestFitZoom(int nZoomLevel)
{
    // Get amount of images in the manager
    int amountOfIndices = m_imageItems.size();

    for (int i = 0; i < amountOfIndices; i++)
    {
        // Get the canvas item
        acImageItem* pImageItem = (acImageItem*)getItem((acImageItemID)i);
        GT_IF_WITH_ASSERT(pImageItem != NULL)
        {

            pImageItem->setImageZoomLevel(nZoomLevel);
            updateGeometries();
        }
    }

    emit zoomChanged(nZoomLevel);
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::mouseMoveEvent
// Description: Called when there is a mount movement
// Arguments:   QMouseEvent* pMouseEvent
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        10/6/2012
// ---------------------------------------------------------------------------
void acImageManager::mouseMoveEvent(QMouseEvent* pMouseEvent)
{
    if (m_managerMode != AC_MANAGER_MODE_TEXT_ITEM)
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(pMouseEvent != NULL)
        {
            // Clear cursor and tooltip (unless we're in pan mode):
            if (m_currentTool != AC_IMAGE_MANAGER_TOOL_PAN)
            {
                setCursor(Qt::ArrowCursor);
            }

            setToolTip("");

            if (m_imageItems.size() > 0)
            {
                // Get the mouse position:
                QPoint currentMousePosition = pMouseEvent->pos();

                // Get the index of the mouse hovered item:
                QModelIndex mouseHoverItem = indexAt(pMouseEvent->pos());

                if (mouseHoverItem.isValid())
                {
                    // If we are using the standard pointer, return pixel information
                    if (m_currentTool == AC_IMAGE_MANAGER_TOOL_STANDARD)
                    {
                        // Send a pixel change information:
                        sendPixelInformation(mouseHoverItem, currentMousePosition);
                    }

                    // If Pan Tool is active and the action is set:
                    if (m_currentTool == AC_IMAGE_MANAGER_TOOL_PAN && m_currentAction == AC_IMAGE_MANAGER_TOOL_PAN)
                    {
                        // Get mouse button state
                        bool isLeftButtonDown = pMouseEvent->buttons() & Qt::LeftButton;
                        panImageItem(mouseHoverItem, currentMousePosition, isLeftButtonDown);
                    }
                }
            }
        }
    }

    // Call the base class implementation:
    QTableView::mouseMoveEvent(pMouseEvent);
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::resetActiveTool
// Description: Reset the currently active tool, and make the standard tool
//              as the currently active tool.
// Author:      Eran Zinman
// Date:        4/7/2007
// ---------------------------------------------------------------------------
void acImageManager::resetActiveTool()
{
    // Set the default pointer as the currently active tool
    m_currentTool = AC_IMAGE_MANAGER_TOOL_STANDARD;
    m_currentAction = AC_IMAGE_MANAGER_TOOL_STANDARD;
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::mousePressEvent
// Description: Called when mouse button is clicked
// Arguments:   QMouseEvent* pMouseEvent
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        10/6/2012
// ---------------------------------------------------------------------------
void acImageManager::mousePressEvent(QMouseEvent* pMouseEvent)
{
    // Sanity check
    GT_IF_WITH_ASSERT(pMouseEvent != NULL)
    {
        if (pMouseEvent->buttons() & Qt::LeftButton)
        {
            // If we are currently panning:
            if (m_currentTool == AC_IMAGE_MANAGER_TOOL_PAN)
            {
                // Save current mouse position (relative to screen)
                m_lastMousePosition = pMouseEvent->pos();

                // Set current action to panning
                m_currentAction = AC_IMAGE_MANAGER_TOOL_PAN;
            }
            else
            {
                // Get current mouse position
                // Get the index of the mouse hovered item:
                QModelIndex mouseHoverItem = indexAt(pMouseEvent->pos());

                if (mouseHoverItem.isValid())
                {
                    // Get the item for this index:
                    acImageItem* pImageItem = getItem(mouseHoverItem);

                    // Not all indices have matching items (when lines are not filled):
                    if (pImageItem != NULL)
                    {
                        // Check if the image real rectangle is hit by this mouse position:
                        if (pImageItem->imageBoundingRect().contains(pMouseEvent->pos()))
                        {
                            // Update the manager image information, according to the mouse position
                            onImageStatusChanged(mouseHoverItem, pMouseEvent->pos(), true);
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        mouseReleaseEvent
// Description: Called when left button is up
// Arguments:   QMouseEvent* pMouseEvent
// Return Val:  void acImageManager::mouseReleaseEvent
// Author:      Sigal Algranaty
// Date:        10/6/2012
// ---------------------------------------------------------------------------
void acImageManager::mouseReleaseEvent(QMouseEvent* pMouseEvent)
{
    // Sanity check
    GT_IF_WITH_ASSERT(pMouseEvent != NULL)
    {

        if (pMouseEvent->buttons() & Qt::LeftButton)
        {

            // If we are currently panning
            if (m_currentAction == AC_IMAGE_MANAGER_TOOL_PAN)
            {
                // Reset last mouse position
                m_lastMousePosition = AC_IMAGE_MANAGER_MOUSE_OUTSIDE_MANAGER;
            }
            else
            {
                // Get current mouse position
                // Get the index of the mouse hovered item:
                QModelIndex mouseHoverItem = indexAt(pMouseEvent->pos());

                if (mouseHoverItem.isValid())
                {
                    // Update the manager image information, according to the mouse position
                    onImageStatusChanged(mouseHoverItem, pMouseEvent->pos(), true);
                }
            }
        }

        // Reset current action
        m_currentAction = AC_IMAGE_MANAGER_TOOL_STANDARD;
    }
}



// ---------------------------------------------------------------------------
// Name:        acImageManager::mouseDoubleClickEvent
// Description: Is handling one of the thumbnails double click
// Arguments:   QMouseEvent* pMouseEvent
// Author:      Sigal Algranaty
// Date:        21/6/2012
// ---------------------------------------------------------------------------
void acImageManager::mouseDoubleClickEvent(QMouseEvent* pMouseEvent)
{
    // Sanity check
    GT_IF_WITH_ASSERT(pMouseEvent != NULL)
    {
        if (m_managerMode == AC_MANAGER_MODE_THUMBNAIL_VIEW)
        {
            // Get the index of the mouse clicked item:
            QModelIndex mouseHoverItem = indexAt(pMouseEvent->pos());

            if (mouseHoverItem.isValid())
            {
                acImageItemID imageItemID = modelIndexToImageIndex(mouseHoverItem);

                // Send a pixel position signal:
                emit pixelPositionChanged(imageItemID, pMouseEvent->pos(), true, true);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        acImageManager::addThumbnailIntoManager
// Description: Adds an QImage object into the image manager
// Arguments:   pImageDataProxy - The thumbnail data proxy
//              imageActionsEnable - image actions mask
//              thumnbSize - Thumbnail size
// Return Val:  The canvas index of the added item
// Author:      Eran Zinman
// Date:        22/5/2007
// ---------------------------------------------------------------------------
acImageItemID acImageManager::addThumbnailIntoManager(acImageDataProxy* pImageDataProxy, unsigned int imageActionsEnable, int thumbSize)
{
    acImageItemID imageItemID(0);

    // Sanity check:
    GT_IF_WITH_ASSERT(pImageDataProxy != NULL)
    {
        // Create a new thumbnail item
        acImageItem* pImageItem = new acImageItem(AC_IMAGE_MANAGER_ID_NONE, imageActionsEnable, thumbSize);


        // Load standard bitmap file into manager:
        bool rc = pImageItem->setImageDataProxy(pImageDataProxy, true /* Load on demand */);
        GT_IF_WITH_ASSERT(rc)
        {
            // Add the image proxy item for later deletion:
            addImageProxyForDeletion(pImageDataProxy);

            // Add the image item into the manager
            gtAutoPtr<acImageItem> aptrAdvanecdImageItem = pImageItem;

            // Get the added image ID
            imageItemID = addItem(aptrAdvanecdImageItem);

            // Set the image item canvas item id:
            pImageItem->setItemId(imageItemID);
        }
    }

    // Return the new added canvas item ID
    return imageItemID;
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::onImageStatusChanged
// Description: When an event is occurred (mouse movement, mouse left button is down)
//              we check if mouse is over an image or not. If it's over an image,
//              send the properties to my parent, else - let my parent know mouse
//              is not over an image.
// Arguments:   mousePos - Mouse position on the canvas
//              mouseLeftDown - Is mouse left button is down or not?
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        14/6/2007
// ---------------------------------------------------------------------------
void acImageManager::onImageStatusChanged(const QModelIndex& itemIndex, const QPoint& mousePos, bool mouseLeftDown)
{
    bool isOnImage = false;

    if (itemIndex.isValid())
    {
        // Get the image item for the index:
        acImageItem* pImageItem = (acImageItem*)getItem(itemIndex);

        if (pImageItem != NULL)
        {
            // Check if the mouse position intersects with the actual rectangle of the image:
            QRect imageRect = pImageItem->imageBoundingRect();

            if (imageRect.contains(mousePos))
            {
                // Calculate the mouse position on the image
                QPoint posOnImage = mousePos - imageRect.topLeft();
                posOnImage = pImageItem->zoomedPositionToRealPosition(posOnImage);

                // Send new item information event to my parent:
                acImageItemID imageItemID = modelIndexToImageIndex(itemIndex);
                emit pixelPositionChanged(imageItemID, posOnImage, mouseLeftDown, false);

                isOnImage = true;
            }
        }
    }

    if (!isOnImage)
    {
        // Notify my parent mouse is not over an image
        emit pixelPositionChanged(AC_IMAGE_MANAGER_ID_NONE, QPoint(-1, -1), mouseLeftDown, false);
    }
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::addImageProxyForDeletion
// Description: Add a proxy for later deletion
// Arguments:   acImageDataProxy* pImageDataProxy
// Author:      Sigal Algranaty
// Date:        21/6/2012
// ---------------------------------------------------------------------------
void acImageManager::addImageProxyForDeletion(acImageDataProxy* pImageDataProxy)
{
    // If the proxy memory is allocated:
    if (pImageDataProxy != NULL)
    {
        // Check if this proxy item already exist (we do not delete proxies, but reuse them, so this proxy item
        // can already exist, if we go down and up in the texture viewer):
        gtPtrVector<acImageDataProxy*>::iterator firstIter = m_imageProxiesVector.begin();
        gtPtrVector<acImageDataProxy*>::iterator endIter = m_imageProxiesVector.end();
        gtPtrVector<acImageDataProxy*>::iterator searchIter = gtFind(firstIter, endIter, pImageDataProxy);

        // If we found the item:
        if (searchIter == endIter)
        {
            // Push the proxy to the vector for later deletion:
            m_imageProxiesVector.push_back(pImageDataProxy);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::getScrollPosition
// Description: Get the current scroll bars positions
// Arguments:   int& xCoord
//              int& yCoord
// Author:      Sigal Algranaty
// Date:        10/6/2012
// ---------------------------------------------------------------------------
void acImageManager::getScrollPosition(int& xCoord, int& yCoord)
{
    xCoord = horizontalScrollBar()->sliderPosition();
    yCoord = verticalScrollBar()->sliderPosition();
}

// ---------------------------------------------------------------------------
// Name:        modelIndexToImageIndex
// Description: Model index to image index
// Arguments:   const QModelIndex& acImageManager
// Return Val:  acImageItemID
// Author:      Sigal Algranaty
// Date:        25/6/2012
// ---------------------------------------------------------------------------
acImageItemID acImageManager::modelIndexToImageIndex(const QModelIndex& index)
{
    acImageItemID retVal = AC_IMAGE_MANAGER_ID_NONE;

    GT_IF_WITH_ASSERT(model() != NULL)
    {
        // Convert the model index to image index:
        retVal = index.row() * model()->columnCount() + index.column();
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::getItem
// Description: Get an item by its index
// Arguments:   const QModelIndex& index
// Return Val:  acImageItem*
// Author:      Sigal Algranaty
// Date:        25/6/2012
// ---------------------------------------------------------------------------
acImageItem* acImageManager::getItem(const QModelIndex& index)
{
    acImageItem* pRetVal = NULL;

    // Get the image item index:
    acImageItemID imageId = modelIndexToImageIndex(index);

    if ((imageId != AC_IMAGE_MANAGER_ID_NONE) && (imageId < (int)m_imageItems.size()))
    {
        // Get the image item for this index:
        pRetVal = getItem(imageId);
    }

    return pRetVal;
}
// ---------------------------------------------------------------------------
// Name:        acImageManager::getItem
// Description:
// Arguments:   int row
//              int col
// Return Val:  acImageItem*
// Author:      Sigal Algranaty
// Date:        10/6/2012
// ---------------------------------------------------------------------------
acImageItem* acImageManager::getItem(acImageItemID imageId)
{
    acImageItem* pRetVal = NULL;
    GT_IF_WITH_ASSERT((imageId >= 0) && (imageId < (int)m_imageItems.size()))
    {
        pRetVal = m_imageItems[imageId];
    }
    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::allocateNewItemIndex
// Description: Allocated a new / reused index for new canvas items.
// Return Val:  int - The allocated index.
// Author:      Yaki Tebeka
// Date:        14/8/2005
// ---------------------------------------------------------------------------
acImageItemID acImageManager::allocateNewItemIndex()
{
    acImageItemID retVal = AC_IMAGE_MANAGER_ID_NONE;

    // First, try to reuse an index:
    int amountOfIndices = m_imageItems.size();

    for (int i = 0; i < amountOfIndices; i++)
    {
        // If we found an unused index:
        if (m_imageItems[i] == NULL)
        {
            retVal = i;
            break;
        }
    }

    // If we didn't find an index that we can reuse:
    if (retVal == -1)
    {
        // Allocate a new index:
        retVal = amountOfIndices;
        m_imageItems.push_back(NULL);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acCanvas::addItem
// Description: Adds an item into the canvas.
// Arguments:   aptrCanvasItem - The canvas item to be added
// Return Val:  int - The canvas index of the added item.
// Author:      Yaki Tebeka
// Date:        29/6/2005
// ---------------------------------------------------------------------------
acImageItemID acImageManager::addItem(gtAutoPtr<acImageItem>& aptrCanvasItem)
{
    acImageItemID retVal = allocateNewItemIndex();

    // Take ownership on the canvas item:
    acImageItem* pCanvasItem = aptrCanvasItem.releasePointedObjectOwnership();
    m_imageItems[retVal] = pCanvasItem;

    // Update the data:
    GT_IF_WITH_ASSERT(m_pImagesModel != NULL)
    {
        m_pImagesModel->updateModel();
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::getItem
// Description: Const access to canvas items.
// Arguments:   itemIndex - The queried item canvas index.
// Return Val:  const acImageManager* - Will get a const pointer to the queried item,
//                                    or NULL if an item of the input index does not
//                                    exist.
// Author:      Yaki Tebeka
// Date:        30/6/2005
// ---------------------------------------------------------------------------
const acImageItem* acImageManager::getItem(acImageItemID imageId) const
{
    const acImageItem* retVal = NULL;

    // Sanity check:
    int amountOfItems = m_imageItems.size();

    if ((0 <= imageId) && (imageId < amountOfItems))
    {
        retVal = m_imageItems[imageId];
    }

    return retVal;
}




// ---------------------------------------------------------------------------
// Name:        acImageManager::setManagerMode
// Description: Sets the current manager mode, also apply special margins
//              and padding if required by the new mode
// Arguments:   managerMode - The new manager mode
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        8/6/2007
// ---------------------------------------------------------------------------
bool acImageManager::setManagerMode(acImagesManagerMode managerMode)
{
    bool retVal = true;

    // Set the new manager mode:
    m_managerMode = managerMode;

    // Special mode handling code
    switch (managerMode)
    {
        case AC_MANAGER_MODE_THUMBNAIL_VIEW:         // Set manager state to show thumbnails of textures
        {
            // Reset the currently active tool, as no tools are enabled in thumbnail view
            resetActiveTool();
            setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
            setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
            setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
            setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        }
        break;

        case AC_MANAGER_MODE_TEXT_ITEM:
        {
            // If there are objects that are currently shown, delete them:
            clearAllObjects();
            setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
            setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        }
        break;

        case AC_MANAGER_MODE_CUBEMAP_TEXTURE:       // Cube map view (show layout as open cube)
        {
            // Prepare cubemap layout:
            prepareCubemapLayout();
        }
        break;

        case AC_MANAGER_MODE_STANDARD_ITEM:
        {
            setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
            setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        }
        break;

        default:
        {
            // Do nothing:
            retVal = true;
        }
        break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::getCanvasItemTextureID
// Description: Return the texture ID of the canvas item
// Arguments:   canvasD - The canvas item ID to get it's data
// Return Val:  Pointer to the item data item if successful, null otherwise
// Author:      Eran Zinman
// Date:        2/7/2007
// ---------------------------------------------------------------------------
void* acImageManager::getItemData(acImageItemID canvasID)
{
    void* pItemData = NULL;

    acImageItem* pImageItem = getItem(canvasID);
    GT_IF_WITH_ASSERT(pImageItem != NULL)
    {
        pItemData = pImageItem->itemData();
    }

    // Return the pointer to the item data
    return pItemData;
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::addImageObject
// Description: Add a free image item to the manager
// Arguments:   dib - The QImage image format to add to the manager
//              pItemData - The data associated with this item
//              objectName - The object name
//              matrixPos - The matrix x,y position
//              imagePossibleActionsMask - the image actions (channels check / zoom / invert etc'):
// Return Val:  acImageItemID
// Author:      Sigal Algranaty
// Date:        5/7/2012
// ---------------------------------------------------------------------------
acImageItemID acImageManager::addImageObject(QImage* pImage, void* pItemData, const gtString& objectName, QPoint matrixPos, unsigned int imagePossibleActionsMask)
{
    GT_UNREFERENCED_PARAMETER(pItemData);

    acImageItemID imageItemID = AC_IMAGE_MANAGER_ID_NONE;

    // Sanity Check:
    GT_IF_WITH_ASSERT(pImage != NULL)
    {
        if (m_managerMode == AC_MANAGER_MODE_CUBEMAP_TEXTURE)
        {
            // This is a cube map texture. In this case we should add "dummy" image items:
            imageItemID = loadCubemapImageItem(pImage, matrixPos, imagePossibleActionsMask);
        }
        else
        {
            // Create the image item:
            acImageItem* pImageItem = new acImageItem(imageItemID, imagePossibleActionsMask, AC_IMAGE_ITEM_NOT_THUMBNAIL);


            // Set the image current image actions:
            pImageItem->setImageItemEnabledActions(imagePossibleActionsMask);

            // Load standard bitmap file into manager
            bool rc = pImageItem->loadFromQImage(pImage);
            GT_IF_WITH_ASSERT(rc)
            {
                // Add the image item into the manager
                gtAutoPtr<acImageItem> aptrAdvanecdImageItem = pImageItem;

                // Get the added image ID
                imageItemID = addItem(aptrAdvanecdImageItem);
            }
        }
    }

    // Get the item and set it's title:
    acImageItem* pImageItem = getItem(imageItemID);
    GT_IF_WITH_ASSERT(pImageItem != NULL)
    {
        pImageItem->setLabels(objectName, L"");
    }

    // Return the new added canvas item ID
    return imageItemID;
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::setItemHeading
// Description: Adds an item heading
// Arguments:   const gtString& itemHeading - the item heading
// Return Val:  bool - Success / Failure
// Author:      Sigal Algranaty
// Date:        22/1/2009
// ---------------------------------------------------------------------------
bool acImageManager::setItemHeading(const gtString& itemHeading)
{
    bool retVal = false;

    // Set the item heading member:
    m_itemHeading = itemHeading;
    retVal = true;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::addThumbnailItem
// Description: Loads a texture into the manager
// Arguments:   pImageDataProxy - The thumbnail data proxy
//              pItemData - The thumbnail data struct with thumbnail details
//              topLabel, bottomLabel - Thumbnail top and bottom label
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        20/12/2007
// ---------------------------------------------------------------------------
bool acImageManager::addThumbnailItem(acImageDataProxy* pImageDataProxy, unsigned int itemEnabledActions, void* pItemData, const gtString& topLabel, const gtString& bottomLabel)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pImageDataProxy != NULL)
    {
        // Add thumbnail item:
        acImageItemID imageItemID = addThumbnailIntoManager(pImageDataProxy, itemEnabledActions, AC_IMAGES_MANAGER_THUMBNAIL_SIZE);
        GT_IF_WITH_ASSERT(imageItemID != AC_IMAGE_MANAGER_ID_NONE)
        {
            // Get the canvas item:
            acImageItem* pImageItem = (acImageItem*)getItem(imageItemID);
            GT_IF_WITH_ASSERT(pImageItem != NULL)
            {
                // Apply the current image action on this item:
                pImageItem->setImageItemActions(m_currentImageActionsChecked);

                // Fill the cached item data details:
                pImageItem->setLabels(topLabel, bottomLabel);
                pImageItem->setItemData(pItemData);

                if (!pImageItem->topLabel().isEmpty())
                {
                    gtString toolTipText;
                    toolTipText.appendFormattedString(AC_STR_ImageViewThumbnailViewItem, pImageItem->topLabel().asCharArray());

                    // Set the tooltip text for the thumbnail:
                    bool rc1 = setObjectToolTip(imageItemID, toolTipText);
                    GT_ASSERT(rc1);

                    retVal = true;
                }
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::clearAllObjects
// Description: Clears all of the textures and buffers from the manager
// Author:      Eran Zinman
// Date:        9/6/2007
// ---------------------------------------------------------------------------
void acImageManager::clearAllObjects()
{
    // Clear the image proxies memory:
    m_imageProxiesVector.deleteElementsAndClear();

    // Reset Scroll Bars data
    horizontalScrollBar()->setValue(0);
    verticalScrollBar()->setValue(0);

    m_averageItemSize = QSize(0, 0);

    if (m_managerMode != AC_MANAGER_MODE_TEXT_ITEM)
    {
        m_imageItems.deleteElementsAndClear();
    }


    // Refresh view
    layout();
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::forceImagesRepaint
// Description: Forces the manager to repaint itself
// Author:      Sigal Algranaty
// Date:        4/7/2012
// ---------------------------------------------------------------------------
void acImageManager::forceImagesRepaint()
{
    bool isVisible = this->isVisible();

    if (isVisible)
    {
        // Re-calculate the image size:
        calculateImagesLayout();

        // Re-calculate the cell dimensions:
        updateCellDimensions();

        // NOTICE: for some reason I should set manually the dirty region, in order
        // for the widget to repaint the table cells:
        QRect dirtyRegionRect = this->visibleRegion().boundingRect();

        // Get the scroll offset:
        int hScrollOffset = horizontalScrollBar()->sliderPosition();
        int vScrollOffset = verticalScrollBar()->sliderPosition();

        if (hScrollOffset > 0)
        {
            dirtyRegionRect.moveLeft(hScrollOffset);
        }

        if (vScrollOffset > 0)
        {
            dirtyRegionRect.moveBottom(vScrollOffset);
        }

        setDirtyRegion(dirtyRegionRect);

        // Repaint me:
        QWidget* pViewPort = this->viewport();

        if (NULL != pViewPort)
        {
            pViewPort->update();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        acImageManager::calculateImagesLayout
// Description: Calculates the sizing of the current images in the manager
// Author:      Sigal Algranaty
// Date:        4/7/2012
// ---------------------------------------------------------------------------
void acImageManager::calculateImagesLayout()
{
    if ((m_managerMode == AC_MANAGER_MODE_STANDARD_ITEM) || (m_managerMode == AC_MANAGER_MODE_CUBEMAP_TEXTURE))
    {
        GT_IF_WITH_ASSERT(m_pImagesModel != NULL)
        {
            GT_IF_WITH_ASSERT((m_pImagesModel->rowCount() >= 1) && (m_pImagesModel->columnCount() >= 1))
            {
                // Get the index for the single object:
                QModelIndex index = m_pImagesModel->index(0, 0);

                if (m_managerMode == AC_MANAGER_MODE_CUBEMAP_TEXTURE)
                {
                    index = m_pImagesModel->index(1, 0);
                }

                // Get the size for a single object (visible rectangle):
                QRect itemRect = itemVisibleRect(index);

                // Get the item:
                acImageItem* pImageItem = getItem(index);
                GT_IF_WITH_ASSERT(pImageItem != NULL)
                {
                    // Get the object calculated QImage item:
                    QImage* pImage = pImageItem->asQImage();
                    GT_IF_WITH_ASSERT(pImage != NULL)
                    {
                        // Set the image as the item rect for the beginning of the calculation:
                        QSize OriginalImageSize = pImageItem->originalImageSize();
                        QRect rectOriginalImage(0, 0, OriginalImageSize.width(), OriginalImageSize.height());// itemRect;
                        m_imageVisibleRect = itemRect;
                        // Calculate the rectangle size of the top label:
                        gtString itemTopLabel = pImageItem->topLabel();

                        if (itemTopLabel.isEmpty())
                        {
                            // Set the actual image rectangle:
                            m_imageActualRect = m_imageVisibleRect;
                            // If the image is bigger then the visual rectangle:
                            int nonDisplayedImageW = pImageItem->zoomedImageSize().width() - m_imageVisibleRect.width();
                            int nonDisplayedImageH = pImageItem->zoomedImageSize().height() - m_imageVisibleRect.height();

                            if (nonDisplayedImageH > 0)
                            {
                                m_imageActualRect.setBottom(m_imageActualRect.bottom() + nonDisplayedImageH);
                            }

                            if (nonDisplayedImageW > 0)
                            {
                                m_imageActualRect.setLeft(m_imageActualRect.left() - nonDisplayedImageW / 2);
                                m_imageActualRect.setWidth(m_imageActualRect.width() + nonDisplayedImageW / 2);
                            }

                            m_isImageVisibleRectCalculated = true;
                        }
                        else
                        {
                            QFont font = (qobject_cast<QWidget*>(this))->font();

                            if (font.pointSize() > 0)
                            {
                                QFont originalFont = font;

                                font.setBold(true);
                                font.setPointSize(font.pointSize() + 1);
                                setFont(font);
                                QRect topTextBoundingRect = QFontMetrics(font).boundingRect(itemTopLabel.asASCIICharArray());
                                int textLeft = ((itemRect.size().width() - topTextBoundingRect.width()) / 2) + itemRect.topLeft().x();
                                QPoint textPosition(textLeft, AC_IMAGE_MANAGER_TOP_MARGIN + itemRect.topLeft().y());

                                // Draw the top label:
                                QTextOption textOptions;
                                textOptions.setAlignment(Qt::AlignTop | Qt::AlignHCenter);
                                textOptions.setWrapMode(QTextOption::WordWrap);
                                QRect textRect(QPoint(itemRect.x(), AC_IMAGE_MANAGER_TOP_MARGIN + itemRect.topLeft().y()), QSize(itemRect.width(), topTextBoundingRect.height()));

                                // Output the text bottom coordinate:
                                int textBottom = textRect.bottom();
                                // Restore the font:
                                setFont(originalFont);

                                // Calculate the image rectangle:
                                // Get the image X and Y positions:
                                int imageYPos = textBottom + AC_IMAGE_MANAGER_IMAGE_TO_LABEL_MARGIN;
                                int imageXPos = itemRect.left() + AC_IMAGE_MANAGER_IMAGE_TO_LABEL_MARGIN;
                                m_imageVisibleRect.setTop(imageYPos);
                                m_imageVisibleRect.setLeft(imageXPos);

                                // Check if image should be centered:
                                int imageX = m_imageVisibleRect.left();
                                int imageRectW = m_imageVisibleRect.width();
                                int imageRealW = pImage->width();
                                int imageXDiff = imageRectW - imageRealW;

                                if (imageXDiff > 0)
                                {
                                    imageX += imageXDiff / 2;
                                }

                                // That is the base position for the image Y coordinate:
                                int imageY = m_imageVisibleRect.top();

                                int imageMaxH = itemRect.height();
                                int imageMaxW = itemRect.width();

                                // If the image is shorter, position it in the middle:
                                int imageRealH = pImage->height();
                                int imageHeightDiff = imageMaxH - imageRealH;

                                if (imageHeightDiff > 0)
                                {
                                    imageY += imageHeightDiff / 2;
                                }

                                m_imageVisibleRect.setWidth(imageMaxW);
                                m_imageVisibleRect.setHeight(imageMaxH);


                                // If the image is bigger then the visual rectangle:
                                int nonDisplayedImageW = pImageItem->zoomedImageSize().width() - m_imageVisibleRect.width();
                                int nonDisplayedImageH = pImageItem->zoomedImageSize().height() - m_imageVisibleRect.height();

                                if (nonDisplayedImageH > 0)
                                {
                                    m_imageActualRect.setBottom(m_imageActualRect.bottom() + nonDisplayedImageH);
                                }

                                if (nonDisplayedImageW > 0)
                                {
                                    m_imageActualRect.setLeft(m_imageActualRect.left() - nonDisplayedImageW / 2);
                                    m_imageActualRect.setWidth(m_imageActualRect.width() + nonDisplayedImageW / 2);
                                }

                                m_isImageVisibleRectCalculated = true;

#pragma message ("TODO: images and buffers: Enlarge items with no bottom label")
                            }
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::setObjectToolTip
// Description: Sets an object's tooltip, according to the active mode
// Arguments:   canvasID - The item to set it tooltip canvas ID
//              toolTipText - The tool tip text to set to the item
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        14/6/2007
// ---------------------------------------------------------------------------
bool acImageManager::setObjectToolTip(acImageItemID imageItemID, const gtString& toolTipText)
{
    bool retVal = false;

    // Get the canvas item
    acImageItem* pImageItem = getItem(imageItemID);
    GT_IF_WITH_ASSERT(pImageItem != NULL)
    {
        // If we are in thumbnail view, add a tool tip to the clickable image
        if (m_managerMode == AC_MANAGER_MODE_THUMBNAIL_VIEW)
        {
            // Set the texture tool tip:
            pImageItem->setToolTipText(toolTipText);
        }
        else
        {
            pImageItem->setToolTipText(toolTipText);
        }

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::getItemActionsByFormat
// Description: Given an item data format, return the item possible actions mask
// Arguments:   oaTexelDataFormat dataFormat
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/4/2010
// ---------------------------------------------------------------------------
bool acImageManager::getItemActionsByFormat(oaTexelDataFormat dataFormat, unsigned int& imageActionsEnabled)
{
    bool retVal = false;

    imageActionsEnabled = 0;

    if (dataFormat == OA_TEXEL_FORMAT_UNKNOWN)
    {
        // No need to throw an exception, it is handled in many other places:
        imageActionsEnabled |= AC_IMAGE_CHANNEL_RED;
        imageActionsEnabled |= AC_IMAGE_CHANNEL_GREEN;
        imageActionsEnabled |= AC_IMAGE_CHANNEL_BLUE;
        imageActionsEnabled |= AC_IMAGE_CHANNEL_ALPHA;
        imageActionsEnabled |= AC_IMAGE_INVERT_FILTER;
        imageActionsEnabled |= AC_IMAGE_GRAYSCALE_FILTER;

        retVal = true;
    }
    else
    {
        // Invert and grayscale are always enabled:
        imageActionsEnabled |= AC_IMAGE_INVERT_FILTER;
        imageActionsEnabled |= AC_IMAGE_GRAYSCALE_FILTER;

        // Get amount of data components in data format
        int amountOfComponents = oaAmountOfTexelFormatComponents(dataFormat);
        GT_IF_WITH_ASSERT(amountOfComponents != -1)
        {
            retVal = true;

            // Loop through the channels
            for (int i = 0; i < amountOfComponents; i++)
            {
                // Get current channel type
                oaTexelDataFormat componentFormat = oaGetTexelFormatComponentType(dataFormat, i);

                switch (componentFormat)
                {
                    case OA_TEXEL_FORMAT_STENCIL:
                    case OA_TEXEL_FORMAT_DEPTH:
                    case OA_TEXEL_FORMAT_VARIABLE_VALUE:
                        break;

                    case OA_TEXEL_FORMAT_LUMINANCE:
                    case OA_TEXEL_FORMAT_INTENSITY:
                        imageActionsEnabled |= AC_IMAGE_CHANNEL_RED;
                        imageActionsEnabled |= AC_IMAGE_CHANNEL_GREEN;
                        imageActionsEnabled |= AC_IMAGE_CHANNEL_BLUE;
                        break;

                    case OA_TEXEL_FORMAT_COLORINDEX:
                        imageActionsEnabled |= AC_IMAGE_CHANNEL_RED;
                        imageActionsEnabled |= AC_IMAGE_CHANNEL_GREEN;
                        imageActionsEnabled |= AC_IMAGE_CHANNEL_BLUE;
                        imageActionsEnabled |= AC_IMAGE_CHANNEL_ALPHA;
                        break;

                    case OA_TEXEL_FORMAT_RED:
                        imageActionsEnabled |= AC_IMAGE_CHANNEL_RED;
                        break;

                    case OA_TEXEL_FORMAT_GREEN:
                        imageActionsEnabled |= AC_IMAGE_CHANNEL_GREEN;
                        break;

                    case OA_TEXEL_FORMAT_BLUE:
                        imageActionsEnabled |= AC_IMAGE_CHANNEL_BLUE;
                        break;

                    case OA_TEXEL_FORMAT_ALPHA:
                        imageActionsEnabled |= AC_IMAGE_CHANNEL_ALPHA;
                        break;

                    default:
                        retVal = false;
                        break;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::writeTextMessage
// Description: Writes a message (and an headline) in the image manager
// Arguments:   headline - Message header (will be displayed in red)
//              message - Message content (will be displayed in white)
// Author:      Sigal Algranaty
// Date:        7/11/2010
// ---------------------------------------------------------------------------
void acImageManager::writeTextMessage(const gtString& headline, const gtString& message, const QColor& titleColor)
{
    // Set the manager mode:
    m_managerMode = AC_MANAGER_MODE_TEXT_ITEM;

    // Get the first image item:
    acImageItem* pImageItem = NULL;

    if (m_imageItems.size() > 0)
    {
        pImageItem = getItem(0);
    }

    if (pImageItem == NULL)
    {
        // Create a new thumbnail item:
        pImageItem = new acImageItem(AC_IMAGE_MANAGER_ID_NONE, 0, AC_IMAGES_MANAGER_THUMBNAIL_SIZE);


        // Add the image item into the manager
        gtAutoPtr<acImageItem> aptrAdvanecdImageItem = pImageItem;

        // Get the added image ID
        acImageItemID imageItemID = addItem(aptrAdvanecdImageItem);

        // Set the image item canvas item id:
        pImageItem->setItemId(imageItemID);
    }

    // Set the text parameters:
    pImageItem->setDisplayedText(message, headline, titleColor);

    // Repaint the window:
    forceImagesRepaint();
}


// ---------------------------------------------------------------------------
// Name:        acImageManager::resizeEvent
// Description:
// Arguments:   QResizeEvent* pResizeEvent
// Author:      Sigal Algranaty
// Date:        19/6/2012
// ---------------------------------------------------------------------------
void acImageManager::resizeEvent(QResizeEvent* pResizeEvent)
{
    // Re-calculate the cells dimensions:
    updateCellDimensions();
    // Call the base class implementation:
    QTableView::resizeEvent(pResizeEvent);
}


// ---------------------------------------------------------------------------
// Name:        acImageManager::getImageVisibleRect
// Description: Get the calculated image visible rect
// Arguments:   QRect& rect
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        4/7/2012
// ---------------------------------------------------------------------------
bool acImageManager::getImageVisibleRect(QRect& rect)
{
    bool retVal = m_isImageVisibleRectCalculated;

    if (retVal)
    {
        rect = m_imageVisibleRect;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acImageManager::loadCubemapImageItem
// Description: Add a cubemap item
// Arguments:   const QPoint& matrixPos
// Return Val:  acImageItem*
// Author:      Sigal Algranaty
// Date:        5/7/2012
// ---------------------------------------------------------------------------
acImageItemID acImageManager::loadCubemapImageItem(QImage* pImage, const QPoint& matrixPos, unsigned int imagePossibleActionsMask)
{
    // Only these items are shown:
    // bool isRealItem = ((matrixPos.x() == 1) || (matrixPos.y() == 1));

    // Get the item index:
    QModelIndex index = model()->index(matrixPos.x(), matrixPos.y());
    acImageItemID itemIndex = modelIndexToImageIndex(index);
    acImageItemID retVal = AC_IMAGE_MANAGER_ID_NONE;

    // Get the item for the specific position:
    acImageItem* pImageItem = getItem(itemIndex);

    GT_IF_WITH_ASSERT(pImageItem != NULL)
    {
        // Set the item actions:
        pImageItem->setImageItemActions(imagePossibleActionsMask);


        // Load standard bitmap file into manager
        bool rc = pImageItem->loadFromQImage(pImage);
        GT_IF_WITH_ASSERT(rc)
        {
            pImageItem->setItemId(itemIndex);
            pImageItem->setIsEmpty(false);
            retVal = pImageItem->itemId();
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acImageManager::prepareCubemapLayout
// Description: Add in advance all the items for the cubemap object
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/7/2012
// ---------------------------------------------------------------------------
void acImageManager::prepareCubemapLayout()
{
    for (int i = 0; i < 12; i++)
    {
        acImageItemID imageItemID = AC_IMAGE_MANAGER_ID_NONE;

        // Add 2 dummy items, then the item, then another one:
        acImageItem* pDummyImageItem = new acImageItem(imageItemID, 0, AC_IMAGE_ITEM_NOT_THUMBNAIL);

        pDummyImageItem->setIsEmpty();

        // Add the image item into the manager
        gtAutoPtr<acImageItem> aptrAdvanecdImageItem1 = pDummyImageItem;
        // Get the added image ID
        imageItemID = addItem(aptrAdvanecdImageItem1);
    }
}


// ---------------------------------------------------------------------------
// Name:        acImageManager::updateData
// Description: Update the data for the table
// Author:      Sigal Algranaty
// Date:        9/7/2012
// ---------------------------------------------------------------------------
void acImageManager::updateData()
{
    // Get the relevant data model:
    QAbstractItemModel* pModel = model();

    if (pModel != NULL)
    {
        // Emit a datqa changed signal:
        QModelIndex indexTop = pModel->index(0, 0);
        QModelIndex indexBottom = pModel->index(pModel->rowCount(), pModel->columnCount());

        emit dataChanged(indexTop, indexBottom);
    }

    // Repaint the table:
    setDirtyRegion(visibleRegion()); QTableView::repaint();
}


// ---------------------------------------------------------------------------
// Name:        acImageManager::updateData
// Description: Update the table cell dimensions - calculate the correct cell
//              dimensions for the current manager content
// Author:      Sigal Algranaty
// Date:        10/7/2012
// ---------------------------------------------------------------------------
void acImageManager::updateCellDimensions()
{
    GT_IF_WITH_ASSERT(m_pImagesModel != NULL)
    {
        if (m_imageItems.size() > 0)
        {
            // Get the current row & column count:
            int colAmount = horizontalHeader()->count();
            int rowAmount = verticalHeader()->count();

            // Set the row and column height according to the manager type:
            int rowH = AC_IMAGES_MANAGER_THUMBNAIL_SIZE + AC_IMAGE_MANAGER_MARGIN * 2;
            int rowW = AC_IMAGES_MANAGER_THUMBNAIL_SIZE + AC_IMAGES_MANAGER_THUMBNAIL_MARGIN * 2;

            if ((m_managerMode == AC_MANAGER_MODE_STANDARD_ITEM) || (m_managerMode == AC_MANAGER_MODE_TEXT_ITEM))
            {
                rowH = size().height() - AC_IMAGES_MANAGER_THUMBNAIL_MARGIN;
                rowW = size().width() - AC_IMAGES_MANAGER_THUMBNAIL_MARGIN;
                rowW = max(rowW, m_imageActualRect.width() + AC_IMAGE_MANAGER_MARGIN);
                rowH = max(rowH, m_imageActualRect.height() + AC_IMAGE_MANAGER_MARGIN);
            }
            else if (m_managerMode == AC_MANAGER_MODE_CUBEMAP_TEXTURE)
            {
                // Get the item:
                GT_IF_WITH_ASSERT(m_pImagesModel != NULL)
                {
                    QModelIndex index = m_pImagesModel->index(1, 0);
                    acImageItem* pImageItem = getItem(index);
                    GT_IF_WITH_ASSERT(pImageItem != NULL)
                    {
                        QImage* pImage = pImageItem->asQImage();
                        GT_IF_WITH_ASSERT(pImage != NULL)
                        {
                            QSize zoomedSize = pImageItem->zoomedImageSize();
                            rowH = max(rowH, zoomedSize.height() + AC_IMAGE_MANAGER_MARGIN * 2);
                            rowW = max(rowW, zoomedSize.width() + AC_IMAGES_MANAGER_THUMBNAIL_MARGIN * 2);
                        }
                    }
                }
            }

            // Set the rows width:
            for (int i = 0; i < rowAmount; i++)
            {
                setRowHeight(i, rowH);
                verticalHeader()->setSectionResizeMode(i, QHeaderView::Interactive);
            }

            // Set the columns width:
            for (int i = 0; i < colAmount; i++)
            {
                setColumnWidth(i, rowW);
                horizontalHeader()->setSectionResizeMode(i, QHeaderView::Interactive);
            }

            // Update the model:
            m_pImagesModel->updateModel();

            if (size() != m_previousViewSize)
            {
                // Re-calculate the image visible rectangle:
                m_isImageVisibleRectCalculated = false;
                calculateImagesLayout();
                m_previousViewSize = size();

                // Re-set the model in order to calculate the column and rows amount:
                int rowAmount1 = verticalHeader()->count();
                int neededRowAmount = m_pImagesModel->rowCount();
                GT_IF_WITH_ASSERT(m_pVerticalHeader != NULL)
                {
                    m_pVerticalHeader->emitSectionCountChanged(rowAmount1, neededRowAmount);
                }
            }

            horizontalScrollBar()->updateGeometry();
            verticalScrollBar()->updateGeometry();

            // When there is a single item, we need to fix the scrollbars, since Qt sometimes add scrollbars when
            // there's no need. Other views scroll bar will not need to be fixed
            if (m_managerMode == AC_MANAGER_MODE_STANDARD_ITEM)
            {
                FixScrollbarsAppearance();
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        acImageManager::itemVisibleRect
// Description: Return a rectangle with the visible area for a certain object
//              For thumbnails, it is simply the visualRect, but for single items,
//              we want the visible rect (without the scroll current range)
// Arguments:   const QModelIndex& index
// Return Val:  QRect
// Author:      Sigal Algranaty
// Date:        12/7/2012
// ---------------------------------------------------------------------------
QRect acImageManager::itemVisibleRect(const QModelIndex& index)
{
    QRect retVal = visualRect(index);

    if ((m_managerMode == AC_MANAGER_MODE_STANDARD_ITEM) || (m_managerMode == AC_MANAGER_MODE_TEXT_ITEM))
    {
        int w = width();
        int h = height();
        int minW = min(retVal.width(), w);// width());
        int minH = min(retVal.height(), h);// height());
        retVal.setSize(QSize(minW, minH));
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::setActiveTool
// Description: Set the active tool (pan / standard)
// Arguments:   acImageManagerToolType tool
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        15/7/2012
// ---------------------------------------------------------------------------
void acImageManager::setActiveTool(acImageManagerToolType tool)
{
    if (tool != m_currentTool)
    {
        m_currentTool = tool;

        if (m_currentTool == AC_IMAGE_MANAGER_TOOL_PAN)
        {
            setCursor(Qt::ClosedHandCursor);
        }
        else
        {
            // Set standard cursor:
            setCursor(QCursor(Qt::ArrowCursor));
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::sendPixelInformation
// Description: Sends a pixel information event
// Arguments:   const QModelIndex& mouseHoverItem
// Author:      Sigal Algranaty
// Date:        15/7/2012
// ---------------------------------------------------------------------------
void acImageManager::sendPixelInformation(const QModelIndex& mouseHoverItem, const QPoint& mousePos)
{
    // Get the item for this index:
    acImageItem* pImageItem = getItem(mouseHoverItem);

    if (pImageItem != NULL)
    {
        // Get the image item:
        GT_IF_WITH_ASSERT(pImageItem != NULL)
        {
            // Check if the image real rectangle is hit by this mouse position:
            if (pImageItem->imageBoundingRect().contains(mousePos))
            {
                // Check if the mouse position is contained in the image rectangle:
                onImageStatusChanged(mouseHoverItem, mousePos, false);
            }
            else
            {
                // Send the status changed with an invalid model index (so that the control panel is cleared):
                onImageStatusChanged(QModelIndex(), QPoint(-1, -1), false);
            }

            if (m_managerMode == AC_MANAGER_MODE_THUMBNAIL_VIEW)
            {
                // Set the item tooltip && cursor:
                setCursor(QCursor(Qt::PointingHandCursor));
                setToolTip(pImageItem->toolTipText().asASCIICharArray());
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::panImageItem
// Description: Perform a pan operation on an image item
// Arguments:   const QModelIndex& mouseHoverItem
// Author:      Sigal Algranaty
// Date:        15/7/2012
// ---------------------------------------------------------------------------
void acImageManager::panImageItem(const QModelIndex& mouseHoverItem, const QPoint& mousePos, bool isLeftButtonDown)
{
    GT_UNREFERENCED_PARAMETER(mouseHoverItem);

    // Check if we got an invalid mouse position
    if (m_lastMousePosition == AC_IMAGE_MANAGER_MOUSE_OUTSIDE_MANAGER)
    {
        // Set the last mouse position to the current mouse position
        m_lastMousePosition = mousePos;
    }


    // If left mouse button is down, and mouse position had changed
    if (isLeftButtonDown && (mousePos != m_lastMousePosition))
    {
        // How much did we progress?
        int hDelta = m_lastMousePosition.x() - mousePos.x();
        int vDelta = m_lastMousePosition.y() - mousePos.y();

        // Convert the xDelta and yDelta to slider new positions:
        int sliderHPos = horizontalScrollBar()->sliderPosition();
        int sliderVPos = verticalScrollBar()->sliderPosition();

        // Set the slider new positions:

        horizontalScrollBar()->setSliderPosition(sliderHPos + hDelta);
        verticalScrollBar()->setSliderPosition(sliderVPos + vDelta);

    }
}

void acImageManager::FixScrollbarsAppearance()
{
    GT_IF_WITH_ASSERT(m_pImagesModel != NULL)
    {
        GT_IF_WITH_ASSERT((m_pImagesModel->rowCount() >= 1) && (m_pImagesModel->columnCount() >= 1))
        {
            // Get the index for the single object:
            QModelIndex index = m_pImagesModel->index(0, 0);

            if (m_managerMode == AC_MANAGER_MODE_CUBEMAP_TEXTURE)
            {
                index = m_pImagesModel->index(1, 0);
            }

            // Get the size for a single object (visible rectangle):
            QRect itemRect = itemVisibleRect(index);

            // Get the item:
            acImageItem* pImageItem = getItem(index);
            GT_IF_WITH_ASSERT(pImageItem != NULL)
            {
                QSize sizeHSB = horizontalScrollBar()->size();
                QSize sizeVSB = verticalScrollBar()->size();
                QString strVSBStyleSheet, strHSBStyleSheet;
                strVSBStyleSheet.sprintf("QScrollBar {width:%dpx;}", sizeVSB.width());
                strHSBStyleSheet.sprintf("QScrollBar {height:%dpx;}", sizeHSB.height());

                if (itemRect.width() - AC_IMAGE_MANAGER_SCROLLBAR_MARGIN > pImageItem->zoomedImageSize().width())
                {
                    horizontalScrollBar()->hide();
                    horizontalScrollBar()->setStyleSheet("QScrollBar {height:0px;}");
                }
                else
                {
                    horizontalScrollBar()->show();
                    horizontalScrollBar()->setStyleSheet(strHSBStyleSheet);
                }

                if (itemRect.height() - AC_IMAGE_MANAGER_SCROLLBAR_MARGIN > pImageItem->zoomedImageSize().height())
                {
                    verticalScrollBar()->hide();
                    verticalScrollBar()->setStyleSheet("QScrollBar {width:0px;}");
                }
                else
                {
                    verticalScrollBar()->show();
                    verticalScrollBar()->setStyleSheet(strVSBStyleSheet);
                }
            }
        }
    }
}

bool acImageManager::eventFilter(QObject*, QEvent* pEvt)
{
    // return false to continue event propagation
    // for all events
    bool retVal = false;
    GT_IF_WITH_ASSERT(NULL != pEvt)
    {
        if (pEvt->type() == QEvent::Wheel)
        {
            // ignore the event if Ctrl button is pressed
            if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) == true)
            {
                pEvt->ignore();
                retVal = true;
            }
        }
    }
    return retVal;
}

void acImageManager::wheelEvent(QWheelEvent* pEvt)
{
    GT_IF_WITH_ASSERT(NULL != pEvt)
    {
        // if you handle the event and don't want it to
        // propagate any further, accept it:
        QScrollBar* pVScrollBar = verticalScrollBar();

        if (NULL != pVScrollBar && !pVScrollBar->isVisible())
        {
            pEvt->accept();
        }
        else
        {
            // if vertical scroll bar is visible enable scrolling with wheel
            QTableView::wheelEvent(pEvt);
        }
    }
}