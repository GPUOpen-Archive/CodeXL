
//=====================================================================

//=====================================================================

#include <AMDTGpuProfiling/gpSummaryTable.h>
#include <AMDTGpuProfiling/gpTraceSummaryWidget.h>
#include <AMDTGpuProfiling/gpTraceDataContainer.h>
#include <AMDTGpuProfiling/APIColorMap.h>
#include <AMDTGpuProfiling/gpTraceView.h>

#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineItem.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineBranch.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>


void gpSummaryTable::Refresh(bool useTimelineScope, quint64 min, quint64 max)
{
    RebuildSummaryMap(useTimelineScope, min, max);
    FillTable();
}

gpSummaryTable::gpSummaryTable(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, quint64 timelineAbsoluteStart)
    :acListCtrl(nullptr), m_pSessionDataContainer(pDataContainer), m_pTraceView(pSessionView), m_lastSelectedRowIndex(-1), m_timelineAbsoluteStart(timelineAbsoluteStart)
{
}

gpSummaryTable::~gpSummaryTable()
{

}

void gpSummaryTable::RebuildSummaryMap(bool useScope, quint64 startTime, quint64 endTime)
{
    // at this point m_apiItemsMap is already filled with all items,
    // we only need to collate them into
    if (useScope)
    {
        // m_allCallItemsMultiMap already contains all items, we just need to insert items within mintime and maxtime into summary map
        BuildSummaryMapInTimelineScope(startTime, endTime);
    }
    else
    {
        // m_allCallItemsMultiMap already contains all items, we just need to collate m_allCallItemsMap into summary map
        CollateAllItemsIntoSummaryMap();
    }
}

void gpSummaryTable::SaveSelection(int row)
{
    m_lastSelectedRowIndex = row;
}

void gpSummaryTable::RestoreSelection()
{
    if (m_lastSelectedRowIndex != -1)
    {
        selectRow(m_lastSelectedRowIndex);
    }
}

void gpSummaryTable::ClearSelection()
{
    clearSelection();
    m_lastSelectedRowIndex = -1;
}

bool gpSummaryTable::Init()
{
    bool rc = InitItems();
    if (rc == true)
    {
        // fill Table widget
        FillTable();
        setSortingEnabled(true);
        setSelectionMode(QAbstractItemView::SingleSelection);
        setContextMenuPolicy(Qt::NoContextMenu);

        // Connect to the cell entered signal
        setMouseTracking(true);
        rc = connect(this, SIGNAL(cellEntered(int, int)), this, SLOT(OnCellEntered(int, int)));
        GT_ASSERT(rc);
    }
    return rc;
}