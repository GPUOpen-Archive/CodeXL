//=====================================================================

//=====================================================================

#include <AMDTGpuProfiling/gpTraceSummaryWidget.h>
#include <AMDTGpuProfiling/gpTraceSummaryTab.h>
#include <AMDTGpuProfiling/gpCommandListSummaryTab.h>
#include <AMDTGpuProfiling/gpStringConstants.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

const int DEFAULT_PROGRESS_FACTOR = 10000;

/////////////////////////////////////////////////////////////////////////////////////
// gpTraceSummaryWidget

gpTraceSummaryWidget::gpTraceSummaryWidget(QWidget* pParent)
    : acTabWidget(pParent), m_pTraceView(nullptr), m_useTimelineSelectionScope(false), m_timelineAbsoluteStart(0), m_timelineStart(0), m_timelineEnd(0)
{
    for (int nTab = 0; nTab < eCallType::MAX_TYPE; nTab++)
    {
        m_tabs[nTab] = nullptr;
    }
}

gpTraceSummaryWidget::~gpTraceSummaryWidget()
{
    bool rc = disconnect(this, SIGNAL(currentChanged(int)), this, SLOT(OnCurrentChanged(int)));
    for (int nTab = 0; nTab < eCallType::MAX_TYPE; nTab++)
    {
        if (m_tabs[nTab] != nullptr)
        {
            rc = disconnect(m_tabs[nTab], SIGNAL(TabUseTimelineSelectionScopeChanged(bool)), this, SLOT(OnUseTimelineSelectionScopeChanged(bool)));
            rc = disconnect(m_tabs[nTab], SIGNAL(TabSummaryItemClicked(ProfileSessionDataItem*)), this, SLOT(OnTabSummaryItemClicked(ProfileSessionDataItem*)));
            if (nTab == eCallType::COMMAND_LIST)
            {
                gpCommandListSummaryTab* pCmdListTab = dynamic_cast<gpCommandListSummaryTab*>(m_tabs[nTab]);
                if (pCmdListTab != nullptr)
                {
                    rc = disconnect(pCmdListTab, SIGNAL(TabSummaryCmdListDoubleClicked(const QString&)), this, SLOT(OnTabSummaryCmdListDoubleClicked(const QString&)));
                }
            }


            delete m_tabs[nTab];
            m_tabs[nTab] = nullptr;
        }
    }
}

void gpTraceSummaryWidget::Init(gpTraceDataContainer* pDataContainer, gpTraceView* pSessionView, quint64 timelineStartTime, quint64 timelineRange)
{
    m_pTraceView = pSessionView;

    int numItems = pDataContainer->QueueItemsCount() + pDataContainer->ThreadsCount();

    afProgressBarWrapper::instance().ShowProgressDialog(L"Loading Summary", numItems * DEFAULT_PROGRESS_FACTOR);
    afProgressBarWrapper::instance().incrementProgressBar();

    m_timelineAbsoluteStart = timelineStartTime;
    m_timelineStart = timelineStartTime;
    m_timelineEnd = m_timelineStart + timelineRange;
    int lastTabIndex = 0;

    for (int i = 0; i < eCallType::MAX_TYPE; i++)
    {
        if (i == eCallType::API_CALL )
        {
            m_tabs[i] = new gpCPUTraceSummaryTab((eCallType)i, m_timelineAbsoluteStart);
        }
        else if ( i == eCallType::GPU_CALL )
        {
            m_tabs[i] = new gpGPUTraceSummaryTab((eCallType)i, m_timelineAbsoluteStart);
        }
        else
        {
            m_tabs[i] = new gpCommandListSummaryTab((eCallType)i, m_timelineAbsoluteStart);
        }

        bool rc = m_tabs[i]->Init(pDataContainer, pSessionView, timelineStartTime, timelineRange);
        if (rc)
        {
            addTab(m_tabs[i], QIcon(), m_tabs[i]->getCaption());

            rc = connect(this, SIGNAL(currentChanged(int)), this, SLOT(OnCurrentChanged(int)));
            GT_ASSERT(rc);
            rc = connect(m_tabs[i], SIGNAL(TabUseTimelineSelectionScopeChanged(bool)), this, SLOT(OnUseTimelineSelectionScopeChanged(bool)));
            GT_ASSERT(rc);
            rc = connect(m_tabs[i], SIGNAL(TabSummaryItemClicked(ProfileSessionDataItem*)), this, SLOT(OnTabSummaryItemClicked(ProfileSessionDataItem*)));
            GT_ASSERT(rc);
            if (i == eCallType::COMMAND_LIST)
            {
                gpCommandListSummaryTab* pCmdListTab = dynamic_cast<gpCommandListSummaryTab*>(m_tabs[i]);
                GT_ASSERT(pCmdListTab != nullptr);
                rc = connect(pCmdListTab, SIGNAL(TabSummaryCmdListDoubleClicked(const QString&)), this, SLOT(OnTabSummaryCmdListDoubleClicked(const QString&)));
                GT_ASSERT(rc);
            }
            lastTabIndex = i;
        }
        else
        {
            delete m_tabs[i];
            m_tabs[i] = nullptr;
        }

        afProgressBarWrapper::instance().incrementProgressBar();
    }

    afProgressBarWrapper::instance().hideProgressBar();

    setCurrentIndex(lastTabIndex);
}

void gpTraceSummaryWidget::SelectAPIRowByCallName(const QString& callName)
{
    if (m_tabs[API_CALL] != nullptr)
    {
        gpTraceSummaryTable* pSummaryTable = dynamic_cast<gpTraceSummaryTable*>(m_tabs[API_CALL]->m_pSummaryTable);
        GT_ASSERT(pSummaryTable != nullptr);
        pSummaryTable->SelectRowByCallName(callName);
    }
}

void gpTraceSummaryWidget::SelectGPURowByCallName(const QString& callName)
{
    if (m_tabs[GPU_CALL] != nullptr)
    {
        gpTraceSummaryTable* pSummaryTable = dynamic_cast<gpTraceSummaryTable*>(m_tabs[GPU_CALL]->m_pSummaryTable);
        GT_ASSERT(pSummaryTable != nullptr);
        pSummaryTable->SelectRowByCallName(callName);
    }
}

void gpTraceSummaryWidget::ClearAPISelection()
{
    if (m_tabs[API_CALL] != nullptr)
    {
        m_tabs[API_CALL]->m_pSummaryTable->clearSelection();
    }
}

void gpTraceSummaryWidget::ClearGPUSelection()
{
    if (m_tabs[GPU_CALL] != nullptr)
    {
        m_tabs[GPU_CALL]->m_pSummaryTable->clearSelection();
    }
}

void gpTraceSummaryWidget::OnTimelineChanged(const QPointF& rangePoint, bool isRelativeRangeStartTime)
{
    if (isRelativeRangeStartTime)
    {
        m_timelineStart = m_timelineAbsoluteStart + rangePoint.x() ;
        m_timelineEnd = m_timelineAbsoluteStart + rangePoint.y();
    }
    else
    {
        m_timelineStart = rangePoint.x();
        m_timelineEnd = m_timelineStart + rangePoint.y();
    }

    for (auto tab : m_tabs)
    {
        if (tab != nullptr)
        {
            tab->OnTimelineChanged(m_timelineStart, m_timelineEnd);
        }
    }

    if (m_useTimelineSelectionScope == true)
    {
        gpSummaryTab* pCurrentTab = qobject_cast<gpSummaryTab*>(currentWidget());
        if (pCurrentTab != nullptr )
        {
            pCurrentTab->RefreshAndMaintainSelection(true);
        }
    }
}
void gpTraceSummaryWidget::OnTimelineFilterChanged(QMap<QString, bool>& threadNameVisibilityMap)
{
    GT_UNREFERENCED_PARAMETER(threadNameVisibilityMap);
    gpSummaryTab* pCurrentTab = qobject_cast<gpSummaryTab*>(currentWidget());
    if (pCurrentTab != nullptr )
    {
        pCurrentTab->OnTimelineFilterChanged();
    }
}
void gpTraceSummaryWidget::OnUseTimelineSelectionScopeChanged(bool check)
{
    m_useTimelineSelectionScope = check;
}

void gpTraceSummaryWidget::OnFind()
{
    // Get the current tab and find the requested text in it
    gpSummaryTab* pCurrentTab = qobject_cast<gpSummaryTab*>(currentWidget());

    if (pCurrentTab != nullptr)
    {
        pCurrentTab->OnFind();
    }
}

void gpTraceSummaryWidget::OnEditSelectAll()
{
    // Get the current tab and apply the select all command on it
    gpSummaryTab* pCurrentTab = qobject_cast<gpSummaryTab*>(currentWidget());

    if (pCurrentTab != nullptr)
    {
        pCurrentTab->OnEditSelectAll();
    }
}

void gpTraceSummaryWidget::OnEditCopy()
{
    // Get the current tab and apply the copy command on it
    gpSummaryTab* pCurrentTab = qobject_cast<gpSummaryTab*>(currentWidget());

    if (pCurrentTab != nullptr)
    {
        pCurrentTab->OnEditCopy();
    }
}

void gpTraceSummaryWidget::OnCurrentChanged(int activeTabIndex)
{
    GT_UNREFERENCED_PARAMETER(activeTabIndex);
    gpSummaryTab* pCurrentTab = qobject_cast<gpSummaryTab*>(currentWidget());
    if (pCurrentTab != nullptr)
    {
        pCurrentTab->SetTimelineScope(m_useTimelineSelectionScope, m_timelineStart, m_timelineEnd);
        pCurrentTab->RefreshAndMaintainSelection(m_useTimelineSelectionScope);
    }
}

void gpTraceSummaryWidget::OnTabSummaryItemClicked(ProfileSessionDataItem* pItem)
{
    emit SummaryItemClicked(pItem);
}

void gpTraceSummaryWidget::SelectCommandList(const QString& commandListName)
{
    gpCommandListSummaryTab* pCurrentTab = qobject_cast<gpCommandListSummaryTab*>(currentWidget());
    if (pCurrentTab != nullptr)
    {
        pCurrentTab->SelectCommandList(commandListName);
    }
}

void gpTraceSummaryWidget::OnTabSummaryCmdListDoubleClicked(const QString& cmdList)
{
    emit SummaryCmdListDoubleClicked(cmdList);
}