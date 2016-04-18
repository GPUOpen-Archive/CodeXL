//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdAPICallsHistoryPanel.cpp
///
//==================================================================================

//------------------------------ gdAPICallsHistoryPanel.cpp ------------------------------

// Qt
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acFindWidget.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAPICallsHistoryPanel.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdAPICallsHistoryView.h>
#include <AMDTGpuDebuggingComponents/Include/toolbars/gdCallsHistoryToolbar.h>

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryPanel::gdAPICallsHistoryPanel
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        28/2/2011
// ---------------------------------------------------------------------------
gdAPICallsHistoryPanel::gdAPICallsHistoryPanel(afProgressBarWrapper* pProgressBar, QWidget* pParent, bool isGlobal)
    : QWidget(pParent), afBaseView(pProgressBar),
      m_pCallsHistoryView(NULL), m_pToolbar(NULL)
{
    setFrameLayout(pProgressBar, isGlobal);
}


// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryPanel::~gdAPICallsHistoryPanel
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        28/2/2011
// ---------------------------------------------------------------------------
gdAPICallsHistoryPanel::~gdAPICallsHistoryPanel()
{

}


// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryPanel::setLayout
// Description: Build the panel layout with the view and toolbar
// Author:      Gilad Yarnitzky
// Date:        28/2/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryPanel::setFrameLayout(afProgressBarWrapper* pProgressBar, bool isGlobal)
{
    QSizePolicy spolicy;
    spolicy.setVerticalPolicy(QSizePolicy::Ignored);
    spolicy.setHorizontalPolicy(QSizePolicy::Ignored);
    setSizePolicy(spolicy);

    // Create a layout for the whole widget:
    QVBoxLayout* pLayout = new QVBoxLayout;


    // No margins:
    pLayout->setContentsMargins(0, 0, 0, 0);

    // Create the toolbar:
    m_pToolbar = new gdCallsHistoryToolbar(this);


    // Create the calls history view:
    m_pCallsHistoryView = new gdAPICallsHistoryView(pProgressBar, this, isGlobal, true);


    // Add the items to the sizer:
    pLayout->addWidget(m_pToolbar, 0);
    pLayout->addWidget(m_pCallsHistoryView, 1);

    // Activate:
    setLayout(pLayout);
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryPanel::onUpdateEdit_SelectAll
// Description: Overrides afBaseView virtual event
// Arguments:   bool& isEnabled
// Author:      Gilad Yarnitzky
// Date:        1/3/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryPanel::onUpdateEdit_SelectAll(bool& isEnabled)
{
    GT_IF_WITH_ASSERT(m_pCallsHistoryView != NULL)
    {
        m_pCallsHistoryView->onUpdateSelectAll(isEnabled);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryPanel::onUpdateEdit_Find
// Description: Overrides afBaseView virtual event
// Arguments:   bool& isEnabled
// Author:      Sigal Algranaty
// Date:        31/3/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryPanel::onUpdateEdit_Find(bool& isEnabled)
{
    GT_IF_WITH_ASSERT(m_pCallsHistoryView != NULL)
    {
        m_pCallsHistoryView->onUpdateEdit_Find(isEnabled);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryPanel::onUpdateEdit_FindNext
// Description: Overrides afBaseView virtual event
// Arguments:   bool& isEnabled
// Author:      Sigal Algranaty
// Date:        31/3/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryPanel::onUpdateEdit_FindNext(bool& isEnabled)
{
    GT_IF_WITH_ASSERT(m_pCallsHistoryView != NULL)
    {
        m_pCallsHistoryView->onUpdateEdit_FindNext(isEnabled);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryPanel::onEdit_Copy
// Description: Overrides afBaseView virtual event
// Arguments:
// Author:      Gilad Yarnitzky
// Date:        1/3/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryPanel::onEdit_Copy()
{
    GT_IF_WITH_ASSERT(m_pCallsHistoryView != NULL)
    {
        m_pCallsHistoryView->onCopy();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryPanel::onEdit_Find
// Description: Overrides afBaseView virtual event
// Arguments:
// Author:      Sigal Algranaty
// Date:        31/3/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryPanel::onEdit_Find()
{
    GT_IF_WITH_ASSERT(m_pCallsHistoryView != NULL)
    {
        m_pCallsHistoryView->onEdit_Find();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryPanel::onEdit_FindNext
// Description: Overrides afBaseView virtual event
// Arguments:
// Author:      Sigal Algranaty
// Date:        31/3/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryPanel::onEdit_FindNext()
{
    GT_IF_WITH_ASSERT(m_pCallsHistoryView != NULL)
    {
        m_pCallsHistoryView->onEdit_FindNext();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryPanel::onEdit_SelectAll
// Description: Overrides afBaseView virtual event
// Arguments:
// Author:      Gilad Yarnitzky
// Date:        1/3/2011
// ---------------------------------------------------------------------------
void gdAPICallsHistoryPanel::onEdit_SelectAll()
{
    GT_IF_WITH_ASSERT(m_pCallsHistoryView != NULL)
    {
        m_pCallsHistoryView->onSelectAll();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryPanel::onFindClick
// Description: Implement the find click button
// Author:      Sigal Algranaty
// Date:        19/2/2012
// ---------------------------------------------------------------------------
void gdAPICallsHistoryPanel::onFindClick()
{

    if (!acFindParameters::Instance().m_findExpr.isEmpty())
    {
        // Define the Qt find flags matching the user selection:
        Qt::MatchFlags findFlags = Qt::MatchContains;

        if (acFindParameters::Instance().m_isCaseSensitive)
        {
            findFlags = findFlags | Qt::MatchCaseSensitive;
        }

        // Sanity check
        GT_IF_WITH_ASSERT(m_pCallsHistoryView != NULL)
        {
            // Call the find click method:
            m_pCallsHistoryView->onFindClick();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdAPICallsHistoryPanel::onFindNext
// Description: Implement the find next slot
// Author:      Sigal Algranaty
// Date:        19/2/2012
// ---------------------------------------------------------------------------
void gdAPICallsHistoryPanel::onFindNext()
{
    // Sanity check
    GT_IF_WITH_ASSERT(m_pCallsHistoryView != NULL)
    {
        // Call the find click method:
        m_pCallsHistoryView->onFindNext();
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        updateToolbarCommands
/// \brief Description: Update the toolbar commands
/// \return True :
/// \return False:
/// -----------------------------------------------------------------------------------------------
void gdAPICallsHistoryPanel::updateToolbarCommands()
{
    GT_IF_WITH_ASSERT(m_pToolbar != NULL)
    {
        m_pToolbar->onUpdateToolbar();
    }
}

void gdAPICallsHistoryPanel::onFindPrev()
{
    // m_isSearchUp flag is up only when find prev button is pressed
    acFindParameters::Instance().m_isSearchUp = true;
    onFindNext();
    acFindParameters::Instance().m_isSearchUp = false;
}

