//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdLocalsView.cpp
///
//==================================================================================

//------------------------------ gdLocalsView.cpp ------------------------------

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTAPIClasses/Include/Events/apCallStackFrameSelectedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdHTMLProperties.h>
#include <AMDTGpuDebuggingComponents/Include/gdPropertiesEventObserver.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdLocalsView.h>

#define GD_LOCALS_VIEW_LINE_HEIGHT 19


// ---------------------------------------------------------------------------
// Name:        gdLocalsView::gdLocalsView
// Description: Constructor
// Author:      Uri Shomroni
// Date:        8/9/2011
// ---------------------------------------------------------------------------
gdLocalsView::gdLocalsView(QWidget* pParent)
    : acTreeCtrl(pParent, 3, false),
      m_pAddWatchAction(NULL),
      m_stackDepth(-2),
      m_pApplicationCommands(NULL)
{
    // Get the application commands instance:
    m_pApplicationCommands = gdApplicationCommands::gdInstance();
    GT_ASSERT(m_pApplicationCommands != NULL);

    // Register myself to listen to debugged process events:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);

    // Initialize the list control headers:
    QStringList columnCaptions;
    columnCaptions << GD_STR_localsViewNameColumnHeader;
    columnCaptions << GD_STR_localsViewValueColumnHeader;
    columnCaptions << GD_STR_localsViewTypeColumnHeader;
    QTreeWidgetItem* pHeaders = new QTreeWidgetItem(columnCaptions);
    setHeaderItem(pHeaders);

    // Connect signals:
    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*, QTreeWidgetItem*)), this, SLOT(onItemSelected(QTreeWidgetItem*, QTreeWidgetItem*)));

    // Extend the context menu:
    extendContextMenu();

    // Enable multi-selection:
    setSelectionMode(QAbstractItemView::ExtendedSelection);
}


// ---------------------------------------------------------------------------
// Name:        gdLocalsView::~gdLocalsView
// Description: Destructor
// Author:      Uri Shomroni
// Date:        8/9/2011
// ---------------------------------------------------------------------------
gdLocalsView::~gdLocalsView()
{
    // Unregister myself from listening to debugged process events:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}


// ---------------------------------------------------------------------------
// Name:        gdLocalsView::onEvent
// Description: Is called when a debugged process event occurs.
// Author:      Uri Shomroni
// Date:        8/9/2011
// ---------------------------------------------------------------------------
void gdLocalsView::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent);  // unused
    apEvent::EventType eveType = eve.eventType();

    bool populateList = false;
    bool updateHeader = false;

    switch (eveType)
    {
        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        case apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED:
        case apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED_EXTERNALLY:
        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        {
            // Clear the list:
            m_stackDepth = -2;
            clearView();
            updateValueColumnHeader();
        }
        break;

        case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
        {
            if (gaCanGetHostVariables())
            {
                const afGlobalVariableChangedEvent& varEve = (const afGlobalVariableChangedEvent&)eve;

                if (afGlobalVariableChangedEvent::CHOSEN_THREAD_INDEX == varEve.changedVariableId())
                {
                    populateList = true;
                    updateHeader = true;
                }
            }
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
        {
            m_stackDepth = 0;

            if (gaIsInKernelDebugging() || gaIsInHSAKernelBreakpoint() || gaCanGetHostVariables())
            {
                populateList = true;
                updateHeader = true;
            }
        }
        break;

        case apEvent::AP_KERNEL_CURRENT_WORK_ITEM_CHANGED_EVENT:
        {
            populateList = true;
            updateHeader = true;
        }
        break;

        case apEvent::AP_BEFORE_KERNEL_DEBUGGING_EVENT:
        case apEvent::AP_AFTER_KERNEL_DEBUGGING_EVENT:
        case apEvent::AP_KERNEL_DEBUGGING_FAILED_EVENT:
        {
            updateHeader = true;
        }
        break;

        case apEvent::AP_CALL_STACK_FRAME_SELECTED_EVENT:
        {
            updateCallStackDepth(eve);
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

                if (!isEnabled)
                {
                    clearView();
                }
            }
        }
        break;

        default:
            // Ignore other events
            break;
    }

    if (populateList)
    {
        // Fill the list:
        populateLocalsList();
    }

    if (updateHeader)
    {
        // Update the value column header:
        updateValueColumnHeader();
    }
}

// ---------------------------------------------------------------------------
// Name:        gdLocalsView::populateLocalsList
// Description: Fills the locals list with the variable names and their values.
// Author:      Uri Shomroni
// Date:        11/9/2011
// ---------------------------------------------------------------------------
void gdLocalsView::populateLocalsList()
{
    // Clear the view:
    clearView();

    const gdGDebuggerGlobalVariablesManager& theGlobalVarsManager = gdGDebuggerGlobalVariablesManager::instance();

    if (gaIsInKernelDebugging() && theGlobalVarsManager.isKernelDebuggingThreadChosen())
    {
        // TO_DO: we can get the values directly - this should provide a performance improvement
        // Get the variable names:
        gtVector<apExpression> variableNames;
        bool rcNm = gaGetKernelDebuggingAvailableVariables(0, variableNames, false, m_stackDepth, true);
        GT_IF_WITH_ASSERT(rcNm)
        {
            // Get the current work item index:
            int currentWorkItemCoord[3] = { -1, -1, -1};
            bool rcCo = gaGetKernelDebuggingCurrentWorkItem(currentWorkItemCoord[0], currentWorkItemCoord[1], currentWorkItemCoord[2]);

            GT_IF_WITH_ASSERT(rcCo)
            {
                // Iterate the variables:
                int numberOfLocals = (int)variableNames.size();

                for (int i = 0 ; i < numberOfLocals; i++)
                {
                    // Get the variable value:
                    const gtString& currentVariableName = variableNames[i].m_name;
                    apExpression variableValue;
                    bool rcVal = gaGetKernelDebuggingExpressionValue(currentVariableName, currentWorkItemCoord, 2, variableValue);
                    GT_IF_WITH_ASSERT(rcVal)
                    {
                        QStringList newRowStringList;
                        newRowStringList << acGTStringToQString(variableValue.m_name);
                        newRowStringList << acGTStringToQString(variableValue.m_value);
                        newRowStringList << acGTStringToQString(variableValue.m_type);
                        QTreeWidgetItem* pItem = addItem(newRowStringList, NULL);

                        recursivelyAddLocalItemChildren(pItem, variableValue, variableValue.m_name);
                    }
                }
            }
        }
    }
    else if (gaIsInHSAKernelBreakpoint())
    {
        // TO_DO: we should get the values directly here, to improve performance:
        gtVector<apExpression> variableNames;
        bool rcVars = gaHSAListVariables(0, variableNames);
        size_t varCount = variableNames.size();

        if (rcVars && (0 < varCount))
        {
            apExpression variableValue;

            for (const apExpression& variableName : variableNames)
            {
                bool rcVar = gaHSAGetExpressionValue(variableName.m_name, 0, variableValue);

                if (rcVar)
                {
                    // Show HSAIL registers as hex values:
                    bool isReg = ((0 < variableName.m_name.length()) && ('$' == variableName.m_name[0]));

                    QStringList newRowStringList;
                    newRowStringList << acGTStringToQString(variableValue.m_name);
                    newRowStringList << acGTStringToQString(isReg ? variableValue.m_valueHex : variableValue.m_value);
                    newRowStringList << acGTStringToQString(variableValue.m_type);
                    addItem(newRowStringList, NULL);
                }
            }
        }
    }
    else if (gaCanGetHostVariables())
    {
        // TO_DO: we should get the values directly here, to improve performance:
        int threadIdx = theGlobalVarsManager.chosenThread();
        osThreadId threadId = OS_NO_THREAD_ID;
        bool rcThd = gaGetThreadId(threadIdx, threadId);
        GT_IF_WITH_ASSERT(rcThd && (OS_NO_THREAD_ID != threadId))
        {
            gtVector<apExpression> variableNames;
            bool rcLoc = gaGetThreadLocals(threadId, m_stackDepth, 0, variableNames, true);
            GT_IF_WITH_ASSERT(rcLoc)
            {
                // Iterate the variables:
                int numberOfLocals = (int)variableNames.size();

                for (int i = 0; i < numberOfLocals; i++)
                {
                    // Get the variable value:
                    const gtString& currentVariableName = variableNames[i].m_name;
                    apExpression variableValue;
                    bool rcVal = gaGetThreadExpressionValue(threadId, m_stackDepth, currentVariableName, 3, variableValue);
                    GT_IF_WITH_ASSERT(rcVal)
                    {
                        QStringList newRowStringList;
                        newRowStringList << acGTStringToQString(variableValue.m_name);
                        newRowStringList << acGTStringToQString(variableValue.m_value);
                        newRowStringList << acGTStringToQString(variableValue.m_type);
                        QTreeWidgetItem* pItem = addItem(newRowStringList, NULL);

                        // Add the expression's children:
                        recursivelyAddLocalItemChildren(pItem, variableValue, variableValue.m_name);
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdLocalsView::updateValueColumnHeader
// Description: Updates the "Value" column header with the work item coordinates
//              if they are available.
// Author:      Uri Shomroni
// Date:        20/9/2011
// ---------------------------------------------------------------------------
void gdLocalsView::updateValueColumnHeader()
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
    QTreeWidgetItem* pHeaderItem = headerItem();
    GT_IF_WITH_ASSERT(pHeaderItem != NULL)
    {
        pHeaderItem->setText(1, newHeader);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdLocalsView::extendContextMenu
// Description: Extend the context menu (add watch actions)
// Author:      Sigal Algranaty
// Date:        26/9/2011
// ---------------------------------------------------------------------------
void gdLocalsView::extendContextMenu()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pContextMenu != NULL)
    {
        _pContextMenu->addSeparator();

        // Create an action for the "Add Watch" menu item:
        m_pAddWatchAction = new QAction(AF_STR_SourceCodeAddWatch, _pContextMenu);

        // Add the action to the context menu:
        _pContextMenu->addAction(m_pAddWatchAction);

        // Connect the action to its handler:
        bool rcConnect = connect(m_pAddWatchAction, SIGNAL(triggered()), this, SLOT(onAddWatch()));
        GT_ASSERT(rcConnect);

        // Connect the menu to an about to show slot:
        rcConnect = connect(_pContextMenu, SIGNAL(aboutToShow()), this, SLOT(onAboutToShowTextContextMenu()));
        GT_ASSERT(rcConnect);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdLocalsView::recursivelyAddLocalItemChildren
// Description: Adds the tree items for the children of the local variable
// Author:      Uri Shomroni
// Date:        19/2/2012
// ---------------------------------------------------------------------------
void gdLocalsView::recursivelyAddLocalItemChildren(QTreeWidgetItem* pItem, const apExpression& currentVariable, const gtString& currentVariableQualifiedName)
{
    const gtVector<apExpression*>& variableChildren = currentVariable.children();

    for (const apExpression* pCurrentChild : variableChildren)
    {
        GT_IF_WITH_ASSERT(nullptr != pCurrentChild)
        {
            gtString currentMemberFullName = currentVariableQualifiedName;
            const gtString& currentMemberName = pCurrentChild->m_name;
            currentMemberFullName.append('.').append(currentMemberName);

            QStringList newRowStringList;
            newRowStringList << acGTStringToQString(currentMemberName);
            newRowStringList << acGTStringToQString(pCurrentChild->m_value);
            newRowStringList << acGTStringToQString(pCurrentChild->m_type);
            QTreeWidgetItem* pMemberItem = addItem(newRowStringList, nullptr, pItem);

            recursivelyAddLocalItemChildren(pMemberItem, *pCurrentChild, currentMemberFullName);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        gdLocalsView::onAddWatch
// Description: Handling the "Add Watch" command
// Author:      Sigal Algranaty
// Date:        25/9/2011
// ---------------------------------------------------------------------------
void gdLocalsView::onAddWatch()
{
    // Get the item at the context menu position:
    QTreeWidgetItem* pItemAtContextMenu = itemAt(_contextMenuPosition);
    GT_IF_WITH_ASSERT(pItemAtContextMenu != NULL)
    {
        // Get the current selected item:
        QString textSelected = pItemAtContextMenu->text(0);

        // Prepare a buffer to hold the variable name.
        gtString textForWatch;
        textForWatch.fromASCIIString(textSelected.toLatin1());

        // If that's a structure, we need to prepend the parent variable name.
        const QTreeWidgetItem* pParent = pItemAtContextMenu->parent();

        while (pParent != NULL)
        {
            // Extract the parent variable name.
            QString parentTxt = pParent->text(0);

            // Convert the parent variable name to wide characters string.
            gtString txtToPrepend;
            txtToPrepend.fromASCIIString(parentTxt.toLatin1());

            // Prepend the parent variable name and the '.' character.
            GT_IF_WITH_ASSERT(!txtToPrepend.isEmpty())
            {
                textForWatch.prepend('.').prepend(txtToPrepend);
            }

            // Make the parent the current item.
            pParent = pParent->parent();
        }

        if (!textForWatch.isEmpty())
        {
            // Get the watch view:
            GT_IF_WITH_ASSERT(m_pApplicationCommands != NULL)
            {
                bool rc = m_pApplicationCommands->addWatchVariable(textForWatch);
                GT_ASSERT(rc);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdLocalsView::onAboutToShowTextContextMenu
// Description: Is called when the text context menu is shown - enable / disable
//              the context menu actions according to the current debug situation
// Author:      Sigal Algranaty
// Date:        25/9/2011
// ---------------------------------------------------------------------------
void gdLocalsView::onAboutToShowTextContextMenu()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((m_pAddWatchAction != NULL))
    {
        // Check if add watch actions should be enabled:
        bool isEnabled = gaIsDebuggedProcessSuspended() && (gaIsInKernelDebugging() || gaIsInHSAKernelBreakpoint() || gaCanGetHostVariables());

        // Set the actions enable state:
        m_pAddWatchAction->setEnabled(isEnabled);
    }
}

// ---------------------------------------------------------------------------
// Name:        gdLocalsView::onCellClicked
// Description: When the user clicks on the "Type watch name" cell, activate the cell
// Arguments:   int row
//              int column
// Author:      Sigal Algranaty
// Date:        20/3/2012
// ---------------------------------------------------------------------------
void gdLocalsView::onItemSelected(QTreeWidgetItem* pCurrent, QTreeWidgetItem* pPrevious)
{
    (void)(pPrevious);  // unused

    if (pCurrent != NULL)
    {
        // Do not display properties when the process is running:
        if (gaIsDebuggedProcessSuspended())
        {
            // Sanity check
            GT_IF_WITH_ASSERT(m_pApplicationCommands != NULL)
            {
                // Build the current variable strings:
                gtString variableName, variableType;
                gtVector<gtString> variableSubName, variableSubValues;

                if (pCurrent->columnCount() >= 2)
                {
                    QString nameStr = pCurrent->text(0);
                    QString valStr = pCurrent->text(1);
                    QString typeStr = pCurrent->text(2);

                    variableName.fromASCIIString(nameStr.toLatin1().data());
                    variableType.fromASCIIString(typeStr.toLatin1().data());
                    gtString variableValue;
                    variableValue.fromASCIIString(valStr.toLatin1().data());
                    variableSubValues.push_back(variableValue);
                }

                gdHTMLProperties htmlProps;
                gtString htmlPropertiesStr;
                afHTMLContent htmlContent;
                htmlProps.buildLocalVariablePropertiesString(variableName, variableType, variableSubName, variableSubValues, htmlContent);
                htmlContent.toString(htmlPropertiesStr);

                // Set the text:
                gdPropertiesEventObserver::instance().setPropertiesFromText(acGTStringToQString(htmlPropertiesStr));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdLocalsView::updateCallStackDepth
// Description:
// Arguments:   const apEvent& eve
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        19/7/2012
// ---------------------------------------------------------------------------
void gdLocalsView::updateCallStackDepth(const apEvent& eve)
{
    const apCallStackFrameSelectedEvent& callStackEvent = (const apCallStackFrameSelectedEvent&)eve;
    int newFrameIndex = callStackEvent.frameIndex();

    if (m_stackDepth != newFrameIndex)
    {
        m_stackDepth = newFrameIndex;

        populateLocalsList();
    }
}
