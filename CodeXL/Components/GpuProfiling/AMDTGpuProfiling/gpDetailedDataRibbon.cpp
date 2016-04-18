//------------------------------ gpDetailedDataRibbon.cpp ------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>

// std
#include <algorithm>

// infra
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acNavigationChart.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimeline.h>
#include <AMDTApplicationComponents/Include/Timeline/acTimelineGrid.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>

// Local
#include <AMDTGpuProfiling/APIColorMap.h>
#include <AMDTGpuProfiling/gpDetailedDataRibbon.h>
#include <AMDTGpuProfiling/gpNavigationRibbon.h>
#include <AMDTGpuProfiling/gpRibbonDataCalculator.h>
#include <AMDTGpuProfiling/gpTraceDataContainer.h>
#include <AMDTGpuProfiling/gpTraceView.h>
#include <AMDTGpuProfiling/ProfileSessionDataItem.h>

#define LAYOUT_MARGINS 7

#define AXIS_RIGHT_MARGIN 5
#define AXIS_TOP_MARGIN 20

#define MAX_HIGHLIGHTED_API_CALLS 20
#define MAX_ALLOWED_API_CALLS_TO_DISPLAY 100000

#define FIXED_BARS_PEN_WIDTH 1

#define Y_BASE_DIVISION 20
#define Y_MID_DIVISION 50
#define Y_MAX_DIVISION 100
#define Y_DIVISION_LOW_RANGE 100
#define Y_DIVISION_HIGH_RANGE 1000
#define TOOLTIP_DISTANCE 3
#define DETAILED_RIBBON_MIN_HEIGHT 60

// transparency levels
#define LOW_LEVEL_TRANSPARENCY 128
#define HIGH_LEVEL_TRANSPARENCY 64

gpDetailedDataRibbon::gpDetailedDataRibbon(QWidget* pParent, gpRibbonDataCalculator* pCalculator) : QWidget(pParent),
    m_pOwningSession(nullptr),
    m_pCustomPlot(nullptr),
    m_pDummyLabel(nullptr),
    m_pInfoLabel(nullptr),
    m_pTimeLine(nullptr),
    m_pTimeLineGrid(nullptr),
    m_inZoomOrPan(false),
    m_visibleGroup(0),
    m_visibleLayersByFlag(0),
    m_pFrameDataCalculator(pCalculator),
    m_wasConcurrencyCalculated(false),
    m_canShowData(true)
{
    m_pOwningSession = (gpTraceView*)pParent;

    for (int i = 0; i < gpNavigationRibbon::eNavigateLayersNum; i++)
    {
        m_barsArray[i] = nullptr;
        m_maxValues[i] = 0;
    }

    InitLayout();

    m_toolTipsTitles = QString(GPU_STR_navigationLayerTooltips).split(",");
}

gpDetailedDataRibbon::~gpDetailedDataRibbon()
{

}

void gpDetailedDataRibbon::InitLayout()
{
    QHBoxLayout* pMainLayout = new QHBoxLayout(this);
    pMainLayout->setContentsMargins(0, 0, 0, 0);

    m_pDummyLabel = new QLabel("");
    QVBoxLayout* pVBox = new QVBoxLayout;

    // Create the navigation chart
    acNavigationChartInitialData initialChartData(QWIDGETSIZE_MAX, NAV_X_AXIS_AUTO_TICK_COUNT, NAV_FULL_RANGE_AXIS_TICKS_INTERVAL, DEFAULT_TIME_RANGE, ACTIVE_RANGE_XAXIS_WIDTH, ACTIVE_RANGE_YAXIS_WIDTH);
    QString label;
    m_pCustomPlot = new acNavigationChart(nullptr, label, &initialChartData);
    m_pCustomPlot->SetNavigationUnitsX(acNavigationChart::eNavigationNanoseconds);
    m_pCustomPlot->HideZoomControl();

    m_pInfoLabel = new QLabel(GP_Str_WarningCantDisplayDetailedRibbon);
    m_pInfoLabel->setVisible(false);
    QFont labelFont = m_pInfoLabel->font();
    labelFont.setPixelSize(30);
    m_pInfoLabel->setFont(labelFont);

    bool rc = connect(m_pCustomPlot, SIGNAL(afterReplot()), this, SLOT(OnAfterReplot()));
    GT_ASSERT(rc);

    // Set the navigation chart display range and margins
    GT_IF_WITH_ASSERT(m_pCustomPlot->axisRect() != nullptr)
    {
        m_pCustomPlot->axisRect()->setMargins(QMargins(GP_NAVIGATION_CHART_LEFT_MARGIN, GP_NAVIGATION_CHART_TOP_MARGIN, GP_NAVIGATION_CHART_RIGHT_MARGIN, 0));
        m_pCustomPlot->axisRect()->setAutoMargins(QCP::msNone);
    }
    pVBox->addWidget(m_pDummyLabel);

    pMainLayout->addLayout(pVBox);
    pMainLayout->addWidget(m_pCustomPlot);
    pMainLayout->addWidget(m_pInfoLabel);
    pMainLayout->addStretch(1);

    QCPAxis* pAxis = m_pCustomPlot->xAxis;
    GT_IF_WITH_ASSERT(pAxis != nullptr)
    {
        pAxis->setVisible(false);
    }

    setLayout(pMainLayout);
}

void gpDetailedDataRibbon::SetTimeLine(acTimeline* pTimeLine)
{
    GT_IF_WITH_ASSERT(pTimeLine != nullptr)
    {
        m_pTimeLine = pTimeLine;
        m_pTimeLineGrid = m_pTimeLine->grid();

        bool rc = connect(pTimeLine, SIGNAL(zoomFactorChanged()), this, SLOT(OnTimeLineZoomOrOffsetChanged()));
        GT_ASSERT(rc);

        rc = connect(pTimeLine, SIGNAL(offsetChanged()), this, SLOT(OnTimeLineZoomOrOffsetChanged()));
        GT_ASSERT(rc);
    }
}

void gpDetailedDataRibbon::CalculateData()
{
    if (m_pFrameDataCalculator != nullptr)
    {
        if (m_pFrameDataCalculator->GetTotalApiCalls() > MAX_ALLOWED_API_CALLS_TO_DISPLAY)
        {
            m_canShowData = false;
        }
    }

    if (m_canShowData)
    {
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

        GT_IF_WITH_ASSERT(m_pTimeLine != nullptr && m_pCustomPlot != nullptr && m_pCustomPlot->xAxis != nullptr)
        {
            // set the initial visible range of the custom plot
            double zoomFactor = m_pTimeLine->zoomFactor() != 0 ? m_pTimeLine->zoomFactor() : 1;
            m_pCustomPlot->xAxis->setRange(m_pTimeLine->visibleStartTime() - m_pTimeLine->startTime(), (m_pTimeLine->visibleStartTime() - m_pTimeLine->startTime() + m_pTimeLine->fullRange()) / zoomFactor);
        }
    }
    else
    {
        m_pCustomPlot->setVisible(false);
        m_pInfoLabel->setVisible(true);
    }
}

void gpDetailedDataRibbon::CalculateBuckets()
{
    // since the y values of the bins will not be displayed they will be set to 0
    GT_IF_WITH_ASSERT(m_pTimeLineGrid != nullptr)
    {
        QVector<double> binsYValues;
        binsYValues.resize(LAYER_DISPLAY_LIMIT);
        m_bucketVals.clear();
        double rangePerBucket = m_pTimeLineGrid->fullRange() * 1.0 / (LAYER_DISPLAY_LIMIT - 1);

        // Sanity check:
        GT_IF_WITH_ASSERT(rangePerBucket > 0)
        {
            for (int nBin = 0; nBin < LAYER_DISPLAY_LIMIT; nBin++)
            {
                binsYValues[nBin] = 0;
                // we need to shift the bucket position by half bucket width so the way we calculate items in the buckets
                // and the way qcustomplot display buckets will be the same:
                // we calculate items in range 0-bucket size and qcustomplot display -bucket size/2 - +bucket size/2
                m_bucketVals.push_back(rangePerBucket * nBin + rangePerBucket / 2);
            }
        }
    }
}

void gpDetailedDataRibbon::CalculateCalls()
{
    QVector<double> drawCallsInBuckets;
    QVector<double> apiCallsInBuckets;
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

        // get top 20 draw calls
        m_pFrameDataCalculator->GetTopCalls(apiCalls, highlightedApiCallsTime, highlightedApiCallsDuration, MAX_HIGHLIGHTED_API_CALLS);

        // add the detailed api calls
        QColor highlightColor = acLIGHT_GREEN;
        QColor dimmedColor = highlightColor;
        dimmedColor.setAlpha(LOW_LEVEL_TRANSPARENCY);
        AddBarsByDataTimeAndDuration(gpNavigationRibbon::eNavigateLayerCPUApiTime, apiCalls, highlightColor, dimmedColor);

        // add the top 20 draw calls
        highlightColor = acDARK_GREEN;
        AddBarsByDataTimeAndDuration(gpNavigationRibbon::eNavigateLayerTopCPUApiCalls, highlightedApiCallsTime, highlightedApiCallsDuration, highlightColor, highlightColor);

        // add api counts as buckets
        highlightColor = QColor(Qt::darkGray);
        dimmedColor = highlightColor;
        dimmedColor.setAlpha(LOW_LEVEL_TRANSPARENCY);
        AddBarsByBuckets(gpNavigationRibbon::eNavigateLayerApiCalls, apiCallsInBuckets, highlightColor, dimmedColor);

        // add draw calls count as buckets
        highlightColor = QColor(Qt::darkBlue);
        dimmedColor = highlightColor;
        dimmedColor.setAlpha(LOW_LEVEL_TRANSPARENCY);
        AddBarsByBuckets(gpNavigationRibbon::eNavigateLayerDrawCalls, drawCallsInBuckets, highlightColor, dimmedColor);
    }
}

void gpDetailedDataRibbon::CalculateGPUData()
{
    QVector<gpRibbonCallsData> gpuOpsCalls;

    QVector<double> highlightedGPUOpsTime;
    QVector<double> highlightedGPUOpsDuration;

    GT_IF_WITH_ASSERT(m_pFrameDataCalculator != nullptr)
    {
        m_pFrameDataCalculator->GetGPUOps(gpuOpsCalls);

        // get top 20 gpu ops calls
        m_pFrameDataCalculator->GetTopCalls(gpuOpsCalls, highlightedGPUOpsTime, highlightedGPUOpsDuration, MAX_HIGHLIGHTED_API_CALLS);

        // add the detailed GPU ops calls
        QColor highlightColor = acLIGHT_PURPLE;
        QColor dimmedColor = highlightColor;
        dimmedColor.setAlpha(LOW_LEVEL_TRANSPARENCY);
        AddBarsByDataTimeAndDuration(gpNavigationRibbon::eNavigateLayerGPUOpsTime, gpuOpsCalls, highlightColor, dimmedColor);

        // add the top 20 GPU ops calls
        highlightColor = acDARK_PURPLE;
        AddBarsByDataTimeAndDuration(gpNavigationRibbon::eNavigateLayerTopGPUCmdsCalls, highlightedGPUOpsTime, highlightedGPUOpsDuration, highlightColor, highlightColor);
    }
}

void gpDetailedDataRibbon::CalculateConcurrency()
{
    if (!m_wasConcurrencyCalculated)
    {
        GT_IF_WITH_ASSERT(m_pFrameDataCalculator != nullptr)
        {
            m_pFrameDataCalculator->CalculateCPUConcurrency();

            QVector<double> threadConcurrency = m_pFrameDataCalculator->CpuMaxThreadConcurrency();
            QVector<double> averageConcurrency = m_pFrameDataCalculator->CpuAverageThreadConcurrency();
            QVector<double> totalConcurrency = m_pFrameDataCalculator->CpuTotalThreadConcurrency();

            // add total thread concurrency
            QColor highlightColor = QColor(Qt::gray);
            QColor dimmedColor = highlightColor;
            highlightColor.setAlpha(LOW_LEVEL_TRANSPARENCY);
            dimmedColor.setAlpha(HIGH_LEVEL_TRANSPARENCY);
            AddBarsByBuckets(gpNavigationRibbon::eNavigateLayerTotalThreads, totalConcurrency, highlightColor, dimmedColor);

            // add threads concurrency
            highlightColor = QColor(Qt::darkMagenta);
            dimmedColor = highlightColor;
            dimmedColor.setAlpha(LOW_LEVEL_TRANSPARENCY);
            AddBarsByBuckets(gpNavigationRibbon::eNavigateLayerMaxThreads, threadConcurrency, highlightColor, dimmedColor);

            // add avg thread concurrency
            highlightColor = QColor(Qt::darkCyan);
            dimmedColor = highlightColor;
            dimmedColor.setAlpha(LOW_LEVEL_TRANSPARENCY);
            AddBarsByBuckets(gpNavigationRibbon::eNavigateLayerAvgThreads, averageConcurrency, highlightColor, dimmedColor);
        }

        m_wasConcurrencyCalculated = true;
    }
}

void gpDetailedDataRibbon::AddBarsByDataTimeAndDuration(int layer, const QVector<gpRibbonCallsData>& barsData, QColor penColor, QColor brushColor)
{
    QVector<double> barsTime;
    QVector<double> barsDuration;

    int numData = barsData.size();

    for (int nData = 0; nData < numData; nData++)
    {
        barsTime.push_back(barsData[nData].m_startTime);
        barsDuration.push_back(barsData[nData].m_duration);
    }

    AddBarsByDataTimeAndDuration(layer, barsTime, barsDuration, penColor, brushColor);
}

void gpDetailedDataRibbon::AddBarsByDataTimeAndDuration(int layer, const QVector<double>& barsTime, const QVector<double>& barsDuration, QColor penColor, QColor brushColor)
{
    AddBars(layer, barsTime, barsDuration, penColor, brushColor, eDetailedDetailedBars);
}

void gpDetailedDataRibbon::AddBarsByBuckets(int layer, QVector<double>& barsDuration, QColor penColor, QColor brushColor)
{
    AddBars(layer, m_bucketVals, barsDuration, penColor, brushColor, eDetailedBucketBars);
}

void gpDetailedDataRibbon::AddBars(int layer, const QVector<double>& barsTime, const QVector<double>& barsDuration, QColor penColor, QColor brushColor, DetaileBarsType barsType)
{
    GT_IF_WITH_ASSERT(m_bucketVals.size() > 0 && layer >= 0 && layer < gpNavigationRibbon::eNavigateLayersNum && m_pCustomPlot != nullptr)
    {
        // get the max duration of the data
        double maxDuration = 0;
        int numData = barsDuration.size();
        m_barsCacheData[layer].resize(numData);

        for (int nData = 0; nData < numData; nData++)
        {
            if (barsDuration[nData] > maxDuration)
            {
                maxDuration = barsDuration[nData];
            }

            // copy the data to the cache data needed for the tooltip mechanism
            m_barsCacheData[layer][nData].m_startTime = barsTime[nData];
            m_barsCacheData[layer][nData].m_duration = barsDuration[nData];
        }

        QCPBars* pBars = new QCPBars(m_pCustomPlot->xAxis, m_pCustomPlot->yAxis);

        QPen pen(penColor);
        QBrush brush(brushColor);
        pen.setWidth(FIXED_BARS_PEN_WIDTH);
        pBars->setPen(pen);
        pBars->setBrush(brush);

        if (eDetailedDetailedBars == barsType)
        {
            pBars->setWidthType(QCPBars::wtAbsolute);
            pBars->setWidth(1);
        }
        else if (eDetailedBucketBars == barsType)
        {
            pBars->setWidthType(QCPBars::wtPlotCoords);
            pBars->setWidth(m_pTimeLine->fullRange() / m_bucketVals.size());
        }

        pBars->setVisible(true);
        pBars->setData(barsTime, barsDuration);
        m_pCustomPlot->addPlottable(pBars);

        m_pCustomPlot->SetYAxisMaxRange(maxDuration);
        m_pCustomPlot->UpdateYAxisRange();

        GT_IF_WITH_ASSERT(layer >= 0 && layer < gpNavigationRibbon::eNavigateLayersNum)
        {
            // each layer can only be defined once
            GT_IF_WITH_ASSERT(m_barsArray[layer] == nullptr)
            {
                m_barsArray[layer] = pBars;
                m_maxValues[layer] = maxDuration;
            }
        }
    }
}

void gpDetailedDataRibbon::OnTimeLineZoomOrOffsetChanged()
{
    if (m_canShowData)
    {
        GT_IF_WITH_ASSERT(m_pCustomPlot != nullptr && m_pTimeLine != nullptr)
        {
            if (!m_inZoomOrPan)
            {
                m_inZoomOrPan = true;

                // the acTimeLine use the m_pTimeLine->fullRange() / m_pTimeLine->zoomFactor() to get the correct range for the scroll bar and not the m_pTimeLine->visibleRange()
                m_pCustomPlot->xAxis->setRange(m_pTimeLine->visibleStartTime() - m_pTimeLine->startTime(), m_pTimeLine->visibleStartTime() - m_pTimeLine->startTime() + m_pTimeLine->visibleRange());
                m_pCustomPlot->replot();
            }

            m_inZoomOrPan = false;
        }
    }
}

QString gpDetailedDataRibbon::GetTooltip(double iTime, QMouseEvent* pMouseEvent)
{
    GT_UNREFERENCED_PARAMETER(pMouseEvent);

    QString toolTipStr;

    if (m_canShowData)
    {

        GT_IF_WITH_ASSERT(m_pTimeLine != nullptr && m_pCustomPlot != nullptr)
        {
            // go through all the layers
            for (int nLayer = 0; nLayer < gpNavigationRibbon::eNavigateLayersNum; nLayer++)
            {
                QCPBars* pCurrentLayer = m_barsArray[nLayer];

                if (pCurrentLayer != nullptr && pCurrentLayer->visible() && m_barsCacheData[nLayer].size() > 0)
                {
                    // find the nearest draw call and return its size but only if its distance is less then 5 pixels
                    // find the nearest draw call and return its size but only if its distance is less then 5 pixels
                    QVector<gpBarsCacheData>::iterator lowBound;
                    QVector<gpBarsCacheData>::iterator highBound;
                    gpBarsCacheData searchData;
                    searchData.m_startTime = iTime;
                    lowBound = std::lower_bound(m_barsCacheData[nLayer].begin(), m_barsCacheData[nLayer].end(), searchData);
                    highBound = std::upper_bound(m_barsCacheData[nLayer].begin(), m_barsCacheData[nLayer].end(), searchData);
                    double timeAsPixel = m_pCustomPlot->xAxis->coordToPixel(iTime);

                    int itIndex = lowBound - m_barsCacheData[nLayer].begin();

                    if (lowBound == m_barsCacheData[nLayer].end())
                    {
                        itIndex--;
                    }

                    // move one place to the left to look one item before the tooltip time and one after and take the closest in the pixel range
                    if (itIndex != 0)
                    {
                        itIndex--;
                    }

                    double nearestDrawCall = m_barsCacheData[nLayer][itIndex].m_startTime;

                    if ((highBound != m_barsCacheData[nLayer].end()) && (fabs((*highBound).m_startTime - iTime)) < fabs((m_barsCacheData[nLayer][itIndex].m_startTime - iTime)))
                    {
                        itIndex = highBound - m_barsCacheData[nLayer].begin();
                        nearestDrawCall = m_barsCacheData[nLayer][itIndex].m_startTime;
                    }

                    double nearestCallAsPixel = m_pCustomPlot->xAxis->coordToPixel(nearestDrawCall);

                    int tooltipDistance = TOOLTIP_DISTANCE;

                    // default distance is a defined distance of pixels
                    // but if the bars are of QCPBars::wtPlotCoords type then use the bar half width coords to pixels distance
                    if (pCurrentLayer->widthType() == QCPBars::wtPlotCoords)
                    {
                        double barWidth = pCurrentLayer->width();
                        // convert to pixels (need to have a point as ref so we use nearest draw call and take half bar width)
                        tooltipDistance = m_pCustomPlot->xAxis->coordToPixel(nearestDrawCall + barWidth / 2) - m_pCustomPlot->xAxis->coordToPixel(nearestDrawCall);

                        // still need to have a minimum distance for tooltip
                        if (tooltipDistance < TOOLTIP_DISTANCE)
                        {
                            tooltipDistance = TOOLTIP_DISTANCE;
                        }
                    }

                    if (fabs(nearestCallAsPixel - timeAsPixel) < tooltipDistance && !m_toolTipsTitles[nLayer].isEmpty() && (m_barsCacheData[nLayer][itIndex].m_duration != 0))
                    {
                        QString  timeAsString = NanosecToTimeString(nearestDrawCall, false, false);
                        QString layerToolTipStr;
                        layerToolTipStr = QString(GP_STR_DrawCallToolTip).arg(m_toolTipsTitles[nLayer]).arg(m_pCustomPlot->TimeToString(m_pCustomPlot->GetNavigationUnitsY(), m_barsCacheData[nLayer][itIndex].m_duration, true));

                        if (!toolTipStr.isEmpty())
                        {
                            toolTipStr.append("<br>");
                        }

                        toolTipStr.append(layerToolTipStr);
                    }
                }
            }
        }
    }

    return toolTipStr;
}

void gpDetailedDataRibbon::OnSetElementsNewWidth(int legendWidth, int timelineWidth)
{
    int realLegendWidth = legendWidth - NAVIGATION_ADDED_SPACE - VERICAL_BOX_ADDED_SPACE;
    m_pDummyLabel->setFixedWidth(realLegendWidth);
    m_pCustomPlot->setFixedWidth(timelineWidth);
}

void gpDetailedDataRibbon::OnAfterReplot()
{
    emit AfterReplot();
}

void gpDetailedDataRibbon::OnLayerVisibilityChanged(int visibleGroup, int visibleLayersByFlag)
{
    if (m_canShowData)
    {
        // go through the parents until we reach the ribbon manager:
        acRibbonManager* pManager = nullptr;
        QWidget* pParent = qobject_cast<QWidget*>(parent());

        while (pParent != nullptr)
        {
            pManager = qobject_cast<acRibbonManager*>(pParent);

            if (pManager != nullptr)
            {
                break;
            }

            pParent = qobject_cast<QWidget*>(pParent->parent());
        }

        // set the ribbon name
        QString ribbonName;

        switch (visibleGroup)
        {
            case gpNavigationRibbon::eNavigateGroupDuration:
                ribbonName = GPU_STR_navigationGroupDuration;
                m_pCustomPlot->SetNavigationUnitsY(acNavigationChart::eNavigationNanoseconds);
                break;

            case gpNavigationRibbon::eNavigateGroupCount:
                ribbonName = GPU_STR_navigationGroupCount;
                m_pCustomPlot->SetNavigationUnitsY(acNavigationChart::eNavigationSingleUnits);
                break;

            case gpNavigationRibbon::eNavigateGroupThread:
            {
                CalculateConcurrency();
                ribbonName = GPU_STR_navigationGroupThreads;
                m_pCustomPlot->SetNavigationUnitsY(acNavigationChart::eNavigationSingleUnits);
                break;
            }

            default:
                GT_ASSERT(false);
                break;

        }

        pManager->ChangeRibbonName(this, ribbonName);

        double maximumVal = 0.0;

        // show/hide the bars
        for (int nLayer = 0; nLayer < gpNavigationRibbon::eNavigateLayersNum; nLayer++)
        {
            QCPBars* pCurrentBar = m_barsArray[nLayer];

            if (pCurrentBar != nullptr)
            {
                bool isVisible = visibleLayersByFlag & (1 << nLayer);
                pCurrentBar->setVisible(isVisible);

                if (pCurrentBar->visible() && m_maxValues[nLayer] > maximumVal)
                {
                    maximumVal = m_maxValues[nLayer];
                }
            }
        }

        GT_IF_WITH_ASSERT(m_pCustomPlot != nullptr)
        {
            m_pCustomPlot->SetYAxisMaxRange(maximumVal);
            m_pCustomPlot->UpdateYAxisRange();
            m_pCustomPlot->replot();
        }

        // we need to store those because the initial emit will come before the data is available
        // and we need to call this function again after the data is created
        m_visibleGroup = visibleGroup;
        m_visibleLayersByFlag = visibleLayersByFlag;
    }
}