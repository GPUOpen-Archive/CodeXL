//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afMainAppWindow.cpp
///
//==================================================================================

// System:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <QList>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>
#include <AMDTAPIClasses/Include/apExecutionMode.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acToolBar.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osFile.h>

// Local:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afIExecutionMode.h>
#include <AMDTApplicationFramework/Include/afIRunModeManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afNewProjectDialog.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afSoftwareUpdaterWindow.h>
#include <AMDTApplicationFramework/Include/afStartupPage.h>
#include <AMDTApplicationFramework/Include/afViewActionHandler.h>
#include <AMDTApplicationFramework/Include/afViewCreatorAbstract.h>
#include <AMDTApplicationFramework/Include/commands/afInitializeApplicationCommand.h>
#include <src/afPredefinedLayouts.h>

// The single instance of this class:
afMainAppWindow* afMainAppWindow::_pMyStaticInstance = nullptr;

// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::afMainAppWindow
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        27/7/2011
// ---------------------------------------------------------------------------
afMainAppWindow::afMainAppWindow() :
    QMainWindow(),
    _pProgressBar(nullptr),
    _modalMode(false),
    m_whileExitingWindow(false),
    m_pViewViewsMenu(nullptr),
    m_pWindowsMenu(nullptr),
    m_pHelpMenu(nullptr),
    m_pViewToolbarsMenu(nullptr),
    m_pFirstNoneFrameworkToolBar(nullptr),
    m_lastLayout(LayoutInitialized),
    m_pConvertDialog(nullptr),
    m_pLayoutEdit(nullptr)
{
    setAcceptDrops(true);

    QRect deskTopRect = QApplication::desktop()->availableGeometry();
    QSize desktopSize = deskTopRect.size();

    QString settingsName = settingFileName();

    QSettings settings(settingsName,  QSettings::IniFormat);

    settings.beginGroup(AF_STR_WindowRestoreGroup);

    // Restore only if contains the specific key.
    if (settings.contains(AF_STR_WindowRestoreSectionPosition))
    {
        // restore the data:
        QSize windowSize = settings.value(AF_STR_WindowRestoreSectionSize).toSize();
        QPoint windowPoint = settings.value(AF_STR_WindowRestoreSectionPosition).toPoint();
        bool windowMaximized = settings.value(AF_STR_WindowRestoreSectionMaximized).toBool();

        // set the window attributes based on the restored data:
        resize(windowSize);

        // Check that the position is in a visible area (might have changed due to change of changing in desktop window size):
        if (!deskTopRect.contains(windowPoint))
        {
            windowPoint = QPoint(0, 0);
        }

        move(windowPoint);

        // restore maximized/normal state:
        setWindowState(windowMaximized ? Qt::WindowMaximized : Qt::WindowNoState);
    }
    else
    {
        // Set size to 80% of desktop size;
        QSize factoredSize = desktopSize * (1 - AF_MAIN_SCREEN_RATIO_REDUCTION);
        resize(factoredSize);
        // the 48 is a rough estimation of menu + title. need to find from system the real value in next version
        QPoint windowPoint((desktopSize.width() - factoredSize.width()) / 2, (desktopSize.height() - factoredSize.height() - 48) / 2);
        move(windowPoint);
    }

    settings.endGroup();

    // Prevent from Qt forcing a minimum size that is wrong:
    layout()->setSizeConstraint(QLayout::SetNoConstraint);

    m_initialLayoutMode = LayoutNoProject;
}

// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::initializeInstance
// Description: Default implementation
// Author:      Sigal Algranaty
// Date:        28/7/2011
// ---------------------------------------------------------------------------
afMainAppWindow* afMainAppWindow::instance()
{
    return _pMyStaticInstance;
}


// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::updateToolbarsCommands
// Description: Default implementation
// Author:      Sigal Algranaty
// Date:        1/8/2011
// ---------------------------------------------------------------------------
void afMainAppWindow::updateToolbarsCommands()
{

}

// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::updateToolbars
// Description: Default implementation
// Author:      Sigal Algranaty
// Date:        1/8/2011
// ---------------------------------------------------------------------------
void afMainAppWindow::updateToolbars()
{

}

// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::onSubWindowClose
// Description: Default implementation
// Arguments:   afQMdiSubWindow* pSubWindowAboutToBeClosed
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        10/8/2011
// ---------------------------------------------------------------------------
void afMainAppWindow::onSubWindowClose(afQMdiSubWindow* pSubWindowAboutToBeClosed)
{
    GT_UNREFERENCED_PARAMETER(pSubWindowAboutToBeClosed);
}

// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::addToolbar
// Description: Default implementation
// Author:      Gilad Yarnitzky
// Date:        27/5/2012
// ---------------------------------------------------------------------------
void afMainAppWindow::addToolbar(acToolBar* pToolbar, acToolBar* pBeforeToolbar, bool frameworkToolbar)
{
    GT_IF_WITH_ASSERT(nullptr != pToolbar)
    {
        if (nullptr == pBeforeToolbar)
        {
            addToolBar(pToolbar);
        }
        else
        {
            insertToolBar(pBeforeToolbar, pToolbar);
        }

        if (!frameworkToolbar && (nullptr == m_pFirstNoneFrameworkToolBar))
        {
            m_pFirstNoneFrameworkToolBar = pToolbar;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::setWindowCaption
// Description: Default implementation
// Arguments:   wxWindow* pWindow
//              const gtString& windowCaption
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/8/2011
// ---------------------------------------------------------------------------
bool afMainAppWindow::setWindowCaption(wxWindow* pWindow, const gtString& windowCaption)
{
    GT_UNREFERENCED_PARAMETER(pWindow);
    GT_UNREFERENCED_PARAMETER(windowCaption);

    return false;
}


// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::createSingleView
// Description: Default implementation
// Arguments:   afViewCreatorAbstract* pCreator
//              int viewIndex
//              const osFilePath& fileName
// Return Val:  QWidget*
// Author:      Sigal Algranaty
// Date:        23/8/2011
// ---------------------------------------------------------------------------
QWidget* afMainAppWindow::createSingleView(afViewCreatorAbstract* pCreator, int viewIndex)
{
    GT_UNREFERENCED_PARAMETER(pCreator);
    GT_UNREFERENCED_PARAMETER(viewIndex);

    return nullptr;
}


// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::storeLayout
// Description: stores last layout
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        25/8/2011
// ---------------------------------------------------------------------------
void afMainAppWindow::storeLayout(const gtString& layoutName)
{
    if (!layoutName.isEmpty())
    {
        QString layoutNameQStr =  QString::fromWCharArray(layoutName.asCharArray());
        QString settingsName = settingFileName();

        QSettings settings(settingsName,  QSettings::IniFormat);
        settings.beginGroup(layoutNameQStr);

        settings.setValue(AF_STR_WindowRestoreWindowState, saveState());

        settings.endGroup();
    }
}


// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::restoreLayout
// Description: restores last layout
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        25/8/2011
// ---------------------------------------------------------------------------
void afMainAppWindow::restoreLayout(LayoutFormats layoutId)
{
    // Set the layout name that is needed
    gtString layoutName = getLayoutName(layoutId);

    QString layoutNameQStr =  QString::fromWCharArray(layoutName.asCharArray());
    QString settingsName = settingFileName();

    QSettings settings(settingsName,  QSettings::IniFormat);

    bool needsRecovery = false;

    // Check if needs recovery: group does not exist, or empty:
    QStringList groupsInSettings = settings.childGroups();

    if (!groupsInSettings.contains(layoutNameQStr))
    {
        needsRecovery = true;
    }
    else
    {
        // Enter the specific layout group
        settings.beginGroup(layoutNameQStr);

        // Restore only if contains the specific key.
        if (!settings.contains(AF_STR_WindowRestoreWindowState))
        {
            needsRecovery = true;
        }

        settings.endGroup();
    }

    // recover if needs recovery:
    if (needsRecovery)
    {
        writePredefineLayout(layoutId);
    }

    // Read the data from the group, at this stage it should be available after recovery at worst case:
    settings.beginGroup(layoutNameQStr);

    if (settings.contains(AF_STR_WindowRestoreWindowState))
    {
        // Restore dock and toolbar states data:
        restoreState(settings.value(AF_STR_WindowRestoreWindowState).toByteArray());
    }
    else
    {
        // there is a problem record it;
        GT_ASSERT(false);
    }

    // exit the group:
    settings.endGroup();
}

// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::resetInitialLayout
// Description: Reset layout to default layout as if no layouts exist
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        1/9/2011
// ---------------------------------------------------------------------------
void afMainAppWindow::resetInitialLayout()
{
    QString settingsName = settingFileName();

    QSettings settings(settingsName,  QSettings::IniFormat);
    // Clear all groups
    // Get the list of the keys (groups) and remove them one by one if they are not the default value
    QStringList groups = settings.childGroups();
    int numGroups = groups.count();

    for (int nGroup = 0 ; nGroup < numGroups ; nGroup++)
    {
        settings.remove(groups[nGroup]);
    }

    // Restore all layouts:
    for (int nLayout = 0 ; nLayout < nLayoutFormats ; nLayout++)
    {
        writePredefineLayout(LayoutFormats(nLayout));
    }

    // restore the current layout
    restoreLayout(m_lastLayout);
}

bool afMainAppWindow::OnFind(bool respondToTextChanged)
{
    GT_UNREFERENCED_PARAMETER(respondToTextChanged);
    return false;
}

// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::openStartupdialog
// Description:
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/2/2012
// ---------------------------------------------------------------------------
void afMainAppWindow::openStartupdialog()
{
    // Get or create the startup pate widget:
    afStartupPage* pStartupPage = nullptr;

    // Build the startup page path:
    osFilePath startupPagePath;
    afGetUserDataFolderPath(startupPagePath);
    startupPagePath.setFileName(AF_STR_StartupPageHTMLFileName);

    // Check if the window is already created:
    afQMdiSubWindow* pExistingSubwindow = findMDISubWindow(startupPagePath);
    bool isNewWindow = (pExistingSubwindow == nullptr);

    if (isNewWindow)
    {
        // Allocate a new startup page widget:
        pStartupPage = new afStartupPage;


        // Check if the window is already created:
        pExistingSubwindow = addMDISubWindow(startupPagePath, pStartupPage, AF_STR_WelcomePage);
        GT_ASSERT(pExistingSubwindow != nullptr);
    }
    else
    {
        pStartupPage = qobject_cast<afStartupPage*>(pExistingSubwindow->widget());
    }

    GT_IF_WITH_ASSERT(pStartupPage != nullptr)
    {
        // Load the HTML from file:
        pStartupPage->UpdateHTML();

        if (isNewWindow)
        {
            pStartupPage->setVisible(true);
            pStartupPage->show();

            pExistingSubwindow->setVisible(true);
            pExistingSubwindow->show();
            pExistingSubwindow->setFocus();
            pExistingSubwindow->raise();
            pExistingSubwindow->activateWindow();
        }
        else
        {
            activateSubWindow(pExistingSubwindow);
        }

    }
}


// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::settingFileName
// Description:
// Return Val:  QString&
// Author:      Gilad Yarnitzky
// Date:        15/3/2012
// ---------------------------------------------------------------------------
QString afMainAppWindow::settingFileName()
{
    osFilePath appDataPath;
    afGetUserDataFolderPath(appDataPath);

    // Add the Settings file name into it
    appDataPath.setFileName(AF_STR_LayoutFileName);
    gtString appDataPathAsStr = appDataPath.asString();

    QString appDataPathAsQStr(QString::fromWCharArray(appDataPathAsStr.asCharArray()));

    return appDataPathAsQStr;
}

/// ------------------------------------------------------------------------------------------------
/// \brief Name:        dragEnterEvent
/// \brief Description: Handle drag enter event
/// \param[in]          QDragEnterEvent *pEvent
/// \return void
/// ------------------------------------------------------------------------------------------------
void afMainAppWindow::dragEnterEvent(QDragEnterEvent* pEvent)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pEvent != nullptr)
    {
        // Check what is the needed drag action for the current event:
        afApplicationTree::DragAction dragActionForFiles = afApplicationCommands::instance()->DragActionForDropEvent(pEvent);

        if (dragActionForFiles != afApplicationTree::DRAG_NO_ACTION)
        {
            pEvent->acceptProposedAction();
        }
    }
}

/// ------------------------------------------------------------------------------------------------
/// \brief Name:        dragMoveEvent
/// \brief Description: Handle drag move event
/// \param[in]          QDragMoveEvent *pEvent
/// \return void
/// ------------------------------------------------------------------------------------------------
void afMainAppWindow::dragMoveEvent(QDragMoveEvent* pEvent)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pEvent != nullptr)
    {
        // Check what is the needed drag action for the current event:
        afApplicationTree::DragAction dragActionForFiles = afApplicationCommands::instance()->DragActionForDropEvent(pEvent);

        if (dragActionForFiles != afApplicationTree::DRAG_NO_ACTION)
        {
            pEvent->acceptProposedAction();
        }
    }
}

/// ------------------------------------------------------------------------------------------------
/// \brief Name:        dropEvent
/// \brief Description: Implements the drop event
/// \param[in]          QDragMoveEvent *pEvent
/// \return void
/// ------------------------------------------------------------------------------------------------
void afMainAppWindow::dropEvent(QDropEvent* pEvent)
{
    afApplicationCommands::instance()->HandleDropEvent(this, pEvent);
}


// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::closeEvent
// Description: Handle the close event
//              store the state of the window: position, size, and max state
// Author:      Gilad Yarnitzky
// Date:        9/12/2012
// ---------------------------------------------------------------------------
void afMainAppWindow::closeEvent(QCloseEvent* pCloseEvent)
{
    GT_UNREFERENCED_PARAMETER(pCloseEvent);

    QString settingsName = settingFileName();

    QSettings settings(settingsName,  QSettings::IniFormat);

    settings.beginGroup(AF_STR_WindowRestoreGroup);

    // restore the data:
    settings.setValue(AF_STR_WindowRestoreSectionSize, size());
    settings.setValue(AF_STR_WindowRestoreSectionPosition, pos());
    settings.setValue(AF_STR_WindowRestoreSectionMaximized, isMaximized());

    settings.endGroup();

    afProgressBarWrapper::instance().ShutDown();

    // Save general settings needed for dialogs not to show:
    afGlobalVariablesManager::instance().saveGlobalSettingsToXMLFile();

    // Destroy the update window here instead of the singleton deleter
    //OS_OUTPUT_DEBUG_LOG(L"Before deleting afSoftwareUpdaterWindow", OS_DEBUG_LOG_DEBUG);
    //delete afSoftwareUpdaterWindow::m_spSoftwareUpdaterWindow;
    //afSoftwareUpdaterWindow::m_spSoftwareUpdaterWindow = nullptr;
    //OS_OUTPUT_DEBUG_LOG(L"After deleting m_spSoftwareUpdaterWindow", OS_DEBUG_LOG_DEBUG);

    // Destroy the afNewProjectDialog window here instead of the singleton deleter
    OS_OUTPUT_DEBUG_LOG(L"Before deleting afNewProjectDialog", OS_DEBUG_LOG_DEBUG);
    delete afNewProjectDialog::m_spMySingleInstance;
    afNewProjectDialog::m_spMySingleInstance = nullptr;
    OS_OUTPUT_DEBUG_LOG(L"After deleting afNewProjectDialog", OS_DEBUG_LOG_DEBUG);

    OS_OUTPUT_DEBUG_LOG(L"Before terminating sysinfocommand thread", OS_DEBUG_LOG_DEBUG);
    afInitializeApplicationCommand initAppCmd(L"", L"");
    initAppCmd.EndSysCommandThread();
    OS_OUTPUT_DEBUG_LOG(L"After terminating sysinfocommand thread", OS_DEBUG_LOG_DEBUG);

    // Let the modes do any mode specific termination that is needed
    afExecutionModeManager::instance().TerminateModes();
}

// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::createToolbarForMode
// Description: create a toolbar for a mode if it is not already created
// Arguments:   const gtString& modeName
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        2/12/2012
// ---------------------------------------------------------------------------
void afMainAppWindow::createToolbarForMode(const gtString& modeName)
{
    GT_IF_WITH_ASSERT(!modeName.isEmpty())
    {
        QString modeToolbarNameAsQString = QString::fromWCharArray(modeName.asCharArray());

        // Fix the modes toolbar name:
        modeToolbarNameAsQString.replace(" Mode", AF_STR_viewsToolbarPostfix);

        // Pass through all mode toolbars and see if one does not already have this mode name:
        acToolBar* pFindExistingToolbar = findToolbarByModeName(acQStringToGTString(modeToolbarNameAsQString));

        if (nullptr == pFindExistingToolbar)
        {
            // check if there are mode specific view. If there are none do not create the toolbar:
            bool shouldCreateToolbar = false;

            int numModeActions = m_modeViewsActionsList.size();

            // Add the mode specific views:
            for (int nModeAction = 0 ; nModeAction < numModeActions ; nModeAction++)
            {
                afViewToolBarData* pCurrentViewData = m_modeViewsActionsList.at(nModeAction);

                GT_IF_WITH_ASSERT(nullptr != pCurrentViewData && nullptr != pCurrentViewData->m_pViewAction)
                {
                    // Add the view action if it belongs to the mode:
                    if (pCurrentViewData->m_modeName == modeName)
                    {
                        shouldCreateToolbar = true;
                    }
                }
            }

            if (shouldCreateToolbar)
            {
                // Store the last toolbar name on the list:
                acToolBar* pToolbar = new acToolBar(this, modeToolbarNameAsQString);
                m_modesViewsToolbarList.push_back(pToolbar);
                pToolbar->setObjectName(modeToolbarNameAsQString);

                // Add the toolbar in the right order:
                addToolbar(pToolbar, m_pFirstNoneFrameworkToolBar, true);

                // add all the framework views to the toolbar:
                int numViewsActions = m_frameworkViewsActionsList.size();

                for (int i = 0 ; i < numViewsActions ; i++)
                {
                    QAction* pCurrentAction = m_frameworkViewsActionsList.at(i);
                    pToolbar->addAction(pCurrentAction);
                }

                bool addedSeparator = false;

                numModeActions = m_modeViewsActionsList.size();

                // Add the mode specific views:
                for (int nModeAction = 0 ; nModeAction < numModeActions ; nModeAction++)
                {
                    afViewToolBarData* pCurrentViewData = m_modeViewsActionsList.at(nModeAction);

                    GT_IF_WITH_ASSERT(nullptr != pCurrentViewData && nullptr != pCurrentViewData->m_pViewAction)
                    {
                        // Add the view action if it belongs to the mode:
                        if (pCurrentViewData->m_modeName == modeName)
                        {
                            // add a separator after the framework views actions if not already done:
                            if (!addedSeparator)
                            {
                                pToolbar->addSeparator();
                                addedSeparator = true;
                            }

                            pToolbar->addAction(pCurrentViewData->m_pViewAction);
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::addActionForView
// Description: Add an action for the added view
// Arguments:   afViewCreatorAbstract* pCreator
//              int viewIndex
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/8/2011
// ---------------------------------------------------------------------------
bool afMainAppWindow::addActionForView(afViewCreatorAbstract* pCreator, int viewIndex, QWidget* pWindowWidget)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pViewViewsMenu != nullptr) && (m_pWindowsMenu != nullptr) && (pWindowWidget != nullptr))
    {
        // Get the file path:
        osFilePath filePath;
        gtString filePathStr;
        bool rc = pCreator->getCurrentlyDisplayedFilePath(filePath);

        if (rc)
        {
            // Get the file path as string:
            filePathStr = filePath.asString();
        }

        // Get the view name:
        gtString viewName, viewCommandName;
        pCreator->titleString(viewIndex, viewName, viewCommandName);

        // create the sub menu from the name if needed:
        QMenu* pViewMenu = m_pViewViewsMenu;
        afActionPositionData positionData;

        gtString executionMode = pCreator->executionMode(viewIndex);
        gtString originalExecutionMode = executionMode;

        if (!executionMode.isEmpty())
        {
            executionMode.prepend(L"&");
        }

        gtString menuPosition = pCreator->modeMenuPosition(viewIndex, positionData);

        if (!executionMode.isEmpty())
        {
            gtString menuString(AF_STR_ViewMenuString);
            menuString.append(AF_Str_MenuSeparator);
            menuString.append(executionMode);
            menuString.append(AF_Str_MenuSeparator);
            menuString.append(menuPosition);

            pViewMenu = getActionMenuItemParentMenu(menuString, m_pViewToolbarsMenu->menuAction(), true);
        }

        // Check what should be the action text:
        QString actionText = acGTStringToQString(filePathStr);

        // Get the file extension:
        gtString fileExtension;
        filePath.getFileExtension(fileExtension);

        if ((fileExtension == AF_STR_CodeXMLImageBuffersFilesExtension) ||
            (fileExtension == AF_STR_CpuProfileFileExtension) ||
            (fileExtension == AF_STR_GpuProfileTraceFileExtension) ||
            (fileExtension == AF_STR_FrameAnalysisTraceFileExtension) ||
            (fileExtension == AF_STR_GpuProfileSessionFileExtension) ||
            (fileExtension == AF_STR_PowerProfileSessionFileExtension) ||
            (fileExtension == AF_STR_FrameAnalysisPerfCountersFileExtension) ||
            (fileExtension == AF_STR_frameAnalysisDashboardFileExtension) ||
            (fileExtension == AF_STR_GpuPerformanceCountersSessionFileExtension) ||
            (actionText.isEmpty()))
        {
            // Internal file:
            actionText = acGTStringToQString(viewCommandName);
        }

        // Create the menu command associated with the view:
        QAction* pAction = new QAction(actionText, this);


        // Set the file path as string on the action:
        if (!filePathStr.isEmpty())
        {
            QVariant var(QString::fromWCharArray(filePathStr.asCharArray()));
            GT_ASSERT(var.isValid());
            pAction->setData(var);
        }

        // Check the view visibility:
        bool isVisible = pCreator->visibility(viewIndex);

        // Create the action and action handler:
        afViewActionHandler* pActionHandler = new afViewActionHandler(pWindowWidget, pAction);

        m_viewsActionHandler.push_back(pActionHandler);

        if (pCreator->isDynamic())
        {
            // Prepend a number to the file name:
            int fileIndex = m_pWindowsMenu->actions().size() - 1;
            QString actionTextWithAccelerator, actionNumberText;
            actionNumberText.sprintf("%d", fileIndex);

            if (fileIndex <= 9)
            {
                actionNumberText.prepend("&");
            }
            else
            {
                actionNumberText = "";
                actionNumberText.sprintf("%d&%d", (int)fileIndex / 10, fileIndex % 10);
            }

            actionTextWithAccelerator.sprintf("%s %s", actionNumberText.toLatin1().data(), pAction->text().toLatin1().data());
            pAction->setText(actionTextWithAccelerator);

            // Add to the windows menu:
            m_pWindowsMenu->addAction(pAction);

            pAction->setCheckable(false);

            // Create the connection:
            bool rcConnect = connect(pAction, SIGNAL(triggered()), pActionHandler, SLOT(onWindowActionClicked()));
            GT_ASSERT(rcConnect);

        }
        else
        {
            // Check if a separator is needed:
            if (pCreator->addSeparator(viewIndex))
            {
                pViewMenu->addSeparator();
            }

            // Add to the view menu:
            pViewMenu->insertAction(m_pViewToolbarsMenu->menuAction(), pAction);

            // The action should be checkable:
            pAction->setCheckable(true);

            // Check if the view action has an icon:
            QPixmap* pPixmap = pCreator->iconAsPixmap(viewIndex);

            if (pPixmap != nullptr)
            {
                pAction->setIcon(QIcon(*pPixmap));
            }

            // Create the connection:
            bool rcConnect = connect(pAction, SIGNAL(triggered()), pActionHandler, SLOT(onViewClicked()));
            GT_ASSERT(rcConnect);

            QDockWidget* pDockWidget = qobject_cast<QDockWidget*>(pWindowWidget);

            if (pDockWidget != nullptr)
            {
                rcConnect = connect(pWindowWidget, SIGNAL(visibilityChanged(bool)), pAction, SLOT(setChecked(bool)));
                GT_ASSERT(rcConnect);
            }

            // If view is related to execution mode add it to its toolbar:
            if (!executionMode.isEmpty())
            {
                afViewToolBarData* pViewData = new afViewToolBarData;


                pViewData->m_modeName = originalExecutionMode;
                pViewData->m_pViewAction = pAction;
                m_modeViewsActionsList.push_back(pViewData);
            }
            else
            {
                // else add it to the framework views list:
                m_frameworkViewsActionsList.push_back(pAction);
            }
        }

        pAction->setChecked(isVisible);

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::getActionMenuItemParentMenu
// Description: Get the parent menu of the action item from its string
//              formated in menuA/subMenuB/subMenuC...
//              Creates sub menus if needed.
// Arguments:   gtString& menuPosition
// Return Val:  QMenu*
// Author:      Gilad Yarnitzky
// Date:        17/7/2011
// ---------------------------------------------------------------------------
QMenu* afMainAppWindow::getActionMenuItemParentMenu(const gtString& menuPosition, QAction* pBeforeAction, bool shouldCreate)
{
    QMenu* pRetVal = nullptr;

    gtString token;
    gtStringTokenizer menuSlashTokenizer(menuPosition, L"/");
    QWidget* pCurrentParent = menuBar();
    GT_IF_WITH_ASSERT(pCurrentParent != nullptr)
    {
        // Pass on the menuPosition string and find the tokens of sub menus
        while (menuSlashTokenizer.getNextToken(token))
        {
            // Get the sub menu:
            QWidget* pCurrentMenu = getSubMenu(pCurrentParent, token, pBeforeAction, shouldCreate);

            if (pCurrentMenu != nullptr)
            {
                // And use it for next stage:
                pCurrentParent = pCurrentMenu;
            }
        }

        // Convert the lats found widget to menu:
        pRetVal = qobject_cast<QMenu*>(pCurrentParent);
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::getSubMenu
// Description: Get a specific sub menu name in a menu
// Arguments:   QObject* parentItem
//              QString& menuName - menu found
// Return Val:  QMenu* - The menu if it is found, nullptr if not
// Author:      Gilad Yarnitzky
// Date:        17/7/2011
// ---------------------------------------------------------------------------
QMenu* afMainAppWindow::getSubMenu(QWidget* pParentItem, gtString& menuName, QAction* pBeforeAction, bool shouldCreate)
{
    QMenu* pRetVal = nullptr;

    // Validate parent object:
    GT_IF_WITH_ASSERT(pParentItem != nullptr)
    {
        // Convert the menuItem name to QString format:
        QString menuNameQt = QString::fromWCharArray(menuName.asCharArray());

        // Check that the pParent object is QMenu or QMenuBar:
        const QMetaObject* pParentMetaObject = pParentItem->metaObject();
        GT_IF_WITH_ASSERT((strcmp(pParentMetaObject->className(), "QMenu") == 0) || (strcmp(pParentMetaObject->className(), "QMenuBar") == 0))
        {
            // Find the menu item:
            QList<QMenu*> childMenus = pParentItem->findChildren<QMenu*>();
            int numChildren = childMenus.count();

            for (int child = 0 ; child < numChildren ; child++)
            {
                if (childMenus[child]->title() == menuNameQt)
                {
                    // Return the first found:
                    pRetVal = childMenus[child];
                    break;
                }
            }

            // If not found create a new one and add it:
            if ((pRetVal == nullptr) && shouldCreate)
            {
                // Create a new menu:
                pRetVal = new QMenu(pParentItem);


                // Set the menu item text:
                pRetVal->setTitle(menuNameQt);

                // Set the help menu if this is the help menu:
                if (menuName == AF_STR_HelpMenuString)
                {
                    m_pHelpMenu = pRetVal;
                }

                // Insert the menu:
                QMenu* pMenu = qobject_cast<QMenu*>(pParentItem);

                if (pMenu != nullptr)
                {
                    // The parent item is a sub menu:
                    pMenu->insertMenu(pBeforeAction, pRetVal);
                }
                else
                {
                    // The parent item is the menu bar:
                    QMenuBar* pMenuBar = qobject_cast<QMenuBar*>(pParentItem);

                    if (m_pHelpMenu != nullptr)
                    {
                        pBeforeAction = m_pHelpMenu->menuAction();
                        pMenuBar->insertMenu(pBeforeAction, pRetVal);
                    }
                    else
                    {
                        pMenuBar->insertMenu(pBeforeAction, pRetVal);
                    }
                }

                // Connect the menu to its slots:
                bool rcConnect = connect(pRetVal, SIGNAL(aboutToShow()), this, SLOT(onAboutToShowMenu()));
                GT_ASSERT(rcConnect);
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::findToolbarByModeName
// Description: find a toolbar for a specific mode by the name of the mode
// Author:      Gilad Yarnitzky
// Date:        2/12/2012
// ---------------------------------------------------------------------------
acToolBar* afMainAppWindow::findToolbarByModeName(const gtString& modeName)
{
    acToolBar* retVal = nullptr;

    int numToolBars = m_modesViewsToolbarList.size();

    QString modeNameAsQString = QString::fromWCharArray(modeName.asCharArray());

    for (int i = 0 ; i < numToolBars; i++)
    {
        acToolBar* pCurrentToolbar = m_modesViewsToolbarList.at(i);
        QString toolBarName = pCurrentToolbar->windowTitle();

        if (toolBarName == modeNameAsQString)
        {
            retVal = pCurrentToolbar;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::onLayoutActionTriggered
// Description:
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        9/12/2012
// ---------------------------------------------------------------------------
void afMainAppWindow::onLayoutActionTriggered()
{
    static int saveLayout = 1;

    // build the layout name:
    gtString saveLayoutStr;
    saveLayoutStr.appendFormattedString(L"defined%d", saveLayout);

    // Set setting name
    QString layoutNameQStr =  QString::fromWCharArray(saveLayoutStr.asCharArray());
    QString settingsName = settingFileName();

    // Open setting file:
    QSettings settings(settingsName,  QSettings::IniFormat);

    settings.beginGroup(layoutNameQStr);

    QByteArray windowState = saveState();
    settings.setValue(AF_STR_WindowRestoreWindowStateOriginal, windowState);

    // move to the next layout if several layouts are recorded:
    saveLayout++;

    // Create dialog of conversion
    m_pConvertDialog = new QDialog(this);
    QVBoxLayout* pBoxLayout = new QVBoxLayout();
    QPushButton* pButton = new QPushButton();
    m_pLayoutEdit = new QTextEdit();

    // add controls to the dialog
    if (nullptr != m_pConvertDialog && nullptr != pBoxLayout && nullptr != pButton && nullptr != m_pLayoutEdit)
    {
        pBoxLayout->addWidget(m_pLayoutEdit);
        pBoxLayout->addWidget(pButton);
        connect(pButton,  SIGNAL(clicked()), this, SLOT(onConvertLayout()));
        m_pConvertDialog->setLayout(pBoxLayout);
        pButton->setText("Convert");
        // Start the dialog
        m_pConvertDialog->exec();

        // Get the text from the text control and start converting:
        QString windowStateConverted;

        // convert the windowState into a string where each '\' is turned to '\\'
        QString convertedString = AF_STR_LayoutSettingConvertedHead;
        convertedString.append(m_pConvertString);
        settings.setValue(AF_STR_WindowRestoreWindowState, convertedString);
    }

    // clear dialog, the dialog deletes all children items (Qt behavior)
    if (nullptr != m_pConvertDialog)
    {
        delete m_pConvertDialog;
        m_pConvertDialog = nullptr;
        m_pLayoutEdit = nullptr;
    }

    settings.endGroup();
}


// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::onConvertLayout
// Description: convert the setting format so it can be copied from an .h file
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        16/12/2012
// ---------------------------------------------------------------------------
void afMainAppWindow::onConvertLayout()
{
    m_pConvertString = m_pLayoutEdit->toPlainText();
    m_pConvertDialog->accept();
}
// Set the current layout mode:

// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::updateLayoutMode
// Description: update the layout based on the new layout
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        10/12/2012
// ---------------------------------------------------------------------------
void afMainAppWindow::updateLayoutMode(LayoutFormats layoutId)
{
    gtString layoutName;

    // Check if the new layout is not the one that is currently in use:
    if (layoutId != m_lastLayout)
    {
        // check if the layout is of an id that is supposed to be stored at all:
        if (layoutId != LayoutNoProject && layoutId != LayoutNoProjectOutput)
        {
            // Store the current layout:
            gtString oldLayoutName = getLayoutName(m_lastLayout);
            storeLayout(oldLayoutName);
        }

        // restore the new layout:
        restoreLayout(layoutId);

        // store layoutId as last used layout
        m_lastLayout = layoutId;
    }
}


// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::getLayoutName
// Description: Get the layout name based on the id
// Author:      Gilad Yarnitzky
// Date:        10/12/2012
// ---------------------------------------------------------------------------
gtString afMainAppWindow::getLayoutName(LayoutFormats layoutId)
{
    gtString retVal;

    switch (layoutId)
    {
        case LayoutInitialized:
        case LayoutNoProject:
            retVal = AF_STR_LayoutNoProjectLoaded;
            break;

        case LayoutNoProjectOutput:
            retVal = AF_STR_LayoutNoProjectOutputLoaded;
            break;

        case LayoutDebug:
            retVal = AF_STR_LayoutDebug;
            break;

        case LayoutDebugKernel:
            retVal = AF_STR_LayoutDebugKernel;
            break;

        case LayoutProfileCPU:
            retVal = AF_STR_LayoutProfileCPU;
            break;

        case LayoutProfileGPU:
            retVal = AF_STR_LayoutProfileGPU;
            break;

        case LayoutProfilePP:
            retVal = AF_STR_LayoutProfilePP;
            break;

        case LayoutKernelAnalyzer:
            retVal = AF_STR_LayoutKernelAnalyze;
            break;

        case LayoutFrameAnalysis:
            retVal = AF_STR_LayoutFrameAnalysis;
            break;

        default:
        {
            GT_ASSERT(false);
        }
        break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::bringDockToFront
// Description: bring a dockable view to front ensuring it is visible:
// Author:      Gilad Yarnitzky
// Date:        21/8/2013
// ---------------------------------------------------------------------------
void afMainAppWindow::bringDockToFront(const gtString& dockName)
{
    int numViewHandlers = (int)m_viewsActionHandler.size();
    QString windowTitle = acGTStringToQString(dockName);

    // Go through the view action handlers:
    for (int i = 0; i < numViewHandlers; i++)
    {
        afViewActionHandler* pViewHandler = m_viewsActionHandler[i];

        if ((nullptr != pViewHandler) && (nullptr != pViewHandler->controlledWidget()) && (nullptr != pViewHandler->action()))
        {
            if (pViewHandler->controlledWidget()->windowTitle() == windowTitle)
            {
                if (pViewHandler->action()->isChecked() != true)
                {
                    pViewHandler->action()->trigger();
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::writePredefineLayout
// Description: Writes a predefined layout to the settings file.
//              Overwrite if already exists in the settings file.
// Author:      Gilad Yarnitzky
// Date:        12/12/2012
// ---------------------------------------------------------------------------
void afMainAppWindow::writePredefineLayout(LayoutFormats layoutId)
{
    // Get the temp file name into which the predefined layout string will be written:
    osFilePath tempSettingPath;
    afGetUserDataFolderPath(tempSettingPath);

    tempSettingPath.setFileName(AF_STR_LayoutTempFile);
    QString tempSettingPathStr = QString::fromWCharArray(tempSettingPath.asString().asCharArray());

    // open the file:
    osFile tempSettingFile;
    tempSettingFile.open(tempSettingPath, osChannel::OS_ASCII_TEXT_CHANNEL, osFile::OS_OPEN_TO_WRITE);

    //write the string in setting style:
    gtString groupName = getLayoutName(layoutId);
    gtString groupString;
    groupString.appendFormattedString(AF_STR_LayoutGroupStrFormat, groupName.asCharArray());
    tempSettingFile.writeString(groupString);

    tempSettingFile.writeString(AF_STR_LayoutSettingStrFormatHead);

    switch (layoutId)
    {
        case LayoutInitialized:
            tempSettingFile.writeString(LayoutNoProjectSTR);
            break;

        case LayoutNoProject:
            tempSettingFile.writeString(LayoutNoProjectSTR);
            break;

        case LayoutNoProjectOutput:
            tempSettingFile.writeString(LayoutNoProjectOutputSTR);
            break;

        case LayoutDebug:
            tempSettingFile.writeString(LayoutDebugSTR);
            break;

        case LayoutDebugKernel:
            tempSettingFile.writeString(LayoutDebugKernelSTR);
            break;

        case LayoutProfileCPU:
            tempSettingFile.writeString(LayoutProfileCPUSTR);
            break;

        case LayoutProfileGPU:
            tempSettingFile.writeString(LayoutProfileGPUSTR);
            break;

        case LayoutProfilePP:
            tempSettingFile.writeString(LayoutProfilePPSTR);
            break;

        case LayoutKernelAnalyzer:
            tempSettingFile.writeString(LayoutKernelAnalyzerSTR);
            break;

        case LayoutFrameAnalysis:
            tempSettingFile.writeString(LayoutFrameAnalysisSTR);
            break;

        case nLayoutFormats:
            break;
    }

    tempSettingFile.close();

    // open settings files (temp and codexl):
    QString settingsName = settingFileName();
    QSettings codeXLSetting(settingsName,  QSettings::IniFormat);
    QSettings tempSettings(tempSettingPathStr, QSettings::IniFormat);

    // Enter the specific layout group
    gtString layoutName = getLayoutName(layoutId);
    QString layoutNameQStr =  QString::fromWCharArray(layoutName.asCharArray());

    codeXLSetting.beginGroup(layoutNameQStr);
    tempSettings.beginGroup(layoutNameQStr);

    // read the layout and write to codexl layout:
    QByteArray layoutData;
    layoutData = tempSettings.value(AF_STR_WindowRestoreWindowState).toByteArray();
    codeXLSetting.setValue(AF_STR_WindowRestoreWindowState, layoutData);

    // close settings files and delete temp settings:
    tempSettings.endGroup();
    codeXLSetting.endGroup();

    tempSettingFile.deleteFile();
}

bool afMainAppWindow::renameMDIWindow(const QString& oldName, const QString& newName, const QString& newFilePath)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(!oldName.isEmpty() && !newName.isEmpty())
    {
        QList<QAction*> windowsActions = m_pWindowsMenu->actions();

        foreach (QAction* pCurrentAction, windowsActions)
        {
            if (pCurrentAction != nullptr)
            {
                QString currentActionText = pCurrentAction->text();

                if (currentActionText.contains(oldName))
                {
                    // Replace the old string with the new one:
                    currentActionText.replace(oldName, newName);

                    // Set the new text:
                    pCurrentAction->setText(currentActionText);

                    // Get the data stored on the action:
                    QVariant newVar(newFilePath);
                    pCurrentAction->setData(newVar);

                    retVal = true;
                }
            }
        }
    }

    return retVal;
}

