//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afActionCreatorAbstract.cpp
///
//==================================================================================

// System:
#include <QAction>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationFramework/Include/afActionCreatorAbstract.h>
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        afActionCreatorAbstract::afActionCreatorAbstract
// Description: Creator
// Author:      Gilad Yarnitzky
// Date:        14/7/2011
// ---------------------------------------------------------------------------
afActionCreatorAbstract::afActionCreatorAbstract(void)
{
}

// ---------------------------------------------------------------------------
// Name:        afActionCreatorAbstract::~afActionCreatorAbstract
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        14/7/2011
// ---------------------------------------------------------------------------
afActionCreatorAbstract::~afActionCreatorAbstract(void)
{
    // Move the items above the removed item one place back:
    // Initialize the views:
    for (int action = 0 ; action < (int)m_actionsCreated.size() - 1 ; action++)
    {
        if (m_actionsCreated[action] != nullptr)
        {
            delete m_actionsCreated[action];
            m_actionsCreated[action] = nullptr;
        }
    }

    m_actionsCreated.clear();

    for (int i = 0 ; i < (int)m_iconsDataVector.size(); i++)
    {
        if (m_iconsDataVector[i]._pCommandPixmap != nullptr)
        {
            delete m_iconsDataVector[i]._pCommandPixmap;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afActionCreatorAbstract::initializeCreator
// Description: Initialize the creator
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void afActionCreatorAbstract::initializeCreator()
{
    // Create a vector of command Ids that are supported by this actions creator object
    populateSupportedCommandIds();

    // Resize the vector to max possible views
    m_actionsCreated.resize(numberActions());

    // Resize the vector of the action icon data:
    m_iconsDataVector.resize(numberActions());

    // Initialize action data:
    for (int i = 0 ; i < (int)m_actionsCreated.size(); i++)
    {
        m_actionsCreated[i] = nullptr;
    }

    // Initialize the actions icons:
    initActionIcons();

}

// ---------------------------------------------------------------------------
// Name:        afActionCreatorAbstract::action
// Description: Get the specific action
// Arguments:   int actionLocalIndex
// Return Val:  QAction*
// Author:      Gilad Yarnitzky
// Date:        17/7/2011
// ---------------------------------------------------------------------------
QAction* afActionCreatorAbstract::action(int actionLocalIndex)
{
    QAction* pReturnedAction = nullptr;

    GT_IF_WITH_ASSERT(actionLocalIndex >= 0 && actionLocalIndex < numberActions())
    {
        pReturnedAction = m_actionsCreated[actionLocalIndex];
    }

    return pReturnedAction;
}

// ---------------------------------------------------------------------------
// Name:        afActionCreatorAbstract::setAction
// Description: Set action of a specific index
// Arguments:   QAction* pAction
//              int actionLocalIndex
//              actionGlobalIndex - the global index for the action
// Author:      Gilad Yarnitzky
// Date:        17/7/2011
// ---------------------------------------------------------------------------
void afActionCreatorAbstract::setAction(QAction* pAction, int actionLocalIndex, int actionGlobalIndex)
{
    GT_IF_WITH_ASSERT(actionLocalIndex >= 0 && actionLocalIndex < numberActions())
    {
        m_actionsCreated[actionLocalIndex] = pAction;

        // Set the global to local mapping:
        m_globalIndexToLocalIndex[actionGlobalIndex] = actionLocalIndex;
    }
}

// ---------------------------------------------------------------------------
// Name:        afActionCreatorAbstract::afCommandIconData::afCommandIconData
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        14/8/2011
// ---------------------------------------------------------------------------
afActionCreatorAbstract::afCommandIconData::afCommandIconData()
    : _pCommandPixmap(nullptr), _isPixmapInitialized(false), _shouldDisplayInMenu(false)
{
}

// ---------------------------------------------------------------------------
// Name:        afActionCreatorAbstract::iconAsPixmap
// Description: Get the icon file name
// Arguments:   int actionLocalIndex
// Author:      Sigal Algranaty
// Date:        14/8/2011
// ---------------------------------------------------------------------------
QPixmap* afActionCreatorAbstract::iconAsPixmap(int actionLocalIndex, bool& shouldUseInMenu)
{
    QPixmap* pRetVal = nullptr;
    shouldUseInMenu = false;

    GT_IF_WITH_ASSERT((actionLocalIndex < (int)m_iconsDataVector.size()) && (actionLocalIndex >= 0))
    {
        // If the icon is initialized:
        if (m_iconsDataVector[actionLocalIndex]._isPixmapInitialized)
        {
            pRetVal = m_iconsDataVector[actionLocalIndex]._pCommandPixmap;
            shouldUseInMenu = m_iconsDataVector[actionLocalIndex]._shouldDisplayInMenu;
        }
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        afActionCreatorAbstract::initActionIcons
// Description: Default implementation
// Author:      Sigal Algranaty
// Date:        14/8/2011
// ---------------------------------------------------------------------------
void afActionCreatorAbstract::initActionIcons()
{

}

// ---------------------------------------------------------------------------
// Name:        afActionCreatorAbstract::commandIdToActionIndex
// Description: Convert a command id to the corresponding local action index
// Author:      Doron Ofek
// Date:        Mar-10, 2015
// ---------------------------------------------------------------------------
bool afActionCreatorAbstract::commandIdToActionIndex(const int commandId, int& actionIndex) const
{
    bool retval = false;
    actionIndex = -1;
    int numOfSupportedCommands = static_cast<int>(m_supportedCommandIds.size());

    // Look for the parameter commandId in the vector of supported command ids
    for (int index = 0; index < numOfSupportedCommands; ++index)
    {
        if (m_supportedCommandIds[index] == commandId)
        {
            // found the corresponding actionIndex
            actionIndex = index;
            retval = true;
            break;
        }
    }

    return retval;
}

// ---------------------------------------------------------------------------
// Name:        afActionCreatorAbstract::initSingleActionIcon
// Description: Initializes a single action icon
// Arguments:   commandId - the command id
//              xpm[] - the action xpm string
//              displayInMenu - should display the icon in menu
// Author:      Sigal Algranaty
// Date:        16/8/2011
// ---------------------------------------------------------------------------
void afActionCreatorAbstract::initSingleActionIcon(int commandId, acIconId iconId, bool displayInMenu)
{
    int actionLocalIndex = 0;
    GT_IF_WITH_ASSERT(commandIdToActionIndex(commandId, actionLocalIndex))
    {
        // Sanity check:
        GT_IF_WITH_ASSERT((actionLocalIndex < (int)m_iconsDataVector.size()) && (actionLocalIndex >= 0))
        {
            GT_IF_WITH_ASSERT(AC_NUMBER_OF_ICON_IDS > iconId)
            {
                QPixmap* pPixmap = new QPixmap;
                acSetIconInPixmap(*pPixmap, iconId);
                m_iconsDataVector[actionLocalIndex]._pCommandPixmap = pPixmap;

                m_iconsDataVector[actionLocalIndex]._shouldDisplayInMenu = displayInMenu;
                m_iconsDataVector[actionLocalIndex]._isPixmapInitialized = true;
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        afActionCreatorAbstract::localFromGlobalActionIndex
// Description: Return a local index from the global index
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        5/9/2011
// ---------------------------------------------------------------------------
int afActionCreatorAbstract::localFromGlobalActionIndex(int actionGlobalIndex) const
{
    int retVal = -1;

    // Find the index within the map:
    gtMap<int, int>::const_iterator iter = m_globalIndexToLocalIndex.find(actionGlobalIndex);

    if (iter != m_globalIndexToLocalIndex.end())
    {
        retVal = (*iter).second;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afActionCreatorAbstract::actionIndexToCommandId
// Description: Return a the command Id stored at the index position in the edit
//              commands vector
// Return Val:  int
// Author:      Doron Ofek
// Date:        Mar-3, 2015
// ---------------------------------------------------------------------------
int afActionCreatorAbstract::actionIndexToCommandId(const int actionIndex) const
{
    // If the index is out of bounds it is best to return an obviously invalid command ID that
    // will be handled by the 'default' block of the switch cases that handle command Ids.
    int commandId = 0xFFFF;

    GT_IF_WITH_ASSERT((actionIndex >= 0) && (actionIndex < (int)m_supportedCommandIds.size()))
    {
        // Get the command id:
        commandId = m_supportedCommandIds[actionIndex];
    }

    return commandId;
}

// ---------------------------------------------------------------------------
// Name:        afActionCreatorAbstract::caption
// Description: Get the caption of the action item. The common edit actions that
//              are shared by most action creators are covered by this function.
//              A derived class should over-ride this function if it implements
//              actions that do not appear in this list.
// Arguments:   [IN] int actionIndex
//              [OUT] gtString& caption
//              [OUT] gtString& tooltip
//              [OUT] gtString& keyboardShortcut
// Author:      Doron Ofek
// Date:        Mar-3, 2015
// ---------------------------------------------------------------------------
bool afActionCreatorAbstract::actionText(int actionIndex, gtString& caption, gtString& tooltip, gtString& keyboardShortcut)
{
    bool retVal = true;


    // Get the command id:
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {

        case ID_CUT:
            caption = AF_STR_Cut;
            tooltip = AF_STR_CutStatusbarString;
            keyboardShortcut = AF_STR_keyboardShortcutCutMenu;
            break;

        case ID_COPY:
            caption = AF_STR_Copy;
            tooltip = AF_STR_CopyStatusbarString;
            keyboardShortcut = AF_STR_keyboardShortcutCopyMenu;
            break;

        case ID_PASTE:
            caption = AF_STR_Paste;
            tooltip = AF_STR_PasteStatusbarString;
            keyboardShortcut = AF_STR_keyboardShortcutPasteMenu;
            break;

        case ID_SELECT_ALL:
            caption = AF_STR_SelectAll;
            tooltip = AF_STR_SelectAllStatusbarString;
            keyboardShortcut = AF_STR_keyboardShortcutSelectAllMenu;
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

        case ID_FIND_PREV:
            caption = AF_STR_FindPrev;
            tooltip = AF_STR_FindPrevStatusbarString;
            keyboardShortcut = AF_STR_keyboardShortcutFindPrevMenu;
            break;

        case ID_SHOW_LINE_NUMBERS:
            caption = AF_STR_ShowLineNumbers;
            tooltip = AF_STR_ShowLineNumbersStatusbarString;
            break;

        case AF_ID_SAVE_FILE:
            caption = AF_STR_saveFile;
            tooltip = AF_STR_saveFileStatusbarString;
            keyboardShortcut = AF_STR_keyboardShortcutSaveMenu;
            break;

        case AF_ID_SAVE_FILE_AS:
            caption = AF_STR_saveFileAs;
            tooltip = AF_STR_saveFileAsStatusbarString;
            break;

            case ID_GO_TO:
            caption = AF_STR_GoTo;
            tooltip = AF_STR_GoToStatusbarString;
            keyboardShortcut = AF_STR_keyboardShortcutGoToMenu;
            break;

        default:
            GT_ASSERT(false);
            retVal = false;
            break;

    };

    return retVal;
}

