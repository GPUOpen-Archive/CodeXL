//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file appMainAppWindow.cpp
///
//==================================================================================

// System:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <gtk/gtk.h>
    #pragma GCC diagnostic ignored "-Wstrict-overflow" //Ignoring QT 5.4.1 GCC error
#endif

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <gdk/gdkx.h>
#endif

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAssertionHandlers/Include/ahDialogBasedAssertionFailureHandler.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/Include/acFindWidget.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acFrozenColumnTreeView.h>
#include <AMDTApplicationComponents/Include/acVirtualListCtrl.h>
#include <AMDTApplicationComponents/Include/acSendErrorReportDialog.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewActivatedEvent.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afActionCreatorAbstract.h>
#include <AMDTApplicationFramework/Include/afActionExecutorAbstract.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afCreatorAction.h>
#include <AMDTApplicationFramework/Include/afDocUpdateManager.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>
#include <AMDTApplicationFramework/Include/afQMdiSubWindow.h>
#include <AMDTApplicationFramework/Include/afQtViewCreatorAbstract.h>
#include <AMDTApplicationFramework/Include/afViewActionHandler.h>
#include <AMDTApplicationFramework/Include/afViewCreatorAbstract.h>
#include <AMDTApplicationFramework/Include/afViewCreatorAction.h>

// Local:
#include <inc/appApplication.h>
#include <inc/appMainAppWindow.h>

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::appMainAppWindow
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        27/7/2011
// ---------------------------------------------------------------------------
appMainAppWindow::appMainAppWindow():
    m_pMDIArea(nullptr),
    m_pSignalWindowMapper(nullptr),
    m_pFindNextAction(nullptr),
    m_pFindPrevAction(nullptr),
    m_pDialogBasedAssertionFailureHandler(nullptr),
    m_pSendErrorReportDialog(nullptr),
    m_pEventsObserver(nullptr),
    m_currentActionGlobalIndex(-1),
    m_pLastFocusedWidget(nullptr),
    m_isInCloseEvent(false),
    m_isFindWidgetInitialized(false)
{
    // Allocate an event observer:
    m_pEventsObserver = new appEventObserver;

    // Sets the MDI Area:
    m_pMDIArea = new appMDIArea;

    // Set the attributes for the Mdi area:
    m_pMDIArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_pMDIArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    if (appApplicationStart::ApplicationData().m_flagEnable.testFlag(AppCreate::APP_ENABLE_MDICENTRAL_VIEW))
    {
        setCentralWidget(m_pMDIArea);
    }

    // Create assertion handler dialog:
    createDialogBasedAssertionFailureHandler();

    // Create send error report dialog:
    createSendErrorReportDialog();

    // Connect the MDI window activation to menus update:
    bool rcConnect = connect(m_pMDIArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(OnMDIActivate(QMdiSubWindow*)));
    GT_ASSERT(rcConnect);

    m_pSignalWindowMapper = new QSignalMapper(this);

    rcConnect = connect(m_pSignalWindowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(onSetActiveSubWindow(QWidget*)));
    GT_ASSERT(rcConnect);

    // Set the view mode as tabbed:
    m_pMDIArea->setViewMode(QMdiArea::TabbedView);

    // The window should be maximized on activation:
    m_pMDIArea->setOption(QMdiArea::DontMaximizeSubWindowOnActivation, false);

    // Set window title:
    setWindowTitle(afGlobalVariablesManager::ProductNameA());

    // Set application icon:
    setApplicationIcon();

    // Create a status bar wrapper:
    QStatusBar* pStatusBar = statusBar();
    GT_IF_WITH_ASSERT(pStatusBar != nullptr)
    {
        // Initialize the progress bar:
        afProgressBarWrapper::instance().initialize(pStatusBar);
        unsigned int scaledHeight = acScalePixelSizeToDisplayDPI(AC_DEFAULT_LINE_HEIGHT);
        pStatusBar->setMaximumHeight((int)scaledHeight);
    }

    // Create the basic menu items for the application:
    initializeApplicationBasicMenus();

    // Initialize the main window:
    createApplicationMenus();

    // Create the mode manager toolbars so they will appear first:
    if (appApplicationStart::ApplicationData().m_initializeExecutionModesManager)
    {
        afExecutionModeManager::instance().createModeToolbar(this);
        afExecutionModeManager::instance().createActionsToolbar(this);
    }

    // Initialize the dynamic objects:
    initDynamicObjects();

    setUnifiedTitleAndToolBarOnMac(true);

    // Initialize the idle timer:
    rcConnect = connect(&m_idleTimer, SIGNAL(timeout()), this, SLOT(onIdleTimerTimeout()));
    GT_ASSERT(rcConnect);
    m_idleTimer.start(1000);
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::~appMainAppWindow
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        10/8/2011
// ---------------------------------------------------------------------------
appMainAppWindow::~appMainAppWindow()
{
    // stop idle timer:
    m_idleTimer.stop();

    // Delete the events observer:
    if (m_pEventsObserver != nullptr)
    {
        delete m_pEventsObserver;
        m_pEventsObserver = nullptr;
    }

    // Cleanup the application commands:
    afApplicationCommands::cleanupInstance();

    // Cleanup my own instance:
    _pMyStaticInstance = nullptr;
}


// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::initDynamicObjects
// Description: Initializes the application dynamic actions and view -
//             should be called after construction
// Author:      Sigal Algranaty
// Date:        10/8/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::initDynamicObjects()
{
    // Create the dynamic items:
    createApplicationInitialActions();

    // Create the frame views:
    createApplicationInitialViews();
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::setApplicationIcon
// Description: Set the main window icon
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/7/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::setApplicationIcon()
{
    // Set window icon:
    QPixmap iconPixMap;
    acSetIconInPixmap(iconPixMap, afGlobalVariablesManager::ProductIconID(), AC_64x64_ICON);
    setWindowIcon(QIcon(iconPixMap));
}


// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::createDialogBasedAssertionFailureHandler
// Description: Creates the dialog based asserting failure handler.
// Author:      Sigal Algranaty
// Date:        10/8/2011
// ---------------------------------------------------------------------------
bool appMainAppWindow::createDialogBasedAssertionFailureHandler()
{
    bool retVal = false;

    // Get the main GUI thread id:
    osThreadId mainGUIThreadId = osGetCurrentThreadId();

    // Create and register the dialog based assertion failure handler:
    m_pDialogBasedAssertionFailureHandler = new ahDialogBasedAssertionFailureHandler(mainGUIThreadId);

    // Register the dialog based assertion handler:
    gtRegisterAssertionFailureHandler(m_pDialogBasedAssertionFailureHandler);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::createSendErrorReportDialog
// Description: Creates the "Send Error Report" dialog.
// Author:      Sigal Algranaty
// Date:        13/2/2012
// ---------------------------------------------------------------------------
void appMainAppWindow::createSendErrorReportDialog()
{
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
    {
        // Create the Send Error Report dialog instance:
        QPixmap iconPixMap;
        acSetIconInPixmap(iconPixMap, afGlobalVariablesManager::ProductIconID(), AC_64x64_ICON);
        m_pSendErrorReportDialog = new acSendErrorReportDialog(this, afGlobalVariablesManager::ProductNameA(), iconPixMap);

        // Register it for receiving debugged process events:
        m_pSendErrorReportDialog->registerForRecievingDebuggedProcessEvents();
    }
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::onCloseEvent
// Description: Handle the window close event
// Arguments:   QCloseEvent *event
// Author:      Sigal Algranaty
// Date:        27/7/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::closeEvent(QCloseEvent* pEvent)
{
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != nullptr)
    {
        // Check if the user wants to exit while debugging:
        bool shouldExit = true;

        if (!m_whileExitingWindow)
        {
            // Do not prompt for exit:
            shouldExit = pApplicationCommands->promptForExit();
        }

        if (shouldExit)
        {
            // Save the current project:
            pApplicationCommands->OnFileSaveProject();

            if (m_pDialogBasedAssertionFailureHandler != nullptr)
            {
                // Unregister assertion handler:
                gtUnRegisterAssertionFailureHandler(m_pDialogBasedAssertionFailureHandler);
            }

            if ((m_pMDIArea != nullptr) && (pEvent != nullptr))
            {
                // Store the last layout in current configuration:
                gtString layoutName = getLayoutName(m_lastLayout);
                storeLayout(layoutName);

                m_pMDIArea->closeAllSubWindows();

                if (m_pMDIArea->currentSubWindow() != nullptr)
                {
                    shouldExit = false;
                }
            }
        }

        if (shouldExit)
        {
            pEvent->accept();

            m_isInCloseEvent = true;
            afMainAppWindow::closeEvent(pEvent);
        }
        else
        {
            pEvent->ignore();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::createApplicationMenus
// Description: Get the user's view menu, or creates new menu, and add the views commands
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        24/7/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::createApplicationMenus()
{
    // Create the view menu for the dynamic views created:
    QMenuBar* pMenuBar = menuBar();
    GT_IF_WITH_ASSERT(pMenuBar != nullptr)
    {
        // Get the view menu:
        gtString menuString = AF_STR_ViewMenuString;
        QString menuQtString = QString::fromWCharArray(menuString.asCharArray());

        m_pViewViewsMenu = getActionMenuItemParentMenu(menuString, nullptr, true);

        // If there is a view menu, create sub menu for the view items:
        if (m_pViewViewsMenu != nullptr)
        {
            // Create a new toolbar menu:
            m_pViewToolbarsMenu = new QMenu(AF_STR_ToolbarMenuStringA);

            // Add the view sub menu to the View menu:
            m_pViewViewsMenu->addMenu(m_pViewToolbarsMenu);

            // Connect the menu to its slots:
            bool rcConnect = connect(m_pViewViewsMenu, SIGNAL(aboutToShow()), this, SLOT(onAboutToShowViewMenu()));
            GT_ASSERT(rcConnect);

            // Connect the menu to its slots:
            rcConnect = connect(m_pViewToolbarsMenu, SIGNAL(aboutToShow()), this, SLOT(onAboutToShowToolbarsMenu()));
            GT_ASSERT(rcConnect);
        }

        // Get the windows menu:
        menuString = AF_STR_WindowsMenuString;

        m_pWindowsMenu = getActionMenuItemParentMenu(menuString, nullptr, true);

        // If there is a view menu, create sub menu for the view items:
        if (m_pWindowsMenu != nullptr)
        {
            // Add a "Close All Documents" action:
            QAction* pCloseAllAction = new QAction(AF_STR_CloseAllDocuments, this);
            pCloseAllAction->setObjectName(AF_STR_CloseAllDocuments);

            // Connect the menu to its slots:
            bool rcConnect = connect(pCloseAllAction, SIGNAL(triggered()), m_pMDIArea, SLOT(closeAllSubWindows()));
            GT_ASSERT(rcConnect);

            // Add the action to the windows menu:
            m_pWindowsMenu->addAction(pCloseAllAction);
            m_pWindowsMenu->addSeparator();

            // Connect the menu to its slots:
            rcConnect = connect(m_pWindowsMenu, SIGNAL(aboutToShow()), this, SLOT(onAboutToShowWindowsMenu()));
            GT_ASSERT(rcConnect);

        }
    }
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::createApplicationInitialActions
// Description: Create actions using the actions creators
// Author:      Gilad Yarnitzky
// Date:        14/7/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::createApplicationInitialActions()
{
    // Create the actions:
    // Get the actions creator:
    gtVector<afActionExecutorAbstract*>& actionsExecutors = afQtCreatorsManager::instance().actionsCreators();

    // Get number of creators:
    int numberActionsExecutors = actionsExecutors.size();

    // Initialize all creators:
    for (int actionExecutorIndex = 0 ; actionExecutorIndex < numberActionsExecutors; actionExecutorIndex++)
    {
        // Get the current action creator:
        afActionExecutorAbstract* pCurrentExecutor = actionsExecutors[actionExecutorIndex];

        GT_IF_WITH_ASSERT(pCurrentExecutor != nullptr)
        {
            // Initialize the creator before it starts creating the actions:
            pCurrentExecutor->initializeCreator();
        }
    }

    // Create all the actions in position "start":
    for (int actionExecutorIndex = 0 ; actionExecutorIndex < numberActionsExecutors; actionExecutorIndex++)
    {
        // Get the current action creator:
        afActionExecutorAbstract* pCurrentExecutor = actionsExecutors[actionExecutorIndex];

        GT_IF_WITH_ASSERT(pCurrentExecutor != nullptr)
        {
            // For each view creator pass through all views that can be created by it:
            int numberCurrentActionsCreated = pCurrentExecutor->numberActions();

            for (int currentActionCreated = 0 ; currentActionCreated < numberCurrentActionsCreated; currentActionCreated++)
            {
                // Create the current start block actions:
                createSingleAction(pCurrentExecutor, currentActionCreated, afActionPositionData::AF_POSITION_MENU_START_BLOCK);
            }
        }
    }

    // Create all the actions in position "middle":
    for (int actionExecutorIndex = 0 ; actionExecutorIndex < numberActionsExecutors; actionExecutorIndex++)
    {
        // Get the current action creator:
        afActionExecutorAbstract* pCurrentExecutor = actionsExecutors[actionExecutorIndex];

        GT_IF_WITH_ASSERT(pCurrentExecutor != nullptr)
        {
            // For each view creator pass through all views that can be created by it:
            int numberCurrentActionsCreated = pCurrentExecutor->numberActions();

            for (int currentActionCreated = 0 ; currentActionCreated < numberCurrentActionsCreated; currentActionCreated++)
            {
                // Create the current action:
                createSingleAction(pCurrentExecutor, currentActionCreated, afActionPositionData::AF_POSITION_MENU_CENTER_BLOCK);
            }
        }
    }

    // Create all the actions in position "end":
    for (int actionExecutorIndex = 0 ; actionExecutorIndex < numberActionsExecutors; actionExecutorIndex++)
    {
        // Get the current action creator:
        afActionExecutorAbstract* pCurrentExecutor = actionsExecutors[actionExecutorIndex];

        GT_IF_WITH_ASSERT(pCurrentExecutor != nullptr)
        {
            // For each view creator pass through all views that can be created by it:
            int numberCurrentActionsCreated = pCurrentExecutor->numberActions();

            for (int currentActionCreated = 0 ; currentActionCreated < numberCurrentActionsCreated; currentActionCreated++)
            {
                // Create the current action:
                createSingleAction(pCurrentExecutor, currentActionCreated, afActionPositionData::AF_POSITION_MENU_END_BLOCK);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::createSingleAction
// Description: Create the action at a specific index
// Arguments:   afActionCreatorAbstract* pCreator
//              int actionLocalIndex
//              afCommandPosition positionType - create action only if it is of the requested position type
// Author:      Gilad Yarnitzky
// Date:        17/7/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::createSingleAction(afActionExecutorAbstract* pActionExecutor, int actionLocalIndex, afActionPositionData::afCommandPosition positionType)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pActionExecutor != nullptr)
    {
        // The new action for creation:
        afCreatorAction* pNewAction = nullptr;

        // Get the menu position data:
        afActionPositionData positionData;
        pActionExecutor->menuPosition(actionLocalIndex, positionData);

        // Create only actions with the requested position type:
        if (positionData.m_actionPosition == positionType)
        {
            // Get the icon caption:
            gtString actionCaption, actionTooltip, actionKeyboardShortcut;
            bool rc = pActionExecutor->actionText(actionLocalIndex, actionCaption, actionTooltip, actionKeyboardShortcut);
            GT_IF_WITH_ASSERT(rc)
            {
                // Create the action:
                pNewAction = new afCreatorAction(actionCaption, this, pActionExecutor);

                // Set the new actions attributes:
                rc = setNewActionProperties(pActionExecutor, actionLocalIndex, pNewAction);
                GT_ASSERT(rc);

                // Set the action global index:
                pNewAction->setActionGlobalIndex(m_currentActionGlobalIndex);

                // Increase the actions global index:
                m_currentActionGlobalIndex++;
            }

            // If this is the find next action, get its pointer:
            if (actionCaption == AF_STR_FindNext)
            {
                m_pFindNextAction = pNewAction;
            }

            // If this is the find prev action, get its pointer:
            if (actionCaption == AF_STR_FindPrev)
            {
                m_pFindPrevAction = pNewAction;
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::createSingleViewAction
// Description: Create a single view action
// Arguments:    afViewCreatorAbstract* pViewCreator
//              int actionLocalIndex
// Author:      Sigal Algranaty
// Date:        1/9/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::createSingleViewAction(afViewCreatorAbstract* pViewCreator, int actionLocalIndex, afActionPositionData::afCommandPosition positionType)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pViewCreator != nullptr)
    {
        // Get the action creator from the view:
        afActionCreatorAbstract* pActionCreator = pViewCreator->actionCreator();
        GT_IF_WITH_ASSERT(pActionCreator != nullptr)
        {
            // The new action for creation:
            afViewCreatorAction* pNewAction = nullptr;

            afActionPositionData positionData;
            gtString actionMenuPosition = pActionCreator->menuPosition(actionLocalIndex, positionData);

            if (positionData.m_actionPosition == positionType)
            {
                // Get the icon caption:
                gtString actionCaption, actionTooltip, actionKeyboardShortcut;
                bool rc = pActionCreator->actionText(actionLocalIndex, actionCaption, actionTooltip, actionKeyboardShortcut);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Find the action within the existing actions:
                    QAction* pExistingAction = findActionByText(actionMenuPosition, actionCaption, false);

                    if (pExistingAction != nullptr)
                    {
                        // Try to cast to a view action;
                        pNewAction = qobject_cast<afViewCreatorAction*>(pExistingAction);
                        GT_IF_WITH_ASSERT(pNewAction != nullptr)
                        {
                            // Set the action details:
                            pActionCreator->setAction(pNewAction, actionLocalIndex, pNewAction->actionGlobalIndex());
                        }
                    }

                    // If the action was not created yet:
                    if (pNewAction == nullptr)
                    {
                        // Create the action:
                        pNewAction = new afViewCreatorAction(actionCaption, this);

                        // Set the new actions attributes:
                        rc = setNewActionProperties(pActionCreator, actionLocalIndex, pNewAction);
                        GT_ASSERT(rc);

                        // Set the action global index:
                        pNewAction->setActionGlobalIndex(m_currentActionGlobalIndex);
                    }
                }

                // If this is the find next action, get its pointer:
                if (actionCaption == AF_STR_FindNext)
                {
                    m_pFindNextAction = pNewAction;
                }

                // If this is the find prev action, get its pointer:
                if (actionCaption == AF_STR_FindPrev)
                {
                    m_pFindPrevAction = pNewAction;
                }
            }
        }
    }
}



// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::setNewActionProperties
// Description: Set the new actions attributes
// Arguments:   afActionCreatorAbstract* pActionCreator - the action creator
//              actionLocalIndex - the action index within the creator
//              pNewAction - the new action
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/9/2011
// ---------------------------------------------------------------------------
bool appMainAppWindow::setNewActionProperties(afActionCreatorAbstract* pActionCreator, int actionLocalIndex, QAction* pNewAction)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pActionCreator != nullptr)
    {
        // Get the action icon:
        bool shouldUseIconInMenu = false;
        QPixmap* pPixmap = pActionCreator->iconAsPixmap(actionLocalIndex, shouldUseIconInMenu);

        // Set the icon (if there is):
        if (pPixmap != nullptr)
        {
            pNewAction->setIcon(QIcon(*pPixmap));
        }

        gtString actionCaption, actionTooltip, actionKeyboardShortcut;
        pActionCreator->actionText(actionLocalIndex, actionCaption, actionTooltip, actionKeyboardShortcut);

        // Set the keyboard shortcut:
        if (!actionKeyboardShortcut.isEmpty())
        {
            QList<QKeySequence> shortcuts;
            QString qShortcut = QString::fromWCharArray(actionKeyboardShortcut.asCharArray());

            // Create a key shortcut from the string:
            QKeySequence keySeauence(qShortcut);
            shortcuts.append(keySeauence);

            // Set the shortcuts:
            pNewAction->setShortcuts(shortcuts);

            // Sset context to application context:
            pNewAction->setShortcutContext(Qt::ApplicationShortcut);
        }

        // Set the tooltip:
        if (!actionTooltip.isEmpty())
        {
            pNewAction->setToolTip(QString::fromWCharArray(actionTooltip.asCharArray()));
        }

        // Create menu item:
        afActionPositionData positionData;
        gtString menuString = pActionCreator->menuPosition(actionLocalIndex, positionData);

        // Look for the action before the requested one:
        QAction* pBeforeAction = findActionByText(positionData.m_beforeActionMenuPosition, positionData.m_beforeActionText, true);

        // Get the "real" before action:
        pBeforeAction = getRealBeforeAction(pBeforeAction);

        // Get or created action menu item parent menu:
        QMenu* pActionMenuItemParent = getActionMenuItemParentMenu(menuString, pBeforeAction, true);
        GT_IF_WITH_ASSERT(pActionMenuItemParent != nullptr)
        {
            // Add separator before the action group:
            if (afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND_GROUP == positionData.m_actionSeparatorType)
            {
                QMenu* pMenu = qobject_cast<QMenu*>(pActionMenuItemParent->parent());
                GT_IF_WITH_ASSERT(pMenu != nullptr)
                {
                    pMenu->insertSeparator(pActionMenuItemParent->menuAction());
                }
            }

            // Check if the action before was created with a separator before it:
            QList<QAction*> menuActions = pActionMenuItemParent->actions();
            int beforeActionIndex = menuActions.indexOf(pBeforeAction, 0);

            if (beforeActionIndex > 0)
            {
                QAction* pBeforeBeforeAction = menuActions[beforeActionIndex - 1];

                if (pBeforeBeforeAction != nullptr)
                {
                    if (pBeforeBeforeAction->isSeparator())
                    {
                        pBeforeAction = pBeforeBeforeAction;
                    }
                }
            }


            // Add the action to the menu item parent:
            pActionMenuItemParent->insertAction(pBeforeAction, pNewAction);

            // Add separator before action:
            if (afActionPositionData::AF_SEPARATOR_BEFORE_COMMAND == positionData.m_actionSeparatorType)
            {
                pActionMenuItemParent->insertSeparator(pNewAction);
            }

            // Set the action's parent:
            pNewAction->setParent(pActionMenuItemParent);

            // Increase the actions global index:
            m_currentActionGlobalIndex++;

            // Connect the action:
            pActionCreator->setAction(pNewAction, actionLocalIndex, m_currentActionGlobalIndex);

            // Connect the action:
            bool rcConnect = connect(pNewAction, SIGNAL(triggered()), this, SLOT(onActionTriggered()));
            GT_ASSERT(rcConnect);


        }

        // Create toolbar item:
        gtString toolbarString = pActionCreator->toolbarPosition(actionLocalIndex);

        if (!toolbarString.isEmpty())
        {
            acToolBar* pActionToolbarParent = getToolbar(toolbarString);
            GT_IF_WITH_ASSERT(pActionToolbarParent != nullptr)
            {
                // Create a tool button:
                QToolButton* pToolButton = new QToolButton(pActionToolbarParent);

                // Set the action:
                pToolButton->setDefaultAction(pNewAction);

                // Add the action to the menu item parent:
                QAction* pToolbarAction = pActionToolbarParent->addWidget(pToolButton);
                int nPosition = pActionCreator->separatorPosition(actionLocalIndex);

                if (nPosition == 1)
                {
                    pActionToolbarParent->addSeparator();
                }

                GT_ASSERT(pToolbarAction != nullptr);
            }
        }

        // Ask the creator to group the action:
        pActionCreator->groupAction(actionLocalIndex);

        retVal = true;

    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::getToolbar
// Description: Get the toolbar with the specific name, create one if needed
// Arguments:   gtString& toolbarName
// Author:      Gilad Yarnitzky
// Date:        17/7/2011
// ---------------------------------------------------------------------------
acToolBar* appMainAppWindow::getToolbar(gtString& toolbarName)
{
    acToolBar* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(!toolbarName.isEmpty())
    {
        // Convert name to QString:
        QString toolbarNameQt(QString::fromWCharArray(toolbarName.asCharArray()));
        toolbarNameQt.append(AF_STR_toolbarPostfix);

        // Get all tool bars:
        QList<acToolBar*> toolbarsList = findChildren<acToolBar*>(toolbarNameQt);

        // Check if one of them already have the toolbar name:
        for (int currentToolbar = 0; currentToolbar < toolbarsList.size(); currentToolbar++)
        {
            GT_IF_WITH_ASSERT(toolbarsList[currentToolbar] != nullptr)
            {
                if (toolbarsList[currentToolbar]->objectName().compare(toolbarNameQt) == 0)
                {
                    pRetVal = toolbarsList[currentToolbar];
                }
            }
        }

        // If toolbar not found, create one:
        if (pRetVal == nullptr)
        {
            // Create the toolbar:
            pRetVal = new acToolBar(this, toolbarNameQt);
            pRetVal->setObjectName(toolbarNameQt);

            // Add the toolbar:
            addToolbar(pRetVal);
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        amdMainAppWindow::createViews
// Description: Create the views to be used in the main window dynamically
//              The views are created by the views creators that registered
// Author:      Gilad Yarnitzky
// Date:        13/7/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::createApplicationInitialViews()
{
    // Get the views creators:
    gtVector<afViewCreatorAbstract*>& viewsCreators = afQtCreatorsManager::instance().viewsCreators();

    // Get number of creators:
    int numberViewsCreators = (int)viewsCreators.size();

    for (int viewCreatorIndex = 0 ; viewCreatorIndex < numberViewsCreators; viewCreatorIndex++)
    {
        afViewCreatorAbstract* pCurrentCreator = viewsCreators[viewCreatorIndex];
        GT_IF_WITH_ASSERT(pCurrentCreator != nullptr)
        {
            // If the creator is used for creating view in the initialization, create the views now:
            if (!pCurrentCreator->isDynamic())
            {
                // For each view creator pass through all views that can be created by it:
                int numberCurrentViewsCreated = pCurrentCreator->amountOfViewTypes();

                for (int currentViewCreated = 0 ; currentViewCreated < numberCurrentViewsCreated; currentViewCreated++)
                {
                    // Create the 'viewCreatorIndex' view:
                    createSingleView(pCurrentCreator, currentViewCreated);
                }

                // Raise all views that should be initially active:
                for (int currentViewActivated = 0 ; currentViewActivated < numberCurrentViewsCreated; currentViewActivated++)
                {
                    // If the current view is active by default:
                    if (pCurrentCreator->initiallyActive(currentViewActivated))
                    {
                        // Get the views by its index:
                        QWidget* pWidgetToRaise = pCurrentCreator->widget(currentViewActivated);
                        GT_IF_WITH_ASSERT(nullptr != pWidgetToRaise)
                        {
                            pWidgetToRaise->raise();
                        }
                    }
                }
            }
        }

        // Create all the actions related to this view creator. Actions are created both for dynamic and static view creators:
        afActionCreatorAbstract* pViewActionCreator = pCurrentCreator->actionCreator();

        // Create all the actions for this creator:
        if (pViewActionCreator != nullptr)
        {
            // For each view creator pass through all views that can be created by it:
            int numberCurrentActionsCreated = pViewActionCreator->numberActions();

            for (int currentActionCreated = 0 ; currentActionCreated < numberCurrentActionsCreated; currentActionCreated++)
            {
                // Create the current action:
                createSingleViewAction(pCurrentCreator, currentActionCreated, afActionPositionData::AF_POSITION_MENU_START_BLOCK);
            }

            for (int currentActionCreated = 0 ; currentActionCreated < numberCurrentActionsCreated; currentActionCreated++)
            {
                // Create the current action:
                createSingleViewAction(pCurrentCreator, currentActionCreated, afActionPositionData::AF_POSITION_MENU_CENTER_BLOCK);
            }

            for (int currentActionCreated = 0 ; currentActionCreated < numberCurrentActionsCreated; currentActionCreated++)
            {
                // Create the current action:
                createSingleViewAction(pCurrentCreator, currentActionCreated, afActionPositionData::AF_POSITION_MENU_END_BLOCK);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::createSingleView
// Description: Create a single view of a creator
// Arguments:   afViewCreatorAbstract* pCreator - the object used for creating the view
//              int viewIndex
// Author:      Sigal Algranaty
// Date:        22/8/2011
// ---------------------------------------------------------------------------
QWidget* appMainAppWindow::createSingleView(afViewCreatorAbstract* pCreator, int viewIndex)
{
    QWidget* pRetVal = nullptr;

    // Sanity check:
    GT_IF_WITH_ASSERT(pCreator != nullptr)
    {
        // Create the QT widget:
        QWidget* pQTWidget = nullptr;
        bool contentWasCreated = pCreator->createViewContent(viewIndex, pQTWidget, nullptr);

        // Set the minimal size of the view if it is a dock view
        afViewCreatorAbstract::afViewType viewType = pCreator->type(viewIndex);

        if (nullptr != pQTWidget && viewType == afViewCreatorAbstract::AF_VIEW_dock)
        {
            QSize initSize = pCreator->initialSize(viewIndex);
            pQTWidget->setMinimumSize(initSize);
        }

        // Create the QT window that is wrapping the widget:
        GT_IF_WITH_ASSERT(pQTWidget != nullptr)
        {
            pRetVal = createQTWindowForView(viewIndex, pCreator, pQTWidget);
            GT_IF_WITH_ASSERT(pRetVal)
            {
                // Create the view that is going to be wrapper:
                if (!contentWasCreated)
                {
                    QWidget* pContentWidget = nullptr;
                    contentWasCreated = pCreator->createViewContent(viewIndex, pContentWidget, pRetVal);
                }

                GT_IF_WITH_ASSERT((pRetVal != nullptr) && contentWasCreated)
                {
                    // Get the icon pixmap:
                    QPixmap* pIconPixmap = pCreator->iconAsPixmap(viewIndex);

                    if (pIconPixmap != nullptr)
                    {
                        pRetVal->setWindowIcon(*pIconPixmap);
                    }
                    else
                    {
                        // Set a default window icon:
                        QPixmap icon16;
                        acSetIconInPixmap(icon16, afGlobalVariablesManager::ProductIconID(), AC_16x16_ICON);
                        pRetVal->setWindowIcon(icon16);
                    }

                    // Get the view type
                    viewType = pCreator->type(viewIndex);

                    if (viewType == afViewCreatorAbstract::AF_VIEW_mdi)
                    {
                        // Get the view initial size:
                        QSize initSize = pCreator->initialSize(viewIndex);

                        // If it is an MDI window, use the MdiArea size as the real size:
                        initSize.setWidth(m_pMDIArea->size().width());
                        initSize.setHeight(m_pMDIArea->size().height());

                        pRetVal->resize(initSize.width(), initSize.height());
                    }

                    bool rcAddViewAction = addActionForView(pCreator, viewIndex, pRetVal);
                    GT_ASSERT(rcAddViewAction);
                    // Set visibility hidden if needed:
                    bool isVisible = pCreator->visibility(viewIndex);

                    // Set the item visibility:
                    pRetVal->setVisible(isVisible);
                    pQTWidget->setVisible(true);

                    // Update the widget:
                    pRetVal->update();
                    m_pMDIArea->update();
                }
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::createQTWindowForView
// Description: Create a widget that creates a QT wrapping to the containing
//              widget according to the creator view type (MDI / Dock)
// Arguments:   afViewCreatorAbstract* pViewCreator - the view creator
//              QWidget* pContainedWidget - the QT widget for the window content
// Return Val:  QWidget* - the wrapping (dock widget / sub window)
// Author:      Sigal Algranaty
// Date:        22/8/2011
// ---------------------------------------------------------------------------
QWidget* appMainAppWindow::createQTWindowForView(int viewIndex, afViewCreatorAbstract* pViewCreator, QWidget* pContainedWidget)
{
    QWidget* pRetVal = nullptr;

    // Get the view name:
    gtString viewName, viewCommandName;
    pViewCreator->titleString(viewIndex, viewName, viewCommandName);

    // Transfer the name to Qt string:
    QString viewNameQt(QString::fromWCharArray(viewName.asCharArray()));

    // Add the wrapper to the main frame based on its type:
    afViewCreatorAbstract::afViewType viewType = pViewCreator->type(viewIndex);

    switch (viewType)
    {
        case afViewCreatorAbstract::AF_VIEW_dock:
        {
            // Create a new doc widget:
            QDockWidget* pDockWidget = new QDockWidget(viewNameQt, this);

            // Set the widget features:
            QDockWidget::DockWidgetFeatures widgetFeatures = pViewCreator->dockWidgetFeatures(viewIndex);
            pDockWidget->setFeatures(widgetFeatures);

            // Set the wrapper as the widget:
            pDockWidget->setWidget(pContainedWidget);

            // Get the widget docking area:
            Qt::DockWidgetArea docArea = Qt::DockWidgetArea(pViewCreator->dockArea(viewIndex));

            // Add the doc widget:
            addDockWidget(docArea, pDockWidget);
            pDockWidget->setObjectName(viewNameQt);

            // Combine docks together if needed:
            dockWithOthers(viewIndex, pViewCreator, pDockWidget);

            pRetVal = pDockWidget;

        }
        break;

        case afViewCreatorAbstract::AF_VIEW_mdi:
        {
            GT_IF_WITH_ASSERT(m_pMDIArea != nullptr)
            {
                // Set the sub window file path:
                osFilePath filePath;
                bool rc = pViewCreator->getCurrentlyDisplayedFilePath(filePath);
                GT_ASSERT(rc);

                // Check if the window is already created:
                afQMdiSubWindow* pExistingSubwindow = findMDISubWindow(filePath);

                if (pExistingSubwindow == nullptr)
                {
                    // Check if the window is already created:
                    pExistingSubwindow = new afQMdiSubWindow(pViewCreator);

                    // Set an empty temporary icon:
                    setWindowIcon(QIcon());

                    // Delete the sub window on close:
                    pExistingSubwindow->setAttribute(Qt::WA_DeleteOnClose);

                    // Set the sub window wrapper:
                    pExistingSubwindow->setWidget(pContainedWidget);

                    // Set the window title:
                    pExistingSubwindow->setWindowTitle(viewNameQt);

                    // Add a sub window:
                    Qt::WindowFlags windowFlags = Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint | Qt::SubWindow;
                    m_pMDIArea->addSubWindow(pExistingSubwindow, windowFlags);

                    pExistingSubwindow->show();
                    // some force updating is needed if the creation is due to an event so the
                    // next two lines do that:
                    pContainedWidget->raise();
                    pContainedWidget->activateWindow();

                    // Hide the view so it can be updated without the user seeing it:
                    pContainedWidget->setVisible(false);

                    // Set the window file path:
                    pExistingSubwindow->setFilePath(filePath);

                    // Get the associated toolbar name from creator:
                    gtString toolbar = pViewCreator->associatedToolbar(viewIndex);

                    // Set the toolbar name:
                    pExistingSubwindow->setAssociatedToolbarName(toolbar);

                    // make sure all mdi views have close button:
                    Q_FOREACH (QTabBar* tab, m_pMDIArea->findChildren<QTabBar*>())
                    {
                        if (tab->parent() == m_pMDIArea && !tab->tabsClosable())
                        {
                            tab->setTabsClosable(true);
                        }
                    }
                }

                // bug 8379: in order to bypass the Qt bug for one time only we reactivate the previous view
                // The previous view must be visible for this to work. If it is not then this is not considered done
                // and this actions need to be tested again next time when an MDI view is created
                // This needs to be done only when there are only up to 3 views (probably only 2 of them are visible
                // another Qt problem: when all views are closed in some cases one is still kept as a child)
                static bool firstTime = true;

                QList<QMdiSubWindow*> subWindowsList = findChildren<QMdiSubWindow*>();

                if (subWindowsList.count() >= 2 && subWindowsList.count() < 4 && firstTime)
                {
                    if (subWindowsList[subWindowsList.count() - 2]->isVisible())
                    {
                        firstTime = false;
                        m_pMDIArea->setActiveSubWindow(subWindowsList[subWindowsList.count() - 2]);
                    }
                }

                // Set the sub window as the widget to display:
                pRetVal = pExistingSubwindow;
            }
        }
        break;

        default:
            GT_ASSERT_EX(false, L"invalid view type");
            break;
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::dockWithOthers
// Description:
// Arguments:   int viewIndex
//              afViewCreatorAbstract* pViewCreator
//              QDockWidget* pDockWidget
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        15/2/2012
// ---------------------------------------------------------------------------
void appMainAppWindow::dockWithOthers(int viewIndex, afViewCreatorAbstract* pViewCreator, QDockWidget* pDockWidget)
{
    GT_IF_WITH_ASSERT(nullptr != pDockWidget)
    {
        // Get the name of the dock widget we want to dock with:
        gtString dockWith = pViewCreator->dockWith(viewIndex);

        // If there is a dock name to dock with, look for it:
        if (!dockWith.isEmpty())
        {
            // Find the order of joining the docks together:
            bool withWidgetAbove = false;

            if (dockWith.startsWith(L"-"))
            {
                // Set the order correctly and remove the order char from name
                withWidgetAbove = true;
                dockWith.getSubString(1, dockWith.length(), dockWith);
            }

            // Transfer the name to Qt string:
            QString dockWithQt(QString::fromWCharArray(dockWith.asCharArray()));

            // find if we have a child dock window with target name:
            QDockWidget* widgetFoundWithName = findChild<QDockWidget*>(dockWithQt);

            // Make the connection in the right order if we found one
            if (nullptr != widgetFoundWithName)
            {
                if (withWidgetAbove)
                {
                    tabifyDockWidget(pDockWidget, widgetFoundWithName);
                }
                else
                {
                    tabifyDockWidget(widgetFoundWithName, pDockWidget);
                }
            }
        }
    }

}


// ---------------------------------------------------------------------------
// Name:        *appMainAppWindow::activeMDISubWindow
// Description: Get the current active MDI child
// Return Val:  afQMdiSubWindow
// Author:      Sigal Algranaty
// Date:        28/7/2011
// ---------------------------------------------------------------------------
afQMdiSubWindow* appMainAppWindow::activeMDISubWindow()
{
    afQMdiSubWindow* pRetVal = nullptr;

    QMdiSubWindow* pActiveSubWindow = m_pMDIArea->activeSubWindow();

    // Down cast to afQMdiSubWindow:
    pRetVal = qobject_cast<afQMdiSubWindow*>(pActiveSubWindow);

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        *appMainAppWindow::findMDISubWindow
// Description: Find an MDI child by file name
// TO_DO: CodeXL frame - implement the association with file name
// Arguments:   const QString &fileName
// Return Val:  QMdiSubWindow
// Author:      Sigal Algranaty
// Date:        28/7/2011
// ---------------------------------------------------------------------------
afQMdiSubWindow* appMainAppWindow::findMDISubWindow(const osFilePath& filePath)
{
    afQMdiSubWindow* pRetVal = nullptr;
    GT_IF_WITH_ASSERT(m_pMDIArea != nullptr)
    {
        // Get the MDI sub windows list:
        QList<QMdiSubWindow*> windowsSubList = m_pMDIArea->subWindowList();

        foreach (QMdiSubWindow* pCurrentSubWindow, windowsSubList)
        {
            GT_IF_WITH_ASSERT(pCurrentSubWindow != nullptr)
            {
                // Get the widget from the window:
                afQMdiSubWindow* pAfQTSubWindow = qobject_cast<afQMdiSubWindow*>(pCurrentSubWindow);

                if (pAfQTSubWindow != nullptr)
                {
                    // Compare file names:
                    if (pAfQTSubWindow->filePath() == filePath)
                    {
                        pRetVal = pAfQTSubWindow;
                        break;
                    }
                }
            }
        }
    }
    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::addMDISubWindow
// Description: Add an MDI sub window
// Arguments:   const osFilePath &fileName
// Return Val:  afQMdiSubWindow*
// Author:      Sigal Algranaty
// Date:        27/2/2012
// ---------------------------------------------------------------------------
afQMdiSubWindow* appMainAppWindow::addMDISubWindow(const osFilePath& filePath, QWidget* pWidget, const QString& caption)
{
    afQMdiSubWindow* pRetVal = nullptr;

    // Sanity check
    GT_IF_WITH_ASSERT(m_pMDIArea != nullptr)
    {
        // Check if the window is already created:
        pRetVal = new afQMdiSubWindow(nullptr);

        // Delete the sub window on close:
        pRetVal->setAttribute(Qt::WA_DeleteOnClose);

        // Set the sub window wrapper:
        pRetVal->setWidget(pWidget);

        // Set the window icon:
        QPixmap icon16;
        acIconId invertedIconId = (acIconId)(afGlobalVariablesManager::ProductIconID() + 1);
        acSetIconInPixmap(icon16, invertedIconId);
        pRetVal->setWindowIcon(icon16);

        // Set the window title:
        pRetVal->setWindowTitle(caption);

        // Add a sub window:
        Qt::WindowFlags windowFlags = Qt::WindowCloseButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowMinimizeButtonHint | Qt::SubWindow;
        m_pMDIArea->addSubWindow(pRetVal, windowFlags);

        pRetVal->show();

        // some force updating is needed if the creation is due to an event so the
        // next two lines do that:
        pWidget->raise();
        pWidget->activateWindow();

        // Hide the view so it can be updated without the user seeing it:
        pWidget->setVisible(false);

        // Set the window file path:
        pRetVal->setFilePath(filePath);

        // make sure all mdi views have close button:
        Q_FOREACH (QTabBar* tab, m_pMDIArea->findChildren<QTabBar*>())
        {
            if (tab->parent() == m_pMDIArea && !tab->tabsClosable())
            {
                tab->setTabsClosable(true);
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::onSetActiveSubWindow
// Description: Activate the window
// Arguments:   QWidget *pWindow
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        31/7/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::onSetActiveSubWindow(QWidget* pWindow)
{
    if (pWindow)
    {
        m_pMDIArea->setActiveSubWindow(qobject_cast<QMdiSubWindow*>(pWindow));
    }
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::onAboutToShowWindowsMenu
// Description: Sends update notifications to all windows menu actions
// Author:      Sigal Algranaty
// Date:        19/9/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::onAboutToShowWindowsMenu()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pWindowsMenu != nullptr)
    {
        // Get the windows menu actions:
        QList<QAction*> windowsActions = m_pWindowsMenu->actions();
        GT_IF_WITH_ASSERT(windowsActions.size() > 0)
        {
            // Get the close all documents action:
            QAction* pCloseAllDocmentsAction = windowsActions[0];
            GT_IF_WITH_ASSERT(pCloseAllDocmentsAction != nullptr)
            {
                GT_IF_WITH_ASSERT(m_pMDIArea != nullptr)
                {
                    // Check if the close all documents action be enabled:
                    bool isEnabled = (m_pMDIArea->subWindowList().size() > 0);
                    pCloseAllDocmentsAction->setEnabled(isEnabled);
                }
            }

            // Look for the action for the active window:
            afQMdiSubWindow* pActiveSubWindow = activeMDISubWindow();

            if (pActiveSubWindow != nullptr)
            {
                // Get the file path:
                QString filePathStr = acGTStringToQString(pActiveSubWindow->filePath().asString());

                foreach (QAction* pAction, windowsActions)
                {
                    if ((pAction != nullptr) && (pCloseAllDocmentsAction != pAction))
                    {
                        pAction->setCheckable(true);
                        pAction->setChecked(filePathStr == pAction->text());
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::onAboutToShowMenu
// Description: Sends update notifications to all actions
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::onAboutToShowMenu()
{
    // Get the menu that triggered the event:
    QObject* pSender = sender();
    GT_IF_WITH_ASSERT(pSender != nullptr)
    {
        // Update the senders child commands:
        updateSingleObjectCommands(pSender);
    }
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::onAboutToShowToolbarsMenu
// Description: Enable / disable the relevant toolbar actions
// Author:      Sigal Algranaty
// Date:        22/2/2012
// ---------------------------------------------------------------------------
void appMainAppWindow::onAboutToShowToolbarsMenu()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pViewToolbarsMenu != nullptr)
    {
        QList<QAction*> toolbarsActions = m_pViewToolbarsMenu->actions();

        foreach (QAction* pAction, toolbarsActions)
        {
            if (pAction != nullptr)
            {
                pAction->setCheckable(true);
                QString toolbarName = pAction->text();
                toolbarName.remove('&');

                if (!toolbarName.isEmpty())
                {
                    acToolBar* pToolbar = findChild<acToolBar*>(toolbarName);

                    if (pToolbar != nullptr)
                    {
                        pAction->setChecked(pToolbar->isVisible());
                    }
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::onAboutToShowViewMenu
// Description: Enable / disable the relevant view actions
// Author:      Sigal Algranaty
// Date:        27/9/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::onAboutToShowViewMenu()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pViewViewsMenu != nullptr)
    {
        // Find the relevant view handler:
        for (int i = 0; i < (int)m_viewsActionHandler.size(); i++)
        {
            afViewActionHandler* pViewHandler = m_viewsActionHandler[i];
            GT_IF_WITH_ASSERT(pViewHandler != nullptr)
            {
                pViewHandler->onUpdateUI();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::updateSingleObjectCommands
// Description: Updates all the commands that are the children of the input object
// Author:      Sigal Algranaty
// Date:        1/8/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::updateSingleObjectCommands(QObject* pObject)
{
    // Get the menu that triggered the event:
    GT_IF_WITH_ASSERT(pObject != nullptr)
    {
        // Get all afCreatorActions of the menu:
        QObjectList itemsList = pObject->children();

        for (int actionNum = 0 ; actionNum < itemsList.count() ; actionNum++)
        {
            // Get current creator action from the list:
            QObject* pCurrentItem = itemsList[actionNum];

            updateSingleAction(pCurrentItem);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::updateSingleAction
// Description: update a single action based on its type
// Arguments:   QObject* pCurrentItem
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        31/5/2012
// ---------------------------------------------------------------------------
void appMainAppWindow::updateSingleAction(QObject* pAction)
{
    // Check if this is a view action:
    bool isViewAction = (strcmp(pAction->metaObject()->className(), "afViewCreatorAction") == 0);

    if (isViewAction)
    {
        updateViewAction(pAction);
    }
    else
    {
        updateGlobalAction(pAction);
    }

}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::updateViewAction
// Description: Update UI for a single view action
// Arguments:   QObject* pAction
// Author:      Sigal Algranaty
// Date:        5/9/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::updateViewAction(QObject* pAction)
{
    // Get the creator for the currently active widget:
    int viewIndex = -1;
    afViewCreatorAbstract* pActiveViewCreator = findFocusedWidgetViewCreator(viewIndex);

    if (pActiveViewCreator != nullptr)
    {
        // Get the action creator:
        afActionCreatorAbstract* pActionCreator = pActiveViewCreator->actionCreator();

        // A view creator does not need to have an action creator:
        if (pActionCreator != nullptr)
        {
            // Down cast the action to a view action:
            afViewCreatorAction* pViewCreatorAction = qobject_cast<afViewCreatorAction*>(pAction);
            GT_IF_WITH_ASSERT(pViewCreatorAction != nullptr)
            {
                // Get the action index:
                int actionGlobalIndex = pViewCreatorAction->actionGlobalIndex();

                // Check if the action has a global id within the creator:
                int localIndex = pActionCreator->localFromGlobalActionIndex(actionGlobalIndex);

                if (localIndex >= 0)
                {
                    // Call the handler:
                    pActiveViewCreator->handleUiUpdate(viewIndex, localIndex);
                }
                else
                {
                    pViewCreatorAction->setEnabled(false);
                }
            }
        }
    }
    else
    {
        // Set the action disabled if no active view changed it's update properties:
        QAction* pActionObject = qobject_cast<QAction*>(pAction);

        if (pActionObject != nullptr)
        {
            pActionObject->setEnabled(false);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::updateGlobalAction
// Description: Update UI for a global action
// Arguments:   QObject* pCurrentItem
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        5/9/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::updateGlobalAction(QObject* pCurrentItem)
{
    QToolButton* pToolButton = qobject_cast<QToolButton*>(pCurrentItem);

    // check if the button was created by the execution mode manager:
    // Convert to afCreatorAction:
    afCreatorAction* pCreatorAction = nullptr;
    pCreatorAction = qobject_cast<afCreatorAction*>(pCurrentItem);

    if (pCreatorAction == nullptr)
    {
        // Convert to tool button:
        if (pToolButton != nullptr)
        {
            pCreatorAction = qobject_cast<afCreatorAction*>(pToolButton->defaultAction());
        }
    }

    if (pCreatorAction != nullptr)
    {
        // Get the action creator:
        afActionExecutorAbstract* pActionExecutor = pCreatorAction->actionExecutor();
        GT_IF_WITH_ASSERT(pActionExecutor != nullptr)
        {
            // Get the action index:
            int actionGlobalIndex = pCreatorAction->actionGlobalIndex();

            // Translate to local:
            int actionLocalIndex = pActionExecutor->localFromGlobalActionIndex(actionGlobalIndex);
            GT_IF_WITH_ASSERT(actionLocalIndex >= 0)
            {
                // Call the handler:
                pActionExecutor->handleUiUpdate(actionLocalIndex);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::onActionTriggered
// Description: Notify creator on action triggered event
// Author:      Gilad Yarnitzky
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::onActionTriggered()
{
    // Store focus in case we lose it:
    QWidget* pWindowThatHasFocus = qApp->activeWindow();

    // Get the sender and down cast to action:
    QAction* pAction = qobject_cast<QAction*>(sender());
    GT_IF_WITH_ASSERT(pAction != nullptr)
    {
        // Check if this is a view action:
        bool isViewAction = (strcmp(pAction->metaObject()->className(), "afViewCreatorAction") == 0);

        if (isViewAction)
        {
            triggerViewAction(pAction);
        }
        else
        {
            triggerGlobalAction(pAction);
        }
    }

    // Restore focus in case we lose it:
    if (!m_whileExitingWindow)
    {
        if (pWindowThatHasFocus && (qApp->activeWindow() != pWindowThatHasFocus))
        {
            pWindowThatHasFocus->setFocus(Qt::NoFocusReason);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::triggerGlobalAction
// Description: Trigger the action
// Arguments:   QObject* pAction
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        5/9/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::triggerGlobalAction(QObject* pAction)
{
    afCreatorAction* pCreatorAction = qobject_cast<afCreatorAction*>(pAction);

    if (pCreatorAction != nullptr)
    {
        // Get the action global index:
        int actionGlobalIndex = pCreatorAction->actionGlobalIndex();

        // Convert the action global index to a local one:
        afActionExecutorAbstract* pActionExecutor = pCreatorAction->actionExecutor();
        GT_IF_WITH_ASSERT(pActionExecutor)
        {
            // Get the local index:
            int actionLocalIndex = pActionExecutor->localFromGlobalActionIndex(actionGlobalIndex);
            GT_IF_WITH_ASSERT(actionLocalIndex >= 0)
            {
                pActionExecutor->handleTrigger(actionLocalIndex);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::triggerViewAction
// Description: Trigger a view action
// Arguments:   QObject* pAction
// Author:      Sigal Algranaty
// Date:        5/9/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::triggerViewAction(QObject* pAction)
{
    // Get the creator for the currently active widget:
    int viewIndex = -1;
    afViewCreatorAbstract* pActiveViewCreator = findFocusedWidgetViewCreator(viewIndex);

    if (pActiveViewCreator != nullptr)
    {
        // Get the action creator (if the view has one):
        afActionCreatorAbstract* pActionCreator = pActiveViewCreator->actionCreator();

        if (pActionCreator != nullptr)
        {
            // Down cast the action to a view action:
            afViewCreatorAction* pViewCreatorAction = qobject_cast<afViewCreatorAction*>(pAction);
            GT_IF_WITH_ASSERT(pViewCreatorAction != nullptr)
            {
                // Get the action index:
                int actionGlobalIndex = pViewCreatorAction->actionGlobalIndex();

                // Check if the action has a global id within the creator:
                int actionLocalIndex = pActionCreator->localFromGlobalActionIndex(actionGlobalIndex);

                if (actionLocalIndex >= 0)
                {
                    pActiveViewCreator->handleTrigger(viewIndex, actionLocalIndex);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::initializeStaticInstance
// Description: This function should be called only once
// Author:      Sigal Algranaty
// Date:        28/7/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::initializeStaticInstance()
{
    GT_IF_WITH_ASSERT(_pMyStaticInstance == nullptr)
    {
        _pMyStaticInstance = new appMainAppWindow;
    }
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::OnMDIActivate
// Description: Update visibility for toolbars when an MDI window is activated
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        28/7/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::OnMDIActivate(QMdiSubWindow* pActivatedWindow)
{
    if (pActivatedWindow != nullptr)
    {
        // Down cast to afQMdiSubWindow
        afQMdiSubWindow* pSubWindow = qobject_cast<afQMdiSubWindow*>(pActivatedWindow);

        if (pSubWindow != nullptr)
        {
            // Send the event that the window is activated
            if (!pSubWindow->filePath().isEmpty())
            {
                apMDIViewActivatedEvent activatedEvent(pSubWindow->filePath());
                apEventsHandler::instance().registerPendingDebugEvent(activatedEvent);
            }

            // update the associated document in the mdi view:
            afDocUpdateManager::instance().ActivateView(pSubWindow);

            // Check what should be the toolbars displayed with the current view:
            gtString toolbar = pSubWindow->associatedToolbarName();

            if (!toolbar.isEmpty())
            {
                // Get the toolbar:
                acToolBar* pToolbar = getToolbar(toolbar);
                GT_IF_WITH_ASSERT(pToolbar != nullptr)
                {
                    // Show the toolbar:
                    pToolbar->show();
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::updateToolbarsCommands
// Description: QT does not update toolbar actions automatically.
//              This function should be called when the action state is changed.
// Author:      Sigal Algranaty
// Date:        1/8/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::updateToolbarsCommands()
{
    // Get all tool bars:
    QList<acToolBar*> toolbarsList = findChildren<acToolBar*>();

    // Check if one of them already have the toolbar name:
    for (int i = 0; i < toolbarsList.size(); i++)
    {
        // Get the current toolbar:
        acToolBar* pToolbar = toolbarsList[i];
        GT_IF_WITH_ASSERT(pToolbar != nullptr)
        {
            // Get the toolbar actions:
            QList<QAction*> toolbarActions = pToolbar->findChildren<QAction*>();

            foreach (QAction* pAction, toolbarActions)
            {
                // Down cast to QWidgetAction:
                QWidgetAction* pWidgetAction = qobject_cast<QWidgetAction*>(pAction);

                if (pWidgetAction != nullptr)
                {
                    if (pWidgetAction->objectName().startsWith(AF_STR_modeToobarButtonBaseName))
                    {
                        afExecutionModeManager::instance().onUpdateToolbarUI(pWidgetAction);
                    }
                    else
                    {
                        // Downcast the widget to a pToolButton:
                        QToolButton* pToolButton = qobject_cast<QToolButton*>(pWidgetAction->defaultWidget());

                        if (pToolButton != nullptr)
                        {
                            if (pToolButton->objectName().startsWith(AF_STR_modeToobarButtonBaseName))
                            {
                                afExecutionModeManager::instance().onUpdateToolbarUI(pToolButton);
                            }
                            else
                            {
                                // Get the toolbar action from the widget:
                                QAction* pToolbarAction = pToolButton->defaultAction();

                                if (pToolbarAction != nullptr)
                                {
                                    // Update the current toolbar actions:
                                    // Check if this is a view action:
                                    bool isViewAction = (strcmp(pToolbarAction->metaObject()->className(), "afViewCreatorAction") == 0);

                                    if (isViewAction)
                                    {
                                        updateViewAction(pToolbarAction);
                                    }
                                    else
                                    {
                                        updateGlobalAction(pToolbarAction);
                                    }
                                }
                            }
                        }
                    }
                }
                else
                {
                    if (pAction->objectName().startsWith(AF_STR_modeToobarButtonBaseName))
                    {
                        // This Button not handled, this is an action button, get its id and pass it to the active execution mode:
                        afExecutionModeManager::instance().onUpdateToolbarUI(pAction);
                    }
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::updateToolbars
// Description: Update toolbar on MDI window open / close
// Author:      Sigal Algranaty
// Date:        1/8/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::updateToolbars()
{
    OnMDIActivate(activeMDISubWindow());
}

// ---------------------------------------------------------------------------
// Name:        afMainAppWindow::onSubWindowClose
// Description: A sub window is about to be close
// Arguments:   afQMdiSubWindow* pSubWindowAboutToBeClosed
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        10/8/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::onSubWindowClose(afQMdiSubWindow* pSubWindowAboutToBeClosed)
{
    GT_IF_WITH_ASSERT(pSubWindowAboutToBeClosed != nullptr)
    {
        if (!m_isInCloseEvent)
        {
            // Remove the sub window from windows menu:
            RemoveSubWindowFromWindowsMenu(pSubWindowAboutToBeClosed);

            // Remove the sub window from its creators:
            RemoveSubWindowFromViewCreators(pSubWindowAboutToBeClosed);

            // Get the widget from the sub window:
            if (pSubWindowAboutToBeClosed->widget() != nullptr)
            {
                pSubWindowAboutToBeClosed->widget()->close();
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::activateSubWindow
// Description: Activate the requested sub window
// Arguments:   afQMdiSubWindow* pSubWindow
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/8/2011
// ---------------------------------------------------------------------------
bool appMainAppWindow::activateSubWindow(afQMdiSubWindow* pSubWindow)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pMDIArea != nullptr)
    {
        // Set the MDI active sub window:
        m_pMDIArea->setActiveSubWindow(pSubWindow);
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::closeMDISubWindow
// Description: Close the requested sub window
// Arguments:   afQMdiSubWindow* pSubWindow
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/5/2012
// ---------------------------------------------------------------------------
bool appMainAppWindow::closeMDISubWindow(afQMdiSubWindow* pSubWindow)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pMDIArea != nullptr)
    {
        // Set the MDI active sub window:
        m_pMDIArea->setActiveSubWindow(pSubWindow);
        m_pMDIArea->closeActiveSubWindow();
        pSubWindow->close();
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::restoreMinimalSize
// Description: Go through the view creators and call the restore size function
// Author:      Sigal Algranaty
// Date:        23/8/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::restoreMinimalSize()
{
    // Get the views creators:
    gtVector<afViewCreatorAbstract*>& viewsCreators = afQtCreatorsManager::instance().viewsCreators();

    // Get number of creators:
    int numberViewsCreators = (int)viewsCreators.size();

    for (int viewCreatorIndex = 0 ; viewCreatorIndex < numberViewsCreators; viewCreatorIndex++)
    {
        afViewCreatorAbstract* pCurrentCreator = viewsCreators[viewCreatorIndex];
        GT_IF_WITH_ASSERT(pCurrentCreator != nullptr)
        {
            pCurrentCreator->restoreMinimalSize();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::onIdleTimerTimeout
// Description: Handle idle event. When using QTimer with interval of 0
//              timeout will be received on idle
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        25/8/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::onIdleTimerTimeout()
{
    // Update each of the toolbar commands:
    updateToolbarsCommands();

    // Get the views creators:
    gtVector<afViewCreatorAbstract*>& viewsCreators = afQtCreatorsManager::instance().viewsCreators();

    // Get number of creators:
    int numberViewsCreators = (int)viewsCreators.size();

    for (int viewCreatorIndex = 0 ; viewCreatorIndex < numberViewsCreators; viewCreatorIndex++)
    {
        afViewCreatorAbstract* pCurrentCreator = viewsCreators[viewCreatorIndex];
        GT_IF_WITH_ASSERT(pCurrentCreator != nullptr)
        {
            pCurrentCreator->updateViewToolbarCommands();
        }
    }

    // Update the documents
    afQMdiSubWindow* pMDIChild = activeMDISubWindow();

    if (pMDIChild != nullptr)
    {
        afDocUpdateManager::instance().ActivateView(pMDIChild, true);
    }

    // force update of documents that requires "always update"
    afDocUpdateManager::instance().ForceAlwasyUpdateDocuments();
}


// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::closeAllSubWindows
// Description:
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        31/8/2011
// ---------------------------------------------------------------------------
bool appMainAppWindow::closeAllSubWindows()
{

    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(m_pMDIArea != nullptr)
    {
        m_pMDIArea->closeAllSubWindows();
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::onFind
// Description: Perform the find operation - connect the find button click to
//              the current active window, and execute the dialog
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        1/9/2011
// ---------------------------------------------------------------------------
bool appMainAppWindow::OnFind(bool respondToTextChanged)
{
    bool retVal = false;

    if (!m_isFindWidgetInitialized)
    {
        // Verify find widget is initialized
        acFindWidget::Instance().ShowFindWidget(false);
        addDockWidget(Qt::TopDockWidgetArea, acFindWidget::Instance().GetDockWidget());
        m_isFindWidgetInitialized = true;
    }

    acFindParameters::Instance().m_shouldRespondToTextChange = respondToTextChanged;

    // Set widget visible
    acFindWidget::Instance().ShowFindWidget(true);

    // Get the current active widget:
    int viewIndex = -1;
    afViewCreatorAbstract* pCreator = findFocusedWidgetViewCreator(viewIndex);

    if (pCreator != nullptr)
    {
        // Get the sub window widget:
        QWidget* pWidget = pCreator->widget(viewIndex);

        if (pWidget != nullptr)
        {
            // Saving last widget with focus so it will get focus back when find widget get closed
            acFindWidget::Instance().SetFindFocusedWidget(pWidget);

            afBaseView* pBaseView = dynamic_cast<afBaseView*>(pWidget);

            if (nullptr != pBaseView)
            {
                gtString selectedText;
                pBaseView->GetSelectedText(selectedText);

                // Set selected text, if there is any, as the default text to search
                if (!selectedText.isEmpty())
                {
                    acFindWidget::Instance().SetTextToFind(acGTStringToQString(selectedText));
                }
            }

            // Sanity check:
            GT_IF_WITH_ASSERT((m_pFindNextAction != nullptr) && (m_pFindPrevAction != nullptr) && (acFindWidget::Instance().GetFindButton() != nullptr))
            {
                // Disconnect from previous widgets:
                // Do not assert the return value, because not always there supposed to be connected widgets:
                m_pFindNextAction->disconnect(SIGNAL(triggered()));
                m_pFindPrevAction->disconnect(SIGNAL(triggered()));
                acFindWidget::Instance().disconnect(SIGNAL(OnFind()));

                // The widget in the current sub window SHOULD have a onFindClick slot, otherwise, the command should be disabled:
                bool rcConnect = connect(&acFindWidget::Instance(), SIGNAL(OnFind()), pWidget, SLOT(onFindClick()));

                if (rcConnect)
                {
                    // Connect this widget to the find next slot:
                    rcConnect = connect(m_pFindNextAction, SIGNAL(triggered()), pWidget, SLOT(onFindNext()));
                    GT_ASSERT(rcConnect);

                    rcConnect = connect(m_pFindPrevAction, SIGNAL(triggered()), pWidget, SLOT(onFindPrev()));
                    GT_ASSERT(rcConnect);

                    // Set the focus for the text box with the find text:
                    acFindWidget::Instance().SetFocusOnFindText();
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::findActionByText
// Description: Find the action handle for the action before requested by the
//              action creator
// Arguments:   const gtString& actionMenuPosition - the string describing
//              the menu position for the action
///             actionText - action text
// Return Val:  QAction* - the requsted action, only if it was already added
// Author:      Sigal Algranaty
// Date:        5/9/2011
// ---------------------------------------------------------------------------
QAction* appMainAppWindow::findActionByText(const gtString& actionMenuPosition, const gtString& actionText, bool shouldCreate)
{
    QAction* pRetVal = nullptr;

    if (!actionMenuPosition.isEmpty())
    {
        // Get the menu for the requested action:
        QMenu* pActionParentMenu = getActionMenuItemParentMenu(actionMenuPosition, nullptr, shouldCreate);

        if (pActionParentMenu != nullptr)
        {
            // Get the main window actions:
            QList<QAction*> mainWindowActions = pActionParentMenu->actions();

            foreach (QAction* pCurrentAction, mainWindowActions)
            {
                if (pCurrentAction != nullptr)
                {
                    // Compare the data to the file path:
                    gtString dataAsStr;
                    dataAsStr.fromASCIIString(pCurrentAction->text().toLatin1().data());

                    if (dataAsStr == actionText)
                    {
                        pRetVal = pCurrentAction;
                        break;
                    }
                }
            }

            if ((pRetVal == nullptr) && (shouldCreate))
            {
                // Check the parent:
                pActionParentMenu = qobject_cast<QMenu*>(pActionParentMenu->parent());

                if (pActionParentMenu != nullptr)
                {
                    // Get the main window actions:
                    mainWindowActions = pActionParentMenu->actions();

                    foreach (QAction* pCurrentAction, mainWindowActions)
                    {
                        if (pCurrentAction != nullptr)
                        {
                            // Compare the data to the file path:
                            gtString dataAsStr;
                            dataAsStr.fromASCIIString(pCurrentAction->text().toLatin1().data());

                            if (dataAsStr == actionText)
                            {
                                pRetVal = pCurrentAction;
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::findFocusedWidgetViewCreator
// Description: This function should find the focused widgets, and if this widget
//              is created by our application, the function should get the view
//              creator from it
//              Currently, we have a problem with WX widgets, which
//              are 'stealingw the keyboard focus.
//              TO_DO: resolve this problem and steal the focus back.
//              This function is currently implemented partially (only returns the
//              active MDI sub window)
// Return Val:  QWidget*
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
afViewCreatorAbstract* appMainAppWindow::findFocusedWidgetViewCreator(int& viewIndex)
{
    afViewCreatorAbstract* pRetVal = nullptr;
    viewIndex = -1;

    // Get the focused widget:
    QWidget* pFocusWidget = qApp->focusWidget();

    // If wx has the focus:
    if ((pFocusWidget == nullptr) || (pFocusWidget == this) || (qobject_cast<QMenuBar*>(pFocusWidget) != nullptr))
    {
        // Get last wx parent that was in focus:
        pFocusWidget = focusedQWidget();

        if ((pFocusWidget == nullptr) || (pFocusWidget == this) || (qobject_cast<QMenuBar*>(pFocusWidget) != nullptr))
        {
            if (m_pMDIArea != nullptr)
            {
                pFocusWidget = m_pMDIArea->currentSubWindow();
            }
        }
    }

    if (pFocusWidget != nullptr)
    {
        bool isMDISubWindow = false;
        // Get the current widget class name:
        const char* pClassName = nullptr;

        if (pFocusWidget->metaObject() != nullptr)
        {
            pClassName = pFocusWidget->metaObject()->className();
            isMDISubWindow = (strcmp(pClassName, "afQMdiSubWindow") == 0);
        }

        if (isMDISubWindow)
        {
            afQMdiSubWindow* pQMdiSubWindow = qobject_cast<afQMdiSubWindow*>(pFocusWidget);
            GT_IF_WITH_ASSERT(pQMdiSubWindow != nullptr)
            {
                pFocusWidget = pQMdiSubWindow->widget();
            }
        }

        GT_IF_WITH_ASSERT(pFocusWidget != nullptr)
        {
            // Find a widget related view creator:
            QWidget* pWidget = pFocusWidget;

            while (pWidget != nullptr)
            {
                pRetVal = findCreatorForWidget(pWidget, viewIndex);

                if (pRetVal != nullptr)
                {
                    break;
                }

                // Keep looking in parent:
                pWidget = pWidget->parentWidget();
            }
        }
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::findCreatorForWidget
// Description: Iterates the view creators and look for the one that created pWidget
// Arguments:   QWidget* pWidget
//              int& viewIndex
// Return Val:  afViewCreatorAbstract*
// Author:      Sigal Algranaty
// Date:        8/12/2011
// ---------------------------------------------------------------------------
afViewCreatorAbstract* appMainAppWindow::findCreatorForWidget(QWidget* pWidget, int& viewIndex)
{
    afViewCreatorAbstract* pRetVal = nullptr;
    viewIndex = -1;

    // Sanity check
    GT_IF_WITH_ASSERT(pWidget != nullptr)
    {
        // Get the views creators:
        gtVector<afViewCreatorAbstract*>& viewsCreators = afQtCreatorsManager::instance().viewsCreators();

        // Get number of creators:
        int numberViewsCreators = (int)viewsCreators.size();

        for (int viewCreatorIndex = 0 ; viewCreatorIndex < numberViewsCreators && (pRetVal == nullptr) ; viewCreatorIndex++)
        {
            afViewCreatorAbstract* pCurrentCreator = viewsCreators[viewCreatorIndex];
            GT_IF_WITH_ASSERT(pCurrentCreator != nullptr)
            {
                // Get the amount of created views by this creator:
                int numViewsCreated = pCurrentCreator->amountOfCreatedViews();

                // check all the views created in a specific creator:
                for (int nView = 0 ; nView < numViewsCreated; nView++)
                {
                    if (pCurrentCreator->widget(nView) == pWidget)
                    {
                        viewIndex = nView;
                        pRetVal = pCurrentCreator;
                        break;
                    }
                }
            }
        }
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::findFocusedWidget
// Description:
// Return Val:  QWidget*
// Author:      Sigal Algranaty
// Date:        11/12/2011
// ---------------------------------------------------------------------------
QWidget* appMainAppWindow::findFocusedWidget()
{
    QWidget* pRetVal = qApp->focusWidget();

    // If wx has the focus:
    if ((pRetVal == nullptr) || (pRetVal == this))
    {
        // Get last wx parent that was in focus:
        pRetVal = focusedQWidget();
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::setFocusedQWidget
// Description: Set focused widget
// Arguments:   QWidget* pFocusedWidget
// Author:      Sigal Algranaty
// Date:        11/12/2011
// ---------------------------------------------------------------------------
void appMainAppWindow::setFocusedQWidget(QWidget* pFocusedWidget)
{
    // Set the last focused widget:
    m_pLastFocusedWidget = pFocusedWidget;

    // Sanity check
    GT_IF_WITH_ASSERT(pFocusedWidget != nullptr)
    {
        pFocusedWidget->setFocus(Qt::NoFocusReason);
    }
}


// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::getRealBeforeAction
// Description: Check if the before action is preceded by a separator, and in this
//              case, set the separator as the before action
// Return Val:  QAction*
// Author:      Sigal Algranaty
// Date:        22/2/2012
// ---------------------------------------------------------------------------
QAction* appMainAppWindow::getRealBeforeAction(QAction* pBeforeAction)
{
    QAction* pRetVal = pBeforeAction;

    // NOTICE, tricky code segment:
    // If the action that the user requested to be inserted before is an action that is preceded by
    // a separator, we want to be before the separator:
    if (pBeforeAction != nullptr)
    {
        // Get the parent:
        QMenu* pMenu = qobject_cast<QMenu*>(pBeforeAction->parent());

        while ((qobject_cast<QMenu*>(pMenu->parent()) != nullptr) && (pMenu != nullptr))
        {
            pMenu = qobject_cast<QMenu*>(pMenu->parent());
        }

        // Check if right after the "before" action, there is a separator:
        QList<QAction*> menuActions = pMenu->actions();

        // Get the "before" action index:
        int beforeActionIndex = menuActions.indexOf(pBeforeAction);

        // Get the action before the before:
        beforeActionIndex --;

        if ((beforeActionIndex >= 0) && ((beforeActionIndex < (int)menuActions.size())))
        {
            QAction* pActionBeforeTheBefore = menuActions[beforeActionIndex];

            if (pActionBeforeTheBefore != nullptr)
            {
                QString actionText = pActionBeforeTheBefore->text();
                QString classNameStr = pActionBeforeTheBefore->metaObject()->className();

                if (actionText.isEmpty() && (classNameStr == QString("QAction")))
                {
                    // It means that the action before is a separator, and this separator should be the action before:
                    pRetVal = pActionBeforeTheBefore;
                }
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::onViewToolbar
// Description: On view toolbar action
// Arguments:   QAction* pAction
// Author:      Sigal Algranaty
// Date:        22/2/2012
// ---------------------------------------------------------------------------
void appMainAppWindow::onViewToolbar()
{
    // Get the action:
    QAction* pAction = qobject_cast<QAction*>(sender());

    // Sanity check
    GT_IF_WITH_ASSERT(pAction != nullptr)
    {
        QString text = pAction->text();
        // remove '&' from the action name since the toolbars name do not have '&' in it
        QString cleanName = text.remove('&');

        acToolBar* pToolbar = findChild<acToolBar*>(cleanName);

        if (nullptr != pToolbar)
        {
            pToolbar->setVisible(pAction->isChecked());
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::initializeApplicationBasicMenus
// Description:
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        26/2/2012
// ---------------------------------------------------------------------------
void appMainAppWindow::initializeApplicationBasicMenus()
{
    // Create the edit menu:
    gtString menuString = AF_STR_FileMenuString;
    QMenu* pFileMenu = getActionMenuItemParentMenu(menuString, nullptr, true);
    GT_ASSERT(nullptr != pFileMenu);

    // Create the edit menu:
    menuString = AF_STR_EditMenuString;
    QMenu* pEditMenu = getActionMenuItemParentMenu(menuString, nullptr, true);
    GT_ASSERT(nullptr != pEditMenu);

    // Create the view menu:
    menuString = AF_STR_ViewMenuString;
    QMenu* pViewMenu = getActionMenuItemParentMenu(menuString, nullptr, true);
    GT_ASSERT(nullptr != pViewMenu);

    if (appApplicationStart::ApplicationData().m_initializeExecutionModesManager)
    {
        // Create the debug menu:
        menuString = AF_STR_DebugMenuString;
        QMenu* pDebugMenu = getActionMenuItemParentMenu(menuString, nullptr, true);
        GT_ASSERT(nullptr != pDebugMenu);

        // Create the profile menu:
        menuString = AF_STR_ProfileMenuString;
        QMenu* pProfileMenu = getActionMenuItemParentMenu(menuString, nullptr, true);
        GT_ASSERT(nullptr != pProfileMenu);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        // Create the analyze menu:
        menuString = AF_STR_FrameAnalysisMenuString;
        QMenu* pFrameAnalysisMenu = getActionMenuItemParentMenu(menuString, nullptr, true);
        GT_ASSERT(nullptr != pFrameAnalysisMenu);
#endif

        // Create the analyze menu:
        menuString = AF_STR_AnalyzeMenuString;
        QMenu* pAnalyzeMenu = getActionMenuItemParentMenu(menuString, nullptr, true);
        GT_ASSERT(nullptr != pAnalyzeMenu);
    }

    // Create the tools menu:
    menuString = AF_STR_ToolsMenuString;
    QMenu* pToolsMenu = getActionMenuItemParentMenu(menuString, nullptr, true);
    GT_ASSERT(nullptr != pToolsMenu);

    // Create the window menu:
    menuString = AF_STR_WindowsMenuString;
    QMenu* pWindowsMenu = getActionMenuItemParentMenu(menuString, nullptr, true);
    GT_ASSERT(nullptr != pWindowsMenu);
}


// ---------------------------------------------------------------------------
// Name:        appMainAppWindow::addToolbar
// Description: Add a toolbar to the main application window
// Arguments:   QToolBar* pToolbar
// Author:      Sigal Algranaty
// Date:        5/3/2012
// ---------------------------------------------------------------------------
void appMainAppWindow::addToolbar(acToolBar* pToolbar, acToolBar* pBeforeToolbar, bool frameworkToolbar)
{
    // Sanity check
    GT_IF_WITH_ASSERT(pToolbar != nullptr)
    {
        // Add the toolbar to the Qt main window:
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

        GT_IF_WITH_ASSERT(m_pViewToolbarsMenu != nullptr)
        {
            QAction* pBeforeAction = nullptr;

            // find the "before" toolbar if there is one:
            if (nullptr != pBeforeToolbar)
            {
                QString beforeToolbarName = pBeforeToolbar->objectName();

                // Look in all the current actions for the before action:
                QList<QAction*> actionsList = m_pViewToolbarsMenu->actions();
                int numActions = actionsList.count();

                for (int nAction = 0 ; nAction < numActions ; nAction++)
                {
                    QString actionName = actionsList.at(nAction)->text();
                    actionName.remove('&');

                    if (actionName == beforeToolbarName)
                    {
                        pBeforeAction = actionsList.at(nAction);
                        break;
                    }
                }
            }

            QString toolbarCommandName = pToolbar->objectName();
            toolbarCommandName.prepend("&");
            QAction* pActionAdded = m_pViewToolbarsMenu->addAction(toolbarCommandName, this, SLOT(onViewToolbar()));

            if (nullptr != pBeforeAction)
            {
                m_pViewToolbarsMenu->removeAction(pActionAdded);
                m_pViewToolbarsMenu->insertAction(pBeforeAction, pActionAdded);
            }
        }
    }
}

void appMainAppWindow::RemoveSubWindowFromWindowsMenu(afQMdiSubWindow* pSubWindowAboutToBeClosed)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pWindowsMenu != nullptr)
    {
        // Send empty event if there are no active views to enable clearing of data.
        QList<QMdiSubWindow*> windowsSubList = m_pMDIArea->subWindowList();

        if (windowsSubList.isEmpty())
        {
            osFilePath emptyFilePath;
            apMDIViewActivatedEvent activatedEvent(emptyFilePath);
            apEventsHandler::instance().registerPendingDebugEvent(activatedEvent);
        }

        // Get the file path as string:
        gtString filePathStr = pSubWindowAboutToBeClosed->filePath().asString();
        QList<QAction*> windowsActions = m_pWindowsMenu->actions();

        foreach (QAction* pCurrentAction, windowsActions)
        {
            if (pCurrentAction != nullptr)
            {
                // Get the data stored on the action:
                QVariant actionData = pCurrentAction->data();
                QString dataStr = actionData.toString();

                // Compare the data to the file path:
                gtString dataAsStr;
                dataAsStr.fromASCIIString(dataStr.toLatin1().data());

                if (dataAsStr == filePathStr)
                {
                    // Remove the action:
                    m_pWindowsMenu->removeAction(pCurrentAction);
                }
            }
        }
    }
}

void appMainAppWindow::RemoveSubWindowFromViewCreators(afQMdiSubWindow* pSubWindowAboutToBeClosed)
{
    // Go through the view action handlers:
    for (int i = 0; i < (int)m_viewsActionHandler.size(); i++)
    {
        afViewActionHandler* pViewHandler = m_viewsActionHandler[i];

        if (pViewHandler != nullptr)
        {
            if (pViewHandler->controlledWidget() == pSubWindowAboutToBeClosed)
            {
                // This action handler should be deleted:
                m_viewsActionHandler.removeItem(i);
                delete pViewHandler;
            }
        }
    }

    // Get the views creators:
    gtVector<afViewCreatorAbstract*>& viewsCreators = afQtCreatorsManager::instance().viewsCreators();
    {
        // Get number of creators:
        int numberViewsCreators = (int)viewsCreators.size();

        for (int viewCreatorIndex = 0; viewCreatorIndex < numberViewsCreators; viewCreatorIndex++)
        {
            afViewCreatorAbstract* pCurrentCreator = viewsCreators[viewCreatorIndex];
            GT_IF_WITH_ASSERT(pCurrentCreator != nullptr)
            {
                pCurrentCreator->onMDISubWindowClose(pSubWindowAboutToBeClosed);
            }
        }
    }
}

