//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStatisticsViewBase.cpp
///
//==================================================================================

//------------------------------ gdStatisticsViewBase.cpp ------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

#include <AMDTGpuDebuggingComponents/Include/views/gdStatisticsViewBase.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afHTMLContent.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>


// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewItemData::gdStatisticsViewItemData
// Description: Constructor
// Return Val:
// Author:      Sigal Algranaty
// Date:        24/5/2009
// ---------------------------------------------------------------------------
gdStatisticsViewBase::gdStatisticsViewItemData::gdStatisticsViewItemData():
    _iconType(AF_ICON_NONE), _functionType(GD_GET_FUNCTIONS_INDEX), _averageAmountOfTimesCalled(0), _averagePercentOfCalls(0), _isItemAvailable(true),
    _functionName(AF_STR_Empty), _functionId(apMonitoredFunctionsAmount), _amountOfRedundantTimesCalled(0), _amountOfEffectiveTimesCalled(0),
    _totalAmountOfTimesCalled(0), _percentageOfRedundantTimesCalled(0),
    _percentageOfTimesCalled(0), _deprecatedAtVersion(AP_GL_VERSION_NONE), _removedAtVersion(AP_GL_VERSION_NONE),
    _functionTypeStr(AF_STR_Empty), _minRange(0), _maxRange(0), _itemChartColor(-1), _numOfBatches(0), _numOfVertices(0), _percentageOfBatches(0), _percentageOfVertices(0)
{

}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewItemData::gdStatisticsViewItemData
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        24/5/2009
// ---------------------------------------------------------------------------
gdStatisticsViewBase::gdStatisticsViewItemData::~gdStatisticsViewItemData()
{
}
// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::gdStatisticsViewBase
// Description: Constructor.
// Arguments:   parent - My parent window.
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
gdStatisticsViewBase::gdStatisticsViewBase(QWidget* pParent, gdStatisticsViewIndex windowIndex, const gtString& statisticsViewShortName)
    : acListCtrl(pParent),
      _pFindAction(NULL), _pFindNextAction(NULL), _pSaveStatisticsAction(NULL), _pSetBreakpointAction(NULL), _pSetBreakpointMultipleAction(NULL),
      _widestColumnIndex(-1), _initialSortColumnIndex(-1), _activeContextId(AP_NULL_CONTEXT, 0), _executionMode(AP_DEBUGGING_MODE), _isDebuggedProcessSuspended(false),
      _statisticsViewShortName(statisticsViewShortName), _windowIndex(windowIndex), _addBreakpointActions(false)
{
    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::init
// Description: Initializes the class after inherited class constructor was called
// Return Val: void
// Author:      Sigal Algranaty
// Date:        20/5/2009
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::init()
{
    // Initialize the list:
    initializeImageList();

    // Call inherit classes to initialize the columns:
    initListCtrlColumns();

    // Create the columns:
    setListCtrlColumns();

    // Set tooltips to list column headings
    setListHeadingToolTips();

    // Extend the context menu:
    extendContextMenu();

    if (_initialSortColumnIndex >= 0)
    {
        // Set the initial sorting values:
        _sortInfo._sortType = getSortTypeByViewerId(_initialSortColumnIndex);
    }
    else
    {
        // This should be set by the subclass
        GT_ASSERT(false);
        _sortInfo._sortType = getSortTypeByViewerId(0);
    }

    bool rcConnect = connect(horizontalHeader(), SIGNAL(sectionClicked(int)), this, SLOT(onColumnHeaderClick(int)));
    GT_ASSERT(rcConnect);
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::onColClick
// Description: Sort the column
// Arguments:   wxListEvent &eve
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::onColumnHeaderClick(int columnIndex)
{
    // Get sort type by viewer id:
    gdStatisticsViewSortDirection sortType = getSortTypeByViewerId(columnIndex);

    // If the column was clicked before, toggle the direction:
    if (sortType == _sortInfo._sortType)
    {
        // Reverse the order if we clicked the same column twice
        if (_sortInfo._sortOrder == Qt::AscendingOrder)
        {
            _sortInfo._sortOrder = Qt::DescendingOrder;
        }
        else
        {
            _sortInfo._sortOrder = Qt::AscendingOrder;
        }
    }
    else
    {
        // Change the sort type without changing the order:
        _sortInfo._sortType = sortType;
        _sortInfo._sortOrder = Qt::AscendingOrder;
    }

    // Sort the table items:
    sortItems(columnIndex, _sortInfo._sortOrder);
    emit columnSorted();
}
// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::~gdStatisticsViewBase
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
gdStatisticsViewBase::~gdStatisticsViewBase()
{
    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::updateFunctionCallsStatisticsList
// Description: Implement in base class
// Author:      Sigal Algranaty
// Date:        28/12/2011
// ---------------------------------------------------------------------------
bool gdStatisticsViewBase::updateFunctionCallsStatisticsList(const apStatistics& currentStatistics)
{
    (void)(currentStatistics);  // unused
    GT_ASSERT_EX(false, L"Should not get here - implement in base");
    return false;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::setListCtrlColumns
// Description: Create the two Columns
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::setListCtrlColumns()
{
    // Create the Statistics View columns:
    QStringList headers;

    // Add the columns to the viewer:
    for (int i = 0; i < (int)_listControlColumnTitles.size(); i++)
    {
        // Set the column text:
        headers << _listControlColumnTitles[i];
    }

    // Initialize the headers:
    initHeaders(headers, _listControlColumnWidths, false);
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::extendContextMenu
// Description: Extend the context menu for the list control
// Author:      Sigal Algranaty
// Date:        27/12/2011
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::extendContextMenu()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pContextMenu != NULL)
    {
        // Find the "Select all" action:
        // Add a separator after the first (copy) action:
        if (m_pSelectAllAction != NULL)
        {
            m_pContextMenu->insertSeparator(m_pSelectAllAction);
        }

        // Insert find + find next actions:
        _pFindAction = new QAction(AF_STR_FindA, m_pContextMenu);

        // Add the action to the context menu:
        m_pContextMenu->insertAction(m_pSelectAllAction, _pFindAction);
        bool rcConnect = connect(_pFindAction, SIGNAL(triggered()), this, SLOT(onFind()));
        GT_ASSERT(rcConnect);

        // Insert find + find next actions:
        _pFindNextAction = new QAction(AF_STR_FindNextA, m_pContextMenu);

        // Add the action to the context menu:
        m_pContextMenu->insertAction(m_pSelectAllAction, _pFindNextAction);
        rcConnect = connect(_pFindNextAction, SIGNAL(triggered()), this, SLOT(onFindNext()));
        GT_ASSERT(rcConnect);

        // Add another separator:
        m_pContextMenu->insertSeparator(m_pSelectAllAction);

        // Insert separator:
        m_pContextMenu->addSeparator();

        if (_addBreakpointActions)
        {
            // Insert separator after the select all command:
            m_pContextMenu->insertSeparator(m_pSelectAllAction);

            // Get the text for the breakpoint action:
            bool isEnabled, isChecked;
            QString bpName;
            onUpdateSetSingleFunctionAsBreakpoint(isEnabled, isChecked, bpName);

            // Create the action for the breakpoint:
            _pSetBreakpointAction = new QAction(bpName, m_pContextMenu);
            _pSetBreakpointAction->setCheckable(true);

            // Add the action to the menu:
            m_pContextMenu->insertAction(_pSaveStatisticsAction, _pSetBreakpointAction);

            // Create the action for the breakpoint:
            _pSetBreakpointMultipleAction = new QAction(GD_STR_BreakpointsBreakOnAllSelectedFunctions, m_pContextMenu);

            // Add the action to the menu:
            m_pContextMenu->insertAction(_pSaveStatisticsAction, _pSetBreakpointMultipleAction);

            // Connect the action:
            bool rc = connect(_pSetBreakpointAction, SIGNAL(triggered()), this, SLOT(onSetSingleFunctionAsBreakpoint()));
            GT_ASSERT(rc);

            rc = connect(_pSetBreakpointMultipleAction, SIGNAL(triggered()), this, SLOT(onSetMultipleFunctionsAsBreakpoints()));
            GT_ASSERT(rc);
        }

        // Build a string for the export command:
        QString exportStr = QString(GD_STR_StatisticsViewerExportStatistics).arg(acGTStringToQString(_statisticsViewShortName));
        _pSaveStatisticsAction = m_pContextMenu->addAction(exportStr, this, SLOT(onSaveStatisticsData()));

        // Add another separator:
        m_pContextMenu->insertSeparator(_pSaveStatisticsAction);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::onAboutToShowContextMenu
// Description: Set enable disable and command name for the actions
// Author:      Sigal Algranaty
// Date:        28/12/2011
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::onAboutToShowContextMenu()
{
    // Call the base class to update its actions:
    acListCtrl::onAboutToShowContextMenu();

    bool isEnabled = false, isChecked = false;
    QString actionName;

    if (_pFindAction != NULL)
    {
        onUpdateFind(isEnabled);
        _pFindAction->setEnabled(isEnabled);
    }

    if (_pFindNextAction != NULL)
    {
        onUpdateFindNext(isEnabled);
        _pFindNextAction->setEnabled(isEnabled);
    }

    if (_pSaveStatisticsAction != NULL)
    {
        onUpdateSaveStatisticsData(isEnabled);
        _pSaveStatisticsAction->setEnabled(isEnabled);
    }

    if (_pSetBreakpointMultipleAction != NULL)
    {
        onUpdateSetMultipleFunctionAsBreakpoint(isEnabled);
        _pSetBreakpointMultipleAction->setEnabled(isEnabled);
        _pSetBreakpointMultipleAction->setVisible(isEnabled);
    }

    if (_pSetBreakpointAction != NULL)
    {
        bool isEnabledSingle = false;
        onUpdateSetSingleFunctionAsBreakpoint(isEnabledSingle, isChecked, actionName);
        _pSetBreakpointAction->setEnabled(!isEnabled && isEnabledSingle);
        _pSetBreakpointAction->setChecked(isChecked);
        _pSetBreakpointAction->setText(actionName);
        _pSetBreakpointAction->setVisible(!isEnabled && isEnabledSingle);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::onSaveStatisticsData
// Description: Export the total statistics data to a file
// Arguments: wxCommandEvent& event
// Return Val: void
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::onSaveStatisticsData()
{
    // Build the file name:
    gtString fileName;
    gtString shortNameWithNoSpaces = _statisticsViewShortName;
    shortNameWithNoSpaces.removeChar(' ');
    fileName.appendFormattedString(GD_STR_saveStatisticsFileName, shortNameWithNoSpaces.asCharArray());

    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
        // Show the save file as dialog:
        QString csvFilePathStr;
        bool rc = pApplicationCommands->ShowQTSaveCSVFileDialog(csvFilePathStr, saveStatisticsDataFileName(), this);

        if (rc)
        {
            // Get the project name:
            gtString projectName = afProjectManager::instance().currentProjectSettings().projectName();

            // Write the string to a file:
            gtString statisticsFileDescription;
            statisticsFileDescription.appendFormattedString(GD_STR_StatisticsViewerFileDescription, _statisticsViewShortName.asCharArray());
            rc = writeListDataToFile(acQStringToGTString(csvFilePathStr), projectName, statisticsFileDescription);
            GT_ASSERT(rc);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::onCopy
// Description: Copy list item to the clipboard
// Arguments: wxCommandEvent& event
// Return Val: void
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::onCopy()
{
    acListCtrl::onEditCopy();
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::onUpdateSaveStatisticsData(bool &isEnabled)
// Description: Check if "Save Statistics" command can be executed
// Arguments:   bool& isEnabled
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::onUpdateSaveStatisticsData(bool& isEnabled)
{
    isEnabled = rowCount() > 0;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::clearAllStatisticsItems
// Description: delete the items from the list
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::clearAllStatisticsItems()
{
    clearList();
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::resetColumnsWidth
// Description: Resets the view columns width according to the view size
// Author:      Sigal Algranaty
// Date:        20/5/2009
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::resetColumnsWidth()
{
    // Resize me:
    resize(size());
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::getSelectedItemIndex
// Description: Return selected item index
// Return Val: int
// Author:      Sigal Algranaty
// Date:        25/5/2009
// ---------------------------------------------------------------------------
int gdStatisticsViewBase::getSelectedItemIndex()
{
    int retVal = -1;
    QList<QTableWidgetItem*> selected = selectedItems();

    if (!selected.isEmpty())
    {
        QTableWidgetItem* pFirst = selected.first();
        GT_IF_WITH_ASSERT(pFirst != NULL)
        {
            retVal = pFirst->row();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::getSelectedFunctionName
// Description: Return selected function name
// Arguments: gtString& functionName
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/5/2009
// ---------------------------------------------------------------------------
bool gdStatisticsViewBase::getSelectedFunctionName(gtString& functionName)
{
    bool retVal = false;

    // Get selected item index:
    int selectedItemIndex = getSelectedItemIndex();

    // Get item data for this item:
    if ((selectedItemIndex >= 0) && (selectedItemIndex < rowCount() - 1))
    {
        gdStatisticsViewItemData* pItemData = (gdStatisticsViewItemData*)getItemData(selectedItemIndex);

        if (pItemData != NULL)
        {
            functionName = pItemData->_functionName;
            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::getItemIconType
// Description: Return the icon type for an item
// Arguments: int itemIndex
//            afIconType& iconType
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/5/2009
// ---------------------------------------------------------------------------
bool gdStatisticsViewBase::getItemIconType(int itemIndex, afIconType& iconType)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((itemIndex < rowCount() - 1) && (itemIndex >= 0))
    {
        // Get item data for this item:
        gdStatisticsViewItemData* pItemData = gdStatisticsViewBase::getItemData(itemIndex);
        GT_IF_WITH_ASSERT(pItemData != NULL)
        {
            iconType = pItemData->_iconType;
            retVal = true;
        }
    }
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::isItemSelected
// Description: Check if this item is currently selected
// Arguments: int itemIndex
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/5/2009
// ---------------------------------------------------------------------------
bool gdStatisticsViewBase::isItemSelected(int itemIndex)
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT((itemIndex < rowCount()) && (itemIndex >= 0))
    {
        // Get the item:
        QTableWidgetItem* pItem = item(itemIndex, 0);
        GT_IF_WITH_ASSERT(pItem != NULL)
        {
            retVal = pItem->isSelected();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::addEmptyListItem
// Description: Add an item to the list stating that the list is empty
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        3/7/2011
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::addEmptyListItem()
{
    QStringList emptyList;

    for (int i = 0 ; i < columnCount(); i++)
    {
        emptyList << "";
    }

    addRow(emptyList, NULL);
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::isCurrentFunctionExistAsBreakpoint
// Description: The function checks if the current selected function is set as breakpoint.
//              Note that this function assumes that only a single function is selected.
// Arguments: funcName - the current selected function name
//            funcId - the current selected function id
//            breakpointId - the breakpoint id within the persistent data manager data structure
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/2/2009
// ---------------------------------------------------------------------------
bool gdStatisticsViewBase::doesCurrentFunctionExistAsBreakpoint(QString& funcName, apMonitoredFunctionId& funcId, int& breakpointId)
{
    bool retVal = false;
    breakpointId = -1;

    // Get the current selected items:
    QList<QTableWidgetItem*> tableSelectedItems = selectedItems();

    // Check if the selected function is a set breakpoint, and set the text accordingly:
    if (tableSelectedItems.size() > 0)
    {
        QTableWidgetItem* pItem = tableSelectedItems.first();
        GT_IF_WITH_ASSERT(pItem != NULL)
        {
            // Get the current function id:
            funcName = pItem->text();

            // If the "function name" has a space, it is actually a function name and parameter
            // (eg glFoo - GL_BAR) so truncate it.
            int firstSpacePosition = funcName.indexOf(' ', 0);

            if (firstSpacePosition != -1)
            {
                funcName.truncate(firstSpacePosition - 1);
            }

            // Get the current function name:
            gtString gtfuncName = acQStringToGTString(funcName);
            bool rc = gaGetMonitoredFunctionId(gtfuncName, funcId);
            GT_IF_WITH_ASSERT(rc)
            {
                // Get the amount of active breakpoints:
                int amountOfBreakpoints = 0;
                bool rc1 = gaGetAmountOfBreakpoints(amountOfBreakpoints);
                GT_IF_WITH_ASSERT(rc1)
                {
                    // Iterate on the active breakpoints
                    for (int i = 0; i < amountOfBreakpoints; i++)
                    {
                        // Get the current breakpoint
                        gtAutoPtr<apBreakPoint> aptrBreakpoint;
                        bool rc2 = gaGetBreakpoint(i, aptrBreakpoint);
                        GT_IF_WITH_ASSERT(rc2)
                        {
                            // Get the breakpoint type:
                            osTransferableObjectType curentBreakpointType = aptrBreakpoint->type();

                            if (curentBreakpointType == OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT)
                            {
                                // Down cast it to apMonitoredFunctionBreakPoint:
                                apMonitoredFunctionBreakPoint* pFunctionBreakpoint = (apMonitoredFunctionBreakPoint*)(aptrBreakpoint.pointedObject());
                                GT_IF_WITH_ASSERT(pFunctionBreakpoint != NULL)
                                {
                                    // Get the breakpoint's function id:
                                    if (funcId == pFunctionBreakpoint->monitoredFunctionId())
                                    {
                                        if (pFunctionBreakpoint->isEnabled())
                                        {
                                            retVal = true;
                                        }

                                        breakpointId = i;
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

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::onSetSingleFunctionAsBreakpoint
// Description: Is called when the user clicks the "Break on: glXxxx" context menu item
// Author:      Sigal Algranaty
// Date:        25/2/2009
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::onSetSingleFunctionAsBreakpoint()
{
    // Check if the action is checked:
    bool shouldBreak = false;
    QAction* pAction = qobject_cast<QAction*>(sender());
    GT_IF_WITH_ASSERT(pAction != NULL)
    {
        shouldBreak = pAction->isChecked();
    }

    gtString funcName;
    apMonitoredFunctionId funcId = apMonitoredFunctionsAmount;

    // Get the function id from the list:
    foreach (QTableWidgetItem* pItem, selectedItems())
    {
        GT_IF_WITH_ASSERT(pItem != NULL)
        {
            gdStatisticsViewItemData* pListItemSelectedData = gdStatisticsViewBase::getItemData(pItem->row());

            if (pListItemSelectedData != NULL)
            {
                funcId = pListItemSelectedData->_functionId;

                // Create new breakpoint:
                apMonitoredFunctionBreakPoint breakpoint;
                breakpoint.setMonitoredFunctionId(funcId);

                // Enable / Disable the breakpoint:
                breakpoint.setEnableStatus(shouldBreak);

                // Set the breakpoint:
                bool rc = gaSetBreakpoint(breakpoint);
                GT_ASSERT(rc);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStateChangeStatisticsView::onSetMultipleFunctionsAsBreakpoints
// Description: Is called when the user clicks the "Break on all selected functions" context menu item
// Author:      Uri Shomroni
// Date:        27/8/2008
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::onSetMultipleFunctionsAsBreakpoints()
{
    // Get the selected items:
    QModelIndexList selected = selectedIndexes();

    foreach (QModelIndex index, selected)
    {
        // Get the current function string:
        QString qfuncName;
        bool rc = getItemText(index.row(), 0, qfuncName);
        GT_IF_WITH_ASSERT(rc)
        {
            // Get the current function id:
            gtString funcName;
            funcName.fromASCIIString(qfuncName.toLatin1());

            // If the "function name" has a space, it is actually a function name and parameter
            // (eg glFoo - GL_BAR) so truncate it.
            int firstSpacePosition = funcName.find(' ', 0);

            if (firstSpacePosition != -1)
            {
                funcName.truncate(0, firstSpacePosition - 1);
            }

            apMonitoredFunctionId funcId = apMonitoredFunctionsAmount;
            bool rcFuncId = gaGetMonitoredFunctionId(funcName, funcId);

            GT_IF_WITH_ASSERT(rcFuncId)
            {
                apMonitoredFunctionBreakPoint breakpoint;
                breakpoint.setMonitoredFunctionId(funcId);
                bool rcBreak = gaSetBreakpoint(breakpoint);
                GT_ASSERT(rcBreak);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::onUpdateSetSingleFunctionAsBreakpoint
// Description: Is called when wx wants to update the "Break on: glXxxx" context menu item
// Author:      Sigal Algranaty
// Date:        25/2/2009
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::onUpdateSetSingleFunctionAsBreakpoint(bool& isEnabled, bool& isChecked, QString& commandName)
{
    QString funcName;
    apMonitoredFunctionId funcId = apMonitoredFunctionsAmount;
    int breakpointId = -1;

    isChecked = doesCurrentFunctionExistAsBreakpoint(funcName, funcId, breakpointId);
    commandName = QString(GD_STR_BreakpointsBreakOn).arg(funcName);

    // Set the item state and text:
    isEnabled = !funcName.isEmpty();
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::onUpdateSetMultipleFunctionAsBreakpoint
// Description: Enable if there are more then one item selected
// Arguments:   bool& isEnabled
// Author:      Sigal Algranaty
// Date:        28/12/2011
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::onUpdateSetMultipleFunctionAsBreakpoint(bool& isEnabled)
{
    // Count the rows selected:
    int amountOfRowsSelected = selectedItems().size() / columnCount();

    isEnabled = (amountOfRowsSelected > 1);
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::getItemData
// Description:
// Arguments:   int itemIndex
// Return Val:  gdStatisticsViewItemData*
// Author:      Sigal Algranaty
// Date:        25/12/2011
// ---------------------------------------------------------------------------
gdStatisticsViewBase::gdStatisticsViewItemData* gdStatisticsViewBase::getItemData(int itemIndex)
{
    gdStatisticsViewItemData* pRetVal = NULL;
    pRetVal = (gdStatisticsViewItemData*)acListCtrl::getItemData(itemIndex);
    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::onFind
// Description: Is handling the event for gdBaseView
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::onFind()
{
    afMainAppWindow* pMainWindow = afMainAppWindow::instance();

    GT_IF_WITH_ASSERT(NULL != pMainWindow)
    {
        pMainWindow->OnFind(true);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::onFindNext
// Description: Is handling the event for gdBaseView
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::onFindNext()
{
    acListCtrl::onFindNext();
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::onUpdateCopy
// Description: Is handling the event for gdBaseView
// Arguments:   bool& isEnabled
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::onUpdateCopy(bool& isEnabled)
{
    acListCtrl::onUpdateEditCopy(isEnabled);
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::onUpdateSelectAll
// Description: Is handling the event for gdBaseView
// Arguments:   bool& isEnabled
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::onUpdateSelectAll(bool& isEnabled)
{
    acListCtrl::onUpdateEditSelectAll(isEnabled);
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::onUpdateFind
// Description: Is handling the event for gdBaseView
// Arguments:   bool& isEnabled
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::onUpdateFind(bool& isEnabled)
{
    acListCtrl::onUpdateEditSelectAll(isEnabled);
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::onUpdateFindNext
// Description: Is handling the event for gdBaseView
// Arguments:   bool& isEnabled
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::onUpdateFindNext(bool& isEnabled)
{
    acListCtrl::onUpdateEditSelectAll(isEnabled);
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::onSelectAll
// Description: Is handling the event for gdBaseView
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::onSelectAll()
{
    acListCtrl::onEditSelectAll();
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::onEvent
// Description: Event must be implemented since it is virtual in apIEventsObserver
//              but for the time, it does nothing
// Arguments: const apEvent& eve
// Author:      Sigal Algranaty
// Date:        14/5/2009
// ---------------------------------------------------------------------------
void gdStatisticsViewBase::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent);  // unused
    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    switch (eventType)
    {

        case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
        {
            // Mark that the debugged process is suspended
            _isDebuggedProcessSuspended = true;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        case apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE:
        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        {
            _isDebuggedProcessSuspended = false;
        }
        break;

        default:
        {
            // Do nothing:
            break;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::getSortTypeByViewerId
// Description: Return the first sort type by viewer id
// Return Val: gdStatisticsViewSortDirection
// Author:      Sigal Algranaty
// Date:        20/5/2009
// ---------------------------------------------------------------------------
gdStatisticsViewSortDirection gdStatisticsViewBase::getSortTypeByViewerId(int columnIndex)
{

    gdStatisticsViewSortDirection retVal = GD_STATISTICS_SORT_NONE;

    // Translate the column index to a sort type:
    int firstColumnId = GD_BATCH_STATISTICS_SORT_BY_RANGE;

    switch (_windowIndex)
    {
        case GD_STATISTICS_VIEW_TOTAL_INDEX:
            firstColumnId = GD_TOTAL_STATISTICS_SORT_BY_TYPE;
            break;

        case GD_STATISTICS_VIEW_FUNCTION_CALLS_INDEX:
            firstColumnId = GD_SORT_FUNCTION_CALLS_STATISTICS_BY_NAME;
            break;

        case GD_STATISTICS_VIEW_DEPRECATION_INDEX:
            firstColumnId = GD_SORT_DEPRECATION_STATISTICS_BY_NAME;
            break;

        case GD_STATISTICS_VIEW_BATCH_INDEX:
            firstColumnId = GD_BATCH_STATISTICS_SORT_BY_RANGE;
            break;

        case GD_STATISTICS_VIEW_FUNCTION_CALLS_HISTORY_INDEX:
            firstColumnId = GD_STATISTICS_SORT_NONE;
            break;

        default:
            GT_ASSERT(0);
    }

    int clickedColumnSortType = firstColumnId + columnIndex;
    retVal = (gdStatisticsViewSortDirection)clickedColumnSortType;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::getItemChartColor
// Description: Return the color index of the item when first added to chart
// Return Val: int
// Author:      Sigal Algranaty
// Date:        24/5/2009
// ---------------------------------------------------------------------------
long gdStatisticsViewBase::getItemChartColor(int itemIndex)
{
    long retVal = 0;

    // Get item data:
    gdStatisticsViewItemData* pItemData = getItemData(itemIndex);
    GT_IF_WITH_ASSERT(pItemData != NULL)
    {
        // Get the item color:
        retVal = pItemData->_itemChartColor;
    }
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::setItemChartColor
// Description: Sets the color index of the item when first added to chart
// Author:      Sigal Algranaty
// Date:        24/5/2009
// ---------------------------------------------------------------------------
bool gdStatisticsViewBase::setItemChartColor(int itemIndex, long itemColor)
{
    bool retVal = false;

    // Get item data:
    gdStatisticsViewItemData* pItemData = getItemData(itemIndex);

    if (pItemData != NULL)
    {
        // Set the item chart color:
        pItemData->_itemChartColor = itemColor;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::getItemChartColor
// Description: Is used for calculation of item chart color
// Arguments: int itemIndex
//            int& amountOfCurrentItemsForColorSelection
//            bool useSavedColors
//            unsigned long& color
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        26/5/2009
// ---------------------------------------------------------------------------
bool gdStatisticsViewBase::getItemChartColor(int itemIndex, int& amountOfCurrentItemsForColorSelection, bool useSavedColors, unsigned long& color)
{
    bool retVal = false;

    // Check if this item color was already set:
    if (useSavedColors)
    {
        long itemColor = getItemChartColor(itemIndex);

        if (itemColor >= 0)
        {
            retVal = true;
        }
    }

    // If saved color was not found:
    if (!retVal)
    {
        // Get amount of colors:
        int amountOfColors = _chartColors.size() - 1;

        // Get the item color index:
        int colorIndex = amountOfCurrentItemsForColorSelection % amountOfColors;

        // Avoid the situation where the first and last pie "slices" have the same color;
        if ((colorIndex == 0) && (itemIndex == (amountOfCurrentItemsForColorSelection - 1)))
        {
            colorIndex++;
        }

        // Get the color:
        if ((colorIndex > ((int)_chartColors.size() - 1)) || (colorIndex < 0))
        {
            GT_ASSERT(false);
            colorIndex = amountOfCurrentItemsForColorSelection % amountOfColors;
        }

        // Get the color:
        color = _chartColors[colorIndex];

        // Save the item chart color:
        setItemChartColor(itemIndex, color);
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::functionTypeToString
// Description: Translate a function type to a string
// Arguments: gdFuncCallsViewTypes _functionType
// Return Val: gtString
// Author:      Sigal Algranaty
// Date:        24/6/2009
// ---------------------------------------------------------------------------
gtString gdStatisticsViewBase::functionTypeToString(gdFuncCallsViewTypes functionType)
{
    gtString retVal;

    switch (functionType)
    {
        case GD_GET_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerGetFunctions;
            break;

        case GD_REDUNDANT_STATE_CHANGE_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerRedundantStateChangeFunctions;
            break;

        case GD_DEPRECATED_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerDeprecatedFunctions;
            break;

        case GD_EFFECTIVE_STATE_CHANGE_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerEffectiveStateChangeFunctions;
            break;

        case GD_STATE_CHANGE_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerStateChangeFunctions;
            break;

        case GD_DRAW_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerDrawFunctions;
            break;

        case GD_RASTER_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerRasterFunctions;
            break;

        case GD_PROGRAM_AND_SHADERS_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerProgramsAndShadersFunctions;
            break;

        case GD_PROGRAM_AND_KERNELS_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerProgramsAndKernelsFunctions;
            break;

        case GD_TEXTURE_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerTextureFunctions;
            break;

        case GD_MATRIX_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerMatrixFunctions;
            break;

        case GD_NAME_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerNameFunctions;
            break;

        case GD_QUERY_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerQueryFunctions;
            break;

        case GD_BUFFER_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerBufferFunctions;
            break;

        case GD_BUFFERS_AND_IMAGE_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerBufferImageFunctions;
            break;

        case GD_QUEUE_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerQueueFunctions;
            break;

        case GD_SYNC_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerSyncFunctions;
            break;

        case GD_FEEDBACK_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerFeedbackFunctions;
            break;

        case GD_VERTEX_ARRAY_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerVertexArrayFunctions;
            break;

        case GD_DEBUG_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerDebugFunctions;
            break;

        case GD_CL_NULL_CONTEXT_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerCLNullContextFunctions;
            break;

        case GD_GL_NULL_CONTEXT_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerGLNullContextFunctions;
            break;

        case GD_CL_CONTEXT_BOUND_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerCLContextBoundFunctions;
            break;

        case GD_GL_CONTEXT_BOUND_FUNCTIONS_INDEX:
            retVal = GD_STR_TotalStatisticsViewerGLContextBoundFunctions;
            break;

        default:
            // we got an unexpected value:
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::icon
// Description: Get the icon for the requested index
// Arguments:   int iconIndex
// Return Val:  QPixmap*
// Author:      Sigal Algranaty
// Date:        25/12/2011
// ---------------------------------------------------------------------------
QPixmap* gdStatisticsViewBase::icon(int iconIndex)
{
    QPixmap* pRetVal = NULL;

    if (iconIndex >= 0)
    {
        GT_IF_WITH_ASSERT(iconIndex < (int)_listIconsVec.size())
        {
            pRetVal = _listIconsVec[iconIndex];
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::allocateNewWidgetItem
// Description: Override the standard widget item
// Arguments:   const QString& text
// Return Val:  QTableWidgetItem*
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
QTableWidgetItem* gdStatisticsViewBase::allocateNewWidgetItem(const QString& text)
{
    // Allocate my own widget item:
    return new gdStatisticsViewBase::gdStatisticsTableWidgetItem(text, this);
}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsTableWidgetItem::gdStatisticsTableWidgetItem
// Description: Constructor
// Arguments:   const QString& text
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
gdStatisticsViewBase::gdStatisticsTableWidgetItem::gdStatisticsTableWidgetItem(const QString& text, gdStatisticsViewBase* pParent)
    : QTableWidgetItem(text), _pParent(pParent)
{

}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsTableWidgetItem::~gdStatisticsTableWidgetItem
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
gdStatisticsViewBase::gdStatisticsTableWidgetItem::~gdStatisticsTableWidgetItem()
{

}

// ---------------------------------------------------------------------------
// Name:        gdStatisticsTableWidgetItem::operator<
// Description: Override for sorting
// Arguments:   const QTableWidgetItem & other
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
bool gdStatisticsViewBase::gdStatisticsTableWidgetItem::operator<(const QTableWidgetItem& other) const
{
    bool retVal = false;

    // Sanity check:
    GT_IF_WITH_ASSERT(_pParent != NULL)
    {
        // Get both item data:
        gdStatisticsViewItemData* pMyItemData = _pParent->getItemData(row());
        gdStatisticsViewItemData* pOtherItemData = _pParent->getItemData(other.row());

        // Compare both items:
        retVal = _pParent->isItemSmallerThen(pMyItemData, pOtherItemData);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdStatisticsViewBase::isItemSmallerThen
// Description: Sort function. Return 1 if item1 is bigger then the other in ascending order
//              -1 if smaller, 0 for equal
// Arguments:   gdStatisticsViewItemData* pItemData1
//              gdStatisticsViewItemData* pItemData1
// Return Val:  int
// Author:      Sigal Algranaty
// Date:        29/12/2011
// ---------------------------------------------------------------------------
bool gdStatisticsViewBase::isItemSmallerThen(gdStatisticsViewItemData* pItemData1, gdStatisticsViewItemData* pItemData2)
{
    int retVal = 0;

    // If one of our items has a NULL data, it means that it is one of the total items.
    // Notice: since we want the total item to be last both in ascending and descending sort order,
    // we 'cheat' and return false / true according to the current sort order:
    if ((pItemData1 == 0) && (pItemData2 == 0))
    {
        retVal = false;
    }
    else if ((pItemData1 == 0) && (pItemData2 != 0))
    {
        if (_sortInfo._sortOrder == Qt::AscendingOrder)
        {
            retVal = false;
        }
        else
        {
            retVal = true;
        }

    }
    else if ((pItemData1 != 0) && (pItemData2 == 0))
    {
        if (_sortInfo._sortOrder == Qt::AscendingOrder)
        {
            retVal = true;
        }
        else
        {
            retVal = false;
        }
    }
    else
    {
        switch (_sortInfo._sortType)
        {
            case GD_TOTAL_STATISTICS_SORT_BY_TYPE:
                retVal = (pItemData1->_functionType < pItemData2->_functionType);
                break;

            case GD_TOTAL_STATISTICS_SORT_BY_CALLS_AMOUNT:
            case GD_SORT_STATE_CHANGE_STATISTICS_BY_TOTAL_CALLS_AMOUNT:
            case GD_SORT_FUNCTION_CALLS_STATISTICS_BY_CALLS_AMOUNT:
            case GD_SORT_DEPRECATION_STATISTICS_BY_TOTAL_CALLS_AMOUNT:
                retVal = (pItemData1->_totalAmountOfTimesCalled < pItemData2->_totalAmountOfTimesCalled);
                break;

            case GD_TOTAL_STATISTICS_SORT_BY_CALLS_PERCENT:
            case GD_SORT_FUNCTION_CALLS_STATISTICS_BY_CALLS_PERCENTAGE:
            case GD_SORT_DEPRECATION_STATISTICS_BY_CALLS_PERCENTAGE:
                retVal = (pItemData1->_percentageOfTimesCalled < pItemData2->_percentageOfTimesCalled);
                break;

            case GD_TOTAL_STATISTICS_SORT_BY_CALLS_AVERAGE_AMOUNT:
                retVal = (pItemData1->_averageAmountOfTimesCalled < pItemData2->_averageAmountOfTimesCalled);
                break;

            case GD_TOTAL_STATISTICS_SORT_BY_CALLS_AVERAGE_PERCENT:
                retVal = (pItemData1->_averagePercentOfCalls < pItemData2->_averagePercentOfCalls);
                break;

            case GD_SORT_STATE_CHANGE_STATISTICS_BY_NAME:
            case GD_SORT_FUNCTION_CALLS_STATISTICS_BY_NAME:
            case GD_SORT_DEPRECATION_STATISTICS_BY_NAME:
                retVal = (pItemData1->_functionName < pItemData2->_functionName);
                break;

            case GD_SORT_STATE_CHANGE_STATISTICS_BY_REDUNDANT_CALLS_AMOUNT:
                retVal = (pItemData1->_amountOfRedundantTimesCalled < pItemData2->_amountOfRedundantTimesCalled);
                break;

            case GD_SORT_STATE_CHANGE_STATISTICS_BY_REDUNDANT_CALLS_PERCENTAGE:
                retVal = (pItemData1->_percentageOfRedundantTimesCalled < pItemData2->_percentageOfRedundantTimesCalled);
                break;

            case GD_SORT_STATE_CHANGE_STATISTICS_BY_EFFECTIVE_CALLS_AMOUNT:
                retVal = (pItemData1->_amountOfEffectiveTimesCalled < pItemData2->_amountOfEffectiveTimesCalled);
                break;

            case GD_BATCH_STATISTICS_SORT_BY_RANGE:
                retVal = (pItemData1->_minRange < pItemData2->_minRange);
                break;

            case GD_BATCH_STATISTICS_SORT_BY_NUM_OF_BATCHES:
                retVal = (pItemData1->_numOfBatches < pItemData2->_numOfBatches);
                break;

            case GD_BATCH_STATISTICS_SORT_BY_NUM_OF_VERTICES:
                retVal = (pItemData1->_numOfVertices < pItemData2->_numOfVertices);
                break;

            case GD_BATCH_STATISTICS_SORT_BY_PERCENTAGE_OF_BATCHES:
                retVal = (pItemData1->_percentageOfBatches < pItemData2->_percentageOfBatches);
                break;

            case GD_SORT_FUNCTION_CALLS_STATISTICS_BY_TYPE:
                retVal = (pItemData1->_functionTypeStr < pItemData2->_functionTypeStr);
                break;

            case GD_BATCH_STATISTICS_SORT_BY_PERCENTAGE_OF_VERTICES:
                retVal = (pItemData1->_percentageOfVertices < pItemData2->_percentageOfVertices);
                break;

            case GD_SORT_DEPRECATION_STATISTICS_BY_DEPRECATED_VERSION:
                retVal = (pItemData1->_deprecatedAtVersion < pItemData2->_deprecatedAtVersion);
                break;

            case GD_SORT_DEPRECATION_STATISTICS_BY_REMOVED_VERSION:
                retVal = (pItemData1->_removedAtVersion < pItemData2->_removedAtVersion);
                break;

            case GD_SORT_DEPRECATION_STATISTICS_BY_DEPRACATION:
                retVal = (pItemData1->_deprecationStatus < pItemData2->_deprecationStatus);
                break;

            default:
            {
                // Unknown sort option:
                GT_ASSERT(0);
            }
            break;
        }
    }

    return retVal;
}
