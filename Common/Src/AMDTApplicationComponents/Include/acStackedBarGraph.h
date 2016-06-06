//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acStackedBarGraph.h
///
//==================================================================================


#ifndef ACSTACKEDBARGRAPH_H
#define ACSTACKEDBARGRAPH_H

#include <AMDTApplicationComponents/Include/acBarsGraph.h>
#include <AMDTApplicationComponents/Include/acStackedBarGraphhData.h>
#include <AMDTBaseTools/Include/gtVector.h>

#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>


class AC_API acStackedBarGraph : public acBarGraph
{
public:
    acStackedBarGraph();
    ~acStackedBarGraph()
    {
        m_xLabels.clear();
    }

    /// sets the bars graphs data
    /// \param countersData is the data to be set
    /// \param allowReplot - if true will cause Replot
    virtual void SetData(const QVector<acBarGraphData*>& countersData, const bool allowReplot);

    /// up[ates the new data to graph
    /// \param pSingleKeyData is the new data to be set into the existing graph
    /// \param allowReplot - if true will cause Replot
    virtual void UpdateData(const QVector<acSingleGraphData*> pSingleKeyData, const bool allowReplot);

    /// get specific bar graph from current plot
    /// \param index of added bar to custom plot
    /// \returns pointer to the specific graph
    QCPBars* GetBarGraph(int index);

    /// delete graph
    void DeleteGraph();

    /// set the XAxis labels
    /// \param xVecLabels the labels to be set
    void SetXAxisLabelsVector(const QVector<QString>& xVecLabels);

    int XLabelsCount() const { return m_xLabels.count(); };

    QVector<acBarName*>& XLabelsVector() { return m_xLabels; };
protected:
    /// setting xAxis keys, ticks and range
    /// \param xLabels is input data labels to be set to the graph bars
    /// \param xVec is output xAxis values vector
    /// \param numOfDataSeries is the number of data series in the graph
    void SetXAxisKeys(const QVector<acBarName*>& xLabels, QVector<double>& xVec, const int numOfDataSeries = 0);

    /// gets the total value for each cumulative bar(per key)and return the max
    /// \param returns the total max
    int GetStackedMaxValue();

    /// create the graph legend next to the graph (and not inside it as in qCustomPlot)
    void CreateGraphLayout();

    /// returns the keys values vector (will be the positions of the bar in the xAxis)
    /// \param xVec is output xAxis values vector
    /// \param numOfDataSeries is the number of data series in the graph
    virtual void GetKeysVector(QVector<double>& xVec, const int numOfDataSeries);

    /// gets the maximum bar YAxis value - for calculating the yAxis range
    /// \param pSingleKeyDataVec is the input data of all data series
    /// \return the max YAxis value
    virtual double GetMaxValueForYAxisRangeCalculations(const QVector<acSingleGraphData*> pSingleKeyDataVec) const;

    /// xAxis labels list
    QVector<acBarName*> m_xLabels;

    /// internal layout grid - stores legend and the dummy element
    QCPLayoutGrid* m_pInternalLayoutGrid;

    /// dummy element
    QCPLayoutElement* m_pInternalBottomElement;
};

#endif // ACSTACKEDBARGRAPH_H