//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStatisticsPanel.cpp
///
//==================================================================================

//------------------------------ gdStatisticsPanel.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <QAction>
#include <QVBoxLayout>

#include <AMDTApplicationComponents/Include/acChartWindow.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunSuspendedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acToolBar.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommandIds.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/views/afPropertiesView.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdStatisticsPanel.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdBatchStatisticsView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStatisticsView.h>

#define GD_STATISTICS_VIEWER_MIN_CHART_WIDTH 150
#define GD_STATISTICS_VIEWER_MIN_CHART_HEIGHT 170

// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::gdStatisticsPanel
// Description: Constructor
// Arguments:   afProgressBarWrapper* pProgressBar
//              QWidget* pParent
//              const QSize& size
// Author:      Sigal Algranaty
// Date:        10/10/2011
// ---------------------------------------------------------------------------
gdStatisticsPanel::gdStatisticsPanel(afProgressBarWrapper* pProgressBar, QWidget* pParent, QMainWindow* pMainWindow)
    : QSplitter(Qt::Vertical, pParent), afBaseView(pProgressBar),
      m_pStatisticsView(NULL), m_pPropertiesView(NULL), m_pChartWindow(NULL),
      m_pToolbar(NULL), m_pClearAction(NULL), m_pBatchAction(NULL),
      m_pTopWidgetLayout(NULL), m_pBottomLayout(NULL), m_pBottomWidget(NULL), m_pTopWidget(NULL),
      m_isDebuggedProcessSuspended(false), m_isInfoUpdated(true)
{
    // Set the layout for the frame:
    setFrameLayout(pProgressBar, pMainWindow);

    // Check if the debugged process is suspended:
    m_isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

    // Reset the info updated flag:
    m_isInfoUpdated = false;

    // Display process not suspended message:
    displayPropertiesWindowMessage(GD_STR_StatisticsViewerPropertiesProcessNotSuspended);

    // Connect the chart item clicked signal:
    bool rc = connect(m_pChartWindow, SIGNAL(chartItemClicked(int)), m_pStatisticsView, SLOT(onChartItemClicked(int)));
    GT_ASSERT(rc);

    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::~gdStatisticsPanel
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        10/10/2011
// ---------------------------------------------------------------------------
gdStatisticsPanel::~gdStatisticsPanel()
{
    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::createToolbar
// Description: Create a toolbar for this frame
// Author:      Sigal Algranaty
// Date:        5/12/2011
// ---------------------------------------------------------------------------
void gdStatisticsPanel::createToolbar()
{
    // Create the toolbar:
    m_pToolbar = new acToolBar(m_pTopWidget);

    // Create the bitmap for the clear statistics button:
    QPixmap clearStatistics;
    acSetIconInPixmap(clearStatistics, AC_ICON_DEBUG_STATISTICS_BATCH);

    // Add the clear statistics action to the toolbar:
    m_pClearAction = m_pToolbar->addAction(clearStatistics, GD_STR_StatisticsViewerClearStatistics);
    GT_IF_WITH_ASSERT(m_pClearAction != NULL)
    {
        // Connect the action to the handler:
        bool rcConnect = connect(m_pClearAction, SIGNAL(triggered(bool)), this, SLOT(onClearButton()));
        GT_ASSERT(rcConnect);
    }

    // Create the bitmap for the details batch button:
    QPixmap detailedBatchData;
    acSetIconInPixmap(detailedBatchData, AC_ICON_DEBUG_STATISTICS_BATCH);

    // Add the show detailed batch action to the toolbar:
    m_pBatchAction = m_pToolbar->addAction(detailedBatchData, GD_STR_StatisticsViewerShowDetailedBatchStatistics);
    GT_IF_WITH_ASSERT(m_pBatchAction != NULL)
    {
        // Connect the action to the handler:
        bool rcConnect = connect(m_pBatchAction , SIGNAL(triggered(bool)), this, SLOT(onDetailedBatchDataButton()));
        GT_ASSERT(rcConnect);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::setFrameLayout
// Description: Build the panel layout with the view and toolbar
// Author:      Sigal Algranaty
// Date:        10/10/2011
// ---------------------------------------------------------------------------
void gdStatisticsPanel::setFrameLayout(afProgressBarWrapper* pProgressBar, QMainWindow* pMainWindow)
{
    setContentsMargins(0, 0, 0, 0);

    m_pTopWidget = new QWidget(this);

    // Create a toolbar and connect its actions:
    createToolbar();

    // Create a layout for the whole frame:
    m_pTopWidgetLayout = new QVBoxLayout(m_pTopWidget);
    m_pTopWidgetLayout->setContentsMargins(0, 0, 0, 0);

    // Create bottom layout:
    createBottomLayout(pMainWindow);

    // Create the calls history view:
    m_pStatisticsView = new gdStatisticsView(pProgressBar, this, this, false);

    // Set the frame layout:
    m_pStatisticsView->setFrameLayout(QSize(0, 0));

    // Set the chart window:
    m_pStatisticsView->setChartWindow(m_pChartWindow);

    // Show the statistics view:
    m_pStatisticsView->show();

    // Add the items to the sizer:
    m_pTopWidgetLayout->addWidget(m_pToolbar, 0);
    m_pTopWidgetLayout->addWidget(m_pStatisticsView, 1);
    m_pTopWidget->setLayout(m_pTopWidgetLayout);

    // Add the bottom and top widgets for split:
    addWidget(m_pTopWidget);
    addWidget(m_pBottomWidget);

    // Set the split proportions:
    setStretchFactor(0, 3);
    setStretchFactor(1, 1);

    // Make sure that none of the children is hidden:
    setChildrenCollapsible(false);

    // Set the width of the split handle to minimum:
    setHandleWidth(5);

    show();
    update();
    raise();
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::createBottomLayout
// Description: Create the layout for the bottom panel
// Author:      Sigal Algranaty
// Date:        6/12/2011
// ---------------------------------------------------------------------------
void gdStatisticsPanel::createBottomLayout(QMainWindow* pMainWindow)
{
    (void)(pMainWindow);  // unused
    m_pBottomWidget = new QWidget(this);

    // Create the chart window:
    m_pChartWindow = new acChartWindow(this, AC_NO_CHART);

    // Set min size:
    m_pChartWindow->resize(GD_STATISTICS_VIEWER_MIN_CHART_WIDTH, GD_STATISTICS_VIEWER_MIN_CHART_HEIGHT);

    // Set the chart window size:
    m_pChartWindow->clearAllData();
    m_pChartWindow->setChartType(AC_NO_CHART);
    m_pChartWindow->recalculateArrays();

    // Show the statistics view:
    m_pChartWindow->show();

    // Create the properties view:
    m_pPropertiesView = new afPropertiesView(_pOwnerProgressBar, m_pBottomWidget);

    // Create an horizontal layout for the bottom window:
    m_pBottomLayout = new QHBoxLayout(m_pBottomWidget);

    // Add the chart and properties to the bottom window:
    m_pBottomLayout->setContentsMargins(0, 0, 0, 0);
    m_pBottomLayout->addWidget(m_pChartWindow, 1);
    m_pBottomLayout->addWidget(m_pPropertiesView, 1);

    // Set the bottom widget layout:
    m_pBottomWidget->setLayout(m_pBottomLayout);
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::onUpdateEdit_Copy
// Description: Overrides afBaseView virtual event
// Arguments:   bool& isEnabled
// Author:      Sigal Algranaty
// Date:        16/10/2011
// ---------------------------------------------------------------------------
void gdStatisticsPanel::onUpdateEdit_Copy(bool& isEnabled)
{
    GT_IF_WITH_ASSERT(m_pStatisticsView != NULL)
    {
        m_pStatisticsView->onUpdateEdit_Copy(isEnabled);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::onUpdateEdit_SelectAll
// Description: Overrides afBaseView virtual event
// Arguments:   bool& isEnabled
// Author:      Sigal Algranaty
// Date:        10/10/2011
// ---------------------------------------------------------------------------
void gdStatisticsPanel::onUpdateEdit_SelectAll(bool& isEnabled)
{
    GT_IF_WITH_ASSERT(m_pStatisticsView != NULL)
    {
        m_pStatisticsView->onUpdateEdit_SelectAll(isEnabled);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::onUpdateEdit_Find
// Description: Overrides afBaseView virtual event
// Arguments:   bool& isEnabled
// Author:      Sigal Algranaty
// Date:        31/3/2011
// ---------------------------------------------------------------------------
void gdStatisticsPanel::onUpdateEdit_Find(bool& isEnabled)
{
    (void)(isEnabled);  // unused
    // TO_DO: Statistics: implement!!!
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::onUpdateEdit_FindNext
// Description: Overrides afBaseView virtual event
// Arguments:   bool& isEnabled
// Author:      Sigal Algranaty
// Date:        31/3/2011
// ---------------------------------------------------------------------------
void gdStatisticsPanel::onUpdateEdit_FindNext(bool& isEnabled)
{
    (void)(isEnabled);  // unused
    // TO_DO: Statistics: implement!!!
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::onEdit_Copy
// Description: Overrides afBaseView virtual event
// Arguments:
// Author:      Sigal Algranaty
// Date:        10/10/2011
// ---------------------------------------------------------------------------
void gdStatisticsPanel::onEdit_Copy()
{
    GT_IF_WITH_ASSERT(m_pStatisticsView != NULL)
    {
        m_pStatisticsView->onEdit_Copy();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::onEdit_Find
// Description: Overrides afBaseView virtual event
// Arguments:
// Author:      Sigal Algranaty
// Date:        31/3/2011
// ---------------------------------------------------------------------------
void gdStatisticsPanel::onEdit_Find()
{
    // TO_DO: Statistics: implement!!!
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::onEdit_FindNext
// Description: Overrides afBaseView virtual event
// Arguments:
// Author:      Sigal Algranaty
// Date:        31/3/2011
// ---------------------------------------------------------------------------
void gdStatisticsPanel::onEdit_FindNext()
{
    // TO_DO: Statistics: implement!!!
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::onEdit_SelectAll
// Description: Overrides afBaseView virtual event
// Arguments:
// Author:      Sigal Algranaty
// Date:        10/10/2011
// ---------------------------------------------------------------------------
void gdStatisticsPanel::onEdit_SelectAll()
{
    GT_IF_WITH_ASSERT(m_pStatisticsView != NULL)
    {
        m_pStatisticsView->onEdit_SelectAll();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::onClearButton
// Description: Clears the statistics
// Arguments:   Event details
// Author:      Sigal Algranaty
// Date:        10/10/2011
// ---------------------------------------------------------------------------
void gdStatisticsPanel::onClearButton()
{
    // Clear the spy statistics:
    bool rc = gaClearFunctionCallsStatistics();
    GT_ASSERT_EX(rc, L"Clear function failed");

    // Mark that the view's information is not updated:
    m_isInfoUpdated = false;

    rc = updateStatisticsViewers();
    GT_ASSERT_EX(rc, L"Statistics viewers update failed");

    if (m_pChartWindow != NULL)
    {
        m_pChartWindow->clearAllData();
        m_pChartWindow->recalculateArrays();
        m_pChartWindow->redrawWindow();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::onDetailedBatchDataButton
// Description: Is called when the user clicks the "Detailed batch data" button
// Author:      Sigal Algranaty
// Date:        10/10/2011
// ---------------------------------------------------------------------------
void gdStatisticsPanel::onDetailedBatchDataButton()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pStatisticsView != NULL)
    {
        m_pStatisticsView->onDetailedBatchDataButton();

        // Update both chart and properties views:
        updateChartAndPropertiesViews(GD_STATISTICS_VIEW_UNKNOWN);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::displayPropertiesWindowMessage
// Description: Displays item information in the properties window
// Arguments: title - a title to show in the window
//            information - the content of the window
// Return Val: void
// Author:      Uri Shomroni
// Date:        7/8/2008
// ---------------------------------------------------------------------------
void gdStatisticsPanel::displayPropertiesWindowMessage(const gtString& title, const gtString& information)
{
    gtString htmlWindowString;

    // Build an HTML header:
    afHTMLContent::buildHTMLHeader(htmlWindowString);

    if (title.length() > 0)
    {
        htmlWindowString.append(AF_STR_HtmlPropertiesNonbreakingSpace AF_STR_HtmlPropertiesNonbreakingSpace);
        htmlWindowString.append(AF_STR_HtmlPropertiesFontHeaderTagStart);
        htmlWindowString.append(title);
        htmlWindowString.append(AF_STR_HtmlPropertiesFontHeaderTagEnd);
    }

    if (information.length() > 0)
    {
        htmlWindowString.append(information);
    }

    // End the HTML string:
    afHTMLContent::endHTML(htmlWindowString);

    m_pPropertiesView->setText(acGTStringToQString(htmlWindowString));
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::onEvent
// Description: Is called when a debugged process event occurs.
// Arguments:   const apEvent& eve - A class representing the event.
// Author:      Sigal Algranaty
// Date:        22/7/2008
// ---------------------------------------------------------------------------
void gdStatisticsPanel::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent);  // unused
    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    bool shouldRefresh = false;

    switch (eventType)
    {

        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        case apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE:
        {
            onProcessTerminated();
            shouldRefresh = true;
        }

        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        case apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED:
        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        {
            onProcessStarted();
            shouldRefresh = true;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
        {
            // Update the textures and buffers viewer
            onProcessRunSuspended((const apDebuggedProcessRunSuspendedEvent&)eve);
            shouldRefresh = true;
        }
        break;

        case apEvent::GD_MONITORED_OBJECT_SELECTED_EVENT:
        {
            if (m_isDebuggedProcessSuspended)
            {
                GT_IF_WITH_ASSERT(m_pStatisticsView != NULL)
                {
                    m_pStatisticsView->onTreeItemSelection((const apMonitoredObjectsTreeSelectedEvent&)eve);
                }
                break;
            }
        }

        case apEvent::AP_BREAKPOINT_HIT:
        {
            m_pStatisticsView->onBreakpointHitEvent();
            break;
        }

        case apEvent::AP_EXECUTION_MODE_CHANGED_EVENT:
        {
            // Enable the view only in debug mode:
            bool isEnabled = true;
            bool modeChanged = gdDoesModeChangeApplyToDebuggerViews(eve, isEnabled);

            if (modeChanged)
            {
                setEnabled(isEnabled);
            }
        }
        break;

        default:
            // Do nothing...
            break;
    }

    if (shouldRefresh)
    {
        // Refresh the views
        updateChartAndPropertiesViews(GD_STATISTICS_VIEW_UNKNOWN);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::updateChartAndPropertiesViews
// Description: updates the chart to reflect the data in the last selected notebook page
//              whose index is currentPageIndex. If currentPageIndex is GD_STATISTICS_VIEW_UNKNOWN, we
//              simply refresh the current page without changing anything.
// Author:      Uri Shomroni
// Date:        31/7/2008
// ---------------------------------------------------------------------------
void gdStatisticsPanel::updateChartAndPropertiesViews(gdStatisticsViewIndex windowIndex)
{
    apExecutionMode currentExecMode;
    bool rcExec = gaGetDebuggedProcessExecutionMode(currentExecMode);
    GT_ASSERT(rcExec);

    if ((currentExecMode != AP_PROFILING_MODE) || !(rcExec))
    {
        if (m_pStatisticsView != NULL)
        {
            if (windowIndex != GD_STATISTICS_VIEW_UNKNOWN)
            {
                m_pStatisticsView->applyPageOnClick(windowIndex);
            }
        }
    }
    else
    {
        // Profile mode - update chart type and properties message:
        m_pChartWindow->setChartType(AC_NO_CHART);
        m_pChartWindow->recalculateArrays();
        m_pChartWindow->redrawWindow();
        displayPropertiesWindowMessage(GD_STR_StatisticsViewerProfileModeProperties);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::updateStatisticsViewers
// Description: Update the statistics viewers
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/7/2008
// ---------------------------------------------------------------------------
bool gdStatisticsPanel::updateStatisticsViewers()
{
    bool retVal = false;

    // If the viewer's information is already updated:
    if (m_isInfoUpdated)
    {
        retVal = true;
    }
    else
    {
        if (m_isDebuggedProcessSuspended)
        {
            GT_IF_WITH_ASSERT(m_pStatisticsView != NULL)
            {
                // Update the view in the notebook:
                retVal = m_pStatisticsView->updateStatisticsViewers();

                // Update both chart and properties views:
                updateChartAndPropertiesViews(GD_STATISTICS_VIEW_UNKNOWN);

                // Mark the the view's information is updated:
                m_isInfoUpdated = true;
            }
        }
        else
        {
            // This function should only be called if the debugged process is suspended or dead:
            retVal = !gaDebuggedProcessExists();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::onProcessTerminated
// Description: Is called when the debugged process is terminated.
// Author:      Sigal Algranaty
// Date:        22/7/2008
// ---------------------------------------------------------------------------
void gdStatisticsPanel::onProcessTerminated()
{
    // Clear the properties:
    displayPropertiesWindowMessage(GD_STR_StatisticsViewerPropertiesProcessNotSuspended);

    GT_IF_WITH_ASSERT(m_pChartWindow != NULL)
    {
        m_pChartWindow->clearAllData();
        m_pChartWindow->redrawWindow();
    }

    // Mark that the debugged process is not suspended
    m_isDebuggedProcessSuspended = false;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::onProcessStarted
// Description: Handles process started event - clears the spy statistics, and
//              also clears the viewers lists
// Return Val: void
// Author:      Sigal Algranaty
// Date:        22/7/2008
// ---------------------------------------------------------------------------
void gdStatisticsPanel::onProcessStarted()
{
    // Mark that the debugged process is not suspended
    m_isDebuggedProcessSuspended = false;

    // Mark that our info is out of date:
    m_isInfoUpdated = false;

    // Clear the viewers from previous results:
    clearAllViewers();

    // Display a message in the properties window:
    displayPropertiesWindowMessage(GD_STR_StatisticsViewerPropertiesProcessNotSuspended);

}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::onProcessRunSuspended
// Description: Is called when the debugged process run is suspended.
// Author:      Eran Zinman
// Date:        24/5/2007
// ---------------------------------------------------------------------------
void gdStatisticsPanel::onProcessRunSuspended(const apDebuggedProcessRunSuspendedEvent& event)
{
    (void)(event);  // unused
    // Mark that the debugged process is suspended
    m_isDebuggedProcessSuspended = true;

    // Get the context which triggered the process suspension:
    apContextID contextId;
    bool rc = gaGetBreakpointTriggeringContextId(contextId);
    GT_IF_WITH_ASSERT(rc)
    {
        // Update all the viewers with the current statistics:
        rc = updateStatisticsViewers();
        GT_ASSERT_EX(rc, L"Statistics viewers update failed");
    }
}
// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::clearAllViewers
// Description: Clears all the viewers from statistics details
// Author:      Sigal Algranaty
// Date:        22/7/2008
// ---------------------------------------------------------------------------
void gdStatisticsPanel::clearAllViewers()
{
    GT_IF_WITH_ASSERT(m_pStatisticsView != NULL)
    {
        m_pStatisticsView->clearAllViewers();
    }

    // Clear the chart:
    updateChartAndPropertiesViews(GD_STATISTICS_VIEW_UNKNOWN);

    // Clear the properties window
    displayPropertiesWindowMessage();
}
// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::onViewShown
// Description: Set the "is shown" flag, and update the view if not updated
// Arguments:   bool isViewShown
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        31/3/2011
// ---------------------------------------------------------------------------
void gdStatisticsPanel::onViewShown(bool isViewShown)
{
    (void)(isViewShown);  // unused

    // Update the according to the current debugged process status:
    if (gaIsDebuggedProcessSuspended())
    {
        apDebuggedProcessRunSuspendedEvent dummySuspendedEvent;
        onProcessRunSuspended(dummySuspendedEvent);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::isActionEnabled
// Description: Checks if the actions is enabled for the input focused widget.
// Arguments:   QWidget* pFocusedWidget - should be one of the panel components
//              int commandId - the command id
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/12/2011
// ---------------------------------------------------------------------------
bool gdStatisticsPanel::isActionEnabled(int commandId)
{
    bool retVal = true;

    GT_IF_WITH_ASSERT((m_pStatisticsView != NULL) && (m_pPropertiesView != NULL) && (m_pStatisticsView != NULL))
    {
        if (m_pStatisticsView->hasFocus())
        {
            if (commandId == ID_COPY)
            {
                onUpdateEdit_Copy(retVal);
            }
            else if (commandId == ID_FIND)
            {
                onUpdateEdit_Find(retVal);
            }
            else if (commandId == ID_FIND_NEXT)
            {
                onUpdateEdit_FindNext(retVal);
            }
            else if (commandId == ID_SELECT_ALL)
            {
                onUpdateEdit_SelectAll(retVal);

            }
        }
        else if (m_pPropertiesView->hasFocus())
        {
            retVal = true;
        }
    }

    // For the chart view - none of the actions are enabled:
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::triggerCommand
// Description: Trigger the requested command for the widget
// Arguments:   int commandId
//              QWidget* pCurrentFocusedWidget - the currently focused widget
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/12/2011
// ---------------------------------------------------------------------------
bool gdStatisticsPanel::triggerCommand(QWidget* pCurrentFocusedWidget, int commandId)
{
    bool retVal = false;
    GT_IF_WITH_ASSERT((m_pStatisticsView != NULL) && (m_pPropertiesView != NULL))
    {
        if (pCurrentFocusedWidget == m_pStatisticsView)
        {
            if (commandId == ID_COPY)
            {
                m_pStatisticsView->onEdit_Copy();
                retVal = true;
            }
            else if (commandId == ID_FIND)
            {
                GT_ASSERT_EX(false, L"Not implemented yet");
            }
            else if (commandId == ID_FIND_NEXT)
            {
                GT_ASSERT_EX(false, L"Not implemented yet");
            }
            else if (commandId == ID_SELECT_ALL)
            {
                m_pStatisticsView->onEdit_SelectAll();
                retVal = true;
            }
        }
        else if (pCurrentFocusedWidget == m_pPropertiesView)
        {
            if (commandId == ID_COPY)
            {
                m_pPropertiesView->copy();
                retVal = true;
            }
            else if (commandId == ID_FIND)
            {
                GT_ASSERT_EX(false, L"Not implemented yet");
            }
            else if (commandId == ID_FIND_NEXT)
            {
                GT_ASSERT_EX(false, L"Not implemented yet");
            }
            else if (commandId == ID_SELECT_ALL)
            {
                m_pPropertiesView->selectAll();
                retVal = true;
            }
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::onFindClick
// Description: Find slot - call the view find slot
// Author:      Sigal Algranaty
// Date:        25/3/2012
// ---------------------------------------------------------------------------
void gdStatisticsPanel::onFindClick()
{
    GT_IF_WITH_ASSERT(m_pStatisticsView != NULL)
    {
        m_pStatisticsView->onFindClick();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsPanel::onFindNext
// Description: Find next slot - call the view find next slot
// Author:      Sigal Algranaty
// Date:        25/3/2012
// ---------------------------------------------------------------------------
void gdStatisticsPanel::onFindNext()
{
    GT_IF_WITH_ASSERT(m_pStatisticsView != NULL)
    {
        m_pStatisticsView->onFindNext();
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        updateToolbarCommands
/// \brief Description: This function enables / disables the toolbar actions according to the current
///                     state of the application
/// \return True :
/// \return False:
/// -----------------------------------------------------------------------------------------------
void gdStatisticsPanel::updateToolbarCommands()
{
    GT_IF_WITH_ASSERT((m_pBatchAction != NULL) && (m_pClearAction != NULL) && (m_pStatisticsView != NULL))
    {
        bool isEnabled = false;
        m_pStatisticsView->onUpdateDetailedBatchDataButton(isEnabled);
        m_pBatchAction->setEnabled(isEnabled);

        onUpdateClearButton(isEnabled);
        m_pClearAction->setEnabled(isEnabled);
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        onUpdateClearButton
/// \brief Description: Should the clear button be enabled?
/// \param[in]          isEnabled
/// \return void
/// -----------------------------------------------------------------------------------------------
void gdStatisticsPanel::onUpdateClearButton(bool& isEnabled)
{
    isEnabled = isVisible() && m_isDebuggedProcessSuspended && gaDebuggedProcessExists();
}

/// -----------------------------------------------------------------------------------------------
void gdStatisticsPanel::GetSelectedText(gtString& selectedText)
{
    GT_IF_WITH_ASSERT(m_pStatisticsView != NULL)
    {
        m_pStatisticsView->GetSelectedText(selectedText);
    }
}

/// -----------------------------------------------------------------------------------------------