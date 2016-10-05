//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppMultiLinePowerStackedPlot.cpp
///
//==================================================================================

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

#include <AMDTPowerProfiling/src/ppMultiLinePowerStackedPlot.h>
#include <AMDTPowerProfiling/src/ppColors.h>

#define TOTAL_GRAPH_VALUE 0

ppMultiLnePowerStackedPlot::ppMultiLnePowerStackedPlot(ppSessionController* pSessionController) : ppMultiLinePlot(pSessionController)

{
    m_isCumulative = true;
    m_isLastGraphTotal = false;
    m_totalGraphName = PP_STR_Counter_Power_TotalAPU;
}

void ppMultiLnePowerStackedPlot::SetGraphInitializationData()
{
    QVector<acMultiLinePlotItemData*> graphData;

    // get session time range fom DB
    SamplingTimeRange samplingRange(0, 0);
    m_pSessionController->GetSessionTimeRange(samplingRange);

    // get session counters from DB
    gtVector<int> counterIds;
    ppDataUtils::GetRelevantCounterIdsByGraphType(counterIds, m_graphType, m_pSessionController);

    // if there is counters selected in the session for getting their data
    if (counterIds.size() > 0)
    {
        // get data from dB
        PrepareTimelineDataFromDB(counterIds,
                                  samplingRange,
                                  graphData);
    }

    if (graphData.size() > 0)
    {
        // init with the data
        bool allowReplot = false;
        InitPlotGraphs(graphData, allowReplot);

        // delete data
        for (int i = 0; i < graphData.count(); i++)
        {
            delete graphData[i];
        }

        ResetPlotInfoTable();

        ResetPlotInfoTableColumnWidth();
        SetInfoTableBySpecificTimePoint(samplingRange.m_toTime);
    }
    else
    {
        // hide graph and it's legend
        m_isShown = false;
    }
}

void ppMultiLnePowerStackedPlot::PrepareTimelineDataFromDB(const gtVector<int>& counterIds,
                                                           SamplingTimeRange& samplingTimeRange,
                                                           QVector<acMultiLinePlotItemData*>& graphData)
{
    GT_IF_WITH_ASSERT(nullptr != m_pSessionController)
    {
        gtMap<int, gtVector<SampledValue>> sampledDataPerCounter;
        m_pSessionController->GetProfilerBL().GetSampledValuesByRange(counterIds, samplingTimeRange, sampledDataPerCounter);

        if (sampledDataPerCounter.size() > 0)
        {
            int id, valuesCount, apuCounterIndexInCountersVec = -1;
            QString name, description;
            acMultiLinePlotItemData* graphItem;
            QVector<double> keyVec, valueVec, totalValueVec, apuValueVec;
            QColor color;
            bool shouldDisableGraph = false;

            int apuCounterId = m_pSessionController->GetAPUCounterID();
            gtMap<int, gtVector<SampledValue> >::iterator it = sampledDataPerCounter.begin();

            // APU counter should always exist
            GT_IF_WITH_ASSERT(apuCounterId != -1 && it != sampledDataPerCounter.end())
            {
                // all values vector of all counters should be in the same size
                valuesCount = ((*it).second).size();

                // init totalValueVec with 0
                for (int j = 0; j < valuesCount; j++)
                {
                    totalValueVec << 0;
                }

                int countersNum = m_relevantCountersVec.size();

                for (int i = 0; i < countersNum; i++)
                {
                    id = m_relevantCountersVec[i];

                    // if counter found in data map from DB
                    if (sampledDataPerCounter.count(id) > 0)
                    {
                        keyVec.clear();
                        valueVec.clear();
                        bool isApuCounter = (id == apuCounterId);

                        if (isApuCounter)
                        {
                            color = PP_OTHERS_COUNTER_COLOR;
                            name = PP_STR_Counter_Power_Other;
                            apuCounterIndexInCountersVec = i;
                            description = PP_STR_OTHER_GRAPH_DESCRIPTION;
                            m_isLastGraphTotal = true;
                        }
                        else
                        {
                            name = m_pSessionController->GetCounterNameById(id);
                            color = m_pSessionController->GetColorForCounter(id);
                            description = m_pSessionController->GetCounterDescription(id);
                        }

                        // check if graph should be disabled
                        shouldDisableGraph = ShouldDisableGraph(m_relevantCountersVec, id);

                        for (int j = 0; j < valuesCount; j++)
                        {
                            keyVec << (sampledDataPerCounter[id])[j].m_sampleTime;
                            valueVec << (sampledDataPerCounter[id])[j].m_sampleValue;

                            // add to total calculation
                            if (!isApuCounter)
                            {
                                int index = GetGraphIndexByName(name);

                                // part of the other calculation only if this graph index is not disabled
                                if (!IsGraphDisabledAtIndex(index))
                                {
                                    totalValueVec[j] += (sampledDataPerCounter[id])[j].m_sampleValue;
                                }
                            }
                        }

                        graphItem = new acMultiLinePlotItemData(keyVec, valueVec, color, name, description, shouldDisableGraph);
                        graphData << graphItem;
                    }
                }


                if (apuCounterIndexInCountersVec != -1)
                {
                    // update the "other" value
                    QVector<double>& otherValVec = graphData[apuCounterIndexInCountersVec]->GetValueVec();

                    // save total apu value
                    apuValueVec = otherValVec;

                    GT_IF_WITH_ASSERT(apuCounterIndexInCountersVec != -1 && otherValVec.size() == totalValueVec.size())
                    {

                        for (int i = 0; i < valuesCount; i++)
                        {
                            otherValVec[i] -= totalValueVec[i];

                            // if the value is negative - replace it with 0
                            if (otherValVec[i] < 0)
                            {
                                otherValVec[i] = 0;
                            }
                        }
                    }
                }

                // if we use the total line graph
                if (m_isLastGraphTotal)
                {
                    SetTotalApuGraph(graphData, apuValueVec);
                }
            }
        }
    }
}

void ppMultiLnePowerStackedPlot::SetTotalApuGraph(QVector<acMultiLinePlotItemData*>& graphData, const QVector<double>& apuValueVec)
{
    GT_UNREFERENCED_PARAMETER(apuValueVec);

    if (m_isLastGraphTotal)
    {
        GT_IF_WITH_ASSERT(graphData.size() > 0)
        {
            // copy key vec from one of the other cunters data (its the same for all counters)_
            QVector<double> keyVec(graphData[0]->GetKeyVec());
            QVector<double> valueVec;

            int count = graphData[0]->GetKeyVec().count();

            // build black line total data series
            // the Total line plot is assigned values of zero so it will be displayed immediately above the other stacked graphs
            for (int i = 0; i < count; i++)
            {
                valueVec << TOTAL_GRAPH_VALUE;
            }

            int id = m_pSessionController->GetAPUCounterID();
            QString description = m_pSessionController->GetCounterDescription(id);

            // check if graph should be disabled
            bool shouldDisableGraph = false;

            // for cumulative graph - add total
            acMultiLinePlotItemData* graphItem = new acMultiLinePlotItemData(keyVec, valueVec, PP_TOTAL_COUNTER_COLOR, m_totalGraphName, description, shouldDisableGraph);
            graphData << graphItem;
        }
    }
}

void ppMultiLnePowerStackedPlot::PrepareTimelineDataFromNewDataEvent(ppQtEventData pSampledDataPerCounter,
                                                                     QVector<acMultiLinePlotItemData*>& graphData)
{
    GT_IF_WITH_ASSERT(nullptr != m_pSessionController)
    {
        int id;
        int apucounterIndex = -1;
        QString name;
        QString description;
        acMultiLinePlotItemData* graphItem;
        QColor color;
        QVector<double> keyVec;
        QVector<double> valueVec;
        QVector<double> enabledCountersValueVec;
        QVector<double> OthersKeyVec;
        QVector<double> othersValueVec;
        double enabledCountersCumulativeValue = 0;
        bool shouldDisableGraph = false;

        int count = m_relevantCountersVec.size();

        int apuCounterId = m_pSessionController->GetAPUCounterID();

        // if cant APU counter in data - do nothing
        GT_IF_WITH_ASSERT(apuCounterId != -1)
        {
            for (int i = 0; i < count; i++)
            {
                id = m_relevantCountersVec[i];

                // if can't find the count  er it's a bug
                GT_IF_WITH_ASSERT(pSampledDataPerCounter->count(id) > 0)
                {
                    bool isApuCounter = (id == apuCounterId);

                    if (isApuCounter)
                    {
                        // set Others counter
                        color = PP_OTHERS_COUNTER_COLOR;
                        name = PP_STR_Counter_Power_Other;
                        apucounterIndex = i;
                        description = PP_STR_OTHER_GRAPH_DESCRIPTION;
                        m_isLastGraphTotal = true;
                    }
                    else
                    {
                        color = m_pSessionController->GetColorForCounter(id);
                        name = m_pSessionController->GetCounterNameById(id);

                        enabledCountersCumulativeValue += pSampledDataPerCounter->at(id).m_sampleValues[0];

                        description = m_pSessionController->GetCounterDescription(id);
                    }

                    // check if graph should be disabled
                    shouldDisableGraph = ShouldDisableGraph(m_relevantCountersVec, id);

                    keyVec.clear();
                    valueVec.clear();

                    keyVec << pSampledDataPerCounter->at(id).m_quantizedTime;
                    valueVec << pSampledDataPerCounter->at(id).m_sampleValues[0];

                    graphItem = new acMultiLinePlotItemData(keyVec, valueVec, color, name, description, shouldDisableGraph);
                    graphData << graphItem;
                }
            }

            if (m_isLastGraphTotal)
            {
                QVector<double>& otherValVec = graphData[apucounterIndex]->GetValueVec();
                otherValVec[0] -= enabledCountersCumulativeValue;

                // if the value is negative - replace with 0
                if (otherValVec[0] < 0)
                {
                    otherValVec[0] = 0;
                }
            }
        }
    }
}

void ppMultiLnePowerStackedPlot::PrepareTimeLineSelectedCountersInitData(QVector<acMultiLinePlotItemData*>& graphData)
{
    if (m_pSessionController != nullptr)
    {
        int count = m_relevantCountersVec.size();
        int apuCounterId = m_pSessionController->GetAPUCounterID();

        // if cant APU counter in data - do nothing
        //GT_IF_WITH_ASSERT(apuCounterId != -1)
        {
            QString name;
            QColor color;
            QString description;
            bool shouldDisableGraph = false;

            for (int i = 0; i < count; i++)
            {
                if (m_relevantCountersVec[i] == apuCounterId)
                {
                    // set Others counter
                    color = PP_OTHERS_COUNTER_COLOR;
                    name = PP_STR_Counter_Power_Other;
                    description = PP_STR_OTHER_GRAPH_DESCRIPTION;

                    m_isLastGraphTotal = true;
                }
                else
                {
                    name = m_pSessionController->GetCounterNameById(m_relevantCountersVec[i]);
                    color = m_pSessionController->GetColorForCounter(m_relevantCountersVec[i]);

                    description = m_pSessionController->GetCounterDescription(m_relevantCountersVec[i]);
                }

                // check if graph should be disabled
                shouldDisableGraph = ShouldDisableGraph(m_relevantCountersVec, m_relevantCountersVec[i]);

                QVector<double> keyVec, valueVec;
                acMultiLinePlotItemData* graphItem = new acMultiLinePlotItemData(keyVec, valueVec, color, name, description, shouldDisableGraph);
                graphData << graphItem;
            }

            if (m_isLastGraphTotal)
            {
                color = PP_TOTAL_COUNTER_COLOR;
                name = m_totalGraphName;

                QVector<double> keyVec, valueVec;
                acMultiLinePlotItemData* graphItem = new acMultiLinePlotItemData(keyVec, valueVec, color, name, "", shouldDisableGraph);
                graphData << graphItem;
            }
        }
    }
}

void ppMultiLnePowerStackedPlot::GetValueVecFromEventData(ppQtEventData pSampledDataPerCounter, QVector<double>& valVec)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pSessionController != nullptr)
    {
        int apuCounterId = m_pSessionController->GetAPUCounterID();
        int countersNum = m_relevantCountersVec.size();

        int id, index, apuCounterIndex = -1;
        QString name;
        double cumulativeValue = 0;

        // add temp data - for finding un-updated data
        for (int i = 0; i < countersNum; i++)
        {
            valVec << -1;
        }

        // for every relevant counter get it's data and insert into data set
        for (int i = 0; i < countersNum; i++)
        {
            id = m_relevantCountersVec[i];

            GT_IF_WITH_ASSERT(pSampledDataPerCounter->count(id) > 0)
            {
                bool isApuCounter = (id == apuCounterId);

                if (isApuCounter)
                {
                    // other will get APU counter value
                    name = PP_STR_Counter_Power_Other;
                }
                else
                {
                    //get graph by its name
                    name = m_pSessionController->GetCounterNameById(id);
                }

                index = GetGraphIndexByName(name);

                // set the value vec - the value for graph in index x will be set in the value vector in index x
                GT_IF_WITH_ASSERT(index > -1 && index < countersNum)
                {
                    if (isApuCounter)
                    {
                        apuCounterIndex = index;
                    }
                    else
                    {
                        // part of the other calculation only if this graph index is not disabled
                        if (!IsGraphDisabledAtIndex(index))
                        {
                            cumulativeValue += pSampledDataPerCounter->at(id).m_sampleValues[0];
                        }
                    }

                    valVec[index] = pSampledDataPerCounter->at(id).m_sampleValues[0];
                }
            }
        }

        if (m_isLastGraphTotal)
        {
            SetApuAndOtherCountersValueToVec(valVec, apuCounterIndex, cumulativeValue);
        }
    }
}

void ppMultiLnePowerStackedPlot::SetApuAndOtherCountersValueToVec(QVector<double>& valVec, int apuCounterIndex, double cumulativeValue)
{
    GT_IF_WITH_ASSERT(apuCounterIndex > -1 && apuCounterIndex < valVec.size())
    {
        valVec[apuCounterIndex] -= cumulativeValue;

        // if the value is negative - replace with 0
        if (valVec[apuCounterIndex] < 0)
        {
            valVec[apuCounterIndex] = 0;
        }

        if (IsLastDataSeriesTotal())
        {
            // add total graph value to values vector
            valVec << TOTAL_GRAPH_VALUE;
        }
    }
}

bool ppMultiLnePowerStackedPlot::ShouldDisableGraph(const gtVector<int>& vec, int id)
{
    // disable if the graph is a child graph and it's parent is selected (counter is in graph)
    bool disable = false;

    // check if graph is a child
    disable = m_pSessionController->IsChildCounter(id);

    if (disable)
    {
        disable = false;

        // check if parent counter is selected to be shown in the plot.
        // if parent is not in the plot, show the child as regular graph
        QString parentName = m_pSessionController->GetCounterParent(id);

        GT_IF_WITH_ASSERT(!parentName.isEmpty())
        {
            int parentId = m_pSessionController->GetCounterIDByName(parentName);

            // search for parent id in counters vector
            int count = vec.size();

            for (int i = 0; i < count; i++)
            {
                if (vec[i] == parentId)
                {
                    disable = true;
                    break;
                }
            }
        }
    }

    return disable;
}
