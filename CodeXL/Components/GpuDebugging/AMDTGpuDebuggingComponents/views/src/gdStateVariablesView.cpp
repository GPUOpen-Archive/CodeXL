//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdStateVariablesView.cpp
///
//==================================================================================

//------------------------------ gdStateVariablesView.cpp ------------------------------

// Warnings:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>

//Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>
#include <QTextBrowser>

// Infra:
#include <AMDTBaseTools/Include/gtIgnoreCompilerWarnings.h>
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunSuspendedEvent.h>
#include <AMDTAPIClasses/Include/Events/apMonitoredObjectsTreeEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/apParameters.h>
#include <AMDTAPIClasses/Include/apAPIConnectionType.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acColours.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/src/afUtils.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdCommandIDs.h>
#include <AMDTGpuDebuggingComponents/Include/gdPropertiesEventObserver.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTGpuDebuggingComponents/Include/dialogs/gdStateVariablesDialog.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdStateVariablesView.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>


// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::gdStateVariablesView
// Description: Constructor.
// Arguments:   pParent - My parent widget.
// Author:      Yaki Tebeka
// Date:        1/11/2003
// ---------------------------------------------------------------------------
gdStateVariablesView::gdStateVariablesView(QWidget* pParent)
    : acListCtrl(pParent, AC_DEFAULT_LINE_HEIGHT, true), m_pShowDialogAction(NULL), m_pExportStateVariablesAction(NULL), m_whileDoubleClick(false)
{
    // Connect the item click and double click events:
    bool rcConnect = connect(this, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(onItemClicked(QTableWidgetItem*)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(this, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(itemDoubleClicked(QTableWidgetItem*)));
    GT_ASSERT(rcConnect);

    rcConnect = connect(this, SIGNAL(currentItemChanged(QTableWidgetItem*, QTableWidgetItem*)), this, SLOT(onCurrentItemChanged(QTableWidgetItem*, QTableWidgetItem*)));
    GT_ASSERT(rcConnect);

    // Set the initial value of the dialog
    setListCtrlColumns();

    // Setting the columns width
    resizeColumnsToContents();

    // Extend the context menu:
    extendContextMenu();

    // Add the message line:
    QStringList newItemStrings;
    newItemStrings << GD_STR_StateVariablesViewDoubleClickToAdd << "";
    addRow(newItemStrings, NULL);
    QTableWidgetItem* pFirstItemInNewLine = item(0, 0);
    GT_IF_WITH_ASSERT(pFirstItemInNewLine != NULL)
    {
        // Set it to be gray:
        pFirstItemInNewLine->setTextColor(acQLIST_EDITABLE_ITEM_COLOR);
    }

    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);
}


// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::~gdStateVariablesView
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        1/11/2003
// ---------------------------------------------------------------------------
gdStateVariablesView::~gdStateVariablesView()
{
    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);

    GT_ASSERT_EX(false, L"Release item datas");
}


// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::onEvent
// Description: Is called when a debugged process event occurs.
//              Send the event to the appropriate event function for displaying
//              it to the user.
// Arguments:   eve - The debugged process event.
// Author:      Yaki Tebeka
// Date:        4/4/2004
// ---------------------------------------------------------------------------
void gdStateVariablesView::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent);  // unused
    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    // Handle the event according to its type:
    switch (eventType)
    {
        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        {
            onProcessCreated();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        {
            onProcessTerminated();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE:
        {
            onProcessCreationFailed();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
        {
            onProcessRunSuspended((const apDebuggedProcessRunSuspendedEvent&)eve);
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        {
            onProcessRunResumed();
        }
        break;

        case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
        {
            // Down cast the event:
            const afGlobalVariableChangedEvent& variableChangedEvent = (const afGlobalVariableChangedEvent&)eve;

            // Handle it:
            onGlobalVariableChanged(variableChangedEvent);
        }
        break;

        case apEvent::GD_MONITORED_OBJECT_SELECTED_EVENT:
        {
            onTreeItemSelection((const apMonitoredObjectsTreeSelectedEvent&)eve);
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
            }
        }
        break;

        default:
            // Do nothing...
            break;
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::onGlobalVariableChanged
// Description: Is called when an application global variable changes its value.
// Author:      Avi Shapira
// Date:        11/5/2004
// ---------------------------------------------------------------------------
void gdStateVariablesView::onGlobalVariableChanged(const afGlobalVariableChangedEvent& event)
{
    // Get id of the global variable that was changed:
    afGlobalVariableChangedEvent::GlobalVariableId variableId = event.changedVariableId();

    // If the chosen context was changed:
    if (variableId == afGlobalVariableChangedEvent::CHOSEN_CONTEXT_ID)
    {
        // Get the new chosen context id:
        gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();

        // Set the new context id:
        _activeContextId = globalVarsManager.chosenContext();

        // Update the list for the selected context:
        updateStateVariablesList();
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::onTreeItemSelection
// Description: Is handling the tree selection event
// Arguments:   const apMonitoredObjectsTreeSelectedEvent& eve
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        27/12/2011
// ---------------------------------------------------------------------------
void gdStateVariablesView::onTreeItemSelection(const apMonitoredObjectsTreeSelectedEvent& eve)
{
    // Get the item data;
    afApplicationTreeItemData* pItemData = (afApplicationTreeItemData*)eve.selectedItemData();

    if (pItemData != NULL)
    {
        gdDebugApplicationTreeData* pGDData = qobject_cast<gdDebugApplicationTreeData*>(pItemData->extendedItemData());

        if (pGDData != NULL)
        {
            // If the debugged process is suspended:
            bool isDebuggedProcessSuspended = gaIsDebuggedProcessSuspended();

            if (isDebuggedProcessSuspended)
            {
                // Set the new context id:
                gdGDebuggerGlobalVariablesManager& globalVarsManager = gdGDebuggerGlobalVariablesManager::instance();
                globalVarsManager.setChosenContext(pGDData->_contextId);

                if (pGDData->_contextId != _activeContextId)
                {
                    // Set the new context:
                    _activeContextId = pGDData->_contextId;

                    // Update the list:
                    updateStateVariablesList();
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::setListCtrlColumns
// Description: Create the two Columns
// Author:      Avi Shapira
// Date:        11/7/2004
// ---------------------------------------------------------------------------
void gdStateVariablesView::setListCtrlColumns()
{
    // Initialize the list control headers:
    QStringList columnCaptions;
    columnCaptions << GD_STR_StateVariablesViewVarName;
    columnCaptions << AF_STR_ValueA;
    initHeaders(columnCaptions, false);
}



// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::AddStateVariable
// Description: Add state variable and its value to the list control
// Arguments:   gtString &variableName
// Author:      Avi Shapira
// Date:        21/9/2005
// ---------------------------------------------------------------------------
void gdStateVariablesView::AddStateVariable(gtString& variableName)
{
    bool rc = true;
    gtString variableValue;
    int variableId = 0;

    // Get the variable id:
    gtAutoPtr<apParameter> aptrStateVariableValue;
    rc = gaGetOpenGLStateVariableId(variableName, variableId);

    // Check if the process is suspended, and if the current triggering context is an OpenGL context:
    apContextID contextId = _activeContextId;

    if (contextId.isDefault())
    {
        rc = gaGetBreakpointTriggeringContextId(contextId);
    }

    if (rc)
    {
        // Get the variable value:
        rc = gaGetOpenGLStateVariableValue(contextId._contextId, variableId, aptrStateVariableValue);

        if (rc)
        {
            aptrStateVariableValue->valueAsString(variableValue);
        }
        else if (gaIsInOpenGLBeginEndBlock(contextId._contextId))
        {
            // If we are in glBegin-glEnd Block:
            variableValue = GD_STR_StateVariablesNoValueBeginEnd;
        }
        else if (contextId.isOpenCLContext())
        {
            variableValue = GD_STR_StateVariablesNoValueInCLContext;
        }
        else
        {
            variableValue = GD_STR_StateVariablesNoValue;
        }
    }

    // Check if the variable exists:
    Qt::MatchFlags findFlags = Qt::MatchExactly;
    QList<QTableWidgetItem*> matchingItems = findItems(acGTStringToQString(variableName), findFlags);

    if (matchingItems.isEmpty())
    {
        // Create a state variable item data:
        gdStateVarListItemData* pItemData = new gdStateVarListItemData;
        pItemData->_variableValue = variableValue;

        // Insert the variable:
        QStringList rowTexts;
        rowTexts << acGTStringToQString(variableName);
        rowTexts << acGTStringToQString(variableValue);

        // Add the item before the last row, to jump over the "Double click to add variables" line:
        addRow(rowTexts, pItemData, false, Qt::Unchecked, NULL, true);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::onProcessCreated
// Description: Is called when the debugged process is created.
//              Disable the listCTRL
// Author:      Avi Shapira
// Date:        13/7/2004
// ---------------------------------------------------------------------------
void gdStateVariablesView::onProcessCreated()
{
    setEnabled(false);
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::onProcessTerminated
// Description: Is called when the debugged process is terminated.
// Author:      Avi Shapira
// Date:        13/7/2004
// ---------------------------------------------------------------------------
void gdStateVariablesView::onProcessTerminated()
{
    // Enable me:
    setEnabled(true);

    // Get the amount of rows:
    int amountOfRows = numberOfVariableRows();

    // Iterate the items and set text to N/A:
    for (int i = 0; i < amountOfRows; i++)
    {
        // Set the variables values to "N/A"
        setItemText(i, 1, GD_STR_StateVariablesNoValueA);

        // Changing back the color of the non changed variables:
        setItemTextColor(i, 0, Qt::black);
        setItemTextColor(i, 1, Qt::black);

        // Initialize the old value of the variable:
        gdStateVarListItemData* pItemData = (gdStateVarListItemData*)(getItemData(i));
        GT_IF_WITH_ASSERT(pItemData != NULL)
        {
            pItemData->_variableValue = GD_STR_StateVariablesNoValue;
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::onProcessCreationFailed
// Description: Is called when the debugged process creation is failed.
// Author:      Sigal Algranaty
// Date:        11/2/2010
// ---------------------------------------------------------------------------
void gdStateVariablesView::onProcessCreationFailed()
{
    // Enable me:
    setEnabled(true);

    // Get the amount of rows:
    int amountOfRows = numberOfVariableRows();

    // Iterate the items and set text to N/A:
    for (int i = 0; i < amountOfRows; i++)
    {
        // Set the variables values to "N/A":
        setItemText(i, 1, GD_STR_StateVariablesNoValueA);

        // Changing back the color of the non changed variables:
        setItemTextColor(i, 0, Qt::black);
        setItemTextColor(i, 1, Qt::black);

        // Initialize the old value of the variable:
        gdStateVarListItemData* pItemData = (gdStateVarListItemData*)(getItemData(i));
        GT_IF_WITH_ASSERT(pItemData != NULL)
        {
            pItemData->_variableValue = GD_STR_StateVariablesNoValue;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::onProcessRunResumed
// Description: Is called when the debugged process is resumed.
//              Disable the listCTRL
// Author:      Avi Shapira
// Date:        13/7/2004
// ---------------------------------------------------------------------------
void gdStateVariablesView::onProcessRunResumed()
{
    // Disable the control:
    setEnabled(false);
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::onProcessRunSuspended
// Description: Is called when the debugged process is suspended.
//              Enable the listCTRL
//              Calls updateStateVariablesList()
// Author:      Avi Shapira
// Date:        13/7/2004
// ---------------------------------------------------------------------------
void gdStateVariablesView::onProcessRunSuspended(const apDebuggedProcessRunSuspendedEvent& event)
{
    (void)(event);  // unused
    // Get the context which triggered the process suspension:
    bool rc = gaIsAPIConnectionActive(AP_SPIES_UTILITIES_API_CONNECTION);

    if (rc)
    {
        rc = gaGetBreakpointTriggeringContextId(_activeContextId);
        GT_IF_WITH_ASSERT(rc)
        {
            // Enable the view:
            setEnabled(true);

            // Update the list of variables values:
            updateStateVariablesList();
        }
    }

    if (!rc)
    {
        // Get the list size
        int listSize = numberOfVariableRows();

        // Add no value:
        for (int i = 0; i < listSize; i++)
        {
            setItemText(i, 1, GD_STR_StateVariablesNoValueA);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::updateStateVariablesList
// Description: Update the value of the variables in the listCTRL
// Author:      Avi Shapira
// Date:        13/7/2004
// ---------------------------------------------------------------------------
void gdStateVariablesView::updateStateVariablesList()
{
    gtAutoPtr<apParameter> aptrStateVariableValue;
    int stateVariableId = 0;
    gtString stateVariableValue;
    bool rc = true;

    // Get the list size - to know at what row to add the new var
    int listSize = numberOfVariableRows();

    for (int i = 0; i < listSize; i++)
    {
        // Get the variable name for item i:
        gtString stateVariableName, stateVariableOldValue;
        QString varName;
        rc = getItemText(i, 0, varName);
        stateVariableName.fromASCIIString(varName.toLatin1());

        // Get the old value of the variable
        gdStateVarListItemData* pItemData = (gdStateVarListItemData*)(getItemData(i));

        if (pItemData != NULL)
        {
            // Get the selected variable previous value:
            stateVariableOldValue = pItemData->_variableValue;
        }

        // Get the variable name
        rc = gaGetOpenGLStateVariableId(stateVariableName, stateVariableId);

        // OpenGL state variables are only supported in OpenGL contexts:
        bool isInBeginEndBlock = false;

        if (_activeContextId.isOpenGLContext())
        {
            // Get the variable value
            rc = rc && gaGetOpenGLStateVariableValue(_activeContextId._contextId, stateVariableId, aptrStateVariableValue);

            if (!rc)
            {
                // If we failed, check if this is due to being in a begin-end block:
                isInBeginEndBlock = gaIsInOpenGLBeginEndBlock(_activeContextId._contextId);
            }
        }
        else // !_chosenContextId.isOpenGLContext()
        {
            rc = false;
        }

        if (rc)
        {
            // Translate the value into a string:
            aptrStateVariableValue->valueAsString(stateVariableValue);

            // Remove new lines:
            stateVariableValue.replace(AF_STR_NewLine, AF_STR_Space, true);

            // Set the user data
            pItemData->_variableValue = stateVariableValue;

        }
        else
        {
            // If we are in glBegin-glEnd Block:
            if (isInBeginEndBlock)
            {
                stateVariableValue = GD_STR_StateVariablesNoValueBeginEnd;
            }
            else // !isInBeginEndBlock
            {
                if (_activeContextId.isOpenCLContext())
                {
                    stateVariableValue = GD_STR_StateVariablesNoValueInCLContext;
                }
                else
                {
                    stateVariableValue = GD_STR_StateVariablesNoValue;
                }
            }
        }

        // Set the item text:
        setItemText(i, 1, acGTStringToQString(stateVariableValue));

        // Set the variables values into the list
        if (stateVariableValue == stateVariableOldValue || stateVariableValue == GD_STR_StateVariablesNoValue || stateVariableValue == GD_STR_StateVariablesNoValueBeginEnd)
        {
            // Changing back the color of the non changed variables:
            setItemTextColor(i, 0, Qt::black);
            setItemTextColor(i, 1, Qt::black);
        }
        else
        {
            if (stateVariableOldValue != GD_STR_StateVariablesNoValue)
            {
                // Changing back the color of the non changed variables:
                setItemTextColor(i, 0, Qt::blue);
                setItemTextColor(i, 1, Qt::blue);
            }
        }
    }


    afApplicationCommands* pApplicationCommands = afApplicationCommands::instance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
        // Build the window caption:
        gtString caption = pApplicationCommands->captionPrefix();

        if (_activeContextId.isDefault())
        {
            caption.append(GD_STR_StateVariablesViewCaptionDefault);
        }
        else
        {
            // Get my display string:
            gtString contextStr;
            _activeContextId.toString(contextStr);

            caption.appendFormattedString(GD_STR_StateVariablesViewCaptionDefaultWithContext, contextStr.asCharArray());
        }

        // Set the caption for the statistics view:
        pApplicationCommands->setWindowCaption(this, caption);
    }

}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::onItemClicked
// Description: Write the details of the selected variable to the properties view
// Arguments:   QTableWidgetItem* pSelectedWidgetItem - the item selected
// Author:      Sigal Algranaty
// Date:        13/12/2011
// ---------------------------------------------------------------------------
void gdStateVariablesView::onItemClicked(QTableWidgetItem* pSelectedWidgetItem)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pSelectedWidgetItem != NULL)
    {
        // Select the item in the requested row:
        QList<int> rows;
        rows << pSelectedWidgetItem->row();

        // Select the requested row:
        selectRows(rows);
    }

    // Call the base class implementation:
    acListCtrl::onItemClicked(pSelectedWidgetItem);
}



// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::onCurrentItemChanged
// Description: This signal is fired when the user selects with keyboard arrows
// Arguments:   QTableWidgetItem * pCurrent
//              QTableWidgetItem* pPrevious
// Author:      Sigal Algranaty
// Date:        22/3/2012
// ---------------------------------------------------------------------------
void gdStateVariablesView::onCurrentItemChanged(QTableWidgetItem* pCurrent, QTableWidgetItem* pPrevious)
{
    (void)(pCurrent);  // unused
    (void)(pPrevious);  // unused

    if (!m_whileDoubleClick)
    {
        // Find the currently selected breakpoint/s:
        QList<int> rows;
        QModelIndexList listOfSelectedIndexes = selectedIndexes();

        foreach (QModelIndex index, listOfSelectedIndexes)
        {
            if (index.isValid())
            {
                if (index.row() < numberOfVariableRows())
                {
                    if (rows.indexOf(index.row()) < 0)
                    {
                        rows << index.row();
                    }
                }
            }
        }

        // Select the requested row:
        selectRows(rows);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::selectRows
// Description: Select the rows in the list of rows
// Arguments:   const gtVector<int>& rowsToSelect
// Author:      Sigal Algranaty
// Date:        13/12/2011
// ---------------------------------------------------------------------------
void gdStateVariablesView::selectRows(const QList<int>& rowsToSelect)
{
    if (gaIsDebuggedProcessSuspended())
    {
        // Build an HTML string with the properties for the selected rows:
        gdHTMLProperties htmlBuilder;
        gtString propertiesViewMessage;

        // If we selected more than one item, multipleItemsSelected = true
        bool multipleItemsSelected = (rowsToSelect.size() > 1);

        if (multipleItemsSelected || (rowsToSelect.size() == 0))
        {
            // Multiple items were selected, show the "Multiple items selected" message:
            afHTMLContent htmlContent;
            htmlBuilder.buildMultipleItemPropertiesString(GD_STR_StateVariablesPropertiesTitle, GD_STR_StateVariablesPropertiesStateVarItem, htmlContent);
            htmlContent.toString(propertiesViewMessage);

        }
        else
        {
            // Get the current row:
            int currentRow = rowsToSelect[0];
            int variableRowsNumber = numberOfVariableRows();

            // Get the widget item for column 0 (variable name):
            QTableWidgetItem* pItem0 = item(currentRow, 0);

            // Get the widget item for column 1 (variable value):
            QTableWidgetItem* pItem1 = item(currentRow, 1);

            GT_IF_WITH_ASSERT((pItem0 != NULL) && (pItem1 != NULL))
            {
                // Get the selected variable name & value:
                gtString variableName, variableValue;

                if (currentRow < variableRowsNumber)
                {
                    variableName.fromASCIIString(pItem0->text().toLatin1());
                    variableValue.fromASCIIString(pItem1->text().toLatin1());
                }

                // Build the HTML string:
                afHTMLContent htmlContent;
                htmlBuilder.buildStateVariablePropertiesString(variableName, variableValue, htmlContent);
                htmlContent.toString(propertiesViewMessage);

                // Set the properties text:
                gdPropertiesEventObserver::instance().setPropertiesFromText(acGTStringToQString(propertiesViewMessage));
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdBreakpointsView::extendContextMenu
// Description: Extend the context menu for the list control
// Author:      Sigal Algranaty
// Date:        13/12/2011
// ---------------------------------------------------------------------------
void gdStateVariablesView::extendContextMenu()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(m_pContextMenu != NULL)
    {
        //Enable m_pDeleteAction
        acListCtrl::m_pDeleteAction->setEnabled(true);

        // Add a separator:
        m_pContextMenu->addSeparator();

        // Create an open breakpoints dialog action:
        m_pShowDialogAction = m_pContextMenu->addAction(GD_STR_StateVariablesAddVariable, this, SLOT(openStateVariableDialog()));

        m_pContextMenu->addSeparator();

        // Create an open breakpoints dialog action:
        m_pExportStateVariablesAction = m_pContextMenu->addAction(GD_STR_contextSaveStateVariables, this, SLOT(onSaveSnapShot()));
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::onEdit_RemoveSelected(wxCommandEvent& event)
// Description: Delete all the selected items from the "State Variables" list
// Author:      Yaki Tebeka
// Date:        14/9/2005
// ---------------------------------------------------------------------------
void gdStateVariablesView::onDeleteSelected()
{
    // Get the selected items:
    QList<QTableWidgetItem*> selectedItemsList = selectedItems();

    // Collect the row number selected:
    gtVector<int> rowsToRemove;

    foreach (QTableWidgetItem* pItem, selectedItemsList)
    {
        GT_IF_WITH_ASSERT(pItem != NULL)
        {
            gtVector<int>::iterator iter = gtFind(rowsToRemove.begin(), rowsToRemove.end(), pItem->row());

            if (iter == rowsToRemove.end())
            {
                rowsToRemove.push_back(pItem->row());
            }
        }
    }

    // Sort the rows:
    gtSort(rowsToRemove.begin(), rowsToRemove.end());

    // Iterate and remove rows:
    int variableRowsNumber = numberOfVariableRows();

    for (int i = rowsToRemove.size() - 1; i >= 0; i--)
    {
        // Do not remove the message line:
        int rowIdx = rowsToRemove[i];

        if (rowIdx < variableRowsNumber)
        {
            removeRow(rowIdx);
        }
    }

    // Also do the work of base class
    acListCtrl::onDeleteSelected();
}


// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::deleteAllStateVariables
// Description: Clear the view - delete all state variables.
// Author:      Avi Shapira
// Date:        21/9/2005
// ---------------------------------------------------------------------------
void gdStateVariablesView::deleteAllStateVariables()
{
    int rowsAmount = numberOfVariableRows();

    for (int i = rowsAmount - 1; i >= 0; i--)
    {
        removeRow(i);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::openStateVariableDialog
// Description: Opens the state variable dialog with activatedStateVar selected
// Arguments:   const QString& activatedStateVar - the selected variable
// Author:      Yoni Rabin
// Date:        28/5/2012
// ---------------------------------------------------------------------------
void gdStateVariablesView::openStateVariableDialog(const QString& activatedStateVar)
{
    gdStateVariablesDialog dialog(NULL);
    dialog.setActivatedStateVar(activatedStateVar);
    int rc = dialog.exec();

    if (QDialog::Accepted == rc)
    {
        deleteAllStateVariables();
        gtVector<gtString> vec = dialog.getSelectedStateVariables();

        for (gtVector<gtString>::iterator it = vec.begin(); it != vec.end(); ++it)
        {
            AddStateVariable(*it);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::itemDoubleClicked
// Description: Open the state variable dialog
// Arguments:   wxListEvent &eve
// Author:      Avi Shapira
// Date:        21/9/2005
// ---------------------------------------------------------------------------
void gdStateVariablesView::itemDoubleClicked(QTableWidgetItem* pWidgetItem)
{
    if (!m_whileDoubleClick)
    {
        // Ignore double click until after we exit this call:
        m_whileDoubleClick = true;

        // Sanity check:
        GT_IF_WITH_ASSERT(pWidgetItem != NULL)
        {
            // Get the activated item StateVar:
            QString activatedStateVar = pWidgetItem->text();
            openStateVariableDialog(activatedStateVar);
        }

        // Call the base class implementation:
        acListCtrl::itemDoubleClicked(pWidgetItem);

        // Stop ignoring double click events:
        m_whileDoubleClick = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::onEdit_SaveSnapShot
// Description: Execute the export state variables command
// Arguments:   wxCommandEvent& event
// Author:      Sigal Algranaty
// Date:        18/9/2011
// ---------------------------------------------------------------------------
void gdStateVariablesView::onSaveSnapShot()
{
    // Get the application commands handler:
    gdApplicationCommands* pApplicationCommands = gdApplicationCommands::gdInstance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
        pApplicationCommands->onFileSaveStateVariables();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::focusInEvent
// Description: Handle the view focus in event (display the selected variables
//              properties
// Arguments:   QFocusEvent * pFocusEvent
// Author:      Sigal Algranaty
// Date:        13/12/2011
// ---------------------------------------------------------------------------
void gdStateVariablesView::focusInEvent(QFocusEvent* pFocusEvent)
{
    (void)(pFocusEvent);  // unused
    // Find the properties view:
    gdApplicationCommands* pApplicationCommands = gdApplicationCommands::gdInstance();
    GT_IF_WITH_ASSERT(pApplicationCommands != NULL)
    {
        // Find the selected state variable:
        QModelIndexList selectedItems = selectedIndexes();

        if (selectedItems.size() > 0)
        {
            // Collect the selected rows:
            QList<int> rows;

            foreach (QModelIndex index, selectedItems)
            {
                rows << index.row();
            }

            selectRows(rows);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::onAboutToShowContextMenu
// Description: Override the base class's about to show function
// Author:      Sigal Algranaty
// Date:        13/12/2011
// ---------------------------------------------------------------------------
void gdStateVariablesView::onAboutToShowContextMenu()
{
    // Call the base class implementation:
    acListCtrl::onAboutToShowContextMenu();

    // Set the specific actions enable state:
    GT_IF_WITH_ASSERT((m_pShowDialogAction != NULL))
    {
        m_pShowDialogAction->setEnabled(true);
    }

    GT_IF_WITH_ASSERT((m_pDeleteAction != NULL))
    {
        m_pDeleteAction->setEnabled(selectedIndexes().size() > 0);
    }

    GT_IF_WITH_ASSERT((m_pExportStateVariablesAction != NULL))
    {
        m_pExportStateVariablesAction->setEnabled(numberOfVariableRows() > 0);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::areItemsSelected
// Description: Are there items selected?
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        13/12/2011
// ---------------------------------------------------------------------------
bool gdStateVariablesView::areItemsSelected()
{
    bool retVal = (selectedIndexes().size() > 0);
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gdStateVariablesView::numberOfVariableRows
// Description: Returns the number of rows that contain state variables, ignoring
//              any message lines and the such
// Author:      Uri Shomroni
// Date:        21/3/2012
// ---------------------------------------------------------------------------
int gdStateVariablesView::numberOfVariableRows()
{
    // Ignore the "Double-click to add" line:
    return (rowCount() - 1);
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        mouseDoubleClickEvent
/// \brief Description: Handle double click event in the state variables view
/// \param[in]          pMouseEvent
/// \return void
/// -----------------------------------------------------------------------------------------------
void gdStateVariablesView::mouseDoubleClickEvent(QMouseEvent* pMouseEvent)
{
    (void)(pMouseEvent);  // unused
    // Get the activated item StateVar:
    openStateVariableDialog();
}
