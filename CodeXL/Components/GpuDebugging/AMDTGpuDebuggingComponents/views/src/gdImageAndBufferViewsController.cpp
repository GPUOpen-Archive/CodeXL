//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdImageAndBufferViewsController.cpp
///
//==================================================================================

//------------------------------ gdImageAndBufferViewsController.cpp ------------------------------

// Qt:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdImagesAndBuffersManager.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImageAndBufferViewsController.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImageAndBufferView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdThumbnailView.h>
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdSaveAllTexturesBuffersDialog.h>



// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::gdImageAndBufferViewsController
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        15/11/2010
// ---------------------------------------------------------------------------
gdImageAndBufferViewsController::gdImageAndBufferViewsController() :
    _pFocusedThumbnailView(NULL), _pFocusedImageBufferView(NULL),
    _activeContext(AP_OPENGL_CONTEXT , 0), _isInGLBeginEndBlock(false),
    _imageActionsEnabled(AC_IMAGE_INVERT_FILTER | AC_IMAGE_GRAYSCALE_FILTER), _isInfoUpdated(false), _isThumbnailViewCurrentlyActive(false),
    _isItemLoaded(false), _isDebuggedProcessSuspended(false)
{
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::~gdImageAndBufferViewsController
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        15/11/2010
// ---------------------------------------------------------------------------
gdImageAndBufferViewsController::~gdImageAndBufferViewsController()
{
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::onStandardPointerTool
// Description: Standard Pointer Tool was selected
// Author:      Eran Zinman
// Date:        21/5/2007
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::onStandardPointer()
{
    // We should not get here when we're in thumbnail view:
    GT_IF_WITH_ASSERT((!_isThumbnailViewCurrentlyActive) && _isItemLoaded)
    {
        // Go through each of the views and set the active tool:
        for (int i = 0; i < (int)_displayedImageBufferViews.size(); i++)
        {
            // Get the current image / buffer view:
            gdImageAndBufferView* pCurrentImageBufferView = _displayedImageBufferViews[i];

            if (pCurrentImageBufferView != NULL)
            {
                // We only allow toggling this tool, not un-toggling it!
                // (un-toggling is done by selecting another tool from the same tool family)
                acImageManagerToolType currentTool = pCurrentImageBufferView->getActiveTool();

                if (currentTool != AC_IMAGE_MANAGER_TOOL_STANDARD)
                {
                    // Set the standard Tool
                    pCurrentImageBufferView->setActiveTool(AC_IMAGE_MANAGER_TOOL_STANDARD);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::onZoomIn
// Description: Zoom in Tool was selected
// Author:      Eran Zinman
// Date:        21/5/2007
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::onZoomIn()
{
    // We should not get here when we are not in thumbnail view:
    if ((!_isThumbnailViewCurrentlyActive) && _isItemLoaded)
    {
        // Get the active image / buffer view:
        GT_IF_WITH_ASSERT(_pFocusedImageBufferView != NULL)
        {
            // Apply the zoom-in action for the focused image / buffer view:
            _pFocusedImageBufferView->onZoomIn();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::shouldCommandBeEnabled
// Description: Checks if a command id should be enabled / disabled
// Arguments:   bool& isChecked
//              int commandId
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/11/2010
// ---------------------------------------------------------------------------
bool gdImageAndBufferViewsController::shouldCommandBeEnabled(bool& isChecked, int commandId) const
{
    bool retVal = false;
    isChecked = false;

    // If the item was not loaded yet, do not enable any of the commands:
    if (_isItemLoaded && _isDebuggedProcessSuspended)
    {
        switch (commandId)
        {
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_NORMAL:
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_PAN:
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BEST_FIT:
            case ID_IMAGES_AND_BUFFERS_VIEWER_ORIGINAL_SIZE:
            case ID_IMAGES_AND_BUFFERS_VIEWER_ZOOM_COMBOBOX:
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_RIGHT:
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_LEFT:
                retVal = shouldEnableImageViewingCommand(isChecked, commandId);
                break;

            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMIN:
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMOUT:
                retVal = shouldEnableZoom(commandId);
                break;

            case ID_IMAGES_AND_BUFFERS_VIEWER_COPY:
            case ID_IMAGES_AND_BUFFERS_VIEWER_SELECT_ALL:
                retVal = shouldEnableEdit();
                break;

            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_RED_CHANNEL:
                retVal = shouldEnableChannelTool(isChecked, AC_IMAGE_CHANNEL_RED);
                break;

            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GREEN_CHANNEL:
                retVal = shouldEnableChannelTool(isChecked, AC_IMAGE_CHANNEL_GREEN);
                break;

            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BLUE_CHANNEL:
                retVal = shouldEnableChannelTool(isChecked, AC_IMAGE_CHANNEL_BLUE);
                break;

            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ALPHA_CHANNEL:
                retVal = shouldEnableChannelTool(isChecked, AC_IMAGE_CHANNEL_ALPHA, false);
                break;

            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GRAYSCALE:
                retVal = shouldEnableChannelTool(isChecked, AC_IMAGE_GRAYSCALE_FILTER, false);
                break;

            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_INVERT:
                retVal = shouldEnableChannelTool(isChecked, AC_IMAGE_INVERT_FILTER, false);
                break;

            case ID_IMAGES_AND_BUFFERS_VIEWER_SELECT_IMAGE_VIEW:
                retVal = shouldEnableImageViewCommand(isChecked);
                break;

            case ID_IMAGES_AND_BUFFERS_VIEWER_SELECT_DATA_VIEW:
                retVal = shouldEnableDataViewCommand(isChecked);
                break;

            default:
                GT_ASSERT_EX(false, L"Unsupported command id");
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::retVal
// Description: Should we Enable / Disable Zoom in or out Tool
// Author:      Eran Zinman
// Date:        13/6/2007
// ---------------------------------------------------------------------------
bool gdImageAndBufferViewsController::shouldEnableZoom(int commandId) const
{
    bool retVal = false;

    // If we are not in thumbnail view
    if ((!_isThumbnailViewCurrentlyActive) && _isItemLoaded)
    {
        // Get the active image / buffer view:
        GT_IF_WITH_ASSERT(_pFocusedImageBufferView != NULL)
        {
            // Only when image view if focused, zoom actions are enabled:
            retVal = _pFocusedImageBufferView->shouldEnableZoom(commandId);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::shouldEnableChannelTool
// Description: Return true iff the tool for the requested color should be enabled,
//              and also checks if the tool should be checked
// Arguments:   bool& isChannelChecked
//              int colorFilter
//              , bool checkByPassFilters - should we check by pass filters
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/11/2010
// ---------------------------------------------------------------------------
bool gdImageAndBufferViewsController::shouldEnableChannelTool(bool& isChannelChecked, int colorFilter, bool checkByPassFilters) const
{
    bool retVal = false;
    isChannelChecked = false;

    // Are there any active filters that bypass RGBA channels? (Like grayscale, invert...):
    bool isAnyBypassFilters = false;
    bool invertModeChecked = ((_lastViewedItemProperties._actionsMask & AC_IMAGE_INVERT_FILTER) != 0);
    bool grayscaleModeChecked = ((_lastViewedItemProperties._actionsMask & AC_IMAGE_GRAYSCALE_FILTER) != 0);

    if (checkByPassFilters)
    {
        isAnyBypassFilters = invertModeChecked || grayscaleModeChecked;
    }


    // Enable / Disable control:
    bool isChannelEnabled = ((_imageActionsEnabled & colorFilter) != 0);
    retVal = (!isAnyBypassFilters) && isChannelEnabled;


    // If control is enabled
    if (retVal)
    {
        // Should the control be toggled or un-toggled?
        isChannelChecked = ((_lastViewedItemProperties._actionsMask & colorFilter) != 0);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
bool gdImageAndBufferViewsController::shouldEnableImageViewingCommand() const
{
    // Check if the active view is an image view:
    bool retVal = false;

    if ((!_isThumbnailViewCurrentlyActive) && _isItemLoaded)
    {
        GT_IF_WITH_ASSERT(_pFocusedImageBufferView != NULL)
        {
            retVal = _pFocusedImageBufferView->isImageViewFocused();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::shouldEnableImageViewingCommand
// Description: Enable / Disable command to are related to image viewing.
//              For example: Zoom, Pan, etc...
// Author:      Eran Zinman
// Date:        17/7/2007
// ---------------------------------------------------------------------------
bool gdImageAndBufferViewsController::shouldEnableImageViewingCommand(bool& isChecked, int eventId) const
{
    // Check if the active view is an image view:
    isChecked = false;
    bool enableCommand = shouldEnableImageViewingCommand();

    if (enableCommand)
    {
        switch (eventId)
        {
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_NORMAL:
            {
                GT_IF_WITH_ASSERT(_pFocusedImageBufferView != NULL)
                {
                    acImageManagerToolType currentTool = _pFocusedImageBufferView->getActiveTool();
                    isChecked = (currentTool == AC_IMAGE_MANAGER_TOOL_STANDARD);
                }
            }
            break;

            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_PAN:
            {
                GT_IF_WITH_ASSERT(_pFocusedImageBufferView != NULL)
                {
                    acImageManagerToolType currentTool = _pFocusedImageBufferView->getActiveTool();
                    isChecked = (currentTool == AC_IMAGE_MANAGER_TOOL_PAN);
                }
            }
            break;

            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BEST_FIT:
            case ID_IMAGES_AND_BUFFERS_VIEWER_ZOOM_COMBOBOX:
            case ID_IMAGES_AND_BUFFERS_VIEWER_ORIGINAL_SIZE:
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_RIGHT:
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_LEFT:
            {
                // These items are uncheckable;
            }
            break;

            default:
            {
                // Something's wrong
                GT_ASSERT(false);
            }
            break;
        }
    }

    return enableCommand;
}


// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::shouldEnableImageViewCommand
// Description: Return true iff the current state enabled the view image command
// Arguments:   bool& isChecked
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/11/2010
// ---------------------------------------------------------------------------
bool gdImageAndBufferViewsController::shouldEnableImageViewCommand(bool& isChecked) const
{
    bool retVal = false;

    // Check if we are in image viewing mode:
    retVal = ((!_isThumbnailViewCurrentlyActive) && _isItemLoaded);

    // Sanity check:
    if ((!_isThumbnailViewCurrentlyActive) && (_pFocusedImageBufferView != NULL))
    {
        // Check if the image view is selected:
        isChecked = _pFocusedImageBufferView->isImageViewFocused();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::shouldEnableDataViewCommand
// Description: Return true iff the current state enabled the view data command
// Arguments:   bool& isChecked
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        15/11/2010
// ---------------------------------------------------------------------------
bool gdImageAndBufferViewsController::shouldEnableDataViewCommand(bool& isChecked) const
{
    bool retVal = false;

    // Check if we are in image viewing mode:
    retVal = ((!_isThumbnailViewCurrentlyActive) && _isItemLoaded);

    // Sanity check:
    if ((!_isThumbnailViewCurrentlyActive) && (_pFocusedImageBufferView != NULL))
    {
        // Check if the image view is selected:
        isChecked = !_pFocusedImageBufferView->isImageViewFocused();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::shouldEnableEdit
// Description: Should we Enable / Disable the select all command
// Author:      Sigal Algranaty
// Date:        9/9/2009
// ---------------------------------------------------------------------------
bool gdImageAndBufferViewsController::shouldEnableEdit() const
{
    // Check if we are in image viewing mode
    bool retVal = ((!_isThumbnailViewCurrentlyActive) && _isItemLoaded);

    // Sanity check:
    if ((!_isThumbnailViewCurrentlyActive) && (_pFocusedImageBufferView != NULL))
    {
        // Only enable edit for data view:
        retVal = !_pFocusedImageBufferView->isImageViewFocused();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::shouldEnableSaveAllCommand
// Description: Enable / Disable the save all textures and buffers command
// Author:      Eran Zinman
// Date:        5/1/2008
// ---------------------------------------------------------------------------
bool gdImageAndBufferViewsController::shouldEnableSaveAllCommand() const
{
    bool retVal = false;

    // The action should be enabled only when there is a focused thumbnail view:
    if (_pFocusedThumbnailView != NULL)
    {
        // Check what is the type of the displayed objects:
        const afApplicationTreeItemData* pDisplayedItemData = _pFocusedThumbnailView->displayedItemData();
        GT_IF_WITH_ASSERT(pDisplayedItemData != NULL)
        {
            // Check what is the displayed objects type:
            afTreeItemType itemType = pDisplayedItemData->m_itemType;
            retVal = ((itemType == AF_TREE_ITEM_GL_TEXTURES_NODE) || (itemType == AF_TREE_ITEM_CL_IMAGES_NODE) || (itemType == AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE) ||
                      (itemType == AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE) || (itemType == AF_TREE_ITEM_GL_FBO_NODE));
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::shouldEnableSaveRawDataCommand
// Description: Enable / Disable the save raw data command
// Author:      Sigal Algranaty
// Date:        17/5/2010
// ---------------------------------------------------------------------------
bool gdImageAndBufferViewsController::shouldEnableSaveRawDataCommand() const
{
    bool retVal = false;

    // The action should be enabled only when there is a focused image buffer view:
    if (_pFocusedImageBufferView != NULL)
    {
        // Check what is the type of the displayed objects:
        const afApplicationTreeItemData* pDisplayedItemData = _pFocusedImageBufferView->displayedItemData();
        GT_IF_WITH_ASSERT(pDisplayedItemData != NULL)
        {
            // Check what is the displayed objects type:
            afTreeItemType itemType = pDisplayedItemData->m_itemType;
            retVal = afApplicationTreeItemData::isItemImageOrBuffer(itemType);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::onZoomOut
// Description: Zoom out Tool was selected
// Author:      Eran Zinman
// Date:        21/5/2007
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::onZoomOut()
{
    // We should not get here when we are not in thumbnail view:
    GT_IF_WITH_ASSERT((!_isThumbnailViewCurrentlyActive) && _isItemLoaded && (_pFocusedImageBufferView != NULL))
    {
        // Apply the zoom out action for the focused image / buffer view:
        _pFocusedImageBufferView->onZoomOut();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::onPanTool
// Description: Pan Tool was selected
// Author:      Eran Zinman
// Date:        21/5/2007
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::onPan()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pFocusedImageBufferView != NULL)
    {
        // Go through each of the views and set the active tool:
        for (int i = 0; i < (int)_displayedImageBufferViews.size(); i++)
        {
            // Get the current image / buffer view:
            gdImageAndBufferView* pCurrentImageBufferView = _displayedImageBufferViews[i];

            if (pCurrentImageBufferView != NULL)
            {
                // We only allow toggling this tool, not un-toggling it!
                // (un-toggling is done by selecting another tool from the same tool family)
                acImageManagerToolType currentTool = pCurrentImageBufferView->getActiveTool();

                if (currentTool != AC_IMAGE_MANAGER_TOOL_PAN)
                {
                    // Clear the current pixel information
                    pCurrentImageBufferView->clearCurrentPixelInformation();

                    // Set the Pan tool
                    pCurrentImageBufferView->setActiveTool(AC_IMAGE_MANAGER_TOOL_PAN);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::onColorChannel
// Description: Enable / Disable the images color channel
//              acImageItemAction colorAction - the color specified
// Author:      Eran Zinman
// Date:        28/5/2007
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::onColorChannel(acImageItemAction colorAction)
{
    // Check if the color button is checked:
    bool isColorChecked = ((_lastViewedItemProperties._actionsMask & colorAction) != 0);

    // Toggle the red check value:
    if (isColorChecked)
    {
        _lastViewedItemProperties._actionsMask &= (~colorAction);
    }
    else
    {
        _lastViewedItemProperties._actionsMask |= colorAction;
    }

    // Go through each of the views and set the active tool:
    for (int i = 0; i < (int)_displayedThumbnailViews.size(); i++)
    {
        // Get the current image / buffer view:
        gdThumbnailView* pCurrentThumbnailView = _displayedThumbnailViews[i];

        if (pCurrentThumbnailView != NULL)
        {
            // Enable / Disable the channel:
            pCurrentThumbnailView->setFilterForAllImages(_lastViewedItemProperties._actionsMask, 0.0);
        }
    }

    // Go through each of the views and set the active tool:
    for (int i = 0; i < (int)_displayedImageBufferViews.size(); i++)
    {
        // Get the current image / buffer view:
        gdImageAndBufferView* pCurrentImageBufferView = _displayedImageBufferViews[i];

        if (pCurrentImageBufferView != NULL)
        {
            // Enable / Disable the channel:
            pCurrentImageBufferView->setFilterForAllImages(_lastViewedItemProperties._actionsMask, 0.0);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::onBestFitTool
// Description: Set the zoom level to be the "Best fit" zoom level
// Author:      Eran Zinman
// Date:        14/7/2007
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::onBestFit()
{
    // Set zoom level to "Best Fit" level:
    GT_IF_WITH_ASSERT(!_isThumbnailViewCurrentlyActive && _isItemLoaded && (_pFocusedImageBufferView != NULL))
    {
        // Set the image view zoom level:
        int zoomLevel = 0;
        _pFocusedImageBufferView->applyBestFit(zoomLevel);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::onRotateLeft
// Description: Rotate the images 90 degrees to the left
// Author:      Eran Zinman
// Date:        17/7/2007
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::onRotateLeft()
{
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pMainWindow != NULL)
    {
        pMainWindow->setCursor(QCursor(Qt::WaitCursor));
    }

    GT_IF_WITH_ASSERT(!_isThumbnailViewCurrentlyActive && _isItemLoaded && (_pFocusedImageBufferView != NULL))
    {
        // Enable / Disable the channel:
        _pFocusedImageBufferView->setFilterForAllImages(AC_IMAGE_ROTATE, 270);
    }

    if (NULL != pMainWindow)
    {
        pMainWindow->setCursor(QCursor(Qt::ArrowCursor));
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::onRotateRight
// Description: Rotate the images 90 degrees to the right
// Author:      Eran Zinman
// Date:        17/7/2007
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::onRotateRight()
{
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pMainWindow != NULL)
    {
        pMainWindow->setCursor(QCursor(Qt::WaitCursor));
    }

    GT_IF_WITH_ASSERT(!_isThumbnailViewCurrentlyActive && _isItemLoaded && (_pFocusedImageBufferView != NULL))
    {
        // Enable / Disable the channel:
        _pFocusedImageBufferView->setFilterForAllImages(AC_IMAGE_ROTATE, 90);
    }

    if (NULL != pMainWindow)
    {
        pMainWindow->setCursor(QCursor(Qt::ArrowCursor));
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::onInvertTool
// Description: Enable / Disable the invert mode
// Author:      Eran Zinman
// Date:        14/7/2007
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::onInvert()
{
    // Get the button status:
    bool isInvertChecked = ((_lastViewedItemProperties._actionsMask & AC_IMAGE_INVERT_FILTER) != 0);

    // Toggle the invert check value (and Untoggle grayscale mode):
    if (isInvertChecked)
    {
        // Remove the invert flag:
        _lastViewedItemProperties._actionsMask &= (~AC_IMAGE_INVERT_FILTER);
    }
    else
    {
        _lastViewedItemProperties._actionsMask |= AC_IMAGE_INVERT_FILTER;
        _lastViewedItemProperties._actionsMask &= (~AC_IMAGE_GRAYSCALE_FILTER);

    }

    // Go through each of the views and set the active tool:
    for (int i = 0; i < (int)_displayedThumbnailViews.size(); i++)
    {
        // Get the current image / buffer view:
        gdThumbnailView* pCurrentThumbnailView = _displayedThumbnailViews[i];

        if (pCurrentThumbnailView != NULL)
        {
            // Enable / Disable the channel:
            pCurrentThumbnailView->setFilterForAllImages(_lastViewedItemProperties._actionsMask, 0.0);
        }
    }

    // Go through each of the views and set the active tool:
    for (int i = 0; i < (int)_displayedImageBufferViews.size(); i++)
    {
        // Get the current image / buffer view:
        gdImageAndBufferView* pCurrentImageBufferView = _displayedImageBufferViews[i];

        if (pCurrentImageBufferView != NULL)
        {
            // Enable / Disable the channel:
            pCurrentImageBufferView->setFilterForAllImages(_lastViewedItemProperties._actionsMask, 0.0);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::onGrayscaleTool
// Description: Enable / Disable the grayscale mode
// Author:      Eran Zinman
// Date:        28/5/2007
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::onGrayscale()
{
    // Get the button status:
    bool isGrayscaleChecked = ((_lastViewedItemProperties._actionsMask & AC_IMAGE_GRAYSCALE_FILTER) != 0);

    // Toggle the invert check value (and Untoggle invert mode):
    if (isGrayscaleChecked)
    {
        // Remove the grayscale filter:
        _lastViewedItemProperties._actionsMask &= (~AC_IMAGE_GRAYSCALE_FILTER);
    }
    else
    {
        // Remove invert mode from checked values:
        _lastViewedItemProperties._actionsMask &= (~AC_IMAGE_INVERT_FILTER);

        // Add invert to filters:
        _lastViewedItemProperties._actionsMask |= AC_IMAGE_GRAYSCALE_FILTER;
    }

    // Go through each of the views and set the active tool:
    for (int i = 0; i < (int)_displayedThumbnailViews.size(); i++)
    {
        // Get the current image / buffer view:
        gdThumbnailView* pCurrentThumbnailView = _displayedThumbnailViews[i];

        if (pCurrentThumbnailView != NULL)
        {
            // Enable / Disable the channel:
            pCurrentThumbnailView->setFilterForAllImages(_lastViewedItemProperties._actionsMask);
        }
    }

    // Go through each of the views and set the active tool:
    for (int i = 0; i < (int)_displayedImageBufferViews.size(); i++)
    {
        // Get the current image / buffer view:
        gdImageAndBufferView* pCurrentImageBufferView = _displayedImageBufferViews[i];

        if (pCurrentImageBufferView != NULL)
        {
            // Enable / Disable the channel:
            pCurrentImageBufferView->setFilterForAllImages(_lastViewedItemProperties._actionsMask);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::resetImageAndDataViews
// Description: Resets the image and data views
// Author:      Eran Zinman
// Date:        30/11/2007
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::resetImageAndDataViews(bool resetFilters)
{
    (void)(resetFilters);  // unused

    // Clear each of the displayed views:
    for (int i = 0; i < (int)_displayedImageBufferViews.size(); i++)
    {
        // Get the current image / buffer view:
        gdImageAndBufferView* pCurrentImageBufferView = _displayedImageBufferViews[i];

        if (pCurrentImageBufferView != NULL)
        {
            // Clear all the images in the Image Manager
            pCurrentImageBufferView->clearView();
        }
    }

    for (int i = 0; i < (int)_displayedThumbnailViews.size(); i++)
    {
        // Get the current thumbnail view:
        gdThumbnailView* pCurrentThumbnailView = _displayedThumbnailViews[i];

        if (pCurrentThumbnailView != NULL)
        {
            // Clear all the images in the Image Manager
            pCurrentThumbnailView->clearView();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::onOriginalSize
// Description: Set's the items zoom level to be 100%
// Arguments:   Event details
// Author:      Eran Zinman
// Date:        17/7/2007
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::onOriginalSize()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pFocusedImageBufferView != NULL)
    {
        // Set the texture zoom level to 100%:
        _pFocusedImageBufferView->setTextureManagerZoomLevel(100);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::onMenuImageViewSelected
// Description: Occurs when the user selected the image view from the
//              "view" menu
// Arguments:   Event details
// Author:      Eran Zinman
// Date:        6/1/2008
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::onMenuImageViewSelected()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((_pFocusedImageBufferView != NULL) && (!_isThumbnailViewCurrentlyActive))
    {
        // Ask the image manager to display the image view:
        _pFocusedImageBufferView->selectImageView(true);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::onMenuDataViewSelected
// Description: Occurs when the user selected the data view from the
//              "view" menu
// Arguments:   Event details
// Author:      Eran Zinman
// Date:        6/1/2008
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::onMenuDataViewSelected()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((_pFocusedImageBufferView != NULL) && (!_isThumbnailViewCurrentlyActive))
    {
        // Ask the image manager to display the image view:
        _pFocusedImageBufferView->selectImageView(false);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::onSaveAllTextureAndBuffers
// Description: Save all textures and buffers as images to the disk
// Arguments:   Event details
// Author:      Eran Zinman
// Date:        5/1/2008
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::onSaveAllAsImage()
{
    // We should not get here while in begin-end block:
    GT_IF_WITH_ASSERT(!_isInGLBeginEndBlock)
    {
        // Create a vector with the supported file types.
        gtVector<apFileType> fileTypesVector;

        fileTypesVector.push_back(AP_BMP_FILE);
        fileTypesVector.push_back(AP_TIFF_FILE);
        fileTypesVector.push_back(AP_JPEG_FILE);
        fileTypesVector.push_back(AP_PNG_FILE);

        GT_IF_WITH_ASSERT(_pFocusedThumbnailView != NULL)
        {
            // Show the save all textures and buffers dialog:
            showSaveAllItemsDialog(fileTypesVector);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::onSaveAllAsRawData
// Description: Save all textures and buffers as raw data to the disk
// Arguments:   Event details
// Author:      Eran Zinman
// Date:        13/1/2008
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::onSaveAllAsRawData()
{
    // We should not get here while in begin-end block:
    GT_IF_WITH_ASSERT(!_isInGLBeginEndBlock)
    {
        // Create a vector with the supported file types.
        gtVector<apFileType> fileTypesVector;
        fileTypesVector.push_back(AP_CSV_FILE);

        GT_IF_WITH_ASSERT(_pFocusedThumbnailView != NULL)
        {
            // Show the save all textures and buffers dialog:
            showSaveAllItemsDialog(fileTypesVector);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::showSaveAllItemsDialog
// Description: Shows the save all textures and buffers to disk dialog
// Arguments:   fileTypesVector - A Vector containing all supported
//              file types format which the user can export the textures
//              and buffers with.
// Author:      Eran Zinman
// Date:        13/1/2008
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::showSaveAllItemsDialog(const gtVector<apFileType>& fileTypesVector)
{
    // Make sure we got at least one file format to save the textures and buffers with
    bool rc1 = (fileTypesVector.size() > 0);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Show the save all textures and buffers dialog
        QString saveDialogTitle = GD_STR_SaveAllTexturesAndBuffersDialogGLTitle;
        apContextType currentContextType = gdGDebuggerGlobalVariablesManager::instance().chosenContextType();

        if (currentContextType == AP_OPENGL_CONTEXT)
        {
            saveDialogTitle = GD_STR_SaveAllTexturesAndBuffersDialogGLTitle;
        }
        else if (currentContextType == AP_OPENCL_CONTEXT)
        {
            saveDialogTitle = GD_STR_SaveAllTexturesAndBuffersDialogCLTitle;
        }

        GT_IF_WITH_ASSERT(_pFocusedThumbnailView != NULL)
        {
            gdSaveAllTexturesBuffersDialog saveDialog(_pFocusedThumbnailView, saveDialogTitle, fileTypesVector);
            saveDialog.setModal(true);

            int retVal = saveDialog.exec();


            // If the user clicked on "ok", get dialog parameters and export all textures and buffers
            if (retVal == QDialog::Accepted)
            {
                // Should we overwrite files?
                bool shouldOverwriteFiles = saveDialog.shouldOverwriteFiles();

                // Get export output format
                apFileType fileType;
                rc1 = saveDialog.getFileType(fileType);
                GT_IF_WITH_ASSERT(rc1)
                {
                    // Get output directory
                    gtString outputDir;
                    bool rc2 = saveDialog.getOutputDirectory(outputDir);
                    GT_IF_WITH_ASSERT(rc2)
                    {
                        // Export all textures and buffers
                        exportAllTexturesAndBuffers(outputDir, fileType, shouldOverwriteFiles);
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::onEditCopy
// Description: Copies selected cell content in data view to the clipboard
// Arguments:   Event details
// Author:      Sigal Algranaty
// Date:        9/9/2009
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::onEditCopy()
{
    GT_IF_WITH_ASSERT(_pFocusedImageBufferView != NULL)
    {
        _pFocusedImageBufferView->onEdit_Copy();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::onEditSelectAll
// Description: Select all cells in data view
// Arguments:   Event details
// Author:      Sigal Algranaty
// Date:        9/9/2009
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::onEditSelectAll()
{
    if (_pFocusedImageBufferView != NULL)
    {
        _pFocusedImageBufferView->onEdit_SelectAll();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::initProgress
// Description: Creates the double progress dialog
// Author:      Eran Zinman
// Date:        8/1/2008
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::initProgress(int amountOfItems)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pFocusedThumbnailView != NULL)
    {
        // Adjust the progress bar dialog's sub title to the context type:
        gtString dialogSubtitle = GD_STR_ImagesAndBuffersViewerExportingGLTexturesAndBuffersMessage;

        if (_activeContext.isOpenGLContext())
        {
            dialogSubtitle = GD_STR_ImagesAndBuffersViewerExportingGLTexturesAndBuffersMessage;
        }
        else if (_activeContext.isOpenCLContext())
        {
            dialogSubtitle = GD_STR_ImagesAndBuffersViewerExportingCLBuffersAndImagesMessage;
        }

        // Set the progress text:
        afProgressBarWrapper::instance().setProgressDetails(GD_STR_ImagesAndBuffersViewerPleaseWaitMessage, amountOfItems);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::exportAllTexturesAndBuffers
// Description: Export all textures and buffers to disk
// Arguments:   outputDir - Output directory
//              fileType - The format to save the files as.
//              overwriteFiles - Should we overwrite files when necessary?
// Author:      Eran Zinman
// Date:        6/1/2008
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::exportAllTexturesAndBuffers(const gtString& outputDir, apFileType fileType, bool overwriteFiles)
{
    // Convert the output directory string to osFilePath
    osFilePath filePath;
    filePath.setFileDirectory(outputDir);

    // Create a vector containing all exportable items from the textures and buffers list
    gtVector<afApplicationTreeItemData*> exportListVector;

    bool rc1 = generateAllItemsExportList(exportListVector);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Get total amount of items that needs to be exported
        long amountOfItems = exportListVector.size();

        // Initialize progress (each item progress is updated 3 times):
        initProgress(amountOfItems * 3);

        // Loop through the items and export all of them
        for (long i = 0; i < amountOfItems; i++)
        {
            // Sanity check:
            if (exportListVector[i] != NULL)
            {
                // Generate a name for the export item (for example: Texture #6, Static Buffer, ... )
                gtString exportItemName, exportFileItemName;
                bool rc2 = generateExportItemName(exportListVector[i], exportItemName, exportFileItemName);
                GT_IF_WITH_ASSERT(rc2)
                {
                    // Generate text which indicates which item is currently being exported
                    gtString updateText;
                    updateText.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportingItemMessage, exportItemName.asCharArray(), i, amountOfItems);
                    afProgressBarWrapper::instance().setProgressText(updateText);

                    // New log entry after item export
                    gtString doubleSliderLog;

                    // A Vector containing export result
                    gtPtrVector<gdFileExporterOutputResult*> exportResult;

                    // Export viewer item to disk:
                    bool rc3 = gdImageAndBufferView::exportViewerItemToDisk(exportListVector[i], filePath, fileType, exportResult, overwriteFiles, exportFileItemName);
                    GT_ASSERT(rc3);

                    // Increment progress:
                    afProgressBarWrapper::instance().incrementProgressBar();
                }
            }
        }
    }

    // Hide progress bar:
    afProgressBarWrapper::instance().hideProgressBar();
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::generateExportItemName
// Description: Generates an export name for the viewer item
// Arguments:   pViewerItem - The item to generate a message for
//              exportName - Output item export name
//              pMonitoredObjectsTree - the objects tree
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        6/1/2008
// ---------------------------------------------------------------------------
bool gdImageAndBufferViewsController::generateExportItemName(afApplicationTreeItemData* pViewerItem, gtString& exportName, gtString& exportFileName)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pViewerItem != NULL)
    {
        gdDebugApplicationTreeData* pGDViewerItem = qobject_cast<gdDebugApplicationTreeData*>(pViewerItem->extendedItemData());
        GT_IF_WITH_ASSERT(pGDViewerItem != NULL)
        {
            retVal = true;

            // Empty output string
            exportName.makeEmpty();

            // Get item type
            afTreeItemType itemType = pViewerItem->m_itemType;

            switch (itemType)
            {
                case AF_TREE_ITEM_GL_TEXTURE:
                {
                    // Get the texture name:
                    GLuint textureName = pGDViewerItem->_textureMiplevelID._textureName;

                    // Match the string to the context Type:
                    if (_activeContext.isOpenGLContext())
                    {
                        exportName.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportItemNameTexture, textureName);
                    }
                    else if (_activeContext.isOpenCLContext())
                    {
                        exportName.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportItemNameImage, textureName);
                    }
                }
                break;

                case AF_TREE_ITEM_GL_STATIC_BUFFER:
                {
                    // Get buffer type:
                    apDisplayBuffer bufferType = pGDViewerItem->_bufferType;

                    gtString bufferName;
                    bool rc1 = apGetBufferName(bufferType, bufferName);
                    GT_IF_WITH_ASSERT(rc1)
                    {
                        // Set the update message
                        exportName.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportItemNameStaticBuffer, bufferName.asCharArray());
                    }
                }
                break;

                case AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER:
                {
                    // Get PBuffer ID
                    int pbufferID = pGDViewerItem->_objectOpenGLName;

                    // Get buffer type
                    apDisplayBuffer bufferType = pGDViewerItem->_bufferType;

                    gtString bufferName;
                    bool rc1 = apGetBufferName(bufferType, bufferName);
                    GT_IF_WITH_ASSERT(rc1)
                    {
                        // Set the update message
                        exportName.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportItemNamePBuffer, pbufferID, bufferName.asCharArray());
                    }
                }
                break;

                case AF_TREE_ITEM_GL_VBO:
                {
                    // Get VBO name:
                    GLuint vboName = pGDViewerItem->_objectOpenGLName;

                    // Set the update message:
                    exportName.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportItemNameVBO, vboName);
                }
                break;

                case AF_TREE_ITEM_CL_IMAGE:
                {
                    // Set the update message:
                    exportName.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportItemNameImage, pGDViewerItem->_objectOpenCLName);
                }
                break;

                case AF_TREE_ITEM_CL_BUFFER:
                {
                    // Set the update message:
                    exportName.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportItemNameCLBuffer, pGDViewerItem->_objectOpenCLName);
                }
                break;

                case AF_TREE_ITEM_CL_SUB_BUFFER:
                {
                    // Set the update message:
                    exportName.appendFormattedString(GD_STR_ImagesAndBuffersViewerExportItemNameCLSubBuffer, pGDViewerItem->_objectOpenCLName);
                }
                break;

                default:
                {
                    GT_ASSERT_EX(false, L"Unsupported item type!");
                    retVal = false;
                }
                break;
            }

            gtString contextStr;
            pGDViewerItem->_contextId.toString(contextStr);
            exportFileName.appendFormattedString(L"%ls-%ls", contextStr.asCharArray(), exportName.asCharArray());
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdTexturesAndBuffersViewer::onSaveImageAs
// Description: Save the currently viewed image to the disk
// Arguments:   Event details
// Author:      Eran Zinman
// Date:        5/1/2008
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::onSaveImageAs(afApplicationTreeItemData* pViewerItem)
{
    GT_IF_WITH_ASSERT(!_isInGLBeginEndBlock && (pViewerItem != NULL))
    {
        gdDebugApplicationTreeData* pGDViewerItem = qobject_cast<gdDebugApplicationTreeData*>(pViewerItem->extendedItemData());
        GT_IF_WITH_ASSERT(pGDViewerItem != NULL)
        {
            // Create a vector with the supported file types.
            gtVector<apFileType> fileTypesVector;

            // Check if this is a texture:
            bool isTexture = afApplicationTreeItemData::isItemImage(pViewerItem->m_itemType);

            bool isTiffImageTextureType = ((pGDViewerItem->_textureType == AP_3D_TEXTURE) || (pGDViewerItem->_textureType == AP_1D_ARRAY_TEXTURE) || (pGDViewerItem->_textureType == AP_2D_ARRAY_TEXTURE));
            // If item is a 3D texture, we only allow TIFF file saving as it's the only format that have multi-layers

            if (isTexture && isTiffImageTextureType)
            {
                // For 3D Textures and texture arrays - We only support Tiff Format
                fileTypesVector.push_back(AP_TIFF_FILE);
            }
            else
            {
                // Else - Show all available image format
                fileTypesVector.push_back(AP_PNG_FILE);
                fileTypesVector.push_back(AP_TIFF_FILE);
                fileTypesVector.push_back(AP_JPEG_FILE);
                fileTypesVector.push_back(AP_BMP_FILE);
            }

            GT_IF_WITH_ASSERT(_pFocusedImageBufferView != NULL)
            {
                // Show the save single file dialog:
                _pFocusedImageBufferView->showSaveSingleFileDialog(fileTypesVector, pViewerItem);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::onSaveRawDataAs
// Description: Save the currently viewed item as raw data to the disk
// Arguments:   Event details
// Author:      Eran Zinman
// Date:        13/1/2008
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::onSaveRawDataAs(afApplicationTreeItemData* pViewerItem)
{
    GT_IF_WITH_ASSERT(!_isInGLBeginEndBlock && (pViewerItem != NULL))
    {
        // Create a vector with the supported file types:
        gtVector<apFileType> fileTypesVector;
        fileTypesVector.push_back(AP_CSV_FILE);

        GT_IF_WITH_ASSERT(_pFocusedImageBufferView != NULL)
        {
            // Show the save single file dialog:
            _pFocusedImageBufferView->showSaveSingleFileDialog(fileTypesVector, pViewerItem);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::enableImageActionsByItemType
// Description: Apply an image item type and format on the image actions enabled
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        22/4/2010
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::enableImageActionsByItemType(afTreeItemType itemType)
{
    // Check the item by type:
    switch (itemType)
    {
        case AF_TREE_ITEM_GL_VBO:
        case AF_TREE_ITEM_GL_VBO_NODE:
        case AF_TREE_ITEM_CL_BUFFER:
        case AF_TREE_ITEM_CL_SUB_BUFFER:
        case AF_TREE_ITEM_CL_BUFFERS_NODE:
        case AF_TREE_ITEM_CL_CONTEXT:
        case AF_TREE_ITEM_GL_RENDER_CONTEXT:
        case AF_TREE_ITEM_APP_ROOT:
        {
            // Filters are irrelevant:
            _imageActionsEnabled = 0;
            break;
        }

        case AF_TREE_ITEM_CL_IMAGES_NODE:
        case AF_TREE_ITEM_GL_TEXTURES_NODE:
        case AF_TREE_ITEM_GL_RENDER_BUFFERS_NODE:
        case AF_TREE_ITEM_GL_PBUFFER_NODE:
        case AF_TREE_ITEM_GL_PBUFFERS_NODE:
        case AF_TREE_ITEM_GL_FBO_NODE:
        case AF_TREE_ITEM_GL_FBO:
        {
            // Enable all:
            _imageActionsEnabled = AC_IMAGE_CHANNEL_RED | AC_IMAGE_CHANNEL_BLUE | AC_IMAGE_CHANNEL_GREEN | AC_IMAGE_CHANNEL_ALPHA | AC_IMAGE_INVERT_FILTER | AC_IMAGE_GRAYSCALE_FILTER;
            break;
        }

        case AF_TREE_ITEM_GL_STATIC_BUFFERS_NODE:
        {
            // Alpha is not relevant for static buffers:
            _imageActionsEnabled = AC_IMAGE_CHANNEL_RED | AC_IMAGE_CHANNEL_BLUE | AC_IMAGE_CHANNEL_GREEN | AC_IMAGE_INVERT_FILTER | AC_IMAGE_GRAYSCALE_FILTER;
            break;
        }

        case AF_TREE_ITEM_CL_IMAGE:
        case AF_TREE_ITEM_GL_TEXTURE:
        case AF_TREE_ITEM_GL_RENDER_BUFFER:
        case AF_TREE_ITEM_GL_FBO_ATTACHMENT:
        case AF_TREE_ITEM_GL_STATIC_BUFFER:
        case AF_TREE_ITEM_GL_PBUFFER_STATIC_BUFFER:
        {
            GT_IF_WITH_ASSERT(_pFocusedImageBufferView != NULL)
            {
                // Get the item's raw file handler:
                acRawFileHandler* pRawDataHandler = _pFocusedImageBufferView->getCurrentRawDataHandler();

                if (pRawDataHandler != NULL)
                {
                    // Check which image actions are enabled according to the image format:
                    bool rc = checkWhichImageActionsAreEnabled(pRawDataHandler);
                    GT_ASSERT(rc);
                }
            }

            break;
        }

        default:
        {
            // Unknown type, do nothing:
            GT_ASSERT(false);
            break;
        }
    }

}

// ---------------------------------------------------------------------------
// Name:        checkWhichImageActionsAreEnabled
// Description: Given a raw file handler, check which filters/ actions can be
//              applied for this image, and set the image actions mask accordingly
// Arguments:   const acRawFileHandler* pRawDataHandler
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/4/2010
// ---------------------------------------------------------------------------
bool gdImageAndBufferViewsController::checkWhichImageActionsAreEnabled(const acRawFileHandler* pRawDataHandler)
{
    bool retVal = false;

    // Sanity Check:
    GT_IF_WITH_ASSERT(pRawDataHandler != NULL)
    {
        // Reset the filters enable state:
        _imageActionsEnabled = AC_IMAGE_INVERT_FILTER | AC_IMAGE_GRAYSCALE_FILTER;

        // Get raw data format:
        oaTexelDataFormat dataFormat = pRawDataHandler->dataFormat();

        // Get the item possible actions by its data format:
        retVal = acImageManager::getItemActionsByFormat(dataFormat, _imageActionsEnabled);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::openImageBufferView
// Description:
// Arguments:   gdImageAndBufferView* pImageBufferView
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        21/12/2010
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::openImageBufferView(gdImageAndBufferView* pImageBufferView)
{
    // Set the current focus image / buffer view:
    _pFocusedImageBufferView = pImageBufferView;

    // Add the view to the displayed views:
    _displayedImageBufferViews.push_back(pImageBufferView);
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::openThumbnailView
// Description:
// Arguments:   gdImageAndBufferView* pImageBufferView - the view to close
// Author:      Sigal Algranaty
// Date:        21/12/2010
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::openThumbnailView(gdThumbnailView* pThumbnailView)
{
    // Set the current focus thumbnail view:
    _pFocusedThumbnailView = pThumbnailView;

    // Add the view to the displayed views:
    _displayedThumbnailViews.push_back(pThumbnailView);
}


// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::closeThumbnailView
// Description: Close the input thumbnail view
// Arguments:   gdThumbnailView* pThumbnailView - the view to close
// Author:      Sigal Algranaty
// Date:        1/5/2011
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::closeThumbnailView(gdThumbnailView* pThumbnailView)
{
    // Find the image view within the existing views:
    int viewIndex = -1;

    for (int i = 0; i < (int)_displayedThumbnailViews.size(); i++)
    {
        if (_displayedThumbnailViews[i] == pThumbnailView)
        {
            viewIndex = i;
            break;
        }
    }

    GT_IF_WITH_ASSERT((viewIndex >= 0) && (viewIndex < (int)_displayedThumbnailViews.size()))
    {
        // Remove the view from the list of views:
        _displayedThumbnailViews.removeItem(viewIndex);
    }

}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::closeImageBufferView
// Description: Close the input image / buffer view
// Arguments:   gdImageAndBufferView* pImageBufferView
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        21/12/2010
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::closeImageBufferView(gdImageAndBufferView* pImageBufferView)
{
    // Find the image view within the existing views:
    int viewIndex = -1;

    for (int i = 0; i < (int)_displayedImageBufferViews.size(); i++)
    {
        if (_displayedImageBufferViews[i] == pImageBufferView)
        {
            viewIndex = i;
            break;
        }
    }

    GT_IF_WITH_ASSERT((viewIndex >= 0) && (viewIndex < (int)_displayedImageBufferViews.size()))
    {
        // Remove the view from the list of views:
        _displayedImageBufferViews.removeItem(viewIndex);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::generateAllItemsExportList
// Description: Generates a vector with all viewer items that will be
//              exported.
// Arguments:   exportListVector - Output viewer items vector.
// Return Val:  bool - Success / Failure
// Author:      Eran Zinman
// Date:        13/1/2008
// ---------------------------------------------------------------------------
bool gdImageAndBufferViewsController::generateAllItemsExportList(gtVector<afApplicationTreeItemData*>& exportListVector)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(gdDebugApplicationTreeHandler::instance() != NULL)
    {
        // Get the current selected context:
        afApplicationTreeItemData* pCurrentSelectedItemData = getCurrentlySelectedViewerItem();

        if (pCurrentSelectedItemData != NULL)
        {
            gdDebugApplicationTreeData* pGDCurrentSelectedItemData = qobject_cast<gdDebugApplicationTreeData*>(pCurrentSelectedItemData->extendedItemData());
            GT_IF_WITH_ASSERT(pGDCurrentSelectedItemData != NULL)
            {
                afTreeItemType imageObjectType = pGDCurrentSelectedItemData->_contextId.isOpenCLContext() ? AF_TREE_ITEM_CL_IMAGE : AF_TREE_ITEM_GL_TEXTURE;

                // Get amount of displayed textures:
                int amountOfDisplayedTextures = gdDebugApplicationTreeHandler::instance()->amountOfDisplayedObjectForType(pGDCurrentSelectedItemData->_contextId, imageObjectType);

                // Loop through the textures:
                for (int i = 0; i < amountOfDisplayedTextures; i++)
                {
                    // Get the item data from the tree item id:
                    afApplicationTreeItemData* pViewerItem = gdDebugApplicationTreeHandler::instance()->getItemDataByType(pGDCurrentSelectedItemData->_contextId, imageObjectType, i);

                    // Is item a texture / static buffer / PBuffer / etc... ?
                    if (afApplicationTreeItemData::isItemImageOrBuffer(pViewerItem->m_itemType))
                    {
                        exportListVector.push_back(pViewerItem);
                    }
                }

                // Get amount of displayed static buffers:
                int amountOfDisplayedStaticBuffers = gdDebugApplicationTreeHandler::instance()->amountOfDisplayedObjectForType(pGDCurrentSelectedItemData->_contextId, AF_TREE_ITEM_GL_STATIC_BUFFER);

                // Loop through the static buffers:
                for (int i = 0; i < amountOfDisplayedStaticBuffers; i++)
                {
                    // Get the item data from the tree item id:
                    afApplicationTreeItemData* pViewerItem = gdDebugApplicationTreeHandler::instance()->getItemDataByType(pGDCurrentSelectedItemData->_contextId, AF_TREE_ITEM_GL_STATIC_BUFFER, i);

                    // Is item a texture / static buffer / PBuffer / etc... ?
                    if (afApplicationTreeItemData::isItemImageOrBuffer(pViewerItem->m_itemType))
                    {
                        exportListVector.push_back(pViewerItem);
                    }
                }


                // Yuri: June 11, 2014 Commented out - VBOs cannot be saved as images ( only as CSV )
                //// Get amount of displayed VBOs:
                //int amountOfDisplayedVBOs = gdDebugApplicationTreeHandler::instance()->amountOfDisplayedObjectForType(pGDCurrentSelectedItemData->_contextId, AF_TREE_ITEM_GL_VBO);

                //// Loop through the VBOs:
                //for (int i = 0; i < amountOfDisplayedVBOs; i++)
                //{
                //    // Get the item data from the tree item id:
                //    afApplicationTreeItemData* pViewerItem = gdDebugApplicationTreeHandler::instance()->getItemDataByType(pGDCurrentSelectedItemData->_contextId, AF_TREE_ITEM_GL_VBO, i);
                //    exportListVector.push_back(pViewerItem);
                //}

                // Get amount of displayed FBOs:
                int amountOfDisplayedFBOs = gdDebugApplicationTreeHandler::instance()->amountOfDisplayedObjectForType(pGDCurrentSelectedItemData->_contextId, AF_TREE_ITEM_GL_FBO);

                // Loop through the PBuffers:
                for (int i = 0; i < amountOfDisplayedFBOs; i++)
                {
                    // Get the item data from the tree item id:
                    afApplicationTreeItemData* pViewerItem = gdDebugApplicationTreeHandler::instance()->getItemDataByType(pGDCurrentSelectedItemData->_contextId, AF_TREE_ITEM_GL_FBO_NODE, i);

                    // Is item a texture / static buffer / PBuffer / etc... ?
                    if (afApplicationTreeItemData::isItemImageOrBuffer(pViewerItem->m_itemType))
                    {
                        exportListVector.push_back(pViewerItem);
                    }
                }

                // Get amount of displayed render buffers:
                int amountOfDisplayedRenderBuffers = gdDebugApplicationTreeHandler::instance()->amountOfDisplayedObjectForType(pGDCurrentSelectedItemData->_contextId, AF_TREE_ITEM_GL_RENDER_BUFFER);

                // Loop through the render buffers:
                for (int i = 0; i < amountOfDisplayedRenderBuffers; i++)
                {
                    // Get the item data from the tree item id:
                    afApplicationTreeItemData* pViewerItem = gdDebugApplicationTreeHandler::instance()->getItemDataByType(pGDCurrentSelectedItemData->_contextId, AF_TREE_ITEM_GL_RENDER_BUFFER, i);

                    // Is item a texture / static buffer / PBuffer / etc... ?
                    if (afApplicationTreeItemData::isItemImageOrBuffer(pViewerItem->m_itemType))
                    {
                        exportListVector.push_back(pViewerItem);
                    }
                }

                // Get amount of displayed pbuffers:
                int amountOfDisplayedPBuffers = gdDebugApplicationTreeHandler::instance()->amountOfDisplayedObjectForType(apContextID(AP_NULL_CONTEXT, 0), AF_TREE_ITEM_GL_PBUFFER_NODE);

                // Loop through the render buffers:
                for (int i = 0; i < amountOfDisplayedPBuffers; i++)
                {
                    // Get the item data from the tree item id:
                    afApplicationTreeItemData* pViewerItem = gdDebugApplicationTreeHandler::instance()->getItemDataByType(pGDCurrentSelectedItemData->_contextId, AF_TREE_ITEM_GL_PBUFFERS_NODE, i);

                    // Is item a texture / static buffer / PBuffer / etc... ?
                    if (afApplicationTreeItemData::isItemImageOrBuffer(pViewerItem->m_itemType))
                    {
                        exportListVector.push_back(pViewerItem);
                    }
                }

                // Yuri: June 11, 2014 Commented out - OpenCL buffers cannot be saved as images ( only as CSV )
                // Get amount of OpenCL buffers:
                //int amountOfDisplayedCLBuffers = gdDebugApplicationTreeHandler::instance()->amountOfDisplayedObjectForType(pGDCurrentSelectedItemData->_contextId, AF_TREE_ITEM_CL_BUFFER);

                //// Loop through the buffers:
                //for (int i = 0; i < amountOfDisplayedCLBuffers; i++)
                //{
                //    // Get the item data from the tree item id:
                //    afApplicationTreeItemData* pViewerItem = gdDebugApplicationTreeHandler::instance()->getItemDataByType(pGDCurrentSelectedItemData->_contextId, AF_TREE_ITEM_CL_BUFFER, i);
                //    exportListVector.push_back(pViewerItem);
                //}
            }
        }

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::saveLastViewedItemProperties
// Description: Saves the last viewed item properties
// Author:      Eran Zinman
// Date:        10/1/2008
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::saveLastViewedItemProperties()
{
    _lastViewedItemProperties._arePropertiesValid = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pFocusedImageBufferView != NULL)
    {
        // Flag that last viewed item properties are valid
        _lastViewedItemProperties._arePropertiesValid = true;

        // If we are in a single item view mode:
        if (!isThumbnailViewActive())
        {
            // Save the properties from the image view:
            _pFocusedImageBufferView->saveLastViewedItemProperties(_pFocusedImageBufferView->currentZoomLevel());
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::updateOpenedViewsOnEvent
// Description: Update each of the open views with according to the current run status.
// Arguments:   bool isSuspended - is the debugged process currently suspended
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        29/12/2010
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::updateOpenedViewsOnEvent(bool isSuspended)
{
    // Set is suspended flag:
    _isDebuggedProcessSuspended = isSuspended;

    // Go through each of the open single views, and perform the right operation:
    for (int i = 0; i < (int)_displayedImageBufferViews.size(); i++)
    {
        // Get the current image / buffer view:
        gdImageAndBufferView* pImageBufferView = _displayedImageBufferViews[i];

        if (pImageBufferView != NULL)
        {
            // Set the monitored tree (it could be that the views were created before the tree existed):
            if (!_isDebuggedProcessSuspended)
            {
                // Initialize the status of the load:
                pImageBufferView->initializeObjectNotLoadedStatus(AF_ITEM_NOT_LOADED, AF_ITEM_LOAD_PROCESS_IS_RUNNING);
            }

            // Perform operations for process run suspended:
            bool doesObjectExist = false;
            pImageBufferView->updateObjectDisplay(doesObjectExist);

            // Apply RGBA filters to the images:
            if (doesObjectExist)
            {
                pImageBufferView->setFilterForAllImages(lastViewedItemProperties()._actionsMask);
            }

            // Update the view:
            pImageBufferView->layout()->activate();
        }
    }

    // Go through each of the open thumbnail views, and perform the right operation:
    for (int i = 0; i < (int)_displayedThumbnailViews.size(); i++)
    {
        // Get the current image / buffer view:
        gdThumbnailView* pThumbnailView = _displayedThumbnailViews[i];

        if (pThumbnailView != NULL)
        {
            if (isSuspended)
            {
                // Perform operations for process run suspended:
                pThumbnailView->updateObjectDisplay();
            }
            else
            {
                // Perform operations for process run:
                pThumbnailView->onProcessNotSuspended();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::setFocusedViews
// Description: The function sets the currently focused view
//              Notice: only one of the view can be focused, therefor the other
//              one should be null
// Arguments:   gdImageAndBufferView* pImageBufferView
//              gdThumbnailView* pThumbnailView
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        29/12/2010
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::setFocusedViews(gdImageAndBufferView* pFocusedImageBufferView, gdThumbnailView* pFocusedThumbnailView, bool isDebuggedProcessSuspended)
{
    _pFocusedImageBufferView = pFocusedImageBufferView;
    _pFocusedThumbnailView = pFocusedThumbnailView;

    _isDebuggedProcessSuspended = isDebuggedProcessSuspended;

    if ((_pFocusedImageBufferView != NULL) && (_pFocusedThumbnailView == NULL))
    {
        _isThumbnailViewCurrentlyActive = false;

        // Check if the loaded item is a 'real' one:
        _isItemLoaded = false;

        if (_pFocusedImageBufferView->displayedItemData() != NULL)
        {
            _isItemLoaded = (_pFocusedImageBufferView->displayedItemData()->_itemLoadStatus._itemLoadStatusType == AF_ITEM_LOAD_SUCCESS);
        }

        // Check which actions are enabled according to the object type:
        enableImageActionsByItemType(_pFocusedImageBufferView->displayedItemID().m_itemType);

        // We call set focus again, since we want the update functions to be called after the focused view is set:
        _pFocusedImageBufferView->update();
    }
    else if ((_pFocusedImageBufferView == NULL) && (_pFocusedThumbnailView != NULL))
    {
        _isThumbnailViewCurrentlyActive = true;

        // Check if the loaded item is a 'real' one:
        _isItemLoaded = false;

        if (_pFocusedThumbnailView->displayedItemData() != NULL)
        {
            _isItemLoaded = (_pFocusedThumbnailView->displayedItemData()->_itemLoadStatus._itemLoadStatusType == AF_ITEM_LOAD_SUCCESS);
        }

        // Check which actions are enabled according to the object type:
        enableImageActionsByItemType(_pFocusedThumbnailView->displayedItemID().m_itemType);

        // We call set focus again, since we want the update functions to be called after the focused view is set:
        _pFocusedThumbnailView->update();
    }
    else
    {
        GT_ASSERT_EX(false, L"Illegal function usage: setFocusedViews");
    }
}

// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::updateOpenedViewsHexDisplayMode
// Description: Update each of the open views with according to the current hex mode.
// Author:      Gilad Yarnitzky
// Date:        19/5/2011
// ---------------------------------------------------------------------------
void gdImageAndBufferViewsController::updateOpenedViewsHexDisplayMode(bool hexDisplayMode)
{
    // Go through each of the open single views, and perform the right operation:
    for (int i = 0; i < (int)_displayedImageBufferViews.size(); i++)
    {
        // Get the current image / buffer view:
        gdImageAndBufferView* pImageBufferView = _displayedImageBufferViews[i];

        if (pImageBufferView != NULL)
        {
            // Set the hex display mode:
            pImageBufferView->setHexDisplayMode(hexDisplayMode);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::getExistingImageBufferView
// Description: Look for an existing image / buffer view. The view should match
//              the object displayed in objectFilePath
// Arguments:   const osFilePath& objectFilePath
// Return Val:  gdImageAndBufferView*
// Author:      Sigal Algranaty
// Date:        24/8/2011
// ---------------------------------------------------------------------------
gdImageAndBufferView* gdImageAndBufferViewsController::getExistingImageBufferView(const osFilePath& objectFilePath)
{
    gdImageAndBufferView* pRetVal = NULL;

    // Get the monitored objects tree:
    gdDebugApplicationTreeHandler* pMonitoredObjectTree = gdDebugApplicationTreeHandler::instance();
    GT_IF_WITH_ASSERT(pMonitoredObjectTree != NULL)
    {
        // Get a matching item data from tree:
        afApplicationTreeItemData* pExistingItemData = NULL;
        bool doesExist = pMonitoredObjectTree->doesItemExist(objectFilePath, pExistingItemData);

        if (doesExist && (pExistingItemData != NULL))
        {
            // Go through each of the open single views, and perform the right operation:
            for (int i = 0; i < (int)_displayedImageBufferViews.size(); i++)
            {
                // Get the current image / buffer view:
                gdImageAndBufferView* pImageBufferView = _displayedImageBufferViews[i];

                if (pImageBufferView != NULL)
                {
                    // Get the current view displayed item data:
                    bool isTheSame = pImageBufferView->displayedItemID().isSameObject(pExistingItemData);

                    if (isTheSame)
                    {
                        pRetVal = pImageBufferView;
                        break;
                    }
                }
            }
        }
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::getExistingThumbnailView
// Description: Search for an existing thumbnail view that is related to the object described in objectFilePath
// Arguments:   const osFilePath& objectFilePath
// Return Val:  gdThumbnailView*
// Author:      Sigal Algranaty
// Date:        24/8/2011
// ---------------------------------------------------------------------------
gdThumbnailView* gdImageAndBufferViewsController::getExistingThumbnailView(const osFilePath& objectFilePath)
{
    gdThumbnailView* pRetVal = NULL;

    // Get the monitored objects tree:
    gdDebugApplicationTreeHandler* pMonitoredObjectTree = gdDebugApplicationTreeHandler::instance();
    GT_IF_WITH_ASSERT(pMonitoredObjectTree != NULL)
    {
        // Get a matching item data from tree:
        afApplicationTreeItemData* pExistingItemData = NULL;
        bool doesExist = pMonitoredObjectTree->doesItemExist(objectFilePath, pExistingItemData);

        if (doesExist && (pExistingItemData != NULL))
        {
            // Go through each of the open single views, and perform the right operation:
            for (int i = 0; i < (int)_displayedThumbnailViews.size(); i++)
            {
                // Get the current image / buffer view:
                gdThumbnailView* pThumbnailView = _displayedThumbnailViews[i];

                if (pThumbnailView != NULL)
                {
                    // Get the current view displayed item data:
                    bool isTheSame = pThumbnailView->displayedItemID().isSameObject(pExistingItemData);

                    if (isTheSame)
                    {
                        pRetVal = pThumbnailView;
                        break;
                    }
                }
            }
        }
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::removeExistingView
// Description: Removes the requested view.
// Arguments:   QWidget* pExistingView
// Return Val:  bool - true iff the window exist within the displayed views
// Author:      Sigal Algranaty
// Date:        1/9/2011
// ---------------------------------------------------------------------------
bool gdImageAndBufferViewsController::removeExistingView(QWidget* pExistingView)
{
    bool retVal = false;

    // Go through each of the views and set the active tool:
    for (int i = 0; i < (int)_displayedThumbnailViews.size(); i++)
    {
        // Check if the window is one of the thumbnail views:
        if (pExistingView == _displayedThumbnailViews[i])
        {
            _displayedThumbnailViews.removeItem(i);
            retVal = true;
            break;
        }
    }

    if (!retVal)
    {
        // Go through each of the views and set the active tool:
        for (int i = 0; i < (int)_displayedImageBufferViews.size(); i++)
        {
            // Check if the window is one of the thumbnail views:
            if (pExistingView == _displayedImageBufferViews[i])
            {
                _displayedImageBufferViews.removeItem(i);
                retVal = true;
                break;
            }
        }
    }

    // Reset the focused view:
    if (_pFocusedImageBufferView == pExistingView)
    {
        _pFocusedImageBufferView = NULL;
    }

    if (_pFocusedThumbnailView == pExistingView)
    {
        _pFocusedThumbnailView = NULL;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdImageAndBufferViewsController::getCurrentlySelectedViewerItem
// Description: Return the item data attached to the currently displayed item
// Return Val:  afApplicationTreeItemData*
// Author:      Sigal Algranaty
// Date:        20/9/2011
// ---------------------------------------------------------------------------
afApplicationTreeItemData* gdImageAndBufferViewsController::getCurrentlySelectedViewerItem()
{
    afApplicationTreeItemData* pRetVal = NULL;

    if (_pFocusedThumbnailView != NULL)
    {
        pRetVal = _pFocusedThumbnailView->displayedItemData();
    }
    else if (_pFocusedImageBufferView != NULL)
    {
        pRetVal = _pFocusedImageBufferView->displayedItemData();
    }

    return pRetVal;
}
