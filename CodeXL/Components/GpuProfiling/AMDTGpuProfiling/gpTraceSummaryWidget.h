//=====================================================================

//=====================================================================
#ifndef _GPTRACESUMMARYWIDGET_H_
#define _GPTRACESUMMARYWIDGET_H_
#include <AMDTGpuProfiling/gpTraceSummaryTab.h>

#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/gpTraceView.h>
#include <AMDTApplicationComponents/Include/acTabWidget.h>

class gpTraceDataContainer;
class APISummaryInfo;
class ProfileSessionDataItem;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// classes structure:
// gpTraceSummaryWidget: contains 2 tabs (class gpSummaryTab) each for a different call type
// gpSummaryTab: contains all UI controls of a tab item:
//              1. summary table (gpSummaryTable) which displays all calls summary
//              2. top 5 table   (acListCtrl) which displays top 5 items of the call selected in the summary table
//              3. QT items: labels, check button
// gpSummaryTable: displays all calls summary of a given type (API / GPU)
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


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
    virtual ~gpTraceSummaryWidget();

    /// Initializes the summary widget and inner tabs
    void Init(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, quint64 timelineStart, quint64 timelineRange); // bool

    /// Select a call in API Summary table
    void SelectAPIRowByCallName(const QString& callName);
    /// Select a call in GPU Summary table
    void SelectGPURowByCallName(const QString& callName);
    /// Clear selection in API Summary table
    void ClearAPISelection();
    /// Clear selection in GPU Summary table
    void ClearGPUSelection();

    void SelectCommandList(const QString& commandListName);

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
