//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afBrowseAction.cpp
///
//==================================================================================

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationFramework/Include/afBrowseAction.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>

afBrowseAction::afBrowseAction(const QString& actionID) : QAction(nullptr)
{
    // Set the action ID:
    setObjectName(actionID);

    // Initialize the last browsed folder stored in the global variables manager:
    Initilaize();
}

afBrowseAction::afBrowseAction(QAction* pAction) : QAction(pAction)
{
    // Initialize the last browsed folder stored in the global variables manager:
    Initilaize();
}

afBrowseAction::~afBrowseAction()
{
    if (!objectName().isEmpty() && !m_lastBrowsedFolder.isEmpty())
    {
        // Update the object completion list:
        afGlobalVariablesManager::instance().SetHistoryList(objectName(), QStringList(m_lastBrowsedFolder));
    }
}

void afBrowseAction::Initilaize()
{
    // Get the last browse location from manager:
    m_lastBrowsedFolder = afGlobalVariablesManager::instance().GetLastBrowseLocation(objectName());
}

void afBrowseAction::SetLastBrowsedFolder(const QString& lastBrowsedFolder)
{
    m_lastBrowsedFolder = lastBrowsedFolder;

    if (!objectName().isEmpty() && !m_lastBrowsedFolder.isEmpty())
    {
        // Get the last browse location from manager:
        afGlobalVariablesManager::instance().SetLastBrowseLocation(objectName(), m_lastBrowsedFolder);
    }
}
