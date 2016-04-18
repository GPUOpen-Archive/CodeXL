//=============================================================
// Copyright (c) 2013 Advanced Micro Devices, Inc.
//
/// \file   GpuSessionActionsCreator.cpp
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
// QT:
#include <QtCore>
#include <QtWidgets>
#include <QAction>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

/// Local:
#include <AMDTGpuProfiling/GpuSessionActionsCreator.h>
#include <AMDTGpuProfiling/gpViewsCreator.h>



GpuSessionActionsCreator::GpuSessionActionsCreator() : afActionCreatorAbstract()
{
}

GpuSessionActionsCreator::~GpuSessionActionsCreator()
{
}

// ---------------------------------------------------------------------------
// Name:        GpuSessionActionsCreator::populateSupportedCommandIds
// Description: Create a vector of command Ids that are supported by this actions creator object
// Author:      Doron Ofek
// Date:        Mar-3, 2015
// ---------------------------------------------------------------------------
void GpuSessionActionsCreator::populateSupportedCommandIds()
{
    // fill the vector of supported command ids:
    m_supportedCommandIds.push_back(ID_COPY);
    m_supportedCommandIds.push_back(ID_SELECT_ALL);
    m_supportedCommandIds.push_back(ID_FIND);
    m_supportedCommandIds.push_back(ID_FIND_NEXT);
    m_supportedCommandIds.push_back(ID_FIND_PREV);
}

gtString GpuSessionActionsCreator::menuPosition(int actionIndex, afActionPositionData& positionData)
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


gtString GpuSessionActionsCreator::toolbarPosition(int actionIndex)
{
    (void)(actionIndex); // unused
    gtString retVal;

    return retVal;
}

void GpuSessionActionsCreator::groupAction(int actionIndex)
{
    (void)(actionIndex); // unused
}
