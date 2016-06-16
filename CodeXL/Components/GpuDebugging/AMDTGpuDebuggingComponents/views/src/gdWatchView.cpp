//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdWatchView.cpp
///
//==================================================================================

//------------------------------ gdWatchView.cpp ------------------------------

// Qt:
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTAPIClasses/Include/Events/apCallStackFrameSelectedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/gdPropertiesEventObserver.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdWatchView.h>


// ---------------------------------------------------------------------------
// Name:        gdWatchView::gdWatchView
// Description: Constructor
// Author:      Uri Shomroni
// Date:        12/9/2011
// ---------------------------------------------------------------------------
gdWatchView::gdWatchView(QWidget* pParent)
    : acListCtrl(pParent), _pAddMultiWatchAction(NULL), _pApplicationCommands(NULL), m_stackDepth(-2), m_currentlyAddingLine(false)
{
    // Get the application commands instance:
    _pApplicationCommands = gdApplicationCommands::gdInstance();
    GT_ASSERT(_pApplicationCommands != NULL);

    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);

    // Initialize the list control headers:
    QStringList columnCaptions;
    columnCaptions << GD_STR_localsViewNameColumnHeader;
    columnCaptions << GD_STR_localsViewValueColumnHeader;
    columnCaptions << GD_STR_localsViewTypeColumnHeader;
    initHeaders(columnCaptions, false);

    // Connect signals:
    bool rcConnect = connect(this, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(onItemChanged(QTableWidgetItem*)));
    GT_ASSERT(rcConnect);

    // Connect signals:
    rcConnect = connect(this, SIGNAL(cellClicked(int, int)), this, SLOT(onCellClicked(int, int)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(this, SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)), this, SLOT(displaySelectedWatchProperties()));
    GT_ASSERT(rcConnect);

    // Allow deleting lines from the table:
    setEnableRowDeletion(true);

    // Add the editable row:
    addEditableWatchRow();

    // Extend the context menu:
    extendContextMenu();

    // Allow drops:
    setAcceptDrops(true);

    // Select whole lines:
    setSelectionBehavior(acListCtrl::SelectRows);

}


// ---------------------------------------------------------------------------
// Name:        gdWatchView::~gdWatchView
// Description: Destructor
// Author:      Uri Shomroni
// Date:        12/9/2011
// ---------------------------------------------------------------------------
gdWatchView::~gdWatchView()
{
    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}


// ---------------------------------------------------------------------------
// Name:        gdWatchView::onEvent
// Description: Is called when a debugged process event occurs.
// Author:      Uri Shomroni
// Date:        12/9/2011
// ---------------------------------------------------------------------------
void gdWatchView::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent);  // unused
    apEvent::EventType eveType = eve.eventType();

    switch (eveType)
    {
        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        case apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED:
        case apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED_EXTERNALLY:
        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        {
            // Reset the depth:
            m_stackDepth = -2;

            // Clear the list:
            clearListValues();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
        {
            // Set the stack depth and fall through:
            m_stackDepth = 0;
        }

        case apEvent::AP_KERNEL_CURRENT_WORK_ITEM_CHANGED_EVENT:
        {
            // Update the values and header:
            updateWatchValues();
        }
        break;

        // No break since we want to continue to the next case
        case apEvent::AP_BEFORE_KERNEL_DEBUGGING_EVENT:
        case apEvent::AP_AFTER_KERNEL_DEBUGGING_EVENT:
        case apEvent::AP_KERNEL_DEBUGGING_FAILED_EVENT:
        {
            // Update the value column header:
            updateValueColumnHeader();
        }
        break;

        case apEvent::AP_EXECUTION_MODE_CHANGED_EVENT:
        {
            // Enable the view only in debug mode:
            bool isEnabled = true;
            bool modeChanged = gdDoesModeChangeApplyToDebuggerViews(eve, isEnabled);

            if (modeChanged)
            {
                setEnabled(isEnabled);

                if (!isEnabled)
                {
                    clearListValues();
                }
            }
        }
        break;

        case apEvent::AP_CALL_STACK_FRAME_SELECTED_EVENT:
        {
            // Set the new frame index:
            const apCallStackFrameSelectedEvent& callStackEvent = (const apCallStackFrameSelectedEvent&)eve;
            int newFrameIndex = callStackEvent.frameIndex();

            if (m_stackDepth != newFrameIndex)
            {
                m_stackDepth = newFrameIndex;

                // Update the values and header:
                updateWatchValues();
            }
        }
        break;

        default:
            // Ignore other events
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdWatchView::onBeforeRemoveRow
// Description: Called before a row is removed
// Author:      Uri Shomroni
// Date:        15/9/2011
// ---------------------------------------------------------------------------
void gdWatchView::onBeforeRemoveRow(int row)
{
    // If this is the last row:
    if (row == (rowCount() - 1))
    {
        // Add a new "enter expression" row to replace it:
        addEditableWatchRow();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdWatchView::onItemChanged
// Description: Called when an item is edited.
// Author:      Uri Shomroni
// Date:        14/9/2011
// ---------------------------------------------------------------------------
void gdWatchView::onItemChanged(QTableWidgetItem* pItem)
{
    // Avoid entering here when we add an item, as it may cause the edited line to be deleted:
    if (!m_currentlyAddingLine)
    {
        m_currentlyAddingLine = true;

        // Sanity check:
        if (pItem != NULL)
        {
            // If this is the first item in the line:
            if (pItem->column() == 0)
            {
                // If the new value is empty or the default string:
                int rowNumber = pItem->row();
                bool wasLastRow = (rowNumber == (rowCount() - 1));
                QString itemText = pItem->text();

                if (itemText.isEmpty() || (itemText == GD_STR_watchViewNewWatchExpressionItem))
                {
                    // Remove the row:
                    removeRow(rowNumber);
                }
                else // !(itemText.isEmpty() || (itemText == GD_STR_watchViewNewWatchExpressionItem))
                {
                    // Fill the row with data:
                    updateWatchLineValue(rowNumber);
                }

                // If the last row just changed:
                if (wasLastRow)
                {
                    // Add a new row:
                    m_currentlyAddingLine = false;
                    addEditableWatchRow();
                    m_currentlyAddingLine = true;
                }
            }
        }

        // Call the base function:
        QTableWidget::itemChanged(pItem);
        m_currentlyAddingLine = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdWatchView::clearListValues
// Description: Clears the list items' "value" and "type" fields, leaving just the
//              names
// Author:      Uri Shomroni
// Date:        12/9/2011
// ---------------------------------------------------------------------------
void gdWatchView::clearListValues()
{
    // Iterate the lines:
    int numberOfListLines = rowCount();

    for (int i = 0; i < numberOfListLines; i++)
    {
        // Clear the value and type cells:
        QTableWidgetItem* pPreviousValue = takeItem(i, 1);
        delete pPreviousValue;
        QTableWidgetItem* pPreviousType = takeItem(i, 2);
        delete pPreviousType;
    }

    // Make sure the value column is up-to-date:
    updateValueColumnHeader();
}

// ---------------------------------------------------------------------------
// Name:        gdWatchView::updateWatchValues
// Description: Updates the values of all watch items
// Author:      Uri Shomroni
// Date:        12/9/2011
// ---------------------------------------------------------------------------
void gdWatchView::updateWatchValues()
{
    // Update the column headers:
    updateValueColumnHeader();

    // If we are in kernel debugging:
    if (gaIsInKernelDebugging() || gaIsInHSAKernelBreakpoint() || gaCanGetHostVariables())
    {
        // Iterate the list items:
        int numberOfListLines = rowCount();

        for (int i = 0; i < numberOfListLines; i++)
        {
            // Update each one:
            updateWatchLineValue(i);
        }
    }
    else // !gaIsInKernelDebugging()
    {
        // Just clear the list values:
        clearListValues();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdWatchView::updateWatchLineValue
// Description: Updates the value and type of a single watch item
// Author:      Uri Shomroni
// Date:        12/9/2011
// ---------------------------------------------------------------------------
void gdWatchView::updateWatchLineValue(int lineNum)
{
    // Clear any previous values:
    QTableWidgetItem* pPreviousValue = takeItem(lineNum, 1);
    delete pPreviousValue;
    QTableWidgetItem* pPreviousType = takeItem(lineNum, 2);
    delete pPreviousType;

    // Will get the new values:
    apExpression variableValue;
    bool gotValue = false;
    bool useHex = false;

    bool openCLDebugging = gaIsInKernelDebugging();
    bool hsaDebugging = gaIsInHSAKernelBreakpoint();
    bool hostDebugging = gaCanGetHostVariables();

    // Get the variable value:
    QTableWidgetItem* pVariableNameItem = item(lineNum, 0);
    GT_IF_WITH_ASSERT(pVariableNameItem != NULL)
    {
        gotValue = true;

        // If the "variable" name is empty or the default string, ignore it:
        QString currentVariableNameQstring = pVariableNameItem->text();

        if (!(currentVariableNameQstring.isEmpty() || (currentVariableNameQstring == GD_STR_watchViewNewWatchExpressionItem)))
        {
            // Set Any items other than the "add" item to be black instead of gray:
            pVariableNameItem->setTextColor(Qt::black);

            gotValue = false;

            // Get the values and type:
            gtString currentVariableName;
            currentVariableName.fromASCIIString(currentVariableNameQstring.toLatin1());

            if (openCLDebugging)
            {
                // Get the current work item index:
                int currentWorkItemCoord[3] = { -1, -1, -1 };
                bool rcCo = gaGetKernelDebuggingCurrentWorkItem(currentWorkItemCoord[0], currentWorkItemCoord[1], currentWorkItemCoord[2]);

                GT_IF_WITH_ASSERT(rcCo)
                {
                    bool rcVal = gaGetKernelDebuggingExpressionValue(currentVariableName, currentWorkItemCoord, 0, variableValue);

                    if (rcVal)
                    {
                        gotValue = true;
                    }
                }
            }
            else if (hsaDebugging)
            {
                bool rcVal = gaHSAGetExpressionValue(currentVariableName, 0, variableValue);

                if (rcVal)
                {
                    gotValue = true;

                    // Show HSAIL registers as hex values:
                    if ((0 < currentVariableName.length()) && ('$' == currentVariableName[0]))
                    {
                        useHex = true;
                    }
                }
            }
            else if (hostDebugging)
            {
                int chosenThreadIndex = gdGDebuggerGlobalVariablesManager::instance().chosenThread();
                osThreadId threadId = OS_NO_THREAD_ID;
                bool rc = gaGetThreadId(chosenThreadIndex, threadId);

                if (rc)
                {
                    bool rcVal = gaGetThreadExpressionValue(threadId, m_stackDepth, currentVariableName, 0, variableValue);

                    if (rcVal)
                    {
                        gotValue = true;
                    }
                }
            }
        }
    }

    if (!gotValue)
    {
        if (openCLDebugging || hsaDebugging || hostDebugging)
        {
            variableValue.m_value = GD_STR_watchViewCannotEvaluateExpression;
        }
    }

    // Update the list item:
    const gtString& usedValue = useHex ? variableValue.m_value : variableValue.m_valueHex;
    if (!usedValue.isEmpty())
    {
        QTableWidgetItem* pValueItem = new QTableWidgetItem(acGTStringToQString(usedValue));
        setItem(lineNum, 1, pValueItem);
        pValueItem->setFlags(pValueItem->flags() & (~Qt::ItemIsEditable));
    }

    if (!variableValue.m_type.isEmpty())
    {
        QTableWidgetItem* pTypeItem = new QTableWidgetItem(acGTStringToQString(variableValue.m_type));
        pTypeItem->setFlags(pTypeItem->flags() & (~Qt::ItemIsEditable));
        setItem(lineNum, 2, pTypeItem);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdWatchView::addEditableRow
// Description: Appends an editable item at the end of the table
// Author:      Uri Shomroni
// Date:        14/9/2011
// ---------------------------------------------------------------------------
void gdWatchView::addEditableWatchRow()
{
    // Avoid entering here when we add an item:
    if (!m_currentlyAddingLine)
    {
        m_currentlyAddingLine = true;

        // Get the index of the row about to be added:
        int newRowNumber = rowCount();

        // Create an empty line:
        QStringList newItemStrings;
        newItemStrings << GD_STR_watchViewNewWatchExpressionItem << "" << "";
        addRow(newItemStrings, NULL);

        // Get the left-most item of the new line:
        QTableWidgetItem* pFirstItemInNewLine = item(newRowNumber, 0);
        GT_IF_WITH_ASSERT(pFirstItemInNewLine != NULL)
        {
            // Make it editable:
            pFirstItemInNewLine->setFlags(pFirstItemInNewLine->flags() | Qt::ItemIsEditable);
            pFirstItemInNewLine->setTextColor(acQLIST_EDITABLE_ITEM_COLOR);
        }

        m_currentlyAddingLine = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdWatchView::updateValueColumnHeader
// Description: Updates the "Value" column header with the work item coordinates
//              if they are available.
// Author:      Uri Shomroni
// Date:        20/9/2011
// ---------------------------------------------------------------------------
void gdWatchView::updateValueColumnHeader()
{
    // Start with the "Value" label:
    QString newHeader = GD_STR_localsViewValueColumnHeader;

    // If we are in kernel debugging:
    if (gaIsInKernelDebugging())
    {
        // Attempt to add the work item coordinates:
        gtString workItemCoordinates;
        bool rcWI = gdKernelDebuggingCurrentWorkItemAsString(workItemCoordinates);

        if (rcWI && (!workItemCoordinates.isEmpty()))
        {
            // Add a space and the coordinates:
            newHeader.append(' ');
            newHeader.append(acGTStringToQString(workItemCoordinates));
        }
    }

    // Set the new string:
    QTableWidgetItem* pSecondHeaderItem = horizontalHeaderItem(1);
    GT_IF_WITH_ASSERT(pSecondHeaderItem != NULL)
    {
        pSecondHeaderItem->setText(newHeader);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdWatchView::addWatch
// Description: Add a watch expression to the watch view
// Arguments:   const gtString& variableName
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/9/2011
// ---------------------------------------------------------------------------
bool gdWatchView::addWatch(const gtString& variableName)
{
    bool retVal = false;

    // Clear the current selection:
    clearSelection();

    // Check if the item exists:
    QList<QTableWidgetItem*> matchingItems = findItems(acGTStringToQString(variableName), Qt::MatchExactly);

    if (!matchingItems.isEmpty())
    {
        foreach (QTableWidgetItem* pItem, matchingItems)
        {
            if (pItem != NULL)
            {
                // We already have a variable with this name, select it:
                if (pItem->column() == 0)
                {
                    pItem->setSelected(true);
                    retVal = true;
                    break;
                }
            }
        }
    }

    if (!retVal)
    {
        // Calculate the line of the new watch:
        int lineNumber = rowCount() - 1;

        // Will get the new values:
        apExpression variableValue;
        bool rcVal = false;
        bool useHex = false;

        bool openclDebugging = gaIsInKernelDebugging();
        bool hsaDebugging = gaIsInHSAKernelBreakpoint();
        bool hostDebugging = gaCanGetHostVariables();

        if (openclDebugging)
        {
            // Get the current work item index:
            int currentWorkItemCoord[3] = { -1, -1, -1 };
            bool rcCo = gaGetKernelDebuggingCurrentWorkItem(currentWorkItemCoord[0], currentWorkItemCoord[1], currentWorkItemCoord[2]);

            GT_IF_WITH_ASSERT(rcCo)
            {
                // Get the values and type:
                rcVal = gaGetKernelDebuggingExpressionValue(variableName, currentWorkItemCoord, 0, variableValue);
            }
        }
        else if (hsaDebugging)
        {
            rcVal = gaHSAGetExpressionValue(variableName, 0, variableValue);

            // Show HSAIL registers as hex values:
            if (rcVal && (0 < variableName.length()) && ('$' == variableName[0]))
            {
                useHex = true;
            }
        }
        else if (hostDebugging)
        {
            int chosenThreadIndex = gdGDebuggerGlobalVariablesManager::instance().chosenThread();
            osThreadId threadId = OS_NO_THREAD_ID;
            bool rc = gaGetThreadId(chosenThreadIndex, threadId);

            if (rc)
            {
                rcVal = gaGetThreadExpressionValue(threadId, m_stackDepth, variableName, 0, variableValue);
            }
        }

        if (!rcVal)
        {
            if (openclDebugging || hsaDebugging || hostDebugging)
            {
                variableValue.m_value = GD_STR_watchViewCannotEvaluateExpression;
            }
        }

        // Update the list item:
        QTableWidgetItem* pNameItem = new QTableWidgetItem(acGTStringToQString(variableName));
        setItem(lineNumber, 0, pNameItem);
        pNameItem->setFlags(pNameItem->flags() | Qt::ItemIsEditable);
        pNameItem->setSelected(true);

        const gtString& usedValue = useHex ? variableValue.m_value : variableValue.m_valueHex;
        if (!usedValue.isEmpty())
        {
            QTableWidgetItem* pValueItem = new QTableWidgetItem(acGTStringToQString(usedValue));
            setItem(lineNumber, 1, pValueItem);
            pValueItem->setFlags(pValueItem->flags() & (~Qt::ItemIsEditable));

            // Select the item:
            pValueItem->setSelected(true);
        }

        if (!variableValue.m_type.isEmpty())
        {
            QTableWidgetItem* pTypeItem = new QTableWidgetItem(acGTStringToQString(variableValue.m_type));
            pTypeItem->setFlags(pTypeItem->flags() & (~Qt::ItemIsEditable));
            setItem(lineNumber, 2, pTypeItem);

            // Select the item:
            pTypeItem->setSelected(true);
        }

        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdWatchView::onAddMultiWatch
// Description: Handling the "Add Multi Watch" command
// Author:      Sigal Algranaty
// Date:        25/9/2011
// ---------------------------------------------------------------------------
void gdWatchView::onAddMultiWatch()
{
    // Get the item at the context menu position:
    QTableWidgetItem* pItemAtContextMenu = itemAt(m_contextMenuPosition);
    GT_IF_WITH_ASSERT(pItemAtContextMenu != NULL)
    {
        // Get the row for the selected item:
        int row = pItemAtContextMenu->row();

        GT_IF_WITH_ASSERT(row >= 0)
        {
            // Get the first column item:
            QTableWidgetItem* pItem0 = item(row, 0);
            GT_IF_WITH_ASSERT(pItem0 != NULL)
            {
                // Get the current selected item:
                QString textSelected = pItem0->text();

                gtString textForWatch;
                textForWatch.fromASCIIString(textSelected.toLatin1());

                if (!textForWatch.isEmpty())
                {
                    // Get the watch view:
                    GT_IF_WITH_ASSERT(_pApplicationCommands != NULL)
                    {
                        bool rc = _pApplicationCommands->displayMultiwatchVariable(textForWatch);
                        GT_ASSERT(rc);
                    }
                }

                // Else do nothing (no text for watch):
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdWatchView::extendContextMenu
// Description: Extend the context menu (add watch & multiwatch actions)
// Author:      Sigal Algranaty
// Date:        26/9/2011
// ---------------------------------------------------------------------------
void gdWatchView::extendContextMenu()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pContextMenu != NULL)
    {
        m_pContextMenu->addSeparator();

        // Create an action for the "Add Multi Watch" menu item:
        _pAddMultiWatchAction = new QAction(AF_STR_SourceCodeAddMultiWatch, m_pContextMenu);

        // Add the action to the context menu:
        m_pContextMenu->addAction(_pAddMultiWatchAction);

        // Connect the action to its handler:
        bool rcConnect = connect(_pAddMultiWatchAction, SIGNAL(triggered()), this, SLOT(onAddMultiWatch()));
        GT_ASSERT(rcConnect);

        // Connect the menu to an about to show slot:
        rcConnect = connect(m_pContextMenu, SIGNAL(aboutToShow()), this, SLOT(onAboutToShowTextContextMenu()));
        GT_ASSERT(rcConnect);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdWatchView::onAboutToShowTextContextMenu
// Description: Is called when the text context menu is shown - enable / disable
//              the context menu actions according to the current debug situation
// Author:      Sigal Algranaty
// Date:        25/9/2011
// ---------------------------------------------------------------------------
void gdWatchView::onAboutToShowTextContextMenu()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pAddMultiWatchAction != NULL)
    {
        // Check if add watch actions should be enabled:
        bool isEnabled = gaIsDebuggedProcessSuspended() && gaIsInKernelDebugging();

        // Set the actions enable state:
        _pAddMultiWatchAction->setEnabled(isEnabled);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdWatchView::onEditPaste
// Description: Implement paste action
// Author:      Sigal Algranaty
// Date:        27/9/2011
// ---------------------------------------------------------------------------
void gdWatchView::onEditPaste()
{
    // Get the clipboard:
    // Sanity check:
    GT_IF_WITH_ASSERT(qApp != NULL)
    {
        // Get the clipboard from the application:
        QClipboard* pClipboard = qApp->clipboard();
        GT_IF_WITH_ASSERT(pClipboard != NULL)
        {
            // Get the copied text from clipboard:
            QString currentClipboardText = pClipboard->text();
            gtString varName;
            varName.fromASCIIString(currentClipboardText.toLatin1());

            // Add this watch:
            bool rc = addWatch(varName);
            GT_ASSERT(rc);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdWatchView::dragEnterEvent
// Description: Handle drag enter event
// Arguments:   QDragEnterEvent *pEvent
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/9/2011
// ---------------------------------------------------------------------------
void gdWatchView::dragEnterEvent(QDragEnterEvent* pEvent)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pEvent != NULL)
    {
        pEvent->acceptProposedAction();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdWatchView::dragMoveEvent
// Description: Handle drag move event
// Arguments:   QDragEnterEvent *pEvent
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/9/2011
// ---------------------------------------------------------------------------
void gdWatchView::dragMoveEvent(QDragMoveEvent* pEvent)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pEvent != NULL)
    {
        pEvent->acceptProposedAction();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdWatchView::dropEvent
// Description: Handle drop event (add a variable with the text dragged)
// Arguments:   QDropEvent *event
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/9/2011
// ---------------------------------------------------------------------------
void gdWatchView::dropEvent(QDropEvent* pEvent)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pEvent != NULL)
    {
        // Get the mime data:
        const QMimeData* pMimeData = pEvent->mimeData();
        GT_IF_WITH_ASSERT(pMimeData != NULL)
        {
            if (pMimeData->hasText())
            {
                // Get the dragged text:
                QString text = pMimeData->text();
                gtString varName;
                varName.fromASCIIString(text.toLatin1());

                if (!varName.isEmpty())
                {
                    // Add a variable:
                    addWatch(varName);
                }
            }
        }

        pEvent->acceptProposedAction();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdWatchView::dragLeaveEvent
// Description: Handle the drag leave event
// Arguments:   QDragLeaveEvent *event
// Author:      Sigal Algranaty
// Date:        27/9/2011
// ---------------------------------------------------------------------------
void gdWatchView::dragLeaveEvent(QDragLeaveEvent* pEvent)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pEvent != NULL)
    {
        pEvent->accept();
    }
}


// ---------------------------------------------------------------------------
// Name:        gdWatchView::onCellClicked
// Description: When the user clicks on the "Type watch name" cell, activate the cell
// Arguments:   int row
//              int column
// Author:      Sigal Algranaty
// Date:        20/3/2012
// ---------------------------------------------------------------------------
void gdWatchView::onCellClicked(int row, int column)
{
    if (row == rowCount() - 1)
    {
        // Get the clicked item:
        QTableWidgetItem* pItem = item(row, column);
        GT_IF_WITH_ASSERT(pItem != NULL)
        {
            editItem(pItem);
        }
    }

    else
    {
        // Sanity check
        GT_IF_WITH_ASSERT(_pApplicationCommands != NULL)
        {
            // Display the currently selected watch expressions:
            displaySelectedWatchProperties();
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdWatchView::displaySelectedWatchProperties
// Description: Find the currently selected watch expressions and display in
//              properties view
// Author:      Sigal Algranaty
// Date:        22/3/2012
// ---------------------------------------------------------------------------
void gdWatchView::displaySelectedWatchProperties()
{
    if (gaIsDebuggedProcessSuspended())
    {
        // Collect the selected rows indices:
        QList<int> selectedRowsList;

        foreach (QModelIndex selectedItem, selectedIndexes())
        {
            if (selectedItem.isValid())
            {
                // If this is not the last item:
                if (selectedItem.row() != (rowCount() - 1))
                {
                    if (selectedRowsList.indexOf(selectedItem.row()) < 0)
                    {
                        selectedRowsList << selectedItem.row();
                    }
                }
            }
        }

        gdHTMLProperties htmlProps;
        gtString htmlPropertiesStr;

        if (selectedRowsList.size() == 1)
        {
            int row = selectedRowsList.first();
            gtString varName, varValue, varType;
            bool rc1 = getItemText(row, 0, varName);
            bool rc2 = getItemText(row, 1, varValue);

            if (!rc2)
            {
                varValue = AF_STR_NotAvailable;
            }

            bool rc3 = getItemText(row, 2, varType);

            if (!rc3)
            {
                varValue = AF_STR_NotAvailable;
            }

            GT_IF_WITH_ASSERT(rc1)
            {
                afHTMLContent htmlContent;
                htmlProps.buildWatchVariablePropertiesString(varName, varValue, varType, htmlContent);
                htmlContent.toString(htmlPropertiesStr);
            }
        }
        else
        {
            htmlProps.buildSimpleHTMLMessage(GD_STR_PropertiesWatchMultipleTitle, GD_STR_PropertiesWatchMultipleText, htmlPropertiesStr, true);
        }

        // Set the text:
        GT_IF_WITH_ASSERT(!htmlPropertiesStr.isEmpty())
        {
            gdPropertiesEventObserver::instance().setPropertiesFromText(acGTStringToQString(htmlPropertiesStr));
        }
    }
}

