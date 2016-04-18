//------------------------------ gpAppController.h ------------------------------

#ifndef __gpAppController_H
#define __gpAppController_H

#include <QtWidgets>

class afApplicationCommands;
class afApplicationTree;
class afApplicationTreeItemData;
class SharedSessionWindow;


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

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/Session.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #include <AMDTGraphicsServerInterface/Include/AMDTGraphicsServerInterface.h>
#endif

// C++
#include <memory>


class SharedProfileProcessMonitor;

// ----------------------------------------------------------------------------------
// Class Name:          gpAppController : public apIEventsObserver
// General Description: This class is handling the flow of execution for the DX frame
//                      analysis mode. The class is communicating the profile tree handler,
//                      the shared profile manager and the execution handler, and is handling
//                      events and signals coming from these objects, in order to run the profile execution flow
// ----------------------------------------------------------------------------------
class AMDT_GPU_PROF_API gpAppController : public QObject, public apIEventsObserver, public afIRunModeManager, public spISharedProfilerPlugin
{
    Q_OBJECT

public:
    // singleton access:
    static gpAppController& Instance();

    virtual ~gpAppController();

    /// Overrides apIEventsObserver:
    virtual void onEvent(const apEvent& eve, bool& vetoEvent);
    virtual const wchar_t* eventObserverName() const { return L"fpEventObserver"; };

    /// overrides spISharedProfilerPlugin
    /// Get layout for the plugin
    virtual afMainAppWindow::LayoutFormats LayoutFormat() { return afMainAppWindow::LayoutProfilePP; }

    /// Get the project setting path
    virtual gtString ProjectSettingsPath() { return AF_STR_LayoutProfilePP; }

    /// Handle invalid project settings
    virtual void HandleInvalidProjectSettings(bool& isProfileSettingsOK, osProcessId& processId);

    /// Handle opening of a project: load old session
    void ProjectOpened();

public slots:
    void OnProfileStarted(const gtString& name, const spISharedProfilerPlugin* const pCallback, osProcessId processId);

    void OnProfilePaused(const bool& toggled, const spISharedProfilerPlugin* const pCallback);
    void OnProfileStopped(const spISharedProfilerPlugin* const pCallback, bool stopAndExit);

    /// Is handling the session rename slot:
    void OnSessionRename(SessionTreeNodeData* pRenamedSessionData, const osFilePath& oldSessionFilePath, const osDirectory& oldSessionDir);

    /// Is called before a session is renamed:
    void OnBeforeSessionRename(SessionTreeNodeData* pAboutToRenameSessionData, bool& isRenameEnabled, QString& renameDisableMessage);

    /// Is handling the import of a session file:
    /// \param strSessionFilePath the imported file path
    /// \param wasImported true if the imported session belong to power profiler and the import process succeeded
    void OnImportSession(const QString& strSessionFilePath, bool& wasImported);

    /// Is handling the session delete signal. The signal is emitted from profile tree:
    /// \param deletedSessionId the session id for the session that is about to be deleted
    /// \param deleteType remove from tree / remove from file system
    /// \param canDelete[out] can be file be deleted
    void OnSessionDelete(ExplorerSessionId deletedSessionId, SessionExplorerDeleteType deleteType, bool& canDelete);

    /// Display a "Process Running" message in the properties view:
    void DisplayProcessRunningHTML();

signals:

    /// Signal to power profile stopped:
    void ProfileStopped(const QString& sessionName);

protected:
    // Do not allow the use of my default constructor:
    gpAppController();

    friend class fpSingletonsDelete;

    /// singleton instance
    static gpAppController* m_spMySingleInstance;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    GraphicsServerCommunication* m_pGraphicsServerCommunication = NULL;
#endif

    /// afIRunModeManager derived functions:
    virtual afRunModes getCurrentRunModeMask();

    virtual bool canStopCurrentRun();
    virtual bool stopCurrentRun();

    virtual bool getExceptionEventDetails(const apExceptionEvent& exceptionEve, osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce);

    /// Create the session for the current profile:
    gpSessionTreeNodeData* CreateSession(gtString& projectName, osFilePath& projectDirPath, gtString& strSessionDisplayName, osDirectory& sessionOsDir);

    /// Activate the existing session represented in pSessionData
    /// \param pSessionData the session item data
    void ActivateExistingSession(SessionTreeNodeData* pSessionData);

    /// Get a list of sessions for the current opened project
    /// \param vector to hold the results
    void GetFrameProfilingSessionsList(gtList<gtString>& sessionsPathAsStrList);

    /// Get the application commands instance:
    afApplicationCommands* m_pApplicationCommands;
    afApplicationTree* m_pApplicationTree;

    /// Currently running session item data
    gpSessionTreeNodeData* m_pCurrentlyRunningSessionData;
};

#endif //__gpAppController_H
