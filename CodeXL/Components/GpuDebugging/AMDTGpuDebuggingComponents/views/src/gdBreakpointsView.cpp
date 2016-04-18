//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdBreakpointsView.cpp
///
//==================================================================================

//------------------------------ gdBreakpointsView.cpp ------------------------------

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtAutoPtr.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTAPIClasses/Include/apBreakPoint.h>
#include <AMDTAPIClasses/Include/apCLKernel.h>
#include <AMDTAPIClasses/Include/apCLObjectID.h>
#include <AMDTAPIClasses/Include/apCLProgram.h>
#include <AMDTAPIClasses/Include/apKernelFunctionNameBreakpoint.h>
#include <AMDTAPIClasses/Include/apKernelSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/apHostSourceBreakPoint.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>
#include <AMDTAPIClasses/Include/apSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointsUpdatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acIcons.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdBreakpointsItemData.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/gdPropertiesEventObserver.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdBreakpointsView.h>


// ---------------------------------------------------------------------------
// Name:        gdBreakpointsView::gdBreakpointsView
// Description: Constructor
// Arguments:   QWidget* pParent
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
gdBreakpointsView::gdBreakpointsView(QWidget* pParent)
    : acListCtrl(pParent, AC_DEFAULT_LINE_HEIGHT, true), m_pDisabledBreakpintPixmap(NULL), m_pEnabledBreakpintPixmap(NULL), m_pShowDialogAction(NULL)
{
    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);

    // Initialize the list control headers:
    QStringList columnCaptions;
    columnCaptions << GD_STR_breakpintsViewNameColumnCaption;
    initHeaders(columnCaptions, false);

    // Add the "double click to add" item:
    addDoubleClickMessageRow();

    // Define the markers:
    m_pEnabledBreakpintPixmap = new QPixmap;
    acSetIconInPixmap(*m_pEnabledBreakpintPixmap, AC_ICON_SOURCE_ENABLED_BREAKPOINT);

    m_pDisabledBreakpintPixmap = new QPixmap;
    acSetIconInPixmap(*m_pDisabledBreakpintPixmap, AC_ICON_SOURCE_DISABLED_BREAKPOINT);

    bool rcConnect = connect(this, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onItemClicked(QTableWidgetItem*)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(this, SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)), this, SLOT(onBreakpointSelected(QTableWidgetItem*, QTableWidgetItem*)));
    GT_ASSERT(rcConnect);

    // Enable row deletion:
    setEnableRowDeletion(true);

    // Extend the context menu:
    extendContextMenu();
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsView::~gdBreakpointsView
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
gdBreakpointsView::~gdBreakpointsView()
{
    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsView::onEvent
// Description: Is called when a debugged process event occurs.
//              Send the event to the appropriate event function for displaying
//              it to the user.
// Arguments:   eve - the debugged process event
//              bool& vetoEvent
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
void gdBreakpointsView::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent);  // unused
    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    QTableWidgetItem* pGITem = horizontalHeaderItem(0);

    if (pGITem != NULL)
    {
        QString text = pGITem->text();
    }

    // Handle the event according to its type:
    switch (eventType)
    {
        case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
        {
            bool rc = updateBreakpoints();
            GT_ASSERT(rc);
            break;
        }
        break;

        case apEvent::AP_BREAKPOINTS_UPDATED_EVENT:
        {
            bool rc;
            const apBreakpointsUpdatedEvent& breakpointUpdatedEvent = (const apBreakpointsUpdatedEvent&)eve;

            if (breakpointUpdatedEvent.updatedBreakpointIndex() == -1)
            {

                rc = updateBreakpoints();
            }
            else
            {
                // Update only the updated breakpoint:
                rc = updateSingleBreakpoint(breakpointUpdatedEvent.updatedBreakpointIndex());
            }

            GT_ASSERT(rc);
            break;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        case apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE:
        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        {
            // Clear the highlight when no longer in suspended state:
            clearBoldItems();
        }
        break;

        case apEvent::AP_BREAKPOINT_HIT:
        {
            bool rc = updateBreakpoints();
            GT_ASSERT(rc);

            const apBreakpointHitEvent& breakpointHitEvent = (const apBreakpointHitEvent&)eve;
            onBreakpointHitEvent(breakpointHitEvent);
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
}


// ---------------------------------------------------------------------------
// Name:        gdBreakpointsView::updateBreakpoints
// Description: Display the current breakpoints
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        6/9/2011
// ---------------------------------------------------------------------------
bool gdBreakpointsView::updateBreakpoints()
{
    bool retVal = false;

    // Do not perform the selection signal while updating the list:
    blockSignals(true);

    // Get the currently highlighted item:
    gtString highlightedText = findHighlightedItemText();

    // Clear the view:
    clearList();

    // Get the amount of breakpoints:
    int breakpointsAmount = 0;
    bool rc = gaGetAmountOfBreakpoints(breakpointsAmount);
    GT_IF_WITH_ASSERT(rc)
    {
        retVal = true;

        // Iterate the breakpoints and add each of them to the list:
        for (int i = 0; i < breakpointsAmount; i++)
        {
            // Get the current breakpoint:
            gtAutoPtr<apBreakPoint> aptrBreakpoint;
            bool rc2 = gaGetBreakpoint(i, aptrBreakpoint);
            GT_IF_WITH_ASSERT(rc2)
            {
                // Get the breakpoint as string:
                gtString bpStr;
                bool rcGetString = gdBreakpointAsString(aptrBreakpoint, bpStr);
                GT_IF_WITH_ASSERT(rcGetString)
                {
                    // Create an item data:
                    gdBreakpointsItemData* pBreakpointData = new gdBreakpointsItemData(aptrBreakpoint);

                    // Add the item to the list:
                    QStringList rowTexts;

                    // Add a row table item:
                    Qt::CheckState checkState = aptrBreakpoint->isEnabled() ? Qt::Checked : Qt::Unchecked;
                    QPixmap* pPixmap = aptrBreakpoint->isEnabled() ? m_pEnabledBreakpintPixmap : m_pDisabledBreakpintPixmap;
                    rowTexts << acGTStringToQString(bpStr);
                    bool rowAdded = addRow(rowTexts, (void*)pBreakpointData, true, checkState, pPixmap);
                    retVal = retVal && rowAdded;
                }
            }
        }
    }

    // Add the last line:
    addDoubleClickMessageRow();

    // If the list contained an highlighted item, highlight it again:
    if (!highlightedText.isEmpty())
    {
        highlightTextItem(highlightedText, true, true);
    }

    // Remove the signals block:
    blockSignals(false);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdBreakpointsView::onBreakpointHitEvent
// Description: Handle breakpoint hit event
// Arguments:   const apBreakpointHitEvent& breakpointHitEvent
// Author:      Sigal Algranaty
// Date:        7/9/2011
// ---------------------------------------------------------------------------
void gdBreakpointsView::onBreakpointHitEvent(const apBreakpointHitEvent& breakpointHitEvent)
{
    // Build a string from the event, and look for the matching item within the list:
    gtString breakpointHitStr = breakpointHitEventToString(breakpointHitEvent);

    if (!breakpointHitStr.isEmpty())
    {
        // Highlight the requested item:
        highlightTextItem(breakpointHitStr, true, true);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsView::onBreakpointHitEvent
// Description: Handle breakpoint hit event
// Arguments:   const apBreakpointHitEvent& breakpointHitEvent
// Author:      Sigal Algranaty
// Date:        7/9/2011
// ---------------------------------------------------------------------------
gtString gdBreakpointsView::breakpointHitEventToString(const apBreakpointHitEvent& breakpointHitEvent)
{
    // Build a string from the event, and look for the matching item within the list:
    gtString retVal;

    switch (breakpointHitEvent.breakReason())
    {
        case AP_MONITORED_FUNCTION_BREAKPOINT_HIT:
        {
            // API Breakpoint:
            const apFunctionCall* pBreakedOnFunctionCall = breakpointHitEvent.breakedOnFunctionCall();
            GT_IF_WITH_ASSERT(pBreakedOnFunctionCall)
            {
                // Get the function name:
                bool rc = gaGetMonitoredFunctionName(pBreakedOnFunctionCall->functionId(), retVal);
                GT_ASSERT(rc);
            }
        }
        break;

        case AP_OPENCL_ERROR_BREAKPOINT_HIT:
        case AP_DETECTED_ERROR_BREAKPOINT_HIT:
        case AP_SOFTWARE_FALLBACK_BREAKPOINT_HIT:
        case AP_GLDEBUG_OUTPUT_REPORT_BREAKPOINT_HIT:
        case AP_REDUNDANT_STATE_CHANGE_BREAKPOINT_HIT:
        case AP_DEPRECATED_FUNCTION_BREAKPOINT_HIT:
        case AP_MEMORY_LEAK_BREAKPOINT_HIT:
        case AP_OPENGL_ERROR_BREAKPOINT_HIT:
        {
            // Get the generic type from the break reason:
            apGenericBreakpointType breakpointType = apGenericBreakpoint::breakpointTypeFromBreakReason(breakpointHitEvent.breakReason());
            GT_IF_WITH_ASSERT(breakpointType != AP_BREAK_TYPE_UNKNOWN)
            {
                bool rc = apGenericBreakpoint::breakpointTypeToString(breakpointType, retVal);
                GT_ASSERT(rc);
            }
        }
        break;

        case AP_KERNEL_SOURCE_CODE_BREAKPOINT_HIT:
        {
            // This is a kernel source breakpoint:
            oaCLProgramHandle programHandle = OA_CL_NULL_HANDLE;
            int lineNumber = -1;
            bool rcLoc = gaGetKernelDebuggingLocation(programHandle, lineNumber);
            GT_IF_WITH_ASSERT(rcLoc)
            {
                apCLObjectID programObjectId;
                bool rcHand = gaGetOpenCLHandleObjectDetails(programHandle, programObjectId);
                GT_IF_WITH_ASSERT(rcHand && (programObjectId._objectType == OS_TOBJ_ID_CL_PROGRAM))
                {
                    // Get the program details:
                    apCLProgram programDetails(OA_CL_NULL_HANDLE);
                    bool rcProg = gaGetOpenCLProgramObjectDetails(programObjectId._contextId, programObjectId._objectId, programDetails);
                    GT_IF_WITH_ASSERT(rcProg)
                    {
                        // Return the parameters only if we have both:
                        osFilePath filePath = programDetails.sourceCodeFilePath();
                        gtString fileName;
                        filePath.getFileNameAndExtension(fileName);
                        retVal.appendFormattedString(L"%ls, %d", fileName.asCharArray(), lineNumber);
                    }
                }
            }
        }
        break;

        case AP_KERNEL_FUNCTION_NAME_BREAKPOINT_HIT:
        {
            // Get the currently debugged kernel function name:
            apCLKernel currentlyDebuggedKernel(OA_CL_NULL_HANDLE, 0, OA_CL_NULL_HANDLE, L"");
            bool rcKer = gaGetCurrentlyDebuggedKernelDetails(currentlyDebuggedKernel);
            GT_IF_WITH_ASSERT(rcKer)
            {
                retVal = currentlyDebuggedKernel.kernelFunctionName();

                // Add a prefix so we'll know it's a kernel:
                retVal.prepend(gdIsHSAKernelName(retVal) ? GD_STR_HSAKernelFunctionNameBreakpointPrefix : GD_STR_KernelFunctionNameBreakpointPrefix);
            }
        }
        break;

        default:
        {
            // There is no breakpoint bound to this breakpoint hit event:
        }
        break;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gdBreakpointsView::openBreakpintsDialog
// Description: double click a breakpoint - open the breakpoints dialog
// Author:      Sigal Algranaty
// Date:        7/9/2011
// ---------------------------------------------------------------------------
void gdBreakpointsView::openBreakpintsDialog()
{
    // Get the application commands handler:
    gdApplicationCommands* pApplicationCommands = gdApplicationCommands::gdInstance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
        // Open the breakpoints dialog:
        pApplicationCommands->openBreakpointsDialog();
    }
}


// ---------------------------------------------------------------------------
// Name:        gdBreakpointsView::onItemClicked
// Description:
// Arguments:   QTableWidgetItem * pItem
// Author:      Sigal Algranaty
// Date:        7/9/2011
// ---------------------------------------------------------------------------
void gdBreakpointsView::onItemClicked(QTableWidgetItem* pItem)
{
    // Call the base class implementation:
    acListCtrl::onItemClicked(pItem);

    GT_IF_WITH_ASSERT(pItem != NULL)
    {
        // If the item is not in the last row:
        if ((rowCount() - 1) > pItem->row())
        {
            // Get the breakpoint item data:
            gdBreakpointsItemData* pItemData = (gdBreakpointsItemData*)getItemData(pItem->row());
            GT_IF_WITH_ASSERT(pItemData != NULL)
            {
                // Get the API index represented by this item data:
                int breakpointAPIIndex = pItemData->breakpointAPIIndex();
                GT_IF_WITH_ASSERT(breakpointAPIIndex >= 0)
                {
                    // Check if the breakpoint is enabled:
                    bool isEnabled = (pItem->checkState() == Qt::Checked);

                    // Update the real breakpoint:
                    gtAutoPtr<apBreakPoint> aptrBreakpoint;
                    bool rc2 = gaGetBreakpoint(breakpointAPIIndex, aptrBreakpoint);
                    GT_IF_WITH_ASSERT(rc2)
                    {
                        // Update the breakpoint:
                        aptrBreakpoint->setEnableStatus(isEnabled);
                        gaSetBreakpoint(*aptrBreakpoint.pointedObject());
                    }

                    // Set the icon:
                    if (isEnabled)
                    {
                        if (m_pEnabledBreakpintPixmap != NULL)
                        {
                            pItem->setIcon(*m_pEnabledBreakpintPixmap);
                        }
                    }
                    else
                    {
                        if (m_pDisabledBreakpintPixmap != NULL)
                        {
                            pItem->setIcon(*m_pDisabledBreakpintPixmap);
                        }
                    }

                    pItemData->_isEnabled = isEnabled;

                    // Trigger breakpoints update event:
                    apBreakpointsUpdatedEvent eve(breakpointAPIIndex);
                    apEventsHandler::instance().registerPendingDebugEvent(eve);
                }
            }
        }
    }

    // Display the selected breakpoints:
    displayCurrentlySelectedBreakpoints();
}


// ---------------------------------------------------------------------------
// Name:        gdBreakpointsView::extendContextMenu
// Description: Extend the context menu for the list control
// Author:      Sigal Algranaty
// Date:        13/9/2011
// ---------------------------------------------------------------------------
void gdBreakpointsView::extendContextMenu()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pContextMenu != NULL)
    {
        // Add a separator:
        m_pContextMenu->addSeparator();

        // Connect the action to delete slot:
        bool rcConnect = connect(m_pDeleteAction, SIGNAL(triggered()), this, SLOT(onDeleteSelected()));
        GT_ASSERT(rcConnect);

        // Create an open breakpoints dialog action:
        m_pShowDialogAction = new QAction(GD_STR_breakpintsViewOpenDialog, this);

        // Connect the action to delete slot:
        rcConnect = connect(m_pShowDialogAction, SIGNAL(triggered()), this, SLOT(openBreakpintsDialog()));
        GT_ASSERT(rcConnect);

        // Add the actions to the menu:
        m_pContextMenu->addAction(m_pDeleteAction);
        m_pContextMenu->addAction(m_pShowDialogAction);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsView::onBeforeRemoveRow
// Description: Before an item row deletion
// Arguments:   int row
// Author:      Sigal Algranaty
// Date:        13/9/2011
// ---------------------------------------------------------------------------
void gdBreakpointsView::onBeforeRemoveRow(int row)
{
    // If the row about to be deleted is the last row:
    if ((rowCount() - 1) == row)
    {
        // Add a new row to replace it:
        addDoubleClickMessageRow();
    }
    else // (rowCount() - 1) > row
    {
        // Get the item data for this row:
        gdBreakpointsItemData* pItemData = (gdBreakpointsItemData*)getItemData(row);
        GT_IF_WITH_ASSERT(pItemData != NULL)
        {
            apBreakPoint* pBreakpointToDelete = NULL;

            // Allocate a breakpoint according to the breakpoint type:
            if (pItemData->_breakpointType == OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT)
            {
                // Allocate an API function breakpoint:
                pBreakpointToDelete = new apMonitoredFunctionBreakPoint(pItemData->_monitoredFunctionId);
            }
            else if (pItemData->_breakpointType == OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT)
            {
                // Allocate a kernel breakpoint:
                pBreakpointToDelete = new apKernelFunctionNameBreakpoint(pItemData->_kernelFunctionName);
            }
            else if (pItemData->_breakpointType == OS_TOBJ_ID_GENERIC_BREAKPOINT)
            {
                // Allocate an API function breakpoint:
                pBreakpointToDelete = new apGenericBreakpoint(pItemData->_genericBreakpointType);
            }
            else if (pItemData->_breakpointType == OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT)
            {
                // Allocate a kernel source code breakpoint:
                if (pItemData->_clProgramHandle != OA_CL_NULL_HANDLE)
                {
                    pBreakpointToDelete = new apKernelSourceCodeBreakpoint(pItemData->_clProgramHandle, pItemData->_sourceCodeLine);
                }
                else
                {
                    pBreakpointToDelete = new apKernelSourceCodeBreakpoint(pItemData->_sourceCodeFilePath, pItemData->_sourceCodeLine);
                }
            }
            else if (pItemData->_breakpointType == OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT)
            {
                // Allocate an API function breakpoint:
                pBreakpointToDelete = new apSourceCodeBreakpoint(pItemData->_sourceCodeFilePath, pItemData->_sourceCodeLine);
            }
            else if (OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT == pItemData->_breakpointType)
            {
                pBreakpointToDelete = new apHostSourceCodeBreakpoint(pItemData->_sourceCodeFilePath, pItemData->_sourceCodeLine);
            }

            GT_IF_WITH_ASSERT(pBreakpointToDelete != NULL)
            {
                // Remove the breakpoint:
                int breakpointIndex = -1;
                bool rc = gaGetBreakpointIndex(*pBreakpointToDelete, breakpointIndex);
                GT_IF_WITH_ASSERT(rc)
                {
                    // Remove this breakpoint:
                    gaRemoveBreakpoint(breakpointIndex);
                }

                // Release the breakpoint memory:
                delete pBreakpointToDelete;

                // Trigger breakpoints update event:
                apBreakpointsUpdatedEvent eve(-1);
                apEventsHandler::instance().registerPendingDebugEvent(eve);
            }

            delete pItemData;

            /// Clear selection for deprecate trigger to onSelectRow event.
            /// Row actually not exist and item data is wrong.
            clearSelection();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdBreakpointsView::onItemsCheckedChange
// Description: The functions is called when the selected items check state is
//              changed
// Author:      Sigal Algranaty
// Date:        20/9/2011
// ---------------------------------------------------------------------------
void gdBreakpointsView::onItemsCheckedChange()
{
    int lastRowNumber = rowCount() - 1;

    // Get the currently selected items:
    QList<QTableWidgetItem*> currentlySelectedItems = selectedItems();

    foreach (QTableWidgetItem* pItem, currentlySelectedItems)
    {
        GT_IF_WITH_ASSERT(pItem != NULL)
        {
            // Get the breakpoint item data:
            int itemRowNumber = pItem->row();

            if (lastRowNumber != itemRowNumber)
            {
                gdBreakpointsItemData* pItemData = (gdBreakpointsItemData*)getItemData(itemRowNumber);
                GT_IF_WITH_ASSERT(pItemData != NULL)
                {
                    // Get the API index represented by this item data:
                    int breakpointAPIIndex = pItemData->breakpointAPIIndex();
                    GT_IF_WITH_ASSERT(breakpointAPIIndex >= 0)
                    {

                        // Check if the breakpoint is enabled:
                        // The breakpoint check state is about to be toggled, so the boolean should be toggled:
                        bool isEnabled = !(pItem->checkState() == Qt::Checked);

                        // Update the real breakpoint:
                        gtAutoPtr<apBreakPoint> aptrBreakpoint;
                        bool rc2 = gaGetBreakpoint(breakpointAPIIndex, aptrBreakpoint);
                        GT_IF_WITH_ASSERT(rc2)
                        {
                            // Update the breakpoint:
                            aptrBreakpoint->setEnableStatus(isEnabled);
                            gaSetBreakpoint(*aptrBreakpoint.pointedObject());
                        }

                        // Set the icon:
                        if (isEnabled)
                        {
                            if (m_pEnabledBreakpintPixmap != NULL)
                            {
                                pItem->setIcon(*m_pEnabledBreakpintPixmap);
                            }
                        }
                        else
                        {
                            if (m_pDisabledBreakpintPixmap != NULL)
                            {
                                pItem->setIcon(*m_pDisabledBreakpintPixmap);
                            }
                        }

                        // Trigger breakpoints update event:
                        apBreakpointsUpdatedEvent eve(breakpointAPIIndex);
                        apEventsHandler::instance().registerPendingDebugEvent(eve);
                    }
                }
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdBreakpointsView::updateSingleBreakpoint
// Description: Update the breakpoint with the requested index
// Arguments:   int updatedBreakpointIndex
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        22/9/2011
// ---------------------------------------------------------------------------
bool gdBreakpointsView::updateSingleBreakpoint(int updatedBreakpointIndex)
{
    bool retVal = false;

    // Get the current breakpoint:
    gtAutoPtr<apBreakPoint> aptrBreakpoint;
    bool rc2 = gaGetBreakpoint(updatedBreakpointIndex, aptrBreakpoint);
    GT_IF_WITH_ASSERT(rc2)
    {
        // Get the breakpoint as string:
        gtString bpStr;
        bool rcGetString = gdBreakpointAsString(aptrBreakpoint, bpStr);
        GT_IF_WITH_ASSERT(rcGetString)
        {
            // Get the item data for the requested index:
            gdBreakpointsItemData* pBreakpointData = (gdBreakpointsItemData*)getItemData(updatedBreakpointIndex);
            GT_IF_WITH_ASSERT(pBreakpointData != NULL)
            {
                // Add a row table item:
                Qt::CheckState checkState = aptrBreakpoint->isEnabled() ? Qt::Checked : Qt::Unchecked;
                QPixmap* pPixmap = aptrBreakpoint->isEnabled() ? m_pEnabledBreakpintPixmap : m_pDisabledBreakpintPixmap;

                // Get the table widget data for this item:
                QTableWidgetItem* pItem = item(updatedBreakpointIndex, 0);
                GT_IF_WITH_ASSERT(pItem != NULL)
                {
                    // Set the row item text:
                    pItem->setText(acGTStringToQString(bpStr));

                    // Set the item icon:
                    pItem->setIcon(*pPixmap);

                    // Set the item check state:
                    pItem->setCheckState(checkState);

                    retVal = true;
                }
            }
        }
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gdBreakpointsView::addDoubleClickMessageRow
// Description: Adds a message line prompting the user to double-click the view
//              to add breakpoints
// Author:      Uri Shomroni
// Date:        21/3/2012
// ---------------------------------------------------------------------------
void gdBreakpointsView::addDoubleClickMessageRow()
{
    int newRowNumber = rowCount();

    // Add the message line:
    QStringList newItemStrings;
    newItemStrings << GD_STR_breakpointsViewDoubleClickToAdd;
    addRow(newItemStrings, NULL);
    QTableWidgetItem* pFirstItemInNewLine = item(newRowNumber, 0);
    GT_IF_WITH_ASSERT(pFirstItemInNewLine != NULL)
    {
        // Set it to be gray:
        pFirstItemInNewLine->setTextColor(acQLIST_EDITABLE_ITEM_COLOR);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsView::mouseDoubleClickEvent
// Description: On mouse double click - open the breakpoints dialog
// Arguments:   QMouseEvent* pMouseEvent
// Author:      Sigal Algranaty
// Date:        20/3/2012
// ---------------------------------------------------------------------------
void gdBreakpointsView::mouseDoubleClickEvent(QMouseEvent* pMouseEvent)
{
    // Open the breakpoints dialog:
    openBreakpintsDialog();

    // Call the base class implementation:
    acListCtrl::mouseDoubleClickEvent(pMouseEvent);
}


// ---------------------------------------------------------------------------
// Name:        gdBreakpointsView::onBreakpointSelected
// Description: Display an HTML properties of the breakpoin
// Arguments:   QTableWidgetItem * pCurrent
//              QTableWidgetItem* pPrevious
// Author:      Sigal Algranaty
// Date:        20/3/2012
// ---------------------------------------------------------------------------
void gdBreakpointsView::onBreakpointSelected(QTableWidgetItem*, QTableWidgetItem*)
{
    // Display the currently selected breakpoints:
    displayCurrentlySelectedBreakpoints();
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsView::displayCurrentlySelectedBreakpoints
// Description: Display the currently selected breakpoints
// Author:      Sigal Algranaty
// Date:        22/3/2012
// ---------------------------------------------------------------------------
void gdBreakpointsView::displayCurrentlySelectedBreakpoints()
{
    if (gaIsDebuggedProcessSuspended())
    {
        // Find the currently selected breakpoint/s:
        QList<int> selectedRows;
        QModelIndexList listOfSelectedIndexes = selectedIndexes();

        foreach (QModelIndex index, listOfSelectedIndexes)
        {
            if (index.isValid())
            {
                int lastItemIndex = rowCount() - 1;

                // Do not add the last item:
                if (index.row() != lastItemIndex)
                {
                    if (selectedRows.indexOf(index.row()) < 0)
                    {
                        selectedRows << index.row();
                    }
                }
            }
        }

        // Build an HTML string for the breakpoint selected:
        gdHTMLProperties htmlBuilder;
        gtString propertiesViewMessage;

        if (selectedRows.length() == 1)
        {
            // Get the item data:
            gdBreakpointsItemData* pItemData = (gdBreakpointsItemData*)getItemData(selectedRows.first());
            GT_IF_WITH_ASSERT(pItemData != NULL)
            {
                // Build the HTML string for this call stack:
                afHTMLContent htmlContent;
                htmlBuilder.buildBreakpointPropertiesString(pItemData, htmlContent);
                htmlContent.toString(propertiesViewMessage);
            }
        }
        else
        {
            // Add a message to select a breakpoint:
            htmlBuilder.buildSimpleHTMLMessage(GD_STR_PropertiesBreakpointsMultipleTitle, GD_STR_PropertiesBreakpointsMultipleText, propertiesViewMessage, true);
        }

        // Set the properties string:
        gdPropertiesEventObserver::instance().setPropertiesFromText(acGTStringToQString(propertiesViewMessage));

    }
}

// ---------------------------------------------------------------------------
// Name:        gdBreakpointsView::onAboutToShowContextMenu
// Description: Enable / Disable context menu actions
// Author:      Sigal Algranaty
// Date:        26/3/2012
// ---------------------------------------------------------------------------
void gdBreakpointsView::onAboutToShowContextMenu()
{
    // Call the base class implementation:
    acListCtrl::onAboutToShowContextMenu();

    GT_IF_WITH_ASSERT(m_pDeleteAction != NULL)
    {
        bool isDeleteEnabled = false;

        // Only if there are items:
        if (rowCount() > 1)
        {
            QModelIndexList selectedBPs = selectedIndexes();
            QTableWidgetItem* pItem = item(rowCount() - 1, 0);

            if (pItem != NULL)
            {
                QModelIndex editableRowIndex = indexFromItem(pItem);
                selectedBPs.removeOne(editableRowIndex);
                isDeleteEnabled = !selectedBPs.isEmpty();
            }
        }

        m_pDeleteAction->setEnabled(isDeleteEnabled);
    }
}
