//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwImagesAndBuffersMDIViewCreator.cpp
///
//==================================================================================

//------------------------------ gwImagesAndBuffersMDIViewCreator.cpp ------------------------------

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewCreateEvent.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>


// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/gdImagesAndBuffersManager.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdImageAndBufferView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdThumbnailView.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afActionCreatorAbstract.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <src/gwImagesAndBuffersActionsCreator.h>
#include <src/gwImagesAndBuffersMDIViewCreator.h>


// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersMDIViewCreator::gwImagesAndBuffersMDIViewCreator
// Description: Creator
// Author:      Sigal Algranaty
// Date:        27/7/2011
// ---------------------------------------------------------------------------
gwImagesAndBuffersMDIViewCreator::gwImagesAndBuffersMDIViewCreator() : _pApplicationCommandsHandler(NULL)
{
    // Get the application commands handler:
    _pApplicationCommandsHandler = gdApplicationCommands::gdInstance();
    GT_ASSERT(_pApplicationCommandsHandler != NULL);

    // Initialize my action creator:
    _pViewActionCreator = new gwImagesAndBuffersActionsCreator;


    // Initialize the creator:
    _pViewActionCreator->initializeCreator();
}



// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersMDIViewCreator::~gwImagesAndBuffersMDIViewCreator
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
gwImagesAndBuffersMDIViewCreator::~gwImagesAndBuffersMDIViewCreator()
{
}

// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersMDIViewCreator::titleString
// Description: Get the title of the created view
// Arguments:   int viewIndex
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
void gwImagesAndBuffersMDIViewCreator::titleString(int viewIndex, gtString& viewTitle, gtString& viewMenuCommand)
{
    (void)(viewIndex); // unused
    GT_IF_WITH_ASSERT(_pCreationEvent != NULL)
    {
        // Source code creation event:
        if (_pCreationEvent->eventType() == apEvent::AP_MDI_CREATED_EVENT)
        {
            // Down cast the event:
            apMDIViewCreateEvent* pImageBufferEvent = (apMDIViewCreateEvent*)_pCreationEvent;
            GT_IF_WITH_ASSERT(pImageBufferEvent != NULL)
            {
                // Check if the title is set on the event:
                viewTitle = pImageBufferEvent->viewTitle();

                if (viewTitle.isEmpty())
                {
                    // Extract the title from the file path:
                    pImageBufferEvent->filePath().getFileNameAndExtension(viewTitle);
                }
            }
        }
    }

    viewMenuCommand = viewTitle;
}


// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersMDIViewCreator::associatedToolbar
// Description: Get the name of the toolbar associated with the requested view
// Arguments:   int viewIndex
// Return Val:  gtString
// Author:      Sigal Algranaty
// Date:        28/7/2011
// ---------------------------------------------------------------------------
gtString gwImagesAndBuffersMDIViewCreator::associatedToolbar(int viewIndex)
{
    (void)(viewIndex); // unused
    gtString retVal;

    // Source code creation event:
    if (_pCreationEvent->eventType() == apEvent::AP_MDI_CREATED_EVENT)
    {
        retVal = AF_STR_ImagesAndBuffersToolbar;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersMDIViewCreator::type
// Description: Get view type
// Arguments:   int viewIndex
// Return Val:  afViewType
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
gwImagesAndBuffersMDIViewCreator::afViewType gwImagesAndBuffersMDIViewCreator::type(int viewIndex)
{
    (void)(viewIndex); // unused
    afViewCreatorAbstract::afViewType retDockArea = AF_VIEW_mdi;

    return retDockArea;
}

// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersMDIViewCreator::dockArea
// Description: Get the docking area
// Arguments:   int viewIndex
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
int gwImagesAndBuffersMDIViewCreator::dockArea(int viewIndex)
{
    (void)(viewIndex); // unused
    int retVal = AF_VIEW_DOCK_LeftDockWidgetArea;
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersMDIViewCreator::dockWidgetFeatures
// Description: Irrelevant for MDI views
// Return Val:  DockWidgetFeatures
// Author:      Sigal Algranaty
// Date:        1/8/2011
// ---------------------------------------------------------------------------
QDockWidget::DockWidgetFeatures gwImagesAndBuffersMDIViewCreator::dockWidgetFeatures(int viewIndex)
{
    (void)(viewIndex); // unused
    return QDockWidget::NoDockWidgetFeatures;
}

// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersMDIViewCreator::initialSize
// Description: Get the initial size
// Arguments:   int viewIndex
// Return Val:  QSize - size of the view
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
QSize gwImagesAndBuffersMDIViewCreator::initialSize(int viewIndex)
{
    (void)(viewIndex); // unused
    QSize retSize(0, 0);

    return retSize;
}

// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersMDIViewCreator::visibility
// Description: Get the initial visibility of the view
// Arguments:   int viewIndex
// Return Val:  bool - Success / failure.
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
bool gwImagesAndBuffersMDIViewCreator::visibility(int viewIndex)
{
    (void)(viewIndex); // unused
    bool retVal = true;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersMDIViewCreator::amountOfViewTypes
// Description: Get number of views that can be created by this creator
// Return Val:  int
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
int gwImagesAndBuffersMDIViewCreator::amountOfViewTypes()
{
    return 1;
}


// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersMDIViewCreator::getCurrentlyDisplayedFilePath
// Description: Get the currently displayed file path
// Arguments:   osFilePath& filePath
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/8/2011
// ---------------------------------------------------------------------------
bool gwImagesAndBuffersMDIViewCreator::getCurrentlyDisplayedFilePath(osFilePath& filePath)
{
    bool retVal = false;

    // Source code creation event:
    if (_pCreationEvent->eventType() == apEvent::AP_MDI_CREATED_EVENT)
    {
        // Down cast the event:
        apMDIViewCreateEvent* pImageBufferEvent = (apMDIViewCreateEvent*)_pCreationEvent;
        GT_IF_WITH_ASSERT(pImageBufferEvent != NULL)
        {
            // Get the file path from the event:
            filePath = pImageBufferEvent->filePath();
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersMDIViewCreator::getMatchingTreeItemData
// Description: Given the event with the object file path, get the item data
//              that is representing this item in the monitored objects tree
// Return Val:  afApplicationTreeItemData*
// Author:      Sigal Algranaty
// Date:        23/8/2011
// ---------------------------------------------------------------------------
afApplicationTreeItemData* gwImagesAndBuffersMDIViewCreator::getMatchingTreeItemData(gdDebugApplicationTreeHandler* pMonitoredObjectTree)
{
    afApplicationTreeItemData* pRetVal = NULL;

    // Sanity check:
    GT_IF_WITH_ASSERT(pMonitoredObjectTree != NULL)
    {
        osFilePath filePath;
        bool rc = getCurrentlyDisplayedFilePath(filePath);
        GT_IF_WITH_ASSERT(rc)
        {
            // Get the item data from tree
            rc = pMonitoredObjectTree->doesItemExist(filePath, pRetVal);
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersMDIViewCreator::createViewContent
// Description: Create the content of the view
// Arguments:   int viewIndex
//              QWidget*& pContentQWidget
//              QWidget* pQParent
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        23/8/2011
// ---------------------------------------------------------------------------
bool gwImagesAndBuffersMDIViewCreator::createViewContent(int viewIndex, QWidget*& pContentQWidget, QWidget* pQParent)
{
    (void)(viewIndex); // unused
    bool retVal = false;

    pContentQWidget = NULL;

    retVal = true;

    // Set the default minimum view height and width
    int minViewWidth = 100;
    int minViewHeight = 30;
    QSize minViewSize(minViewWidth, minViewHeight);

    // Get the monitored objects tree:
    gdDebugApplicationTreeHandler* pMonitoredObjectTree = gdDebugApplicationTreeHandler::instance();
    GT_IF_WITH_ASSERT(pMonitoredObjectTree != NULL)
    {
        // Get the matching item data from tree:
        afApplicationTreeItemData* pDisplayedItemData = getMatchingTreeItemData(pMonitoredObjectTree);

        // Sanity check:
        GT_IF_WITH_ASSERT(pDisplayedItemData != NULL)
        {
            // Create a QT window:
            if (afApplicationTreeItemData::isItemThumbnail(pDisplayedItemData->m_itemType))
            {
                // Create a thumbnail view:
                gdThumbnailView* pThumbnailView = gdImagesAndBuffersManager::instance().createNewThumbnailView(pQParent, QSize(-1, -1));
                GT_IF_WITH_ASSERT(pThumbnailView != NULL)
                {
                    pContentQWidget = pThumbnailView;

                    // Add the window to the opened views:
                    bool rc = gdImagesAndBuffersManager::instance().displayThumbnailItem(pThumbnailView, pDisplayedItemData);
                    GT_ASSERT(rc);

                    // Check if the debugged process is suspended:
                    bool isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

                    // Set the focused view:
                    gdImagesAndBuffersManager::instance().controller().setFocusedViews(NULL, pThumbnailView, isDebuggedProcessSuspended);
                }

            }
            else if (afApplicationTreeItemData::isItemImageOrBuffer(pDisplayedItemData->m_itemType))
            {
                // Create a image / buffer view:
                gdImageAndBufferView* pImageBufferView = gdImagesAndBuffersManager::instance().createNewImageAndBufferView(pQParent, QSize(-1, -1));
                GT_IF_WITH_ASSERT(pImageBufferView != NULL)
                {
                    pContentQWidget = pImageBufferView;
                    bool rc = gdImagesAndBuffersManager::instance().displayItem(pImageBufferView, pDisplayedItemData);
                    GT_ASSERT(rc);

                    // Check if the debugged process is suspended:
                    bool isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

                    // Set the focused view:
                    gdImagesAndBuffersManager::instance().controller().setFocusedViews(pImageBufferView, NULL, isDebuggedProcessSuspended);
                }
            }
        }
    }

    if (pContentQWidget != NULL)
    {
        // Set the created window:
        m_viewsCreated.push_back(pContentQWidget);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersMDIViewCreator::displayExistingView
// Description: Display the existing view with the event details. If the view does not
//              exist, false is returned
// Arguments:   const apMDIViewCreateEvent& mdiViewEvent
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/8/2011
// ---------------------------------------------------------------------------
bool gwImagesAndBuffersMDIViewCreator::displayExistingView(const apMDIViewCreateEvent& mdiViewEvent)
{
    bool retVal = false;

    // Check if the debugged process is suspended:
    bool isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

    // Get the file path from the event:
    osFilePath filePath = mdiViewEvent.filePath();

    gdImageAndBufferView* pImageBufferView = gdImagesAndBuffersManager::instance().controller().getExistingImageBufferView(filePath);

    if (pImageBufferView == NULL)
    {
        gdThumbnailView* pThumbnailView = gdImagesAndBuffersManager::instance().controller().getExistingThumbnailView(filePath);

        if (pThumbnailView != NULL)
        {
            // Update the object display:
            pThumbnailView->updateObjectDisplay();

            // Set the focused view:
            gdImagesAndBuffersManager::instance().controller().setFocusedViews(NULL, pThumbnailView, isDebuggedProcessSuspended);

            retVal = true;
        }
    }
    else
    {
        // if pImageBufferView already exists leave it as is
        retVal = true;
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersMDIViewCreator::onMDISubWindowClose
// Description: Handle sub window close
// Arguments:   afQMdiSubWindow* pMDISubWindow
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/9/2011
// ---------------------------------------------------------------------------
bool gwImagesAndBuffersMDIViewCreator::onMDISubWindowClose(afQMdiSubWindow* pMDISubWindow)
{
    bool retVal = true;

    // Sanity check:
    GT_IF_WITH_ASSERT(pMDISubWindow != NULL)
    {
        // Get the sub window widget:
        QWidget* pWidget = pMDISubWindow->widget();

        if (pWidget != NULL)
        {
            // Remove the view from the manager:
            retVal = gdImagesAndBuffersManager::instance().controller().removeExistingView(pMDISubWindow->widget());
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersMDIViewCreator::handleTrigger
// Description: Handle the action when it is triggered
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gwImagesAndBuffersMDIViewCreator::handleTrigger(int viewIndex, int actionIndex)
{
    // Get the action index as command id:
    int actionCommandId = actionIndex + ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_FIRST_ACTION;

    switch (actionCommandId)
    {
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_NORMAL:
            gdImagesAndBuffersManager::instance().controller().onStandardPointer();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_PAN:
            gdImagesAndBuffersManager::instance().controller().onPan();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMIN:
            gdImagesAndBuffersManager::instance().controller().onZoomIn();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMOUT:
            gdImagesAndBuffersManager::instance().controller().onZoomOut();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BEST_FIT:
            gdImagesAndBuffersManager::instance().controller().onBestFit();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_ORIGINAL_SIZE:
            gdImagesAndBuffersManager::instance().controller().onOriginalSize();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_RED_CHANNEL:
            gdImagesAndBuffersManager::instance().controller().onColorChannel(AC_IMAGE_CHANNEL_RED);
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GREEN_CHANNEL:
            gdImagesAndBuffersManager::instance().controller().onColorChannel(AC_IMAGE_CHANNEL_GREEN);
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BLUE_CHANNEL:
            gdImagesAndBuffersManager::instance().controller().onColorChannel(AC_IMAGE_CHANNEL_BLUE);
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ALPHA_CHANNEL:
            gdImagesAndBuffersManager::instance().controller().onColorChannel(AC_IMAGE_CHANNEL_ALPHA);
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GRAYSCALE:
            gdImagesAndBuffersManager::instance().controller().onGrayscale();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_INVERT:
            gdImagesAndBuffersManager::instance().controller().onInvert();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_LEFT:
            gdImagesAndBuffersManager::instance().controller().onRotateLeft();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_RIGHT:
            gdImagesAndBuffersManager::instance().controller().onRotateRight();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_COPY:
            gdImagesAndBuffersManager::instance().controller().onEditCopy();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_SELECT_ALL:
            gdImagesAndBuffersManager::instance().controller().onEditSelectAll();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_SELECT_IMAGE_VIEW:
            gdImagesAndBuffersManager::instance().controller().onMenuImageViewSelected();
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_SELECT_DATA_VIEW:
            gdImagesAndBuffersManager::instance().controller().onMenuDataViewSelected();
            break;

        default:
            GT_ASSERT_EX(false, L"Unsupported command id");

    }

    // Handle the UI update of the actions:
    handleUiUpdate(viewIndex, actionIndex);

}

// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersMDIViewCreator::handleUiUpdate
// Description: Handle UI update
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gwImagesAndBuffersMDIViewCreator::handleUiUpdate(int viewIndex, int actionIndex)
{
    (void)(viewIndex); // unused
    bool isActionEnabled = false, isActionChecked = false, isActionCheckable = false, isActionVisible = false;

    GT_IF_WITH_ASSERT(_pApplicationCommandsHandler != NULL)
    {
        // Get the action index as command id:
        int actionCommandId = actionIndexToCommandId(actionIndex);

        switch (actionCommandId)
        {
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_NORMAL:
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_PAN:
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMIN:
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMOUT:
            case ID_IMAGES_AND_BUFFERS_VIEWER_ORIGINAL_SIZE:
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BEST_FIT:
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_LEFT:
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_RIGHT:
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_RED_CHANNEL:
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GREEN_CHANNEL:
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BLUE_CHANNEL:
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ALPHA_CHANNEL:
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GRAYSCALE:
            case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_INVERT:
                isActionEnabled = gdImagesAndBuffersManager::instance().controller().shouldCommandBeEnabled(isActionChecked, actionCommandId);
                isActionCheckable = true;
                break;

            default:
            {
                GT_ASSERT_EX(false, L"Unsupported application command");
                break;
            }
        }
    }

    // Check if there is an image buffer view opened:
    isActionVisible = gdImagesAndBuffersManager::instance().controller().isImageBufferItemDisplayed();

    // Sanity check:
    GT_IF_WITH_ASSERT(_pViewActionCreator != NULL)
    {
        // Get the QT action:
        QAction* pAction = _pViewActionCreator->action(actionIndex);
        GT_IF_WITH_ASSERT(pAction != NULL)
        {
            // Set the action enable / disable:
            pAction->setEnabled(isActionEnabled);

            if (isActionEnabled)
            {
                // Set the action checkable state:
                pAction->setCheckable(isActionCheckable);

                // Set the action check state:
                pAction->setChecked(isActionChecked);

                // Set the action visibility:
                pAction->setVisible(isActionVisible);
            }
        }
    }
}
