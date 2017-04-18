//=====================================================================
// Copyright (c) 2012 Advanced Micro Devices, Inc. All rights reserved.
//
/// \author GPU Developer Tools
/// \file $File: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProfileManager.h $
/// \version $Revision: #52 $
/// \brief :  This file contains ProfileManager class
//
//=====================================================================
// $Id: //devtools/main/CodeXL/Components/GpuProfiling/AMDTGpuProfiling/ProfileManager.h#52 $
// Last checkin:   $DateTime: 2016/04/18 06:02:03 $
// Last edited by: $Author: salgrana $
// Change list:    $Change: 569613 $
//=====================================================================

#ifndef _PROFILE_MANAGER_H_
#define _PROFILE_MANAGER_H_

#include <qtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

#include <QtCore>
#include <QtWidgets>

#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>

#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTAPIClasses/Include/Events/apIEventsObserver.h>
#include <AMDTSharedProfiling/inc/SharedProfileManager.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// For Remote Profiling.
#include <AMDTRemoteClient/Include/CXLDaemonClient.h>

#include <TSingleton.h>

#include "SessionExplorerDefs.h"
#include "ProfileParam.h"
#include "Session.h"
#include "SessionManager.h"
#include "CounterManager.h"
#include "ProfileSettingData.h"
#include <AMDTGpuProfiling/Util.h>
#include "ProfileApplicationTreeHandler.h"
#include "ProjectSettings.h"
#include "ProfileProcessMonitor.h"
#include "AsyncRemoteGpuProfilingTask.h"

// need to undef Bool after all includes so the moc will compile in Linux
#undef Bool

class apMonitoredObjectsTreeActivatedEvent;
class gpExecutionMode;

/// class that manages profiling for the GPU Profiler
class AMDT_GPU_PROF_API ProfileManager : public QObject, public apIEventsObserver, public TSingleton<ProfileManager>,
    public IAsyncErrorMessageConsumer, public spISharedProfilerPlugin
{
    Q_OBJECT

    /// provide access to our private constructor
    friend class TSingleton < ProfileManager >;

public:
    /// events callback function
    /// \param eve the event
    /// \param[out] vetoEvent flag indicating whether or not the event should be vetoed
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);

    /// Gets the events observer name -- used for logging
    /// \return the events observer name
    virtual const wchar_t* eventObserverName() const;

    /// Adds the profile types to the Profile Menu
    void SetupGPUProfiling();

    /// Call CodeXLGpuProfiler executable to generate summary pages from atp file. Listen for the SummaryPagesGenerationFinished signal to know when async generation is complete
    /// \param strInputAtpFile input atp file
    /// \param strSessionName the name of the session that summary pages are being generated for
    /// \param[out] strErrorMessageOut error message out
    /// \return True if pages are generated successfully
    bool GenerateSummaryPages(const QString& strInputAtpFile, const QString& strSessionName, QString& strErrorMessageOut);

    /// Call CodeXLGpuProfiler executable to generates Occupancy pages. Listen for the OccupancyFileGenerationFinished signal to know when async generation is complete
    /// \param pSession session of which occupancy is displayed
    /// \param pOccInfo occupancy information
    /// \param callIndex call index
    /// \param[out] strErrorMessageOut error message if any
    /// \return True if success in generating
    bool GenerateOccupancyPage(GPUSessionTreeItemData* pSessionData, const IOccupancyInfoDataHandler* pOccInfo, int callIndex, QString& strErrorMessageOut);

    /// Loads the current project settings
    void LoadCurrentProjectSettings();

    /// Gets the project settings for the current project
    /// \return the project settings for the current project
    ProjectSettings* GetCurrentProjectSettings() { return m_pCurrentProjectSettings; }

    /// Sets the error message for future use.
    /// \param m_strRemoteProfilingError an error message for an error which occured during remote profiling
    virtual void ConsumeErrorMessage(const QString& errorMsg);

    /// implementing the needed functions from spISharedProfilerPlugin
    virtual afMainAppWindow::LayoutFormats LayoutFormat() { return afMainAppWindow::LayoutProfileGPU; }

    /// Get the project setting path
    virtual gtString ProjectSettingsPath() { return AF_STR_LayoutProfileGPU; }

    /// Handle invalid project settings
    virtual void HandleInvalidProjectSettings(bool& isProfileSettingsOK, osProcessId& processId);

    /// Return the frame analysis mode manager
    gpExecutionMode* GetFrameAnalysisModeManager() const { return m_pFrameAnalysisMode; };

    /// Add the specified session to the explorer (or to the deferred loading list for when the explorer is shown in the future) and optionally shows the session
    /// \param pSession the session to add and show
    /// \param doShow flag indicating if the session should be shown or not
    /// \param isNewSession flag to indicate the session is already created or a new session
    void AddSessionToExplorer(GPUSessionTreeItemData* pSession, bool doShow, bool isNewSession);

    /// returns if profile
    virtual bool IsProfileEnabled();

protected:
    /// Profile parameters
    ProfileParam m_profileParameters;

    /// Rename all the specific files for GPU sessions
    /// \param pRenamedSessionData the item data for the session been renamed
    /// \param oldSessionFilePath the old session file path
    /// \param oldSessionDirectory the old session directory
    void HandleGPUSessionRename(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath);

    /// Rename all the specific files for frame analysis sessions
    /// \param pRenamedSessionData the item data for the session been renamed
    /// \param oldSessionFilePath the old session file path
    /// \param oldSessionDirectory the old session directory
    void HandleFASessionRename(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath);

protected slots:
    /// Handler for when Profiling starts
    /// \param profileTypeStr profile type string
    /// \param pCallback Unused callback parameter
    void ProfileStartedHandler(const gtString& profileTypeStr, const spISharedProfilerPlugin* const pCallback, osProcessId processId);

    /// \param pCallback used to check if the call is for us
    /// \param stopAndexit
    void onProfileStopped(const spISharedProfilerPlugin* const pCallback, bool stopAndExit);

    /// Handler for when a profiling session ends
    /// \param exitCode exit code, unused parameter
    /// \param the type of the profile action finished
    void ProfilingFinishedHandler(int exitCode, ProfileProcessMonitor::ProfileServerRunType runType);

    /// handler for when generation of the Summary Pages is complete
    /// \param success indicates whether or not CodeXLGpuProfiler completed successfully
    /// \param strError error message on failure
    void SummaryPagesGenerationFinishedHandler(bool success, const QString& strError);

    /// Handler for when a session is being deleted
    /// \param sessionId the id of the session
    /// \param deleteType flag indicating whether the session is merely being removed from the tree view, or if the user has requested that the session be removed from disk as well
    /// \param[out] canDelete flag indicating if the session was successfully deleted
    void SessionDeletedHandler(ExplorerSessionId sessionId, SessionExplorerDeleteType deleteType, bool& canDelete);

    /// Handler for when a session is being renamed
    /// \param pRenamedSessionData the item data for the session been renamed
    /// \param oldSessionFilePath the old session file path
    /// \param oldSessionDirectory the old session directory
    void SessionRenamedHandler(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath, const osDirectory& oldSessionDirectory);

    /// Is called before a session is renamed:
    /// \param pAboutToRenameSessionData the item data for the session which is about to be renamed
    /// \param isRenameEnabled is the rename enabled?
    /// \param renameDisableMessage message for the user is the rename is disabled
    void OnBeforeSessionRename(SessionTreeNodeData* pAboutToRenameSessionData, bool& isRenameEnabled, QString& renameDisableMessage);

    /// handler for when a file is imported
    /// \param strFileName the name of the file being imported
    /// \param[out] imported flag indicating if this file was successfully imported.  Should ONLY be set to true if the file was imported.
    void OnImportSession(const QString& strFileName, bool& imported);

signals:
    /// Profiling is finished
    /// \param profileSuccess indicates whether or not CodeXLGpuProfiler completed successfully
    /// \param strError error if any
    /// \param pSession newly generated session
    void ProfilingFinished(bool profileSuccess, QString strError, GPUSessionTreeItemData* pSession);

    /// Generation of the Summary Pages is complete
    /// \param success indicates whether or not CodeXLGpuProfiler completed successfully
    /// \param strError error message on failure
    void SummaryPagesGenerationFinished(bool success, const QString& strError);

    /// Generation of the Occupancy HTML file is complete
    /// \param success indicates whether or not CodeXLGpuProfiler completed successfully
    /// \param strError error message on failure
    /// \param strOccupancyHTMLFileName the name of the Occupancy HTML file that was generated
    void OccupancyFileGenerationFinished(bool success, const QString& strError, const QString& strOccupancyHTMLFileName);

private:

    /// Initializes the static instance of ProfileManager
    ProfileManager();

    /// Destroys the static instance of ProfileManager
    virtual ~ProfileManager();

    /// Makes the signals/slots connection for the SesionExplorer and adds the file filter types
    void HookupSessionExplorer();

    /// Get the appropriate CodeXLGpuProfiler executable
    /// \param[out] strServer the name of the CodeXLGpuProfiler executable
    /// \param[out] strErrorMessageOut an error message if the function fails
    /// \return true if the CodeXLGpuProfiler exe if returned, false otherwise
    bool GetProfilerServer(osFilePath& strServer, QString& strErrorMessageOut);

    /// Launch CodeXLGpuProfiler executable with the specified options for the specified reason
    /// \param strServer full path to the CodeXLGpuProfiler executable
    /// \param strOptions the set of options to pass to CodeXLGpuProfiler
    /// \param runType the reason CodeXLGpuProfiler is being executed
    /// \param[out] strErrorMessageOut error message, if any
    /// \return true if the server is successfully launched, false otherwise
    bool LaunchProfilerServer(const osFilePath& strServer, const gtString& strOptions, ProfileProcessMonitor::ProfileServerRunType runType, QString& strErrorMessageOut);

    /// Do profiling after setting up actual environment
    /// \param profileType type of profiling
    /// \param[out] strErrorMessageOut error if any
    /// \return True if success in profiling
    bool ProfileProject(GPUProfileType profileType, QString& strErrorMessageOut);

    /// Check the profile setting data for errors
    /// \param profileType type of profiling
    /// \param projectSettings the project setting data
    /// \param[out] strErrorMessageOut error if any
    /// \return true if there is valid data, false otherwise
    bool HasValidProfileSettingData(GPUProfileType profileType, const apProjectSettings& projectSettings, QString& strErrorMessageOut);

    /// shows the profile setting dialog and collect data
    /// \param profData to be used for profile setting dialog
    /// \param errInSetting if there is error is setting
    /// \param strErrorMessage output message of that error
    /// \return to continue or not
    bool ShowProfileSettingDialog(ProfileSettingData& profData, bool& errInSetting, const QString& strErrorMessage);

    /// Get the last saved setting of the current
    /// project in the current VS session.
    /// \param strProjFullName Project name(part of key)
    /// \param strPlatformConfig Profile and config(part of key)
    /// \param strProjectCommand Executable name with full  path
    /// \param lastUsedWasOriginal Whether last used setting was original
    /// \return True if found
    bool GetSettingOfProject(const QString& strProjFullName, const QString& strPlatformConfig, QString& strProjectCommand, bool& lastUsedWasOriginal);


    /// Get the last saved setting of the current
    /// project in the current VS session.
    /// \param strProjFullName Project name(part of key)
    /// \param strPlatformConfig Profile and config(part of key)
    /// \param strProjectCommand Executable name with full  path
    /// \param lastUsedWasOriginal Whether last used setting was original
    void SaveSettingOfProject(const QString& strProjFullName, const QString& strPlatformConfig,
                              const QString& strProjectCommand, bool lastUsedWasOriginal);

    /// Gets the session from a session Id
    /// \param sessionId the id of the session
    /// \return the session, or NULL if the specified session does not exist
    GPUSessionTreeItemData* GetSessionFromSessionId(ExplorerSessionId sessionId);

    /// handler for when a project is opened
    void ProjectOpened();

    /// handler for when a different project is closed
    void ProjectClosed();

    /// Handler for when a tree item is activated
    /// \param activationEvent - the event sent for the activation
    void OnTreeItemActivatedEvent(const apMonitoredObjectsTreeActivatedEvent& activationEvent);

    /// helper to add an imported session to the session explorer
    /// \param strSessionName the name of the session
    /// \param profileType the type of the session begin added
    /// \param strSessionFile the full path of the session file
    void AddImportedSession(const QString& strSessionName, GPUProfileType profileType, const QString& strSessionFile);

    /// helper to deal with a profiling CodeXLGpuProfiler session completing
    /// \param exitCode the exit code of the CodeXLGpuProfiler process
    void HandleProfileFinished(int exitCode);

    /// Handles the situation when the profile has finished, and there is no output file
    /// \param strError the user error output message
    void HandleMissingProfileOutput(QString& strError);

    /// helper to deal with a summary generation CodeXLGpuProfiler session completing
    /// \param exitCode the exit code of the CodeXLGpuProfiler process
    void HandleGenSummaryFinished(int exitCode);

    /// helper to deal with a occupancy generation CodeXLGpuProfiler session completing
    /// \param exitCode the exit code of the CodeXLGpuProfiler process
    void HandleGenOccupancyFinished(int exitCode);

    /// Import a session file:
    /// \param pImportedSessionItemData the imported session item data
    /// \param importedSessionDir the imported session destination folder
    bool DoImport(GPUSessionTreeItemData* pTempSessionItemData, const osDirectory& importedSessionDir);

    /// helper extract kernels names from a coma separated string and write to m_pSpecificKernelsFile file
    /// it will be used as input for profiling
    bool ExportSpecificKernelsToFile(gtString& kernels);

    /// Exracts data from archived frame analysis session and creates gpSessionTreeNodeData
    bool ExtractArchivedSession(const QString& strSessionFilePath, osDirectory sessionOSDir, const QString& strProjName, gtString& xmlFileNewName, gtString& strSessionDisplayName);

    /// Rename session files according to imported session naming convention
   bool RenameSessionFiles(const osDirectory& sessionDir, const gtString& stringToReplace, const gtString& newString);


    QStringList                       m_openedProjectList;      ///< List of opened project in this Profile session
    ProfileApplicationTreeHandler*    m_pSessionExplorer;       ///< Reference to GPUSessionTreeItemData Explorer View
    ProjectSettings*                  m_pCurrentProjectSettings; ///< Will have current project settings
    QList<GPUSessionTreeItemData*>    m_deferredSessionList;    ///< list of sessions to add to the explorer when it is shown
    ProfileProcessMonitor*            m_pProfileProcessMonitor; ///< thread instance that monitors the profiled process
    ProfileProcessMonitor*            m_pPagesGenerationProcessMonitor;///< thread instance that monitors the occupancy and summary page generation process
    AsyncRemoteGpuProfilingTask*      m_pRemoteProfilingTask;   ///< thread instance that executes remote profiling
    QFile*                            m_tempEnvVarFile;         ///< temporary file used to pass environment variables to the backend
    QFile*                            m_pSpecificKernelsFile;   ///< temporary file used to pass specific kernel list to profile to the backend

    QString                           m_strImportedSessionName; ///< used to store session name when generating summary pages asynchronously
    QString                           m_strImportedFileName;    ///< used to store file name when generating summary pages asynchronously

    QString                           m_strOccParamsFile;       ///< used to store occupancy params file name when generating occupancy HTML page asynchronously
    QString                           m_strOutputOccHTMLPage;   ///< used to store the name of the HTML file when generating occupancy HTML page asynchronously
    QString                           m_strRemoteProfilingError;///< used to the error message for errors which occur during asynchronous remote profiling

    // frame analysis execution mode
    gpExecutionMode* m_pFrameAnalysisMode;

    /// Static list of file extensions that should be renamed when the session is renamed
    static QList<gtString>           m_sAdditionalFileExtensionsToRename;


    std::vector<unsigned int>        m_kernelOccupancyChartGenerated; ///< flag indicating kernel occupancy chart is generated or not

    /// process id of the GPU profiler
    osProcessId m_GPUProfilerProcessId;
};

#endif // _PROFILE_MANAGER_H_

