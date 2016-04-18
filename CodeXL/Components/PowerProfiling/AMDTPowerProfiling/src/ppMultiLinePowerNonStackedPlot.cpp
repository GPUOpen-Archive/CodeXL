//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppMultiLinePowerNonStackedPlot.cpp
///
//==================================================================================

#include <AMDTPowerProfiling/src/ppMultiLinePowerNonStackedPlot.h>

#include <AMDTPowerProfiling/Include/ppStringConstants.h>

ppMultiLinePowerNonStackedPlot::ppMultiLinePowerNonStackedPlot(ppSessionController* pSessionController) : ppMultiLnePowerStackedPlot(pSessionController)
{
    m_isCumulative = false;
}

void ppMultiLinePowerNonStackedPlot::SetApuAndOtherCountersValueToVec(QVector<double>& valVec, int apuCounterIndex, double cumulativeValue)
{
    // if the APU counter selected - add the APU graph value and the "other" graph value
    if (m_isLastGraphTotal && apuCounterIndex > -1 && apuCounterIndex < valVec.size())
    {
        double apuOrigValue = valVec[apuCounterIndex];

        // get the "other" value
        valVec[apuCounterIndex] -= cumulativeValue;

        // if the value is negative - replace with 0
        if (valVec[apuCounterIndex] < 0)
        {
            valVec[apuCounterIndex] = 0;
        }

        // add total value (APU value) if the graph display it
        if (IsLastDataSeriesTotal())
        {
            // add APU counter value to values vector (total value)
            valVec << apuOrigValue;
        }
    }
}

void ppMultiLinePowerNonStackedPlot::SetTotalApuGraph(QVector<acMultiLinePlotItemData*>& graphData, const QVector<double>& apuValueVec)
{
    // if the APU counter selected - add the APU graph and the "other" graph
    if (m_isLastGraphTotal && graphData.size() > 0)
    {
        // copy key vec from one of the other cunters data (its the same for all counters)_
        int count = graphData[0]->GetKeyVec().count();

        GT_IF_WITH_ASSERT(count == apuValueVec.size())
        {
            // build black line total data series
            QVector<double> keyVec(graphData[0]->GetKeyVec());
            QVector<double> valueVec(apuValueVec);

            int id = m_pSessionController->GetAPUCounterID();
            QString description = m_pSessionController->GetCounterDescription(id);

            // for cumulative graph - add total
            acMultiLinePlotItemData* graphItem = new acMultiLinePlotItemData(keyVec, valueVec, PP_TOTAL_COUNTER_COLOR, m_totalGraphName, description);
            graphData << graphItem;
        }
    }
}