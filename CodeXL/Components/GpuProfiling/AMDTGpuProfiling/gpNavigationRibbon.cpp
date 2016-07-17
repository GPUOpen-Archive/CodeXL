//------------------------------ gpNavigationRibbon.cpp ------------------------------

// Qt

// infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acNavigationChart.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineGrid.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineFiltersDialog.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acIcons.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>

// Local
#include <AMDTGpuProfiling/gpNavigationRibbon.h>
#include <AMDTGpuProfiling/gpTraceDataContainer.h>
#include <AMDTGpuProfiling/gpTraceView.h>
#include <AMDTGpuProfiling/gpRibbonDataCalculator.h>
#include <AMDTGpuProfiling/gpUIManager.h>

#define LAYOUT_MARGINS 5 // as as the acTimeLineGrid ms_GRID_MARGIN

#define AXIS_RIGHT_MARGIN 10
#define AXIS_TOP_MARGIN 20

#define MAX_HIGHLIGHTED_API_CALLS 20
#define NAVIGATE_BY_TOP_MARGIN 0
#define DROP_BOX_TOP_MARGIN 15


#define GP_StrFilterButtonStyle "QPushButton {border: none; background-color: transparent;}" \
    "QPushButton:pressed { border: solid gray 1; padding: 2px; background-color: transparent;border: none;}" \
    "QPushButton:hover { border: 1px  solid gray; padding: 2px; background-color: transparent;}"


/// Static member initialization
int gpNavigationRibbon::m_sLayerToGroupConnection[eNavigateLayersNum] = { eNavigateGroupCount, eNavigateGroupCount, eNavigateGroupDuration, eNavigateGroupDuration , eNavigateGroupDuration, eNavigateGroupDuration, eNavigateGroupThread, eNavigateGroupThread, eNavigateGroupThread };

gpNavigationRibbon::gpNavigationRibbon(QWidget* pParent, gpRibbonDataCalculator* pCalculator) : QWidget(pParent),
    m_pOwningSession(nullptr),
    m_pNavigateByVBox(nullptr),
    m_pGroupComboBox(nullptr),
    m_pNavigationChart(nullptr),
    m_pTimeLine(nullptr),
    m_pTimeLineGrid(nullptr),
    m_inZoomOrPan(false),
    m_pFiltersLink(nullptr),
    m_pThreadFilterDialog(nullptr),
    m_pApiCallsLayer(nullptr),
    m_pDrawCallsLayer(nullptr),
    m_pMaxCPUApiTimeLayer(nullptr),
    m_pMaxGPUOpsTimeLayer(nullptr),
    m_pMaxDurationLayer(nullptr),
    m_pMaxGPUOpsLayer(nullptr),
    m_pThreadConcurrencyLayer(nullptr),
    m_pAvgThreadConcurrencyLayer(nullptr),
    m_pTotalConcurrencyLayer(nullptr),
    m_pPresentBars(nullptr),
    m_pFrameDataCalculator(pCalculator),
    m_wasConcurrencyCalculated(false)
{
    m_pOwningSession = (gpTraceView*)pParent;

    for (int i = 0; i < eNavigateLayersNum; i++)
    {
        m_pNavigateLayer[i] = nullptr;
    }

    InitLayout();

    m_presentData.clear();
}

gpNavigationRibbon::~gpNavigationRibbon()
{

}

void gpNavigationRibbon::InitLayout()
{
    QHBoxLayout* pMainLayout = new QHBoxLayout(this);
    pMainLayout->setContentsMargins(0, NAVIGATE_BY_TOP_MARGIN, 0, 0);
    m_pNavigateByVBox = new QVBoxLayout;
    m_pGroupComboBox = new QComboBox();
    QStringList groupList = { GPU_STR_navigationGroupDuration, GPU_STR_navigationGroupCount, GPU_STR_navigationGroupThreads };
    m_pGroupComboBox->addItems(groupList);

    m_pNavigateLayer[eNavigateLayerApiCalls] = new QCheckBox(GPU_STR_navigationLayerApiCalls);
    m_pNavigateLayer[eNavigateLayerDrawCalls] = new QCheckBox(GPU_STR_navigationLayerDrawCalls);
    m_pNavigateLayer[eNavigateLayerTopGPUCmdsCalls] = new QCheckBox(GPU_STR_navigationLayerTopGPUOps);
    m_pNavigateLayer[eNavigateLayerTopCPUApiCalls] = new QCheckBox(GPU_STR_navigationLayerTopCPUDrawCalls);
    m_pNavigateLayer[eNavigateLayerGPUOpsTime] = new QCheckBox(GPU_STR_navigationLayerGPUCmds);
    m_pNavigateLayer[eNavigateLayerCPUApiTime] = new QCheckBox(GPU_STR_navigationLayerCPUApi);
    m_pNavigateLayer[eNavigateLayerMaxThreads] = new QCheckBox(GPU_STR_navigationLayerMaxSimultaneaus);
    m_pNavigateLayer[eNavigateLayerAvgThreads] = new QCheckBox(GPU_STR_navigationLayerAvgSimultaneaus);
    m_pNavigateLayer[eNavigateLayerTotalThreads] = new QCheckBox(GPU_STR_navigationLayerTotalSimultaneaus);

    // Check all check boxes by default (is needed for the delayed concurrency calculation)
    m_pNavigateLayer[eNavigateLayerApiCalls]->setChecked(true);
    m_pNavigateLayer[eNavigateLayerDrawCalls]->setChecked(true);
    m_pNavigateLayer[eNavigateLayerTopGPUCmdsCalls]->setChecked(true);
    m_pNavigateLayer[eNavigateLayerTopCPUApiCalls]->setChecked(true);
    m_pNavigateLayer[eNavigateLayerGPUOpsTime]->setChecked(true);
    m_pNavigateLayer[eNavigateLayerCPUApiTime]->setChecked(true);
    m_pNavigateLayer[eNavigateLayerMaxThreads]->setChecked(true);
    m_pNavigateLayer[eNavigateLayerAvgThreads]->setChecked(true);
    m_pNavigateLayer[eNavigateLayerTotalThreads]->setChecked(true);

    m_pNavigateByVBox->addWidget(m_pGroupComboBox);
    bool rc;

    for (int i = 0; i < eNavigateLayersNum; i++)
    {
        m_pNavigateByVBox->addWidget(m_pNavigateLayer[i]);

        // set the initial navigation item by before the connection
        rc = connect(m_pNavigateLayer[i], SIGNAL(stateChanged(int)), this, SLOT(OnNavigateStateChanged(int)));
        GT_ASSERT(rc);
    }

    m_pNavigateByVBox->addStretch(1);
    m_pNavigateByVBox->setContentsMargins(0, DROP_BOX_TOP_MARGIN, 0, 0);
    pMainLayout->addLayout(m_pNavigateByVBox);


    // Create the navigation chart
    m_pNavigationChart = new acNavigationChart();

    // Set the navigation chart display range and margins
    GT_IF_WITH_ASSERT(m_pNavigationChart->axisRect() != nullptr)
    {
        m_pNavigationChart->axisRect()->setMargins(QMargins(GP_NAVIGATION_CHART_LEFT_MARGIN, AC_NAVIGATION_CHART_TOP_MARGIN, GP_NAVIGATION_CHART_RIGHT_MARGIN, AC_NAVIGATION_CHART_BOTTOM_MARGIN));
        m_pNavigationChart->axisRect()->setAutoMargins(QCP::msNone);
    }
    m_pNavigationChart->SetDefaultTimeRange(-1);
    m_pNavigationChart->SetMinimumRange(-0.01);
    m_pNavigationChart->SetNavigationUnitsX(acNavigationChart::eNavigationNanoseconds);
    m_pNavigationChart->SetUnitsDisplayType(acNavigationChart::eNavigationDisplayMilliOnly);
    m_pNavigationChart->UseTimelineSync();

    pMainLayout->addWidget(m_pNavigationChart);
    pMainLayout->addStretch(1);

    // filters dialog link in the top-right corner
    m_pFiltersLink = new gpFilterButton(m_pNavigationChart);

    QPixmap pix;
    rc = acSetIconInPixmap(pix, AC_ICON_RIBBON_FILTER);
    GT_ASSERT(rc);

    QIcon ButtonIcon(pix);
    m_pFiltersLink->setIcon(ButtonIcon);

    m_pFiltersLink->setVisible(false);
    rc = connect(m_pFiltersLink, SIGNAL(clicked()), this, SLOT(OnDisplayFiltersClicked()));
    GT_ASSERT(rc);
    rc = connect(m_pNavigationChart, SIGNAL(ChartEnter(acNavigationChart*)), m_pFiltersLink, SLOT(OnChartEnter(acNavigationChart*)));
    GT_ASSERT(rc);
    rc = connect(m_pNavigationChart, SIGNAL(ChartLeave()), m_pFiltersLink, SLOT(OnChartLeave()));
    GT_ASSERT(rc);

    m_pThreadFilterDialog = new acTimelineFiltersDialog(this);

    GT_ASSERT(rc);
    rc = connect(m_pGroupComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnGroupChanged(int)));
    GT_ASSERT(rc);

    // select initial group
    OnGroupChanged(0);

    setLayout(pMainLayout);
}

void gpNavigationRibbon::SetTimeLine(acTimeline* pTimeLine)
{
    GT_IF_WITH_ASSERT(pTimeLine != nullptr && m_pNavigationChart != nullptr)
    {
        m_pTimeLine = pTimeLine;
        m_pTimeLineGrid = m_pTimeLine->grid();

        GT_IF_WITH_ASSERT(m_pTimeLineGrid != nullptr)
        {
            m_pTimeLineGrid->SetRightMargins(AXIS_RIGHT_MARGIN);
        }

        // make the zoom in zoom out connections
        bool rc = connect(m_pNavigationChart, SIGNAL(RangeChangedByUser(const QPointF&)), this, SLOT(OnRangeChangedByUser(const QPointF&)));
        GT_ASSERT(rc);

        rc = connect(pTimeLine, SIGNAL(zoomFactorChanged()), this, SLOT(OnTimeLineZoomOrOffsetChanged()));
        GT_ASSERT(rc);

        rc = connect(pTimeLine, SIGNAL(offsetChanged()), this, SLOT(OnTimeLineZoomOrOffsetChanged()));
        GT_ASSERT(rc);
    }
}

void gpNavigationRibbon::CalculateData()
{
    GT_IF_WITH_ASSERT(m_pGroupComboBox != nullptr)
    {
        m_pGroupComboBox->blockSignals(true);

        for (int i = 0; i < eNavigateLayersNum; i++)
        {
            if (m_pNavigateLayer[i] != nullptr)
            {
                m_pNavigateLayer[i]->blockSignals(true);
            }
        }

        // Calculate the buckets sizes this will be the default data for the main graph for calculating the indexes
        afApplicationCommands::instance()->StartPerformancePrintout("gpNavigationRibbon::CalculateBuckets");
        CalculateBuckets();
        afApplicationCommands::instance()->EndPerformancePrintout("gpNavigationRibbon::CalculateBuckets");

        // calculate API calls and Draw calls
        afApplicationCommands::instance()->StartPerformancePrintout("gpNavigationRibbon::CalculateCalls");
        CalculateCalls();
        afApplicationCommands::instance()->EndPerformancePrintout("gpNavigationRibbon::CalculateCalls");

        // calculate gpu related data
        afApplicationCommands::instance()->StartPerformancePrintout("gpNavigationRibbon::CalculateGPUData");
        CalculateGPUData();
        afApplicationCommands::instance()->EndPerformancePrintout("gpNavigationRibbon::CalculateGPUData");

        m_pGroupComboBox->blockSignals(false);

        for (int i = 0; i < eNavigateLayersNum; i++)
        {
            if (m_pNavigateLayer[i] != nullptr)
            {
                m_pNavigateLayer[i]->blockSignals(false);
            }
        }

        // make sure visibility is correct
        afApplicationCommands::instance()->StartPerformancePrintout("gpNavigationRibbon::OnGroupChanged");
        OnGroupChanged(m_pGroupComboBox->currentIndex());
        afApplicationCommands::instance()->EndPerformancePrintout("gpNavigationRibbon::OnGroupChanged");
    }
}

void gpNavigationRibbon::CalculateBuckets()
{
    // since the y values of the bins will not be displayed they will be set to 0
    GT_IF_WITH_ASSERT(m_pTimeLineGrid != nullptr)
    {
        QVector<double> binsYValues;
        binsYValues.resize(LAYER_DISPLAY_LIMIT);
        m_bucketVals.clear();
        // one bucket goes to 0
        double rangePerBucket = m_pTimeLineGrid->fullRange() * 1.0 / (LAYER_DISPLAY_LIMIT - 1);

        // Sanity check:
        GT_IF_WITH_ASSERT(rangePerBucket > 0)
        {
            for (int nBin = 0; nBin < LAYER_DISPLAY_LIMIT; nBin++)
            {
                binsYValues[nBin] = 0;
                m_bucketVals.push_back(rangePerBucket * nBin);
            }
        }
        m_pNavigationChart->SetOfflineData(m_bucketVals, binsYValues);
    }
}

void gpNavigationRibbon::CalculateCalls()
{
    QVector<double> drawCallsInBuckets;
    QVector<double> apiCallsInBuckets;
    QVector<double> maxApiCallTimeInBucket;
    QVector<double> highlightedApiCallsTime;
    QVector<double> highlightedApiCallsDuration;
    QVector<gpRibbonCallsData> drawCalls;
    QVector<gpRibbonCallsData> apiCalls;

    GT_IF_WITH_ASSERT(m_pFrameDataCalculator != nullptr)
    {
        drawCalls = m_pFrameDataCalculator->GetDrawCalls();
        apiCalls = m_pFrameDataCalculator->GetApiCalls();

        // Get the number of draw calls per bucket:
        m_pFrameDataCalculator->CallsToBuckets(drawCalls, drawCallsInBuckets, gpRibbonDataCalculator::eBucketCountMode);

        // Get the number of api calls per bucket:
        m_pFrameDataCalculator->CallsToBuckets(apiCalls, apiCallsInBuckets, gpRibbonDataCalculator::eBucketCountMode);

        // Get the longest api call per bucket:
        m_pFrameDataCalculator->CallsToBuckets(apiCalls, maxApiCallTimeInBucket, gpRibbonDataCalculator::eBucketMaxValueMode);

        // get top 20 draw calls
        m_pFrameDataCalculator->GetTopCalls(apiCalls, highlightedApiCallsTime, highlightedApiCallsDuration, MAX_HIGHLIGHTED_API_CALLS);

        // Create the layers
        // number of api calls per bucket layer:
        QColor highlightColor = QColor(Qt::darkGray);
        QColor dimmedColor = highlightColor;
        dimmedColor.setAlpha(128);
        AddIndexedLayer(m_pApiCallsLayer, apiCallsInBuckets, eNavigateLayerApiCalls, acNavigationChartLayer::eNavigationLayerBar, dimmedColor, QColor(Qt::white), highlightColor, dimmedColor);

        // number of api calls per bucket layer:
        highlightColor = QColor(Qt::darkBlue);
        dimmedColor = highlightColor;
        dimmedColor.setAlpha(128);
        AddIndexedLayer(m_pDrawCallsLayer, drawCallsInBuckets, eNavigateLayerDrawCalls, acNavigationChartLayer::eNavigationLayerBar, dimmedColor, QColor(Qt::white), highlightColor, dimmedColor);

        // number of max api calls time per bucket layer:
        highlightColor = acLIGHT_GREEN;
        dimmedColor = highlightColor;
        dimmedColor.setAlpha(128);
        AddIndexedLayer(m_pMaxCPUApiTimeLayer, maxApiCallTimeInBucket, eNavigateLayerCPUApiTime, acNavigationChartLayer::eNavigationLayerBar, dimmedColor, QColor(Qt::white), highlightColor, dimmedColor, false);

        // add the top 20 draw calls
        highlightColor = acDARK_GREEN;
        dimmedColor = highlightColor;
        dimmedColor.setAlpha(128);
        AddNoneIndexedLayer(m_pMaxDurationLayer, highlightedApiCallsTime, highlightedApiCallsDuration, eNavigateLayerTopCPUApiCalls, acNavigationChartLayer::eNavigationLayerBar, dimmedColor, dimmedColor, highlightColor, highlightColor);
    }
}

void gpNavigationRibbon::CalculateConcurrency()
{
    if (!m_wasConcurrencyCalculated)
    {
        GT_IF_WITH_ASSERT(m_pFrameDataCalculator != nullptr)
        {
            m_pFrameDataCalculator->CalculateCPUConcurrency();
            QVector<double> threadConcurrency = m_pFrameDataCalculator->CpuMaxThreadConcurrency();
            QVector<double> averageConcurrency = m_pFrameDataCalculator->CpuAverageThreadConcurrency();
            QVector<double> totalConcurrency = m_pFrameDataCalculator->CpuTotalThreadConcurrency();

            // the order of the insertion of the layer is important. from the layer that has higher values to the lower values.
            // add total thread concurrency
            QColor highlightColor = QColor(Qt::gray);
            QColor dimmedColor = highlightColor;
            highlightColor.setAlpha(128);
            dimmedColor.setAlpha(64);
            AddIndexedLayer(m_pTotalConcurrencyLayer, totalConcurrency, eNavigateLayerTotalThreads, acNavigationChartLayer::eNavigationLayerBar, dimmedColor, QColor(Qt::white), highlightColor, dimmedColor);

            // add the maximum concurrency layer
            highlightColor = QColor(Qt::darkMagenta);
            dimmedColor = highlightColor;
            dimmedColor.setAlpha(128);
            AddIndexedLayer(m_pThreadConcurrencyLayer, threadConcurrency, eNavigateLayerMaxThreads, acNavigationChartLayer::eNavigationLayerBar, dimmedColor, QColor(Qt::white), highlightColor, dimmedColor);

            // add the average concurrency layer
            highlightColor = QColor(Qt::darkCyan);
            dimmedColor = highlightColor;
            dimmedColor.setAlpha(128);
            AddIndexedLayer(m_pAvgThreadConcurrencyLayer, averageConcurrency, eNavigateLayerAvgThreads, acNavigationChartLayer::eNavigationLayerBar, dimmedColor, QColor(Qt::white), highlightColor, dimmedColor);
        }

        m_wasConcurrencyCalculated = true;
    }
}

void gpNavigationRibbon::CalculateGPUData()
{
    QVector<gpRibbonCallsData> gpuOpsCalls;

    QVector<double> GPUOpsMaxDuration;
    QVector<double> highlightedGPUOpsTime;
    QVector<double> highlightedGPUOpsDuration;

    GT_IF_WITH_ASSERT(m_pFrameDataCalculator != nullptr)
    {
        m_pFrameDataCalculator->GetGPUOps(gpuOpsCalls);

        // Get the longest gpu ops call per bucket:
        m_pFrameDataCalculator->CallsToBuckets(gpuOpsCalls, GPUOpsMaxDuration, gpRibbonDataCalculator::eBucketMaxValueMode);

        // get top 20 gpu ops calls
        m_pFrameDataCalculator->GetTopCalls(gpuOpsCalls, highlightedGPUOpsTime, highlightedGPUOpsDuration, MAX_HIGHLIGHTED_API_CALLS);

        // number of gpu longest call per bucket layer:
        QColor highlightColor = acLIGHT_PURPLE;
        QColor dimmedColor = highlightColor;
        dimmedColor.setAlpha(128);
        AddIndexedLayer(m_pMaxGPUOpsTimeLayer, GPUOpsMaxDuration, eNavigateLayerGPUOpsTime, acNavigationChartLayer::eNavigationLayerBar, dimmedColor, QColor(Qt::white), highlightColor, dimmedColor, false);

        // add the top 20 gpu ops calls
        highlightColor = acDARK_PURPLE;
        dimmedColor = highlightColor;
        dimmedColor.setAlpha(128);
        AddNoneIndexedLayer(m_pMaxGPUOpsLayer, highlightedGPUOpsTime, highlightedGPUOpsDuration, eNavigateLayerTopGPUCmdsCalls, acNavigationChartLayer::eNavigationLayerBar, dimmedColor, dimmedColor, highlightColor, highlightColor);
    }
}

void gpNavigationRibbon::AddIndexedLayer(acNavigationChartLayer*& pLayer, QVector<double>& yData, eNavigateDataBy layerIndex, acNavigationChartLayer::eNavigationChartLayerType layerType, QColor dimmedPenColor, QColor dimmedBrushColor, QColor highlightPenColor, QColor highlightBrushColor, bool visible)
{
    GT_IF_WITH_ASSERT(m_pNavigationChart != nullptr)
    {
        if (nullptr == pLayer)
        {
            pLayer = new acNavigationChartLayer;
            pLayer->m_layerYData = yData;
            pLayer->m_layerOriginalYData = yData;
            pLayer->m_type = layerType;
            pLayer->m_visible = visible;
            pLayer->m_dimmedPen = QPen(dimmedPenColor);
            pLayer->m_dimmedBrush = QBrush(dimmedBrushColor);
            pLayer->m_highlightedPen = QPen(highlightPenColor);
            pLayer->m_highlightedBrush = QBrush(highlightBrushColor);
            pLayer->m_layerId = layerIndex;
            m_pNavigationChart->AddDataLayer(pLayer);
            GT_IF_WITH_ASSERT(m_pNavigateLayer[layerIndex] != nullptr)
            {
                m_pNavigateLayer[layerIndex]->setChecked(visible);
                SetCheckBoxColorIcon(m_pNavigateLayer[layerIndex], highlightBrushColor);
            }
        }
        else
        {
            pLayer->m_layerYData = yData;
            pLayer->m_layerOriginalYData = yData;
        }
    }
}


void gpNavigationRibbon::AddNoneIndexedLayer(acNavigationChartLayerNonIndex*& pLayer, QVector<double>& xData, QVector<double>& yData, eNavigateDataBy layerIndex, acNavigationChartLayer::eNavigationChartLayerType layerType, QColor dimmedPenColor, QColor dimmedBrushColor, QColor highlightPenColor, QColor highlightBrushColor, bool visible)
{
    GT_IF_WITH_ASSERT(m_pNavigationChart != nullptr)
    {
        if (nullptr == pLayer)
        {
            pLayer = new acNavigationChartLayerNonIndex;
            pLayer->m_layerXData = xData;
            pLayer->m_layerYData = yData;
            pLayer->m_layerOriginalYData = yData;
            pLayer->m_type = layerType;
            pLayer->m_visible = false;
            pLayer->m_dimmedPen = QPen(dimmedPenColor);
            pLayer->m_dimmedBrush = QBrush(dimmedBrushColor);
            pLayer->m_highlightedPen = QPen(highlightPenColor);
            pLayer->m_highlightedBrush = QBrush(highlightBrushColor);
            pLayer->m_layerId = layerIndex;
            m_pNavigationChart->AddDataLayer(pLayer);
            GT_IF_WITH_ASSERT(m_pNavigateLayer[layerIndex] != nullptr)
            {
                m_pNavigateLayer[layerIndex]->setChecked(visible);
                SetCheckBoxColorIcon(m_pNavigateLayer[layerIndex], highlightBrushColor);
            }
        }
        else
        {
            pLayer->m_layerXData = xData;
            pLayer->m_layerYData = yData;
            pLayer->m_layerOriginalYData = yData;
        }
    }
}

void gpNavigationRibbon::SetCheckBoxColorIcon(QCheckBox* pCheckBox, QColor& color)
{
    GT_IF_WITH_ASSERT(pCheckBox != nullptr)
    {
        QPixmap* pIconPixmap = new QPixmap(10, 10);
        QPainter* pIconPainter = new QPainter(pIconPixmap);
        pIconPainter->fillRect(0, 0, 10, 10, Qt::white);
        pIconPainter->fillRect(0, 0, 10, 10, color);
        pCheckBox->setIcon(QIcon(*pIconPixmap));
    }
}

void gpNavigationRibbon::UpdateAxisData()
{
    // set the maximum range:
    GT_IF_WITH_ASSERT(m_pTimeLineGrid != nullptr)
    {
        quint64 range = m_pTimeLineGrid->fullRange();
        m_pNavigationChart->GetActiveRangeXAxis()->setRange(0, range);
    }
}

void gpNavigationRibbon::OnNavigateStateChanged(int state)
{
    GT_UNREFERENCED_PARAMETER(state);
    // the update of the layer will be done in the layer visibility event 
    EmitLayersVisibility();
}

void gpNavigationRibbon::OnRangeChangedByUser(const QPointF& rangePoint)
{
    GT_IF_WITH_ASSERT(m_pTimeLine != nullptr)
    {
        if (!m_inZoomOrPan)
        {
            m_inZoomOrPan = true;
            m_pTimeLine->SetVisibleRange(rangePoint.x(), rangePoint.y() - rangePoint.x());
        }

        m_inZoomOrPan = false;
    }
}

void gpNavigationRibbon::OnTimeLineZoomOrOffsetChanged()
{
    GT_IF_WITH_ASSERT(m_pNavigationChart != nullptr)
    {
        if (!m_inZoomOrPan)
        {
            m_inZoomOrPan = true;

            // the acTimeLine use the m_pTimeLine->fullRange() / m_pTimeLine->zoomFactor() to get the correct range for the scroll bar and not the m_pTimeLine->visibleRange()
            m_pNavigationChart->SetVisibleRange(m_pTimeLine->visibleStartTime() - m_pTimeLine->startTime(), m_pTimeLine->fullRange() / m_pTimeLine->zoomFactor());
        }

        m_inZoomOrPan = false;
    }
}

void gpNavigationRibbon::OnDisplayFiltersClicked()
{
    // open thread profiling settings dialog window
    GT_IF_WITH_ASSERT(m_pThreadFilterDialog != nullptr)
    {
        m_pThreadFilterDialog->DisplayDialog(m_pTimeLine->GetBranches());

        // if ok button was clicked - update the link label
        if (QDialog::Accepted == m_pThreadFilterDialog->exec())
        {
            emit m_pTimeLine->VisibilityFilterChanged(m_pThreadFilterDialog->getThreadVisibilityMap());
        }
    }
}

/// Received when timeline filters are changed
/// \param threadNameVisibilityMap - a map of thread names and visibility
void gpNavigationRibbon::OnTimelineFilterChanged(QMap<QString, bool>& threadNameVisibilityMap)
{
    GT_UNREFERENCED_PARAMETER(threadNameVisibilityMap);

    // calculate the data again
    // calculate API calls and Draw calls
    CalculateCalls();

    // force redrawing of the data
    OnTimeLineZoomOrOffsetChanged();
}

void gpNavigationRibbon::OnGroupChanged(int index)
{
    // The combo box change operation might take time, so we only emit a signal, and handle it in a slot, to make sure that the
    // UI is responsive when the high performance operation is executing
    GT_UNREFERENCED_PARAMETER(index);

    for (int nLayer = 0; nLayer < eNavigateLayersNum; nLayer++)
    {
        GT_IF_WITH_ASSERT(m_pNavigateLayer[nLayer] != nullptr)
        {
            bool isVisible = m_sLayerToGroupConnection[nLayer] == index;
            m_pNavigateLayer[nLayer]->setVisible(isVisible);
        }
        // the update of the layer will be done in the layer visibility event 
    }

    EmitLayersVisibility();
}


void gpNavigationRibbon::OnLayerVisibilityChanged(int visibleGroup, int visibleLayersByFlag)
{

    GT_UNREFERENCED_PARAMETER(visibleLayersByFlag);
    GT_IF_WITH_ASSERT((m_pNavigationChart != nullptr) && (m_pFrameDataCalculator != nullptr))
    {
        m_pNavigationChart->SetNavigationUnitsY(visibleGroup == eNavigateGroupDuration ? acNavigationChart::eNavigationNanoseconds : acNavigationChart::eNavigationSingleUnits);

        for (int nLayer = 0; nLayer < eNavigateLayersNum; nLayer++)
        {
            GT_IF_WITH_ASSERT(m_pNavigateLayer[nLayer] != nullptr)
            {
                m_pNavigateLayer[nLayer]->setVisible(m_sLayerToGroupConnection[nLayer] == visibleGroup);
            }
            m_pNavigationChart->SetLayerVisiblity(nLayer, m_pNavigateLayer[nLayer]->checkState() == Qt::Checked && m_sLayerToGroupConnection[nLayer] == visibleGroup);
        }

        m_pNavigationChart->UpdateYAxisRangeBasedOnVisibleLayers();

        // only for the visible group of duration do the min height calculation
        if (visibleGroup == eNavigateGroupDuration)
        {
            m_pNavigationChart->UpdateMinHeightValues();
        }

        // create the new present bars
        if (m_pPresentBars != nullptr)
        {
            m_pNavigationChart->removePlottable(m_pPresentBars);
        }

        // Add only the CPU present if there are any
        int presentSize = m_presentData.size();
        if (presentSize > 1)
        {
            QVector<double> xData;
            QVector<double> yData;
            xData.resize(presentSize - 1);
            yData.resize(presentSize - 1);
            for (int nPresent = 1; nPresent < presentSize; nPresent++)
            {
                double upperRange = m_pNavigationChart->yAxis->range().upper;
                yData[nPresent - 1] = upperRange;
                xData[nPresent - 1] = m_presentData[nPresent];
            }
            m_pPresentBars = new QCPBars(m_pNavigationChart->xAxis, m_pNavigationChart->yAxis);
            m_pPresentBars->setData(xData, yData);

            // set the display properties of the bar
            m_pPresentBars->setPen(gpUIManager::Instance()->DashedLinePen());
            m_pNavigationChart->addPlottable(m_pPresentBars);
        }

        emit m_pNavigationChart->RangeChangedByUser(m_pNavigationChart->GetCurrentRange());
    }
}

void gpNavigationRibbon::OnSetElementsNewWidth(int legendWidth, int timelineWidth)
{
    bool rc = true;

    // check all the check boxes are valid
    for (int i = 0; i < eNavigateLayersNum; i++)
    {
        rc = rc && (m_pNavigateLayer[i] != nullptr);
    }

    int realLegendWidth = legendWidth - NAVIGATION_ADDED_SPACE - VERICAL_BOX_ADDED_SPACE;
    GT_IF_WITH_ASSERT(rc && m_pGroupComboBox != nullptr && m_pNavigationChart != nullptr)
    {
        for (int i = 0; i < eNavigateLayersNum; i++)
        {
            m_pNavigateLayer[i]->setMinimumWidth(realLegendWidth);
            m_pNavigateLayer[i]->setMaximumWidth(realLegendWidth);
        }

        m_pGroupComboBox->setMinimumWidth(realLegendWidth);
        m_pGroupComboBox->setMaximumWidth(realLegendWidth);
        m_pNavigationChart->setFixedWidth(timelineWidth);
    }
}

void gpNavigationRibbon::EmitLayersVisibility()
{
    // Calculate the current visible group and filter
    CalculateCurrentVisibility();

    // Emit a signal that the visibility was changed
    emit LayerVisibilityChanged();
}

void gpNavigationRibbon::CalculateCurrentVisibility()
{
    m_visibleLayersByFlag = 0;
    m_visibleGroup = 0;

    GT_IF_WITH_ASSERT(m_pNavigationChart != nullptr)
    {
        for (int nLayer = 0; nLayer < eNavigateLayersNum; nLayer++)
        {
            acNavigationChartLayer* pCurrentLayer = m_pNavigationChart->GetLayerById(nLayer);

            if (pCurrentLayer != nullptr)
            {
                if (pCurrentLayer->m_visible)
                {
                    m_visibleLayersByFlag += (1 << nLayer);
                }
            }
        }
    }

    GT_IF_WITH_ASSERT(m_pGroupComboBox != nullptr)
    {
        m_visibleGroup = m_pGroupComboBox->currentIndex();
    }
}
void gpNavigationRibbon::GetCurrentVisibility(int& visibleGroup, int& visibleLayersByFlag)
{
    CalculateCurrentVisibility();

    visibleGroup = m_visibleGroup;
    visibleLayersByFlag = m_visibleLayersByFlag;
}

//////////////////////////////////////////////////////////////
gpFilterButton::gpFilterButton(QWidget* pParent) : QPushButton(pParent), m_isMouseIn(false)
{
    setStyleSheet(GP_StrFilterButtonStyle);
}

void gpFilterButton::enterEvent(QEvent* pEvent)
{
    // Call the base class implementation:
    QPushButton::enterEvent(pEvent);

    // Enable the button:
    setEnabled(true);

    // Make sure that the signal is fired only once:
    if (!m_isMouseIn)
    {
        // Emit a signal for button area enter:
        emit ButtonEnterLeave(true);
    }

    m_isMouseIn = true;
}

void gpFilterButton::leaveEvent(QEvent* pEvent)
{
    // Call the base class implementation:
    QPushButton::leaveEvent(pEvent);

    // Disable the button:
    setEnabled(false);

    // Make sure that the signal is fired only once:
    if (m_isMouseIn)
    {
        // Emit a signal for button area leave:
        emit ButtonEnterLeave(false);
    }

    m_isMouseIn = false;
}

void gpFilterButton::OnChartEnter(acNavigationChart* pNavigationChart)
{
    int iconSize = 16;
    QPoint topRight = QPoint(pNavigationChart->width(), 0);
    topRight.setX(topRight.x() - acScalePixelSizeToDisplayDPI(GP_NAVIGATION_CHART_RIGHT_MARGIN) - iconSize);
    topRight.setY(topRight.y() + acScalePixelSizeToDisplayDPI(GP_NAVIGATION_CHART_TOP_MARGIN) + iconSize / 2);

    QRect labelRect = QRect(topRight.x() - 5, topRight.y() + 5, iconSize, iconSize);
    setGeometry(labelRect);

    setVisible(true);
}

void gpFilterButton::OnChartLeave()
{
    setVisible(false);
}