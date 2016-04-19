//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwWXViewsActionsCreator.cpp
///
//==================================================================================

//------------------------------ gwWXViewsActionsCreator.h ------------------------------

// QT:
#include <QAction>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>

// Local:
#include <src/gwWXViewsActionsCreator.h>


// ---------------------------------------------------------------------------
// Name:        gwWXViewsActionsCreator::gwWXViewsActionsCreator
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        5/9/2011
// ---------------------------------------------------------------------------
gwWXViewsActionsCreator::gwWXViewsActionsCreator()
{

    // Add the edit action commands:
    _supportedCommandIds.push_back(ID_COPY);
    _supportedCommandIds.push_back(ID_FIND);
    _supportedCommandIds.push_back(ID_FIND_NEXT);
    _supportedCommandIds.push_back(ID_SELECT_ALL);
}

// ---------------------------------------------------------------------------
// Name:        gwWXViewsActionsCreator::~gwWXViewsActionsCreator
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        5/9/2011
// ---------------------------------------------------------------------------
gwWXViewsActionsCreator::~gwWXViewsActionsCreator()
{
}

// ---------------------------------------------------------------------------
// Name:        gwWXViewsActionsCreator::caption
// Description: Get the caption of the action item
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        5/9/2011
// ---------------------------------------------------------------------------
bool gwWXViewsActionsCreator::actionText(int actionIndex, gtString& caption, gtString& tooltip, gtString& keyboardShortcut)
{
    bool retVal = true;

    // Sanity check:
    GT_IF_WITH_ASSERT((actionIndex >= 0) && (actionIndex < (int)_supportedCommandIds.size()))
    {
        // Get the command id:
        int commandId = _supportedCommandIds[actionIndex];

        switch (commandId)
        {

            case ID_COPY:
                caption = AF_STR_Copy;
                tooltip = AF_STR_CopyStatusbarString;
                keyboardShortcut = AF_STR_keyboardShortcutCopyMenu;
                break;

            case ID_FIND:
                caption = AF_STR_Find;
                tooltip = AF_STR_FindStatusbarString;
                keyboardShortcut = AF_STR_keyboardShortcutFindMenu;
                break;

            case ID_FIND_NEXT:
                caption = AF_STR_FindNext;
                tooltip = AF_STR_FindNextStatusbarString;
                keyboardShortcut = AF_STR_keyboardShortcutFindNextMenu;
                break;

            case ID_SELECT_ALL:
                caption = AF_STR_SelectAll;
                tooltip = AF_STR_SelectAllStatusbarString;
                keyboardShortcut = AF_STR_keyboardShortcutSelectAllMenu;
                break;

            default:
                GT_ASSERT(false);
                retVal = false;
                break;

        };
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gwWXViewsActionsCreator::menuPosition
// Description: Menu position
//              Each hierarchy on the ,emu include name/priority.
//              If separator is needed after the item then 's' after the priority is needed
//              in case of a sub menu if one item is marked with an 's' it is enough to mark a separator after it:
// Arguments:   int actionIndex
//              positionData - defines the item position within its parent menu
// Author:      Sigal Algranaty
// Date:        5/9/2011
// ---------------------------------------------------------------------------
gtString gwWXViewsActionsCreator::menuPosition(int actionIndex, afActionPositionData& positionData)
{
    (void)(positionData); // unused
    gtString retVal;

    // Sanity check:
    GT_IF_WITH_ASSERT((actionIndex >= 0) && (actionIndex < (int)_supportedCommandIds.size()))
    {
        // Get the command id:
        int commandId = _supportedCommandIds[actionIndex];

        switch (commandId)
        {
            case ID_COPY:
            case ID_FIND:
            case ID_FIND_NEXT:
            case ID_SELECT_ALL:
                retVal = AF_STR_EditMenuString;
                break;

            default:
                GT_ASSERT(false);
                break;
        };
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwWXViewsActionsCreator::toolbarPosition
// Description: Toolbar position
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        5/9/2011
// ---------------------------------------------------------------------------
gtString gwWXViewsActionsCreator::toolbarPosition(int actionIndex)
{
    (void)(actionIndex); // unused
    gtString retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwWXViewsActionsCreator::numberActions
// Description: Get number of actions created
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        5/9/2011
// ---------------------------------------------------------------------------
int gwWXViewsActionsCreator::numberActions()
{
    int retVal = (int)_supportedCommandIds.size();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gwWXViewsActionsCreator::groupAction
// Description: Group actions if needed
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        5/9/2011
// ---------------------------------------------------------------------------
void gwWXViewsActionsCreator::groupAction(int actionIndex)
{
    (void)(actionIndex); // unused
}
