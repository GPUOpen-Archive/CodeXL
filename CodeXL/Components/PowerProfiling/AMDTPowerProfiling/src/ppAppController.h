//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppAppController.h
///
//==================================================================================

//------------------------------ ppAppController.h ------------------------------

#ifndef __PPAPPCONTROLLER_H
#define __PPAPPCONTROLLER_H

#include <QtWidgets>

class afApplicationCommands;
class afApplicationTree;
class afApplicationTreeItemData;
class ppProcessMonitor;
class ppWindowsManagerHelper;

#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtSet.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>

#include <AMDTApplicationFramework/Include/afIRunModeManager.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

#include <AMDTSharedProfiling/inc/SharedProfileManager.h>
#include <AMDTSharedProfiling/inc/SessionTreeNodeData.h>

// C++
#include <memory>

// Powerprofiler midtier classes
#include <AMDTPowerProfilingMidTier/include/PowerProfilerCore.h>
#include <AMDTPowerProfilingMidTier/include/PowerProfilerBL.h>

// Local
#include <AMDTPowerProfiling/src/ppCountersSortOrder.h>


typedef std::shared_ptr<const gtMap<int, PPSampledValuesBatch>> ppEventDataSharedPtr;
typedef gtMap<int, PPSampledValuesBatch> ppEventData;
typedef ppEventData* ppEventDataPtr;
typedef QSharedPointer<ppEventData> ppQtEventData;
typedef PPResult ppQtPwrProfErrorCode;

Q_DECLARE_METATYPE(ppQtEventData)
Q_DECLARE_METATYPE(ppQtPwrProfErrorCode)

// This struct is used to store the counter data in a map that index the counter id to the counter data
struct ppControllerCounterData
{
    QString m_name;
    QString m_description;
    QColor m_color;
    AMDTPwrCategory m_category;
    AMDTPwrUnit m_units;
};

class PP_API ppSessionTreeNodeData : public SessionTreeNodeData
{
    Q_OBJECT

};

// Minimum / Maximum sampling interval constants:

#if AMDT_BUILD_ACCESS == AMDT_PUBLIC_ACCESS
    #define PP_MIN_SAMPLING_INTERVAL 100
#elif AMDT_BUILD_ACCESS == AMDT_NDA_ACCESS
    #define PP_MIN_SAMPLING_INTERVAL 1
#elif AMDT_BUILD_ACCESS == AMDT_INTERNAL_ACCESS
    #define PP_MIN_SAMPLING_INTERVAL 1
#else
    #error Unknown build access
#endif

#define PP_MAX_SAMPLING_INTERVAL 2000
#define PP_DEFAULT_SAMPLING_INTERVAL 100


// ----------------------------------------------------------------------------------
// Class Name:          ppProjectSettings
// General Description: Is used to store power profile project settings data
// Author:              Sigal Algranaty
// Creation Date:       2/10/2014
// ----------------------------------------------------------------------------------
class ppProjectSettings
{
public:
    ppProjectSettings();
    ~ppProjectSettings();

    unsigned int m_samplingInterval;
    gtVector<int>  m_enabledCounters;
};

class SharedProfileProcessMonitor;

// ----------------------------------------------------------------------------------
// Class Name:          ppAppController : public apIEventsObserver
// General Description: An (apEvent) event observer, responsible of translating apEvent-s
//                      to events that can be consumed by Visual studio.
// Author:              Gilad Yarnitzky
// Creation Date:       25/8/2014
// ----------------------------------------------------------------------------------
class PP_API ppAppController : public QObject, public apIEventsObserver, public afIRunModeManager, public spISharedProfilerPlugin
{
    Q_OBJECT

public:
    // singleton access:
    static ppAppController& instance();

    virtual ~ppAppController();

    /// init middle tier
    void InitMiddleTier(const bool isInitForCounterSelection = false);

    /// returns if executable is set for special cases:
    virtual bool IsSpecialExetableCaseSet();

    /// Return true iff the plug in can start profile with no project loaded:
    /// For power profile, we always return true, since an exe file is not mandatory for pp session execution:
    virtual bool CanRunWithoutProject() { return true; }

    /// Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"ppEventObserver"; };

    /// overrides spISharedProfilerPlugin
    /// Get layout for the plugin
    virtual afMainAppWindow::LayoutFormats LayoutFormat() { return afMainAppWindow::LayoutProfilePP; }

    /// Get the project setting path
    virtual gtString ProjectSettingsPath() { return AF_STR_LayoutProfilePP; }

    /// Handle invalid project settings
    virtual void HandleInvalidProjectSettings(bool& isProfileSettingsOK, osProcessId& processId);

    /// Data handling callback function for the middle tier controller, need a "C" type function hence static function:
    static void ProfileDataHandler(ppEventDataSharedPtr pSampledDataPerCounter, void* pParams);

    /// Error handling callback function for the middle tier controller:
    static void ProfileErrorHandler(PPResult errorCode, void* pParams);

    /// the function that emits the new data signal:
    void EmitNewPowerProfileData(ppEventDataSharedPtr pSampledDataPerCounter);

    /// a function that emits the error signal:
    void EmitProfileError(ppQtPwrProfErrorCode errorCode);

    /// Get a specific counter cached information:
    ppControllerCounterData* GetCounterInformationById(int counterId);

    /// Handle opening of a project: load old session
    void ProjectOpened();

    /// Project sampling interval accessors:
    void SetCurrentProjectSamplingInterval(unsigned int interval) { m_currentProjectSettings.m_samplingInterval = interval; }
    unsigned int GetCurrentProjectSamplingInterval() { return m_currentProjectSettings.m_samplingInterval; }

    /// Project enabled counters:
    /// Set the current project enabled counters. Notice: When there is no project loaded, the current project contain the default settings
    /// The function emits a signal that the counters has changed
    /// \param counters the list of enabled counters for the current project
    void SetCurrentProjectEnabledCounters(const gtVector<int>& counters);

    /// Get the list of enabled counters for the current project (or list of default counters when there is no project):
    /// \param counters[out] the list of enabled counters for the current project
    void GetCurrentProjectEnabledCounters(gtVector<int>& counters);

    void GetCurrentProjectEnabledCountersByCategory(AMDTPwrCategory searchCategory, gtVector<int>& counterIdsByCategory);

    /// Data after load handling:
    bool IsDataAfterLoad() const { return m_afterLoadData == true; }

    /// Set after load project data flag
    void SetAfterLoadFlag() { m_afterLoadData = true; }

    /// Clear after load project data flag
    void ClearAfterLoadFlag() { m_afterLoadData = false; }

    /// \returns a const reference to a set of all counters by category
    const gtMap < AMDTPwrCategory, gtSet <int> >& GetAllCountersByCategory() const { return m_countersByCategory; };

    /// \param[in] searchCategory the search key - counters category
    /// \returns a const reference to a map of all counters in a category
    bool GetAllCountersInCategory(AMDTPwrCategory searchCategory, gtSet<int>& countersInCategory) const;

    /// \param[in] defaultCounters default values for enabled counters
    void GetDefaultCounters(gtVector<int>& defaultCounters);

    /// gets the APU counter ID from backend
    /// \return APU counter id
    int GetAPUCounterIdFromBackend();

    /// \return true when Power Profiling session running
    bool SessionIsOn() { return m_sessionIsOn; };

    /// \return true if Backend is initialized.
    bool IsMidTierInitialized() const;
    PPResult MidTierInitError() const { return m_midTierLastInitError; }

    /// Show initialization fail error msg dialog
    void ShowFailedErrorDialog(bool useInitError = true, PPResult errorToUse = PPR_NO_ERROR);

    /// check if this counter is a child counter
    /// \param counterName is the counter name
    /// \returns true if the counter has a parent
    bool IsChildCounter(const QString counterName) const;

    /// gets the name of the parent counter
    /// \param counterName is the counter that we looking for it's parent
    /// \returns the name of the parent counter
    QString GetCounterParent(const QString counterName) const;

public slots:
    void onProfileStarted(const gtString& name, const spISharedProfilerPlugin* const pCallback, osProcessId processId);

    void onProfilePaused(const bool& toggled, const spISharedProfilerPlugin* const pCallback);
    void onProfileStopped(const spISharedProfilerPlugin* const pCallback, bool stopAndExit);

    void onRemoteRuntimeEvent(ppQtPwrProfErrorCode errorCode);

    /// Get project name from path is the session data
    QString GetProjectNameFromSessionDir(osDirectory& sessionDirectory);

    /// get the core interface
    PowerProfilerCore& GetMiddleTierController() { return m_middleTierCore; }

    /// Get the currently executed online session name:
    QString GetExecutedSessionName() const { return m_executedSessionName; }

    /// Is handling the session rename slot:
    void OnSessionRename(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath, const osDirectory& oldSessionDir);

    /// Is called before a session is renamed:
    void OnBeforeSessionRename(SessionTreeNodeData* pAboutToRenameSessionData, bool& isRenameEnabled, QString& renameDisableMessage);

    /// Get the current session info:
    const AMDTProfileSessionInfo& GetSessionInfo() const {return m_currentSessionInfo;};

    /// Is handling the import of a session file:
    /// \param strSessionFilePath the imported file path
    /// \param wasImported true if the imported session belong to power profiler and the import process succeeded
    void OnImportSession(const QString& strSessionFilePath, bool& wasImported);

    /// Is handling the session delete signal. The signal is emitted from profile tree:
    /// \param deletedSessionId the session id for the session that is about to be deleted
    /// \param deleteType remove from tree / remove from file system
    /// \param canDelete[out] can be file be deleted
    void OnSessionDelete(ExplorerSessionId deletedSessionId, SessionExplorerDeleteType deleteType, bool& canDelete);

    /// Is called when the current running session window is about to be closed:
    /// \param subwindowFilePath the file path of the sub window which is about to be closed
    /// \param [out] shouldClose true iff the window can be closed
    void OnBeforeActiveSubWindowClose(const osFilePath& subwindowFilePath, bool& shouldClose);

    /// Display a "Process Running" message in the properties view:
    void DisplayProcessRunningHTML();

    /// SortCountersInCategory determines the sort order for display per counters in category
    /// \param[in] categoryID  the category counterIDs belong to
    /// \param     counterIDs  vector of counters IDs to sort
    void SortCountersInCategory(AMDTPwrCategory counterCategory, gtVector<int>& counterIDs);

    /// Sets a pointer to the windows helper member:
    void SetWindowsHelper(ppWindowsManagerHelper* pWindowsManagerHelper) { m_pWindowsManagerHelper = pWindowsManagerHelper; };

    PPResult UpdateCountersInfoMap();

signals:
    /// Signal to distribute power profile new data:
    void NewPowerProfileData(ppQtEventData pSampledDataPerCounter);

    /// Signal to power profile stopped:
    void ProfileStopped(const QString& sessionName);

    void CountersSelectionModified();

    /// Signal to indicate a fatal communication failure:
    void RemoteRuntimeEvent(ppQtPwrProfErrorCode errorCode);

protected:
    // Do not allow the use of my default constructor:
    ppAppController();

    friend class ppSingletonsDelete;

    /// singleton instance
    static ppAppController* m_spMySingleInstance;

    /// afIRunModeManager derived functions:
    virtual afRunModes getCurrentRunModeMask();

    virtual bool canStopCurrentRun();
    virtual bool stopCurrentRun();

    virtual bool getExceptionEventDetails(const apExceptionEvent& exceptionEve, osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce);

    /// Prepare the data for start session and open the session if needed
    /// \param session profile type
    bool PrepareStartSession(const gtString& profileTypeStr);

    /// Start middle tier session profiling. (Work on a local file path and not pass ref)
    /// \param start a session the specified session data
    bool StartSession(ppSessionTreeNodeData* pProfileData);

    /// Initializes the current session information:
    /// \param pProfileData the item data representing the session information
    void InitSessionInfo(const ppSessionTreeNodeData* pProfileData);

    /// Create the session for the current profile:
    /// \param profileTypeStr the profile type
    /// \param wasSessionCreated true if a new session was created, false is an empty existing session was used
    ppSessionTreeNodeData* CreateSession(const gtString& profileTypeStr, bool& wasSessionCreated);

    /// Activate the tree item that was clicked:
    /// \param tree item to activate
    bool ActivateItem(QTreeWidgetItem* pItemToActivate);

    /// Activate the existing session represented in pSessionData
    /// \param pSessionData the session item data
    void ActivateExistingSession(SessionTreeNodeData* pSessionData);

    /// Get a list of sessions for the current opened project
    /// \param vector to hold the results
    void GetPowerProfilingSessionsList(gtList<gtString>& sessionsPathAsStrList);

    /// Create a session for a loaded session:
    /// \param session file path to load from
    void CreateLoadedSession(gtString& sessionFilePath);

    /// Helper function, goes over the topology tree and collect counters info
    /// \param[in]   pDevice: the device root item to start from
    /// \param[out]   countersByCategory: per category, a list of counters data collected
    void AggregateCountersByCategory(const PPDevice* pDevice);

    /// Modify middle tier enabled counters to match m_EnabledCounters
    void SetMiddleTierEnabledCountersList();

    /// Get the application commands instance:
    afApplicationCommands* m_pApplicationCommands;
    afApplicationTree* m_pApplicationTree;

    /// executed session name:
    QString m_executedSessionName;

    /// Power profiler middle tier controller:
    PowerProfilerCore m_middleTierCore;
    PPResult m_midTierLastInitError;
    bool m_isMidTierInitialized;

    /// cache of counters information
    gtMap<int, ppControllerCounterData> m_countersInfoMap;

    /// Thread to monitor the executed application
    SharedProfileProcessMonitor* m_pMonitorProcessThread;

    /// Store the currently loaded project settings:
    ppProjectSettings m_currentProjectSettings;

    /// Counters Id's saved by category
    gtMap < AMDTPwrCategory, gtSet <int> > m_countersByCategory;

    /// Flag to indicate data is after load:
    bool m_afterLoadData;

    /// True when power profiling session is running
    bool m_sessionIsOn;

    /// Contain the current session information:
    AMDTProfileSessionInfo m_currentSessionInfo;

    /// Currently running session tree node data:
    ppSessionTreeNodeData* m_pCurrentRunningSessionData;

    /// Object for sorting counters
    ppCountersSortOrder m_countersSortOrder;

    /// Contain a pointer to windows helper object. Will be used to implement different behavior in VS:
    ppWindowsManagerHelper* m_pWindowsManagerHelper;

};

#endif //__PPAPPCONTROLLER_H
