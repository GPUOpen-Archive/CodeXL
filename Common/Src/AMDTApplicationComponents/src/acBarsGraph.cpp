//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acBarsGraph.cpp
///
//==================================================================================


#include <AMDTApplicationComponents/Include/acBarsGraph.h>
#include <AMDTBaseTools/Include/gtAssert.h>

#define AC_BARS_GRAPH_CAPTION_FONT_SIZE 8

acBarGraph::acBarGraph() :
    m_pCustomPlot(NULL),
    m_numOfBars(0),
    m_shouldCreateLegend(false),
    m_graphTitle(""),
    m_pGraphPlotTitle(NULL)
{
    m_pCustomPlot = new QCustomPlot();
}

acBarGraph::~acBarGraph()
{
    m_dataSeriesVec.clear();

    delete m_pGraphPlotTitle;

    m_pCustomPlot->clearGraphs();
    m_pCustomPlot->deleteLater();
}

void acBarGraph::Init(const double xUpperBound, const double xTickStep,
                      const double yUpperBound, const double yTickStep,
                      const QString& xAxisTitle, const QString& yAxisTitle,
                      bool allowReplot,
                      bool isLegendRequired)
{
    DeleteGraph();

    // setting plot range
    m_pCustomPlot->xAxis->setRange(0, xUpperBound);
    m_pCustomPlot->yAxis->setRange(0, yUpperBound);

    m_pCustomPlot->xAxis->setAutoTickLabels(false);
    m_pCustomPlot->xAxis->setTickLabelColor(Qt::black);

    if (xTickStep == 0)
    {
        m_pCustomPlot->xAxis->setAutoTickStep(true);
    }
    else
    {
        m_pCustomPlot->xAxis->setAutoTickStep(false);
        m_pCustomPlot->xAxis->setTickStep(xTickStep);
    }

    m_pCustomPlot->xAxis->setAutoTicks(true);
    m_pCustomPlot->xAxis->setSubTickCount(0);
    m_pCustomPlot->xAxis->setTickLabelPadding(TICK_LABEL_PADDING);
    m_pCustomPlot->xAxis->setTickLengthOut(TICK_LEN_OUT);
    m_pCustomPlot->xAxis->setTickLengthIn(TICK_LEN_IN);

    // axis jump units
    if (yTickStep == 0)
    {
        m_pCustomPlot->yAxis->setAutoTickStep(true);
    }
    else
    {
        m_pCustomPlot->yAxis->setAutoTickStep(false);
        m_pCustomPlot->yAxis->setTickStep(yTickStep);
    }

    m_pCustomPlot->yAxis->setAutoSubTicks(false);
    m_pCustomPlot->yAxis->setSubTickCount(0);
    m_pCustomPlot->yAxis->setTickLabelColor(Qt::black);

    QFont font;
    font.setBold(true);

    // yAxis Label
    if (!yAxisTitle.isEmpty())
    {
        m_pCustomPlot->yAxis->setLabel(yAxisTitle);
        m_pCustomPlot->yAxis->setLabelColor(Qt::black);
        m_pCustomPlot->yAxis->setLabelFont(font);
    }

    // xAxis Label
    if (!xAxisTitle.isEmpty())
    {
        m_pCustomPlot->xAxis->setLabel(xAxisTitle);
        m_pCustomPlot->xAxis->setLabelColor(Qt::black);
        m_pCustomPlot->xAxis->setLabelFont(font);
    }

    // Set the flag indicating the legend requirement:
    m_shouldCreateLegend = isLegendRequired;

    // Build the graph layout:
    CreateGraphLayout();

    m_pCustomPlot->setMinimumHeight(200);
    m_pCustomPlot->setMinimumWidth(300);

    if (allowReplot)
    {
        Replot();
    }

    connect(m_pCustomPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(OnPlotHovered(QMouseEvent*)));
}

void acBarGraph::SetXAxisTickLabelRotation(double degree)
{
    m_pCustomPlot->xAxis->setTickLabelRotation(degree);
}

void acBarGraph::DecreaseXAxisTickLabelFont(int dec)
{
    QFont font = m_pCustomPlot->xAxis->tickLabelFont();
    font.setPointSize(font.pointSize() - dec);
    m_pCustomPlot->xAxis->setTickLabelFont(font);
}

void acBarGraph::SetXAxisTickLabelPadding(int padding)
{
    m_pCustomPlot->xAxis->setTickLabelPadding(padding);
}

int acBarGraph::SetBarsData(const QVector<double>& yData, QCPBars* bars, QVector<double>& xData, int barWidth)
{
    int maxVal = 0;

    // set bars data and bar width
    bars->setData(xData, yData);

    // change bar out line color and width
    QPen pen(Qt::lightGray);
    pen.setWidth(BARS_PEN_WIDTH);
    bars->setPen(pen);

    bars->setWidth(barWidth);
    bars->setVisible(true);

    for (int i = 0; i < m_numOfBars; i++)
    {
        maxVal = yData[i] > maxVal ? yData[i] : maxVal;
    }

    return maxVal;
}

void acBarGraph::ResetGraphYRange(int maxBarVal)
{
    // change yAxis upper bound when exceeding 90% of upper bound
    double axisTopRange = m_pCustomPlot->yAxis->range().upper;
    double tickStep = m_pCustomPlot->yAxis->tickStep();

    while (maxBarVal > axisTopRange * 0.9)
    {
        axisTopRange *= 2;
        tickStep *= 2;
    }

    m_pCustomPlot->yAxis->setRange(0, axisTopRange);
    m_pCustomPlot->yAxis->setTickStep(tickStep);
}

void acBarGraph::SetGraphYRange(int low, int high)
{
    // change yAxis upper bound when exceeding 90% of upper bound
    double axisTopRange = m_pCustomPlot->yAxis->range().upper;
    double tickStep = m_pCustomPlot->yAxis->tickStep();
    int numOfTicks = 0;
    double newTickStep = 0;

    GT_IF_WITH_ASSERT(tickStep > 0)
    {
        numOfTicks = axisTopRange / tickStep;
        newTickStep = (high - low) / numOfTicks;
        m_pCustomPlot->yAxis->setTickStep(newTickStep);
    }

    m_pCustomPlot->yAxis->setRange(low, high);
}

QCustomPlot* acBarGraph::GetPlot()
{
    return m_pCustomPlot;
}

void acBarGraph::DeleteGraph()
{
    if (m_dataSeriesVec.count() > 0)
    {
        m_pCustomPlot->clearPlottables();
        m_dataSeriesVec.clear();
    }

    m_numOfBars = 0;
}

int acBarGraph::GetBarGraphIndexByName(QString name)
{
    int retVal = -1;

    for (int i = 0; i < m_dataSeriesVec.count(); i++)
    {
        if (m_dataSeriesVec[i]->name() == name)
        {
            retVal = i;
            break;
        }
    }

    return retVal;
}

void acBarGraph::SetGraphYAxisTitle(QString title)
{
    m_pCustomPlot->yAxis->setLabel(title);
    Replot();
}

void acBarGraph::Replot()
{
    // if the plot is not visible - don't replot it
    bool isPlotVisible = !m_pCustomPlot->visibleRegion().isEmpty();

    if (isPlotVisible)
    {
        m_pCustomPlot->replot();
    }
}

void acBarGraph::OnPlotHovered(QMouseEvent* pMouseEvent)
{
    bool isValueToDisplayFound = false;

    QPoint mousePos = pMouseEvent->pos();
    GT_IF_WITH_ASSERT(NULL != m_pCustomPlot)
    {
        QCPAxisRect*  pPlotRect = m_pCustomPlot->axisRect();
        GT_IF_WITH_ASSERT(NULL != pPlotRect)
        {
            if (pPlotRect->rect().contains(mousePos) &&
                !m_dataSeriesVec.empty())
            {
                for (int i = 0; i < m_dataSeriesVec.size(); ++i)
                {
                    GT_IF_WITH_ASSERT(NULL != m_dataSeriesVec[i])
                    {
                        if (-1 != m_dataSeriesVec[i]->selectTest(mousePos, true))
                        {
                            // get coord on the xAxis
                            double posKey = m_pCustomPlot->xAxis->pixelToCoord(mousePos.x());

                            QMap<double, QCPBarData>::iterator it = m_dataSeriesVec[i]->data()->begin();
                            QMap<double, QCPBarData>::iterator itEnd = m_dataSeriesVec[i]->data()->end();
                            int barsWidth = m_dataSeriesVec[i]->width();

                            for (; it != itEnd; it++)
                            {
                                //get bar voord bounds
                                double barlowerBound = (*it).key - barsWidth;
                                double barUpperBound = (*it).key + barsWidth;

                                // find key in bound and get bar data
                                if (posKey >= barlowerBound && posKey <= barUpperBound)
                                {
                                    m_pCustomPlot->parentWidget()->setToolTip(QString::number((*it).value, 'f', m_pCustomPlot->xAxis->numberPrecision()));
                                    m_pCustomPlot->parentWidget()->setToolTipDuration(1000);
                                    isValueToDisplayFound = true;
                                    break;
                                }
                            }

                            break;
                        }
                    }
                }
            }

        }

        if (!isValueToDisplayFound)
        {
            m_pCustomPlot->parentWidget()->setToolTip("");
            m_pCustomPlot->parentWidget()->setToolTipDuration(0);
        }
    }
}

void acBarGraph::CreateGraphLayout()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((GetPlot() != NULL) && (GetPlot()->plotLayout() != NULL))
    {
        // If the user set a caption, add it to the graph layout:
        if (!m_graphTitle.isEmpty())
        {
            // If the graph title element was not yet created:
            if (m_pGraphPlotTitle == NULL)
            {
                // Create the plot title element:
                m_pGraphPlotTitle = new QCPPlotTitle(GetPlot(), m_graphTitle);

                // Create a "dummy" element for the right side, to make sure that the title is left aligned:
                QCPLayoutElement* pRightDummyElement = new QCPLayoutElement(GetPlot());

                QFont defaultFont = m_pGraphPlotTitle->font();
                defaultFont.setPointSize(AC_BARS_GRAPH_CAPTION_FONT_SIZE);
                m_pGraphPlotTitle->setFont(defaultFont);

                // Add the caption to the graph layout:
                GetPlot()->plotLayout()->insertRow(0);

                // Create a dummy grid for the top line:
                QCPLayoutGrid* pDummyGrid = new QCPLayoutGrid();

                // Set 0 margins and spacing:
                pDummyGrid->setColumnSpacing(0);
                pDummyGrid->setMargins(QMargins(0, 0, 0, 0));

                pDummyGrid->addElement(0, 0, m_pGraphPlotTitle);
                pDummyGrid->addElement(0, 1, pRightDummyElement);
                GetPlot()->plotLayout()->addElement(0, 0, pDummyGrid);
            }
            else
            {
                m_pGraphPlotTitle->setText(m_graphTitle);
            }
        }
    }
}

void acBarGraph::SetGraphTitle(const QString& graphTitle)
{
    // Set the title:
    m_graphTitle = graphTitle;

    // If the QCustomPlot item was already created, update it:
    if (m_pGraphPlotTitle != NULL)
    {
        m_pGraphPlotTitle->setText(graphTitle);
    }
}
