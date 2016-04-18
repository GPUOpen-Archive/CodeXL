//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afQMdiSubWindow.cpp
///
//==================================================================================

#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFileLauncher.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afQMdiSubWindow.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afDocUpdateManager.h>

// ---------------------------------------------------------------------------
// Name:        afQMdiSubWindow::afQMdiSubWindow
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        27/7/2011
// ---------------------------------------------------------------------------
afQMdiSubWindow::afQMdiSubWindow(afViewCreatorAbstract* pViewCreator) :
    QMdiSubWindow(), _pViewCreator(pViewCreator), _isActive(false), m_isAllowedToBeClosed(true), m_closeMdiWhileBlocked(false)
{
}
// ---------------------------------------------------------------------------
// Name:        afQMdiSubWindow::afQMdiSubWindow
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        27/7/2011
// ---------------------------------------------------------------------------
afQMdiSubWindow::~afQMdiSubWindow()
{
}


// ---------------------------------------------------------------------------
// Name:        afQMdiSubWindow::closeEvent
// Description: Handle sub window close event
// Arguments:   QCloseEvent *pCloseEvent
// Author:      Sigal Algranaty
// Date:        10/8/2011
// ---------------------------------------------------------------------------
void afQMdiSubWindow::closeEvent(QCloseEvent* pCloseEvent)
{
    if (IsAllowedToBeClosed())
    {
        /// Emit a signal indicating that the sub window is about to close:
        bool shouldClose = true;
        afMainAppWindow* pMainWindow = afMainAppWindow::instance();
        GT_IF_WITH_ASSERT(pMainWindow != nullptr)
        {
            pMainWindow->EmitSubwindowCloseSignal(filePath(), shouldClose);
        }

        if (shouldClose)
        {
            // Clear the document update manager:
            afDocUpdateManager::instance().RemoveSubWindow(this);

            // Call the base class implementation:
            QMdiSubWindow::closeEvent(pCloseEvent);

            // Get the instance for the main window:
            pMainWindow = afMainAppWindow::instance();
            GT_IF_WITH_ASSERT(pMainWindow != nullptr)
            {
                pMainWindow->onSubWindowClose(this);
            }

            // Make sure the focus is not left on this window
            QWidget* focusWidget = qApp->focusWidget();

            if (focusWidget == this)
            {
                clearFocus();
            }
        }
        else
        {
            pCloseEvent->ignore();
        }
    }
    else
    {
        SetCloseWhileBlockedFromClosing();
        pCloseEvent->ignore();
    }
}

// ---------------------------------------------------------------------------
void afQMdiSubWindow::setFilePath(const osFilePath& filePath)
{
    _displayedFilePath = filePath;

    // if the file path extension type is not one of the excluded types (one that we use internally) then add the menu items.
    gtString fileExtension;
    filePath.getFileExtension(fileExtension);

    if ((L"gdcxl" != fileExtension) && (L"gpsession" != fileExtension) && (L"ebp" != fileExtension) && (L"atp" != fileExtension) && (L"csv" != fileExtension) && (L"occupancy" != fileExtension) && (L"perfmarker" != fileExtension) &&
        (L"cxlovr" != fileExtension) && (L"cxltxt" != fileExtension) && (L"cxlisa" != fileExtension) && (L"cxlil" != fileExtension))
    {
        QMenu* pMainMenu = systemMenu();

        if (nullptr != pMainMenu)
        {
            QAction* pFirstAction = pMainMenu->actions().at(0);
            QAction* pCopyPathAction = new QAction(AF_STR_CopyFullPath, this);

            bool rc = connect(pCopyPathAction, SIGNAL(triggered()), this, SLOT(OnCopyPathAction()));
            GT_ASSERT(rc);

            QAction* pOpenContaningAction = new QAction(AF_STR_OpenContaningFolder, this);

            rc = connect(pOpenContaningAction, SIGNAL(triggered()), this, SLOT(OnOpenContainingFolder()));
            GT_ASSERT(rc);

            if (nullptr != pFirstAction)
            {
                pMainMenu->insertAction(pFirstAction, pCopyPathAction);
                pMainMenu->insertAction(pFirstAction, pOpenContaningAction);
                pMainMenu->insertSeparator(pFirstAction);
            }
            else
            {
                pMainMenu->addAction(pCopyPathAction);
                pMainMenu->addAction(pOpenContaningAction);
            }
        }
    }
}

// ---------------------------------------------------------------------------
void afQMdiSubWindow::OnCopyPathAction()
{
    GT_IF_WITH_ASSERT(qApp != nullptr)
    {
        // Get the clipboard from the application:
        QClipboard* pClipboard = qApp->clipboard();
        GT_IF_WITH_ASSERT(pClipboard != nullptr)
        {
            gtString filePathAsString = filePath().asString();
            QString filePathAsQtString = acGTStringToQString(filePathAsString);
            pClipboard->setText(filePathAsQtString);
        }
    }
}

// ---------------------------------------------------------------------------
void afQMdiSubWindow::OnOpenContainingFolder()
{
    afApplicationCommands::instance()->OpenContainingFolder(filePath());
}

// ---------------------------------------------------------------------------
void afQMdiSubWindow::SetAllowedToBeClosed(bool allowedToBeClosed)
{
    m_isAllowedToBeClosed = allowedToBeClosed;

    if (allowedToBeClosed && m_closeMdiWhileBlocked)
    {
        close();
    }
}
