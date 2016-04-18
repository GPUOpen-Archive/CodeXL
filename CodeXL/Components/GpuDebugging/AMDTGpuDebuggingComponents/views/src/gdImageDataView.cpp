//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdImageDataView.cpp
///
//==================================================================================

//------------------------------ gdImageDataView.cpp ------------------------------

// Qt:
#include <QtWidgets>

#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>

// Infra:
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acCommandIDs.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acQtIncludes.h>
#include <AMDTApplicationComponents/Include/acRawDataExporter.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apHexChangedEvent.h>
#include <AMDTApplicationComponents/Include/acImageItemDelegate.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdImagesAndBuffersManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdUpdateUIEvent.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImageDataView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImagesAndBuffersControlPanel.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImagesAndBuffersExporter.h>

// Image control panel default height:
#define GD_IMAGES_AND_BUFFERS_MAXIMUM_MANAGER_SIZE 10000
#define GD_IMAGES_AND_BUFFERS_CONTROL_PANEL_MIN_WIDTH 400
#define GD_IMAGES_AND_BUFFERS_DEFAULT_ZOOM_LEVEL 100
#define GD_IMAGES_AND_BUFFERS_MARGIN 50

// Static members initialization:
gtVector<unsigned int> gdImageDataView::_stat_availableZoomLevelsVector;
afApplicationTreeItemData* gdImageDataView::_pStaticEmptyItemData = nullptr;
// ---------------------------------------------------------------------------
// Name:        gdImageDataView::gdImageDataView
// Description: Constructor
// Arguments:   pParent - The view's parent
// Author:      Sigal Algranaty
// Date:        8/11/2010
// ---------------------------------------------------------------------------
gdImageDataView::gdImageDataView(QWidget* pParent, afProgressBarWrapper* pProgressBar)
    : QWidget(pParent), afBaseView(pProgressBar),
      _pImageViewManager(nullptr), _pDataViewManager(nullptr), m_pTabWidget(nullptr), _pMainLayout(nullptr),
      _previousSize(-1, -1), _pImageControlPanel(nullptr), _ignoreSelections(false), _pDisplayedItemTreeData(nullptr),
      _currentZoomLevel(0), m_bFlipImage(false), m_nSelectedPixelImageItemID(-1), m_pSelectedPixelImageItem(nullptr),
      m_pointSelectedPixelPosition(QPoint()), m_bPixelInfoSelected(false)
{

    // Add the cube map faces names:
    _dataViewPagesNames.push_back(GD_STR_ImagesAndBuffersViewerCubeMapPositiveX);
    _dataViewPagesNames.push_back(GD_STR_ImagesAndBuffersViewerCubeMapNegativeX);
    _dataViewPagesNames.push_back(GD_STR_ImagesAndBuffersViewerCubeMapPositiveY);
    _dataViewPagesNames.push_back(GD_STR_ImagesAndBuffersViewerCubeMapNegativeY);
    _dataViewPagesNames.push_back(GD_STR_ImagesAndBuffersViewerCubeMapPositiveZ);
    _dataViewPagesNames.push_back(GD_STR_ImagesAndBuffersViewerCubeMapNegativeZ);

    // Initialize display properties:
    _currentViewsDisplayProperties._viewState = GD_UNINITIALIZED_VIEW_STATE;
    _currentViewsDisplayProperties._wasSplitProgrammaticallyDeleted = false;
    _currentViewsDisplayProperties._lastSelectedPageIndex = -1;
    _currentViewsDisplayProperties._forceControlPanelDisplay = false;

    if (_stat_availableZoomLevelsVector.size() == 0)
    {
        // Initialize the available zoom levels:
        _stat_availableZoomLevelsVector.push_back(5);
        _stat_availableZoomLevelsVector.push_back(10);
        _stat_availableZoomLevelsVector.push_back(25);
        _stat_availableZoomLevelsVector.push_back(33);
        _stat_availableZoomLevelsVector.push_back(40);
        _stat_availableZoomLevelsVector.push_back(50);
        _stat_availableZoomLevelsVector.push_back(75);
        _stat_availableZoomLevelsVector.push_back(90);
        _stat_availableZoomLevelsVector.push_back(100);
        _stat_availableZoomLevelsVector.push_back(150);
        _stat_availableZoomLevelsVector.push_back(200);
        _stat_availableZoomLevelsVector.push_back(300);
        _stat_availableZoomLevelsVector.push_back(400);
        _stat_availableZoomLevelsVector.push_back(500);
        _stat_availableZoomLevelsVector.push_back(600);
        _stat_availableZoomLevelsVector.push_back(700);
        _stat_availableZoomLevelsVector.push_back(800);
        _stat_availableZoomLevelsVector.push_back(900);
        _stat_availableZoomLevelsVector.push_back(1000);
    }

    // Create the item data used for displaying "dummy" items:
    if (_pStaticEmptyItemData == nullptr)
    {
        _pStaticEmptyItemData = new afApplicationTreeItemData;
        gdDebugApplicationTreeData* pGDData = new gdDebugApplicationTreeData;
        _pStaticEmptyItemData->setExtendedData(pGDData);
    }

    // Set the erase back ground to mode to correctly draw the background when needed
    setAutoFillBackground(true);
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::~gdImageDataView
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        8/11/2010
// ---------------------------------------------------------------------------
gdImageDataView::~gdImageDataView()
{
}

void gdImageDataView::UpdateTextureHeading(gdDebugApplicationTreeData* pGDData, int newMipLevel)
{
    GT_UNREFERENCED_PARAMETER(pGDData);
    GT_UNREFERENCED_PARAMETER(newMipLevel);
    GT_ASSERT_EX(false, L"Should not get here");
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::clearView
// Description: Clear all my components
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        10/11/2010
// ---------------------------------------------------------------------------
void gdImageDataView::clearView()
{
    // Clear the image manager items
    GT_IF_WITH_ASSERT(_pImageViewManager != nullptr)
    {
        // Clear objects:
        _pImageViewManager->clearAllObjects();

        // Set the image manager mode to standard layout:
        _pImageViewManager->setManagerMode(AC_MANAGER_MODE_TEXT_ITEM);

        // Clear the texture manager tooltip
        _pImageViewManager->setToolTip(AF_STR_EmptyA);

        // Reset the cursor to default:
        _pImageViewManager->setCursor(Qt::ArrowCursor);
    }

    // Clear the data view manager
    GT_IF_WITH_ASSERT(_pDataViewManager != nullptr)
    {
        _pDataViewManager->clearGrid();
    }

    GT_IF_WITH_ASSERT(_pImageControlPanel != nullptr)
    {
        // Clear the pixel information from the image information panel
        _pImageControlPanel->clearControlPanel(!m_bPixelInfoSelected);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::setFrameLayout
// Description: Sets the single view frame layout
// Arguments:   const QSize& viewSize
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/11/2010
// ---------------------------------------------------------------------------
bool gdImageDataView::setFrameLayout(const QSize& viewSize)
{
    (void)(viewSize);  // unused
    bool retVal = false;

    // Ignore the notebook selections:
    _ignoreSelections = true;

    // Create the AUI notebook:
    m_pTabWidget = new acTabWidget;
    m_pTabWidget->setTabsClosable(false);

    // Connect key down event handler for current component:
    bool rc = connect(m_pTabWidget, SIGNAL(currentChanged(int)), this, SLOT(onNoteBookPageChange(int)));
    GT_ASSERT(rc);

    // Create the Image and Data viewers:
    _pImageViewManager = new acImageManager(m_pTabWidget, AC_MANAGER_MODE_STANDARD_ITEM);

    // Installing event filter to the viewport widget
    GT_IF_WITH_ASSERT(_pImageViewManager->viewport() != nullptr)
    {
        _pImageViewManager->viewport()->installEventFilter(_pImageViewManager);
    }

    // Set the manager mode to default:
    _pImageViewManager->setManagerMode(AC_MANAGER_MODE_STANDARD_ITEM);

    // Add the image view to the AUI notebook:
    m_pTabWidget->addTab(_pImageViewManager, GD_STR_ImagesAndBuffersViewerImageViewerCaption);

    Qt::WindowFlags flags = _pImageViewManager->windowFlags();
    flags ^= Qt::WindowCloseButtonHint;
    _pImageViewManager->setWindowFlags(flags);

    // Connect the pixel change signal of the data view:
    rc = connect(_pImageViewManager, SIGNAL(pixelPositionChanged(acImageItemID, const QPoint&, bool, bool)), this, SLOT(onImageItemEvent(acImageItemID, const QPoint&, bool, bool)));
    GT_ASSERT(rc);

    // Create the data view manager:
    _pDataViewManager = new acDataView(m_pTabWidget, _dataViewPagesNames);
    flags = _pDataViewManager->windowFlags();
    flags ^= Qt::WindowCloseButtonHint;
    _pDataViewManager->setWindowFlags(flags);

    // Connect the pixel change signal of the data view:
    rc = connect(_pDataViewManager, SIGNAL(pixelPositionChanged(acImageItemID, const QPoint&, bool, bool)), this, SLOT(onDataPixelMouseEvent(acImageItemID, const QPoint&, bool, bool)));
    GT_ASSERT(rc);

    // Connect the pixel change signal of the data view:
    rc = connect(_pDataViewManager, SIGNAL(hexChanged(bool)), this, SLOT(onHexChanged(bool)));
    GT_ASSERT(rc);

    //Connect zoom change to zoom related parameters ( currently for first time best fit ZoomLevel )
    rc = connect(_pImageViewManager, SIGNAL(zoomChanged(int)), this, SLOT(onZoomChanged(int)));
    GT_ASSERT(rc);

    // Add the data view to the AUI notebook:
    m_pTabWidget->addTab(_pDataViewManager, GD_STR_ImagesAndBuffersViewerDataViewerCaption);

    // Create the image / buffer control panel:
    _pImageControlPanel = new gdImagesAndBuffersControlPanel(this);

    // Create the main window sizer:
    _pMainLayout = new QHBoxLayout;

    // Add the views to the sizer:
    _pMainLayout->addWidget(m_pTabWidget, 1);
    _pMainLayout->addWidget(_pImageControlPanel, 0, Qt::AlignTop);

    _pMainLayout->setContentsMargins(0, 0, 0, 0);

    // activate
    setLayout(_pMainLayout);

    // Clear the view:
    clearView();

    // Set the manager mode to default:
    _pImageViewManager->setManagerMode(AC_MANAGER_MODE_STANDARD_ITEM);

    // Initialize Hex flag:
    _pDataViewManager->setHexDisplayMode(gaIsHexDisplayMode());

    // Cancel ignore of notebook selections:
    _ignoreSelections = false;

    layout()->activate();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::resizeEvent
// Description: Is called when the widget is resized
// Arguments:   QResizeEvent* pResizeEvent
// Author:      Sigal Algranaty
// Date:        27/6/2012
// ---------------------------------------------------------------------------
void gdImageDataView::resizeEvent(QResizeEvent* pResizeEvent)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((_pImageControlPanel != nullptr) && (_pDataViewManager != nullptr) && (pResizeEvent != nullptr))
    {
        // Apply best fit for images and cubemaps:
        if ((_pImageViewManager->managerMode() == AC_MANAGER_MODE_STANDARD_ITEM) || (_pImageViewManager->managerMode() == AC_MANAGER_MODE_CUBEMAP_TEXTURE))
        {
            if (_pImageViewManager->amountOfImagesInManager() > 0)
            {
                // Check if the size had changed since the last size change:
                bool wasSizeChanged = ((_previousSize.width() != pResizeEvent->size().width()) || (_previousSize.height() != pResizeEvent->size().height()));

                if (wasSizeChanged)
                {
                    // This is the first time that the window size is set, so apply best fit:
                    // applyBestFit(_currentZoomLevel);

                    // Set the current size:
                    _previousSize = pResizeEvent->size();
                }

                // If the size is too small, and the control panel is expanded, hide it:
                bool shouldControlPanelBeExpanded = (pResizeEvent->size().width() > GD_IMAGES_AND_BUFFERS_CONTROL_PANEL_MIN_WIDTH);

                // Set the panel "is expanded" flag:
                _pImageControlPanel->setIsExpanded(shouldControlPanelBeExpanded);
                _pDataViewManager->repaint();
            }
        }
    }

    // Call the base class implementation:
    QWidget::resizeEvent(pResizeEvent);
}


// ---------------------------------------------------------------------------
// Name:        gdImageDataView::applyLastViewedItemProperties
// Description: Apply the last viewed item properties
// Arguments:   bool applyZoom - should apply the last viewed item zoom settings
// Author:      Eran Zinman
// Date:        10/1/2008
// ---------------------------------------------------------------------------
void gdImageDataView::applyLastViewedItemProperties(const acDisplayedItemProperties& lastViewedItemProperties, bool applyZoom)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((_pImageViewManager != nullptr) && (_pDataViewManager != nullptr))
    {
        if ((_pImageViewManager->amountOfImagesInManager() > 0) && (_pImageViewManager->managerMode() != AC_MANAGER_MODE_TEXT_ITEM))
        {
            if (applyZoom)
            {
                // Set the zoom level:
                _pImageViewManager->setFilterForAllImages(AC_IMAGE_ZOOM, lastViewedItemProperties._zoomLevel, false);

                // Set image view manager scroll position:
                QModelIndex index = _pImageViewManager->model()->index(lastViewedItemProperties._position.x(), lastViewedItemProperties._position.y());
                _pImageViewManager->scrollTo(index);
            }

            // Set the image and data views rotation angle:
            _pImageViewManager->setFilterForAllImages(AC_IMAGE_ROTATE, lastViewedItemProperties._rotateAngle, false);
            _pDataViewManager->rotateDataView(lastViewedItemProperties._rotateAngle);

            // Apply RGBA filters to the data view:
            _pDataViewManager->applyImageActions(lastViewedItemProperties._actionsMask);

            // Apply RGBA filters to the images:
            _pImageViewManager->setFilterForAllImages(lastViewedItemProperties._actionsMask, 0.0 , false);
        }
    }

    // Copy properties to me:
    _lastViewedItemProperties = lastViewedItemProperties;
}


// ---------------------------------------------------------------------------
// Name:        gdImageDataView::saveLastViewedItemProperties
// Description: Saves the currently displayed item properties
// Arguments:   acDisplayedItemProperties& lastViewedItemProperties
//              bool applyZoom
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        8/11/2010
// ---------------------------------------------------------------------------
void gdImageDataView::saveLastViewedItemProperties(int currentZoomLevel)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pImageViewManager && _pDataViewManager)
    {
        // Get first item the raw data handler:
        acRawFileHandler* pRawDataHandler = _pDataViewManager->getRawDataHandler();

        if (pRawDataHandler != nullptr)
        {
            // Save current zoom level:
            _lastViewedItemProperties._zoomLevel = currentZoomLevel;

            // Get image view manager scroll position
            int xCoord = 0;
            int yCoord = 0;
            _pImageViewManager->getScrollPosition(xCoord, yCoord);

            // Save image view manager position
            _lastViewedItemProperties._position = QPoint(xCoord, yCoord);

            // Save item rotate angle:
            _lastViewedItemProperties._rotateAngle = 0;
            bool rc1 = _pDataViewManager->getRotationAngle(_lastViewedItemProperties._rotateAngle);
            GT_ASSERT(rc1);

            // Save currently active page
            _lastViewedItemProperties._activePage = pRawDataHandler->activePage();

            // Save is values normalized status
            _lastViewedItemProperties._isNormalized = pRawDataHandler->isNormalized();

            // If values are normalized, save min and max values
            if (_lastViewedItemProperties._isNormalized)
            {
                // Save minimum and maximum values
                pRawDataHandler->getMinMaxValues(_lastViewedItemProperties._minValue, _lastViewedItemProperties._maxValue);

                _lastViewedItemProperties._isNormalized = false;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::setActiveTool
// Description: Sets the image manager active tool (for Pan / Standard actions)
// Arguments:   acImageManagerToolType tool
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        8/11/2010
// ---------------------------------------------------------------------------
void gdImageDataView::setActiveTool(acImageManagerToolType tool)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pImageViewManager != nullptr)
    {
        _pImageViewManager->setActiveTool(tool);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::getActiveTool
// Description: Sets the image manager active tool (for Pan / Standard actions)
// Return Val:  acImageManagerToolType - the active tool
// Author:      Sigal Algranaty
// Date:        8/11/2010
// ---------------------------------------------------------------------------
acImageManagerToolType gdImageDataView::getActiveTool() const
{
    acImageManagerToolType retVal = AC_IMAGE_MANAGER_TOOL_STANDARD;
    // Sanity check:
    GT_IF_WITH_ASSERT(_pImageViewManager != nullptr)
    {
        retVal = _pImageViewManager->activeTool();
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::resetActiveTool
// Description: Resets the active tool
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        8/11/2010
// ---------------------------------------------------------------------------
void gdImageDataView::resetActiveTool()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pImageViewManager != nullptr)
    {
        _pImageViewManager->resetActiveTool();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::onStandardPointerTool
// Description: Standard Pointer Tool was selected
// Arguments:   event - Event details
// Author:      Eran Zinman
// Date:        21/5/2007
// ---------------------------------------------------------------------------
void gdImageDataView::onStandardPointerTool()
{
    // We only allow toggling this tool, not un-toggling it!
    // (un-toggling is done by selecting another tool from the same tool family)
    acImageManagerToolType currentTool = _pImageViewManager->activeTool();

    if (currentTool != AC_IMAGE_MANAGER_TOOL_STANDARD)
    {
        // Set the standard Tool
        _pImageViewManager->setActiveTool(AC_IMAGE_MANAGER_TOOL_STANDARD);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::isImageViewFocused
// Description: Return true iff the image view is focused (false if the data view is)
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/11/2010
// ---------------------------------------------------------------------------
bool gdImageDataView::isImageViewFocused() const
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((_pImageViewManager != nullptr) && (m_pTabWidget != nullptr))
    {
        // Get the image view index:
        int imageViewIndex = m_pTabWidget->indexOf(_pImageViewManager);

        if (imageViewIndex >= 0)
        {
            // Get the currently focused page
            int selectedPage = m_pTabWidget->currentIndex();

            // Is the currently focused page is the texture manager page?
            retVal = (selectedPage == imageViewIndex);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::isImageViewShown
// Description: Return true iff the image view is shown
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/11/2010
// ---------------------------------------------------------------------------
bool gdImageDataView::isImageViewShown() const
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pImageViewManager != nullptr)
    {
        retVal = _pImageViewManager->isVisible();
    }

    return retVal;

}


// ---------------------------------------------------------------------------
// Name:        gdImageDataView::clearCurrentPixelInformation
// Description: Clear the current pixel information from the control panel
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        10/11/2010
// ---------------------------------------------------------------------------
void gdImageDataView::clearCurrentPixelInformation()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((_pImageControlPanel != nullptr) && (_pImageViewManager != nullptr))
    {
        // Clear the current pixel information from the control panel:
        _pImageControlPanel->clearPixelInformation(false);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::setFilterForAllImages
// Description: Set the filter for all images
// Arguments:   unsigned int filter
//              double value
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        10/11/2010
// ---------------------------------------------------------------------------
void gdImageDataView::setFilterForAllImages(unsigned int filter, double value)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((_pImageViewManager != nullptr) && (_pDataViewManager != nullptr))
    {
        // Clear the current pixel information from the control panel:
        _pImageViewManager->setFilterForAllImages(filter, value);

        if (filter == AC_IMAGE_ROTATE)
        {
            // Rotate the data view 270 degrees to the right
            _pDataViewManager->rotateDataView(value);
        }
        else
        {
            if (filter != AC_IMAGE_ZOOM)
            {
                // Apply image actions for the data view:
                _pDataViewManager->applyImageActions(filter);

                // Set the last viewed item properties:
                _lastViewedItemProperties._actionsMask = filter;
            }
        }

        // Adjust the manager layout after performing the image actions:
        _pImageViewManager->forceImagesRepaint();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::updatePixelPosition
// Description: Update the pixel position for the image and data view
// Arguments:   int canvasId
//              acImageItem* pImageItem
//              QPoint posOnImage
//              bool leftButtonDown
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        10/11/2010
// ---------------------------------------------------------------------------
void gdImageDataView::updatePixelPosition(acImageItemID canvasId, acImageItem* pImageItem, QPoint posOnImage, bool leftButtonDown)
{
    if (canvasId != AC_IMAGE_MANAGER_ID_NONE)
    {
        QPoint PosOnOriginalImage;

        GT_IF_WITH_ASSERT(_pImageViewManager != nullptr)
        {
            // Update the single item view with the pixel position information:
            _pImageControlPanel->updateControlPanel(canvasId, pImageItem, posOnImage, leftButtonDown, true);
        }

        // On left click, update data viewer position
        if (leftButtonDown)
        {
            GT_IF_WITH_ASSERT(_pDataViewManager != nullptr)
            {
                //Save selected pixel data
                SaveSelectedPixelData(canvasId, pImageItem, posOnImage);
                zoomedImagePositionToImagePosition(m_pSelectedPixelImageItem, m_pointSelectedPixelPosition, PosOnOriginalImage, true, /*flip Y*/false);

                //_pDataViewManager->highlightPixelPosition(m_nSelectedPixelImageItemID, PosOnOriginalImage, true); // for future implementation of parallel image and data views
            }
        }
    }
    else
    {
        // Sanity check:
        GT_IF_WITH_ASSERT(_pImageControlPanel != nullptr)
        {
            // Clear the current pixel information:
            _pImageControlPanel->clearPixelInformation(false);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImageDataView::onDataPixelMouseEvent
// Description: Occurs when pixel position in the texture data view grid had changed
// Arguments:   acImageItemID imageItemID
//              const QPoint& posOnImage
//              bool mouseLeftDown
//              bool mouseDoubleClick
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        21/6/2012
// ---------------------------------------------------------------------------
void gdImageDataView::onDataPixelMouseEvent(acImageItemID imageItemID, const QPoint& posOnImage, bool mouseLeftDown, bool mouseDoubleClick)
{
    bool isImageItem = true;
    (void)(posOnImage);  // unused
    acRawFileHandler* pRawDataHandler = getCurrentRawDataHandler();

    if (pRawDataHandler != nullptr)
    {
        oaTexelDataFormat dataFormat = pRawDataHandler->dataFormat();
        isImageItem = !oaIsBufferTexelFormat(dataFormat);

        if (isImageItem && imageItemID != AC_IMAGE_MANAGER_ID_NONE)
        {
            // Get image item associated with this canvas ID
            acImageItem* pImageItem = (acImageItem*)_pImageViewManager->getItem(imageItemID);

            if (pImageItem != nullptr)
            {
                // Get the last grid pixel position
                QPoint gridPixelPosition = _pDataViewManager->getCurrentGridPixelPosition();

                if (gridPixelPosition != AC_DATA_VIEW_PIXEL_POSITION_NOT_IN_GRID)
                {
                    if (mouseDoubleClick)
                    {
                        // Handle double click:
                        bool rc = handleDataCellDoubleClick(imageItemID, pImageItem, gridPixelPosition);
                        GT_ASSERT(rc);
                    }
                    else
                    {
                        // Update the information panel
                        _pImageControlPanel->updateControlPanel(imageItemID, pImageItem, gridPixelPosition, mouseLeftDown, false);
                    }
                }
            }
            else
            {
                //information should be cleared
                isImageItem = false;
            }
        }
    }

    if (!isImageItem)
    {
        // Clear the current pixel information:
        _pImageControlPanel->clearPixelInformation(false);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::getBestFitPercaetage
// Description: Return the zoom percentage that will give us the "best fit" for
//              the item image view
//              zoom level
// Author:      Eran Zinman
// Date:        9/7/2007
// ---------------------------------------------------------------------------
int gdImageDataView::getBestFitPercentage()
{
    // By default - return 100%
    int retVal = 100;
    // Sanity check:
    GT_IF_WITH_ASSERT(_pImageViewManager != nullptr)
    {
        // Get the displayed image item:
        acImageItem* pImageItem = _pImageViewManager->getItem(0);
        GT_IF_WITH_ASSERT(pImageItem != nullptr)
        {
            // Get the manager client canvas size
            QSize imageAvailableSize = pImageItem->imageAvailableBoundingRect().size();

            if (!imageAvailableSize.isEmpty())
            {
                QSize imageZoomedSize = pImageItem->zoomedImageSize();
                QSize imageOriginalSize = pImageItem->originalImageSize();
                QRect imageBoundingRect = pImageItem->imageBoundingRect();
                int nOriginalWidth = -1;
                int nOriginalHeight = -1;

                if ((imageZoomedSize.width() > imageZoomedSize.height())         &&
                    (imageOriginalSize.width() > imageOriginalSize.height())     &&
                    (imageBoundingRect.width() > imageBoundingRect.height())) // original image orientation
                {
                    nOriginalWidth = imageOriginalSize.width();
                    nOriginalHeight = imageOriginalSize.height();
                }
                else
                {
                    nOriginalWidth = imageOriginalSize.height();
                    nOriginalHeight = imageOriginalSize.width();
                }

                GT_IF_WITH_ASSERT(!imageZoomedSize.isEmpty() && (nOriginalWidth != 0) && (nOriginalHeight != 0))
                {
                    // Now, let's calculate the best fit percentage on x & y axis
                    int bestFitPercentageX = int((double(imageAvailableSize.width() - GD_IMAGES_AND_BUFFERS_MARGIN) * double(GD_IMAGES_AND_BUFFERS_DEFAULT_ZOOM_LEVEL) / double(nOriginalWidth)));
                    int bestFitPercentageY = int((double(imageAvailableSize.height() - GD_IMAGES_AND_BUFFERS_MARGIN) * double(GD_IMAGES_AND_BUFFERS_DEFAULT_ZOOM_LEVEL) / double(nOriginalHeight)));
                    // Let's use the smaller between the two
                    retVal = min(bestFitPercentageX, bestFitPercentageY);
                    retVal -= GD_IMAGES_AND_DATA_MIN_ZOOM_LEVEL;
                    // Make sure we are returning a positive integer, minimum is 1%
                    retVal = max(retVal, 1);
                    return retVal;
                }
            }
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acImagesBuffersManager::isNewZoomLevelPossible
// Description: This function checks if the new required zoom level
//              is not too big for the texture manager, according to the images
//              inside.
// Arguments:   newZoomLevel - The new required zoom level
// Return Val:  New zoom level possible / not possible
// Author:      Eran Zinman
// Date:        12/7/2007
// ---------------------------------------------------------------------------
bool gdImageDataView::isNewZoomLevelPossible(int newZoomLevel) const
{
    bool retVal = true;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pImageViewManager != nullptr)
    {
        if ((_pImageViewManager->managerMode() == AC_MANAGER_MODE_STANDARD_ITEM) || (_pImageViewManager->managerMode() == AC_MANAGER_MODE_CUBEMAP_TEXTURE))
        {
            // Get the image item:
            acImageItem* pImageItem = _pImageViewManager->getItem(0);
            GT_IF_WITH_ASSERT(pImageItem != nullptr)
            {
                // Get the image size:
                // Calculate "wasted" space occupied by "paddings" and "margins"
                QSize imageOriginalSize = pImageItem->originalImageSize();


                // Now let's what's the new size of the images will be
                float zoomMultiplier = float(newZoomLevel) / float(100.0f);

                // Let's check what's the new size will be (in pixels):
                long newWidth = long(float(imageOriginalSize.width()) * zoomMultiplier);
                long newHeight = long(float(imageOriginalSize.height()) * zoomMultiplier);

                // Now let's check if we are over the limit?
                if ((newWidth > GD_IMAGES_AND_BUFFERS_MAXIMUM_MANAGER_SIZE) || (newHeight > GD_IMAGES_AND_BUFFERS_MAXIMUM_MANAGER_SIZE))
                {
                    // We are over the limit!
                    retVal = false;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImageDataView::setImageViewMode
// Description: Setup the image view mode according to the texture type
// Arguments:   apTextureType textureType
// Return Val:  bool - success / failure
// Author:      Eran Zinman
// Date:        20/12/2007
// ---------------------------------------------------------------------------
bool gdImageDataView::setImageViewMode(apTextureType textureType)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pImageViewManager != nullptr)
    {
        retVal = true;

        switch (textureType)
        {
            case AP_UNKNOWN_TEXTURE_TYPE:
            case AP_1D_TEXTURE:
            case AP_TEXTURE_RECTANGLE:
            case AP_2D_TEXTURE:
            case AP_3D_TEXTURE:
            case AP_1D_ARRAY_TEXTURE:
            case AP_2D_ARRAY_TEXTURE:
            case AP_2D_TEXTURE_MULTISAMPLE:
            case AP_2D_TEXTURE_MULTISAMPLE_ARRAY:
            case AP_BUFFER_TEXTURE:
                // Standard 2/3 Dimensional Texture
                _pImageViewManager->setManagerMode(AC_MANAGER_MODE_STANDARD_ITEM);
                break;

            case AP_CUBE_MAP_TEXTURE:
            case AP_CUBE_MAP_ARRAY_TEXTURE:
                // Special layout to hold an "open box" like layout
                _pImageViewManager->setManagerMode(AC_MANAGER_MODE_CUBEMAP_TEXTURE);
                break;

            default:
            {
                // Texture is not supported
                retVal = false;
                GT_ASSERT_EX(false, L"Unknown Texture Type!");
            }
            break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::displayRelevantNotebookPages
// Description: Display the requested view
// Arguments:   displayProperties - the requested display properties (input & output)
//              int& lastSelectedPageIndex - the last page selected index
// Author:      Sigal Algranaty
// Date:        10/11/2010
// ---------------------------------------------------------------------------
void gdImageDataView::displayRelevantNotebookPages()
{
    // Sanity check
    GT_IF_WITH_ASSERT((_pDataViewManager != nullptr) && (_pImageViewManager != nullptr) && (m_pTabWidget != nullptr))
    {
        // Hide / Show the control panel:
        GT_IF_WITH_ASSERT((_pMainLayout != nullptr) && (_pImageControlPanel != nullptr))
        {
            // Check if the control panel should be shown:
            bool showControlPanel = ((_currentViewsDisplayProperties._viewState != GD_TEXT_ONLY)  && (_currentViewsDisplayProperties._viewState != GD_UNINITIALIZED_VIEW_STATE)) ||
                                    _currentViewsDisplayProperties._forceControlPanelDisplay;

            // show image view if view state is image and data/ data and image or text only ( text is displayed as image)
            bool showImageView = ((_currentViewsDisplayProperties._viewState != GD_DATA_ONLY) && (_currentViewsDisplayProperties._viewState != GD_UNINITIALIZED_VIEW_STATE));

            // show data view if view state is
            bool showDataView = ((_currentViewsDisplayProperties._viewState != GD_TEXT_ONLY) && (_currentViewsDisplayProperties._viewState != GD_UNINITIALIZED_VIEW_STATE));

            // For multiwatch variables always show control panel:
            if (_pDisplayedItemTreeData != nullptr)
            {
                if (_pDisplayedItemTreeData->m_itemType == AF_TREE_ITEM_CL_KERNEL_VARIABLE)
                {
                    showControlPanel = true;
                }
            }

            // Check if the control panel is shown / hidden:
            (void) _pImageControlPanel->isVisible();
            _pImageControlPanel->setVisible(showControlPanel);

            if (!showImageView)
            {
                m_pTabWidget->removeTab(m_pTabWidget->indexOf(_pImageViewManager));
            }
            else
            {
                // Add the image view to the AUI notebook:
                m_pTabWidget->addTab(_pImageViewManager, GD_STR_ImagesAndBuffersViewerImageViewerCaption);
            }

            if (!showDataView)
            {
                m_pTabWidget->removeTab(m_pTabWidget->indexOf(_pDataViewManager));
            }
            else
            {
                // Add the data view to the AUI notebook:
                int dataViewIndex = m_pTabWidget->addTab(_pDataViewManager, GD_STR_ImagesAndBuffersViewerDataViewerCaption);

                // If this view has the "Data and image" option, select the data view:
                if (_currentViewsDisplayProperties._viewState == GD_DATA_AND_IMAGE)
                {
                    m_pTabWidget->blockSignals(true);
                    m_pTabWidget->setCurrentIndex(dataViewIndex);
                    m_pTabWidget->blockSignals(false);
                }
            }

            // Show / Hide the control panel ins the sizer:
            _pMainLayout->activate();

            m_pTabWidget->setTabBarVisible(_currentViewsDisplayProperties._viewState != GD_TEXT_ONLY);

            // Refresh the tab widget:
            m_pTabWidget->updateGeometry();
            _pDataViewManager->update();
            _pImageControlPanel->update();
        }

        setupControlPanel();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::applyBestFit
// Description: Perform "Best Fit" on the displayed item size
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        10/11/2010
// ---------------------------------------------------------------------------
void gdImageDataView::applyBestFit(int& newZoomLevel)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pImageViewManager != nullptr)
    {
        if ((_pImageViewManager->managerMode() == AC_MANAGER_MODE_STANDARD_ITEM) || ((_pImageViewManager->managerMode() == AC_MANAGER_MODE_CUBEMAP_TEXTURE)))
        {
            // Get the "Best Fit" zoom level
            newZoomLevel = getBestFitPercentage();

            // Check that zoom level is not higher than the maximum zoom level
            if (newZoomLevel > GD_IMAGES_AND_DATA_MAX_ZOOM_LEVEL)
            {
                // Set zoom level to be the maximum (not more)
                newZoomLevel = GD_IMAGES_AND_DATA_MAX_ZOOM_LEVEL;
            }

            // Is zoom level not possible?
            if (!isNewZoomLevelPossible(newZoomLevel))
            {
                // Set default zoom level
                newZoomLevel = 100;
            }

            // Set the new zoom level:
            if (newZoomLevel == 0)
            {
                newZoomLevel = 100;
            }

            setTextureManagerZoomLevel(newZoomLevel);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::normalizeRawData
// Description: Normalizes a raw file is it's necessary
// Arguments:   acRawFileHandler* pRawDataHandler
//              afTreeItemType displayedItemType - the data for the displayed item
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/3/2011
// ---------------------------------------------------------------------------
bool gdImageDataView::normalizeRawData(acRawFileHandler* pRawDataHandler)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((_pDisplayedItemTreeData != nullptr) && (pRawDataHandler != nullptr) && (_pImageControlPanel != nullptr))
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != nullptr)
        {
            bool normalizeValues = false;

            // Check if the item is suitable for double slider, otherwise - normalization is irrelevant:
            normalizeValues = _pImageControlPanel->isDisplayedItemSuitableForDoubleSlider();

            // For static buffers only display the
            bool isStaticBuffer = ((_pDisplayedItemTreeData->m_itemType == AF_TREE_ITEM_GL_STATIC_BUFFER) ||
                                   (_pDisplayedItemTreeData->m_itemType == AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER) ||
                                   (_pDisplayedItemTreeData->m_itemType == AF_TREE_ITEM_GL_RENDER_BUFFER));

            // For now we normalize the values only for DEPTH and STENCIL buffers:
            if (isStaticBuffer)
            {
                normalizeValues = ((pGDData->_bufferType == AP_DEPTH_BUFFER)
                                   || (pGDData->_bufferType == AP_DEPTH_ATTACHMENT_EXT)
                                   || (pGDData->_bufferType == AP_STENCIL_BUFFER));
            }

            if (_pDisplayedItemTreeData->m_itemType == AF_TREE_ITEM_CL_KERNEL_VARIABLE)
            {
                normalizeValues = true;
            }

            // Get the file texel format:
            oaTexelDataFormat texelFormat = pRawDataHandler->dataFormat();

            // Should we normalize the values?
            if (normalizeValues)
            {
                retVal = pRawDataHandler->normalizeValues(texelFormat);
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
// Name:        gdImageDataView::loadItemToImageAndDataViews
// Description: Displays the texture in the image manager
// Arguments:   pRawFileHandler - The raw file data
//              objectName - The object name
//              matrixPos - The cell in the matrix which we add
//              the object into
// Return Val:  bool - success / failure
// Author:      Eran Zinman
// Date:        24/5/2007
// ---------------------------------------------------------------------------
bool gdImageDataView::loadItemToImageAndDataViews(acRawFileHandler* pRawDataHandler, const gtString& objectName, QPoint matrixPos)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pRawDataHandler != nullptr)
    {
        // Set the control panel raw data handler:
        _pImageControlPanel->setDisplayedFileHandler(pRawDataHandler);


        m_bFlipImage = pRawDataHandler->shouldYFlipImage();

        for (unsigned int i = 0; i < AC_MAX_RAW_FILE_FILTER_HANDLERS; i++)
        {
            // Get the filter raw file handlers:
            acRawFileHandler* pFilterRawFileHandler = getCurrentFilterRawDataHandler(i);

            if (nullptr != pFilterRawFileHandler)
            {
                // Set the filter for the raw file handler:
                pRawDataHandler->setFilterRawFileHandler(pFilterRawFileHandler, i);
            }
            else // nullptr == pFilterRawFileHandler
            {
                break;
            }
        }

        // Normalize the raw data if necessary:
        bool rcNormalize = normalizeRawData(pRawDataHandler);
        GT_ASSERT(rcNormalize);

        // Load the raw data to the image viewer, get the image canvas ID:
        int canvasID = loadRawDataToImageViewer(pRawDataHandler, objectName, matrixPos);
        GT_IF_WITH_ASSERT(canvasID != -1)
        {
            gdDebugApplicationTreeData* pGDData  = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
            GT_IF_WITH_ASSERT(pGDData != nullptr)
            {
                // Check if the current loaded item contain multiple pages:
                bool doesItemContainMultiplePages = false;

                if ((_pDisplayedItemTreeData->m_itemType == AF_TREE_ITEM_GL_TEXTURE) && ((pGDData->_textureType == AP_CUBE_MAP_TEXTURE || pGDData->_textureType == AP_CUBE_MAP_ARRAY_TEXTURE)))
                {
                    doesItemContainMultiplePages = true;
                }

                // Loads the buffer into the data view
                bool rc = loadRawDataToDataViewer(pRawDataHandler, canvasID, true, doesItemContainMultiplePages);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Load the texture into the texture manager
                    retVal = true;
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImageDataView::setupControlPanel
// Description: Setups the control panel to show / hide the right sliders
// Author:      Sigal Algranaty
// Date:        14/11/2010
// ---------------------------------------------------------------------------
void gdImageDataView::setupControlPanel()
{
    GT_IF_WITH_ASSERT((_pImageControlPanel != nullptr) && (_pDataViewManager != nullptr))
    {
        bool showControlPanel = false;

        if (_pDisplayedItemTreeData != nullptr)
        {
            // Check the item load status:
            showControlPanel = _pDisplayedItemTreeData->isItemDisplayed();

            // Display control panel always for variables:
            showControlPanel = showControlPanel || (_pDisplayedItemTreeData->m_itemType == AF_TREE_ITEM_CL_KERNEL_VARIABLE);
        }

        if (showControlPanel)
        {
            // Get the displayed raw file handler
            acRawFileHandler* pRawFileHandler = _pDataViewManager->getRawDataHandler();

            // Set the raw file handler:
            _pImageControlPanel->setDisplayedFileHandler(pRawFileHandler);
        }

        // Change the item status to stale is needed:
        // Setup the panel for item load:
        _pImageControlPanel->setupPanel(_pDisplayedItemTreeData);
    }

    layout()->activate();
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::exportSpyData
// Description: Make sure viewer item data is cached into disk,
//              meaning that it's make sure that the from the spy side,
//              data was written to disk.
// Arguments:   pItemData - The viewer item to cache
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        27/5/2007
// ---------------------------------------------------------------------------
bool gdImageDataView::exportSpyData(afApplicationTreeItemData* pItemData)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pItemData != nullptr)
    {
        gdDebugApplicationTreeData* pGDData  = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != nullptr)
        {
            retVal = pGDData->_isDataCached;

            // If item was already cached to disk, there is no need for us to cache it again!
            if (!retVal)
            {
                // Get item type
                afTreeItemType itemType = pItemData->m_itemType;

                switch (itemType)
                {
                    case AF_TREE_ITEM_GL_TEXTURE:
                    {
                        // Get the texture name:
                        apGLTextureMipLevelID textureID = pGDData->_textureMiplevelID;

                        // Extract the texture raw data to disk
                        gtVector<apGLTextureMipLevelID> texturesVector;
                        texturesVector.push_back(textureID);

                        // Check if the texture is dirty, and update the texture data if it is:
                        bool dirtyImageExists = true, dirtyRawDataExists = true;
                        bool rc = gaIsTextureImageDirty(pGDData->_contextId._contextId, pGDData->_textureMiplevelID, dirtyImageExists, dirtyRawDataExists);
                        GT_ASSERT(rc);

                        if (dirtyRawDataExists)
                        {
                            // Update the texture raw data:
                            retVal = gaUpdateTextureRawData(pGDData->_contextId._contextId, texturesVector);

                            if (!retVal)
                            {
                                OS_OUTPUT_DEBUG_LOG(GD_STR_LogMsg_couldNotUpdateTextureRawData, OS_DEBUG_LOG_DEBUG);
                            }
                        }
                        else
                        {
                            retVal = true;
                        }

                        break;
                    }

                    case AF_TREE_ITEM_CL_IMAGE:
                    {
                        // OpenCL textures:
                        // Extract the texture raw data to disk
                        gtVector<int> texturesVector;
                        texturesVector.push_back(pGDData->_objectOpenCLIndex);

                        // Check if the texture is dirty, and update the texture data if it is:
                        // TO_DO: OpenCL buffers implement
                        //bool dirtyImageExists = true;
                        bool dirtyRawDataExists = true;

                        // bool rc = gaIsTextureImageDirty(pItemData->_contextId._contextId, pItemData->_textureMiplevelID, dirtyImageExists, dirtyRawDataExists);;
                        if (dirtyRawDataExists)
                        {
                            retVal = gaUpdateOpenCLImageRawData(pGDData->_contextId._contextId, texturesVector);

                            if (!retVal)
                            {
                                OS_OUTPUT_DEBUG_LOG(GD_STR_LogMsg_couldNotUpdateTextureRawData, OS_DEBUG_LOG_DEBUG);
                            }
                        }
                        else
                        {
                            retVal = true;
                        }
                    }
                    break;

                    case AF_TREE_ITEM_GL_STATIC_BUFFER:
                    {
                        // Get buffer type
                        apDisplayBuffer bufferType = pGDData->_bufferType;

                        // Extract buffer raw data to disk
                        retVal = gaUpdateStaticBufferRawData(pGDData->_contextId._contextId, bufferType);
                    }
                    break;

                    case AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER:
                    {
                        // Get PBuffer ID:
                        int pbufferID = pGDData->_objectOpenGLName;

                        // Get buffer type:
                        apDisplayBuffer bufferType = pGDData->_bufferType;

                        // Get PBuffer render context ID:
                        int pbufferContextID = pGDData->_contextId._contextId;

                        // Extract buffer raw data to disk
                        retVal = gaUpdatePBufferStaticBufferRawData(pbufferContextID, pbufferID, bufferType);
                    }
                    break;

                    case AF_TREE_ITEM_GL_RENDER_BUFFER:
                    {
                        // Get Render Buffer ID
                        int renderBufferID = pGDData->_objectOpenGLName;

                        gtVector<GLuint> renderBuffersVector;
                        renderBuffersVector.push_back(renderBufferID);

                        // Extract buffer raw data to disk
                        retVal = gaUpdateRenderBufferRawData(pGDData->_contextId._contextId, renderBuffersVector);
                    }
                    break;

                    case AF_TREE_ITEM_GL_VBO:
                    {
                        // Get VBO name:
                        GLuint vboName = pGDData->_objectOpenGLName;

                        gtVector<GLuint> vboVector;
                        vboVector.push_back(vboName);

                        // Extract VBO raw data to disk:
                        retVal = gaUpdateVBORawData(pGDData->_contextId._contextId, vboVector);
                    }
                    break;

                    case AF_TREE_ITEM_CL_BUFFER:
                    {
                        gtVector<int> bufferIdsVector;
                        bufferIdsVector.push_back(pGDData->_objectOpenCLIndex);

                        // Extract the CLBuffer raw data to disk:
                        retVal = gaUpdateOpenCLBufferRawData(pGDData->_contextId._contextId, bufferIdsVector);
                    }
                    break;

                    case AF_TREE_ITEM_CL_SUB_BUFFER:
                    {
                        gtVector<int> bufferIdsVector;
                        bufferIdsVector.push_back(pGDData->_objectOpenCLIndex);

                        // Extract the CLSubBuffer raw data to disk:
                        retVal = gaUpdateOpenCLSubBufferRawData(pGDData->_contextId._contextId, bufferIdsVector);
                    }
                    break;


                    case AF_TREE_ITEM_GL_FBO_ATTACHMENT:
                    {
                        bool isTextureAttachment = false;
                        bool rc = apGLFBO::isTextureAttachmentTarget(pGDData->_bufferAttachmentTarget, isTextureAttachment);
                        GT_IF_WITH_ASSERT(rc)
                        {
                            if (!isTextureAttachment)
                            {
                                // Get Framebuffer ID:
                                int renderBufferID = pGDData->_objectOpenGLName;

                                gtVector<GLuint> renderBuffersVector;
                                renderBuffersVector.push_back(renderBufferID);

                                // Extract buffer raw data to disk
                                retVal = gaUpdateRenderBufferRawData(pGDData->_contextId._contextId, renderBuffersVector);
                            }
                            else
                            {

                                // Get the texture ID:
                                apGLTextureMipLevelID textureID = pGDData->_textureMiplevelID;

                                // Extract the texture raw data to disk:
                                gtVector<apGLTextureMipLevelID> texturesVector;
                                texturesVector.push_back(textureID);

                                retVal = gaUpdateTextureRawData(pGDData->_contextId._contextId, texturesVector);
                            }
                        }
                    }
                    break;

                    default:
                    {
                        GT_ASSERT_EX(false, L"Unknown item type!");
                        retVal = false;
                    }
                    break;
                }

                // If caching was successful, flag that the item data was cached
                if (retVal)
                {
                    // Flag that the buffer content was already cached to disk
                    pGDData->_isDataCached = true;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::loadRawDataToImageManager
// Description: Converts the raw data into an image and loads it into the
//              image manager.
// Arguments:   pRawFileHandler - The raw file handler object
//              objectName - The object name
//              matrixPos - The matrix position to add the image into
// Return Val:  The canvasID of the inserted image if successful,
//              -1 if operation failed.
// Author:      Eran Zinman
// Date:        18/8/2007
// ---------------------------------------------------------------------------
int gdImageDataView::loadRawDataToImageViewer(acRawFileHandler* pRawFileHandler, const gtString& objectName, QPoint matrixPos)
{
    int canvasID = -1;
    GT_IF_WITH_ASSERT(nullptr != _pImageViewManager)
    {
        QAbstractItemDelegate* pDelegate =  _pImageViewManager->itemDelegate();
        acImageItemDelegate* pImageItemDelegate = dynamic_cast<acImageItemDelegate*>(pDelegate);

        if (nullptr != pImageItemDelegate)
        {
            pImageItemDelegate->resetFirstTimeBestFit();
        }
    }
    // Sanity check:
    GT_IF_WITH_ASSERT((pRawFileHandler != nullptr) && (_pDisplayedItemTreeData != nullptr) && (_pImageControlPanel != nullptr))
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != nullptr)
        {
            // Set the control panel raw data handler:
            _pImageControlPanel->setDisplayedFileHandler(pRawFileHandler);

            // Does the raw data have more than 1 page?
            if (pRawFileHandler->amountOfExternalPages() > 1)
            {
                // If we are showing an image that was previously shown, show the last selected page
                if (_lastViewedItemProperties._arePropertiesValid)
                {
                    bool rc1 = pRawFileHandler->setActivePage(_lastViewedItemProperties._activePage);
                    GT_ASSERT(rc1);
                }
                else
                {
                    // If raw data have several pages, use middle page as default
                    pRawFileHandler->setMiddlePageAsActivePage();
                }

                // For texture layer attachments:
                if (pGDData->_textureLayer != 0)
                {
                    bool rc1 = pRawFileHandler->setActivePage(pGDData->_textureLayer);
                    GT_ASSERT(rc1);
                }
            }

            // Convert raw data to a Qimage format:
            QImage* pImage = pRawFileHandler->convertToQImage();
            GT_IF_WITH_ASSERT(pImage != nullptr)
            {
                // Add thumbnail to the image viewer:
                unsigned int imageActions = 0;
                bool rcImageActions = acImageManager::getItemActionsByFormat(pRawFileHandler->dataFormat(), imageActions);
                GT_ASSERT(rcImageActions);

                // Add the image to the image view manager:
                canvasID = _pImageViewManager->addImageObject(pImage, (void*)_pDisplayedItemTreeData, objectName, matrixPos, imageActions);
                GT_ASSERT(canvasID != -1);
            }
        }
    }

    return canvasID;
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::loadRawIntoDataView
// Description: Loads a raw data object into the data view, and links
//              it with the image canvas id
// Arguments:   pRawDataHandler - The raw data handler.
//              canvasID - The linked canvas Id
//              bool isImageViewShown
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        18/8/2007
// ---------------------------------------------------------------------------
bool gdImageDataView::loadRawDataToDataViewer(acRawFileHandler* pRawDataHandler, int canvasID, bool isImageViewShown, bool doesItemContainMultiplePages)
{
    bool retVal = false;

    // Do we have a valid canvasID?
    GT_IF_WITH_ASSERT((canvasID != -1) && (pRawDataHandler != nullptr) && (_pDataViewManager != nullptr))
    {
        GT_IF_WITH_ASSERT(_pImageControlPanel != nullptr)
        {
            // Set the displayed raw file handler:
            _pImageControlPanel->setDisplayedFileHandler(pRawDataHandler);
        }

        // Create the information for the data view:
        acDataViewItem::acDataViewInfo dataViewInfo;

        // Get specific data information for multi watch view:
        if (pRawDataHandler->dataFormat() == OA_TEXEL_FORMAT_VARIABLE_VALUE)
        {
            // Get the size of the X & Y coordinates group sizes:
            bool rc1 = gaGetKernelDebuggingLocalWorkSize(0, dataViewInfo._yCoordGroupSize);
            GT_ASSERT(rc1);
            rc1 = gaGetKernelDebuggingLocalWorkSize(1, dataViewInfo._xCoordGroupSize);
            // Do not call GT_ASSERT(rc1) - for one dimensional variables, the result should be false:

            // Get the global work offset:
            rc1 = gaGetKernelDebuggingGlobalWorkOffset(0, dataViewInfo._xCoordOffset);
            GT_ASSERT(rc1);
            rc1 = gaGetKernelDebuggingGlobalWorkOffset(1, dataViewInfo._yCoordOffset);
            // Do not call GT_ASSERT(rc1) - for one dimensional variables, the result should be false:
        }
        else
        {
            // Otherwise the groups are calculated by the amount of components:
            dataViewInfo._yCoordGroupSize = oaAmountOfTexelFormatComponents(pRawDataHandler->dataFormat());
        }

        // Set the filter raw file handlers:
        for (unsigned int i = 0; i < AC_MAX_RAW_FILE_FILTER_HANDLERS; i++)
        {
            acRawFileHandler* pCurrentFilter = getCurrentFilterRawDataHandler(i);

            if (nullptr != pCurrentFilter)
            {
                dataViewInfo._pFilterRawFileHandler[i] = pCurrentFilter;
            }
            else
            {
                break;
            }
        }

        // Load the raw data into the data grid:
        bool rc1 = _pDataViewManager->loadDataIntoGridView(canvasID, pRawDataHandler, isImageViewShown, doesItemContainMultiplePages, dataViewInfo);
        GT_IF_WITH_ASSERT(rc1)
        {
            _pDataViewManager->setActiveTable(canvasID);
            retVal = true;
        }

        // Update the data view manager:
        _pDataViewManager->repaint();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::forceImageManagerRepaint
// Description: Force the repaint of the images window
// Author:      Sigal Algranaty
// Date:        10/11/2010
// ---------------------------------------------------------------------------
void gdImageDataView::forceImageManagerRepaint()
{
    GT_IF_WITH_ASSERT(_pImageViewManager != nullptr)
    {
        _pImageViewManager->forceImagesRepaint();
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImageDataView::calculateImagesLayout
// Description: Preform a calculation of the images sizes within the manager
// Author:      Sigal Algranaty
// Date:        4/7/2012
// ---------------------------------------------------------------------------
void gdImageDataView::calculateImagesLayout()
{
    GT_IF_WITH_ASSERT(_pImageViewManager != nullptr)
    {
        _pImageViewManager->calculateImagesLayout();
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImageDataView::selectImageView
// Description: Select the image / data view
// Arguments:   bool select
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        11/11/2010
// ---------------------------------------------------------------------------
void gdImageDataView::selectImageView(bool select)
{
    // Get image view page index
    int imagePageIndex = m_pTabWidget->indexOf(_pImageViewManager);
    int dataPageIndex = m_pTabWidget->indexOf(_pDataViewManager);

    int selectedPage = (select) ? imagePageIndex : dataPageIndex;

    GT_IF_WITH_ASSERT(selectedPage != -1)
    {
        // Set the selected page index:
        if (m_pTabWidget->currentIndex() != selectedPage)
        {
            // Select the right page:
            m_pTabWidget->setCurrentIndex(selectedPage);

            // Update the AUI notebook:
            m_pTabWidget->update();

            // Update this page:
            update();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImageDataView::onUpdateEdit_Copy
// Description: enable copy from the VS menu
// Arguments:   bool &isEnabled
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        7/2/2011
// ---------------------------------------------------------------------------
void gdImageDataView::onUpdateEdit_Copy(bool& isEnabled)
{
    isEnabled = false;
    GT_IF_WITH_ASSERT(m_pTabWidget != nullptr)
    {
        int dataPageIndex = m_pTabWidget->indexOf(_pDataViewManager);

        if (m_pTabWidget->currentIndex() == dataPageIndex)
        {
            isEnabled = true;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImageDataView::onUpdateEdit_SelectAll
// Description: enable select all from the VS menu
// Arguments:   bool &isEnabled
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        7/2/2011
// ---------------------------------------------------------------------------
void gdImageDataView::onUpdateEdit_SelectAll(bool& isEnabled)
{
    isEnabled = false;
    GT_IF_WITH_ASSERT(m_pTabWidget != nullptr)
    {
        int dataPageIndex = m_pTabWidget->indexOf(_pDataViewManager);

        if (m_pTabWidget->currentIndex() == dataPageIndex)
        {
            isEnabled = true;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::onEditCopy
// Description: Copies selected cell content in data view to the clipboard
// Arguments:   Event details
// Author:      Sigal Algranaty
// Date:        9/9/2009
// ---------------------------------------------------------------------------
void gdImageDataView::onEdit_Copy()
{
    if (_pDataViewManager != nullptr)
    {
        _pDataViewManager->onEdit_Copy();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::onEditSelectAll
// Description: Select all cells in data view
// Arguments:   Event details
// Author:      Sigal Algranaty
// Date:        9/9/2009
// ---------------------------------------------------------------------------
void gdImageDataView::onEdit_SelectAll()
{
    if (_pDataViewManager != nullptr)
    {
        _pDataViewManager->onEdit_SelectAll();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::showSaveSingleFileDialog
// Description: Shows the save single file dialog, with the given files types
//              which we can save the file as.
// Arguments:   fileTypesVector - A vector containing all the file types
//              which we can use in order to save the file with.
// Author:      Eran Zinman
// Date:        13/1/2008
// ---------------------------------------------------------------------------
void gdImageDataView::showSaveSingleFileDialog(const gtVector<apFileType>& fileTypesVector, afApplicationTreeItemData* pItemData)
{
    // Make sure we got at least one file format in the file types vector
    bool rc1 = (fileTypesVector.size() > 0);
    GT_IF_WITH_ASSERT(rc1 && (pItemData != nullptr))
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != nullptr)
        {
            // Generate a suggested file name for saving the image:
            osFilePath suggestedFilePath(osFilePath::OS_USER_DOCUMENTS);
            gtString suggestedFileName;
            bool rc2 = generateSuggestedExportFileName(pItemData, suggestedFileName);
            GT_IF_WITH_ASSERT(rc2)
            {
                QString strWildcards;

                // Get amount of file types
                int amountOfFileTypes = fileTypesVector.size();

                // Generate the wild cards:
                for (int i = 0; i < amountOfFileTypes; i++)
                {
                    // Convert to gdSaveFileDialogFileType type
                    apFileType fileType = fileTypesVector[i];

                    // Get current wildcard value
                    gtString fileTypeWildcard;
                    bool rc = apFileTypeWildcard(fileType, fileTypeWildcard);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        // Add wildcard value
                        strWildcards.append(acGTStringToQString(fileTypeWildcard));

                        // Add "|" separator after every wildcard, except from the last one
                        if (i != (amountOfFileTypes - 1))
                        {
                            strWildcards.append(";;");
                        }
                    }
                }

                gtString suggestedExtension;
                bool rc3 = apFileTypeToFileExtensionString(fileTypesVector[0], suggestedExtension);
                GT_IF_WITH_ASSERT(rc3)
                {
                    suggestedFilePath.setFileExtension(suggestedExtension);
                    suggestedFilePath.setFileName(suggestedFileName);
                }

                // Get the application commands handler:
                afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
                GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
                {
                    // Open the save file dialog:
                    QString defaultFilePath = acGTStringToQString(suggestedFilePath.asString());
                    QString pathStr = pApplicationCommands->ShowFileSelectionDialog(AF_STR_saveImageDialogHeader, defaultFilePath, strWildcards, nullptr, true);

                    if (!pathStr.isEmpty())
                    {
                        osFilePath selectedFilePath(acQStringToGTString(pathStr));

                        // Get the file extension:
                        gtString extension;
                        selectedFilePath.getFileExtension(extension);
                        apFileType fileType;
                        bool rc = apFileExtensionStringToFileType(extension, fileType);
                        GT_IF_WITH_ASSERT(rc)
                        {
                            // Separate output file path to output directory and output filename
                            osDirectory outputDir;
                            bool rc4 = selectedFilePath.getFileDirectory(outputDir);

                            gtString fileName;
                            bool rc5 = selectedFilePath.getFileName(fileName);
                            GT_IF_WITH_ASSERT(rc4 && rc5)
                            {
                                // This flag will indicate if export was successful or not
                                bool exportSuccess = false;

                                // Check if this is a texture:
                                bool isTexture = ((pItemData->m_itemType == AF_TREE_ITEM_GL_TEXTURE) || (pItemData->m_itemType == AF_TREE_ITEM_CL_IMAGE));

                                // If output format is spreadsheet format, or texture is a 3d texture, just
                                // use the regular export mechanism used by the "Export all textures and buffers" command
                                if (fileType == AP_CSV_FILE || (isTexture && pGDData->_textureType == AP_3D_TEXTURE)
                                    || (pGDData->_textureType == AP_1D_ARRAY_TEXTURE) || (pGDData->_textureType == AP_2D_ARRAY_TEXTURE))
                                {
                                    // A Vector containing export result
                                    gtPtrVector<gdFileExporterOutputResult*> exportResult;

                                    // Export viewer item  to disk
                                    bool rc6 = exportViewerItemToDisk(pItemData, outputDir, fileType, exportResult, true, fileName);
                                    GT_ASSERT(rc6);

                                    // Get amount of files exported
                                    int amountOfFiles = exportResult.size();

                                    // Loop through the exported files and check if any error occurred
                                    exportSuccess = true;

                                    for (int i = 0; i < amountOfFiles; i++)
                                    {
                                        // Get current export object
                                        gdFileExporterOutputResult* pExportResult = exportResult[i];

                                        if (pExportResult != nullptr)
                                        {
                                            if (pExportResult->fileStatus != GD_EXPORT_FILE_SUCCESS)
                                            {
                                                exportSuccess = false;
                                                break;
                                            }
                                        }
                                    }
                                }
                                else
                                {
                                    // Save currently viewed object to disk in the WYSIWYG method
                                    exportSuccess = exportCurrentlyViewedObjectToDisk(selectedFilePath, fileType);
                                }

                                // If export failed
                                if (!exportSuccess)
                                {
                                    // Notify the user export had failed:
                                    acMessageBox::instance().critical(AF_STR_ErrorA, GD_STR_ErrorMessageSaveFileFailed, QMessageBox::Ok);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::generateSuggestedExportFileName
// Description: Generates a suggested file name for saving a file
// Arguments:   _pDisplayedItemTreeData - The viewer item to generate the file name to
//              suggestedFileName - Output suggested file name
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        5/1/2008
// ---------------------------------------------------------------------------
bool gdImageDataView::generateSuggestedExportFileName(afApplicationTreeItemData* pItemData, gtString& suggestedFileName)
{
    (void)(pItemData);  // unused
    bool retVal = false;

    GT_IF_WITH_ASSERT(_pDisplayedItemTreeData != nullptr)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != nullptr)
        {
            // Clear the suggested file name
            suggestedFileName.makeEmpty();

            // Get the current project name:
            gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();
            retVal = true;

            // Add default prefix to the file name (project name and context ID)
            suggestedFileName.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportFilePrefix, projectName.asCharArray(), pGDData->_contextId._contextId);

            // Get the viewer item type
            afTreeItemType itemType = _pDisplayedItemTreeData->m_itemType;

            switch (itemType)
            {
                case AF_TREE_ITEM_GL_TEXTURE:
                {
                    // Generate a suggest file name:
                    suggestedFileName.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportTextureSuffix, pGDData->_textureMiplevelID._textureName, pGDData->_textureMiplevelID._textureMipLevel);
                }
                break;

                case AF_TREE_ITEM_CL_IMAGE:
                {
                    // Generate a file name:
                    suggestedFileName.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportImageSuffix, pGDData->_textureMiplevelID._textureName);
                }
                break;

                case AF_TREE_ITEM_GL_RENDER_BUFFER:
                {
                    // Generate a suggest file name:
                    suggestedFileName.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportRenderBufferSuffix, pGDData->_objectOpenGLName);
                }
                break;

                case AF_TREE_ITEM_CL_BUFFER:
                {
                    // Generate a suggest file name
                    suggestedFileName.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportCLBufferSuffix, pGDData->_objectOpenCLName);
                }
                break;

                case AF_TREE_ITEM_CL_SUB_BUFFER:
                {
                    // Generate a suggest file name
                    suggestedFileName.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportCLSubBufferSuffix, pGDData->_objectOpenCLName);
                }
                break;

                case AF_TREE_ITEM_GL_VBO:
                {
                    // Generate a suggest file name
                    suggestedFileName.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportVBOSuffix, pGDData->_objectOpenGLName);
                }
                break;

                case AF_TREE_ITEM_GL_STATIC_BUFFER:
                {
                    // Get static buffer type
                    apDisplayBuffer bufferType = pGDData->_bufferType;

                    // Convert the buffer type into a string describing the buffer
                    gtString bufferNameCode;
                    retVal = apGetBufferNameCode(bufferType, bufferNameCode);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        // Generate a suggest file name
                        suggestedFileName.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportStaticBufferSuffix, bufferNameCode.asCharArray());
                    }
                }
                break;

                case AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER:
                {
                    // Get static buffer type
                    apDisplayBuffer bufferType = pGDData->_bufferType;

                    // Get PBuffer ID
                    int pbufferID = pGDData->_objectOpenGLName;

                    // Convert the buffer type into a string describing the buffer
                    gtString bufferNameCode;
                    retVal = apGetBufferNameCode(bufferType, bufferNameCode);
                    GT_IF_WITH_ASSERT(retVal)
                    {
                        // Generate a suggest file name
                        suggestedFileName.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportPBufferSuffix, pbufferID, bufferNameCode.asCharArray());
                    }
                }
                break;

                default:
                {
                    GT_ASSERT_EX(false, L"Unsupported item type!");
                    retVal = false;
                }
                break;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImageDataView::exportCurrentlyViewedObjectToDisk
// Description: Export the current viewed object (in the textures and buffers
//              viewer) to disk according to the defined file type
// Arguments:   outputFilePath - The file name to save the viewer object to.
//              fileType - The format to save the files with
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        12/2/2008
// ---------------------------------------------------------------------------
bool gdImageDataView::exportCurrentlyViewedObjectToDisk(const osFilePath& outputFilePath, apFileType fileType)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((_pImageViewManager != nullptr) && (_pDataViewManager != nullptr))
    {
        // Convert file type to QImage format
        gtString fileExtension = L"png";
        bool rc1 = acRawDataExporter::convertFileTypeToImageFormat(fileType, fileExtension);
        GT_IF_WITH_ASSERT(rc1)
        {
            retVal = true;

            // Get amount of items in textures and buffers manager
            int amountOfItems = _pImageViewManager->amountOfImagesInManager();

            // Loop through all the items and apply the double slider change
            for (int i = 0; i < amountOfItems; i++)
            {
                // Get the image item associated with this canvas id
                acImageItem* pImageItem = (acImageItem*)_pImageViewManager->getItem(i);

                // Generate output file path
                osFilePath filePath = outputFilePath;
                filePath.setFileExtension(fileExtension);

                // If we need to save multiple images, just add the data grid "header" to the file name
                if (amountOfItems > 1)
                {
                    gtString dataGridHeader;
                    bool rc2 = _pDataViewManager->getTableName(i, dataGridHeader);
                    GT_IF_WITH_ASSERT(rc2)
                    {
                        // Strip spaces from name
                        dataGridHeader.replace(AF_STR_Space, AF_STR_Empty);

                        // Add name to the output file name as a suffix
                        gtString fileName;
                        bool rc3 = filePath.getFileName(fileName);
                        GT_IF_WITH_ASSERT(rc3)
                        {
                            fileName.appendFormattedString(L"-%ls", dataGridHeader.asCharArray());

                            // Set the new file name
                            filePath.setFileName(fileName);
                        }
                    }
                }

                // Generates an image snapshot of what currently seen on screen (WYSIWYG) in QImage format
                QImage* pImageSnapshot = pImageItem->generateImageSnapshot();
                GT_IF_WITH_ASSERT(pImageSnapshot != nullptr)
                {
                    // Save the image to the disk:
                    bool rc4 = pImageSnapshot->save(acGTStringToQString(filePath.asString()));

                    if (!rc4)
                    {
                        // Item export had failed
                        retVal = false;
                    }

                    // Unload image snapshot
                    delete pImageSnapshot;
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::displayCurrentTextureMiplevel
// Description: Displays currently displayed texture with another mip level:
// Arguments: int miplevel
// Return Val: void
// Author:      Sigal Algranaty
// Date:        12/1/2009
// ---------------------------------------------------------------------------
void gdImageDataView::displayCurrentTextureMiplevel(int miplevel, bool forceReload)
{
    (void)(miplevel);  // unused
    (void)(forceReload);  // unused
    GT_ASSERT_EX(false, L"This function should be implemented in inherited classes");
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::getCurrentRawDataHandler
// Description: Return the currently display raw data handler
// Return Val:  acRawFileHandler*
// Author:      Sigal Algranaty
// Date:        10/11/2010
// ---------------------------------------------------------------------------
acRawFileHandler* gdImageDataView::getCurrentRawDataHandler() const
{
    acRawFileHandler* pRetVal = nullptr;
    GT_IF_WITH_ASSERT(_pDataViewManager != nullptr)
    {
        pRetVal = _pDataViewManager->getRawDataHandler();
    }
    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::displayViewStateByItemType
// Description: Return a display view state according to the displayed item type
// Arguments:   afTreeItemType itemType
// Author:      Sigal Algranaty
// Date:        11/8/2010
// ---------------------------------------------------------------------------
void gdImageDataView::setViewStateAccordingToItemType(afTreeItemType itemType, const afItemLoadStatus& itemLoadStatus)
{
    // Check if the item type if a thumbnail item type:
    bool isThumb = ((itemType == AF_TREE_ITEM_GL_TEXTURES_NODE) ||
                    (itemType == AF_TREE_ITEM_GL_PBUFFERS_NODE) ||
                    (itemType == AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE) ||
                    (itemType == AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE) ||
                    (itemType == AF_TREE_ITEM_GL_PBUFFERS_NODE) ||
                    (itemType == AF_TREE_ITEM_GL_FBO_NODE) ||
                    (itemType == AF_TREE_ITEM_GL_FBO_NODE) ||
                    (itemType == AF_TREE_ITEM_GL_VBO_NODE) ||
                    (itemType == AF_TREE_ITEM_CL_BUFFERS_NODE));

    // For thumbnail view, display only images:
    if (isThumb)
    {
        GT_ASSERT_EX(false, L"Should not get here");
    }
    else if (itemLoadStatus._itemLoadStatusType != AF_ITEM_LOAD_SUCCESS)
    {
        _currentViewsDisplayProperties._viewState = GD_TEXT_ONLY;
    }
    // For data buffers, show only data:
    else if ((itemType == AF_TREE_ITEM_GL_VBO) || (itemType == AF_TREE_ITEM_CL_BUFFER) || (itemType == AF_TREE_ITEM_CL_SUB_BUFFER))
    {
        _currentViewsDisplayProperties._viewState = GD_DATA_ONLY;
    }

    else if (itemType == AF_TREE_ITEM_CL_KERNEL_VARIABLE)
    {
        _currentViewsDisplayProperties._viewState = GD_DATA_AND_IMAGE;
    }
    else
    {
        _currentViewsDisplayProperties._viewState = GD_IMAGE_AND_DATA;
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImageDataView::adjustViewerAfterItemLoading
// Description: Adjust an item after its loading
// Arguments:   gdDebugApplicationTreeData* _pDisplayedItemTreeData
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        10/11/2010
// ---------------------------------------------------------------------------
void gdImageDataView::adjustViewerAfterItemLoading()
{
    if (_pDisplayedItemTreeData != nullptr)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != nullptr)
        {
            // Get the display view state:
            setViewStateAccordingToItemType(_pDisplayedItemTreeData->m_itemType, _pDisplayedItemTreeData->_itemLoadStatus);
            bool shouldShowControlPanel = true;

            if (_pDisplayedItemTreeData->isItemDisplayed())
            {
                shouldShowControlPanel = false;

                // For textures with miplevels, do not hide control panel, even if their load had failed:
                if (pGDData->_textureMiplevelID._textureMipLevel > 0)
                {
                    shouldShowControlPanel = true;
                }
            }

            GT_ASSERT(_pDisplayedItemTreeData->_itemLoadStatus._itemLoadStatusType != AF_ITEM_PAGE_LOAD_ERROR);

            GT_IF_WITH_ASSERT(_pImageControlPanel != nullptr)
            {
                // Show / Hide the control panel:
                _pImageControlPanel->setVisible(shouldShowControlPanel);
            }

            // Display the relevant notebook pages for the current displayed item:
            displayRelevantNotebookPages();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImageDataView::adjust3dTextureLevel
// Description: Is called when the user changes the texture mip levels.
//              If the displayed texture is a 3d texture, the function adjusts the
//              3d displayed level to the new miplevel
// Arguments:   int oldLevel
//              int newLevel
// Author:      Sigal Algranaty
// Date:        25/6/2009
// ---------------------------------------------------------------------------
void gdImageDataView::adjust3dTextureLevel(int oldLevel, int newLevel)
{
    GT_IF_WITH_ASSERT(_pDisplayedItemTreeData != nullptr)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(_pDisplayedItemTreeData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != nullptr)
        {
            // Check if this item is a 3d texture:
            if (pGDData->_textureType == AP_3D_TEXTURE)
            {
                if (_lastViewedItemProperties._arePropertiesValid)
                {
                    // Calculate the new 3d level:
                    int levelDifference = oldLevel - newLevel;

                    if (levelDifference > 0)
                    {
                        _lastViewedItemProperties._activePage /= (levelDifference * 2);
                    }
                    else if (levelDifference < 0)
                    {
                        _lastViewedItemProperties._activePage *= (levelDifference * -2);
                    }
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImageDataView::onImageItemEvent
// Description: Occurs when an image inside the image manager was changed,
//              hovered, got focus, lost focus or clicked
//              In summary: almost anything that might happen to him
// Arguments:   event - Event details
// Author:      Eran Zinman
// Date:        2/7/2007
// ---------------------------------------------------------------------------
void gdImageDataView::onImageItemEvent(acImageItemID imageItemID, const QPoint& posOnImage, bool mouseLeftDown, bool mouseDoubleClick)
{
    (void)(mouseDoubleClick);  // unused
    // Sanity check:
    GT_IF_WITH_ASSERT(_pImageViewManager != nullptr)
    {
        if (_pImageViewManager->isVisible() && (_pImageViewManager->managerMode() != AC_MANAGER_MODE_TEXT_ITEM))
        {
            // Contain the selected image item;
            acImageItem* pImageItem = nullptr;

            // Get item canvas ID:
            if (imageItemID != AC_IMAGE_MANAGER_ID_NONE)
            {
                // Get image item:
                pImageItem = (acImageItem*)_pImageViewManager->getItem(imageItemID);
                GT_IF_WITH_ASSERT(pImageItem != nullptr)
                {
                    // Update the single item view with the pixel position information:
                    updatePixelPosition(imageItemID, pImageItem, posOnImage, mouseLeftDown);
                }
            }
            else
            {
                GT_IF_WITH_ASSERT(_pImageControlPanel != nullptr)
                {
                    // Clear the pixel information when the mouse is out of the image:
                    _pImageControlPanel->clearControlPanel(!m_bPixelInfoSelected);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::onNoteBookPageChange
// Description: Is handling notebook page change
// Arguments:   wxAuiNotebookEvent& event
// Author:      Sigal Algranaty
// Date:        3/8/2010
// ---------------------------------------------------------------------------
void gdImageDataView::onNoteBookPageChange(int currentPage)
{
    if (!_ignoreSelections)
    {
        // Set the new page index:
        _currentViewsDisplayProperties._lastSelectedPageIndex = currentPage;
    }

    // Update UI events:
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();

    // Sanity check:
    GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
    {
        pApplicationCommands->updateToolbarCommands();
    }

    // Get the page and repaint it:
    GT_IF_WITH_ASSERT(m_pTabWidget != nullptr)
    {
        // Get the widget:
        QWidget* pCurrent = m_pTabWidget->widget(currentPage);

        if (pCurrent != nullptr)
        {
            acDataView* pDataView = qobject_cast<acDataView*>(pCurrent);

            if (nullptr != pDataView)
            {
                if ((nullptr != m_pSelectedPixelImageItem) && (m_nSelectedPixelImageItemID != -1))
                {
                    QPoint PosOnOriginalImage;
                    zoomedImagePositionToImagePosition(m_pSelectedPixelImageItem, m_pointSelectedPixelPosition, PosOnOriginalImage, true, /*flip Y*/false);
                    _pDataViewManager->highlightPixelPosition(m_nSelectedPixelImageItemID, PosOnOriginalImage);
                }

                pDataView->setBestFitGridCellSizes();
            }
            else
            {
                pCurrent->repaint();
            }
        }
    }

    if (_pImageControlPanel != nullptr)
    {
        // Clear the pixel information from the image information panel
        _pImageControlPanel->clearControlPanel(!m_bPixelInfoSelected);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImageDataView::exportViewerItemToDisk
// Description: Export a viewer item to the disk
// Arguments:   pViewerItem - The viewer item to export
//              outputDir - A Path of the output directory
//              fileType - Output format to save the files as.
//              exportResult - Vector containing export result
//              suggestedFileName - Suggested output file name.
//              overwriteFiles - Should we overwrite existing files?
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        6/1/2008
// ---------------------------------------------------------------------------
bool gdImageDataView::exportViewerItemToDisk(afApplicationTreeItemData* pViewerItem, const osDirectory& outputDir,
                                             apFileType fileType, gtPtrVector<gdFileExporterOutputResult*>& exportResult,
                                             bool overwriteFiles, gtString suggestedFileName)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pViewerItem != nullptr)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pViewerItem->extendedItemData());
        GT_IF_WITH_ASSERT(pGDData != nullptr)
        {
            // Cache spy data:
            bool rc1 = exportSpyData(pViewerItem);

            if (rc1)
            {
                // Increment progress:
                afProgressBarWrapper::instance().incrementProgressBar();

                // File exporter class, set output directory and should we overwrite files or not flag
                gdImagesAndBuffersExporter fileExporter(pGDData->_contextId);
                fileExporter.enableFileOverwrite(overwriteFiles);

                // If suggested file name exists, set default file name for the fileExporter
                fileExporter.setOutputFileName(suggestedFileName);

                bool rc2 = fileExporter.setOutputDirectory(outputDir);
                GT_IF_WITH_ASSERT(rc2)
                {
                    // Get item type
                    afTreeItemType itemType = pViewerItem->m_itemType;

                    switch (itemType)
                    {
                        case AF_TREE_ITEM_GL_TEXTURE:
                        {
                            gdExportedTextureID exportedTextureID;
                            exportedTextureID._contextID = pGDData->_contextId;

                            // Set the texture id:
                            exportedTextureID._textureId = pGDData->_textureMiplevelID;

                            // Export texture to disk:
                            retVal = fileExporter.exportTexture(exportedTextureID, fileType);
                        }
                        break;

                        case AF_TREE_ITEM_CL_IMAGE:
                        {
                            gdExportedTextureID exportedTextureID;
                            exportedTextureID._contextID = pGDData->_contextId;

                            // Set the texture id:
                            exportedTextureID._textureId._textureName = pGDData->_objectOpenCLIndex;

                            // Export texture to disk:
                            retVal = fileExporter.exportTexture(exportedTextureID, fileType);
                        }
                        break;

                        case AF_TREE_ITEM_GL_VBO:
                        {
                            // Export VBO to disk:
                            retVal = fileExporter.exportVBO(pGDData->_objectOpenGLName, fileType);
                        }
                        break;

                        case AF_TREE_ITEM_CL_BUFFER:
                        {
                            // Export buffer to disk:
                            retVal = fileExporter.exportCLBuffer(pGDData->_objectOpenCLIndex, fileType);
                        }
                        break;

                        case AF_TREE_ITEM_CL_SUB_BUFFER:
                        {
                            // Export sub-buffer to disk:
                            retVal = fileExporter.exportCLSubBuffer(pGDData->_objectOpenCLIndex, fileType);
                        }
                        break;

                        case AF_TREE_ITEM_GL_STATIC_BUFFER:
                        {
                            // Get static buffer type
                            apDisplayBuffer bufferType = pGDData->_bufferType;

                            // Export static buffer to disk
                            retVal = fileExporter.exportStaticBuffer(bufferType, fileType);
                        }
                        break;


                        case AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER:
                        {
                            // Get the PBuffer ID
                            int pbufferID = pGDData->_objectOpenGLName;

                            // Get the PBuffer spy context id
                            int pbufferContextID = pGDData->_contextId._contextId;

                            // Get the PBuffer static buffer type
                            apDisplayBuffer bufferType = pGDData->_bufferType;

                            // Export PBuffer to disk
                            retVal = fileExporter.exportPBuffer(pbufferID, bufferType, pbufferContextID, fileType);
                        }
                        break;

                        default:
                        {
                            // Unsupported file type
                            GT_ASSERT_EX(false, L"Unsupported file type!");
                            retVal = false;
                        }
                        break;
                    }
                }

                // Increment progress:
                afProgressBarWrapper::instance().incrementProgressBar();

                // If item was exported ok, return result vector
                if (retVal)
                {
                    fileExporter.getExportResult(exportResult);
                }
            }
            else
            {
                OS_OUTPUT_DEBUG_LOG(GD_STR_LogMsg_couldNotExportSpyData, OS_DEBUG_LOG_DEBUG);
            }
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImageDataView::onZoomIn
// Description: Zoom in the displayed image
// Author:      Sigal Algranaty
// Date:        26/12/2010
// ---------------------------------------------------------------------------
void gdImageDataView::onZoomIn()
{
    if (_currentZoomLevel < GD_IMAGES_AND_DATA_MAX_ZOOM_LEVEL)
    {
        // Change the zoom level
        int nextZoomLevel = getNextZoomLevel(_currentZoomLevel, false);

        // Did we not change it value?
        if (nextZoomLevel == 1)
        {
            ++nextZoomLevel;
        }

        // Let's check if new zoom level is possible
        if (isNewZoomLevelPossible(nextZoomLevel))
        {
            // Show wait cursor:
            setCursor(QCursor(Qt::BusyCursor));

            // Apply the zoom:
            _currentZoomLevel = nextZoomLevel;
            setFilterForAllImages(AC_IMAGE_ZOOM, _currentZoomLevel);

            // Show hourglass:
            setCursor(QCursor(Qt::ArrowCursor));
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImageDataViewsController::onZoomOut
// Description: Zoom out Tool was selected
// Author:      Eran Zinman
// Date:        21/5/2007
// ---------------------------------------------------------------------------
void gdImageDataView::onZoomOut()
{
    // Get the new zoom level
    int nextZoomLevel = getNextZoomLevel(_currentZoomLevel, true);

    // Apply the new zoom:
    setFilterForAllImages(AC_IMAGE_ZOOM, nextZoomLevel);

    // Set the new zoom level:
    _currentZoomLevel = nextZoomLevel;
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::getNextZoomLevel
// Description: Return the next/previous zoom level in the zoom levels array
// Arguments:   currentZoomLevel - The current zoom level.
//              upDirection - If true search up, otherwise, search down.
// Return Val:  The new zoom Level
// Author:      Eran Zinman
// Date:        13/6/2007
// ---------------------------------------------------------------------------
int gdImageDataView::getNextZoomLevel(unsigned int currentZoomLevel, bool upDirection) const
{
    // The new zoom level default value:
    int retVal = 1;

    // Get the amount of zoom levels:
    int amountOfZoomLevels = (int)_stat_availableZoomLevelsVector.size();

    // Loop through the zoom possible values:
    for (int i = 0; i < amountOfZoomLevels  ; i++)
    {
        // Get the current zoom level:
        unsigned int zoomLevel = _stat_availableZoomLevelsVector[i];

        // If we found a zoom level higher than our zoom level
        if (zoomLevel >= currentZoomLevel)
        {
            if (upDirection)//zooming out
            {
                if (i > 0)
                {
                    retVal = _stat_availableZoomLevelsVector[i - 1];
                }
            }
            else //zooming in
            {
                if (i < (amountOfZoomLevels - 1))
                {
                    retVal = _stat_availableZoomLevelsVector[i + 1];
                }
            }

            break;
        }
    }

    // Return the new zoom level
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataViewsController::shouldEnableZoom
// Description: Should we Enable / Disable Zoom in or out Tool
// Author:      Eran Zinman
// Date:        13/6/2007
// ---------------------------------------------------------------------------
bool gdImageDataView::shouldEnableZoom(int commandId) const
{
    bool retVal = false;

    // Only when image view if focused, zoom actions are enabled:
    retVal = isImageViewFocused();

    if (retVal)
    {
        // Zoom in:
        if (commandId == ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMIN)
        {
            retVal = retVal && (_currentZoomLevel < GD_IMAGES_AND_DATA_MAX_ZOOM_LEVEL);
        }
        else
        {
            retVal = retVal && (_currentZoomLevel > GD_IMAGES_AND_DATA_MIN_ZOOM_LEVEL);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataViewsController::setTextureManagerZoomLevel
// Description: Changes the image buffer view zoom level to the new one.
//              Notice
//              ======
//              This function doesn't check if the new zoom level is possible.
//              You must check if zoom level is possible before calling this
//              function.
// Arguments:   zoomLevel - New  zoom level
// Author:      Eran Zinman
// Date:        21/7/2007
// ---------------------------------------------------------------------------
void gdImageDataView::setTextureManagerZoomLevel(int zoomLevel)
{
    // Save the new zoom level
    if (_currentZoomLevel != zoomLevel)
    {
        // Change the images zoom level (inside the manager):
        setFilterForAllImages(AC_IMAGE_ZOOM, zoomLevel);
        _currentZoomLevel = zoomLevel;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::tryToChangeZoomLevel
// Description: Try to change a zoom level
// Return Val:  bool - true iff the new zoom level is possible for the displayed item
// Author:      Sigal Algranaty
// Date:        15/11/2010
// ---------------------------------------------------------------------------
bool gdImageDataView::tryToChangeZoomLevel(const gtString& enteredString)
{
    bool retVal = false;

    // Get selection text
    gtString comboSelection = enteredString;

    // Remove the percentage sign from the end
    comboSelection.removeTrailing('%');

    // Convert the string to number
    int zoomLevel;

    // If convert was successful
    if (comboSelection.toIntNumber(zoomLevel))
    {
        // Let's check if new zoom level is possible
        if (isNewZoomLevelPossible(zoomLevel))
        {
            // Set the texture manager new zoom level
            setTextureManagerZoomLevel(zoomLevel);

            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::isZoomComboBoxTextValid
// Description: Checks is the input text for a combo box is valid
// Arguments:   const gtString& enteredString - the entered text
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/11/2010
// ---------------------------------------------------------------------------
bool gdImageDataView::isZoomComboBoxTextValid(const gtString& enteredString)
{
    bool retVal = false;

    // Get the string that the user had entered
    gtString zoomString = enteredString;

    // Remove the percentage sign from the end (if exists)
    zoomString.removeTrailing('%');

    // Convert the string to number
    int zoomLevel;

    // If convert was successful
    if (zoomString.toIntNumber(zoomLevel))
    {
        retVal = true;

        // Check that the user entered a value within range
        if (zoomLevel > GD_IMAGES_AND_DATA_MAX_ZOOM_LEVEL || zoomLevel < GD_IMAGES_AND_DATA_MIN_ZOOM_LEVEL)
        {
            retVal = false;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::displayItemMessageIfNeeded
// Description: If the object was not displayed, get the appropriate message and display it
// Author:      Sigal Algranaty
// Date:        7/4/2011
// ---------------------------------------------------------------------------
void gdImageDataView::displayItemMessageIfNeeded()
{
    // Sanity check:
    if (_pDisplayedItemTreeData != nullptr)
    {
        // If the item load was failed, display the failure description:
        if (!_pDisplayedItemTreeData->isItemDisplayed())
        {
            // Get the failure message as string:
            gtString failureMsg, failureTitle;
            _pDisplayedItemTreeData->getItemStatusAsString(_pDisplayedItemTreeData->m_itemType, failureTitle, failureMsg);

            GT_IF_WITH_ASSERT(_pImageViewManager != nullptr)
            {
                // Write text only in thumbnail mode:
                _pImageViewManager->setManagerMode(AC_MANAGER_MODE_TEXT_ITEM);

                // Write the text message in thumbnail view:
                // TO_DO: Sigal errors: check the color
                _pImageViewManager->writeTextMessage(failureTitle, failureMsg);

                // Check if the control panel display should be forced:
                // This should be done only for multi watch variables, when the process is suspended:
                bool forceControlPanelDisplay = false;

                if (_pDisplayedItemTreeData->m_itemType == AF_TREE_ITEM_CL_KERNEL_VARIABLE)
                {
                    forceControlPanelDisplay = (_pDisplayedItemTreeData->_itemLoadStatus._loadStatusDescription != AF_ITEM_LOAD_PROCESS_IS_RUNNING);
                }

                // Set the new view state:
                _currentViewsDisplayProperties._viewState = GD_TEXT_ONLY;
                _currentViewsDisplayProperties._forceControlPanelDisplay = forceControlPanelDisplay;

                // Select the data view:
                displayRelevantNotebookPages();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::getCurrentFilterRawDataHandler
// Description: Default implementation for base class
// Return Val:  acRawFileHandler*
// Author:      Sigal Algranaty
// Date:        13/3/2011
// ---------------------------------------------------------------------------
acRawFileHandler* gdImageDataView::getCurrentFilterRawDataHandler(unsigned int index)
{
    (void)(index);  // unused
    return nullptr;
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::handleDataCellDoubleClick
// Description: Nothing to do for base class
// Arguments:    int canvasId
//              acImageItem* pImageItem
//              QPoint posOnImage
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/3/2011
// ---------------------------------------------------------------------------
bool gdImageDataView::handleDataCellDoubleClick(acImageItemID canvasId, acImageItem* pImageItem, QPoint posOnImage)
{
    (void)(canvasId);  // unused
    (void)(pImageItem);  // unused
    (void)(posOnImage);  // unused
    return true;
}

// ---------------------------------------------------------------------------
// Name:        gdImageDataView::setHexDisplayMode
// Description: Update the hexadecimal display flag for the multiwatch
// Arguments:   bool hexDisplayMode
// Author:      Sigal Algranaty
// Date:        26/5/2011
// ---------------------------------------------------------------------------
void gdImageDataView::setHexDisplayMode(bool hexDisplayMode)
{
    // Update the control panel with the hex display:
    GT_IF_WITH_ASSERT(_pImageControlPanel != nullptr)
    {
        // Clear the pixel information:
        _pImageControlPanel->clearPixelInformation(false);
    }

    // Get the data view from the multi watch view:
    acDataView* pDataView = dataView();
    GT_IF_WITH_ASSERT(pDataView != nullptr)
    {
        // Update the view acDataView:
        pDataView->setHexDisplayMode(hexDisplayMode);
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        onHexChanged
/// \brief Description: Handling the hex changed signal
/// \return True :
/// \return False:
/// -----------------------------------------------------------------------------------------------
void gdImageDataView::onHexChanged(bool displayHex)
{
    apHexChangedEvent hexEvent(displayHex);
    apEventsHandler::instance().registerPendingDebugEvent(hexEvent);
}



/// -----------------------------------------------------------------------------------------------
/// \brief Name:        onZoomChanged
/// \brief Description: Handling zoom change signal
/// -----------------------------------------------------------------------------------------------
void gdImageDataView::onZoomChanged(int nNewZoomLevel)
{
    _lastViewedItemProperties._zoomLevel = nNewZoomLevel;
    setTextureManagerZoomLevel(nNewZoomLevel);
}


bool gdImageDataView::zoomedImagePositionToImagePosition(acImageItem* pImageItem, const QPoint& posOnImage, QPoint& absolutePosOnImage, bool isHoveringImage, bool bFlipMatters)
{
    GT_UNREFERENCED_PARAMETER(isHoveringImage);
    bool bRetVal = false;

    absolutePosOnImage = posOnImage;
    GT_IF_WITH_ASSERT((pImageItem != nullptr))
    {
        // Get image size (and check that we got a valid size)
        QSize origImageSize = pImageItem->originalImageSize();

        GT_IF_WITH_ASSERT((origImageSize.width() >= 1) && (origImageSize.height() >= 1))
        {
            // Get mouse position. We flip Y position because in openGL the origin (0, 0) is bottom left
            int imageXPos = posOnImage.x();
            int imageYPos = posOnImage.y();

            if (bFlipMatters && m_bFlipImage)
            {
                imageYPos = origImageSize.height() - imageYPos - 1;
            }

            absolutePosOnImage = QPoint(imageXPos, imageYPos);
            bRetVal = true;
        }
    }
    return bRetVal;
}

void gdImageDataView::SaveSelectedPixelData(acImageItemID canvasId, acImageItem* pImageItem, QPoint posOnImage)
{
    m_nSelectedPixelImageItemID = canvasId;
    m_pSelectedPixelImageItem = pImageItem;
    m_pointSelectedPixelPosition = posOnImage;
    m_bPixelInfoSelected = true;
}

void gdImageDataView::wheelEvent(QWheelEvent* pEvent)
{
    GT_IF_WITH_ASSERT(nullptr != pEvent)
    {
        if (QApplication::keyboardModifiers().testFlag(Qt::ControlModifier) == true)
        {
            Qt::MouseButtons btns = pEvent->buttons();

            if (btns == Qt::NoButton)
            {
                //if mouse wheel was rotated away from user then zoom in, otherwise - zoom out
                if (pEvent->angleDelta().y() > 0)
                {
                    onZoomIn();
                }
                else
                {
                    onZoomOut();
                }
            }
        }
    }
}
