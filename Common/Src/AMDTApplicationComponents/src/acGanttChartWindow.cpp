//------------------------------ acGanttChartWindow.cpp ------------------------------

// wxWindows pre compiled header:
#include <AMDTApplicationComponents/Include/acWxWidgetsIncludes.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtStringTokenizer.h>

// Local:
#include <AMDTApplicationComponents/Include/acGanttChartWindow.h>
#include <inc/acOpenGLChartWindowResources.h>

BEGIN_EVENT_TABLE(acGanttChartWindow, acOpenGLChartWindow)
    EVT_MOTION(acGanttChartWindow::onMouseMove)
    EVT_MOUSEWHEEL(acGanttChartWindow::onMouseWheel)
    EVT_LEFT_DOWN(acGanttChartWindow::onLeftDown)
    EVT_SCROLLWIN(acGanttChartWindow::onScrollWin)
END_EVENT_TABLE()

// The data we get is in ns, so this will help us make the _horizontalScale values a bit saner:
#define AC_GANTT_HORIZONTAL_SCALE_MULTIPLIER 0.000001 // 1,000,000ns = 1ms
#define AC_GANTT_HORIZONTAL_SCALE_MIN_INDEX -20
#define AC_GANTT_HORIZONTAL_SCALE_MAX_INDEX 20
#define AC_GANTT_HORIZONTAL_SCROLLBAR_STEPS 5000.0

#define AC_GANTT_LINE_TOP_OF_FIRST_LINE ((acOpenGLChartPositionDataType)1.0)
#define AC_GANTT_LINE_HEIGHT ((acOpenGLChartPositionDataType)0.25)
#define AC_GANTT_LINE_SPACING (AC_GANTT_LINE_HEIGHT / 8.0)
#define AC_GANTT_RULER_MIN_LABEL_SPACING 250
#define AC_GANTT_RULER_HEIGHT ((acOpenGLChartPositionDataType)0.17)
#define AC_GANTT_RULER_BACKGROUND_COLOR wxColour(255, 255, 223)

// Generate a unique id for the gantt item clicked wxWindows event:
wxEventType acEVT_GANTT_CHART_ITEM_CLICKED_EVENT = wxNewEventType();

// ---------------------------------------------------------------------------
// Name:        acGanttChartItemClickedEvent::acChartItemClickedEvent
// Description: Constructor
// Author:      Uri Shomroni
// Date:        28/2/2010
// ---------------------------------------------------------------------------
acGanttChartItemClickedEvent::acGanttChartItemClickedEvent(int clickedItemSeries, int clickedItemIndex, int clickedItemOriginalIndex)
    : wxEvent(0, acEVT_GANTT_CHART_ITEM_CLICKED_EVENT), _clickedItemSeries(clickedItemSeries), _clickedItemIndex(clickedItemIndex), _clickedItemOriginalIndex(clickedItemOriginalIndex)
{
    // The event should travel up the containment hierarchy from child to parent
    // searching for an event handler:
    ResumePropagation(wxEVENT_PROPAGATE_MAX);
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartItemClickedEvent::Clone
// Description: Returns a new copy of self. It is the caller responsibility to
//              delete this copy
// Author:      Uri Shomroni
// Date:        28/2/2010
// ---------------------------------------------------------------------------
wxEvent* acGanttChartItemClickedEvent::Clone() const
{
    wxEvent* retVal = new acGanttChartItemClickedEvent(_clickedItemSeries, _clickedItemIndex, _clickedItemOriginalIndex);
    GT_ASSERT(retVal);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::acGanttChartWindow
// Description: Constructor
// Author:      Uri Shomroni
// Date:        22/2/2010
// ---------------------------------------------------------------------------
acGanttChartWindow::acGanttChartWindow(wxWindow* pParent, wxColour bgColor, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
    : acOpenGLChartWindow(pParent, bgColor, id, pos, size, style | wxHSCROLL | wxVSCROLL),
      _minValue(0), _maxValue(0), _numberOfChartLines(0), _numberOfLinesShown(0), _horizontalScaleIndex(0), _horizontalScale(1.0), _seriesLabelsWidth(0.0f), _rulerStepSize(0), _rulerLabelsNumber(0), _rulerLongestLabelLength(0), _rulerSubdivisions(0), _topLineShown(0), _leftValueShown(0)
{
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::~acGanttChartWindow
// Description: Destructor
// Author:      Uri Shomroni
// Date:        22/2/2010
// ---------------------------------------------------------------------------
acGanttChartWindow::~acGanttChartWindow()
{
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::onMouseMove
// Description: Called when the mouse moves inside the window
// Author:      Uri Shomroni
// Date:        25/2/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::onMouseMove(wxMouseEvent& eve)
{
    // Get the pointer position:
    wxPoint pointerPosition = eve.GetPosition();

    // Get the bar data associated with this position:
    const acGanttChartBarData* pBarData = NULL;
    const acGanttChartBackgroundSectionData* pBackgroundSectionData = NULL;
    int seriesIndex = -1;
    int barIndex = -1;
    windowPositionToSeriesAndBarIndex(pointerPosition, seriesIndex, barIndex, &pBackgroundSectionData, &pBarData);

    if (pBarData != NULL)
    {
        // If we found a bar, show its tooltip:
        SetToolTip(pBarData->_tooltip.asCharArray());
    }
    else if (pBackgroundSectionData != NULL)
    {
        // If we are over a background section, show its tooltip:
        SetToolTip(pBackgroundSectionData->_toolTip.asCharArray());
    }
    else
    {
        // Clear the tooltip:
        UnsetToolTip();
    }
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::onMouseWheel
// Description: Called when the mouse wheel is moved in the window boundaries
// Author:      Uri Shomroni
// Date:        24/2/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::onMouseWheel(wxMouseEvent& eve)
{
    int wheelRotation = eve.GetWheelRotation();
    int wheelDelta = eve.GetWheelDelta();

    if ((wheelRotation >= wheelDelta) || (wheelRotation <= -wheelDelta))
    {
        // Add or subtract the number of "clicks" in the mouse wheel:
        int newScaleIndex = _horizontalScaleIndex;
        newScaleIndex += wheelRotation / wheelDelta;

        if (newScaleIndex > AC_GANTT_HORIZONTAL_SCALE_MAX_INDEX)
        {
            newScaleIndex = AC_GANTT_HORIZONTAL_SCALE_MAX_INDEX;
        }
        else if (newScaleIndex < AC_GANTT_HORIZONTAL_SCALE_MIN_INDEX)
        {
            newScaleIndex = AC_GANTT_HORIZONTAL_SCALE_MIN_INDEX;
        }

        // If this was a change:
        if (_horizontalScaleIndex != newScaleIndex)
        {
            // Get the former viewport width:
            acGanttChartDataType oldViewportWidth = valueOfViewPortWidth();

            // Calculate the new scale:
            double newScale = 1.0;

            for (int i = 0; i < newScaleIndex; i++)
            {
                newScale /= 0.75;
            }

            for (int i = 0; i > newScaleIndex; i--)
            {
                newScale *= 0.75;
            }

            // Mark the new scale and its index:
            _horizontalScaleIndex = newScaleIndex;
            _horizontalScale = newScale;

            // Get the new viewport width and make the view center on the same point:
            acGanttChartDataType newViewportWidth = valueOfViewPortWidth();

            if (oldViewportWidth > newViewportWidth)
            {
                _leftValueShown += ((oldViewportWidth - newViewportWidth) / 2);
            }
            else // (oldViewportWidth <= newViewportWidth)
            {
                _leftValueShown -= ((newViewportWidth - oldViewportWidth) / 2);
            }

            // Update the ruler step and string:
            updateRuler();

            // Update the data:
            updateDataAndLoadToOpenGL();

            // Redraw the window, but not immediately, since wxGLCanvas does not allow OGL
            // calls outside its paint event in some implementations.
            Refresh();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::onLeftDown
// Description: Called when the left mouse button is clicked on the chart
// Author:      Uri Shomroni
// Date:        28/2/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::onLeftDown(wxMouseEvent& eve)
{
    wxPoint clickedPos = eve.GetPosition();

    const acGanttChartBarData* pBarData = NULL;
    int seriesIndex = -1;
    int barIndex = -1;
    windowPositionToSeriesAndBarIndex(clickedPos, seriesIndex, barIndex, NULL, &pBarData);

    if (seriesIndex > -1)
    {
        // If we clicked a bar, get its original index:
        int barOriginalIndex = -1;

        if (barIndex > -1)
        {
            // If we got a bar, it should have data:
            GT_IF_WITH_ASSERT(pBarData != NULL)
            {
                // Get the index:
                barOriginalIndex = pBarData->_originalIndex;

                GT_ASSERT(barOriginalIndex > -1);
            }
        }

        // Send the event to my parent window:
        wxWindow* pMyParentWindow = wxWindow::GetParent();

        if (pMyParentWindow != NULL)
        {
            // Create a wxEvent with the series clicked (and the bar index if available:
            acGanttChartItemClickedEvent chartItemClickedEve(seriesIndex, barIndex, barOriginalIndex);
            pMyParentWindow->ProcessWindowEvent(chartItemClickedEve);
        }
    }

    // Process the event normally:
    eve.Skip();
}

// ---------------------------------------------------------------------------
// Name:        acImageManager::onScrollWindow
// Description: Called when the scroll controls on the window are manipulated
// Author:      Uri Shomroni
// Date:        24/2/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::onScrollWin(wxScrollWinEvent& eve)
{
    // Get the event type
    wxEventType eveType = eve.GetEventType();

    // We only handle line up, line down and dragging scroll events manually
    if ((eveType == wxEVT_SCROLLWIN_THUMBTRACK) || (eveType == wxEVT_SCROLLWIN_LINEUP) || (eveType == wxEVT_SCROLLWIN_LINEDOWN) || (eveType == wxEVT_SCROLLWIN_PAGEUP) || (eveType == wxEVT_SCROLLWIN_PAGEDOWN))
    {
        // Get new (desired) scroll position
        int newScrollPosition = eve.GetPosition();

        // Get the relevant scroll bar (horizontal / vertical);
        int scrollOrient = eve.GetOrientation();
        int scrollThumbSize = GetScrollThumb(scrollOrient);

        int stepSize = 0;

        if (scrollOrient == wxVERTICAL)
        {
            stepSize = 1;
        }
        else if (scrollOrient == wxHORIZONTAL)
        {
            stepSize = max(scrollThumbSize / 4, 1);
            // stepSize = (int)AC_GANTT_HORIZONTAL_SCROLLBAR_STEPS / 1000
        }

        int oldScrollPosition = GetScrollPos(scrollOrient);

        // Are we scrolling only one line up or down?
        if (eveType == wxEVT_SCROLLWIN_LINEUP)
        {
            // Decrease current scroll position by 1 step
            newScrollPosition = oldScrollPosition - stepSize;
        }
        else if (eveType == wxEVT_SCROLLWIN_LINEDOWN)
        {
            // Increase current scroll position by 1 step
            newScrollPosition = oldScrollPosition + stepSize;
        }
        else if (eveType == wxEVT_SCROLLWIN_PAGEUP)
        {
            // Decrease current scroll position by 4 steps
            newScrollPosition = oldScrollPosition - 4 * stepSize;
        }
        else if (eveType == wxEVT_SCROLLWIN_PAGEDOWN)
        {
            // Increase current scroll position by 4 steps
            newScrollPosition = oldScrollPosition + 4 * stepSize;
        }

        // Scroll to the new position
        SetScrollPos(scrollOrient, newScrollPosition, true);

        // Get the actual position, so that values out of range will get clipped:
        int newRealScrollPosition = GetScrollPos(scrollOrient);

        if (scrollOrient == wxVERTICAL)
        {
            // Vertical scroll, one scroll value is one line in the chart:
            _topLineShown = newRealScrollPosition;
            clampTopLineToValidRange();
        }
        else if (scrollOrient == wxHORIZONTAL)
        {
            // Horizontal scroll, AC_GANTT_HORIZONTAL_SCROLLBAR_STEPS scroll units are equal to the entire range between
            // _minValue and _maxValue:
            _leftValueShown = acGanttChartDataType(((double)newRealScrollPosition * ((double)_maxValue - (double)_minValue)) / AC_GANTT_HORIZONTAL_SCROLLBAR_STEPS) + _minValue;

            // Update the leftmost ruler section to be shown:
            updateFirstRulerStepToDraw();
        }
    }
    else
    {
        // For all other scroll events - handle them automatically
        eve.Skip();
    }

    // Refresh view
    Refresh();
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::clearAllData
// Description: Clears all the data from the vectors
// Author:      Uri Shomroni
// Date:        25/2/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::clearAllData()
{
    _backgroundSectionsData.clear();
    _barsDataBySeries.deleteElementsAndClear();
    _markersData.clear();

    // Load the empty data to clear the view:
    updateBeforeDataLoadedIntoOpenGL();

    // Mark that the OpenGL data is invalid:
    _isNewDataPending = true;

    // Redraw the window, but not immediately, since wxGLCanvas does not allow OGL
    // calls outside its paint event in some implementations.
    Refresh();
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::updateGanttChart
// Description: Update the gantt chart OpenGL data after loading series and other
//              data
// Author:      Uri Shomroni
// Date:        11/3/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::updateGanttChart()
{
    // Load the data into OpenGL:
    updateDataAndLoadToOpenGL();

    // Redraw the window, but not immediately, since wxGLCanvas does not allow OGL
    // calls outside its paint event in some implementations.
    Refresh();
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::setNumberOfSeries
// Description: Sets the number of series in the chart by assigning that many
//              items in the relevant vectors
// Author:      Uri Shomroni
// Date:        24/2/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::setNumberOfSeries(int numberOfSeries)
{
    // Clear any former data:
    _backgroundSectionsData.clear();
    _barsDataBySeries.deleteElementsAndClear();

    // Items at negative line numbers will always be off screen:
    acGanttChartBackgroundSectionData defaultBackgroundSection(bgColor());

    // Add the data:
    for (int i = 0; i < numberOfSeries; i++)
    {
        _backgroundSectionsData.push_back(defaultBackgroundSection);
        gtVector<acGanttChartBarData>* pCurrentSeriesVec = new gtVector<acGanttChartBarData>;
        _barsDataBySeries.push_back(pCurrentSeriesVec);
    }
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::setSeriesData
// Description: Sets the background color of a series
// Author:      Uri Shomroni
// Date:        24/2/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::setSeriesData(int seriesIndex, const wxColour& bgCol, const gtString& toolTip, const gtString& label)
{
    // Sanity check:
    int numberOfBackgroundSections = (int)_backgroundSectionsData.size();
    GT_IF_WITH_ASSERT((seriesIndex >= 0) && (seriesIndex < numberOfBackgroundSections))
    {
        // Set the color and tooltip:
        acGanttChartBackgroundSectionData& sectionBackgroundData = _backgroundSectionsData[seriesIndex];
        sectionBackgroundData._sectionColor = bgCol;
        sectionBackgroundData._toolTip = toolTip;
        sectionBackgroundData._label = label;
    }
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::addBarToSeries
// Description: Adds a bar to a series
// Author:      Uri Shomroni
// Date:        24/2/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::addBarToSeries(int seriesIndex, acGanttChartDataType barStart, acGanttChartDataType barEnd, const wxColour& barColor, int originalIndex, const gtString& label, const gtString& tooltip)
{
    // Sanity check index:
    int numberOfSeries = (int)_barsDataBySeries.size();
    GT_IF_WITH_ASSERT((seriesIndex >= 0) && (seriesIndex < numberOfSeries))
    {
        // Sanity check pointer:
        gtVector<acGanttChartBarData>* pSeriesVector = _barsDataBySeries[seriesIndex];
        GT_IF_WITH_ASSERT(pSeriesVector != NULL)
        {
            // Make sure the bar will be visible:
            if (barStart < barEnd)
            {
                // Create and add the item:
                acGanttChartBarData newBarData(barStart, barEnd, barColor, originalIndex, label, tooltip);
                pSeriesVector->push_back(newBarData);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::clearSeriesData
// Description: Clear all the bars from a series
// Author:      Uri Shomroni
// Date:        24/2/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::clearSeriesData(int seriesIndex)
{
    // Sanity check index:
    int numberOfSeries = (int)_barsDataBySeries.size();
    GT_IF_WITH_ASSERT((seriesIndex >= 0) && (seriesIndex < numberOfSeries))
    {
        // Sanity check pointer:
        gtVector<acGanttChartBarData>* pSeriesVector = _barsDataBySeries[seriesIndex];
        GT_IF_WITH_ASSERT(pSeriesVector != NULL)
        {
            // Clear the vector:
            pSeriesVector->clear();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::scrollWindowToBarPosition
// Description: Scrolls the view to be centered on the middle of the selected bar
// Author:      Uri Shomroni
// Date:        28/2/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::scrollWindowToBarPosition(int seriesIndex, int barIndex)
{
    // Series index sanity check:
    int numberOfSeries = (int)_barsDataBySeries.size();
    GT_IF_WITH_ASSERT((seriesIndex >= 0) && (seriesIndex < numberOfSeries))
    {
        // Pointer sanity check:
        const gtVector<acGanttChartBarData>* pSeriesData = _barsDataBySeries[seriesIndex];
        GT_IF_WITH_ASSERT(pSeriesData != NULL)
        {
            // bar index sanity check:
            const gtVector<acGanttChartBarData>& seriesData = *pSeriesData;
            int numberOfBarsInSeries = (int)seriesData.size();

            if ((barIndex >= 0) && (barIndex < numberOfBarsInSeries))
            {
                const acGanttChartBarData& barData = seriesData[barIndex];

                // Calculate the viewport's net width:
                acGanttChartDataType viewportNetWidthValue = valueOfViewPortWidth();
                acGanttChartDataType seriesLabelWidthValue = 0;

                if (_horizontalScale > 0.0)
                {
                    seriesLabelWidthValue = (acGanttChartDataType)((double)_seriesLabelsWidth / (_horizontalScale * AC_GANTT_HORIZONTAL_SCALE_MULTIPLIER));
                }

                if (viewportNetWidthValue > seriesLabelWidthValue)
                {
                    viewportNetWidthValue -= seriesLabelWidthValue;
                }
                else // viewportNetWidthValue <= seriesLabelWidthValue
                {
                    viewportNetWidthValue = 0;
                }

                // Calculate the new view left edge and top line:
                int newTopLine = barData._barLine;
                acGanttChartDataType newLeftEdge = ((barData._barStart + barData._barEnd) / 2) - (viewportNetWidthValue / 2);

                // Snap to the range:
                if ((newLeftEdge + viewportNetWidthValue) > _maxValue)
                {
                    newLeftEdge = (_maxValue > viewportNetWidthValue) ? (_maxValue - viewportNetWidthValue) : 0;
                }

                if (newLeftEdge < _minValue)
                {
                    newLeftEdge = _minValue;
                }

                // Scroll there:
                scrollWindow(newTopLine, newLeftEdge);
            }
            else // (barIndex >= 0) && (barIndex < numberOfBarsInSeries)
            {
                // Illegal bar index, just scroll to the first line in the series:
                int numberOfBackgroundSectionsData = (int)_backgroundSectionsData.size();
                GT_IF_WITH_ASSERT(seriesIndex < numberOfBackgroundSectionsData)
                {
                    // Scroll to the first line of this queue:
                    int newTopLine = _backgroundSectionsData[seriesIndex]._firstLine;

                    scrollWindow(newTopLine, _leftValueShown);
                }

                // We SHOULD get here only when scrolling to "bar -1":
                GT_ASSERT(barIndex == -1);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::getChartPartAmounts
// Description: Returns the total number of bars, background sections, and markers
// Author:      Uri Shomroni
// Date:        23/2/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::getChartPartAmounts(unsigned int& backgroundSections, unsigned int& bars, unsigned int& markers, unsigned int& labelCharacters, unsigned int& vscrollingControlRects, unsigned int& hscrollingControlRects, unsigned int& hscrollingControlLines) const
{
    unsigned int numberOfBackgroundSections = (unsigned int)_backgroundSectionsData.size();
    unsigned int numberOfSeries = (unsigned int)_barsDataBySeries.size();
    unsigned int totalNumberOfBars = 0;
    unsigned int totalBarLabelCharacters = 0;
    unsigned int totalNumberOfVScrControlRectangles = 0;
    unsigned int totalNumberOfHScrControlRectangles = 0;
    unsigned int totalNumberOfHScrControlLines = 0;

    for (unsigned int i = 0; i < numberOfSeries; i++)
    {
        const gtVector<acGanttChartBarData>* pCurrentSeriesData = _barsDataBySeries[i];
        GT_IF_WITH_ASSERT(pCurrentSeriesData != NULL)
        {
            const gtVector<acGanttChartBarData>& currentSeriesData = *pCurrentSeriesData;
            unsigned int numberOfBarsInSeries = (unsigned int)currentSeriesData.size();
            totalNumberOfBars += numberOfBarsInSeries;

            for (unsigned int j = 0; j < numberOfBarsInSeries; j++)
            {
                const acGanttChartBarData& currentBarData = currentSeriesData[j];
                unsigned int charactersInCurrentBarLabel = labelLengthInsideGanttBar(currentBarData);
                totalBarLabelCharacters += charactersInCurrentBarLabel;
            }
        }
    }

    unsigned int numberOfMarkers = (unsigned int)_markersData.size();

    // We only draw series labels (and their side bar) if we have any series:
    if (_numberOfChartLines > 0)
    {
        for (unsigned int i = 0; i < numberOfBackgroundSections; i++)
        {
            // Each series will have a control label, which is a white rectangle, and the label on top
            // has one rectangle per character:
            totalNumberOfVScrControlRectangles += (_backgroundSectionsData[i]._label.length() + 1);
        }

        totalNumberOfVScrControlRectangles++; // Plus one for the background's sidebar
    }

    // We only show a ruler if we have data:
    if (chartHasData())
    {
        totalNumberOfHScrControlRectangles = (_rulerMajorStepsToDraw * _rulerLongestLabelLength) + 1; // Plus one for the background
        totalNumberOfHScrControlLines = _rulerMajorStepsToDraw * _rulerSubdivisions; // _rulerSubdivions for each label
    }

    // Each series should have a section of background:
    GT_ASSERT(numberOfSeries == numberOfBackgroundSections);

    backgroundSections = numberOfBackgroundSections;
    bars = totalNumberOfBars;
    markers = numberOfMarkers;
    labelCharacters = totalBarLabelCharacters;
    vscrollingControlRects = totalNumberOfVScrControlRectangles;
    hscrollingControlRects = totalNumberOfHScrControlRectangles;
    hscrollingControlLines = totalNumberOfHScrControlLines;
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::fillBackgroundSectionsData
// Description: Fills the data for the background sections into the supplied arrays
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        23/2/2010
// ---------------------------------------------------------------------------
bool acGanttChartWindow::fillBackgroundSectionsData(const acOpenGLChartDataPointers& bgDataPointers)
{
    bool retVal = true;

    int numberOfBGSections = (int)_backgroundSectionsData.size();

    // Coordinates shared between all sections:
    acOpenGLChartPositionRect rect;
    rect._x0 = (numberOfBGSections > 0) ? horizontalPosition(_minValue) : 0.0f;
    rect._x1 = (numberOfBGSections > 0) ? horizontalPosition(_maxValue) : 0.0f;

    // Iterate the sections:
    for (int i = 0; i < numberOfBGSections; i++)
    {
        // Shared coordiantes and colors
        const acGanttChartBackgroundSectionData& currentSectionData = _backgroundSectionsData[i];
        const wxColour& sectionColor = currentSectionData._sectionColor;
        rect._y0 = lineTop(currentSectionData._firstLine, false);
        rect._y1 = lineBottom(currentSectionData._lastLine, false);
        acOpenGLChartColorDataType sectionColorAsArray[3] = {sectionColor.Red(), sectionColor.Green(), sectionColor.Blue()};

        // The index in the arrays of the first component of position and color for these vertices:
        unsigned int pos0 = i * AC_OPENGL_CHART_BACKGROUND_SECTION_AMOUNT_OF_POSITIONS * AC_OPENGL_CHART_POSITION_DATA_SIZE;
        unsigned int col0 = i * AC_OPENGL_CHART_BACKGROUND_SECTION_AMOUNT_OF_POSITIONS * AC_OPENGL_CHART_COLOR_DATA_SIZE;
        unsigned int txc0 = i * AC_OPENGL_CHART_BACKGROUND_SECTION_AMOUNT_OF_POSITIONS * AC_OPENGL_CHART_TEX_COORD_DATA_SIZE;
        unsigned int ind0 = i * AC_OPENGL_CHART_BACKGROUND_SECTION_AMOUNT_OF_VERTICES;
        acOpenGLChartIndexDataType ind0Val = (i * AC_OPENGL_CHART_BACKGROUND_SECTION_AMOUNT_OF_POSITIONS) + firstBackgroundIndex();

        // Add the rectangle:
        addRectangleToArrays(rect, sectionColorAsArray, sectionColorAsArray, _nullRectTexCoords, pos0, col0, txc0, ind0, ind0Val, bgDataPointers);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::fillBarsData
// Description: Fills the data for the bars into the supplied arrays
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        23/2/2010
// ---------------------------------------------------------------------------
bool acGanttChartWindow::fillBarsData(const acOpenGLChartDataPointers& barDataPointers, const acOpenGLChartDataPointers& labelDataPointers, int& barLabelPositions, int& barLabelVertices)
{
    bool retVal = true;

    // Prepare the arrays offset for the outlines:
    const unsigned int& barsToOutlinesPositionsOffset = barsToOutlinesPositions();
    const unsigned int& barsToOutlinesIndicesOffset = barsToOutlinesIndices();

    // Define data pointers for the bar outlines:
    acOpenGLChartDataPointers barOutlinesDataPointers;
    barOutlinesDataPointers._pPositions = &(barDataPointers._pPositions[barsToOutlinesPositionsOffset * AC_OPENGL_CHART_POSITION_DATA_SIZE]);
    barOutlinesDataPointers._pColors = &(barDataPointers._pColors[barsToOutlinesPositionsOffset * AC_OPENGL_CHART_COLOR_DATA_SIZE]);
    barOutlinesDataPointers._pTexCoords = &(barDataPointers._pTexCoords[barsToOutlinesPositionsOffset * AC_OPENGL_CHART_TEX_COORD_DATA_SIZE]);
    barOutlinesDataPointers._pIndices = &(barDataPointers._pIndices[barsToOutlinesIndicesOffset]);

    // Iterate the series
    int amountOfSeries = (int)_barsDataBySeries.size();
    int totalIndex = 0;
    int totalLabelVertices = 0;
    int totalLabelIndices = 0;

    for (int i = 0; i < amountOfSeries; i++)
    {
        // Pointer sanity check:
        const gtVector<acGanttChartBarData>* pCurrentSeries = _barsDataBySeries[i];
        GT_IF_WITH_ASSERT(pCurrentSeries != NULL)
        {
            const gtVector<acGanttChartBarData>& currentSeries = *pCurrentSeries;

            // Iterate the bars:
            int amountOfBarsInSeries = (int)currentSeries.size();

            for (int j = 0; j < amountOfBarsInSeries; j++, totalIndex++)
            {
                // Shared coordinates and colors:
                const acGanttChartBarData& currentBarData = currentSeries[j];
                const wxColour& barColor = currentBarData._barColor;
                acOpenGLChartPositionRect rect;
                rect._x0 = horizontalPosition(currentBarData._barStart);
                rect._x1 = horizontalPosition(currentBarData._barEnd);
                rect._y0 = lineTop(currentBarData._barLine, true);
                rect._y1 = lineBottom(currentBarData._barLine, true);
                acOpenGLChartColorDataType color0[3] = {barColor.Red() / 2 + 127, barColor.Green() / 2 + 127, barColor.Blue() / 2 + 127};
                acOpenGLChartColorDataType color1[3] = {barColor.Red(), barColor.Green(), barColor.Blue()};

                // The index in the arrays of the first component of position and color for these vertices:
                unsigned int pos0 = totalIndex * AC_OPENGL_CHART_BAR_AMOUNT_OF_POSITIONS * AC_OPENGL_CHART_POSITION_DATA_SIZE;
                unsigned int col0 = totalIndex * AC_OPENGL_CHART_BAR_AMOUNT_OF_POSITIONS * AC_OPENGL_CHART_COLOR_DATA_SIZE;
                unsigned int txc0 = totalIndex * AC_OPENGL_CHART_BAR_AMOUNT_OF_POSITIONS * AC_OPENGL_CHART_TEX_COORD_DATA_SIZE;
                unsigned int ind0 = totalIndex * AC_OPENGL_CHART_BAR_AMOUNT_OF_VERTICES;
                acOpenGLChartIndexDataType ind0Val = (totalIndex * AC_OPENGL_CHART_BAR_AMOUNT_OF_POSITIONS) + firstBarIndex();

                // Add the bar:
                addRectangleToArrays(rect, color0, color1, _nullRectTexCoords, pos0, col0, txc0, ind0, ind0Val, barDataPointers, true);

                // The index in the arrays of the first component of position and color for these vertices:
                unsigned int outPos0 = totalIndex * AC_OPENGL_CHART_BAR_OUTLINE_AMOUNT_OF_POSITIONS * AC_OPENGL_CHART_POSITION_DATA_SIZE;
                unsigned int outCol0 = totalIndex * AC_OPENGL_CHART_BAR_OUTLINE_AMOUNT_OF_POSITIONS * AC_OPENGL_CHART_COLOR_DATA_SIZE;
                unsigned int outTxc0 = totalIndex * AC_OPENGL_CHART_BAR_OUTLINE_AMOUNT_OF_POSITIONS * AC_OPENGL_CHART_TEX_COORD_DATA_SIZE;
                unsigned int outInd0 = totalIndex * AC_OPENGL_CHART_BAR_OUTLINE_AMOUNT_OF_VERTICES;
                acOpenGLChartIndexDataType outInd0Val = (totalIndex * AC_OPENGL_CHART_BAR_OUTLINE_AMOUNT_OF_POSITIONS) + firstBarIndex() + barsToOutlinesPositionsOffset;

                // Add the bar outline:
                addRectangleOutlineToArrays(rect, _textAndDetailColorAsArray, _textAndDetailColorAsArray, _nullRectTexCoords, outPos0, outCol0, outTxc0, outInd0, outInd0Val, barOutlinesDataPointers, true);

                // Add the bar's label:
                int addedVertices = 0;
                int addedIndices = 0;
                acOpenGLChartDataPointers currentLabelDataPointers;
                currentLabelDataPointers._pPositions = &(labelDataPointers._pPositions[totalLabelVertices * AC_OPENGL_CHART_POSITION_DATA_SIZE]);
                currentLabelDataPointers._pColors = &(labelDataPointers._pColors[totalLabelVertices * AC_OPENGL_CHART_COLOR_DATA_SIZE]);
                currentLabelDataPointers._pTexCoords = &(labelDataPointers._pTexCoords[totalLabelVertices * AC_OPENGL_CHART_TEX_COORD_DATA_SIZE]);
                currentLabelDataPointers._pIndices = &(labelDataPointers._pIndices[totalLabelIndices]);
                addTextLinesInsideBar(rect, totalLabelVertices + firstLabelIndex(), currentBarData._label, L"", false, addedVertices, addedIndices, currentLabelDataPointers);
                totalLabelVertices += (addedVertices > 0) ? addedVertices : 0;
                totalLabelIndices += (addedIndices > 0) ? addedIndices : 0;
            }
        }
    }

    // Output the data sizes:
    barLabelPositions = totalLabelVertices;
    barLabelVertices = totalLabelIndices;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::fillMarkersData
// Description: Fills the data for the markers into the supplied arrays
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        23/2/2010
// ---------------------------------------------------------------------------
bool acGanttChartWindow::fillMarkersData(const acOpenGLChartDataPointers& markerDataPointers)
{
    bool retVal = true;

    // TO_DO: implement me!

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::fillControlsData
// Description: Fills the data for the (window-fixed) controls into the supplied arrays
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        15/3/2010
// ---------------------------------------------------------------------------
bool acGanttChartWindow::fillControlsData(const acOpenGLChartDataPointers& controlDataPointers)
{
    bool retVal = true;

    int amountOfSeries = (int)_backgroundSectionsData.size();
    int amountOfVertices = 0;
    int amountOfIndices = 0;

    // Common coordinates:
    acOpenGLChartPositionRect rect;

    rect._x0 = 0.0f;
    rect._x1 = rect._x0 + _seriesLabelsWidth;

    // If we have any data, show a bar on the side:
    if (_numberOfChartLines > 0)
    {
        // Note that using wxSystemSettings::GetMetric(wxSYS_SCREEN_#) gives the entire screen and not the inside of the taskbar.
        acOpenGLChartPositionDataType screenHeightInOpenGLCoords = (acOpenGLChartPositionDataType)(wxSystemSettings::GetMetric(wxSYS_SCREEN_Y)) / AC_OPENGL_CHART_WINDOW_PIXEL_SIZE_OF_1_Y_UNIT;

        // Add one rectangle for the background:
        rect._y0 = lineTop(0, false);
        rect._y1 = lineBottom(_numberOfChartLines, false);
        acOpenGLChartPositionDataType by1Max = rect._y0 - screenHeightInOpenGLCoords; // Make sure this covers at least one screen, if we don't have enough data

        if (rect._y1 > by1Max)
        {
            rect._y1 = by1Max;
        }

        const wxColour& bColor = bgColor();
        // acOpenGLChartColorDataType bgColor0[3] = {bColor.Red(), bColor.Green(), bColor.Blue()};
        acOpenGLChartColorDataType bgColor1[3] = {bColor.Red() / 2 + 95, bColor.Green() / 2 + 95, bColor.Blue() / 2 + 95};

        // The index in the arrays of the first component of position and color for these vertices:
        unsigned int bgPos0 = amountOfVertices * AC_OPENGL_CHART_POSITION_DATA_SIZE;
        unsigned int bgCol0 = amountOfVertices * AC_OPENGL_CHART_COLOR_DATA_SIZE;
        unsigned int bgTxc0 = amountOfVertices * AC_OPENGL_CHART_TEX_COORD_DATA_SIZE;
        unsigned int bgInd0 = amountOfIndices;
        acOpenGLChartIndexDataType bgInd0Val = amountOfVertices + firstControlIndex();

        // Add the background:
        addRectangleToArrays(rect, bgColor1, bgColor1, _nullRectTexCoords, bgPos0, bgCol0, bgTxc0, bgInd0, bgInd0Val, controlDataPointers, false);

        // Increment the counters:
        amountOfVertices += AC_OPENGL_CHART_CONTROL_RECT_AMOUNT_OF_POSITIONS;
        amountOfIndices += AC_OPENGL_CHART_CONTROL_RECT_AMOUNT_OF_VERTICES;

        // Iterate the series
        for (int i = 0; i < amountOfSeries; i++)
        {
            // Shared coordinates and colors:
            const acGanttChartBackgroundSectionData& currentSeriesData = _backgroundSectionsData[i];
            const wxColour& seriesColor = currentSeriesData._sectionColor;
            rect._y0 = lineTop(currentSeriesData._firstLine, false);
            rect._y1 = lineBottom(currentSeriesData._lastLine, false);
            acOpenGLChartColorDataType seriesColor0[3] = {seriesColor.Red(), seriesColor.Green(), seriesColor.Blue()};
            acOpenGLChartColorDataType seriesColor1[3] = {seriesColor.Red() / 2 + 63, seriesColor.Green() / 2 + 63, seriesColor.Blue() / 2 + 63};

            // The index in the arrays of the first component of position and color for these vertices:
            unsigned int pos0 = amountOfVertices * AC_OPENGL_CHART_POSITION_DATA_SIZE;
            unsigned int col0 = amountOfVertices * AC_OPENGL_CHART_COLOR_DATA_SIZE;
            unsigned int txc0 = amountOfVertices * AC_OPENGL_CHART_TEX_COORD_DATA_SIZE;
            unsigned int ind0 = amountOfIndices;
            acOpenGLChartIndexDataType ind0Val = amountOfVertices + firstControlIndex();

            // Add the background:
            addRectangleToArrays(rect, seriesColor0, seriesColor1, _nullRectTexCoords, pos0, col0, txc0, ind0, ind0Val, controlDataPointers, true);

            // Increment the counters:
            amountOfVertices += AC_OPENGL_CHART_CONTROL_RECT_AMOUNT_OF_POSITIONS;
            amountOfIndices += AC_OPENGL_CHART_CONTROL_RECT_AMOUNT_OF_VERTICES;

            // Add the series label string:
            int addedVertices = 0;
            int addedIndices = 0;
            acOpenGLChartDataPointers currentLabelDataPointers;
            currentLabelDataPointers._pPositions = &(controlDataPointers._pPositions[amountOfVertices * AC_OPENGL_CHART_POSITION_DATA_SIZE]);
            currentLabelDataPointers._pColors = &(controlDataPointers._pColors[amountOfVertices * AC_OPENGL_CHART_COLOR_DATA_SIZE]);
            currentLabelDataPointers._pTexCoords = &(controlDataPointers._pTexCoords[amountOfVertices * AC_OPENGL_CHART_TEX_COORD_DATA_SIZE]);
            currentLabelDataPointers._pIndices = &(controlDataPointers._pIndices[amountOfIndices]);
            addTextLinesInsideBar(rect, amountOfVertices + firstControlIndex(), currentSeriesData._label, L"", false, addedVertices, addedIndices, currentLabelDataPointers);
            amountOfVertices += (addedVertices > 0) ? addedVertices : 0;
            amountOfIndices += (addedIndices > 0) ? addedIndices : 0;
        }
    }

    // If we have a ruler:
    if (chartHasData())
    {
        // Ruler positions:
        acOpenGLChartPositionRect bgRect;
        bgRect._y0 = 0.0f;
        bgRect._y1 = bgRect._y0 + AC_GANTT_RULER_HEIGHT;
        acOpenGLChartPositionDataType ryMid0 = (bgRect._y0 + (bgRect._y1 * 5.0f)) / 6.0f;
        acOpenGLChartPositionDataType ryMid1 = (bgRect._y0 + (bgRect._y1 * 2.0f)) / 3.0f;

        // Create the ruler background:
        bgRect._x0 = horizontalPosition(_minValue) - _seriesLabelsWidth;
        bgRect._x1 = horizontalPosition(_minValue + rulerLength(), true);
        wxColour rulerBgColor = AC_GANTT_RULER_BACKGROUND_COLOR;
        acOpenGLChartColorDataType rulerBgColorAsArray[3] = {rulerBgColor.Red(), rulerBgColor.Green(), rulerBgColor.Blue()};
        unsigned int pos0 = amountOfVertices * AC_OPENGL_CHART_POSITION_DATA_SIZE;
        unsigned int col0 = amountOfVertices * AC_OPENGL_CHART_COLOR_DATA_SIZE;
        unsigned int txc0 = amountOfVertices * AC_OPENGL_CHART_TEX_COORD_DATA_SIZE;
        unsigned int ind0 = amountOfIndices;
        acOpenGLChartIndexDataType ind0Val = amountOfVertices + firstControlIndex();

        // Draw the ruler background:
        addRectangleToArrays(bgRect, rulerBgColorAsArray, rulerBgColorAsArray, _nullRectTexCoords, pos0, col0, txc0, ind0, ind0Val, controlDataPointers);

        // Increment the counters:
        amountOfVertices += AC_OPENGL_CHART_CONTROL_RECT_AMOUNT_OF_POSITIONS;
        amountOfIndices += AC_OPENGL_CHART_CONTROL_RECT_AMOUNT_OF_VERTICES;

        // Create the ruler labels and markers:
        acGanttChartDataType currentScaleMark = _minValue + (_rulerStepSize * _rulerFirstStepToDraw);
        acOpenGLChartPositionDataType rulerLabelWidth = ((acOpenGLChartPositionDataType)_rulerLongestLabelLength * (_characterOpenGLWidth + _characterSpacingOpenGLWidth + _characterMarginOpenGLWidth) * 2.0f);

        for (unsigned int i = 0; i < _rulerMajorStepsToDraw; i++)
        {
            // Give a fictitious X size so all the label will be written:
            acOpenGLChartPositionRect labelRect;
            acOpenGLChartPositionDataType rx = horizontalPosition(currentScaleMark, true);
            labelRect._x0 = rx;
            labelRect._x1 = labelRect._x0 + rulerLabelWidth;
            labelRect._y0 = bgRect._y0;
            labelRect._y1 = bgRect._y1;

            // Create a ruler label, and make sure it is the right size:
            gtString currentRulerLabel;
            rulerValueToLabel((currentScaleMark > _minValue ? currentScaleMark - _minValue : 0), currentRulerLabel);
            unsigned int originalLabelLength = (unsigned int)(currentRulerLabel.length());

            if (originalLabelLength > _rulerLongestLabelLength)
            {
                // This might happen for every label, so only assert in Debug builds:
#if AMDT_BUILD_CONFIGURATION == AMDT_DEBUG_BUILD
                GT_ASSERT(false);
#endif

                currentRulerLabel.truncate(0, _rulerLongestLabelLength - 1);
            }
            else if (originalLabelLength < _rulerLongestLabelLength)
            {
                // We need to fill the label with spaces to make sure the data is the right size:
                while (currentRulerLabel.length() < (int)_rulerLongestLabelLength)
                {
                    currentRulerLabel.append(' ');
                }
            }

            // Add the current label:
            int addedVertices = 0;
            int addedIndices = 0;
            acOpenGLChartDataPointers currentLabelDataPointers;
            currentLabelDataPointers._pPositions = &(controlDataPointers._pPositions[amountOfVertices * AC_OPENGL_CHART_POSITION_DATA_SIZE]);
            currentLabelDataPointers._pColors = &(controlDataPointers._pColors[amountOfVertices * AC_OPENGL_CHART_COLOR_DATA_SIZE]);
            currentLabelDataPointers._pTexCoords = &(controlDataPointers._pTexCoords[amountOfVertices * AC_OPENGL_CHART_TEX_COORD_DATA_SIZE]);
            currentLabelDataPointers._pIndices = &(controlDataPointers._pIndices[amountOfIndices]);
            addTextLinesInsideBar(labelRect, amountOfVertices + firstControlIndex(), currentRulerLabel, L"", false, addedVertices, addedIndices, currentLabelDataPointers);
            amountOfVertices += (addedVertices > 0) ? addedVertices : 0;
            amountOfIndices += (addedIndices > 0) ? addedIndices : 0;

            // Move to the next ruler mark:
            currentScaleMark += _rulerStepSize;
        }

        // Add the marker lines:
        acOpenGLChartPositionDataType markerSpacing = (horizontalPosition(_minValue + _rulerStepSize, true) - horizontalPosition(_minValue, true)) / (acOpenGLChartPositionDataType)_rulerSubdivisions;
        wxColour markerColor = *wxBLACK;
        acOpenGLChartColorDataType markerColorAsArray[3] = {markerColor.Red(), markerColor.Green(), markerColor.Blue()};

        for (unsigned int i = 0; i < _rulerMajorStepsToDraw; i++)
        {
            acOpenGLChartPositionDataType currentLabelMarkersBase = horizontalPosition(_minValue + ((_rulerFirstStepToDraw + i) * _rulerStepSize), true);

            for (unsigned int j = 0; j < _rulerSubdivisions; j++)
            {
                // The major marker is the first in each subdivision
                bool majorMarker = (j == 0);
                bool secondaryMarker = (!majorMarker) && ((j % 5) == 0);
                bool minorMarker = (!majorMarker) && (!secondaryMarker);

                // The index in the arrays of the first component of position and color for these vertices:
                unsigned int pos0 = amountOfVertices * AC_OPENGL_CHART_POSITION_DATA_SIZE;
                unsigned int col0 = amountOfVertices * AC_OPENGL_CHART_COLOR_DATA_SIZE;
                unsigned int txc0 = amountOfVertices * AC_OPENGL_CHART_TEX_COORD_DATA_SIZE;
                unsigned int ind0 = amountOfIndices;
                acOpenGLChartPositionRect markerRectPosition;
                markerRectPosition._x0 = currentLabelMarkersBase + ((acOpenGLChartPositionDataType)j * markerSpacing);
                markerRectPosition._x1 = currentLabelMarkersBase + ((acOpenGLChartPositionDataType)j * markerSpacing);
                markerRectPosition._y0 = (minorMarker ? ryMid0 : (secondaryMarker ? ryMid1 : bgRect._y0));
                markerRectPosition._y1 = bgRect._y1;

                acOpenGLChartIndexDataType ind0Val = amountOfVertices + firstControlIndex();

                // Add the marker:
                addLineToArrays(markerRectPosition , markerColorAsArray, markerColorAsArray, _nullLineTexCoords, pos0, col0, txc0, ind0, ind0Val, controlDataPointers);

                // Increment the counters:
                amountOfVertices += AC_OPENGL_CHART_CONTROL_LINE_AMOUNT_OF_POSITIONS;
                amountOfIndices += AC_OPENGL_CHART_CONTROL_LINE_AMOUNT_OF_VERTICES;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::updateBeforeDataLoadedIntoOpenGL
// Description: Called before the acOpenGLChartWindow implementation loads data
//              into OpenGL. This should make all preparations needed for the data
//              loading
// Author:      Uri Shomroni
// Date:        11/3/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::updateBeforeDataLoadedIntoOpenGL()
{
    // Reposition the data and set the line heights according to it:
    calculateDataLines();

    // Update the scrollbars:
    updateScrollbarRangeAndPosition();
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::updateBeforeControlsDataLoadedIntoOpenGL
// Description: Called before the acOpenGLChartWindow implementation loads the controls
//              data into OpenGL. This should make all preparations needed for the data
//              loading
// Author:      Uri Shomroni
// Date:        11/4/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::updateBeforeControlsDataLoadedIntoOpenGL()
{
    // Update the ruler data:
    updateRuler();
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::beforeDrawingOpenGLData
// Description: Called before drawing the OpenGL data, with the context already current
// Author:      Uri Shomroni
// Date:        11/3/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::beforeDrawingOpenGLData()
{
    // Move the camera to its position:
    glPushMatrix();
    moveCamera(true, true);
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::afterDrawingOpenGLData
// Description: Called after drawing the OpenGL data, with the context still current
// Author:      Uri Shomroni
// Date:        11/3/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::afterDrawingOpenGLData()
{
    // Restore the camera to the origin:
    glPopMatrix();
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::beforeDrawingVScrollingWindowControls
// Description: Called before drawing the window controls
// Author:      Uri Shomroni
// Date:        15/3/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::beforeDrawingVScrollingWindowControls()
{
    // Move the camera, but only on the Y axis:
    glPushMatrix();
    moveCamera(false, true);
}

// ---------------------------------------------------------------------------
// Name:        ::afterDrawingVScrollingWindowControls
// Description: Called after drawing the window controls
// Author:      Uri Shomroni
// Date:        15/3/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::afterDrawingVScrollingWindowControls()
{
    // Restore the camera to the origin:
    glPopMatrix();
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::beforeDrawingHScrollingWindowControls
// Description: Called before drawing the window controls
// Author:      Uri Shomroni
// Date:        15/3/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::beforeDrawingHScrollingWindowControls()
{
    // Move the camera, but only on the X axis:
    glPushMatrix();
    moveCamera(true, false);

    // Adjust the ruler to the bottom of the window:
    acOpenGLChartPositionDataType dy = rulerVerticalPosition();
    glTranslatef(0.0f, dy, 0.0f);
}

// ---------------------------------------------------------------------------
// Name:        ::afterDrawingVScrollingWindowControls
// Description: Called after drawing the window controls
// Author:      Uri Shomroni
// Date:        15/3/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::afterDrawingHScrollingWindowControls()
{
    // Restore the camera to the origin:
    glPopMatrix();
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::beforeResize
// Description: Called before the window is resized to allow making OpenGL comensations
//              for the new size
// Author:      Uri Shomroni
// Date:        11/3/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::beforeResize()
{
    // Recalculate the new number of lines shown:
    acOpenGLChartPositionDataType newWindowHeight = (acOpenGLChartPositionDataType)(GetClientSize().GetHeight()) / AC_OPENGL_CHART_WINDOW_PIXEL_SIZE_OF_1_Y_UNIT;
    _numberOfLinesShown = (int)((newWindowHeight - AC_GANTT_RULER_HEIGHT) / AC_GANTT_LINE_HEIGHT);

    // Show at least one line and at most as many as there are in the graph:
    if (_numberOfLinesShown < 1)
    {
        _numberOfLinesShown = 1;
    }

    // Update the V scrollbar:
    updateScrollbarRangeAndPosition();

    // Scroll to our current position, so that the view will not show empty space if not needed:
    clampTopLineToValidRange();
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::scrollWindow
// Description: Scrolls the window to the position selected
// Author:      Uri Shomroni
// Date:        24/2/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::scrollWindow(int topLineShown, acGanttChartDataType leftValueShown)
{
    // Set the values:
    _topLineShown = topLineShown;
    _leftValueShown = leftValueShown;
    clampTopLineToValidRange();

    // Update the scroll bars:
    updateScrollbarRangeAndPosition();

    // Update the leftmost ruler value shown:
    updateFirstRulerStepToDraw();

    // Refresh the view:
    Refresh();
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::clampTopLineToValidRange
// Description: Clamps the _topLineShown member value to its valid range
// Author:      Uri Shomroni
// Date:        12/4/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::clampTopLineToValidRange()
{
    // Avoid showing more than one line of empty space below the chart, and avoid scrolling to any line higher than #0:
    if ((_topLineShown + _numberOfLinesShown) > _numberOfChartLines)
    {
        _topLineShown = _numberOfChartLines - _numberOfLinesShown;
    }

    if (_topLineShown < 0)
    {
        _topLineShown = 0;
    }
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::updateScrollbarRangeAndPosition
// Description: Updates the scrollbars' ranges and positions according to the
//              scroll members
// Author:      Uri Shomroni
// Date:        24/2/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::updateScrollbarRangeAndPosition()
{
    // Enable the vertical scrollbar iff we have more than one line:
    if (_numberOfChartLines > 1)
    {
        SetScrollbar(wxVERTICAL, _topLineShown, _numberOfLinesShown, _numberOfChartLines, true);
    }
    else // (_numberOfChartLines <= 1)
    {
        SetScrollbar(wxVERTICAL, 0, 0, 0, true);
    }

    // Enable the horizontal scrollbar iff we have data. Also note that if hScrollbarPage >= AC_GANTT_HORIZONTAL_SCROLLBAR_STEPS,
    // no scrollbar would be displayed either:
    if ((_maxValue > _minValue) && chartHasData())
    {
        // Converts OpenGL units to scrollbar units:
        double hScrollbarValueToRange = (AC_GANTT_HORIZONTAL_SCROLLBAR_STEPS / ((double)_maxValue - (double)_minValue)) / (_horizontalScale * AC_GANTT_HORIZONTAL_SCALE_MULTIPLIER);

        // The size of the shown data:
        double hScrollbarPage = (((double)GetClientSize().GetWidth() / (double)AC_OPENGL_CHART_WINDOW_PIXEL_SIZE_OF_1_X_UNIT) - (double)_seriesLabelsWidth) * hScrollbarValueToRange;

        if (hScrollbarPage < 0.0)
        {
            hScrollbarPage = 0.0;
        }

        // The position of the left end of the scroll bar:
        double hScrollbarPos = AC_GANTT_HORIZONTAL_SCROLLBAR_STEPS * ((double)_leftValueShown - (double)_minValue) / ((double)_maxValue - (double)_minValue);

        // Move the scrollbar:
        SetScrollbar(wxHORIZONTAL, (int)hScrollbarPos, (int)hScrollbarPage, (int)AC_GANTT_HORIZONTAL_SCROLLBAR_STEPS, true);
    }
    else // (_maxValue <= _minValue) || !chartHasData()
    {
        SetScrollbar(wxHORIZONTAL, 0, 0, 0, true);
    }
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::windowPositionToSeriesAndBarIndex
// Description: Performs a pick operation on the chart, translating a window
//              position to a series index and a bar index. If the point is in
//              a series background, the returned value will be seriesIndex = n
//              barIndex = -1.
//              If ppBackgroundSectionData or ppBarData != NULL, returns a pointer to the found bar data as well.
// Author:      Uri Shomroni
// Date:        25/2/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::windowPositionToSeriesAndBarIndex(const wxPoint& windowPos, int& seriesIndex, int& barIndex, const acGanttChartBackgroundSectionData** ppBackgroundSectionData, const acGanttChartBarData** ppBarData)
{
    // Set "not found" values:
    seriesIndex = -1;
    barIndex = -1;

    // Get the client rectangle in window coordinates, and check if the point is over the OpenGL area:
    wxRect clientRect = GetClientRect();

    if (clientRect.Contains(windowPos))
    {
        // Translate to client coordinate:
        wxPoint clientPos(windowPos.x - clientRect.GetLeft(), windowPos.y - clientRect.GetTop());

        // Translate the x coordinate to a time offset:
        acOpenGLChartPositionDataType xCoord = horizontalPosition(_leftValueShown) - _seriesLabelsWidth + ((acOpenGLChartPositionDataType)clientPos.x / AC_OPENGL_CHART_WINDOW_PIXEL_SIZE_OF_1_X_UNIT);
        acGanttChartDataType hPosition = (acGanttChartDataType)((xCoord / (_horizontalScale * AC_GANTT_HORIZONTAL_SCALE_MULTIPLIER)) + (((double)_minValue + (double)_maxValue) / 2.0));

        // If this is inside the chart:
        if ((hPosition >= _minValue) && (hPosition <= _maxValue))
        {
            // Translate the y coordinates to a line #:
            acOpenGLChartPositionDataType yCoord = lineTop(_topLineShown, false) - ((acOpenGLChartPositionDataType)clientPos.y / AC_OPENGL_CHART_WINDOW_PIXEL_SIZE_OF_1_Y_UNIT);
            int lineNumber = (int)((AC_GANTT_LINE_TOP_OF_FIRST_LINE - yCoord) / AC_GANTT_LINE_HEIGHT);

            // Verify the result:
            GT_ASSERT((yCoord >= lineBottom(lineNumber, false)) && (yCoord <= lineTop(lineNumber, false)));

            // If this is a legal line number:
            if (lineNumber >= 0)
            {
                // Find which series this line belongs to:
                int numberOfBackgroundSections = (int)_backgroundSectionsData.size();

                for (int i = 0; i < numberOfBackgroundSections; i++)
                {
                    // Check if the line number matches this line:
                    const acGanttChartBackgroundSectionData& currentSectionBackground = _backgroundSectionsData[i];

                    if ((lineNumber >= currentSectionBackground._firstLine) && (lineNumber <= currentSectionBackground._lastLine))
                    {
                        // Return its index:
                        seriesIndex = i;

                        if (ppBackgroundSectionData != NULL)
                        {
                            // Return its data, if requested:
                            *ppBackgroundSectionData = &currentSectionBackground;
                        }

                        break;
                    }
                }

                // If we found a series, see if the value is in one of its bars:
                if (seriesIndex > -1)
                {
                    // Check if we are not in the line spacing:
                    if ((yCoord > lineBottom(lineNumber, true)) && (yCoord < lineTop(lineNumber, true)))
                    {
                        // The y coordinate is inside a line's content, see if the h position is inside a bar:
                        int numberOfSeries = (int)_barsDataBySeries.size();
                        GT_IF_WITH_ASSERT(seriesIndex < numberOfSeries)
                        {
                            // Pointer sanity check:
                            const gtVector<acGanttChartBarData>* pSeriesVector = _barsDataBySeries[seriesIndex];
                            GT_IF_WITH_ASSERT(pSeriesVector != NULL)
                            {
                                // Iterate the bars:
                                const gtVector<acGanttChartBarData>& seriesVector = *pSeriesVector;
                                int barsInSeries = (int)seriesVector.size();

                                for (int j = 0; j < barsInSeries; j++)
                                {
                                    // If we are on the same line as the bar and between its start and end:
                                    const acGanttChartBarData& currentBar = seriesVector[j];

                                    if ((lineNumber == currentBar._barLine) && (hPosition >= currentBar._barStart) && (hPosition <= currentBar._barEnd))
                                    {
                                        // Return the index:
                                        barIndex = j;

                                        if (ppBarData != NULL)
                                        {
                                            // Return the data, if requested:
                                            *ppBarData = &currentBar;
                                        }

                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::rulerValueToLabel
// Description: Creates a ruler scale mark value and makes a label our of it using
//              _rulerLabelSeparators.
// Author:      Uri Shomroni
// Date:        15/3/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::rulerValueToLabel(acGanttChartDataType scaleMarkValue, gtString& label)
{
    label.makeEmpty();

    if (!_rulerLabelSeparators.isEmpty())
    {
        int numberOfSeparators = _rulerLabelSeparators.length();
        acGanttChartDataType scaleMarkValueTrunc = scaleMarkValue;

        for (int i = 0; i < numberOfSeparators; i++)
        {
            // The last item should have all left over values:
            acGanttChartDataType currentMagnitudeValue = scaleMarkValueTrunc;
            bool lastMagnitude = i < numberOfSeparators - 1;

            if (lastMagnitude)
            {
                currentMagnitudeValue = scaleMarkValueTrunc % 1000;
            }

            if (currentMagnitudeValue > 0)
            {
                // Translate the current magnitude value to a string:
                gtString magnitudeValueAsString;
                magnitudeValueAsString.appendFormattedString(L"%llu", currentMagnitudeValue);

                if (lastMagnitude && (currentMagnitudeValue >= 1000))
                {
                    magnitudeValueAsString.addThousandSeperators();
                }

                // Prepend the separators and the values so that the smallest value will be leftmost:
                label.prepend(_rulerLabelSeparators[i]).prepend(magnitudeValueAsString);
            }

            // Discard the data for this order of magnitude and stop if we have no data left:
            scaleMarkValueTrunc /= 1000;

            if (scaleMarkValueTrunc == 0)
            {
                break;
            }
        }
    }
    else
    {
        // Just make a string out of the number:
        label.appendFormattedString(L"%llu", scaleMarkValue).addThousandSeperators();
    }

    // Make the label 0 if the value is 0:
    if (label.isEmpty())
    {
        label = '0';
    }
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::updateRuler
// Description: Updates the ruler's range and labels
// Author:      Uri Shomroni
// Date:        15/3/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::updateRuler()
{
    // Using the scale, figure out which ruler size is needed:
    _rulerStepSize = 1;
    unsigned int rulerStepTens = 0;
    bool goOn = true;

    while (goOn)
    {
        double rulerStepTimesScale = (_horizontalScale * AC_GANTT_HORIZONTAL_SCALE_MULTIPLIER * (double)_rulerStepSize);
        double stepTimesScaleToMinScreenSize = rulerStepTimesScale / (double)(AC_GANTT_RULER_MIN_LABEL_SPACING / AC_OPENGL_CHART_WINDOW_PIXEL_SIZE_OF_1_X_UNIT);

        if (stepTimesScaleToMinScreenSize < 0.1)
        {
            _rulerStepSize *= 10;
            rulerStepTens++;
        }
        else // stepTimesScaleToMinScreenSize >= 0.1
        {
            // The subdivisions should be so that each line will be at the previous order of magnitude:
            _rulerSubdivisions = 10;

            // Allow 2*10^n and 5*10^n sizes:
            if (stepTimesScaleToMinScreenSize < 0.2)
            {
                _rulerStepSize *= 5;
                _rulerSubdivisions = 5;
            }
            else if (stepTimesScaleToMinScreenSize < 0.5)
            {
                _rulerStepSize *= 2;
                _rulerSubdivisions = 2;
            }

            // Use this size:
            goOn = false;
        }
    }

    // Recalculate the number of ruler steps to draw so that one screen's width is filled (so we only need to recreate the
    // data on scrolling. Add one for buffering the offset:
    acGanttChartDataType scrWidthValue = screenWidthValue();
    _rulerMajorStepsToDraw = (unsigned int)(ceil((double)scrWidthValue / (double)_rulerStepSize)) + 1;

    // Update the ruler:
    updateFirstRulerStepToDraw();

    // We will write zeros down to the smallest order of magnitudes (i.e. if the step is 500,000 units,
    // the smallest order of magnitude is 1,000). However, this cannot be smaller than the number of separators we have:
    unsigned int rulerSmallestMagnitude = rulerStepTens / 3;
    unsigned int numberOfSeparators = (unsigned int)(_rulerLabelSeparators.length());

    if (rulerSmallestMagnitude > numberOfSeparators - 1)
    {
        rulerSmallestMagnitude = numberOfSeparators - 1;
    }

    if (chartHasData())
    {
        acGanttChartDataType rulerLen = rulerLength();

        // There are ceil(len / stepsize) major divisions, and a label for each:
        _rulerLabelsNumber = (rulerLen + _rulerStepSize - 1) / _rulerStepSize;

        // The longest length is the length of the longest string, minus 4 per order of magnitude of the step (3 digits, plus one separator):
        gtString highestNumberLabel;
        highestNumberLabel.appendFormattedString(L"%llu", rulerLen).addThousandSeperators();
        _rulerLongestLabelLength = highestNumberLabel.length() - (4 * rulerSmallestMagnitude);

        // If we have separators, at least one will be added in the end:
        if (numberOfSeparators > 0)
        {
            _rulerLongestLabelLength++;
        }
    }
    else // !chartHasData()
    {
        _rulerLabelsNumber = 0;
        _rulerLongestLabelLength = 0;
    }
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::updateFirstRulerStepToDraw
// Description: Updates the position of the first ruler step to draw based on
//              the left value shown and the ruler step size
// Author:      Uri Shomroni
// Date:        11/4/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::updateFirstRulerStepToDraw()
{
    int firstRulerStep = 0;

    // This value will only have meaning if we have a ruler:
    if (_rulerStepSize > 0)
    {
        acGanttChartDataType leftPointOffset = (_leftValueShown > _minValue) ? (_leftValueShown - _minValue) : 0;

        // Round down, for the minor steps to be shown properly:
        firstRulerStep = leftPointOffset / _rulerStepSize;
    }

    // Set the value:
    _rulerFirstStepToDraw = firstRulerStep;

    // Update the ruler data in OpenGL:
    _isNewControlsDataPending = true;
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::calculateDataLines
// Description: Uses the bars data to determine which line each bar will go to
//              and how tall each series will be
// Author:      Uri Shomroni
// Date:        24/2/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::calculateDataLines()
{
    int numberOfSeries = (int)_barsDataBySeries.size();
    int numberOfBackgroundSections = (int)_backgroundSectionsData.size();

    // Sanity check:
    if (numberOfSeries != numberOfBackgroundSections)
    {
        GT_ASSERT(numberOfSeries == numberOfBackgroundSections);
        numberOfSeries = min(numberOfSeries, numberOfBackgroundSections);
    }

    // Used for finding an empty spot:
    int firstFreeLine = 0;
    gtVector<acGanttChartDataType> lineLowestValue;
    gtVector<acGanttChartDataType> lineHighestValue;

    int longestSeriesLabelLength = 0;

    // Iterate the series:
    for (int i = 0; i < numberOfSeries; i++)
    {
        // Pointer sanity check:
        gtVector<acGanttChartBarData>* pCurrentSeriesVec = _barsDataBySeries[i];
        GT_IF_WITH_ASSERT(pCurrentSeriesVec != NULL)
        {
            gtVector<acGanttChartBarData>& currentSeriesVec = *pCurrentSeriesVec;
            acGanttChartBackgroundSectionData& currentSeriesBackgroundSection = _backgroundSectionsData[i];

            // Ignore series with no bars:
            int numberOfBarsInSeries = (int)currentSeriesVec.size();

            if (numberOfBarsInSeries > 0)
            {
                // Each series gets a new line:
                int currentSeriesBaseLine = firstFreeLine;
                currentSeriesBackgroundSection._firstLine = currentSeriesBaseLine;

                // Iterate the bars in the series:
                for (int j = 0; j < numberOfBarsInSeries; j++)
                {
                    // Get the bar Data:
                    acGanttChartBarData& currentBar = currentSeriesVec[j];
                    const acGanttChartDataType& barStart = currentBar._barStart;
                    const acGanttChartDataType& barEnd = currentBar._barEnd;
                    bool needsNewLine = true;

                    // See if we can find a place to put this bar in an existing line:
                    for (int l = currentSeriesBaseLine; l < firstFreeLine; l++)
                    {
                        // If this is before or after all an existing line's data (remember that barEnd > barStart):
                        if (barEnd <= lineLowestValue[l])
                        {
                            lineLowestValue[l] = barStart;
                            currentBar._barLine = l;
                            needsNewLine = false;
                            break;
                        }

                        if (barStart >= lineHighestValue[l])
                        {
                            lineHighestValue[l] = barEnd;
                            currentBar._barLine = l;
                            needsNewLine = false;
                            break;
                        }
                    }

                    // If all lines are occupied (or this is the first bar), add a new one, containing just this bar for now:
                    if (needsNewLine)
                    {
                        currentBar._barLine = firstFreeLine;
                        lineLowestValue.push_back(barStart);
                        lineHighestValue.push_back(barEnd);
                        firstFreeLine++;
                    }
                }

                // Mark the last line used as the section's last line:
                currentSeriesBackgroundSection._lastLine = firstFreeLine - 1;

                // Get the series label length:
                int currentSeriesLabelLength = currentSeriesBackgroundSection._label.length();

                if (currentSeriesLabelLength > longestSeriesLabelLength)
                {
                    // Change the maximum:
                    longestSeriesLabelLength = currentSeriesLabelLength;
                }
            }
        }
    }

    // Note how many lines we have in total:
    _numberOfChartLines = firstFreeLine;

    // Calculate the width of the labels area:
    if (longestSeriesLabelLength > 0)
    {
        // Add a space equal to two characters
        longestSeriesLabelLength += 2;

        // A label of n characters also needs two margins and n-1 spaces:
        _seriesLabelsWidth = (_characterOpenGLWidth * (acOpenGLChartPositionDataType)longestSeriesLabelLength) + (_characterSpacingOpenGLWidth * (acOpenGLChartPositionDataType)(longestSeriesLabelLength - 1)) + (_characterMarginOpenGLWidth * 2.0f);
    }
    else // longestSeriesLabelLength <= 0
    {
        _seriesLabelsWidth = 0.0f;
    }

    // Calculate _minValue and _maxValue:
    calculateDataLimits(lineLowestValue, lineHighestValue);
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::calculateDataLimits
// Description: Modifies _minValue and _maxValue to match the line starts and
//              ends in lineLowestValues and lineHighestValues
// Author:      Uri Shomroni
// Date:        24/2/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::calculateDataLimits(const gtVector<acGanttChartDataType>& lineLowestValues, const gtVector<acGanttChartDataType>& lineHighestValues)
{
    // Calculate the data range:
    int lowestEdges = (int)lineLowestValues.size();
    int highestEdges = (int)lineHighestValues.size();

    if (lowestEdges < highestEdges)
    {
        GT_ASSERT(lowestEdges == highestEdges);
        lowestEdges = min(lowestEdges, highestEdges);
    }

    // Calculate the gantt's minimum and maximum values:
    for (int k = 0; k < lowestEdges; k++)
    {
        if (k == 0)
        {
            // The basic values are the first line's value:
            _minValue = lineLowestValues[k];
            _maxValue = lineHighestValues[k];
        }
        else
        {
            // Update the min value:
            const acGanttChartDataType& currentLineStart = lineLowestValues[k];

            if (currentLineStart < _minValue)
            {
                _minValue = currentLineStart;
            }

            // Update the max value:
            const acGanttChartDataType& currentLineEnd = lineHighestValues[k];

            if (currentLineEnd > _maxValue)
            {
                _maxValue = currentLineEnd;
            }
        }
    }

    // Make sure we got okay values:
    GT_ASSERT(_minValue <= _maxValue);

    // Get the viewport size:
    acGanttChartDataType viewportWidth = valueOfViewPortWidth();

    if (_leftValueShown < _minValue)
    {
        // If the _leftValueShown is too small, reset it to the left edge of the range:
        _leftValueShown = _minValue;
    }
    else if (_leftValueShown + viewportWidth > _maxValue)
    {
        // If the entire range is smaller than the viewport, snap left, otherwise, snap right:
        if ((_maxValue - _minValue) > viewportWidth)
        {
            _leftValueShown = _maxValue - viewportWidth;
        }
        else // ((_maxValue - _minValue) <= viewportWidth)
        {
            _leftValueShown = _minValue;
        }
    }

    // Use the new min and max values to update the ruler step and string:
    updateRuler();
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::chartHasData
// Description: Returns true iff the chart has any data
// Author:      Uri Shomroni
// Date:        15/3/2010
// ---------------------------------------------------------------------------
bool acGanttChartWindow::chartHasData() const
{
    bool retVal = false;

    // Go over all the series:
    int numberOfSeries = (int)_barsDataBySeries.size();

    for (int i = 0; i < numberOfSeries; i++)
    {
        // Pointer sanity check:
        const gtVector<acGanttChartBarData>* pCurrentSeries = _barsDataBySeries[i];
        GT_IF_WITH_ASSERT(pCurrentSeries != NULL)
        {
            // If this series has at least one bar:
            if (pCurrentSeries->size() > 0)
            {
                // There is data in the chart, no need to look further:
                retVal = true;
                break;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::screenWidthValue
// Description: Returns the value of the screen width in data units (considers
//              the zoom level)
// Author:      Uri Shomroni
// Date:        11/4/2010
// ---------------------------------------------------------------------------
acGanttChartDataType acGanttChartWindow::screenWidthValue() const
{
    // Note that using wxSystemSettings::GetMetric(wxSYS_SCREEN_#) gives the entire screen and not the inside of the taskbar.
    acOpenGLChartPositionDataType screenWidthInOpenGLCoords = (acOpenGLChartPositionDataType)(wxSystemSettings::GetMetric(wxSYS_SCREEN_X)) / AC_OPENGL_CHART_WINDOW_PIXEL_SIZE_OF_1_X_UNIT;
    acGanttChartDataType retVal = (acGanttChartDataType)(screenWidthInOpenGLCoords / (_horizontalScale * AC_GANTT_HORIZONTAL_SCALE_MULTIPLIER));

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::lineTop
// Description: Calculates the Y coordinate of the top of a line in the chart,
//              with or without spacing
// Author:      Uri Shomroni
// Date:        23/2/2010
// ---------------------------------------------------------------------------
acOpenGLChartPositionDataType acGanttChartWindow::lineTop(int lineIndex, bool withSpacing) const
{
    acOpenGLChartPositionDataType retVal = AC_GANTT_LINE_TOP_OF_FIRST_LINE - ((acOpenGLChartPositionDataType)lineIndex * AC_GANTT_LINE_HEIGHT);

    if (withSpacing)
    {
        retVal -= AC_GANTT_LINE_SPACING;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        lineBottom
// Description: Calculates the Y coordinate of the bottom of a line in the chart,
//              with or without spacing
// Author:      Uri Shomroni
// Date:        23/2/2010
// ---------------------------------------------------------------------------
acOpenGLChartPositionDataType acGanttChartWindow::lineBottom(int lineIndex, bool withSpacing) const
{
    acOpenGLChartPositionDataType retVal = AC_GANTT_LINE_TOP_OF_FIRST_LINE - ((acOpenGLChartPositionDataType)lineIndex * AC_GANTT_LINE_HEIGHT) - AC_GANTT_LINE_HEIGHT;

    if (withSpacing)
    {
        retVal += AC_GANTT_LINE_SPACING;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::horizontalPosition
// Description: Translates a data value to an X coordinate.
// Author:      Uri Shomroni
// Date:        23/2/2010
// ---------------------------------------------------------------------------
acOpenGLChartPositionDataType acGanttChartWindow::horizontalPosition(acGanttChartDataType dataValue, bool allowOutOfRange) const
{
    acOpenGLChartPositionDataType retVal = 0.0f;

    GT_IF_WITH_ASSERT(((dataValue >= _minValue) && (dataValue <= _maxValue)) || allowOutOfRange)
    {
        // Calculate as an offset from the center, then multiply by the scale:
        double offsetFromCenter = (double)dataValue - (((double)_minValue + (double)_maxValue) / 2.0);
        retVal = (acOpenGLChartPositionDataType)(offsetFromCenter * _horizontalScale * AC_GANTT_HORIZONTAL_SCALE_MULTIPLIER);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::rulerVerticalPosition
// Description: Returns the OpenGL vertical position for the ruler (transformed
//              ignoring the y change in moveCamera()
// Author:      Uri Shomroni
// Date:        16/3/2010
// ---------------------------------------------------------------------------
acOpenGLChartPositionDataType acGanttChartWindow::rulerVerticalPosition() const
{
    acOpenGLChartPositionDataType retVal = 0.0f;

    // The base position is the top of the window, so we need to move the ruler down
    // the length of the screen:
    int windowHeight = GetClientSize().GetHeight();

    if (windowHeight > 0)
    {
        retVal = (-1.0f * (acOpenGLChartPositionDataType)windowHeight) / AC_OPENGL_CHART_WINDOW_PIXEL_SIZE_OF_1_Y_UNIT;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::valueOfViewPortWidth
// Description: Returns the viewport's width in data units (depending on the
//              zoom level and window size)
// Author:      Uri Shomroni
// Date:        28/2/2010
// ---------------------------------------------------------------------------
acGanttChartDataType acGanttChartWindow::valueOfViewPortWidth() const
{
    int viewportWidth = GetClientSize().GetWidth();
    acOpenGLChartPositionDataType viewPortOpenGLWidth = (acOpenGLChartPositionDataType)((double)viewportWidth / AC_OPENGL_CHART_WINDOW_PIXEL_SIZE_OF_1_X_UNIT);

    acGanttChartDataType retVal = (acGanttChartDataType)((double)viewPortOpenGLWidth / (_horizontalScale * AC_GANTT_HORIZONTAL_SCALE_MULTIPLIER));

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::rulerLength
// Description: Returns the ruler length in data units
// Author:      Uri Shomroni
// Date:        16/3/2010
// ---------------------------------------------------------------------------
acGanttChartDataType acGanttChartWindow::rulerLength() const
{
    acGanttChartDataType retVal = (_maxValue > _minValue) ? (_maxValue - _minValue) : 0;

    // Make sure the ruler will be visible no matter the window size.
    acGanttChartDataType scrWidthValue = screenWidthValue();

    if (retVal < scrWidthValue)
    {
        retVal = scrWidthValue;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::labelLengthInsideGanttBar
// Description: Returns the maximal number of characters, including spacing,
//              that can fit inside a bar with a length of barWidthValue
// Author:      Uri Shomroni
// Date:        2/3/2010
// ---------------------------------------------------------------------------
unsigned int acGanttChartWindow::labelLengthInsideGanttBar(const acGanttChartBarData& barData) const
{
    // Translate the bar ends to OpenGL units in the current zoom level and get the difference:
    acOpenGLChartPositionDataType barOpenGLWidth = horizontalPosition(barData._barEnd) - horizontalPosition(barData._barStart);

    // Use the parent class utility function to get the length:
    unsigned int retVal = labelLengthInsideBar(barOpenGLWidth, barData._label);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        acGanttChartWindow::moveCamera
// Description: Moves the camera to the position given by _topLineShown and _leftValueShown
// Author:      Uri Shomroni
// Date:        24/2/2010
// ---------------------------------------------------------------------------
void acGanttChartWindow::moveCamera(bool moveH, bool moveV)
{
    acOpenGLChartPositionDataType dx = -horizontalPosition(_leftValueShown);
    acOpenGLChartPositionDataType dy = lineTop(0, false) - lineTop(_topLineShown, false) - AC_GANTT_LINE_TOP_OF_FIRST_LINE;

    // If we do not want to move along one of the axes, reset the value now:
    if (moveH)
    {
        // Shift the X value by the labels' width:
        dx += _seriesLabelsWidth;
    }
    else // !moveH
    {
        dx = 0.0f;
    }

    if (!moveV)
    {
        dy = 0.0f;
    }

    // Make the top left point in the window stay the same when scaling:
    wxSize windowSize = GetClientSize();
    int windowWidth = windowSize.GetWidth();
    int windowHeight = windowSize.GetHeight();
    dx -= (((acOpenGLChartPositionDataType)windowWidth / 2.0f) / AC_OPENGL_CHART_WINDOW_PIXEL_SIZE_OF_1_X_UNIT);
    dy += (((acOpenGLChartPositionDataType)windowHeight / 2.0f) / AC_OPENGL_CHART_WINDOW_PIXEL_SIZE_OF_1_Y_UNIT);


    glTranslatef(dx, dy, 0.0f);
}

