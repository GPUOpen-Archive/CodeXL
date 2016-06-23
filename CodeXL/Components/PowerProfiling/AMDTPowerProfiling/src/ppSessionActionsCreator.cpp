//=============================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
//
/// \file   ppSessionActionsCreator.cpp
/// \author GPU Developer Tools
/// \version $Revision: $
/// \brief Description:
//
//=============================================================
// $Id: $
// Last checkin:   $DateTime: $
// Last edited by: $Author: $
// Change list:    $Change: $
//=============================================================

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

/// Local:
#include <AMDTPowerProfiling/src/ppSessionActionsCreator.h>



// ---------------------------------------------------------------------------
// Name:        ppSessionActionsCreator::populateSupportedCommandIds
// Description: Create a vector of command Ids that are supported by this actions creator object
// Author:      Roman Bober
// Date:        Jun-23, 2016
// ---------------------------------------------------------------------------
void ppSessionActionsCreator::populateSupportedCommandIds()
{
    // fill the vector of supported command ids:
    m_supportedCommandIds.push_back(ID_COPY);
    m_supportedCommandIds.push_back(ID_SELECT_ALL);
    m_supportedCommandIds.push_back(ID_FIND);
    m_supportedCommandIds.push_back(ID_FIND_NEXT);
    m_supportedCommandIds.push_back(ID_FIND_PREV);
}

gtString ppSessionActionsCreator::menuPosition(int actionIndex, afActionPositionData& positionData)
{
    gtString retVal;

    positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_NONE;

    // Get the command id:
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {
    case ID_COPY:
        retVal = AF_STR_EditMenuString;
        break;

    case ID_SELECT_ALL:
        positionData.m_actionSeparatorType = afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND;
        retVal = AF_STR_EditMenuString;
        break;

    case ID_FIND:
        retVal = AF_STR_EditMenuString;
        break;

    case ID_FIND_NEXT:
        retVal = AF_STR_EditMenuString;
        break;

    case ID_FIND_PREV:
        retVal = AF_STR_EditMenuString;
        break;

    default:
        GT_ASSERT(false);
        break;

    };

    return retVal;
}


gtString ppSessionActionsCreator::toolbarPosition(int actionIndex)
{
    GT_UNREFERENCED_PARAMETER(actionIndex); // unused
    return gtString();
}

void ppSessionActionsCreator::groupAction(int actionIndex)
{
    GT_UNREFERENCED_PARAMETER(actionIndex); // unused
}
