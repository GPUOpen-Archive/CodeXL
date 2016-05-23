//=====================================================================

//=====================================================================
#ifndef _GPTRACESUMMARYTAB_H_
#define _GPTRACESUMMARYTAB_H_

#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTGpuProfiling/gpTraceSummaryTable.h>

class gpTraceDataContainer;
class APISummaryInfo;
class ProfileSessionDataItem;
class gpTraceSummaryTable;

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

    void SetTimelineScope(bool useTimeline, quint64 timelineStart, quint64 timelineEnd);

    void RestoreSelection();

    virtual void RefreshAndMaintainSelection(bool check) = 0;

    public slots:
    /// Toggles using selection scope for summary tables
    /// \param check - 'true' to use selection scope, 'false' otherwise
    void OnUseTimelineSelectionScopeChanged(bool check);


    /// Handles selection change in summary table: updates top20 table accordingly
    virtual void OnSummaryTableSelectionChanged() = 0;

    /// Handles cell click change in summary table: zooms in on the min \ max item
    /// \param row - table row index
    /// \param col - table col index
    virtual void OnSummaryTableCellClicked(int row, int col) = 0;

    /// Handles changes in timeline
    /// \param startTime - start time of the timeline
    /// \param endTime - end time of the timeline
    void OnTimelineChanged(quint64 startTime, quint64 endTime);

    /// Handles cell click change in top20 table: check if <show more...> button was pressed
    /// \param row - table row index
    /// \param col - table col index
    void OnTop20TableCellClicked(int row, int col);

    /// Handles cell double-click change in top20 table: select item in timeline
    /// \param row - table row index
    /// \param col - table col index
    virtual void OnTop20TableCellDoubleClicked(int row, int col) = 0;

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

    void TabSummaryCmdListClicked(const QString& commandList);

protected:
    /// Handles item selection in timeline and trace tables.
    /// \param pItem - session data item
    void SyncSelectionInOtherControls(ProfileSessionDataItem* pItem);
    /// Builds top20 table.
    /// \param callIndex - the call index
    /// \param callName - the call name
    /// \param showAll - 'false' by default, means top 5 item are displayed, 'true' means all items should be displayed
    void FillTop20Table(CallIndexId callIndex, const QString& callName, bool showAll = false);


    virtual void FillTop20List(CallIndexId callIndex, const QString& callName) = 0;
    virtual void SetTop20TableCaption(const QString& callName) = 0;

    eCallType m_callType;

    gpSummaryTable* m_pSummaryTable;
    gpTraceView* m_pTraceView;
    /// The session data container
    gpTraceDataContainer* m_pSessionDataContainer;
    QList<ProfileSessionDataItem*> m_top20ItemList;
    acListCtrl* m_pTop20Table;
    quint64 m_timelineStart;
    quint64 m_timelineEnd;
    QLabel* m_pTop20Caption;
    CallIndexId m_currentCallIndex;

private:

    QCheckBox* m_pChkboxUseScope;

    bool m_useTimelineSelectionScope;
    QString m_currentCallName;

};

class gpTraceSummaryTab : public gpSummaryTab
{
    Q_OBJECT
public:
    gpTraceSummaryTab(eCallType callType) :gpSummaryTab(callType) {}
    virtual void RefreshAndMaintainSelection(bool check);

public slots :
    /// Handles selection change in summary table: updates top20 table accordingly
    virtual void OnSummaryTableSelectionChanged();
    virtual void OnTop20TableCellDoubleClicked(int row, int col);
    virtual void OnSummaryTableCellClicked(int row, int col);
    virtual void OnSummaryTableCellDoubleClicked(int, int) {}

protected:
    virtual void FillTop20List(CallIndexId callIndex, const QString& callName);
    virtual void SetTop20TableCaption(const QString& callName);

};

class gpCommandListSummaryTab : public gpSummaryTab
{
    Q_OBJECT
public:
    gpCommandListSummaryTab(eCallType callType) :gpSummaryTab(callType) {}
    virtual void RefreshAndMaintainSelection(bool check);
public slots :
    /// Handles selection change in summary table: updates top20 table accordingly
    virtual void OnSummaryTableSelectionChanged();
    virtual void OnSummaryTableCellClicked(int row, int col);
    virtual void OnSummaryTableCellDoubleClicked(int row, int col);
    virtual void OnTop20TableCellDoubleClicked(int row, int col);
    void SelectCommandList(const QString& commandListName);

protected:
    virtual void FillTop20List(CallIndexId callIndex, const QString& callName);
    virtual void SetTop20TableCaption(const QString& callName);
};

#endif //_GPTRACESUMMARYTAB_H_
