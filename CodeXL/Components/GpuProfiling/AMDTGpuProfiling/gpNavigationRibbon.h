//------------------------------ gpNavigationRibbon.h ------------------------------

#ifndef __GPNAVIGATIONRIBBON_H_
#define __GPNAVIGATIONRIBBON_H_

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// infra
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTApplicationComponents/Include/acNavigationChart.h>

// local
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>

class acNavigationChart;
class gpTraceDataContainer;
class acTimeline;
class acTimelineGrid;
class gpTraceView;
class acTimelineFiltersDialog;
class gpRibbonDataCalculator;

// this is added due to layout spacing
#define NAVIGATION_ADDED_SPACE 5
// this pace is added by the extra vertical box layout
#define VERICAL_BOX_ADDED_SPACE 2

#define GP_NAVIGATION_CHART_RIGHT_MARGIN 10
#define GP_NAVIGATION_CHART_LEFT_MARGIN 40
#define GP_NAVIGATION_CHART_TOP_MARGIN 8


class gpNavigationRibbonMaxTimeData
{
public:
    /// Time of execution
    double m_time;

    /// duration
    double m_duration;

    bool operator<(const gpNavigationRibbonMaxTimeData& other) const { return m_time < other.m_time; };
};

class gpNagivationThreadSegmentData
{
public:
    /// start time of segment
    double m_startTime;

    /// end time of the segment
    double m_endTime;

    /// id of the thread
    osThreadId m_threadID;
};

class gpFilterButton;

class AMDT_GPU_PROF_API gpNavigationRibbon : public QWidget
{
    Q_OBJECT
public:
    enum eNavigateDataBy
    {
        eNavigateLayerApiCalls = 0,
        eNavigateLayerDrawCalls,
        eNavigateLayerTopGPUCmdsCalls,
        eNavigateLayerTopCPUApiCalls,
        eNavigateLayerGPUOpsTime,
        eNavigateLayerCPUApiTime,
        eNavigateLayerMaxThreads,
        eNavigateLayerAvgThreads,
        eNavigateLayerTotalThreads,
        eNavigateLayersNum
    };

    enum eNavigateGroup
    {
        eNavigateGroupDuration = 0,
        eNavigateGroupCount,
        eNavigateGroupThread,
        eNavigateGroupsNum
    };

    gpNavigationRibbon(QWidget* pParent, gpRibbonDataCalculator* pCalculator);
    virtual ~gpNavigationRibbon();

    /// Set the grid time line
    void SetTimeLine(acTimeline* pTimeLine);

    /// update the data by time
    void CalculateData();

    /// Update the axis division based on the time line data
    void UpdateAxisData();

    /// Get the navigation chart
    acNavigationChart* NavigationChart() { return m_pNavigationChart; }

    /// Set the present data
    void SetPresentData(QVector<double>& presentData) { m_presentData = presentData; }

    /// calculate concurrency of threads
    void CalculateConcurrency();

    /// handle layer visible change
    void OnLayerVisibilityChanged(int visibleGroup, int visibleLayersByFlag);

    /// Emit layers visibility
    void EmitLayersVisibility();

    /// Checks the current visibility flags
    void CalculateCurrentVisibility();

    /// Get the current visible layers
    /// \param visibleGroup the current visible group
    /// \param the current visibility filter
    void GetCurrentVisibility(int& visibleGroup, int& visibleLayersByFlag);

    /// Enable navigation controls based on the type of sessions that are displayed (CPU/GPU)
    /// \param isCPUTraceDisplayed is cpu traced displayed
    /// \param isGPUTraceDisplayed is cpu traced displayed
    void EnableCpuGpuGroups(bool isCPUTraceDisplayed, bool isGPUTraceDisplayed);

signals:
    /// emitted when the user change the selection of visible layers (either by selecting a single layer check box or selecting a group of layers with the combobox)
    void LayerVisibilityChanged();

protected slots:

    /// Update the navigation with the correct data
    void OnNavigateStateChanged(int state);

    /// Received when timeline filters are changed
    /// \param threadNameVisibilityMap - a map of thread names and visibility
    void OnTimelineFilterChanged(QMap<QString, bool>& threadNameVisibilityMap);

    /// Handle the group selection
    void OnGroupChanged(int index);

    /// handle the setting of elements width
    /// \param legendWidth is the width of the legend part of the view
    /// \param timelineWidth is the width of the time line part of the view
    void OnSetElementsNewWidth(int legendWidth, int timelineWidth);

    /// Handle the range change in the navigation ribbon
    /// \param rangePoint
    void OnRangeChangedByUser(const QPointF& rangePoint);

    /// Handle the change of scroll bar in the time line (either by changing position or dragging it)
    void OnTimeLineZoomOrOffsetChanged();

    void OnDisplayFiltersClicked();

private:

    /// initialize the layout of the view
    void InitLayout();

    /// Calculate the buckets (also used as the main graph for calculating the indexes)
    void CalculateBuckets();

    /// Calculate Draw calls and API calls
    void CalculateCalls();

    /// calculate GPU related data
    void CalculateGPUData();

    /// Add an indexed layer
    void AddIndexedLayer(acNavigationChartLayer*& pLayer, QVector<double>& yData, eNavigateDataBy layerIndex, acNavigationChartLayer::eNavigationChartLayerType layerType, QColor dimmedPenColor, QColor dimmedBrushColor, QColor highlightPenColor, QColor highlightBrushColor, bool visible = true);

    /// add a none indexed layer
    void AddNoneIndexedLayer(acNavigationChartLayerNonIndex*& pLayer, QVector<double>& xData, QVector<double>& yData, eNavigateDataBy layerIndex, acNavigationChartLayer::eNavigationChartLayerType layerType, QColor dimmedPenColor, QColor dimmedBrushColor, QColor highlightPenColor, QColor highlightBrushColor, bool visible = true);

    /// Set the checkbox color icon
    void SetCheckBoxColorIcon(QCheckBox* pCheckBox, QColor& color);

    /// Owning session
    gpTraceView* m_pOwningSession;

    // VBox that holds the navigate by section
    QVBoxLayout* m_pNavigateByVBox;

    /// The combo box that selects which group of check boxes will be displayed
    QComboBox* m_pGroupComboBox;

    /// checks boxes to hide/show navigation layers
    QCheckBox* m_pNavigateLayer[eNavigateLayersNum];

    /// Navigation chart
    acNavigationChart* m_pNavigationChart;

    /// Controlling Time line
    acTimeline* m_pTimeLine;

    /// Controlling Time line grid;
    acTimelineGrid* m_pTimeLineGrid;

    /// flag if we are in a navigation zoom/pan or timeline zoom/pan
    bool m_inZoomOrPan;

    /// buckets positions used as x coordinates
    QVector<double> m_bucketVals;

    acTimelineFiltersDialog* m_pThreadFilterDialog;
    gpFilterButton* m_pFiltersLink;

    /// the api layers
    acNavigationChartLayer* m_pApiCallsLayer;
    acNavigationChartLayer* m_pDrawCallsLayer;
    acNavigationChartLayer* m_pMaxCPUApiTimeLayer;
    acNavigationChartLayer* m_pMaxGPUOpsTimeLayer;
    acNavigationChartLayerNonIndex* m_pMaxDurationLayer;
    acNavigationChartLayerNonIndex* m_pMaxGPUOpsLayer;
    acNavigationChartLayer* m_pThreadConcurrencyLayer;
    acNavigationChartLayer* m_pAvgThreadConcurrencyLayer;
    acNavigationChartLayer* m_pTotalConcurrencyLayer;

    /// the present data
    QVector<double> m_presentData;

    /// present bars that display the present data
    QCPBars* m_pPresentBars;

    /// A pointer to the data calculator, shared by all ribbons, for performance reasons
    gpRibbonDataCalculator* m_pFrameDataCalculator;

    /// Concurrency will be calculated on demand, for performance reasons
    bool m_wasConcurrencyCalculated;

    /// Static member relating visible layers to combo groups
    static int m_sLayerToGroupConnection[eNavigateLayersNum];

    // last visible group
    int m_visibleGroup;

    /// last layers visibility flags
    int m_visibleLayersByFlag;
};

///////////////////////////////////////////////
// THe Button which displays the timeline visibility filters: appears only when the mouse hovers over the navigation chart
class gpFilterButton : public QPushButton
{
    Q_OBJECT
public:
    /// Constructor:
    gpFilterButton(QWidget* pParent = nullptr);

    /// Destructor:
    virtual ~gpFilterButton() {}

signals:

    /// Triggered when the button area is entered or left:
    /// \param wasEntered was the area entered (false for leave)
    void ButtonEnterLeave(bool wasEntered);

public slots:
    /// Makes the button visible when mouse hovers over the chart
    void OnChartEnter(acNavigationChart* pNavigationChart);
    /// Makes the button invisible when mouse doesn't hover over the chart
    void OnChartLeave();

protected:

    /// Overrides QPushButton (in order to control the style of the button):
    virtual void enterEvent(QEvent* pEvent);

    /// Overrides QPushButton (in order to control the style of the button):
    virtual void leaveEvent(QEvent* pEvent);

    /// True if the mouse pointer is in the button area:
    bool m_isMouseIn;
};

#endif // __GPNAVIGATIONRIBBON_H_