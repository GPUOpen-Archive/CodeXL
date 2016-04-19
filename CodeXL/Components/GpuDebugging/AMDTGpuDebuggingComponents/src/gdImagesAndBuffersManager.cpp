//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdImagesAndBuffersManager.cpp
///
//==================================================================================

//------------------------------ gdImagesAndBuffersManager.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/gdImagesAndBuffersManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdPropertiesEventObserver.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImageAndBufferView.h>

// Static members initializations:
gdImagesAndBuffersManager* gdImagesAndBuffersManager::_pMySingleInstance = nullptr;

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::gdImagesAndBuffersManager
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        15/12/2010
// ---------------------------------------------------------------------------
gdImagesAndBuffersManager::gdImagesAndBuffersManager():
    _pThumbnailView(nullptr), _pImageBufferView(nullptr)
{
    // Reset the controller:
    _imagesAndBuffersController.resetImageAndDataViews(true);
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::~gdImagesAndBuffersManager
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        15/12/2010
// ---------------------------------------------------------------------------
gdImagesAndBuffersManager::~gdImagesAndBuffersManager()
{
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::instance
// Description: Returns the single instance of this class. Creates it on
//              the first call to this function.
// Return Val:  gdImagesAndBuffersManager&
// Author:      Sigal Algranaty
// Date:        15/12/2010
// ---------------------------------------------------------------------------
gdImagesAndBuffersManager& gdImagesAndBuffersManager::instance()
{
    // If my single instance was not created yet - create it:
    if (_pMySingleInstance == nullptr)
    {
        _pMySingleInstance = new gdImagesAndBuffersManager;

    }

    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::applyLastViewedItemProperties
// Description:
// Arguments: bool applyZoom - should apply the last viewed item zoom settings
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/12/2010
// ---------------------------------------------------------------------------
void gdImagesAndBuffersManager::applyLastViewedItemProperties(bool applyZoom)
{
    // If the thumbnail view is shown:
    if (_imagesAndBuffersController.isThumbnailViewActive())
    {
        GT_IF_WITH_ASSERT(_pThumbnailView != nullptr)
        {
            _pThumbnailView->applyLastViewedItemProperties(_imagesAndBuffersController.lastViewedItemProperties());
        }
    }
    // The image / buffer view is shown:
    else
    {
        GT_IF_WITH_ASSERT(_pImageBufferView != nullptr)
        {
            _pImageBufferView->applyLastViewedItemProperties(_imagesAndBuffersController.lastViewedItemProperties(), applyZoom);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::isToolbarCommandEnabled
// Description: Used to check if a toolbar command is should be checked
// Arguments:   int commandId
//              bool &isEnabled
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/12/2010
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersManager::isToolbarCommandEnabled(int commandId)
{
    bool retVal = false;
    bool isChecked = false;

    // Call the controller:
    retVal = _imagesAndBuffersController.shouldCommandBeEnabled(isChecked, commandId);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::updateCheckableToolbarCommand
// Description: Checks if a command should be enabled / checked. Handling
//              all the toolbar update commands
// Arguments:   commandId - the command id
// Author:      Sigal Algranaty
// Date:        15/11/2010
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersManager::isToolbarCheckedCommandEnabled(int commandId, bool& isChecked)
{
    bool retVal = false;

    // Call the controller:
    retVal = _imagesAndBuffersController.shouldCommandBeEnabled(isChecked, commandId);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::resetActiveTools
// Description: Resets the active tool (enabled standard pointer and disables
//              everything else)
// Author:      Eran Zinman
// Date:        24/7/2007
// ---------------------------------------------------------------------------
void gdImagesAndBuffersManager::resetActiveTools()
{
    // TO_DO: VS textures and buffers viewer: implement
    /*
    if (_pToolsAndZoomToolBar != nullptr)
    {
    // Toggle the standard pointer tool
    _pToolsAndZoomToolBar->ToggleTool(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_NORMAL, true);

    // Un-toggle all other tools
    _pToolsAndZoomToolBar->ToggleTool(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_PAN, false);
    }
    */
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::resetImageAndDataViews
// Description: Resets the image and data views
// Author:      Eran Zinman
// Date:        30/11/2007
// ---------------------------------------------------------------------------
void gdImagesAndBuffersManager::resetImageAndDataViews(bool resetFilters)
{
    // Reset in controller:
    _imagesAndBuffersController.resetImageAndDataViews(resetFilters);

    if (resetFilters)
    {
        // Reset all the active filters
        _imagesAndBuffersController.resetActiveFilters();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::loadItemIntoViewer
// Description: Loads an item into the textures and buffers viewer
// Arguments:   pViewerItem - The item details to load
// Author:      Eran Zinman
// Date:        25/11/2007
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersManager::loadItemIntoViewer(afApplicationTreeItemData* pViewerItem)
{
    bool retVal = false;

    // Contain true iff the item was loaded successfully:
    bool rcLoadItem = false;
    bool rcDisplayProperties = false;

    // Sanity Check:
    GT_IF_WITH_ASSERT(pViewerItem != nullptr)
    {
        // Did we have any selected item?
        if (pViewerItem->m_itemType != AF_TREE_ITEM_ITEM_NONE)
        {
            // First let's clean the textures and buffers viewer:
            // NOTICE: Do not reset filters:
            resetImageAndDataViews(false);

            // Load viewer item according to the item type
            afTreeItemType itemType = pViewerItem->m_itemType;

            // Check if debugged process is suspended:
            bool isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

            // If the item is a thumbnail, display it in the thumbnail view:
            if (afApplicationTreeItemData::isItemTypeRoot(itemType))
            {
                GT_IF_WITH_ASSERT(_pThumbnailView != nullptr)
                {
                    // Set the focused thumbnail view:
                    _imagesAndBuffersController.setFocusedViews(nullptr, _pThumbnailView, isDebuggedProcessSuspended);

                    // Ask the thumbnail view to display thumbnail item:
                    rcLoadItem = _pThumbnailView->displayThumbnailItem(pViewerItem);
                }
            }
            else
            {
                // Display the item in the single item view:
                GT_IF_WITH_ASSERT(_pImageBufferView != nullptr)
                {
                    // Set the focused image / buffer view:
                    _imagesAndBuffersController.setFocusedViews(_pImageBufferView, nullptr, isDebuggedProcessSuspended);

                    rcLoadItem = _pImageBufferView->displayItem(pViewerItem);
                }
            }

            // Display the thumbnail view item in properties view:
            rcDisplayProperties = gdPropertiesEventObserver::instance().displayItemProperties(pViewerItem, false, false, true);

            // Adjust the image and data views after loading the item
            adjustViewerAfterItemLoading(pViewerItem);
        }
    }

    // We are done loading the item, mark the the last viewed item properties are no longer valid
    _imagesAndBuffersController.resetDisplayedProperties();

    retVal = rcDisplayProperties && rcLoadItem;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::updateRGBChannelsStatus
// Description: Updates the RGB channels buttons enabled status and
//              linked status
// Author:      Eran Zinman
// Date:        8/1/2008
// ---------------------------------------------------------------------------
void gdImagesAndBuffersManager::updateRGBChannelsStatus(afApplicationTreeItemData* pLoadedItemData)
{
    if (pLoadedItemData != nullptr)
    {
        // Apply this raw data handler properties to the possible image actions:
        _imagesAndBuffersController.enableImageActionsByItemType(pLoadedItemData->m_itemType);

        if (_imagesAndBuffersController.isThumbnailViewActive())
        {
            // Sanity check:
            GT_IF_WITH_ASSERT(_pThumbnailView != nullptr)
            {
                _pThumbnailView->setFilterForAllImages(_imagesAndBuffersController.lastViewedItemProperties()._actionsMask);
            }
        }
        else
        {
            // Sanity check:
            GT_IF_WITH_ASSERT(_pImageBufferView != nullptr)
            {
                // Apply RGBA filters to the images:
                _pImageBufferView->setFilterForAllImages(_imagesAndBuffersController.lastViewedItemProperties()._actionsMask);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::adjustViewerAfterItemLoading
// Description: Adjust the textures and buffers viewer after loading an
//              item (texture / buffer / thumbnail view / etc...)
// Arguments:   gdDebugApplicationTreeData* pLoadedItem - the loaded item data
// Author:      Sigal Algranaty
// Date:        15/12/2010
// ---------------------------------------------------------------------------
void gdImagesAndBuffersManager::adjustViewerAfterItemLoading(afApplicationTreeItemData* pLoadedItemData)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((_pImageBufferView != nullptr) && (pLoadedItemData != nullptr))
    {
        if (!_imagesAndBuffersController.isThumbnailViewActive())
        {
            // Adjust the viewer:
            _pImageBufferView->adjustViewerAfterItemLoading();
        }

        // Update image view RGBA enabled and linked status
        updateRGBChannelsStatus(pLoadedItemData);

        // Set zoom level to "Best Fit" level:
        if (!_imagesAndBuffersController.isThumbnailViewActive())
        {
            // Set the image view zoom level:
            int zoomLevel = 0;
            _pImageBufferView->applyBestFit(zoomLevel);

            // Set the texture manager new zoom level
            _pImageBufferView->setTextureManagerZoomLevel(zoomLevel);
        }

        // Adjust the manager layout after adding the texture
        _pImageBufferView->forceImageManagerRepaint();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::toggleToolsForCommandEvent
// Description: Toogle tool for a specific command event
// Arguments:   wxCommandEvent& event
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        15/12/2010
// ---------------------------------------------------------------------------
void gdImagesAndBuffersManager::toggleToolsForCommandEvent(int commandId)
{
    (void)(commandId);  // unused

    // TO_DO: VS textures and buffers viewer: implement me!
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::onToolbarEvent
// Description: Handling the toolbar and menu items events
// Arguments:   wxCommandEvent& event
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        15/11/2010
// ---------------------------------------------------------------------------
void gdImagesAndBuffersManager::onToolbarEvent(int eventId)
{
    // Handle the event by its id (call the controller right function):
    switch (eventId)
    {
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_NORMAL:
            _imagesAndBuffersController.onStandardPointer();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_PAN:
            _imagesAndBuffersController.onPan();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMIN:
            _imagesAndBuffersController.onZoomIn();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMOUT:
            _imagesAndBuffersController.onZoomOut();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BEST_FIT:
            _imagesAndBuffersController.onBestFit();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_ORIGINAL_SIZE:
            _imagesAndBuffersController.onOriginalSize();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_RED_CHANNEL:
            _imagesAndBuffersController.onColorChannel(AC_IMAGE_CHANNEL_RED);
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GREEN_CHANNEL:
            _imagesAndBuffersController.onColorChannel(AC_IMAGE_CHANNEL_GREEN);
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BLUE_CHANNEL:
            _imagesAndBuffersController.onColorChannel(AC_IMAGE_CHANNEL_BLUE);
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ALPHA_CHANNEL:
            _imagesAndBuffersController.onColorChannel(AC_IMAGE_CHANNEL_ALPHA);
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GRAYSCALE:
            _imagesAndBuffersController.onGrayscale();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_INVERT:
            _imagesAndBuffersController.onInvert();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_LEFT:
            _imagesAndBuffersController.onRotateLeft();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_RIGHT:
            _imagesAndBuffersController.onRotateRight();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_ALL_AS_IMAGE:
            _imagesAndBuffersController.onSaveAllAsImage();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_ALL_AS_RAWDATA:
            _imagesAndBuffersController.onSaveAllAsRawData();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_COPY:
            _imagesAndBuffersController.onEditCopy();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_SELECT_ALL:
            _imagesAndBuffersController.onEditSelectAll();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_SELECT_IMAGE_VIEW:
            _imagesAndBuffersController.onMenuImageViewSelected();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_SELECT_DATA_VIEW:
            _imagesAndBuffersController.onMenuDataViewSelected();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_RAWDATA_AS:
            _imagesAndBuffersController.onSaveRawDataAs(_imagesAndBuffersController.getCurrentlySelectedViewerItem());
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_SAVE_IMAGE_AS:
            _imagesAndBuffersController.onSaveImageAs(_imagesAndBuffersController.getCurrentlySelectedViewerItem());
            break;

        default:
            GT_ASSERT_EX(false, L"Unsupported command id");

    }

    // Toggle tools for this command:
    toggleToolsForCommandEvent(eventId);

}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::createNewImageAndBufferView
// Description: Create a new view for an image / buffer object
// Arguments:   QWidget* pParentWindow
//              wxSize viewSize
// Return Val:  gdImageAndBufferView*
// Author:      Sigal Algranaty
// Date:        18/10/2010
// ---------------------------------------------------------------------------
gdImageAndBufferView* gdImagesAndBuffersManager::createNewImageAndBufferView(QWidget* pParentWindow, const QSize& viewSize)
{
    gdImageAndBufferView* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(gdDebugApplicationTreeHandler::instance() != nullptr)
    {
        // Create a new view for the selected object:
        pRetVal = new gdImageAndBufferView(pParentWindow, &afProgressBarWrapper::instance(), gdDebugApplicationTreeHandler::instance());


        // Set the view size:
        pRetVal->resize(viewSize);

        // Set the view frame layout:
        pRetVal->setFrameLayout(viewSize);
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::createNewThumbnailView
// Description: Create a new view for an images / buffers thumbnails
// Arguments:   QWidget* pParentWindow
//              const QSize& viewSize
// Return Val:  gdThumbnailView*
// Author:      Sigal Algranaty
// Date:        27/12/2010
// ---------------------------------------------------------------------------
gdThumbnailView* gdImagesAndBuffersManager::createNewThumbnailView(QWidget* pParentWindow, const QSize& viewSize)
{
    gdThumbnailView* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(gdDebugApplicationTreeHandler::instance() != nullptr)
    {
        // Create a new view for the selected object:
        pRetVal = new gdThumbnailView(pParentWindow, &afProgressBarWrapper::instance(), gdDebugApplicationTreeHandler::instance());


        // Set the view size:
        pRetVal->resize(viewSize);

        // Set the view frame layout:
        pRetVal->setFrameLayout(viewSize);
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::displayItem
// Description: Display an item in the image buffer view
// Arguments:   gdImageAndBufferView* pImageAndBufferView
//              const gtString& itemName
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/12/2010
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersManager::displayItem(gdImageAndBufferView* pImageAndBufferView, const gtString& itemName)
{
    bool retVal = false;

    // Set the current image and buffer view:
    _pImageBufferView = pImageAndBufferView;

    // Get the object id from the object text:
    afApplicationTreeItemData itemID;
    gdDebugApplicationTreeData* pExtendedData = new gdDebugApplicationTreeData;

    itemID.setExtendedData(pExtendedData);
    int additionalParameter = -1;
    retVal = gdHTMLProperties::htmlLinkToObjectDetails(itemName, itemID, additionalParameter);

    if (retVal && (_pImageBufferView != nullptr))
    {
        // Get the requested tree item data:
        afApplicationTreeItemData* pItemData = nullptr;
        GT_IF_WITH_ASSERT(gdDebugApplicationTreeHandler::instance() != nullptr)
        {
            // Get the item data belongs to the matching item:
            pItemData = gdDebugApplicationTreeHandler::instance()->FindMatchingTreeItem(itemID);

            // Display the item data:
            retVal = displayItem(pImageAndBufferView, pItemData);
        }
    }
    else
    {
        // Set the displayed item id:
        _pImageBufferView->setDisplayedItemID(itemID);

        // Display the object:
        bool doesObjectExist = false;
        _pImageBufferView->updateObjectDisplay(doesObjectExist);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::displayItem
// Description: Display the item data
// Arguments:   gdImageAndBufferView* pImageBufferView
//              afApplicationTreeItemData* pItemData
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        16/9/2011
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersManager::displayItem(gdImageAndBufferView* pImageBufferView, afApplicationTreeItemData* pItemData)
{
    bool retVal = false;

    // Set the image / buffer view:
    _pImageBufferView = pImageBufferView;
    GT_IF_WITH_ASSERT(_pImageBufferView != nullptr)
    {
        // Notify the controller that this image / buffer view is currently focused:
        _imagesAndBuffersController.openImageBufferView(_pImageBufferView);

        // Check if debugged process is suspended:
        bool isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

        // Set the image / buffer view as the active view:
        _imagesAndBuffersController.setFocusedViews(_pImageBufferView, nullptr, isDebuggedProcessSuspended);

        // If the item exist in the tree, display it:
        if (pItemData != nullptr)
        {
            // Load the requested item:
            retVal = _pImageBufferView->displayItem(pItemData);

            afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
            GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
            {
                afApplicationTree* pApplicationTree = afApplicationCommands::instance()->applicationTree();
                GT_IF_WITH_ASSERT(pApplicationTree != nullptr)
                {
                    // Add the item data to the selected items list:
                    pApplicationTree->addSelectedItem(pItemData, true);

                    // Adjust the item after loading:
                    _pImageBufferView->adjustViewerAfterItemLoading();

                    // Update image view RGBA enabled and linked status
                    updateRGBChannelsStatus(pItemData);

                    // Calculate the current sizes of the image objects within the manager:
                    _pImageBufferView->calculateImagesLayout();

                    // Adjust the manager layout after adding the texture
                    _pImageBufferView->forceImageManagerRepaint();

                    // We are done loading the item, mark the the last viewed item properties are no longer valid
                    _imagesAndBuffersController.resetDisplayedProperties();
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::displayThumbnailItem
// Description: Display an item in the thumbnail view
// Arguments:   gdThumbnailView* pThumbnailView - the view to display the object in
//              const gtString& itemName
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/12/2010
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersManager::displayThumbnailItem(gdThumbnailView* pThumbnailView, const gtString& itemName)
{
    bool retVal = false;

    // Get the object id from the object text:
    afApplicationTreeItemData itemID;
    gdDebugApplicationTreeData* pExtendedData = new gdDebugApplicationTreeData;

    itemID.setExtendedData(pExtendedData);
    int additionalParameter = -1;
    retVal = gdHTMLProperties::htmlLinkToObjectDetails(itemName, itemID, additionalParameter);

    if (retVal)
    {
        // Get the requested tree item data:
        afApplicationTreeItemData* pItemData = nullptr;

        // Get the object details out of the file name:
        if (gdDebugApplicationTreeHandler::instance() != nullptr)
        {
            // Get the requested tree item data:
            pItemData = gdDebugApplicationTreeHandler::instance()->FindMatchingTreeItem(itemID);
        }

        // Display the thumbnail view:
        retVal = displayThumbnailItem(pThumbnailView, pItemData);

    }
    else
    {
        // Set the displayed item id:
        _pThumbnailView->setDisplayedItemID(itemID);

        // Display the object:
        _pThumbnailView->updateObjectDisplay();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::displayThumbnailItem
// Description: Display the thumbnail view
// Arguments:   gdThumbnailView* pThumbnailView
//              gdDebugApplicationTreeData* pItemData
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        16/9/2011
// ---------------------------------------------------------------------------
bool gdImagesAndBuffersManager::displayThumbnailItem(gdThumbnailView* pThumbnailView, afApplicationTreeItemData* pItemData)
{
    bool retVal = false;

    // Set the current thumbnail view:
    _pThumbnailView = pThumbnailView;

    // Check if debugged process is suspended:
    bool isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

    // Notify the controller for the new thumbnail view:
    _imagesAndBuffersController.openThumbnailView(_pThumbnailView);

    if (pItemData != nullptr)
    {
        // Load the requested item:
        retVal = pThumbnailView->displayThumbnailItem(pItemData);

        // Set the current thumbnail view as focused:
        _imagesAndBuffersController.setFocusedViews(nullptr, _pThumbnailView, isDebuggedProcessSuspended);

        // Update image view RGBA enabled and linked status
        updateRGBChannelsStatus(pItemData);

        // We are done loading the item, mark the the last viewed item properties are no longer valid
        _imagesAndBuffersController.resetDisplayedProperties();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::onImageBufferSize
// Description: Is handling the size change of an image / buffer window
// Arguments:   gdImageAndBufferView* pImageBufferView
//              wxSize windowSize
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        26/12/2010
// ---------------------------------------------------------------------------
void gdImagesAndBuffersManager::onImageBufferSize(gdImageAndBufferView* pImageBufferView, const QSize& windowSize)
{
    // Sanity check:
    if (pImageBufferView != nullptr)
    {
        if (windowSize != QSize(0, 0))
        {
            // Adjust the item after loading:
            pImageBufferView->adjustViewerAfterItemLoading();

            // Update image view RGBA enabled and linked status
            updateRGBChannelsStatus(pImageBufferView->displayedItemData());

            // Set the image view zoom level:
            int zoomLevel = 0;
            pImageBufferView->applyBestFit(zoomLevel);

            // Set the texture manager new zoom level
            pImageBufferView->setTextureManagerZoomLevel(zoomLevel);

            // Adjust the manager layout after adding the texture
            pImageBufferView->forceImageManagerRepaint();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImagesAndBuffersManager::updateviewOnEvent
// Description: Update the opened images / buffers / thumbnail views
// Arguments:   apEvent::EventType eventType
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        29/12/2010
// ---------------------------------------------------------------------------
void gdImagesAndBuffersManager::updateOpenedViewsOnEvent(bool isSuspended)
{
    if (gdDebugApplicationTreeHandler::instance() != nullptr)
    {
        if (isSuspended)
        {
            // Update view on process run suspension:
            gdDebugApplicationTreeHandler::instance()->updateMonitoredObjectsTree();
        }
    }

    // Ask the images and buffers controller to update the views:
    _imagesAndBuffersController.updateOpenedViewsOnEvent(isSuspended);
}


/// -----------------------------------------------------------------------------------------------
/// \brief Name:         updateActiveViewFocus
/// \brief Description:  Updates the active view focus
/// \param[in]           pActiveView pointer to QWidget - should be casted to appropriate view
/// -----------------------------------------------------------------------------------------------
void gdImagesAndBuffersManager::updateActiveViewFocus(QWidget* pActiveView)
{
    // Check if debugged process is suspended:
    bool isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

    gdImageAndBufferView* pImageAndBufferView = dynamic_cast<gdImageAndBufferView*>(pActiveView);

    if (pImageAndBufferView != nullptr)
    {
        // Set the focused thumbnail view:
        _imagesAndBuffersController.setFocusedViews(pImageAndBufferView, nullptr, isDebuggedProcessSuspended);
    }
    else
    {
        gdThumbnailView* pThumbnailView = dynamic_cast<gdThumbnailView*>(pActiveView);

        if (pThumbnailView != nullptr)
        {
            _imagesAndBuffersController.setFocusedViews(nullptr, pThumbnailView, isDebuggedProcessSuspended);
        }
    }
}
