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
      m_pAddWatchAction(NULL), m_pAddMultiWatchAction(NULL),
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
        // Get the variable names:
        gtVector<gtString> variableNames;
        bool rcNm = gaGetKernelDebuggingAvailableVariables(variableNames, false, m_stackDepth);
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
                    const gtString& currentVariableName = variableNames[i];
                    gtString variableValue;
                    gtString variableValueHex;
                    gtString variableType;
                    bool rcVal = gaGetKernelDebuggingVariableValueString(currentVariableName, currentWorkItemCoord, variableValue, variableValueHex, variableType);
                    GT_IF_WITH_ASSERT(rcVal)
                    {
                        QStringList newRowStringList;
                        newRowStringList << acGTStringToQString(currentVariableName);
                        newRowStringList << acGTStringToQString(variableValue);
                        newRowStringList << acGTStringToQString(variableType);
                        QTreeWidgetItem* pItem = addItem(newRowStringList, NULL);

                        recursivelyAddLocalItemChildren(pItem, currentVariableName, currentWorkItemCoord);
                    }
                }
            }
        }
    }
    else if (gaIsInHSAKernelBreakpoint())
    {
        gtVector<gtString> variableNames;
        bool rcVars = gaHSAListVariables(variableNames);
        size_t varCount = variableNames.size();

        if (rcVars && (0 < varCount))
        {
            gtString variableValue;
            gtString variableValueHex;
            gtString variableType;

            for (const gtString& variableName : variableNames)
            {
                variableValue.makeEmpty();
                bool rcVar = gaHSAGetVariableValue(variableName, variableValue, variableValueHex, variableType);

                if (rcVar)
                {
                    // Show HSAIL registers as hex values:
                    bool isReg = ((0 < variableName.length()) && ('$' == variableName[0]));

                    QStringList newRowStringList;
                    newRowStringList << acGTStringToQString(variableName);
                    newRowStringList << acGTStringToQString(isReg ? variableValueHex : variableValue);
                    newRowStringList << acGTStringToQString(variableType);
                    addItem(newRowStringList, NULL);
                }
            }
        }
    }
    else if (gaCanGetHostVariables())
    {
        int threadIdx = theGlobalVarsManager.chosenThread();
        osThreadId threadId = OS_NO_THREAD_ID;
        bool rcThd = gaGetThreadId(threadIdx, threadId);
        GT_IF_WITH_ASSERT(rcThd && (OS_NO_THREAD_ID != threadId))
        {
            gtVector<gtString> variableNames;
            bool rcLoc = gaGetThreadLocals(threadId, m_stackDepth, variableNames);
            GT_IF_WITH_ASSERT(rcLoc)
            {
                // Iterate the variables:
                int numberOfLocals = (int)variableNames.size();

                for (int i = 0; i < numberOfLocals; i++)
                {
                    // Get the variable value:
                    const gtString& currentVariableName = variableNames[i];
                    gtString variableValue;
                    gtString variableValueHex;
                    gtString variableType;
                    bool rcVal = gaGetThreadVariableValue(threadId, m_stackDepth, currentVariableName, variableValue, variableValueHex, variableType);
                    GT_IF_WITH_ASSERT(rcVal)
                    {
                        QStringList newRowStringList;
                        newRowStringList << acGTStringToQString(currentVariableName);
                        newRowStringList << acGTStringToQString(variableValue);
                        newRowStringList << acGTStringToQString(variableType);
                        QTreeWidgetItem* pItem = addItem(newRowStringList, NULL);

                        // TO_DO: support structs?
                        (void)pItem;
                        // recursivelyAddLocalItemChildren(pItem, currentVariableName, currentWorkItemCoord);
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
// Description: Extend the context menu (add watch & multiwatch actions)
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

        // Create an action for the "Add Multi Watch" menu item:
        m_pAddMultiWatchAction = new QAction(AF_STR_SourceCodeAddMultiWatch, _pContextMenu);

        // Add the action to the context menu:
        _pContextMenu->addAction(m_pAddMultiWatchAction);

        // Connect the action to its handler:
        rcConnect = connect(m_pAddMultiWatchAction, SIGNAL(triggered()), this, SLOT(onAddMultiWatch()));
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
void gdLocalsView::recursivelyAddLocalItemChildren(QTreeWidgetItem* pItem, const gtString& currentVariableName, const int currentWorkItemCoord[3])
{
    gtVector<gtString> variableChildren;
    bool rcVars = gaGetKernelDebuggingVariableMembers(currentVariableName, variableChildren);
    GT_IF_WITH_ASSERT(rcVars)
    {
        int numberOfChildren = (int)variableChildren.size();

        for (int i = 0; i < numberOfChildren; i++)
        {
            gtString currentMemberFullName = currentVariableName;
            const gtString& currentMemberName = variableChildren[i];
            currentMemberFullName.append('.').append(currentMemberName);
            gtString variableValue;
            gtString variableValueHex;
            gtString variableType;
            bool rcVal = gaGetKernelDebuggingVariableValueString(currentMemberFullName, currentWorkItemCoord, variableValue, variableValueHex, variableType);
            GT_IF_WITH_ASSERT(rcVal)
            {
                QStringList newRowStringList;
                newRowStringList << acGTStringToQString(currentMemberName);
                newRowStringList << acGTStringToQString(variableValue);
                newRowStringList << acGTStringToQString(variableType);
                QTreeWidgetItem* pMemberItem = addItem(newRowStringList, NULL, pItem);

                recursivelyAddLocalItemChildren(pMemberItem, currentMemberFullName, currentWorkItemCoord);
            }
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
// Name:        gdLocalsView::onAddMultiWatch
// Description: Handling the "Add Multi Watch" command
// Author:      Sigal Algranaty
// Date:        25/9/2011
// ---------------------------------------------------------------------------
void gdLocalsView::onAddMultiWatch()
{
    // Get the item at the context menu position:
    QTreeWidgetItem* pItemAtContextMenu = itemAt(_contextMenuPosition);
    GT_IF_WITH_ASSERT(pItemAtContextMenu != NULL)
    {
        // Get the current selected item:
        QString textSelected = pItemAtContextMenu->text(0);

        gtString textForWatch;
        textForWatch.fromASCIIString(textSelected.toLatin1());

        if (!textForWatch.isEmpty())
        {
            // Get the watch view:
            GT_IF_WITH_ASSERT(m_pApplicationCommands != NULL)
            {
                bool rc = m_pApplicationCommands->displayMultiwatchVariable(textForWatch);
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
    GT_IF_WITH_ASSERT((m_pAddWatchAction != NULL) && (m_pAddMultiWatchAction != NULL))
    {
        // Check if add watch actions should be enabled:
        bool isEnabled = gaIsDebuggedProcessSuspended() && (gaIsInKernelDebugging() || gaIsInHSAKernelBreakpoint() || gaCanGetHostVariables());

        // Set the actions enable state:
        m_pAddWatchAction->setEnabled(isEnabled);
        m_pAddMultiWatchAction->setEnabled(isEnabled);
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
