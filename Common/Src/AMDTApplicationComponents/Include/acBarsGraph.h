//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acBarsGraph.h
///
//==================================================================================


#ifndef ACBARSGRAPH_H
#define ACBARSGRAPH_H

#include <qcustomplot.h>

#include <QtWidgets>
#include <qobject.h>
#include <qstring.h>
#include <qvector.h>

#include <AMDTApplicationComponents/Include/acBarGraphData.h>
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

#define TICK_LEN_OUT 2
#define TICK_LEN_IN  1
#define TICK_LABEL_PADDING 10
#define BARS_X_KEYS_STEPS 10
#define BARS_PEN_WIDTH 1

class AC_API acSingleGraphData
{
public:
    acSingleGraphData(const QVector<double>& yData, int index = 0)
    {
        m_yData = yData;
        m_graphIndex = index;
    }

    ~acSingleGraphData()
    {
        m_yData.clear();
    }

    int m_graphIndex;
    QVector<double> m_yData;
};

class AC_API acBarName
{
public:
    acBarName(double lower, double upper)
    {
        m_lowerBound = lower;
        m_upperBound = upper;

        m_name = GetBarNameByBounds(lower, upper);
    }

    static QString GetBarNameByBounds(const double lower, const double upper)
    {
        QString name;

        if (lower == upper)
        {
            name = QString("%1").arg(lower);
        }
        else
        {
            name = QString("%1-%2").arg(lower).arg(upper);
        }

        return name;
    }
    void GetRange(double& lower, double& upper)
    {
        lower = m_lowerBound;
        upper = m_upperBound;
    }

    QString GetName() { return m_name; }

    double m_lowerBound;
    double m_upperBound;
    QString m_name;
};

class AC_API acBarGraph : public QObject
{
    Q_OBJECT
public:
    ~acBarGraph();

    // init graph customPlot member, Axis properties, labels and titles
    void Init(const double xUpperBound, const double xTickStep,
              const double yUpperBound, const double yTickStep,
              const QString& xAxisTitle, const QString& yAxisTitle,
              bool allowReplot,
              bool isLegendRequired = false);

    QCustomPlot* GetPlot();
    void Replot();

    int GetNumOfDataSeries() { return m_dataSeriesVec.size(); }
    int GetNumOfBars() { return m_numOfBars; }

    /// Sets the graph title. The title will be located at the top left corner of the graph:
    /// \param graphTitle the requested graph title.
    void SetGraphTitle(const QString& graphTitle);

    virtual void DeleteGraph();

    virtual void UpdateData(const QVector<acSingleGraphData*> pSingleKeyData, const bool allowReplot) = 0;
    virtual QCPBars* GetBarGraph(int index = 0) = 0;
    virtual void SetData(const QVector<acBarGraphData*>& data, const bool allowReplot) = 0;
    int GetBarGraphIndexByName(QString name);
    void SetGraphYAxisTitle(QString title);
    void SetGraphYRange(int low, int high);

    /// sets xAxis tick label rotation degree
    /// \param degree is the rotation angle degree (+right, -left)
    void SetXAxisTickLabelRotation(double degree);

    /// decreases the font size of the xAxis tick Label
    /// \param dec is how much to decrease from current value
    void DecreaseXAxisTickLabelFont(int dec);

    /// sets the XAxis tick label padding
    /// \param padding
    void SetXAxisTickLabelPadding(int padding);

public slots:
    void OnPlotHovered(QMouseEvent* pMouseEvent);

protected:
    acBarGraph();
    int SetBarsData(const QVector<double>& yData, QCPBars* bars, QVector<double>& xData, int barWidth);
    void ResetGraphYRange(int maxBarVal);

    /// Virtual function. Will be derived in inherited classes if needed:
    virtual void CreateGraphLayout();


    QCustomPlot* m_pCustomPlot;
    QVector<QCPBars*> m_dataSeriesVec;
    int m_numOfBars;

    /// true iff a legend should be created:
    bool m_shouldCreateLegend;

    /// Contains the graph caption (will be located in top left corner of the graph):
    QString m_graphTitle;

    /// Contain the QCustomPlot left title object (needed for dynamic update):
    QCPPlotTitle* m_pGraphPlotTitle;
};

#endif // ACBARSGRAPH_H
