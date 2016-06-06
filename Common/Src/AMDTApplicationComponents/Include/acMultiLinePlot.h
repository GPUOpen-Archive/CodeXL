//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acMultiLinePlot.h
///
//==================================================================================


#ifndef ACMULTILINEPLOT_H
#define ACMULTILINEPLOT_H

#include <qcustomplot.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>
#include <AMDTApplicationComponents/Include/acMultiLinePlotData.h>
#include <AMDTApplicationComponents/Include/acCustomPlot.h>
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/Include/acVectorLineGraph.h>

// the check-box column number in the legend
#define LEGEND_CHECKBOX_COLUMN 0

class AC_API acSingleLineGraph : public acVectorLineGraph
{
public:
    acSingleLineGraph(QCPAxis* keyAxis, QCPAxis* valueAxis, bool disable = false) : acVectorLineGraph(keyAxis, valueAxis)
    {
        m_shouldBeDisabled = disable;
    }

    void SetHidden(bool hide) { m_isHidden = hide; }
    virtual void draw(QCPPainter* painter);
    bool IsHidden() { return m_isHidden; }
    QCPRange GetKeyRange(bool& foundRange) { return getKeyRange(foundRange); }
    QColor GetColor() { return m_color; }
    void SetColor(QColor color) { m_color = color; }
    void SetDescription(const QString graphDescription) { m_description = graphDescription; }
    QString GetGraphDescription() { return m_description; }

    bool ShouldBeDisabled() { return m_shouldBeDisabled; }

private:
    bool m_isHidden;
    QColor m_color;
    QString m_description;

    bool m_shouldBeDisabled;
};

class AC_API acMultiLinePlot : public QObject
{
    Q_OBJECT
public:
    enum GraphValuesType
    {
        GRAPHVALUESTYPE_INT,
        GRAPHVALUESTYPE_DOUBLE
    };

    acMultiLinePlot(bool isCumulative, bool isLastGraphTotal);

    ~acMultiLinePlot()
    {
    }

    /// Initializes the plot properties:
    void InitPlotAxesLabels(const QString& xLabel, const QString& yLabel);

    void Replot();
    bool IsPlotVisible() { return !m_pCustomPlot->visibleRegion().isEmpty(); }

    acCustomPlot* GetPlot() const { return m_pCustomPlot; }
    acSingleLineGraph* GetGraph(int index) const { return m_pGraphsVec[index]; }
    void InitPlotInfoTable();
    void ResetPlotInfoTable();
    void SetInfoTableBySpecificTimePoint(const double& timeKey);
    void GetGraphsNames(QVector<QString>& names) const;
    void HideGraph(int i, bool hide);
    void HideGraph(const QString& name, bool hide);

    /// hides the specific graph by removing its value from all graphs above it.
    /// it's own value becomes the non cumulative value.
    /// \param index - the removed graph index in graphs vector
    /// \param hide - true=hide, false=show
    virtual void HideSingleLineGraphFromCumulative(int index, bool hide);

    void AddDataToTimeLineGraph(const double key, const QVector<double>& valVec,
                                const double xRangeStart, const double xRangeEnd,
                                bool removeOld, bool allowReplot);
    void ChangeGraphRangeByMidPoint(double userMidIndex, double userRange);
    void ChangeGraphRangeByBothPoints(double startPoint, double endPoint);

    void InitPlotGraphs(QVector<acMultiLinePlotItemData*>& graphItemsData, bool allowReplot);
    void InitPlotWithEmptyGraphs(QVector<acMultiLinePlotItemData*>& graphItemsData, bool allowReplot);
    acSingleLineGraph* InitPlotInner(acMultiLinePlotItemData* graphItemData, QColor& graphColor);

    int GetGraphIndexByName(const QString& name) const;

    int GetNumberOfDataSeriesInGraph() const { return m_pGraphsVec.count(); }

    bool IsGraphCumulative() const { return m_isCumulative; }

    bool HasLegend() const { return (NULL != m_pPlotInfoTable); }

    bool IsLastDataSeriesTotal() const { return m_isLastGraphTotal; }

    QCPRange GetGraphRange() const;

    void HideXAxisLabels(bool hide);

    acListCtrl* GetPlotInfoTable() const { return m_pPlotInfoTable; }

    virtual void GetGraphsValues(double key, QMap<QString, double>& names_values);

    virtual void SetPlotToolTip(const QString& toolTipText, int nTrackingLineAbscissa);

    void EnableInfoTableAddRemoveCounters(bool enable);

    bool GetInfoTableSelectedGraphValue(QString& graphName, QString& valueStr) const;

    bool GetInfoTableValueByGraphName(gtString graphName, double& value) const;

    double getGraphValueByKey(acSingleLineGraph* graph, const double& key) const;

    /// Set the tooltip HTML style:
    static void SetTooltipHTMLStyle(const QString& style) { m_sTooltipHTMLStyle = style; };

    /// Initialize tracking line tool tip
    void InitTrackingToolTip();

    /// checks if the graph is disabled by it's index
    /// \param graphIndexthe graph index
    /// \returns true if the graph should be disabled - means disabled in the legend and not shown in the plot
    bool IsGraphDisabledAtIndex(int graphIndex);

    /// gets the hidden/not hidden state of all graphs (checked/unchecked in legend)
    /// \param hiddenStateVec as an input vector. element x in the vector will represent graph[x].m_isHidden value
    void GetAllGraphsHiddenState(QVector<bool>& hiddenStateVec);

signals:

    // Is emitted when the "add / remove" counters line is activated:
    void AddRemoveActivated();
    void TrackingXAxis(double key, int nTrackingLineAbscissa);

public slots: // to del if protected works
    void OnPlotHovered(QMouseEvent* pMouseEvent);
    int UpdateTrackingTime(int nOldKey);

protected slots:
    /// this function is called when one of the graph is selected by user. will cause the legend relevant row to be selected as well
    void OnGraphSelectionChanged();

    /// this function called on double clicking legends row. it will activate the legends add/remove row and open the counters dialog box
    /// \param pItem - the selected item
    void OnLegendTableItemActivate(QTableWidgetItem* pItem);

    /// will be called when the cell content is change. will check if the check-box was selected/unselected
    /// \param row - cell row index
    /// \param col - cell col index
    void OnCellClicked(int row, int column);

    /// will be called on selected legend row changed.
    void OnInfoTableSelectedItemChanged();

    /// handles the fill of colors between graphs
    void OrderGraphChannelFill();

private:

    void HideGraphsLegend(int i, bool hide);
    void ClearGraph();

    /// resets the graph y axis range
    /// \param maxBarVal is the value to be set for the upper range - in case it is larger then the current one
    /// \param minNegValue is the value to be set for the lower range - in case it is smaller then the current one (has to be negative value)
    void ResetGraphYRange(int maxBarVal, int minNegValue);

    /// Sets selected color for the graphs:
    void SetSelectedColorsForGraphs(int selectedRowIndex);

    /// Add icons for each of the graphs:
    /// \param numGraphs the number of graphs
    void SetRowIcons(int numGraphs);

    /// Resize the table columns according to the columns content width:
    void ResizeColumns();

    /// disable counters that should be disabled by their m_souldBeDisbled value
    /// the disabled counter will not be shown in the plot, and will be unchecked and disabled in the legend
    void DisableCountersThatShuldBeDisabled();

protected:

    enum acPlotLegendIndex
    {
        AC_PLOT_LEGEND_NAME_COL_INDEX = 0,
        AC_PLOT_LEGEND_VALUE_COL_INDEX
    };

    bool m_isCumulative;
    bool m_isLastGraphTotal;

    acCustomPlot* m_pCustomPlot;
    QVector<acSingleLineGraph*> m_pGraphsVec;
    acListCtrl* m_pPlotInfoTable;
    QString m_plotName;
    QLabel* m_pValuesTextLabel;
    int m_addRemoveRowNum;

    /// Graph values type (double/int)
    GraphValuesType m_valuesType;

    /// Graph units:
    QString m_unitsStr;

    /// Contain the tooltip HTML style:
    static QString m_sTooltipHTMLStyle;
};


#endif
