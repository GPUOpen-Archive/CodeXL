//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acMultiLinePlotData.h
///
//==================================================================================


#ifndef ACMULTILINEPLOTDATA_H
#define ACMULTILINEPLOTDATA_H

#include <qcustomplot.h>
#include <qstring.h>
#include <QtWidgets>

#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

class AC_API acMultiLinePlotItemData
{
public:
    acMultiLinePlotItemData(QVector<double> KeyVec, QVector<double> valueVec, const QColor& color,
                            const QString& graphName, const QString graphDescription, bool disableGraph = false)
    {
        m_keyVec = KeyVec;
        m_valueVec = valueVec;
        m_color = color;
        m_graphName = graphName;
        m_description = graphDescription;
        m_shouldBeDisabled = disableGraph;
    }

    ~acMultiLinePlotItemData()
    {
        m_keyVec.clear();
        m_valueVec.clear();
    }

    /// get graphs keys vector
    /// \returns the vector of keys
    QVector<double>& GetKeyVec() { return m_keyVec; }

    /// get graphs values vector
    /// \returns the vector of values
    QVector<double>& GetValueVec() { return m_valueVec; }

    /// get graphs color
    /// \returns the color of the graph
    QColor& GetColor() { return m_color; };

    /// gets the graph name
    /// \returns the name of the graph
    QString& GetGraphName() { return m_graphName; }

    /// get graphs description
    /// \returns a description of the graph
    QString GetDescription() { return m_description; }

    /// should the graphs be disabled - not shown in the plot and disabled in the legend
    /// \returns true if the graph should be disabled
    bool ShouldGraphBeDisabled() { return m_shouldBeDisabled; }

private:
    /// keys vector
    QVector<double> m_keyVec;
    /// values vector
    QVector<double> m_valueVec;
    /// graphs color
    QColor m_color;
    /// graphs name
    QString m_graphName;
    /// graphs description
    QString m_description;

    /// should the graph not be shown in the plot and disabled in the legend
    bool m_shouldBeDisabled;
};

#endif