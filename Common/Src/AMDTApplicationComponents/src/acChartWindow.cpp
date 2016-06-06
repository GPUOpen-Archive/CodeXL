//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acChartWindow.cpp
///
//==================================================================================

//------------------------------ acChartWindow.cpp ------------------------------

#include <AMDTApplicationComponents/Include/acChartWindow.h>

// Qt:
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>

// Local:
// wxWindows pre compiled header:
#include <inc/acStringConstants.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

#define AC_BAR_CHART_V_TILT_RATIO 0.5
#define AC_BAR_CHART_H_TILT_RATIO 0.25
// Maximal bar chart width (for chart with 1 or 2 bars):


void acPieWidget::paintEvent(QPaintEvent* e)
{
    QWidget::paintEvent(e);

    if (NULL != m_pOwner && AC_PIE_CHART == m_pOwner->_chartType && !m_pieSectorsAnglesVector.empty())
    {
        m_pOwner->_calculatedAmountOfItems = m_pOwner->_dataItems.size();
        QPainter painter;
        painter.begin(this);

        QRect windowRect = rect();
        QRect drawRect;
        float  startAngle = 0;
        float  endAngle = 0;

        if (windowRect.width() > windowRect.height())
        {
            int delta = (windowRect.width() - windowRect.height()) / 2;
            drawRect.setCoords(delta, 0, delta + windowRect.height() - 1, windowRect.height() - 1);
        }
        else
        {
            int delta = (windowRect.height() - windowRect.width()) / 2;
            drawRect.setCoords(0, delta, windowRect.width() - 1, delta + windowRect.width() - 1);
        }

        for (unsigned int i = 0; i < m_pOwner->_calculatedAmountOfItems; i++)
        {
            if (i > 0)
            {
                startAngle = m_pieSectorsAnglesVector[i - 1];
            }

            endAngle = m_pieSectorsAnglesVector[i];
            QBrush pieBrush(m_pOwner->_dataItems[i]._pointColor);
            QPen originalPen = painter.pen();

            if (m_pOwner->_dataItems[i]._isSelected)
            {
                QPen selectionPen(Qt::red);
                selectionPen.setWidth(3);
                painter.setPen(selectionPen);
            }

            painter.setBrush(pieBrush);
            painter.drawPie(drawRect, startAngle, endAngle - startAngle);
            painter.setPen(originalPen);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acChartWindow::acChartWindow
// Description: Constructor
// Author:      Uri Shomroni
// Date:        14/8/2008
// ---------------------------------------------------------------------------
acChartWindow::acChartWindow(QWidget* pParent, acChartType chartType) :
    QWidget(pParent),
    _chartType(chartType), _highlightColor(Qt::black), _barWidth(-1),
    _barSpacing(-1), _totalValue(0), _maxValue(0), _referenceValue(0),
    _isLeftMouseDown(false), _lastMousePosition(0, 0),
    _hRotation(0.0), _vRotation(-45.0),
    _areArraysValid(true), _calculatedAmountOfItems(0)
{
    setMouseTracking(true);
    setAutoFillBackground(false);

    m_pGraphWidget = new QCustomPlot(this);
    m_pGraphWidget->setAttribute(Qt::WA_TransparentForMouseEvents);

    m_pPieWidget = new acPieWidget(this);
    m_pPieWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_pNoInfoLabel = new QLabel(AC_STR_chartNoData, this);
    QFont labelFont = m_pNoInfoLabel->font();
    labelFont.setPointSize(20);
    m_pNoInfoLabel->setFont(labelFont);

    m_pGraphWidget->setVisible(false);
    m_pPieWidget->setVisible(false);
}

// ---------------------------------------------------------------------------
// Name:        acChartWindow::~acChartWindow
// Description: Destructor
// Author:      Uri Shomroni
// Date:        14/8/2008
// ---------------------------------------------------------------------------
acChartWindow::~acChartWindow()
{
    ClearGraphs();
    ClearPieChart();
}

// ---------------------------------------------------------------------------
void acChartWindow::ClearGraphs()
{
    m_pGraphWidget->clearPlottables();

    // No need to delete the pointers since the clearPlottables does that
    m_barDataVector.clear();
}

// ---------------------------------------------------------------------------
void acChartWindow::ClearPieChart()
{
    // No need to delete the pointers since the clearPlottables does that
    GT_IF_WITH_ASSERT(NULL != m_pPieWidget)
    {
        m_pPieWidget->m_pieSectorsAnglesVector.clear();
    }
}

// ---------------------------------------------------------------------------
// Name:        acChartWindow::paintGL
// Description: Called on a wx paint event
// Author:      Uri Shomroni
// Date:        14/8/2008
// ---------------------------------------------------------------------------
void acChartWindow::paintGL()
{
    // Do not attempt to draw if the window is not on-screen:
    // Recalculate arrays if amount of items was changed:
    if ((!_areArraysValid) || (_calculatedAmountOfItems != (unsigned int)_dataItems.size()))
    {
        recalculateArrays();
    }

    // Draw the correct chart type:
    if (_chartType == AC_NO_CHART)
    {
        drawEmptyChart();
    }

    else
    {
        // Check if the chart is empty:
        bool isEmpty = (_dataItems.size() == 0);

        if (!isEmpty)
        {
            if (_chartType == AC_PIE_CHART)
            {
                drawPieChart();
            }
            else if (_chartType == AC_BAR_CHART)
            {
                drawBarChart();
            }
            else
            {
                // This happens if you don't add a new chart type here
                GT_ASSERT(false);
            }
        }
        else
        {
            drawEmptyChart();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acChartWindow::resizeGL
// Description: Called when the chart window is resized or moved
// Author:      Uri Shomroni
// Date:        14/8/2008
// ---------------------------------------------------------------------------
void acChartWindow::resizeGL(int width, int height)
{
    GT_UNREFERENCED_PARAMETER(width);
    GT_UNREFERENCED_PARAMETER(height);

    // Refresh:
    redrawWindow();
}

// ---------------------------------------------------------------------------
// Name:        acChartWindow::mousePointToBarIndex
// Description:
// Arguments:   QPointmousePoint
// Return Val:  selected bar index - int
// Author:      Yuri Rshtunique
// Date:        23/11/2014
// ---------------------------------------------------------------------------
int acChartWindow::mousePointToBarIndex(QPoint mousePoint)
{
    int retVal = -1;

    // find selected bar index
    for (uint i = 0; i < m_barDataVector.size(); ++i)
    {
        if (-1 != m_barDataVector[i]->selectTest(mousePoint, true))
        {
            retVal = i;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acChartWindow::mousePointToPieIndex
// Description:
// Arguments:   QPointmousePoint
// Return Val:  selected pie sector index - int
// Author:      Yuri Rshtunique
// Date:        8/12/2014
// ---------------------------------------------------------------------------
int acChartWindow::mousePointToPieIndex(QPoint mousePoint)
{
    int retVal = -1;
    GT_IF_WITH_ASSERT(NULL != m_pPieWidget)
    {
        if (!m_pPieWidget->m_pieSectorsAnglesVector.empty())
        {
            QRect pieRect = m_pPieWidget->rect();
            int radius = pieRect.width() / 2;
            QPoint center = QPoint((pieRect.right() - pieRect.left()) / 2, (pieRect.bottom() - pieRect.top()) / 2);
            QLineF mouseLine(center, mousePoint);
            float mouseLineAngle = mouseLine.angle();

            // find selected pie index
            for (uint i = 0; i < m_pPieWidget->m_pieSectorsAnglesVector.size(); ++i)
            {
                //calculate if the mousePoint is in current sector
                float startAngle = 0;
                float endAngle = 0;

                if (i == 0)
                {
                    startAngle = 0.0;
                }
                else
                {
                    startAngle = m_pPieWidget->m_pieSectorsAnglesVector[i - 1];
                    startAngle /= 16.0;
                }

                endAngle = m_pPieWidget->m_pieSectorsAnglesVector[i] / 16.0;

                if (mouseLineAngle >= startAngle && mouseLineAngle <= endAngle && mouseLine.length() <= radius)
                {
                    retVal = i;
                    break;
                }
            }
        }
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        acChartWindow::onLeftMouseDown
// Description: Called when the chart window is left-clicked
// Author:      Uri Shomroni
// Date:        14/8/2008
// ---------------------------------------------------------------------------
void acChartWindow::mousePressEvent(QMouseEvent* pEvent)
{
    if (pEvent != NULL)
    {
        // For bar chart - handle click events:
        if (pEvent->button() == Qt::LeftButton)
        {
            // Get the mouse position:
            QPoint mousePoint = pEvent->pos();
            int index = -1;

            if ((_chartType == AC_BAR_CHART))
            {
                // set mousePoint.y to axisRect bottom to be able to select the shortest bar
                GT_IF_WITH_ASSERT(NULL != m_pGraphWidget && NULL != m_pGraphWidget->axisRect())
                {
                    mousePoint.setY(m_pGraphWidget->axisRect()->bottom());
                }

                // Check the index of the bar clicked by user:
                index = mousePointToBarIndex(mousePoint);
            }
            else
            {
                // set mousePoint.y to axisRect bottom to be able to select the shortest bar
                GT_IF_WITH_ASSERT(NULL != m_pPieWidget)
                {
                    // Check the index of the pie sector clicked by user:
                    index = mousePointToPieIndex(mousePoint);
                }

            }

            if ((index >= 0) && (index < (int)_dataItems.size()))
            {
                // Get the original (list) clicked item index:
                acChartDataPoint dataPoint = _dataItems[index];

                // Create a acChartItemClickedEvent event:
                if (dataPoint._originalItemIndex >= 0)
                {
                    // Emit a chart item clicked signal:
                    emit chartItemClicked(dataPoint._originalItemIndex);
                }
            }
        }

        // Store the mouse down position for rotation:
        if (!_isLeftMouseDown)
        {
            _isLeftMouseDown = true;

            // Store the mouse starting position:
            _lastMousePosition = pEvent->pos();
        }
    }

    // Call the base class implementation:
    QWidget::mousePressEvent(pEvent);
}


// ---------------------------------------------------------------------------
// Name:        acChartWindow::mouseReleaseEvent
// Description: Is handling mouse release event
// Arguments:   QMouseEvent* pEvent
// Author:      Sigal Algranaty
// Date:        23/1/2012
// ---------------------------------------------------------------------------
void acChartWindow::mouseReleaseEvent(QMouseEvent* pEvent)
{
    if (pEvent != NULL)
    {
        if (pEvent->button() == Qt::LeftButton)
        {
            _isLeftMouseDown = false;
        }
    }

    // Call the base class implementation:
    QWidget::mouseReleaseEvent(pEvent);
}

// ---------------------------------------------------------------------------
// Name:        acChartWindow::onMouseMove
// Description: Called when the mouse moves over the chart window
// Author:      Uri Shomroni
// Date:        14/8/2008
// ---------------------------------------------------------------------------
void acChartWindow::mouseMoveEvent(QMouseEvent* pEvent)
{
    if (pEvent != NULL)
    {
        // For bar chart - set tooltip according to mouse location:
        if (_chartType == AC_BAR_CHART)
        {
            // Get the picked bar:
            int index = mousePointToBarIndex(pEvent->pos());

            // Get the data item:
            if (index < (int)_dataItems.size() && index >= 0)
            {
                acChartDataPoint dataPoint = _dataItems[index];
                setToolTip(dataPoint._tooltip.asASCIICharArray());
            }
        }

        // For pie chart - set tooltip according to mouse location:
        if (_chartType == AC_PIE_CHART)
        {
            // Get the picked sector:
            int index = mousePointToPieIndex(pEvent->pos());

            // Get the data item:
            if (index < (int)_dataItems.size() && index >= 0)
            {
                acChartDataPoint dataPoint = _dataItems[index];
                setToolTip(dataPoint._tooltip.asASCIICharArray());
            }
        }

        if (_isLeftMouseDown)
        {
            // Calculate the rotation:
            QPoint currentMousePos = pEvent->pos();
            QSize currentWindowSize = size();

            // Don't rotate if we somehow got here without recording the last position:
            if (!_lastMousePosition.isNull())
            {
                _hRotation -= (360.0 * (currentMousePos.x() - _lastMousePosition.x()) / currentWindowSize.width());
                _vRotation -= (360.0 * (currentMousePos.y() - _lastMousePosition.y()) / currentWindowSize.height());
            }

            _lastMousePosition = currentMousePos;

            // Redraw the chart:
            paintGL();
        }
    }

    // Call the base class implementation:
    QWidget::mouseMoveEvent(pEvent);
}

// ---------------------------------------------------------------------------
void acChartWindow::resizeEvent(QResizeEvent* event)
{
    m_pGraphWidget->resize(event->size());
    m_pPieWidget->resize(event->size());
}

// ---------------------------------------------------------------------------
// Name:        acChartWindow::addDataPoint
// Description: Adds a data point to the graph
// Author:      Uri Shomroni
// Date:        14/8/2008
// ---------------------------------------------------------------------------
void acChartWindow::addDataPoint(const acChartDataPoint& dataIn)
{
    _dataItems.push_back(dataIn);

    _totalValue += dataIn._value;

    if (dataIn._value > _maxValue)
    {
        _maxValue = dataIn._value;
    }

    _areArraysValid = false;
}

// ---------------------------------------------------------------------------
// Name:        acChartWindow::clearAllData
// Description: Clears all data from the graph
// Author:      Uri Shomroni
// Date:        14/8/2008
// ---------------------------------------------------------------------------
void acChartWindow::clearAllData()
{
    _dataItems.clear();
    _maxValue = 0;
    _totalValue = 0;
    _areArraysValid = false;
}

// ---------------------------------------------------------------------------
// Name:        acChartWindow::setSelection
// Description: Sets the item numbered position to be selected / deselected
// Author:      Uri Shomroni
// Date:        14/8/2008
// ---------------------------------------------------------------------------
void acChartWindow::setSelection(int position, bool selected)
{
    int l = _dataItems.size();

    // Sanity check:
    GT_IF_WITH_ASSERT((position < l) && (position >= 0))
    {
        _dataItems[position]._isSelected = selected;
    }
}

// ---------------------------------------------------------------------------
// Name:        acChartWindow::clearAllSelection
// Description: Clears the selection from all the items
// Author:      Uri Shomroni
// Date:        14/8/2008
// ---------------------------------------------------------------------------
void acChartWindow::clearAllSelection()
{
    int l = _dataItems.size();

    // Clear each item's selected flag:
    for (int i = 0; i < l; i++)
    {
        _dataItems[i]._isSelected = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        acChartWindow::redrawWindow
// Description: Redraw the window
// Author:      Uri Shomroni
// Date:        18/8/2008
// ---------------------------------------------------------------------------
void acChartWindow::redrawWindow()
{
    // Paint the GL canvas:
    paintGL();
}

// ---------------------------------------------------------------------------
// Name:        acChartWindow::recalculateArrays
// Description: Recalculate the size and arrays of vertices and normals.
// Author:      Uri Shomroni
// Date:        27/8/2008
// ---------------------------------------------------------------------------
void acChartWindow::recalculateArrays()
{
    switch (_chartType)
    {
        case AC_NO_CHART:
        {
            _calculatedAmountOfItems = 0;
            _dataItems.clear();
        }
        break;

        case AC_PIE_CHART:
        {
            calculatePieChart();
        }
        break;

        case AC_BAR_CHART:
        {
            calculateBarChart();
        }
        break;

        default:
        {
            // We added a new chart type, but not here:
            GT_ASSERT(false);
        }
        break;
    }

    // Mark that the arrays are valid:
    _areArraysValid = true;

    // Redraw window after calculation:
    redrawWindow();
}

// ---------------------------------------------------------------------------
// Name:        acChartWindow::drawPieChart
// Description: Draws the pie chart
// Author:      Uri Shomroni
// Date:        27/8/2008
// ---------------------------------------------------------------------------
void acChartWindow::drawPieChart()
{
    m_pGraphWidget->setVisible(false);
    m_pNoInfoLabel->setVisible(false);
    m_pPieWidget->setVisible(true);
    m_pPieWidget->repaint();
}

// ---------------------------------------------------------------------------
// Name:        acChartWindow::drawBarChart
// Description: Draws the bar chart
// Author:      Uri Shomroni
// Date:        27/8/2008
// ---------------------------------------------------------------------------
void acChartWindow::drawBarChart()
{
    m_pGraphWidget->setVisible(true);
    m_pNoInfoLabel->setVisible(false);
    m_pPieWidget->setVisible(false);
}

// ---------------------------------------------------------------------------
// Name:        acChartWindow::calculatePieChart
// Description: creates QPainterPath for each sector of the pie
//              and populates vector for future mouse click detection
// Author:      Yuri Rshtunique
// Date:        November 8, 2014
// ---------------------------------------------------------------------------
void acChartWindow::calculatePieChart()
{
    GT_IF_WITH_ASSERT(NULL != m_pPieWidget)
    {
        m_pPieWidget->m_pieSectorsAnglesVector.clear();
        _calculatedAmountOfItems = _dataItems.size();
        float pieAngle = 0.0;
        float cumulativeAngle = 0.0;

        for (unsigned int i = 0; i < _calculatedAmountOfItems; i++)
        {
            pieAngle = ((_dataItems[i]._value * 360) / (_totalValue * 1.0)) * 16;
            cumulativeAngle += pieAngle;
            m_pPieWidget->m_pieSectorsAnglesVector.push_back(cumulativeAngle);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acChartWindow::calculateBarChart
// Description: Draw a bar chart based off the data in _dataItems
// Author:      Uri Shomroni
// Date:        14/8/2008
// ---------------------------------------------------------------------------
void acChartWindow::calculateBarChart()
{
    _calculatedAmountOfItems = _dataItems.size();

    // Clear the old graphs:
    ClearGraphs();

    // Calculate each of the bars:
    for (unsigned int i = 0; i < _calculatedAmountOfItems; i++)
    {
        QColor barColor = _dataItems[i]._pointColor;

        // Set each bar data:
        QCPBars* pNewBarData = new QCPBars(m_pGraphWidget->xAxis, m_pGraphWidget->yAxis);
        m_barDataVector.push_back(pNewBarData);
        m_pGraphWidget->addPlottable(pNewBarData);
        pNewBarData->addData(QCPBarData(i, _dataItems[i]._value));
        pNewBarData->setBrush(QBrush(barColor));

        if (_dataItems[i]._isSelected)
        {
            QPen selectionPen(Qt::red);
            selectionPen.setWidth(3);
            pNewBarData->setPen(selectionPen);
        }
        else
        {
            // no pen;
            QPen selectionPen(barColor);
            selectionPen.setWidth(1);
            pNewBarData->setPen(selectionPen);
        }
    }

    // Set general graph info
    m_pGraphWidget->xAxis->setRange(-1, _calculatedAmountOfItems + 1);
    m_pGraphWidget->xAxis->setTickLabels(false);
    m_pGraphWidget->xAxis->setTicks(false);
    m_pGraphWidget->yAxis->setRange(0, _maxValue);
    m_pGraphWidget->replot();
}

// ---------------------------------------------------------------------------
// Name:        acChartWindow::setChartType
// Description: Set the chart type. Also update pending viewport since calculations
//              are different for bar and pie.
// Arguments:   const acChartType& chartType
// Author:      Sigal Algranaty
// Date:        29/1/2012
// ---------------------------------------------------------------------------
void acChartWindow::setChartType(const acChartType& chartType)
{
    _chartType = chartType;
    _areArraysValid = false;

}

// ---------------------------------------------------------------------------
// Name:        acChartWindow::drawEmptyChart
// Description: Draw an empty chart. Add "No Data Available" text
// Author:      Sigal Algranaty
// Date:        7/2/2012
// ---------------------------------------------------------------------------
void acChartWindow::drawEmptyChart()
{
    m_pGraphWidget->setVisible(false);
    m_pNoInfoLabel->setVisible(true);
    m_pPieWidget->setVisible(false);
}
