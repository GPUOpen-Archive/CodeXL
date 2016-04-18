//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpAppController.h
///
//==================================================================================

//------------------------------ tpAppController.h ------------------------------

#ifndef __tpAppController_H
#define __tpAppController_H

#include <QtWidgets>

class afApplicationCommands;
class afApplicationTree;
class afApplicationTreeItemData;
class ppProcessMonitor;
class ppWindowsManagerHelper;


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

class SharedProfileProcessMonitor;
class  tpSessionTreeNodeData;

// ----------------------------------------------------------------------------------
// Class Name:          tpAppController : public apIEventsObserver
// General Description: An object responsible for the managing of application flow of
//                      thread profile
// ----------------------------------------------------------------------------------
class tpAppController : public QObject, public apIEventsObserver, public afIRunModeManager, public spISharedProfilerPlugin
{
    Q_OBJECT

public:
    // singleton access:
    static tpAppController& Instance();

    virtual ~tpAppController();

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

    /// Handle opening of a project: load old session
    void ProjectOpened();

    /// Read the run info and store it into the input session data
    /// \param strSessionFilePath the session file path
    /// \param pSessionData the session data on which the data will be store
    void ReadRunInfo(const QString& strSessionFilePath, tpSessionTreeNodeData* pSessionData);

public slots:
    void OnProfileStarted(const gtString& name, const spISharedProfilerPlugin* const pCallback, osProcessId processId);

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
    tpAppController();

    /// singleton instance
    static tpAppController* m_spMySingleInstance;

    /// afIRunModeManager derived functions:
    virtual afRunModes getCurrentRunModeMask();

    virtual bool canStopCurrentRun();
    virtual bool stopCurrentRun();

    virtual bool getExceptionEventDetails(const apExceptionEvent& exceptionEve, osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded, bool& kernelDebuggingEnteredAtLeastOnce);

    /// Create the session for the current running profile session:
    void AddSessionToTree();

    void GetThreadProfilingSessionsList(gtList<gtString>& sessionsPathAsStrList);

    /// Activate the tree item that was clicked:
    /// \param tree item to activate
    bool ActivateItem(QTreeWidgetItem* pItemToActivate);

    /// Create a session for a loaded session:
    /// \param session file path to load from
    void CreateLoadedSession(gtString& sessionFilePath);

    bool LaunchProcess();

    /// Write the session info for the currently running session:
    void WriteRunInfo();

protected:

    /// Get the application commands instance:
    afApplicationTree* m_pApplicationTree;

    /// The currently running session data:
    tpSessionTreeNodeData* m_pCurrentlyRunningSessionData;

    /// Thread to monitor the executed application:
    SharedProfileProcessMonitor* m_pMonitorProcessThread;

};

#endif //__tpAppController_H
