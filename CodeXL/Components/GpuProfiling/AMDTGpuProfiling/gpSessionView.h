//------------------------------ gpSessionView.h ------------------------------

#ifndef __gpSessionView_H
#define __gpSessionView_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acTabWidget.h>
#include <AMDTRemoteClient/Include/RemoteClientDataTypes.h>

// Framework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/gpBaseSessionView.h>
#include <AMDTGpuProfiling/gpUIManager.h>


class gpSessionTreeNodeData;
class acThumbnailView;


Q_DECLARE_METATYPE(FrameInfo);


class AMDT_GPU_PROF_API gpSessionView : public gpBaseSessionView
{
    Q_OBJECT

public:

    /// Constructor
    gpSessionView(QWidget* pParent, const osFilePath& sessionPath);

    /// Destructor
    virtual ~gpSessionView();

    /// Display the session file. This function should be implemented for session views with multiple children
    /// \param sessionFilePath the file path for the requested session
    /// \param sessionInnerPage the item type describing the inner view to open, or AF_TREE_ITEM_ITEM_NONE when the root is supposed to open
    /// \param [out] errorMessage when the display fails, errorMessage should contain a message for the user
    virtual bool DisplaySession(const osFilePath& sessionFilePath, afTreeItemType sessionInnerPage, QString& errorMessage);

private slots:

    /// Handles the "Simulate User Capture" button (only for debug)
    void OnSimulateCaptureButtonClick();

    /// Handles the "Capture" button
    void OnCaptureButtonClick();
    /// Handles the "Capture" button
    void OnCaptureCPUButtonClick();
    /// Handles the "Capture" button
    void OnCaptureGPUButtonClick();

    /// Handles the "Stop" button
    void OnStopButtonClick();

    /// Handles the "Open Timeline" button
    void OnOpenTimelineButtonClick();

    void OnItemDoubleClicked(const QVariant& capturedFrameId);
    void OnItemPressed(const QVariant& capturedFrameId);

    /// Is handling the FrameCaptured signal
    void OnCaptureFrameData(int firstFrameIndex, int lastFrameIndex);

    /// \param pRunningSessionData the item data describing the session
    void OnApplicationRunEnded(gpSessionTreeNodeData* pRunningSessionData);

    /// Is handling the UI update signal
    void OnUpdateUI();

    /// Is called for each timer timeout
    void OnCurrentFrameUpdate(QPixmap* pCurrentFramePixmap, const FrameInfo& frameInfo);

    /// Handle termination of the update thread
    void OnUpdateThreadFinished();

private:

    /// init the view layout
    void InitLayout();

    /// Add all the thumbnails for a session that is not running
    void FillOfflineSessionThumbnails();

    /// Add captured frame to view
    /// \param frameInfo the struct describing the frame information
    void AddCapturedFrame(const FrameInfo& frameInfo);

    /// Set the main image frame
    /// \param frameIndex the current frame index (by default -1 for the current frame)
    void DisplayCurrentFrame(FrameIndex frameIndex);

    /// Fill the execution HTML description
    void FillExecutionDetails();

    /// update number of frames in dashboard caption
    void UpdateCaption();

    /// The item data holds the currently running session
    gpSessionTreeNodeData* m_pSessionData;

    /// The image
    QLabel* m_pImageLabel;

    /// A button to simulate a user caption. Is used only for testing the user capture feature,
    /// until it is implemented in the server
    QToolButton* m_pSimulateUserCaptureButton;

    /// Capture button
    QToolButton* m_pCaptureButton;
    QToolButton* m_pCaptureButtonCPU;
    QToolButton* m_pCaptureButtonGPU;

    /// Stop button
    QToolButton* m_pStopButton;

    /// Open timeline button
    QToolButton* m_pOpenTimelineButton;

    /// Frame details
    QLabel* m_pCurrentFrameDetailsLabel;

    /// Current frame caption
    QLabel* m_pCurrentFrameCaptionLabel;

    /// Execution HTML label
    QLabel* m_pPropsHTMLLabel;

    QLabel* m_pCapturedFramesCaptionLabel;

    /// The thumbnail view containing the captured frames
    acThumbnailView* m_pSnapshotsThumbView;

    /// Is the session currently running?
    bool m_isSessionRunning;

    /// Used to block the call to DisplayCurrentFrame on timer tick
    bool m_isDisplayingCurrentFrame;

    /// Is used for the frame capture simulation. Should be removed when the user capture is implemented in the server
    int m_lastCapturedFrameIndex;

    osFilePath m_sessionPath;
};
#endif // __gpSessionView_H