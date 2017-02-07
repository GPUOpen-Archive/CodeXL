//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppSummaryView.cpp
///
//==================================================================================

//------------------------------ ppSummaryView.cpp ------------------------------

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <AMDTPowerProfiling/src/ppSummaryView.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acCustomPlot.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// AMDTSharedProfiling:
#include <AMDTSharedProfiling/inc/StringConstants.h>

// Local:
#include <AMDTPowerProfiling/Include/ppAppWrapper.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>
#include <AMDTPowerProfiling/src/ppColoredBarGraphData.h>
#include <AMDTPowerProfiling/src/ppStackedBarGraphData.h>
#include <AMDTPowerProfiling/src/ppSummaryView.h>
#include <AMDTPowerProfiling/src/ppSessionController.h>
#include <AMDTPowerProfiling/src/ppSessionView.h>
#include <AMDTPowerProfiling/src/ppAidFunctions.h>

// PP_TO_DO - decide the size of graph
#define DEFAULT_BAR_GAP 10
#define DEFAULT_NUM_OF_BARS_POWER 8
#define DEFAULT_NUM_OF_BARS_FREQ 3
#define DEFAULT_POWER_GRAPH_YAxis_RANGE 20
#define DEFAULT_POWER_GRAPH_YAxis_TICKS 5

// Frequency graph init
#define YAXIS_UPPER_BOUND 5
#define YAXIS_TICK_STEP 1

// Power Profile initial values
#define DEFAULT_UNINITIALIZED_TIME_INTERVAL 99999

#define SECONDS_IN_MINUTE 60
#define MILLISECONDS_IN_SECOND 1000
#define MILLISECONDS_IN_MINUTE (MILLISECONDS_IN_SECOND * SECONDS_IN_MINUTE)     // 1000 * 60
#define MILLISECONDS_IN_HOUR  (MILLISECONDS_IN_MINUTE * SECONDS_IN_MINUTE)      // 1000 * 60 * 60


#define FREQUENCY_GRAPH_BARS_VALUE_TYPE ppDataUtils::STACKEDBARGRAPH_VALUETYPE_MILLISECONDS

#define PP_SUMMARY_PLOT_MIN_HEIGHT 350
#define PP_SUMMARY_PLOT_MIN_WIDTH 1200

#define PP_SUMMARY_PLOT_EXTRA_HEIGHT 100

// ---------------------------------------------------------------------------
ppSummaryView::ppSummaryView(ppSessionView* pParentSession, ppSessionController* pSessionController) :
    acSplitter(Qt::Vertical, pParentSession),
    m_pHTMLInfoWindow(nullptr),
    m_pPowerGraph(nullptr),
    m_pEnergyGraph(nullptr),
    m_pFreqCpuGraph(nullptr),
    m_pFreqGpuGraph(nullptr),
    m_isGraphsDataInitialized(false),
    m_pTopRightHorizontalSplitter(nullptr),
    m_pTopLeftHorizontalSplitter(nullptr),
    m_lastEventPowerOtherCounterData(0),
    m_lastEventAverageOtherCounterData(0),
    m_pSessionController(pSessionController),
    m_samplingInterval(DEFAULT_UNINITIALIZED_TIME_INTERVAL),
    m_pParentSession(pParentSession),
    m_currentPowerMeasurementUnit(MEASUREMENTUNIT_JOULES),
    m_apuCounterLastAverageValue(0),
    m_apuCounterLastCumulativeValue(0)
{
    // Set a white background:
    QPalette p = palette();
    p.setColor(backgroundRole(), Qt::white);
    p.setColor(QPalette::Base, Qt::white);
    setAutoFillBackground(true);
    setPalette(p);

    setStyleSheet("background-color:white");

    // Set 0 margins:
    setContentsMargins(0, 0, 0, 0);

    ppAppController& appController = ppAppController::instance();

    // Do not allow the collapse of top or bottom sections:
    setChildrenCollapsible(false);

    // get session state
    ppSessionController::SessionState currentState = ppSessionController::PP_SESSION_STATE_NEW;
    GT_IF_WITH_ASSERT(m_pSessionController != nullptr)
    {
        // Get the state from the controller:
        currentState = m_pSessionController->GetSessionState();
    }

    // init graphs
    InitGraphsArea(GetCpuFamily());

    // Create the HTML window for the summary information:
    m_pHTMLInfoWindow = new acQHTMLWindow(nullptr);
    // Excluding extra information section as it is redundant
    //addWidget(m_pHTMLInfoWindow);

    int samplingDuration = 0;

    if (currentState != ppSessionController::PP_SESSION_STATE_COMPLETED)
    {
        // don't connect to OnNewProfileData on exsiting session loaded
        m_OnNewProfileDataConnected = connect(&appController, SIGNAL(NewPowerProfileData(ppQtEventData)), this, SLOT(OnNewProfileData(ppQtEventData)));
    }
    else
    {
        // if got here from play button and not from create new event
        SetGraphsData(false);

        SamplingTimeRange samplingRange(0, 0);
        m_pSessionController->GetSessionTimeRange(samplingRange);

        samplingDuration = samplingRange.m_toTime - samplingRange.m_fromTime;
    }

    // Set the duration:
    SetDurationLabel(samplingDuration);

    // Initialize the session information HTML:
    UpdateSessionInformation();

    // connect to selected counters changed
    bool rc = connect(&(ppAppController::instance()), SIGNAL(CountersSelectionModified()), this, SLOT(OnSelectedCountersChanged()));
    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
ppSummaryView::~ppSummaryView()
{

    m_lastEventPowerGrpahData.clear();
    m_lastEventAverageData.clear();
    m_lastAggregatedQuantized.clear();
    m_lastCPUBucketsPerCounter.clear();
    m_lastGPUBucketsPerCounter.clear();
    m_selectedCunterIdsPerGraphMap.clear();
}

typename ppSummaryView::CpuFamily ppSummaryView::GetCpuFamily() const
{
    AMDTProfileSessionInfo info;
    m_pSessionController->GetProfilerBL().GetSessionInfo(info);
    return info.m_cpuFamily >= 0x17 ? CPU_FAMILY_17 : CPU_FAMILY_OTHERS;
}

// ---------------------------------------------------------------------------
void ppSummaryView::OnNewProfileData(ppQtEventData pSampledDataPerCounter)
{

    if (m_OnNewProfileDataConnected && pSampledDataPerCounter != nullptr && pSampledDataPerCounter->size() > 0)
    {

        // Bucket width for online calculation.
        const unsigned CPU_HISTOGRAM_ONLINE_BUCKET_WIDTH = 350;
        const unsigned GPU_HISTOGRAM_ONLINE_BUCKET_WIDTH = 150;

        // update upper duration label
        gtMap<int, PPSampledValuesBatch> eventData = *(pSampledDataPerCounter.data());
        gtMap<int, PPSampledValuesBatch>::iterator it = eventData.begin();
        int miliRange = (*it).second.m_quantizedTime;
        SetDurationLabel(miliRange);

        // in first time - update sampling interval from controller
        if (m_samplingInterval == DEFAULT_UNINITIALIZED_TIME_INTERVAL)
        {
            if (PPR_NO_ERROR != ppAppController::instance().GetMiddleTierController().GetCurrentSamplingIntervalMS(m_samplingInterval))
            {
                m_samplingInterval = DEFAULT_UNINITIALIZED_TIME_INTERVAL;
            }
        }

        // update event last data member with the new event data
        ppDataUtils::GeSummarytPowerGraphDataFromEvent(pSampledDataPerCounter,
                                                       m_lastEventPowerGrpahData,
                                                       m_lastEventPowerOtherCounterData,
                                                       m_lastEventAverageData,
                                                       m_lastEventAverageOtherCounterData,
                                                       m_lastAggregatedQuantized,
                                                       m_samplingInterval,
                                                       m_pSessionController);

        // get icounter ids for freq CPU graph
        gtVector<int> counterIds;
        bool isGetCountersOk = GetCounterIdsByGraphType(ppDataUtils::SUMMARY_FREQUENCY_CPU, counterIds);

        // if not empty - get data
        if (isGetCountersOk && counterIds.size() > 0)
        {
            ppDataUtils::GetSummaryFrequencyDataFromEvent(counterIds,
                                                          CPU_HISTOGRAM_ONLINE_BUCKET_WIDTH, m_samplingInterval,
                                                          m_lastCPUBucketsPerCounter,
                                                          m_pSessionController,
                                                          pSampledDataPerCounter);
        }

        // get counter ids for freq GPU graph
        counterIds.clear();
        isGetCountersOk = GetCounterIdsByGraphType(ppDataUtils::SUMMARY_FREQUENCY_GPU, counterIds);

        // if not empty - get data
        if (isGetCountersOk && counterIds.size() > 0)
        {
            ppDataUtils::GetSummaryFrequencyDataFromEvent(counterIds,
                                                          GPU_HISTOGRAM_ONLINE_BUCKET_WIDTH, m_samplingInterval,
                                                          m_lastGPUBucketsPerCounter,
                                                          m_pSessionController,
                                                          pSampledDataPerCounter);
        }

        // in first time call to SetGraphsData. else call to updateGrpahsData
        if (!m_isGraphsDataInitialized)
        {
            SetGraphsData(true);
        }
        else
        {
            UpdateGraphs();
        }
    }
}

// ---------------------------------------------------------------------------
void ppSummaryView::OnProfileStopped(const QString& sessionName)
{
    GT_UNREFERENCED_PARAMETER(sessionName);

    ppAppController& appController = ppAppController::instance();

    disconnect(&appController, SIGNAL(NewPowerProfileData(ppQtEventData)), this, SLOT(OnNewProfileData(ppQtEventData)));
    m_OnNewProfileDataConnected = false;

    // Update the graphs with the most up-to-date data.
    InitGraphs();

    // Reset the value of the 'Other' data before going to the DB.
    m_lastEventPowerOtherCounterData = 0.0;

    // Set the graphs data.
    SetGraphsData(false);
    SamplingTimeRange samplingRange(0, 0);
    GT_IF_WITH_ASSERT(m_pSessionController != nullptr)
    {
        m_pSessionController->GetSessionTimeRange(samplingRange);
    }

    int samplingDuration = samplingRange.m_toTime - samplingRange.m_fromTime;
    // Set the duration:
    SetDurationLabel(samplingDuration);
    // Update the session information window with the profile details:
    UpdateSessionInformation();
    ReplotAllGrpahs();
}

void ppSummaryView::SetupGraphs()
{
    m_pFreqCpuGraph = new acGroupedBarsGraph();
    m_pFreqGpuGraph = new acGroupedBarsGraph();

    m_pFreqCpuGraph->SetGraphTitle(PP_STR_SummaryCPUFrequencyCaption);
    m_pFreqGpuGraph->SetGraphTitle(PP_STR_SummaryGPUFrequencyCaption);

    m_pFreqCpuGraph->GetPlot()->setMinimumSize(QSize(PP_SUMMARY_PLOT_MIN_WIDTH, PP_SUMMARY_PLOT_MIN_HEIGHT + PP_SUMMARY_PLOT_EXTRA_HEIGHT));
    m_pFreqGpuGraph->GetPlot()->setMinimumSize(QSize(PP_SUMMARY_PLOT_MIN_WIDTH, PP_SUMMARY_PLOT_MIN_HEIGHT + PP_SUMMARY_PLOT_EXTRA_HEIGHT));

    // Set the graphs background color:
    QColor graphsBGColor = Qt::black;
    graphsBGColor.setAlpha(7);

    m_pFreqCpuGraph->GetPlot()->xAxis->axisRect()->setBackground(graphsBGColor);
    m_pFreqGpuGraph->GetPlot()->xAxis->axisRect()->setBackground(graphsBGColor);

    // Add zooming and dragging functionality
    m_pFreqCpuGraph->GetPlot()->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_pFreqGpuGraph->GetPlot()->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    m_pFreqGpuGraph->GetPlot()->axisRect()->setRangeDragAxes(m_pFreqGpuGraph->GetPlot()->xAxis, 0);
    m_pFreqGpuGraph->GetPlot()->axisRect()->setRangeZoomAxes(m_pFreqGpuGraph->GetPlot()->xAxis, 0);
    m_pFreqCpuGraph->GetPlot()->axisRect()->setRangeDragAxes(m_pFreqCpuGraph->GetPlot()->xAxis, 0);
    m_pFreqCpuGraph->GetPlot()->axisRect()->setRangeZoomAxes(m_pFreqCpuGraph->GetPlot()->xAxis, 0);

    // Create the graphs:
    m_pPowerGraph = new acColoredBarsGraph();
    m_pEnergyGraph = new acColoredBarsGraph();

    m_pPowerGraph->SetGraphTitle(PP_STR_SummaryAveragePowerCaption);
    m_pEnergyGraph->SetGraphTitle(PP_STR_SummaryCumulativeEnergyCaption);

    // Set the minimum height for the graphs:
    m_pPowerGraph->GetPlot()->setMinimumSize(QSize(PP_SUMMARY_PLOT_MIN_WIDTH, PP_SUMMARY_PLOT_MIN_HEIGHT));
    m_pEnergyGraph->GetPlot()->setMinimumSize(QSize(PP_SUMMARY_PLOT_MIN_WIDTH, PP_SUMMARY_PLOT_MIN_HEIGHT));

    m_pPowerGraph->GetPlot()->xAxis->axisRect()->setBackground(graphsBGColor);
    m_pEnergyGraph->GetPlot()->xAxis->axisRect()->setBackground(graphsBGColor);
}

// ---------------------------------------------------------------------------
void ppSummaryView::InitGraphsAreaForOthers()
{
    // Create a scroll area for the top layer of the view:
    QScrollArea* pScrollArea = new QScrollArea;

    // Set the scrollbar area policy:
    pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    QWidget* pScrollAreaWidget = new QWidget;
    pScrollArea->setWidget(pScrollAreaWidget);

    // Create vertical layout for the scroll area:
    QVBoxLayout* pVScrollingAreaLayout = new QVBoxLayout;
    pScrollAreaWidget->setLayout(pVScrollingAreaLayout);

    m_pSessionDurationLabel = new QLabel(PP_STR_SummaryDurationLabel);
    pVScrollingAreaLayout->addWidget(m_pSessionDurationLabel, 0, Qt::AlignLeft);

    SetupGraphs();

    acSplitter* pTopHorizontalSplitter = new acSplitter(Qt::Horizontal);

    m_pTopLeftHorizontalSplitter = new acSplitter(Qt::Vertical);
    m_pTopRightHorizontalSplitter = new acSplitter(Qt::Vertical);

    // Connect the left splitter move signal (synchronize with the right one):
    bool rc = connect(m_pTopLeftHorizontalSplitter, SIGNAL(splitterMoved(int, int)), this, SLOT(OnTopSplitterMoved(int, int)));
    GT_ASSERT(rc);

    // Connect the right splitter move signal (synchronize with the left one):
    rc = connect(m_pTopRightHorizontalSplitter, SIGNAL(splitterMoved(int, int)), this, SLOT(OnTopSplitterMoved(int, int)));
    GT_ASSERT(rc);

    QList<int> splitterSizes;
    splitterSizes << 1;
    splitterSizes << 1;

    m_pTopLeftHorizontalSplitter->addWidget(m_pEnergyGraph->GetPlot());
    m_pTopRightHorizontalSplitter->addWidget(m_pPowerGraph->GetPlot());

    m_pTopLeftHorizontalSplitter->addWidget(m_pFreqCpuGraph->GetPlot());

    m_pTopLeftHorizontalSplitter->setCollapsible(0, false);
    m_pTopLeftHorizontalSplitter->setCollapsible(1, false);
    m_pTopLeftHorizontalSplitter->setStretchFactor(0, 1);
    m_pTopLeftHorizontalSplitter->setStretchFactor(1, 1);
    m_pTopLeftHorizontalSplitter->setSizes(splitterSizes);

    m_pTopRightHorizontalSplitter->addWidget(m_pFreqGpuGraph->GetPlot());

    m_pTopRightHorizontalSplitter->setCollapsible(0, false);
    m_pTopRightHorizontalSplitter->setCollapsible(1, false);
    m_pTopRightHorizontalSplitter->setStretchFactor(0, 1);
    m_pTopRightHorizontalSplitter->setStretchFactor(1, 1);
    m_pTopRightHorizontalSplitter->setSizes(splitterSizes);

    pTopHorizontalSplitter->addWidget(m_pTopLeftHorizontalSplitter);
    pTopHorizontalSplitter->addWidget(m_pTopRightHorizontalSplitter);

    pTopHorizontalSplitter->setCollapsible(0, false);
    pTopHorizontalSplitter->setCollapsible(1, false);

    pVScrollingAreaLayout->addWidget(pTopHorizontalSplitter, 1);

    addWidget(pScrollAreaWidget);

    InitGraphs();

}

void ppSummaryView::AddGraphVertically(QCustomPlot* plot, GraphContainer container, QWidget* parent)
{
    QWidget* widget = m_pGraphContainers[container];
    widget->setLayout(new QVBoxLayout);
    widget->layout()->addWidget(plot);
    if (container == CONTAINER_PP_FREQ_CPU_GRAPH ||
        container == CONTAINER_PP_FREQ_GPU_GRAPH)
    {
        widget->setMinimumSize(PP_SUMMARY_PLOT_MIN_WIDTH, PP_SUMMARY_PLOT_MIN_HEIGHT + PP_SUMMARY_PLOT_EXTRA_HEIGHT);
    }
    else
    {
        widget->setMinimumSize(PP_SUMMARY_PLOT_MIN_WIDTH, PP_SUMMARY_PLOT_MIN_HEIGHT);
    }
    parent->layout()->addWidget(widget);
}

// ---------------------------------------------------------------------------
void ppSummaryView::InitGraphsAreaForFamily17()
{
    // Create a scroll area for the top layer of the view
    setLayout(new QVBoxLayout);
    QWidget* pScrollAreaWidget = new QWidget(this);
    QScrollArea* pScrollArea = new QScrollArea;
    pScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    // Create vertical layout for the scroll area:
    QVBoxLayout* pVScrollingAreaLayout = new QVBoxLayout(pScrollAreaWidget);
    pScrollAreaWidget->setLayout(pVScrollingAreaLayout);
    pScrollArea->setWidget(pScrollAreaWidget);
    pScrollArea->setWidgetResizable(true);

    SetupGraphs();
    m_pEnergyGraph->DecreaseXAxisTickLabelFont(1);
    m_pPowerGraph->DecreaseXAxisTickLabelFont(1);

    m_pSessionDurationLabel = new QLabel(PP_STR_SummaryDurationLabel);
    pVScrollingAreaLayout->addWidget(m_pSessionDurationLabel, 0, Qt::AlignLeft);

    for (int i = 0; i < TOTAL_CONTAINER_PP; ++i)
        m_pGraphContainers[i] = new QWidget(this);

    AddGraphVertically(m_pEnergyGraph->GetPlot(), CONTAINER_PP_ENERGY_GRAPH, pScrollAreaWidget);
    AddGraphVertically(m_pPowerGraph->GetPlot(), CONTAINER_PP_POWER_GRAPH, pScrollAreaWidget);
    AddGraphVertically(m_pFreqCpuGraph->GetPlot(), CONTAINER_PP_FREQ_CPU_GRAPH, pScrollAreaWidget);
    AddGraphVertically(m_pFreqGpuGraph->GetPlot(), CONTAINER_PP_FREQ_GPU_GRAPH, pScrollAreaWidget);
    
    addWidget(pScrollArea);

    InitGraphs();

}

void ppSummaryView::InitGraphsArea(CpuFamily family)
{
    switch (family)
    {
    case CPU_FAMILY_17: 
        InitGraphsAreaForFamily17();
        break;
    case CPU_FAMILY_OTHERS: 
        InitGraphsAreaForOthers();
        break;
    default:
        InitGraphsAreaForOthers();
        break;
    }
}

void ppSummaryView::InitGraphs()
{
    // get current counter ids
    SetSelectedCounterIdsPerGraphMap();

    // init graphs
    InitPowerGraphs();
    InitCpuGraph();
    InitGpuGraph();
}

void ppSummaryView::SetGraphsData(bool isDataFromEvent)
{
    m_isGraphsDataInitialized = true;

    // set graphs with new data
    SetPowerOrEnergyGraphData(isDataFromEvent, ppDataUtils::POWERGRAPHTYPE_CUMULATIVE);
    SetPowerOrEnergyGraphData(isDataFromEvent, ppDataUtils::POWERGRAPHTYPE_AVERAGE);

    // Set the graphs captions:
    SetPowerGraphsCaption();
    m_pEnergyGraph->Replot();
    m_pPowerGraph->Replot();

    SetFreqGraphData(isDataFromEvent, ppDataUtils::SUMMARY_FREQUENCY_CPU);
    SetFreqGraphData(isDataFromEvent, ppDataUtils::SUMMARY_FREQUENCY_GPU);
}

void ppSummaryView::UpdateGraphs()
{
    // update is called always from event (OnNewProfileData)

    // update Power graph
    UpdatePowerGraphData();

    // update CPU graph
    UpdateStackedBarGraphData(m_pFreqCpuGraph, ppDataUtils::SUMMARY_FREQUENCY_CPU);

    // update GPU graph
    UpdateStackedBarGraphData(m_pFreqGpuGraph, ppDataUtils::SUMMARY_FREQUENCY_GPU);
}

// ---------------------------- Power graph ---------------------------------

void ppSummaryView::InitPowerGraphs()
{
    bool shouldDisplayPowerGraphs = false;
    GT_IF_WITH_ASSERT(m_pSessionController != nullptr)
    {
        // Check if there are power counters, and display the power charts only in this case:
        const gtVector<AMDTDeviceType> deviceTypes;
        gtVector<int> countersVector;
        m_pSessionController->GetEnabledCountersByTypeAndCategory(deviceTypes, AMDT_PWR_CATEGORY_POWER, countersVector);
        shouldDisplayPowerGraphs = (countersVector.size() > 0);

        if (shouldDisplayPowerGraphs)
        {
            // Init power graph:
            QString strUnit = GetPowerGraphUnit(ppDataUtils::POWERGRAPHTYPE_CUMULATIVE);
            QString powerTitle = QString(PP_STR_SummaryViewEnergyCaptionWithUnits).arg(strUnit);
            m_pEnergyGraph->Init((DEFAULT_NUM_OF_BARS_POWER + 1) * DEFAULT_BAR_GAP, 0,
                                 DEFAULT_POWER_GRAPH_YAxis_RANGE, DEFAULT_POWER_GRAPH_YAxis_RANGE / DEFAULT_POWER_GRAPH_YAxis_TICKS,
                                 AF_STR_EmptyA, powerTitle, true);

            // Init energy graph:
            strUnit = GetPowerGraphUnit(ppDataUtils::POWERGRAPHTYPE_AVERAGE);
            QString energyTitle = QString(PP_STR_SummaryViewPowerCaptionWithUnits).arg(strUnit);
            m_pPowerGraph->Init((DEFAULT_NUM_OF_BARS_POWER + 1) * DEFAULT_BAR_GAP, 0,
                                DEFAULT_POWER_GRAPH_YAxis_RANGE, DEFAULT_POWER_GRAPH_YAxis_RANGE / DEFAULT_POWER_GRAPH_YAxis_TICKS,
                                AF_STR_EmptyA, energyTitle, true);
        }

        if (GetCpuFamily() == CPU_FAMILY_17)
        {
            m_pGraphContainers[CONTAINER_PP_ENERGY_GRAPH]->setVisible(shouldDisplayPowerGraphs);
            m_pGraphContainers[CONTAINER_PP_POWER_GRAPH]->setVisible(shouldDisplayPowerGraphs);
        }
        m_pPowerGraph->GetPlot()->setVisible(shouldDisplayPowerGraphs);
        m_pEnergyGraph->GetPlot()->setVisible(shouldDisplayPowerGraphs);
    }
}

void ppSummaryView::SetPowerOrEnergyGraphData(bool isDataFromEvent, const ppDataUtils::PowerGraphType sampledGraphType)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pPowerGraph != nullptr) && (m_pEnergyGraph != nullptr))
    {
        QVector<acBarGraphData*> counterDataVec;

        // get ids from counters map
        gtVector<int> counterIds;
        bool isGetCountersOk = GetCounterIdsByGraphType(ppDataUtils::SUMMARY_POWER, counterIds);

        // if there is counter ids related to Power graph
        if (isGetCountersOk && counterIds.size() > 0)
        {
            double totalValue = 0;

            if (GetCpuFamily() == CPU_FAMILY_17)
            {
                if (counterIds.size() > 10)
                {
                    int ratio = PP_SUMMARY_PLOT_MIN_WIDTH / 10;
                    if (sampledGraphType == ppDataUtils::POWERGRAPHTYPE_CUMULATIVE)
                        m_pGraphContainers[CONTAINER_PP_ENERGY_GRAPH]->setMinimumWidth(PP_SUMMARY_PLOT_MIN_WIDTH + (ratio*(counterIds.size()-10)));
                    else
                        m_pGraphContainers[CONTAINER_PP_POWER_GRAPH]->setMinimumWidth(PP_SUMMARY_PLOT_MIN_WIDTH + (ratio*(counterIds.size() - 10)));
                }
            }

            if (isDataFromEvent)
            {
                // Get the data from event:
                gtMap<int, double>& lastEventDataMap = (sampledGraphType == ppDataUtils::POWERGRAPHTYPE_CUMULATIVE) ? m_lastEventPowerGrpahData : m_lastEventAverageData;
                double& lastEventOtherCounter = (sampledGraphType == ppDataUtils::POWERGRAPHTYPE_CUMULATIVE) ? m_lastEventPowerOtherCounterData : m_lastEventAverageOtherCounterData;
                totalValue = ppDataUtils::GetSummaryPowerGraphInitializationDataFromEvent(counterDataVec, lastEventDataMap, lastEventOtherCounter, counterIds, m_pSessionController);
            }
            else
            {
                // if getting data for existing session - get data from DB
                totalValue = ppDataUtils::GetSummaryPowerGraphInitializationDataFromDB(counterDataVec, m_lastEventPowerOtherCounterData, sampledGraphType, m_pSessionController, counterIds);
            }

            // Create graph bars from data collected:
            bool allowReplot = true;

            if (sampledGraphType == ppDataUtils::POWERGRAPHTYPE_CUMULATIVE)
            {
                // update measurement unit only for cumulative graph. the energy graph is always in watt units.
                UpdateEnergyGraphMeasurementUnit(counterDataVec);

                m_pEnergyGraph->SetData(counterDataVec, allowReplot);
                m_apuCounterLastCumulativeValue = totalValue;
            }
            else
            {
                m_pPowerGraph->SetData(counterDataVec, allowReplot);
                m_apuCounterLastAverageValue = totalValue;
            }

            for (int i = 0; i < counterDataVec.count(); i++)
            {
                delete counterDataVec[i];
            }
        }
    }
}

void ppSummaryView::UpdatePowerGraphData()
{
    GT_IF_WITH_ASSERT((nullptr != m_pPowerGraph) && (m_pEnergyGraph != nullptr))
    {
        // get counter ids
        gtVector<int> graphIds;
        bool isGetCountersOk = GetCounterIdsByGraphType(ppDataUtils::SUMMARY_POWER, graphIds);

        if (isGetCountersOk && graphIds.size() > 0)
        {
            // Get data for the existing graph:
            QVector<acSingleGraphData*> data1;
            m_apuCounterLastAverageValue = ppDataUtils::GetSummaryPowerGraphNewData(data1, m_pPowerGraph->XLabels(), m_lastEventAverageData, m_lastEventAverageOtherCounterData, graphIds, m_pSessionController);

            // Update the graph data1
            bool allowReplot = true;
            m_pPowerGraph->UpdateData(data1, allowReplot);

            // Clear the data1 and collect it for the cumulative energy graph:
            data1.clear();
            m_apuCounterLastCumulativeValue = ppDataUtils::GetSummaryPowerGraphNewData(data1, m_pEnergyGraph->XLabels(), m_lastEventPowerGrpahData, m_lastEventPowerOtherCounterData, graphIds, m_pSessionController);

            // Update the energy graph measurement units, and fix the data1 accordingly:
            UpdateEnergyGraphMeasurementUnit(data1);

            // Update the graph data1
            m_pEnergyGraph->UpdateData(data1, allowReplot);

            // delete data1
            for (int i = 0; i < data1.count(); i++)
            {
                delete data1[i];
            }

            // Set the graphs captions:
            SetPowerGraphsCaption();

        }
    }
}

// ----------------------------- CPU graph ----------------------------------

void ppSummaryView::InitCpuGraph()
{
    bool allowReplot = true;
    bool isLegendRequired = true;

    gtVector<int> counterIds;
    bool isGetCountersOk = GetCounterIdsByGraphType(ppDataUtils::SUMMARY_FREQUENCY_CPU, counterIds);

    // dont init graph if has no relevant counters
    if (isGetCountersOk && counterIds.size() > 0)
    {
        m_pFreqCpuGraph->GetPlot()->setVisible(true);

        m_pFreqCpuGraph->Init((DEFAULT_NUM_OF_BARS_FREQ + 1) * DEFAULT_BAR_GAP, 0,
                              YAXIS_UPPER_BOUND, YAXIS_TICK_STEP,
                              AF_STR_EmptyA, PP_STR_SummaryCPUFrequencyYAxisLabel,
                              allowReplot,
                              isLegendRequired);

        // Frequency should be treated as integer:
        m_pFreqCpuGraph->GetPlot()->xAxis->setNumberPrecision(0);

        m_pFreqCpuGraph->SetXAxisTickLabelRotation(20);
        //to make the labels smaller uncomment this row - waiting for size approval - m_pFreqCpuGraph->DecreaseXAxisTickLabelFont(2);
        m_pFreqCpuGraph->SetXAxisTickLabelPadding(1);
    }
    else
    {
        m_pFreqCpuGraph->GetPlot()->setVisible(false);
        if (GetCpuFamily() == CPU_FAMILY_17)
            m_pGraphContainers[CONTAINER_PP_FREQ_CPU_GRAPH]->setVisible(false);
    }
}

// ----------------------------- GPU graph ---------------------------------

void ppSummaryView::InitGpuGraph()
{
    bool allowReplot = true;
    bool isLegendRequired = true;

    gtVector<int> counterIds;
    bool isGetCountersOk = GetCounterIdsByGraphType(ppDataUtils::SUMMARY_FREQUENCY_GPU, counterIds);

    // dont init graph if has no relebant counters
    if (isGetCountersOk && counterIds.size() > 0)
    {
        m_pFreqGpuGraph->GetPlot()->setVisible(true);

        m_pFreqGpuGraph->Init((DEFAULT_NUM_OF_BARS_POWER + 1) * DEFAULT_BAR_GAP, 0,
                              YAXIS_UPPER_BOUND, YAXIS_TICK_STEP,
                              AF_STR_EmptyA, PP_STR_SummaryCPUFrequencyYAxisLabel,
                              allowReplot,
                              isLegendRequired);

        // Frequency should be treated as integer:
        m_pFreqGpuGraph->GetPlot()->xAxis->setNumberPrecision(0);

        m_pFreqGpuGraph->SetXAxisTickLabelRotation(20);
        //to make the labels smaller uncomment this row - waiting for size approval - m_pFreqGpuGraph->DecreaseXAxisTickLabelFont(2);
        m_pFreqGpuGraph->SetXAxisTickLabelPadding(1);
    }
    else
    {
        m_pFreqGpuGraph->GetPlot()->setVisible(false);
        if (GetCpuFamily() == CPU_FAMILY_17)
            m_pGraphContainers[CONTAINER_PP_FREQ_GPU_GRAPH]->setVisible(false);
    }
}

// ----------------------------- CPU + GPU --------------------------------

void ppSummaryView::SetFreqGraphData(bool isDataFromEvent, ppDataUtils::GraphViewCategoryType graphType)
{
    gtVector<int> counterIds;
    bool isGetCountersOk = false;

    if (graphType == ppDataUtils::SUMMARY_FREQUENCY_CPU)
    {
        isGetCountersOk = GetCounterIdsByGraphType(ppDataUtils::SUMMARY_FREQUENCY_CPU, counterIds);
    }
    else //SUMMARY_FREQUENCY_GPU
    {
        isGetCountersOk = GetCounterIdsByGraphType(ppDataUtils::SUMMARY_FREQUENCY_GPU, counterIds);
    }

    if (isGetCountersOk && counterIds.size() > 0)
    {
        // get data from DB and convert into graph input data
        QVector<acBarGraphData*> counterDataVec;

        gtMap<int, gtVector<HistogramBucket> >* pCountesDataMapRef = nullptr;

        gtMap<int, gtVector<HistogramBucket> > countersDataMapFromDB;

        // get data map
        if (isDataFromEvent)
        {
            //from event - use the data map saved in m_lastCPUBucketsPerCounter / m_lastGPUBucketsPerCounter
            if (graphType == ppDataUtils::SUMMARY_FREQUENCY_CPU)
            {
                pCountesDataMapRef = &m_lastCPUBucketsPerCounter;
            }
            else // GPU
            {
                pCountesDataMapRef = &m_lastGPUBucketsPerCounter;
            }
        }
        else
        {
            // get data from DB
            ppDataUtils::GetSummaryFrequencyDataFromDB(graphType, countersDataMapFromDB, m_pSessionController);
            pCountesDataMapRef = &countersDataMapFromDB;
        }

        if (pCountesDataMapRef != nullptr)
        {
            // set data vector for initializing the bars
            int countersNum = counterIds.size();

            QString name;
            QColor color;

            for (int i = 0; i < countersNum; i++)
            {
                int counterId = counterIds[i];

                if (pCountesDataMapRef->count(counterId) > 0 && pCountesDataMapRef->at(counterId).size() > 0)
                {
                    // get id, name and color for new graph
                    name = m_pSessionController->GetCounterNameById(counterId);
                    color = m_pSessionController->GetColorForCounter(counterId);

                    // Cut the "Frequency" category string off the counter name:
                    ppDataUtils::CutCategoryFromCounterName(name);

                    counterDataVec.append(new ppStackedBarGraphData(pCountesDataMapRef->at(counterId), color, name, (double)FREQUENCY_GRAPH_BARS_VALUE_TYPE));
                }
            }
        }

        bool allowReplot = true;

        if (graphType == ppDataUtils::SUMMARY_FREQUENCY_CPU)
        {
            // create graph bars from data collected:
            m_pFreqCpuGraph->SetData(counterDataVec, allowReplot);
        }
        else //GPU
        {
            // create graph bars from data collected
            m_pFreqGpuGraph->SetData(counterDataVec, allowReplot);
        }

        for (int i = 0; i < counterDataVec.count(); i++)
        {
            delete counterDataVec[i];
        }
    }
}

void ppSummaryView::UpdateStackedBarGraphData(acStackedBarGraph* graph, ppDataUtils::GraphViewCategoryType graphType)
{
    gtVector<int> counterIds;
    bool isGetCountersOk = false;

    if (graphType == ppDataUtils::SUMMARY_FREQUENCY_CPU)
    {
        isGetCountersOk = GetCounterIdsByGraphType(ppDataUtils::SUMMARY_FREQUENCY_CPU, counterIds);
    }
    else
    {
        isGetCountersOk = GetCounterIdsByGraphType(ppDataUtils::SUMMARY_FREQUENCY_GPU, counterIds);
    }

    if (isGetCountersOk && counterIds.size() > 0)
    {
        gtMap<int, gtVector<HistogramBucket> >::const_iterator it;
        gtMap<int, gtVector<HistogramBucket> >::const_iterator itEnd;

        if (graphType == ppDataUtils::SUMMARY_FREQUENCY_CPU)
        {
            it = m_lastCPUBucketsPerCounter.begin();
            itEnd = m_lastCPUBucketsPerCounter.end();
        }
        else // GPU
        {
            it = m_lastGPUBucketsPerCounter.begin();
            itEnd = m_lastGPUBucketsPerCounter.end();
        }

        QVector<acSingleGraphData*> graphData;
        QVector<QString> newLabels;
        bool shouldChangeLabels = false;
        bool isSetExistingBarData = true;

        for (; it != itEnd; it++)
        {
            const gtVector<HistogramBucket>& bucketsVec = (*it).second;
            int numOfBars = bucketsVec.size();

            if (numOfBars != graph->XLabelsCount())
            {
                // the new data can't be smalled number of bars
                GT_IF_WITH_ASSERT(numOfBars > graph->XLabelsCount())
                {
                    // build new graph
                    if (graphType == ppDataUtils::SUMMARY_FREQUENCY_CPU)
                    {
                        InitCpuGraph();
                    }
                    else //GPU
                    {
                        InitGpuGraph();
                    }

                    SetFreqGraphData(true, graphType);
                }
                isSetExistingBarData = false;
                break;
            }
            // has graphs and can use existing bar graph
            else if (graph->GetNumOfDataSeries() != 0)
            {
                if (m_pSessionController != nullptr)
                {
                    //get counter for data series (bar graph) index
                    QString counterName = m_pSessionController->GetCounterNameById((*it).first);

                    // Fix the counter name (we shortened it for graphic reasons) (replaced "CPU Corei Frequency" by "Corei"):
                    ppDataUtils::CutCategoryFromCounterName(counterName);

                    // Get the graph index for this counter:
                    int graphIndex = graph->GetBarGraphIndexByName(counterName);

                    GT_IF_WITH_ASSERT(graphIndex != -1)
                    {
                        QVector<double> valuesVec;
                        QVector<QString> xlabels;

                        ppDataUtils::SummaryFreqGraphPrepareStackedSingleGraphNewData(valuesVec,
                                                                                      bucketsVec,
                                                                                      graph->XLabelsVector(),
                                                                                      shouldChangeLabels,
                                                                                      newLabels,
                                                                                      (double)FREQUENCY_GRAPH_BARS_VALUE_TYPE);

                        acSingleGraphData* singleGraphData = new acSingleGraphData(valuesVec, graphIndex);
                        graphData << singleGraphData;
                    }
                    else
                    {
                        isSetExistingBarData = false;
                    }
                }
            }
            else
            {
                isSetExistingBarData = false;
            }
        }

        if (isSetExistingBarData)
        {
            // will replot after setAxisLabels
            bool allowReplot = false;
            graph->UpdateData(graphData, allowReplot);

            if (shouldChangeLabels)
            {
                graph->SetXAxisLabelsVector(newLabels);
            }

            graph->Replot();
        }

        for (int i = 0; i < graphData.count(); i++)
        {
            delete graphData[i];
        }
    }
}

void ppSummaryView::UpdateSessionInformation()
{
    AMDTProfileSessionInfo currentSessionInfo;

    GT_IF_WITH_ASSERT((m_pSessionController != nullptr) && (m_pHTMLInfoWindow != nullptr) && (m_pParentSession != nullptr))
    {
        // Will contain the HTML string to store:
        gtString htmlStr;
        afHTMLContent content;

        if (m_pSessionController->GetSessionState() == ppSessionController::PP_SESSION_STATE_COMPLETED)
        {
            // Get the session info from middle tier for running session and from app controller for offline sessions:
            bool rcGetInfo = m_pSessionController->GetProfilerBL().GetSessionInfo(currentSessionInfo);
            GT_IF_WITH_ASSERT(rcGetInfo)
            {
                content.addHTMLItem(afHTMLContent::AP_HTML_TITLE, PP_STR_SummaryExecutionHeader);

                gtString htmlLineStr;
                htmlLineStr.appendFormattedString(PM_Str_ViewHTMLLineStructure, PP_STR_SummaryProfileScope, currentSessionInfo.m_sessionScope.asCharArray());
                content.addHTMLItem(afHTMLContent::AP_HTML_LINE, htmlLineStr);

                // Display specific application session details only for session which execute an application:
                if (currentSessionInfo.m_sessionScope != PM_STR_ProfileScopeSystemWide)
                {
                    htmlLineStr.makeEmpty();
                    htmlLineStr.appendFormattedString(PM_Str_ViewHTMLLineStructure, PP_STR_SummaryTargetPath, currentSessionInfo.m_targetAppPath.asCharArray());
                    content.addHTMLItem(afHTMLContent::AP_HTML_LINE, htmlLineStr);

                    htmlLineStr.makeEmpty();
                    htmlLineStr.appendFormattedString(PM_Str_ViewHTMLLineStructure, PP_STR_SummaryWorkingDirectory, currentSessionInfo.m_targetAppWorkingDir.asCharArray());
                    content.addHTMLItem(afHTMLContent::AP_HTML_LINE, htmlLineStr);

                    htmlLineStr.makeEmpty();
                    htmlLineStr.appendFormattedString(PM_Str_ViewHTMLLineStructure, PP_STR_SummaryCommandLineArgs, currentSessionInfo.m_targetAppCmdLineArgs.asCharArray());
                    content.addHTMLItem(afHTMLContent::AP_HTML_LINE, htmlLineStr);

                    htmlLineStr.makeEmpty();
                    htmlLineStr.appendFormattedString(PM_Str_ViewHTMLLineStructure, PP_STR_SummaryEnvVars, currentSessionInfo.m_targetAppEnvVars.asCharArray());
                    content.addHTMLItem(afHTMLContent::AP_HTML_LINE, htmlLineStr);
                }

                htmlLineStr.makeEmpty();
                htmlLineStr.appendFormattedString(PM_Str_ViewHTMLLineStructure, PP_STR_SummarySessionDirectory, currentSessionInfo.m_sessionDir.asCharArray());
                content.addHTMLItem(afHTMLContent::AP_HTML_LINE, htmlLineStr);


                content.addHTMLItem(afHTMLContent::AP_HTML_TITLE, PP_STR_SummaryProfileDetailsHeader);

                htmlLineStr.makeEmpty();
                htmlLineStr.appendFormattedString(PM_Str_ViewHTMLLineStructure, PP_STR_SummarySessionType, PP_STR_SummaryProfilePower);
                content.addHTMLItem(afHTMLContent::AP_HTML_LINE, htmlLineStr);

                htmlLineStr.makeEmpty();
                htmlLineStr.appendFormattedString(PM_Str_ViewHTMLLineStructure, PP_STR_SummaryProfileStartTime, currentSessionInfo.m_sessionStartTime.asCharArray());
                content.addHTMLItem(afHTMLContent::AP_HTML_LINE, htmlLineStr);

                htmlLineStr.makeEmpty();
                htmlLineStr.appendFormattedString(PM_Str_ViewHTMLLineStructure, PP_STR_SummaryProfileEndTime, currentSessionInfo.m_sessionEndTime.asCharArray());
                content.addHTMLItem(afHTMLContent::AP_HTML_LINE, htmlLineStr);
            }
        }

        else
        {
            // When the session is running, create a short HTML description:
            content.addHTMLItem(afHTMLContent::AP_HTML_TITLE, PP_STR_SummaryExecutionHeader);

            if (m_pSessionController->GetSessionState() == ppSessionController::PP_SESSION_STATE_NEW)
            {
                content.addHTMLItem(afHTMLContent::AP_HTML_LINE, PP_STR_SummaryNewCreatedMessage);
            }
            else  // running
            {
                content.addHTMLItem(afHTMLContent::AP_HTML_LINE, PP_STR_SummaryRunningMessage);
            }
        }

        // Get the HTML as string:
        content.toString(htmlStr);

        // Convert the HTML items into a string:
        m_pHTMLInfoWindow->setHtml(acGTStringToQString(htmlStr));
    }
}

void ppSummaryView::SetSelectedCounterIdsPerGraphMap()
{
    // set ids map by graph type
    m_selectedCunterIdsPerGraphMap.clear();

    gtVector<int> idsVec;
    ppDataUtils::GetRelevantCounterIdsByGraphType(idsVec, ppDataUtils::SUMMARY_POWER, m_pSessionController);
    //order Ids
    ppAppController::instance().SortCountersInCategory(AMDT_PWR_CATEGORY_POWER, idsVec);
    m_selectedCunterIdsPerGraphMap[ppDataUtils::SUMMARY_POWER] = idsVec;

    idsVec.clear();
    ppDataUtils::GetRelevantCounterIdsByGraphType(idsVec, ppDataUtils::SUMMARY_FREQUENCY_CPU, m_pSessionController);
    //order Ids
    ppAppController::instance().SortCountersInCategory(AMDT_PWR_CATEGORY_FREQUENCY, idsVec);
    m_selectedCunterIdsPerGraphMap[ppDataUtils::SUMMARY_FREQUENCY_CPU] = idsVec;

    idsVec.clear();
    ppDataUtils::GetRelevantCounterIdsByGraphType(idsVec, ppDataUtils::SUMMARY_FREQUENCY_GPU, m_pSessionController);
    //order Ids
    ppAppController::instance().SortCountersInCategory(AMDT_PWR_CATEGORY_FREQUENCY, idsVec);
    m_selectedCunterIdsPerGraphMap[ppDataUtils::SUMMARY_FREQUENCY_GPU] = idsVec;
}

void ppSummaryView::ReplotAllGrpahs()
{
    GT_IF_WITH_ASSERT(m_pEnergyGraph != nullptr)
    {
        m_pEnergyGraph->GetPlot()->replot();
    }
    GT_IF_WITH_ASSERT(m_pPowerGraph != nullptr)
    {
        m_pPowerGraph->GetPlot()->replot();
    }

    GT_IF_WITH_ASSERT(m_pFreqCpuGraph != nullptr)
    {
        m_pFreqCpuGraph->GetPlot()->replot();
    }

    GT_IF_WITH_ASSERT(m_pFreqGpuGraph != nullptr)
    {
        m_pFreqGpuGraph->GetPlot()->replot();
    }
}

bool ppSummaryView::GetCounterIdsByGraphType(ppDataUtils::GraphViewCategoryType type, gtVector<int>& selectedCounters)
{
    bool retVal = false;

    // assert if can't find the graph type in the map
    GT_IF_WITH_ASSERT(m_selectedCunterIdsPerGraphMap.count(type) != 0)
    {
        selectedCounters = m_selectedCunterIdsPerGraphMap[type];
        retVal = true;
    }

    return retVal;
}

void ppSummaryView::OnSelectedCountersChanged()
{
    GT_IF_WITH_ASSERT(m_pSessionController != nullptr)
    {
        // Get the state from the controller:
        ppSessionController::SessionState  currentState = m_pSessionController->GetSessionState();

        if (currentState == ppSessionController::PP_SESSION_STATE_NEW)
        {
            InitGraphs();
        }
    }
}

void ppSummaryView::SetDurationLabel(int milliRange)
{
    GT_IF_WITH_ASSERT(m_pSessionDurationLabel != nullptr)
    {
        int inMili = milliRange % MILLISECONDS_IN_SECOND;
        int inSeconds = (milliRange / MILLISECONDS_IN_SECOND) % SECONDS_IN_MINUTE;
        int inMinutes = (milliRange / MILLISECONDS_IN_MINUTE) % SECONDS_IN_MINUTE;
        int inHours = (milliRange / MILLISECONDS_IN_HOUR) % SECONDS_IN_MINUTE;

        QString q;
        q.sprintf(PP_STR_SummaryDurationLabel, inHours, inMinutes, inSeconds, inMili);
        m_pSessionDurationLabel->setText(q);

        // Hide the session duration for new session window:
        m_pSessionDurationLabel->setVisible(milliRange > 0);
    }
}

QString ppSummaryView::GetPowerGraphUnit(ppDataUtils::PowerGraphType graphType)
{
    QString title;

    if (graphType == ppDataUtils::POWERGRAPHTYPE_AVERAGE)
    {
        title = PP_STR_UnitsWatts;
    }
    else if (graphType == ppDataUtils::POWERGRAPHTYPE_CUMULATIVE)
    {
        title = PP_STR_UnitsJoules;

        switch (m_currentPowerMeasurementUnit)
        {
            case MEASUREMENTUNIT_JOULES:
                title = PP_STR_UnitsJoules;
                break;

            case MEASUREMENTUNIT_KILOJOULES:
                title = PP_STR_UnitsKiloJoules;
                break;

            case MEASUREMENTUNIT_MEGAJOULES:
                title = PP_STR_UnitsMegaJoules;
                break;

            case MEASUREMENTUNIT_GIGAJOULES:
                title = PP_STR_UnitsGigaJoules;
                break;

            case MEASUREMENTUNIT_TERAJOULES:
                title = PP_STR_UnitsTeraJoules;
                break;

            default:
                title = PP_STR_UnitsJoules;
                break;
        }
    }

    return title;
}

template<class T>
void ppSummaryView::UpdateEnergyGraphMeasurementUnit(const QVector<T*>& dataVec)
{
    GT_IF_WITH_ASSERT(m_pEnergyGraph != nullptr)
    {
        PowerGraphMeasurementUnit startUnit = m_currentPowerMeasurementUnit;

        int dataVecSize = dataVec.size();

        // if the current unit is Tera don't check - it is the max unit
        if (dataVecSize > 0 && m_currentPowerMeasurementUnit < MEASUREMENTUNIT_TERAJOULES)
        {
            for (int i = 0; i < dataVecSize; i++)
            {
                // for each data in vector - get the double values data vector
                QVector<double>& dataLocal = dataVec[i]->m_yData;
                int valsNum = dataLocal.size();

                for (int j = 0; j < valsNum; j++)
                {
                    if (dataLocal[j] > pow((double)MEASUREMENTUNIT_POWER_BASE, (double)MEASUREMENTUNIT_TERAJOULES))
                    {
                        m_currentPowerMeasurementUnit = MEASUREMENTUNIT_TERAJOULES;
                        break;  //this is the max unit - no need to continue
                    }
                    else if (m_currentPowerMeasurementUnit < MEASUREMENTUNIT_GIGAJOULES && dataLocal[j] > pow((double)MEASUREMENTUNIT_POWER_BASE, (double)MEASUREMENTUNIT_GIGAJOULES))
                    {
                        m_currentPowerMeasurementUnit = MEASUREMENTUNIT_GIGAJOULES;
                    }
                    else if (m_currentPowerMeasurementUnit < MEASUREMENTUNIT_MEGAJOULES && dataLocal[j] > pow((double)MEASUREMENTUNIT_POWER_BASE, (double)MEASUREMENTUNIT_MEGAJOULES))
                    {
                        m_currentPowerMeasurementUnit = MEASUREMENTUNIT_MEGAJOULES;
                    }
                    else if (m_currentPowerMeasurementUnit < MEASUREMENTUNIT_KILOJOULES && dataLocal[j] > pow((double)MEASUREMENTUNIT_POWER_BASE, (double)MEASUREMENTUNIT_KILOJOULES))
                    {
                        m_currentPowerMeasurementUnit = MEASUREMENTUNIT_KILOJOULES;
                    }

                    // else = stay with Joules
                }
            }

            // Cumulative energy title:
            // Find the current measurement unit string:
            QString unitStr = GetPowerGraphUnit(ppDataUtils::POWERGRAPHTYPE_CUMULATIVE);
            QString title = QString(PP_STR_SummaryViewEnergyCaptionWithUnits).arg(unitStr);

            // if measurement unit changed - update graph range to 0..10 for the new unit
            if (startUnit != m_currentPowerMeasurementUnit)
            {
                m_pEnergyGraph->SetGraphYRange(0, 10);

                // update title
                m_pEnergyGraph->SetGraphYAxisTitle(title);
            }
        }
    }

    // convert data values by measurement unit
    if (m_currentPowerMeasurementUnit > MEASUREMENTUNIT_JOULES)
    {
        int countersNum = dataVec.size();

        // foreach counter
        for (int i = 0; i < countersNum; i++)
        {
            QVector<double>& data1 = dataVec[i]->m_yData;
            int valuesNum = data1.size();

            // update the value to be fit with the displayed unit. all values from input are in Joules
            for (int j = 0; j < valuesNum; j++)
            {
                data1[j] = data1[j] / pow((double)MEASUREMENTUNIT_POWER_BASE, (double)m_currentPowerMeasurementUnit);
            }
        }
    }
}

void ppSummaryView::OnTopSplitterMoved(int index, int position)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pTopRightHorizontalSplitter != nullptr) && (m_pTopLeftHorizontalSplitter != nullptr))
    {
        if (sender() == m_pTopLeftHorizontalSplitter)
        {
            m_pTopRightHorizontalSplitter->blockSignals(true);
            m_pTopRightHorizontalSplitter->MoveSplitter(index, position);
            m_pTopRightHorizontalSplitter->blockSignals(false);
        }
        else if (sender() == m_pTopRightHorizontalSplitter)
        {
            m_pTopLeftHorizontalSplitter->blockSignals(true);
            m_pTopLeftHorizontalSplitter->MoveSplitter(index, position);
            m_pTopLeftHorizontalSplitter->blockSignals(false);
        }
    }
}

void ppSummaryView::SetPowerGraphsCaption()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pEnergyGraph != nullptr) && (m_pPowerGraph != nullptr))
    {
        // Create the strings describing the total energy in Joules:
        QString energyGraphText, powerGraphText;
        double value = m_apuCounterLastCumulativeValue / pow((double)MEASUREMENTUNIT_POWER_BASE, (double)m_currentPowerMeasurementUnit);
        gtString strValue;
        strValue.appendFormattedString(L"%.2f", value);
        strValue.addThousandSeperators();
        QString qstrValue = acGTStringToQString(strValue);
        energyGraphText = QString(PP_STR_SummaryViewPowerGraphTypeAverage).arg(qstrValue).arg(GetPowerGraphUnit(ppDataUtils::POWERGRAPHTYPE_CUMULATIVE));

        strValue.makeEmpty();
        strValue.appendFormattedString(L"%.2f", m_apuCounterLastAverageValue);
        strValue.addThousandSeperators();
        qstrValue = acGTStringToQString(strValue);
        powerGraphText = QString(PP_STR_SummaryViewEnergyGraphTypeCumulative).arg(qstrValue).arg(PP_STR_UnitsWatts);

        // Update the total / average text items in power & energy graphs:
        if (GetCpuFamily() == CPU_FAMILY_17)
        {
            
        }
        else
        {
            m_pPowerGraph->SetGraphTitle(powerGraphText);
            m_pEnergyGraph->SetGraphTitle(energyGraphText);
        }
        
    }
}
