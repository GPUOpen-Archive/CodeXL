//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file appMainAppWindow.h
///
//==================================================================================

#ifndef __APPMAINAPPWINDOW_H
#define __APPMAINAPPWINDOW_H

#include <QtWidgets>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afActionPositionData.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

// Forward declaration:
class ahDialogBasedAssertionFailureHandler;
class gdStateVariablesView;
class gdPropertiesView;
class acSendErrorReportDialog;
class afViewCreatorAbstract;
class afActionExecutorAbstract;
class afWxViewCreatorAbstract;
class afQtViewCreatorAbstract;
class afActionCreatorAbstract;
class afViewActionHandler;
class afQMdiSubWindow;

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QActionGroup;
class QMdiArea;
class QMdiSubWindow;
class QSignalMapper;
QT_END_NAMESPACE

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTApplicationComponents/Include/acToolBar.h>

// Forward declaration:
#include <inc/appMDIArea.h>
#include <inc/appEventObserver.h>
#include <inc/appApplicationDLLBuild.h>


class APP_API appMainAppWindow : public afMainAppWindow
{
    Q_OBJECT

public:

    virtual ~appMainAppWindow();
    void setApplicationIcon();

    static void initializeStaticInstance();

    virtual void initDynamicObjects();
    void restoreMinimalSize();

    // Override afMainAppWindow:
    virtual QWidget* createSingleView(afViewCreatorAbstract* pCreator, int viewIndex);
    virtual void updateToolbarsCommands();
    virtual void onSubWindowClose(afQMdiSubWindow* pSubWindowAboutToBeClosed);

    virtual void updateToolbars();

    /// Open the find toolbar
    /// \param respondToTextChanged should we respond to each character click?
    virtual bool OnFind(bool respondToTextChanged);

    // MDI:
    virtual afQMdiSubWindow* activeMDISubWindow();
    virtual afQMdiSubWindow* findMDISubWindow(const osFilePath& fileName);
    virtual afQMdiSubWindow* addMDISubWindow(const osFilePath& filePath, QWidget* pWidget, const QString& caption);
    virtual bool closeMDISubWindow(afQMdiSubWindow* pSubWindow);
    virtual bool activateSubWindow(afQMdiSubWindow* pSubWindow);
    virtual bool closeAllSubWindows();
    virtual QMdiArea* mdiArea() const {return m_pMDIArea;};

    // Add a toolbar to the main application window:
    virtual void addToolbar(acToolBar* pToolbar, acToolBar* pBeforeToolbar = NULL, bool frameworkToolbar = false);

    // Focus QWidget:
    virtual QWidget* findFocusedWidget();
    void setFocusedQWidget(QWidget* pFocusedWidget);
    QWidget* focusedQWidget() { return m_pLastFocusedWidget; }

    // update UI of a single action/command
    void updateSingleAction(QObject* pAction);

protected slots:

    void onActionTriggered();
    void onAboutToShowMenu();
    void onAboutToShowWindowsMenu();
    void onAboutToShowViewMenu();
    void onAboutToShowToolbarsMenu();
    void onSetActiveSubWindow(QWidget* window);
    void onIdleTimerTimeout();
    void onViewToolbar();

    /// Is handling the MDI activate signal of the MDI area
    /// \param pActivatedWindow the activated window
    void OnMDIActivate(QMdiSubWindow* pActivatedWindow);

protected:

    // Do not allow the use of my default constructor:
    appMainAppWindow();

    // Override:
    virtual void closeEvent(QCloseEvent* pCloseEvent);

    // View & Actions utilities:
    void createApplicationInitialActions();
    void createApplicationInitialViews();


    void createApplicationMenus();
    void updateSingleObjectCommands(QObject* pObject);

    // Application dialogs:
    bool createDialogBasedAssertionFailureHandler();
    void createSendErrorReportDialog();


    /// Removes the sub window from the windows menu:
    /// \param pSubWindowAboutToBeClosed the sub window about to be closed
    void RemoveSubWindowFromWindowsMenu(afQMdiSubWindow* pSubWindowAboutToBeClosed);

    /// Removes the sub window from the view creators that hold it:
    /// \param pSubWindowAboutToBeClosed the sub window about to be closed
    void RemoveSubWindowFromViewCreators(afQMdiSubWindow* pSubWindowAboutToBeClosed);

    QAction* getRealBeforeAction(QAction* pBeforeAction);

    // Create a single dynamic view:
    void createSingleAction(afActionExecutorAbstract* pActionExecutor, int actionIndex, afActionPositionData::afCommandPosition positionType);

    // Create a single dynamic view:
    void createSingleViewAction(afViewCreatorAbstract* pViewCreator, int actionIndex, afActionPositionData::afCommandPosition positionType);

    // Set a new created action properties:
    bool setNewActionProperties(afActionCreatorAbstract* pActionCreator, int actionIndex, QAction* pNewAction);

    // Find a specific toolbar:
    acToolBar* getToolbar(gtString& toolbarName);

    // Create a QT widget for the view creator:
    QWidget* createQTWindowForView(int viewIndex, afViewCreatorAbstract* pViewCreator, QWidget* pContainedWidget);

    // Dock the received dock widget with other dock widget if needed
    void dockWithOthers(int viewIndex, afViewCreatorAbstract* pViewCreator, QDockWidget* pDockWidget);

    // Find the action before:
    QAction* findActionByText(const gtString& addedActionString, const gtString& beforeActionText, bool shouldCreate);

    // Actions utilities:
    void updateViewAction(QObject* pAction);
    void updateGlobalAction(QObject* pCurrentItem);

    void triggerViewAction(QObject* pAction);
    void triggerGlobalAction(QObject* pCurrentItem);

    /// Initiate m_pFindWidget and sets a pointer to it where it's needed
    void InitFindWidget();

    // Return the focused widget view creator:
    afViewCreatorAbstract* findFocusedWidgetViewCreator(int& viewIndex);
    afViewCreatorAbstract* findCreatorForWidget(QWidget* pWidget, int& viewIndex);
    void initializeApplicationBasicMenus();

protected:

    // Internal functionality:
    appMDIArea* m_pMDIArea;
    QSignalMapper* m_pSignalWindowMapper;

    QAction* m_pFindNextAction;
    QAction* m_pFindPrevAction;

    // Assertion failure handler:
    ahDialogBasedAssertionFailureHandler* m_pDialogBasedAssertionFailureHandler;

    // Send error report dialog:
    acSendErrorReportDialog* m_pSendErrorReportDialog;

    // Events observer:
    appEventObserver* m_pEventsObserver;

    // Idle timer:
    QTimer m_idleTimer;

    // Action global index:
    int m_currentActionGlobalIndex;

    // Last QWidget focused:
    QWidget* m_pLastFocusedWidget;

    // Contain true while close event is executed:
    bool m_isInCloseEvent;

    /// True iff he find widget is initialized:
    bool m_isFindWidgetInitialized;
};

#endif // __APPMAINAPPWINDOW_H
