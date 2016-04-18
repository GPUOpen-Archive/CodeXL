//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppMultiLinePowerStackedPlot.h
///
//==================================================================================


#ifndef PP_MULTILINESTACKEDPLOT
#define PP_MULTILINESTACKEDPLOT

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

#include <AMDTPowerProfiling/src/ppMultiLinePlot.h>

class ppMultiLnePowerStackedPlot : public ppMultiLinePlot
{
    Q_OBJECT
public:
    /// constructor for class
    ppMultiLnePowerStackedPlot(ppSessionController* pSessionController);

    /// Sets graph first time data, if its from first event (one key)or only once from data (multi keys)
    virtual void SetGraphInitializationData();

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

    /// adds the APU counter value (total value) to the counters values vector
    /// \param valVec - the values vector
    /// \param apuCounterIndex - APU counter Id
    /// \param cumulativeValue - sum values of all selected counters (without APU)
    virtual void SetApuAndOtherCountersValueToVec(QVector<double>& valVec, int apuCounterIndex, double cumulativeValue);

    /// add creation of graph for Total APU value
    /// \param graphData - vector of data for graphs creation
    /// \param apuValueVec - values of APU counter
    virtual void SetTotalApuGraph(QVector<acMultiLinePlotItemData*>& graphData, const QVector<double>& apuValueVec);

    /// should the graph (by it's id) be disabled
    /// \param vec is the selected counters ids vector
    /// \param id is the asked counter id
    /// \returns true if the counter should be disabled in the legend
    bool ShouldDisableGraph(const gtVector<int>& vec, int id);

    // graph name of Total
    QString m_totalGraphName;
};

#endif
