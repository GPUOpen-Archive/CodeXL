//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppAppController.cpp
///
//==================================================================================

//------------------------------ ppEventObserver.cpp ------------------------------

// Qt:
#include <QtWidgets>

#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <unistd.h>
#endif

// Infra:
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osMachine.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEvent.h>
#include <AMDTAPIClasses/Include/Events/apProfileProcessTerminatedEvent.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <unistd.h>
#endif


// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/views/afApplicationTreeItemData.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/SharedProfileSettingPage.h>
#include <AMDTSharedProfiling/inc/StringConstants.h>
#include <AMDTSharedProfiling/inc/SharedProfileManager.h>
#include <AMDTSharedProfiling/inc/SharedProfileProcessMonitor.h>
#include <AMDTSharedProfiling/inc/SessionTreeNodeData.h>
#include <AMDTSharedProfiling/inc/ProfileApplicationTreeHandler.h>

// Local:
#include <AMDTPowerProfiling/Include/ppAppWrapper.h>
#include <AMDTPowerProfiling/Include/ppWindowsManagerHelper.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>
#include <AMDTPowerProfiling/src/ppAppController.h>
#include <AMDTPowerProfiling/src/ppColors.h>
#include <AMDTPowerProfiling/src/ppMDIViewCreator.h>
#include <AMDTPowerProfiling/src/ppSessionView.h>
#include <AMDTPowerProfiling/src/ppTreeHandler.h>
#include <AMDTPowerProfiling/src/ppAidFunctions.h>

// BL:
#include <AMDTPowerProfilingMidTier/include/PowerProfilerBL.h>

// Remote.
// This mechanism will be refactored (definition should go to a common header).
#include <AMDTRemoteClient/Include/CXLDaemonClient.h>

ppAppController* ppAppController::m_spMySingleInstance = nullptr;

ppProjectSettings::ppProjectSettings() : m_samplingInterval(PP_DEFAULT_SAMPLING_INTERVAL)
{
}
ppProjectSettings::~ppProjectSettings()
{
}


// ---------------------------------------------------------------------------
// Name:        ppEventObserver::ppEventObserver
// Description: Constructor
// Author:      Gilad Yarnitzky
// Date:        25/8/2014
// ---------------------------------------------------------------------------
ppAppController::ppAppController() : m_pApplicationCommands(nullptr), m_pApplicationTree(nullptr), m_midTierLastInitError(PPR_NO_ERROR), m_isMidTierInitialized(false),
    m_pMonitorProcessThread(nullptr), m_sessionIsOn(false), m_pCurrentRunningSessionData(nullptr), m_pWindowsManagerHelper(nullptr)
{
    if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        // Register as an events observer:
        apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);
    }

    // 'Register' to the shared profiling manager
    //Connect to the shared profile manager
    bool rcConnect = connect(&(SharedProfileManager::instance()), SIGNAL(profileStarted(const gtString&, const spISharedProfilerPlugin * const, osProcessId)),
                             this, SLOT(onProfileStarted(const gtString&, const spISharedProfilerPlugin * const, osProcessId)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(&(SharedProfileManager::instance()), SIGNAL(profileBreak(const bool&, const spISharedProfilerPlugin * const)),
                        this, SLOT(onProfilePaused(const bool&, const spISharedProfilerPlugin * const)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(&(SharedProfileManager::instance()), SIGNAL(profileStopped(const spISharedProfilerPlugin * const, bool)),
                        this, SLOT(onProfileStopped(const spISharedProfilerPlugin * const, bool)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(ProfileApplicationTreeHandler::instance(), SIGNAL(SessionRenamed(SessionTreeNodeData*, const osFilePath&, const osDirectory&)),
                        this, SLOT(OnSessionRename(SessionTreeNodeData*, const osFilePath&, const osDirectory&)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(ProfileApplicationTreeHandler::instance(), SIGNAL(BeforeSessionRename(SessionTreeNodeData*, bool&, QString&)),
                        this, SLOT(OnBeforeSessionRename(SessionTreeNodeData*, bool&, QString&)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(ProfileApplicationTreeHandler::instance(), SIGNAL(FileImported(const QString&, bool&)), this, SLOT(OnImportSession(const QString&, bool&)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(ProfileApplicationTreeHandler::instance(), SIGNAL(SessionDeleted(ExplorerSessionId, SessionExplorerDeleteType, bool&)), this, SLOT(OnSessionDelete(ExplorerSessionId, SessionExplorerDeleteType, bool&)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(this, SIGNAL(RemoteRuntimeEvent(ppQtPwrProfErrorCode)), this, SLOT(onRemoteRuntimeEvent(ppQtPwrProfErrorCode)));
    GT_ASSERT(rcConnect);

    // Add the power profile sessions extension to import list:
    ProfileApplicationTreeHandler::instance()->AddImportFileFilter(PP_STR_PowerProfileFileExtensionCaption, PP_STR_dbFileExtSearchString, PM_STR_PROFILE_MODE);

    // If the current hardware doesn't support power profile, add a message to the output window:
    InitMiddleTier();

    if (m_midTierLastInitError == PPR_NOT_SUPPORTED)
    {
        afGlobalVariablesManager::instance().AppendUnsupportedMessage(PP_STR_PowerProfilerNotSupportedOnHW);
    }

}

// ---------------------------------------------------------------------------
ppAppController& ppAppController::instance()
{
    // If this class single instance was not already created:
    if (m_spMySingleInstance == nullptr)
    {
        // Create it:
        m_spMySingleInstance = new ppAppController;
        GT_ASSERT(m_spMySingleInstance);
    }

    return *m_spMySingleInstance;
}

// ---------------------------------------------------------------------------
ppAppController::~ppAppController()
{
    // Register as an events observer:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}

// ---------------------------------------------------------------------------
void ppAppController::onEvent(const apEvent& eve, bool& vetoEvent)
{
    GT_UNREFERENCED_PARAMETER(vetoEvent);

    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    // handle the Global var changed event
    switch (eventType)
    {
        case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
        {
            const afGlobalVariableChangedEvent& globalVarChangedEvent = dynamic_cast<const afGlobalVariableChangedEvent&>(eve);
            // Get id of the global variable that was changed
            afGlobalVariableChangedEvent::GlobalVariableId variableId = globalVarChangedEvent.changedVariableId();

            // If the project file path was changed
            if (variableId == afGlobalVariableChangedEvent::CURRENT_PROJECT)
            {
                // If project is empty, set the default counters:
                const apProjectSettings& projSettings = afProjectManager::instance().currentProjectSettings();

                //on Project open
                if (projSettings.projectName().isEmpty())
                {
                    gtVector<int> defaultCounters;
                    GetDefaultCounters(defaultCounters);

                    // set enabled counters to default values
                    SetCurrentProjectEnabledCounters(defaultCounters);
                }


                // Make sure we closed an already open power profiler:
                if (IsMidTierInitialized())
                {
                    m_middleTierCore.ShutdownPowerProfiler();
                }

                //don't initialize mid tier counters for remote session by default
                if (false == projSettings.isRemoteTarget())
                {
                    InitMiddleTier();
                }

                // Perform actions needed for the project opening
                ProjectOpened();
            }
        }
        break;

        case apEvent::AP_PROFILE_PROCESS_TERMINATED:
        {
            const apProfileProcessTerminatedEvent& profileProcessTerminateEvent = dynamic_cast<const apProfileProcessTerminatedEvent&>(eve);

            if (profileProcessTerminateEvent.profilerName() == PP_STR_dbFileExt)
            {
                SharedProfileManager::instance().stopCurrentRun();
                m_sessionIsOn = false;
            }
        }
        break;

        default:
            break;
    }
}

// ---------------------------------------------------------------------------
afRunModes ppAppController::getCurrentRunModeMask()
{
    afRunModes retVal = 0;
    return retVal;
}

// ---------------------------------------------------------------------------
bool ppAppController::canStopCurrentRun()
{
    return true;
}

// ---------------------------------------------------------------------------
bool ppAppController::stopCurrentRun()
{
    return true;
}

// ---------------------------------------------------------------------------
bool ppAppController::getExceptionEventDetails(const apExceptionEvent& exceptionEve, osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce)
{
    (void)(&exceptionEve); // unused
    (void)(exceptionCallStack); // unused
    (void)(openCLEnglineLoaded); // unused
    (void)(openGLEnglineLoaded); // unused
    (void)(kernelDebuggingEnteredAtLeastOnce); // unused
    return true;
}

/// Handle invalid project settings
void ppAppController::HandleInvalidProjectSettings(bool& isProfileSettingsOK, osProcessId& processId)
{
    // Check if the project is set:
    bool isProjectSet = !afProjectManager::instance().currentProjectSettings().projectName().isEmpty();
    bool isExeExist = !afProjectManager::instance().currentProjectSettings().executablePath().isEmpty();
    ProfileSessionScope scope = SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope;

    // Open the project settings in the following cases:
    QString infoMessage = PM_STR_StartProfilingNoProjectIsLoaded;

    if (isProjectSet && (scope != PM_PROFILE_SCOPE_SYS_WIDE) && (processId == 0))
    {
        SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE;
        isProfileSettingsOK = true;
        processId = PROCESS_ID_UNBOUND;
    }
    // Attach to process:
    else if (isProjectSet && (scope == PM_PROFILE_SCOPE_SINGLE_EXE) && (processId != 0))
    {
        isProfileSettingsOK = true;
    }
    else if (isProjectSet && !isExeExist && (scope == PM_PROFILE_SCOPE_SYS_WIDE))
    {
        isProfileSettingsOK = true;
        processId = PROCESS_ID_UNBOUND;
    }
    else if (!isProjectSet && (processId != 0) && !isExeExist)
    {
        // Attach to process:
        afApplicationCommands::instance()->CreateDefaultProject(PM_STR_PROFILE_MODE);
        isProfileSettingsOK = true;
    }
    else if (!isProjectSet && !isExeExist)
    {
        // Attach to process:
        afApplicationCommands::instance()->CreateDefaultProject(PM_STR_PROFILE_MODE);
        SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE;
        SharedProfileSettingPage::Instance()->RestoreCurrentSettings();

        isProfileSettingsOK = true;
    }
}

// ---------------------------------------------------------------------------
void ppAppController::onProfileStarted(const gtString& profileTypeStr, const spISharedProfilerPlugin* const pCallback, osProcessId processId)
{
    // Do not start a profile if this is not a power profile session:
    if ((profileTypeStr == acQStringToGTString(PM_profileTypePowerProfile)) && (pCallback == static_cast<spISharedProfilerPlugin*>(this)))
    {
        GT_UNREFERENCED_PARAMETER(processId);

        OS_OUTPUT_DEBUG_LOG(PP_STR_logStartProfiling, OS_DEBUG_LOG_INFO);


        InitMiddleTier();

        bool startedOk = IsMidTierInitialized();

        if (startedOk)
        {
            m_sessionIsOn = true;
            startedOk = PrepareStartSession(profileTypeStr);
        }

        if (false == startedOk)
        {
            ShowFailedErrorDialog(true);
            onProfileStopped(this, false);
            SharedProfileManager::instance().onProfileEnded();
            // find the power profiling root item and select it
            afApplicationTreeItemData* pPowerProfilingRootData = ppTreeHandler::instance().GetApplicationTree()->getTreeItemData(ppTreeHandler::instance().GetApplicationTree()->headerItem());

            if (nullptr != pPowerProfilingRootData)
            {
                afApplicationCommands::instance()->applicationTree()->selectItem(pPowerProfilingRootData, false);
            }
        }

    }
}

// ---------------------------------------------------------------------------
void ppAppController::ShowFailedErrorDialog(bool useInitError, PPResult errorToUse)
{
    gtString errorMsg;
    osFilePath systemPath;
    PPResult usedError = (useInitError ? m_midTierLastInitError : errorToUse);

    if (PPR_NOT_SUPPORTED == usedError)
    {
        errorMsg = PP_STR_PowerProfilerNotSupportedErrorMessage;
    }
    else if (PPR_DRIVER_ALREADY_IN_USE == usedError)
    {
        // Check if this is a remote or local session.
        const apProjectSettings& theProjSettings = afProjectManager::instance().currentProjectSettings();
        bool isRemoteSession = theProjSettings.isRemoteTarget();

        if (isRemoteSession)
        {
            errorMsg = PP_STR_DriverAlreadyInUseMessageRemote;
        }
        else
        {
            errorMsg = PP_STR_DriverAlreadyInUseMessageLocal;
        }
    }
    else if (PPR_DRIVER_VERSION_MISMATCH == usedError)
    {
        errorMsg = PP_STR_DriverVersionMismatchMessage;
    }
    else if (PPR_REMOTE_CONNECTION_ERROR == usedError)
    {
        errorMsg = PP_STR_RemoteConnectionErrorMessage;
    }
    else if (PPR_REMOTE_HANDSHAKE_FAILURE == usedError)
    {
        // Try to get the specific handshake error message from the middle tier.
        m_middleTierCore.GetLastErrorMessage(errorMsg);

        if (errorMsg.isEmpty())
        {
            // Use the generic handshake failure message.
            errorMsg << PP_STR_RemoteHandshakeFailureErrorMessage;
        }
    }
    else if (PPR_HYPERVISOR_NOT_SUPPORTED == usedError)
    {
        errorMsg << PP_STRHypervisorNotSupportedErrorMessage;
    }
    else if (PPR_COUNTERS_NOT_ENABLED == usedError)
    {
        errorMsg << PP_STRCountersNotEnabledErrorMessage;
    }
    else if (PPR_WARNING_SMU_DISABLED == usedError)
    {
        errorMsg << PP_STR_SmuDisabledMsg;
    }
    else if (PPR_DB_MIGRATE_FAILURE == usedError)
    {
        errorMsg << PP_STR_dbMigrateFailureMsg;
    }
    else if (PPR_WRONG_PROJECT_SETTINGS == usedError)
    {
        errorMsg << PP_STR_ProjectSettingsPathsInvalid;
    }
    else
    {
        systemPath.setPath(osFilePath::OS_SYSTEM_DIRECTORY);
        gtString driverName = PP_STR_DriverPath;
        gtString fullDriverName = systemPath.asString();
        fullDriverName.append(driverName);
        errorMsg.append(PP_STR_PowerProfilerNotInitializedPrefix);
        errorMsg.appendFormattedString(PP_STR_PowerProfilerNotInitializedErrorMessage, fullDriverName.asCharArray(), (int)usedError);
    }

    acMessageBox::instance().warning(afGlobalVariablesManager::ProductNameA(), acGTStringToQString(errorMsg));
    GT_ASSERT_EX(false, PP_STR_logMiddleTierNotInit);
    OS_OUTPUT_DEBUG_LOG(PP_STR_logMiddleTierNotInit, OS_DEBUG_LOG_INFO);
}

// ---------------------------------------------------------------------------
void ppAppController::InitMiddleTier(const bool isInitForCounterSelection /*= false*/)
{
    PPResult rcMidTier = PPR_NO_ERROR;

    // Check if this is a remote or local session.
    const apProjectSettings& theProjSettings = afProjectManager::instance().currentProjectSettings();
    bool isRemoteSession = theProjSettings.isRemoteTarget();

    PowerProfilerCoreInitDetails coreInitDetails;

    if (isRemoteSession)
    {
        coreInitDetails.m_isRemoteProfiling = true;
        coreInitDetails.m_remoteHostAddr = theProjSettings.remoteTargetName();
        coreInitDetails.m_remotePortNumber = theProjSettings.remoteTargetDaemonConnectionPort();
        m_countersByCategory.clear(); //on remote profile we clear cache anyway

        if (m_middleTierCore.isRemotePP())//If previous session was a remote scenario then we do cleanup to verify nothings remains from it
        {
            m_middleTierCore.StopProfiling();
        }
    }

    // Currently the details container is empty.
    // It will be filled when remote power profiling is integrated into the CodeXL UI.
    rcMidTier = m_middleTierCore.InitPowerProfiler(coreInitDetails);

    if (rcMidTier < PPR_FIRST_ERROR)
    {
        m_isMidTierInitialized = true;

        PPResult rcMidTierInner = UpdateCountersInfoMap();

        gtList<PPDevice*> systemDevices;
        PPResult sysTopologyRes = GetMiddleTierController().GetSystemTopology(systemDevices);

        // Create a map that per category id stores a list with all counters in that category
        if (rcMidTierInner < PPR_FIRST_ERROR)
        {
            GT_IF_WITH_ASSERT(sysTopologyRes < PPR_FIRST_ERROR)
            {
                // First, clear the previously cached data.
                m_countersByCategory.clear();

                // Get the system's root.
                PPDevice* pSystemRoot = systemDevices.front();
                AggregateCountersByCategory(pSystemRoot);
            }
        }
        else
        {
            rcMidTier = rcMidTierInner;
        }
    }
    else if (rcMidTier == PPR_DRIVER_ALREADY_IN_USE)
    {
        // Check if this is a remote or local session.
        if (isRemoteSession)
        {
            // Notify the user and update the GUI accordingly.
            onProfileStopped(this, false);
        }
    }

    // Set the default counters in current settings:
    if (m_currentProjectSettings.m_enabledCounters.empty())
    {
        gtVector<int> defaultCounters;
        GetDefaultCounters(defaultCounters);
        m_currentProjectSettings.m_enabledCounters = defaultCounters;
    }

    SetMiddleTierEnabledCountersList();

    if (isInitForCounterSelection && m_middleTierCore.isRemotePP())//If this init is called only for counter selection from a remote host then we need to do cleanup to avoid interfering with any future profile sessions.
    {
        m_middleTierCore.StopProfiling();
    }

    m_midTierLastInitError = rcMidTier;
}

PPResult ppAppController::UpdateCountersInfoMap()
{
    // Cache the counters information:
    gtMap<int, AMDTPwrCounterDesc*> countersMap;
    PPResult rcMidTier = GetMiddleTierController().GetAllCountersDetails(countersMap);

    // Clear the previously cached data.
    m_countersInfoMap.clear();

    if (rcMidTier < PPR_FIRST_ERROR)
    {
        gtMap<int, AMDTPwrCounterDesc*>::iterator CountersIt = countersMap.begin();

        ppColorsMap& theColorsMap = ppColorsMap::Instance();

        for (; CountersIt != countersMap.end(); CountersIt++)
        {
            const AMDTPwrCounterDesc* pCurrentCounterDesc = (*CountersIt).second;
            int currentIndex = (*CountersIt).first;
            ppControllerCounterData counterData;
            counterData.m_name = pCurrentCounterDesc->m_name;
            counterData.m_description = pCurrentCounterDesc->m_description;
            counterData.m_color = theColorsMap.GetColorForCounterName(pCurrentCounterDesc->m_counterID);
            counterData.m_category = pCurrentCounterDesc->m_category;
            counterData.m_units = pCurrentCounterDesc->m_units;

            m_countersInfoMap[currentIndex] = counterData;
        }
    }

    return rcMidTier;
}

// ---------------------------------------------------------------------------
bool ppAppController::PrepareStartSession(const gtString& profileTypeStr)
{
    // Create a new session:
    bool wasSessionCreated = false;
    bool startedOk = false;
    ppSessionTreeNodeData* pSessionData = CreateSession(profileTypeStr, wasSessionCreated);

    GT_IF_WITH_ASSERT(nullptr != pSessionData)
    {
        // Set the sampling interval after the project settings are loaded:
        m_middleTierCore.SetSamplingIntervalMS(m_currentProjectSettings.m_samplingInterval);

        // Start the session:
        startedOk = StartSession(pSessionData);

        // In visual studio, a file that was not yet created cannot be opened. Therefore we should re-open the file at this stahe
        if (afGlobalVariablesManager::instance().isRunningInsideVisualStudio() && (pSessionData->m_pParentData != nullptr))
        {
            afApplicationCommands::instance()->OpenFileAtLine(pSessionData->m_pParentData->m_filePath, ppSessionController::PP_SESSION_STATE_RUNNING, -1, AF_TREE_ITEM_PP_TIMELINE);
        }

        // If an existing session was used, activate it:
        ActivateExistingSession(pSessionData);
    }

    if (!afGlobalVariablesManager::instance().isRunningInsideVisualStudio())
    {
        static bool sMainWindowInitialized = false;

        if (!sMainWindowInitialized)
        {
            if (afMainAppWindow::instance() != nullptr)
            {
                bool rc = connect(afMainAppWindow::instance(), SIGNAL(SubWindowAboutToClose(const osFilePath&, bool&)), this, SLOT(OnBeforeActiveSubWindowClose(const osFilePath&, bool&)));
                GT_ASSERT(rc);

                sMainWindowInitialized = true;
            }
        }
    }

    return startedOk;
}

// ---------------------------------------------------------------------------
bool ppAppController::StartSession(ppSessionTreeNodeData* pProfileData)
{
    bool exeLaunchedSuccess = false;
    GT_IF_WITH_ASSERT((nullptr != pProfileData) && (pProfileData->m_pParentData != nullptr))
    {
        if (IsMidTierInitialized())
        {
            // Check if this is a remote or local session.
            const apProjectSettings& theProjSettings = afProjectManager::instance().currentProjectSettings();
            bool isRemoteSession = theProjSettings.isRemoteTarget();

            // Check if EXE should be launched:
            bool shouldLaunchEXE = !isRemoteSession && (SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope != PM_PROFILE_SCOPE_SYS_WIDE);
            exeLaunchedSuccess = !shouldLaunchEXE;

            if (shouldLaunchEXE)
            {
                exeLaunchedSuccess = !pProfileData->m_exeFullPath.isEmpty();

                if (exeLaunchedSuccess)
                {
                    osProcessHandle processHandle;
                    osThreadHandle processThreadHandle;
                    osProcessId processId;

                    bool bCreateWindow = true;
                    bool isWindowsStoreApp = !afProjectManager::instance().currentProjectSettings().windowsStoreAppUserModelID().isEmpty();
                    gtString projectCmdlineArguments = afProjectManager::instance().currentProjectSettings().commandLineArguments();

                    gtString exePath = acQStringToGTString(pProfileData->m_exeFullPath);
                    const auto& environmentVariablesList = afProjectManager::instance().currentProjectSettings().environmentVariables();
                    std::vector<osEnvironmentVariable> envVars{ environmentVariablesList.begin(), environmentVariablesList.end() };
                    osEnvVarScope environmentVariablesSetter(envVars);

                    if (isWindowsStoreApp)
                    {
                        // Execute the windows store app:
                        osFilePath filePath;
                        exeLaunchedSuccess = osLaunchSuspendedWindowsStoreApp(exePath, projectCmdlineArguments, processId, processHandle, filePath);
                    }

                    else
                    {
                        // execute the application
                        exeLaunchedSuccess = osLaunchSuspendedProcess(exePath, projectCmdlineArguments, acQStringToGTString(pProfileData->m_workingDirectory),
                                                                      processId, processHandle, processThreadHandle, bCreateWindow, true);
                    }

                    if (exeLaunchedSuccess)
                    {
                        if (isWindowsStoreApp)
                        {
                            osResumeSuspendedWindowsStoreApp(processHandle, true);
                        }
                        else
                        {
                            osResumeSuspendedProcess(processId, processHandle, processThreadHandle, true);
                        }

                        // Create a thread to monitor the application
                        m_pMonitorProcessThread = new SharedProfileProcessMonitor(processId, PP_STR_dbFileExt);
                        m_pMonitorProcessThread->execute();
                    }
                }
            }

            if (exeLaunchedSuccess || !shouldLaunchEXE)
            {
                afApplicationCommands::instance()->OnFileSaveProject();

                // Mark the session data as running:
                pProfileData->m_isSessionRunning = true;

                // Set the current running session data:
                m_pCurrentRunningSessionData = pProfileData;

                m_currentSessionInfo.m_sessionDbFullPath = pProfileData->m_pParentData->m_filePath.asString();

                // Initialize the session info:
                InitSessionInfo(pProfileData);

                // Set the sampling interval with the current settings:
                m_middleTierCore.SetSamplingIntervalMS(m_currentProjectSettings.m_samplingInterval);

                ApplicationLaunchDetails targetAppDetails;

                if (isRemoteSession && m_currentSessionInfo.m_sessionScope == PM_STR_ProfileScopeSystemWideWithFocus)
                {
                    // Target application path.
                    gtString targetAppPath = theProjSettings.executablePath().asString();

                    if (!targetAppPath.isEmpty())
                    {
                        // Target working dir.
                        gtString targetWorkingDir = theProjSettings.workDirectory().asString();

                        // Environment variables.
                        const gtList<osEnvironmentVariable>& envVarList = theProjSettings.environmentVariables();

                        // Create a copy of the environment variables list.
                        gtVector<osEnvironmentVariable> envVars;

                        for (const auto& envVar : envVarList)
                        {
                            envVars.push_back(envVar);
                        }

                        // Prepare the target application details.
                        targetAppDetails = ApplicationLaunchDetails(targetAppPath, targetWorkingDir, theProjSettings.commandLineArguments(), envVars);
                    }
                }

                // Start the profile with the current session information:
                AppLaunchStatus targetAppLaunchStatus = rasUnknown;
                PPResult rc = m_middleTierCore.StartProfiling(m_currentSessionInfo, targetAppDetails,
                                                              targetAppLaunchStatus, ppAppController::ProfileDataHandler, this, ppAppController::ProfileErrorHandler, this);
                exeLaunchedSuccess = (PPR_NO_ERROR == rc);
                QString warningMessage;

                if ((rc == PPR_WARNING_SMU_DISABLED) || (rc == PPR_WARNING_IGPU_DISABLED))
                {
                    warningMessage = (rc == PPR_WARNING_IGPU_DISABLED) ? PP_STR_iGPUDisabledMsg : PP_STR_SmuDisabledMsg;
                }
                else if (rc != PPR_NO_ERROR)
                {
                    m_midTierLastInitError = rc;
                    // Reset the GUI.
                    SharedProfileManager::instance().stopCurrentRun();
                    m_sessionIsOn = false;
                }
                else if (isRemoteSession && targetAppDetails.m_isLaunchRequired && targetAppLaunchStatus != rasOk)
                {
                    if (targetAppLaunchStatus == rasApplicationNotFound)
                    {
                        warningMessage.append(PP_STR_TargetApplicationNotFoundErrorMessage);
                        warningMessage.append(acGTStringToQString(targetAppDetails.m_remoteAppFullPath));
                        warningMessage.append(PP_STR_CouldNotBeLocatedOnRemoteMachineErrorMessage);
                    }
                    else if (targetAppLaunchStatus == rasWorkingDirNotFound)
                    {
                        warningMessage.append(PP_STR_TargetWorkingDirNotFoundErrorMessage);
                        warningMessage.append(acGTStringToQString(targetAppDetails.m_remoteAppFullPath));
                        warningMessage.append(PP_STR_CouldNotBeLocatedOnRemoteMachineErrorMessage);
                    }
                    else
                    {
                        // Generic error message.
                        warningMessage = PP_STR_RemoteFailedToLaunchTargetAppErrorMessage;
                    }
                }

                if (!warningMessage.isEmpty())
                {
                    // Display the message to the user.
                    acMessageBox::instance().warning(afGlobalVariablesManager::ProductNameA(), warningMessage);
                }
            }
        }
    }
    return exeLaunchedSuccess;
}

// ---------------------------------------------------------------------------
ppSessionTreeNodeData* ppAppController::CreateSession(const gtString& profileTypeStr, bool& wasSessionCreated)
{
    ppSessionTreeNodeData* pRetVal = nullptr;

    // Create the session data:
    const apProjectSettings& projectSettings = afProjectManager::instance().currentProjectSettings();

    afIsValidApplicationInfo isValidApplicationInfo;
    bool isAppValid = false;
    bool isWorkingFolderValid = false;
    isValidApplicationInfo.isWInStoreAppRadioButtonChecked = projectSettings.windowsStoreAppUserModelID().isEmpty() == false;
    isValidApplicationInfo.workingFolderPath = projectSettings.workDirectory().asString();
    isValidApplicationInfo.appFilePath = projectSettings.executablePath().asString();
    isValidApplicationInfo.isRemoteSession = projectSettings.isRemoteTarget();

    ProfileSessionScope profileScope = SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope;

    osPortAddress portAddress;
    if (isValidApplicationInfo.isRemoteSession)
    {
      portAddress.setAsRemotePortAddress(projectSettings.remoteTargetName(), projectSettings.remoteTargetDaemonConnectionPort());
      isValidApplicationInfo.portAddress = &portAddress;

    }
    if (profileScope != PM_PROFILE_SCOPE_SYS_WIDE)
    {
        afIsApplicationPathsValid(isValidApplicationInfo, isAppValid, isWorkingFolderValid);
    }


    GT_IF_WITH_ASSERT((profileScope == PM_PROFILE_SCOPE_SYS_WIDE) || (isAppValid && isWorkingFolderValid))
    {
        gtString projectName = projectSettings.projectName();
        gtString projectDir = afProjectManager::instance().currentProjectFilePath().fileDirectoryAsString();
    
        gtString strSessionDisplayName;
        osDirectory sessionOsDir;
        osFilePath projectDirPath(projectDir);
    
        // Only create new session if there is no empty one:
        wasSessionCreated = !ProfileApplicationTreeHandler::instance()->DoesEmptySessionExist();
    
        // get the next session name and dir from the session naming helper (and clean the dir if there is any old cruft)
        ProfileApplicationTreeHandler::instance()->GetNextSessionNameAndDir(projectName, projectDirPath, strSessionDisplayName, sessionOsDir, wasSessionCreated);
    
        // Try to use the empty session item data:
        afApplicationTreeItemData* pEmptySession = ProfileApplicationTreeHandler::instance()->RenameEmptySession(acGTStringToQString(strSessionDisplayName));
    
        if (pEmptySession == nullptr)
        {
            pRetVal = new ppSessionTreeNodeData;
            pRetVal->m_pParentData = new afApplicationTreeItemData;
        }
        else
        {
            pRetVal = qobject_cast<ppSessionTreeNodeData*>(pEmptySession->extendedItemData());
    
            // In case of rename empty data, remove the old session file:
            osFilePath newSessionDBFile;
            newSessionDBFile.setFileDirectory(pRetVal->SessionDir());
            newSessionDBFile.setFileName(PM_STR_NewSessionName);
            newSessionDBFile.setFileExtension(PP_STR_dbFileExt);
    
            if (newSessionDBFile.exists())
            {
                osFile fileToDelete(newSessionDBFile);
                bool rc = fileToDelete.deleteFile();
                GT_ASSERT(rc);
            }
        }
    
        GT_IF_WITH_ASSERT((pRetVal != nullptr) && (pRetVal->m_pParentData != nullptr))
        {
            pRetVal->m_name = acGTStringToQString(strSessionDisplayName);
            pRetVal->m_displayName = pRetVal->m_name;
            pRetVal->m_profileTypeStr = acGTStringToQString(profileTypeStr);
    
            // get the last dir path this is our session name:
            m_executedSessionName = GetProjectNameFromSessionDir(sessionOsDir);
    
            osFilePath outputFilePath(sessionOsDir.directoryPath());
            outputFilePath.setFileName(strSessionDisplayName);
            outputFilePath.setFileExtension(PP_STR_dbFileExt);
            pRetVal->m_pParentData->m_filePath = outputFilePath;
    
            pRetVal->m_isImported = false;
            pRetVal->m_projectName = acGTStringToQString(projectName);
    
            pRetVal->m_commandArguments.clear();
              
            pRetVal->m_workingDirectory = acGTStringToQString(isValidApplicationInfo.workingFolderPath);
            gtString exeName;
            projectSettings.executablePath().getFileNameAndExtension(exeName);
            pRetVal->m_exeName = acGTStringToQString(exeName);
            pRetVal->m_exeFullPath = acGTStringToQString(isValidApplicationInfo.appFilePath);
    
            if (pRetVal->m_exeFullPath.isEmpty())
            {
                pRetVal->m_exeFullPath = acGTStringToQString(projectSettings.windowsStoreAppUserModelID());
            }
    
            pRetVal->m_profileScope = profileScope;
    
            if (pRetVal->m_profileScope == PM_PROFILE_SCOPE_SINGLE_EXE)
            {
                // Single exe scope is not supported for this profile type:
                pRetVal->m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE_FOCUS_ON_EXE;
                SharedProfileSettingPage::Instance()->CurrentSharedProfileSettings().m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE_FOCUS_ON_EXE;
    
            }
    
            pRetVal->m_shouldProfileEntireDuration = true;
    
            if (wasSessionCreated)
            {
                // Add the created session to the tree:
                ProfileApplicationTreeHandler::instance()->AddSession(pRetVal, true);
            }
    
            GT_IF_WITH_ASSERT(pRetVal->m_pParentData != nullptr)
            {
                pRetVal->m_pParentData->m_filePath = outputFilePath;
                pRetVal->m_pParentData->m_filePathLineNumber = ppSessionController::PP_SESSION_STATE_RUNNING;
            }
        }
    } 
    else
    {
        m_midTierLastInitError = PPR_WRONG_PROJECT_SETTINGS;
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
QString ppAppController::GetProjectNameFromSessionDir(osDirectory& sessionDirectory)
{
    QString retStr;

    gtString fullPathStr = sessionDirectory.directoryPath().asString();
    osDirectory upDirectory(sessionDirectory);
    gtString upOneDirPathStr = upDirectory.upOneLevel().directoryPath().asString();

    gtString projectNameAsStr;

    if (0 != upOneDirPathStr.compare(fullPathStr))
    {
        fullPathStr.getSubString(upOneDirPathStr.length() + 1, fullPathStr.length(), projectNameAsStr);

        GT_ASSERT(projectNameAsStr.length() != 0);
        retStr = acGTStringToQString(projectNameAsStr);
    }

    return retStr;

}

// ---------------------------------------------------------------------------
void ppAppController::onProfilePaused(const bool& toggled, const spISharedProfilerPlugin* const pCallback)
{
    GT_UNREFERENCED_PARAMETER(toggled);
    GT_UNREFERENCED_PARAMETER(pCallback);
}

// ---------------------------------------------------------------------------
void ppAppController::onProfileStopped(const spISharedProfilerPlugin* const pCallback, bool stopAndExit)
{
    GT_UNREFERENCED_PARAMETER(stopAndExit);

    if (pCallback == static_cast<spISharedProfilerPlugin*>(this))
    {
        // Mark the session as not running:
        if ((m_pCurrentRunningSessionData != nullptr) && (m_pCurrentRunningSessionData->m_pParentData != nullptr))
        {
            OS_OUTPUT_DEBUG_LOG(PP_STR_logStopProfiling, OS_DEBUG_LOG_INFO);

            QString executedSessionName = m_executedSessionName;
            m_executedSessionName.clear();

            // Get the session info again from DB. This is needed, since the end time is being calculated in GetSessionInfo
            // and there is no other way to get it:
            PowerProfilerBL tempBL;
            bool rc = tempBL.OpenPowerProfilingDatabaseForRead(m_pCurrentRunningSessionData->m_pParentData->m_filePath.asString());
            GT_IF_WITH_ASSERT(rc)
            {
                tempBL.GetSessionInfo(m_currentSessionInfo);

                m_pCurrentRunningSessionData->m_startTime = acGTStringToQString(m_currentSessionInfo.m_sessionStartTime);
                m_pCurrentRunningSessionData->m_endTime = acGTStringToQString(m_currentSessionInfo.m_sessionEndTime);
            }

            // Clear the session info:
            m_currentSessionInfo.Clear();

            if (IsMidTierInitialized())
            {
                // Mark the mid tier as uninitialized.
                m_isMidTierInitialized = false;
                m_middleTierCore.StopProfiling();
                bool isLocalSession = !afProjectManager::instance().currentProjectSettings().isRemoteTarget();

                if (isLocalSession)
                {
                    m_middleTierCore.ShutdownPowerProfiler();
                }
            }

            //terminate process monitor thread
            if (nullptr != m_pMonitorProcessThread)
            {
                if (!m_pMonitorProcessThread->processEnded())
                {
                    m_pMonitorProcessThread->terminate();
                }

                delete m_pMonitorProcessThread;
                m_pMonitorProcessThread = nullptr;
            }

            SharedProfileManager::instance().onProfileEnded();

            m_sessionIsOn = false;

            m_pCurrentRunningSessionData->m_isSessionRunning = false;

            // Select the running session in the tree:
            afApplicationCommands::instance()->applicationTree()->selectItem(m_pCurrentRunningSessionData->m_pParentData, false);
            m_pCurrentRunningSessionData->m_pParentData->m_filePathLineNumber = ppSessionController::PP_SESSION_STATE_COMPLETED;

            // Update the state for the child item data:
            afApplicationTreeItemData* pChildData1 = afApplicationCommands::instance()->applicationTree()->GetChildItemData(m_pCurrentRunningSessionData->m_pParentData->m_pTreeWidgetItem, 0);
            afApplicationTreeItemData* pChildData2 = afApplicationCommands::instance()->applicationTree()->GetChildItemData(m_pCurrentRunningSessionData->m_pParentData->m_pTreeWidgetItem, 1);
            GT_IF_WITH_ASSERT((pChildData2 != nullptr) && (pChildData1 != nullptr))
            {
                pChildData1->m_filePathLineNumber = ppSessionController::PP_SESSION_STATE_COMPLETED;
                pChildData2->m_filePathLineNumber = ppSessionController::PP_SESSION_STATE_COMPLETED;
            }

            // Emit the profile stopped signal:
            emit ProfileStopped(executedSessionName);
        }
    }
}

// ---------------------------------------------------------------------------
bool ppAppController::ActivateItem(QTreeWidgetItem* pItemToActivate)
{
    bool retVal = false;

    if (nullptr == m_pApplicationTree)
    {
        m_pApplicationTree = ppTreeHandler::instance().GetApplicationTree();
    }

    GT_IF_WITH_ASSERT((nullptr != m_pApplicationTree) && (nullptr != pItemToActivate))
    {
        afApplicationTreeItemData* pActivatedItemData = m_pApplicationTree->getTreeItemData(pItemToActivate);
        GT_IF_WITH_ASSERT(nullptr != pActivatedItemData)
        {
            ppSessionTreeNodeData* pExtendedData  = dynamic_cast<ppSessionTreeNodeData*>(pActivatedItemData->extendedItemData());

            if (nullptr != pExtendedData)
            {
                afTreeItemType itemType = pActivatedItemData->m_itemType;

                if (itemType == AF_TREE_ITEM_PROFILE_EMPTY_SESSION && !IsMidTierInitialized())
                {
                    ShowFailedErrorDialog();
                }
                else
                {
                    ppSessionController::SessionState state = ppSessionController::PP_SESSION_STATE_NEW;

                    // If this is the current item in the controller send line 1 as a signal to listen to the events:
                    osDirectory sessionDir = pExtendedData->SessionDir();
                    QString sessionNameInEvent = GetProjectNameFromSessionDir(sessionDir);

                    // Check if this is an empty session:
                    bool isEmptySession = (itemType == AF_TREE_ITEM_PROFILE_EMPTY_SESSION);

                    if ((itemType == AF_TREE_ITEM_PP_TIMELINE) || (itemType == AF_TREE_ITEM_PP_SUMMARY))
                    {
                        // Find the parent session:
                        afApplicationTreeItemData* pParentSession = ProfileApplicationTreeHandler::instance()->FindParentSessionItemData(pActivatedItemData);
                        GT_IF_WITH_ASSERT(pParentSession != nullptr)
                        {
                            isEmptySession = (pParentSession->m_itemType == AF_TREE_ITEM_PROFILE_EMPTY_SESSION);
                        }
                    }

                    if (isEmptySession)
                    {
                        state = ppSessionController::PP_SESSION_STATE_NEW;
                    }
                    else
                    {
                        if (sessionNameInEvent == m_executedSessionName)
                        {
                            state = ppSessionController::PP_SESSION_STATE_RUNNING;
                        }
                        else
                        {
                            state = ppSessionController::PP_SESSION_STATE_COMPLETED;
                        }
                    }

                    switch (itemType)
                    {

                        case AF_TREE_ITEM_PROFILE_EMPTY_SESSION:
                        case AF_TREE_ITEM_PROFILE_SESSION:
                        case AF_TREE_ITEM_PP_TIMELINE:
                        {
                            apMDIViewCreateEvent openFileEvent(AF_STR_PowerProfileViewsCreatorID, pActivatedItemData->m_filePath, L"", AF_TREE_ITEM_PP_TIMELINE, (int)state);
                            apEventsHandler::instance().registerPendingDebugEvent(openFileEvent);
                            retVal = true;
                        }
                        break;

                        case AF_TREE_ITEM_PP_SUMMARY:
                        {
                            apMDIViewCreateEvent openFileEvent(AF_STR_PowerProfileViewsCreatorID, pActivatedItemData->m_filePath, L"", AF_TREE_ITEM_PP_SUMMARY, (int)state);
                            apEventsHandler::instance().registerPendingDebugEvent(openFileEvent);
                            retVal = true;
                        }
                        break;

                        default:
                            GT_ASSERT(false);
                            break;
                    }

                    // Select the item in tree:
                    m_pApplicationTree->selectItem(pActivatedItemData, false);
                }
            }
        }

    }

    return retVal;
}

// ---------------------------------------------------------------------------
void ppAppController::ProfileDataHandler(std::shared_ptr<const gtMap<int, PPSampledValuesBatch>> pSampledDataPerCounter, void* pParams)
{
    ppAppController* pAppController = (ppAppController*)(pParams);

    pAppController->EmitNewPowerProfileData(pSampledDataPerCounter);
}

// ---------------------------------------------------------------------------
void ppAppController::ProfileErrorHandler(PPResult errorCode, void* pParams)
{
    ppAppController* pAppController = static_cast<ppAppController*>(pParams);

    if (pAppController != nullptr)
    {
        // Emit the error signal.
        pAppController->EmitProfileError(errorCode);
    }
}

// ---------------------------------------------------------------------------
void ppAppController::onRemoteRuntimeEvent(ppQtPwrProfErrorCode errorCode)
{
    // Stop the current session, and reset the GUI.
    onProfileStopped(this, false);

    if (errorCode == PPR_COMMUNICATION_FAILURE)
    {
        // Notify the user.
        acMessageBox::instance().critical(afGlobalVariablesManager::ProductNameA(), PP_STR_RemoteFatalCommunicationErrorMessage);
    }
}

// ---------------------------------------------------------------------------
void ppAppController::EmitNewPowerProfileData(ppEventDataSharedPtr pSampledDataPerCounter)
{
    // Copy the data to a local struct and emit that since Qt is having problems emitting shared_ptr of map
    // it also assumes that the data used is previous event was already done:
    ppEventDataPtr pCopyOfData = new ppEventData;

    if (nullptr != pCopyOfData)
    {
        ppEventData::const_iterator it = pSampledDataPerCounter->begin();
        ppEventData::const_iterator itEnd = pSampledDataPerCounter->end();

        for (; it != itEnd; it++)
        {
            (*pCopyOfData)[(*it).first] = (*it).second;
        }

        ppQtEventData dataSharedPtr(pCopyOfData);

        emit NewPowerProfileData(dataSharedPtr);
    }
}

// ---------------------------------------------------------------------------
void ppAppController::ProjectOpened()
{
    gtList<gtString> sessionsPathAsStrList;
    GetPowerProfilingSessionsList(sessionsPathAsStrList);

    // Create a session for each item:
    gtList<gtString>::iterator sessionsIt = sessionsPathAsStrList.begin();

    for (; sessionsIt != sessionsPathAsStrList.end(); sessionsIt++)
    {
        // Get the current session name:
        gtString currentSession = (*sessionsIt);

        // Do not add the empty session to the tree:
        if (currentSession.find(PM_STR_NewSessionName) < 0)
        {
            // Add the session:
            CreateLoadedSession(currentSession);
        }
    }
}

// ---------------------------------------------------------------------------
void ppAppController::GetPowerProfilingSessionsList(gtList<gtString>& sessionsPathAsStrList)
{
    // Get the project profile sessions directory:
    osFilePath projectFilePath;
    afGetUserDataFolderPath(projectFilePath);

    // Add the 'ProjectName_ProfileOutput' to the folder:
    gtString projectProfilesLocation = afProjectManager::instance().currentProjectSettings().projectName();
    projectProfilesLocation += AF_STR_ProfileDirExtension;
    projectFilePath.appendSubDirectory(projectProfilesLocation);

    gtString projectFolderString = projectFilePath.fileDirectoryAsString();
    projectFolderString.append(osFilePath::osPathSeparator);

    // Get all the session folders for this project:
    osDirectory projectDirectory;
    gtList<osFilePath> sessionsDirectoriesList;
    projectDirectory.setDirectoryFullPathFromString(projectFolderString);
    bool isExsistingProjectDirectory = projectDirectory.exists();

    if (isExsistingProjectDirectory && projectDirectory.getSubDirectoriesPaths(osDirectory::SORT_BY_DATE_ASCENDING, sessionsDirectoriesList))
    {
        // check each path if it has a '.cxldb' file that shows it is a power profiling session:
        gtList<osFilePath>::iterator sessionsIt = sessionsDirectoriesList.begin();

        for (; sessionsIt != sessionsDirectoriesList.end(); sessionsIt++)
        {
            gtString currentSessionDir = (*sessionsIt).asString();

            osFilePath currentSessionFilePath;
            currentSessionFilePath.setFileDirectory(currentSessionDir);

            // Get the directory name (should have the same name as the session file):
            QDir dir(acGTStringToQString(currentSessionDir));
            gtString sessionFileName = acQStringToGTString(dir.dirName());
            currentSessionFilePath.setFileName(sessionFileName);
            currentSessionFilePath.setFileExtension(PP_STR_dbFileExt);

            // check if the file exits and if it does add it to the list:
            if (currentSessionFilePath.exists())
            {
                sessionsPathAsStrList.push_back(currentSessionFilePath.asString());
            }
        }
    }
}

// ---------------------------------------------------------------------------
void ppAppController::CreateLoadedSession(gtString& sessionFilePath)
{
    // Create the session data:
    osFilePath sessionPath(sessionFilePath);
    osDirectory sessionDirectory;
    sessionPath.getFileDirectory(sessionDirectory);
    gtString projectName = acQStringToGTString(GetProjectNameFromSessionDir(sessionDirectory));

    // Create the session item data:
    ppSessionTreeNodeData* pRetVal = new ppSessionTreeNodeData();
    pRetVal->m_pParentData = new afApplicationTreeItemData;

    // Get session info from the DB.
    gtString exePathAsStr;
    PowerProfilerBL tmpBl;
    bool isOk = tmpBl.OpenPowerProfilingDatabaseForRead(sessionFilePath);

    if (isOk)
    {
        AMDTProfileSessionInfo sessionInfo;
        isOk = tmpBl.GetSessionInfo(sessionInfo);

        if (isOk)
        {
            gtString exeName;
            osFilePath executablePath(sessionInfo.m_targetAppPath);
            executablePath.getFileNameAndExtension(exeName);
            pRetVal->m_exeName = acGTStringToQString(exeName);
            pRetVal->m_exeFullPath = acGTStringToQString(exePathAsStr);
            pRetVal->m_workingDirectory = acGTStringToQString(sessionInfo.m_targetAppWorkingDir);
            pRetVal->m_startTime = acGTStringToQString(sessionInfo.m_sessionStartTime);
            pRetVal->m_endTime = acGTStringToQString(sessionInfo.m_sessionEndTime);
        }
    }

    gtString projectDir = sessionDirectory.directoryPath().fileDirectoryAsString();

    pRetVal->m_name = acGTStringToQString(projectName);
    pRetVal->m_displayName = pRetVal->m_name;
    pRetVal->m_profileTypeStr = acGTStringToQString(PP_STR_OnlineProfileName);

    pRetVal->m_isImported = false;
    pRetVal->m_projectName = acGTStringToQString(projectName);

    pRetVal->m_commandArguments.clear();

    pRetVal->m_pParentData->m_filePath = sessionFilePath;
    pRetVal->m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE;
    pRetVal->m_shouldProfileEntireDuration = true;

    ExplorerSessionId sessionId = ProfileApplicationTreeHandler::instance()->AddSession(pRetVal, false);

    // Get the item data for the added session:
    afApplicationTreeItemData* pAddedSession = ProfileApplicationTreeHandler::instance()->GetSessionNodeItemData(sessionId);
    GT_IF_WITH_ASSERT(pAddedSession != nullptr)
    {
        pAddedSession->m_filePath = sessionFilePath;
        pAddedSession->m_filePathLineNumber = ppSessionController::PP_SESSION_STATE_COMPLETED;
    }

}

// ---------------------------------------------------------------------------
ppControllerCounterData* ppAppController::GetCounterInformationById(int counterId)
{
    ppControllerCounterData* pRetVal = nullptr;

    if (m_countersInfoMap.count(counterId) != 0)
    {
        pRetVal = &m_countersInfoMap[counterId];
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
void ppAppController::AggregateCountersByCategory(const PPDevice* pDevice)
{
    GT_IF_WITH_ASSERT(nullptr != pDevice)
    {
        for (AMDTPwrCounterDesc* pCounterDesc : pDevice->m_supportedCounters)
        {
            // validating category
            GT_IF_WITH_ASSERT((pCounterDesc->m_category >= 0) && (pCounterDesc->m_category < AMDT_PWR_CATEGORY_CNT))
            {
                // Using QMap prevents duplicate counters
                m_countersByCategory[pCounterDesc->m_category].insert(pCounterDesc->m_counterID);
            }
        }

        for (const PPDevice* pSubDevice : pDevice->m_subDevices)
        {
            AggregateCountersByCategory(pSubDevice);
        }
    }
}

// ---------------------------------------------------------------------------
void ppAppController::GetDefaultCounters(gtVector<int>& defaultCounters)
{
    defaultCounters.empty();

    gtSet<int> powerCounters;
    bool rcPower = GetAllCountersInCategory(AMDT_PWR_CATEGORY_POWER, powerCounters);
    gtSet<int> frequencyCounters;
    bool rcFreq = GetAllCountersInCategory(AMDT_PWR_CATEGORY_FREQUENCY, frequencyCounters);

    if (rcPower)
    {
        for (int counter : powerCounters)
        {
            const ppControllerCounterData* counterData = GetCounterInformationById(counter);

            GT_IF_WITH_ASSERT(nullptr != counterData)
            {
                if ((counterData->m_name == PP_STR_Counter_Power_TotalAPU)  || (counterData->m_name == PP_STR_Counter_Power_Other) ||
                    (counterData->m_name == PP_STR_Counter_Power_IGPU) || (counterData->m_name == PP_STR_Counter_Power_CU1) ||
                    (counterData->m_name == PP_STR_Counter_Power_CU0)  || (counterData->m_name == PP_STR_Counter_Power_GFX) ||
                    counterData->m_name.endsWith(PP_STR_Counter_Power_DGPU))
                {
                    defaultCounters.push_back(counter);
                }
            }
        }
    }

    if (rcFreq)
    {
        for (int counter : frequencyCounters)
        {
            ppControllerCounterData* counterData = GetCounterInformationById(counter);

            GT_IF_WITH_ASSERT(nullptr != counterData)
            {
                if ((counterData->m_name == PP_STR_Counter_Freq_Core3)     || (counterData->m_name == PP_STR_Counter_Freq_Core2) ||
                    (counterData->m_name == PP_STR_Counter_Freq_Core1)     || (counterData->m_name == PP_STR_Counter_Freq_Core0) ||
                    (counterData->m_name == PP_STR_Counter_Freq_IGPU)      ||
                    (counterData->m_name == PP_STR_Counter_AvgFreq_Core3) || (counterData->m_name == PP_STR_Counter_AvgFreq_Core2) ||
                    (counterData->m_name == PP_STR_Counter_AvgFreq_Core1)  || (counterData->m_name == PP_STR_Counter_AvgFreq_Core0) ||
                    (counterData->m_name == PP_STR_Counter_AvgFreq_IGPU) || (counterData->m_name == PP_STR_Counter_AvgFreq_GFX))
                {
                    defaultCounters.push_back(counter);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
bool ppAppController::GetAllCountersInCategory(AMDTPwrCategory searchCategory, gtSet<int>& countersInCategory) const
{
    // std throws exception when using at(x) if x does not exist in the set. need to check before calling
    bool retVal = false;
    countersInCategory.clear();

    if (GetAllCountersByCategory().count(searchCategory) != 0)
    {
        countersInCategory = m_countersByCategory.at(searchCategory);
        retVal = true;
    }

    return retVal;
};

// ---------------------------------------------------------------------------
void ppAppController::SetMiddleTierEnabledCountersList()
{
    if (m_isMidTierInitialized)
    {
        gtVector<int> currentEenabledCounters;
        PPResult getCountersRes = GetMiddleTierController().GetEnabledCounters(currentEenabledCounters);

        GT_IF_WITH_ASSERT(getCountersRes < PPR_FIRST_ERROR)
        {
            // going over all counters we want to enable
            for (int counterToSet : m_currentProjectSettings.m_enabledCounters)
            {
                int curCounterIndex = -1;

                // look for the counter needed to be enable in list of already enabled counters
                for (size_t i = 0; i < currentEenabledCounters.size(); i++)
                {

                    if (counterToSet == currentEenabledCounters[i])
                    {
                        // counter already enabled
                        curCounterIndex = i;
                        break;
                    }
                }

                if (curCounterIndex >= 0)
                {
                    // counter is already enabled, remove if from current list
                    currentEenabledCounters.removeItem(curCounterIndex);
                }
                else
                {
                    // counter was not found on current enabled counters list
                    PPResult enableRes = GetMiddleTierController().EnableCounter(counterToSet);
                    GT_ASSERT(enableRes < PPR_FIRST_ERROR);
                }
            }

            // going over current counters list, if a counter was enabled and also flagged as enabled in the list of future counters it was removed from list
            // since there is nothing to do with it
            // counters that remained on the list should be disabled
            for (int counter : currentEenabledCounters)
            {
                PPResult disnableRes = GetMiddleTierController().DisableCounter(counter);
                GT_ASSERT(disnableRes < PPR_FIRST_ERROR);
            }
        }
    }
}

void ppAppController::OnSessionRename(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath, const osDirectory& oldSessionDir)
{
    // Get the MDI creator:
    ppMDIViewCreator* pViewsCreator = ppAppWrapper::instance().MDIViewCreator();

    if (pViewsCreator != nullptr)
    {
        pViewsCreator->OnSessionRename(pRenamedSessionData, oldSessionFilePath, oldSessionDir);
    }

    GT_IF_WITH_ASSERT(pRenamedSessionData != nullptr)
    {
        afApplicationTreeItemData* pTimelineItemData = ProfileApplicationTreeHandler::instance()->FindSessionChildItemData(pRenamedSessionData->m_pParentData, AF_TREE_ITEM_PP_TIMELINE);
        afApplicationTreeItemData* pSummaryItemData = ProfileApplicationTreeHandler::instance()->FindSessionChildItemData(pRenamedSessionData->m_pParentData, AF_TREE_ITEM_PP_SUMMARY);
        // Sanity check:
        GT_IF_WITH_ASSERT((pTimelineItemData != nullptr) && (pSummaryItemData != nullptr))
        {
            pTimelineItemData->m_filePath = pRenamedSessionData->m_pParentData->m_filePath;
            pTimelineItemData->m_filePathLineNumber = pRenamedSessionData->m_pParentData->m_filePathLineNumber;

            pSummaryItemData->m_filePath = pRenamedSessionData->m_pParentData->m_filePath;
            pSummaryItemData->m_filePathLineNumber = pRenamedSessionData->m_pParentData->m_filePathLineNumber;
        }
    }
}

void ppAppController::OnBeforeSessionRename(SessionTreeNodeData* pAboutToRenameSessionData, bool& isRenameEnabled, QString& renameDisableMessage)
{
    // Get the MDI creator:
    ppMDIViewCreator* pViewsCreator = ppAppWrapper::instance().MDIViewCreator();

    if (pViewsCreator != nullptr)
    {
        pViewsCreator->OnBeforeSessionRename(pAboutToRenameSessionData, isRenameEnabled, renameDisableMessage);
    }
}

void ppAppController::ActivateExistingSession(SessionTreeNodeData* pSessionData)
{
    // Open the session MDI:
    afApplicationTreeItemData* pItemData = pSessionData->m_pParentData;
    GT_IF_WITH_ASSERT((pItemData != nullptr) && (pItemData->m_pTreeWidgetItem != nullptr) && (m_pWindowsManagerHelper != nullptr))
    {
        // Check which item should be activated:
        QTreeWidgetItem* pItemToActivate = pItemData->m_pTreeWidgetItem;

        // Look for the session node:
        if (pItemData->m_itemType == AF_TREE_ITEM_PP_SUMMARY)
        {
            pItemToActivate = pItemData->m_pTreeWidgetItem->parent();
        }

        // Activate the created session item:
        ActivateItem(pItemToActivate);

        // Activate the session window:
        m_pWindowsManagerHelper->ActivateSessionWindow(pSessionData);
    }
}

void ppAppController::GetCurrentProjectEnabledCountersByCategory(AMDTPwrCategory searchCategory, gtVector<int>& counterIdsByCategory)
{
    // Clear the counters:
    counterIdsByCategory.clear();

    // go through the list of counters in project:
    auto iter = m_currentProjectSettings.m_enabledCounters.begin();

    for (; iter != m_currentProjectSettings.m_enabledCounters.end(); iter++)
    {
        // Get the counter data:
        ppControllerCounterData* pCurrentCounterData = GetCounterInformationById(*iter);
        GT_IF_WITH_ASSERT(pCurrentCounterData != nullptr)
        {
            if (pCurrentCounterData->m_category == searchCategory)
            {
                counterIdsByCategory.push_back(*iter);
            }
        }
    }
}

void ppAppController::InitSessionInfo(const ppSessionTreeNodeData* pProfileData)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pProfileData != nullptr) && (pProfileData->m_pParentData != nullptr))
    {
        // Build the session db file path:
        osFilePath dbFilePath = pProfileData->m_pParentData->m_filePath;

        QFile file(acGTStringToQString(dbFilePath.asString()));

        if (file.exists())
        {
            // Remove the file:
            file.remove();
        }

        // Get the local machine name:
        bool rc = osGetLocalMachineName(m_currentSessionInfo.m_targetMachineName);
        GT_ASSERT(rc);

        // Set the executable path:
        m_currentSessionInfo.m_targetAppPath = afProjectManager::instance().currentProjectSettings().executablePath().asString();

        if (m_currentSessionInfo.m_targetAppPath.isEmpty())
        {
            m_currentSessionInfo.m_targetAppPath = afProjectManager::instance().currentProjectSettings().windowsStoreAppUserModelID();
        }

        // Set the working directory:
        m_currentSessionInfo.m_targetAppWorkingDir = afProjectManager::instance().currentProjectSettings().workDirectory().asString();

        // Set the command line arguments:
        m_currentSessionInfo.m_targetAppCmdLineArgs = afProjectManager::instance().currentProjectSettings().commandLineArguments();

        // Set the environment variables:
        afProjectManager::instance().currentProjectSettings().environmentVariablesAsString(m_currentSessionInfo.m_targetAppEnvVars);

        // Set the profile type:
        m_currentSessionInfo.m_sessionType = acQStringToGTString(pProfileData->m_profileTypeStr);

        m_currentSessionInfo.m_sessionScope = PM_STR_ProfileScopeSystemWide;

        if (pProfileData->m_profileScope == PM_PROFILE_SCOPE_SYS_WIDE_FOCUS_ON_EXE)
        {
            m_currentSessionInfo.m_sessionScope = PM_STR_ProfileScopeSystemWideWithFocus;
        }

        // Set the profile start time:
        // set the current time as profile end time
        osTime timing;
        timing.setFromCurrentTime();
        timing.dateAsString(m_currentSessionInfo.m_sessionStartTime, osTime::NAME_SCHEME_FILE, osTime::LOCAL);


    }
}

void ppAppController::OnImportSession(const QString& strSessionFilePath, bool& wasImported)
{
    wasImported = false;

    // Extract the import file extension, and check if this is a profile session:
    osFilePath importedSessionFilePath(acQStringToGTString(strSessionFilePath));
    gtString importedFileExtension;
    importedSessionFilePath.getFileExtension(importedFileExtension);

    if (importedFileExtension.compareNoCase(PP_STR_dbFileExt) == 0)
    {
        osDirectory sessionOldDirectory;
        importedSessionFilePath.getFileDirectory(sessionOldDirectory);

        gtString importProfile;
        importedSessionFilePath.getFileName(importProfile);
        osDirectory projectPath;
        afProjectManager::instance().currentProjectFilePath().getFileDirectory(projectPath);
        gtString projName = afProjectManager::instance().currentProjectSettings().projectName();

        // Allocate a new session data:
        ppSessionTreeNodeData* pImportSessionData = new ppSessionTreeNodeData;
        pImportSessionData->m_pParentData = new afApplicationTreeItemData;

        // Set the imported session data parameters:
        gtString profileFileName;
        importedSessionFilePath.getFileName(profileFileName);
        QString sessionName = acGTStringToQString(profileFileName);

        pImportSessionData->m_displayName = QString("%1 (%2)").arg(sessionName).arg(PM_STR_ImportedSessionPostfix);
        pImportSessionData->m_profileTypeStr = acGTStringToQString(PP_STR_OnlineProfileName);
        pImportSessionData->m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE;


        osDirectory baseDir;
        ProfileApplicationTreeHandler::instance()->GetNextSessionNameAndDir(projName, projectPath, profileFileName, baseDir);

        if (sessionOldDirectory.directoryPath() != baseDir.directoryPath())
        {
            // Update the path for the imported profile file path:
            pImportSessionData->m_pParentData->m_filePath = baseDir.directoryPath();
            pImportSessionData->m_pParentData->m_filePath.setFileExtension(PP_STR_dbFileExt);
            pImportSessionData->m_pParentData->m_filePath.setFileName(profileFileName);
            pImportSessionData->m_pParentData->m_filePathLineNumber = ppSessionController::PP_SESSION_STATE_COMPLETED;

            // Copy the file to the new directory:
            bool rc = osCopyFile(importedSessionFilePath, pImportSessionData->m_pParentData->m_filePath, false);
            GT_ASSERT(rc);

            pImportSessionData->m_isImported = true;

            // Define a profiel BL to get the session information from:
            PowerProfilerBL tempBL;
            rc = tempBL.OpenPowerProfilingDatabaseForRead(acQStringToGTString(strSessionFilePath));

            GT_IF_WITH_ASSERT(rc)
            {
                AMDTProfileSessionInfo info;
                tempBL.GetSessionInfo(info);

                pImportSessionData->m_commandArguments = acGTStringToQString(info.m_targetAppCmdLineArgs);
                pImportSessionData->m_envVariables = info.m_targetAppEnvVars;
                pImportSessionData->m_exeFullPath = acGTStringToQString(info.m_targetAppPath);
                pImportSessionData->m_workingDirectory = acGTStringToQString(info.m_targetAppWorkingDir);
                pImportSessionData->m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE;
                pImportSessionData->m_startTime = acGTStringToQString(info.m_sessionStartTime);
                pImportSessionData->m_endTime = acGTStringToQString(info.m_sessionEndTime);

                if (info.m_sessionScope != PM_STR_ProfileScopeSystemWide)
                {
                    pImportSessionData->m_profileScope = PM_PROFILE_SCOPE_SYS_WIDE_FOCUS_ON_EXE;
                }

                // Add the imported session to the tree:
                ProfileApplicationTreeHandler::instance()->AddSession(pImportSessionData, true);

                //Save the session list to the project
                afApplicationCommands::instance()->OnFileSaveProject();

                wasImported = true;
            }
            else
            {
                ShowFailedErrorDialog(false, PPR_DB_MIGRATE_FAILURE);
            }
        }
    }
}

void ppAppController::OnSessionDelete(ExplorerSessionId deletedSessionId, SessionExplorerDeleteType deleteType, bool& canDelete)
{
    GT_UNREFERENCED_PARAMETER(deleteType);

    // Get the data related to session that is about to be deleted:
    ppSessionTreeNodeData* pSessionData = qobject_cast<ppSessionTreeNodeData*>(ProfileApplicationTreeHandler::instance()->GetSessionTreeNodeData(deletedSessionId));

    if ((pSessionData != nullptr) && (pSessionData->m_pParentData != nullptr))
    {
        canDelete = true;

        // If this is the running session:
        if (pSessionData->m_isSessionRunning)
        {
            // Ask the user if he wants to close the profile session:
            int userAnswer = acMessageBox::instance().question(AF_STR_QuestionA, PP_STR_SessionStopConfirm, QMessageBox::Yes | QMessageBox::No);

            if (userAnswer == QMessageBox::No)
            {
                canDelete = false;
            }
            else
            {
                // Stop session before deleting it
                SharedProfileManager::instance().stopCurrentRun();
            }
        }

        if (canDelete)
        {
            // Get the MDI views creator:
            ppMDIViewCreator* pViewsCreator = ppAppWrapper::instance().MDIViewCreator();

            if (pViewsCreator != nullptr)
            {
                pViewsCreator->OnSessionDelete(pSessionData->m_pParentData->m_filePath.asString());
            }
        }
    }
}


int ppAppController::GetAPUCounterIdFromBackend()
{
    int apuCounterId = -1;

    gtList<PPDevice*> systemDevices;
    PPResult sysTopologyRes = GetMiddleTierController().GetSystemTopology(systemDevices);
    GT_ASSERT(sysTopologyRes < PPR_FIRST_ERROR);

    // Get the counters from backend
    char apuPowerName[] = PP_STR_Counter_Power_TotalAPU;

    // Create a map that per category id stores a list with all counters in that category
    GT_IF_WITH_ASSERT(sysTopologyRes < PPR_FIRST_ERROR)
    {
        // Get the system's root.
        PPDevice* pSystemRoot = systemDevices.front();
        GT_IF_WITH_ASSERT(nullptr != pSystemRoot)
        {
            for (PPDevice* pDevice : pSystemRoot->m_subDevices)
            {
                // Is this the APU?
                if (AMDT_PWR_DEVICE_PACKAGE == pDevice->m_deviceType)
                {
                    // Loop over the counters associated with the APU
                    for (AMDTPwrCounterDesc* pCounter : pDevice->m_supportedCounters)
                    {
                        // If this counter is the 'Total APU Power' counter
                        if (pCounter->m_category == AMDT_PWR_CATEGORY_POWER &&
                            0 == strcmp(pCounter->m_name, apuPowerName))
                        {
                            // Found the required counter
                            apuCounterId = pCounter->m_counterID;
                            break;
                        }
                    }
                }
            }
        }
    }

    return apuCounterId;
}

void ppAppController::OnBeforeActiveSubWindowClose(const osFilePath& sessionFilePath, bool& shouldClose)
{
    // If this is the running session:
    if (sessionFilePath == m_currentSessionInfo.m_sessionDbFullPath)
    {
        // Ask the user if he wants to close the profile session:
        shouldClose = true;
        int userAnswer = acMessageBox::instance().question(AF_STR_QuestionA, PP_STR_SessionStopConfirm, QMessageBox::Yes | QMessageBox::No);

        if (userAnswer == QMessageBox::No)
        {
            shouldClose = false;
        }
    }
}

void ppAppController::SortCountersInCategory(AMDTPwrCategory counterCategory, gtVector<int>& counterIDs)
{
    m_countersSortOrder.SortCountersInCategory(counterCategory, counterIDs);
}

bool ppAppController::IsSpecialExetableCaseSet()
{
    return !afProjectManager::instance().currentProjectSettings().windowsStoreAppUserModelID().isEmpty();
}

void ppAppController::DisplayProcessRunningHTML()
{
    gtString propertiesHTMLMessage;
    afHTMLContent htmlContent(AF_STR_PropertiesProcessRunning);
    htmlContent.addHTMLItem(afHTMLContent::AP_HTML_NO_BG_LINE, PM_STR_PROCESS_IS_RUNNING_MESSAGE);
    htmlContent.toString(propertiesHTMLMessage);
    afPropertiesView* pPropertiesView = afApplicationCommands::instance()->propertiesView();
    GT_IF_WITH_ASSERT(pPropertiesView != nullptr)
    {
        pPropertiesView->setHTMLText(acGTStringToQString(propertiesHTMLMessage), nullptr);
    }
}

bool ppAppController::IsMidTierInitialized() const
{
    return m_isMidTierInitialized;
}

void ppAppController::EmitProfileError(ppQtPwrProfErrorCode errorCode)
{
    emit RemoteRuntimeEvent(errorCode);
}

void ppAppController::SetCurrentProjectEnabledCounters(const gtVector<int>& countersToEnable)
{
    bool areCountersModified = false;

    //check if anything changed to establish the value of areCountersModified:
    if (countersToEnable.size() == m_currentProjectSettings.m_enabledCounters.size())
    {
        for (int counterToSet : countersToEnable)
        {
            bool wasCurrentCounterFound = false;

            for (size_t i = 0; i < m_currentProjectSettings.m_enabledCounters.size(); i++)
            {
                if (counterToSet == m_currentProjectSettings.m_enabledCounters[i])
                {
                    // counter already enabled
                    wasCurrentCounterFound = true;
                    break;
                }
            }

            if (!wasCurrentCounterFound)
            {
                // found at list one item that changed, no need to continue
                areCountersModified = true;
                break;
            }
        }
    }
    else
    {
        areCountersModified = true;
    }


    if (areCountersModified)
    {
        // update enabled counters
        m_currentProjectSettings.m_enabledCounters.clear();
        m_currentProjectSettings.m_enabledCounters.assign(countersToEnable.begin(), countersToEnable.end());

        // signal only if actual changes were made to counters selection
        emit CountersSelectionModified();
    }
}

void ppAppController::GetCurrentProjectEnabledCounters(gtVector<int>& enabledCounters)
{
    // if enabled counters are not initialized
    if (m_currentProjectSettings.m_enabledCounters.size() == 0)
    {
        // Set Project Settings enabled counters to default
        gtVector<int> defaultCounters;
        GetDefaultCounters(defaultCounters);

        // set enabled counters to default values
        SetCurrentProjectEnabledCounters(defaultCounters);
    }

    enabledCounters = m_currentProjectSettings.m_enabledCounters;
}


bool ppAppController::IsChildCounter(const QString counterName) const
{
    bool ret = false;
    ret = ppHierarchyMap::Instance().IsChildCounter(counterName);
    return ret;
}

QString ppAppController::GetCounterParent(const QString counterName) const
{
    QString ret;
    ret = ppHierarchyMap::Instance().GetCounterParent(counterName);
    return ret;
}
