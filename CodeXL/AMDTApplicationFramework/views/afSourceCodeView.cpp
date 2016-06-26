//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file afSourceCodeView.cpp
///
//==================================================================================

// Qt
#include <qtIgnoreCompilerWarnings.h>
#include <QtWidgets>

// QScintilla:
#include <Qsci/qscilexercpp.h>
#include <Qsci/qsciabstractapis.h>

// Infra:
#include <AMDTBaseTools/Include/gtAlgorithms.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osFile.h>
#include <AMDTAPIClasses/Include/apCLKernel.h>
#include <AMDTAPIClasses/Include/apCLProgram.h>
#include <AMDTAPIClasses/Include/apKernelSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/apHostSourceBreakPoint.h>
#include <AMDTAPIClasses/Include/apGLShaderObject.h>
#include <AMDTAPIClasses/Include/apSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/Events/apAddWatchEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointsUpdatedEvent.h>
#include <AMDTApplicationComponents/Include/acFindWidget.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>
#include <AMDTApplicationComponents/Include/acGoToLineDialog.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afApplicationCommands.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afProgressBarWrapper.h>
#include <AMDTApplicationFramework/Include/afSourceCodeViewsManager.h>
#include <AMDTApplicationFramework/Include/views/afSourceCodeView.h>


// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::afSourceCodeView
// Description: Constructor.
// Arguments:   parent - My parent Qt widget
// Author:      Sigal Algranaty
// Date:        2/8/2011
// ---------------------------------------------------------------------------
afSourceCodeView::afSourceCodeView(QWidget* pParent, bool shouldShowLineNumbers, unsigned int contextMenuMask):
    acSourceCodeView(pParent, shouldShowLineNumbers, contextMenuMask), afBaseView(&afProgressBarWrapper::instance()),
    m_pApplicationCommands(nullptr), _clickedBreakpointLine(-1),
    _pAddWatchAction(nullptr), _pAddMultiWatchAction(nullptr), _pShowLineNumbersAction(nullptr),
    m_enableBreakpoints(true), m_pMatchingTreeItemData(nullptr)
{
    // Get the application commands instance:
    m_pApplicationCommands = afApplicationCommands::instance();
    GT_ASSERT(m_pApplicationCommands != nullptr);

    // Connect the margin clicked signal:
    bool rcConnect = connect(this, SIGNAL(marginClicked(int, int, Qt::KeyboardModifiers)), this, SLOT(marginClicked(int, int, Qt::KeyboardModifiers)));
    GT_ASSERT(rcConnect);

    // Connect the text changed signal:
    rcConnect = connect(this, SIGNAL(textChanged()), this, SLOT(onTextChanged()));
    GT_ASSERT(rcConnect);

    // Connect the menu about to show action signal:
    rcConnect = connect(_pTextContextMenu, SIGNAL(aboutToShow()), this, SLOT(onMenuAboutToShow()));
    GT_ASSERT(rcConnect);

    // Add commands to the context menu:
    addCommandsToContextMenu(contextMenuMask);
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::~afSourceCodeView
// Description: Destructor
// Author:      Sigal Algranaty
// Date:        2/8/2011
// ---------------------------------------------------------------------------
afSourceCodeView::~afSourceCodeView()
{
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::marginClicked
// Description: Handle the margin click function
// Arguments:   int margin - the margin id
//              int line - the line clicked
//              Qt::KeyboardModifiers state
// Author:      Sigal Algranaty
// Date:        9/8/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::marginClicked(int margin, int line, Qt::KeyboardModifiers state)
{
    GT_UNREFERENCED_PARAMETER(margin);
    GT_UNREFERENCED_PARAMETER(state);

    if (m_enableBreakpoints)
    {
        // Check if there is a code in this line:
        QString textInLine = text(line);

        gtString fileExtName;
        filePath().getFileExtension(fileExtName);
        //bool isClFile = (AF_STR_clSourceFileExtension == fileExtName);
        //bool isHSAILfile = (AF_STR_hsailSourceFileExtension == fileExtName);

        bool isLineEmpty = textInLine.isEmpty() || textInLine.isNull() || (textInLine == "\n") || (textInLine == "\r\n");

        if (!isLineEmpty /* && (isClFile || isHSAILfile)*/)
        {
            afPluginConnectionManager& thePluginConnectionManager = afPluginConnectionManager::instance();

            // Get the line 1 based:
            int sourceCodeLine = line + 1;

            // Check if a breakpoint exists in this line:
            apBreakPoint* pBreakpoint = findBreakpointInLine(sourceCodeLine);

            if (pBreakpoint != nullptr)
            {
                // If the breakpoint is disabled, enable it:
                if (!pBreakpoint->isEnabled())
                {
                    pBreakpoint->setEnableStatus(true);
                    bool rc = thePluginConnectionManager.setBreakpoint(*pBreakpoint);
                    GT_ASSERT(rc);
                }
                else
                {
                    // Remove the breakpoint:
                    bool rc = thePluginConnectionManager.removeBreakpoint(*pBreakpoint);
                    GT_ASSERT(rc)
                }
            }
            else
            {
                // Get the type of breakpoint we need to add:
                osTransferableObjectType fileBreakpointsType = thePluginConnectionManager.breakpointTypeFromSourcePath(_filePath);

                if (OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT == fileBreakpointsType)
                {
                    // Allocate a new kernel source code breakpoint:
                    apKernelSourceCodeBreakpoint* pKernelSourceCodeBreakpoint = new apKernelSourceCodeBreakpoint(_filePath, sourceCodeLine);
                    pKernelSourceCodeBreakpoint->setEnableStatus(true);

                    // Set the breakpoint:
                    bool rc = thePluginConnectionManager.setBreakpoint(*pKernelSourceCodeBreakpoint);
                    GT_ASSERT(rc);
                }
                else if (OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT == fileBreakpointsType)
                {
                    apHostSourceCodeBreakpoint* pHostSourceCodeBreakpoint = new apHostSourceCodeBreakpoint(_filePath, sourceCodeLine);
                    pHostSourceCodeBreakpoint->setEnableStatus(true);

                    bool rc = thePluginConnectionManager.setBreakpoint(*pHostSourceCodeBreakpoint);
                    GT_ASSERT(rc);
                }
                else
                {
                    GT_ASSERT(OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT == fileBreakpointsType);

                    // Allocate a new source code breakpoint:
                    apSourceCodeBreakpoint* pSourceCodeBreakpoint = new apSourceCodeBreakpoint(_filePath, sourceCodeLine);
                    pSourceCodeBreakpoint->setEnableStatus(true);

                    // Set the breakpoint:
                    bool rc = thePluginConnectionManager.setBreakpoint(*pSourceCodeBreakpoint);
                    GT_ASSERT(rc);
                }
            }

            // Trigger breakpoints update event:
            apBreakpointsUpdatedEvent eve(-1);
            apEventsHandler::instance().registerPendingDebugEvent(eve);

            // Add the markers for the breakpoints in scintilla editor:
            updateBreakpointsFromTheAPI();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::updateBreakpointsFromTheAPI
// Description: Add the breakpoints to scintilla editor
// Author:      Sigal Algranaty
// Date:        17/8/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::updateBreakpointsFromTheAPI()
{
    // Delete all breakpoints markers:
    markerDeleteAll(_enabledBreakpointMarkerIndex);
    markerDeleteAll(_disabledBreakpointMarkerIndex);

    // Get amount of breakpoints from API:
    gtVector<apBreakPoint*> currentSetBreakpoints;
    afPluginConnectionManager& thePluginConnectionManager = afPluginConnectionManager::instance();
    thePluginConnectionManager.getSetBreakpoints(currentSetBreakpoints);
    int amountOfBreakpoints = (int)currentSetBreakpoints.size();

    for (int i = 0 ; i < amountOfBreakpoints ; i++)
    {
        // Get the breakpoint in the current index:
        apBreakPoint* pCurrentBreakpoint = currentSetBreakpoints[i];
        GT_IF_WITH_ASSERT(nullptr != pCurrentBreakpoint)
        {
            osTransferableObjectType currentBPType = pCurrentBreakpoint->type();

            if (currentBPType)
            {
                // Check if the breakpoint is a kernel source code breakpoint:
                if (OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT == currentBPType)
                {
                    // Down cast it to apKernelSourceCodeBreakpoint:
                    apKernelSourceCodeBreakpoint* pKernelBreakpoint = (apKernelSourceCodeBreakpoint*)pCurrentBreakpoint;
                    GT_IF_WITH_ASSERT(pKernelBreakpoint != nullptr)
                    {
                        // Get the line number (zero based):
                        int lineNumber = pKernelBreakpoint->lineNumber() - 1;

                        if (thePluginConnectionManager.doesBreakpointMatchFile(*pKernelBreakpoint, _filePath) && (lineNumber >= 0))
                        {
                            if (pKernelBreakpoint->isEnabled())
                            {
                                markerAdd(lineNumber, _enabledBreakpointMarkerIndex);
                            }
                            else
                            {
                                markerAdd(lineNumber, _disabledBreakpointMarkerIndex);
                            }
                        }
                    }
                }
                else if (OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT == currentBPType)
                {
                    // There is host source program bound, compare to a source code breakpoints:
                    // Down cast it to apHostSourceCodeBreakpoint:
                    apHostSourceCodeBreakpoint* pSourceCodeBreakpoint = (apHostSourceCodeBreakpoint*)pCurrentBreakpoint;
                    GT_IF_WITH_ASSERT(pSourceCodeBreakpoint != nullptr)
                    {
                        // Get the line number (zero based):
                        int lineNumber = pSourceCodeBreakpoint->lineNumber() - 1;

                        // Compare the file path and line number:
                        if ((pSourceCodeBreakpoint->filePath() == _filePath) && (lineNumber >= 0))
                        {
                            if (pSourceCodeBreakpoint->isEnabled())
                            {
                                markerAdd(lineNumber, _enabledBreakpointMarkerIndex);
                            }
                            else
                            {
                                markerAdd(lineNumber, _disabledBreakpointMarkerIndex);
                            }
                        }

                        setCursorPosition(lineNumber, 0);
                    }
                }
                else if (OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT == currentBPType)
                {
                    // There is no OpenCL program bound, compare to a source code breakpoints:
                    // Down cast it to apSourceCodeBreakpoint:
                    apSourceCodeBreakpoint* pSourceCodeBreakpoint = (apSourceCodeBreakpoint*)pCurrentBreakpoint;
                    GT_IF_WITH_ASSERT(pSourceCodeBreakpoint != nullptr)
                    {
                        // Get the line number (zero based):
                        int lineNumber = pSourceCodeBreakpoint->lineNumber() - 1;

                        // Compare the file path and line number:
                        if ((pSourceCodeBreakpoint->filePath() == _filePath) && (lineNumber >= 0))
                        {
                            if (pSourceCodeBreakpoint->isEnabled())
                            {
                                markerAdd(lineNumber, _enabledBreakpointMarkerIndex);
                            }
                            else
                            {
                                markerAdd(lineNumber, _disabledBreakpointMarkerIndex);
                            }
                        }
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::initializeMarginContextMenu
// Description: Initializes the menu for margin right click
// Author:      Sigal Algranaty
// Date:        15/8/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::initializeMarginContextMenu(int clickedLine, int lineMarkers)
{
    GT_UNREFERENCED_PARAMETER(lineMarkers);

    // Save the clicked line:
    _clickedBreakpointLine = clickedLine;

    // Initialize the menu if it's not initialized yet:
    if (_pMarginContextMenu == nullptr)
    {
        // Create the margin context menu:
        _pMarginContextMenu = new QMenu(this);

        QKeySequence shortcutCut1;
        _pMarginContextMenu->addAction(AF_STR_SourceCodeDeleteBreakpoint, this, SLOT(onDeleteBreakpoint()), shortcutCut1);

        QKeySequence shortcutCut("Ctrl+F9");
        _pMarginContextMenu->addAction(AF_STR_SourceCodeDisableBreakpoint, this, SLOT(onDisableBreakpoint()), shortcutCut);
        _pMarginContextMenu->addAction(AF_STR_SourceCodeEnableBreakpoint, this, SLOT(onEnableBreakpoint()), shortcutCut);

        bool rcConnect = connect(_pMarginContextMenu, SIGNAL(aboutToShow()), this, SLOT(onAboutToBreakpointsContextMenu()));
        GT_ASSERT(rcConnect);
    }

}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::displayMarginContextMenu
// Description: Display the context menu on margin right click
// Arguments:   QPoint& position - the right click position
// Author:      Sigal Algranaty
// Date:        15/8/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::displayMarginContextMenu(QPoint position)
{
    // Get the margin width:
    int marginW = marginWidth(0) + marginWidth(1) + marginWidth(2) + 1;

    // Get the line number for this position:
    QPoint positionStartLine(marginW, position.y());

    // Get the clicked line:
    int clickedLine = lineAt(positionStartLine);

    if (clickedLine >= 0)
    {
        // Get the markers for the clicked line:
        unsigned int lineMarkers = markersAtLine(clickedLine);

        // Check if there is a breakpoint in this line:
        bool hasBreakpoint = ((lineMarkers & (1 << _enabledBreakpointMarkerIndex)) || (lineMarkers & (1 << _disabledBreakpointMarkerIndex)));

        if (hasBreakpoint)
        {
            // Initialize the menu:
            initializeMarginContextMenu(clickedLine, lineMarkers);

            // Show the menu:
            _pMarginContextMenu->exec(acMapToGlobal(this, position));

            // Clear the clicked line:
            _clickedBreakpointLine = -1;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::onTextChanged
// Description: Is handling the text change signal
// Author:      Sigal Algranaty
// Date:        16/8/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::onTextChanged()
{
    if (!_ignoreTextChanged && !m_mdiFilePath.isEmpty())
    {
        // Mark the file as modified:
        m_isModified = QsciScintilla::isUndoAvailable();
        // Get the application commands instance:
        GT_IF_WITH_ASSERT(m_pApplicationCommands != nullptr )
        {
            if (m_isModified)
            {
                m_pApplicationCommands->MarkMDIWindowAsChanged(m_mdiFilePath, true);

                // If tree selection is on:
                if ((m_pMatchingTreeItemData != nullptr) && (m_pMatchingTreeItemData->m_pTreeWidgetItem != nullptr))
                {
                    // Get the application tree:
                    afApplicationTree* pTree = m_pApplicationCommands->applicationTree();
                    GT_IF_WITH_ASSERT((pTree != nullptr) && (pTree->treeControl() != nullptr))
                    {
                        // Select the item in tree:
                        m_pMatchingTreeItemData->m_pTreeWidgetItem->setSelected(true);
                        pTree->treeControl()->setCurrentItem(m_pMatchingTreeItemData->m_pTreeWidgetItem);
                    }
                }
            }
            else
            {
                m_pApplicationCommands->MarkMDIWindowAsChanged(m_mdiFilePath, false);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::undo
// Description: Override undo
// Author:      Sigal Algranaty
// Date:        16/8/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::undo()
{
    // Call the base class implementation:
    acSourceCodeView::undo();

    // Perform onTextChanged():
    onTextChanged();
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::enableExistingBreakpoint
// Description: Enable the breakpoint in the requested line
// Arguments:   int line
//              bool isEnabled
// Return Val:  int - the existing breakpoint index
// Author:      Sigal Algranaty
// Date:        17/8/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::enableExistingBreakpoint(int line, bool isEnabled)
{
    // Find the breakpoint object in the requested line:
    apBreakPoint* pBreakpoint = findBreakpointInLine(line + 1);
    GT_IF_WITH_ASSERT(pBreakpoint != nullptr)
    {
        pBreakpoint->setEnableStatus(isEnabled);

        // Set the breakpoint:
        afPluginConnectionManager::instance().setBreakpoint(*pBreakpoint);
    }
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::onDisableBreakpoint
// Description: Disable the requested breakpoint
// Author:      Sigal Algranaty
// Date:        17/8/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::onDisableBreakpoint()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((_clickedBreakpointLine >= 0) && (_clickedBreakpointLine < lines()))
    {
        // Search for a breakpoint in the clicked line:
        enableExistingBreakpoint(_clickedBreakpointLine, false);

        // Add the breakpoints to the editor:
        updateBreakpointsFromTheAPI();
    }

}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::onEnableBreakpoint
// Description: Enable the clicked breakpoint
// Author:      Sigal Algranaty
// Date:        17/8/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::onEnableBreakpoint()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((_clickedBreakpointLine >= 0) && (_clickedBreakpointLine < lines()))
    {
        // Search for a breakpoint in the clicked line:
        enableExistingBreakpoint(_clickedBreakpointLine, true);

        // Add the breakpoints to the editor:
        updateBreakpointsFromTheAPI();
    }
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::onDeleteBreakpoint
// Description: Delete the clicked breakpoint
// Author:      Sigal Algranaty
// Date:        17/8/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::onDeleteBreakpoint()
{
    // Sanity check:
    GT_IF_WITH_ASSERT((_clickedBreakpointLine >= 0) && (_clickedBreakpointLine < lines()))
    {
        // Find the breakpoint object in the requested line:
        apBreakPoint* pBreakpoint = findBreakpointInLine(_clickedBreakpointLine + 1);
        GT_IF_WITH_ASSERT(pBreakpoint != nullptr)
        {
            // Remove the breakpoint:
            afPluginConnectionManager::instance().removeBreakpoint(*pBreakpoint);

            // Add the breakpoints to the editor:
            updateBreakpointsFromTheAPI();

            // Trigger breakpoints update event:
            apBreakpointsUpdatedEvent eve(-1);
            apEventsHandler::instance().registerPendingDebugEvent(eve);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::displayFile
// Description: Overrides acSourceCodeView. Binding the program handle to the
//              displayed source code file
// Arguments:   fileName - the displayed file path
//              int lineNumber - line number
//              pcIndex - Contain the index of the displayed source code in the call stack frame:
//                        (0 for top, 1 for others -1 for code not in debug mode):
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        21/8/2011
// ---------------------------------------------------------------------------
bool afSourceCodeView::displayFile(const osFilePath& fileName, int lineNumber, int pcIndex)
{
    bool retVal = false;

    // Call the base class implementation:
    retVal = acSourceCodeView::displayFile(fileName, lineNumber, pcIndex);

    // Update the breakpoints from the API:
    updateBreakpointsFromTheAPI();

    onShowLineNumbers(_shouldShowLineNumbers);

    // Get the line 1 based:
    int sourceCodeLine = lineNumber - 1;

    // Set the cursor in the opened file:
    setCursorPosition(sourceCodeLine, 0);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::onAboutToBreakpointsContextMenu
// Description: Update context menu commands before it is shown
// Author:      Sigal Algranaty
// Date:        31/8/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::onAboutToBreakpointsContextMenu()
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pMarginContextMenu != nullptr)
    {
        // Get the markers for the clicked line:
        unsigned int lineMarkers = markersAtLine(_clickedBreakpointLine);

        // Check if the breakpoint in the clicked line is enabled / disabled:
        bool isEnabled = (lineMarkers & (1 << _enabledBreakpointMarkerIndex));

        // Get the context menu actions:
        QList<QAction*> actions = _pMarginContextMenu->actions();

        foreach (QAction* pCurrentAction, actions)
        {
            if (pCurrentAction != nullptr)
            {
                gtASCIIString actionText(pCurrentAction->text().toLatin1().data());

                if (actionText == AF_STR_SourceCodeDisableBreakpoint)
                {
                    pCurrentAction->setVisible(isEnabled);
                }

                if (actionText == AF_STR_SourceCodeEnableBreakpoint)
                {
                    pCurrentAction->setVisible(!isEnabled);
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::onFindClick
// Description: Handles the find click button
// Author:      Sigal Algranaty
// Date:        1/9/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::onFindClick()
{
    // Clear the status text:
    setProgressText(AF_STR_Ready);

    if (!acFindParameters::Instance().m_findExpr.isEmpty())
    {
        // Perform the find operation:
        int line = 0, index = 0;
        getCursorPosition(&line, &index);

        if (!selectedText().isEmpty())
        {
            if (acFindParameters::Instance().m_isSearchUp)
            {
                index--;
            }
        }

        acFindParameters::Instance().m_lastResult = findFirst(acFindParameters::Instance().m_findExpr, false, acFindParameters::Instance().m_isCaseSensitive, false, false, !acFindParameters::Instance().m_isSearchUp, line, index, true);

        if (!acFindParameters::Instance().m_lastResult)
        {
            // Set a "passed the end of the document status text":
            setProgressText(AF_STR_SourceCodePassedEndOfDocument);

            // If the text was not found, search from start:
            int lineNumber = acFindParameters::Instance().m_isSearchUp ? lines() : 0;

            // Perform the find operation
            acFindParameters::Instance().m_lastResult = findFirst(acFindParameters::Instance().m_findExpr, false, acFindParameters::Instance().m_isCaseSensitive, false, false, !acFindParameters::Instance().m_isSearchUp, lineNumber, 0, true);

            if (!acFindParameters::Instance().m_lastResult)
            {
                // Build the status text:
                gtString msg;
                msg.fromASCIIString(acFindParameters::Instance().m_findExpr.toLatin1().data());
                msg.prepend(AF_STR_SourceCodeTextWasNotFound);
                setProgressText(msg);
            }
        }

        // Update the find toolbar:
        acFindWidget::Instance().UpdateUI();
    }
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::onFindNext
// Description: Implements onFindNext action
// Author:      Sigal Algranaty
// Date:        4/9/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::onFindNext()
{
    // Clear the status text:
    setProgressText(AF_STR_Ready);

    // If the last find result was successful:
    if (acFindParameters::Instance().m_lastResult)
    {
        acFindParameters::Instance().m_lastResult = findNext();
    }
    else
    {
        // Restart find:
        acFindParameters::Instance().m_lastResult = findFirst(acFindParameters::Instance().m_findExpr, false, acFindParameters::Instance().m_isCaseSensitive, false, false, !acFindParameters::Instance().m_isSearchUp, -1, -1, true);

        if (!acFindParameters::Instance().m_lastResult)
        {
            // Set a "passed the end of the document status text":
            setProgressText(AF_STR_SourceCodePassedEndOfDocument);

            // If the text was not found, search from start:
            int lineNumber = acFindParameters::Instance().m_isSearchUp ? lines() : 0;

            // Perform the find operation
            acFindParameters::Instance().m_lastResult = findFirst(acFindParameters::Instance().m_findExpr, false, acFindParameters::Instance().m_isCaseSensitive, false, false, !acFindParameters::Instance().m_isSearchUp, lineNumber, 0, true);

            if (!acFindParameters::Instance().m_lastResult)
            {
                // Build the status text:
                gtString msg;
                msg.fromASCIIString(acFindParameters::Instance().m_findExpr.toLatin1().data());
                msg.prepend(AF_STR_SourceCodeTextWasNotFound);
                setProgressText(msg);
            }
        }
    }
}

// ---------------------------------------------------------------------------
void afSourceCodeView::onFindPrev()
{
    // m_isSearchUp flag is up only when find prev button is pressed
    acFindParameters::Instance().m_isSearchUp = true;
    onFindNext();
    acFindParameters::Instance().m_isSearchUp = false;
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::keyPressEvent
// Description: Is handling a key press event
// Arguments:   QKeyEvent *pKeyEvent
// Author:      Sigal Algranaty
// Date:        14/9/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::keyPressEvent(QKeyEvent* pKeyEvent)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pKeyEvent != nullptr)
    {
        if ((QKeySequence)pKeyEvent->key() == QKeySequence::fromString("F9"))
        {
            // Get the cursor position:
            int line = -1, index = -1;
            getCursorPosition(&line, &index);

            // Get the line 1 based:
            int sourceCodeLine = line + 1;

            // The user clicked CTRL-F9:
            if (pKeyEvent->modifiers() == Qt::ControlModifier)
            {
                // Check if there is a breakpoint set in this line:
                apBreakPoint* pBreakpoint = findBreakpointInLine(sourceCodeLine);

                if (pBreakpoint != nullptr)
                {
                    pBreakpoint->setEnableStatus(!pBreakpoint->isEnabled());

                    // Set the breakpoint:
                    bool rc = afPluginConnectionManager::instance().setBreakpoint(*pBreakpoint);
                    GT_ASSERT(rc);
                }
            }
            // No modifiers - the user clicked F9:
            else
            {
                // Same as clicking the margin for this line:
                marginClicked(0, line, Qt::NoModifier);
            }

            // Add the markers for the breakpoints in scintilla editor:
            updateBreakpointsFromTheAPI();

            // Trigger breakpoints update event:
            apBreakpointsUpdatedEvent eve(-1);
            apEventsHandler::instance().registerPendingDebugEvent(eve);
        }
    }

    // Call the base class implementation:
    acSourceCodeView::keyPressEvent(pKeyEvent);
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::findBreakpointInLine
// Description: Check if a breakpoint exist in the requested line
// Arguments:   int line
//              int& breakpointIndex
//              bool& isEnabled
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        14/9/2011
// ---------------------------------------------------------------------------
apBreakPoint* afSourceCodeView::findBreakpointInLine(int line)
{
    apBreakPoint* pRetVal = nullptr;

    // Get amount of breakpoints from API:
    gtVector<apBreakPoint*> currentSetBreakpoints;
    afPluginConnectionManager& thePluginConnectionManager = afPluginConnectionManager::instance();
    thePluginConnectionManager.getSetBreakpoints(currentSetBreakpoints);
    int amountOfBreakpoints = (int)currentSetBreakpoints.size();

    // Get this file's breakpoint type:
    osTransferableObjectType fileBreakpointsType = thePluginConnectionManager.breakpointTypeFromSourcePath(_filePath);

    for (int i = 0 ; i < amountOfBreakpoints ; i++)
    {
        // Get the breakpoint in the current index:
        apBreakPoint* pCurrentBreakpoint = currentSetBreakpoints[i];
        GT_IF_WITH_ASSERT(nullptr != pCurrentBreakpoint)
        {
            osTransferableObjectType currentBPType = pCurrentBreakpoint->type();

            if (fileBreakpointsType == currentBPType)
            {
                // Check if the breakpoint is a kernel source code breakpoint:
                if (OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT == currentBPType)
                {
                    // Down cast it to apKernelSourceCodeBreakpoint:
                    apKernelSourceCodeBreakpoint* pKernelBreakpoint = (apKernelSourceCodeBreakpoint*)pCurrentBreakpoint;
                    GT_IF_WITH_ASSERT(pKernelBreakpoint != nullptr)
                    {
                        if ((pKernelBreakpoint->lineNumber() == line) && thePluginConnectionManager.doesBreakpointMatchFile(*pKernelBreakpoint, _filePath))
                        {
                            pRetVal = pCurrentBreakpoint;
                            break;
                        }
                    }
                }
                else if (OS_TOBJ_ID_SOURCE_CODE_BREAKPOINT == currentBPType)
                {
                    // There is no OpenCL program bound, compare to a source code breakpoints:
                    // Down cast it to apSourceCodeBreakpoint:
                    apSourceCodeBreakpoint* pSourceCodeBreakpoint = (apSourceCodeBreakpoint*)pCurrentBreakpoint;
                    GT_IF_WITH_ASSERT(pSourceCodeBreakpoint != nullptr)
                    {
                        // Compare the file path and line number:
                        if ((pSourceCodeBreakpoint->lineNumber() == line) && (pSourceCodeBreakpoint->filePath() == _filePath))
                        {
                            pRetVal = pCurrentBreakpoint;
                            break;
                        }
                    }
                }
                else if (OS_TOBJ_ID_HOST_SOURCE_CODE_BREAKPOINT == currentBPType)
                {
                    apHostSourceCodeBreakpoint* pSourceCodeBreakpoint = (apHostSourceCodeBreakpoint*)pCurrentBreakpoint;
                    GT_IF_WITH_ASSERT(pSourceCodeBreakpoint != nullptr)
                    {
                        // Compare the file path and line number:
                        if ((pSourceCodeBreakpoint->lineNumber() == line) && (pSourceCodeBreakpoint->filePath() == _filePath))
                        {
                            pRetVal = pCurrentBreakpoint;
                            break;
                        }
                    }
                }
            }
        }
    }

    return pRetVal;
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::addCommandsToContextMenu
// Description: Add commands to context menu
// Author:      Sigal Algranaty
// Date:        25/9/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::addCommandsToContextMenu(unsigned int contextMenuMask)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(_pTextContextMenu != nullptr)
    {
        if ((contextMenuMask & AC_ContextMenu_LineNumbers) == AC_ContextMenu_LineNumbers)
        {
            // Create a show / hide white spaces action:
            _pShowLineNumbersAction = new QAction(AF_STR_sourceCodeShowLineNumbers, _pTextContextMenu);
            _pShowLineNumbersAction->setCheckable(true);

            // Connect the action to a callback:
            bool rc = connect(_pShowLineNumbersAction, SIGNAL(toggled(bool)), this, SLOT(onShowLineNumbers(bool)));
            GT_ASSERT(rc);

            // Add the action to the context menu:
            _pTextContextMenu->addAction(_pShowLineNumbersAction);

            _pTextContextMenu->addSeparator();
        }

        if ((contextMenuMask & AC_ContextMenu_Watch) == AC_ContextMenu_Watch)
        {
            // Create an action for the "Add Watch" menu item:
            _pAddWatchAction = new QAction(AF_STR_SourceCodeAddWatch, _pTextContextMenu);

            // Add the action to the context menu:
            _pTextContextMenu->addAction(_pAddWatchAction);

            // Connect the action to its handler:
            bool rcConnect = connect(_pAddWatchAction, SIGNAL(triggered()), this, SLOT(onAddWatch()));
            GT_ASSERT(rcConnect);

            // Create an action for the "Add Multi Watch" menu item:
            _pAddMultiWatchAction = new QAction(AF_STR_SourceCodeAddMultiWatch, _pTextContextMenu);

            // Add the action to the context menu:
            _pTextContextMenu->addAction(_pAddMultiWatchAction);

            // Connect the action to its handler:
            rcConnect = connect(_pAddMultiWatchAction, SIGNAL(triggered()), this, SLOT(onAddMultiWatch()));
            GT_ASSERT(rcConnect);
        }

        // Connect the menu to an about to show slot:
        bool rcConnect = connect(_pTextContextMenu, SIGNAL(aboutToShow()), this, SLOT(onAboutToShowTextContextMenu()));
        GT_ASSERT(rcConnect);
    }
}


// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::onAddWatch
// Description: Handling the "Add Watch" command
// Author:      Sigal Algranaty
// Date:        25/9/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::onAddWatch()
{
    // Get the currently selected text:
    QString textSelected = selectedText();

    if (textSelected.isEmpty())
    {
        // Get the word in the current position:
        textSelected = wordAtPoint(_currentMousePosition);
    }

    gtString textForWatch;
    textForWatch.fromASCIIString(textSelected.toLatin1());

    if (!textForWatch.isEmpty())
    {
        // Get the watch view:
        GT_IF_WITH_ASSERT(m_pApplicationCommands != nullptr)
        {
            apAddWatchEvent watchEvent(textForWatch, false);
            apEventsHandler::instance().registerPendingDebugEvent(watchEvent);
        }
    }

    // Else do nothing (no text for watch):
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::onAddMultiWatch
// Description: Handling the "Add Multi Watch" command
// Author:      Sigal Algranaty
// Date:        25/9/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::onAddMultiWatch()
{
    // Get the currently selected text:
    QString textSelected = selectedText();

    if (textSelected.isEmpty())
    {
        // Get the word in the current position:
        textSelected = wordAtPoint(_currentMousePosition);
    }

    gtString textForWatch;
    textForWatch.fromASCIIString(textSelected.toLatin1());

    if (!textForWatch.isEmpty())
    {
        // Get the watch view:
        GT_IF_WITH_ASSERT(m_pApplicationCommands != nullptr)
        {
            apAddWatchEvent watchEvent(textForWatch, true);
            apEventsHandler::instance().registerPendingDebugEvent(watchEvent);
        }
    }

    // Else do nothing (no text for watch):
}

// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::onAboutToShowTextContextMenu
// Description: Is called when the text context menu is shown - enable / disable
//              the context menu actions according to the current debug situation
// Author:      Sigal Algranaty
// Date:        25/9/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::onAboutToShowTextContextMenu()
{
    // Sanity check:
    if ((_pAddWatchAction != nullptr) && (_pAddMultiWatchAction != nullptr))
    {
        // Check if add watch actions should be enabled:
        afRunModes neededModes = AF_DEBUGGED_PROCESS_EXISTS | AF_DEBUGGED_PROCESS_SUSPENDED | AF_DEBUGGED_PROCESS_IN_KERNEL_DEBUGGING;
        bool isEnabled = (neededModes == ((afPluginConnectionManager::instance().getCurrentRunModeMask()) & neededModes));

        // Set the actions enable state:
        _pAddWatchAction->setEnabled(isEnabled);
        _pAddMultiWatchAction->setEnabled(isEnabled);
    }
}


// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::mouseMoveEvent
// Description: Override the mouse move event
// Arguments:   QMouseEvent *pEvent
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        26/9/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::mouseMoveEvent(QMouseEvent* pEvent)
{
    // Call the base class implementation:
    acSourceCodeView::mouseMoveEvent(pEvent);

    // Sanity check:
    GT_IF_WITH_ASSERT(pEvent != nullptr)
    {
        // Save the current mouse position:
        _currentMousePosition = pEvent->pos();
    }
}


// ---------------------------------------------------------------------------
// Name:        afSourceCodeView::onShowLineNumbers
// Description: Override acSourceCodeView implementation - sets the global value
//              to show / hide
// Arguments:   bool show
// Author:      Sigal Algranaty
// Date:        27/9/2011
// ---------------------------------------------------------------------------
void afSourceCodeView::onShowLineNumbers(bool show)
{
    if ((_pShowLineNumbersAction != nullptr) && (m_pApplicationCommands != nullptr))
    {
        showLineNumbers(show);
        _pShowLineNumbersAction->setChecked(show);

        // Set the global flag:
        m_pApplicationCommands->setViewLineNumbers(show);
    }
}

// ---------------------------------------------------------------------------
void afSourceCodeView::onMenuAboutToShow()
{
    if (_pShowLineNumbersAction != nullptr)
    {
        _pShowLineNumbersAction->setChecked(afSourceCodeViewsManager::instance().showLineNumbers());
    }
}

// ---------------------------------------------------------------------------
void afSourceCodeView::UpdateDocument(const osFilePath& docToUpdate)
{
    GT_UNREFERENCED_PARAMETER(docToUpdate);

    UpdateFile();
}

// ---------------------------------------------------------------------------
void afSourceCodeView::mouseDoubleClickEvent(QMouseEvent* pEvent)
{
    // Send this event to parent class to process.
    QsciScintilla::mouseDoubleClickEvent(pEvent);
    pEvent->ignore();

    // Get the line number that had the double click event
    QString textLine;
    QPoint cursorPos = this->cursor().pos();
    cursorPos = this->mapFromGlobal(cursorPos);
    int currentLine = this->lineAt(cursorPos);
    // May not be able to get the line

    const QStringList lines = text().split(QRegExp("[\r\n]"));
    emit textLineDoubleClicked(lines, currentLine);
}

// ---------------------------------------------------------------------------
void afSourceCodeView::GetSelectedText(gtString& selectedText)
{
    QString text = this->selectedText();
    selectedText = acQStringToGTString(text);
}

void afSourceCodeView::OnGoToLine()
{
    unsigned int lineCount = lines();
    int currentLine = 0;
    int currentLineIndex = 0;
    getCursorPosition(&currentLine, &currentLineIndex);

    acGoToLineDialog dlg(nullptr, lineCount);

    int rc = afApplicationCommands::instance()->showModal(&dlg);

    if (QDialog::Accepted == rc)
    {
        unsigned int selectedLine = dlg.GetLineNumber()-1;
        setCursorPosition(selectedLine, 0);

        int h = height();
        int th = textHeight(selectedLine);
        int visibleRowsCount = h / th;

        int ensureVisibleRow = selectedLine - visibleRowsCount / 2;
        if (ensureVisibleRow < 0)
        {
            ensureVisibleRow = 0;
        }
        setFirstVisibleLine(ensureVisibleRow);
        ensureLineVisible(selectedLine);
    }

}
// ---------------------------------------------------------------------------