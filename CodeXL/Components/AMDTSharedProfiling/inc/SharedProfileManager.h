//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file SharedProfileManager.h
///
//==================================================================================

#ifndef _SharedProfileManager_H
#define _SharedProfileManager_H

// Ignore warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#ifdef _WIN32
    #pragma warning(push)
    #pragma warning(disable: 4251)
#endif


// Qt:
#include <QtCore>
#include <QtWidgets>

#include <qwidget.h>

// Framework:
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afIExecutionMode.h>
#include <AMDTApplicationFramework/Include/afIRunModeManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>

#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// local
#include "LibExport.h"

// need to undef Bool after all includes so the moc will compile in Linux
#undef Bool

#define PROCESS_ID_UNBOUND ((osProcessId)-1)

class spISharedProfilerPlugin
{
public:
    /// returns if executable is set for special cases:
    virtual bool IsSpecialExetableCaseSet() { return false; }

    /// Return true iff the plug in can start profile with no project loaded:
    virtual bool CanRunWithoutProject() { return false; }

    /// Is attach allowed for this type of profiling
    virtual bool IsAttachEnabled() { return false; }

    /// returns if profile
    virtual bool IsProfileEnabled() { return true; }

    /// update action UI
    virtual void updateUI(afExecutionCommandId commandId, QAction* pAction) { GT_UNREFERENCED_PARAMETER(commandId); GT_UNREFERENCED_PARAMETER(pAction); }

    /// Get layout for the plugin
    virtual afMainAppWindow::LayoutFormats LayoutFormat() = 0;

    /// Get the project setting path
    virtual gtString ProjectSettingsPath() = 0;

    /// Handle invalid project settings
    virtual void HandleInvalidProjectSettings(bool& isProfileSettingsOK, osProcessId& processId) { GT_UNREFERENCED_PARAMETER(isProfileSettingsOK); GT_UNREFERENCED_PARAMETER(processId); };
};

/// Enums covering the initial static menu items
enum pmMenuItemCommands
{
    // Profile menu:
    /// Display the appropriate project settings dialog tab
    ID_PM_PROFILE_SETTINGS_DIALOG = SHARED_PROFILING_FIRST_COMMAND_ID,
    /// Starts a new profile
    ID_PM_START_PROFILE,
    /// Pause or resume the profile
    ID_PM_PAUSE_PROFILE,
    /// Stop the profile
    ID_PM_STOP_PROFILE,
    /// Attach to process and start profile
    ID_PM_ATTACH_PROFILE,
    /// Display the currently selected profile
    ID_PM_SELECTED_PROFILE,

    PM_LAST_COMMAND_ID = ID_PM_SELECTED_PROFILE
};

enum pmNumericConstants
{

    /// Used to keep track of the static menu item count
    COUNT_OF_STATIC_PM_MENUS = PM_LAST_COMMAND_ID - SHARED_PROFILING_FIRST_COMMAND_ID + 1,

    /// The profile can only be started
    SPM_ONLY_START = 0,
    /// The profile can also be paused and resumed
    SPM_ALLOW_PAUSE = 0x1,
    /// The profile can also be prematurely stopped
    SPM_ALLOW_STOP = 0x2,
    /// The profile pause button should be hidden
    SPM_HIDE_PAUSE = 0x4
};

///Expected use case:
/// SharedProfileManager::instance().registerProfile(“CPU: test profile”, this);
/// QObject::connect(&(SharedProfileManager::instance()), SIGNAL(profileStarted(const gtString&, const void* const)),
///                 this, SLOT(onProfileStarted(const gtString&, const void* const)));
/// QObject::connect(this, SIGNAL(profileEnded()), &(SharedProfileManager::instance()),
///                 SLOT(onProfileEnded()));

class AMDTSHAREDPROFILING_API SharedProfileManager : public QObject, public afIExecutionMode,
    public afIRunModeManager, public apIEventsObserver
{
    Q_OBJECT
public:
    /// Get the singleton instance
    static SharedProfileManager& instance();
    /// Destructor
    ~SharedProfileManager();

    //inherited from afIRunModeManager
    virtual afRunModes getCurrentRunModeMask();
    virtual bool canStopCurrentRun();
    virtual bool stopCurrentRun();
    virtual bool getExceptionEventDetails(const apExceptionEvent& exceptionEve,
                                          osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded,
                                          bool& kernelDebuggingEnteredAtLeastOnce);

    //TODO Add icons
    /// Add the profile to the list, emits the profileStarted signal with the callback
    void registerProfileType(const gtString& name, spISharedProfilerPlugin* pCallback, const gtString& projectSettingsTab,
                             int flag = SPM_ONLY_START);

    /// removes the profile to the list
    void unregisterProfileType(const gtString& name, void* pCallback);

    ///Returns the current profile selection
    const gtString currentSelection();

    /// Set the profile selection, ignored if the profile isn't registered:
    /// \return true on success false on failure
    bool SelectProfileType(const gtString& selection);

    // Mode name for identification
    virtual gtString modeName();

    /// Execution status relevance
    /// returns true if relevant
    virtual bool IsExecutionStatusRelevant() { return true; }

    // The name of the action the mode encompasses
    virtual gtString modeActionString();

    // The action verb the mode encompasses
    virtual gtString modeVerbString();

    // Mode description for tooltips
    virtual gtString modeDescription();

    /// Perform a startup action. Return true iff the mode support the requested action:
    virtual bool ExecuteStartupAction(afStartupAction action);

    /// Perform a startup action. Return true iff the mode support the requested action:
    virtual bool IsStartupActionSupported(afStartupAction action);

    /// Return true iff the execution mode supports remote host scenario for the requested session type:
    virtual bool IsRemoteEnabledForSessionType(const gtString& sessionType);

    // Does the current selected profile type support run with no project?
    virtual bool IsStartupActionSupportedWithNoProject(afExecutionCommandId commandId);

    // Get the name of each session type
    virtual gtString selectedSessionTypeName() {return m_selectedProfile;};

    // Execute the command
    virtual void execute(afExecutionCommandId commandId);

    // Handle the UI update
    virtual void updateUI(afExecutionCommandId commandId, QAction* pAction);

    // Execute the session type change command
    virtual void execute(int sessionTypeIndex);

    // Handle the session type UI update
    virtual void updateUI(int sessionTypeIndex, QAction* pAction);

    // Get the number of session type (from the VS hardcoded list)
    virtual int numberSessionTypes();

    // Get the name of each session type
    virtual gtString sessionTypeName(int sessionTypeIndex);

    // Get the icon of each session type
    virtual QPixmap* sessionTypeIcon(int sessionTypeIndex);

    // Return the index for the requested session type:
    virtual int indexForSessionType(const gtString& sessionType);

    // return the layout name used for this mode at specific time:
    virtual afMainAppWindow::LayoutFormats layoutFormat();

    /// return the project settings path used for this mode at specific time:
    virtual gtString ProjectSettingsPath();

    /// get the properties view message to start the execution of the mode:
    virtual gtString HowToStartModeExecutionMessage();

    /// Allow the mode to terminate gracefully at the end of CodeXL. by default nothing needs to be done
    virtual void Terminate();

    /// Show the appropriate project settings tab
    void onInvokeProjectSettings();

    void onInvokeAttachToProcess();

    // Start profiling
    void onStartAction(osProcessId processId = 0);

    void HandleInvalidProjectSettings(bool& isProfileSettingsOK, osProcessId& processId);

    //Select "Profiling mode"
    void onSelectProfileMode(bool updateOnlySessionIndex);
    //Pause or resume the profiling
    void onPauseToggle();
    /// Handle all gui actions needed when the selection changes
    void updateSelected(const gtString& selected);

    // Sets the pause state of the profile
    void setPaused(bool toggled);

    //Whether the profiling is startable
    bool isProfilingOkay(bool checkEXEFile = true);
    //Whether the profiling is occuring
    bool isProfiling();

    /// returns the profile index for a profile string
    bool isStartEnabled(bool& checkable, bool& checked);
    bool isAttachEnabled(bool& checkable, bool& checked);
    bool isProfileModeEnabled(bool& checkable, bool& checked);
    bool isProfileEnabled(int profileIndex, bool& checkable, bool& checked);
    bool isPausedEnabled(bool& checkable, bool& checked, bool& hidden);
    bool isStopEnabled(bool& checkable, bool& checked);
    bool isProjectSettingsEnabled(bool& checkable, bool& checked);


    /// Return the current "Start Profile" action text:
    QString FindStartProfileActionText(bool getTooltip = false);

    const gtVector<gtString>& profiles();

    ///Visual studio direct calls
    //Visual studio command ids
    enum
    {
        SPM_VS_START = 0,
        SPM_VS_ATTACH,
        SPM_VS_PAUSE,
        SPM_VS_STOP,
        SPM_VS_PROFILE_MODE,
        SPM_VS_CPU_ASSESS_PERF,
        SPM_VS_CPU_CLU,
        SPM_VS_CPU_CUSTOM,
        SPM_VS_CPU_IBS,
        SPM_VS_CPU_L2,
        SPM_VS_CPU_BR,
        SPM_VS_CPU_DATA_ACCESS,
        SPM_VS_CPU_INST_ACCESS,
        SPM_VS_CPU_TIMER,
        SPM_VS_THREAD,
        SPM_VS_GPU_PERF_COUNT,
        SPM_VS_GPU_APP_TRACE,
        SPM_VS_PP_ONLINE,
        SPM_VS_DX,
    };

    ///Visual studio direct action
    void vsProfileAction(int vsId);
    ///Visual studio direct ui check
    bool enableVsProfileAction(int vsId, bool& shouldCheck, bool& shouldShow);

    /// apIEventsObserver events callback function
    /// \param eve the event
    /// \param[out] vetoEvent flag indicating whether or not the event should be vetoed
    void onEvent(const apEvent& eve, bool& vetoEvent);

    /// apIEventsObserver Gets the events observer name -- used for logging
    /// \return the events observer name
    const wchar_t* eventObserverName() const;

    /// Set flag to indicate if import is running
    /// \param running if importing is running or not
    void setImportIsRunning(bool running);
    bool isImportIsRunning()const;
    void setExportIsRunning(bool running);
    bool isExportIsRunning()const;

    /// Start / End profile session processing
    /// \param isProcessingComplete is the current session processing is complete?
    void SetProfileSessionProcessingComplete(bool isProcessingComplete);

    /// Return true iff there is no profile session processing:
    /// \return true iif there is no current profile session
    bool IsProfileComplete() const { return m_profileSessionProcessCompleted; }

    /// Updates the current text for the start profile action:
    void UpdateProfileMenuItemText();

signals:
    void profileSelectionChanged(const gtString& name);
    void profileStarted(const gtString& profileTypeStr, const spISharedProfilerPlugin* const pCallback, osProcessId processId);
    void profileBreak(const bool& toggled, const spISharedProfilerPlugin* const pCallback);
    void profileStopped(const spISharedProfilerPlugin* const pCallback, bool stopAndExit);

public slots:
    void onProfileEnded();

protected:
    /// Constructor
    SharedProfileManager();

    ///Update the title bar when necessary
    void updateApplicationTitle();

    /// Handler for when a global variable changes
    /// \param event the global variable changed event
    void GlobalVariableChangedHandler(const afGlobalVariableChangedEvent& event);

    /// The singleton instance
    static SharedProfileManager* m_pMySingleInstance;

    /// The currently selected profile
    gtString m_selectedProfile;

    /// The count of 'static' menu items
    int m_menuCmdCount;

    /// True iff a profile process is currently running:
    bool m_profileIsRunning;

    /// True iff the current profile session processing is completed:
    bool m_profileSessionProcessCompleted;

    /// List of registered profiles
    gtVector<gtString> m_profilesList;

    typedef gtMap<gtString, spISharedProfilerPlugin*> ProfileCallback;
    /// Map the profile string to the callback
    ProfileCallback m_callbackMap;

    //Whether the pause is toggled, based on onBreakAction
    bool m_paused;

    typedef gtMap<gtString, int> FlagMap;
    /// Map the profile string to the toolbar enablement
    FlagMap m_flags;

    typedef gtMap<int, gtString> ProfileMap;
    ///Maps from Visual studio command ids to profile names
    ProfileMap m_profileLookup;

    typedef gtMap<gtString, gtString> ProfileSettingsMap;
    /// Map the profile string to the project settings tab
    ProfileSettingsMap m_projectSettingsMap;

    /// Indicates whether import is running
    bool m_importIsRunning;
    bool m_exportIsRunning;
};

#endif //_SharedProfileManager_H
