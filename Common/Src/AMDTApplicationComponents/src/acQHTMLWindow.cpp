//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acQHTMLWindow.cpp
///
//==================================================================================

//------------------------------ acQHTMLWindow.cpp ------------------------------

// Qt:
#include <QMenu>
#include <QKeyEvent>
#include <QTextBrowser>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
// Local:
#include <AMDTApplicationComponents/Include/acQHTMLWindow.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <inc/acStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        acQHTMLWindow::acQHTMLWindow
// Description: Constructor
// Arguments:   QWidget* pParent
// Author:      Sigal Algranaty
// Date:        29/11/2011
// ---------------------------------------------------------------------------
acQHTMLWindow::acQHTMLWindow(QWidget* pParent)
    : QTextBrowser(pParent)
{
    // Context menu:
    setContextMenuPolicy(Qt::CustomContextMenu);
    bool rc = connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onContextMenuEvent(const QPoint&)));
    GT_ASSERT(rc);

    rc = connect(this, SIGNAL(anchorClicked(const QUrl&)), this, SLOT(onLinkClicked(const QUrl&)));
    GT_ASSERT(rc);

    // Allow focus in this widget:
    setFocusPolicy(Qt::ClickFocus);

    // Initialize the context menu:
    initContextMenu();
}

// ---------------------------------------------------------------------------
// Name:        acQHTMLWindow::initContextMenu
// Description: Initialize the context menu
// Author:      Sigal Algranaty
// Date:        29/11/2011
// ---------------------------------------------------------------------------
void acQHTMLWindow::initContextMenu()
{
    // Allocate a menu:
    _pContextMenu = new QMenu(this);


    // Create copy action:
    _pCopyAction = new QAction(AC_STR_listCtrlCopy, this);


    // Connect the action to delete slot:
    bool rcConnect = connect(_pCopyAction, SIGNAL(triggered()), this, SLOT(onEditCopy()));
    GT_ASSERT(rcConnect);

    // Create select all action:
    _pSelectAllAction = new QAction(AC_STR_listCtrlSelectAll, this);


    // Connect the action to delete slot:
    rcConnect = connect(_pSelectAllAction, SIGNAL(triggered()), this, SLOT(onEditSelectAll()));
    GT_ASSERT(rcConnect);

    // Connect the menu to its slots:
    rcConnect = connect(_pContextMenu, SIGNAL(aboutToShow()), this, SLOT(onAboutToShowContextMenu()));
    GT_ASSERT(rcConnect);

    // Add the actions:
    _pContextMenu->addAction(_pCopyAction);
    _pContextMenu->addAction(_pSelectAllAction);
}

// ---------------------------------------------------------------------------
// Name:        acQHTMLWindow::onEditCopy
// Description: Is handling the copy command
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        29/11/2011
// ---------------------------------------------------------------------------
void acQHTMLWindow::onEditCopy()
{
    copy();
}

// ---------------------------------------------------------------------------
// Name:        acQHTMLWindow::onUpdateEditCopy
// Description: Is handling the update copy command
// Author:      Sigal Algranaty
// Date:        29/11/2011
// ---------------------------------------------------------------------------
void acQHTMLWindow::onUpdateEditCopy(bool& isEnabled)
{
    // Enable only if text is selected:
    isEnabled = textCursor().hasSelection();
}

// ---------------------------------------------------------------------------
// Name:        acQHTMLWindow::OnLinkClicked
// Description: Is called when a link is clicked inside the HTML control.
// Arguments:   clickedLink - A class that contains the clicked link info.
// Author:      Yaki Tebeka
// Date:        3/1/2005
// ---------------------------------------------------------------------------
void acQHTMLWindow::onLinkClicked(const QUrl& link)
{
    GT_UNREFERENCED_PARAMETER(link);
}


// ---------------------------------------------------------------------------
// Name:        acQHTMLWindow::onContextMenuEvent
// Description: Is connected to the context menu request signal - display the
//              context menu if it is initialized
// Arguments:   const QPoint &
// Author:      Sigal Algranaty
// Date:        29/11/2011
// ---------------------------------------------------------------------------
void acQHTMLWindow::onContextMenuEvent(const QPoint& position)
{
    if (_pContextMenu != NULL)
    {
        _pContextMenu->exec(acMapToGlobal(this, position));
    }
}


// ---------------------------------------------------------------------------
// Name:        acQHTMLWindow::onEditSelectAll
// Description: Select all the text
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        29/11/2011
// ---------------------------------------------------------------------------
void acQHTMLWindow::onEditSelectAll()
{
    // Select the text:
    selectAll();
}

// ---------------------------------------------------------------------------
// Name:        acListCtrl::onUpdateEditSelectAll
// Description:  Check if select all command should be enabled
// Arguments:   bool& isEnabled
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        29/11/2011
// ---------------------------------------------------------------------------
void acQHTMLWindow::onUpdateEditSelectAll(bool& isEnabled)
{
    isEnabled = true;
}


// ---------------------------------------------------------------------------
// Name:        acQHTMLWindow::onAboutToShowContextMenu
// Description: Update actions visibility and enable state
// Author:      Sigal Algranaty
// Date:        29/11/2011
// ---------------------------------------------------------------------------
void acQHTMLWindow::onAboutToShowContextMenu()
{
    // Sanity check:
    bool isEnabled = false;
    GT_IF_WITH_ASSERT((_pCopyAction != NULL) && (_pSelectAllAction != NULL))
    {
        // Set the copy action enable state:
        onUpdateEditCopy(isEnabled);
        _pCopyAction->setEnabled(isEnabled);

        onUpdateEditSelectAll(isEnabled);
        _pSelectAllAction->setEnabled(isEnabled);
    }
}


// ---------------------------------------------------------------------------
// Name:        acQHTMLWindow::keyPressEvent
// Description: Overriding QTextBrowser key down event
// Arguments:   QKeyEvent * pKeyEvent
// Author:      Sigal Algranaty
// Date:        29/11/2011
// ---------------------------------------------------------------------------
void acQHTMLWindow::keyPressEvent(QKeyEvent* pKeyEvent)
{
    // Sanity check
    GT_IF_WITH_ASSERT(pKeyEvent != NULL)
    {
        int keyCode = pKeyEvent->key();

        // See comments in gdaGDebuggerFrame::onKeyDown
        switch (keyCode)
        {
            case 'a':
            case 'A':
            {
                // check if the Ctrl key is pressed
                if (pKeyEvent->modifiers() == Qt::ControlModifier)
                {
                    // The user pressed Ctrl + A:
                    acQHTMLWindow::selectAll();
                }
            }
            break;

            default:
                break;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        acQHTMLWindow::setSource
// Description: Prevent Qt from actviating links in its own way
// Arguments:   const QUrl & name
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        18/1/2012
// ---------------------------------------------------------------------------
void acQHTMLWindow::setSource(const QUrl& name)
{
    GT_UNREFERENCED_PARAMETER(name);
}
