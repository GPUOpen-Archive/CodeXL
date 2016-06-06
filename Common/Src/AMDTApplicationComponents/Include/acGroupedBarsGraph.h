//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acGroupedBarsGraph.h
///
//==================================================================================


#ifndef ACGROUPEDBARSGRAPH_H
#define ACGROUPEDBARSGRAPH_H

#include <AMDTApplicationComponents/Include/acBarsGraph.h>
#include <AMDTApplicationComponents/Include/acStackedBarGraphhData.h>
#include <AMDTApplicationComponents/Include/acStackedBarGraph.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>


class AC_API acGroupedBarsGraph : public acStackedBarGraph
{
protected:
    /// returns the keys values vector (will be the positions of the bar in the xAxis)
    /// \param xVec is output xAxis values vector
    /// \param numOfDataSeries is the number of data series in the graph
    virtual void GetKeysVector(QVector<double>& xVec, const int numOfDataSeries);

public:
    acGroupedBarsGraph();

    /// sets the bars graphs data
    /// \param countersData is the data to be set
    /// \param allowReplot - if true will cause Replot
    virtual void SetData(QVector<acBarGraphData*>& countersData, bool allowReplot);

    /// up[ates the new data to graph
    /// \param pSingleKeyData is the new data to be set into the existing graph
    /// \param allowReplot - if true will cause Replot
    virtual double GetMaxValueForYAxisRangeCalculations(const QVector<acSingleGraphData*> pSingleKeyDataVec) const;
};

#endif // ACGROUPEDBARSGRAPH_H