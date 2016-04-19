//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspImagesAndBuffersManager.cpp
///
//==================================================================================

//------------------------------ vspImagesAndBuffersManager.cpp ------------------------------

#include "stdafx.h"

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/views/afPropertiesView.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTree.h>

// AMDTGpuDebuggingComponents
#include <AMDTGpuDebuggingComponents/Include/views/gdImageAndBufferView.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdPropertiesEventObserver.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>

// Local:
#include <src/vspImagesAndBuffersManager.h>
#include <src/vspWindowsManager.h>

// Static members initializations:
vspImagesAndBuffersManager* vspImagesAndBuffersManager::_pMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        vspImagesAndBuffersManager::vspImagesAndBuffersManager
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        15/12/2010
// ---------------------------------------------------------------------------
vspImagesAndBuffersManager::vspImagesAndBuffersManager():
    _pMonitoredObjectsExplorer(NULL), _pPropertiesView(NULL), _pThumbnailView(NULL),
    _pImageBufferView(NULL)
{
}

// ---------------------------------------------------------------------------
// Name:        vspImagesAndBuffersManager::~vspImagesAndBuffersManager
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        15/12/2010
// ---------------------------------------------------------------------------
vspImagesAndBuffersManager::~vspImagesAndBuffersManager()
{
}

// ---------------------------------------------------------------------------
// Name:        vspImagesAndBuffersManager::instance
// Description: Returns the single instance of this class. Creates it on
//              the first call to this function.
// Return Val:  vspImagesAndBuffersManager&
// Author:      Sigal Algranaty
// Date:        15/12/2010
// ---------------------------------------------------------------------------
vspImagesAndBuffersManager& vspImagesAndBuffersManager::instance()
{
    // If my single instance was not created yet - create it:
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new vspImagesAndBuffersManager;

    }

    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        vspImagesAndBuffersManager::applyLastViewedItemProperties
// Description:
// Arguments: bool applyZoom - should apply the last viewed item zoom settings
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/12/2010
// ---------------------------------------------------------------------------
void vspImagesAndBuffersManager::applyLastViewedItemProperties(bool applyZoom)
{
    // If the thumbnail view is shown:
    if (_imagesAndBuffersController.isThumbnailViewActive())
    {
        GT_IF_WITH_ASSERT(_pThumbnailView != NULL)
        {
            _pThumbnailView->applyLastViewedItemProperties(_imagesAndBuffersController.lastViewedItemProperties());
        }
    }
    // The image / buffer view is shown:
    else
    {
        GT_IF_WITH_ASSERT(_pImageBufferView != NULL)
        {
            _pImageBufferView->applyLastViewedItemProperties(_imagesAndBuffersController.lastViewedItemProperties(), applyZoom);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspImagesAndBuffersManager::isToolbarCommandEnabled
// Description: Used to check if a toolbar command is should be checked
// Arguments:   int commandId
//              bool &isEnabled
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/12/2010
// ---------------------------------------------------------------------------
bool vspImagesAndBuffersManager::isToolbarCommandEnabled(int commandId)
{
    bool retVal = false;
    bool isChecked = false;

    // Call the controller:
    retVal = _imagesAndBuffersController.shouldCommandBeEnabled(isChecked, commandId);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspImagesAndBuffersManager::updateCheckableToolbarCommand
// Description: Checks if a command should be enabled / checked. Handling
//              all the toolbar update commands
// Arguments:   commandId - the command id
// Author:      Sigal Algranaty
// Date:        15/11/2010
// ---------------------------------------------------------------------------
bool vspImagesAndBuffersManager::isToolbarCheckedCommandEnabled(int commandId, bool& isChecked)
{
    bool retVal = false;

    // Call the controller:
    retVal = _imagesAndBuffersController.shouldCommandBeEnabled(isChecked, commandId);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspImagesAndBuffersManager::resetActiveTools
// Description: Resets the active tool (enabled standard pointer and disables
//              everything else)
// Author:      Eran Zinman
// Date:        24/7/2007
// ---------------------------------------------------------------------------
void vspImagesAndBuffersManager::resetActiveTools()
{
    // TO_DO: VS textures and buffers viewer: implement
    /*
    if (_pToolsAndZoomToolBar != NULL)
    {
    // Toggle the standard pointer tool
    _pToolsAndZoomToolBar->ToggleTool(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_NORMAL, true);

    // Un-toggle all other tools
    _pToolsAndZoomToolBar->ToggleTool(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_PAN, false);
    }
    */
}

// ---------------------------------------------------------------------------
// Name:        vspImagesAndBuffersManager::resetImageAndDataViews
// Description: Resets the image and data views
// Author:      Eran Zinman
// Date:        30/11/2007
// ---------------------------------------------------------------------------
void vspImagesAndBuffersManager::resetImageAndDataViews(bool resetFilters)
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
// Name:        vspImagesAndBuffersManager::loadItemIntoViewer
// Description: Loads an item into the textures and buffers viewer
// Arguments:   pViewerItem - The item details to load
// Author:      Eran Zinman
// Date:        25/11/2007
// ---------------------------------------------------------------------------
bool vspImagesAndBuffersManager::loadItemIntoViewer(afApplicationTreeItemData* pViewerItem)
{
    bool retVal = false;

    // Contain true iff the item was loaded successfully:
    bool rcLoadItem = false;
    bool rcDisplayProperties = false;

    // Sanity Check:
    GT_IF_WITH_ASSERT(pViewerItem != NULL)
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
                GT_IF_WITH_ASSERT(_pThumbnailView != NULL)
                {
                    // Set the focused thumbnail view:
                    _imagesAndBuffersController.setFocusedViews(NULL, _pThumbnailView, isDebuggedProcessSuspended);

                    // Ask the thumbnail view to display thumbnail item:
                    rcLoadItem = _pThumbnailView->displayThumbnailItem(pViewerItem);
                }
            }
            else
            {
                // Display the item in the single item view:
                GT_IF_WITH_ASSERT(_pImageBufferView != NULL)
                {
                    // Set the focused image / buffer view:
                    _imagesAndBuffersController.setFocusedViews(_pImageBufferView, NULL, isDebuggedProcessSuspended);

                    rcLoadItem = _pImageBufferView->displayItem(pViewerItem);
                }
            }

            // Display the thumbnail object in properties view:
            // Sanity check:
            GT_IF_WITH_ASSERT(_pPropertiesView != NULL)
            {
                // Display the thumbnail view item in properties view:
                rcDisplayProperties = gdPropertiesEventObserver::instance().displayItemProperties(pViewerItem, false, false, true);
            }

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
// Name:        vspImagesAndBuffersManager::updateRGBChannelsStatus
// Description: Updates the RGB channels buttons enabled status and
//              linked status
// Author:      Eran Zinman
// Date:        8/1/2008
// ---------------------------------------------------------------------------
void vspImagesAndBuffersManager::updateRGBChannelsStatus(afApplicationTreeItemData* pLoadedItemData)
{
    if (pLoadedItemData != NULL)
    {
        // Apply this raw data handler properties to the possible image actions:
        _imagesAndBuffersController.enableImageActionsByItemType(pLoadedItemData->m_itemType);

        if (_imagesAndBuffersController.isThumbnailViewActive())
        {
            // Sanity check:
            GT_IF_WITH_ASSERT(_pThumbnailView != NULL)
            {
                _pThumbnailView->setFilterForAllImages(_imagesAndBuffersController.lastViewedItemProperties()._actionsMask);
            }
        }
        else
        {
            // Sanity check:
            GT_IF_WITH_ASSERT(_pImageBufferView != NULL)
            {
                // Apply RGBA filters to the images:
                _pImageBufferView->setFilterForAllImages(_imagesAndBuffersController.lastViewedItemProperties()._actionsMask);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspImagesAndBuffersManager::adjustViewerAfterItemLoading
// Description: Adjust the textures and buffers viewer after loading an
//              item (texture / buffer / thumbnail view / etc...)
// Arguments:   afApplicationTreeItemData* pLoadedItem - the loaded item data
// Author:      Sigal Algranaty
// Date:        15/12/2010
// ---------------------------------------------------------------------------
void vspImagesAndBuffersManager::adjustViewerAfterItemLoading(afApplicationTreeItemData* pLoadedItemData)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((_pImageBufferView != NULL) && (pLoadedItemData != NULL))
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
// Name:        vspImagesAndBuffersManager::toggleToolsForCommandEvent
// Description: Toogle tool for a specific command event
// Arguments:   wxCommandEvent& event
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        15/12/2010
// ---------------------------------------------------------------------------
void vspImagesAndBuffersManager::toggleToolsForCommandEvent(int commandId)
{
    GT_UNREFERENCED_PARAMETER(commandId);
    // TO_DO: VS textures and buffers viewer: implement me!
}

// ---------------------------------------------------------------------------
// Name:        vspImagesAndBuffersManager::onToolbarEvent
// Description: Handling the toolbar and menu items events
// Arguments:   wxCommandEvent& event
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        15/11/2010
// ---------------------------------------------------------------------------
void vspImagesAndBuffersManager::onToolbarEvent(gdImageActionId eventId)
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

        default:
            GT_ASSERT_EX(false, L"Unsupported command id");

    }

    // Toggle tools for this command:
    toggleToolsForCommandEvent(eventId);

}


// ---------------------------------------------------------------------------
// Name:        vspImagesAndBuffersManager::createNewImageAndBufferView
// Description: Create a new view for an image / buffer object
// Arguments:   wxWindow* pParentWindow
//              QSize viewSize
// Return Val:  gdImageAndBufferView*
// Author:      Sigal Algranaty
// Date:        18/10/2010
// ---------------------------------------------------------------------------
gdImageAndBufferView* vspImagesAndBuffersManager::createNewImageAndBufferView(QWidget* pParentWindow, QSize viewSize)
{
    gdImageAndBufferView* pRetVal = NULL;

    // Create a new view for the selected object:
    pRetVal = new gdImageAndBufferView(pParentWindow, &afProgressBarWrapper::instance(), gdDebugApplicationTreeHandler::instance());


    // Set the view size:
    pRetVal->resize(viewSize);

    // Set the view frame layout:
    pRetVal->setFrameLayout(viewSize);

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        vspImagesAndBuffersManager::createNewThumbnailView
// Description: Create a new view for an images / buffers thumbnails
// Arguments:   wxWindow* pParentWindow
//              wxSize viewSize
// Return Val:  gdThumbnailView*
// Author:      Sigal Algranaty
// Date:        27/12/2010
// ---------------------------------------------------------------------------
gdThumbnailView* vspImagesAndBuffersManager::createNewThumbnailView(QWidget* pParentWindow, QSize viewSize)
{
    gdThumbnailView* pRetVal = NULL;

    // Create a new view for the selected object:
    pRetVal = new gdThumbnailView(pParentWindow, &afProgressBarWrapper::instance(), gdDebugApplicationTreeHandler::instance());


    // Set the view size:
    pRetVal->resize(viewSize);

    // Set the view frame layout:
    pRetVal->setFrameLayout(viewSize);

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        vspImagesAndBuffersManager::displayItem
// Description: Display an item in the image buffer view
// Arguments:   gdImageAndBufferView* pImageAndBufferView
//              const gtString& itemName
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/12/2010
// ---------------------------------------------------------------------------
bool vspImagesAndBuffersManager::displayItem(gdImageAndBufferView* pImageAndBufferView, const gtString& itemName)
{
    bool retVal = false;

    // Set the current image and buffer view:
    _pImageBufferView = pImageAndBufferView;

    // Check if debugged process is suspended:
    bool isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

    // Get the object id from the object text:
    afApplicationTreeItemData itemID;
    itemID.setExtendedData(new gdDebugApplicationTreeData);

    int additionalParameter = -1;
    retVal = gdHTMLProperties::htmlLinkToObjectDetails(itemName, itemID, additionalParameter);
    GT_IF_WITH_ASSERT(retVal && (_pImageBufferView != NULL))
    {
        // Get the requested tree item data:
        afApplicationTreeItemData* pItemData = NULL;

        if (_pMonitoredObjectsExplorer == NULL)
        {
            // Get the object details out of the file name:
            _pMonitoredObjectsExplorer = vspWindowsManager::instance().monitoredObjectsTree(NULL, QSize(-1, -1));
        }


        // Notify the controller that this image / buffer view is currently focused:
        _imagesAndBuffersController.openImageBufferView(_pImageBufferView);

        // Set the image / buffer view as the active view:
        _imagesAndBuffersController.setFocusedViews(_pImageBufferView, NULL, isDebuggedProcessSuspended);

        // If the tree was already created:
        if (_pMonitoredObjectsExplorer != NULL)
        {
            // Get the item data belongs to the matching item:
            pItemData = (afApplicationTreeItemData*)gdDebugApplicationTreeHandler::instance()->FindMatchingTreeItem(itemID);

            // If the item exist in the tree, display it:
            if (pItemData != NULL)
            {
                // Load the requested item:
                bool rcDisplayItem = _pImageBufferView->displayItem(pItemData);
                GT_ASSERT(rcDisplayItem);

                // Add the item data to the selected items list:
                _pMonitoredObjectsExplorer->addSelectedItem(pItemData, true);

                // Adjust the item after load:
                _pImageBufferView->adjustViewerAfterItemLoading();

                // Adjust the item after loading:
                _pImageBufferView->adjustViewerAfterItemLoading();

                // Update image view RGBA enabled and linked status
                updateRGBChannelsStatus(pItemData);

                // Set the image view zoom level:
                int zoomLevel = 0;
                _pImageBufferView->applyBestFit(zoomLevel);

                // Set the texture manager new zoom level
                _pImageBufferView->setTextureManagerZoomLevel(zoomLevel);

                // Execute a dummy size event:
                _pImageBufferView->repaint();
                _pImageBufferView->forceImageManagerRepaint();

                // We are done loading the item, mark the the last viewed item properties are no longer valid
                _imagesAndBuffersController.resetDisplayedProperties();
            }
            else
            {
                // Set the displayed item id:
                _pImageBufferView->setDisplayedItemID(itemID);

                // Display the object:
                bool doesObjectExist = false;
                _pImageBufferView->updateObjectDisplay(doesObjectExist);
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
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        vspImagesAndBuffersManager::displayThumbnailItem
// Description: Display an item in the thumbnail view
// Arguments:   gdThumbnailView* pThumbnailView - the view to display the object in
//              const gtString& itemName
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/12/2010
// ---------------------------------------------------------------------------
bool vspImagesAndBuffersManager::displayThumbnailItem(gdThumbnailView* pThumbnailView, const gtString& itemName)
{
    bool retVal = false;

    // Set the current thumbnail view:
    _pThumbnailView = pThumbnailView;

    // Check if debugged process is suspended:
    bool isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

    // Get the object id from the object text:
    afApplicationTreeItemData itemID;
    itemID.setExtendedData(new gdDebugApplicationTreeData);

    int additionalParameter = -1;
    retVal = gdHTMLProperties::htmlLinkToObjectDetails(itemName, itemID, additionalParameter);
    GT_IF_WITH_ASSERT(retVal)
    {
        // Get the requested tree item data:
        afApplicationTreeItemData* pItemData = NULL;

        // Get the object details out of the file name:
        afApplicationTree* pObjectsTree = vspWindowsManager::instance().monitoredObjectsTree(NULL, QSize(-1, -1));

        if (pObjectsTree != NULL)
        {
            // Get the requested tree item data:
            pItemData = (afApplicationTreeItemData*)gdDebugApplicationTreeHandler::instance()->FindMatchingTreeItem(itemID);
        }

        // Notify the controller for the new thumbnail view:
        _imagesAndBuffersController.openThumbnailView(_pThumbnailView);

        if (pItemData != NULL)
        {
            // Set the current thumbnail view as focused:
            _imagesAndBuffersController.setFocusedViews(NULL, _pThumbnailView, isDebuggedProcessSuspended);

            // Load the requested item:
            bool rcDisplayItem = pThumbnailView->displayThumbnailItem(pItemData);
            GT_ASSERT(rcDisplayItem);


            // Update image view RGBA enabled and linked status
            updateRGBChannelsStatus(pItemData);

            // We are done loading the item, mark the the last viewed item properties are no longer valid
            _imagesAndBuffersController.resetDisplayedProperties();
        }

        else
        {
            // Set the displayed item id:
            _pThumbnailView->setDisplayedItemID(itemID);

            // Display the object:
            _pThumbnailView->updateObjectDisplay();
        }
    }


    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspImagesAndBuffersManager::onImageBufferSize
// Description: Is handling the size change of an image / buffer window
// Arguments:   gdImageAndBufferView* pImageBufferView
//              wxSize windowSize
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        26/12/2010
// ---------------------------------------------------------------------------
void vspImagesAndBuffersManager::onImageBufferSize(gdImageAndBufferView* pImageBufferView, QSize windowSize)
{
    // Sanity check:
    if (pImageBufferView != NULL)
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
// Name:        vspImagesAndBuffersManager::updateviewOnEvent
// Description: Update the opened images / buffers / thumbnail views
// Arguments:   apEvent::EventType eventType
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        29/12/2010
// ---------------------------------------------------------------------------
void vspImagesAndBuffersManager::updateOpenedViewsOnEvent(bool isSuspended)
{
    if (_pMonitoredObjectsExplorer != NULL)
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


