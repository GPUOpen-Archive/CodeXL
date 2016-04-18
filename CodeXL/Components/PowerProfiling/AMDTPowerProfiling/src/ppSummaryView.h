//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppSummaryView.h
///
//==================================================================================

//------------------------------ ppSummaryView.h ------------------------------

#ifndef __PPSUMMARYVIEW_H
#define __PPSUMMARYVIEW_H

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtList.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTApplicationComponents/Include/acBarsGraph.h>
#include <AMDTApplicationComponents/Include/acSplitter.h>
#include <AMDTApplicationComponents/Include/acColoredBarsGraph.h>
#include <AMDTApplicationComponents/Include/acGroupedBarsGraph.h>

// Power profiler MidTier classes
#include <AMDTPowerProfilingMidTier/include/PowerProfilerCore.h>
#include <AMDTPowerProfiling/src/ppAppController.h>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>
#include <AMDTPowerProfiling/src/ppDataUtils.h>


// Forward declaration
class ppSessionController;
class ppSessionView;


#define MEASUREMENTUNIT_POWER_BASE 1000

class PP_API ppSummaryView : public acSplitter
{
    Q_OBJECT

public:

    /// Constructor:
    ppSummaryView(ppSessionView* pParentSession, ppSessionController* pSessionController);
    virtual ~ppSummaryView();

    /// Update the summary view with the stopped profile session data
    /// \param sessionName - session name
    void OnProfileStopped(const QString& sessionName);

    /// updates the session information window
    void UpdateSessionInformation();

private slots:
    /// Handles the new data emit
    /// \param pSampledDataPerCounter - the new data
    void OnNewProfileData(ppQtEventData pSampledDataPerCounter);

    /// handles change of selected counters from project settings or dialog
    void OnSelectedCountersChanged();

    /// Is handling the move of the top splitter. The slot is handling the synchronization of both top splitters:
    /// \param index the index of the moved element
    /// \param position the positions moved
    void OnTopSplitterMoved(int index, int position);

private:
    enum PowerGraphMeasurementUnit : unsigned long
    {
        MEASUREMENTUNIT_JOULES = 0,
        MEASUREMENTUNIT_KILOJOULES = 1,
        MEASUREMENTUNIT_MEGAJOULES = 2,
        MEASUREMENTUNIT_GIGAJOULES = 3,
        MEASUREMENTUNIT_TERAJOULES = 4
    };

    /// inits the graph scrolled area and its widgets
    void InitGraphsArea();

    /// calls to all graphs init functions
    void InitGraphs();

    /// inits the Power graph
    void InitPowerGraphs();

    /// inits the CPU graph
    void InitCpuGraph();

    /// inits the CPU graph
    void InitGpuGraph();

    /// calls to all graphs set data functions
    /// \param data from event - true if this function is called from new data event
    void SetGraphsData(bool isDataFromEvent);

    /// sets the Power graph with the new event data
    /// \param isDataFromEvent true if this function is called from new data event
    /// \param sampledGraphType power / energy
    void SetPowerOrEnergyGraphData(bool isDataFromEvent, const ppDataUtils::PowerGraphType sampledGraphType);

    /// sets the CPU/GPU graphs with the new event data
    /// \param data from event - true if this function is called from new data event
    /// \graphType - the graph type to be set (CPU/GPU)
    void SetFreqGraphData(bool isDataFromEvent, ppDataUtils::GraphViewCategoryType graphType);

    /// calls to all graphs update data functions
    void UpdateGraphs();

    /// updates Power graph with new data from event
    void UpdatePowerGraphData();

    /// Sets the power and energy graphs caption (the current average / cumulative energy / power) values:
    void SetPowerGraphsCaption();

    /// updates CPU/GPU graphs with new data from event
    /// \param graph - the graph to be updated
    /// \graphType - the graph type
    void UpdateStackedBarGraphData(acStackedBarGraph* graph, ppDataUtils::GraphViewCategoryType graphType);

    /// gets the counter ids list per graph type, from the counters per graph map
    /// \param type is the graph type
    /// \param selectedCounters is list of counter ids related for this graph
    /// \returns if function sucseeded or not
    bool GetCounterIdsByGraphType(ppDataUtils::GraphViewCategoryType type, gtVector<int>& selectedCounters);

    /// inits the ciynters ids per graph type map
    void SetSelectedCounterIdsPerGraphMap();

    /// calculates and sets the duration label
    /// \param miliRange is the duration in milliseconds
    void SetDurationLabel(int miliRange);

    /// Updates the current measurement units of cumulative energy graph.
    /// Also, the function updates the values in the data vector, according to the new measurement units:
    /// \param dataVec is the values vector
    template<class T>
    void UpdateEnergyGraphMeasurementUnit(const QVector<T*>& dataVec);

    QString GetPowerGraphUnit(ppDataUtils::PowerGraphType graphType);

    /// replot all visible graphs
    void ReplotAllGrpahs();

    QLabel* m_pSessionDurationLabel;

    /// session info
    acQHTMLWindow* m_pHTMLInfoWindow;

    /// power upper graph:
    acColoredBarsGraph* m_pPowerGraph;

    /// energy upper graph:
    acColoredBarsGraph* m_pEnergyGraph;

    /// CPU freq graph
    acGroupedBarsGraph* m_pFreqCpuGraph;

    /// GPU freq graph
    acGroupedBarsGraph* m_pFreqGpuGraph;

    /// graphs initialization flag
    bool m_isGraphsDataInitialized;

    /// Splitters:
    acSplitter* m_pTopRightHorizontalSplitter;
    acSplitter* m_pTopLeftHorizontalSplitter;

    /// last event data
    gtMap<int, double> m_lastEventPowerGrpahData;       /// last cumulative power graph event data
    double m_lastEventPowerOtherCounterData;            /// last cumulative power 'Other' counter data
    gtMap<int, double> m_lastEventAverageData;          /// last average power graph event data
    double m_lastEventAverageOtherCounterData;          /// last average power 'Other' counter data
    gtMap<int, unsigned> m_lastAggregatedQuantized;     /// for BL use only - for calculating the average
    gtMap<int, gtVector<HistogramBucket> > m_lastCPUBucketsPerCounter;  /// last CPU graph data from event
    gtMap<int, gtVector<HistogramBucket> > m_lastGPUBucketsPerCounter;  /// last GPU graph data from event

    ppSessionController* m_pSessionController;  /// session controller
    unsigned int m_samplingInterval;        /// sampling intervals
    ppSessionView* m_pParentSession;    /// Contain the parent session view:
    QMap<ppDataUtils::GraphViewCategoryType, gtVector<int> > m_selectedCunterIdsPerGraphMap;    /// map of graph types and the counter ids selected fro them

    /// Contain the current measurement unit for the cumulative power graph. The unit is changes according to the power value scale:
    PowerGraphMeasurementUnit m_currentPowerMeasurementUnit;

    /// Contain the last APU counter average value:
    double m_apuCounterLastAverageValue;

    /// Contain the last APU counter cumulative value:
    double m_apuCounterLastCumulativeValue;
    bool m_OnNewProfileDataConnected = false;
};

#endif // __PPSUMMARYVIEW_H
