//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStatisticsView.cpp
///
//==================================================================================

//------------------------------ gdStatisticsView.cpp ------------------------------

#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <QTextBrowser>

#include <AMDTApplicationComponents/Include/acChartWindow.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTAPIClasses/Include/apBreakPoint.h>
#include <AMDTAPIClasses/Include/apGenericBreakpoint.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdDeprecationStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/gdStatisticsPanel.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdAPICallsHistoryView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdBatchStatisticsView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdDeprecationStatisticsView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdFunctionCallsStatisticsView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStateChangeStatisticsView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStatisticsView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdTotalStatisticsView.h>


// Constants for the dialog layout:

#define GD_STATISTICS_VIEWER_NOTEBOOK_MIN_HEIGHT 150
#define GD_STATISTICS_VIEWER_NOTEBOOK_MIN_WIDTH 300

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::gdStatisticsView
// Description: Constructor
// Arguments:   pParent - The statistics viewer's parent
//              viewerTitle - The title of the statistics viewer.
// Author:      Sigal Algranaty
// Date:        22/7/2008
// ---------------------------------------------------------------------------
gdStatisticsView::gdStatisticsView(afProgressBarWrapper* pProgressBar, QWidget* pParent, gdStatisticsPanel* pParentStatisticsPanel, bool isAnalyzeModeEnabled)
    : QTabWidget(pParent),
      _pProgressBar(pProgressBar), _pChartWindow(NULL), _pParentStatisticsPanel(pParentStatisticsPanel),
      _bottomPanelProp(1), _leftPanelProp(1), _activeContext(AP_OPENGL_CONTEXT, 0), _previousDisplayedContext(AP_OPENGL_CONTEXT, -1), _pStatistics(NULL),
      _lastSelectedNoteBookPane(GD_STATISTICS_VIEW_TOTAL_INDEX),
      _isAnalyzeModeEnabled(isAnalyzeModeEnabled), _ignoreListSelectionEvents(false), _forceChartUpdate(false)
{
    // Set my min size:
    resize(GD_STATISTICS_VIEWER_NOTEBOOK_MIN_WIDTH, GD_STATISTICS_VIEWER_NOTEBOOK_MIN_HEIGHT);

    // Allocate new statistics object:
    _pStatistics = new apStatistics();


    // Initialize pages display flags:
    for (int i = 0 ; i <= GD_STATISTICS_VIEW_LAST_VIEWER_INDEX; i++)
    {
        _shouldDisplayPages[i] = true;
        _pages[i] = NULL;
    }

    // Initialize pages captions:
    m_pagesCaptions[GD_STATISTICS_VIEW_TOTAL_INDEX] = GD_STR_StatisticsViewerTotalStatisticsCaption;
    m_pagesCaptions[GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX] = GD_STR_StatisticsViewerFunctionCallsStatisticsCaption;
    m_pagesCaptions[GD_STATISTICS_VIEW_DEPRECATION_INDEX] = GD_STR_StatisticsViewerDeprecatedFunctionCallsStatisticsCaption;
    m_pagesCaptions[GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX] = GD_STR_StatisticsViewerFunctionCallsHistoryCaption;
    m_pagesCaptions[GD_STATISTICS_VIEW_BATCH_INDEX] = GD_STR_StatisticsViewerBatchStatisticsCaption;

}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::~gdStatisticsView
// Description: Destructor
// Author:      Eran Zinman
// Date:        22/5/2007
// ---------------------------------------------------------------------------
gdStatisticsView::~gdStatisticsView()
{
    // Delete the statistics pointer:
    delete _pStatistics;
    _pStatistics = NULL;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::clearAllViewers
// Description: Clears all the viewers from statistics details
// Author:      Sigal Algranaty
// Date:        22/7/2008
// ---------------------------------------------------------------------------
void gdStatisticsView::clearAllViewers()
{
    // Clear each of the viewers:
    for (int i = 0; i <= GD_STATISTICS_VIEW_LAST_VIEWER_INDEX; i++)
    {
        if (_pages[i] != NULL)
        {
            _pages[i]->clearAllStatisticsItems();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::initializeColorVectors
// Description: Initializes the color vectors' values:
// Author:      Uri Shomroni
// Date:        29/7/2008
// ---------------------------------------------------------------------------
void gdStatisticsView::initializeColorVectors()
{
    unsigned long currentColor;

    // Generate colors for the functions calls graphs:
    GT_IF_WITH_ASSERT((_pages[GD_STATISTICS_VIEW_TOTAL_INDEX] != NULL) &&
                      (_pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX] != NULL) &&
                      (_pages[GD_STATISTICS_VIEW_DEPRECATION_INDEX] != NULL))
    {
        currentColor = 0x6666FF; // Blue-Purple
        _pages[GD_STATISTICS_VIEW_TOTAL_INDEX]->addChartColor(currentColor);
        _pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX]->addChartColor(currentColor);
        _pages[GD_STATISTICS_VIEW_DEPRECATION_INDEX]->addChartColor(currentColor);
        currentColor = 0x0066FF; // Middle Blue
        _pages[GD_STATISTICS_VIEW_TOTAL_INDEX]->addChartColor(currentColor);
        _pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX]->addChartColor(currentColor);
        _pages[GD_STATISTICS_VIEW_DEPRECATION_INDEX]->addChartColor(currentColor);
        currentColor = 0x33CCFF; // Sea Blue
        _pages[GD_STATISTICS_VIEW_TOTAL_INDEX]->addChartColor(currentColor);
        _pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX]->addChartColor(currentColor);
        _pages[GD_STATISTICS_VIEW_DEPRECATION_INDEX]->addChartColor(currentColor);
        currentColor = 0x99CCFF; // Light Blue
        _pages[GD_STATISTICS_VIEW_TOTAL_INDEX]->addChartColor(currentColor);
        _pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX]->addChartColor(currentColor);
        _pages[GD_STATISTICS_VIEW_DEPRECATION_INDEX]->addChartColor(currentColor);
        currentColor = 0x99CC33; // Olive Green
        _pages[GD_STATISTICS_VIEW_TOTAL_INDEX]->addChartColor(currentColor);
        _pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX]->addChartColor(currentColor);
        _pages[GD_STATISTICS_VIEW_DEPRECATION_INDEX]->addChartColor(currentColor);
        currentColor = 0x008800; // Dark Green
        _pages[GD_STATISTICS_VIEW_TOTAL_INDEX]->addChartColor(currentColor);
        _pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX]->addChartColor(currentColor);
        _pages[GD_STATISTICS_VIEW_DEPRECATION_INDEX]->addChartColor(currentColor);
        currentColor = 0x00BB00; // Middle green
        _pages[GD_STATISTICS_VIEW_TOTAL_INDEX]->addChartColor(currentColor);
        _pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX]->addChartColor(currentColor);
        _pages[GD_STATISTICS_VIEW_DEPRECATION_INDEX]->addChartColor(currentColor);
        currentColor = 0x99FF00; // Bright green
        _pages[GD_STATISTICS_VIEW_TOTAL_INDEX]->addChartColor(currentColor);
        _pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX]->addChartColor(currentColor);
        _pages[GD_STATISTICS_VIEW_DEPRECATION_INDEX]->addChartColor(currentColor);

        // The last color is the highlight color, which isn't used on functions that aren't selected:
        currentColor = 0xFFFFFF; // White
        _pages[GD_STATISTICS_VIEW_TOTAL_INDEX]->addChartColor(currentColor);
        _pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX]->addChartColor(currentColor);
        _pages[GD_STATISTICS_VIEW_DEPRECATION_INDEX]->addChartColor(currentColor);
    }


    GT_IF_WITH_ASSERT(_pages[GD_STATISTICS_VIEW_BATCH_INDEX] != NULL)
    {
        // Colors for the batch statistics graph:
        currentColor = 0xFF6633; // Red warning
        _pages[GD_STATISTICS_VIEW_BATCH_INDEX]->addChartColor(currentColor);
        currentColor = 0xFF8833; // Red
        _pages[GD_STATISTICS_VIEW_BATCH_INDEX]->addChartColor(currentColor);
        currentColor = 0xFFAA33; // Red
        _pages[GD_STATISTICS_VIEW_BATCH_INDEX]->addChartColor(currentColor);
        currentColor = 0xFFCC33; // Orange warning
        _pages[GD_STATISTICS_VIEW_BATCH_INDEX]->addChartColor(currentColor);
        currentColor = 0xFFDD44; // Orange
        _pages[GD_STATISTICS_VIEW_BATCH_INDEX]->addChartColor(currentColor);
        currentColor = 0xFFEE55; // Orange
        _pages[GD_STATISTICS_VIEW_BATCH_INDEX]->addChartColor(currentColor);
        currentColor = 0xFFFF66; // Yellow warning
        _pages[GD_STATISTICS_VIEW_BATCH_INDEX]->addChartColor(currentColor);
        currentColor = 0xDDEE44; // Yellow
        _pages[GD_STATISTICS_VIEW_BATCH_INDEX]->addChartColor(currentColor);
        currentColor = 0xBBDD22; // Yellow
        _pages[GD_STATISTICS_VIEW_BATCH_INDEX]->addChartColor(currentColor);
        currentColor = 0x99CC00; // Draw step green
        _pages[GD_STATISTICS_VIEW_BATCH_INDEX]->addChartColor(currentColor);
        currentColor = 0x66CC00; // Frame step green
        _pages[GD_STATISTICS_VIEW_BATCH_INDEX]->addChartColor(currentColor);
        currentColor = 0x33BB00; // Green
        _pages[GD_STATISTICS_VIEW_BATCH_INDEX]->addChartColor(currentColor);
        currentColor = 0x00BB00; // Go Green
        _pages[GD_STATISTICS_VIEW_BATCH_INDEX]->addChartColor(currentColor);
    }

}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::updateStatisticsViewers
// Description: Update the statistics viewers
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/7/2008
// ---------------------------------------------------------------------------
bool gdStatisticsView::updateStatisticsViewers()
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((_pStatistics != NULL) && (_pParentStatisticsPanel != NULL))
    {
        // Hide / Show panels according to context type:
        showPagesAccordingToContextType();

        // Ignore list selection events while updating:
        _ignoreListSelectionEvents = true;

        // Get the updated statistics from the API:
        retVal = gaGetCurrentStatistics(_activeContext, _pStatistics);
        GT_IF_WITH_ASSERT(retVal)
        {
            // Update al pages:
            for (int i = 0; i <= GD_STATISTICS_VIEW_LAST_VIEWER_INDEX; i++)
            {
                if (_pages[i] != NULL)
                {
                    // Set the viewer's active context:
                    _pages[i]->setActiveContext(_activeContext);

                    // Set the debug process suspension flag:
                    _pages[i]->setIsSuspended(gaIsDebuggedProcessSuspended());

                    // Update the function statistics:
                    bool rc = _pages[i]->updateFunctionCallsStatisticsList(*_pStatistics);
                    GT_ASSERT(rc);

                    // Refresh the page:
                    _pages[i]->update();
                }
                else
                {
                    GT_IF_WITH_ASSERT(i == GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX)
                    {
                        GT_IF_WITH_ASSERT(_pCallHistoryView != NULL)
                        {
                            // Set the viewer's active context:
                            _pCallHistoryView->setActiveContext(_activeContext);

                            // Set the debug process suspension flag:
                            _pCallHistoryView->setIsSuspended(gaIsDebuggedProcessSuspended());

                            // Update the function statistics:
                            _pCallHistoryView->updateFunctionCallsStatisticsList(*_pStatistics);
                        }
                    }
                }
            }
        }

        // Continue handling list selection events:
        _ignoreListSelectionEvents = false;

        // Update the chart and properties with the selected view:
        _pParentStatisticsPanel->updateChartAndPropertiesViews(_lastSelectedNoteBookPane);
    }

    update();
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::applyPageOnClick
// Description: updates the chart to reflect the data in the last selected notebook page
//              whose index is currentPageIndex. If currentPageIndex is wxID_ANY, we
//              simply refresh the current page without changing anything.
// Author:      Uri Shomroni
// Date:        31/7/2008
// ---------------------------------------------------------------------------
void gdStatisticsView::applyPageOnClick(gdStatisticsViewIndex windowIndex)
{
    // If we sent unknown id, we actually want to refresh the views:
    if (windowIndex == GD_STATISTICS_VIEW_UNKNOWN)
    {
        windowIndex = _lastSelectedNoteBookPane;
        _lastSelectedNoteBookPane = GD_STATISTICS_VIEW_UNKNOWN;
    }

    _forceChartUpdate = true;

    switch (windowIndex)
    {
        case GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX:
        {
            onFunctionStatisticsClick(NULL);
        }
        break;

        case GD_STATISTICS_VIEW_TOTAL_INDEX:
        {
            onTotalStatisticsClick(NULL);
        }
        break;

        case GD_STATISTICS_VIEW_DEPRECATION_INDEX:
        {
            onDeprecationStatsClick(NULL);
        }
        break;

        case GD_STATISTICS_VIEW_BATCH_INDEX:
        {
            onBatchStatsClick(NULL);
        }
        break;

        case GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX:
        {
            onCallsHistoryClick(QModelIndex());
        }
        break;

        default:
        {
            // This could only happen if we added a new page type and didn't include it here
            GT_ASSERT(false);
        }
        break;
    }

}

// ---------------------------------------------------------------------------
// Name:       gdStatisticsView::getViewerById
// Description:
// Arguments:  windowIndex - the requested page index
// Return Val: gdStatisticsViewBase*
// Author:      Sigal Algranaty
// Date:        24/6/2009
// ---------------------------------------------------------------------------
gdStatisticsViewBase* gdStatisticsView::statisticsPage(gdStatisticsViewIndex windowIndex)
{
    gdStatisticsViewBase* pRetVal = NULL;

    switch (windowIndex)
    {
        case GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX:
            pRetVal = _pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX];
            break;

        case GD_STATISTICS_VIEW_TOTAL_INDEX:
            pRetVal = _pages[GD_STATISTICS_VIEW_TOTAL_INDEX];
            break;

        case GD_STATISTICS_VIEW_DEPRECATION_INDEX:
            pRetVal = _pages[GD_STATISTICS_VIEW_DEPRECATION_INDEX];
            break;

        case GD_STATISTICS_VIEW_BATCH_INDEX:
            pRetVal = _pages[GD_STATISTICS_VIEW_BATCH_INDEX];
            break;

        case GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX:
            pRetVal = _pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX];
            break;

        default:
        {
            pRetVal = NULL;
            break;
        }
        break;
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::createStatisticsPages
// Description: Creates the statistics viewer pages
// Author:      Sigal Algranaty
// Date:        14/7/2008
// ---------------------------------------------------------------------------
void gdStatisticsView::createStatisticsPages()
{
    // Set my minimal size:
    resize(GD_STATISTICS_VIEWER_NOTEBOOK_MIN_WIDTH, GD_STATISTICS_VIEWER_NOTEBOOK_MIN_HEIGHT);

    // Create the total statistics tab:
    _pages[GD_STATISTICS_VIEW_TOTAL_INDEX] = new gdTotalStatisticsView(this);


    // Create the function calls statistics tab:
    _pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX] = new gdFunctionCallsStatisticsView(this);


    // Create the deprecation statistics tab:
    _pages[GD_STATISTICS_VIEW_DEPRECATION_INDEX] = new gdDeprecationStatisticsView(this);


    // Create the calls history statistics tab:
    _pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX] = NULL;
    _pCallHistoryView = new gdAPICallsHistoryView(_pProgressBar, this, false, false);


    // Create the batch statistics tab:
    _pages[GD_STATISTICS_VIEW_BATCH_INDEX] = new gdBatchStatisticsView(this);

    // Create the total statistics tab:
    gdTotalStatisticsView* pTotalView = (gdTotalStatisticsView*)_pages[GD_STATISTICS_VIEW_TOTAL_INDEX];
    insertTab(GD_STATISTICS_VIEW_TOTAL_INDEX, pTotalView, m_pagesCaptions[GD_STATISTICS_VIEW_TOTAL_INDEX]);

    gdFunctionCallsStatisticsView* pFunctionCallsView = (gdFunctionCallsStatisticsView*)_pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX];
    insertTab(GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX, pFunctionCallsView, m_pagesCaptions[GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX]);

    gdDeprecationStatisticsView* pDeprecationView = (gdDeprecationStatisticsView*)_pages[GD_STATISTICS_VIEW_DEPRECATION_INDEX];
    insertTab(GD_STATISTICS_VIEW_DEPRECATION_INDEX, pDeprecationView, m_pagesCaptions[GD_STATISTICS_VIEW_DEPRECATION_INDEX]);

    insertTab(GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX, _pCallHistoryView, m_pagesCaptions[GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX]);

    gdBatchStatisticsView* pBatchView = (gdBatchStatisticsView*)_pages[GD_STATISTICS_VIEW_BATCH_INDEX];
    insertTab(GD_STATISTICS_VIEW_BATCH_INDEX, pBatchView, m_pagesCaptions[GD_STATISTICS_VIEW_BATCH_INDEX]);

    // Connect signals to slots:
    bool rc = connectViewToHandlers();
    GT_ASSERT(rc);

    // Refresh the tab view:
    QWidget* pWidget = (QWidget*)(_pages[0]);
    setCurrentWidget(pWidget);

    update();
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::setFrameLayout
// Description: Create the view layout.
// Arguments:   wxSize& toolbarSize - output - the toolbar size
//              wxSize& chartSize - output - the chart size
// Author:      Sigal Algranaty
// Date:        18/10/2010
// ---------------------------------------------------------------------------
void gdStatisticsView::setFrameLayout(const QSize& viewSize)
{
    // Create the statistics pages layout:
    createStatisticsPages();

    // Set my min size:
    resize(viewSize.width(), viewSize.height());

    // Initialize the chart color vectors:
    initializeColorVectors();
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::onUpdateShowDetailedBatchData
// Description: Is called when the UI wants to update the view detailed batch data
// Arguments: wxUpdateUIEvent& eve
// Return Val: void
// Author:      Sigal Algranaty
// Date:        19/5/2009
// ---------------------------------------------------------------------------
void gdStatisticsView::onUpdateShowDetailedBatchData(bool& isEnabled, bool& isChecked)
{
    // Check if the active view is the batch statistics view:
    bool isBatchStatisticsShown = (_lastSelectedNoteBookPane == GD_STATISTICS_VIEW_BATCH_INDEX);
    isChecked = false;

    if (isBatchStatisticsShown)
    {
        GT_IF_WITH_ASSERT(_pages[GD_STATISTICS_VIEW_BATCH_INDEX] != NULL)
        {
            // Down cast the page to a batch statistics viewer:
            gdBatchStatisticsView* pBatchViewer = (gdBatchStatisticsView*)_pages[GD_STATISTICS_VIEW_BATCH_INDEX];
            GT_IF_WITH_ASSERT(pBatchViewer != NULL)
            {
                isChecked = pBatchViewer->shouldShowDetailedData();
            }
        }
    }

    isEnabled = isBatchStatisticsShown;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::onStateChangeStatsClick
// Description: Is called when the state change statistics list is clicked
// Author:      Uri Shomroni
// Date:        29/7/2008
// ---------------------------------------------------------------------------
void gdStatisticsView::onStateChangeStatsClick(QTableWidgetItem* pClickedItem)
{
    (void)(pClickedItem);  // unused
    /*if (!_ignoreListSelectionEvents)
    {
        // Down cast:
        gdStateChangeStatisticsView* pStateChangeStatisticsViewer = (gdStateChangeStatisticsView*)_pages[GD_STATISTICS_VIEW_STATE_CHANGE_INDEX];

        int clickedIndex = 0;
        if (pClickedItem != NULL)
        {
            // Get the clicked row index:
            clickedIndex = pClickedItem->row();
        }

        // Linux calls a few of these events with an m_itemindex of -1 while sorting.
        // We want to ignore them.
        if ((clickedIndex != -1) && (pStateChangeStatisticsViewer != NULL))
        {
            bool isNewData = (_lastSelectedNoteBookPane != GD_STATISTICS_VIEW_STATE_CHANGE_INDEX) || _forceChartUpdate;

            int effectiveColorsAmount = pStateChangeStatisticsViewer->amountOfChartColors();
            int itemsAmount = pStateChangeStatisticsViewer->rowCount() - 1;
            _pChartWindow->setChartType(AC_PIE_CHART);
            if (_pChartWindow != NULL)
            {
                gtString functionHtmlString;
                if (isNewData)
                {
                    _pChartWindow->clearAllData();
                    _pChartWindow->setHighlightColor(QColor(0, 0, 0, 125));
                }
                else
                {
                    // Just clear the selection status:
                    _pChartWindow->clearAllSelection();
                }

                // Get selected item index:
                int selectedItemIndex = pStateChangeStatisticsViewer->getSelectedItemIndex();

                // Check that we have colors and information
                if ((effectiveColorsAmount > 0) && (itemsAmount > 0))
                {
                    // These will be used when displaying the properties:
                    gtString propertiesTitle = GD_STR_TotalStatisticsViewerStateChangeFunctions;
                    gtString propertiesComment = GD_STR_HtmlPropertiesRedundantDefaultComment;

                    // This will hold each pie chart data point (slice)
                    acChartDataPoint dataPoint;

                    int j = 0;
                    // Generate a set of pie pieces for effective calls instead of just one block:
                    for (int i = 0; i < itemsAmount; i++)
                    {
                        // Check if this item is selected:
                        bool isSelected = pStateChangeStatisticsViewer->isItemSelected(i);

                        // Set the item selection:
                        dataPoint._isSelected = isSelected;

                        // Set the item tooltip:
                        dataPoint._tooltip = pStateChangeStatisticsViewer->getItemTooltip(i);

                        gtUInt64 effectiveCalls = 0;
                        gtUInt64 redundantCalls = 0;
                        long redundantColor = -1;
                        long effectiveColor = -1;
                        bool rc = pStateChangeStatisticsViewer->getItemProperties(i, effectiveCalls, redundantCalls, effectiveColor, redundantColor);
                        GT_IF_WITH_ASSERT(rc)
                        {
                            dataPoint._value = effectiveCalls;
                            dataPoint._pointColor = effectiveColor;

                            if (effectiveCalls > 0)
                            {
                                dataPoint._originalItemIndex = i;
                                if (isNewData)
                                {
                                    _pChartWindow->addDataPoint(dataPoint);
                                }
                                else
                                {
                                    _pChartWindow->setSelection(j++, dataPoint._isSelected);
                                }
                            }
                        }
                    }

                    for (int i = 0; i < itemsAmount; i++)
                    {
                        // Check if this item is selected:
                        bool isSelected = pStateChangeStatisticsViewer->isItemSelected(i);

                        // Set the item selection:
                        dataPoint._isSelected = isSelected;

                        gtUInt64 effectiveCalls = 0;
                        gtUInt64 redundantCalls = 0;
                        long redundantColor = -1;
                        long effectiveColor = -1;
                        bool rc = pStateChangeStatisticsViewer->getItemProperties(i, effectiveCalls, redundantCalls, effectiveColor, redundantColor);
                        GT_IF_WITH_ASSERT(rc)
                        {
                            dataPoint._value = redundantCalls;
                            dataPoint._pointColor = redundantColor;

                            if (redundantCalls > 0)
                            {
                                dataPoint._originalItemIndex = i;
                                if (isNewData)
                                {
                                    _pChartWindow->addDataPoint(dataPoint);
                                }
                                else
                                {
                                    _pChartWindow->setSelection(j++, dataPoint._isSelected);
                                }
                            }
                        }
                    }

                    // Check if item is selected (and ignore the last item (total):
                    if ((selectedItemIndex >= 0) && (selectedItemIndex < itemsAmount ))
                    {
                        // Build an HTML string for this state change function:
                        // Get the selected function name:
                        gtString selectedFunctionName;
                        bool rc1 = pStateChangeStatisticsViewer->getSelectedFunctionName(selectedFunctionName);

                        // Get the selected function icon:
                        afIconType selectedFunctionIconType;
                        bool rc2 = pStateChangeStatisticsViewer->getItemIconType(selectedItemIndex, selectedFunctionIconType);

                        gdHTMLProperties htmlBuilder;
                        htmlBuilder.buildStateChangeFunctionHTMLPropertiesString(selectedFunctionName, selectedFunctionIconType, functionHtmlString);

                        GT_ASSERT(rc1 && rc2);
                    }
                }

                GT_IF_WITH_ASSERT(_pParentStatisticsPanel != NULL)
                {
                    // clear the properties window:
                    _pParentStatisticsPanel->displayPropertiesWindowMessage(AF_STR_Empty, functionHtmlString);
                }

                if (isNewData)
                {
                    _pChartWindow->recalculateArrays();
                }

                _pChartWindow->redrawWindow();

                // The window should be refreshed, so that the chart is repainted:
                update();

                // Mark that the chart drawn belongs to this window:
                _lastSelectedNoteBookPane = GD_STATISTICS_VIEW_STATE_CHANGE_INDEX;
            }
        }
    }*/
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::onDeprecationStatsClick
// Description: Is called when the deprecation statistics list is clicked
// Author:      Sigal Algranaty
// Date:        25/2/2009
// ---------------------------------------------------------------------------
void gdStatisticsView::onDeprecationStatsClick(QTableWidgetItem* pClickedItem)
{
    if (!_ignoreListSelectionEvents)
    {
        gdDeprecationStatisticsView* pDeprecationStatisticsViewer = (gdDeprecationStatisticsView*)_pages[GD_STATISTICS_VIEW_DEPRECATION_INDEX];

        // Get the clicked index:
        int clickedIndex = 0;

        if (pClickedItem != NULL)
        {
            // Get the clicked row index:
            clickedIndex = pClickedItem->row();
        }

        // Linux calls a few of these events with an m_itemindex of -1 while sorting.
        // We want to ignore them.
        if ((clickedIndex != -1) && (pDeprecationStatisticsViewer != NULL))
        {
            // Update the chart:
            updateViewerChart(GD_STATISTICS_VIEW_DEPRECATION_INDEX, pDeprecationStatisticsViewer, 2, false, AC_PIE_CHART);

            // Update properties view:
            int amountOfDeprecatedItems = pDeprecationStatisticsViewer->rowCount() - 1;
            gtString htmlPropertiesStr;

            // Get the selected item index:
            int selectedItemIndex = pDeprecationStatisticsViewer->getSelectedItemIndex();

            // Get debugged execution process mode:
            apExecutionMode executionMode = AP_DEBUGGING_MODE;
            bool rc = gaGetDebuggedProcessExecutionMode(executionMode);
            GT_ASSERT(rc);

            // If an item is selected:
            if ((selectedItemIndex < amountOfDeprecatedItems) && (selectedItemIndex > -1))
            {
                _pChartWindow->setSelection(selectedItemIndex, true);
                _pChartWindow->redrawWindow();
                // Get the selected function name:
                gtString fnName;
                rc = pDeprecationStatisticsViewer->getSelectedFunctionName(fnName);
                GT_ASSERT(rc);

                // Get the selected function deprecation version, and remove version:
                apAPIVersion deprecatedAtVersion = AP_GL_VERSION_NONE;
                apAPIVersion removedAtVersion = AP_GL_VERSION_NONE;
                rc = pDeprecationStatisticsViewer->getItemDeprecationVersions(selectedItemIndex, deprecatedAtVersion, removedAtVersion);
                GT_ASSERT(rc);

                apFunctionDeprecationStatus functionDeprecationStatus;
                rc = pDeprecationStatisticsViewer->getItemDeprecationStatus(selectedItemIndex, functionDeprecationStatus);
                GT_ASSERT(rc);

                int functionId;
                rc = pDeprecationStatisticsViewer->getItemFunctionId(selectedItemIndex, functionId);
                GT_ASSERT(rc);

                gdHTMLProperties htmlBuilder;
                htmlBuilder.buildDeprecatedFunctionHTMLPropertiesString(functionId, fnName, functionDeprecationStatus, deprecatedAtVersion, removedAtVersion, htmlPropertiesStr);
            }
            // If we clicked the "More..." item:
            else if ((selectedItemIndex == amountOfDeprecatedItems) && (executionMode != AP_ANALYZE_MODE) && _isAnalyzeModeEnabled)
            {
                htmlPropertiesStr.append(GD_STR_DeprecationFunctionCallMoreMessage);
            }
            else
            {
                htmlPropertiesStr.makeEmpty();
            }

            // Display the properties message:
            GT_IF_WITH_ASSERT(_pParentStatisticsPanel != NULL)
            {
                // clear the properties window:
                _pParentStatisticsPanel->displayPropertiesWindowMessage(AF_STR_Empty, htmlPropertiesStr);
            }
        }
    }
}




// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::onBatchStatsClick
// Description: Is called when the batch statistics list is clicked
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gdStatisticsView::onBatchStatsClick(QTableWidgetItem* pClickedItem)
{
    if (!_ignoreListSelectionEvents)
    {
        // Down cast the batch statistics viewer:
        gdBatchStatisticsView* pBatchStatisticsViewer = (gdBatchStatisticsView*)_pages[GD_STATISTICS_VIEW_BATCH_INDEX];

        if (NULL != pBatchStatisticsViewer)
        {
            // Get the clicked item index:
            int clickedIndex = 0;

            if (pClickedItem != NULL)
            {
                // Get the clicked row index:
                clickedIndex = pClickedItem->row();
            }

            // Linux calls a few of these events with an m_itemindex of -1 while sorting.
            // We want to ignore them.
            // Get the view selected index:
            if ((clickedIndex >= 0) && (pBatchStatisticsViewer != NULL))
            {
                // Get the batch view selected index:
                int selectedItemIndex = pBatchStatisticsViewer->getSelectedItemIndex();

                // Update the chart:
                updateViewerChart(GD_STATISTICS_VIEW_BATCH_INDEX, pBatchStatisticsViewer, 1, true, AC_BAR_CHART);

                // Build the properties string for this statistics item:
                gtString htmlPropertiesStr;

                if ((selectedItemIndex >= 0) && (selectedItemIndex < pBatchStatisticsViewer->rowCount() - 1))
                {
                    if (NULL != _pChartWindow)
                    {
                        _pChartWindow->setSelection(selectedItemIndex, true);
                    }

                    // Get the information for the selected statistics item:
                    int minAmountOfVertices = 0, maxAmountOfVertices = 0;
                    float verticesPercentage = 0, batchesPercentage = 0;
                    afIconType iconType = AF_ICON_NONE;
                    pBatchStatisticsViewer->getItemInformation(selectedItemIndex, minAmountOfVertices, maxAmountOfVertices, verticesPercentage, batchesPercentage, iconType);


                    gdHTMLProperties htmlBuilder;
                    htmlBuilder.buildBatchStatisticsPropertiesString(minAmountOfVertices, maxAmountOfVertices, verticesPercentage, batchesPercentage, iconType, htmlPropertiesStr);
                }

                // Display the properties message:
                GT_IF_WITH_ASSERT(_pParentStatisticsPanel != NULL)
                {
                    // clear the properties window:
                    _pParentStatisticsPanel->displayPropertiesWindowMessage(AF_STR_Empty, htmlPropertiesStr);
                }
            }
            else
            {
                GT_IF_WITH_ASSERT(_pParentStatisticsPanel != NULL)
                {
                    // clear the properties window:
                    _pParentStatisticsPanel->displayPropertiesWindowMessage();
                }

            }
        }

        // Mark that the chart drawn belongs to this window:
        _lastSelectedNoteBookPane = GD_STATISTICS_VIEW_BATCH_INDEX;

        if (NULL != _pChartWindow)
        {
            _pChartWindow->recalculateArrays();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::updateViewerChart
// Description: Fills in the chart for the requested viewer
// Arguments: wxWindowID viewerId
//            gdStatisticsViewBase* pViewer
//            int sampledDataIndex
//            bool useColorsBeforeSort - should we use the original colors when the items were inserted to chart (before sort)?
// Return Val: void
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gdStatisticsView::updateViewerChart(gdStatisticsViewIndex windowIndex, gdStatisticsViewBase* pViewer, int sampledDataIndex, bool useColorsBeforeSort, acChartType chartType)
{
    if (_pChartWindow != NULL)
    {
        // Check if data should be refreshed:
        bool isNewData = (_lastSelectedNoteBookPane != windowIndex) || _forceChartUpdate;
        _pChartWindow->setChartType(chartType);

        if (isNewData)
        {
            // Clear the data in the chart:
            _pChartWindow->clearAllData();
            _pChartWindow->setHighlightColor(QColor(0, 0, 0, 125));
        }
        else
        {
            // Just clear the selection status:
            _pChartWindow->clearAllSelection();
        }

        // Get amount of colors:
        int amountOfColors = pViewer->amountOfChartColors();
        GT_ASSERT(amountOfColors > 0);

        // Get amount of items from viewer:
        int amountOfViewerItems = pViewer->rowCount() - 1;

        if ((amountOfColors > 0) && (amountOfViewerItems > 0))
        {
            // Counts the real added items:
            int amountOfNonZeroItems = -1;

            // Loop through the items and add to chart:
            for (int currentItemIndex = 0; currentItemIndex < amountOfViewerItems; currentItemIndex++)
            {
                // Add this item to chart:
                bool rc = addItemToChart(pViewer, isNewData, amountOfNonZeroItems, useColorsBeforeSort, currentItemIndex, sampledDataIndex, false);
                GT_ASSERT(rc);
            }
        }

        // Recalculate the chart:
        if (isNewData)
        {
            if (chartType == AC_BAR_CHART)
            {
                // Get the total data value, to give the chart reference value:
                unsigned long totalItemValue = 1;
                int totalItemIndex = pViewer->rowCount() - 2;

                if (totalItemIndex >= 0)
                {
                    // Get the text in the requested column:
                    QString qstr;
                    pViewer->getItemText(totalItemIndex, sampledDataIndex, qstr);
                    gtString totalItemAsString;
                    totalItemAsString.fromASCIIString(qstr.toLatin1());
                    bool rcNum = totalItemAsString.toUnsignedLongNumber(totalItemValue);
                    GT_ASSERT(rcNum);
                }

                // Set the custom max value so that bars representing 50% of all function calls
                // will look 50% high and so on:
                _pChartWindow->setReferenceValue(totalItemValue);
            }

            _pChartWindow->recalculateArrays();
        }

        // Redraw:
        _pChartWindow->redrawWindow();

        // Set the viewer id as the last one selected:
        _lastSelectedNoteBookPane = windowIndex;
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::addItemToChart
// Description: Add a single item to the chart
// Arguments: wxListItem& listItem
//            const gdStatisticsViewBase* pViewer - the viewer to add the item to chart from
//            bool isNewData
//            int& amountOfNonZeroItems
//            bool useColorsBeforeSort - should we use the original colors (before list was sorted)
//            int itemIndex
//            int sampledDataIndex
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        20/5/2009
// ---------------------------------------------------------------------------
bool gdStatisticsView::addItemToChart(gdStatisticsViewBase* pViewer, bool isNewData, int& amountOfNonZeroItems, bool useColorsBeforeSort, int itemIndex, int sampledDataIndex, bool isSelected)
{
    bool retVal = true;

    // Get amount of colors:
    int amountOfColors = pViewer->amountOfChartColors();
    GT_ASSERT(amountOfColors > 0);

    // This will hold each pie chart data point (slice | bar):
    acChartDataPoint dataPoint;

    // Get the value of the column in line currentItemIndex (ie # the sampled data column value):

    QString qstr;
    pViewer->getItemText(itemIndex, sampledDataIndex, qstr);
    gtString sampledDataAsString;
    sampledDataAsString.fromASCIIString(qstr.toLatin1());

    // Translate the string to integer:
    unsigned int sampledDataValue = 0;
    sampledDataAsString.toUnsignedIntNumber(sampledDataValue);

    // Set the data point value:
    dataPoint._value = sampledDataValue;

    // Add the items with positive value:
    if (sampledDataValue > 0)
    {
        // Increase the amount of items actually added:
        amountOfNonZeroItems ++;

        // Get the color of the inserted item:
        unsigned long color = 0;
        bool rc = pViewer->getItemChartColor(itemIndex, amountOfNonZeroItems, useColorsBeforeSort, color);
        GT_ASSERT(rc);

        // Set the data point selection status:
        dataPoint._isSelected = isSelected;

        // Set the data point color:
        dataPoint._pointColor = color;

        // Set the item tooltip:
        dataPoint._tooltip = pViewer->getItemTooltip(itemIndex);

        if (isNewData)
        {
            dataPoint._originalItemIndex = itemIndex;

            // Add the data to the chard:
            _pChartWindow->addDataPoint(dataPoint);
        }
        else
        {
            // Set the chart selection:
            _pChartWindow->setSelection(amountOfNonZeroItems, dataPoint._isSelected);
        }
    }

    return retVal;

}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::onCallsHistoryClick
// Description: Is called when the calls history list is clicked
// Author:      Uri Shomroni
// Date:        7/8/2008
// ---------------------------------------------------------------------------
void gdStatisticsView::onCallsHistoryClick(const QModelIndex& index)
{
    (void)(index);  // unused

    // wxMac throws selection events when we select programmatically, so make sure this view is shown:
    if (_pChartWindow != NULL)
    {
        // Clear the chart:
        _pChartWindow->clearAllData();
        _pChartWindow->setChartType(AC_NO_CHART);
        _pChartWindow->setHighlightColor(QColor(0, 0, 0));
        _pChartWindow->recalculateArrays();
        _pChartWindow->redrawWindow();

        if (_pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX] != NULL)
        {
            // Get the calls history selected item index:
            int selectedFunctionCallIndex = _pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX]->getSelectedItemIndex();

            // Get amount of function calls:
            int functionsAmount = _pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX]->rowCount();

            // If we don't have focus, set the focus to the last item:
            if (selectedFunctionCallIndex == -1)
            {
                selectedFunctionCallIndex = functionsAmount - 1;
            }

            gtString propertiesTitle;
            gtString propertiesContent;

            // If there is a valid selected item:
            if ((selectedFunctionCallIndex > -1) && (selectedFunctionCallIndex < functionsAmount))
            {
                // Get the function calls history item:
                afIconType iconType = AF_ICON_NONE;
                gdStatisticsViewBase::gdStatisticsViewItemData* pItemData = _pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX]->getItemData(selectedFunctionCallIndex);
                GT_IF_WITH_ASSERT(pItemData != NULL)
                {
                    iconType = pItemData->_iconType;
                    propertiesTitle = pItemData->_functionName;
                }

                // Build the HTML string for the properties content:
                gdHTMLProperties htmlPropsBuilder;
                apExecutionMode executionMode = AP_DEBUGGING_MODE;
                bool rc = gaGetDebuggedProcessExecutionMode(executionMode);
                GT_ASSERT(rc);
                htmlPropsBuilder.buildFunctionCallHTMLPropertiesString(executionMode, _activeContext, selectedFunctionCallIndex, iconType, propertiesTitle, propertiesContent);
            }

            // Display the properties message in the properties window:
            // TO_DO: deprecation model: get the real second icon
            GT_IF_WITH_ASSERT(_pParentStatisticsPanel != NULL)
            {
                // clear the properties window:
                _pParentStatisticsPanel->displayPropertiesWindowMessage(AF_STR_Empty, propertiesContent);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::onFunctionStatisticsClick
// Description: Is called when the calls statistics list is clicked
// Author:      Uri Shomroni
// Date:        29/7/2008
// ---------------------------------------------------------------------------
void gdStatisticsView::onFunctionStatisticsClick(QTableWidgetItem* pClickedItem)
{
    if (!_ignoreListSelectionEvents)
    {
        // Down cast the function calls statistics viewer:
        gdFunctionCallsStatisticsView* pFunctionCallStatisticsViewer = (gdFunctionCallsStatisticsView*)_pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX];

        int clickedIndex = 0;

        if (pClickedItem != NULL)
        {
            // Get the clicked row index:
            clickedIndex = pClickedItem->row();
        }

        // Linux calls a few of these events with an m_itemindex of -1 while sorting.
        // We want to ignore them.
        if ((clickedIndex != -1) && (pFunctionCallStatisticsViewer != NULL))
        {
            // Update the chart:
            updateViewerChart(GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX, pFunctionCallStatisticsViewer, 2, false, AC_PIE_CHART);

            int selectedItemIndex = pFunctionCallStatisticsViewer->getSelectedItemIndex();
            int amountOfItems = pFunctionCallStatisticsViewer->rowCount() - 1;

            // Get selected function name:
            gtString functionName;
            bool rc1 = pFunctionCallStatisticsViewer->getSelectedFunctionName(functionName);

            GT_ASSERT(rc1 || (selectedItemIndex == -1) || (selectedItemIndex == amountOfItems));

            // Build a properties string to this function:
            gdHTMLProperties htmlBuilder;
            gtString functionHTMLProperties;

            // Get the selected item icon type:
            afIconType iconType = AF_ICON_NONE;

            if ((selectedItemIndex >= 0) && (selectedItemIndex < amountOfItems))
            {
                _pChartWindow->setSelection(selectedItemIndex, true);
                _pChartWindow->redrawWindow();
                bool rc2 = pFunctionCallStatisticsViewer->getItemIconType(selectedItemIndex, iconType);
                GT_ASSERT(rc2);

                htmlBuilder.buildFunctionHTMLPropertiesString(_activeContext, functionName, iconType, functionHTMLProperties);
            }

            // Show the relevant properties or clear the properties window:
            GT_IF_WITH_ASSERT(_pParentStatisticsPanel != NULL)
            {
                // clear the properties window:
                _pParentStatisticsPanel->displayPropertiesWindowMessage(AF_STR_Empty, functionHTMLProperties);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::onTotalStatisticsClick
// Description: Is called when the total statistics list is clicked
// Author:      Uri Shomroni
// Date:        30/7/2008
// ---------------------------------------------------------------------------
void gdStatisticsView::onTotalStatisticsClick(QTableWidgetItem* pClickedItem)
{
    if (!_ignoreListSelectionEvents)
    {
        // Down cast the total statistics viewer:
        gdTotalStatisticsView* pTotalStatisticsViewer = (gdTotalStatisticsView*)_pages[GD_STATISTICS_VIEW_TOTAL_INDEX];

        int clickedIndex = 0;

        if (pClickedItem != NULL)
        {
            // Get the clicked row index:
            clickedIndex = pClickedItem->row();
        }

        if ((clickedIndex != -1) && (pTotalStatisticsViewer != NULL) && (_pChartWindow != NULL))
        {
            bool isNewData = (_lastSelectedNoteBookPane != GD_STATISTICS_VIEW_TOTAL_INDEX) || _forceChartUpdate;

            // Get total view items amount:
            int itemsAmount = pTotalStatisticsViewer->rowCount() - 1;
            _pChartWindow->setChartType(AC_BAR_CHART);

            if (isNewData)
            {
                _pChartWindow->clearAllData();
                _pChartWindow->setHighlightColor(QColor(0, 0, 0, 125));
            }
            else
            {
                // Just clear the selection status:
                _pChartWindow->clearAllSelection();
            }

            // Check that we have colors and information
            if (itemsAmount > 0)
            {
                int j = 0;

                // Get the view selected index:
                int selectedItemIndex = pTotalStatisticsViewer->getSelectedItemIndex();

                acChartDataPoint dataPoint;
                int amountOfCurrentItemsForColorSelection = 0;

                for (int i = 0; i < itemsAmount; i++)
                {
                    // Get the item chart color:
                    unsigned long color = 0;
                    bool rc1 = pTotalStatisticsViewer->getItemChartColor(i, amountOfCurrentItemsForColorSelection, false, color);
                    GT_IF_WITH_ASSERT(rc1)
                    {
                        // Get the item number of calls:
                        gtUInt64 numberOfCalls = 0;
                        bool isItemAvailable = false;
                        bool rc2 = pTotalStatisticsViewer->getItemNumOfCalls(i, numberOfCalls, isItemAvailable);
                        GT_IF_WITH_ASSERT(rc2)
                        {
                            // Add only the available items:
                            if (isItemAvailable)
                            {
                                // Set the data point attributes:
                                dataPoint._pointColor = color;
                                dataPoint._value = numberOfCalls;
                                dataPoint._isSelected = pTotalStatisticsViewer->isItemSelected(i);
                                dataPoint._tooltip = pTotalStatisticsViewer->getItemTooltip(i);

                                if (isNewData)
                                {
                                    dataPoint._originalItemIndex = i;

                                    // Add the current bar to the chart control
                                    _pChartWindow->addDataPoint(dataPoint);
                                }
                                else
                                {
                                    _pChartWindow->setSelection(j++, dataPoint._isSelected);
                                }
                            }
                        }
                    }
                }

                if (isNewData)
                {
                    QString qstr;
                    pTotalStatisticsViewer->getItemText(itemsAmount, 2, qstr);
                    gtString totalItemAsString;
                    totalItemAsString.fromASCIIString(qstr.toLatin1());
                    unsigned long totalFuncs = 0;
                    bool rcNum = totalItemAsString.toUnsignedLongNumber(totalFuncs);
                    GT_ASSERT(rcNum);

                    // Set the custom max value so that bars representing 50% of all function calls
                    // will look 50% high and so on:
                    _pChartWindow->setReferenceValue(totalFuncs);
                }

                bool isLastItem = ((selectedItemIndex >= itemsAmount) || (selectedItemIndex < 0));

                // Get the information for the selected item:
                gdFuncCallsViewTypes functionType;

                if (!isLastItem)
                {
                    bool rc = pTotalStatisticsViewer->getItemFunctionType(selectedItemIndex, functionType);
                    GT_ASSERT(rc);
                }

                // Build an HTML string for the function type description:
                gdHTMLProperties htmlPropertiesBuilder;
                gtString htmlPropertiesString;
                htmlPropertiesBuilder.buildTotalStatisticsPropertiesString(_activeContext, functionType, isLastItem, htmlPropertiesString);

                // Display the HTML properties message:
                GT_IF_WITH_ASSERT(_pParentStatisticsPanel != NULL)
                {
                    // clear the properties window:
                    _pParentStatisticsPanel->displayPropertiesWindowMessage(AF_STR_Empty, htmlPropertiesString);
                }
            }

            if (isNewData)
            {
                _pChartWindow->recalculateArrays();
            }

            _pChartWindow->redrawWindow();

            // Mark that the chart drawn belongs to this window:
            _lastSelectedNoteBookPane = GD_STATISTICS_VIEW_TOTAL_INDEX;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::onPageChange
// Description: Is called after the tab page changes
// Author:      Uri Shomroni
// Date:        30/7/2008
// ---------------------------------------------------------------------------
void gdStatisticsView::onPageChange(int newPage)
{
    // Get the page:
    QWidget* pSelectedPage = widget(newPage);
    GT_IF_WITH_ASSERT(pSelectedPage != NULL)
    {
        if (newPage != GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX)
        {
            // Down cast:
            gdStatisticsViewBase* pBase = qobject_cast<gdStatisticsViewBase*>(pSelectedPage);
            GT_IF_WITH_ASSERT(pBase != NULL)
            {
                // Get the window id of the selected page:
                gdStatisticsViewIndex selectedWindowIndex = pBase->windowIndex();

                // Sanity check
                GT_IF_WITH_ASSERT(_pParentStatisticsPanel != NULL)
                {
                    _pParentStatisticsPanel->updateChartAndPropertiesViews(selectedWindowIndex);
                }
            }
        }
        else
        {
            GT_IF_WITH_ASSERT(_pCallHistoryView != NULL)
            {
                // Perform click on history view:
                onCallsHistoryClick(QModelIndex());
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::onChartItemClicked
// Description: Handles the event of a user clicking on a chart item
// Arguments:   chartItemIndex - the clicked item index
// Author:      Sigal Algranaty
// Date:        24/6/2009
// ---------------------------------------------------------------------------
void gdStatisticsView::onChartItemClicked(int chartItemIndex)
{
    if (chartItemIndex >= 0)
    {
        // Get the current selected viewer:
        gdStatisticsViewBase* pViewer = statisticsPage(_lastSelectedNoteBookPane);
        GT_IF_WITH_ASSERT(pViewer != NULL)
        {
            // Clear the list selection:
            pViewer->clearSelection();

            // Select the chart clicked item and make sure it is visible:
            pViewer->ensureRowVisible(chartItemIndex, true);

            // NOTICE: Even though we set the selected item, since the user clicks the mouse within another window's area,
            // we can't count on the automatic selection events to work correctly, cause even if we do trust it, there comes
            // some new platform that makes a new problem :) (Sigal 7.7.09)

            // Build the properties string for this statistics item:
            gtString htmlPropertiesStr;

            if ((chartItemIndex >= 0) && (chartItemIndex < pViewer->rowCount() - 1))
            {
                // Build the properties string to batch/total viewer:
                // Build an HTML string for the function type description:
                gdHTMLProperties htmlPropBuilder;

                if (_lastSelectedNoteBookPane == GD_STATISTICS_VIEW_TOTAL_INDEX)
                {
                    gdFuncCallsViewTypes functionType;

                    // Down cast the total statistics viewer:
                    gdTotalStatisticsView* pTotalStatisticsViewer = (gdTotalStatisticsView*)_pages[GD_STATISTICS_VIEW_TOTAL_INDEX];

                    GT_IF_WITH_ASSERT(pTotalStatisticsViewer != NULL)
                    {
                        bool rc = pTotalStatisticsViewer->getItemFunctionType(chartItemIndex, functionType);
                        GT_ASSERT(rc);
                    }
                    htmlPropBuilder.buildTotalStatisticsPropertiesString(_activeContext, functionType, false, htmlPropertiesStr);
                }

                else if (_lastSelectedNoteBookPane == GD_STATISTICS_VIEW_BATCH_INDEX)
                {
                    // Down cast the total statistics viewer:
                    gdBatchStatisticsView* pBatchStatisticsViewer = (gdBatchStatisticsView*)_pages[GD_STATISTICS_VIEW_BATCH_INDEX];

                    // Get the information for the selected statistics item:
                    int minAmountOfVertices = 0, maxAmountOfVertices = 0;
                    float verticesPercentage = 0, batchesPercentage = 0;
                    afIconType iconType = AF_ICON_NONE;
                    GT_IF_WITH_ASSERT(pBatchStatisticsViewer != NULL)
                    {
                        pBatchStatisticsViewer->getItemInformation(chartItemIndex, minAmountOfVertices, maxAmountOfVertices, verticesPercentage, batchesPercentage, iconType);
                    }

                    htmlPropBuilder.buildBatchStatisticsPropertiesString(minAmountOfVertices, maxAmountOfVertices, verticesPercentage, batchesPercentage, iconType, htmlPropertiesStr);
                }
                else if (_lastSelectedNoteBookPane == GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX)
                {
                    // Down cast the total statistics viewer:
                    gdFunctionCallsStatisticsView* pFunctionCallsStatisticsViewer = (gdFunctionCallsStatisticsView*)_pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX];
                    GT_IF_WITH_ASSERT(pFunctionCallsStatisticsViewer != NULL)
                    {
                        gtString functionName;
                        pFunctionCallsStatisticsViewer->getSelectedFunctionName(functionName);
                        int selectedItemIndex = pFunctionCallsStatisticsViewer->getSelectedItemIndex();
                        afIconType iconType;
                        pFunctionCallsStatisticsViewer->getItemIconType(selectedItemIndex, iconType);
                        htmlPropBuilder.buildFunctionHTMLPropertiesString(_activeContext, functionName, iconType, htmlPropertiesStr);
                    }
                }
                else if (_lastSelectedNoteBookPane == GD_STATISTICS_VIEW_DEPRECATION_INDEX)
                {
                    // Down cast the total statistics viewer:
                    gdDeprecationStatisticsView* pDeprecationStatisticsViewer = (gdDeprecationStatisticsView*)_pages[GD_STATISTICS_VIEW_DEPRECATION_INDEX];
                    GT_IF_WITH_ASSERT(pDeprecationStatisticsViewer != NULL)
                    {
                        // Get the selected function name:
                        gtString fnName;
                        bool rc = pDeprecationStatisticsViewer->getSelectedFunctionName(fnName);
                        GT_ASSERT(rc);

                        // Get the selected function deprecation version, and remove version:
                        apAPIVersion deprecatedAtVersion = AP_GL_VERSION_NONE;
                        apAPIVersion removedAtVersion = AP_GL_VERSION_NONE;
                        rc = pDeprecationStatisticsViewer->getItemDeprecationVersions(chartItemIndex, deprecatedAtVersion, removedAtVersion);
                        GT_ASSERT(rc);

                        apFunctionDeprecationStatus functionDeprecationStatus;
                        rc = pDeprecationStatisticsViewer->getItemDeprecationStatus(chartItemIndex, functionDeprecationStatus);
                        GT_ASSERT(rc);

                        int functionId;
                        rc = pDeprecationStatisticsViewer->getItemFunctionId(chartItemIndex, functionId);
                        GT_ASSERT(rc);

                        gdHTMLProperties htmlBuilder;
                        htmlBuilder.buildDeprecatedFunctionHTMLPropertiesString(functionId, fnName, functionDeprecationStatus, deprecatedAtVersion, removedAtVersion, htmlPropertiesStr);
                    }
                }
                else
                {
                    GT_ASSERT(false);
                }
            }

            // Display the properties message:
            GT_IF_WITH_ASSERT(_pParentStatisticsPanel != NULL)
            {
                // clear the properties window:
                _pParentStatisticsPanel->displayPropertiesWindowMessage(AF_STR_Empty, htmlPropertiesStr);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::showPagesAccordingToContextType
// Description: Hide / Show pages according to OpenCL / OpenGL context type
// Author:      Sigal Algranaty
// Date:        25/3/2010
// ---------------------------------------------------------------------------
void gdStatisticsView::showPagesAccordingToContextType()
{
    // Check if the currently displayed context is a NULL context:
    bool isNullContext = _activeContext.isDefault();

    // Check if the current displayed context is an OpenCL context:
    bool isCLContext = _activeContext.isOpenCLContext();

    // Check if the displayed pages should be updated:
    bool shouldUpdate = ((_activeContext._contextType != _previousDisplayedContext._contextType) || (!_previousDisplayedContext.isValid()));

    if (shouldUpdate)
    {
        // Vector of booleans containing true iff the page i is currently displayed:
        bool shouldDisplayPages[GD_STATISTICS_VIEW_LAST_VIEWER_INDEX + 1];

        // Initialize the display flag with true:
        for (int i = 0; i <= GD_STATISTICS_VIEW_LAST_VIEWER_INDEX; i++)
        {
            shouldDisplayPages[i] = true;
        }

        // Update OpenCL context:
        if (isCLContext || isNullContext)
        {
            shouldDisplayPages[GD_STATISTICS_VIEW_BATCH_INDEX] = false;
        }

        // Get the currently selected page:
        int currentlyDisplayedPage = currentIndex();

        // Check if the currently page exist in the new pages layout:
        int newDisplayedPageIndex = currentlyDisplayedPage;

        if (shouldDisplayPages[newDisplayedPageIndex] == false)
        {
            newDisplayedPageIndex = 0;
        }

        // Update the new displayed page index with the future remove / add pages:
        for (int i = 0; i <= GD_STATISTICS_VIEW_LAST_VIEWER_INDEX ; i++)
        {
            // If this page would be removed, update the selected page index:
            if (_shouldDisplayPages[i] && !shouldDisplayPages[i])
            {
                if (newDisplayedPageIndex >= i)
                {
                    newDisplayedPageIndex --;
                }
            }

            // If this page would be added later, update the selected page index:
            if (!_shouldDisplayPages[i] && shouldDisplayPages[i])
            {
                // Update the selected page with the new page added:
                if (i <= newDisplayedPageIndex)
                {
                    newDisplayedPageIndex ++;
                }
            }
        }

        // If the new displayed page index should not be displayed, set it to 0:
        if ((newDisplayedPageIndex < 0) || (newDisplayedPageIndex > GD_STATISTICS_VIEW_LAST_VIEWER_INDEX))
        {
            newDisplayedPageIndex = 0;
        }

        // If the currently selected page should not be displayed, select page 0 (to enable the page remove):
        if (shouldDisplayPages[currentlyDisplayedPage] == false)
        {
            // If selected page should be removed, set the notebook selection to the first page (which is displayed always):
            setCurrentWidget(0);
        }

        // Disable pages that are not supposed to be shown:
        int pagesCount = count();

        for (int i = 0; i < pagesCount; i++)
        {
            // Get the window from the notebook:
            QWidget* pWin = widget(i);

            if (pWin != NULL)
            {
                if (_shouldDisplayPages[i] && !shouldDisplayPages[i])
                {
                    // Hide the window:
                    pWin->hide();

                    // Disable the page:
                    setTabEnabled(i, false);
                }
                else
                {
                    setTabEnabled(i, true);

                    // Show the page:
                    pWin->show();
                }
            }
        }

        // Add the pages that were hidden, and should be displayed:
        for (int i = 0; i <= GD_STATISTICS_VIEW_LAST_VIEWER_INDEX; i++)
        {
            // Update the page status:
            _shouldDisplayPages[i] = shouldDisplayPages[i];
        }

        // Set the notebook selection:
        setCurrentIndex(newDisplayedPageIndex);
    }

    // Set the previously displayed context:
    _previousDisplayedContext = _activeContext;
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::windowIDToTabViewerID
// Description: Translate a window id to a tab viewer index
// Arguments:   int cmdId
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        31/3/2010
// ---------------------------------------------------------------------------
int gdStatisticsView::windowIDToTabViewerID(int cmdId)
{
    int retVal = -1;

    switch (cmdId)
    {
        case ID_STATISTICS_VIEWER_VIEW_SHOW_CALLS_HISTORY:
            retVal = GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX;
            break;

        case ID_STATISTICS_VIEWER_VIEW_SHOW_CALLS_STATISTICS:
            retVal = GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX;
            break;

        case ID_STATISTICS_VIEWER_VIEW_SHOW_DEPRECATION_STATISTICS:
            retVal = GD_STATISTICS_VIEW_DEPRECATION_INDEX;
            break;

        case ID_STATISTICS_VIEWER_VIEW_SHOW_TOTAL_STATISTICS:
            retVal = GD_STATISTICS_VIEW_TOTAL_INDEX;
            break;

        case ID_STATISTICS_VIEWER_VIEW_SHOW_BATCH_STATISTICS:
            retVal = GD_STATISTICS_VIEW_BATCH_INDEX;
            break;

        default:
        {
            // This shouldn't happen
            GT_ASSERT(false);
        }
        break;
    }

    return retVal;

}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::onDetailedBatchDataButton
// Description: Is called when the user clicks the "Detailed batch data" button
// Author:      Sigal Algranaty
// Date:        10/10/2011
// ---------------------------------------------------------------------------
void gdStatisticsView::onDetailedBatchDataButton()
{
    // Down cast the batch statistics viewer:
    gdBatchStatisticsView* pBatchStatisticsViewer = (gdBatchStatisticsView*)_pages[GD_STATISTICS_VIEW_BATCH_INDEX];
    GT_IF_WITH_ASSERT((pBatchStatisticsViewer != NULL) && (_pStatistics != NULL))
    {
        // Check if the tool is checked:
        bool showDetailedData = pBatchStatisticsViewer->shouldShowDetailedData();

        // Toggle:
        showDetailedData = !showDetailedData;

        // Set the 'Show detailed data' flag:
        pBatchStatisticsViewer->setShowDetailedData(showDetailedData);

        // Update the viewer:
        pBatchStatisticsViewer->updateFunctionCallsStatisticsList(*_pStatistics);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::onUpdateDetailedBatchDataButton
// Description: Is called when wxWindows wants to update the status of the
//              "Open Recorded File" command button.
// Author:      Avi Shapira
// Date:        23/3/2005
// ---------------------------------------------------------------------------
void gdStatisticsView::onUpdateDetailedBatchDataButton(bool& isEnabled)
{
    // Tool is enabled when batch view is selected:
    if (_lastSelectedNoteBookPane == GD_STATISTICS_VIEW_BATCH_INDEX)
    {
        // Down cast the batch statistics viewer:
        gdBatchStatisticsView* pBatchStatisticsViewer = (gdBatchStatisticsView*)_pages[GD_STATISTICS_VIEW_BATCH_INDEX];
        GT_IF_WITH_ASSERT(pBatchStatisticsViewer != NULL)
        {
            // Check if there is enough statistics to show details:
            isEnabled = pBatchStatisticsViewer->showDetailedDataEnabled();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::exportStatistics
// Description: Export a statistics page content to a CSV file
// Arguments:   int pageIndex - the requested page index
//               - the event handle
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        18/10/2010
// ---------------------------------------------------------------------------
void gdStatisticsView::exportStatistics(int pageIndex)
{
    // Call the relevant page callback:
    GT_IF_WITH_ASSERT(_pages[pageIndex] != NULL)
    {
        _pages[pageIndex]->onSaveStatisticsData();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::pageItemCount
// Description: Return the items count for the requested page
// Arguments:   int pageIndex
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        18/10/2010
// ---------------------------------------------------------------------------
int gdStatisticsView::pageItemCount(int pageIndex)
{
    int retVal = 0;

    GT_IF_WITH_ASSERT(_pages[pageIndex] != NULL)
    {
        retVal = _pages[pageIndex]->rowCount();
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::selectedPage
// Description: Return the currently selected page index
// Return Val:  gdStatisticsPageIndex
// Author:      Sigal Algranaty
// Date:        18/10/2010
// ---------------------------------------------------------------------------
int gdStatisticsView::selectedPageIndex()
{
    int retVal = GD_STATISTICS_VIEW_TOTAL_INDEX;

    // Get the notebook selected page:
    size_t selectedPageIndex = currentIndex();

    retVal = (int)selectedPageIndex;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::setSelectedPageFromGlobalIndex
// Description: Set selection from global index (page enumeration)
// Arguments:   gdStatisticsPageIndex selectedPageIndex
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        18/10/2010
// ---------------------------------------------------------------------------
void gdStatisticsView::setSelectedPageFromGlobalIndex(int selectedPageIndex)
{
    // Get the chosen window index:
    int chosenWindow = windowIDToTabViewerID(selectedPageIndex);

    // Set the selected viewer:
    GT_IF_WITH_ASSERT((chosenWindow >= 0) && (chosenWindow <= GD_STATISTICS_VIEW_LAST_VIEWER_INDEX))
    {
        setCurrentIndex(chosenWindow);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::setSelectedPage
// Description:
// Arguments:   gdStatisticsPageIndex selectedPageIndex
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        18/10/2010
// ---------------------------------------------------------------------------
void gdStatisticsView::setSelectedPage(int selectedPageIndex)
{
    setCurrentIndex(selectedPageIndex);
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::selectedPage
// Description: Return the notebook selected page
// Return Val:  gdStatisticsViewBase*
// Author:      Sigal Algranaty
// Date:        18/10/2010
// ---------------------------------------------------------------------------
gdStatisticsViewBase* gdStatisticsView::selectedPage()
{
    gdStatisticsViewBase* pRetVal = NULL;

    // Get the notebook selection tab:
    int currentlySelectedTab = selectedPageIndex();

    // Return the selected page:
    GT_IF_WITH_ASSERT((currentlySelectedTab >= 0) && (currentlySelectedTab <= GD_STATISTICS_VIEW_LAST_VIEWER_INDEX))
    {
        if (_pages[currentlySelectedTab] != NULL)
        {
            pRetVal = _pages[currentlySelectedTab];
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::onUpdateEdit_Copy
// Description: enables the copy command in the VS edit menu
// Arguments:   bool &isEnabled
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        6/2/2011
// ---------------------------------------------------------------------------
void gdStatisticsView::onUpdateEdit_Copy(bool& isEnabled)
{
    // Get the notebook selected page:
    gdStatisticsViewBase* pSelectedPage = selectedPage();

    isEnabled = false;

    if (pSelectedPage != NULL)
    {
        pSelectedPage->onUpdateCopy(isEnabled);
    }
    else if (selectedPageIndex() == GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX)
    {
        GT_IF_WITH_ASSERT(_pCallHistoryView != NULL)
        {
            _pCallHistoryView->onUpdateEditCopy(isEnabled);
        }
    }

}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::onUpdateEdit_SelectAll
// Description: enables the select all command in the VS edit menu
// Arguments:   bool &isEnabled
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        6/2/2011
// ---------------------------------------------------------------------------
void gdStatisticsView::onUpdateEdit_SelectAll(bool& isEnabled)
{
    // Get the notebook selected page:
    gdStatisticsViewBase* pSelectedPage = selectedPage();

    isEnabled = false;

    if (pSelectedPage != NULL)
    {
        pSelectedPage->onUpdateSelectAll(isEnabled);
    }
    else if (selectedPageIndex() == GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX)
    {
        GT_IF_WITH_ASSERT(_pCallHistoryView != NULL)
        {
            isEnabled = false;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::onEdit_Copy
// Description: execute the copy command in the VS edit menu
// Arguments:
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        6/2/2011
// ---------------------------------------------------------------------------
void gdStatisticsView::onEdit_Copy()
{
    // Get the notebook selected page:
    gdStatisticsViewBase* pSelectedPage = selectedPage();

    if (pSelectedPage != NULL)
    {
        pSelectedPage->onCopy();
    }
    else if (selectedPageIndex() == GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX)
    {
        GT_IF_WITH_ASSERT(_pCallHistoryView != NULL)
        {
            _pCallHistoryView->onCopy();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::onEdit_SelectAll
// Description: execute the select all command in the VS edit menu
// Arguments:
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        6/2/2011
// ---------------------------------------------------------------------------
void gdStatisticsView::onEdit_SelectAll()
{
    // Get the notebook selected page:
    gdStatisticsViewBase* pSelectedPage = selectedPage();

    if (pSelectedPage != NULL)
    {
        pSelectedPage->onSelectAll();
    }
    else if (selectedPageIndex() == GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX)
    {
        GT_IF_WITH_ASSERT(_pCallHistoryView != NULL)
        {
            _pCallHistoryView->onSelectAll();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::onTreeItemSelection
// Description: Handling a tree item selection event
// Arguments:   apMonitoredObjectsTreeSelectedEvent& eve - the event thrown from the tree
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/1/2011
// ---------------------------------------------------------------------------
void gdStatisticsView::onTreeItemSelection(const apMonitoredObjectsTreeSelectedEvent& eve)
{
    // Get the item data;
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)eve.selectedItemData();

    if (pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());

        if (pGDData != NULL)
        {
            // The context the item context in the view:
            bool rcDisplay = displayContext(pGDData->_contextId);
            GT_ASSERT(rcDisplay);
        }
        else // pGDData == NULL
        {
            // If we selected the tree root, display the null context:
            if (AF_TREE_ITEM_APP_ROOT == pItemData->m_itemType)
            {
                static const apContextID nullCtxId(AP_OPENGL_CONTEXT, 0);
                bool rcNull = displayContext(nullCtxId);
                GT_ASSERT(rcNull);
            }
        }
    }
}


void gdStatisticsView::onBreakpointHitEvent()
{
    apContextID contextId;
    bool rc = gaGetBreakpointTriggeringContextId(contextId);

    if (rc)
    {
        // The context the item context in the view:
        bool rcDisplay = displayContext(contextId);
        GT_ASSERT(rcDisplay);
    }

}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::raiseView
// Description: Show the view as top window
// Author:      Sigal Algranaty
// Date:        27/1/2011
// ---------------------------------------------------------------------------
void gdStatisticsView::raiseView()
{
    gdApplicationCommands* pApplicationCommands = gdApplicationCommands::gdInstance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
        pApplicationCommands->raiseStatisticsView();
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::displayContext
// Description: Display the requested context statistics
// Arguments:   contextID - the context to display
//              forceUpdate - get the fresh data even if the context id is the same
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        30/1/2011
// ---------------------------------------------------------------------------
bool gdStatisticsView::displayContext(const apContextID& contextID, bool forceUpdate)
{
    bool retVal = true;

    if (forceUpdate || _activeContext != contextID)
    {
        // Set my active context:
        _activeContext = contextID;

        // Set the process debug suspension flag:
        bool isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

        // Go through each of the pages and update the context id:
        for (int i = 0; i < GD_STATISTICS_VIEW_LAST_VIEWER_INDEX; i++)
        {
            if (_pages[i] != NULL)
            {
                _pages[i]->setActiveContext(_activeContext);
                _pages[i]->setIsSuspended(isDebuggedProcessSuspended);
            }
        }

        // Update the statistics viewer:
        retVal = updateStatisticsViewers();
        GT_ASSERT_EX(retVal, L"Statistics viewers update failed");

        afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
        GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
        {
            // Build the window caption:
            gtString caption = pApplicationCommands->captionPrefix();

            if (contextID.isDefault())
            {
                caption.append(GD_STR_statisticsViewCaptionDefault);
            }
            else
            {
                // Get my display string:
                gtString contextStr;
                _activeContext.toString(contextStr);

                caption.appendFormattedString(GD_STR_statisticsViewCaptionWithContext, contextStr.asCharArray());
            }

            // Set the caption for the statistics view:
            pApplicationCommands->setWindowCaption(_pParentStatisticsPanel, caption);
        }

        // Make sure that the chart view is refreshed:
        GT_IF_WITH_ASSERT(_pChartWindow != NULL)
        {
            _pChartWindow->update();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::connectViewToHandlers
// Description:
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        28/12/2011
// ---------------------------------------------------------------------------
bool gdStatisticsView::connectViewToHandlers()
{
    bool retVal = false;

    // Down cast the total statistics tab:
    gdTotalStatisticsView* pTotalView = (gdTotalStatisticsView*)_pages[GD_STATISTICS_VIEW_TOTAL_INDEX];
    GT_IF_WITH_ASSERT(pTotalView != NULL)
    {
        // Connect item clicked and activated events:
        retVal = connect(pTotalView, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(onTotalStatisticsClick(QTableWidgetItem*)));
        retVal = connect(pTotalView, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onTotalStatisticsClick(QTableWidgetItem*))) && retVal;
        retVal = connect(pTotalView, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onTotalStatisticsClick(QTableWidgetItem*))) && retVal;
        retVal = connect(pTotalView, SIGNAL(itemSelectionChanged()), this, SLOT(onListSelectionChanged())) && retVal;
        retVal = connect(pTotalView, SIGNAL(columnSorted()), this, SLOT(onColumnSorted())) && retVal;
    }

    // Down cast the function calls statistics tab:
    gdFunctionCallsStatisticsView* pFunctionCallsView = (gdFunctionCallsStatisticsView*)_pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX];
    GT_IF_WITH_ASSERT(pFunctionCallsView != NULL)
    {
        // Connect item clicked and activated events:
        retVal = connect(pFunctionCallsView, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(onFunctionStatisticsClick(QTableWidgetItem*))) && retVal;
        retVal = connect(pFunctionCallsView, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onFunctionStatisticsClick(QTableWidgetItem*))) && retVal;
        retVal = connect(pFunctionCallsView, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onFunctionStatisticsClick(QTableWidgetItem*))) && retVal;
        retVal = connect(pFunctionCallsView, SIGNAL(itemSelectionChanged()), this, SLOT(onListSelectionChanged())) && retVal;
        retVal = connect(pFunctionCallsView, SIGNAL(columnSorted()), this, SLOT(onColumnSorted())) && retVal;
    }

    // Down cast the deprecation statistics tab:
    gdDeprecationStatisticsView* pDeprecationView = (gdDeprecationStatisticsView*)_pages[GD_STATISTICS_VIEW_DEPRECATION_INDEX];
    GT_IF_WITH_ASSERT(pDeprecationView != NULL)
    {
        // Connect item clicked and activated events:
        retVal = connect(pDeprecationView, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(onDeprecationStatsClick(QTableWidgetItem*))) && retVal;
        retVal = connect(pDeprecationView, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onDeprecationStatsClick(QTableWidgetItem*))) && retVal;
        retVal = connect(pDeprecationView, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onDeprecationStatsClick(QTableWidgetItem*))) && retVal;
        retVal = connect(pDeprecationView, SIGNAL(itemSelectionChanged()), this, SLOT(onListSelectionChanged())) && retVal;
        retVal = connect(pDeprecationView, SIGNAL(columnSorted()), this, SLOT(onColumnSorted())) && retVal;

    }

    // Down cast the deprecation statistics tab:
    gdBatchStatisticsView* pBatchView = (gdBatchStatisticsView*)_pages[GD_STATISTICS_VIEW_BATCH_INDEX];
    GT_IF_WITH_ASSERT(pBatchView != NULL)
    {
        // Connect item clicked and activated events:
        retVal = connect(pBatchView, SIGNAL(itemActivated(QTableWidgetItem*)), this, SLOT(onBatchStatsClick(QTableWidgetItem*))) && retVal;
        retVal = connect(pBatchView, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onBatchStatsClick(QTableWidgetItem*))) && retVal;
        retVal = connect(pBatchView, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onBatchStatsClick(QTableWidgetItem*))) && retVal;
        retVal = connect(pBatchView, SIGNAL(itemSelectionChanged()), this, SLOT(onListSelectionChanged())) && retVal;
        retVal = connect(pBatchView, SIGNAL(columnSorted()), this, SLOT(onColumnSorted())) && retVal;
    }

    // Calls history selection:
    GT_IF_WITH_ASSERT(_pCallHistoryView != NULL)
    {
        // Connect item clicked and activated events:
        retVal = connect(_pCallHistoryView, SIGNAL(activated(const QModelIndex&)), this, SLOT(onCallsHistoryClick(const QModelIndex&))) && retVal;
        retVal = connect(_pCallHistoryView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(onCallsHistoryClick(const QModelIndex&))) && retVal;
        retVal = connect(_pCallHistoryView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(onCallsHistoryClick(const QModelIndex&))) && retVal;
        retVal = connect(_pCallHistoryView, SIGNAL(entered(const QModelIndex&)), this, SLOT(onCallsHistoryClick(const QModelIndex&))) && retVal;
        retVal = connect(_pCallHistoryView, SIGNAL(pressed(const QModelIndex&)), this, SLOT(onCallsHistoryClick(const QModelIndex&))) && retVal;
    }

    // Connect the page change:
    (void) connect(this, SIGNAL(currentChanged(int)), this, SLOT(onPageChange(int)));
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::onListSelectionChanged
// Description: Is handling list selection change for all lists
// Author:      Sigal Algranaty
// Date:        28/12/2011
// ---------------------------------------------------------------------------
void gdStatisticsView::onListSelectionChanged()
{
    acListCtrl* pListCtrl = qobject_cast<acListCtrl*>(sender());
    GT_IF_WITH_ASSERT(pListCtrl != NULL)
    {
        // Get the selected item:
        QTableWidgetItem* pSelected = NULL;

        if (!pListCtrl->selectedItems().isEmpty())
        {
            // Do not udpate chart:
            _forceChartUpdate = false;

            // Get the first selected item:
            pSelected = pListCtrl->selectedItems().first();

            if (pListCtrl == _pages[GD_STATISTICS_VIEW_TOTAL_INDEX])
            {
                onTotalStatisticsClick(pSelected);
            }
            else if (pListCtrl == _pages[GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX])
            {
                onFunctionStatisticsClick(pSelected);
            }
            else if (pListCtrl == _pages[GD_STATISTICS_VIEW_DEPRECATION_INDEX])
            {
                onDeprecationStatsClick(pSelected);
            }
            else if (pListCtrl == _pages[GD_STATISTICS_VIEW_BATCH_INDEX])
            {
                onBatchStatsClick(pSelected);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::onFindClick
// Description: Find slot - call the find slot of the currently selected page
// Author:      Sigal Algranaty
// Date:        25/3/2012
// ---------------------------------------------------------------------------
void gdStatisticsView::onFindClick()
{
    // Get the current selected page index:
    int selectedIndex = selectedPageIndex();

    if (selectedIndex == GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX)
    {
        GT_IF_WITH_ASSERT(_pCallHistoryView != NULL)
        {
            _pCallHistoryView->onFindClick();
        }
    }
    else
    {
        // Get the notebook selected page:
        gdStatisticsViewBase* pSelectedPage = selectedPage();
        GT_IF_WITH_ASSERT(pSelectedPage != NULL)
        {
            pSelectedPage->onFindClick();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsView::onFindNext
// Description: Find next slot - call the find next slot of the selected page
// Author:      Sigal Algranaty
// Date:        25/3/2012
// ---------------------------------------------------------------------------
void gdStatisticsView::onFindNext()
{
    // Get the current selected page index:
    int selectedIndex = selectedPageIndex();

    if (selectedIndex == GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX)
    {
        GT_IF_WITH_ASSERT(_pCallHistoryView != NULL)
        {
            _pCallHistoryView->onFindNext();
        }
    }
    else
    {
        // Get the notebook selected page:
        gdStatisticsViewBase* pSelectedPage = selectedPage();
        GT_IF_WITH_ASSERT(pSelectedPage != NULL)
        {
            pSelectedPage->onFindNext();
        }
    }
}

// -----------------------------------------------------------------------------------------------
void gdStatisticsView::GetSelectedText(gtString& selectedText)
{
    // Get the notebook selected page:
    gdStatisticsViewBase* pSelectedPage = selectedPage();
    GT_IF_WITH_ASSERT(pSelectedPage != NULL)
    {
        pSelectedPage->GetSelectedText(selectedText);
    }
}

void gdStatisticsView::onColumnSorted()
{
    GT_IF_WITH_ASSERT(_pChartWindow)
    {
        _forceChartUpdate = true;
        int selectedIndex = selectedPageIndex();

        switch (selectedIndex)
        {
            case GD_STATISTICS_VIEW_TOTAL_INDEX:
                onTotalStatisticsClick(NULL);
                break;

            case GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX:
                onFunctionStatisticsClick(NULL);
                break;

            case GD_STATISTICS_VIEW_BATCH_INDEX:
                onBatchStatsClick(NULL);
                break;

            case GD_STATISTICS_VIEW_DEPRECATION_INDEX:
                onDeprecationStatsClick(NULL);
                break;
        }

        _pChartWindow->recalculateArrays();
    }
}