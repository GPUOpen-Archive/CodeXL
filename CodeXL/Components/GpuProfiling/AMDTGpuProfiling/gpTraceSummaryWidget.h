//=====================================================================

//=====================================================================
#ifndef _GPTRACESUMMARYWIDGET_H_
#define _GPTRACESUMMARYWIDGET_H_
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/Include/acTabWidget.h>
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/gpTraceView.h>
#include <AMDTGpuProfiling/gpTraceSummaryTable.h>

class gpTraceDataContainer;
class APISummaryInfo;
class ProfileSessionDataItem;
class gpTraceSummaryTable;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// classes structure:
// gpTraceSummaryWidget: contains 2 tabs (class gpSummaryTab) each for a different call type
// gpSummaryTab: contains all UI controls of a tab item:
//              1. summary table (gpSummaryTable) which displays all calls summary
//              2. top 5 table   (acListCtrl) which displays top 5 items of the call selected in the summary table
//              3. QT items: labels, check button
// gpSummaryTable: displays all calls summary of a given type (API / GPU)
// gpSummaryLogic: collects all calls of a given type and collates them by call name
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------------------
// Class Name:          gpSummaryTab
// General Description: Handles all UI controls related to the same call type: API\GPU
//              1. summary table (gpSummaryTable) which displays all calls summary
//              2. top 5 table   (acListCtrl) which displays top 5 items of the call selected in the summary table
//              3. QT items: labels, check button
// ----------------------------------------------------------------------------------
class gpSummaryTab : public QWidget
{
    Q_OBJECT

    friend class gpTraceSummaryWidget;
public:
    /// Ctor
    /// \param callType the type of items displayed in this tab
    gpSummaryTab(eCallType callType);

    /// Initializes the summary widget
    /// \param pDataContainer
    /// \param pSessionView
    bool Init(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, quint64 timelineStart, quint64 timelineEnd);

    bool GetSelectedCallIndex(CallIndexId& apiCall, QString& callName);

    void SetTimelineScope(bool useTimeline, quint64 timelineStart, quint64 timelineEnd);

    void RestoreSelection();

public slots:
    /// Toggles using selection scope for summary tables
    /// \param check - 'true' to use selection scope, 'false' otherwise
    void OnUseTimelineSelectionScopeChanged(bool check);

    void RefreshAndMaintainSelection(bool check);

    /// Handles selection change in summary table: updates top5 table accordingly
    void OnSummaryTableSelectionChanged();

    /// Handles cell click change in summary table: zooms in on the min \ max item
    /// \param row - table row index
    /// \param col - table col index
    void OnSummaryTableCellClicked(int row, int col);

    /// Handles changes in timeline
    /// \param startTime - start time of the timeline
    /// \param endTime - end time of the timeline
    void OnTimelineChanged(quint64 startTime, quint64 endTime);

    /// Handles cell click change in top5 table: check if <show more...> button was pressed
    /// \param row - table row index
    /// \param col - table col index
    void OnTop5TableCellClicked(int row, int col);

    /// Handles cell double-click change in top5 table: select item in timeline
    /// \param row - table row index
    /// \param col - table col index
    void OnTop5TableCellDoubleClicked(int row, int col);

    /// Handles thread visibility filter change in summary table: show only items which belong to a visible thread
    void OnTimelineFilterChanged();

    /// Is handling the find click
    void OnFind();

    /// Is handling select all edit command
    void OnEditSelectAll();

    /// Is handling copy edit command
    void OnEditCopy();

signals:
    /// Called when <ue timeline scope> checkbox in one of the tabs is checked.
    void TabUseTimelineSelectionScopeChanged(bool check);

    void TabSummaryItemClicked(ProfileSessionDataItem* pItem);

private:
    /// Handles item selection in timeline and trace tables.
    /// \param pItem - session data item
    void SyncSelectionInOtherControls(ProfileSessionDataItem* pItem);

    /// Builds top5 table.
    /// \param callIndex - the call index
    /// \param callName - the call name
    /// \param showAll - 'false' by default, means top 5 item are displayed, 'true' means all items should be displayed
    void FillTop5Table(CallIndexId callIndex, const QString& callName, bool showAll = false);

    eCallType m_callType;

    gpTraceView* m_pTraceView;
    gpTraceSummaryTable* m_pSummaryTable;
    acListCtrl* m_pTop5Table;
    QLabel* m_pTop5Caption;
    QCheckBox* m_pChkboxUseScope;

    bool m_useTimelineSelectionScope;
    quint64 m_timelineStart;
    quint64 m_timelineEnd;
    QList<ProfileSessionDataItem*> m_top5ItemList;
    QString m_currentCallName;
    CallIndexId m_currentCallIndex;

};

// ----------------------------------------------------------------------------------
// Class Name:          gpTraceSummaryWidget
// General Description: The top-most container for all tabs
// ----------------------------------------------------------------------------------
class gpTraceSummaryWidget : public acTabWidget
{
    Q_OBJECT

public:
    /// class constructor.
    gpTraceSummaryWidget(QWidget* pParent = nullptr);

    /// class destructor.
    virtual ~gpTraceSummaryWidget();/// API Summary widget class

    ///
    void Init(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, quint64 timelineStart, quint64 timelineRange); // bool

    /// Select a call in API Summary table
    void SelectAPIRowByCallName(const QString& callName);
    /// Select a call in GPU Summary table
    void SelectGPURowByCallName(const QString& callName);
    /// Clear selection in API Summary table
    void ClearAPISelection();
    /// Clear selection in GPU Summary table
    void ClearGPUSelection();

public slots:
    /// Updates min and max time
    /// \param rangePoint - the min and max values
    /// \param isRelativeRangeStartTime - determines whether to use the min value as is or to add it to the current start time (different controls send the data in different ways)
    void OnTimelineChanged(const QPointF& rangePoint, bool isRelativeRangeStartTime = true);
    ///
    void OnTimelineFilterChanged(QMap<QString, bool>& threadNameVisibilityMap);

    void OnCurrentChanged(int activeTabIndex);

    void OnUseTimelineSelectionScopeChanged(bool check);

    /// Is handling the find click
    void OnFind();

    /// Is handling select all edit command
    void OnEditSelectAll();

    /// Is handling copy edit command
    void OnEditCopy();

    void OnTabSummaryItemClicked(ProfileSessionDataItem* pItem);

signals:
    void SummaryItemClicked(ProfileSessionDataItem* pItem);

private:

    /// tabs
    gpSummaryTab* m_tabs[eCallType::MAX_TYPE];
    /// trace view
    gpTraceView* m_pTraceView;

    bool m_useTimelineSelectionScope;
    quint64 m_timelineAbsoluteStart;
    quint64 m_timelineStart;
    quint64 m_timelineEnd;
};


#endif //_GPTRACESUMMARYWIDGET_H_
