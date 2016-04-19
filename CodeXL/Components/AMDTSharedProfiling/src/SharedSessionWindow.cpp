//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SharedSessionWindow.cpp
///
//==================================================================================

#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTApplicationComponents/Include/acFindParameters.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

// Local:
#include <inc/SharedSessionWindow.h>
#include <inc/ProfileApplicationTreeHandler.h>

QMap<QWidget*, bool> SharedSessionWindow::m_sessionsMap;

SharedSessionWindow::SharedSessionWindow(QWidget* pParent) : QWidget(pParent)
{
    m_sessionsMap[this] = true;
}

SharedSessionWindow::~SharedSessionWindow()
{
    m_sessionsMap.remove(this);
}

void SharedSessionWindow::onUpdateEdit_Copy(bool& isEnabled)
{
    isEnabled = false;
}

void SharedSessionWindow::onUpdateEdit_SelectAll(bool& isEnabled)
{
    isEnabled = false;
}

void SharedSessionWindow::onUpdateEdit_Find(bool& isEnabled)
{
    isEnabled = false;
}

void SharedSessionWindow::onUpdateEdit_FindNext(bool& isEnabled)
{
    isEnabled = false;
}

void SharedSessionWindow::OnEditCopy()
{
    // Do nothing (should be implemented in child classes):
}

void SharedSessionWindow::OnEditSelectAll()
{
    // Do nothing (should be implemented in child classes):
}

void SharedSessionWindow::onFindClick()
{
    // Do nothing (should be implemented in child classes):
}

void SharedSessionWindow::onFindNext()
{
    // Do nothing (should be implemented in child classes):
}


void SharedSessionWindow::onFindPrev()
{
    // m_isSearchUp flag is up only when find prev button is pressed
    afMainAppWindow* pMainAppWindow = afMainAppWindow::instance();

    GT_IF_WITH_ASSERT(NULL != pMainAppWindow)
    {
        // in order to search up, search up flag must be true assume it wasn't set prior:
        acFindParameters::Instance().m_isSearchUp = true;

        onFindNext();

        // reset search up flag after completing the search
        acFindParameters::Instance().m_isSearchUp = false;
    }
}

bool SharedSessionWindow::IsSessionExistInMap(QWidget* wid)
{
    return m_sessionsMap.contains(wid);
}

bool SharedSessionWindow::DisplaySession(const osFilePath& sessionFilePath, afTreeItemType sessionInnerPage, QString& errorMessage)
{
    GT_UNREFERENCED_PARAMETER(sessionInnerPage);
    GT_UNREFERENCED_PARAMETER(errorMessage);

    bool retVal = false;

    // Set the session file path
    m_sessionFilePath = sessionFilePath;

    // Find the session item data
    m_pSessionData = ProfileApplicationTreeHandler::instance()->FindSessionDataByProfileFilePath(m_sessionFilePath);

    if (m_pSessionData != nullptr)
    {
        retVal = true;
    }

    return retVal;
}

void SharedSessionWindow::UpdateRenamedSession(const osFilePath& oldSessionFileName, const osFilePath& newSessionFileName)
{
    GT_UNREFERENCED_PARAMETER(oldSessionFileName);

    if (m_pSessionData != nullptr)
    {
        m_sessionFilePath = newSessionFileName;
    }
}
