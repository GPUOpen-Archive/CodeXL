//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file tpThreadsView.cpp
///
//==================================================================================

//------------------------------ tpThreadsView.cpp ------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acNavigationChart.h>
#include <AMDTApplicationComponents/Include/acQHTMLWindow.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afHTMLContent.h>

// Local:
#include <inc/StringConstants.h>
#include <inc/tpThreadsView.h>
#include <inc/tpTreeHandler.h>
#include <inc/tpThreadsViewLegend.h>
#include <inc/tpThreadsViewControlPanel.h>
#include <inc/tpThreadsTimeline.h>

#define TP_TIMELINE_NAVIGATION_CHART_MARGINS QMargins(0, 20, 0, 40)

tpThreadsView::tpThreadsView(QWidget* pParent, tpSessionData* pSessionData, tpSessionTreeNodeData* pSessionTreeData) :
    QWidget(pParent),
    m_pNavigationChart(nullptr),
    m_pTimeline(nullptr),
    m_pControlPanel(nullptr),
    m_pSessionData(pSessionData)
{
    QGridLayout* pMainLayout = new QGridLayout;

    m_pControlPanel = new tpThreadsViewControlPanel(nullptr, m_pSessionData);
    m_pNavigationChart = new acNavigationChart;
    m_pNavigationChart->yAxis->axisRect()->setMargins(TP_TIMELINE_NAVIGATION_CHART_MARGINS);
    m_pNavigationChart->xAxis->axisRect()->setMargins(TP_TIMELINE_NAVIGATION_CHART_MARGINS);

    m_pTimeline = new tpThreadsTimeline(nullptr, m_pSessionData, pSessionTreeData, m_pNavigationChart);
    m_pLegend = new tpThreadsViewLegend(nullptr, m_pSessionData);

    m_pNavigationChart->yAxis->setTickLabels(false);
    m_pNavigationChart->xAxis->axisRect()->setAutoMargins(QCP::msNone);

    pMainLayout->addWidget(m_pControlPanel, 0, 0, 1, 1, Qt::AlignLeft);
    pMainLayout->addWidget(new QWidget, 0, 1, 1, 1, Qt::AlignLeft);
    pMainLayout->addWidget(m_pNavigationChart, 1, 0, 1, 1);

    // Connect the control panel settings changed to the threads timeline slot:
    bool rc = connect(m_pControlPanel, SIGNAL(OnControlPanelDateChanged(const tpControlPanalData&)), m_pTimeline, SLOT(OnSettingsChanged(const tpControlPanalData&)));
    GT_ASSERT(rc);

    // Add a widget with the fixed width of a scroll bar, to make sure that the navigation chart is aligned to the timeline
    // (that has a scroll bar):
    int scrollW = qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
    QWidget* pWidget = new QWidget;
    pWidget->setFixedWidth(scrollW);
    pMainLayout->addWidget(pWidget, 1, 1, 1, 1);

    pMainLayout->addWidget(m_pTimeline, 2, 0, 1, 2);
    pMainLayout->addWidget(m_pLegend, 3, 0, 1, 2);

    setLayout(pMainLayout);

    FillNavigationChart();

    m_pTimeline->show();

}

tpThreadsView::~tpThreadsView()
{

}

void tpThreadsView::FillNavigationChart()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pNavigationChart != nullptr)
    {

    }
}


void tpThreadsView::resizeEvent(QResizeEvent* pEvent)
{
    QWidget::resizeEvent(pEvent);

    // Sanity check:
    GT_IF_WITH_ASSERT((m_pTimeline != nullptr) && (m_pNavigationChart != nullptr))
    {
        // Add a left margin to the navigation chart, so that it will have the same xAxis as the timeline:
        int timelineLeft, timelineRight;
        m_pTimeline->grid()->GetGridPosition(timelineLeft, timelineRight);

        QMargins margins = m_pNavigationChart->xAxis->axisRect()->margins();
        margins.setLeft(timelineLeft);
        m_pNavigationChart->xAxis->axisRect()->setMargins(margins);
        m_pNavigationChart->replot();
    }
}
