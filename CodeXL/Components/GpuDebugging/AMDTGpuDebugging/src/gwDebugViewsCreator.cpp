//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwDebugViewsCreator.cpp
///
//==================================================================================

//------------------------------ gwDebugViewsCreator.h ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTApplicationComponents/Include/acListCtrl.h>
#include <AMDTApplicationComponents/Include/acVirtualListCtrl.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afCommandIds.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdAPICallsHistoryPanel.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdStatisticsPanel.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdBreakpointsView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdCallStackView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdDebuggedProcessEventsView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdLocalsView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdMemoryView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdMultiWatchView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStateVariablesView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdWatchView.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>

// Local:
#include <src/gwDebugActionsCreator.h>
#include <src/gwDebugViewsCreator.h>

// View size definition:
#define GD_BOTTOM_ROW_HEIGHT_PROPORTION(x) (x / 4)
#define GD_MAIN_VIEWS_HEIGHT_PROPORTION(x) ((x * 3) / 4)
#define GD_SIDE_VIEWS_WIDTH_PROPORTION(x) ((x * 3) / 10)
#define GD_BOTTOM_ROW_WIDE_VIEWS_WIDTH_PROPORTION(x) ((x * 2) / 5)
#define GD_BOTTOM_ROW_NARROW_VIEWS_WIDTH_PROPORTION(x) (x / 5)

// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::gwDebugViewsCreator
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
gwDebugViewsCreator::gwDebugViewsCreator():
    m_pApplicationCommandsHandler(NULL), m_pPropertiesView(NULL), m_pBreakpointsView(NULL), m_pLocalsView(NULL),
    m_pWatchView(NULL), m_pStatisticsPanel(NULL), m_pStateVariablesView(NULL), m_pCommandQueuesView(NULL), m_pAPICallsHistoryPanel(NULL),
    m_pDebuggedProcessEventsView(NULL), m_pMemoryView(NULL), m_pCallStackView(NULL),
    m_pMultiWatchView1(NULL), m_pMultiWatchView2(NULL), m_pMultiWatchView3(NULL)
{
    // Initialize the view captions:
    m_viewCaptions[gdBreakpointsViewIndex] = GD_STR_breakpointsViewCaption;
    m_viewCaptions[gdLocalsViewIndex] = GD_STR_localsViewCaption;
    m_viewCaptions[gdWatchViewIndex] = GD_STR_watchViewCaption;
    m_viewCaptions[gdStatisticsPanelIndex] = GD_STR_statisticsViewCaptionDefault;
    m_viewCaptions[gdStateVariablesViewIndex] = GD_STR_StateVariablesViewCaptionDefault;
    m_viewCaptions[gdAPICallsHistoryPanelIndex] = GD_STR_callsHistoryViewCaptionDefault;
    m_viewCaptions[gdDebuggedProcessEventsViewIndex] = GD_STR_DebuggedProcessEventsViewCaption;
    m_viewCaptions[gdMemoryViewIndex] = GD_STR_memoryViewCaptionDefault;
    m_viewCaptions[gdCallStackViewIndex] = GD_STR_CallsStackViewCaption;
    m_viewCaptions[gdMultiWatch1Index] = GD_STR_multiwatchView1Caption;
    m_viewCaptions[gdMultiWatch2Index] = GD_STR_multiwatchView2Caption;
    m_viewCaptions[gdMultiWatch3Index] = GD_STR_multiwatchView3Caption;

    // Initialize the view captions:
    m_viewMenuCommands[gdBreakpointsViewIndex] = GD_STR_breakpointsViewCommandName;
    m_viewMenuCommands[gdLocalsViewIndex] = GD_STR_localsViewCommandName;
    m_viewMenuCommands[gdWatchViewIndex] = GD_STR_watchViewCommandName;
    m_viewMenuCommands[gdStatisticsPanelIndex] = GD_STR_statisticsViewCommandName;
    m_viewMenuCommands[gdStateVariablesViewIndex] = GD_STR_StateVariablesViewCommandName;
    m_viewMenuCommands[gdAPICallsHistoryPanelIndex] = GD_STR_callsHistoryViewCommandName;
    m_viewMenuCommands[gdDebuggedProcessEventsViewIndex] = GD_STR_DebuggedProcessEventsViewCommandName;
    m_viewMenuCommands[gdMemoryViewIndex] = GD_STR_memoryViewCommandName;
    m_viewMenuCommands[gdCallStackViewIndex] = GD_STR_CallsStackViewCommandName;
    m_viewMenuCommands[gdMultiWatch1Index] = GD_STR_multiwatchView1CommandName;
    m_viewMenuCommands[gdMultiWatch2Index] = GD_STR_multiwatchView2CommandName;
    m_viewMenuCommands[gdMultiWatch3Index] = GD_STR_multiwatchView3CommandName;


    // Array with the views docking areas:
    m_viewDockingAreas[gdBreakpointsViewIndex] = AF_VIEW_DOCK_BottomDockWidgetArea;
    m_viewDockingAreas[gdLocalsViewIndex] = AF_VIEW_DOCK_BottomDockWidgetArea;
    m_viewDockingAreas[gdWatchViewIndex] = AF_VIEW_DOCK_BottomDockWidgetArea;
    m_viewDockingAreas[gdStatisticsPanelIndex] = AF_VIEW_DOCK_RightDockWidgetArea;
    m_viewDockingAreas[gdStateVariablesViewIndex] = AF_VIEW_DOCK_BottomDockWidgetArea;
    // Profiling: m_viewDockingAreas[gdCommandQueuesViewIndex] = AF_VIEW_DOCK_RightDockWidgetArea;
    m_viewDockingAreas[gdAPICallsHistoryPanelIndex] = AF_VIEW_DOCK_BottomDockWidgetArea;
    m_viewDockingAreas[gdDebuggedProcessEventsViewIndex] = AF_VIEW_DOCK_BottomDockWidgetArea;
    m_viewDockingAreas[gdMemoryViewIndex] = AF_VIEW_DOCK_RightDockWidgetArea;
    m_viewDockingAreas[gdCallStackViewIndex] = AF_VIEW_DOCK_BottomDockWidgetArea;

    m_viewDockingAreas[gdMultiWatch1Index] = AF_VIEW_DOCK_RightDockWidgetArea;
    m_viewDockingAreas[gdMultiWatch2Index] = AF_VIEW_DOCK_RightDockWidgetArea;
    m_viewDockingAreas[gdMultiWatch3Index] = AF_VIEW_DOCK_RightDockWidgetArea;


    // Initialize the array with the views docking features:
    m_viewDockingFeatures[gdStateVariablesViewIndex] = QDockWidget::AllDockWidgetFeatures;
    m_viewDockingFeatures[gdBreakpointsViewIndex] = QDockWidget::AllDockWidgetFeatures;
    m_viewDockingFeatures[gdLocalsViewIndex] = QDockWidget::AllDockWidgetFeatures;
    m_viewDockingFeatures[gdWatchViewIndex] = QDockWidget::AllDockWidgetFeatures;
    m_viewDockingFeatures[gdStatisticsPanelIndex] = QDockWidget::AllDockWidgetFeatures;
    // Profiling: m_viewDockingFeatures[gdCommandQueuesViewIndex] = QDockWidget::AllDockWidgetFeatures;
    m_viewDockingFeatures[gdAPICallsHistoryPanelIndex] = QDockWidget::AllDockWidgetFeatures;
    m_viewDockingFeatures[gdDebuggedProcessEventsViewIndex] = QDockWidget::AllDockWidgetFeatures;
    m_viewDockingFeatures[gdMemoryViewIndex] = QDockWidget::AllDockWidgetFeatures;
    m_viewDockingFeatures[gdCallStackViewIndex] = QDockWidget::AllDockWidgetFeatures;
    m_viewDockingFeatures[gdMultiWatch1Index] = QDockWidget::AllDockWidgetFeatures;
    m_viewDockingFeatures[gdMultiWatch2Index] = QDockWidget::AllDockWidgetFeatures;
    m_viewDockingFeatures[gdMultiWatch3Index] = QDockWidget::AllDockWidgetFeatures;

    // Create an action creator:
    _pViewActionCreator = new gwDebugActionsCreator;

    _pViewActionCreator->initializeCreator();
}

// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::initialize
// Description: Initialize the creator
// Author:      Sigal Algranaty
// Date:        8/12/2011
// ---------------------------------------------------------------------------
void gwDebugViewsCreator::initialize()
{
    // Initialize the icons:
    initSingleViewIcon(gdBreakpointsViewIndex, AC_ICON_DEBUG_VIEW_BREAKPOINTS);
    initSingleViewIcon(gdAPICallsHistoryPanelIndex, AC_ICON_DEBUG_VIEW_CALLHISTORY);
    initSingleViewIcon(gdLocalsViewIndex, AC_ICON_DEBUG_VIEW_LOCALS);
    initSingleViewIcon(gdWatchViewIndex, AC_ICON_DEBUG_VIEW_WATCH);
    initSingleViewIcon(gdStatisticsPanelIndex, AC_ICON_DEBUG_VIEW_STATISTICS);
    initSingleViewIcon(gdDebuggedProcessEventsViewIndex, AC_ICON_DEBUG_VIEW_EVENTS);
    initSingleViewIcon(gdCallStackViewIndex, AC_ICON_DEBUG_VIEW_CALLSTACK);
    initSingleViewIcon(gdStateVariablesViewIndex, AC_ICON_DEBUG_VIEW_STATEVARS);
    initSingleViewIcon(gdMemoryViewIndex, AC_ICON_DEBUG_VIEW_MEMORY);
    initSingleViewIcon(gdMultiWatch1Index, AC_ICON_DEBUG_VIEW_MULTIWATCH);
    initSingleViewIcon(gdMultiWatch2Index, AC_ICON_DEBUG_VIEW_MULTIWATCH);
    initSingleViewIcon(gdMultiWatch3Index, AC_ICON_DEBUG_VIEW_MULTIWATCH);
}

// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::~gwDebugViewsCreator
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
gwDebugViewsCreator::~gwDebugViewsCreator()
{
}

// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::titleString
// Description: Get the title of the created view
// Arguments:   int viewIndex
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
void gwDebugViewsCreator::titleString(int viewIndex, gtString& viewTitle, gtString& viewMenuCommand)
{
    GT_IF_WITH_ASSERT((viewIndex >= 0) && (viewIndex < gdDebugAmountOfViews))
    {
        // Return the view caption:
        viewTitle = m_viewCaptions[viewIndex];
        viewMenuCommand = m_viewMenuCommands[viewIndex];
    }
}

// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::menuPosition
// Description: Get the menu position in the menu view:
// Author:      Gilad Yarnitzky
// Date:        9/7/2012
// ---------------------------------------------------------------------------
gtString gwDebugViewsCreator::modeMenuPosition(int actionIndex, afActionPositionData& positionData)
{
    (void)(positionData); // unused
    gtString retVal = L"";

    if ((actionIndex >= gdMultiWatch1Index) && (actionIndex <= gdMultiWatch3Index))
    {
        retVal.append(GD_STR_MultiWatchViewsMenuString);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::associatedToolbar
// Description: Get the name of the toolbar associated with the requested view
// Arguments:   int viewIndex
// Return Val:  gtString
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
gtString gwDebugViewsCreator::associatedToolbar(int viewIndex)
{
    gtString retVal;

    if ((viewIndex == gdMultiWatch1Index) || (viewIndex == gdMultiWatch2Index) || (viewIndex == gdMultiWatch3Index))
    {
        retVal = GD_STR_OpenCLDebuggingToolbarName;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::type
// Description: Get view type
// Arguments:   int viewIndex
// Return Val:  afViewType
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
afViewCreatorAbstract::afViewType gwDebugViewsCreator::type(int viewIndex)
{
    (void)(viewIndex); // unused
    afViewCreatorAbstract::afViewType retDockArea = AF_VIEW_dock;

    return retDockArea;
}

// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::dockArea
// Description: Get the docking area
// Arguments:   int viewIndex
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
int gwDebugViewsCreator::dockArea(int viewIndex)
{
    int retVal = AF_VIEW_DOCK_LeftDockWidgetArea;

    GT_IF_WITH_ASSERT((viewIndex >= 0) && (viewIndex < gdDebugAmountOfViews))
    {
        // Return the view caption:
        retVal = m_viewDockingAreas[viewIndex];
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::dockWith
// Description: Define for each dock if it is docked with another view
// Arguments:   int viewIndex
// Return Val:  gtString
// Author:      Gilad Yarnitzky
// Date:        27/2/2012er
// ---------------------------------------------------------------------------
gtString gwDebugViewsCreator::dockWith(int viewIndex)
{
    gtString retString = L"";

    GT_IF_WITH_ASSERT((viewIndex >= 0) && (viewIndex < gdDebugAmountOfViews))
    {
        switch (viewIndex)
        {
            case gdMemoryViewIndex:
                break;

            case gdStatisticsPanelIndex:
                retString.append(m_viewCaptions[gdMemoryViewIndex]);
                break;

            case gdAPICallsHistoryPanelIndex:
                break;

            case gdWatchViewIndex:
                retString.append(m_viewCaptions[gdAPICallsHistoryPanelIndex]);
                break;

            case gdLocalsViewIndex:
                retString.append(m_viewCaptions[gdWatchViewIndex]);
                break;

            case gdStateVariablesViewIndex:
                retString.append(m_viewCaptions[gdLocalsViewIndex]);
                break;

            case gdDebuggedProcessEventsViewIndex:
                break;

            case gdCallStackViewIndex:
                retString.append(m_viewCaptions[gdDebuggedProcessEventsViewIndex]);
                break;

            case gdBreakpointsViewIndex:
                retString.append(m_viewCaptions[gdCallStackViewIndex]);
                break;

            case gdMultiWatch1Index:
                break;

            case gdMultiWatch2Index:
                retString = m_viewCaptions[gdMultiWatch1Index];
                break;

            case gdMultiWatch3Index:
                retString = m_viewCaptions[gdMultiWatch2Index];
                break;

            default:
                break;
        }
    }

    return retString;
}

// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::dockWidgetFeatures
// Description: Get the docking features
// Return Val:  DockWidgetFeatures
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
QDockWidget::DockWidgetFeatures gwDebugViewsCreator::dockWidgetFeatures(int viewIndex)
{
    QDockWidget::DockWidgetFeatures retVal = QDockWidget::NoDockWidgetFeatures;

    GT_IF_WITH_ASSERT((viewIndex >= 0) && (viewIndex < gdDebugAmountOfViews))
    {
        // Return the view caption:
        retVal = m_viewDockingFeatures[viewIndex];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::initialSize
// Description: Get the initial size
// Arguments:   int viewIndex
// Return Val:  QSize - size of the view
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
QSize gwDebugViewsCreator::initialSize(int viewIndex)
{
    QSize retSize(0, 0);
    QSize desktopSize = QApplication::desktop()->rect().size();

    // Set the default minimum view height and width
    int minViewWidth = 100;
    int minViewHeight = 30;
    QSize minViewSize(minViewWidth, minViewHeight);

    switch (viewIndex)
    {

        case gdBreakpointsViewIndex:
            retSize.setWidth(GD_BOTTOM_ROW_WIDE_VIEWS_WIDTH_PROPORTION(desktopSize.width()));
            retSize.setHeight(GD_BOTTOM_ROW_HEIGHT_PROPORTION(desktopSize.height()));
            break;

        case gdLocalsViewIndex:
            retSize.setWidth(GD_BOTTOM_ROW_WIDE_VIEWS_WIDTH_PROPORTION(desktopSize.width()));
            retSize.setHeight(GD_BOTTOM_ROW_HEIGHT_PROPORTION(desktopSize.height()));
            break;

        case gdWatchViewIndex:
            retSize.setWidth(GD_BOTTOM_ROW_WIDE_VIEWS_WIDTH_PROPORTION(desktopSize.width()));
            retSize.setHeight(GD_BOTTOM_ROW_HEIGHT_PROPORTION(desktopSize.height()));
            break;

        case gdStatisticsPanelIndex:
            retSize.setWidth(GD_SIDE_VIEWS_WIDTH_PROPORTION(desktopSize.width()));
            retSize.setHeight(GD_MAIN_VIEWS_HEIGHT_PROPORTION(desktopSize.height()));
            break;

        case gdStateVariablesViewIndex:
            retSize.setWidth(GD_BOTTOM_ROW_WIDE_VIEWS_WIDTH_PROPORTION(desktopSize.width()));
            retSize.setHeight(GD_BOTTOM_ROW_HEIGHT_PROPORTION(desktopSize.height()));
            break;

        case gdAPICallsHistoryPanelIndex:
            retSize.setWidth(GD_BOTTOM_ROW_WIDE_VIEWS_WIDTH_PROPORTION(desktopSize.width()));
            retSize.setHeight(GD_BOTTOM_ROW_HEIGHT_PROPORTION(desktopSize.height()));
            break;

        case gdDebuggedProcessEventsViewIndex:
            retSize.setWidth(GD_BOTTOM_ROW_WIDE_VIEWS_WIDTH_PROPORTION(desktopSize.width()));
            retSize.setHeight(GD_BOTTOM_ROW_HEIGHT_PROPORTION(desktopSize.height()));
            break;

        case gdCallStackViewIndex:
            retSize.setWidth(GD_BOTTOM_ROW_WIDE_VIEWS_WIDTH_PROPORTION(desktopSize.width()));
            retSize.setHeight(GD_BOTTOM_ROW_HEIGHT_PROPORTION(desktopSize.height()));
            break;

        case gdMemoryViewIndex:
            retSize.setWidth(GD_SIDE_VIEWS_WIDTH_PROPORTION(desktopSize.width()));
            retSize.setHeight(GD_MAIN_VIEWS_HEIGHT_PROPORTION(desktopSize.height()));
            break;

        case gdMultiWatch1Index:
        case gdMultiWatch2Index:
        case gdMultiWatch3Index:
            retSize.setWidth(GD_SIDE_VIEWS_WIDTH_PROPORTION(desktopSize.width()));
            retSize.setHeight(GD_MAIN_VIEWS_HEIGHT_PROPORTION(desktopSize.height()));
            break;

        default:
            GT_ASSERT_EX(false, L"View creation failure");
            break;
    }

    return retSize;
}

// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::visibility
// Description: Get the initial visibility of the view
// Arguments:   int viewIndex
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
bool gwDebugViewsCreator::visibility(int viewIndex)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((viewIndex >= 0) && (viewIndex < gdDebugAmountOfViews))
    {
        switch (viewIndex)
        {
            case gdBreakpointsViewIndex:
            case gdLocalsViewIndex:
            case gdWatchViewIndex:
            case gdStateVariablesViewIndex:
            case gdAPICallsHistoryPanelIndex:
            case gdDebuggedProcessEventsViewIndex:
            case gdCallStackViewIndex:
                retVal = true;
                break;

            case gdMultiWatch1Index:
            case gdMultiWatch2Index:
            case gdMultiWatch3Index:
                retVal = false;
                break;

            default:
                retVal = false;
                break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::initiallyActive
// Description: Get the initial active/inactive status of the view
// Author:      Uri Shomroni
// Date:        27/3/2012
// ---------------------------------------------------------------------------
bool gwDebugViewsCreator::initiallyActive(int viewIndex)
{
    bool retVal = false;

    GT_IF_WITH_ASSERT((viewIndex >= 0) && (viewIndex < gdDebugAmountOfViews))
    {
        switch (viewIndex)
        {
            case gdMemoryViewIndex:
            case gdAPICallsHistoryPanelIndex:
            case gdDebuggedProcessEventsViewIndex:
                retVal = true;
                break;

            case gdStatisticsPanelIndex:
            case gdWatchViewIndex:
            case gdLocalsViewIndex:
            case gdStateVariablesViewIndex:
            case gdCallStackViewIndex:
            case gdBreakpointsViewIndex:
                retVal = false;
                break;

            default:
                retVal = false;
                break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::amountOfViewTypes
// Description: Get number of views that can be created by this creator
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
int gwDebugViewsCreator::amountOfViewTypes()
{
    return gdDebugAmountOfViews;
}

// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::createViewContent
// Description: Create the WX inner window
// Arguments:   int viewIndex
// Return Val:  bool - Success / Failure
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
bool gwDebugViewsCreator::createViewContent(int viewIndex, QWidget*& pContentQWidget, QWidget* pQParent)
{
    bool retVal = true;

    pContentQWidget = NULL;

    // Set the default minimum view height and width
    int minViewWidth = 100;
    int minViewHeight = 30;
    QSize minViewSize(minViewWidth, minViewHeight);

    switch (viewIndex)
    {

        case gdStatisticsPanelIndex:
        {
            // Create the command queues real time statistics view:
            m_pStatisticsPanel = new gdStatisticsPanel(&afProgressBarWrapper::instance(), pQParent, afMainAppWindow::instance());
            pContentQWidget = m_pStatisticsPanel;

        }
        break;

        case gdBreakpointsViewIndex:
        {
            // Create the breakpoints view:
            m_pBreakpointsView = new gdBreakpointsView(pQParent);

            pContentQWidget = m_pBreakpointsView;
        }
        break;

        case gdLocalsViewIndex:
        {
            // Create the locals view:
            m_pLocalsView = new gdLocalsView(pQParent);

            pContentQWidget = m_pLocalsView;
        }
        break;

        case gdWatchViewIndex:
        {
            // Create the watch view:
            m_pWatchView = new gdWatchView(pQParent);

            pContentQWidget = m_pWatchView;
        }
        break;

        case gdStateVariablesViewIndex:
        {
            // Create the state variables view:
            m_pStateVariablesView = new gdStateVariablesView(pQParent);
            pContentQWidget = m_pStateVariablesView;

        }
        break;

        case gdAPICallsHistoryPanelIndex:
        {
            // Create the API calls history view:
            m_pAPICallsHistoryPanel = new gdAPICallsHistoryPanel(&afProgressBarWrapper::instance(), pQParent, true);

            pContentQWidget = m_pAPICallsHistoryPanel;
            break;
        }

        case gdDebuggedProcessEventsViewIndex:
        {
            // Create the call stack view:
            m_pDebuggedProcessEventsView = new gdDebuggedProcessEventsView(pQParent);

            pContentQWidget = m_pDebuggedProcessEventsView;
        }
        break;

        case gdCallStackViewIndex:
        {
            // Check if the properties view was already created:
            gdApplicationCommands* pApplicationCommands = gdApplicationCommands::gdInstance();
            GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
            {
                // Create the call stack view:
                m_pCallStackView = new gdCallStackView(pQParent);

                pContentQWidget = m_pCallStackView;
            }
        }
        break;

        case gdMemoryViewIndex:
        {
            // Create the command queues real time statistics view:
            m_pMemoryView = new gdMemoryView(&afProgressBarWrapper::instance(), pQParent);

            pContentQWidget = m_pMemoryView;
        }
        break;

        case gdMultiWatch1Index:
        {
            // Create the API calls history view:
            m_pMultiWatchView1 = new gdMultiWatchView(pQParent, &afProgressBarWrapper::instance());

            m_pMultiWatchView1->initialize(minViewSize);
            pContentQWidget = m_pMultiWatchView1;
        }
        break;

        case gdMultiWatch2Index:
        {
            // Create the API calls history view:
            m_pMultiWatchView2 = new gdMultiWatchView(pQParent, &afProgressBarWrapper::instance());

            m_pMultiWatchView2->initialize(minViewSize);
            pContentQWidget = m_pMultiWatchView2;
        }
        break;

        case gdMultiWatch3Index:
        {
            // Create the API calls history view:
            m_pMultiWatchView3 = new gdMultiWatchView(pQParent, &afProgressBarWrapper::instance());

            m_pMultiWatchView3->initialize(minViewSize);
            pContentQWidget = m_pMultiWatchView3;
        }
        break;

        default:
        {
            GT_ASSERT_EX(false, L"View creation failure");
            retVal = false;
        }
        break;
    }

    // Set the created window:
    m_viewsCreated.push_back(pContentQWidget);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::handleTrigger
// Description: Handle the action when it is triggered
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
void gwDebugViewsCreator::handleTrigger(int viewIndex, int actionIndex)
{
    // Get the main application window:
    afMainAppWindow* pAppMainWindow = afMainAppWindow::instance();

    // Get the current focused widget:
    GT_IF_WITH_ASSERT(pAppMainWindow != NULL)
    {
        // Get the focused widget:
        QWidget* pFocusWidget = pAppMainWindow->findFocusedWidget();

        // Check if this is an acListCtrl (if we got here, the focused widget should be a list control):
        acListCtrl* pListCtrl = qobject_cast<acListCtrl*> (pFocusWidget);
        acVirtualListCtrl* pVirtualListCtrl = qobject_cast<acVirtualListCtrl*> (pFocusWidget);

        // We should not get here if the active view is not a afSourceCodeView:
        if (pListCtrl != NULL)
        {
            // Handle list control action:
            handleListControlAction(pListCtrl, actionIndex);
        }

        // We should not get here if the active view is not a afSourceCodeView:
        else if (pVirtualListCtrl != NULL)
        {
            // Handle list control action:
            handleVirtualListControlAction(pVirtualListCtrl, actionIndex);
        }

        // Statistics:
        else if (viewIndex == gdStatisticsPanelIndex)
        {
            // One of the components of the statistics view:
            GT_IF_WITH_ASSERT(m_pStatisticsPanel != NULL)
            {
                // Get the command id:
                int commandId = actionIndexToCommandId(actionIndex);

                // Check if this command id is enabled for the current focused widget:
                bool rc = m_pStatisticsPanel->triggerCommand(pFocusWidget, commandId);
                GT_ASSERT(rc);
            }
        }
        else if (viewIndex == gdMemoryViewIndex)
        {
            // One of the components of the statistics view:
            GT_IF_WITH_ASSERT(m_pMemoryView != NULL)
            {
#pragma message ("TODO:Handle memory actions")
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::handleUiUpdate
// Description: Handle UI update
// Arguments:   int actionIndex
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
void gwDebugViewsCreator::handleUiUpdate(int viewIndex, int actionIndex)
{
    (void)(viewIndex); // unused
    bool isActionEnabled = false, isActionChecked = false, isActionCheckable = false;

    // Get the main window:
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();
    GT_IF_WITH_ASSERT(pMainWindow != NULL)
    {
        // Get the current focused widget:
        QWidget* pFocusWidget = pMainWindow->findFocusedWidget();
        GT_IF_WITH_ASSERT(pFocusWidget != NULL)
        {
            // Check if this is an acListCtrl (if we got here, the focused widget should be a list control):
            acListCtrl* pListCtrl = qobject_cast<acListCtrl*> (pFocusWidget);

            // We should not get here if the active view is not a afSourceCodeView:
            if (pListCtrl != NULL)
            {
                // Get the command id:
                int commandId = actionIndexToCommandId(actionIndex);

                switch (commandId)
                {

                    case ID_COPY:
                        isActionEnabled = (pListCtrl->rowCount() > 0);
                        break;

                    case ID_FIND:
                    case ID_FIND_NEXT:
                    case ID_SELECT_ALL:
                        isActionEnabled = true;
                        break;

                    default:
                        GT_ASSERT_EX(false, L"Unknown event id");
                        break;
                }
            }

            // Statistics:
            else
            {
#pragma message ("TODO: Support memory actions (Base view)")
                // One of the components of the statistics view:
                GT_IF_WITH_ASSERT(m_pStatisticsPanel != NULL)
                {
                    // Get the command id:
                    int commandId = actionIndexToCommandId(actionIndex);

                    // Check if this command id is enabled for the current focused widget:
                    isActionEnabled = m_pStatisticsPanel->isActionEnabled(commandId);
                }
            }
        }
    }

    // Sanity check:
    GT_IF_WITH_ASSERT(_pViewActionCreator)
    {
        // Get the QT action:
        QAction* pAction = _pViewActionCreator->action(actionIndex);
        GT_IF_WITH_ASSERT(pAction != NULL)
        {
            // Set the action enable / disable:
            pAction->setEnabled(isActionEnabled);

            // Set the action checkable state:
            pAction->setCheckable(isActionCheckable);

            // Set the action check state:
            pAction->setChecked(isActionChecked);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::handleListControlAction
// Description: Handle trigger of a list control action
// Arguments:   acListCtrl* pListCtrl
//              int actionIndex
// Author:      Sigal Algranaty
// Date:        19/2/2012
// ---------------------------------------------------------------------------
void gwDebugViewsCreator::handleListControlAction(acListCtrl* pListCtrl, int actionIndex)
{
    // Handle the action by its id:
    // Get the command id:
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {
        case ID_COPY:
            pListCtrl->onEditCopy();
            break;


        case ID_FIND:
        {
            // Get an instance of the main window:
            afMainAppWindow* pMainWindow = afMainAppWindow::instance();
            GT_IF_WITH_ASSERT(pMainWindow != NULL)
            {
                pMainWindow->OnFind(true);
            }
        }
        break;

        case ID_FIND_NEXT:
            // When opening the find dialog, the find next action is connected, so we're connected to the
            // relevant slot:
            break;

        case ID_SELECT_ALL:
            pListCtrl->onEditSelectAll();
            break;

        default:
        {
            GT_ASSERT_EX(false, L"Unsupported application command");
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::handleVirtualListControlAction
// Description: Handle trigger of a virtual list control action
// Arguments:   acListCtrl* pListCtrl
//              int actionIndex
// Author:      Sigal Algranaty
// Date:        19/2/2012
// ---------------------------------------------------------------------------
void gwDebugViewsCreator::handleVirtualListControlAction(acVirtualListCtrl* pVirtualListCtrl, int actionIndex)
{
    // Handle the action by its id:
    // Get the command id:
    int commandId = actionIndexToCommandId(actionIndex);

    switch (commandId)
    {

        case ID_COPY:
            pVirtualListCtrl->onEditCopy();
            break;

        case ID_FIND:
        {
            // Get an instance of the main window:
            afMainAppWindow* pMainWindow = afMainAppWindow::instance();
            GT_IF_WITH_ASSERT(pMainWindow != NULL)
            {
                pMainWindow->OnFind(true);
            }
        }
        break;

        case ID_FIND_NEXT:
            // When opening the find dialog, the find next action is connected, so we're connected to the
            // relevant slot:
            break;

        case ID_SELECT_ALL:
            // Do nothing:
            break;

        default:
        {
            GT_ASSERT_EX(false, L"Unsupported application command");
            break;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gwDebugViewsCreator::addSeparator
// Description: Should a separator be added before the view menu item?
// Arguments:   int viewIndex
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/2/2012
// ---------------------------------------------------------------------------
bool gwDebugViewsCreator::addSeparator(int viewIndex)
{
    bool retVal = false;

    switch (viewIndex)
    {

        case gdAPICallsHistoryPanelIndex:
        case gdLocalsViewIndex:
        case gdMemoryViewIndex:
        {
            retVal = true;
        }
        break;

    }

    return retVal;
}


void gwDebugViewsCreator::updateViewToolbarCommands()
{
    GT_IF_WITH_ASSERT(m_pAPICallsHistoryPanel != NULL)
    {
        m_pAPICallsHistoryPanel->updateToolbarCommands();
    }

    GT_IF_WITH_ASSERT(m_pStatisticsPanel != NULL)
    {
        m_pStatisticsPanel->updateToolbarCommands();
    }
}



