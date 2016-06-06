//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acGroupedBarsGraph.cpp
///
//==================================================================================


// Warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtAssert.h>

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Local:
#include <AMDTApplicationComponents/Include/acGroupedBarsGraph.h>


#define BARS_WIDTH 10

acGroupedBarsGraph::acGroupedBarsGraph()
{
}

void acGroupedBarsGraph::SetData(QVector<acBarGraphData*>& countersData, bool allowReplot)
{
    QCPBars* bars;
    m_dataSeriesVec.clear();
    m_numOfBars = 0;

    bool isxAxisLabeSet = false;
    QVector<double> xVec;

    QCPBarsGroup* barsGroup = new QCPBarsGroup(m_pCustomPlot);

    double maxValue = 0;
    int numOfDataSeries = countersData.size();

    foreach (acBarGraphData* singleData, countersData)
    {
        acStackedBarGraphhData* data = static_cast<acStackedBarGraphhData*>(singleData);
        (void)data;

        m_numOfBars = data->m_yData.count();

        // set xAxis Labels only once per multi graph
        if (!isxAxisLabeSet)
        {
            SetXAxisKeys(data->m_xLabels, xVec, numOfDataSeries);
            isxAxisLabeSet = true;
        }

        bars = new QCPBars(m_pCustomPlot->xAxis, m_pCustomPlot->yAxis);
        m_dataSeriesVec.append(bars);

        // set bars name for legend
        bars->setName(data->m_graphName);

        // reset graph range by max value
        double tmpMax = SetBarsData(data->m_yData, bars, xVec, BARS_WIDTH);
        maxValue = maxValue > tmpMax ? maxValue : tmpMax;

        // change bar fill color
        // supports only 1 color per bars graph. ignores color list after first
        bars->setBrush(QBrush(data->m_barsColor, Qt::SolidPattern));

        // move above prev bars graph
        barsGroup->append(bars);
        barsGroup->setSpacingType(QCPBarsGroup::stAbsolute);
        barsGroup->setSpacing(1);

        m_pCustomPlot->addPlottable(bars);
    }

    ResetGraphYRange(maxValue);

    if (allowReplot)
    {
        Replot();
    }
}

void acGroupedBarsGraph::GetKeysVector(QVector<double>& xVec, const int numOfDataSeries)
{
    // set xAxis key values
    for (int i = 0; i < m_numOfBars; i++)
    {
        xVec << (i + 1)* numOfDataSeries * 1.5 * m_numOfBars;
    }
}

double acGroupedBarsGraph::GetMaxValueForYAxisRangeCalculations(const QVector<acSingleGraphData*> pSingleKeyDataVec) const
{
    double max = 0, tmpVal = 0;

    int dataSeriesNum = m_dataSeriesVec.size();
    GT_IF_WITH_ASSERT(dataSeriesNum == pSingleKeyDataVec.size())
    {
        for (int i = 0; i < m_numOfBars; i++)
        {
            for (int j = 0; j < dataSeriesNum; j++)
            {
                GT_IF_WITH_ASSERT(pSingleKeyDataVec[j]->m_yData.size() == m_numOfBars)
                {
                    tmpVal = pSingleKeyDataVec[j]->m_yData[i];
                    max = tmpVal > max ? tmpVal : max;
                }
            }
        }
    }

    return max;
}