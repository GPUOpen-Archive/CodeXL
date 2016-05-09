//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppMultiLinePlot.h
///
//==================================================================================

#ifndef PP_MULTILINTPLOT
#define PP_MULTILINTPLOT

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Local
#include <AMDTPowerProfiling/src/ppDataUtils.h>

// Power profiler midtier classes
#include <AMDTPowerProfilingMidTier/include/PowerProfilerBL.h>

// Framework:
#include <AMDTApplicationComponents/Include/acMultiLinePlot.h>


class ppMultiLinePlot : public acMultiLinePlot
{
    Q_OBJECT
public:

    /// constructor for class
    ppMultiLinePlot(ppSessionController* pSessionController);

    /// Set the plot properties:
    /// \param xAxisTitle x Axis title
    /// \param yAxisTitle y Axis title
    /// \param graphCategory graph category
    /// \param graphType the graph type
    /// \param valuesType the graph values type (double / int)
    /// \param unitsStr string describing the legend units
    void InitPlot(const QString& xAxisTitle, const QString& yAxisTitle, AMDTPwrCategory graphCategory, ppDataUtils::GraphViewCategoryType graphType, GraphValuesType valuesType, const QString& unitsStr);

    /// sets graph first time data, if its from first event (one key)or only once from data (multi keys)
    virtual void SetGraphInitializationData();

    /// Sets the plot and table visibility:
    void SetShown(bool isShown, bool shouldUpdateVisibility);

    /// Is the plot and table visible?
    bool IsShown() const { return m_isShown; }

    /// Updates the plot and table visibility. This function should be called after the parent view is initialized,
    /// therefore is separated from "SetShown"
    void UpdateVisibility();

    /// updates the graphs range
    /// \param isReplotNeeded - is replot needed at the end of the range reset
    /// \param samplingRange - the range to be set
    void UpdatePlotRange(bool isReplotNeeded, SamplingTimeRange samplingRange);

    /// updates existing plotwith new key and value (value for each graph in plot)
    /// \param pSampledDataPerCounter - event data for all counters
    /// \param samplingRange - is the range for the plot
    /// \param bShouldUpdateLastSample - prevents updating values table last sample during real time tool tips
    void AddNewDataToExistingGraph(ppQtEventData pSampledDataPerCounter, SamplingTimeRange samplingRange, bool bShouldUpdateLastSample);

    /// checks if there relevant counters for this graph type
    bool IsRelevantCountersVecEmpty() const { return (m_relevantCountersVec.size() == 0); }

    /// init the graph type with the relevent counters and creates empty graph per coiunter
    void InitPlotWithSelectedCounters();

    /// deletes the Multi graph
    void Delete();

    /// Return the graph type:
    ppDataUtils::GraphViewCategoryType GraphType() const { return m_graphType; }

    /// set all graphs check/unchecked in legend, by the input vector
    /// each item in the vector relates to the same index graph.
    /// the graph is index x will be checked/unchecked according to the value in hiddenStateVec[x]
    /// \param hiddenStateVec is an input vector with true(uncheck)/false(check) values
    void SetPowerGraphLegendCheckState(QVector<bool> hiddenStateVec);

    /// resets the info table column with so that the value won't be hidden
    void ResetPlotInfoTableColumnWidth();

protected slots :
    /// when add/remove button created - opens the dialog box
    void OnAddRemoveCountersAccepted();

protected:
    /// prepares the data for building the graph for the first time
    /// \param graphData - vector of data to be set into the graph
    virtual void PrepareTimeLineSelectedCountersInitData(QVector<acMultiLinePlotItemData*>& graphData);

    /// gets the data from dB and prepares it for building the graph
    /// \counterIds - relevant counters ids for this graph type
    /// \param pSampledDataPerCounter - event data
    /// \param graphData - graphs data to be set into the graph
    virtual void PrepareTimelineDataFromDB(const gtVector<int>& counterIds,
                                           SamplingTimeRange& samplingTimeRange,
                                           QVector<acMultiLinePlotItemData*>& graphData);

    /// prepares the data from the evnt for adding to the existing graph
    /// \param pSampledDataPerCounter - event data
    virtual void PrepareTimelineDataFromNewDataEvent(ppQtEventData pSampledDataPerCounter,
                                                     QVector<acMultiLinePlotItemData*>& graphData);

    /// part of AddNewDataToExistingGraph - sets the vector of value by existing graphs order, with the new event data
    /// \param pSampledDataPerCounter - event data
    /// \param valVec - the counters values vector for the specific time key, ordered by the graphs order in the plot
    virtual void GetValueVecFromEventData(ppQtEventData pSampledDataPerCounter, QVector<double>& valVec);

    /// hides the specific graph by removing its value from all graphs above it.
    /// it's own value is the non cumulative value.
    /// the total graph will always remain the same value regardless the hidden graphs below it.
    /// the other graphs acts as all other graphs.
    /// \param index - the removed graph index in graphs vector
    /// \param hide - true=hide, false=show
    virtual void HideSingleLineGraphFromCumulative(int index, bool hide);


    ppSessionController* m_pSessionController;      /// session controller
    AMDTPwrCategory m_graphCategory;                /// graph category
    ppDataUtils::GraphViewCategoryType m_graphType; // graph type
    gtVector<int> m_relevantCountersVec;            /// graphs selected relevant counter ids
    bool m_isShown;                                 /// Are the table and plot shown?
};

#endif
