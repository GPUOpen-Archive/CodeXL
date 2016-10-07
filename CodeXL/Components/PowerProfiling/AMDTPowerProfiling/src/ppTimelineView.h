//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file ppTimelineView.h
///
//==================================================================================

//------------------------------ ppTimelineView.h ------------------------------

#ifndef __PPTIMELINEVIEW_H
#define __PPTIMELINEVIEW_H

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <qcustomplot.h>

// Framework:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTApplicationComponents/Include/acMultiLinePlot.h>

// Powerprofiler midtier classes
#include <AMDTPowerProfilingMidTier/include/PowerProfilerCore.h>
#include <AMDTPowerProfiling/src/ppAppController.h>

// Local:
#include <AMDTPowerProfiling/Include/ppAMDTPowerProfilingDLLBuild.h>
#include <AMDTPowerProfiling/src/ppDataUtils.h>
#include <AMDTPowerProfiling/src/ppMultiLinePlot.h>

// Forward declaration
class ppSessionController;
class ppSessionNavigationChart;
class ppSessionView;

// ----------------------------------------------------------------------------------
// Class Name:          ppRibbonButton
// General Description: The class inherits QPushButton, and implement a button for the
//                      timeline ribbons. This class will contain the containing ribbon
//                      index, and will emit signals in mouse enter / leave events so that
//                      the containing view will change ribbon graphic properties
// ----------------------------------------------------------------------------------
class ppRibbonButton : public QPushButton
{
    Q_OBJECT

public:

    enum ButtonType
    {
        PP_RIBBON_BUTTON_UP,
        PP_RIBBON_BUTTON_DOWN
    };

    /// Constructor:
    ppRibbonButton(QWidget* pParent = nullptr);

    /// Destructor:
    ~ppRibbonButton();

    /// Set the amount of ribbons:
    static void SetRibbonCount(int count) { m_sRibbonsCount = count; }

    /// Accessors to the button plot index and button type:
    int PlotIndex() const { return m_plotIndex; };
    void SetPlotIndex(int index) { m_plotIndex = index; };
    void SetButtonType(ButtonType type) { m_buttonType = type; };
signals:

    /// Is triggered when the button area is entered or left:
    /// \param plotIndex the plot index of the button on which the button entered:
    /// \param buttonType the type of the button (up / down)
    /// \param wasEntered was the area entered (false for leave)
    void ButtonEnterLeave(int plotIndex, ppRibbonButton::ButtonType buttonType, bool wasEntered);

protected:

    /// Overrides QPushButton (in order to control the style of the button):
    virtual void enterEvent(QEvent* pEvent);

    /// Overrides QPushButton (in order to control the style of the button):
    virtual void leaveEvent(QEvent* pEvent);

    /// Contain the plot index (the plot on which the button controls):
    int m_plotIndex;

    /// The button type (up / down):
    ButtonType m_buttonType;

    /// Contain the amount of ribbons (will be used for down button enable / disable state):
    static int m_sRibbonsCount;

    /// True iff the mouse pointer is in the button area:
    bool m_isMouseIn;

};

struct RibbonButtons
{
    RibbonButtons() : m_pUpButton(nullptr), m_pDownButton(nullptr) {}

    ppRibbonButton* m_pUpButton;
    ppRibbonButton* m_pDownButton;
};

typedef gtMap<acCustomPlot*, RibbonButtons> RibbonButtonsMap;

class PP_API ppTimeLineView : public QWidget
{
    Q_OBJECT

public:
    ppTimeLineView(ppSessionView* pParent, ppSessionController* pSessionController);

    virtual ~ppTimeLineView();

public slots:

    /// updates session state new / running / complete
    void UpdateProfileState();

    /// handle the profile ended slot, clear all unnecessary connections:
    /// \param sessionName - session name
    void OnProfileStopped(const QString& sessionName);

protected slots:

    /// Handle the new data emit
    /// \param pSampledDataPerCounter - data from event
    void OnNewProfileData(ppQtEventData pSampledDataPerCounter);

    /// adding new data from event to navigation chart
    /// \param pSampledDataPerCounter - data from event
    /// \returns the current range for all graphs
    SamplingTimeRange AddNewDataToSessionNavigationChart(ppQtEventData pSampledDataPerCounter, bool shouldReplot);

    /// updates ribbons graphs time range
    /// \param range to be set
    /// \param sessionName - session name
    void OnRangeChangedByUser(const QPointF& range);

    /// handles change of power graph type between cumulative / non-cumulative
    void OnChangePowerGraphDisplayType();

    /// handle power graph context menu selection between cumulative/non-cumulative
    void OnSelectedCountersChanged();

    /// handle power graph context menu popup
    void OnContextMenuRequest(const QPoint& pos);

    void OnTrackingXAxis(double key, int nTrackingLineAbscissa);

    void UpdateTrackingLine();

    /// Slot handling the drag and drop event of the plots:
    /// \param pDroppedIntoPlot the dropped into plot - the plot that an item was dragged into it
    /// \param pDraggedPlot the dragged plot
    void OnPlotDropped(acCustomPlot* pDroppedIntoPlot, acCustomPlot* pDraggedPlot);

    /// Is handling the change of the navigation counter selection combo box:
    /// \param counterName the new selected counter name
    void OnNavigationCounterSelectionChange(const QString& counterName);

    /// Is handling the click on the navigation counter link:
    void OnNavigationCounterLinkClicked(const QString& link);

    void DragPlotIntoOther(int plotIndex1, int plotIndex2);

    /// Slot handling the click of up button:
    void OnRibbonUp();

    /// Slot handling the click of down button:
    void OnRibbonDown();

    /// Is handling the enter event of the ribbon plots:
    void OnPlotEntered(acCustomPlot* pPlot);

    /// Is handling the leave event of the ribbon plots:
    void OnPlotLeave(acCustomPlot* pPlot);

    /// Is handling the ribbon buttons enter and leave events:
    /// \param buttonPlotIndex the button plot index
    /// \param buttonType the type of the button (enum)
    /// \param wasEntered was the button area entered or left
    void OnRibbonButtonEnterLeave(int buttonPlotIndex, ppRibbonButton::ButtonType buttonType, bool wasEntered);

    /// Is called when the plot mouse move is called. Will implement drag of the whole timeline view:
    void OnPlotMouseMove(QMouseEvent* pMouseEvent);

    /// Is called when the plot mouse move is called. Will implement range zoom in and out:
    void OnPlotMouseWheel(QWheelEvent* pMouseEvent);

protected:
    /// Overrides QWidget:
    virtual void resizeEvent(QResizeEvent* pResizeEvent);
    virtual void showEvent(QShowEvent* pEvent);

    /// Debug print function. Call it only for debugging plots geometry issues:
    void DebugPrintPlotsGeomeries();

    /// Initializes the widgets and layout for the view:
    void InitViewLayout();

    /// Updates the navigation chart with the current navigation counter ID:
    void UpdateNavigationCounter();

    /// Add a counter selection widgets (label and combo box for selecting the navigation counter):
    void AddNavigationCounterSelectionWidgets(QWidget* pContainingWidget);

private:

    /// initializes all graphs
    void InitGraphs();

    //sets first data to initialized graphs
    void SetGraphsInitializationData();

    /// adds new data from event to the existing graphs
    /// \param pSampledDataPerCounter = data for all counters from backend event
    /// \param bShouldUpdateLastSample - prevents updating graphs value tables last sample for real time tooltips
    void AddNewDataToAllExistingGraphs(ppQtEventData pSampledDataPerCounter, bool bShouldUpdateLastSample);

    /// Init graphs:
    void InitStackedGraph(ppDataUtils::GraphViewCategoryType categoryType, AMDTPwrCategory type, QString graphName,   QString postFixUnit);
    /// Generic function that initializes and add a plot to the grid according to requested graph category type:
    void InitGraphByCategory(ppDataUtils::GraphViewCategoryType graphCategory);

    /// add the initialized plot inside ribbons vector and inside the grid layout in the Timeline view tab
    /// \param plot - is the graphs plot
    void AddPlotToGrid(ppMultiLinePlot* plot);

    /// Initialize the set of buttons for the requested plot. Add the created buttons to the input buttons layout:
    /// \param pPlot the plot for which the buttons should be created
    void InitRibbonButtons(ppMultiLinePlot* pPlot);

    /// updates all graphs range
    /// \param newRange - the new range to be updated to the graphs
    /// \param isReplotNeeded - do we need to use the replot function in this point
    void UpdateRibbonsTimeRange(const SamplingTimeRange& newRange, bool isReplotNeeded);

    /// Is handling the construction of offline session in navigation chart:
    void SetOfflineSessionData();

    bool IsRibbonTypeShown(ppDataUtils::GraphViewCategoryType type);

    /// Adds an item to the grid last row, to make sure that the ribbons are not higher then it should:
    void AddStretchItemToGrid();

    /// replots all graphs in ribbons vector that are currently visible
    void ReplotVisibleGraphs();

    /// replots all graphs in the ribbons vector
    void ReplotAllGraphs();

    /// enable the add/remove counters butoon in the info table of all graphs in ribbons vector
    /// \param enable-true, disable-false
    void EnableRibbonsInfoTabelAndRemoveCounters(bool enable);

    /// Calculates the information table fixed width according to current font metric:
    /// \return the current width
    int CalculateInfoTableFixedWidth();

    /// Adds label padding to the plots, so that all the labels will be left aligned, regarding the number of digits in the tick label:
    void FixLabelPaddingDiffs();

    /// Fix the right margin of the navigation chart manually, so that its margins are aligned with the
    /// ribbons plot margins:
    void FixNavigationChartRightMarginDiff();

    /// deletes all graphs and legends from the grid layout
    void RemoveAllRibbons();

    /// Draw the bounding box for the active range:
    void DrawBoundingBox();

    /// Calculate the bottom ribbons Y coordinate. This coordinate is used for the geometry of the tooltip and bounding box:
    void CalculateCurrentBottomRibbonsYCoord();

    /// Add all ribbon to grid:
    void AddRibbonsToGrid();

    /// Show / hide the ribbon buttons for a specific plot:
    /// \param pPlot the plot to show or hide its buttons
    /// \param shouldShow true if the buttons should be shown, false for hide
    void ShowRibbonButtons(acCustomPlot* pPlot, bool shouldShow);

    /// Clear the highlight for all the plots. Highlight is done when the up / down arrow are hovered, and is supposed to display
    /// the expected result of the button
    void ClearPlotsBackgrounds();

    /// Main view layout:
    QVBoxLayout* m_pMainLayout;
    QScrollArea* m_pBottomScrollArea;
    QVBoxLayout* m_pBottomVLayout;

    /// Widget used to avoid the vertical stretch of the ribbons:
    QWidget* m_pGridStretchWidget;

    /// This vector holds all the visible ribbons. Each of the ribbons was added since it contain one of the user's selected counters.
    /// The user can switch from stacked to non-stacked, this vector will contain only the selected from these two:
    QVector<ppMultiLinePlot*> m_visibleRibbonsVec;

    /// This vector holds all the visible ribbons. Each of the ribbons was added since it contain one of the user's selected counters.
    /// The user can switch from stacked to non-stacked, this vector will contain them both, even though only one of then is visible:
    QVector<ppMultiLinePlot*> m_allRibbonsVec;

    /// Contain the power plot:
    /// Both power and stacked power plots should be kept separately, since it is not always displayed,
    /// and it will only be in m_visibleRibbonsVec when it is displayed:
    ppMultiLinePlot* m_pPowerPlot;

    /// Stacked power plot:
    ppMultiLinePlot* m_pPowerStackedPlot;

    /// time range shown for all graphs
    SamplingTimeRange m_samplingTimeRange;

    /// session controller
    ppSessionController* m_pSessionController;

    /// navigation chart
    ppSessionNavigationChart* m_pSessionNavigationChart;

    /// graphs time intervals
    double m_timeLineGraphInterval;

    /// Navigation counter ID:
    int m_navigationCounterID;

    /// Contain the parent session view:
    ppSessionView* m_pParentSession;

    /// line widget of tracking
    QWidget* m_pTrackingLine;
    QWidget* m_pBoundingBoxLeftLine;
    QWidget* m_pBoundingBoxRightLine;
    QWidget* m_pBoundingBoxBottomLine;

    /// Label for selecting a counter for session navigation:
    QLabel* m_pNavigationCounterSelectionLabel;

    /// Combo box for selecting a counter for session navigation:
    QComboBox* m_pNavigationCounterSelectionComboBox;

    bool m_shouldDrawTrackingLine;
    int m_trackingLineXCoordinate;

    QLabel* m_pTimeLabel;
    double m_trackLineKey;
    bool m_isRangeChanged;

    /// True iff the range bounding line was already painted:
    bool m_wasRangeBoundingLinePainted;

    /// True if the plots visibility is updated (happens on first show):
    bool m_isVisibilityUpdated;

    /// last data update time. used if no catalyst is installed:
    int m_lastReplotTime;

    /// Contain the Y coordinate of the bottom ribbon:
    int m_bottomRibbonYCoord;

    /// A map plot -> ribbon buttons:
    RibbonButtonsMap m_ribbonButtonsMap;

    /// A flag preventing from ShowRibbonButtons calling itself:
    bool m_isInShowRibbonButtons;

};

#endif // __PPTIMELINEVIEW_H
