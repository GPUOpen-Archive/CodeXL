//------------------------------ gpExecutionMode.h ------------------------------

#ifndef __GPEXECUTIONMODE_H
#define __GPEXECUTIONMODE_H

// Infra:
#include <AMDTOSWrappers/Include/osTime.h>

// Framework:
#include <AMDTApplicationFramework/Include/afIExecutionMode.h>
#include <AMDTApplicationFramework/Include/afIRunModeManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/gpProjectSettings.h>
#include <AMDTGpuProfiling/gpRemoteGraphicsBackendHandler.h>
#include <AMDTGpuProfiling/gpUIManager.h>

class GraphicsServerCommunication;
class gpSessionTreeNodeData;
class ProfileProcessMonitor;
class SessionTreeNodeData;

// ----------------------------------------------------------------------------------
// Class Name:           gpExecutionMode
// General Description:  This class inherits afIExecutionMode and afIRunModeManager and
//                       is implementing the mode management and run mode management functionality
//                       for the frame analysis mode.
//                       The class is also managing the connection and communication with the
//                       frame analysis graphics server
// ----------------------------------------------------------------------------------
class AMDT_GPU_PROF_API gpExecutionMode : public afIExecutionMode, public afIRunModeManager
{
public:
    gpExecutionMode();
    virtual ~gpExecutionMode();

    void Initialize();

    /// Mode name for identification
    virtual gtString modeName();

    /// Execution status relevance
    /// returns true if relevant
    virtual bool IsExecutionStatusRelevant() { return true; }

    /// The name of the action the mode encompasses
    virtual gtString modeActionString();

    /// The action verb the mode encompasses
    virtual gtString modeVerbString();

    /// Mode description for tooltips
    virtual gtString modeDescription();

    /// Perform a startup action. Return true iff the mode support the requested action:
    virtual bool ExecuteStartupAction(afStartupAction action);

    /// Perform a startup action. Return true iff the mode support the requested action:
    virtual bool IsStartupActionSupported(afStartupAction action);

    /// Return true iff the execution mode supports remote host scenario for the requested session type:
    virtual bool IsRemoteEnabledForSessionType(const gtString& sessionType);

    /// Execute the command
    virtual void execute(afExecutionCommandId commandId);

    /// Handle the UI update
    virtual void updateUI(afExecutionCommandId commandId, QAction* pAction);

    /// Get the number of session type
    virtual int numberSessionTypes() { return 1; }

    /// Get the name of each session type
    virtual gtString sessionTypeName(int sessionTypeIndex);

    /// Get the icon of each session type
    virtual QPixmap* sessionTypeIcon(int sessionTypeIndex);

    /// Return the index for the requested session type:
    virtual int indexForSessionType(const gtString& sessionType) { (void)(sessionType); return 0;};

    /// return the layout name used for this mode at specific time:
    virtual afMainAppWindow::LayoutFormats layoutFormat() { return afMainAppWindow::LayoutFrameAnalysis; };

    /// return the project settings path used for this mode
    virtual gtString ProjectSettingsPath() { return GPU_STR_projectSettingExtensionDisplayName; };

    /// is the mode enabled at all
    virtual bool isModeEnabled();

    /// get the properties view message to start the execution of the mode:
    virtual gtString HowToStartModeExecutionMessage();

    /// Overrides afIRunModeManager
    virtual afRunModes getCurrentRunModeMask();

    /// Overrides afIRunModeManager
    virtual bool canStopCurrentRun();

    /// Overrides afIRunModeManager
    virtual bool stopCurrentRun();

    /// Overrides afIRunModeManager
    virtual bool getExceptionEventDetails(const apExceptionEvent& exceptionEve,
                                          osCallStack& exceptionCallStack, bool& openCLEnglineLoaded, bool& openGLEnglineLoaded,
                                          bool& kernelDebuggingEnteredAtLeastOnce);

    /// handle ending of the application execution
    void ApplicationEnded();


    /// Get the frame trace for a specific frame in a session
    /// The function checks if the trace file is already cached on the local disk, and if not, it run the server
    /// and get the trace from the server replay
    /// \param sessionFilePath the session file path
    /// \param frameIndex the first and last frame indices (equal for single frame trace)
    /// \param traceFilePath[out] local path for the trace file
    /// \return true for success
    bool GetFrameTraceFromServer(const osFilePath& sessionFilePath, FrameIndex frameIndex, osFilePath& traceFilePath);

    /// Get the frame objects for a specific frame in a session
    /// The function checks if the object database file is already cached on the local disk, and if not, it run the server
    /// and get the object database from the server replay
    /// \param sessionFilePath the session file path
    /// \param frameIndex the frame index
    /// \param objectFilePath[out] local path for the trace file
    /// \return true for success
    bool GetFrameObject(const osFilePath& sessionFilePath, int frameIndex, osFilePath& objectFilePath);

    /// Shuts down the graphics server, and close the communication object
    void ShutServerDown();

    /// Capture type enumeration, consisted with CaptureType in Graphics\Server\Common\Tracing\CaptureTypes.h
    enum FrameAnalysisCaptureType
    {
        FrameAnalysisCaptureType_APITrace = 1,
        FrameAnalysisCaptureType_GPUTrace = 2,
        FrameAnalysisCaptureType_LinkedTrace = 3,
        FrameAnalysisCaptureType_FullFrameCapture = 4
    };
    /// Captures the current frame, and add the relevant tree nodes and files
    /// \return true for success
    bool CaptureFrame(FrameAnalysisCaptureType captureType);

    /// Captures the performance counters data for the current frame, writes the trace file and opens it
    /// \param frameIndex the index of the frame that should be captured
    /// \param shouldOpenFile should the performance counters file be opened?
    /// \return true for success
    bool CapturePerformanceCounters(FrameIndex frameIndex, bool shouldOpenFile);

    // Get the server communication connection
    GraphicsServerCommunication* GetGraphicsServerComminucation() { return m_pGraphicsServerCommunication; }

    /// Get access to the settings
    gpProjectSettings& ProjectSettings() { return m_settings; }


    /// Start frame analysis
    void OnStartFrameAnalysis();

    /// Capture frame analysis and show the user the dashboard of the current image
    void OnFrameAnalysisCapture(gpExecutionMode::FrameAnalysisCaptureType captureType);

    /// Is start enabled?
    bool IsStartEnabled(gtString& startActionText);

    /// Is stop enabled?
    bool IsStopEnabled();

    /// Is capture action enabled?
    bool IsCaptureEnabled();

    /// Set the is capturing flag.
    void SetIsCapturing(bool isCapturing) { m_isCapturing = isCapturing; }

    /// Get a captured frame data for the current running session
    /// \param frameInfo[out]. frameInfo.m_frameIndex contains the requested frame index, and the other fields are filled by the function
    /// \return true for success
    bool GetCapturedFrameData(FrameInfo& frameInfo);

    /// Get a captured frame data for a requested project and session
    /// \param projectName the project for which the frame data is required
    /// \param sessionName the session for which the frame data is required
    /// \param frameInfo[out]. frameInfo.m_frameIndex contains the requested frame index, and the other fields are filled by the function
    /// \return true for success
    bool GetCapturedFrameData(const gtString& projectName, const gtString& sessionName, FrameInfo& frameInfo);

    /// Connects to the server (defined in the current project settings), and loads the sessions from the server
    /// For each session, this function creates a cached copy on the local machine
    /// \return true for success
    bool RefreshLoadedProjectSessionsFromServer();

    /// Update the capture frames from server (the user can capture a frame using keyboard buttons. We want this frames to be displayed in the client UI)
    void UpdateCapturedFrameFromServer();

    /// Allow the mode to terminate gracefully at the end of CodeXL.
    void Terminate();

    /// Prepares the trace file for the given frame
    /// \param sessionFilePath the session file path (cxlfovr)
    /// \param frameIndex the first and last frame indices (equal for single frame trace)
    /// \param pTreeNodeData the frame tree node data
    /// \param pTraceView the session view
    bool PrepareTraceFile(const osFilePath& sessionFilePath, FrameIndex frameIndex, SessionTreeNodeData* pTreeNodeData, gpBaseSessionView* pTraceView, bool prepareTraceData = true);

    /// Handles remote agent on execution mode change.
    void onExecutionModeChanged();

    /// Delete the frame analysis session on the server
    /// \param pSessionData the deleted session data
    void HandleSessionDeletion(GPUSessionTreeItemData* pSessionData);

protected:

    /// Check that the project settings are valid for frame analysis
    /// \return true for success
    bool CheckProjectSettings();

    /// Update the application title, and the tree text with the status of the frame analysis
    void UpdateApplicationTitle();

    /// Create new running session
    /// This function is called after the server is initialized
    /// The function initializes a session and add it to the tree
    void CreateNewSession();

    /// Create a session
    gpSessionTreeNodeData* CreateSession(gtString& projectName, gtString& strSessionDisplayName, osDirectory& sessionOsDir);

    /// Launch the graphic server
    /// \param messageShown was an error message shown in the launchServer function
    /// \param frameXMLFullPath will contain the trace server xml full path in a replay mode
    /// \return true for successful launch
    bool LaunchServer(bool& messageShown, const gtASCIIString& frameXMLFullPath = "");

    /// Initialize the CodeXL remote agent
    /// \return true for success
    bool InitializeCodeXLRemoteAgent();

    bool ConnectToServer();


private:
    void TerminateRemoteAgent();
    bool GetFrameAnalisysServerPaths(gtString& capturePlayerPathAsStr, osFilePath& serverPath) const;
    /// Kills running Raptr.exe or project defined processes, that could jeopardize the FA execution
    /// \return None
    void HandleSpecialRunningProcesses();
    /// Kills running process that is defined in project settings, that could jeopardize the FA execution
    /// \return None
    void HandleRunningProjectProcess();

    /// Returns user error message  when Rapture processes are killed
    /// \param isRaptrRunning input parameter, true if Rapture process is running
    /// \param isFrapsRunning input parameter, true if Frap process is running
    /// \return QString formatted error message
    QString GetRaptErrMessage(const bool isRaptrRunning, const bool isFrapsRunning) const;

private:

    /// Is frame analysis running:
    bool m_isFrameAnalysisRunning;

    /// Graphics communication server
    GraphicsServerCommunication* m_pGraphicsServerCommunication;

    /// process id of the launched
    osProcessId m_serverProcessID;

    /// executed application process id
    gtASCIIString m_applicationProcessID;

    /// Handles communication with the remote Graphics beck end server
    gpRemoteGraphicsBackendHandler m_remoteGraphicsBackendServerLauncher;

    /// Settings
    gpProjectSettings m_settings;

    /// CodeXL agent process ID
    osProcessId m_cxlAgentProcessID;

    /// Timer that keep the last updated captured frames time
    osTime m_capturedFramesUpdateTime;

    /// Contain true while the capture operation is in progress
    bool m_isCapturing;
    bool m_isRemoteAgentInUse = false;

    bool m_isExportInProgress;
    bool m_isImportInProgress;

    /// Is frame analysis running:
    bool m_isFrameAnalysisConnecting;

    FrameAnalysisCaptureType m_captureType;
};
#endif //__GPEXECUTIONMODE_H

