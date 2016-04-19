//------------------------------ gpDetailedDataRibbon.h ------------------------------

#ifndef __GPDETAILEDDATARIBBON_H_
#define __GPDETAILEDDATARIBBON_H_

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra
#include <AMDTApplicationComponents/Include/acRibbonManager.h>

// Local
#include <AMDTGpuProfiling/AMDTGpuProfilerDefs.h>
#include <AMDTGpuProfiling/gpRibbonDataCalculator.h>
#include <AMDTGpuProfiling/gpNavigationRibbon.h>

class acTimeline;
class gpTraceDataContainer;
class gpTraceView;
class acTimelineGrid;
class QCustomPlot;
class acNavigationChart;

class gpBarsCacheData
{
public:
    /// the time of the draw call
    double m_startTime;

    /// the duration of the draw call
    double m_duration;

    /// sort function
    bool operator<(const gpBarsCacheData& other) const { return m_startTime < other.m_startTime; };
};

class AMDT_GPU_PROF_API gpDetailedDataRibbon : public QWidget, public acIRibbonTime
{
    Q_OBJECT
public:

    gpDetailedDataRibbon(QWidget* pParent, gpRibbonDataCalculator* pCalculator);
    virtual ~gpDetailedDataRibbon();

    /// Set the grid time line
    void SetTimeLine(acTimeline* pTimeLine);

    /// update the data by time
    void CalculateData();

    /// Implement the acIRibbonTime interface:

    /// string to display as the tool tip for a specific time
    virtual QString GetTooltip(double iTime, QMouseEvent* pMouseEvent);

    /// handle layer visible change
    void OnLayerVisibilityChanged(int visibleGroup, int visibleLayersByFlag);

signals:
    void AfterReplot();

protected slots:

    /// Handle the change of scroll bar in the time line (either by changing position or dragging it)
    void OnTimeLineZoomOrOffsetChanged();

    /// handle the setting of elements width
    /// \param legendWidth is the width of the legend part of the view
    /// \param timelineWidth is the width of the time line part of the view
    void OnSetElementsNewWidth(int legendWidth, int timelineWidth);

    void OnAfterReplot();

private:
    enum DetaileBarsType
    {
        eDetailedDetailedBars = 0,
        eDetailedBucketBars
    };

    /// initialize the layout of the view
    void InitLayout();

    /// Calculate the buckets (also used as the main graph for calculating the indexes)
    void CalculateBuckets();

    /// Calculate the bins (also used as the main graph for calculating the indexes)
    void CalculateCalls();

    /// calculate GPU related data
    void CalculateGPUData();

    /// calculate concurrency of threads
    void CalculateConcurrency();

    /// Add bars by Time and duration but the data arrives as CallData
    /// \param barsData holds the time and duration of each bar
    /// \param penColr the color of the bars
    void AddBarsByDataTimeAndDuration(int layer, const QVector<gpRibbonCallsData>& barsData, QColor penColor, QColor brushColor);

    /// Add bars by Time and duration but the data arrives as CallData
    /// \param barsTime holds the bars time
    /// \param barsDuration hold the bars duration
    /// \param penColr the color of the bars
    void AddBarsByDataTimeAndDuration(int layer, const QVector<double>& barsTime, const QVector<double>& barsDuration, QColor penColor, QColor brushColor);

    /// Add bars by buckets n but the data arrives as CallData
    /// \param barsDuration hold the bars duration
    /// \param penColr the color of the bars
    void AddBarsByBuckets(int layer, QVector<double>& barsDuration, QColor penColor, QColor brushColor);

    /// Add bars service function to add bars of all types
    /// \param barsTime holds the bars time
    /// \param barsDuration hold the bars duration
    /// \param penColr the color of the bars
    /// \param barsType bars type
    void AddBars(int layer, const QVector<double>& barsTime, const QVector<double>& barsDuration, QColor penColor, QColor brushColor, DetaileBarsType barsType);

    /// Owning session
    gpTraceView* m_pOwningSession;

    /// Navigation chart
    acNavigationChart* m_pCustomPlot;

    /// dummy empty label that helps keep the spacing the same as the navigation
    QLabel* m_pDummyLabel;

    /// a lable that tells that there are too much API calls and data can't be displayed
    QLabel* m_pInfoLabel;

    /// Controlling Time line
    acTimeline* m_pTimeLine;

    /// Controlling Time line grid;
    acTimelineGrid* m_pTimeLineGrid;

    /// flag if we are in a navigation zoom/pan or timeline zoom/pan
    bool m_inZoomOrPan;

    /// bars data for quick access since the qcpbars stores the data as map that does not allow us
    /// fast search for the tooltip and we need it as vector for faster search
    QVector<gpBarsCacheData> m_barsCacheData[gpNavigationRibbon::eNavigateLayersNum];

    /// buckets positions used as x coordinates
    QVector<double> m_bucketVals;

    /// custom plot bars stored for show/hide
    QCPBars* m_barsArray[gpNavigationRibbon::eNavigateLayersNum];

    /// max value of each layer used to set the y axis maximum
    double m_maxValues[gpNavigationRibbon::eNavigateLayersNum];

    // last visible group
    int m_visibleGroup;

    /// last layers visibility flags
    int m_visibleLayersByFlag;

    /// tooltips title for each layer
    QStringList m_toolTipsTitles;

    /// A pointer to the data calculator, shared by all ribbons, for performance reasons
    gpRibbonDataCalculator* m_pFrameDataCalculator;

    /// Concurrency will be calculated on demand, for performance reasons
    bool m_wasConcurrencyCalculated;

    /// a flag to indicate if the view can actually show data (if there are not too many API calls for example)
    bool m_canShowData;
};

#endif // __GPDETAILEDDATARIBBON_H_