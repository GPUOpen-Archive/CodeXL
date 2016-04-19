//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppDataUtils.cpp
///
//==================================================================================


#include <AMDTPowerProfiling/src/ppDataUtils.h>
#include <AMDTPowerProfiling/src/ppColoredBarGraphData.h>
#include <AMDTPowerProfiling/src/ppStackedBarGraphData.h>

#include <AMDTPowerProfileAPI/inc/AMDTPowerProfileDataTypes.h>

#include <AMDTPowerProfiling/src/ppAppController.h>
#include <AMDTPowerProfilingMidTier/include/PowerProfilerBL.h>
#include <AMDTPowerProfilingMidTier/include/PowerProfilerCore.h>
#include <AMDTPowerProfiling/Include/ppStringConstants.h>
#include <AMDTPowerProfiling/src/ppAidFunctions.h>

#include <algorithm>

// --------------------Summary view helping functions ---------------------------

double ppDataUtils::GetSummaryPowerGraphInitializationDataFromDB(QVector<acBarGraphData*>& data, double& otherCounterValue,
                                                                 const PowerGraphType sampledGraphType,
                                                                 ppSessionController* pSessionController,
                                                                 const gtVector<int>& counterIds)
{
    double apuCounterValue = -1;
    double retTotalVal = -1;

    GT_IF_WITH_ASSERT(nullptr != pSessionController)
    {
        gtMap<int, double> datamMap;

        if (sampledGraphType == POWERGRAPHTYPE_CUMULATIVE)
        {
            pSessionController->GetProfilerBL().GetCurrentCumulativeEnergyConsumptionInJoule(counterIds, datamMap, otherCounterValue);
        }
        else if (sampledGraphType == POWERGRAPHTYPE_AVERAGE)
        {
            unsigned samplingIntervalMs = pSessionController->GetSamplingTimeInterval();
            pSessionController->GetProfilerBL().GetCurrentAveragePowerConsumptionInWatt(counterIds, samplingIntervalMs, datamMap, otherCounterValue);
        }
        else
        {
            GT_ASSERT_EX(false, PP_STR_DebugMessageUnknownGraphType);
        }

        int apuId = pSessionController->GetAPUCounterID();
        double total = 0;

        gtMap<int, double>::iterator it = datamMap.begin();
        gtMap<int, double>::iterator itEnd = datamMap.end();
        double dGpuCountersTotalValue = 0.0;

        for (; it != itEnd; it++)
        {
            // check if this map item has a relevant counter id
            if (it->first == apuId)
            {
                apuCounterValue = it->second;
            }
            else
            {
                if (IsDGpuCounter(it->first, pSessionController))
                {
                    dGpuCountersTotalValue += it->second;
                }

                total += it->second;
            }
        }

        // if APU counter selected - get its value as the total value
        if (apuCounterValue != -1)
        {
            GT_IF_WITH_ASSERT(datamMap.count(apuId) > 0)
            {
                datamMap[apuId] = otherCounterValue;
            }
            // return the APU counter + dGpu's values as the total value
            retTotalVal = apuCounterValue + dGpuCountersTotalValue;
        }
        else
        {
            // if the APU counter is not part of the selected counters -
            // return the calculates sum of present counters
            retTotalVal = total;
        }

        // init data for graph
        acBarGraphData* barsData = new ppColoredBarGraphData(datamMap, pSessionController->GetAPUCounterID(), pSessionController, counterIds);
        data << barsData;
    }

    return retTotalVal;
}

double ppDataUtils::GetSummaryPowerGraphInitializationDataFromEvent(QVector<acBarGraphData*>& counterDataVec,
                                                                    gtMap<int, double>& bucketsData,
                                                                    double otherCounterData,
                                                                    const gtVector<int>& counterIds,
                                                                    ppSessionController* pSessionController)
{
    double apuCounterValue = -1;
    double retTotalVal = -1;
    double total = 0;
    double dGpuCountersTotalValue = 0.0;

    gtMap<int, double> relevantCountersData;
    int CountersNum = counterIds.size();

    if (CountersNum > 0)
    {
        int id;
        int apuId = pSessionController->GetAPUCounterID();

        for (int i = 0; i < CountersNum; i++)
        {
            // check if this map item has a relevant counter id
            id = counterIds[i];

            if (bucketsData.count(id) > 0)
            {
                relevantCountersData[id] = bucketsData[id];

                if (id == apuId)
                {
                    //for APU counter - get the value only and dont set the map yet
                    apuCounterValue = bucketsData[id];
                }
                else
                {
                    if (IsDGpuCounter(id, pSessionController))
                    {
                        dGpuCountersTotalValue += bucketsData[id];
                    }

                    total += bucketsData[id];
                }
            }
        }

        // if the APU counter is part of the selected counters
        if (apuCounterValue != -1)
        {
            // insert as a last bar of the 'other' value. use the APU counter id (will be changed later to 'other' label)
            GT_IF_WITH_ASSERT(relevantCountersData.count(apuId) > 0)
            {
                relevantCountersData[apuId] = otherCounterData;
            }

            // return the APU counter + dGpu's values as the total value
            retTotalVal = apuCounterValue + dGpuCountersTotalValue;
        }
        else
        {
            // if APU is not selected - return the calculate sum of selected counters
            retTotalVal = total;
        }
    }

    // init data for graph
    acBarGraphData* barsData = new ppColoredBarGraphData(relevantCountersData, pSessionController->GetAPUCounterID(), pSessionController, counterIds);
    counterDataVec << barsData;

    return retTotalVal;
}

void ppDataUtils::GeSummarytPowerGraphDataFromEvent(const ppQtEventData pSampledDataPerCounter,
                                                    gtMap<int, double>& accumulatedEnergyInJoule, double& cumulativeOtherCounterValueInJoule,
                                                    gtMap<int, double>& averagePowerInWatt, double& averageOtherCounterValueInWatt,
                                                    gtMap<int, unsigned>& aggregatedQuantized,
                                                    unsigned int currSamplingInterval,
                                                    ppSessionController* pSessionController)
{
    gtMap<int, PPSampledValuesBatch> eventData = *(pSampledDataPerCounter.data());

    // Sanity check:
    GT_IF_WITH_ASSERT(pSessionController != nullptr)
    {
        pSessionController->GetProfilerBL().UpdateCumulativeAndAverageHistograms(eventData,
                currSamplingInterval,
                accumulatedEnergyInJoule, cumulativeOtherCounterValueInJoule,
                averagePowerInWatt, averageOtherCounterValueInWatt,
                aggregatedQuantized);
    }
}

double ppDataUtils::GetSummaryPowerGraphNewData(QVector<acSingleGraphData*>& data,
                                                const QVector<QString>& graphLabels,
                                                const gtMap<int, double>& consumptionPerCounter,
                                                double otherCounterConsumption,
                                                const gtVector<int>& counterIds,
                                                ppSessionController* pSessionController)
{
    double apuCounterValue = -1;
    double dGpuCountersTotalValue = 0.0;
    double retTotalVal = -1;
    double total = 0;

    QMap<QString, double> namedCountersDataMap;

    if (pSessionController != nullptr && counterIds.size() > 0 && consumptionPerCounter.size() > 0)
    {
        gtMap<int, double>::const_iterator it = consumptionPerCounter.begin();
        gtMap<int, double>::const_iterator itEnd = consumptionPerCounter.end();

        int id;
        QString tmpStr;

        int apuId = pSessionController->GetAPUCounterID();
        QString apuCounterName;

        // get data map for only relevant counters and change the id to named label
        for (; it != itEnd; it++)
        {
            // check if this map item has a relevant counter id
            id = it->first;

            if (std::find(counterIds.begin(), counterIds.end(), id) != counterIds.end())
            {
                tmpStr = pSessionController->GetCounterNameById(id);
                // Get the short name of the counter (that's how we save it when we add data):
                CutCategoryFromCounterName(tmpStr);
                namedCountersDataMap[tmpStr] = it->second;

                if (id == apuId)
                {
                    //for APU counter - get the value only and dont set the map yet
                    apuCounterValue = it->second;
                    apuCounterName = tmpStr;
                }
                else
                {
                    if (IsDGpuCounter(id, pSessionController))
                    {
                        dGpuCountersTotalValue += it->second;
                    }

                    total += it->second;
                }
            }
        }

        // if the APU coubter is in the selected counters list
        if (apuCounterValue != -1)
        {
            GT_IF_WITH_ASSERT(namedCountersDataMap.count(apuCounterName) > 0)
            {
                namedCountersDataMap.remove(apuCounterName);
                namedCountersDataMap[PP_STR_Counter_Power_Other] = otherCounterConsumption;
            }
            // return the APU counter + dGpu's values as the total value
            retTotalVal = apuCounterValue + dGpuCountersTotalValue;
        }
        else
        {
            // if not - return the total sum of all selected counters
            retTotalVal = total;
        }

        QVector<double> yVec;
        int count = graphLabels.count();

        for (int i = 0; i < count; i++)
        {
            // if there is an existing bar label that is not exist in the new event data - assert (existing counter data is missing)
            GT_IF_WITH_ASSERT(namedCountersDataMap.count(graphLabels[i]) != 0)
            {
                yVec << namedCountersDataMap[graphLabels[i]];
            }
        }

        acSingleGraphData* pSingleData = new acSingleGraphData(yVec);
        data << pSingleData;
    }

    return retTotalVal;
}


// ------- Freq ----------

void ppDataUtils::SummaryFreqGraphPrepareStackedSingleGraphNewData(QVector<double>& yVec,
                                                                   const gtVector<HistogramBucket>& bucketsVec,
                                                                   const QVector<acBarName*>& graphLabels,
                                                                   bool& shouldChangeLabels,
                                                                   QVector<QString>& newLabels,
                                                                   double valueDevUnit)
{
    // get ordered buckets
    QVector<int> orderedIndexs;

    int count = bucketsVec.size();
    QVector<double> inputLowerBoundVec;

    for (int i = 0; i < count; i++)
    {
        inputLowerBoundVec << bucketsVec[i].m_lowerBound;
    }

    double tmpMin;
    int tmpIndex;

    for (int i = 0; i < count; i++)
    {
        tmpMin = 100000;
        tmpIndex = -1;

        for (int j = 0; j < count; j++)
        {
            if (inputLowerBoundVec[j] != -1)
            {
                if (inputLowerBoundVec[j] < tmpMin)
                {
                    tmpMin = inputLowerBoundVec[j];
                    tmpIndex = j;
                }
            }
        }

        orderedIndexs << tmpIndex;
        inputLowerBoundVec[tmpIndex] = -1;
    }

    // get ordered values and bar names from input
    int index;
    QVector<QString> origLabels;

    for (int i = 0; i < count; i++)
    {
        index = orderedIndexs[i];
        newLabels << acBarName::GetBarNameByBounds(bucketsVec[index].m_lowerBound, bucketsVec[index].m_upperBound);
        yVec << bucketsVec[index].m_value / valueDevUnit;
        origLabels << graphLabels[i]->GetName();
    }

    if (origLabels.count() != newLabels.count())
    {
        //PP_TO_DOassert
    }
    else
    {
        // if already found that we need to changed the labels for this run - dont check again
        // supports only the same bar labels for all counters in the same graph
        if (!shouldChangeLabels)
        {
            shouldChangeLabels = SummaryFreqCheckBarTitlesShouldBeChanged(origLabels, newLabels);
        }
    }
}

bool ppDataUtils::SummaryFreqCheckBarTitlesShouldBeChanged(const QVector<QString>& currentxAxisLabels, const QVector<QString>& inputxAxisLabels)
{
    bool retVal = false;
    int count = currentxAxisLabels.count();

    for (int i = 0; i < count; i++)
    {
        if (currentxAxisLabels[i] != inputxAxisLabels[i])
        {
            retVal = true;
            break;
        }
    }

    return retVal;
}

void ppDataUtils::GetSummaryFrequencyDataFromDB(ppDataUtils::GraphViewCategoryType graphType,
                                                gtMap<int, gtVector<HistogramBucket> >& bucketPerCounter,
                                                ppSessionController* pSessionController)
{
    // Histograms bucket widths for offline sessions.
    const unsigned CPU_HISTOGRAM_OFFLINE_BUCKET_WIDTH = 350;
    const unsigned GPU_HISTOGRAM_OFFLINE_BUCKET_WIDTH = 150;

    GT_IF_WITH_ASSERT(nullptr != pSessionController)
    {
        gtVector<int> counterIds;
        GetSummaryGraphCounters(graphType, pSessionController, counterIds);

        if (counterIds.size() > 0)
        {
            if (graphType == SUMMARY_FREQUENCY_CPU)
            {
                pSessionController->GetProfilerBL().GetCurrentFrequenciesHistogram(CPU_HISTOGRAM_OFFLINE_BUCKET_WIDTH, counterIds, bucketPerCounter);
            }
            else if (graphType == SUMMARY_FREQUENCY_GPU)
            {
                pSessionController->GetProfilerBL().GetCurrentFrequenciesHistogram(GPU_HISTOGRAM_OFFLINE_BUCKET_WIDTH, counterIds, bucketPerCounter);
            }
            else
            {
                // Unknown histogram graph type!
                GT_ASSERT(false);
            }
        }
    }
}

void ppDataUtils::GetSummaryFrequencyDataFromEvent(const gtVector<int>& counterIds, unsigned int bucketWidth, unsigned int currSamplingInterval,
                                                   gtMap<int, gtVector<HistogramBucket> >& bucketPerCounter,
                                                   ppSessionController* pSessionController,
                                                   ppQtEventData pSampledDataPerCounter)
{
    GT_IF_WITH_ASSERT(nullptr != pSessionController)
    {
        if (counterIds.size() > 0)
        {
            gtMap<int, PPSampledValuesBatch> eventData = *(pSampledDataPerCounter.data());

            if (counterIds.size() > 0)
            {
                pSessionController->GetProfilerBL().CalculateOnlineFrequencyHistograms(bucketWidth, currSamplingInterval, eventData, counterIds, bucketPerCounter);
            }
        }
    }
}

void ppDataUtils::GetSummaryGraphCounters(GraphViewCategoryType graphType,
                                          ppSessionController* pSessionController,
                                          gtVector<int>& counterIds)
{
    GT_IF_WITH_ASSERT(nullptr != pSessionController)
    {
        GetRelevantCounterIdsByGraphType(counterIds, graphType, pSessionController);
    }
}

//------------------------- General ---------------------

void ppDataUtils::GetRelevantCounterIdsByGraphType(gtVector<int>& allrelevantCounters, GraphViewCategoryType graphType, ppSessionController* pSessionController)
{
    allrelevantCounters.clear();
    AMDTPwrCategory category = AMDT_PWR_CATEGORY_POWER;
    bool isCategoryFound = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(pSessionController != nullptr)
    {
        // get category - default - no category -1
        switch (graphType)
        {
            case SUMMARY_POWER:
            case TIMELINE_POWER:
            case TIMELINE_POWER_DGPU:
                category = AMDT_PWR_CATEGORY_POWER;
                isCategoryFound = true;
                break;

            case SUMMARY_FREQUENCY_CPU:
            case SUMMARY_FREQUENCY_GPU:
            case TIMELINE_FREQUENCY:
                category = AMDT_PWR_CATEGORY_FREQUENCY;
                isCategoryFound = true;
                break;

            case TIMELINE_TEMPERATURE:
                category = AMDT_PWR_CATEGORY_TEMPERATURE;
                isCategoryFound = true;
                break;

            case TIMELINE_CURRENT:
                category = AMDT_PWR_CATEGORY_CURRENT;
                isCategoryFound = true;
                break;

            case TIMELINE_VOLTAGE:
                category = AMDT_PWR_CATEGORY_VOLTAGE;
                isCategoryFound = true;
                break;

            case TIMELINE_CPU_CORE_PSTATE:
            case TIMELINE_CPU_CORE_CSTATE:
                category = AMDT_PWR_CATEGORY_DVFS;
                isCategoryFound = true;
                break;
        }

        // if category exist
        if (isCategoryFound)
        {
            // get counters from project settings
            gtVector<AMDTDeviceType> tmpTyesVec;  // all counters

            if (graphType == SUMMARY_FREQUENCY_CPU)
            {
                // prepare specific types for CPU
                tmpTyesVec.push_back(AMDT_PWR_DEVICE_CPU_CORE);
                tmpTyesVec.push_back(AMDT_PWR_DEVICE_CPU_COMPUTE_UNIT);
            }
            else if (graphType == SUMMARY_FREQUENCY_GPU)
            {
                // prepare specific types for GPU
                tmpTyesVec.push_back(AMDT_PWR_DEVICE_INTERNAL_GPU);
                tmpTyesVec.push_back(AMDT_PWR_DEVICE_EXTERNAL_GPU);
            }

            // get relevant counters for category
            pSessionController->GetEnabledCountersByTypeAndCategory(tmpTyesVec, category, allrelevantCounters);

            // remove irrelevant counters from AMDT_PWR_CATEGORY_DVFS category
            if (graphType == TIMELINE_CPU_CORE_PSTATE ||
                graphType == TIMELINE_CPU_CORE_CSTATE)
            {
                RemoveStateGraphIrrelevantCounters(allrelevantCounters, pSessionController, (graphType == TIMELINE_CPU_CORE_PSTATE));
            }

            // remove irrelevant counters from TIMELINE_POWER_DGPU category
            if (graphType == TIMELINE_POWER_DGPU ||
                graphType == TIMELINE_POWER)
            {
                RemovePowerDgpuGraphIrrelevantCounters(allrelevantCounters, pSessionController, (graphType == TIMELINE_POWER_DGPU));
            }

            //order Ids
            ppAppController::instance().SortCountersInCategory(category, allrelevantCounters);

            // revert Ids list for timeline graphs - graphs is in the opposite order to legend order
            if (graphType == TIMELINE_POWER ||
                graphType == TIMELINE_POWER_DGPU ||
                graphType == TIMELINE_FREQUENCY ||
                graphType == TIMELINE_TEMPERATURE ||
                graphType == TIMELINE_CURRENT ||
                graphType == TIMELINE_VOLTAGE ||
                graphType == TIMELINE_CPU_CORE_PSTATE ||
                graphType == TIMELINE_CPU_CORE_CSTATE)
            {
                // cumulative graphs should be inserted into plot in reverted order (so the first graph will be the upper)
                RevertCountersVecOrder(allrelevantCounters);
            }
        }
    }
}

void ppDataUtils::RevertCountersVecOrder(gtVector<int>& countersVec)
{
    int count = countersVec.size();
    int tmpId, tmpindex;

    for (int i = 0; i < count / 2; i++)
    {
        tmpId = countersVec[i];
        tmpindex = count - i - 1;
        countersVec[i] = countersVec[tmpindex];
        countersVec[tmpindex] = tmpId;
    }
}

void ppDataUtils::CutCategoryFromCounterName(QString& counterName)
{
    if (counterName.contains(PP_STR_SummaryFrequencyCounterPostfix))
    {
        // remove the "Avg. Frequency"
        counterName.remove(PP_STR_SummaryFrequencyCounterPostfix);

        if (counterName.contains(PP_STR_SummaryCPUFrequencyCounterPrefix))
        {
            // remove "CPU " prefix
            counterName.remove(PP_STR_SummaryCPUFrequencyCounterPrefix);
        }
    }
    else if (counterName.contains(PP_STR_SummaryCPUPowerCounterPostfix))
    {
        // if contains is "Power"
        if (counterName.contains(PP_STR_SummaryCPUFrequencyCounterPrefix))
        {
            // if contain "CPU" remove it
            counterName.remove(PP_STR_SummaryCPUFrequencyCounterPrefix);
        }

        // if contain remove "Power
        counterName.remove(PP_STR_SummaryCPUPowerCounterPostfix);
    }
}

void ppDataUtils::RemoveStateGraphIrrelevantCounters(gtVector<int>& allrelevantCounters, ppSessionController* pSessionController, bool isPState)
{
    GT_IF_WITH_ASSERT(pSessionController != nullptr)
    {
        int count = allrelevantCounters.size();

        for (int i = count - 1; i >= 0; i--)
        {
            // if the counter is in percent units - this counter is a cState counter and not pState
            bool isPercentUnits = pSessionController->IsCounterInPercentUnits(allrelevantCounters[i]);

            if ((isPState && isPercentUnits) ||
                (!isPState && !isPercentUnits))
            {
                // remove the irrelevant counters from list
                // for P-state graph - remove the C-state counters
                // for C-state graph - remove the P-state counters
                allrelevantCounters.removeItem(i);
            }
        }
    }
}

void ppDataUtils::RemovePowerDgpuGraphIrrelevantCounters(gtVector<int>& allrelevantCounters, ppSessionController* pSessionController, bool isDgpu)
{
    GT_IF_WITH_ASSERT(pSessionController != nullptr)
    {
        int count = allrelevantCounters.size();
        QString counterName;

        for (int i = count - 1; i >= 0; i--)
        {
            // if the counter is in percent units - this counter is a cState counter and not pState
            counterName = pSessionController->GetCounterNameById(allrelevantCounters[i]);
            bool isDgpuCounter = counterName.contains(PP_STR_TimeLineDgpuCounterPart);

            if ((isDgpu && !isDgpuCounter) ||
                (!isDgpu && isDgpuCounter))
            {
                // remove the irrelevant counters from list
                // for Power graph - remove the dGPU counters
                // for dGPU graph - remove all other power counters
                allrelevantCounters.removeItem(i);
            }
        }
    }
}

bool ppDataUtils::IsDGpuCounter(const int counterId, ppSessionController* pSessionController)
{
    bool result = false;
    GT_IF_WITH_ASSERT(pSessionController != nullptr)
    {
        const AMDTPwrCounterDesc*  descr = pSessionController->GetCounterDescriptor(counterId);

        GT_IF_WITH_ASSERT(descr != nullptr)
        {
            AMDTDeviceType deviceType = static_cast<AMDTDeviceType>(-1);

            if (pSessionController->GetDeviceType(descr->m_deviceId, deviceType))
            {
                result = deviceType == AMDT_PWR_DEVICE_EXTERNAL_GPU;
            }
        }
    }

    return result;
}
