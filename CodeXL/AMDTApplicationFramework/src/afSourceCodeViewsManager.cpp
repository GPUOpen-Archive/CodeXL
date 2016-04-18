//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afSourceCodeViewsManager.cpp
///
//==================================================================================

// Qt:
#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtASCIIString.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>

#include <AMDTApplicationFramework/Include/views/afSourceCodeView.h>

// Local:
#include <AMDTApplicationFramework/Include/afDocUpdateManager.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afQMdiSubWindow.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afSourceCodeViewsManager.h>


// Static member initialization:
afSourceCodeViewsManager* afSourceCodeViewsManager::_pMySingleInstance = nullptr;

// ---------------------------------------------------------------------------
// Name:        afSourceCodeViewsManager::afSourceCodeViewsManager
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        18/8/2011
// ---------------------------------------------------------------------------
afSourceCodeViewsManager::afSourceCodeViewsManager() : _showLineNumbers(true)
{
    // Register as an events observer:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeViewsManager::~afSourceCodeViewsManager
// Description: Destructor
// Author:      Uri Shomroni
// Date:        16/9/2010
// ---------------------------------------------------------------------------
afSourceCodeViewsManager::~afSourceCodeViewsManager()
{
    // Register as an events observer:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeViewsManager::instance
// Description: My creation function
// Return Val:  afSourceCodeViewsManager&
// Author:      Sigal Algranaty
// Date:        18/8/2011
// ---------------------------------------------------------------------------
afSourceCodeViewsManager& afSourceCodeViewsManager::instance()
{
    // If this class single instance was not already created:
    if (_pMySingleInstance == nullptr)
    {
        // Create it:
        _pMySingleInstance = new afSourceCodeViewsManager;
        GT_ASSERT(_pMySingleInstance);
    }

    return *_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeViewsManager::onEvent
// Description: Handle events
// Arguments:   const apEvent& eve
//              bool& vetoEvent
// Author:      Sigal Algranaty
// Date:        17/8/2011
// ---------------------------------------------------------------------------
void afSourceCodeViewsManager::onEvent(const apEvent& eve, bool& vetoEvent)
{
    GT_UNREFERENCED_PARAMETER(vetoEvent);

    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    switch (eventType)
    {
        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        case apEvent::AP_INFRASTRUCTURE_STARTS_BEING_BUSY_EVENT:
        {
            clearProgramCounters();
            bindExistingBreakpoints();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED:
        {
            bindExistingBreakpoints();
        }
        break;

        case apEvent::AP_OPENCL_PROGRAM_CREATED_EVENT:
        {
            const apOpenCLProgramCreatedEvent creationEve = (const apOpenCLProgramCreatedEvent&)eve;
            onProgramStatusChanged(creationEve.contextID(), creationEve.programIndex(), false);
        }
        break;

        case apEvent::AP_KERNEL_SOURCE_BREAKPOINTS_UPDATED_EVENT:
        {
            onKernelSourceCodeUpdate((const apKernelSourceBreakpointsUpdatedEvent&)eve);
        }
        break;

        case apEvent::AP_BREAKPOINTS_UPDATED_EVENT:
        {
            bindExistingBreakpoints();
        }
        break;

        case apEvent::AP_OPENCL_PROGRAM_DELETED_EVENT:
        {
            const apOpenCLProgramCreatedEvent& creationEve = (const apOpenCLProgramCreatedEvent&)eve;
            onProgramStatusChanged(creationEve.contextID(), creationEve.programIndex(), true);
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        {
            // Unding all kernel source code functions:
            onProgramStatusChanged(-1, -1, true);

            onProcessTerminate();
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
// Name:        afSourceCodeViewsManager::getSourceCodeWindow
// Description: Get / Create a source code view to display the requested file path
// Arguments:   const osFilePath& sourceCodeFilePath
//              int lineNumber
//              programCounterIndex - Contain the index of the displayed source code in the call stack frame:
//                        (0 for top, 1 for others -1 for code not in debug mode):
// Return Val:  afSourceCodeView*
// Author:      Sigal Algranaty
// Date:        18/8/2011
// ---------------------------------------------------------------------------
afSourceCodeView* afSourceCodeViewsManager::getSourceCodeWindow(const osFilePath& sourceCodeFilePath, int lineNumber, int programCounterIndex, QWidget* pParent)
{
    afSourceCodeView* pRetVal = nullptr;

    // Search for an existing source code view:
    int viewIndex = -1;
    pRetVal = getExistingView(sourceCodeFilePath, viewIndex);

    if (pRetVal == nullptr)
    {
        // Create a source code view:
        unsigned int contextMenuMask = acSourceCodeView::AC_ContextMenu_Default;

        if (afExecutionModeManager::instance().isActiveMode(L"Debug Mode"))
        {
            contextMenuMask = acSourceCodeView::AC_ContextMenu_Debug;
        }

        pRetVal = new afSourceCodeView(pParent, _showLineNumbers, contextMenuMask);
        pRetVal->SetMDIFilePath(sourceCodeFilePath);

        // Display the source code item:
        pRetVal->displayFile(sourceCodeFilePath, lineNumber, programCounterIndex);

        // Add the source code view to the vector:
        _displayedSourceCodeViewsVector.push_back(pRetVal);

        // register the document to the update mechanism:
        afDocUpdateManager::instance().RegisterDocument(pRetVal, sourceCodeFilePath, pRetVal, true);
    }

    if (programCounterIndex >= 0)
    {
        // Add the mapping for the counter index and line number:
        _sourceCodeFilesPCLineNumbers[sourceCodeFilePath.asString()] = pair<int, int> (lineNumber, programCounterIndex);
    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        afSourceCodeViewsManager::getExistingView
// Description: Return the object and index for the source code view with the
//              requested file path
// Arguments:   const osFilePath& filePath
//              int& viewIndex - output - the view index
// Return Val:  afSourceCodeView*
// Author:      Sigal Algranaty
// Date:        18/8/2011
// ---------------------------------------------------------------------------
afSourceCodeView* afSourceCodeViewsManager::getExistingView(const osFilePath& filePath, int& viewIndex)
{
    afSourceCodeView* pRetVal = nullptr;

    // Search within the existing windows if the item data is already displayed:
    for (int i = 0; i < (int)_displayedSourceCodeViewsVector.size(); i++)
    {
        // Get the current view:
        afSourceCodeView* pSourceCodeView = _displayedSourceCodeViewsVector[i];

        if (pSourceCodeView != nullptr)
        {
            // If this view display the requested path, return it:
            if (pSourceCodeView->filePath() == filePath)
            {
                pRetVal = pSourceCodeView;
                viewIndex = i;
            }
        }
    }

    if (pRetVal != nullptr)
    {
        // Down cast the view parent to an MDI sub window:
        afQMdiSubWindow* pMDISubWindow = qobject_cast<afQMdiSubWindow*>(pRetVal->parent());

        // Get the main window:
        afMainAppWindow* pAppMainWindow = afMainAppWindow::instance();
        GT_IF_WITH_ASSERT(pAppMainWindow != nullptr)
        {
            // Activate this window:
            pAppMainWindow->activateSubWindow(pMDISubWindow);
        }

    }

    return pRetVal;
}


// ---------------------------------------------------------------------------
// Name:        afSourceCodeViewsManager::onSubWindowClose
// Description: Remove the closed sub window
// Arguments:   afQMdiSubWindow* pSubWindowAboutToBeClosed
// Author:      Sigal Algranaty
// Date:        18/8/2011
// ---------------------------------------------------------------------------
void afSourceCodeViewsManager::onSubWindowClose(afQMdiSubWindow* pSubWindowAboutToBeClosed)
{
    // Check if the sub window contain source code view:
    gtASCIIString subWindowWidgetClass;

    if (pSubWindowAboutToBeClosed->widget() != nullptr)
    {
        gtASCIIString className(pSubWindowAboutToBeClosed->widget()->metaObject()->className());

        if (className == "afSourceCodeView")
        {
            // Down cast the widget to a source code view:
            afSourceCodeView* pSourceCodeView = qobject_cast<afSourceCodeView*>(pSubWindowAboutToBeClosed->widget());
            GT_IF_WITH_ASSERT(pSourceCodeView != nullptr)
            {
                // Look for the source code view location in the vector:
                int viewIndex = -1;

                for (int i = 0; i < (int)_displayedSourceCodeViewsVector.size(); i++)
                {
                    if (pSourceCodeView == _displayedSourceCodeViewsVector[i])
                    {
                        viewIndex = i;
                    }
                }

                // Remove the view from my vector:
                GT_IF_WITH_ASSERT(viewIndex >= 0)
                {
                    _displayedSourceCodeViewsVector.removeItem(viewIndex);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeViewsManager::clearProgramCounters
// Description: Handles process run resumed event
// Author:      Sigal Algranaty
// Date:        18/8/2011
// ---------------------------------------------------------------------------
void afSourceCodeViewsManager::clearProgramCounters()
{
    // Go through the views, and clear the program counter:
    for (int i = 0; i < (int)_displayedSourceCodeViewsVector.size(); i++)
    {
        // Get the current view:
        afSourceCodeView* pSourceCodeView = _displayedSourceCodeViewsVector[i];

        if (pSourceCodeView != nullptr)
        {
            pSourceCodeView->setProgramCounter(-1, -1);
        }
    }

    // Clear the program counters mappings:
    _sourceCodeFilesPCLineNumbers.clear();
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeViewsManager::onProgramStatusChanged
// Description: Handle program deletion / creation - bind / unbind all the
//              breakpoint to this program
// Arguments:   int contextId
//              int programId
//              bool programDeleted
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        19/7/2012
// ---------------------------------------------------------------------------
void afSourceCodeViewsManager::onProgramStatusChanged(int contextId, int programId, bool programDeleted)
{
    // Get amount of breakpoints from API:
    afPluginConnectionManager& thePluginConnectionManager = afPluginConnectionManager::instance();
    thePluginConnectionManager.bindProgramToBreakpoints(contextId, programId, programDeleted);

    // Go through the views, and clear the program counter:
    for (int i = 0; i < (int)_displayedSourceCodeViewsVector.size(); i++)
    {
        // Get the current view:
        afSourceCodeView* pSourceCodeView = _displayedSourceCodeViewsVector[i];

        if (pSourceCodeView != nullptr)
        {
            // Update the source code breakpoints
            pSourceCodeView->updateBreakpointsFromTheAPI();
        }
    }
}



// ---------------------------------------------------------------------------
// Name:        afSourceCodeViewsManager::bindExistingBreakpoints
// Description: Handle process suspension
// Author:      Sigal Algranaty
// Date:        21/8/2011
// ---------------------------------------------------------------------------
void afSourceCodeViewsManager::bindExistingBreakpoints()
{
    // Go through the views, and clear the program counter:
    for (int i = 0; i < (int)_displayedSourceCodeViewsVector.size(); i++)
    {
        // Get the current view:
        afSourceCodeView* pSourceCodeView = _displayedSourceCodeViewsVector[i];

        if (pSourceCodeView != nullptr)
        {
            // Add the API breakpoints to the editors:
            pSourceCodeView->updateBreakpointsFromTheAPI();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeViewsManager::onKernelSourceCodeUpdate
// Description: Handle kernel source code update
// Arguments:   const apKernelSourceBreakpointsUpdatedEvent& eve
// Author:      Sigal Algranaty
// Date:        18/8/2011
// ---------------------------------------------------------------------------
void afSourceCodeViewsManager::onKernelSourceCodeUpdate(const apKernelSourceBreakpointsUpdatedEvent& eve)
{
    afPluginConnectionManager& thePluginConnectionManager = afPluginConnectionManager::instance();
    bool rc = thePluginConnectionManager.onKernelSourceCodeUpdate(eve);
    GT_ASSERT(rc);

    // Go through the views, and clear the program counter:
    for (int i = 0; i < (int)_displayedSourceCodeViewsVector.size(); i++)
    {
        // Get the current view:
        afSourceCodeView* pSourceCodeView = _displayedSourceCodeViewsVector[i];

        if (pSourceCodeView != nullptr)
        {
            // Try to bind the source code to a program handle:
            pSourceCodeView->updateBreakpointsFromTheAPI();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeViewsManager::onProcessTerminate
// Description: Handle process termination event
// Author:      Sigal Algranaty
// Date:        18/8/2011
// ---------------------------------------------------------------------------
void afSourceCodeViewsManager::onProcessTerminate()
{
    // Go through the views, and clear the program counter:
    for (int i = 0; i < (int)_displayedSourceCodeViewsVector.size(); i++)
    {
        // Get the current view:
        afSourceCodeView* pSourceCodeView = _displayedSourceCodeViewsVector[i];

        if (pSourceCodeView != nullptr)
        {
            // Clear the program counter:
            pSourceCodeView->setProgramCounter(-1, -1);
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        afSourceCodeViewsManager::removeSourceCodeView
// Description: Remove the requested source code view from my list
// Arguments:   afSourceCodeView* pSourceCodeView
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        23/8/2011
// ---------------------------------------------------------------------------
void afSourceCodeViewsManager::removeSourceCodeView(afSourceCodeView* pSourceCodeView)
{
    // Look for the item index:
    int index = -1;

    for (int i = 0; i < (int)_displayedSourceCodeViewsVector.size(); i++)
    {
        if (_displayedSourceCodeViewsVector[i] == pSourceCodeView)
        {
            index = i;
            break;
        }
    }

    GT_IF_WITH_ASSERT(index >= 0)
    {
        _displayedSourceCodeViewsVector.removeItem(index);

        delete pSourceCodeView;
    }
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeViewsManager::setViewLineNumbers
// Description: Set the white spaces visibility for all source code views
// Arguments:   bool showWhiteSpaces
// Author:      Sigal Algranaty
// Date:        18/9/2011
// ---------------------------------------------------------------------------
void afSourceCodeViewsManager::setViewLineNumbers(bool showLineNumbers)
{
    if (_showLineNumbers != showLineNumbers)
    {
        for (int i = 0; i < (int)_displayedSourceCodeViewsVector.size(); i++)
        {
            if (_displayedSourceCodeViewsVector[i] != nullptr)
            {
                _displayedSourceCodeViewsVector[i]->showLineNumbers(showLineNumbers);
            }
        }

        // Set the new show white spaces flag:
        _showLineNumbers = showLineNumbers;
    }
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeViewsManager::getLineNumberAndProgramCounter
// Description: Get the current displayed counter index and line number for the
//              requested file path
// Arguments:   const osFilePath& filePath - the file path
//              int& lineNumber - the displayed line number
//              int& programCounterIndex - the program counter index
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        2/10/2011
// ---------------------------------------------------------------------------
bool afSourceCodeViewsManager::getLineNumberAndProgramCounter(const osFilePath& filePath, int& lineNumber, int& programCounterIndex)
{
    bool retVal = false;
    lineNumber = -1;
    programCounterIndex = -1;

    // Get the displayed file path details:
    gtMap<gtString, pair<int, int> >::const_iterator iter = _sourceCodeFilesPCLineNumbers.find(filePath.asString());

    if (iter != _sourceCodeFilesPCLineNumbers.end())
    {
        lineNumber = (*iter).second.first;
        programCounterIndex = (*iter).second.second;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afSourceCodeViewsManager::setLineNumberAndProgramCounter
// Description: Set a file path program counter and line number
// Arguments:   const osFilePath& filePath
//              int lineNumber
//              int programCounterIndex
// Author:      Sigal Algranaty
// Date:        2/10/2011
// ---------------------------------------------------------------------------
void afSourceCodeViewsManager::setLineNumberAndProgramCounter(const osFilePath& filePath, int lineNumber, int programCounterIndex)
{
    _sourceCodeFilesPCLineNumbers[filePath.asString()] = pair<int, int> (lineNumber, programCounterIndex);
}


// ---------------------------------------------------------------------------
// Name:        afSourceCodeViewsManager::isFileOpen
// Description: Checks if a file is displayed
// Arguments:   const osFilePath& filePath
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        27/3/2012
// ---------------------------------------------------------------------------
bool afSourceCodeViewsManager::isFileOpen(const osFilePath& filePath)
{
    bool retVal = false;

    for (int i = 0 ; i < (int)_displayedSourceCodeViewsVector.size(); i++)
    {
        // Get the current view:
        afSourceCodeView* pSourceCodeView = _displayedSourceCodeViewsVector[i];

        if (pSourceCodeView != nullptr)
        {
            // If this view display the requested path, return it:
            if (pSourceCodeView->filePath() == filePath)
            {
                retVal = true;
            }
        }
    }

    return retVal;
}

