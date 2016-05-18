//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afGeneralActionsCreator.cpp
///
//==================================================================================

// QT:
#include <QAction>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afGeneralActionsCreator.h>


// ---------------------------------------------------------------------------
// Name:        afGeneralActionsCreator::afGeneralActionsCreator
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        16/9/2011
// ---------------------------------------------------------------------------
afGeneralActionsCreator::afGeneralActionsCreator()
{
}

// ---------------------------------------------------------------------------
// Name:        afGeneralActionsCreator::~afGeneralActionsCreator
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        16/9/2011
// ---------------------------------------------------------------------------
afGeneralActionsCreator::~afGeneralActionsCreator()
{
}

// ---------------------------------------------------------------------------
// Name:        afGeneralActionsCreator::populateSupportedCommandIds
// Description: Create a vector of command Ids that are supported by this actions creator object
// Author:      Doron Ofek
// Date:        Mar-3, 2015
// ---------------------------------------------------------------------------
void afGeneralActionsCreator::populateSupportedCommandIds()
{
    // fill the vector of supported command ids:
    m_supportedCommandIds.push_back(ID_CUT);
    m_supportedCommandIds.push_back(ID_COPY);
    m_supportedCommandIds.push_back(ID_PASTE);
    m_supportedCommandIds.push_back(ID_FIND);
    m_supportedCommandIds.push_back(ID_FIND_NEXT);
    m_supportedCommandIds.push_back(ID_FIND_PREV);
    m_supportedCommandIds.push_back(ID_SELECT_ALL);
    m_supportedCommandIds.push_back(AF_ID_SAVE_FILE);
    m_supportedCommandIds.push_back(AF_ID_SAVE_FILE_AS);
    m_supportedCommandIds.push_back(ID_GO_TO);
}

// ---------------------------------------------------------------------------
// Name:        afGeneralActionsCreator::menuPosition
// Description: Menu position
//              Each hierarchy on the ,emu include name/priority.
//              If separator is needed after the item then 's' after the priority is needed
//              in case of a sub menu if one item is marked with an 's' it is enough to mark a separator after it:
// Arguments:   int actionIndex
//              positionData - defines the item position within its parent menu
// Author:      Sigal Algranaty
// Date:        16/9/2011
// ---------------------------------------------------------------------------
gtString afGeneralActionsCreator::menuPosition(int actionIndex, afActionPositionData& positionData)
{
    gtString retVal;

    positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_NONE;

    // Get the command id:
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {

        case ID_CUT:
        case ID_COPY:
        case ID_PASTE:
        case ID_FIND_NEXT:
        case ID_FIND_PREV:
        {
            retVal = AF_STR_EditMenuString;
        }
        break;

        case ID_FIND:
        case ID_SELECT_ALL:
        case ID_GO_TO:
        {
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
            retVal = AF_STR_EditMenuString;
        }
        break;

        case AF_ID_SAVE_FILE:
        case AF_ID_SAVE_FILE_AS:
        {
            retVal = AF_STR_FileMenuString;
            positionData.m_beforeActionMenuPosition = AF_STR_FileMenuString;
            positionData.m_beforeActionText = AF_STR_ProjectSettings;
        }
        break;

        default:
            GT_ASSERT(false);
            break;
    };

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afGeneralActionsCreator::toolbarPosition
// Description: Toolbar position
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        16/9/2011
// ---------------------------------------------------------------------------
gtString afGeneralActionsCreator::toolbarPosition(int actionIndex)
{
    GT_UNREFERENCED_PARAMETER(actionIndex);

    gtString retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afGeneralActionsCreator::groupAction
// Description: Group actions if needed
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        16/9/2011
// ---------------------------------------------------------------------------
void afGeneralActionsCreator::groupAction(int actionIndex)
{
    GT_UNREFERENCED_PARAMETER(actionIndex);
}
