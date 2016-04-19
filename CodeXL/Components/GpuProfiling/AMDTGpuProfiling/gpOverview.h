//------------------------------ gpOverview.h ------------------------------

#ifndef __GPOVERVIEW_H
#define __GPOVERVIEW_H

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTApplicationComponents/Include/acTabWidget.h>

// Framework:
#include <AMDTApplicationFramework/Include/views/afBaseView.h>

// Local:
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/gpBaseSessionView.h>
#include <AMDTGpuProfiling/gpUIManager.h>


class gpSessionTreeNodeData;
class acTreeCtrl;


class AMDT_GPU_PROF_API gpOverview : public gpBaseSessionView
{
    Q_OBJECT

public:

    /// Constructor
    gpOverview(QWidget* pParent);

    virtual ~gpOverview();

    /// Display the session file. This function should be implemented for session views with multiple children
    /// \param sessionFilePath the file path for the requested session
    /// \param sessionInnerPage the item type describing the inner view to open, or AF_TREE_ITEM_ITEM_NONE when the root is supposed to open
    virtual bool DisplaySession(const osFilePath& sessionFilePath, afTreeItemType sessionInnerPage);

    /// Set the data model
    virtual void SetProfileDataModel(gpTraceDataModel* pSessionDataModel);

private slots:

    /// Is handling the click on the capture frame data button
    void OnRenderButton();

    /// Is handling the timeline link click
    void OnTimelineClick();

    /// Is handling the profile link click
    void OnProfileClick(const QString& link);

    /// Is handling the object link click
    void OnObjectClick();

    void OnUpdatePresetChanged(const QString&);
    void OnItemClicked(QTreeWidgetItem*);
    void OnAddRemoveCounters();

private:

    /// init the view layout
    void InitLayout();

    /// Send selected counters to the server
    void SendCountersSelectedToServer();

    /// Is called after the selected counters are updated.
    /// The function updates the view labels with the current selected performance counters
    void UpdateSelectedPerformanceCountersTexts();

    /// Display the frame info on the widgets that contain this data
    void DisplayFrameInfo();
private:

    /// Main view layout:
    QVBoxLayout* m_pMainLayout;

    /// frame analysis label that needs changing based on the frame number
    QLabel* m_pFrameDescriptionLabel;

    /// Timeline link label
    QLabel* m_pTimelineLabel;

    /// TODO Object link label
    QLabel* m_pObjectLabel;

    /// Perf counters link label
    QLabel* m_pPerfCountersLabel;

    /// Pre defined counters sets combobox
    QComboBox* m_pPredefinedCountersComboBox;

    /// Counters Label
    QLabel* m_pCountersLabel;

    /// Counters Tree control
    acTreeCtrl* m_pCountersTree;

    /// Counter description
    QLabel* m_pCounterDescLabel;

    /// add remove counters button
    QPushButton* m_pAddRemoveButton;

    /// Main caption label
    QLabel* m_pCaptionLabel;

    /// The image
    QLabel* m_pImageLabel;

    /// HTML statistics label
    QLabel* m_pStatisticsHTMLLabel;

    /// Render again button
    QPushButton* m_pRenderButton;

    /// Contain the frame information for this frame
    FrameInfo m_frameInfo;


};
#endif // __GPOVERVIEW_H
