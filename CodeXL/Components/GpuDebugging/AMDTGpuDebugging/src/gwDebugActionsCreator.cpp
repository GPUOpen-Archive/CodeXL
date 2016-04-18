//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwDebugActionsCreator.cpp
///
//==================================================================================

//------------------------------ gwDebugActionsCreator.h ------------------------------

// QT:
#include <QAction>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afCommandIds.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>

// Local:
#include <src/gwDebugActionsCreator.h>


// ---------------------------------------------------------------------------
// Name:        gwDebugActionsCreator::gwDebugActionsCreator
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        16/9/2011
// ---------------------------------------------------------------------------
gwDebugActionsCreator::gwDebugActionsCreator()
{
}

// ---------------------------------------------------------------------------
// Name:        gwDebugActionsCreator::~gwDebugActionsCreator
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        16/9/2011
// ---------------------------------------------------------------------------
gwDebugActionsCreator::~gwDebugActionsCreator()
{
}

// ---------------------------------------------------------------------------
// Name:        gwDebugActionsCreator::populateSupportedCommandIds
// Description: Create a vector of command Ids that are supported by this actions creator object
// Author:      Doron Ofek
// Date:        Mar-3, 2015
// ---------------------------------------------------------------------------
void gwDebugActionsCreator::populateSupportedCommandIds()
{
    // fill the vector of supported command ids:
    m_supportedCommandIds.push_back(ID_COPY);
    m_supportedCommandIds.push_back(ID_FIND);
    m_supportedCommandIds.push_back(ID_FIND_NEXT);
    m_supportedCommandIds.push_back(ID_SELECT_ALL);
}

// ---------------------------------------------------------------------------
// Name:        gwDebugActionsCreator::menuPosition
// Description: Menu position
//              Each hierarchy on the ,emu include name/priority.
//              If separator is needed after the item then 's' after the priority is needed
//              in case of a sub menu if one item is marked with an 's' it is enough to mark a separator after it:
// Arguments:   int actionIndex
//              positionData - defines the item position within its parent menu
// Author:      Sigal Algranaty
// Date:        16/9/2011
// ---------------------------------------------------------------------------
gtString gwDebugActionsCreator::menuPosition(int actionIndex, afActionPositionData& positionData)
{
    gtString retVal;

    positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_NONE;

    // Get the command id:
    int commandId = actionIndexToCommandId(actionIndex);

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

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwDebugActionsCreator::toolbarPosition
// Description: Toolbar position
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        16/9/2011
// ---------------------------------------------------------------------------
gtString gwDebugActionsCreator::toolbarPosition(int actionIndex)
{
    (void)(actionIndex); // unused
    gtString retVal;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwDebugActionsCreator::groupAction
// Description: Group actions if needed
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        16/9/2011
// ---------------------------------------------------------------------------
void gwDebugActionsCreator::groupAction(int actionIndex)
{
    (void)(actionIndex); // unused

}
