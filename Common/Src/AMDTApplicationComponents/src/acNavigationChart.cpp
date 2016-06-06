//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acNavigationChart.cpp
///
//==================================================================================

//------------------------------ SessionNavigationChart.cpp ------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <QPolygon>
#include <qpoint.h>

// Infra:
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTBaseTools/Include/gtAssert.h>


// Local:
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acNavigationChart.h>
#include <AMDTApplicationComponents/Include/acDisplay.h>
#include <AMDTApplicationComponents/inc/acStringConstants.h>


#define NAVIGATION_CHART_MAX_LABEL_LEN 15

/// GUI constants for attributes relevant only for navigation chart:
static const int RANGE_HANDLES_PADDING = 32;
static const int RANGE_SHIFT_FROM_HANDLE_TOP = 30;
static const int RANGE_BOUNDING_LINE_WIDTH = 2;
static const int MINIMUM_ACTIVE_RANGE = 5000;
static const int INITIAL_YMAX = 20;
static const int RANGE_LABELS_Y_OFFSET = 1;

#define DEFAULT_SAMPLING_INTERVAL 100

#define NavigationChartTimeLabelStyle "QLabel {color: %1; font-size: 10px; font-weight: normal}"
#define NAV_TOP_MARGIN 10
#define NAV_FULL_RANGE_X_AXIS_PADDING 22

#define Y_BASE_DIVISION 5
#define Y_LOW_DIVISON 20
#define Y_MID_DIVISION 50
#define Y_MAX_DIVISION 100
#define Y_DIVISION_LOW_RANGE 10
#define Y_DIVISION_MID_RANGE 100
#define Y_DIVISION_HIGH_RANGE 1000
#define Y_NUMBER_TICKS 3

const Qt::PenStyle RANGE_BOUNDING_LINE_STYLE = Qt::SolidLine;

acNavigationChartLayer::acNavigationChartLayer() : m_layerId(-1), m_type(eNavigationLayerBar), m_visible(true), m_pGraphLine(nullptr)
{
    for (int i = 0; i < acNavigationChart::eNumHighlightedBars; i++)
    {
        m_pGraphBars[i] = nullptr;
    }
}

acNavigationChartLayer::~acNavigationChartLayer()
{
    if (m_pGraphLine != nullptr)
    {
        delete m_pGraphLine;
    }

    for (int i = 0; i < acNavigationChart::eNumHighlightedBars; i++)
    {
        if (m_pGraphBars[i] != nullptr)
        {
            delete m_pGraphBars[i];
        }
    }
}

acNavigationChartLayerNonIndex::acNavigationChartLayerNonIndex()
{

}

acNavigationChartLayerNonIndex::~acNavigationChartLayerNonIndex()
{
    if (m_pGraphLine != nullptr)
    {
        delete m_pGraphLine;
    }

}

acNavigationChart::acNavigationChart(QWidget* pParent,
    const QString& dataLabel,
    const acNavigationChartInitialData* initialData,
    acNavigationChartColorsData* colorsData) :
    QCustomPlot(pParent),
    m_pCurrentLeftRangeHandlePixmap(nullptr),
    m_pCurrentRightRangeHandlePixmap(nullptr),
    m_pRangePixmap(nullptr),
    m_pCurrentRangeLeftPixmap(nullptr),
    m_pCurrentRangeRightPixmap(nullptr),
    m_pCurrentRangeCenterPixmap(nullptr),
    m_pCurrentRangeStretchPixmap(nullptr),
    m_pActiveRangeBeginTimeLabel(nullptr),
    m_pActiveRangeTimeDeltaLabel(nullptr),
    m_pActiveRangeEndTimeLabel(nullptr),
    m_isLeftRangeHandleHovering(false),
    m_isLeftRangeHandlePressed(false),
    m_isRightRangeHandleHovering(false),
    m_isRightRangeHandlePressed(false),
    m_xAxisSelectedValueLow(-1),
    m_xAxisSelectedValueHigh(-1),
    m_isRangePressed(false),

    m_isRangeHovering(false),

    m_qstrAcquisitionDataLabel(dataLabel),
    m_minY(-1),
    m_maxY(-1),
    m_maxYSoFar(0.0),
    m_rangeState(RANGE_STATE_MID_END),
    m_leftHandleDataOffset(0.0),
    m_rightHandleDataOffset(0.0),
    m_samplingInterval(DEFAULT_SAMPLING_INTERVAL),
    m_pAllSessionGraph(nullptr),
    m_pAllSessionXAxis(nullptr),
    m_pActiveRangeXAxis(nullptr),
    m_pYAxis(nullptr),
    m_pFullRangeStaticTicksAxis(nullptr),
    m_defaultTimeRange(DEFAULT_TIME_RANGE),
    m_minimumRange(MINIMUM_ACTIVE_RANGE),
    m_unitsX(eNavigationMilliseconds),
    m_unitsY(eNavigationMilliseconds),
    m_unitDisplay(eNavigationDisplayAll),
    m_zoomCtrlEnabled(true),
    m_shouldUseTimelineSync(false),
    m_showTimelineSync(false),
    m_timelineSyncPos(0)
{
    // if initial data is null - create a default data
    if (initialData == nullptr)
    {
        initialData = new acNavigationChartInitialData();
    }

    // if colors data is null - create a default data
    if (colorsData != nullptr)
    {
        m_pColorsData = colorsData;
    }
    else
    {
        m_pColorsData = new acNavigationChartColorsData();
    }

    m_xAxisLowHighSelectedValuesDelta = initialData->m_xAxisLowHighValuesDelta;
    m_chartHeight = initialData->m_chartHeight;
    m_tickCount = initialData->m_tickCount;
    m_fullRangeTickInterval = initialData->m_fullRangeTickInterval;
    m_activeRangeXAxisWidth = initialData->m_activeRangeXAxisWidth;
    m_activeRangeYAxisWidth = initialData->m_activeRangeYAxisWidth;

    m_dragOffset = 0;
    m_rangeState = RANGE_STATE_DEFAULT;

    // plottables do not need the legend and using this with our derived classes can cause a crash for some reason
    setAutoAddPlottableToLegend(false);

    // Depending on shape of the range control handle set the data offset:
    // 0.0 = left side of the handle
    // handleWidth/2 = middle of the handle
    // handleWidth = right side of the handle will point to the data on xAxis
    LoadRangeControlImages();

    // add 2 graphs for inactive and active range
    InitGraphs();

    m_currentRange = QCPRange(0, 0);

    if (nullptr != m_pAllSessionGraph)
    {
        m_pAllSessionGraph->SetVectorData(m_vXData, m_vYData);
    }

    m_pActiveRangeBeginTimeLabel = new QLabel(this);
    m_pActiveRangeTimeDeltaLabel = new QLabel(this);
    m_pActiveRangeEndTimeLabel = new QLabel(this);

    QString styleSheetStr = QString(NavigationChartTimeLabelStyle).arg(m_pColorsData->m_axidTicksLabelColor.name());
    QColor orange(acQAMD_ORANGE_PRIMARY_COLOUR);
    QString styleSheetStrOrange = QString(NavigationChartTimeLabelStyle).arg(orange.name());

    m_pActiveRangeBeginTimeLabel->setStyleSheet(styleSheetStr);
    m_pActiveRangeTimeDeltaLabel->setStyleSheet(styleSheetStrOrange);
    m_pActiveRangeEndTimeLabel->setStyleSheet(styleSheetStr);

    replot();

    bool rc = connect(this, SIGNAL(RangeChangedByUser(const QPointF&)), this, SLOT(OnRangeChangeEnded(const QPointF&)));
    GT_ASSERT(rc);

    // set the default margins:
    QMargins margins(AC_NAVIGATION_CHART_MARGINS.left(), AC_NAVIGATION_CHART_MARGINS.top(), AC_NAVIGATION_CHART_MARGINS.right(), AC_NAVIGATION_CHART_MARGINS.bottom());

    // Set the margins to the navigation chart:
    GT_IF_WITH_ASSERT(xAxis != nullptr && yAxis != nullptr  && xAxis->axisRect() != nullptr && yAxis->axisRect() != nullptr)
    {
        yAxis->axisRect()->setAutoMargins(QCP::msNone);
        xAxis->axisRect()->setAutoMargins(QCP::msNone);
        yAxis->axisRect()->setMargins(margins);
        xAxis->axisRect()->setMargins(margins);
    }

    UpdateYAxisRange();
}

acNavigationChart::~acNavigationChart()
{
}

void acNavigationChart::paintEvent(QPaintEvent* event)
{
    QCustomPlot::paintEvent(event);

    QPainter painter(this);

    if (m_zoomCtrlEnabled)
    {
        bool shouldDrawBoundingLine = ShouldDrawBoundingLine();

        if (shouldDrawBoundingLine)
        {
            DrawRangeBoundingLine(painter);
            DrawRangeControl(painter);
        }
    }

    if (m_showTimelineSync)
    {
        DrawTimelineSync(painter);
    }
}

void acNavigationChart::resizeEvent(QResizeEvent* event)
{
    QCustomPlot::resizeEvent(event);

    if (!m_vXData.isEmpty())
    {
        //calculate number of labels: highPix-lowPix/interval
        m_leftHandlePosition = QPoint(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueLow) - m_leftHandleDataOffset + 1, m_pAllSessionXAxis->axisRect()->top() + RANGE_HANDLES_PADDING);
        m_rightHandlePosition = QPoint(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueHigh) - m_rightHandleDataOffset + 1, m_pAllSessionXAxis->axisRect()->top() + RANGE_HANDLES_PADDING);
        SetStaticTickLabels();
        SetActiveRangeXAxisTickLabels();
        GT_IF_WITH_ASSERT(nullptr != m_pAllSessionGraph)
        {
            if (!m_pAllSessionGraph->VectorData()->empty())
            {
                HighlightActiveRange();
            }
        }
    }

    int left;
    int right;
    GetBounds(left, right);

    emit BoundingChanged(left, right);
}

void acNavigationChart::enterEvent(QEvent* pEvent)
{
    QCustomPlot::enterEvent(pEvent);
    emit ChartEnter(this);
}

void acNavigationChart::leaveEvent(QEvent* pEvent)
{
    QCustomPlot::leaveEvent(pEvent);
    emit ChartLeave();
}

void acNavigationChart::GetBounds(int& lower, int& upper)
{
    // the bounds are send in the navigation coords. the receiver must convert it to its coord system. this is simpler
    // since the sender can't know if it is in a parent or not and if it should convert the units or not.
    int rightMargin = (axisRect()->minimumMargins().right() > 0 ? axisRect()->minimumMargins().right() - 1 : 0);
    lower = m_pActiveRangeXAxis->axisRect()->left();                 // match the drawing of handle
    upper = m_pActiveRangeXAxis->axisRect()->right() + 1 + rightMargin; // match the drawing of the handle
}

void acNavigationChart::mouseMoveEvent(QMouseEvent* event)
{
    QPoint mousePos = event->pos();

    if (m_zoomCtrlEnabled)
    {
        if (!m_vXData.isEmpty())
        {
            // check if mouse cursor is over range control handle:
            if (IsOverLeftHandle(mousePos))
            {
                if (!m_isLeftRangeHandleHovering) // if over handle and hovering flag is off turn it on  and put hover image
                {
                    if (!m_isLeftRangeHandlePressed && !m_isRightRangeHandlePressed && !m_isRangePressed)
                    {
                        m_isLeftRangeHandleHovering = true;
                        m_pCurrentLeftRangeHandlePixmap = &m_navigationPixmaps.m_leftRangeHandlePixmapHov;
                        repaint();
                    }
                }
            }
            else
            {
                if (m_isLeftRangeHandleHovering)
                {
                    m_isLeftRangeHandleHovering = false;

                    if (!m_isLeftRangeHandlePressed)
                    {
                        m_pCurrentLeftRangeHandlePixmap = &m_navigationPixmaps.m_leftRangeHandlePixmap;
                        repaint();
                    }
                }
            }

            if (IsOverRightHandle(mousePos))
            {
                if (!m_isRightRangeHandleHovering)
                {
                    if (!m_isLeftRangeHandlePressed && !m_isRightRangeHandlePressed && !m_isRangePressed)
                    {
                        m_isRightRangeHandleHovering = true;
                        m_pCurrentRightRangeHandlePixmap = &m_navigationPixmaps.m_rightRangeHandlePixmapHov;
                        repaint();
                    }
                }
            }
            else
            {
                if (m_isRightRangeHandleHovering)
                {
                    m_isRightRangeHandleHovering = false;

                    if (!m_isRightRangeHandlePressed)
                    {
                        m_pCurrentRightRangeHandlePixmap = &m_navigationPixmaps.m_rightRangeHandlePixmap;
                        repaint();
                    }
                }
            }

            if (IsOverRange(mousePos))
            {
                if (!m_isRangeHovering) // if hovering flag is off turn it on  and use hover image
                {
                    if (!m_isRangePressed && !m_isLeftRangeHandlePressed && !m_isRightRangeHandlePressed)
                    {
                        m_isRangeHovering = true;
                        m_pCurrentRangeLeftPixmap = &m_navigationPixmaps.m_rangePixmapLeftHov;
                        m_pCurrentRangeCenterPixmap = &m_navigationPixmaps.m_rangePixmapCenterHov;
                        m_pCurrentRangeRightPixmap = &m_navigationPixmaps.m_rangePixmapRightHov;
                        m_pCurrentRangeStretchPixmap = &m_navigationPixmaps.m_rangePixmapStretchHov;
                        m_pCurrentLeftRangeHandlePixmap = &m_navigationPixmaps.m_leftRangeHandlePixmapHov;
                        m_pCurrentRightRangeHandlePixmap = &m_navigationPixmaps.m_rightRangeHandlePixmapHov;
                        repaint();
                    }
                }
            }
            else
            {
                if (m_isRangeHovering)
                {
                    m_isRangeHovering = false;

                    if (!m_isRangePressed)
                    {
                        m_pCurrentRangeLeftPixmap = &m_navigationPixmaps.m_rangePixmapLeft;
                        m_pCurrentRangeCenterPixmap = &m_navigationPixmaps.m_rangePixmapCenter;
                        m_pCurrentRangeRightPixmap = &m_navigationPixmaps.m_rangePixmapRight;
                        m_pCurrentRangeStretchPixmap = &m_navigationPixmaps.m_rangePixmapStretch;

                        if (!m_isLeftRangeHandleHovering)
                        {
                            m_pCurrentLeftRangeHandlePixmap = &m_navigationPixmaps.m_leftRangeHandlePixmap;
                        }

                        if (!m_isRightRangeHandleHovering)
                        {
                            m_pCurrentRightRangeHandlePixmap = &m_navigationPixmaps.m_rightRangeHandlePixmap;
                        }

                        repaint();
                    }
                }
            }

            if ((event->buttons() & Qt::LeftButton)) // if left mouse button is pressed while moving
            {
                if (m_isLeftRangeHandlePressed)
                {
                    DragLeftHandleTo(mousePos);
                }
                else if (m_isRightRangeHandlePressed)
                {
                    DragRightHandleTo(mousePos);
                }
                else if (m_isRangePressed)
                {
                    DragRangeTo(mousePos.x());
                }

                // While dragging, set a hand cursor:
                setCursor(Qt::PointingHandCursor);

                if (m_currentRange != QCPRange(m_xAxisSelectedValueLow, m_xAxisSelectedValueHigh))
                {
                    // Emit a range changed signal:
                    emit RangeChangedByUser(QPointF(m_xAxisSelectedValueLow, m_xAxisSelectedValueHigh));

                    m_currentRange = QCPRange(m_xAxisSelectedValueLow, m_xAxisSelectedValueHigh);
                    SetRangeState();
                    SetActiveRangeXAxisTickLabels();
                    GT_IF_WITH_ASSERT(nullptr != m_pAllSessionGraph)
                    {
                        if (!m_pAllSessionGraph->VectorData()->empty())
                        {
                            HighlightActiveRange();
                        }
                    }
                }
            }
        }
    }

    if (m_shouldUseTimelineSync)
    {
        double realPos = m_pAllSessionXAxis->pixelToCoord(mousePos.x());
        // check if the mouse is between the two range control. if it is show the timeline sync and emit a signal
        if (realPos >= m_xAxisSelectedValueLow && realPos <= m_xAxisSelectedValueHigh)
        {
            m_timelineSyncPos = realPos;
            m_showTimelineSync = true;
            emit ShowTimeLine(m_showTimelineSync, m_timelineSyncPos);
            repaint();
        }
        else
        {
            bool shouldReapint = m_showTimelineSync;
            m_showTimelineSync = false;
            if (shouldReapint)
            {
                emit ShowTimeLine(m_showTimelineSync, m_timelineSyncPos);
                repaint();
            }
        }
    }
}

void acNavigationChart::mousePressEvent(QMouseEvent* event)
{
    if (m_zoomCtrlEnabled)
    {
        m_lastPoint = event->pos();

        if (event->button() == Qt::LeftButton)
        {
            if (IsOverLeftHandle(m_lastPoint))
            {
                // calculate offset as (mousePos - leftHandlePos)
                m_dragOffset = m_lastPoint.x() - m_leftHandlePosition.x();
                m_isLeftRangeHandlePressed = true;
                m_pCurrentLeftRangeHandlePixmap = &m_navigationPixmaps.m_leftRangeHandlePixmapPressed;
            }
            else if (IsOverRightHandle(m_lastPoint))
            {
                m_dragOffset = m_lastPoint.x() - m_rightHandlePosition.x();
                m_isRightRangeHandlePressed = true;
                m_pCurrentRightRangeHandlePixmap = &m_navigationPixmaps.m_rightRangeHandlePixmapPressed;
            }
            else if (IsOverRange(m_lastPoint))
            {
                m_dragOffset = m_lastPoint.x() - m_leftHandlePosition.x();
                m_isRangePressed = true;
                m_pCurrentRangeLeftPixmap = &m_navigationPixmaps.m_rangePixmapLeftPressed;
                m_pCurrentRangeCenterPixmap = &m_navigationPixmaps.m_rangePixmapCenterPressed;
                m_pCurrentRangeRightPixmap = &m_navigationPixmaps.m_rangePixmapRightPressed;
                m_pCurrentRangeStretchPixmap = &m_navigationPixmaps.m_rangePixmapStretchPressed;
                // both handles should be pressed when dragging the range
                m_pCurrentLeftRangeHandlePixmap = &m_navigationPixmaps.m_leftRangeHandlePixmapPressed;
                m_pCurrentRightRangeHandlePixmap = &m_navigationPixmaps.m_rightRangeHandlePixmapPressed;
            }
        }
    }

    QCustomPlot::mousePressEvent(event);
    repaint();
}

void acNavigationChart::wheelEvent(QWheelEvent* pEvent)
{
    if (m_zoomCtrlEnabled)
    {
        QVector<double>::iterator lastData = m_vXData.end();
        lastData--;
        Qt::MouseButtons btns = pEvent->buttons();
        QCustomPlot::wheelEvent(pEvent);

        // The zoomed calculated range:
        QCPRange calculatedZoomRange(m_xAxisSelectedValueLow, m_xAxisSelectedValueHigh);

        if (btns == Qt::NoButton)
        {
            bool shouldZoomIn = (pEvent->angleDelta().y() > 0); // rotated away from user = zoom in the range

            if (m_isRangeHovering)
            {
                m_pCurrentRangeLeftPixmap = &m_navigationPixmaps.m_rangePixmapLeftPressed;
                m_pCurrentRangeCenterPixmap = &m_navigationPixmaps.m_rangePixmapCenterPressed;
                m_pCurrentRangeRightPixmap = &m_navigationPixmaps.m_rangePixmapRightPressed;
                m_pCurrentRangeStretchPixmap = &m_navigationPixmaps.m_rangePixmapStretchPressed;
                m_pCurrentLeftRangeHandlePixmap = &m_navigationPixmaps.m_leftRangeHandlePixmapPressed;
                m_pCurrentRightRangeHandlePixmap = &m_navigationPixmaps.m_rightRangeHandlePixmapPressed;

                CalculateRangeAfterZoom(shouldZoomIn, calculatedZoomRange);

            }
            else if (m_isLeftRangeHandleHovering)
            {
                m_pCurrentLeftRangeHandlePixmap = &m_navigationPixmaps.m_leftRangeHandlePixmapPressed;

                if (shouldZoomIn)
                {
                    if (m_xAxisSelectedValueLow > m_vXData[0])
                    {
                        calculatedZoomRange.lower = m_xAxisSelectedValueLow - m_samplingInterval;
                    }
                }
                else
                {
                    if ((m_xAxisSelectedValueHigh - m_xAxisSelectedValueLow) > m_samplingInterval)
                    {
                        calculatedZoomRange.lower = m_xAxisSelectedValueLow + m_samplingInterval;
                    }
                }
            }
            else if (m_isRightRangeHandleHovering)
            {
                m_pCurrentRightRangeHandlePixmap = &m_navigationPixmaps.m_rightRangeHandlePixmapPressed;

                if (shouldZoomIn)
                {
                    if (m_xAxisSelectedValueHigh < *lastData)
                    {
                        calculatedZoomRange.upper = m_xAxisSelectedValueHigh + m_samplingInterval;
                    }
                }
                else
                {
                    if ((m_xAxisSelectedValueHigh - m_xAxisSelectedValueLow) > m_samplingInterval)
                    {
                        calculatedZoomRange.upper = m_xAxisSelectedValueHigh - m_samplingInterval;
                    }
                }
            }
        }

        // Apply the new range calculate for the zoom out / in:
        ApplyRangeAfterZoom(calculatedZoomRange);
    }
}

void acNavigationChart::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_zoomCtrlEnabled)
    {
        QPoint mousePos = event->pos();

        if (IsOverLeftHandle(mousePos))
        {
            m_isLeftRangeHandleHovering = true;
            m_pCurrentLeftRangeHandlePixmap = &m_navigationPixmaps.m_leftRangeHandlePixmapHov;
        }
        else
        {
            m_isLeftRangeHandleHovering = false;
            m_pCurrentLeftRangeHandlePixmap = &m_navigationPixmaps.m_leftRangeHandlePixmap;
        }

        if (IsOverRightHandle(mousePos))
        {
            m_isRightRangeHandleHovering = true;
            m_pCurrentRightRangeHandlePixmap = &m_navigationPixmaps.m_rightRangeHandlePixmapHov;
        }
        else
        {
            m_isRightRangeHandleHovering = false;
            m_pCurrentRightRangeHandlePixmap = &m_navigationPixmaps.m_rightRangeHandlePixmap;
        }

        if (IsOverRange(mousePos))
        {
            m_isRangeHovering = true;
            m_pCurrentRangeLeftPixmap = &m_navigationPixmaps.m_rangePixmapLeftHov;
            m_pCurrentRangeCenterPixmap = &m_navigationPixmaps.m_rangePixmapCenterHov;
            m_pCurrentRangeRightPixmap = &m_navigationPixmaps.m_rangePixmapRightHov;
            m_pCurrentRangeStretchPixmap = &m_navigationPixmaps.m_rangePixmapStretchHov;
            m_pCurrentLeftRangeHandlePixmap = &m_navigationPixmaps.m_leftRangeHandlePixmapHov;
            m_pCurrentRightRangeHandlePixmap = &m_navigationPixmaps.m_rightRangeHandlePixmapHov;
        }
        else
        {
            m_isRangeHovering = false;
            m_pCurrentRangeLeftPixmap = &m_navigationPixmaps.m_rangePixmapLeft;
            m_pCurrentRangeCenterPixmap = &m_navigationPixmaps.m_rangePixmapCenter;
            m_pCurrentRangeRightPixmap = &m_navigationPixmaps.m_rangePixmapRight;
            m_pCurrentRangeStretchPixmap = &m_navigationPixmaps.m_rangePixmapStretch;

            if (!m_isLeftRangeHandleHovering)
            {
                m_pCurrentLeftRangeHandlePixmap = &m_navigationPixmaps.m_leftRangeHandlePixmap;
            }

            if (!m_isRightRangeHandleHovering)
            {
                m_pCurrentRightRangeHandlePixmap = &m_navigationPixmaps.m_rightRangeHandlePixmap;
            }
        }

        GT_IF_WITH_ASSERT(nullptr != m_pActiveRangeXAxis)
        {
            QCPRange newRange = m_pActiveRangeXAxis->range();

            if (newRange.lower != m_xAxisSelectedValueLow || newRange.upper != m_xAxisSelectedValueHigh)
            {
                m_pActiveRangeXAxis->setRange(m_xAxisSelectedValueLow, m_xAxisSelectedValueHigh);
                SetActiveRangeXAxisTickLabels();
            }

            emit RangeChangedByUser(QPointF(m_xAxisSelectedValueLow, m_xAxisSelectedValueHigh));
            emit RangeChangedByUserEnded(QPointF(m_xAxisSelectedValueLow, m_xAxisSelectedValueHigh));
        }

        m_isLeftRangeHandlePressed = false;
        m_isRightRangeHandlePressed = false;
        m_isRangePressed = false;
        m_dragOffset = 0;
        repaint();

        // Reset to default cursor:
        setCursor(Qt::ArrowCursor);
    }
}

bool acNavigationChart::AddNewData(double key, double value, QCPRange& retRange, bool shouldReplot)
{
    bool rangeChanged = false;

    GT_IF_WITH_ASSERT(nullptr != m_pAllSessionGraph  && m_samplingInterval != 0)
    {
        m_pAllSessionGraph->AddDataToVector(key, value);
        m_vXData.push_back(key);
        m_vYData.push_back(value);

        if (value > m_maxYSoFar)
        {
            m_maxYSoFar = value;
            UpdateYAxisRange();
        }

        //if range hadn't been change by user show last 10 seconds
        QVector<double>::iterator lastData = m_vXData.end();
        lastData--;

        switch (m_rangeState)
        {
            case RANGE_STATE_START_END: // m_nLow unchanged m_nHigh = dKey
                m_xAxisSelectedValueHigh = (*lastData);
                rangeChanged = true;
                break;

            case RANGE_STATE_MID_END:
                m_xAxisSelectedValueHigh = (*lastData);
                m_xAxisSelectedValueLow = m_xAxisSelectedValueHigh - m_xAxisLowHighSelectedValuesDelta;
                rangeChanged = true;
                break;

            case RANGE_STATE_START_MID://constant range
            case RANGE_STATE_MID_MID:
                rangeChanged = false;
                break;

            default: //default - MID_END
                m_xAxisSelectedValueHigh = (*lastData);
                m_xAxisSelectedValueLow = m_xAxisSelectedValueHigh - m_xAxisLowHighSelectedValuesDelta;
                rangeChanged = true;
                break;
        }

        if (m_vXData.size() > GetDefaultTimeRange() / m_samplingInterval)
        {
            m_leftHandlePosition = QPoint(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueLow) - m_leftHandleDataOffset + 1, m_pAllSessionXAxis->axisRect()->top() + RANGE_HANDLES_PADDING);
            m_rightHandlePosition = QPoint(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueHigh) - m_rightHandleDataOffset + 1, m_pAllSessionXAxis->axisRect()->top() + RANGE_HANDLES_PADDING);
            m_pAllSessionXAxis->setRange(m_vXData[0], *lastData);
            m_pFullRangeStaticTicksAxis->setRange(m_vXData[0], *lastData);
        }
        else
        {
            m_pAllSessionXAxis->setRange((double)m_xAxisSelectedValueLow, *lastData);
            m_pFullRangeStaticTicksAxis->setRange((double)m_xAxisSelectedValueLow, *lastData);
        }

        SetStaticTickLabels();
        SetActiveRangeXAxisTickLabels();
        GT_IF_WITH_ASSERT(nullptr != m_pAllSessionGraph)
        {
            if (!m_pAllSessionGraph->VectorData()->empty())
            {
                if (shouldReplot)
                {
                    HighlightActiveRange();
                }
            }
        }
    }

    if (rangeChanged)
    {
        m_currentRange = retRange;
    }

    retRange = QCPRange(m_xAxisSelectedValueLow, m_xAxisSelectedValueHigh);
    return rangeChanged;
}

void acNavigationChart::DragLeftHandleTo(QPoint mousePos)
{
    GT_IF_WITH_ASSERT(nullptr != m_pAllSessionGraph)
    {
        acDataVector* pVectorData = m_pAllSessionGraph->VectorData();
        int vectorSize = pVectorData->size();

        if (vectorSize > 0)
        {
            double FirstPos = m_pAllSessionXAxis->coordToPixel(pVectorData->at(0).Key());
            double HighPos = m_pAllSessionXAxis->coordToPixel(double(m_xAxisSelectedValueHigh - GetMinimumRange()));
            int nearestIndex = -1;

            if (mousePos.x() <= FirstPos)
            {
                m_xAxisSelectedValueLow = pVectorData->at(0).Key();
            }
            else if (mousePos.x() > HighPos)
            {
                m_pAllSessionGraph->GetNearestIndexToKey(double(m_xAxisSelectedValueHigh - GetMinimumRange()), -1, nearestIndex);

                if ((nearestIndex >= 1) && (nearestIndex < (int)pVectorData->size()))
                {
                    m_xAxisSelectedValueLow = pVectorData->at(nearestIndex - 1).Key();
                }
            }
            else
            {
                double InterpolatedCoord = m_pAllSessionXAxis->pixelToCoord(mousePos.x());

                if (m_pAllSessionGraph->GetNearestIndexToKey(InterpolatedCoord, -1, nearestIndex))
                {
                    m_xAxisSelectedValueLow = m_pAllSessionGraph->VectorData()->at(nearestIndex).Key();

                    if (m_xAxisSelectedValueLow == m_xAxisSelectedValueHigh && nearestIndex > 0)
                    {
                        m_xAxisSelectedValueLow = m_pAllSessionGraph->VectorData()->at(nearestIndex - 1).Key();
                    }
                }
            }

            m_xAxisLowHighSelectedValuesDelta = m_xAxisSelectedValueHigh - m_xAxisSelectedValueLow;
        }

        m_leftHandlePosition.setX(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueLow) - m_leftHandleDataOffset + 1);
    }
}
void acNavigationChart::DragRightHandleTo(QPoint mousePos)
{

    acDataVector* pVectorData = m_pAllSessionGraph->VectorData();
    int vectorSize = pVectorData->size();

    if (vectorSize > 0)
    {
        double LastPos = m_pAllSessionXAxis->coordToPixel(pVectorData->at(vectorSize - 1).Key());
        double LowPos = m_pAllSessionXAxis->coordToPixel(double(m_xAxisSelectedValueLow + GetMinimumRange()));
        int nearestIndex = -1;

        if (mousePos.x() >= LastPos)
        {
            m_xAxisSelectedValueHigh = pVectorData->at(vectorSize - 1).Key();
        }
        else if (mousePos.x() < LowPos)
        {
            m_pAllSessionGraph->GetNearestIndexToKey(double(m_xAxisSelectedValueLow + GetMinimumRange()), -1, nearestIndex);
            m_xAxisSelectedValueHigh = pVectorData->at(nearestIndex + 1).Key();
        }
        else
        {
            double InterpolatedCoord = m_pAllSessionXAxis->pixelToCoord(mousePos.x());

            if (m_pAllSessionGraph->GetNearestIndexToKey(InterpolatedCoord, -1, nearestIndex))
            {
                m_xAxisSelectedValueHigh = m_pAllSessionGraph->VectorData()->at(nearestIndex).Key();

                if ((m_xAxisSelectedValueHigh == m_xAxisSelectedValueLow) && (nearestIndex < (vectorSize - 1)))
                {
                    m_xAxisSelectedValueHigh = m_pAllSessionGraph->VectorData()->at(nearestIndex + 1).Key();
                }
            }
        }

        m_xAxisLowHighSelectedValuesDelta = m_xAxisSelectedValueHigh - m_xAxisSelectedValueLow;
    }

    m_rightHandlePosition.setX(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueHigh) - m_rightHandleDataOffset + 1);
}

void acNavigationChart::DragRangeTo(const int& mouseXPosition)
{
    acDataVector* pVectorData = m_pAllSessionGraph->VectorData();
    int vectorSize = pVectorData->size();

    if (vectorSize > 0)
    {
        // Get the range info
        int indexLow = 0;
        int indexHigh = 0;
        m_pAllSessionGraph->GetNearestIndexToKey(m_xAxisSelectedValueLow, -1, indexLow);
        m_pAllSessionGraph->GetNearestIndexToKey(m_xAxisSelectedValueHigh, -1, indexHigh);

        int indexRange = indexHigh - indexLow;
        double upperLimit = pVectorData->at(vectorSize - 1).Key();
        int upperLimitIndex = vectorSize - 1;

        double lowerPix = m_pAllSessionXAxis->coordToPixel(pVectorData->at(0).Key());
        double higherPix = m_pAllSessionXAxis->coordToPixel(upperLimit);

        double rangePos = double(mouseXPosition - m_dragOffset); // position of lower end
        double highPos = rangePos + m_rightHandlePosition.x() - m_leftHandlePosition.x();

        if (rangePos < lowerPix)
        {
            m_xAxisSelectedValueLow = pVectorData->at(0).Key();
            m_xAxisSelectedValueHigh = pVectorData->at(indexRange).Key();  // check if nIndexRange can exceed
        }
        else if (highPos > higherPix)
        {
            m_xAxisSelectedValueHigh = pVectorData->at(upperLimitIndex).Key();
            m_xAxisSelectedValueLow = pVectorData->at(upperLimitIndex - indexRange).Key();
        }
        else
        {
            double interpolatedCoord = m_pAllSessionXAxis->pixelToCoord(rangePos);
            int interpolateCoordIndex;
            m_pAllSessionGraph->GetNearestIndexToKey(interpolatedCoord, -1, interpolateCoordIndex);
            // mouse position coordinate - now examine low and high ends
            int lowIndex = interpolateCoordIndex;

            if (lowIndex < 0)
            {
                lowIndex = 0;
            }

            int highIndex = lowIndex + indexRange;

            if (highIndex > upperLimitIndex)
            {
                highIndex = upperLimitIndex;
                lowIndex = highIndex - indexRange;
            }

            m_xAxisSelectedValueLow = pVectorData->at(lowIndex).Key();
            m_xAxisSelectedValueHigh = pVectorData->at(highIndex).Key();
        }

        m_leftHandlePosition.setX(int(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueLow) - m_leftHandleDataOffset + 1));
        m_rightHandlePosition.setX(int(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueHigh) - m_rightHandleDataOffset + 1));
    }
}

bool acNavigationChart::IsOverLeftHandle(const QPoint& mousePos) const
{
    bool bRes = false;
    // check if mouse cursor is over range control handle:
    QRect rectLeftRangeHandle = QRect(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueLow) - m_navigationPixmaps.m_leftRangeHandlePixmap.width(),
                                      m_pActiveRangeXAxis->axisRect()->bottom() - m_navigationPixmaps.m_leftRangeHandlePixmap.height() - 1,
                                      m_navigationPixmaps.m_leftRangeHandlePixmap.width(), m_navigationPixmaps.m_leftRangeHandlePixmap.height());

    if (rectLeftRangeHandle.contains(mousePos))
    {
        bRes = true;
    }

    return bRes;
}

bool acNavigationChart::IsOverRange(const QPoint& mousePos) const
{
    bool bRes = false;
    // check if mouse cursor is over range control time line:
    QRect rectRange = QRect(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueLow) + 1,
                            m_pActiveRangeXAxis->axisRect()->bottom() + 1,
                            m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueHigh) - m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueLow) - 1,
                            m_navigationPixmaps.m_rangePixmapCenter.height());

    if (rectRange.contains(mousePos))
    {
        bRes = true;
    }

    return bRes;
}


bool acNavigationChart::IsOverRightHandle(const QPoint& mousePos) const
{
    bool bRes = false;
    // check if mouse cursor is over range control handle:
    QRect rectRightRangeHandle = QRect(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueHigh),
                                       m_pActiveRangeXAxis->axisRect()->bottom() - m_navigationPixmaps.m_rightRangeHandlePixmap.height() - 1,
                                       m_navigationPixmaps.m_rightRangeHandlePixmap.rect().width(), m_navigationPixmaps.m_rightRangeHandlePixmap.height());

    if (rectRightRangeHandle.contains(mousePos))
    {
        bRes = true;
    }

    return bRes;
}

void acNavigationChart::DrawRangeBoundingLine(QPainter& painter)
{
    QPoint polyline[8];

    int rightMargin = (axisRect()->minimumMargins().right() > 0 ? axisRect()->minimumMargins().right() - 1 : 0);

    polyline[0] = QPoint(m_pActiveRangeXAxis->axisRect()->left(), m_pActiveRangeXAxis->axisRect()->bottom() + RANGE_SHIFT_FROM_HANDLE_TOP);// at the active range line left
    polyline[1] = QPoint(m_pActiveRangeXAxis->axisRect()->left(), m_pActiveRangeXAxis->axisRect()->bottom() + 3); // at the active range line left just below XAxis
    polyline[2] = QPoint(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueLow), m_pActiveRangeXAxis->axisRect()->bottom() + 3);// at the m_nLow just below X-axis
    polyline[3] = QPoint(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueLow), axisRect()->top() - 3); // at the m_nLow and x-Axis rect top
    polyline[4] = QPoint(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueHigh), axisRect()->top() - 3); // at the m_nHigh and x_Axis rect top
    polyline[5] = QPoint(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueHigh), m_pActiveRangeXAxis->axisRect()->bottom() + 3);// at the m_nHigh just below X-axis
    polyline[6] = QPoint(m_pActiveRangeXAxis->axisRect()->right() + 1 + rightMargin, m_pActiveRangeXAxis->axisRect()->bottom() + 3); // at the active range line right just below XAxis
    polyline[7] = QPoint(m_pActiveRangeXAxis->axisRect()->right() + 1 + rightMargin, m_pActiveRangeXAxis->axisRect()->bottom() + RANGE_SHIFT_FROM_HANDLE_TOP + 3);// at the active range line right
    QPen myPen(acQAMD_ORANGE_PRIMARY_COLOUR, RANGE_BOUNDING_LINE_WIDTH, Qt::SolidLine);
    UpdateActiveRangeBoundingLineTimeLabels(polyline[3].x(), polyline[4].x(), polyline[4].y());
    myPen.setJoinStyle(Qt::MiterJoin);
    painter.setPen(myPen);
    painter.drawPolyline(polyline, 8);
}

void acNavigationChart::DrawRangeControl(QPainter& painter)
{

    GT_IF_WITH_ASSERT(nullptr != m_pAllSessionXAxis)
    {
        int nStretchCount = m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueHigh) - m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueLow) - m_navigationPixmaps.m_rangePixmapCenter.width() - 2 * m_navigationPixmaps.m_rangePixmapLeft.width();
        int nRangeLeftPix = m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueLow);
        int nRangeLeftPixTest = xAxis->coordToPixel(m_xAxisSelectedValueLow);
        GT_ASSERT(nRangeLeftPix == nRangeLeftPixTest);
        int nRangeRightPix = m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueHigh);
        int nRangeTop = m_pActiveRangeXAxis->axisRect()->bottom() + 1;
        int nHandlesTop = m_pActiveRangeXAxis->axisRect()->bottom() - m_navigationPixmaps.m_leftRangeHandlePixmap.height() - 1;
        int nXSoFar = nRangeLeftPix + 1;

        // Calculate the positions to draw the range control:
        QPoint leftHandlePos(nRangeLeftPix - m_navigationPixmaps.m_leftRangeHandlePixmap.width(), nHandlesTop);
        QPoint leftRangePos(nXSoFar, nRangeTop);
        QPoint rightHandlePos(nRangeRightPix, nHandlesTop);

        nXSoFar += m_navigationPixmaps.m_rangePixmapLeft.width();
        QRect rectLeftStretch(nXSoFar, nRangeTop, nStretchCount / 2, m_navigationPixmaps.m_rangePixmapStretch.height());

        nXSoFar += rectLeftStretch.width();
        QPoint rangeCenterPos(nXSoFar, nRangeTop);

        nXSoFar += m_navigationPixmaps.m_rangePixmapCenter.width();
        QRect rectRightStretch(nXSoFar, nRangeTop, nStretchCount - nStretchCount / 2, m_navigationPixmaps.m_rangePixmapStretch.height());

        nXSoFar += rectRightStretch.width();
        QPoint rightRangePos(nXSoFar, nRangeTop);

        QRect geom = rect();
        bool isValid = (geom.contains(leftHandlePos) && geom.contains(leftRangePos) && geom.contains(rectLeftStretch) && geom.contains(rangeCenterPos)
                        && geom.contains(rectRightStretch) && geom.contains(rightRangePos) && geom.contains(rightHandlePos));

        GT_IF_WITH_ASSERT(isValid)
        {
            painter.drawPixmap(leftHandlePos, *m_pCurrentLeftRangeHandlePixmap);
            painter.drawPixmap(leftRangePos, *m_pCurrentRangeLeftPixmap);
            painter.drawPixmap(rectLeftStretch, *m_pCurrentRangeStretchPixmap);
            painter.drawPixmap(rangeCenterPos, *m_pCurrentRangeCenterPixmap);
            painter.drawPixmap(rectRightStretch, *m_pCurrentRangeStretchPixmap);
            painter.drawPixmap(rightRangePos, *m_pCurrentRangeRightPixmap);
            painter.drawPixmap(rightHandlePos, *m_pCurrentRightRangeHandlePixmap);
        }
    }
}

void acNavigationChart::HighlightActiveRange()
{
    GT_IF_WITH_ASSERT(nullptr != m_pAllSessionGraph)
    {
        if (!m_vXData.empty())
        {
            // clear old segments
            m_pAllSessionGraph->ClearSegments();

            // find the indexes of the low high points
            int lowIndex, highIndex;
            bool rcLow = m_pAllSessionGraph->GetNearestIndexToKey(m_xAxisSelectedValueLow, -1, lowIndex);
            bool rcHigh = m_pAllSessionGraph->GetNearestIndexToKey(m_xAxisSelectedValueHigh, -1, highIndex);

            // create the segments
            if (rcLow && rcHigh)
            {
                m_pAllSessionGraph->setPen(QPen(m_pColorsData->m_lineInactiveColor));
                m_pAllSessionGraph->setBrush(QBrush(m_pColorsData->m_fillInactiveColor));

                // add the segment before the highlight if there is one
                if (lowIndex > 0)
                {
                    m_pAllSessionGraph->AddSegment(lowIndex, QPen(m_pColorsData->m_lineInactiveColor), QBrush(m_pColorsData->m_fillInactiveColor));
                }

                // add highlight segment
                // We need to pre-blend the alpha into white, so that the alpha color does not
                // get applied to the gray fill color of the inactive range:
                QColor fillBrush(Qt::white);
                acBlendInto(fillBrush, m_pColorsData->m_fillActiveColor);

                if (highIndex > lowIndex)
                {
                    m_pAllSessionGraph->AddSegment(highIndex, QPen(m_pColorsData->m_lineActiveColor), QBrush(fillBrush));
                }

                // if there is a segment after the highlighted area
                if (highIndex < (int)m_pAllSessionGraph->VectorData()->size() - 1)
                {
                    m_pAllSessionGraph->AddSegment(m_pAllSessionGraph->VectorData()->size(), QPen(m_pColorsData->m_lineInactiveColor), QBrush(m_pColorsData->m_fillInactiveColor));
                }
            }

            replot();
        }
    }
}

void acNavigationChart::InitGraphs()
{
    m_pAllSessionGraph = acVectorLineGraph::AddGraph(this);

    axisRect()->setMinimumMargins(QMargins(0, NAV_TOP_MARGIN, 0, 0));

    GT_IF_WITH_ASSERT(nullptr != m_pAllSessionGraph)
    {
        m_pAllSessionGraph->setLineStyle(QCPGraph::lsLine);
        m_pAllSessionGraph->setPen(QPen(m_pColorsData->m_lineInactiveColor));
        m_pAllSessionGraph->setBrush(QBrush(m_pColorsData->m_fillInactiveColor));
        m_pAllSessionXAxis = xAxis;
        GT_IF_WITH_ASSERT(nullptr != m_pAllSessionXAxis)
        {
            m_pAllSessionXAxis->setVisible(false);
        }
    }

    // Get the default label font:
    QFont xAxisFont = xAxis->tickLabelFont();
    // Reduce font size
    xAxisFont.setPointSize(xAxisFont.pointSize() - 1);
    //Calculate font height in pixels for numbers
    int nXAxisFontHeight = QFontMetrics(xAxisFont).boundingRect("0").height();
    m_pFullRangeStaticTicksAxis = axisRect()->addAxis(QCPAxis::atBottom);
    GT_IF_WITH_ASSERT(nullptr != m_pFullRangeStaticTicksAxis)
    {
        m_pFullRangeStaticTicksAxis->setAutoTicks(false);
        m_pFullRangeStaticTicksAxis->setAutoTickLabels(false);
        m_pFullRangeStaticTicksAxis->setRange(0, m_samplingInterval);
        m_pFullRangeStaticTicksAxis->setAutoTickCount(m_tickCount);
        m_pFullRangeStaticTicksAxis->setTickLabelColor(m_pColorsData->m_axidTicksLabelColor);
        m_pFullRangeStaticTicksAxis->setBasePen(QPen(acQAMD_GRAY2_COLOUR, RANGE_BOUNDING_LINE_WIDTH, RANGE_BOUNDING_LINE_STYLE));
        m_pFullRangeStaticTicksAxis->setPadding(NAV_FULL_RANGE_X_AXIS_PADDING);
        m_pFullRangeStaticTicksAxis->setTickLabelFont(xAxisFont);
        int nFullRangeAxisTickLabelsPadding = m_pFullRangeStaticTicksAxis->tickLabelPadding();
        // Revert the padding, taking on account labels font height and xAxis width to show the labels above the xAxes
        nFullRangeAxisTickLabelsPadding = -1 * (nFullRangeAxisTickLabelsPadding + nXAxisFontHeight + m_activeRangeXAxisWidth + 1);
        m_pFullRangeStaticTicksAxis->setTickLabelPadding(nFullRangeAxisTickLabelsPadding);
    }

    m_pActiveRangeXAxis = axisRect()->addAxis(QCPAxis::atBottom);
    GT_IF_WITH_ASSERT(nullptr != m_pActiveRangeXAxis)
    {
        m_pActiveRangeXAxis->setAutoTicks(false);
        m_pActiveRangeXAxis->setAutoTickLabels(false);
        m_pActiveRangeXAxis->setRange(0, m_samplingInterval);
        m_pActiveRangeXAxis->setVisible(true);
        m_pActiveRangeXAxis->setBasePen(QPen(acQAMD_GRAY2_COLOUR, RANGE_BOUNDING_LINE_WIDTH, RANGE_BOUNDING_LINE_STYLE));
        m_pActiveRangeXAxis->setBasePen(QPen(m_pColorsData->m_axidTicksLabelColor, m_activeRangeXAxisWidth, RANGE_BOUNDING_LINE_STYLE));
        m_pActiveRangeXAxis->setTickLabelColor(m_pColorsData->m_axidTicksLabelColor);
        m_pActiveRangeXAxis->setTickLengthIn(0);
        m_pActiveRangeXAxis->setSubTickLengthIn(0);
        m_pActiveRangeXAxis->setSubTickLengthOut(5);
        m_pActiveRangeXAxis->setTickLabelFont(xAxisFont);
        int nActiveRangeAxisTickLabelsPadding = m_pActiveRangeXAxis->tickLabelPadding();
        nActiveRangeAxisTickLabelsPadding = -1 * (nActiveRangeAxisTickLabelsPadding + nXAxisFontHeight + m_activeRangeXAxisWidth + 1);
        m_pActiveRangeXAxis->setTickLabelPadding(nActiveRangeAxisTickLabelsPadding);
    }

    m_pYAxis = yAxis;
    GT_IF_WITH_ASSERT(nullptr != m_pYAxis)
    {
        m_pYAxis->setRange(0, INITIAL_YMAX);
        m_pYAxis->setBasePen(QPen(acQAMD_GRAY2_COLOUR, m_activeRangeYAxisWidth, RANGE_BOUNDING_LINE_STYLE));
        m_pYAxis->grid()->setVisible(false);
        m_pYAxis->setLabel(m_qstrAcquisitionDataLabel);
        QFont font = m_pYAxis->labelFont();
        font.setBold(true);
        font.setPixelSize(AC_NAVIGATION_CHART_FONT_SIZE);
        m_pYAxis->setLabelFont(font);
        m_pYAxis->setAutoTickStep(false);
        m_pYAxis->setAutoTicks(false);
        m_pYAxis->setAutoTickLabels(false);
        m_pYAxis->setTickStep(m_pYAxis->range().upper / 3);
        m_pYAxis->setAutoSubTicks(false);
        m_pYAxis->setSubTickCount(0);
        m_pYAxis->setTickLabelColor(Qt::black);
        m_pYAxis->setNumberFormat(QString("f"));
        m_pYAxis->setNumberPrecision(0);
        m_pYAxis->setTickLabelColor(m_pColorsData->m_axidTicksLabelColor);
        QFont m_pYAxisTickLabelFont = m_pYAxis->labelFont();
        m_pYAxisTickLabelFont.setBold(false);
        m_pYAxis->setTickLabelFont(m_pYAxisTickLabelFont);
    }
    setMinimumHeight(m_chartHeight);
    setMaximumHeight(m_chartHeight);
    setPlottingHints(QCP::phFastPolylines | QCP::phForceRepaint);
    m_vActiveRangeTicks.clear();
    m_vActiveRangeTickLabels.clear();
    m_vStaticTicks.clear();
    m_vStaticTickLabels.clear();
}


void acNavigationChart::PositionRangeControl(const QCPRange& range)
{
    if (range.lower != m_xAxisSelectedValueLow)
    {
        m_xAxisSelectedValueLow = range.lower;
        m_leftHandlePosition.setX(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueLow) - m_leftHandleDataOffset);
    }

    if (m_xAxisSelectedValueHigh != range.upper)
    {
        m_xAxisSelectedValueHigh = range.upper;
        m_rightHandlePosition.setX(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueHigh) - m_rightHandleDataOffset);
    }
}

void acNavigationChart::SetRangeState()
{
    QVector<double>::iterator lastData = m_vXData.end();
    lastData--;

    if (m_xAxisSelectedValueLow <= m_vXData[0] && m_xAxisSelectedValueHigh == *lastData)
    {
        m_rangeState = RANGE_STATE_START_END;
    }
    else if (m_xAxisSelectedValueLow == m_vXData[0] && m_xAxisSelectedValueHigh < *lastData)
    {
        m_rangeState = RANGE_STATE_START_MID;
    }
    else if (m_xAxisSelectedValueLow > m_vXData[0] && m_xAxisSelectedValueHigh < *lastData)
    {
        m_rangeState = RANGE_STATE_MID_MID;
    }
    else if (m_xAxisSelectedValueLow > m_vXData[0] && m_xAxisSelectedValueHigh == *lastData)
    {
        m_rangeState = RANGE_STATE_MID_END;
    }
}

void acNavigationChart::LoadRangeControlImages()
{
    m_navigationPixmaps.m_leftRangeHandlePixmap.load(QString(":/RangeHandleLeft.png"));
    m_navigationPixmaps.m_leftRangeHandlePixmapHov.load(QString(":/RangeHandleHovLeft.png"));
    m_navigationPixmaps.m_leftRangeHandlePixmapPressed.load(QString(":/RangeHandlePressedLeft.png"));
    m_navigationPixmaps.m_rightRangeHandlePixmap.load(QString(":/RangeHandleRight.png"));
    m_navigationPixmaps.m_rightRangeHandlePixmapHov.load(QString(":/RangeHandleHovRight.png"));
    m_navigationPixmaps.m_rightRangeHandlePixmapPressed.load(QString(":/RangeHandlePressedRight.png"));
    m_leftHandleDataOffset = m_navigationPixmaps.m_leftRangeHandlePixmap.width() / 2.0;
    m_rightHandleDataOffset = m_navigationPixmaps.m_rightRangeHandlePixmap.width() / 2.0;
    m_navigationPixmaps.m_rangePixmapCenter.load(QString(":/RangeCenter.png"));
    m_navigationPixmaps.m_rangePixmapLeft.load(QString(":/RangeLeft.png"));
    m_navigationPixmaps.m_rangePixmapRight.load(QString(":/RangeRight.png"));
    m_navigationPixmaps.m_rangePixmapStretch.load(QString(":/RangeStretch.png"));
    m_navigationPixmaps.m_rangePixmapLeftHov.load(QString(":/RangeLeftHov.png"));
    m_navigationPixmaps.m_rangePixmapCenterHov.load(QString(":/RangeCenterHov.png"));
    m_navigationPixmaps.m_rangePixmapRightHov.load(QString(":/RangeRightHov.png"));
    m_navigationPixmaps.m_rangePixmapStretchHov.load(QString(":/RangeStretchHov.png"));
    m_navigationPixmaps.m_rangePixmapLeftPressed.load(QString(":/RangeLeftPressed.png"));
    m_navigationPixmaps.m_rangePixmapRightPressed.load(QString(":/RangeRightPressed.png"));
    m_navigationPixmaps.m_rangePixmapCenterPressed.load(QString(":/RangeCenterPressed.png"));
    m_navigationPixmaps.m_rangePixmapStretchPressed.load(QString(":/RangeStretchPressed.png"));
    m_pCurrentRightRangeHandlePixmap = &m_navigationPixmaps.m_rightRangeHandlePixmap;
    m_pCurrentLeftRangeHandlePixmap = &m_navigationPixmaps.m_leftRangeHandlePixmap;
    m_pCurrentRangeLeftPixmap = &m_navigationPixmaps.m_rangePixmapLeft;
    m_pCurrentRangeRightPixmap = &m_navigationPixmaps.m_rangePixmapRight;
    m_pCurrentRangeCenterPixmap = &m_navigationPixmaps.m_rangePixmapCenter;
    m_pCurrentRangeStretchPixmap = &m_navigationPixmaps.m_rangePixmapStretch;
}

QString acNavigationChart::TimeToString(eNavigationUnits units, double time, bool shouldShowSmallestUnits, bool shouldUseWords)
{
    QString retStr;

    if (units == eNavigationNanoseconds)
    {
        retStr = NanosecToTimeString(time, shouldShowSmallestUnits, shouldUseWords, m_unitDisplay);
    }
    else if (units == eNavigationMilliseconds)
    {
        retStr = MsecToTimeString(time, shouldShowSmallestUnits, shouldUseWords);
    }
    else if (units == eNavigationMicroseconds)
    {
        retStr = MicroSecToTimeString(time, shouldShowSmallestUnits, shouldUseWords);
    }
    else if (units == eNavigationSingleUnits)
    {
        retStr = QString::number(time);
    }
    else
    {
        GT_ASSERT(false);
    }

    return retStr;
}

void acNavigationChart::SetStaticTickLabels()
{
    GT_IF_WITH_ASSERT(nullptr != m_pFullRangeStaticTicksAxis)
    {
        if (!m_vXData.empty())
        {
            int nNumOfStaticTicks = m_pFullRangeStaticTicksAxis->axisRect()->width() / m_fullRangeTickInterval;
            m_vStaticTicks.clear();
            m_vStaticTickLabels.clear();
            QVector<double>::iterator lastData = m_vXData.end();
            lastData--;
            double FirstTick = double(m_pFullRangeStaticTicksAxis->pixelToCoord(axisRect()->left()));
            double LastTick = double(m_pFullRangeStaticTicksAxis->pixelToCoord(axisRect()->right() + 1));
            double TickStep = (LastTick - FirstTick)  / (nNumOfStaticTicks - 1);
            double nValueStep = (LastTick - FirstTick)  / (nNumOfStaticTicks - 1);

            for (int i = 0; i < nNumOfStaticTicks; ++i)
            {
                m_vStaticTicks << FirstTick + i* TickStep;
                m_vStaticTickLabels << TimeToString(m_unitsX, FirstTick + i * nValueStep, false);
            }

            m_pFullRangeStaticTicksAxis->setTickVector(m_vStaticTicks);
            m_pFullRangeStaticTicksAxis->setTickVectorLabels(m_vStaticTickLabels);
        }
    }
}

void acNavigationChart::SetYAxisTickLabels()
{
    QVector<QString> yAxisStringVector;
    QVector<double> yAxisValueVector;

    GT_IF_WITH_ASSERT(m_pYAxis != nullptr)
    {
        double yRangeStep = m_pYAxis->range().size() / (Y_NUMBER_TICKS + 1);
        double yStart = m_pYAxis->range().lower;

        for (int nTick = 0; nTick < Y_NUMBER_TICKS + 2; nTick++)
        {
            yAxisValueVector << yStart + yRangeStep* nTick;
            yAxisStringVector << TimeToString(m_unitsY, yStart + yRangeStep * nTick, true);
        }

        m_pYAxis->setTickVector(yAxisValueVector);
        m_pYAxis->setTickVectorLabels(yAxisStringVector);
    }
}

void acNavigationChart::SetActiveRangeXAxisTickLabels()
{
    GT_IF_WITH_ASSERT(nullptr != m_pActiveRangeXAxis && nullptr != m_pFullRangeStaticTicksAxis)
    {
        if (!m_vXData.empty() && !m_pActiveRangeXAxis->autoTicks())
        {
            int nNumOfActiveTicks = m_pActiveRangeXAxis->axisRect()->width() / m_fullRangeTickInterval;
            m_vActiveRangeTicks.clear();
            m_vActiveRangeTickLabels.clear();
            double FirstTick = double(m_pActiveRangeXAxis->pixelToCoord(axisRect()->left()));
            double LastTick = double(m_pActiveRangeXAxis->pixelToCoord(axisRect()->right() + 1));
            double TickStep = (LastTick - FirstTick) / (nNumOfActiveTicks - 1);
            double nValueStep = (double(m_xAxisSelectedValueHigh) - double(m_xAxisSelectedValueLow)) / (nNumOfActiveTicks - 1);

            for (int i = 0; i < nNumOfActiveTicks; ++i)
            {
                m_vActiveRangeTicks << FirstTick + i* TickStep;
                m_vActiveRangeTickLabels << TimeToString(m_unitsX, m_xAxisSelectedValueLow + i * nValueStep, true);
            }

            m_pActiveRangeXAxis->setTickVector(m_vActiveRangeTicks);
            m_pActiveRangeXAxis->setTickVectorLabels(m_vActiveRangeTickLabels);
        }
    }
}
void acNavigationChart::SetOfflineData(const QVector<double>& xData, const QVector<double>& yData)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(!xData.isEmpty() && !yData.isEmpty() && xData.size() == yData.size() && nullptr != m_pAllSessionGraph)
    {
        m_vXData = xData;
        m_vYData = yData;
        QVector<double>::iterator lastData = m_vXData.end();
        lastData--;
        LoadRangeControlImages();
        m_pAllSessionGraph->SetVectorData(m_vXData, m_vYData);

        m_xAxisSelectedValueHigh = (*lastData);
        m_xAxisSelectedValueLow = xData[0];

        // if session is more than default time range, show only the default time range.
        if (m_xAxisSelectedValueHigh - m_xAxisSelectedValueLow > GetDefaultTimeRange())
        {
            m_xAxisSelectedValueLow = m_xAxisSelectedValueHigh - GetDefaultTimeRange();
        }

        SetRangeState();

        m_maxYSoFar = -DBL_MAX;

        for (auto it : m_vYData)
        {
            if (it > m_maxYSoFar)
            {
                m_maxYSoFar = it;
            }
        }

        UpdateYAxisRange();

        if (m_vXData.size() > GetDefaultTimeRange() / m_samplingInterval)
        {
            m_leftHandlePosition = QPoint(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueLow) - m_leftHandleDataOffset + 1, m_pAllSessionXAxis->axisRect()->top() + RANGE_HANDLES_PADDING);
            m_rightHandlePosition = QPoint(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueHigh) - m_rightHandleDataOffset + 1, m_pAllSessionXAxis->axisRect()->top() + RANGE_HANDLES_PADDING);
            m_pAllSessionXAxis->setRange(m_vXData[0], *lastData);
            m_pFullRangeStaticTicksAxis->setRange(m_vXData[0], *lastData);
            m_pActiveRangeXAxis->setRange(m_vXData[0], *lastData);
        }
        else
        {
            m_pAllSessionXAxis->setRange((double)m_xAxisSelectedValueLow, *lastData);
            m_pFullRangeStaticTicksAxis->setRange((double)m_xAxisSelectedValueLow, *lastData);
            m_pActiveRangeXAxis->setRange((double)m_xAxisSelectedValueLow, *lastData);
        }

        SetStaticTickLabels();
        SetActiveRangeXAxisTickLabels();
        GT_IF_WITH_ASSERT(nullptr != m_pAllSessionGraph)
        {
            if (!m_pAllSessionGraph->VectorData()->empty())
            {
                HighlightActiveRange();
            }
        } // note that while dragging replot is not called and graph is frozen
    }
}

void acNavigationChart::SetYAxisValues(int minY, int maxY, const QString& yAxisLabel)
{
    // Set the min + max Y axis values:
    m_minY = minY;
    m_maxY = maxY;

    // Reset the max Y axis value measured so far:
    m_maxYSoFar = 0;

    GT_IF_WITH_ASSERT(nullptr != m_pYAxis)
    {
        if (m_vXData.isEmpty())
        {
            m_pYAxis->setRange(0, INITIAL_YMAX);
        }
        else
        {
            m_pYAxis->setRange(0, m_maxY);
        }

        int len = yAxisLabel.length();

        if (len > NAVIGATION_CHART_MAX_LABEL_LEN)
        {
            // if label is too long.. chop it and add "..."
            QString chopedLabel(yAxisLabel);
            chopedLabel.chop(len - NAVIGATION_CHART_MAX_LABEL_LEN + 3);
            chopedLabel.append("...");
            m_pYAxis->setLabel(chopedLabel);
        }
        else
        {
            m_pYAxis->setLabel(yAxisLabel);
        }

        m_qstrAcquisitionDataLabel = yAxisLabel;
    }
}

void acNavigationChart::UpdateYAxisRange()
{
    GT_IF_WITH_ASSERT(m_pYAxis != nullptr)
    {
        double newHighest = m_maxYSoFar;
        // find the nearest number one log10 smaller
        int base10 = (int)floor(log10(m_maxYSoFar));

        if (base10 > 0)
        {
            double roundVal = pow(10, base10);
            newHighest = floor(newHighest / roundVal) * roundVal + roundVal;
        }
        else
        {
            // currently we don't handle ranges 0 or smaller
            if (m_maxYSoFar > 0)
            {
                double multiplier = 1;

                // check small range
                if (newHighest < 1)
                {
                    // find the negative log10
                    while (newHighest * multiplier < 1)
                    {
                        multiplier *= 10;
                    }
                }
            }
        }

        // in the single units mode make sure the number divides nicely into  the number of ticks we have
        if (eNavigationSingleUnits == m_unitsY)
        {
            newHighest = ceil(newHighest / (Y_NUMBER_TICKS + 1)) * (Y_NUMBER_TICKS + 1);
        }

        m_pYAxis->setRange(0, newHighest);
        m_pYAxis->setTickStep(m_maxYSoFar / Y_NUMBER_TICKS);
    }

    SetYAxisTickLabels();
}

double acNavigationChart::GetDefaultTimeRange()
{
    double defaultTime = m_defaultTimeRange;

    if (defaultTime <= 0)
    {
        int numData = m_vXData.count();
        defaultTime = m_vXData[numData - 1] - m_vXData[0];
    }

    return defaultTime;
}

double acNavigationChart::GetMinimumRange()
{
    double range = 0;

    if (m_minimumRange < 0)
    {
        int numData = m_vXData.count();
        GT_IF_WITH_ASSERT(numData > 0)
        {
            double deltaTime = m_vXData[numData - 1] - m_vXData[0];
            range = -1 * deltaTime * m_minimumRange;
        }
    }
    else if (m_minimumRange > 0)
    {
        range = m_minimumRange;
    }
    else
    {
        GT_ASSERT(false);
    }

    return range;
}

bool acNavigationChart::ShouldDrawBoundingLine()
{
    bool retVal = (m_samplingInterval > 0);

    if (retVal)
    {
        // interval is set, but samples arrival time can vary
        retVal = (m_vXData.size() >= 10000 / m_samplingInterval);
    }

    return retVal;
}

void acNavigationChart::UpdateActiveRangeBoundingLineTimeLabels(int startPosition, int endPosition, int yPosition)
{
    GT_IF_WITH_ASSERT(nullptr != m_pActiveRangeBeginTimeLabel && nullptr != m_pActiveRangeTimeDeltaLabel && nullptr != m_pActiveRangeEndTimeLabel)
    {
        QString qstrActiveRangeBeginTime(TimeToString(m_unitsX, m_xAxisSelectedValueLow , false));
        QString qstrActiveRangeTimeDelta(TimeToString(m_unitsX, (m_xAxisSelectedValueHigh - m_xAxisSelectedValueLow), false, true));
        QString qstrActiveRangeEndTime(TimeToString(m_unitsX, m_xAxisSelectedValueHigh , false));
        m_pActiveRangeBeginTimeLabel->setText(qstrActiveRangeBeginTime);
        m_pActiveRangeTimeDeltaLabel->setText(qstrActiveRangeTimeDelta);
        m_pActiveRangeEndTimeLabel->setText(qstrActiveRangeEndTime);

        m_pActiveRangeBeginTimeLabel->adjustSize();
        m_pActiveRangeTimeDeltaLabel->adjustSize();
        m_pActiveRangeEndTimeLabel->adjustSize();

        m_pActiveRangeBeginTimeLabel->setGeometry(startPosition,
                                                  yPosition - m_pActiveRangeBeginTimeLabel->height() - RANGE_LABELS_Y_OFFSET,
                                                  m_pActiveRangeBeginTimeLabel->width(),
                                                  m_pActiveRangeBeginTimeLabel->height());

        m_pActiveRangeTimeDeltaLabel->setGeometry(startPosition + (endPosition - startPosition) / 2 - m_pActiveRangeTimeDeltaLabel->width() / 2,
                                                  yPosition - m_pActiveRangeTimeDeltaLabel->height() - RANGE_LABELS_Y_OFFSET,
                                                  m_pActiveRangeTimeDeltaLabel->width(),
                                                  m_pActiveRangeTimeDeltaLabel->height());

        m_pActiveRangeEndTimeLabel->setGeometry(endPosition - m_pActiveRangeEndTimeLabel->width(),
                                                yPosition - m_pActiveRangeEndTimeLabel->height() - RANGE_LABELS_Y_OFFSET,
                                                m_pActiveRangeEndTimeLabel->width(),
                                                m_pActiveRangeEndTimeLabel->height());

        if (m_pActiveRangeBeginTimeLabel->geometry().right() < m_pActiveRangeTimeDeltaLabel->geometry().left())
        {
            m_pActiveRangeBeginTimeLabel->setVisible(true);
        }
        else
        {
            m_pActiveRangeBeginTimeLabel->setVisible(false);
        }

        if (m_pActiveRangeEndTimeLabel->geometry().left() > m_pActiveRangeTimeDeltaLabel->geometry().right())
        {
            m_pActiveRangeEndTimeLabel->setVisible(true);
        }
        else
        {
            m_pActiveRangeEndTimeLabel->setVisible(false);
        }
    }
}

void acNavigationChart::ApplyRangeAfterZoom(const QCPRange& rangeAfterZoom)
{
    // pass new range to position handles
    if (m_currentRange != rangeAfterZoom)
    {
        // Set the new range:
        m_currentRange = rangeAfterZoom;

        // Position the range control:
        PositionRangeControl(rangeAfterZoom);

        m_pActiveRangeXAxis->setRange(m_xAxisSelectedValueLow, m_xAxisSelectedValueHigh);
        SetRangeState();
        SetActiveRangeXAxisTickLabels();
        emit RangeChangedByUser(QPointF(m_xAxisSelectedValueLow, m_xAxisSelectedValueHigh));
        emit RangeChangedByUserEnded(QPointF(m_xAxisSelectedValueLow, m_xAxisSelectedValueHigh));
        GT_IF_WITH_ASSERT(nullptr != m_pAllSessionGraph)
        {
            if (!m_pAllSessionGraph->VectorData()->empty())
            {
                HighlightActiveRange();
            }
        }
    }
}

void acNavigationChart::CalculateRangeAfterZoom(bool shouldZoomIn, QCPRange& rangeAfterZoom)
{
    // Initialize to current range:
    rangeAfterZoom = m_currentRange;

    if (shouldZoomIn)
    {
        // Find the last data item:
        QVector<double>::iterator lastData = m_vXData.end();
        lastData--;

        if (m_xAxisSelectedValueLow > m_vXData[0])
        {
            rangeAfterZoom.lower = m_xAxisSelectedValueLow - m_samplingInterval;
        }

        if (m_xAxisSelectedValueHigh < *lastData)
        {
            rangeAfterZoom.upper = m_xAxisSelectedValueHigh + m_samplingInterval;
        }
    }
    else
    {
        // Zoom out the range:
        if (m_xAxisSelectedValueHigh - m_xAxisSelectedValueLow > m_samplingInterval)
        {
            rangeAfterZoom.lower = m_xAxisSelectedValueLow + m_samplingInterval;

            // Check if there is space to zoom out:
            if (m_xAxisSelectedValueHigh - rangeAfterZoom.lower > m_samplingInterval)
            {
                rangeAfterZoom.upper = m_xAxisSelectedValueHigh - m_samplingInterval;
            }
        }
    }
}

void acNavigationChart::SetVisibleRange(const double startVisibleTime, const double visibleRange)
{
    if (m_pAllSessionXAxis != nullptr)
    {
        m_xAxisSelectedValueLow = startVisibleTime;
        m_xAxisSelectedValueHigh = startVisibleTime + visibleRange;

        m_leftHandlePosition.setX(int(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueLow) - m_leftHandleDataOffset + 1));
        m_rightHandlePosition.setX(int(m_pAllSessionXAxis->coordToPixel(m_xAxisSelectedValueHigh) - m_rightHandleDataOffset + 1));

        m_currentRange = QCPRange(m_xAxisSelectedValueLow, m_xAxisSelectedValueHigh);

        m_pActiveRangeXAxis->setRange(m_xAxisSelectedValueLow, m_xAxisSelectedValueHigh);
    }

    SetRangeState();
    SetActiveRangeXAxisTickLabels();
    GT_IF_WITH_ASSERT(nullptr != m_pAllSessionGraph)
    {
        if (!m_pAllSessionGraph->VectorData()->empty())
        {
            HighlightActiveRange();
            OnRangeChangeEnded(QPointF(m_xAxisSelectedValueLow, m_xAxisSelectedValueHigh));
        }
    }
}

void acNavigationChart::OnRangeChangeEnded(const QPointF& newRange)
{
    // pass through all the layers and create the sub ranges if the layer is visible
    int numLayers = m_layersVector.size();

    for (int nLayer = 0; nLayer < numLayers; nLayer++)
    {
        // delete old layer plotable
        acNavigationChartLayer* pCurrentLayer = m_layersVector[nLayer];

        if (pCurrentLayer != nullptr)
        {
            RemoveLayerPlotable(pCurrentLayer);

            if (pCurrentLayer->m_visible)
            {
                // add the graphs based on the type of the layer:
                acNavigationChartLayerNonIndex* pNonIndexLayer = dynamic_cast<acNavigationChartLayerNonIndex*>(pCurrentLayer);

                if (pNonIndexLayer != nullptr)
                {
                    AddNonIndexedLayerGraphs(pNonIndexLayer);
                }
                else
                {
                    int lowIndex = 0;
                    int highIndex = m_vXData.size() - 1;
                    bool rcLow = m_pAllSessionGraph->GetNearestIndexToKey(newRange.x(), -1, lowIndex);
                    bool rcHigh = m_pAllSessionGraph->GetNearestIndexToKey(newRange.y(), -1, highIndex);

                    if (rcLow && rcHigh)
                    {
                        AddIndexedLayerGraphs(pCurrentLayer, lowIndex, highIndex);
                    }
                }
            }
        }
    }

    replot();
}

void acNavigationChart::RemoveLayerPlotable(acNavigationChartLayer* pLayer)
{
    GT_IF_WITH_ASSERT(pLayer != nullptr)
    {
        if (pLayer->m_type == acNavigationChartLayer::eNavigationLayerGraph)
        {
            removePlottable(pLayer->m_pGraphLine);
            pLayer->m_pGraphLine = nullptr;
        }
        else
        {
            for (int nBar = 0; nBar < eNumHighlightedBars; nBar++)
            {
                if (pLayer->m_pGraphBars[nBar] != nullptr)
                {
                    removePlottable(pLayer->m_pGraphBars[nBar]);
                    // removePlottable also deletes it
                    pLayer->m_pGraphBars[nBar] = nullptr;
                }
            }
        }
    }
}

void acNavigationChart::AddIndexedLayerGraphs(acNavigationChartLayer* pLayer, int fromDataPoint, int toDataPoint)
{
    GT_IF_WITH_ASSERT(pLayer != nullptr)
    {
        if (pLayer->m_type == acNavigationChartLayer::eNavigationLayerGraph)
        {
            acVectorLineGraph* pVectorGraph = new acVectorLineGraph(xAxis, yAxis);
            pVectorGraph->SetVectorData(m_vXData, pLayer->m_layerYData);

            if (fromDataPoint > 0)
            {
                pVectorGraph->AddSegment(fromDataPoint, pLayer->m_dimmedPen, pLayer->m_dimmedBrush);
            }

            pVectorGraph->AddSegment(toDataPoint, pLayer->m_highlightedPen, pLayer->m_highlightedBrush);

            if (toDataPoint < m_vXData.size() - 1)
            {
                pVectorGraph->AddSegment(m_vXData.size(), pLayer->m_dimmedPen, pLayer->m_dimmedBrush);
            }

            addPlottable(pVectorGraph);
        }
        else
        {
            // first ribbon is from zero until the low index if low index is not zero and that is light gray color
            if (fromDataPoint != 0)
            {
                AddBar(pLayer, eBeforeHighlightedBar, 0, fromDataPoint - 1);
            }

            // add the main section of the bars
            if (toDataPoint == m_vXData.size() - 1)
            {
                AddBar(pLayer, eHighlightedBar, fromDataPoint, toDataPoint);
            }
            else
            {
                AddBar(pLayer, eHighlightedBar, fromDataPoint, toDataPoint - 1);
            }

            // Add the ribbon after
            if (toDataPoint != m_vXData.size() - 1)
            {
                AddBar(pLayer, eAfterHighlightedBar, toDataPoint, m_vXData.size() - 1);
            }
        }
    }
}

void acNavigationChart::AddNonIndexedLayerGraphs(acNavigationChartLayerNonIndex* pLayer)
{
    GT_IF_WITH_ASSERT(pLayer != nullptr)
    {
        if (pLayer->m_layerXData.size() > 0)
        {
            int toDataPoint = pLayer->m_layerXData.size() - 1;
            int dataSize = toDataPoint;
            int fromDataPoint = 0;

            // find the high index that is lower the the high range
            for (int i = 0; i < dataSize; i++)
            {
                if (pLayer->m_layerXData[i] > m_xAxisSelectedValueLow)
                {
                    fromDataPoint = i;
                    break;
                }
            }

            // check if m_xAxisSelectedValueLow larger then any of our points
            if (m_xAxisSelectedValueLow > pLayer->m_layerXData[dataSize])
            {
                fromDataPoint = dataSize;
            }

            // find the low index that is higher then the lower range
            for (int i = fromDataPoint; i < dataSize; i++)
            {
                if (pLayer->m_layerXData[i] > m_xAxisSelectedValueHigh)
                {
                    toDataPoint = i;
                    break;
                }
            }

            // check if m_xAxisSelectedValueHigh is larger of any of our points
            if (m_xAxisSelectedValueHigh > pLayer->m_layerXData[dataSize])
            {
                toDataPoint = dataSize;
            }

            if (fromDataPoint != 0)
            {
                AddBar(pLayer, eBeforeHighlightedBar, 0, fromDataPoint - 1);
            }

            // add the main section of the bars
            AddBar(pLayer, eHighlightedBar, fromDataPoint, toDataPoint - 1);

            // Add the ribbon after
            if (toDataPoint < pLayer->m_layerXData.size())
            {
                AddBar(pLayer, eAfterHighlightedBar, toDataPoint, pLayer->m_layerXData.size() - 1);
            }
        }
    }
}

void acNavigationChart::AddBar(acNavigationChartLayer* pLayer, int barIndex, int fromDataPoint, int toDataPoint)
{
    GT_IF_WITH_ASSERT(pLayer != nullptr)
    {
        // copy the data to vectors that will be inserted to the bars.
        QVector<double> xData;
        QVector<double> yData;
        acNavigationChartLayerNonIndex* pNonIndexLayer = dynamic_cast<acNavigationChartLayerNonIndex*>(pLayer);

        for (int i = fromDataPoint; i <= toDataPoint; i++)
        {
            // add data based on type of layer
            if (pNonIndexLayer != nullptr)
            {
                xData.push_back(pNonIndexLayer->m_layerXData[i]);
            }
            else
            {
                xData.push_back(m_vXData[i]);
            }

            yData.push_back(pLayer->m_layerYData[i]);
        }

        pLayer->m_pGraphBars[barIndex] = new QCPBars(xAxis, yAxis);
        pLayer->m_pGraphBars[barIndex]->setData(xData, yData);
        // change bar out line color and width
        QPen pen(barIndex == eBeforeHighlightedBar || barIndex == eAfterHighlightedBar ? pLayer->m_dimmedPen : pLayer->m_highlightedPen);
        QBrush brush(barIndex == eBeforeHighlightedBar || barIndex == eAfterHighlightedBar ? pLayer->m_dimmedBrush : pLayer->m_highlightedBrush);
        pen.setWidth(1);
        pLayer->m_pGraphBars[barIndex]->setPen(pen);
        pLayer->m_pGraphBars[barIndex]->setBrush(brush);

        // set the type of the bars based on the type of the layer
        if (pNonIndexLayer != nullptr)
        {
            pLayer->m_pGraphBars[barIndex]->setWidthType(QCPBars::wtAbsolute);
            pLayer->m_pGraphBars[barIndex]->setWidth(1);
        }
        else
        {
            pLayer->m_pGraphBars[barIndex]->setWidthType(QCPBars::wtAxisRectRatio);
            pLayer->m_pGraphBars[barIndex]->setWidth(1.0 / m_vXData.size()); // 1/200 of the width of the rect
        }

        pLayer->m_pGraphBars[barIndex]->setVisible(true);
        addPlottable(pLayer->m_pGraphBars[barIndex]);
    }
}

// Adding data layer impairs the performance of the display when dragging the sliders therefor there is a limit on the number of items
// layers can have. NonIndexed layers allows more freedom but damage the performance even farther so limits are even smaller on those layer
// Currently there is no limit on number of layer but probably more then 10 layers will cause very bad performance issues and we might need to consider
// putting limitation on that also, but since we do not see a scenario where a user will display all 10 layers at once we did not find
// the need to put that restriction now
void acNavigationChart::AddDataLayer(acNavigationChartLayer* pLayer)
{
    GT_IF_WITH_ASSERT(pLayer != nullptr)
    {
        bool addLayer = false;
        acNavigationChartLayerNonIndex* pNonIndexLayer = dynamic_cast<acNavigationChartLayerNonIndex*>(pLayer);

        if (pNonIndexLayer != nullptr)
        {
            // if it is a non index layer check it meets the points limit
            GT_IF_WITH_ASSERT(pNonIndexLayer->m_layerXData.size() <= NON_INDEX_LAYER_DISPLAY_LIMIT)
            {
                addLayer = true;
            }
        }
        else
        {
            // if this is a normal layer make sure it has the same number of points as the main data layer
            // make sure it dose not pass the limit number of points
            GT_IF_WITH_ASSERT(pLayer->m_layerYData.size() == m_vXData.size() && pLayer->m_layerYData.size() <= LAYER_DISPLAY_LIMIT)
            {
                addLayer = true;
            }
        }

        if (addLayer)
        {
            // check that there are no two layers with the same layer id
            bool foundSameId = false;
            int numLayers = m_layersVector.size();

            for (int nLayer = 0; nLayer < numLayers && !foundSameId ; nLayer++)
            {
                acNavigationChartLayer* pCurrentLayer = m_layersVector[nLayer];
                GT_IF_WITH_ASSERT(pCurrentLayer != nullptr)
                {
                    if (pCurrentLayer->m_layerId == pLayer->m_layerId)
                    {
                        foundSameId = true;
                        break;
                    }
                }
            }

            GT_IF_WITH_ASSERT(!foundSameId)
            {
                m_layersVector.push_back(pLayer);
                m_pAllSessionGraph->setVisible(false);

                // calculate the layer max value
                pLayer->m_layerMaxValue = -DBL_MAX;
                int dataSize = pLayer->m_layerYData.size();

                for (int i = 0; i < dataSize; i++)
                {
                    if (pLayer->m_layerYData[i] > pLayer->m_layerMaxValue)
                    {
                        pLayer->m_layerMaxValue = pLayer->m_layerYData[i];
                    }
                }

                OnRangeChangeEnded(QPointF(m_xAxisSelectedValueLow, m_xAxisSelectedValueHigh));
            }
        }
    }
}

void acNavigationChart::SetLayerVisiblity(int layerID, bool visible)
{
    bool foundLayer = false;
    int numLayers = m_layersVector.size();

    for (int nLayer = 0; nLayer < numLayers && !foundLayer; nLayer++)
    {
        acNavigationChartLayer* pCurrentLayer = m_layersVector[nLayer];
        GT_IF_WITH_ASSERT(pCurrentLayer != nullptr)
        {
            if (pCurrentLayer->m_layerId == layerID)
            {
                foundLayer = true;
                pCurrentLayer->m_visible = visible;
                break;
            }
        }
    }

    // the layers indexes can be defined in the UI before the data is entered so layers can be "shown/hiden"
    // before the data is added and layers can not be found this way so this is not an GT_ASSERT
    if (foundLayer)
    {
        // calculate the max range
        double maxVisible = -DBL_MAX;

        for (int nLayer = 0; nLayer < numLayers; nLayer++)
        {
            acNavigationChartLayer* pCurrentLayer = m_layersVector[nLayer];
            GT_IF_WITH_ASSERT(pCurrentLayer != nullptr)
            {
                if (pCurrentLayer->m_visible && pCurrentLayer->m_layerMaxValue > maxVisible)
                {
                    maxVisible = pCurrentLayer->m_layerMaxValue;
                }
            }
        }

        if (maxVisible != -DBL_MAX)
        {
            m_maxYSoFar = maxVisible;
            UpdateYAxisRange();
        }
    }
}

bool acNavigationChart::IsDisplayingFullRange()
{
    bool retVal = false;

    acDataVector* pVectorData = m_pAllSessionGraph->VectorData();

    if (pVectorData != nullptr)
    {
        int vectorSize = pVectorData->size();

        if (vectorSize > 0)
        {
            if (m_xAxisSelectedValueLow == pVectorData->at(0).Key() && m_xAxisSelectedValueHigh == pVectorData->at(vectorSize - 1).Key())
            {
                retVal = true;
            }
        }
    }

    return retVal;
}

acNavigationChartLayer* acNavigationChart::GetLayerById(int layerID)
{
    acNavigationChartLayer* pRetVal = nullptr;

    int numLayers = m_layersVector.size();

    for (int nLayer = 0; nLayer < numLayers; nLayer++)
    {
        acNavigationChartLayer* pCurrentLayer = m_layersVector[nLayer];
        GT_IF_WITH_ASSERT(pCurrentLayer != nullptr)
        {
            if (pCurrentLayer->m_layerId == layerID)
            {
                pRetVal = pCurrentLayer;
                break;
            }
        }
    }

    return pRetVal;
}

void acNavigationChart::OnShowTimeLine(bool visible, double timePos)
{
    m_showTimelineSync = visible;
    m_timelineSyncPos = timePos;

    repaint();
}

void acNavigationChart::DrawTimelineSync(QPainter& painter)
{
    GT_UNREFERENCED_PARAMETER(painter);
    if (m_showTimelineSync)
    {
        // pass through all the data lines and find the highest y value between all of them to show the sync point
        double highValue = 0;
        int numLayers = m_layersVector.size();
        for (int nLayer = 0; nLayer < numLayers; nLayer++)
        {
            // delete old layer plotable
            acNavigationChartLayer* pCurrentLayer = m_layersVector[nLayer];

            if (pCurrentLayer != nullptr && pCurrentLayer->m_visible)
            {
                double layerNearestValue = 0;
                // check if it is a normal layer or none indexed layer
                acNavigationChartLayerNonIndex* pNonIndexLayer = dynamic_cast<acNavigationChartLayerNonIndex*>(pCurrentLayer);
                if (pNonIndexLayer != nullptr)
                {
                    int numPoints = pNonIndexLayer->m_layerXData.count();
                    for (int nPoint = 0; nPoint < numPoints; nPoint++)
                    {
                        if (pNonIndexLayer->m_layerXData[nPoint] > m_timelineSyncPos - m_pAllSessionGraph->KeyInterval() / 2 &&
                            pNonIndexLayer->m_layerXData[nPoint] <= m_timelineSyncPos + m_pAllSessionGraph->KeyInterval() / 2)
                        {
                            layerNearestValue = pNonIndexLayer->m_layerYData[nPoint];
                            break;
                        }
                    }
                }
                else
                {
                    int syncPos;
                    bool rc = m_pAllSessionGraph->GetNearestIndexToKey(m_timelineSyncPos, -1, syncPos);
                    if (rc)
                    {
                        layerNearestValue = pCurrentLayer->m_layerYData[syncPos];
                    }
                }

                if (layerNearestValue > highValue)
                {
                    highValue = layerNearestValue;
                }
            }
        }

        // Draw the point
        // convert the x,y positions to the pixels position
        QPoint centerPoint = QPoint(m_pAllSessionXAxis->coordToPixel(m_timelineSyncPos), yAxis->coordToPixel(highValue));
/*        QPen timelinePen(Qt::red);
        timelinePen.setWidth(3);
        painter.setPen(timelinePen);
        painter.drawEllipse(centerPoint, 2, 2);*/
        // draw the vertical line
        QPen timelinePen(QColor(185, 185, 185, 255), 2, Qt::SolidLine);
        painter.setPen(timelinePen);
        QPoint bottomPoint(m_pAllSessionXAxis->coordToPixel(m_timelineSyncPos), yAxis->coordToPixel(0));
        QPoint topPoint(m_pAllSessionXAxis->coordToPixel(m_timelineSyncPos), yAxis->coordToPixel(yAxis->range().upper));
        QLine centerLine(bottomPoint, topPoint);
        painter.drawLine(centerLine);
        // draw the triangle
        QPoint triLine[4];
        triLine[0] = QPoint(centerPoint.x(), centerPoint.y() - 3);// at the active range line left
        triLine[1] = QPoint(centerPoint.x()- 5, centerPoint.y() - 6); // at the active range line left just below XAxis
        triLine[2] = QPoint(centerPoint.x() + 4, centerPoint.y() - 6);// at the m_nLow just below X-axis
        triLine[3] = QPoint(centerPoint.x(), centerPoint.y() - 3); // at the m_nLow and x-Axis rect top
        QPoint frameLine[4];
        frameLine[0] = QPoint(centerPoint.x(), centerPoint.y() -1 );// at the active range line left
        frameLine[1] = QPoint(centerPoint.x() - 7, centerPoint.y() - 8); // at the active range line left just below XAxis
        frameLine[2] = QPoint(centerPoint.x() + 6, centerPoint.y() - 8);// at the m_nLow just below X-axis
        frameLine[3] = QPoint(centerPoint.x(), centerPoint.y() -1 ); // at the m_nLow and x-Axis rect top
        QPen trainglePen(acQYELLOW_WARNING_COLOUR, 2, Qt::SolidLine);
        painter.setPen(trainglePen);
        painter.drawPolyline(triLine, 4);
        QPen trainglePenFrame(acQRED_WARNING_COLOUR, 1, Qt::SolidLine);
        painter.setPen(trainglePenFrame);
        painter.drawPolyline(frameLine, 4);

    }
}
