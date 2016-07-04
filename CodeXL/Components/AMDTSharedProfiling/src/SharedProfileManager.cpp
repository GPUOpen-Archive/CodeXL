//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SharedProfileManager.cpp
///
//==================================================================================

#ifdef _WIN32
    #include <Windows.h>
#endif

// QT:
#include <QtCore>
#include <QtWidgets>
#include <QtWidgets/QAction>
#include <QtWidgets/QMessageBox>

// AMDTOSWrappers:
#include <AMDTOSWrappers/Include/osCpuid.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osProcess.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunStartedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessTerminatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afHTMLContent.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afQtCreatorsManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>
#include <AMDTApplicationFramework/Include/dialogs/afAttachToProcessDialog.h>

// Local:
#include <inc/SharedProfileManager.h>
#include <inc/SharedProfileSettingPage.h>
#include <inc/StringConstants.h>

/// Static variable instance
SharedProfileManager* SharedProfileManager::m_pMySingleInstance = NULL;


SharedProfileManager::SharedProfileManager() : QObject(), afIExecutionMode(), afIRunModeManager(),
    m_selectedProfile(PM_STR_SELECT_EMPTY),
    m_menuCmdCount(COUNT_OF_STATIC_PM_MENUS), m_profileIsRunning(false), m_profileSessionProcessCompleted(true), m_paused(false),
    m_importIsRunning(false), m_exportIsRunning(false)
{
    m_profileLookup.insert(ProfileMap::value_type(SPM_VS_CPU_ASSESS_PERF, acQStringToGTString(PM_profileTypeAssesPerformancePrefix)));
    m_profileLookup.insert(ProfileMap::value_type(SPM_VS_CPU_CLU, acQStringToGTString(PM_profileTypeCLUPrefix)));
    m_profileLookup.insert(ProfileMap::value_type(SPM_VS_CPU_CUSTOM, acQStringToGTString(PM_profileTypeCustomProfilePrefix)));
    m_profileLookup.insert(ProfileMap::value_type(SPM_VS_CPU_IBS, acQStringToGTString(PM_profileTypeInstructionBasedSamplingPrefix)));
    m_profileLookup.insert(ProfileMap::value_type(SPM_VS_CPU_L2, acQStringToGTString(PM_profileTypeInvestigateInstructionL2CacheAccessPrefix)));
    m_profileLookup.insert(ProfileMap::value_type(SPM_VS_CPU_BR, acQStringToGTString(PM_profileTypeInvestigateBranchingPrefix)));
    m_profileLookup.insert(ProfileMap::value_type(SPM_VS_CPU_DATA_ACCESS, acQStringToGTString(PM_profileTypeInvestigateDataAccessPrefix)));
    m_profileLookup.insert(ProfileMap::value_type(SPM_VS_CPU_INST_ACCESS, acQStringToGTString(PM_profileTypeInvestigateInstructionAccessPrefix)));
    m_profileLookup.insert(ProfileMap::value_type(SPM_VS_CPU_TIMER, acQStringToGTString(PM_profileTypeTimeBasedPrefix)));

    m_profileLookup.insert(ProfileMap::value_type(SPM_VS_THREAD, acQStringToGTString(PM_profileTypeThreadProfilePrefix)));

    m_profileLookup.insert(ProfileMap::value_type(SPM_VS_GPU_PERF_COUNT, acQStringToGTString(PM_profileTypePerformanceCountersPrefix)));
    m_profileLookup.insert(ProfileMap::value_type(SPM_VS_GPU_APP_TRACE, acQStringToGTString(PM_profileTypeApplicationTracePrefix)));

    m_profileLookup.insert(ProfileMap::value_type(SPM_VS_PP_ONLINE, acQStringToGTString(PM_profileTypePowerProfilePrefix)));

    // Set the default value for m_selectedProfile for the VS to prevent events on start up (bug 361693)
    m_selectedProfile = m_profileLookup.at(SharedProfileManager::SPM_VS_CPU_TIMER);

    // Register as an events observer
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);
}

SharedProfileManager::~SharedProfileManager(void)
{
}

void SharedProfileManager::onProfileEnded()
{
    m_profileIsRunning = false;

    // Ensure everything is enabled and shown properly in VS:
    onSelectProfileMode(false);
    updateApplicationTitle();
    afApplicationCommands::instance()->updateToolbarCommands();
}

void SharedProfileManager::registerProfileType(const gtString& name, spISharedProfilerPlugin* pCallback,
                                               const gtString& projectSettingsTab, int flag)
{
    // Add to profile list
    m_profilesList.push_back(name);
    m_callbackMap.insert(ProfileCallback::value_type(name, pCallback));
    m_projectSettingsMap.insert(ProfileSettingsMap::value_type(name, projectSettingsTab));
    m_flags.insert(FlagMap::value_type(name, flag));

    // Add the profile type to the settings page:
    SharedProfileSettingPage::Instance()->AddProfileType(acGTStringToQString(name));
}

void SharedProfileManager::unregisterProfileType(const gtString& name, void* pCallback)
{
    unsigned int i;

    // if selected
    if (0 == name.compareNoCase(m_selectedProfile))
    {
        //set to default
        m_selectedProfile = PM_STR_SELECT_EMPTY;
    }

    // Remove from profile list
    for (i = 0; i < m_profilesList.size(); i++)
    {
        if (0 == name.compareNoCase(m_profilesList.at(i)))
        {
            m_profilesList.removeItem(i);
            break;
        }
    }

    // Remove from callback map
    ProfileCallback::iterator it = m_callbackMap.begin();
    ProfileCallback::iterator itEnd = m_callbackMap.end();

    for (; it != itEnd; it++)
    {
        if ((0 == name.compareNoCase((*it).first))
            && (pCallback == (*it).second))
        {
            m_callbackMap.erase(it);
            break;
        }
    }
}

SharedProfileManager& SharedProfileManager::instance()
{
    // If this class single instance was not already created:
    if (NULL == m_pMySingleInstance)
    {
        // Create it:
        m_pMySingleInstance = new SharedProfileManager;

    }

    return *m_pMySingleInstance;
}

const gtString SharedProfileManager::currentSelection()
{
    return m_selectedProfile;
}

bool SharedProfileManager::SelectProfileType(const gtString& selection)
{
    bool retVal = false;
    gtVector<gtString>::const_iterator it = m_profilesList.begin();
    gtVector<gtString>::const_iterator itEnd = m_profilesList.end();

    //check to see if the selection is a registered profile
    int index = 0;

    for (; it != itEnd; it++, index++)
    {
        if (*it == selection)
        {
            updateSelected(selection);

            retVal = true;
            break;
        }
    }

    return retVal;
}

void SharedProfileManager::updateSelected(const gtString& selected)
{
    if (selected != m_selectedProfile)
    {
        m_selectedProfile = selected;
        onSelectProfileMode(true);
        emit profileSelectionChanged(selected);
    }
}

void SharedProfileManager::onInvokeAttachToProcess()
{
    // check if  SharedProfileSettingPage radio box is set to PM_PROFILE_SCOPE_SYS_WIDE_FOCUS_ON_EXE
    bool isSysWideProfiling = false;
    GT_IF_WITH_ASSERT(SharedProfileSettingPage::Instance() != NULL)
    {
        isSysWideProfiling = (SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope == PM_PROFILE_SCOPE_SYS_WIDE_FOCUS_ON_EXE);
    }
    afAttachToProcessDialog attachToProcess(isSysWideProfiling);

    //make the dialog window modal
    int rc = QDialog::Rejected;
    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
        rc = pApplicationCommands->showModal(&attachToProcess);
    }

    // after dialog box is closed
    if (rc == QDialog::Accepted)
    {
        // check if need to update Profile Setting Page - update only if different from current
        bool isSysWide = false;
        isSysWide = attachToProcess.IsSystemWideProfiling();

        if (isSysWide != isSysWideProfiling)
        {
            if (isSysWide)
            {
                SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE_FOCUS_ON_EXE;
            }
            else
            {
                SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope = PM_PROFILE_SCOPE_SINGLE_EXE;
            }

            //update SharedProfileSettingPage UI by the changes made to m_currentSettings of settingPage
            SharedProfileSettingPage::Instance()->RestoreCurrentSettings();
        }

        osProcessId processId = attachToProcess.GetProcessID();
        bool isProjectLoaded = !afProjectManager::instance().currentProjectFilePath().isEmpty();

        if (!isProjectLoaded)
        {
            afApplicationCommands::instance()->CreateDefaultProject(PM_STR_PROFILE_MODE);
        }

        onStartAction(processId);
    }
}

void SharedProfileManager::onStartAction(osProcessId processId)
{
    // Start the session in log file:
    osDebugLog::instance().StartSession();

    // Check if project is mandatory for start action:
    bool isProjectMandatory = !IsStartupActionSupportedWithNoProject(AF_EXECUTION_ID_START);

    bool isProjectLoaded = !afProjectManager::instance().currentProjectFilePath().isEmpty();

    if (!isProjectLoaded && isProjectMandatory)
    {
        // Display the startup dialog:
        afExecutionModeManager::instance().DisplayStartupDialog();
    }
    else
    {
        // Look for exe path only if the process id is null:
        bool shouldCheckExe = (processId == 0);

        bool isProfileSettingsOK = isProfilingOkay(shouldCheckExe);

        if (!isProfileSettingsOK)
        {
            HandleInvalidProjectSettings(isProfileSettingsOK, processId);
        }

        // For system wide - set the process ID to unbound:
        if (SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope == PM_PROFILE_SCOPE_SYS_WIDE)
        {
            processId = PROCESS_ID_UNBOUND;
        }

        // Check if the settings are now OK:
        if (isProfileSettingsOK)
        {
            m_profileIsRunning = true;

            gtString propertiesHTMLMessage;
            afHTMLContent htmlContent(AF_STR_PropertiesProcessRunning);
            htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, PM_STR_PROCESS_IS_RUNNING_MESSAGE);
            htmlContent.toString(propertiesHTMLMessage);
            afPropertiesView* pPropertiesView = afApplicationCommands::instance()->propertiesView();
            GT_IF_WITH_ASSERT(pPropertiesView != NULL)
            {
                pPropertiesView->setHTMLText(acGTStringToQString(propertiesHTMLMessage), NULL);
            }

            // Make sure the toolbars are updated so double clicked for start won't happen.
            // The timer that updates the toolbars are too slow
            afApplicationCommands::instance()->updateToolbarCommands();

            emit profileStarted(m_selectedProfile, m_callbackMap[m_selectedProfile], processId);
        }

    }

    updateApplicationTitle();
}

bool SharedProfileManager::isProfilingOkay(bool checkEXEFile)
{
    bool retVal = false;

    if (0 == m_selectedProfile.compareNoCase(PM_STR_SELECT_EMPTY))
    {
        gtVector<gtString>::const_iterator it = m_profilesList.begin();
        gtVector<gtString>::const_iterator itEnd = m_profilesList.end();

        if (it != itEnd)
        {
            updateSelected(*it);
        }
    }

    bool isSessionTypeValid = !m_selectedProfile.isEmpty();
    bool isProjectLoaded = !afProjectManager::instance().currentProjectFilePath().asString().isEmpty();
    bool canRunWithNoProject = false;
    bool isExeSet = !afProjectManager::instance().currentProjectSettings().executablePath().isEmpty();

    if (!isExeSet)
    {
        spISharedProfilerPlugin* pPlugin = m_callbackMap[m_selectedProfile];

        if (NULL != pPlugin)
        {
            isExeSet = pPlugin->IsSpecialExetableCaseSet();
            canRunWithNoProject = !pPlugin->CanRunWithoutProject();
        }
    }

    bool isProfileActive = afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE);
    retVal = isSessionTypeValid && (isProjectLoaded || canRunWithNoProject) && isProfileActive;

    if (checkEXEFile)
    {
        retVal = retVal && isExeSet;
    }

    return retVal;
}

bool SharedProfileManager::isProfiling()
{
    return m_profileIsRunning;
}

gtString SharedProfileManager::modeName()
{
    return PM_STR_PROFILE_MODE;
}

gtString SharedProfileManager::modeActionString()
{
    return PM_STR_PROFILE_MODE_ACTION;
}

gtString SharedProfileManager::modeVerbString()
{
    return PM_STR_PROFILE_MODE_VERB;
}

gtString SharedProfileManager::modeDescription()
{
    return PM_STR_PROFILE_MODE_DESCRIPTION;
}

void SharedProfileManager::updateApplicationTitle()
{
    // only update the application title in standalone (doesn't make sense in VS - vspApplicationCommands::setApplicationCaption will assert)
    if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
        GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
        {
            // Set the application caption:
            gtString titleBarString;
            afCalculateCodeXLTitleBarString(titleBarString);
            afApplicationCommands::instance()->setApplicationCaption(titleBarString);

            // Update the tree root text:
            afApplicationTree* pApplicationTree = pApplicationCommands->applicationTree();
            GT_IF_WITH_ASSERT(pApplicationTree != NULL)
            {
                pApplicationTree->updateTreeRootText();
            }
        }
    }
}

void SharedProfileManager::execute(afExecutionCommandId commandId)
{
    switch (commandId)
    {
        case AF_EXECUTION_ID_START:
            onStartAction();
            break;

        case AF_EXECUTION_ID_BREAK:
            onPauseToggle();
            break;

        case AF_EXECUTION_ID_STOP:
            stopCurrentRun();
            break;

        default: break;
    }
}

void SharedProfileManager::onPauseToggle()
{
    m_paused = !m_paused;
    emit profileBreak(m_paused, m_callbackMap[m_selectedProfile]);
    updateApplicationTitle();
}

bool SharedProfileManager::isStartEnabled(bool& checkable, bool& checked)
{
    checkable = false;
    checked = false;
    return afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE) &&
           (!m_profileIsRunning) &&
           (!m_importIsRunning) &&
           (m_profileSessionProcessCompleted);
}

bool SharedProfileManager::isAttachEnabled(bool& checkable, bool& checked)
{
    bool retVal = isStartEnabled(checkable, checked);

    if (retVal)
    {
        spISharedProfilerPlugin* pPlugin = m_callbackMap[m_selectedProfile];

        if (NULL != pPlugin)
        {
            retVal = pPlugin->IsAttachEnabled();
        }
    }

    return retVal;
}

bool SharedProfileManager::isProfileModeEnabled(bool& checkable, bool& checked)
{
    checkable = true;

    // Check if in profile mode:
    bool profileMode = (afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE));

    // Check if anything is running at all:
    afRunModes runModes = afPluginConnectionManager::instance().getCurrentRunModeMask();

    // Set the correct return values:
    checked = profileMode;

    // Enable if not in profile and noting is running (can not enable if debugging even if not in profile):
    bool enabled = (!(profileMode) && (runModes == 0));

    return enabled;
}

bool SharedProfileManager::isProfileEnabled(int profileIndex, bool& checkable, bool& checked)
{
    if ((profileIndex < 0) || (profileIndex > (int)(m_profilesList.size() - 1)))
    {
        return false;
    }

    spISharedProfilerPlugin* pPlugin = m_callbackMap[m_profilesList.at(profileIndex)];

    if (NULL != pPlugin)
    {
        if (!pPlugin->IsProfileEnabled())
        {
            return false;
        }
    }

    checkable = true;
    checked = (0 == m_selectedProfile.compareNoCase(m_profilesList.at(profileIndex)));
    return (!m_profileIsRunning)
           && (afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE));
}

bool SharedProfileManager::isProjectSettingsEnabled(bool& checkable, bool& checked)
{
    checkable = false;
    checked = false;
    return (!m_profileIsRunning)
           && (afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE));
}

bool SharedProfileManager::isPausedEnabled(bool& checkable, bool& checked, bool& shown)
{
    shown = !(SPM_HIDE_PAUSE & m_flags[m_selectedProfile]);
    checkable = ((isProfilingOkay(false))
                 && (0 != (SPM_ALLOW_PAUSE & m_flags[m_selectedProfile]))
                 && (m_profileIsRunning));
    checked = m_paused
              && (m_profileIsRunning);
    return ((isProfilingOkay(false))
            && (0 != (SPM_ALLOW_PAUSE & m_flags[m_selectedProfile]))
            && (m_profileIsRunning));
}

bool SharedProfileManager::isStopEnabled(bool& checkable, bool& checked)
{
    bool retVal = false;
    checkable = false;
    checked = false;

    bool isStopEnabledForCurrentProfile = SPM_ALLOW_STOP & m_flags[m_selectedProfile];
    retVal = isProfilingOkay(false) && isStopEnabledForCurrentProfile && m_profileIsRunning;
    return retVal;
}

void SharedProfileManager::updateUI(afExecutionCommandId commandId, QAction* pAction)
{
    // if import is running don't update UI
    if (m_importIsRunning)
    {
        return;
    }

    bool isActionEnabled = false;
    bool isActionToggled = false;
    bool canBeToggled = false;
    bool isActionVisible = true;


    GT_IF_WITH_ASSERT(NULL != pAction)
    {
        pAction->setVisible(true);

        switch (commandId)
        {
            case AF_EXECUTION_ID_START:
            {
                isActionEnabled = isStartEnabled(canBeToggled, isActionToggled);
            }
            break;

            case AF_EXECUTION_ID_BREAK:
            {
                isActionEnabled = isPausedEnabled(canBeToggled, isActionToggled, isActionVisible);
                pAction->setText(AF_STR_PauseProfiling);
            }
            break;

            case AF_EXECUTION_ID_STOP:
                isActionEnabled = isStopEnabled(canBeToggled, isActionToggled);
                break;

            case AF_EXECUTION_ID_API_STEP:
            case AF_EXECUTION_ID_DRAW_STEP:
            case AF_EXECUTION_ID_FRAME_STEP:
            case AF_EXECUTION_ID_STEP_IN:
            case AF_EXECUTION_ID_STEP_OVER:
            case AF_EXECUTION_ID_STEP_OUT:
            case AF_EXECUTION_ID_CANCEL_BUILD:
            case AF_EXECUTION_ID_BUILD:
            case AF_EXECUTION_ID_CAPTURE:
            case AF_EXECUTION_ID_CAPTURE_CPU:
            case AF_EXECUTION_ID_CAPTURE_GPU:
            {
                isActionVisible = false;
                isActionEnabled = false;
                break;
            }
            break;

            default:
                pAction->setVisible(false);
                break;
        }
    }

    pAction->setEnabled(isActionEnabled);
    pAction->setCheckable(canBeToggled);
    pAction->setChecked(isActionToggled);
    pAction->setVisible(isActionVisible);

    // enable the specific plugin to overwrite/add the default updateUI behavior:
    spISharedProfilerPlugin* pPlugin = m_callbackMap[m_selectedProfile];

    if (NULL != pPlugin)
    {
        pPlugin->updateUI(commandId, pAction);
    }

}

void SharedProfileManager::execute(int sessionTypeIndex)
{
    GT_IF_WITH_ASSERT((sessionTypeIndex >= 0) && (sessionTypeIndex < (int)m_profilesList.size()))
    {
        gtString selectedProfile = m_profilesList[sessionTypeIndex];
        updateSelected(selectedProfile);
    }
}

void SharedProfileManager::updateUI(int sessionTypeIndex, QAction* pAction)
{
    bool isActionCheckable = false, isActionChecked = false;
    bool isEnabled = isProfileEnabled(sessionTypeIndex, isActionCheckable, isActionChecked);
    GT_IF_WITH_ASSERT(pAction != NULL)
    {
        // bool isProfile = afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE);
        pAction->setCheckable(isActionCheckable);
        pAction->setChecked(isActionChecked);
        pAction->setEnabled(isEnabled);
    }
}

gtString SharedProfileManager::sessionTypeName(int sessionTypeIndex)
{
    gtString retVal;
    GT_IF_WITH_ASSERT((sessionTypeIndex < (int)m_profilesList.size()) && (sessionTypeIndex >= 0))
    {
        retVal = m_profilesList[sessionTypeIndex];
    }
    return retVal;
}

QPixmap* SharedProfileManager::sessionTypeIcon(int sessionTypeIndex)
{
    (void)(sessionTypeIndex); // unused
    QPixmap* pPixmap = new QPixmap;
    acSetIconInPixmap(*pPixmap, AC_ICON_PROFILE_MODE);

    return pPixmap;
}

void SharedProfileManager::setPaused(bool toggled)
{
    m_paused = toggled;
}

int SharedProfileManager::indexForSessionType(const gtString& selected)
{
    int index(-1);
    gtVector<gtString>::const_iterator it = m_profilesList.begin();
    gtVector<gtString>::const_iterator itEnd = m_profilesList.end();

    //check to see if the selection is a registered profile
    int find = 0;

    for (; it != itEnd; it++, find++)
    {
        if (*it == selected)
        {
            index = find;
            break;
        }
    }

    return index;
}

// return the layout name used for this mode at specific time:
afMainAppWindow::LayoutFormats SharedProfileManager::layoutFormat()
{
    afMainAppWindow::LayoutFormats retVal = afMainAppWindow::LayoutProfileGPU;

    spISharedProfilerPlugin* pPlugin = m_callbackMap[m_selectedProfile];

    if (NULL != pPlugin)
    {
        retVal = pPlugin->LayoutFormat();
    }

    return retVal;
}

/// return the project settings path used for this mode at specific time:
gtString SharedProfileManager::ProjectSettingsPath()
{
    gtString retVal = AF_STR_LayoutProfileGPU;

    spISharedProfilerPlugin* pPlugin = m_callbackMap[m_selectedProfile];

    if (NULL != pPlugin)
    {
        retVal = pPlugin->ProjectSettingsPath();
    }

    return retVal;
}

/// return the project settings path used for this mode at specific time:
gtString SharedProfileManager::HowToStartModeExecutionMessage()
{
    gtString retStr = PM_STR_PropertiesExecutionInformationSA;

    if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        retStr = PM_STR_PropertiesExecutionInformationVS;
    }

    return retStr;
}

/// Allow the mode to terminate gracefully at the end of CodeXL. by default nothing needs to be done
void SharedProfileManager::Terminate()
{
    // kill the gpu profile backend process since there is no other way to kill it,
    // if it started by the user and we terminated in the middle
    gtVector<gtString> processNames;
    processNames.push_back(L"CodeXLGpuProfiler");

    osProcessId currentProcessId = osGetCurrentProcessId();
    osTerminateProcessesByName(processNames, currentProcessId, false);
}

void SharedProfileManager::GetToolbarStartButtonText(gtString& buttonText, bool fullString /*= true*/)
{
    buttonText = PM_STR_startButtonProfileMode;
    gtString currentType = sessionTypeName(afExecutionModeManager::instance().activeSessionType());
    if (currentType.startsWith(L"CPU"))
    {
        buttonText = PM_STR_startButtonProfileCPUMode;
    }
    else if (currentType.startsWith(L"GPU") || currentType.startsWith(PM_profileTypeApplicationTraceWide))
    {
        buttonText = PM_STR_startButtonProfileGPUMode;
    }
    else
    {
        buttonText = PM_STR_startButtonProfilePowerMode;
    }

    gtString exeFileName;
    afProjectManager::instance().currentProjectSettings().executablePath().getFileNameAndExtension(exeFileName);
    bool isSystemWide = (SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope == PM_PROFILE_SCOPE_SYS_WIDE);

    // add the (..) section if in profile and full info is needed
    if (isSystemWide && fullString)
    {
        if (!exeFileName.isEmpty())
        {
            buttonText.appendFormattedString(PM_STR_startButtonExeAndSystemWide, exeFileName.asCharArray());
        }
        else
        {
            buttonText.append(PM_STR_startButtonSystemWide);
        }
    }
}

///Visual studio direct ui check
bool SharedProfileManager::enableVsProfileAction(int vsId, bool& shouldCheck, bool& shouldShow)
{
    bool enabled(false);
    bool checkable;
    shouldShow = true;

    switch (vsId)
    {
        case SPM_VS_START:
            enabled = isStartEnabled(checkable, shouldCheck);
            break;

        case SPM_VS_ATTACH:
            enabled = isAttachEnabled(checkable, shouldCheck);
            break;

        case SPM_VS_PAUSE:
            enabled = isPausedEnabled(checkable, shouldCheck, shouldShow);
            break;

        case SPM_VS_STOP:
            enabled = isStopEnabled(checkable, shouldCheck);
            break;

        case SPM_VS_PROFILE_MODE:
            enabled = isProfileModeEnabled(checkable, shouldCheck);
            break;

        case SPM_VS_CPU_ASSESS_PERF:
        case SPM_VS_CPU_CLU:
        case SPM_VS_CPU_CUSTOM:
        case SPM_VS_CPU_IBS:
        case SPM_VS_CPU_L2:
        case SPM_VS_CPU_BR:
        case SPM_VS_CPU_DATA_ACCESS:
        case SPM_VS_CPU_INST_ACCESS:
        case SPM_VS_CPU_TIMER:
        case SPM_VS_GPU_PERF_COUNT:
        case SPM_VS_GPU_APP_TRACE:
        case SPM_VS_PP_ONLINE:
            enabled = isProfileEnabled(indexForSessionType(m_profileLookup.at(vsId)),
                                       checkable, shouldCheck);
            break;

        default:
            GT_ASSERT_EX(false, L"Unsupported application command");
            break;
    }

    return enabled;
}

void SharedProfileManager::onSelectProfileMode(bool updateOnlySessionIndex)
{
    // Check what is the current mode and session type, and register an event if needed:
    bool isProfile = afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE);
    bool isSessionType = (afExecutionModeManager::instance().activeSessionType() == indexForSessionType(m_selectedProfile));

    if (!isProfile || !isSessionType)
    {
        apExecutionModeChangedEvent executionModeEvent(PM_STR_PROFILE_MODE, indexForSessionType(m_selectedProfile), updateOnlySessionIndex);
        apEventsHandler::instance().registerPendingDebugEvent(executionModeEvent);
    }
}

///Visual studio direct action
void SharedProfileManager::vsProfileAction(int vsId)
{
    switch (vsId)
    {
        case SPM_VS_START:
            onStartAction();
            break;

        case SPM_VS_ATTACH:
            onInvokeAttachToProcess();
            break;

        case SPM_VS_PAUSE:
            onPauseToggle();
            break;

        case SPM_VS_STOP:
            stopCurrentRun();
            break;

        case SPM_VS_PROFILE_MODE:
            onSelectProfileMode(false);
            break;

        case SPM_VS_CPU_ASSESS_PERF:
        case SPM_VS_CPU_CLU:
        case SPM_VS_CPU_CUSTOM:
        case SPM_VS_CPU_IBS:
        case SPM_VS_CPU_L2:
        case SPM_VS_CPU_BR:
        case SPM_VS_CPU_DATA_ACCESS:
        case SPM_VS_CPU_INST_ACCESS:
        case SPM_VS_CPU_TIMER:
        case SPM_VS_GPU_PERF_COUNT:
        case SPM_VS_GPU_APP_TRACE:
        case SPM_VS_PP_ONLINE:
            updateSelected(m_profileLookup.at(vsId));
            break;

        default:
            GT_ASSERT_EX(false, L"Unsupported application command");
            break;
    }
}

const gtVector<gtString>& SharedProfileManager::profiles()
{
    return m_profilesList;
}

afRunModes SharedProfileManager::getCurrentRunModeMask()
{
    afRunModes retVal = 0;

    if (m_profileIsRunning)
    {
        retVal |= AF_DEBUGGED_PROCESS_EXISTS;

        if (m_paused)
        {
            retVal |= AF_DEBUGGED_PROCESS_PAUSED;
        }
        else
        {
            retVal |= AF_DEBUGGED_PROCESS_RUNNING;
        }
    }

    return retVal;
}

bool SharedProfileManager::canStopCurrentRun()
{
    return m_profileIsRunning;
}

bool SharedProfileManager::stopCurrentRun()
{
    // End the session in the log file:
    osDebugLog::instance().EndSession();

    emit profileStopped(m_callbackMap[m_selectedProfile], m_stopAndExit);
    return true;
}

bool SharedProfileManager::getExceptionEventDetails(const apExceptionEvent& exceptionEve,
                                                    osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded,
                                                    bool& kernelDebuggingEnteredAtLeastOnce)
{
    (void)(exceptionEve); // unused
    (void)(exceptionCallStack); // unused
    (void)(openCLEnglineLoaded); // unused
    (void)(openGLEnglineLoaded); // unused
    (void)(kernelDebuggingEnteredAtLeastOnce); // unused
    //Do we need this if not debugging?
    return true;
}

void SharedProfileManager::onInvokeProjectSettings()
{
    afApplicationCommands* pCommands = afApplicationCommands::instance();

    // Sanity check:
    GT_IF_WITH_ASSERT(pCommands != NULL)
    {
        pCommands->OnProjectSettings(m_projectSettingsMap[m_selectedProfile]);
    }
}

void SharedProfileManager::onEvent(const apEvent& eve, bool& vetoEvent)
{
    Q_UNUSED(vetoEvent);
    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    switch (eventType)
    {

        case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
        {
            GlobalVariableChangedHandler((const afGlobalVariableChangedEvent&)eve);
            break;
        }

        case apEvent::AP_EXECUTION_MODE_CHANGED_EVENT:
        {
            if (afExecutionModeManager::instance().isActiveMode(PM_STR_PROFILE_MODE))
            {
                // Update the profile type menu:
                UpdateProfileMenuItemText();

                // Update the settings page with the new profile type:
                const apExecutionModeChangedEvent& execChangedEvent = (const apExecutionModeChangedEvent&)eve;

                if (!execChangedEvent.sessionTypeName().isEmpty())
                {
                    SharedProfileSettingPage::Instance()->OnProfileTypeChanged(execChangedEvent.sessionTypeName());
                }
            }

            break;
        }

        default:
            // Do nothing:
            break;
    }

}

const wchar_t* SharedProfileManager::eventObserverName() const
{
    return L"SharedProfileManager";
}

void SharedProfileManager::GlobalVariableChangedHandler(const afGlobalVariableChangedEvent& event)
{
    // Get id of the global variable that was changed
    afGlobalVariableChangedEvent::GlobalVariableId variableId = event.changedVariableId();

    SharedProfileSettingPage::Instance()->SetAfterProjectLoadedStatus(false);

    // If the project file path was changed
    if (variableId == afGlobalVariableChangedEvent::CURRENT_PROJECT)
    {
        SelectProfileType(afProjectManager::instance().currentProjectSettings().lastActiveSessionType());
    }

    SharedProfileSettingPage::Instance()->SetAfterProjectLoadedStatus(true);
}

void SharedProfileManager::setImportIsRunning(bool running)
{
    m_importIsRunning = running;
}

bool SharedProfileManager::isImportIsRunning()const
{
    return m_importIsRunning;
}

void SharedProfileManager::setExportIsRunning(bool running)
{
    m_exportIsRunning = running;
}

bool SharedProfileManager::isExportIsRunning()const
{
    return m_exportIsRunning;
}

void SharedProfileManager::SetProfileSessionProcessingComplete(bool isProcessingComplete)
{
    m_profileSessionProcessCompleted = isProcessingComplete;
}

int SharedProfileManager::numberSessionTypes()
{
    int retVal = 1;

    if (m_profilesList.size() > 0)
    {
        retVal = (int)m_profilesList.size();
    }

    return retVal;
}

QString SharedProfileManager::FindStartProfileActionText(bool getTooltip)
{
    QString retVal = PM_STR_MENU_START_NO_PARAMS;
    ProfileSessionScope scope = SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope;
    bool isExeSet = !afProjectManager::instance().currentProjectSettings().executablePath().isEmpty();

    QString argsStr;

    if ((scope == PM_PROFILE_SCOPE_SYS_WIDE) || ((scope == PM_PROFILE_SCOPE_SYS_WIDE_FOCUS_ON_EXE) && (!isExeSet)))
    {
        argsStr = PM_STR_SYSTEM_WIDE;
    }
    else if (isExeSet)
    {
        gtString fileName;
        afProjectManager::instance().currentProjectSettings().executablePath().getFileNameAndExtension(fileName);

        if (!argsStr.isEmpty())
        {
            argsStr += " + ";
        }

        argsStr += acGTStringToQString(fileName);
    }

    if (!argsStr.isEmpty())
    {
        retVal = QString(PM_STR_MENU_START_WITH_PARAMS).arg(argsStr);
    }

    if (getTooltip)
    {
        retVal.replace('\t', ' ');
    }

    return retVal;
}

void SharedProfileManager::UpdateProfileMenuItemText()
{
    QString actionText = FindStartProfileActionText();

    afExecutionModeManager::instance().UpdateStartActionTooltip(actionText);
}

void SharedProfileManager::HandleInvalidProjectSettings(bool& isProfileSettingsOK, osProcessId& processId)
{
    spISharedProfilerPlugin* pPlugin = m_callbackMap[m_selectedProfile];

    if (NULL != pPlugin)
    {
        pPlugin->HandleInvalidProjectSettings(isProfileSettingsOK, processId);
    }
}

bool SharedProfileManager::ExecuteStartupAction(afStartupAction action)
{
    bool retVal = false;

    if ((action == AF_NO_PROJECT_USER_ACTION_CREATE_NEW_PROJECT_PROFILE) || (action == AF_NO_PROJECT_USER_ACTION_CREATE_NEW_PROJECT_FRAME_ANALYZE))
    {
        afApplicationCommands::instance()->OnFileNewProject();
        retVal = true;
    }

    else if (action == AF_NO_PROJECT_USER_ACTION_CPU_PROFILE_SYSTEM_WIDE)
    {
        afApplicationCommands::instance()->CreateDefaultProject(PM_STR_PROFILE_MODE);

        SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE;
        onStartAction(PROCESS_ID_UNBOUND);
        retVal = true;
    }
    else if (action == AF_NO_PROJECT_USER_ACTION_CPU_PROFILE_ATTACH_TO_PROCESS)
    {
        onInvokeAttachToProcess();
        retVal = true;
    }

    return retVal;
}

bool SharedProfileManager::IsStartupActionSupported(afStartupAction action)
{
    bool retVal = false;

    if ((action == AF_NO_PROJECT_USER_ACTION_CREATE_NEW_PROJECT_PROFILE) ||
        (action == AF_NO_PROJECT_USER_ACTION_CPU_PROFILE_SYSTEM_WIDE) ||
        (action == AF_NO_PROJECT_USER_ACTION_CPU_PROFILE_ATTACH_TO_PROCESS))
    {
        retVal = true;
    }

    return retVal;
}

bool SharedProfileManager::IsStartupActionSupportedWithNoProject(afExecutionCommandId commandId)
{
    bool retVal = false;

    // Power profile supports start with no project:
    if (commandId == AF_EXECUTION_ID_START)
    {
        if (m_selectedProfile == m_profileLookup.at(SharedProfileManager::SPM_VS_PP_ONLINE))
        {
            retVal = true;
        }
    }

    return retVal;
}

bool SharedProfileManager::IsRemoteEnabledForSessionType(const gtString& sessionType)
{
    bool retVal = false;

    // Currently, remote host profiling is supported only for GPU profile and power profile:
    QString sessionTypeStr = acGTStringToQString(sessionType);

    if ((sessionTypeStr == PM_profileTypePowerProfilePrefix) || (sessionTypeStr == PM_profileTypePerformanceCountersPrefix) || (sessionTypeStr == PM_profileTypeApplicationTracePrefix))
    {
        retVal = true;
    }

    return retVal;
}

