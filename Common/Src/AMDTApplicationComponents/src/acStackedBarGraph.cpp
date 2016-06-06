//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acStackedBarGraph.cpp
///
//==================================================================================


// Warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Local:
#include <AMDTApplicationComponents/Include/acStackedBarGraph.h>

#define BARS_WIDTH 10

acStackedBarGraph::acStackedBarGraph() : m_pInternalLayoutGrid(NULL), m_pInternalBottomElement(NULL)
{
}

void acStackedBarGraph::SetData(const QVector<acBarGraphData*>& countersData, const bool allowReplot)
{
    QCPBars* bars;
    m_dataSeriesVec.clear();
    m_numOfBars = 0;

    bool isxAxisLabeSet = false;
    QVector<double> xVec;

    foreach (acBarGraphData* singleData, countersData)
    {
        acStackedBarGraphhData* data = static_cast<acStackedBarGraphhData*>(singleData);
        (void)data;

        // set xAxis Labels only once per multi graph
        if (!isxAxisLabeSet)
        {
            SetXAxisKeys(data->m_xLabels, xVec);
            isxAxisLabeSet = true;
        }

        bars = new QCPBars(m_pCustomPlot->xAxis, m_pCustomPlot->yAxis);
        m_dataSeriesVec.append(bars);

        // set bars name for legend
        bars->setName(data->m_graphName);

        m_numOfBars = data->m_yData.count();

        // reset graph range by max value
        int maxValue = SetBarsData(data->m_yData, bars, xVec, BARS_WIDTH);

        // change bar fill color
        // supports only 1 color per bars graph. ignores color list after first
        bars->setBrush(QBrush(data->m_barsColor, Qt::SolidPattern));

        // move above prev bars graph
        int dataSeriesSize = m_dataSeriesVec.size();

        if (dataSeriesSize > 1)
        {
            // bypass qt compile error on Linux very ugly: m_dataSeriesVec.at(dataSeriesSize-2); Linux compilation fail at this line no matter what kind of tricks I tried
            for (int nBar = 0 ; nBar < dataSeriesSize ; nBar ++)
            {
                if (nBar == dataSeriesSize - 2)
                {
                    QCPBars* pBar = m_dataSeriesVec.at(dataSeriesSize - 2);
                    bars->moveBelow(pBar);
                }
            }
        }

        m_pCustomPlot->addPlottable(bars);

        // the the stacked max value by going over all data series exist
        if (m_dataSeriesVec.size() > 1)
        {
            maxValue = GetStackedMaxValue();
        }

        ResetGraphYRange(maxValue);
    }

    if (allowReplot)
    {
        Replot();
    }
}

void acStackedBarGraph::SetXAxisLabelsVector(const QVector<QString>& xVecLabels)
{
    m_pCustomPlot->xAxis->setTickVectorLabels(xVecLabels);
}

void acStackedBarGraph::SetXAxisKeys(const QVector<acBarName*>& xLabels, QVector<double>& xVec, const int numOfDataSeries)
{
    m_pCustomPlot->xAxis->setAutoTicks(false);
    m_pCustomPlot->yAxis->setAutoTickStep(false);

    int numOfBars = xLabels.count();

    // update class member - copy x_labels data from input to graph member
    acBarName* barNameData;

    for (int i = 0; i < numOfBars; i++)
    {
        barNameData = new acBarName(xLabels[i]->m_lowerBound, xLabels[i]->m_upperBound);
        m_xLabels.append(barNameData);
    }

    // copy to names vector for setting the Axis labels
    QVector<QString> xVecLabels;

    for (int i = 0; i < numOfBars; i++)
    {
        xVecLabels << xLabels[i]->GetName();
    }

    m_pCustomPlot->xAxis->setTickVectorLabels(xVecLabels);

    GetKeysVector(xVec, numOfDataSeries);

    m_pCustomPlot->xAxis->setTickVector(xVec);
    m_pCustomPlot->xAxis->setRange(0, xVec[numOfBars - 1]);
    m_pCustomPlot->xAxis->rescale();
}

void acStackedBarGraph::GetKeysVector(QVector<double>& xVec, const int numOfDataSeries)
{
    GT_UNREFERENCED_PARAMETER(numOfDataSeries);

    // set xAxis key values
    for (int i = 0; i < m_numOfBars; i++)
    {
        xVec << BARS_WIDTH*(i * 2 + 1);
    }
}

void acStackedBarGraph::UpdateData(const QVector<acSingleGraphData*> pSingleKeyDataVec, const bool allowReplot)
{
    int dataSeriesNum = m_dataSeriesVec.size();
    int index;

    GT_IF_WITH_ASSERT(dataSeriesNum == pSingleKeyDataVec.count())
    {
        for (int i = 0; i < dataSeriesNum; i++)
        {
            QVector<double>& yData = (pSingleKeyDataVec[i])->m_yData;
            index = (pSingleKeyDataVec[i])->m_graphIndex;

            GT_IF_WITH_ASSERT((index < dataSeriesNum) && (yData.size() == m_numOfBars))
            {
                QVector<double> xData;
                // keys - places of X to put bars
                GetKeysVector(xData, dataSeriesNum);

                // set bars data and bar width
                m_dataSeriesVec[index]->setData(xData, yData);
            }
        }

        // update yAxis range if needed
        double max = GetMaxValueForYAxisRangeCalculations(pSingleKeyDataVec);
        ResetGraphYRange(max);

        if (allowReplot)
        {
            Replot();
        }
    }
}

double acStackedBarGraph::GetMaxValueForYAxisRangeCalculations(const QVector<acSingleGraphData*> pSingleKeyDataVec) const
{
    double max = 0, cumulativeVal = 0;

    int dataSeriesNum = m_dataSeriesVec.size();
    GT_IF_WITH_ASSERT(dataSeriesNum == pSingleKeyDataVec.size())
    {
        for (int i = 0; i < m_numOfBars; i++)
        {
            cumulativeVal = 0;

            for (int j = 0; j < dataSeriesNum; j++)
            {
                GT_IF_WITH_ASSERT(pSingleKeyDataVec[j]->m_yData.size() == m_numOfBars)
                {
                    cumulativeVal += pSingleKeyDataVec[j]->m_yData[i];
                }
            }

            max = cumulativeVal > max ? cumulativeVal : max;
        }
    }

    return max;
}

void acStackedBarGraph::CreateGraphLayout()
{
    // Call the base class implementation:
    acBarGraph::CreateGraphLayout();

    // If a legend should be create3d, locate it with the QCustomPlot layout:
    if (m_shouldCreateLegend)
    {
        if (NULL != m_pCustomPlot && NULL != m_pCustomPlot->legend)
        {
            // Make the plot visible:
            m_pCustomPlot->legend->setVisible(true);

            // Get the graph grid layout:
            QCPLayoutGrid* pGraphGridLayout = m_pCustomPlot->plotLayout();

            // Sanity check:
            GT_IF_WITH_ASSERT(pGraphGridLayout != NULL)
            {
                // Add the legend to the grid with some dummy widgets to set the legend in the center:
                if (NULL == m_pInternalLayoutGrid && NULL == m_pInternalBottomElement)
                {
                    // Get legend from custom plot service:
                    QSize size = m_pCustomPlot->legend->iconSize();
                    size.setHeight(size.height() - 6);
                    m_pCustomPlot->legend->setColumnSpacing(0);
                    m_pCustomPlot->legend->setRowSpacing(0);

                    m_pCustomPlot->legend->setIconSize(size);
                    m_pCustomPlot->legend->setBorderPen(QPen(Qt::transparent));

                    m_pInternalLayoutGrid = new QCPLayoutGrid();
                    m_pInternalBottomElement = new QCPLayoutElement(m_pCustomPlot);

                    m_pInternalLayoutGrid->addElement(0, 0, m_pCustomPlot->legend);
                    m_pInternalLayoutGrid->addElement(1, 0, m_pInternalBottomElement);

                    pGraphGridLayout->addElement(1, 1, m_pInternalLayoutGrid);

                    // The legend should take 1/5 of the width:
                    pGraphGridLayout->setColumnStretchFactor(0, 4);
                    pGraphGridLayout->setColumnStretchFactor(1, 1);
                }
            }
        }
    }
}

QCPBars* acStackedBarGraph::GetBarGraph(int index = 0)
{
    if (index < m_dataSeriesVec.size() && index >= 0)
    {
        return m_dataSeriesVec[index];
    }
    else
    {
        return NULL;
    }
}

void acStackedBarGraph::DeleteGraph()
{
    acBarGraph::DeleteGraph();

    int count = m_xLabels.count();

    for (int i = 0; i < count; i++)
    {
        delete m_xLabels[i];
    }

    m_xLabels.clear();
}

int acStackedBarGraph::GetStackedMaxValue()
{
    // init verctor
    int maxValue = 0;

    QVector<double> cumulativeValuePerBarVec;

    for (int i = 0; i < m_numOfBars; i++)
    {
        cumulativeValuePerBarVec << 0;
    }

    // go over each data series
    int dataSeriesSize = m_dataSeriesVec.size();

    for (int nDataSeries = 0; nDataSeries < dataSeriesSize; nDataSeries++)
    {
        QCPBars* pBar = m_dataSeriesVec.at(nDataSeries);
        QCPBarDataMap* pDataMap = pBar->data();
        QMap<double, QCPBarData>::iterator it = pDataMap->begin();
        QMap<double, QCPBarData>::iterator itEnd = pDataMap->end();

        GT_IF_WITH_ASSERT(pDataMap->size() == m_numOfBars)
        {
            // go over each bar in data series
            for (int i = 0; it != itEnd && i < m_numOfBars; it++, i++)
            {
                cumulativeValuePerBarVec[i] += it->value;
            }
        }
    }

    // get max of all stacked bars total values
    for (int i = 0; i < m_numOfBars; i++)
    {
        maxValue = cumulativeValuePerBarVec[i] > maxValue ? cumulativeValuePerBarVec[i] : maxValue;
    }

    return maxValue;
}