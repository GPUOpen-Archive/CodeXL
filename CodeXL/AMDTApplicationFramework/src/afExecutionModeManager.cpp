//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afExecutionModeManager.cpp
///
//==================================================================================

//Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acIcons.h>

// Local:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/dialogs/afStartupDialog.h>
#include <AMDTApplicationFramework/src/afModeProxyStyle.h>

// Static member initializations:
afExecutionModeManager* afExecutionModeManager::m_spMySingleInstance = nullptr;

// ---------------------------------------------------------------------------
// Name:        afExecutionModeManager::instance
// Description: Returns my single instance. Creates it if it doesn't exist yet
// Author:      Gilad Yarnitzky
// Date:        9/5/2012
// ---------------------------------------------------------------------------
afExecutionModeManager& afExecutionModeManager::instance()
{
    if (nullptr == m_spMySingleInstance)
    {
        m_spMySingleInstance = new afExecutionModeManager;
    }

    return *m_spMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        afExecutionModeManager::afExecutionModeManager
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        9/5/2012
// ---------------------------------------------------------------------------
afExecutionModeManager::afExecutionModeManager()
    : m_activeMode(-1), m_activeSessionType(0), m_pActionsToolbar(nullptr), m_pModesToolbar(nullptr), m_pStartAction(nullptr), m_pAPILevelStepAction(nullptr)
{
    // There is no active mode initially until a mode is registered;
}

// ---------------------------------------------------------------------------
// Name:        afExecutionModeManager::~afExecutionModeManager
// Description: Destructor
// Author:      Gilad Yarnitzky
// Date:        9/5/2012
// ---------------------------------------------------------------------------
afExecutionModeManager::~afExecutionModeManager()
{
}

// ---------------------------------------------------------------------------
// Name:        afExecutionModeManager::registerExecutionMode
// Description: Adds an execution mode
// Author:      Gilad Yarnitzky
// Date:        9/5/2012
// ---------------------------------------------------------------------------
void afExecutionModeManager::registerExecutionMode(afIExecutionMode* piExecutionMode)
{
    GT_IF_WITH_ASSERT(nullptr != piExecutionMode)
    {
        // See if this manager was already registered:
        int numberOfModes = (int)m_executionModes.size();
        bool shouldAdd = (nullptr != piExecutionMode);

        for (int i = 0; i < numberOfModes; i++)
        {
            if (piExecutionMode == m_executionModes[i])
            {
                shouldAdd = false;
                break;
            }
        }

        // If this is a new manager:
        GT_IF_WITH_ASSERT(shouldAdd)
        {
            GT_IF_WITH_ASSERT(piExecutionMode->numberSessionTypes() > 0)
            {
                m_executionModes.push_back(piExecutionMode);

                // if there is no mode selected, select the first one
                if (-1 == m_activeMode)
                {
                    if (piExecutionMode->isModeEnabled())
                    {
                        // use the last added active mode which is in the last position
                        m_activeMode = m_executionModes.size() - 1;
                    }
                }
            }

            // set the default last selected session type to 0:
            m_lastActiveSesstionTypeUsed.push_back(0);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        afExecutionModeManager::isActiveMode
// Description: check if the active mode is of a specific name
// Author:      Gilad Yarnitzky
// Date:        13/5/2012
// ---------------------------------------------------------------------------
bool afExecutionModeManager::isActiveMode(const gtString& modeName)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT(m_activeMode >= 0 && m_activeMode < (int)m_executionModes.size())
    {
        if (m_executionModes.at(m_activeMode)->modeName() == modeName)
        {
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afExecutionModeManager::activeMode
// Description: Returns the currently active mode
// Author:      Uri Shomroni
// Date:        22/5/2012
// ---------------------------------------------------------------------------
afIExecutionMode* afExecutionModeManager::activeMode() const
{
    afIExecutionMode* retVal = nullptr;

    GT_IF_WITH_ASSERT((0 <= m_activeMode) && ((int)m_executionModes.size() > m_activeMode))
    {
        retVal = m_executionModes[m_activeMode];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afExecutionModeManager::CreateModeToolbar
// Description: Create the mode toolbar with the actions
// Author:      Gilad Yarnitzky
// Date:        9/5/2012
// ---------------------------------------------------------------------------
void afExecutionModeManager::createModeToolbar(afMainAppWindow* pParentWindow)
{
    // Create the toolbar if there is more then one mode
    //    GT_IF_WITH_ASSERT(m_executionModes.size() > 0)
    if (m_executionModes.size() > 0)
    {
        GT_IF_WITH_ASSERT(pParentWindow != nullptr)
        {
            acToolBar* pModesToolbar = new acToolBar(pParentWindow, AF_STR_modeToolbar);
            pModesToolbar->setObjectName(AF_STR_modeToolbar);
            pParentWindow->addToolbar(pModesToolbar, nullptr, true);

            // Pass through all the execution modes and create the first toolbar button
            int numModes = m_executionModes.size();

            for (int nMode = 0 ; nMode < numModes ; nMode++)
            {
                // Create the base tool button for the mode
                QToolButton* pToolButton = new QToolButton(pModesToolbar);
                m_executionModesButtons.push_back(pToolButton);

                QString buttonName = AF_STR_modeToobarButtonBaseName;
                buttonName.append(QString::fromWCharArray(m_executionModes.at(nMode)->modeName().asCharArray()));
                pToolButton->setObjectName(buttonName);

                pModesToolbar->addWidget(pToolButton);

                // Create the action (take the icon from the first session type)
                QAction* pButtonAction = new QAction("", pModesToolbar);
                pButtonAction->setData(QVariant(nMode));

                // Set the action attributes from the first session type
                pButtonAction->setIcon(QIcon(*(m_executionModes.at(nMode)->sessionTypeIcon(0))));

                // set if the mode is enabled.
                pButtonAction->setEnabled(m_executionModes.at(nMode)->isModeEnabled());

                // set the action as checkable:
                pButtonAction->setCheckable(true);

                QStyle* pOldStyle = pToolButton->style();

                if (nullptr != pOldStyle)
                {
                    pToolButton->setStyle(new afModeProxyStyle(pOldStyle));
                }

                // Add the action to the main window so it will always be active:
                pParentWindow->addAction(pButtonAction);

                // Make the connection between the action and the handler (this manager):
                connect(pButtonAction, SIGNAL(triggered()), this, SLOT(onModeTriggered()));
                pToolButton->setDefaultAction(pButtonAction);

                afIExecutionMode* pCurrentMode = m_executionModes[nMode];

                if (pCurrentMode != nullptr)
                {
                    // Get the amount of session types for this mode:
                    int sessionTypeCount = pCurrentMode->numberSessionTypes();

                    if (sessionTypeCount > 1)
                    {
                        QMenu* pSessionTypesMenu = new QMenu;

                        // Add the mode action:
                        QString modeStr = acGTStringToQString(pCurrentMode->modeName());
                        QAction* pAction = pSessionTypesMenu->addAction(modeStr, this, SLOT(onSessionTypeChange()));
                        pAction->setData(QVariant(nMode));
                        pSessionTypesMenu->addSeparator();

                        for (int i = 0; i < sessionTypeCount; i++)
                        {
                            QString sessionTypeStr = acGTStringToQString(pCurrentMode->sessionTypeName(i));
                            pAction = pSessionTypesMenu->addAction(sessionTypeStr, this, SLOT(onSessionTypeChange()));
                            GT_IF_WITH_ASSERT(pAction != nullptr)
                            {
                                // Set the session type index and mode index as the data of the action:
                                int modeSessionTypeIndex = (i + 1) * 10 + nMode;
                                pAction->setData(QVariant(modeSessionTypeIndex));
                            }
                        }

                        // Set the menu pop up mode:
                        pToolButton->setPopupMode(QToolButton::MenuButtonPopup);

                        // Set the button menu:
                        pToolButton->setMenu(pSessionTypesMenu);

                        // Connect the menu to it's about to show slot:
                        bool rc = connect(pSessionTypesMenu, SIGNAL(aboutToShow()), this, SLOT(onSessionTypesMenuAboutToShow()));
                        GT_ASSERT(rc);
                    }
                }
            }
        }
    }

    // Update the tooltips for the mode actions:
    UpdateModeActionsTooltips();
}


// ---------------------------------------------------------------------------
// Name:        afExecutionModeManager::CreateActionsToolbar
// Description: Create the actions toolbar with the actions
// Author:      Gilad Yarnitzky
// Date:        9/5/2012
// ---------------------------------------------------------------------------
void afExecutionModeManager::createActionsToolbar(afMainAppWindow* pParentWindow)
{
    GT_IF_WITH_ASSERT(pParentWindow != nullptr)
    {
        m_pActionsToolbar = new acToolBar(pParentWindow, AF_STR_actionsToolbar);
        m_pActionsToolbar->setObjectName(AF_STR_actionsToolbar);
        pParentWindow->addToolbar(m_pActionsToolbar, nullptr, true);

        // The start action is added separately, since it is added as drop list, and not as simple button:
        AddStartAction(pParentWindow);

        for (int nAction = AF_EXECUTION_ID_BREAK; nAction < AF_EXECUTION_LAST_COMMAND_ID; nAction++)
        {
            // The API-level steps are hidden under a common "menu" item:
            if ((AF_EXECUTION_ID_API_STEP == nAction) || (AF_EXECUTION_ID_DRAW_STEP == nAction) || (AF_EXECUTION_ID_FRAME_STEP == nAction))
            {
                // Add the special button instead of the first one of these:
                if (nullptr == m_pAPILevelStepAction)
                {
                    // Add the special steps:
                    AddAPILevelStepsAction(pParentWindow);
                }

                // Do not add the action itself:
                continue;
            }

            // Set the pixmap that will be added to the action:
            QPixmap actionPixmap;
            QAction* pButtonAction = nullptr;

            switch (nAction)
            {
                case AF_EXECUTION_ID_BREAK:             acSetIconInPixmap(actionPixmap, AC_ICON_EXECUTION_PAUSE);               break;

                case AF_EXECUTION_ID_STOP:              acSetIconInPixmap(actionPixmap, AC_ICON_EXECUTION_STOP);                break;

                // case AF_EXECUTION_ID_API_STEP:          acSetIconInPixmap(actionPixmap, AC_ICON_EXECUTION_API_STEP);            break;
                // case AF_EXECUTION_ID_DRAW_STEP:         acSetIconInPixmap(actionPixmap, AC_ICON_EXECUTION_DRAW_STEP);           break;
                // case AF_EXECUTION_ID_FRAME_STEP:        acSetIconInPixmap(actionPixmap, AC_ICON_EXECUTION_FRAME_STEP);          break;
                case AF_EXECUTION_ID_STEP_OVER:         acSetIconInPixmap(actionPixmap, AC_ICON_EXECUTION_STEP_OVER);           break;

                case AF_EXECUTION_ID_STEP_IN:           acSetIconInPixmap(actionPixmap, AC_ICON_EXECUTION_STEP_IN);             break;

                case AF_EXECUTION_ID_STEP_OUT:          acSetIconInPixmap(actionPixmap, AC_ICON_EXECUTION_STEP_OUT);            break;

                case AF_EXECUTION_ID_BUILD:             acSetIconInPixmap(actionPixmap, AC_ICON_EXECUTION_BUILD);               break;

                case AF_EXECUTION_ID_CANCEL_BUILD:      acSetIconInPixmap(actionPixmap, AC_ICON_EXECUTION_CANCEL_BUILD);        break;

                case AF_EXECUTION_ID_CAPTURE:           acSetIconInPixmap(actionPixmap, AC_ICON_EXECUTION_CAPTURE);             break;
                case AF_EXECUTION_ID_CAPTURE_CPU:       acSetIconInPixmap(actionPixmap, AC_ICON_EXECUTION_CAPTURE);             break;
                case AF_EXECUTION_ID_CAPTURE_GPU:       acSetIconInPixmap(actionPixmap, AC_ICON_EXECUTION_CAPTURE);             break;

                default: break;
            }

            if (pButtonAction == nullptr)
            {
                pButtonAction = new QAction("", m_pActionsToolbar);
            }

            pButtonAction->setData(QVariant(nAction));

            // Set the action icon:
            pButtonAction->setIcon(QIcon(actionPixmap));

            // make the button uncheckable:
            pButtonAction->setCheckable(false);

            QString actionName;
            actionName = actionName.sprintf("%s%d", AF_STR_modeToobarButtonBaseName, nAction);
            pButtonAction->setObjectName(actionName);

            // Set the action text:
            switch (nAction)
            {
                case AF_EXECUTION_ID_BREAK:                 pButtonAction->setText(AF_STR_BreakDebugging);      break;

                case AF_EXECUTION_ID_STOP:                  pButtonAction->setText(AF_STR_StopDebugging);       break;

                // case AF_EXECUTION_ID_API_STEP:              pButtonAction->setText(AF_STR_APIStepDebugging);    break;
                // case AF_EXECUTION_ID_DRAW_STEP:             pButtonAction->setText(AF_STR_DrawStepDebugging);   break;
                // case AF_EXECUTION_ID_FRAME_STEP:            pButtonAction->setText(AF_STR_FrameStepDebugging);  break;
                case AF_EXECUTION_ID_STEP_OVER:             pButtonAction->setText(AF_STR_StepOverDebugging);   break;

                case AF_EXECUTION_ID_STEP_IN:               pButtonAction->setText(AF_STR_StepInDebugging);     break;

                case AF_EXECUTION_ID_STEP_OUT:              pButtonAction->setText(AF_STR_StepOutDebugging);    break;

                case AF_EXECUTION_ID_BUILD:                 pButtonAction->setText(AF_STR_Build);               break;

                case AF_EXECUTION_ID_CANCEL_BUILD:          pButtonAction->setText(AF_STR_CancelBuild);         break;

                case AF_EXECUTION_ID_CAPTURE:               pButtonAction->setText(AF_STR_CaptureFrame);         break;
                case AF_EXECUTION_ID_CAPTURE_CPU:           pButtonAction->setText(AF_STR_CaptureFrame);         break;
                case AF_EXECUTION_ID_CAPTURE_GPU:           pButtonAction->setText(AF_STR_CaptureFrame);         break;

                default: break;
            }

            // Set the short cut keyboard:
            switch (nAction)
            {
                case AF_EXECUTION_ID_BREAK:                 pButtonAction->setShortcut(QKeySequence(AF_STR_PauseDebuggingShortcut));    break;

                case AF_EXECUTION_ID_STOP:                  pButtonAction->setShortcut(QKeySequence(AF_STR_StopDebuggingShortcut));     break;

                case AF_EXECUTION_ID_BUILD:                 pButtonAction->setShortcut(QKeySequence(AF_STR_BuildShortcut));             break;

                case AF_EXECUTION_ID_CANCEL_BUILD:          pButtonAction->setShortcut(QKeySequence(AF_STR_CancelBuildShortcut));       break;

                default: break;
            }

            // Add the action to the main window so it will always be active:
            pParentWindow->addAction(pButtonAction);

            // Make the connection between the action and the handler (this manager):
            connect(pButtonAction, SIGNAL(triggered()), this, SLOT(OnActionTriggered()));
            m_pActionsToolbar->addAction(pButtonAction);
        }
    }
}

void afExecutionModeManager::SetActionTooltip(afExecutionCommandId commandID, const QString& tooltipStr)
{
    if (m_pActionsToolbar != nullptr)
    {
        bool found = false;

        QList<QAction*> actions = m_pActionsToolbar->actions();
        QList<QAction*>::iterator it = actions.begin();

        for (; it != actions.end(); it++)
        {
            QAction* pAction = (*it);
            GT_IF_WITH_ASSERT(pAction != nullptr)
            {
                // Get the action id for the current action:
                afExecutionCommandId actionId = (afExecutionCommandId)pAction->data().toInt();

                if (actionId == commandID)
                {
                    pAction->setToolTip(tooltipStr);
                    found = true;
                }
            }
        }

        if (!found)
        {
            // Search within the API-level steps menu:
            QToolButton* pToolbutton = qobject_cast<QToolButton*>(m_pActionsToolbar->widgetForAction(m_pAPILevelStepAction));

            if (pToolbutton != NULL)
            {
                // Get its menu:
                QMenu* pActionsMenu = pToolbutton->menu();

                if (pActionsMenu != NULL)
                {
                    // Initialize the actions:
                    QList<QAction*> stepActions = pActionsMenu->actions();
                    it = stepActions.begin();

                    for (; it != stepActions.end(); it++)
                    {
                        QAction* pAction = (*it);
                        GT_IF_WITH_ASSERT(pAction != nullptr)
                        {
                            // Get the action id for the current action:
                            afExecutionCommandId actionId = (afExecutionCommandId)pAction->data().toInt();

                            if (actionId == commandID)
                            {
                                pAction->setToolTip(tooltipStr);
                                found = true;
                            }
                        }
                    }
                }
            }
        }

        GT_ASSERT(found);
    }
}


// ---------------------------------------------------------------------------
// Name:        afExecutionModeManager::onUpdateToolbarUI
// Description: Handle the update of the toolbar buttons
// Author:      Gilad Yarnitzky
// Date:        9/5/2012
// ---------------------------------------------------------------------------
void afExecutionModeManager::onUpdateToolbarUI(QToolButton* pButton)
{
    afRunModes runModes = afPluginConnectionManager::instance().getCurrentRunModeMask();

    QAction* pButtonAction = pButton->defaultAction();
    GT_IF_WITH_ASSERT(nullptr != pButtonAction)
    {
        // check if the button name has a mode name in it. if it has then this is a mode button, check/uncheck it based on the active mode:
        QString buttonName = pButton->objectName();

        int numModes = (int)m_executionModes.size();

        for (int nMode = 0 ; nMode < numModes; nMode++)
        {
            QString modeName = QString::fromWCharArray(m_executionModes.at(nMode)->modeName().asCharArray());

            if (buttonName.endsWith(modeName))
            {
                // Set the check state
                bool actionChecked = (nMode == m_activeMode);
                pButtonAction->setChecked(actionChecked);

                // Set the enable mode:
                // only if the run mode is not any execution mode the modes are enabled:
                bool isModeEnabled = m_executionModes.at(nMode)->isModeEnabled();
                bool actionEnabled = (runModes == 0) && isModeEnabled;
                pButtonAction->setEnabled(actionEnabled);
            }
        }
    }
}

void afExecutionModeManager::onUpdateToolbarUI(QAction* pAction)
{
    // Button not handled, this is an action button, get its id and pass it to the active execution mode:
    int nAction = pAction->data().toInt();

    // Send the current active execution mode the command to update the ui
    if (m_activeMode >= 0 && m_activeMode < (int)m_executionModes.size())
    {
        afIExecutionMode* pExecutor = m_executionModes.at(m_activeMode);
        GT_IF_WITH_ASSERT(nullptr != pExecutor)
        {
            pExecutor->updateUI((afExecutionCommandId)nAction, pAction);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        afExecutionModeManager::onActionTriggered
// Description: Handle the triggered toolbar actions
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        9/5/2012
// ---------------------------------------------------------------------------
void afExecutionModeManager::OnActionTriggered()
{
    QObject* pSender = sender();
    QAction* pAction = qobject_cast<QAction*>(pSender);

    if (nullptr == pAction)
    {
        // If the sender isn't directly an action, it might be
        // the "last API-level step" button:
        QToolButton* pToolButton = qobject_cast<QToolButton*>(pSender);

        if ((nullptr != pToolButton) && (qobject_cast<QToolButton*>(m_pActionsToolbar->widgetForAction(m_pAPILevelStepAction)) == pToolButton))
        {
            pAction = m_pAPILevelStepAction;
        }
    }

    GT_IF_WITH_ASSERT(pAction != nullptr)
    {
        // Get the action id for this action:
        afExecutionCommandId actionId = (afExecutionCommandId)pAction->data().toInt();

        // Send the current active execution mode the command to execute
        if (m_activeMode >= 0 && m_activeMode < (int)m_executionModes.size())
        {
            afIExecutionMode* pExecutor = m_executionModes.at(m_activeMode);
            GT_IF_WITH_ASSERT(nullptr != pExecutor)
            {
                pExecutor->execute(actionId);
            }
        }

        // Make sure the action is not checked:
        pAction->setChecked(false);
    }
}

void afExecutionModeManager::OnStartAction()
{
    bool isActionHandled = false;
    bool isProjectLoaded = !afProjectManager::instance().currentProjectFilePath().isEmpty();

    if (!isProjectLoaded)
    {
        bool isStartSupportedWithNoProject = false;

        // Check if the current executor support start with no project:
        if (m_activeMode >= 0 && m_activeMode < (int)m_executionModes.size())
        {
            afIExecutionMode* pExecutor = m_executionModes.at(m_activeMode);

            if (pExecutor != nullptr)
            {
                isStartSupportedWithNoProject = pExecutor->IsStartupActionSupportedWithNoProject(AF_EXECUTION_ID_START);
            }
        }

        if (!isStartSupportedWithNoProject)
        {
            // Display the startup dialog:
            bool rc = DisplayStartupDialog();
            GT_ASSERT(rc);
            isActionHandled = true;
        }
    }

    if (!isActionHandled)
    {
        // Send the current active execution mode the command to execute
        if (m_activeMode >= 0 && m_activeMode < (int)m_executionModes.size())
        {
            afIExecutionMode* pExecutor = m_executionModes.at(m_activeMode);
            GT_IF_WITH_ASSERT(nullptr != pExecutor)
            {
                pExecutor->execute(AF_EXECUTION_ID_START);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afExecutionModeManager::onModeTriggered
// Description: Handle the triggered toolbar actions
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        9/5/2012
// ---------------------------------------------------------------------------
void afExecutionModeManager::onModeTriggered()
{
    QAction* pAction = qobject_cast<QAction*>(sender());
    GT_IF_WITH_ASSERT(pAction != nullptr)
    {
        // Get the mode index:
        int activeModeIndex = pAction->data().toInt();

        // Set the active mode:
        m_activeMode = activeModeIndex;

        // Find the name for this mode:
        GT_IF_WITH_ASSERT((activeModeIndex < (int)m_executionModes.size()) && (activeModeIndex >= 0))
        {
            afIExecutionMode* pExecutionMode = m_executionModes[activeModeIndex];
            GT_IF_WITH_ASSERT(pExecutionMode != nullptr)
            {
                apExecutionModeChangedEvent executionModeEvent(pExecutionMode->modeName(), -1);
                apEventsHandler::instance().registerPendingDebugEvent(executionModeEvent);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        afExecutionModeManager::onChangedModeEvent
// Description: Handle the changed mode event
// Arguments:   afExecutionModeChangedEvent& eve
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        13/5/2012
// ---------------------------------------------------------------------------
void afExecutionModeManager::onChangedModeEvent(apExecutionModeChangedEvent& eve)
{
    bool modeChanged = false;
    int currentMode = -1;
    QString eveModeName = QString::fromWCharArray(eve.modeType().asCharArray());
    QString eveSettingName = QString::fromWCharArray(eve.sessionTypeName().asCharArray());

    // Set the mode using the name:
    int numModes = (int)m_executionModes.size();

    for (int nMode = 0 ; nMode < numModes; nMode++)
    {
        QString modeName = QString::fromWCharArray(m_executionModes.at(nMode)->modeName().asCharArray());

        // IF the name is of a registered mode name use it:
        if (eveModeName == modeName && m_executionModes.at(nMode) != nullptr && m_executionModes.at(nMode)->isModeEnabled())
        {
            currentMode = nMode;

            if (!eve.onlySessionTypeIndex())
            {
                modeChanged = true;
            }
        }
    }

    if (!modeChanged)
    {
        // if in VS and event is empty it is because the project was closed move back to mode 0
        if (eveModeName.isEmpty() && eveSettingName.isEmpty() && eve.sessionTypeIndex() == -1 && afGlobalVariablesManager::instance().isRunningInsideVisualStudio() && afProjectManager::instance().currentProjectFilePath().asString().isEmpty())
        {
            currentMode = 0;
            modeChanged = true;
        }
    }

    if (modeChanged || (eve.onlySessionTypeIndex() && currentMode != -1))
    {
        // Get the session type as string:
        int sessionTypeIndex = eve.sessionTypeIndex();
        gtString sessionTypeStr = eve.sessionTypeName();

        // Session type is identified by a string:
        if (currentMode >= 0 && currentMode < (int)m_executionModes.size())
        {
            afIExecutionMode* pExecutor = m_executionModes.at(currentMode);
            GT_IF_WITH_ASSERT(nullptr != pExecutor)
            {
                if (!sessionTypeStr.isEmpty() && (sessionTypeIndex == -1))
                {
                    sessionTypeIndex = pExecutor->indexForSessionType(sessionTypeStr);
                }
                else if ((sessionTypeIndex != -1) && sessionTypeStr.isEmpty())
                {
                    sessionTypeStr = pExecutor->sessionTypeName(sessionTypeIndex);
                }
            }
        }

        // Set the session type if it is not -1 (used when just moving to the mode without changing session type)
        if (sessionTypeIndex != -1)
        {
            // check that the index type is in the limits of the mode range:
            if (sessionTypeIndex >= 0 && sessionTypeIndex < m_executionModes.at(currentMode)->numberSessionTypes())
            {
                if ((eve.onlySessionTypeIndex() && (m_activeMode == currentMode)) || modeChanged)
                {
                    m_activeSessionType = sessionTypeIndex;
                }

                // store the last active session type for restoration:
                m_lastActiveSesstionTypeUsed.at(currentMode) = sessionTypeIndex;

                // Set the user last active mode:
                afProjectManager::instance().setLastActiveSessionType(sessionTypeStr);

                if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
                {
                    // update the active mode icon based on the session type index:
                    QToolButton* pModeButton = m_executionModesButtons.at(currentMode);
                    QAction* pModeAction = pModeButton->defaultAction();

                    pModeAction->setIcon(QIcon(*(m_executionModes.at(currentMode)->sessionTypeIcon(0))));
                }
            }
        }
        else
        {
            // restore the last used session type for the new mode
            m_activeSessionType = m_lastActiveSesstionTypeUsed.at(currentMode);
        }
    }

    // Force the current mode after all session type updates are done
    // Session type index can be done even if there is no mode updates this is why they are done before now:
    if (modeChanged)
    {
        m_activeMode = currentMode;

        // Set the user last active mode:
        afProjectManager::instance().setLastActiveMode(m_executionModes.at(currentMode)->modeName());

        // Need to select the root node:
        // 1: Maybe a node that is now hidden was selected so this will unselect it and update the information correctly
        // 2: If the root node was selected we still need to reselect it in order to update the text for the new mode
        afApplicationTree* pAppTree = afApplicationCommands::instance()->applicationTree();

        if (nullptr != pAppTree && nullptr != pAppTree->rootItemData())
        {
            pAppTree->selectItem(pAppTree->rootItemData(), false);
        }
    }

    // Update the application title in the SA:
    if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        gtString titleBarString;
        afCalculateCodeXLTitleBarString(titleBarString);
        afApplicationCommands::instance()->setApplicationCaption(titleBarString);

        // update the layouts. Default is no project loaded:
        gtString projectPath = afProjectManager::instance().currentProjectFilePath().asString();
        afMainAppWindow::LayoutFormats currentLayout = afMainAppWindow::LayoutNoProject;

        if (afMainAppWindow::instance() != nullptr)
        {
            currentLayout = afMainAppWindow::instance()->initialLayoutMode();
        }

        // If there is a project, check the mode:
        if (!projectPath.isEmpty())
        {
            afIExecutionMode* pCurrentMode = activeMode();

            if (nullptr != pCurrentMode)
            {
                currentLayout = pCurrentMode->layoutFormat();
            }
        }

        // Set the layout based on what was found:
        GT_IF_WITH_ASSERT(afMainAppWindow::instance() != nullptr)
        {
            afMainAppWindow::instance()->updateLayoutMode(currentLayout);
        }
    }

    // Update the tooltips for the mode actions:
    UpdateModeActionsTooltips();
}

// ---------------------------------------------------------------------------
// Name:        afExecutionModeManager::updateMenuUIbyKeyEvent
// Description: Update the needed item in the toolbar managed by the afExecutionModeManager
// Author:      Gilad Yarnitzky
// Date:        31/5/2012
// ---------------------------------------------------------------------------
void afExecutionModeManager::updateExecutionToolbar(QKeyEvent* pKeyEvent)
{
    QKeySequence eventKeySequence(pKeyEvent->key() + pKeyEvent->modifiers());

    if (nullptr != m_pActionsToolbar)
    {
        // Find the action that has the same key shortcut
        QList<QAction*> actionsList = m_pActionsToolbar->actions();

        int numActions = actionsList.count();

        for (int nCount = 0 ; nCount < numActions ; nCount++)
        {
            QAction* pCurrentAction = actionsList.at(nCount);

            if (nullptr != pCurrentAction)
            {
                if (pCurrentAction->shortcut() == eventKeySequence)
                {
                    onUpdateToolbarUI(pCurrentAction);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afExecutionModeManager::createModesToolbars
// Description: create the modes toolbars
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        2/12/2012
// ---------------------------------------------------------------------------
void afExecutionModeManager::createModesToolbars()
{
    int numModes = m_executionModes.size();

    for (int nMode = 0; nMode < numModes ; nMode++)
    {
        afIExecutionMode* pCurrentMode = m_executionModes.at(nMode);
        // Create the mode specific view toolbar:
        afMainAppWindow::instance()->createToolbarForMode(pCurrentMode->modeName());
    }
}


void afExecutionModeManager::UpdateModeActionsTooltips()
{
    // Go over the modes and update its tooltips:
    for (int modeIndex = 0; modeIndex < (int)m_executionModesButtons.size(); modeIndex++)
    {
        // Sanity check - make sure that all the vectors are at the same size:
        GT_IF_WITH_ASSERT((modeIndex < (int)m_executionModes.size()) && (modeIndex < (int)m_executionModesButtons.size()) && (modeIndex < (int)m_lastActiveSesstionTypeUsed.size()))
        {
            afIExecutionMode* pMode = m_executionModes[modeIndex];
            QToolButton* pModeButton = m_executionModesButtons[modeIndex];
            int sessionTypeIndex = m_lastActiveSesstionTypeUsed[modeIndex];
            GT_IF_WITH_ASSERT((pModeButton != nullptr) && (pMode != nullptr))
            {
                QAction* pAction = pModeButton->defaultAction();
                GT_IF_WITH_ASSERT(pAction != nullptr)
                {
                    afIExecutionMode* pExeMode = m_executionModes.at(modeIndex);
                    GT_IF_WITH_ASSERT(pExeMode != nullptr)
                    {
                        QString modeName = QString::fromWCharArray(pExeMode->modeName().asCharArray());
                        QString sessionName = QString::fromWCharArray(pExeMode->sessionTypeName(sessionTypeIndex).asCharArray());
                        QString modeDesc = QString::fromWCharArray(pExeMode->modeDescription().asCharArray());
                        QString actionDesc = acGTStringToQString(pExeMode->modeActionString());

                        QString tooltipTitle = modeName;

                        // Debug session type might be "debug" or "debug mode" so do not add it to the tool tip:
                        if (!modeName.contains(sessionName))
                        {
                            tooltipTitle = tooltipTitle + " (" + sessionName + ")";
                        }

                        if (modeIndex != m_activeMode)
                        {
                            tooltipTitle.prepend(AF_STR_modeSwitchToTooltipPart1);
                        }

                        QString tooltipString;
                        acBuildFormattedTooltip(tooltipTitle, modeDesc, tooltipString);

                        pAction->setText(tooltipString);

                        if (modeIndex == m_activeMode)
                        {
                            // Build a tooltip for the local button, and set it
                            QString localButtonTooltip = QString(AF_STR_StartTooltip).arg(actionDesc);

                            if (m_pStartAction != nullptr)
                            {
                                m_pStartAction->UpdateTooltip(localButtonTooltip);
                            }
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afExecutionModeManager::lastSessionUsedForMode
// Description: Get the last session name use for a specific mode if that mode exists
// Author:      Gilad Yarnitzky
// Date:        6/1/2013
// ---------------------------------------------------------------------------
gtString afExecutionModeManager::lastSessionUsedForMode(const gtString& modeName)
{
    gtString retVal = L"";

    int usedMode = -1;

    int numModes = (int)m_executionModes.size();

    for (int nMode = 0 ; nMode < numModes; nMode++)
    {
        // IF the name is of a registered mode name use it:
        if (m_executionModes.at(nMode)->modeName() == modeName)
        {
            usedMode = nMode;
            break;
        }
    }

    if (-1 != usedMode)
    {
        retVal = m_executionModes.at(usedMode)->sessionTypeName(m_lastActiveSesstionTypeUsed.at(usedMode));
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        afExecutionModeManager::isStartActionEnabled
// Description: Checks if "Start" action is currently enabled or not
// Return Val:  bool if "Start" action is currently enabled or not
// Author:      Bhattacharyya Koushik
// Date:        04/01/2013
// ---------------------------------------------------------------------------
bool afExecutionModeManager::isStartActionEnabled()
{
    bool result = false;

    // Find the button that has the action with the same key shortcut:
    if (m_pActionsToolbar != nullptr)
    {
        QList<QAction*> actionsList = m_pActionsToolbar->actions();
        int numActions = actionsList.count();

        for (int nCount = 0 ; nCount < numActions ; nCount++)
        {
            QAction* pCurrentAction = actionsList.at(nCount);

            if (nullptr != pCurrentAction)
            {
                if (AF_EXECUTION_ID_START == pCurrentAction->data().toInt())
                {
                    result = pCurrentAction->isEnabled();
                }
            }
        }
    }

    return result;
}

// ---------------------------------------------------------------------------
// Name:        afExecutionModeManager::updateStartAction
// Description: Checks if "Start" action is currently enabled or not
// Arguments:  bool if "Start" action is currently enabled or not
// Author:      Bhattacharyya Koushik
// Date:        04/01/2013
// ---------------------------------------------------------------------------
void afExecutionModeManager::updateStartAction(bool enable)
{
    // Find the button that has the action with the same key shortcut
    if (m_pActionsToolbar != nullptr)
    {
        QList<QAction*> actionsList = m_pActionsToolbar->actions();

        int numActions = actionsList.count();

        for (int nCount = 0 ; nCount < numActions ; nCount++)
        {
            QAction* pCurrentAction = actionsList.at(nCount);

            if (nullptr != pCurrentAction)
            {
                if (AF_EXECUTION_ID_START == pCurrentAction->data().toInt())
                {
                    pCurrentAction->setEnabled(enable);
                }
            }
        }
    }
}

void afExecutionModeManager::onSessionTypeChange()
{
    QAction* pAction = qobject_cast<QAction*>(sender());
    GT_IF_WITH_ASSERT(pAction != nullptr)
    {
        int modeSessionIndex = pAction->data().toInt();

        int sessionTypeIndex = (modeSessionIndex > 10) ? ((modeSessionIndex / 10) - 1) : -1;
        int modeIndex = (modeSessionIndex % 10);

        // Get the executor for this mode:
        GT_IF_WITH_ASSERT((modeIndex >= 0) && (modeIndex < (int)m_executionModes.size()) && (modeIndex < (int)m_executionModesButtons.size()))
        {
            afIExecutionMode* pMode = m_executionModes[modeIndex];
            QToolButton* pButton = m_executionModesButtons[modeIndex];
            GT_IF_WITH_ASSERT((pMode != nullptr) && (pButton != nullptr))
            {
                pButton->click();

                // This is for the generic action:
                if (sessionTypeIndex < 0)
                {
                    sessionTypeIndex = pMode->indexForSessionType(pMode->selectedSessionTypeName());
                }

                GT_IF_WITH_ASSERT(sessionTypeIndex >= 0)
                {
                    // Execute the session type:
                    pMode->execute(sessionTypeIndex);

                    // Set the active session type:
                    m_activeSessionType = sessionTypeIndex;

                    apExecutionModeChangedEvent executionModeEvent(pMode->modeName(), sessionTypeIndex);
                    apEventsHandler::instance().registerPendingDebugEvent(executionModeEvent);
                }
            }
        }
    }

    // Update the tooltips for the mode actions:
    UpdateModeActionsTooltips();
}

void afExecutionModeManager::onSessionTypesMenuAboutToShow()
{
    QMenu* pMenu = qobject_cast<QMenu*>(sender());
    GT_IF_WITH_ASSERT(pMenu != nullptr)
    {
        QToolButton* pToolbutton = nullptr;

        for (int i = 0 ; i < (int)m_executionModesButtons.size(); i++)
        {
            QToolButton* pTemp = m_executionModesButtons[i];

            if (pTemp != nullptr)
            {
                if (pTemp->menu() == pMenu)
                {
                    pToolbutton = pTemp;
                    break;
                }
            }
        }

        GT_IF_WITH_ASSERT(pToolbutton != nullptr)
        {
            QAction* pButtonAction = pToolbutton->defaultAction();
            GT_IF_WITH_ASSERT(pButtonAction != nullptr)
            {
                int nAction = pButtonAction->data().toInt();

                // Send the current active execution mode the command to update the ui
                if (m_activeMode >= 0 && m_activeMode < (int)m_executionModes.size())
                {
                    afIExecutionMode* pExecutor = m_executionModes.at(nAction);
                    GT_IF_WITH_ASSERT(nullptr != pExecutor)
                    {
                        QList<QAction*> menuActions = pMenu->actions();

                        QAction* pModeAction = menuActions[0];

                        if (pModeAction != nullptr)
                        {
                            int modeIndex = pModeAction->data().toInt();
                            pModeAction->setEnabled(m_activeMode != modeIndex);
                            pModeAction->setCheckable(true);
                            pModeAction->setChecked(m_activeMode == modeIndex);
                            gtString modeFullStr;
                            gtString sessionTypeStr = pExecutor->selectedSessionTypeName();
                            modeFullStr.appendFormattedString(L"%ls - %ls", pExecutor->modeName().asCharArray(), sessionTypeStr.asCharArray());
                            pModeAction->setText(acGTStringToQString(modeFullStr));
                        }

                        int amountOfActions = menuActions.size();
                        int sessionTypesAmount = pExecutor->numberSessionTypes();
                        GT_IF_WITH_ASSERT(sessionTypesAmount == (amountOfActions - 2))
                        {
                            for (int i = 2; i < amountOfActions; i++)
                            {
                                pExecutor->updateUI(i - 2, menuActions[i]);
                            }
                        }
                    }
                }
            }
        }
    }
}

void afExecutionModeManager::UpdateStartActionTooltip(const QString& tooltip)
{
    // Find the button that has the action with the same key shortcut
    if (m_pActionsToolbar != nullptr)
    {
        QList<QAction*> actionsList = m_pActionsToolbar->actions();

        int numActions = actionsList.count();

        for (int nCount = 0 ; nCount < numActions ; nCount++)
        {
            QAction* pCurrentAction = actionsList.at(nCount);

            if (nullptr != pCurrentAction)
            {
                if (AF_EXECUTION_ID_START == pCurrentAction->data().toInt())
                {
                    pCurrentAction->setText(tooltip);
                }
            }
        }
    }
}

bool afExecutionModeManager::DisplayStartupDialog()
{
    bool retVal = false;

    afStartupAction userSelection = AF_NO_PROJECT_USER_ACTION_NONE;
    afStartupDialog dlg(afMainAppWindow::instance());

    int dlgResult = afApplicationCommands::instance()->showModal(&dlg);

    if (dlgResult == QDialog::Accepted)
    {
        // Check what is the user selection:
        userSelection = dlg.GetUserSelection();

        if (userSelection == AF_NO_PROJECT_USER_ACTION_CREATE_NEW_PROJECT)
        {
            // Open the create new project dialog:
            afApplicationCommands::instance()->OnFileNewProject();
            retVal = true;
        }

        if (!retVal)
        {
            // Go though the modes, and find the mode handling this selection:
            for (int i = 0 ; i < (int)m_executionModes.size(); i++)
            {
                // Get the current mode:
                afIExecutionMode* pExecutionMode = m_executionModes.at(i);

                if (pExecutionMode != nullptr)
                {
                    // If this executer is handling the action, go for it:
                    if (pExecutionMode->IsStartupActionSupported(userSelection))
                    {
                        // Execute the requested startup action:
                        pExecutionMode->ExecuteStartupAction(userSelection);

                        // Switch to the mode of this executor:
                        apExecutionModeChangedEvent executionModeEvent(pExecutionMode->modeName(), -1);
                        apEventsHandler::instance().registerPendingDebugEvent(executionModeEvent);
                        onChangedModeEvent(executionModeEvent);

                        retVal = true;
                        break;
                    }
                }
            }
        }
    }
    else
    {
        retVal = true;
    }

    return retVal;
}

bool afExecutionModeManager::RunUserStartupAction(afStartupAction userStartupAction)
{
    bool retVal = false;

    if (userStartupAction == AF_NO_PROJECT_USER_ACTION_CREATE_NEW_PROJECT)
    {
        // Open the create new project dialog:
        afApplicationCommands::instance()->OnFileNewProject();
        retVal = true;
    }

    if (!retVal)
    {
        // Go though the modes, and find the mode handling this selection:
        for (int i = 0; i < (int)m_executionModes.size(); i++)
        {
            // Get the current mode:
            afIExecutionMode* pExecutionMode = m_executionModes.at(i);

            if (pExecutionMode != nullptr)
            {
                // If this executer is handling the action, go for it:
                if (pExecutionMode->IsStartupActionSupported(userStartupAction))
                {
                    // Switch to the mode of this executor:
                    apExecutionModeChangedEvent executionModeEvent(pExecutionMode->modeName(), -1);
                    apEventsHandler::instance().registerPendingDebugEvent(executionModeEvent);
                    onChangedModeEvent(executionModeEvent);

                    // Execute the requested startup action:
                    pExecutionMode->ExecuteStartupAction(userStartupAction);

                    retVal = true;
                    break;
                }
            }
        }
    }

    return retVal;
}

void afExecutionModeManager::AddStartAction(afMainAppWindow* pParentWindow)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pActionsToolbar != nullptr)
    {
        // Get the icon for the start button:
        QPixmap* pActionPixmap = new QPixmap;
        bool rc = acSetIconInPixmap(*pActionPixmap, AC_ICON_EXECUTION_PLAY);
        GT_ASSERT(rc);

        // Create the start action:
        QStringList startActionsList;
        startActionsList << AF_STR_modeToolbarHostLocal;
        startActionsList << AF_STR_modeToolbarHostConfigureSettings;
        m_pStartAction = m_pActionsToolbar->AddToolbutton(pActionPixmap, AF_STR_modeToolbarHostLocal, startActionsList, SIGNAL(clicked()), this, SLOT(OnStartAction()), SLOT(OnHostChange()), SLOT(OnHostsMenuAboutToShow()));

        QString actionName;
        actionName = actionName.sprintf("%s%d", AF_STR_modeToobarButtonBaseName, AF_EXECUTION_ID_START);
        m_pStartAction->setObjectName(actionName);

        m_pStartAction->setData(QVariant(AF_EXECUTION_ID_START));

        // Add the action to the main window so it will always be active:
        pParentWindow->addAction(m_pStartAction);

        // Set the start button shortcut:
        m_pStartAction->UpdateShortcut(QKeySequence(AF_STR_StartDebuggingShortcut));
    }
}

void afExecutionModeManager::AddAPILevelStepsAction(afMainAppWindow* pParentWindow)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(nullptr != m_pActionsToolbar)
    {
        QPixmap apiStepPixmap;
        QPixmap drawStepPixmap;
        QPixmap frameStepPixmap;
        acSetIconInPixmap(apiStepPixmap, AC_ICON_EXECUTION_API_STEP);
        acSetIconInPixmap(drawStepPixmap, AC_ICON_EXECUTION_DRAW_STEP);
        acSetIconInPixmap(frameStepPixmap, AC_ICON_EXECUTION_FRAME_STEP);
        QIcon apiStepIcon(apiStepPixmap);
        QIcon drawStepIcon(drawStepPixmap);
        QIcon frameStepIcon(frameStepPixmap);

        QStringList stepActions;
        stepActions << AF_STR_APIStepDebugging;
        stepActions << AF_STR_DrawStepDebugging;
        stepActions << AF_STR_FrameStepDebugging;

        // Set API step as the initial selection:
        m_pAPILevelStepAction = m_pActionsToolbar->AddToolbutton(&apiStepPixmap, AF_STR_APIStepDebugging, stepActions, SIGNAL(clicked()), this, SLOT(OnActionTriggered()), SLOT(OnAPILevelStepChange()), SLOT(OnAPILevelStepAboutToShow()));
        QString actionName;
        actionName = actionName.sprintf("%s%d", AF_STR_modeToobarButtonBaseName, AF_EXECUTION_LAST_COMMAND_ID + 1);
        m_pAPILevelStepAction->setObjectName(actionName);

        // Add the action to the main window so it will always be active:
        pParentWindow->addAction(m_pAPILevelStepAction);

        // Get the tool button:
        QToolButton* pToolbutton = qobject_cast<QToolButton*>(m_pActionsToolbar->widgetForAction(m_pAPILevelStepAction));

        if (pToolbutton != NULL)
        {
            // Hide the text for this button:
            pToolbutton->setToolButtonStyle(Qt::ToolButtonIconOnly);

            // Get its menu:
            QMenu* pActionsMenu = pToolbutton->menu();

            if (pActionsMenu != NULL)
            {
                // Initialize the actions:
                foreach (QAction* pAction, pActionsMenu->actions())
                {
                    QString actionText = pAction->text();

                    if (AF_STR_APIStepDebugging == actionText)
                    {
                        pAction->setIcon(apiStepIcon);
                        pAction->setData(QVariant(AF_EXECUTION_ID_API_STEP));
                    }
                    else if (AF_STR_DrawStepDebugging == actionText)
                    {
                        pAction->setIcon(drawStepIcon);
                        pAction->setData(QVariant(AF_EXECUTION_ID_DRAW_STEP));
                    }
                    else if (AF_STR_FrameStepDebugging == actionText)
                    {
                        pAction->setIcon(frameStepIcon);
                        pAction->setData(QVariant(AF_EXECUTION_ID_FRAME_STEP));
                    }
                    else
                    {
                        GT_ASSERT(false);
                    }
                }
            }
        }

        // Set the start button shortcut:
        m_pAPILevelStepAction->setData(QVariant(AF_EXECUTION_ID_API_STEP));
        // m_pAPILevelStepAction->UpdateShortcut(QKeySequence(AF_STR_APIStepDebuggingShortcut)); // Do not set the shortcut, to avoid conflicts with the menu items in gwMenuActionsExecutor.
    }
}

void afExecutionModeManager::OnHostChange()
{
    // Downcast the sender to an action:
    QAction* pAction = qobject_cast<QAction*>(sender());
    GT_IF_WITH_ASSERT(pAction != nullptr)
    {
        if (pAction->text() == AF_STR_modeToolbarHostConfigureSettings)
        {
            // Open the project settings, so that the user can configure the remote host id:
            afApplicationCommands::instance()->OnProjectSettingsEditRemoteHost();
        }
        else
        {
            // Extract the project name from the host name:
            QString currentProjectName = acGTStringToQString(afProjectManager::instance().currentProjectSettings().projectName());

            // Get the project name with the selected host:
            QString changedProjectName = afProjectManager::GetProjectNameWithRemoteHost(currentProjectName, pAction->text());

            // If the user decides to change the project, we should set the mode and session type to the current. (in this flow,
            // even though the project is changed, the current mode and session type should be kept [user from his point of view only changed the host]):
            int currentSessionType = m_activeSessionType;
            int currentMode = m_activeMode;

            if (currentProjectName != changedProjectName)
            {
                // Popup a message box, to ask the user if he wants to switch project:
                QString message = QString(AF_STR_modeToolbarHostSelectionProjectChangeQuestion).arg(changedProjectName);
                int userInput = acMessageBox::instance().question(afGlobalVariablesManager::ProductNameA(), message, QMessageBox::Yes | QMessageBox::No);

                if (userInput == QMessageBox::Yes)
                {
                    // Build the project path:
                    osFilePath projectPath;
                    afApplicationCommands::instance()->getProjectsFilePath(acQStringToGTString(changedProjectName), projectPath);

                    // Open the project with the specified host:
                    afApplicationCommands::instance()->OnFileOpenProject(projectPath.asString());

                    gtString lastSessionType;

                    // Check with the current active mode manager if remote host scenario is supported:
                    afIExecutionMode* pExecutionMode = m_executionModes[currentMode];
                    GT_IF_WITH_ASSERT(pExecutionMode != nullptr)
                    {
                        // Find the session type as string:
                        gtString lastSessionTypeName = pExecutionMode->sessionTypeName(currentSessionType);
                        gtString lastActiveMode = pExecutionMode->modeName();

                        if ((afProjectManager::instance().currentProjectSettings().lastActiveMode() != lastActiveMode) || (afProjectManager::instance().currentProjectSettings().lastActiveSessionType() != lastSessionTypeName))
                        {
                            // Change the mode and session type to the previous ones:
                            apExecutionModeChangedEvent executionModeEvent(lastActiveMode, lastSessionTypeName);
                            apEventsHandler::instance().registerPendingDebugEvent(executionModeEvent);
                        }
                    }
                }
            }
        }
    }
}

void afExecutionModeManager::OnAPILevelStepChange()
{
    // Execute the action:
    OnActionTriggered();

    // Update the main button:
    QAction* pAction = qobject_cast<QAction*>(sender());
    GT_IF_WITH_ASSERT(nullptr != pAction)
    {
        afExecutionCommandId actionId = AF_EXECUTION_ID_API_STEP;
        QString actionText = pAction->text();

        if (AF_STR_APIStepDebugging == actionText)
        {
            actionId = AF_EXECUTION_ID_API_STEP;
        }
        else if (AF_STR_DrawStepDebugging == actionText)
        {
            actionId = AF_EXECUTION_ID_DRAW_STEP;
        }
        else if (AF_STR_FrameStepDebugging == actionText)
        {
            actionId = AF_EXECUTION_ID_FRAME_STEP;
        }
        else
        {
            GT_ASSERT(false);
        }

        // Update the "last clicked command" data:
        m_pAPILevelStepAction->UpdateText(actionText);
        m_pAPILevelStepAction->UpdateTooltip(pAction->toolTip());
        m_pAPILevelStepAction->setData(QVariant(actionId));
        m_pAPILevelStepAction->UpdateIcon(pAction->icon());
        // m_pAPILevelStepAction->UpdateShortcut(shortcut); // Do not set the shortcut, to avoid conflicts with the menu items in gwMenuActionsExecutor.
    }
}

void afExecutionModeManager::OnAPILevelStepAboutToShow()
{
    // Do nothing right now
}

void afExecutionModeManager::UpdateHostsList()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pStartAction != nullptr)
    {
        QStringList newHostsList;

        newHostsList << AF_STR_modeToolbarHostLocal;

        // If the current project is not empty:
        gtString currentProjectPath;
        gtString currentProjectName = afProjectManager::instance().currentProjectSettings().projectName();

        if (!currentProjectName.isEmpty())
        {
            // Get the current list of projects:
            gtVector<gtString> recentlyUsedProjectsNames;
            afApplicationCommands::instance()->FillRecentlyUsedProjectsNames(recentlyUsedProjectsNames, currentProjectPath);

            // Find the current project name as QString:
            QString currentProjectNamesStr = acGTStringToQString(currentProjectName);

            // Get the current project name with no host:
            QString currentLocalProjectName = afProjectManager::GetProjectNameWithLocalHost(currentProjectNamesStr);

            // Iterate the projects, and look for the same project name, with a different host:
            // We want to fill the hosts menu with the list of hosts:
            int numberOfRecentProjects = (int)recentlyUsedProjectsNames.size();

            for (int i = 0; i < numberOfRecentProjects; i++)
            {
                gtString tempName;
                osFilePath tempPathStr(recentlyUsedProjectsNames[i]);
                tempPathStr.getFileName(tempName);
                QString tempNameStr = acGTStringToQString(tempName);

                // Get the iterated project name with no host:
                QString tempLocalProjectName = afProjectManager::GetProjectNameWithLocalHost(tempNameStr);

                // If this is the same project name with another host, add it to the list:
                if ((tempLocalProjectName == currentLocalProjectName) && (tempNameStr != currentProjectNamesStr))
                {
                    // Get the host name:
                    QString currentHostName = afProjectManager::GetHostFromProjectName(tempNameStr);

                    if (currentHostName != AF_STR_modeToolbarHostLocal)
                    {
                        newHostsList << currentHostName;
                    }
                }
            }

            // Get the host name from current project:
            QString hostName = afProjectManager::GetHostFromProjectName(currentProjectNamesStr);

            // Set the current host as the text for the start button:
            m_pStartAction->UpdateText(hostName);

            newHostsList << AF_STR_modeToolbarHostConfigureSettings;

            m_pStartAction->UpdateStringList(newHostsList);
        }
    }
}

void afExecutionModeManager::OnHostsMenuAboutToShow()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pStartAction != nullptr)
    {
        bool isRemoteHostEnabled = IsRemoteHostEnabled();

        // Enable / Disable all the actions in the hosts menu:
        m_pStartAction->UpdateMenuItemsEnabled(isRemoteHostEnabled);
    }
}

void afExecutionModeManager::UpdateStartActionVisibility(bool isVisible, bool isEnabled)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pStartAction != nullptr)
    {
        m_pStartAction->UpdateVisible(isVisible);
        m_pStartAction->UpdateEnabled(isEnabled);
    }
}

bool afExecutionModeManager::IsRemoteHostEnabled()
{
    bool retVal = false;

    // Check if a process is running / suspended:
    bool doesProcessExist = afPluginConnectionManager::instance().getCurrentRunModeMask() & AF_DEBUGGED_PROCESS_EXISTS;
    bool isProcessSuspended = afPluginConnectionManager::instance().getCurrentRunModeMask() & AF_DEBUGGED_PROCESS_SUSPENDED;
    bool isProjectLoaded = !afProjectManager::instance().currentProjectSettings().projectName().isEmpty();

    // Enable the hosts list only when we are not running or suspended:
    retVal = !(isProcessSuspended || doesProcessExist);

    // Enable the remote host menu only when there is a loaded project:
    retVal = retVal && isProjectLoaded;

    if (retVal)
    {
        // Check with the current active mode manager if remote host scenario is supported:
        afIExecutionMode* pExecutionMode = activeMode();
        GT_IF_WITH_ASSERT(pExecutionMode != nullptr)
        {
            // Find the session type as string:
            gtString sessionTypeName = pExecutionMode->sessionTypeName(m_activeSessionType);

            // Check if remote is supported for the session type:
            retVal = pExecutionMode->IsRemoteEnabledForSessionType(sessionTypeName);
        }
    }

    return retVal;
}

void afExecutionModeManager::TerminateModes()
{
    // pass through all the modes
    for (int i = 0; i < (int)m_executionModes.size(); i++)
    {
        // Get the current mode:
        afIExecutionMode* pExecutionMode = m_executionModes.at(i);

        if (pExecutionMode != nullptr)
        {
            // let the mode terminate
            pExecutionMode->Terminate();
        }
    }
}