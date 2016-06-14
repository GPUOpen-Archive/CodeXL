//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afExecutionModeManager.h
///
//==================================================================================

#ifndef __AFEXECUTIONMODEMANAGER_H
#define __AFEXECUTIONMODEMANAGER_H

// Qt
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>
#include <AMDTApplicationComponents/Include/acToolBar.h>

// Local:
#include <AMDTApplicationFramework/Include/afApplicationFrameworkDLLBuild.h>
#include <AMDTApplicationFramework/Include/afIExecutionMode.h>

class acToolBar;
class afMainAppWindow;

class AF_API afExecutionModeManager : QObject
{
    Q_OBJECT

public:
    static afExecutionModeManager& instance();
    ~afExecutionModeManager();

    /// Was the execution modes manager initialized:
    bool Initialized() const { return !m_executionModes.empty(); }

    void registerExecutionMode(afIExecutionMode* piExecutionMode);
    bool isActiveMode(const gtString& modeName);
    afIExecutionMode* activeMode() const;
    int activeSessionType() const {return m_activeSessionType;};

    // Toolbar management:
    void createModeToolbar(afMainAppWindow* pParentWindow);
    void createActionsToolbar(afMainAppWindow* pParentWindow);

    /// Set the tooltip for the requested action:
    /// \param commandID the enumeration describing the command id
    /// \param tooltipStr the tooltip string
    void SetActionTooltip(afExecutionCommandId commandID, const QString& tooltipStr);

    // Handle the update UI of the button:
    void onUpdateToolbarUI(QToolButton* pButton);
    void onUpdateToolbarUI(QAction* pAction);

    // Handle the execution mode changed event:
    void onChangedModeEvent(apExecutionModeChangedEvent& eve);

    // update the execution toolbar by key event:
    void updateExecutionToolbar(QKeyEvent* pKeyEvent);

    // Create modes toolbars:
    void createModesToolbars();

    // get the name of the last session used for a specific mode:
    gtString lastSessionUsedForMode(const gtString& modeName);

    // check if  start action is enabled or disabled, used for import action
    bool isStartActionEnabled();

    // Update start action, used for import action
    void updateStartAction(bool enable);

    /// Update the tooltip for the start action:
    void UpdateStartActionTooltip(const QString& tooltip);

    /// Display the startup dialog and handle user selection:
    bool DisplayStartupDialog();

    /// Run user selection startup action:
    bool RunUserStartupAction(afStartupAction userStartupAction);

    // Update the hosts list button:
    void UpdateHostsList();

    /// Show / hide the start action from menu. The start is shown and hidden separately, since the action requires a more complex
    /// UI and is implemented with acWidgetAction, so the default functionality cannot be used:
    void UpdateStartActionVisibility(bool isVisible, bool isEnabled);

    /// Return true iff current the remote host functionality is enabled:
    bool IsRemoteHostEnabled();

    /// let the execution mode terminate at the end
    void TerminateModes();

protected slots:

    /// Slot for handling each of the actions (the slot is not handling the start action, which is implemented differently):
    void OnActionTriggered();

    /// Handle start action. The start action is implemented separately, since it is added as acWidgetAction rather then QAction:
    void OnStartAction();

    void onModeTriggered();
    void onSessionTypeChange();
    void onSessionTypesMenuAboutToShow();

    /// Will be called when the hosts drop list is about to show:
    void OnHostsMenuAboutToShow();

    /// Is handling the change of the host drop list:
    void OnHostChange();

    /// Debugger API-level step commands:
    // void OnAPILevelStepAction(); // We update the main button's data, so it doesn't need a special handler
    void OnAPILevelStepChange();
    void OnAPILevelStepAboutToShow();

protected:
    /// Update the tooltips for all the actions in the modes toolbar:
    void UpdateModeActionsTooltips();

    /// Add the start button drop list button:
    void AddStartAction(afMainAppWindow* pParentWindow);

    /// Add the debugger-specific steps (API, Draw and Frame):
    void AddAPILevelStepsAction(afMainAppWindow* pParentWindow);

private:
    // Only the instance method can create this class:
    afExecutionModeManager();

private:
    // Only afSingletonsDelete can delete my instance:
    friend class afSingletonsDelete;

private:
    static afExecutionModeManager* m_spMySingleInstance;

private:
    gtVector<afIExecutionMode*> m_executionModes;
    gtVector<QToolButton*>      m_executionModesButtons;
    gtVector<int>               m_lastActiveSesstionTypeUsed;

    int m_activeMode;
    int m_activeSessionType;

    // Toolbars:
    acToolBar* m_pActionsToolbar;
    acToolBar* m_pModesToolbar;

    /// Widget action for the start and debugger step actions.
    /// NOTICE: (Sigal) The reason for using acWidgetAction:
    /// The requirement for this actions are:
    /// 1. Display text + icon button, with menu (this requires a QToolbutton. QAction cannot display text, icon and menu. )
    /// 2. It should be possible to show and hide this widget (start action is not relevant in KA mode)
    /// It is impossible to show and hide QWidget on a toolbar, so I decided to use acWidgetAction (which inherits QWidgetAction.)
    acWidgetAction* m_pStartAction;
    acWidgetAction* m_pStartActionCpu;
    acWidgetAction* m_pStartActionGpu;
    acWidgetAction* m_pAPILevelStepAction;
};
#endif //__AFEXECUTIONMODEMANAGER_H

