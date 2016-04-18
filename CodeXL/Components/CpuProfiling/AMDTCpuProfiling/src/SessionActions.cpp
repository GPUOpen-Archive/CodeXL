//=============================================================
// (c) 2012 Advanced Micro Devices, Inc.
//
/// \author franksw
/// \version $Revision: #12 $
/// \brief  A brief file description that Doxygen makes note of.
//
//=============================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/src/SessionActions.cpp#12 $
// Last checkin:   $DateTime: 2016/02/15 03:06:55 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 559604 $
//=============================================================

// QT:
#include <QtCore>
#include <QtWidgets>
#include <QAction>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

//local
#include <inc/SessionActions.h>
#include <inc/CommandIds.h>
#include <inc/CommandsHandler.h>
#include <inc/StringConstants.h>

SessionActions::SessionActions() : afActionCreatorAbstract(), m_pCommandsHandler(nullptr)
{
    m_pCommandsHandler = CommandsHandler::instance();
}

SessionActions::~SessionActions()
{
}

// ---------------------------------------------------------------------------
// Name:        SessionActions::populateSupportedCommandIds
// Description: Create a vector of command Ids that are supported by this actions creator object
// Author:  AMD Developer Tools Team
// Date:        Mar-3, 2015
// ---------------------------------------------------------------------------
void SessionActions::populateSupportedCommandIds()
{
    // fill the vector of supported command ids:
    m_supportedCommandIds.push_back(ID_COPY);
    m_supportedCommandIds.push_back(ID_FIND);
    m_supportedCommandIds.push_back(ID_FIND_NEXT);
    m_supportedCommandIds.push_back(ID_SELECT_ALL);
}

gtString SessionActions::menuPosition(int actionIndex, afActionPositionData& positionData)
{
    gtString retVal;

    positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_NONE;

    // Get the command id:
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {
        case ID_COPY:
        case ID_FIND_NEXT:
            retVal = AF_STR_EditMenuString;
            break;

        case ID_FIND:
        case ID_SELECT_ALL:
            positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
            retVal = AF_STR_EditMenuString;
            break;

        default:
            GT_ASSERT(false);
            break;
    };

    return retVal;
}


gtString SessionActions::toolbarPosition(int actionIndex)
{
    (void)(actionIndex); // unused
    gtString retVal;

    return retVal;
}

void SessionActions::groupAction(int actionIndex)
{
    (void)(actionIndex); // unused
}
