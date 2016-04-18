//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdMemoryViewBase.cpp
///
//==================================================================================

//------------------------------ gdMemoryViewBase.cpp --------------------------------

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <AMDTApplicationComponents/Include/acQtIncludes.h>

#include <AMDTApplicationComponents/Include/acChartWindow.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apCLBuffer.h>
#include <AMDTAPIClasses/Include/apCLCommandQueue.h>
#include <AMDTAPIClasses/Include/apCLContext.h>
#include <AMDTAPIClasses/Include/apCLEvent.h>
#include <AMDTAPIClasses/Include/apCLImage.h>
#include <AMDTAPIClasses/Include/apCLKernel.h>
#include <AMDTAPIClasses/Include/apCLPipe.h>
#include <AMDTAPIClasses/Include/apCLProgram.h>
#include <AMDTAPIClasses/Include/apCLSampler.h>
#include <AMDTAPIClasses/Include/apGLDisplayList.h>
#include <AMDTAPIClasses/Include/apGLFBO.h>
#include <AMDTAPIClasses/Include/apGLProgram.h>
#include <AMDTAPIClasses/Include/apGLRenderBuffer.h>
#include <AMDTAPIClasses/Include/apGLPipeline.h>
#include <AMDTAPIClasses/Include/apGLSampler.h>
#include <AMDTAPIClasses/Include/apGLRenderContextInfo.h>
#include <AMDTAPIClasses/Include/apGLShaderObject.h>
#include <AMDTAPIClasses/Include/apGLSync.h>
#include <AMDTAPIClasses/Include/apGLTexture.h>
#include <AMDTAPIClasses/Include/apGLVBO.h>
#include <AMDTAPIClasses/Include/apPBuffer.h>
#include <AMDTAPIClasses/Include/apStaticBuffer.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunSuspendedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apMemoryLeakEvent.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acChartWindow.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdAllocatedObjectsCreationStackView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdDebuggedProcessEventsView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdMemoryAnalysisDetailsView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdMemoryViewBase.h>
#include <AMDTGpuDebuggingComponents/Include/gdDebugApplicationTreeData.h>

// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::gdMemoryViewBase
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        18/1/2011
// ---------------------------------------------------------------------------
gdMemoryViewBase::gdMemoryViewBase(afProgressBarWrapper* pProgressBar, gdDebugApplicationTreeHandler* pObjectsTree):
    afBaseView(pProgressBar), _pAllocatedObjectsCreationStackView(NULL), _pMemoryDetailsView(NULL),
    _pChartWindow(NULL), _updatingOnProcessSuspension(false),
    _isAutomaticallySelectingTreeItem(false), _isDebuggedProcessSuspended(false), _isInfoUpdated(false),
    _isDuringViewsUpdate(false), _isTreeUpdatedByMe(true), m_pLastSelectedTreeItemId(NULL)
{
    (void)(pObjectsTree);  // unused
    // Initialize the colors vector:
    initializeColors();

    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);

}

// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::~gdMemoryViewBase
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        22/9/2008
// ---------------------------------------------------------------------------
gdMemoryViewBase::~gdMemoryViewBase()
{
    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);

    if (_pAllocatedObjectsCreationStackView != NULL)
    {
        delete _pAllocatedObjectsCreationStackView;
        _pAllocatedObjectsCreationStackView = NULL;
    }

    if (_pChartWindow != NULL)
    {
        delete _pChartWindow;
        _pChartWindow = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::clearAllViewers
// Description: Clears all the viewers
// Author:      Sigal Algranaty
// Date:        22/7/2008
// ---------------------------------------------------------------------------
void gdMemoryViewBase::clearAllViewers(bool displayNAMessage, bool forgetLastSelection)
{
    (void)(forgetLastSelection);  // unused

    // Clear the first memory tab view:
    if (_pMemoryDetailsView != NULL)
    {
        if (displayNAMessage)
        {
            _pMemoryDetailsView->clearAndDisplayMessage();
        }
        else
        {
            _pMemoryDetailsView->clearContents();
        }
    }

    if (_pAllocatedObjectsCreationStackView != NULL)
    {
        osCallStack emptyStack;
        _pAllocatedObjectsCreationStackView->updateCallsStack(emptyStack);
    }

    // Clear the chart:
    clearPieChart();
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::updateMemoryAnalysisViewers
// Description: Update the memory viewers
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/9/2008
// ---------------------------------------------------------------------------
bool gdMemoryViewBase::updateMemoryAnalysisViewers()
{
    bool retVal = false;

    if ((!_isInfoUpdated) && (!_isDuringViewsUpdate) && _isTreeUpdatedByMe)
    {
        _isDuringViewsUpdate = true;
        GT_IF_WITH_ASSERT(gdDebugApplicationTreeHandler::instance() != NULL)
        {
            // Update the application memory analysis tree:
            gdDebugApplicationTreeHandler::instance()->setInfoUpdated(false);
            retVal = gdDebugApplicationTreeHandler::instance()->updateMonitoredObjectsTree();
        }

        _isInfoUpdated = true;

        _isDuringViewsUpdate = false;
    }
    else
    {
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::updateChart
// Description: Updates the chart
// Author:      Sigal Algranaty
// Date:        22/9/2008
// ---------------------------------------------------------------------------
void gdMemoryViewBase::updateChart(const afApplicationTreeItemData* pItemData)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((_pMemoryDetailsView != NULL) && (_pMemoryDetailsView != NULL) && (pItemData != NULL))
    {
        // If there are any items in the list and the debugged process is suspended:
        if ((_pMemoryDetailsView->rowCount() > 0) && _isDebuggedProcessSuspended)
        {
            afApplicationTreeItemData* pDisplayedItemData = NULL;

            // Find the list id to display for this item:
            afApplicationTreeItemData listId;
            gdDebugApplicationTreeData* pGDData = new gdDebugApplicationTreeData;

            listId.setExtendedData(pGDData);
            bool rcGetList = _pMemoryDetailsView->findDisplayedListIDByItemID(pItemData, listId);
            GT_IF_WITH_ASSERT(rcGetList)
            {
                // Get the "Real" item data for the item represented by the list id:
                pDisplayedItemData = gdDebugApplicationTreeHandler::instance()->FindMatchingTreeItem(listId);
                GT_IF_WITH_ASSERT(pDisplayedItemData != NULL)
                {
                    // Recreate the chart:
                    updatePieChart(pDisplayedItemData);
                }
            }
        }

        // If there's multi-selection or no selection, update the selection in the chart:
        if (_pMemoryDetailsView->amountOfSelectedRows() != 1)
        {
            highlightSelectedItemsInChart();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::onProcessRunSuspended
// Description: Is called when the debugged process run is suspended.
// Author:      Sigal Algranaty
// Date:        22/9/2008
// ---------------------------------------------------------------------------
void gdMemoryViewBase::onProcessRunSuspended(const apDebuggedProcessRunSuspendedEvent& event)
{
    (void)(event);  // unused
    // Mark that the debugged process is suspended
    _isDebuggedProcessSuspended = true;

    // If the viewer is currently visible, update it:
    if (!_isDuringViewsUpdate)
    {
        _updatingOnProcessSuspension = true;

        // Update all the viewers with the current memory analysis:
        bool rc = updateMemoryAnalysisViewers();
        GT_ASSERT_EX(rc, L"Memory analysis viewers update failed");

        _updatingOnProcessSuspension = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::onListColumnClicked
// Description:
// Arguments: wxListEvent& eve
// Return Val: void
// Author:      Sigal Algranaty
// Date:        22/9/2008
// ---------------------------------------------------------------------------
void gdMemoryViewBase::onListColumnClicked(int clickedItemIndex)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pMemoryDetailsView != NULL)
    {
        // Get the clicked item index:
        if ((clickedItemIndex >= 0) && (clickedItemIndex < _pMemoryDetailsView->rowCount()))
        {
            // Get the item data from the list:
            const afApplicationTreeItemData* pItemData = (const afApplicationTreeItemData*)_pMemoryDetailsView->getItemData(clickedItemIndex);
            GT_IF_WITH_ASSERT(pItemData != NULL)
            {
                // Recalculate the chart:
                updateChart(pItemData);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::onListItemSelected
// Description: Om memory list item selection
// Arguments: wxListEvent &eve
// Return Val: void
// Author:      Sigal Algranaty
// Date:        2/10/2008
// ---------------------------------------------------------------------------
void gdMemoryViewBase::onListItemSelected(QTableWidgetItem* pItem)
{
    if (pItem != NULL)
    {
        // Get the clicked list item:
        int selectedItemIndex = pItem->row();

        // Linux calls these events with id = -1:
        if (selectedItemIndex >= 0)
        {
            // Get the memory object item data:
            afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)(_pMemoryDetailsView->getItemData(selectedItemIndex));

            if (pItemData != NULL)
            {
                gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
                GT_IF_WITH_ASSERT(pGDItemData != NULL)
                {
                    // Only select the tree item if there is just one item selected:
                    if (_pMemoryDetailsView->amountOfSelectedRows() <= 1)
                    {
                        // Check the tree item id of the clicked object:
                        if (pItemData->m_pTreeWidgetItem != NULL)
                        {
                            // Highlight the selected items in the pie chart:
                            highlightSelectedItemsInChart();

                            // Display the selected item call stack:
                            displayItemCallStack(pItemData);

                            // Move the focus back to the list (it is taken by the selection event):
                            _pMemoryDetailsView->setFocus();

                            // Set the selected item name in the status bar:
                            QString itemText = gdDebugApplicationTreeHandler::instance()->GetTreeItemText(pItemData->m_pTreeWidgetItem);
                            setStatusText(acQStringToGTString(itemText), false);
                        }
                    }
                    else // numberOfSelectedItems > 1
                    {
                        afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
                        GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
                        {
                            afApplicationTree* pApplicationTree = pApplicationCommands->applicationTree();
                            GT_IF_WITH_ASSERT(pApplicationTree != NULL)
                            {
                                // Just update the other views:
                                pApplicationTree->treeUnselectAll();
                            }

                        }

                        // Display the size for the current selected items:
                        displaySelectedItemsMemorySize();

                        // Update the pie chart with the currently selected items:
                        highlightSelectedItemsInChart();
                    }
                }
            }
        }
    }
    else
    {
        osCallStack emptyStack;
        _pAllocatedObjectsCreationStackView->updateCallsStack(emptyStack);
    }

}


// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::onListItemDeselected
// Description:
// Arguments:   wxListEvent& eve
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        9/2/2011
// ---------------------------------------------------------------------------
void gdMemoryViewBase::onListItemDeselected(QTableWidgetItem* pItem)
{
    (void)(pItem);  // unused

    // Handle the event only if there is there is single item selected)
    if (_pMemoryDetailsView->amountOfSelectedRows() == 1)
    {
        QTableWidgetItem* pSelectedItem = _pMemoryDetailsView->selectedItems().first();
        onListItemSelected(pSelectedItem);
    }

}

// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::onListItemActivated
// Description: Double click an memory object item (select the object in the application tree view)
// Arguments: wxListEvent &eve
// Return Val: void
// Author:      Sigal Algranaty
// Date:        2/10/2008
// ---------------------------------------------------------------------------
void gdMemoryViewBase::onListItemActivated(QTableWidgetItem* pItem)
{
    if (pItem != NULL)
    {
        // Linux calls these events with id = -1:
        if (pItem->row() >= 0)
        {
            // Get the clicked item data:
            afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)(_pMemoryDetailsView->getItemData(pItem->row()));

            if (pItemData != NULL)
            {
                // Check the tree item id of the clicked object:
                if (pItemData->m_pTreeWidgetItem != NULL)
                {
                    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
                    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
                    {
                        afApplicationTree* pApplicationTree = pApplicationCommands->applicationTree();
                        GT_IF_WITH_ASSERT(pApplicationTree != NULL)
                        {
                            // If item is selected, do not select it again:
                            if (pApplicationTree->getTreeSelection() != pItemData->m_pTreeWidgetItem)
                            {
                                // Highlight the selected items in the pie chart:
                                highlightSelectedItemsInChart();

                                // Select the tree item:
                                pApplicationTree->treeSelectItem(pItemData->m_pTreeWidgetItem, true);

                                // Move the focus back to the list (it is taken by the selection event):
                                _pMemoryDetailsView->setFocus();
                            }
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::onUpdateEdit_Copy
// Description: enables the copy command in the VS edit menu
// Arguments:
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        6/2/2011
// ---------------------------------------------------------------------------
void gdMemoryViewBase::onUpdateEdit_Copy(bool& isEnabled)
{
    // Down cast to a list control object:
    acListCtrl* pListCtrl = (acListCtrl*)_pMemoryDetailsView;
    GT_IF_WITH_ASSERT(pListCtrl != NULL)
    {
        isEnabled = false;
        pListCtrl->onUpdateEditCopy(isEnabled);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::onUpdateEdit_SelectAll
// Description: enables the select all command in the VS edit menu
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        6/2/2011
// ---------------------------------------------------------------------------
void gdMemoryViewBase::onUpdateEdit_SelectAll(bool& isEnabled)
{
    // Down cast to a list control object:
    acListCtrl* pListCtrl = (acListCtrl*)_pMemoryDetailsView;
    GT_IF_WITH_ASSERT(pListCtrl != NULL)
    {
        isEnabled = false;
        pListCtrl->onUpdateEditSelectAll(isEnabled);
    }
}
// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::onEdit_Copy
// Description: execute the copy command in the VS edit menu
// Arguments:
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        6/2/2011
// ---------------------------------------------------------------------------
void gdMemoryViewBase::onEdit_Copy()
{
    if (_pMemoryDetailsView != NULL)
    {
        _pMemoryDetailsView->onEdit_Copy();
    }
}


// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::onEdit_SelectAll
// Description: execute the select all command in the VS edit menu
// Arguments:   wxCommandEvent& event
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        6/2/2011
// ---------------------------------------------------------------------------
void gdMemoryViewBase::onEdit_SelectAll()
{
    if (_pMemoryDetailsView != NULL)
    {
        _pMemoryDetailsView->onEdit_SelectAll();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::onEvent
// Description: Is called when a debugged process event occurs.
// Arguments:   const apEvent& eve - A class representing the event.
// Author:      Sigal Algranaty
// Date:        22/7/2008
// ---------------------------------------------------------------------------
void gdMemoryViewBase::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent);  // unused
    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    switch (eventType)
    {

        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        case apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE:
        {
            onProcessTerminated();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        case apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED:
        {
            onProcessStarted();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
        {
            // Update the textures and buffers viewer
            onProcessRunSuspended((const apDebuggedProcessRunSuspendedEvent&)(eve));
        }
        break;

        case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
        {
            onGlobalVariableChanged(eve);
        }
        break;

        case apEvent::GD_MONITORED_OBJECT_SELECTED_EVENT:
        {
            onTreeItemSelection((const apMonitoredObjectsTreeSelectedEvent&)eve);
            break;
        }

        case apEvent::AP_EXECUTION_MODE_CHANGED_EVENT:
        {
            onExecutionModeChangedEvent(eve);
        }
        break;

        default:
            // Do nothing...
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::onProcessTerminated
// Description: Is called when the debugged process is terminated.
// Return Val: void
// Author:      Sigal Algranaty
// Date:        22/7/2008
// ---------------------------------------------------------------------------
void gdMemoryViewBase::onProcessTerminated()
{
    // Clear all the viewers and display "not available" messages, but remember the last selection:
    clearAllViewers(true);

    // Mark that the debugged process is not suspended
    _isDebuggedProcessSuspended = false;

    // Mark that our info is out of date:
    _isInfoUpdated = false;
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::onProcessStarted
// Description: Handles process started event - clears the spy memory, and
//              also clears the viewers lists
// Return Val: void
// Author:      Sigal Algranaty
// Date:        22/7/2008
// ---------------------------------------------------------------------------
void gdMemoryViewBase::onProcessStarted()
{
    // Clear all the viewers and display "not available" messages, but remember the last selection:
    clearAllViewers(true, false);

    // Mark that the debugged process is not suspended
    _isDebuggedProcessSuspended = false;

    // Mark that our info is out of date:
    _isInfoUpdated = false;
}


// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::displaySelectedItemsMemorySize
// Description: Is called when multiple items are selected - display the selected items in properties window
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/10/2008
// ---------------------------------------------------------------------------
void gdMemoryViewBase::displaySelectedItemsMemorySize()
{
    // Sanity check
    GT_IF_WITH_ASSERT((_pAllocatedObjectsCreationStackView != NULL) && (_pMemoryDetailsView != NULL))
    {
        // Get the currently selected items amount:
        int numOfItemsSelected = _pMemoryDetailsView->amountOfSelectedRows();

        // Display only when there is more then 1 selected item:
        GT_IF_WITH_ASSERT(numOfItemsSelected > 1)
        {
            int selectionMemorySize = 0;
            QList<int> rowsCalculated;

            foreach (QTableWidgetItem* pCurrentSelectedItem, _pMemoryDetailsView->selectedItems())
            {
                if (pCurrentSelectedItem != NULL)
                {
                    if (rowsCalculated.indexOf(pCurrentSelectedItem->row()) < 0)
                    {
                        // Get the currently selected item data:
                        afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)(_pMemoryDetailsView->getItemData(pCurrentSelectedItem->row()));

                        if (pItemData != NULL)
                        {
                            gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
                            GT_IF_WITH_ASSERT(pGDData != NULL)
                            {
                                selectionMemorySize += pItemData->m_objectMemorySize;

                                // Highlight the object in the pie chart:
                                if (pGDData->_pieChartIndex >= 0)
                                {
                                    _pChartWindow->setSelection(pGDData->_pieChartIndex, true);
                                }
                            }
                        }

                        rowsCalculated << pCurrentSelectedItem->row();
                    }
                }
            }

            QString sumStr;

            if (selectionMemorySize == 0)
            {
                sumStr = GD_STR_MemoryAnalysisViewerObjectSizeInsignificantA;
            }
            else
            {
                gtString sizeInKB;
                sizeInKB.appendFormattedString(L"%d", selectionMemorySize);
                sizeInKB.addThousandSeperators();
                sizeInKB.append(AF_STR_KilobytesShort);
                sumStr.append(acGTStringToQString(sizeInKB));
            }

            osCallStack emptyStack;
            _pAllocatedObjectsCreationStackView->setEmptyCallStackString(sumStr);
            _pAllocatedObjectsCreationStackView->updateCallsStack(emptyStack);
            QStringList header;
            header << GD_STR_MemoryAnalysisViewerObjectsSize;
            _pAllocatedObjectsCreationStackView->initHeaders(header, false);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::highlightSelectedItemsInChart
// Description: Highlight the currently list's selected items in the pie chart
// Return Val: void
// Author:      Sigal Algranaty
// Date:        15/10/2008
// ---------------------------------------------------------------------------
void gdMemoryViewBase::highlightSelectedItemsInChart()
{
    // Clear the chart's selections:
    _pChartWindow->clearAllSelection();

    // For each item selected - highlight in the pie chart:
    if (_pMemoryDetailsView->rowCount() > 0)
    {
        foreach (QTableWidgetItem* pSelected, _pMemoryDetailsView->selectedItems())
        {
            if (pSelected != NULL)
            {
                // Get the currently selected item data:
                afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)(_pMemoryDetailsView->getItemData(pSelected->row()));

                if (pItemData != NULL)
                {
                    gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());

                    if (pGDItemData != NULL)
                    {
                        // Highlight the object in the pie chart:
                        if (pGDItemData->_pieChartIndex >= 0)
                        {
                            _pChartWindow->setSelection(pGDItemData->_pieChartIndex, true);
                        }
                    }
                }
            }
        }
    }

    // Refresh the chart view according to the new selections:
    _pChartWindow->redrawWindow();
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::clearPieChart
// Description: Clears the pie chart
// Author:      Uri Shomroni
// Date:        10/11/2008
// ---------------------------------------------------------------------------
void gdMemoryViewBase::clearPieChart()
{
    if (_pChartWindow != NULL)
    {
        _pChartWindow->clearAllData();
        _pChartWindow->recalculateArrays();
        _pChartWindow->redrawWindow();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::initializeColors
// Description: Initializes the color vectors' values:
// Return Val: void
// Author:      Sigal Algranaty
// Date:        2/10/2008
// ---------------------------------------------------------------------------
void gdMemoryViewBase::initializeColors()
{
    unsigned long currentColor;

    // Generate colors for the functions calls graphs:
    currentColor = 0x6666FF; // Blue-Purple
    _memoryObjectsColors.push_back(currentColor);
    currentColor = 0x0066FF; // Middle Blue
    _memoryObjectsColors.push_back(currentColor);
    currentColor = 0x33CCFF; // Sea Blue
    _memoryObjectsColors.push_back(currentColor);
    currentColor = 0x99CCFF; // Light Blue
    _memoryObjectsColors.push_back(currentColor);
    currentColor = 0x99CC33; // Olive Green
    _memoryObjectsColors.push_back(currentColor);
    currentColor = 0x008800; // Dark Green
    _memoryObjectsColors.push_back(currentColor);
    currentColor = 0x00BB00; // Middle green
    _memoryObjectsColors.push_back(currentColor);
    currentColor = 0x99FF00; // Bright green
    _memoryObjectsColors.push_back(currentColor);

    // The last color is the highlight color, which isn't used on functions that aren't selected:
    currentColor = 0xFFFFFF; // White
    _memoryObjectsColors.push_back(currentColor);
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::updatePieChart
// Description: Update a memory objects pie chart display
// Arguments: gdDebugApplicationTreeData* pMemoryItemData - the updated item data
// Return Val: void
// Author:      Sigal Algranaty
// Date:        24/9/2008
// ---------------------------------------------------------------------------
void gdMemoryViewBase::updatePieChart(afApplicationTreeItemData* pMemoryItemData)
{
    GT_IF_WITH_ASSERT((pMemoryItemData != NULL) && (_pChartWindow != NULL))
    {
        // Set the pie chart:
        _pChartWindow->setChartType(AC_PIE_CHART);

        // Clear the pie chart:
        _pChartWindow->clearAllData();

        // Get the selected items tree item id:
        QTreeWidgetItem* pSelectedItemId = pMemoryItemData->m_pTreeWidgetItem;

        GT_IF_WITH_ASSERT(pSelectedItemId != NULL)
        {
            // The chart data object:
            acChartDataPoint dataPoint;

            dataPoint._isSelected = false;

            // Check if the item is a root item:
            bool isRoot = afApplicationTreeItemData::isItemTypeRoot(pMemoryItemData->m_itemType);

            // Check if the item is one of the items that are both parents and children:
            isRoot = isRoot || (pMemoryItemData->m_itemType == AF_TREE_ITEM_CL_PROGRAM);
            isRoot = isRoot || (pMemoryItemData->m_itemType == AF_TREE_ITEM_GL_FBO);
            isRoot = isRoot || (pMemoryItemData->m_itemType == AF_TREE_ITEM_GL_RENDER_CONTEXT) || (pMemoryItemData->m_itemType == AF_TREE_ITEM_CL_CONTEXT);

            bool isTreeRoot = false;

            afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
            GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
            {
                afApplicationTree* pApplicationTree = pApplicationCommands->applicationTree();
                GT_IF_WITH_ASSERT(pApplicationTree != NULL)
                {
                    // Check if the selected item is the root of the tree:
                    QTreeWidgetItem* pTreeRootItemId = pApplicationTree->getTreeRootItem();

                    if (pSelectedItemId == pTreeRootItemId)
                    {
                        isTreeRoot = true;
                    }

                    // Get the parent tree item id (the selected, or it's parent in case of leaf):
                    QTreeWidgetItem* pParentTreeItemId = pSelectedItemId;

                    if (!isRoot && !isTreeRoot)
                    {
                        pParentTreeItemId = gdDebugApplicationTreeHandler::instance()->getTreeItemParent(pSelectedItemId);
                    }

                    int colorIndex = 0;
                    int addedItemIndex = 0;

                    for (int i = 0; i < pParentTreeItemId->childCount(); i++)
                    {
                        QTreeWidgetItem* pCurrentChildItemId = pParentTreeItemId->child(i);

                        if (pCurrentChildItemId != NULL)
                        {
                            afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)(gdDebugApplicationTreeHandler::instance()->getTreeItemData(pCurrentChildItemId));

                            if (pItemData != NULL)
                            {
                                gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
                                GT_IF_WITH_ASSERT(pGDData != NULL)
                                {
                                    if ((pItemData->m_objectMemorySize > 0) && (pItemData->m_itemType != AF_TREE_ITEM_GL_PBUFFERS_NODE))
                                    {
                                        // Fill the pie chart fields:
                                        dataPoint._value = pItemData->m_objectMemorySize;
                                        dataPoint._pointColor = _memoryObjectsColors[colorIndex];
                                        bool isSelectedInList = false;
                                        int listIndex = pGDData->_listViewIndex;

                                        if ((listIndex > -1) && (listIndex < _pMemoryDetailsView->rowCount()))
                                        {
                                            QTableWidgetItem* pItem = _pMemoryDetailsView->item(listIndex, 0);
                                            isSelectedInList = _pMemoryDetailsView->isItemSelected(pItem);
                                        }

                                        dataPoint._isSelected = (!isRoot && ((pCurrentChildItemId == pSelectedItemId) || isSelectedInList));
                                        dataPoint._originalItemIndex = i;
                                        _pChartWindow->addDataPoint(dataPoint);

                                        // Set the item's pie chart index for later use:
                                        pGDData->_pieChartIndex = addedItemIndex++;

                                        // Increment the color:
                                        colorIndex = (colorIndex + 1) % (_memoryObjectsColors.size() - 1);
                                    }
                                    else
                                    {
                                        // Note that this item is not in the pie chart:
                                        pGDData->_pieChartIndex = -1;
                                    }
                                }
                            }
                        }
                    }

                    _pChartWindow->recalculateArrays();
                    _pChartWindow->setChartType(AC_PIE_CHART);
                    _pChartWindow->redrawWindow();
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdMemoryViewBase::displayItemCallStack
// Description: Display the item call stack. The function is old, but its' functionality
//              was separated from properties display.
// Arguments:   gdDebugApplicationTreeData* pMemoryItemData
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        14/10/2010
// ---------------------------------------------------------------------------
void gdMemoryViewBase::displayItemCallStack(const afApplicationTreeItemData* pItemData)
{
    // Sanity check:
    GT_IF_WITH_ASSERT((pItemData != NULL) && (_pAllocatedObjectsCreationStackView != NULL))
    {
        gdDebugApplicationTreeData* pGDItemData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());
        GT_IF_WITH_ASSERT(pGDItemData != NULL)
        {
            bool callsStackShown = false;
            QStringList header;
            header << GD_STR_MemoryAnalysisViewerObjectCreationCallStackCaption;
            _pAllocatedObjectsCreationStackView->initHeaders(header, false);

            afTreeItemType propertiesItemType = pItemData->m_itemType;

            switch (propertiesItemType)
            {

                case AF_TREE_ITEM_GL_RENDER_CONTEXT:
                {
                    apGLRenderContextInfo contextInfo;
                    bool rc = gaGetRenderContextDetails(pGDItemData->_contextId._contextId, contextInfo);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(contextInfo);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_CL_CONTEXT:
                {
                    apCLContext contextInfo;
                    bool rc = gaGetOpenCLContextDetails(pGDItemData->_contextId._contextId, contextInfo);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(contextInfo);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_GL_TEXTURE:
                {
                    apGLTexture texDetails;
                    bool rcTex = gaGetTextureObjectDetails(pGDItemData->_contextId._contextId, pGDItemData->_objectOpenGLName, texDetails);
                    GT_IF_WITH_ASSERT(rcTex)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(texDetails);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_GL_RENDER_BUFFER:
                {
                    // Get the render buffer details:
                    apGLRenderBuffer renderBufferDetails(0);
                    bool rc = gaGetRenderBufferObjectDetails(pGDItemData->_contextId._contextId, pGDItemData->_objectOpenGLName, renderBufferDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(renderBufferDetails);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_GL_PBUFFER_NODE:
                {
                    int bufferID = (int)pGDItemData->_objectOpenGLName;
                    apPBuffer pbufferDetails;
                    bool rcPBO = gaGetPBufferObjectDetails(bufferID, pbufferDetails);
                    GT_IF_WITH_ASSERT(rcPBO)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(pbufferDetails);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_GL_PROGRAM_PIPELINE:
                {
                    int pipelineID = (int)pGDItemData->_objectOpenGLName;
                    apGLPipeline pipelineDetailsBuffer;
                    bool rc = gaGetPipelineObjectDetails(pGDItemData->_contextId._contextId, pipelineID, pipelineDetailsBuffer);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(pipelineDetailsBuffer);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_GL_SAMPLER:
                {
                    int samplerID = (int)pGDItemData->_objectOpenGLName;
                    apGLSampler samplerDetailsBuffer;
                    bool rc = gaGetSamplerObjectDetails(pGDItemData->_contextId._contextId, samplerID, samplerDetailsBuffer);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(samplerDetailsBuffer);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_GL_STATIC_BUFFER:
                {
                    // Get the render buffer details:
                    apStaticBuffer staticBufferDetails;
                    bool rc = gaGetStaticBufferObjectDetails(pGDItemData->_contextId._contextId, pGDItemData->_bufferType, staticBufferDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(staticBufferDetails);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_GL_VBO:
                {
                    // Get the render buffer details:
                    apGLVBO vboDetails;
                    bool rc = gaGetVBODetails(pGDItemData->_contextId._contextId, pGDItemData->_objectOpenGLName, vboDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(vboDetails);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_GL_SYNC_OBJECT:
                {
                    // Get the sync object details:
                    apGLSync syncObjectDetails;
                    bool rc = gaGetSyncObjectDetails(pGDItemData->_syncIndex, syncObjectDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(syncObjectDetails);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_GL_PROGRAM:
                {
                    apGLProgram programDetails;
                    bool rc = gaGetProgramObjectDetails(pGDItemData->_contextId._contextId, pGDItemData->_objectOpenGLName, programDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(programDetails);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_GL_VERTEX_SHADER:
                case AF_TREE_ITEM_GL_TESSELLATION_CONTROL_SHADER:
                case AF_TREE_ITEM_GL_TESSELLATION_EVALUATION_SHADER:
                case AF_TREE_ITEM_GL_GEOMETRY_SHADER:
                case AF_TREE_ITEM_GL_FRAGMENT_SHADER:
                case AF_TREE_ITEM_GL_COMPUTE_SHADER:
                case AF_TREE_ITEM_GL_UNSUPPORTED_SHADER:
                {
                    gtAutoPtr<apGLShaderObject> aptrShaderDetails = NULL;
                    bool rc = gaGetShaderObjectDetails(pGDItemData->_contextId._contextId, pGDItemData->_objectOpenGLName, aptrShaderDetails);
                    GT_IF_WITH_ASSERT(rc && (aptrShaderDetails.pointedObject() != NULL))
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(*aptrShaderDetails);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_GL_DISPLAY_LIST:
                {
                    apGLDisplayList displayListDetails;
                    bool rc = gaGetDisplayListObjectDetails(pGDItemData->_contextId._contextId, pGDItemData->_objectOpenGLName, displayListDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(displayListDetails);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_GL_FBO:
                {
                    apGLFBO fboDetails;
                    bool rc = gaGetFBODetails(pGDItemData->_contextId._contextId, pGDItemData->_objectOpenGLName, fboDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(fboDetails);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_CL_IMAGE:
                {
                    apCLImage textureDetails;
                    bool rc = gaGetOpenCLImageObjectDetails(pGDItemData->_contextId._contextId, pGDItemData->_objectOpenCLIndex, textureDetails);

                    if (rc)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(textureDetails);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_CL_BUFFER:
                {
                    apCLBuffer bufferDetails;
                    bool rc = gaGetOpenCLBufferObjectDetails(pGDItemData->_contextId._contextId, pGDItemData->_objectOpenCLIndex, bufferDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(bufferDetails);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_CL_PIPE:
                {
                    apCLPipe pipeDetails;
                    bool rc = gaGetOpenCLPipeObjectDetails(pGDItemData->_contextId._contextId, pGDItemData->_objectOpenCLIndex, pipeDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(pipeDetails);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_CL_COMMAND_QUEUE:
                {
                    apCLCommandQueue commandQueueDetails;
                    bool rc = gaGetCommandQueueDetails(pGDItemData->_contextId._contextId, pGDItemData->_objectOpenCLIndex, commandQueueDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(commandQueueDetails);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_CL_PROGRAM:
                {
                    apCLProgram programDetails(0);
                    bool rc = gaGetOpenCLProgramObjectDetails(pGDItemData->_contextId._contextId, pGDItemData->_objectOpenCLIndex, programDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(programDetails);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_CL_KERNEL:
                {
                    // Use the HTML properties object to build the properties page:
                    // Get the kernel details:
                    apCLKernel kernelDetails(OA_CL_NULL_HANDLE, -1, OA_CL_NULL_HANDLE, AF_STR_Empty);
                    bool rc = gaGetOpenCLKernelObjectDetails(pGDItemData->_contextId._contextId, pGDItemData->_clKernelHandle, kernelDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(kernelDetails);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_CL_SAMPLER:
                {
                    apCLSampler samplerDetails;
                    bool rc = gaGetOpenCLSamplerObjectDetails(pGDItemData->_contextId._contextId, pGDItemData->_objectOpenCLIndex, samplerDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(samplerDetails);
                        callsStackShown = true;
                    }
                }
                break;

                case AF_TREE_ITEM_CL_EVENT:
                {
                    apCLEvent eventDetails(OA_CL_NULL_HANDLE, OA_CL_NULL_HANDLE, false);
                    bool rc = gaGetOpenCLEventObjectDetails(pGDItemData->_contextId._contextId, pGDItemData->_objectOpenCLIndex, eventDetails);
                    GT_IF_WITH_ASSERT(rc)
                    {
                        _pAllocatedObjectsCreationStackView->getAndDisplayObjectCreationStack(eventDetails);
                        callsStackShown = true;
                    }
                }
                break;

                default:
                {
                    // We added a new tree item type, but didn't add it here
                    callsStackShown = false;
                }
                break;
            }

            if (!callsStackShown)
            {
                // Generate a "Click an item" message by sending an empty calls stack:
                osCallStack emptyStack;
                _pAllocatedObjectsCreationStackView->setEmptyCallStackString(GD_STR_MemoryAnalysisViewerStackItemMessage);
                _pAllocatedObjectsCreationStackView->updateCallsStack(emptyStack);
            }
        }
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        onExecutionModeChangedEvent
/// \brief Description: Is called when the execution mode is changed
/// \param[in]          execModeEvent
/// \return void
/// -----------------------------------------------------------------------------------------------
void gdMemoryViewBase::onExecutionModeChangedEvent(const apEvent& execModeEvent)
{
    // Clear the first memory tab view:
    GT_IF_WITH_ASSERT((_pMemoryDetailsView != NULL) && (_pAllocatedObjectsCreationStackView != NULL) && (_pChartWindow != NULL))
    {
        bool isEnabled = true;
        bool modeChanged = gdDoesModeChangeApplyToDebuggerViews(execModeEvent, isEnabled);

        if (modeChanged)
        {
            _pMemoryDetailsView->clearAndDisplayMessage();

            // Enable / disable all internal views:
            _pAllocatedObjectsCreationStackView->setEnabled(isEnabled);
            _pMemoryDetailsView->setEnabled(isEnabled);
            _pChartWindow->setEnabled(isEnabled);
        }
    }
}


