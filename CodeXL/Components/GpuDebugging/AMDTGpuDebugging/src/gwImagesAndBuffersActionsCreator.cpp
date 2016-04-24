//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwImagesAndBuffersActionsCreator.cpp
///
//==================================================================================

//------------------------------ gwImagesAndBuffersActionsCreator.h ------------------------------

// QT:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdImagesAndBuffersManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <src/gwImagesAndBuffersActionsCreator.h>

// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersActionsCreator::gwImagesAndBuffersActionsCreator
// Description: Creator
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
gwImagesAndBuffersActionsCreator::gwImagesAndBuffersActionsCreator()
{
}

// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersActionsCreator::~gwImagesAndBuffersActionsCreator
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
gwImagesAndBuffersActionsCreator::~gwImagesAndBuffersActionsCreator()
{

}

// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersActionsCreator::populateSupportedCommandIds
// Description: Create a vector of command Ids that are supported by this actions creator object
// Author:      Doron Ofek
// Date:        Mar-3, 2015
// ---------------------------------------------------------------------------
void gwImagesAndBuffersActionsCreator::populateSupportedCommandIds()
{
    // fill the vector of supported command ids:
    m_supportedCommandIds.push_back(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_NORMAL);
    m_supportedCommandIds.push_back(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_PAN);
    m_supportedCommandIds.push_back(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMIN);
    m_supportedCommandIds.push_back(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMOUT);
    m_supportedCommandIds.push_back(ID_IMAGES_AND_BUFFERS_VIEWER_ORIGINAL_SIZE);
    m_supportedCommandIds.push_back(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BEST_FIT);
    m_supportedCommandIds.push_back(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_LEFT);
    m_supportedCommandIds.push_back(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_RIGHT);
    m_supportedCommandIds.push_back(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_RED_CHANNEL);
    m_supportedCommandIds.push_back(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GREEN_CHANNEL);
    m_supportedCommandIds.push_back(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BLUE_CHANNEL);
    m_supportedCommandIds.push_back(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ALPHA_CHANNEL);
    m_supportedCommandIds.push_back(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GRAYSCALE);
    m_supportedCommandIds.push_back(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_INVERT);
}

// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersActionsCreator::initApplicationIcons
// Description: Create the icons for the application commands
// Author:      Sigal Algranaty
// Date:        26/7/2011
// ---------------------------------------------------------------------------
void gwImagesAndBuffersActionsCreator::initActionIcons()
{
    // Initialize the commands icons:

    // Normal (pick) command:
    initSingleActionIcon(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_NORMAL - ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_FIRST_ACTION, AC_ICON_DEBUG_TEXVW_TOOLBAR_MOUSE);

    // Pan command:
    initSingleActionIcon(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_PAN - ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_FIRST_ACTION, AC_ICON_DEBUG_TEXVW_TOOLBAR_PAN);

    // Zoom in command:
    initSingleActionIcon(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMIN - ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_FIRST_ACTION, AC_ICON_DEBUG_TEXVW_TOOLBAR_ZOOMIN);

    // Zoom out command:
    initSingleActionIcon(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMOUT - ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_FIRST_ACTION, AC_ICON_DEBUG_TEXVW_TOOLBAR_ZOOMOUT);

    // Original size command:
    initSingleActionIcon(ID_IMAGES_AND_BUFFERS_VIEWER_ORIGINAL_SIZE - ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_FIRST_ACTION, AC_ICON_DEBUG_TEXVW_TOOLBAR_ORIGINAL);

    // Best fit command:
    initSingleActionIcon(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BEST_FIT - ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_FIRST_ACTION, AC_ICON_DEBUG_TEXVW_TOOLBAR_BEST);

    // Rotate left:
    initSingleActionIcon(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_LEFT - ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_FIRST_ACTION, AC_ICON_DEBUG_TEXVW_TOOLBAR_ROTATEL);

    // Rotate right:
    initSingleActionIcon(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_RIGHT - ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_FIRST_ACTION, AC_ICON_DEBUG_TEXVW_TOOLBAR_ROTATER);

    // Red channel:
    initSingleActionIcon(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_RED_CHANNEL - ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_FIRST_ACTION, AC_ICON_DEBUG_TEXVW_TOOLBAR_RED);

    // Green channel:
    initSingleActionIcon(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GREEN_CHANNEL - ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_FIRST_ACTION, AC_ICON_DEBUG_TEXVW_TOOLBAR_GREEN);

    // Blue channel:
    initSingleActionIcon(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BLUE_CHANNEL - ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_FIRST_ACTION, AC_ICON_DEBUG_TEXVW_TOOLBAR_BLUE);

    // Alpha channel:
    initSingleActionIcon(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ALPHA_CHANNEL - ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_FIRST_ACTION, AC_ICON_DEBUG_TEXVW_TOOLBAR_ALPHA);

    // Gray-scale:
    initSingleActionIcon(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GRAYSCALE - ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_FIRST_ACTION, AC_ICON_DEBUG_TEXVW_TOOLBAR_GRAY);

    // Invert:
    initSingleActionIcon(ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_INVERT - ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_FIRST_ACTION, AC_ICON_DEBUG_TEXVW_TOOLBAR_INVERT);
}


// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersActionsCreator::caption
// Description: Get the caption of the action item
// Arguments:   int actionIndex
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
bool gwImagesAndBuffersActionsCreator::actionText(int actionIndex, gtString& caption, gtString& tooltip, gtString& keyboardShortcut)
{
    (void)(keyboardShortcut); // unused
    bool retVal = true;

    // Get the action index as command id:
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_NORMAL:
            caption = GD_STR_ImagesAndBuffersViewerToolbarSelectTooltip;
            tooltip = GD_STR_ImagesAndBuffersViewerToolbarSelectTooltip;
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_PAN:
            caption = GD_STR_ImagesAndBuffersViewerToolbarPanTooltip;
            tooltip = GD_STR_ImagesAndBuffersViewerToolbarPanTooltip;
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMIN:
            caption = GD_STR_ImagesAndBuffersViewerToolbarZoomInTool;
            tooltip = GD_STR_ImagesAndBuffersViewerToolbarZoomInTool;
            keyboardShortcut = GD_STR_keyboardShortcutZoomInString;
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMOUT:
            caption = GD_STR_ImagesAndBuffersViewerToolbarZoomOutTool;
            tooltip = GD_STR_ImagesAndBuffersViewerToolbarZoomOutTool;
            keyboardShortcut = GD_STR_keyboardShortcutZoomOutString;
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_ORIGINAL_SIZE:
            caption = GD_STR_ImagesAndBuffersViewerToolbarOriginalSizeTool;
            tooltip = GD_STR_ImagesAndBuffersViewerToolbarOriginalSizeTool;
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BEST_FIT:
            caption = GD_STR_ImagesAndBuffersViewerToolbarBestFitTool;
            tooltip = GD_STR_ImagesAndBuffersViewerToolbarBestFitTool;
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_LEFT:
            caption = GD_STR_ImagesAndBuffersViewerToolbarRotateLeft;
            tooltip = GD_STR_ImagesAndBuffersViewerToolbarRotateLeft;
            keyboardShortcut = GD_STR_keyboardShortcutLeftString;
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_RIGHT:
            caption = GD_STR_ImagesAndBuffersViewerToolbarRotateRight;
            tooltip = GD_STR_ImagesAndBuffersViewerToolbarRotateRight;
            keyboardShortcut = GD_STR_keyboardShortcutRightString;
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_RED_CHANNEL:
            caption = GD_STR_ImagesAndBuffersViewerToolbarRedChannelFilter;
            tooltip = GD_STR_ImagesAndBuffersViewerToolbarRedChannelFilter;
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GREEN_CHANNEL:
            caption = GD_STR_ImagesAndBuffersViewerToolbarGreenChannelFilter;
            tooltip = GD_STR_ImagesAndBuffersViewerToolbarGreenChannelFilter;
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BLUE_CHANNEL:
            caption = GD_STR_ImagesAndBuffersViewerToolbarBlueChannelFilter;
            tooltip = GD_STR_ImagesAndBuffersViewerToolbarBlueChannelFilter;
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ALPHA_CHANNEL:
            caption = GD_STR_ImagesAndBuffersViewerToolbarAlphaChannelFilter;
            tooltip = GD_STR_ImagesAndBuffersViewerToolbarAlphaChannelFilter;
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GRAYSCALE:
            caption = GD_STR_ImagesAndBuffersViewerToolbarGrayscaleMode;
            tooltip = GD_STR_ImagesAndBuffersViewerToolbarGrayscaleMode;
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_INVERT:
            caption = GD_STR_ImagesAndBuffersViewerToolbarInvertMode;
            tooltip = GD_STR_ImagesAndBuffersViewerToolbarInvertMode;
            break;

        default:
            GT_ASSERT(false);
            retVal = false;
            break;

    };

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersActionsCreator::separatorPosition
// Description: separator position
// Returns:     -1 if separator is needed before actionIndex item, 1 if after and 0 if no separator is needed
// Arguments:   int actionIndex
// Author:      Yuri Rshtunique
// Date:        5/8/2014
// ---------------------------------------------------------------------------
int gwImagesAndBuffersActionsCreator::separatorPosition(int actionIndex)
{
    int retVal = 0;

    // Get the action index as command id:
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMIN:
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_LEFT:
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_RED_CHANNEL:
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_INVERT:
            retVal = -1;
            break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_PAN:
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BEST_FIT:
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_RIGHT:
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ALPHA_CHANNEL:
            retVal = 1; // separator should be added
            break;

        default:
            retVal = 0; // no separator should be added
            break;
    };

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersActionsCreator::menuPosition
// Description: Menu position
//              Each hierarchy on the ,emu include name/priority.
//              If separator is needed after the item then 's' after the priority is needed
//              in case of a sub menu if one item is marked with an 's' it is enough to mark a separator after it:
// Arguments:   int actionIndex
//              positionData - defines the item position within its parent menu
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
gtString gwImagesAndBuffersActionsCreator::menuPosition(int actionIndex, afActionPositionData& positionData)
{
    gtString retVal = GD_STR_ImagesMenuString;

    positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_NONE;

    // Get the action index as command id:
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_NORMAL:
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_PAN:
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMOUT:
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BEST_FIT:
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_RIGHT:
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GREEN_CHANNEL:
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_BLUE_CHANNEL:
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ALPHA_CHANNEL:
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_GRAYSCALE:
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_INVERT:
        {
            retVal = GD_STR_ToolsMenuString;
            retVal.append(AF_Str_MenuSeparator);
            retVal.append(GD_STR_ImagesMenuString);
            positionData.m_beforeActionMenuPosition = GD_STR_ToolsMenuString;
            positionData.m_beforeActionMenuPosition.append(AF_Str_MenuSeparator);
            positionData.m_beforeActionText = AF_STR_ToolsSystemInfo;

        }
        break;

        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ZOOMIN:
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_RED_CHANNEL:
        case ID_IMAGES_AND_BUFFERS_VIEWER_TOOLBAR_ROTATE_LEFT:
        case ID_IMAGES_AND_BUFFERS_VIEWER_ORIGINAL_SIZE:
        {
            retVal = GD_STR_ToolsMenuString;
            retVal.append(AF_Str_MenuSeparator);
            retVal.append(GD_STR_ImagesMenuString);
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
        }
        break;

        default:
            GT_ASSERT_EX(false, L"Unsupported action index");
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersActionsCreator::toolbarPosition
// Description: Toolbar position
// Arguments:   int actionIndex
// Author:      Gilad Yarnitzky
// Date:        18/7/2011
// ---------------------------------------------------------------------------
gtString gwImagesAndBuffersActionsCreator::toolbarPosition(int actionIndex)
{
    gtString retVal;

    // Get the action index as command id:
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
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
        {
            retVal = AF_STR_ImagesAndBuffersToolbar;
        }
        break;

        default:
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwImagesAndBuffersActionsCreator::groupAction
// Description: If the requested action should be a part of a group, create
//              the action group and add the relevant actions to the group
// Arguments:   int actionIndex
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        24/7/2011
// ---------------------------------------------------------------------------
void gwImagesAndBuffersActionsCreator::groupAction(int actionIndex)
{
    (void)(actionIndex); // unused
}
