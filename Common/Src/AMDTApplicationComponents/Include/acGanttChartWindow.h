//------------------------------ acGanttChartWindow.h ------------------------------

#ifndef __ACGANTTCHARTWINDOW_H
#define __ACGANTTCHARTWINDOW_H

// Infra:
#include <AMDTBaseTools/Include/gtPtrVector.h>
#include <AMDTBaseTools/Include/gtString.h>

// Local:
#include <AMDTApplicationComponents/Include/acApplicationComponentsDLLBuild.h>
#include <AMDTApplicationComponents/Include/acOpenGLChartWindow.h>

typedef gtUInt64 acGanttChartDataType;

// ----------------------------------------------------------------------------------
// Class Name:          AC_API acGanttChartBackgroundSectionData
// General Description: Holds all the data for a section of the chart's background
// Author:               Uri Shomroni
// Creation Date:        23/2/2010
// ----------------------------------------------------------------------------------
struct AC_API acGanttChartBackgroundSectionData
{
public:
    acGanttChartBackgroundSectionData(const wxColour& sectionColor)
        : _firstLine(-1), _lastLine(-1), _sectionColor(sectionColor) {};
    ~acGanttChartBackgroundSectionData() {};

public:
    int _firstLine;
    int _lastLine;
    wxColour _sectionColor;
    gtString _toolTip;
    gtString _label;
};

// ----------------------------------------------------------------------------------
// Class Name:          AC_API acGanttChartBarData
// General Description: Holds all the data for a single bar in the chart
// Author:              Uri Shomroni
// Creation Date:       23/2/2010
// ----------------------------------------------------------------------------------
struct AC_API acGanttChartBarData
{
public:
    acGanttChartBarData(acGanttChartDataType barStart, acGanttChartDataType barEnd, const wxColour& barColor, int originalIndex, const gtString& label, const gtString& tooltip)
        : _barStart(barStart), _barEnd(barEnd), _barColor(barColor), _originalIndex(originalIndex), _label(label), _tooltip(tooltip), _barLine(-1) {};
    ~acGanttChartBarData() {};

public:
    acGanttChartDataType _barStart;
    acGanttChartDataType _barEnd;
    wxColour _barColor;
    int _originalIndex;
    gtString _label;
    gtString _tooltip;
    int _barLine;
};

// ----------------------------------------------------------------------------------
// Class Name:          AC_API acGanttChartMarkerData
// General Description: Holds the data for a marker (barrier) in the chart
// Author:              Uri Shomroni
// Creation Date:       23/2/2010
// ----------------------------------------------------------------------------------
struct AC_API acGanttChartMarkerData
{
public:
    acGanttChartMarkerData(acGanttChartDataType markerPosition, int markerFirstLine, int markerLastLine, const wxColour& markerColor)
        : _markerPosition(markerPosition), _markerColor(markerColor), _markerFirstLine(markerFirstLine), _markerLastLine(markerLastLine) {};
    ~acGanttChartMarkerData() {};

public:
    acGanttChartDataType _markerPosition;
    wxColour _markerColor;
    int _markerFirstLine;
    int _markerLastLine;
};

// ----------------------------------------------------------------------------------
// Class Name:          acGanttChartItemClickedEvent : public wxEvent
// General Description: A wxWindows event that is fired when the user clicks a gantt chart item
// Author:              Uri Shomroni
// Creation Date:       28/2/2010
// ----------------------------------------------------------------------------------
class AC_API acGanttChartItemClickedEvent : public wxEvent
{
public:
    acGanttChartItemClickedEvent(int clickedItemSeries, int clickedItemIndex, int clickedItemOriginalIndex);

    // Overrides wxEvent:
    virtual wxEvent* Clone() const;

    int clickedItemSeries() const {return _clickedItemSeries;};
    int clickedItemIndex() const {return _clickedItemIndex;};
    int clickedItemOriginalIndex() const {return _clickedItemOriginalIndex;};

private:
    // Do not allow the use of my default constructor:
    acGanttChartItemClickedEvent();

private:
    // The pressed chart item details:
    int _clickedItemSeries;
    int _clickedItemIndex;
    int _clickedItemOriginalIndex;
};

extern AC_API wxEventType acEVT_GANTT_CHART_ITEM_CLICKED_EVENT; // Event type
typedef void (wxEvtHandler::*acGanttChartItemClickedEventHandler)(acGanttChartItemClickedEvent&); // Event handler method definition

// Macro for creating wxWindows event table entry:
#define EVT_AC_GANTT_CHART_ITEM_CLICKED_EVENT(func) wxEventTableEntry(acEVT_GANTT_CHART_ITEM_CLICKED_EVENT, -1, -1, wxNewEventTableFunctor(acEVT_GANTT_CHART_ITEM_CLICKED_EVENT, (wxObjectEventFunction)(wxEventFunction)(acGanttChartItemClickedEventHandler)& func), (wxObject*) NULL ),


// ----------------------------------------------------------------------------------
// Class Name:          AC_API acGanttChartWindow : public wxGLCanvas
// General Description: A window drawing a gantt chart using OpenGL.
// Author:              Uri Shomroni
// Creation Date:       22/2/2010
// ----------------------------------------------------------------------------------
class AC_API acGanttChartWindow : public acOpenGLChartWindow
{
public:
    acGanttChartWindow(wxWindow* pParent, wxColour bgColor, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~acGanttChartWindow();

    // wx events:
    void onMouseMove(wxMouseEvent& eve);
    void onMouseWheel(wxMouseEvent& eve);
    void onLeftDown(wxMouseEvent& eve);
    void onScrollWin(wxScrollWinEvent& eve);

    // Data loading:
    void clearAllData();
    void updateGanttChart();
    void setNumberOfSeries(int numberOfSeries);
    void setSeriesData(int seriesIndex, const wxColour& bgCol, const gtString& toolTip, const gtString& label);
    void addBarToSeries(int seriesIndex, acGanttChartDataType barStart, acGanttChartDataType barEnd, const wxColour& barColor, int originalIndex, const gtString& label, const gtString& tooltip);
    void clearSeriesData(int seriesIndex);
    void scrollWindowToBarPosition(int seriesIndex, int barIndex);
    void setRulerLabelSeparators(const gtString& separators) {_rulerLabelSeparators = separators;};

protected:
    // Override acOpenGLChartWindow
    virtual void getChartPartAmounts(unsigned int& backgroundSections, unsigned int& bars, unsigned int& markers, unsigned int& labelCharacters, unsigned int& vscrollingControlRects, unsigned int& hscrollingControlRects, unsigned int& hscrollingControlLines) const;
    virtual bool fillBackgroundSectionsData(const acOpenGLChartDataPointers& bgDataPointers);
    virtual bool fillBarsData(const acOpenGLChartDataPointers& barDataPointers, const acOpenGLChartDataPointers& labelDataPointers, int& barLabelPositions, int& barLabelVertices);
    virtual bool fillMarkersData(const acOpenGLChartDataPointers& markerDataPointers);
    virtual bool fillControlsData(const acOpenGLChartDataPointers& controlDataPointers);
    virtual void updateBeforeDataLoadedIntoOpenGL();
    virtual void updateBeforeControlsDataLoadedIntoOpenGL();
    virtual void beforeDrawingOpenGLData();
    virtual void afterDrawingOpenGLData();
    virtual void beforeDrawingVScrollingWindowControls();
    virtual void afterDrawingVScrollingWindowControls();
    virtual void beforeDrawingHScrollingWindowControls();
    virtual void afterDrawingHScrollingWindowControls();
    virtual void beforeResize();

private:
    // Disallow use of my default constructor:
    acGanttChartWindow();

    // Windowing:
    void scrollWindow(int topLineShown, acGanttChartDataType leftValueShown);
    void clampTopLineToValidRange();
    void updateScrollbarRangeAndPosition();
    void windowPositionToSeriesAndBarIndex(const wxPoint& windowPos, int& seriesIndex, int& barIndex, const acGanttChartBackgroundSectionData** ppBackgroundSectionData = NULL, const acGanttChartBarData** ppBarData = NULL);
    void rulerValueToLabel(acGanttChartDataType scaleMarkValue, gtString& label);
    void updateRuler();
    void updateFirstRulerStepToDraw();

    // Data calculation and construction:
    void calculateDataLines();
    void calculateDataLimits(const gtVector<acGanttChartDataType>& lineLowestValues, const gtVector<acGanttChartDataType>& lineHighestValues);
    bool chartHasData() const;
    acGanttChartDataType screenWidthValue() const;

    // Converting data to OpenGL:
    acOpenGLChartPositionDataType lineTop(int lineIndex, bool withSpacing) const;
    acOpenGLChartPositionDataType lineBottom(int lineIndex, bool withSpacing) const;
    acOpenGLChartPositionDataType horizontalPosition(acGanttChartDataType dataValue, bool allowOutOfRange = false) const;
    acOpenGLChartPositionDataType rulerVerticalPosition() const;
    acGanttChartDataType valueOfViewPortWidth() const;
    acGanttChartDataType rulerLength() const;
    unsigned int labelLengthInsideGanttBar(const acGanttChartBarData& barData) const;

    // OpenGL functions:
    void moveCamera(bool moveH, bool moveV);

private:
    // Data Limits:
    acGanttChartDataType _minValue;
    acGanttChartDataType _maxValue;
    int _numberOfChartLines;
    int _numberOfLinesShown;

    // Horizontal scaling and scrolling:
    int _horizontalScaleIndex;
    double _horizontalScale;
    acOpenGLChartPositionDataType _seriesLabelsWidth;

    // Ruler:
    acGanttChartDataType _rulerStepSize;
    unsigned int _rulerLabelsNumber;
    unsigned int _rulerLongestLabelLength;
    gtString _rulerLabelSeparators;
    unsigned int _rulerSubdivisions;
    unsigned int _rulerMajorStepsToDraw;
    unsigned int _rulerFirstStepToDraw;

    // Viewport positioning:
    int _topLineShown;
    acGanttChartDataType _leftValueShown;

    // Chart data:
    gtVector<acGanttChartBackgroundSectionData> _backgroundSectionsData;
    gtPtrVector<gtVector<acGanttChartBarData>* > _barsDataBySeries;
    gtVector<acGanttChartMarkerData> _markersData;

private:
    DECLARE_EVENT_TABLE();
};

#endif //__ACGANTTCHARTWINDOW_H

