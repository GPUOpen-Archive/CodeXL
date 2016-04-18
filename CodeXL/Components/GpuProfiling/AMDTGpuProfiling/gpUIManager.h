//------------------------------ gpUIManager.h ------------------------------


#ifndef _GPUIMANAGER_H_
#define _GPUIMANAGER_H_

#include <qtIgnoreCompilerWarnings.h>

// Qt:
#include <QtCore>
#include <QtWidgets>

// Infra:
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtList.h>

#include <TSingleton.h>

// Infra:
#include <AMDTRemoteClient/Include/RemoteClientDataTypes.h>


// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/gpSessionUpdaterThread.h>

class gpSessionTreeNodeData;
class GPUSessionTreeItemData;
class gpSessionView;
class gpTraceDataModel;
class gpObjectDataModel;
class SharedSessionWindow;
class gpBaseSessionView;


/// Holds the meta data for a frame analysis session.
/// This struct will be stored and loaded in the cxldsh file, and will be displayed in the session dashboard view
struct gpFASessionData
{
public:

    /// The session working folder
    gtString m_workingFolder;

    /// The session executable full path
    gtString m_exePath;

    /// The session command line arguments
    gtString m_cmdArgs;

    /// The session environment variables
    gtString m_envVars;

    /// Was the session executed on a remote host?
    bool m_isRemoteHost;

    /// Remote host IP (or an empty string for local host)
    gtString m_hostIP;

    /// Fill sessionData with data from the current project settings
    void Init();

    /// Writes the sessionData to the dashboard file
    bool Write(const osFilePath& dashboardFilePath);

    /// Reads the sessionData from the dashboard file
    bool Read(const osFilePath& dashboardFilePath);

    /// Returns 'true' if host is on a remote machine
    bool IsRemoteHost();
};


/// A class handling the signals and slots management required for communication between all UI elements of the
/// Frame analysis mode
class AMDT_GPU_PROF_API gpUIManager : public QObject, public TSingleton<gpUIManager>
{
    Q_OBJECT

    // required so that TSingleton can access our constructor
    friend class TSingleton<gpUIManager>;

public:

    /// Emits a signal that the application execution was stopped
    void OnApplicationEnded();

    /// Return the currently running session data
    gpSessionTreeNodeData* CurrentlyRunningSessionData() const { return m_pCurrentlyRunningSessionData; };

    /// Set the currently running session data
    void SetCurrentlyRunningSessionData(gpSessionTreeNodeData* pSessionData);

    /// Get the list of frames thumbnails for the requested session
    /// \param sessionFilePath the session file path
    /// \param frameFoldersList the list of frame captured on the file system
    void GetListOfFrameFolders(const osFilePath& sessionFilePath, QList<int>& frameIndicesList);

    /// Get the number frames thumbnails for the requested session
    /// \param sessionFilePath the session file path
    /// \return the number frames in the requested session
    int GetNumberOfFrameFolders(const osFilePath& sessionFilePath);

    /// Is called when a session window is about to be closed
    /// \param pClosedSessionWindow the about to be closed session window
    void OnWindowClose(SharedSessionWindow* pClosedSessionWindow);

    /// Prepares a data model for the frame analysis session if necessary
    /// \param the displayed file path
    /// \param pSessionView the created session view
    /// \param pSessionData the owner session item data
    void PrepareTraceData(const osFilePath& filePath, gpBaseSessionView* pSessionView, GPUSessionTreeItemData* pSessionData);

    /// Prepares a Object data model for the frame analysis session if necessary
    /// \param the displayed file path
    /// \param pSessionView the created session view
    /// \param pSessionData the owner session item data
    void PrepareObjectData(const osFilePath& filePath, gpBaseSessionView* pSessionView, GPUSessionTreeItemData* pSessionData);

    /// Parse the XML text into a FrameInfo struct
    /// \param frameInfoXML the frame information as an XML string
    /// \param frameInfo[output] the frame information as struct
    bool FrameInfoFromXML(const gtASCIIString& frameInfoXML, FrameInfo& frameInfo);

    /// Write the FrameInfo struct into an XML string
    /// \param frameInfothe frame information as struct
    /// \param frameInfoXML[output] the frame information as an XML string
    void FrameInfoToXML(const FrameInfo& frameInfo, QString& xmlString);

    /// Get the frame info that was saved on disk in run-time
    /// \param sessionFilePath the session file path
    /// \param frameInfo[out] the requested frame info. The frame index is already set as input
    /// \return true on success
    bool GetFrameInfo(const osFilePath& sessionFilePath, FrameInfo& frameInfo);

    /// Returns the paths for a frame
    /// \param sessionFilePath the session file path
    /// \param frameIndex the frame index
    /// \param frameDir[out] the frame directory
    /// \param overviewFilePath[out] the overview full file path as string
    /// \param thumbFilePath[out] the thumbnail full file path as string
    /// \param shouldCreateFrameDir (true by default) should the frame folder be created if it doesn't exist. This flag should be turned off when the function
    ///                             is used only for paths calculations
    /// \return true for success
    bool GetPathsForFrame(const osFilePath& sessionFilePath, int frameIndex, QDir& frameDir, QString& overviewFilePath, QString& thumbFilePath, bool shouldCreateFrameDir = true);

    /// Extracts the frame info from an overview file
    /// \param overviewFilePath the overview file path
    /// \param frameInfo[out] the frame info struct
    bool GetFrameImageAndInfo(const QString& overviewFilePath, FrameInfo& frameInfo);

    /// Get all sessions and frames from the input XML, get it from the remote server, and cache it on local disk
    /// \param capturedFramesAsXML the XML describing the existing sessions and frames on the remote server
    /// \return true for success
    bool CacheProjectSessionsList(const gtString& capturedFramesAsXML);

    bool ProjectSessionsMapFromXML(const gtString& capturedFramesAsXML, gtMap<gtString, gtVector<int>>& sessionFramesMap);

    /// Does session has captured frames?
    bool DoesSessionHasFrames(const QDir& sessionDir);

    /// Saves the run-time captured frame data on disk. The function saved data that will later enable opening the
    /// session view when the server is not connected
    /// \param frameInfo the frame info struct
    /// \return true for success
    bool SaveCurrentSessionFrameDataOnDisk(const FrameInfo& frameInfo);

    gpSessionUpdaterThread& UIUpdaterThread() { return m_updaterThread; };

    /// Parse the XML that we got from the server, and add the captured frames to the list of captured frames
    /// \param capturedFramesAsXML the XML string describing the captured frames
    void AddCapturedFramesFromServer(const gtString& capturedFramesAsXML);

    /// Get the file path of the current executing frame analysis
    osFilePath CurrentExecutingFrameAnalysisPath();

    /// Create and write a dashboard file with the session details
    void WriteDashboardFile();

    /// Read a dashboard file with the session details
    bool ReadDashboardFile(gpFASessionData& sessionData, const osFilePath& dashBoardFilePath);

    /// Get the dashed line pen
    const QPen& DashedLinePen() const { return m_dashedLinePen; }

    /// Close all the opened file related to the session
    /// \param pSession the item data describing the session for which all windows should be closed
    void CloseAllSessionWindows(GPUSessionTreeItemData* pSession);

    /// Is called when stop or capture buttons are clicked from anywhere outside the views, is updating all the UI buttons
    void UpdateUI();

protected slots:
    /// Emits a signal that a frame was captured
    /// \param frameInfo the captured frame info
    void OnFrameCapture(int frameIndex);

    /// Is called when the current running session window is about to be closed:
    /// \param subwindowFilePath the file path of the sub window which is about to be closed
    /// \param [out] shouldClose true iff the window can be closed
    void OnBeforeActiveSubWindowClose(const osFilePath& sessionFilePath, bool& shouldClose);

signals:

    /// Signal stating that a frame was captured
    void ApplicationRunEnded(gpSessionTreeNodeData* pRunningSessionData);

    /// Notify that a new captured frame is ready
    void CapturedFrameUpdateReady(int);

    /// Notify the views that the UI should be updated
    void UIUpdated();

private:
    /// Initializes the GpuProfilerMDIViewsCreator singleton
    gpUIManager();

    ~gpUIManager();

    /// check if current sessionData still valid
    bool IsSessionDataValid();

    /// Contain the currently running session data
    gpSessionTreeNodeData* m_pCurrentlyRunningSessionData;

    /// Map from session name to data model
    QMap<osFilePath, gpTraceDataModel*> m_traceFilePathToDataModelMap;

    QMap<osFilePath, gpObjectDataModel*> m_objectFilePathToDataModelMap;

    /// Thread that is used to update the data from the server
    gpSessionUpdaterThread m_updaterThread;

    /// Is used to draw dashed lines in trace views
    QPen m_dashedLinePen;
};

#endif // _GPVIEWSCREATOR_H_
