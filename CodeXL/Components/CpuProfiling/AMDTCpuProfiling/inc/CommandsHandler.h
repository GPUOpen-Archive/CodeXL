//==================================================================================
// Copyright (c) 2012-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file CommandsHandler.h
/// \brief  The CodeAnalyst component profiling engine logic
///
//==================================================================================
// $Id: //devtools/main/CodeXL/Components/CpuProfiling/AMDTCpuProfiling/inc/CommandsHandler.h#37 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author:  AMD Developer Tools Team
// Change list:    $Change: 569613 $
//=============================================================
#ifndef _CommandsHandler_H
#define _CommandsHandler_H

// Qt:
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTOSWrappers/Include/osEnvironmentVariable.h>
#include <AMDTSharedProfiling/inc/SharedProfileManager.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Backend:
#include <AMDTCpuProfilingControl/inc/CpuProfileControl.h>
#include <AMDTCpuProfilingTranslation/inc/CpuProfileDataTranslation.h>

//local
#include <inc/CpuProjectHandler.h>

// need to undef Bool after all includes so the moc will compile in Linux
#undef Bool

//pre-definees;
class ProfileProcessMonitor;

enum CpuProfEngineStates
{
    CPUPROF_STATE_INVALID = 0,
    CPUPROF_STATE_READY,
    CPUPROF_STATE_PROFILING,
    CPUPROF_STATE_PAUSED
};

/// Singleton class that will handle the logic of the profiling commands
class  CommandsHandler : public QObject, public apIEventsObserver, public spISharedProfilerPlugin
{
    Q_OBJECT;
    friend class AmdtCpuProfiling;
public:
    /// Return my singleton instance:
    static CommandsHandler* instance();

    /// Destructor
    ~CommandsHandler();

    /// Initialize the singleton
    bool initialize();

    //Profiling logic
    /// Called to start the profiling
    HRESULT onStartProfiling(const gtString& profileType, osProcessId processId = 0);

    /// Setup the profile session:
    /// \param profileType the profile type
    /// \param processId the profile process ID
    /// \param scope the profile scope
    /// \param isLaunchingJava[out] is this a java profile session?
    /// \param projectCmdlineArguments[out] the command line arguments after the profile session setup
    /// \param errorMessage[out] an error message for the log, describing the error
    HRESULT SetupProfileSession(const gtString& profileType, osProcessId processId, ProfileSessionScope scope, bool& isLaunchingJava, gtString& projectCmdlineArguments, gtString& errorMessage);

    /// Launch the profile session:
    /// \param processId the profile process ID
    /// \param projectCmdlineArguments the command line arguments as string for the input of the launched process
    /// \param errorMessage[out] an error message for the log, describing the error
    HRESULT LaunchProfileSession(osProcessId processId, bool isLaunchingJava, const gtString& projectCmdlineArguments, gtString& errorMessage);

    /// Called to toggle whether the profiling is paused
    HRESULT onTogglePauseProfiling(bool pause);

    /// Called to stop profiling
    HRESULT onStopProfiling(bool stopAndExit = false);

    /// Called to start translating the raw data files
    void startTranslating(ReaderHandle* pHandle, const gtString& translatedPath, bool importing);

    //State Reporting
    /// Returns whether the profile is currently profiling
    bool isProfiling() const;
    /// Returns whether the profile is currently paused
    bool isPaused() const;
    /// Returns whether profiling is supported
    bool isProfilingOkay() const;

    /// View menu commands:
    virtual void onViewResetGUILayout();

    // Overrides apIEventsObserver
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"CpuProfiling"; };

    /// spISharedProfilerPlugin interface implemented:
    /// in CPU Allow metro app for cpu profile
    virtual bool IsSpecialExetableCaseSet();

    /// Is attach allowed for this type of profiling
    virtual bool IsAttachEnabled() { return true; };

    // returns if profile
    virtual bool IsProfileEnabled();

    /// update action UI
    virtual void updateUI(afExecutionCommandId commandId, QAction* pAction);

    /// Get layout for the plugin
    virtual afMainAppWindow::LayoutFormats LayoutFormat() { return afMainAppWindow::LayoutProfileCPU; }

    /// Get the project setting path
    virtual gtString ProjectSettingsPath() { return AF_STR_LayoutProfileCPU; }

    /// Handle invalid project settings
    virtual void HandleInvalidProjectSettings(bool& isProfileSettingsOK, osProcessId& processId);

public slots:
    void onProfileStarted(const gtString& profileTypeStr, const spISharedProfilerPlugin* const pCallback, osProcessId processId);
    void onProfilePaused(const bool& toggled, const spISharedProfilerPlugin* const pCallback);
    void onProfileStopped(const spISharedProfilerPlugin* const pCallback, bool stopAndExit);

signals:
    void profileEnded();

protected:
    /// Do not allow the use of my constructor
    CommandsHandler();

    /// Register my instance
    static bool registerInstance(CommandsHandler* pCommandsHandlerInstance);

    /// My single instance:
    static CommandsHandler* m_pMySingleInstance;

    ///Helper function to check if the event file is available or not
    bool isEventFileAvailable();

    ///Helper function to verify pmc events and set the configuration for profiling
    HRESULT verifyAndSetEvents(EventConfiguration** ppDriverEvents);

private:
    HRESULT enableProfiling();
    HRESULT initializeSessionInfo();
    HRESULT setupTimerConfiguration(gtString& errorMessage);
    HRESULT setupEventConfiguration(gtString& errorMessage);
    HRESULT setupIbsConfiguration(gtString& errorMessage);

    bool saveEnvironmentVariables(gtList<osEnvironmentVariable>& envVars);
    void loadEnvironmentVariables(const gtList<osEnvironmentVariable>& envVars);

    unsigned int* constructAttachPidArray(unsigned int& launchedPid, unsigned int& count) const;

    bool trySetupClrProfilingEnvironment(gtList<osEnvironmentVariable>& envVars) const;
    bool trySetupJavaProfilingEnvironment(gtString& javaAgentArg, bool is64BitApp) const;
    HRESULT tryStartProfiling(int retries) const;

    /// Report an error:
    /// \param appendDriverError append the driver error to the string
    /// \param userMessage the user message to output
    /// \param pFormatString the string for the log file
    void ReportErrorMessage(bool appendDriverError, const gtString& userMessage, const wchar_t* pFormatString, ...) const;

    /// Notify help message to user:
    /// \param userMessage the user message to output
    void ReportHelpMessage(const gtString& userMessage) const;

#ifdef _WIN32
    /// if current platform(Processor, OS etc. combination) supports CPU Profiling
    /// \return true if current platform(Processor, OS etc. combination) supports CPU Profiling
    bool IsPlatformSupportsCPUProfiling();
#endif

    /// Run-time flags
    CpuProfEngineStates m_state;

    ///Whether profiling is enabled
    bool m_profilingEnabled;

    ///The profile session path
    gtString m_sessionPath;

    //The current profile session
    CPUSessionTreeItemData m_profileSession;

    /// The profile process monitoring thread
    ProfileProcessMonitor* m_processMonitor;

    //The profile translation thread return value
    HRESULT m_translatedRet;

};
#endif //_CommandsHandler_H
