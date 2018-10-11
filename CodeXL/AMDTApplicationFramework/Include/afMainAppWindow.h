//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afMainAppWindow.h
///
//==================================================================================

#ifndef __AFMAINAPPWINDOW_H
#define __AFMAINAPPWINDOW_H

// System:
#include <QtWidgets>

class wxWindow;

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>

// Forward declaration:
class acFindWidget;
class gdProgressBarWrapper;
class afViewCreatorAbstract;
class afQMdiSubWindow;
class afViewActionHandler;
class acToolBar;

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationFramework/Include/afQMdiSubWindow.h>
#include <AMDTApplicationFramework/Include/views/afPropertiesView.h>

#define AF_MAIN_SCREEN_RATIO_REDUCTION 0.1

// Service data class for the creation of view toolbars
class afViewToolBarData
{
public:
    QAction* m_pViewAction;
    gtString m_modeName;
};

// ----------------------------------------------------------------------------------
// Class Name:          AF_API afMainAppWindow : public QMainWindow
// General Description: Base class for the application window
// Author:              Sigal Algranaty
// Creation Date:       2/8/2011
// ----------------------------------------------------------------------------------
class AF_API afMainAppWindow : public QMainWindow
{
    Q_OBJECT

public:

    // Override afMainAppWindow:
    virtual QWidget* createSingleView(afViewCreatorAbstract* pCreator, int viewIndex);

    // Services for classes, should be implemented by inherited:
    virtual void updateToolbarsCommands();
    virtual void updateToolbars();
    virtual void onSubWindowClose(afQMdiSubWindow* pSubWindowAboutToBeClosed);
    virtual void initDynamicObjects() {};
    virtual afQMdiSubWindow* activeMDISubWindow() {return nullptr;}
    virtual afQMdiSubWindow* findMDISubWindow(const osFilePath& fileName) { (void)(fileName); return nullptr;};
    virtual afQMdiSubWindow* addMDISubWindow(const osFilePath& filePath, QWidget* pWidget, const QString& caption) { (void)(filePath); (void)(pWidget); (void)(caption); return nullptr;};
    virtual bool closeMDISubWindow(afQMdiSubWindow* pSubWindow) { (void)(pSubWindow); return false;};
    virtual bool activateSubWindow(afQMdiSubWindow* pSubWindow) { (void)(pSubWindow); return false;}
    virtual bool closeAllSubWindows() {return false;}

    /// Open the find toolbar
    /// \param respondToTextChanged should we respond to each character click?
    virtual bool OnFind(bool respondToTextChanged);

    virtual QMdiArea* mdiArea() const {return nullptr;};

    // Add a toolbar to the main application window:
    virtual void addToolbar(acToolBar* pToolbar, acToolBar* pBeforeToolbar = nullptr, bool frameworkToolbar = false);

    // Window caption:
    virtual bool setWindowCaption(wxWindow* pWindow, const gtString& windowCaption);

    // Should prompt for exit:
    void setExitingWindow(bool isExiting) { m_whileExitingWindow = isExiting;}

    // Progress bar:
    gdProgressBarWrapper* progressBarWrapper() const {return _pProgressBar;};

    static afMainAppWindow* instance();

    virtual QWidget* findFocusedWidget() {return nullptr;};

    // Open a startup dialog:
    virtual void openStartupdialog();

    // Mark modal mode (used in linux):
    bool isModal()                  { return _modalMode; }
    void setModal(bool modalMode)   { _modalMode = modalMode; }

    bool renameMDIWindow(const QString& oldName, const QString& newName, const QString& newFilePath);

    // Create a toolbar for a mode if not created:
    void createToolbarForMode(const gtString& modeName);

    // Get the menu for the specific item:
    QMenu* getActionMenuItemParentMenu(const gtString& menuPosition, QAction* pBeforeAction, bool shouldCreate);

    // Layout functions:
    enum LayoutFormats
    {
        LayoutInitialized = -1,
        LayoutNoProject = 0,
        LayoutNoProjectOutput,
        LayoutDebug,
        LayoutProfileGPU,
        LayoutKernelAnalyzer,
        nLayoutFormats
    };
    // Store layout:
    void storeLayout(const gtString& layoutName);

    // Restore layout:
    void restoreLayout(LayoutFormats layoutId);

    // Reset layout to system default:
    void resetInitialLayout();

    // set initial layout mode
    void setInitialLayoutMode(LayoutFormats initialLayoutMode) { m_initialLayoutMode = initialLayoutMode; };

    // get the initial layout mode
    LayoutFormats initialLayoutMode() { return m_initialLayoutMode; };

    // Set the current layout mode:
    void updateLayoutMode(LayoutFormats layoutId);

    // bring a dockable view to front ensuring it is visible:
    void bringDockToFront(const gtString& dockName);

    void EmitSubwindowCloseSignal(const osFilePath& filePath, bool& shouldClose) { emit SubWindowAboutToClose(filePath, shouldClose); }

signals:

    /// Is emitted when the sub window is about to be closed:
    void SubWindowAboutToClose(const osFilePath& filePath, bool& shouldClose);

protected:

    /// Overrides QWidget drag & drop implementation:
    virtual void dropEvent(QDropEvent* pEvent);
    virtual void dragMoveEvent(QDragMoveEvent* pEvent);
    virtual void dragEnterEvent(QDragEnterEvent* pEvent);
    virtual void closeEvent(QCloseEvent* pCloseEvent);

    // Add and action to the menu and toolbar for a view
    bool addActionForView(afViewCreatorAbstract* pCreator, int viewIndex, QWidget* pWindowWidget);

    // Get a specific sub menu name in a menu:
    QMenu* getSubMenu(QWidget* pParentItem, gtString& menuName, QAction* pBeforeAction, bool shouldCreate);

    // find the toolbar of a mode:
    acToolBar* findToolbarByModeName(const gtString& modeName);

protected slots:

    void onLayoutActionTriggered();
    void onConvertLayout();

protected:

    // Get the layout name based on the id:
    gtString getLayoutName(LayoutFormats layoutId);

    // get the setting name:
    QString settingFileName();

    // write predefined layout to setting file:
    void writePredefineLayout(LayoutFormats layoutId);

    // Do not allow the use of my default constructor:
    afMainAppWindow();

    static afMainAppWindow* _pMyStaticInstance;
    gdProgressBarWrapper* _pProgressBar;
    bool _modalMode;
    bool m_whileExitingWindow;

    // pointers to menus to toolbars
    // pointer to the &view menu:
    QMenu* m_pViewViewsMenu;

    // pointer to the &windows menu:
    QMenu* m_pWindowsMenu;

    // pointer to the &help menu:
    QMenu* m_pHelpMenu;

    // menu that holds a list of all available toolbars:
    QMenu* m_pViewToolbarsMenu;

    // list of toolbars. One is created for each mode:
    gtVector<acToolBar*> m_modesViewsToolbarList;

    // list of framework views action:
    gtVector<QAction*> m_frameworkViewsActionsList;

    // list of mode specific views actions that need to knows its mode:
    gtVector<afViewToolBarData*> m_modeViewsActionsList;

    // first none framework toolbar
    acToolBar* m_pFirstNoneFrameworkToolBar;

    // List of viewsActionHandlers:
    gtVector<afViewActionHandler*> m_viewsActionHandler;

    // last used layout
    LayoutFormats m_lastLayout;

    // initial layout used (with or without output)
    LayoutFormats m_initialLayoutMode;

    // layout convert dialog
    QDialog* m_pConvertDialog;
    QTextEdit* m_pLayoutEdit;
    QString m_pConvertString;

};

#endif // __AFMAINAPPWINDOW_H
