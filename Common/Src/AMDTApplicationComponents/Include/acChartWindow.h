//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acChartWindow.h
///
//==================================================================================

//------------------------------ acChartWindow.h ------------------------------

#ifndef __ACCHARTWINDOW_H
#define __ACCHARTWINDOW_H

#define acChartDataType unsigned long

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QWidget>
#include <QLabel>

#include <qcustomplot.h>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>

enum AC_API acChartType
{
    AC_NO_CHART,
    AC_PIE_CHART,
    AC_BAR_CHART
};

class AC_API acChartDataPoint
{
public:
    acChartDataPoint(): _value(0), _pointColor(0, 0, 0), _isSelected(false), _tooltip(L""), _originalItemIndex(-1) {};
    acChartDataType _value;
    QColor _pointColor;
    bool _isSelected;
    gtString _tooltip;
    int _originalItemIndex;
};

class acPieWidget;

class AC_API acChartWindow : public QWidget
{
    friend class acPieWidget;

    Q_OBJECT

public:
    acChartWindow(QWidget* pParent, acChartType chartType = AC_NO_CHART);
    virtual ~acChartWindow();

    // Data changing functions:
    void addDataPoint(const acChartDataPoint& data);
    void clearAllData();

    // Handlers for selection highlighting:
    void setSelection(int position, bool selected);
    void clearAllSelection();

    // Accessors:
    void setHighlightColor(const QColor& newColor) {_highlightColor = newColor;};
    void setChartType(const acChartType& chartType);
    void setReferenceValue(const acChartDataType& refVal) {_referenceValue = refVal; _areArraysValid = false;};

    // Other functions:
    void redrawWindow();
    void recalculateArrays();

Q_SIGNALS:

    void chartItemClicked(int chartItemIndex);

protected:

    // QGLWidget overrides:
    virtual void paintGL();
    virtual void resizeGL(int width, int height);

    // QWidget overrides:
    virtual void mousePressEvent(QMouseEvent* pEvent);
    virtual void mouseReleaseEvent(QMouseEvent* pEvent);
    virtual void mouseMoveEvent(QMouseEvent* pEvent);

    virtual void resizeEvent(QResizeEvent* event);

private:

    int mousePointToBarIndex(QPoint mousePoint);
    int mousePointToPieIndex(QPoint mousePoint);

    // Draw graphs:
    void drawPieChart();
    void drawBarChart();
    void drawEmptyChart();

    // Pie chart related methods:
    void calculatePieChart();
    void ClearPieChart();

    // Bar chart related methods:
    void calculateBarChart();

    void ClearGraphs();



private:

    acChartType _chartType;
    gtVector<acChartDataPoint> _dataItems;
    QColor _highlightColor;

    double _barWidth;
    double _barSpacing;

    // Store the total and the maximum values for calculations:
    acChartDataType _totalValue;
    acChartDataType _maxValue;

    // A reference point to be used instead of the max value instead of _maxvalue if it is higher
    acChartDataType _referenceValue;

    // Is the left mouse button pressed down on the window?
    bool _isLeftMouseDown;
    QPoint _lastMousePosition;

    // degrees of rotation of the view:
    float _hRotation;
    float _vRotation;

    // Did we use recalculateArrays() since last changing something?
    bool _areArraysValid;

    // The amount of items contained by the chart, while the chart vectors were calculated:
    unsigned int _calculatedAmountOfItems;

    QCustomPlot* m_pGraphWidget;
    gtVector<QCPBars*> m_barDataVector;

    acPieWidget* m_pPieWidget;

    QLabel* m_pNoInfoLabel;
};


class acPieWidget: public QWidget
{
    friend class acChartWindow;
public:
    acPieWidget(acChartWindow* pParent) : QWidget(pParent) {m_pOwner = pParent;};
    virtual ~acPieWidget() {};

    virtual void paintEvent(QPaintEvent* e);

private:
    acChartWindow* m_pOwner;

    gtVector<float> m_pieSectorsAnglesVector;

};

#endif //__ACCHARTWINDOW_H
