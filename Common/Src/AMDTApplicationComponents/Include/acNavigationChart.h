//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file acNavigationChart.h
///
//==================================================================================

//------------------------------ acNavigationChart.h ------------------------------
#ifndef __ACNNAVIGATIONCHART_H
#define __ACNNAVIGATIONCHART_H

#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <qobject.h>
#include <qcustomplot.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>
#include <AMDTApplicationComponents/Include/acVectorLineGraph.h>
#include <AMDTApplicationComponents/Include/acColours.h>

static const QColor FILL_ACTIVE_DEFAULT_COLOR(0, 170, 181, 20);

#define NAV_HEIGHT 165
#define NAV_X_AXIS_AUTO_TICK_COUNT 5
#define NAV_FULL_RANGE_AXIS_TICKS_INTERVAL 150
#define DEFAULT_TIME_RANGE 10000 //in milliseconds
#define ACTIVE_RANGE_XAXIS_WIDTH 2
#define ACTIVE_RANGE_YAXIS_WIDTH 1

#define LAYER_DISPLAY_LIMIT 2000
#define NON_INDEX_LAYER_DISPLAY_LIMIT 100

#define AC_NAVIGATION_CHART_FONT_SIZE 10
#define AC_NAVIGATION_CHART_RIGHT_MARGIN 60
#define AC_NAVIGATION_CHART_LEFT_MARGIN 60
#define AC_NAVIGATION_CHART_TOP_MARGIN 20
#define AC_NAVIGATION_CHART_BOTTOM_MARGIN 32
#define AC_NAVIGATION_CHART_MARGINS QMargins(AC_NAVIGATION_CHART_LEFT_MARGIN, AC_NAVIGATION_CHART_TOP_MARGIN, AC_NAVIGATION_CHART_RIGHT_MARGIN, AC_NAVIGATION_CHART_BOTTOM_MARGIN)

enum eRangeState
{
    RANGE_STATE_DEFAULT = 0, //last 10 seconds, both change
    RANGE_STATE_START_END,         //  full range - start remains constant, end changes
    RANGE_STATE_START_MID,         // constant range
    RANGE_STATE_MID_END,           // both m_nLow and m_nHigh change
    RANGE_STATE_MID_MID,           // constant range
};

class acNavigationChartInitialData
{
public:
    /// default constructor
    acNavigationChartInitialData()
    {
        m_chartHeight = NAV_HEIGHT;
        m_tickCount = NAV_X_AXIS_AUTO_TICK_COUNT;
        m_fullRangeTickInterval = NAV_FULL_RANGE_AXIS_TICKS_INTERVAL;
        m_xAxisLowHighValuesDelta = DEFAULT_TIME_RANGE;
        m_activeRangeXAxisWidth = ACTIVE_RANGE_XAXIS_WIDTH;
        m_activeRangeYAxisWidth = ACTIVE_RANGE_YAXIS_WIDTH;
    }

    /// constructor
    acNavigationChartInitialData(int chartHeight, int tickCount, int fullRangeTickInterval,
                                 int xAxisLowHighValuesDelta, int activeRangeXAxisWidth, int activeRangeYAxisWidth)
    {
        m_chartHeight = chartHeight;
        m_tickCount = tickCount;
        m_fullRangeTickInterval = fullRangeTickInterval;
        m_xAxisLowHighValuesDelta = xAxisLowHighValuesDelta;
        m_activeRangeXAxisWidth = activeRangeXAxisWidth;
        m_activeRangeYAxisWidth = activeRangeYAxisWidth;
    }

    /// navigation chart height
    int m_chartHeight;
    /// number of ticks
    int m_tickCount;
    /// ticks interval
    int m_fullRangeTickInterval;
    /// delta between xAxis low and high values. A negative value means the entire range
    int m_xAxisLowHighValuesDelta;
    ///active range xAxis width
    int m_activeRangeXAxisWidth;
    ///active range yAxis width
    int m_activeRangeYAxisWidth;
};

class acNavigationChartColorsData
{
public:

    /// default constructor
    acNavigationChartColorsData()
    {
        m_axidTicksLabelColor = acQAMD_GRAY1_COLOUR;
        m_lineActiveColor = acQAMD_CYAN_OVERLAP_COLOUR;
        m_lineInactiveColor = acQAMD_GRAY3_COLOUR;
        m_fillActiveColor = FILL_ACTIVE_DEFAULT_COLOR;
        m_fillInactiveColor = acQAMD_GRAY_LIGHT_COLOUR;
    }

    /// constructor
    acNavigationChartColorsData(QColor axidTicksLabelColor,
                                QColor lineActiveColor, QColor lineInactiveColor,
                                QColor fillActiveColor, QColor fillInactiveColor)
    {
        m_axidTicksLabelColor = axidTicksLabelColor;
        m_lineActiveColor = lineActiveColor;
        m_lineInactiveColor = lineInactiveColor;
        m_fillActiveColor = fillActiveColor;
        m_fillInactiveColor = fillInactiveColor;
    }

    /// ticks label color
    QColor m_axidTicksLabelColor;
    /// active line color
    QColor m_lineActiveColor;
    /// inactive line color
    QColor m_lineInactiveColor;
    /// active fill color
    QColor m_fillActiveColor;
    /// inactive fill color
    QColor m_fillInactiveColor;
};

struct acNavigationChartPixmaps
{
    /// left range handle pixmap
    QPixmap m_leftRangeHandlePixmap;
    /// left range handle pixmap hovered
    QPixmap m_leftRangeHandlePixmapHov;
    /// left range handle pixmap pressed
    QPixmap m_leftRangeHandlePixmapPressed;

    /// right range handle pixmap
    QPixmap m_rightRangeHandlePixmap;
    /// right range handle pixmap hovered
    QPixmap m_rightRangeHandlePixmapHov;
    /// right range handle pixmap pressed
    QPixmap m_rightRangeHandlePixmapPressed;

    /// range pixmap center
    QPixmap m_rangePixmapCenter;
    /// range pixmap center hovered
    QPixmap m_rangePixmapCenterHov;
    /// range pixmap center pressed
    QPixmap m_rangePixmapCenterPressed;

    /// range pixmap left
    QPixmap m_rangePixmapLeft;
    /// range pixmap left hovered
    QPixmap m_rangePixmapLeftHov;
    /// range pixmap left pressed
    QPixmap m_rangePixmapLeftPressed;

    /// range pixmap right
    QPixmap m_rangePixmapRight;
    /// range pixmap right hovered
    QPixmap m_rangePixmapRightHov;
    /// range pixmap right pressed
    QPixmap m_rangePixmapRightPressed;

    /// range pixmap stretched
    QPixmap m_rangePixmapStretch;
    /// range pixmap stretched hovered
    QPixmap m_rangePixmapStretchHov;
    /// range pixmap stretched pressed
    QPixmap m_rangePixmapStretchPressed;
};

class AC_API acNavigationChartLayer
{
public:
    acNavigationChartLayer();
    virtual ~acNavigationChartLayer();

    enum eNavigationChartLayerType
    {
        eNavigationLayerGraph = 0,
        eNavigationLayerBar
    };

    /// layer id of the layer. when adding a layer the system will check that there are no two layers with the same id so it must be set
    /// corretly before adding the layer
    int m_layerId;

    /// type of layer
    eNavigationChartLayerType m_type;

    /// data of layer (using the main navigation chart x data
    QVector<double> m_layerYData;

    /// is the layer visible
    bool m_visible;

    /// Pen for the highlighted section
    QPen m_highlightedPen;

    /// Pen  for the dimmed section
    QPen m_dimmedPen;

    /// brush for the highlighted section
    QBrush m_highlightedBrush;

    /// brush for the dimmed section
    QBrush m_dimmedBrush;

    /// graph line that is generate for the layer in case it is a eNavigationLayrGraph
    acVectorLineGraph* m_pGraphLine;

    /// Bars that are generated for the layer in case this is a eNavigationLayrBar (3 are needed at most)
    QCPBars* m_pGraphBars[3];

    /// calculated layer max value to be used when showing/hiding layers to scale the navigation chart
    double m_layerMaxValue;
};

class AC_API acNavigationChartLayerNonIndex : public acNavigationChartLayer
{
public:
    acNavigationChartLayerNonIndex();
    virtual ~acNavigationChartLayerNonIndex();

    // none indexed layers also need the X values and should be used as little as possible since they
    // do not use the optimization mechanism.
    QVector<double> m_layerXData;
};

class AC_API acNavigationChart : public QCustomPlot
{
    Q_OBJECT

    friend class eNavigationLayrBar;

public:

    enum eBarsindexs
    {
        eBeforeHighlightedBar = 0,
        eHighlightedBar,
        eAfterHighlightedBar,
        eNumHighlightedBars
    };

    enum eNavigationUnits
    {
        eNavigationNanoseconds,
        eNavigationMicroseconds,
        eNavigationMilliseconds,
        eNavigationSingleUnits
    };

    enum eNaviationDisplay
    {
        eNavigationDisplayAll,
        eNavigationDisplayMilliOnly,
    };

    /// constructor
    /// \param pParent is the charts parent widget
    /// \param dataLabel is the acquisition data label
    acNavigationChart(QWidget* pParent, const QString& dataLabel,
                      const acNavigationChartInitialData* initialData = NULL,
                      acNavigationChartColorsData* colorsData = NULL);

    acNavigationChart() : acNavigationChart(NULL, "") {};

    virtual ~acNavigationChart();

    /// load range control images
    void LoadRangeControlImages();

    /// initializes graphs and axes
    void InitGraphs();

    /// adds new point and returns new range
    /// \param dKey - new data key
    /// \param Value - new data value
    /// \param retRange is the range
    /// \param shouldReplot - is true if replot is needed
    /// \returns true if the range changed
    bool AddNewData(double dKey, double dValue, QCPRange& retRange, bool shouldReplot);

    /// Updates the Y axis range depending on current Y value
    void UpdateYAxisRange();

    /// sets off-line data
    /// \param xData is the x data vector
    /// \param yData is the y data vector
    void SetOfflineData(const QVector<double>& xData, const QVector<double>& yData);

    /// Sets the Y Axis min-max values:
    /// \param minY the Y axis range minimum value
    /// \param maxY the Y axis range minimum value
    /// \param yAxisLabel the Y axis label
    void SetYAxisValues(int minY, int maxY, const QString& yAxisLabel);

    /// Gets the active range xAxis
    /// \returns the axis
    QCPAxis* GetActiveRangeXAxis() { return m_pActiveRangeXAxis; }

    /// Sets the interval between sample reading
    /// \param nInterval is the interval
    void SetInterval(int nInterval) { m_samplingInterval = nInterval; }

    /// Translates time into format based on the units the chart is using
    /// \param time - milliseconds or microseconds based on the units the chart is using
    /// \param bForActiveRange is true if for active range
    /// \param bUseWords
    /// \returns the time as string
    QString TimeToString(eNavigationUnits units, double time, bool shouldShowSmallestUnits = false, bool bUseWords = false);

    /// Returns the current range state: start-end, start-mid, mid-mid or mid-end
    /// \returns range state
    eRangeState GetRangeState()const { return m_rangeState; }

    /// Get the default timeline range. If it is defined as negative then the entire range is returned
    /// \return is the m_defaultTimeRange or full time range if m_defaultTimeRange is negative.
    double GetDefaultTimeRange();

    // Sets the default timeline range  If it is defined as negative then the entire range is returned
    /// \param defaultTimeRange
    void SetDefaultTimeRange(double defaultTimeRange) { m_defaultTimeRange = defaultTimeRange; }

    /// Get minimum time range
    /// \return minimum time range
    double GetMinimumRange();

    /// Set the minimum time range: Positive numbers are absolute values.Negative numbers defines percent(-0.1 = 10 % range..).
    /// Param minimum time range
    void SetMinimumRange(double minimumRange) { m_minimumRange = minimumRange;  }

    /// Checks if currently the bounding line should be drawn:
    /// \return true if the bounding line should be painted.
    bool ShouldDrawBoundingLine();

    /// Drag the range to requested mouse X position
    /// \param mouseXPosition
    void DragRangeTo(const int& mouseXPosition);

    /// Highlight the currently active range:
    void HighlightActiveRange();

    /// Applies a new range after a zoom operation:
    /// \param rangeAfterZoom the new  range
    void ApplyRangeAfterZoom(const QCPRange& rangeAfterZoom);

    /// Calculates the new range after zoom operation:
    /// \param shouldZoomIn
    /// \param rangeAfterZoom the calculated range after zoom
    void CalculateRangeAfterZoom(bool shouldZoomIn, QCPRange& rangeAfterZoom);

    /// gets the current range
    /// \returns the range
    const QPoint GetCurrentRange() const { return QPoint(m_xAxisSelectedValueLow, m_xAxisSelectedValueHigh); };

    /// Set the visible time range
    /// \param startVisibleTime starting visible time
    /// \param visibleRange range of visible time
    void SetVisibleRange(const double startVisibleTime, const double visibleRange);

    /// Add a layer of data
    void AddDataLayer(acNavigationChartLayer* pLayer);

    /// Set a layer visibility mode
    /// \param layerID layer identification
    /// \param visible visibility mode
    void SetLayerVisiblity(int layerID, bool visible);

    /// set the units that the navigation chart is using
    void SetNavigationUnitsX(eNavigationUnits iUnits) { m_unitsX = iUnits; }
    void SetNavigationUnitsY(eNavigationUnits iUnits) { m_unitsY = iUnits; }

    /// Get the units of the navigation chart
    eNavigationUnits GetNavigationUnitsX() { return m_unitsX; }
    eNavigationUnits GetNavigationUnitsY() { return m_unitsY; }

    /// Set the dislay Type
    void SetUnitsDisplayType(eNaviationDisplay displayType) { m_unitDisplay = displayType; }

    /// Get the bounds position
    void GetBounds(int& lower, int& upper);

    /// return true if displaying full range of data
    bool IsDisplayingFullRange();

    /// Get layer by id
    /// \param layerId the layer id to return
    /// \return the layer if found
    acNavigationChartLayer* GetLayerById(int layerID);

    /// Set the max range of y axis
    void SetYAxisMaxRange(double yMaxRange) { m_maxYSoFar = yMaxRange; };

    /// sets that the navigation chart does not have zoom control
    void HideZoomControl() { m_zoomCtrlEnabled = false; }

    /// set to use the timeline sync mechanism
    void UseTimelineSync() { m_shouldUseTimelineSync = true; }

    /// is showing timeline sync
    bool ShowingTimeline() { return m_showTimelineSync; }

protected:
    /// paint event
    /// \param event
    virtual void paintEvent(QPaintEvent* event);

    /// draw range control
    /// \param painter
    void DrawRangeControl(QPainter& painter);

    /// draw range bounding line
    /// \param painter
    void DrawRangeBoundingLine(QPainter& painter);

    /// draw time line sync
    /// \param painter
    void DrawTimelineSync(QPainter& painter);

    /// mouse move event
    /// \param mouse event
    virtual void mouseMoveEvent(QMouseEvent* event);

    /// mouse press event
    /// \param mouse event
    virtual void mousePressEvent(QMouseEvent* event);

    /// mouse release event
    /// \param mouse event
    virtual void mouseReleaseEvent(QMouseEvent* event);

    /// mouse wheel event
    /// \param mouse event
    virtual void wheelEvent(QWheelEvent* event);

    /// resize event
    /// \param event
    virtual void resizeEvent(QResizeEvent* event);

    /// Overrides QCustomPlot (in order to control the style of the button):
    virtual void enterEvent(QEvent* pEvent);

    /// Overrides QCustomPlot (in order to control the style of the button):
    virtual void leaveEvent(QEvent* pEvent);

    /// drag left handle to
    /// \param mousePos is the mouse position
    void DragLeftHandleTo(QPoint mousePos);

    /// drag right handle to
    /// \param mousePos is the mouse position
    void DragRightHandleTo(QPoint mousePos);

    /// is over left handle
    /// \param mousePos is the mouse position
    /// \returns true if is over the left handle
    bool IsOverLeftHandle(const QPoint& mousePos)const;

    /// is over right handle
    /// \param mousePos is the mouse position
    /// \returns true if is over the right handle
    bool IsOverRightHandle(const QPoint& mousePos)const;

    /// is over range
    /// \param mousePos is the mouse position
    /// \returns true if is over the range
    bool IsOverRange(const QPoint& mousePos) const;

    /// Sets positions of left and right range handles to be updated on paint event
    void PositionRangeControl(const QCPRange& range);

    /// Sets the state or position of the active range relative to full range:
    /// START-END; START-MID; MID-MID; MID-END
    void SetRangeState();

    /// Updates and sets time line active range axis ticks and their labels
    void SetActiveRangeXAxisTickLabels();

    /// Updates and sets full time line axis ticks and their labels
    void SetStaticTickLabels();

    /// update the y axis labels ticks and labels
    void SetYAxisTickLabels();

    // Updates active range time labels - begin time, duration, end time
    // and places the labels over the active range bounding line
    /// \param nStartPosition is active range start time label left coordinate
    /// \param nEndPosition is active range end time label right coordinate
    /// \param nYPosition is active range time labels Y coordinate
    void UpdateActiveRangeBoundingLineTimeLabels(int startPosition, int endPosition, int yPosition);

    /// Remove a layer plotables
    /// \param Layer to remove its plotables
    void RemoveLayerPlotable(acNavigationChartLayer* pLayer);

    /// Add an Index layer graphs
    /// \param Layer to adds it data to the plotables
    /// \param from data point index
    /// \param to data point index
    void AddIndexedLayerGraphs(acNavigationChartLayer* pLayer, int fromDataPoint, int toDataPoint);

    /// Add a NON Index layer graphs
    /// \param Layer to adds it data to the plotables
    void AddNonIndexedLayerGraphs(acNavigationChartLayerNonIndex* pLayer);

    /// add a bar to the plotables
    /// \param Layer to adds it data to the plotables
    /// \param bar index to add
    /// \param from data point index
    /// \param to data point index
    /// \param qt color index of the bars
    void AddBar(acNavigationChartLayer* pLayer, int barIndex, int fromDataPoint, int toDataPoint);

signals:
    /// signal for range changed by user
    /// \param point
    void RangeChangedByUser(const QPointF&);

    /// signal that range change interaction was done
    /// \param point
    void RangeChangedByUserEnded(const QPointF&);

    /// signal that the span lines were changed (usually by resize event)
    /// \param low low span point
    /// \param high high span point
    void BoundingChanged(int&, int&);

    /// signal that the mouse is over the chart (needed by gpFilterButton)
    void ChartEnter(acNavigationChart*);
    /// signal that the mouse is leaving the chart (needed by gpFilterButton)
    void ChartLeave();

    /// Signal for the tooltip line used to show sync on other ribbons
    /// \param visible if the line should be displayed or not
    /// \param time of the line
    void ShowTimeLine(bool, double);

protected slots:

    /// handle the end of range change
    /// \param point
    void OnRangeChangeEnded(const QPointF&);

    /// handle a timeline change to show a sync point if needed
    /// \param visible if the line should be displayed or not
    /// \param time of the line
    void OnShowTimeLine(bool visible, double timePos);

private:

    /// all pixmaps related to navigation chart
    acNavigationChartPixmaps m_navigationPixmaps;

    /// pointer to current pixmap of left range handle
    QPixmap* m_pCurrentLeftRangeHandlePixmap;
    /// pointer to current pixmap of right range handle
    QPixmap* m_pCurrentRightRangeHandlePixmap;
    /// pointer to range pixmap
    QPixmap* m_pRangePixmap;
    /// pointer to current left range pixmap
    QPixmap* m_pCurrentRangeLeftPixmap;
    /// pointer to current right range pixmap
    QPixmap* m_pCurrentRangeRightPixmap;
    /// pointer to current center range pixmap
    QPixmap* m_pCurrentRangeCenterPixmap;
    /// pointer to current stretch range pixmap
    QPixmap* m_pCurrentRangeStretchPixmap;


    //Time labels displayed over active range bounding line
    QLabel* m_pActiveRangeBeginTimeLabel;
    QLabel* m_pActiveRangeTimeDeltaLabel;
    QLabel* m_pActiveRangeEndTimeLabel;

    bool m_isLeftRangeHandleHovering;
    bool m_isLeftRangeHandlePressed;
    bool m_isRightRangeHandleHovering;
    bool m_isRightRangeHandlePressed;
    QPoint m_leftHandlePosition;
    QPoint m_rightHandlePosition;
    int m_dragOffset;

    QPoint m_lastPoint;
    QCPRange m_currentRange;

    QVector<double>m_vXData;
    QVector<double>m_vYData;
    QVector<double> m_vActiveRangeTicks;
    QVector<QString> m_vActiveRangeTickLabels;
    QVector<double> m_vStaticTicks;
    QVector<QString> m_vStaticTickLabels;

    double m_xAxisSelectedValueLow;
    double m_xAxisSelectedValueHigh;
    bool m_isRangePressed;
    bool m_isRangeHovering;
    QString m_qstrAcquisitionDataLabel;
    double m_minY;
    double m_maxY;
    double m_maxYSoFar;
    eRangeState m_rangeState;

    // defines which part of handle points to data
    int m_leftHandleDataOffset;
    int m_rightHandleDataOffset;

    int m_samplingInterval;

    /// the main (and only) graph in the navigation chart
    acVectorLineGraph*   m_pAllSessionGraph;
    /// all session axis
    QCPAxis*    m_pAllSessionXAxis;
    /// active range axis
    QCPAxis*    m_pActiveRangeXAxis;
    /// Y axis
    QCPAxis*    m_pYAxis;
    /// full range axis
    QCPAxis*    m_pFullRangeStaticTicksAxis;

    /// navigation chart height
    int m_chartHeight;
    /// number of ticks
    int m_tickCount;
    /// ticks interval
    int m_fullRangeTickInterval;
    /// active range xAxis width
    int m_activeRangeXAxisWidth;
    ///active range yAxis width
    int m_activeRangeYAxisWidth;
    /// time delta between range low and high
    int m_xAxisLowHighSelectedValuesDelta;

    acNavigationChartColorsData* m_pColorsData;

    // default time range. Negative number means the entire range
    double m_defaultTimeRange;

    // Minimum time range.
    double m_minimumRange;

    /// a vector of layers
    gtVector<acNavigationChartLayer*> m_layersVector;

    /// units that the navigation chart
    eNavigationUnits m_unitsX;
    eNavigationUnits m_unitsY;

    /// Display format
    eNaviationDisplay m_unitDisplay;

    /// Flag to show if the control uses the zoom control
    bool m_zoomCtrlEnabled;

    /// should use timeline sync mechanism at all
    bool m_shouldUseTimelineSync;
        
        /// should the time line sync indication be shown
    bool m_showTimelineSync;

    /// time line indication for sync
    double m_timelineSyncPos;
};



#endif // __ACNNAVIGATIONCHART_H


