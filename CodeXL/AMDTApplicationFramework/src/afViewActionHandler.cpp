//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afViewActionHandler.cpp
///
//==================================================================================

// System:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afQMdiSubWindow.h>
#include <AMDTApplicationFramework/Include/afViewActionHandler.h>

// ---------------------------------------------------------------------------
// Name:        afViewAction::afViewAction
// Description: Constructor
// Arguments:   QWidget* pControlledWidget - control widget for updates
// Author:      Gilad Yarnitzky
// Date:        20/7/2011
// ---------------------------------------------------------------------------
afViewActionHandler::afViewActionHandler(QWidget* pControlledWidget, QAction* pAction) : _pControlledWidget(pControlledWidget), _pAction(pAction)
{

}


// ---------------------------------------------------------------------------
// Name:        afViewAction::~afViewAction
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        20/7/2011
// ---------------------------------------------------------------------------
afViewActionHandler::~afViewActionHandler(void)
{

}


// ---------------------------------------------------------------------------
// Name:        afViewActionHandler::onViewClicked
// Description: Handle the toggle state
// Author:      Gilad Yarnitzky
// Date:        20/7/2011
// ---------------------------------------------------------------------------
void afViewActionHandler::onViewClicked()
{
    // Sanity check
    GT_IF_WITH_ASSERT(_pAction != nullptr)
    {
        // Toggle the visibility of this view:
        bool shouldShow = _pAction->isChecked();
        controlledWidget()->setVisible(shouldShow);

        if (shouldShow)
        {
            controlledWidget()->setFocus();
            controlledWidget()->raise();
            controlledWidget()->activateWindow();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        afViewActionHandler::onWindowActionClicked
// Description: Handle the window menu item
// Author:      Sigal Algranaty
// Date:        25/8/2011
// ---------------------------------------------------------------------------
void afViewActionHandler::onWindowActionClicked()
{
    // Down cast to sub window:
    afQMdiSubWindow* pSubWindow = qobject_cast<afQMdiSubWindow*>(controlledWidget());

    // Sanity check:
    GT_IF_WITH_ASSERT(pSubWindow != nullptr)
    {
        // Get the instance for the main window:
        afMainAppWindow* pMainWindow = afMainAppWindow::instance();
        GT_IF_WITH_ASSERT(pMainWindow != nullptr)
        {
            // Activate this window:
            pMainWindow->activateSubWindow(pSubWindow);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        afViewActionHandler::onUpdateUI
// Description: Update the enable / checked state of the action
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/9/2011
// ---------------------------------------------------------------------------
void afViewActionHandler::onUpdateUI()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((_pAction != nullptr) && (_pControlledWidget != nullptr))
    {
        _pAction->setChecked(_pControlledWidget->isVisible());
    }
}
