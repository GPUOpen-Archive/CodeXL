//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acColoredBarsGraph.cpp
///
//==================================================================================


#include <AMDTApplicationComponents/Include/acColoredBarsGraph.h>
#include <AMDTApplicationComponents/Include/acColoredBarGraphData.h>
#include <AMDTApplicationComponents/Include/acQCPColoredBars.h>

#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtAssert.h>

#define BARS_WIDTH 10

acColoredBarsGraph::acColoredBarsGraph()
{
}

void acColoredBarsGraph::SetData(const QVector<acBarGraphData*>& barGrpahsData, const bool allowReplot)
{
    // supports only one colored bar graph
    GT_IF_WITH_ASSERT(barGrpahsData.count() == 1)
    {
        m_dataSeriesVec.clear();

        acColoredBarGraphData* barsData = static_cast<acColoredBarGraphData*>(barGrpahsData[0]);

        acQCPColoredBars* bars = new acQCPColoredBars(m_pCustomPlot->xAxis, m_pCustomPlot->yAxis, barsData->m_barsColors);

        m_dataSeriesVec.append(bars);

        m_numOfBars = barsData->m_yData.size();

        QVector<double> xVec;
        xVec.resize(barsData->m_xLabels.count());
        SetXAxisKeys(barsData->m_xLabels, xVec);

        int maxBarVal = SetBarsData(barsData->m_yData, bars, xVec, BARS_WIDTH);

        double xAxisUpperRange = (m_numOfBars * 2) * BARS_WIDTH;
        m_pCustomPlot->xAxis->setRange(0, xAxisUpperRange);

        m_pCustomPlot->addPlottable(bars);

        // update yAxis range if needed
        ResetGraphYRange(maxBarVal);

        if (allowReplot)
        {
            Replot();
        }
    }
}

void acColoredBarsGraph::SetXAxisKeys(const QVector<QString>& xVecLabels, QVector<double>& xVec)
{
    int numOfBars = xVecLabels.count();

    // set xAxis keys
    for (int i = 0; i < numOfBars; i++)
    {
        xVec[i] = BARS_WIDTH * (i * 2 + 1);
        m_xLabelsStringsVector << xVecLabels[i];
    }

    m_pCustomPlot->xAxis->setAutoTickStep(false);
    m_pCustomPlot->xAxis->setAutoTicks(false);

    m_pCustomPlot->xAxis->setTickVector(xVec);
    m_pCustomPlot->xAxis->setTickVectorLabels(xVecLabels);

    m_pCustomPlot->xAxis->rescale();
}

void acColoredBarsGraph::UpdateData(const QVector<acSingleGraphData*> pSingleKeyData, const bool allowReplot)
{
    // supports only 1 bars graph - index == 0
    if (pSingleKeyData.size() == 1 &&
        (pSingleKeyData[0])->m_graphIndex < m_dataSeriesVec.size())
    {
        QVector<double>& yData = (pSingleKeyData[0])->m_yData;
        int index = (pSingleKeyData[0])->m_graphIndex;

        if (yData.size() == m_numOfBars)
        {
            QVector<double> xData;

            // keys - places of X to put bars
            for (int i = 0; i < m_numOfBars; i++)
            {
                xData << BARS_WIDTH*(i * 2 + 1);
            }

            // set bars data and bar width
            m_dataSeriesVec[index]->setData(xData, yData);

            // update yAxis range if needed
            int max = 0;

            for (int i = 0; i < m_numOfBars; i++)
            {
                max = yData[i] > max ? yData[i] : max;
            }

            ResetGraphYRange(max);

            if (allowReplot)
            {
                Replot();
            }
        }
    }
}

QCPBars* acColoredBarsGraph::GetBarGraph(int index = 0)
{
    if (index >= 0 && m_dataSeriesVec.count() > 0)
    {
        return m_dataSeriesVec[0];
    }
    else
    {
        return NULL;
    }
}