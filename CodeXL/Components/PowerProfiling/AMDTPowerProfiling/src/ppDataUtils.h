//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppDataUtils.h
///
//==================================================================================


#ifndef PPDATAUTILS_H
#define PPDATAUTILS_H

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Framework:
#include <AMDTApplicationComponents/Include/acColoredBarsGraph.h>
#include <AMDTApplicationComponents/Include/acMultiLinePlotData.h>

#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtSet.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// Local:
#include <AMDTPowerProfiling/src/ppAppController.h>
#include <AMDTPowerProfiling/src/ppSessionController.h>

class ppDataUtils
{
public:
    enum GraphViewCategoryType
    {
        TIMELINE_POWER,
        TIMELINE_POWER_DGPU,
        TIMELINE_FREQUENCY,
        TIMELINE_TEMPERATURE,
        TIMELINE_VOLTAGE,
        TIMELINE_CURRENT,
        TIMELINE_CPU_CORE_PSTATE,
        TIMELINE_CPU_CORE_CSTATE,
        TIMELINE_CATEGORY_TYPES_COUNT = TIMELINE_CPU_CORE_CSTATE,
        SUMMARY_POWER,
        SUMMARY_FREQUENCY_CPU,
        SUMMARY_FREQUENCY_GPU
    };

    enum PowerGraphType
    {
        POWERGRAPHTYPE_CUMULATIVE = 1,
        POWERGRAPHTYPE_AVERAGE
    };

    enum StackedBarGraphValueType
    {
        STACKEDBARGRAPH_VALUETYPE_MILLISECONDS = 1,
        STACKEDBARGRAPH_VALUETYPE_SECONDS = 1000,
        STACKEDBARGRAPH_VALUETYPE_MINUTSS = 60 * STACKEDBARGRAPH_VALUETYPE_SECONDS
    };

    static void GeSummarytPowerGraphDataFromEvent(const ppQtEventData pSampledDataPerCounter,
                                                  gtMap<int, double>& accumulatedEnergyInJoule, double& cumulativeOtherCounterValueInJoule,
                                                  gtMap<int, double>& averagePowerInWatt, double& averageOtherCounterValueInWatt,
                                                  gtMap<int, unsigned>& aggregatedQuantized,
                                                  unsigned int currSamplingInterval,
                                                  ppSessionController* pSessionController);

    static void SummaryFreqGraphPrepareStackedSingleGraphNewData(QVector<double>& yVec,
                                                                 const gtVector<HistogramBucket>& bucketsVec,
                                                                 const QVector<acBarName*>& graphLabels,
                                                                 bool& shouldChangeLabels,
                                                                 QVector<QString>& newLabels,
                                                                 double valueDevUnit);

    /// gets power graph data for graph initialization
    /// \param data - output data to be set to the graph
    /// \param smapledGrapgType - input type of the power graph (cumulative / average)
    static double GetSummaryPowerGraphInitializationDataFromDB(QVector<acBarGraphData*>& data, double& otherCounterValue,
                                                               const PowerGraphType smapledGrapgType,
                                                               ppSessionController* pSessionController,
                                                               const gtVector<int>& counterIds);

    static double GetSummaryPowerGraphInitializationDataFromEvent(QVector<acBarGraphData*>& counterDataVec,
                                                                  gtMap<int, double>& bucketsData, double otherCounterData,
                                                                  const gtVector<int>& counterIds,
                                                                  ppSessionController* pSessionController);


    /// gets power graph new data for existing and initialized graph
    /// \param data - output data to be set to the graph
    /// \param smapledGrapgType - input type of the power graph (cumulative / average)
    /// \param graphLabels - as the counters map is not ordered by the graph exisiting bars, we need the bars labels to set the ordered values vector
    static double GetSummaryPowerGraphNewData(QVector<acSingleGraphData*>& data,
                                              const QVector<QString>& graphLabels,
                                              const gtMap<int, double>& consumptionPerCounter,
                                              double otherCounterConsumption,
                                              const gtVector<int>& counterIds,
                                              ppSessionController* pSessionController);

    static void GetSummaryFrequencyDataFromDB(ppDataUtils::GraphViewCategoryType graphType,
                                              gtMap<int, gtVector<HistogramBucket> >& bucketPerCounter,
                                              ppSessionController* pSessionController);

    static void GetSummaryFrequencyDataFromEvent(const gtVector<int>& counterIds, unsigned int bucketWidth, unsigned int currSamplingInterval,
                                                 gtMap<int, gtVector<HistogramBucket> >& bucketPerCounter,
                                                 ppSessionController* pSessionController,
                                                 ppQtEventData pSampledDataPerCounter);

    static void GetRelevantCounterIdsByGraphType(gtVector<int>& allrelevantCounters,
                                                 GraphViewCategoryType graphType,
                                                 ppSessionController* pSessionController);

    /// Extracts a short name from a counter name (cuts off the category).
    /// For example: CPU Core0 Frequency -> Core0
    ///              CPU CU0 Power -> CPU CU0
    /// \param counterName [in + out] the original counter name. The function will edit the parameter itself and will cut off the category
    static void CutCategoryFromCounterName(QString& counterName);

private:
    static void GetSummaryGraphCounters(GraphViewCategoryType graphType,
                                        ppSessionController* pSessionController,
                                        gtVector<int>& counterIds);

    static void RevertCountersVecOrder(gtVector<int>& countersVec);

    /// checks if the bar titles should be changed (bucket bounds changed)
    /// \param currentLabels is the existing graph bar titles
    /// \param inputLabels is the new bar titles needed
    static bool SummaryFreqCheckBarTitlesShouldBeChanged(const QVector<QString>& currentLabels,
                                                         const QVector<QString>& inputLabels);

    static QColor GetOthersGrpahColor();

    /// removes from counters list the irrelevant counters for the P-State / C-State graphs
    /// \allrelevantCounters the original counters list and also the output counters list
    /// \pSessionController is a pointer to the session controller
    /// \param isPState is true when the graph is a P-state graph, and false when the graph is C-State graph
    static void RemoveStateGraphIrrelevantCounters(gtVector<int>& allrelevantCounters, ppSessionController* pSessionController, bool isPState);

    /// removes from counters list the irrelevant counters for the Power/dGPU graphs
    /// \allrelevantCounters the original counters list and also the output counters list
    /// \pSessionController is a pointer to the session controller
    /// \param isDgpu is true when the graph is a dGPU graph, and false when the graph is regular Power graph
    static void RemovePowerDgpuGraphIrrelevantCounters(gtVector<int>& allrelevantCounters, ppSessionController* pSessionController, bool isDgpu);
    /// returns true if counter is dGpu counter
    /// \param counterId input parameter
    /// \param pSessionController input parameter session controller to extract device id and type
    static bool IsDGpuCounter(const int counterId, ppSessionController* pSessionController);
};

#endif
