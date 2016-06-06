//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file appQtApplication.cpp
///
//==================================================================================

// Compiler warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osCallsStackReader.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acSendErrorReportDialog.h>

// Framework:
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afNewProjectDialog.h>
#include <AMDTApplicationFramework/Include/dialogs/afGlobalSettingsDialog.h>
#include <AMDTApplicationFramework/Include/afUnhandledExceptionHandler.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>

// Local:
#include <src/appQtApplication.h>
#include <inc/appMainAppWindow.h>
#include <inc/appStringConstants.h>

// This variable is used in a static version in order to get access to the app so we can emit an event from there:
appQtApplication* gpQtApplicationForEmit = NULL;
// This is the csBuffer that will store the out of memory call stack. easier to store it this way then start passing it in an Qt event
osCallStack gOutOfMemoryCsBuffer;

// ---------------------------------------------------------------------------
// Name:        appQtApplication::appQtApplication
// Description: constructor
// Author:      Gilad Yarnitzky
// Date:        30/5/2012
// ---------------------------------------------------------------------------
appQtApplication::appQtApplication(int& argc, char** argv) : QApplication(argc, argv)
{
    // Register osExceptionCode as a meta-type for Qt. This allows Qt to queue instances
    //  of that type, which will be required to handle the exceptions across threads.
    qRegisterMetaType<osExceptionCode>("osExceptionCode");

    // Register this object which runs in the main thread as the top-level handler of exceptions in CodeXL.
    afUnhandledExceptionHandler& unhandledExceptionsHandler = afUnhandledExceptionHandler::instance();
    bool rc = connect(&unhandledExceptionsHandler, SIGNAL(UnhandledExceptionSignal(osExceptionCode, void*)),
                      this, SLOT(UnhandledExceptionHandler(osExceptionCode, void*)), Qt::AutoConnection);
    GT_ASSERT(rc);

    gpQtApplicationForEmit = this;
}


// ---------------------------------------------------------------------------
// Name:        appQtApplication::~appQtApplication
// Description: destructor
// Author:      Gilad Yarnitzky
// Date:        30/5/2012
// ---------------------------------------------------------------------------
appQtApplication::~appQtApplication()
{

}


// ---------------------------------------------------------------------------
// Name:        appQtApplication::notify
// Description: overwrite notify to capture the keyevent for handling shortcut updates
// Author:      Gilad Yarnitzky
// Date:        31/5/2012
// ---------------------------------------------------------------------------
bool appQtApplication::notify(QObject* pReceiver, QEvent* pEvent)
{
    QKeyEvent* pKeyEvent = dynamic_cast<QKeyEvent*>(pEvent);

    if (NULL != pKeyEvent)
    {
        // Check if it is an short cut key at all:
        bool shouldCheck = false;

        // Check keyboard control keys pressed or functions keys were pressed
        if ((pKeyEvent->modifiers() & Qt::ShiftModifier) || (pKeyEvent->modifiers() & Qt::ControlModifier) || (pKeyEvent->modifiers() & Qt::AltModifier) ||
            (pKeyEvent->key() >= Qt::Key_F1 && pKeyEvent->key() <= Qt::Key_F35))
        {
            shouldCheck = true;
        }

        if (shouldCheck)
        {
            // Check if the key has an short cut that needs a menu update:
            afMainAppWindow* pMainWindow = afMainAppWindow::instance();

            GT_IF_WITH_ASSERT(NULL != pMainWindow)
            {
                // Pass through all the menus in the menu bar:
                QMenuBar* pMenu = pMainWindow->menuBar();

                GT_IF_WITH_ASSERT(NULL != pMainWindow)
                {
                    QList<QMenu*> menusList = pMenu->findChildren<QMenu*>();
                    int numMenus = menusList.count();

                    for (int nMenu = 0 ; nMenu < numMenus ; nMenu++)
                    {
                        updateMenuUIbyKeyEvent(menusList[nMenu], pKeyEvent);
                    }
                }
            }

            // Update the toolbar that is managed by the afExecutionModeManager
            afExecutionModeManager::instance().updateExecutionToolbar(pKeyEvent);
        }
    }

    // Let the application handle the event normally:
    return QApplication::notify(pReceiver, pEvent);
}


// ---------------------------------------------------------------------------
// Name:        appQtApplication::updateMenuUIbyKeyEvent
// Description:
// Arguments:   QMenu* pMenu
//              QKeyEvent* pKeyEvent
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        31/5/2012
// ---------------------------------------------------------------------------
void appQtApplication::updateMenuUIbyKeyEvent(QMenu* pMenu, QKeyEvent* pKeyEvent)
{
    // Change the event into a key sequence:
    QKeySequence eventKeySequence(pKeyEvent->key() + pKeyEvent->modifiers());

    // Pass through all the actions in the menu:
    QList<QAction*> actionsList = pMenu->actions();
    int numActions = actionsList.count();

    for (int aAction = 0 ; aAction < numActions ; aAction++)
    {
        QAction* pCurrenctAction = actionsList[aAction];

        if (NULL != pCurrenctAction)
        {
            // If the action has the same key sequence as the event send the update ui command:
            if (pCurrenctAction->shortcut() == eventKeySequence)
            {
                afMainAppWindow* pMainWindow = afMainAppWindow::instance();
                appMainAppWindow* pAppMainWindow = dynamic_cast<appMainAppWindow*>(pMainWindow);

                GT_IF_WITH_ASSERT(NULL != pAppMainWindow)
                {
                    pAppMainWindow->updateSingleAction((QObject*)pCurrenctAction);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        appQtApplication::deleteSingletonDialog
// Description: Singleton QDialog/QWidget should be deleted before QApplication
//              gets destroyed and disconnected from x11 server. This is
//              for deletion of all sigleton QDialog/QWidget.
// Return Val:  void
// Author:      Bhattacharyya Koushik
// Date:        4/10/2012
// ---------------------------------------------------------------------------
void appQtApplication::deleteSingletonDialog()
{
    afNewProjectDialog::instance().deleteLater();
    afGlobalSettingsDialog::instance().deleteLater();
}

// ---------------------------------------------------------------------------
// Name:        appQtApplication::UnhandledExceptionHandler
// Description: This handler will be the application's top-level exception handler
// Arguments:   osExceptionCode exceptionCode - the unhandled exception's code
//              void* pExceptionContext - the unhandled exception's context
// Return Val:  void
// Author:      Amit Ben-Moshe
// Date:        19/10/2014
// ---------------------------------------------------------------------------
void appQtApplication::UnhandledExceptionHandler(osExceptionCode exceptionCode, void* pExceptionContext)
{
    // Display the exception to the user:
    QPixmap iconPixMap;
    acSetIconInPixmap(iconPixMap, afGlobalVariablesManager::ProductIconID(), AC_64x64_ICON);
    acSendErrorReportDialog* pSendErrorReportDialog = new acSendErrorReportDialog(NULL, afGlobalVariablesManager::ProductNameA(), iconPixMap);
    GT_IF_WITH_ASSERT(pSendErrorReportDialog != NULL)
    {
        bool allowDifferentSystemPath = false;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        // On windows, the remote process debugger is used for 64-bit debugging. We should allow the different system file path in this case:
        allowDifferentSystemPath = afCanAllowDifferentSystemPath();
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

        // Display the dialog to the user:
        pSendErrorReportDialog->onUnhandledException(exceptionCode, pExceptionContext, allowDifferentSystemPath);

        // Clean up:
        delete pSendErrorReportDialog;

        // Exit the application (this prevents the OS error report dialog from appearing):
        osExitCurrentProcess(0);
    }

    // Inform the global exceptions handler to stop waiting.
    afUnhandledExceptionHandler::instance().StopWaiting();
}

// ---------------------------------------------------------------------------
void appQtApplication::AppMemAllocFailureHandler()
{
    // First, free the reserved memory so that we don't get stuck.
    gtFreeReservedMemory();

    // Retrieve the call stack for the current thread.
    osCallsStackReader csReader;
    bool isOk = csReader.getCurrentCallsStack(gOutOfMemoryCsBuffer, true, true);
    GT_ASSERT(isOk);

    // emits the signal os it will be handled in the main thread since this can happen in
    // a different thread:
    // We can't use here GT_ASSERT because dialog are not allowed to be displayed here
    if (NULL != gpQtApplicationForEmit)
    {
        gpQtApplicationForEmit->emit AppMemAllocFailureSignal();
    }
}

// ---------------------------------------------------------------------------
void appQtApplication::ClientMemAllocFailureHandler()
{
    // emits the signal os it will be handled in the main thread since this can happen in
    // a different thread:
    // We can't use here GT_ASSERT because dialog are not allowed to be displayed here
    if (NULL != gpQtApplicationForEmit)
    {
        gpQtApplicationForEmit->emit ClientMemAllocFailureSignal();
    }
}

// ---------------------------------------------------------------------------
void appQtApplication::OnAppMemAllocFailureSignal()
{
    // Warn the user.
    acMessageBox::instance().critical(APP_STR_MEM_ALLOC_FAILURE_HEADLINE, APP_STR_MEM_ALLOC_FAILURE_EXIT_MSG);

    // Open up an issue report dialog in order to allow the user to report the issue.
    QPixmap iconPixMap;
    acSetIconInPixmap(iconPixMap, afGlobalVariablesManager::ProductIconID(), AC_64x64_ICON);
    acSendErrorReportDialog reportDialog(NULL, afGlobalVariablesManager::ProductNameA(), iconPixMap);

    bool allowDifferentSystemPath = false;
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // On windows, the remote process debugger is used for 64-bit debugging. We should allow the different system file path in this case:
    allowDifferentSystemPath = afCanAllowDifferentSystemPath();
#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

    reportDialog.onMemoryAllocationFailure(gOutOfMemoryCsBuffer, allowDifferentSystemPath);

    // Exit.
    osExitCurrentProcess(-1);
}

// ---------------------------------------------------------------------------
void appQtApplication::OnClientMemAllocFailureSignal()
{
    acMessageBox::instance().critical(AF_STR_memAllocFailureHeadline, AF_STR_memAllocFailureClientMsg);
}
