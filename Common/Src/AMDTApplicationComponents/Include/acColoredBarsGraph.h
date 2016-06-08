//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acColoredBarsGraph.h
///
//==================================================================================


#ifndef ACCOOREDBARSGRAPH_H
#define ACCOOREDBARSGRAPH_H

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

#include <AMDTApplicationComponents/Include/acBarsGraph.h>

#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

class AC_API acColoredBarsGraph : public acBarGraph
{
private:
    void SetXAxisKeys(const QVector<QString>& xVecLabels, QVector<double>& xVec);

public:
    acColoredBarsGraph();
    ~acColoredBarsGraph()
    {
        m_xLabelsStringsVector.clear();
    }

    /// Get the vector of X labels:
    QVector<QString>& XLabels() { return m_xLabelsStringsVector; };

    void SetData(const QVector<acBarGraphData*>& data, const bool allowReplot);
    void UpdateData(const QVector<acSingleGraphData*> pSingleKeyData, const bool allowReplot);
    QCPBars* GetBarGraph(int index);
    void DeleteGraph()
    {
        m_xLabelsStringsVector.clear();
        acBarGraph::DeleteGraph();
    }

protected:

    /// Vector of X axis labels:
    QVector<QString> m_xLabelsStringsVector;
};

#endif // ACCOOREDBARSGRAPH_H
