//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gwEventObserver.cpp
///
//==================================================================================

//------------------------------ gwEventObserver.cpp ------------------------------

// Qt:
#include <QtWidgets>
#include <Qsci/qsciscintilla.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osCallStackFrame.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/Events/apAddWatchEvent.h>
#include <AMDTAPIClasses/Include/Events/apAfterKernelDebuggingEvent.h>
#include <AMDTAPIClasses/Include/Events/apBeforeKernelDebuggingEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessRunSuspendedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apFlushTextureImageEvent.h>
#include <AMDTAPIClasses/Include/Events/apHexChangedEvent.h>
#include <AMDTAPIClasses/Include/Events/apKernelDebuggingFailedEvent.h>
#include <AMDTAPIClasses/Include/Events/apKernelWorkItemChangedEvent.h>
#include <AMDTAPIClasses/Include/Events/apMDIViewActivatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDeferredCommandEvent.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTApplicationFramework/Include/afMainAppWindow.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdApplicationCommands.h>
#include <AMDTGpuDebuggingComponents/Include/gdGDebuggerGlobalVariablesManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdImagesAndBuffersManager.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdFlushTexturesImagesCommand.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdMultiWatchView.h>
#include <AMDTGpuDebuggingComponents/Include/views/gdWatchView.h>

// Local:
#include <src/gwEventObserver.h>
#include <AMDTGpuDebugging/Include/gwgDEBuggerAppWrapperDLLBuild.h>
#include <AMDTGpuDebugging/Include/gwgDEBuggerAppWrapper.h>
#include <AMDTGpuDebugging/Include/gwKernelWorkItemToolbar.h>
#include <AMDTGpuDebugging/Include/gwStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        gwEventObserver::gwEventObserver
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
gwEventObserver::gwEventObserver()
{
    // Register as an events observer:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);

    // Get the single instance of the application commands object:
    _pApplicationCommands = afApplicationCommands::instance();
    GT_ASSERT(NULL != _pApplicationCommands);
    _pGDApplicationCommands = gdApplicationCommands::gdInstance();
    GT_ASSERT(NULL != _pGDApplicationCommands);
}

// ---------------------------------------------------------------------------
// Name:        gwEventObserver::~gwEventObserver
// Description: Destructor
// Author:      Uri Shomroni
// Date:        16/9/2010
// ---------------------------------------------------------------------------
gwEventObserver::~gwEventObserver()
{
    // Register as an events observer:
    apEventsHandler::instance().unregisterEventsObserver(*this);
}

// ---------------------------------------------------------------------------
// Name:        gwEventObserver::onEvent
// Description: Is called when a debugged process event occurs.
// Arguments:   const apEvent& eve
//              bool& vetoEvent
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        21/7/2011
// ---------------------------------------------------------------------------
void gwEventObserver::onEvent(const apEvent& eve, bool& vetoEvent)
{
    (void)(vetoEvent); // unused
    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    // Handle the event according to its type:
    switch (eventType)
    {
        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        {
            // Get the application commands single instance:
            GT_IF_WITH_ASSERT(NULL != _pApplicationCommands)
            {
                _pApplicationCommands->updateToolbarCommands();

                // Update layout:
                updateLayout();

                // Update application title:
                updateApplicationTitle();

                updateWIToolbar(true, true);

            }
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        case apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE:
        {
            // Get the application commands single instance:
            GT_IF_WITH_ASSERT(_pApplicationCommands != NULL)
            {
                _pApplicationCommands->updateToolbarCommands();

                // Update layout:
                updateLayout();

                // Update application title:
                updateApplicationTitle();

                // This ensures that our Work Item combo boxes get disabled when we are done.
                gwKernelWorkItemToolbar* pKernelWorkItemsToolbar = gwgDEBuggerAppWrapper::kernelWorkItemToolbar();
                GT_IF_WITH_ASSERT(pKernelWorkItemsToolbar != NULL)
                {
                    pKernelWorkItemsToolbar->onAfterKernelDebuggingEvent((const apAfterKernelDebuggingEvent&)eve);
                }
                updateWIToolbar(true, true);

            }
        }
        break;


        case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
        {
            // Get the application commands single instance:
            GT_IF_WITH_ASSERT(NULL != _pApplicationCommands)
            {
                _pApplicationCommands->updateToolbarCommands();

                // Down cast the event to process run suspended event:
                apDebuggedProcessRunSuspendedEvent event = (apDebuggedProcessRunSuspendedEvent&)eve;

                // Open the breakpoint source code:
                bool rc = openBreakpointSourceCode(eve.triggeringThreadId());
                GT_ASSERT(rc);

                // Check if we need to flush all textures since openGL log was recorded
                bool wasRecorded = gaWasOpenGLDataRecordedInDebugSession();

                if (wasRecorded)
                {
                    // Trigger a flush textures event. This event will be handled after the AP_DEBUGGED_PROCESS_RUN_SUSPENDED was fully
                    // handled, letting other clients registered to AP_DEBUGGED_PROCESS_RUN_SUSPENDED finish their work.
                    apFlushTextureImageEvent flushTexturedEvent;

                    apEventsHandler::instance().registerPendingDebugEvent(flushTexturedEvent);
                }

                // Update layout:
                updateLayout();

                // Update application title:
                updateApplicationTitle();

                updateWIToolbar(true, false);
            }
        }
        break;

        case apEvent::AP_FLUSH_TEXTURE_IMAGES_EVENT:
        {
            if (!gaIsInKernelDebugging())
            {
                gdFlushTexturesImagesCommand flushTexturesCommand(NULL);
                (void) flushTexturesCommand.execute();
            }
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        {
            // Get the application commands single instance:
            // gdApplicationCommands* pApplicationCommands = gdApplicationCommands::gdInstance();
            (void) gdApplicationCommands::gdInstance();
            GT_IF_WITH_ASSERT(NULL != _pApplicationCommands)
            {
                _pApplicationCommands->updateToolbarCommands();

                // Update application title:
                updateApplicationTitle();

                updateWIToolbar(true, false);
            }
        }
        break;

        case apEvent::AP_BREAKPOINT_HIT:
        case apEvent::APP_GLOBAL_VARIABLE_CHANGED:
        {
            if (apEvent::AP_BREAKPOINT_HIT == eventType)
            {
                if (gaIsHostBreakPoint())
                {
                    gdGDebuggerGlobalVariablesManager& globalVars = gdGDebuggerGlobalVariablesManager::instance();
                    int threadIndex = 0;

                    GT_IF_WITH_ASSERT(gaGetBreakpointTriggeringThreadIndex(threadIndex))
                    {
                        globalVars.setChosenThread(threadIndex, false);
                    }
                }
            }

            updateWIToolbar(false, false);
        }
        break;

        case apEvent::AP_BEFORE_KERNEL_DEBUGGING_EVENT:
        {
            updateWIToolbar(false, true);
        }
        break;

        case apEvent::AP_KERNEL_CURRENT_WORK_ITEM_CHANGED_EVENT:
        {
            // We do not need to rebuild the WI toolbar when the work item changed.
        }
        break;

        case apEvent::AP_AFTER_KERNEL_DEBUGGING_EVENT:
        {
            // Get the kernel work items toolbar:
            gwKernelWorkItemToolbar* pKernelWorkItemsToolbar = gwgDEBuggerAppWrapper::kernelWorkItemToolbar();
            GT_IF_WITH_ASSERT(pKernelWorkItemsToolbar != NULL)
            {
                pKernelWorkItemsToolbar->onAfterKernelDebuggingEvent((const apAfterKernelDebuggingEvent&)eve);
            }
            updateWIToolbar(false, true);
        }
        break;

        case apEvent::AP_KERNEL_DEBUGGING_FAILED_EVENT:
        {
            // Notify the user:
            const apKernelDebuggingFailedEvent& kernelDebuggingFailedEvent = (const apKernelDebuggingFailedEvent&)eve;
            gtString failureMessage;
            kernelDebuggingFailedEvent.getKernelDebuggingFailureString(failureMessage);

            // If it's an OpenCL error:
            if (kernelDebuggingFailedEvent.failureReason() == apKernelDebuggingFailedEvent::AP_KERNEL_ENQUEUE_ERROR)
            {
                // Add the error code:
                gtString clErrorAsString;
                gdOpenCLErrorToString(kernelDebuggingFailedEvent.openCLError(), clErrorAsString);
                failureMessage.append(clErrorAsString);
            }

            // Display the message:
            acMessageBox::instance().critical(AF_STR_ErrorA, failureMessage.asASCIICharArray(), QMessageBox::Ok);
        }
        break;

        case apEvent::AP_KERNEL_DEBUGGING_INTERRUPTED_EVENT:
        {
            // Display a warning message about the interrupted kernel debugging:
            QMessageBox::StandardButton userAnswer = acMessageBox::instance().question(GD_STR_KernelDebuggingInterruptedTitle,
                                                     GD_STR_QuestionKernelDebuggingInterrupted,
                                                     QMessageBox::Yes | QMessageBox::No);

            if (userAnswer == QMessageBox::Yes)
            {
                // Temporarily disable all breakpoints so when we resume the process execution it will continue until the debugged kernel execution begins
                gaTemporarilyDisableAllBreakpoints();

                // Send an event to resume debugged process after the event handling is complete
                apDeferredCommandEvent deferredCommandEvent(apDeferredCommandEvent::AP_DEFERRED_COMMAND_RESUME_DEBUGGED_PROCESS, apDeferredCommandEvent::AP_GW_EVENT_OBSERVER);
                apEventsHandler::instance().registerPendingDebugEvent(deferredCommandEvent);
            }
        }
        break;

        case apEvent::AP_ADD_WATCH_EVENT:
        {
            apAddWatchEvent addWatchEvent = (const apAddWatchEvent&)eve;
            onAddWatch(addWatchEvent);
        }
        break;

        case apEvent::AP_HEX_CHANGED_EVENT:
        {
            // Update images and buffer views:
            apHexChangedEvent hexEvent = (const apHexChangedEvent&)eve;
            gaSetHexDisplayMode(hexEvent.displayHex());

            gdImagesAndBuffersManager::instance().updateOpenedViewsHexMode(hexEvent.displayHex());
        }
        break;

        case apEvent::AP_MDI_ACTIVATED_EVENT:
        {
            afQMdiSubWindow* pActiveMdiView = afMainAppWindow::instance()->activeMDISubWindow();

            if (pActiveMdiView != nullptr)
            {
                // set focus to active view:
                gdImagesAndBuffersManager::instance().updateActiveViewFocus(pActiveMdiView->widget());

                // if it is a source view set this as a tooltip handler
                const apMDIViewActivatedEvent& activationEvent = (const apMDIViewActivatedEvent&)eve;
                gtString fileExt;
                activationEvent.filePath().getFileExtension(fileExt);

                if (fileExt == AF_STR_clSourceFileExtension)
                {
                    acSourceCodeView* pSourceView = qobject_cast<acSourceCodeView*>(pActiveMdiView->widget());

                    if (nullptr != pSourceView)
                    {
                        pSourceView->SetTooltipResolver(this);
                    }
                }
            }
        }
        break;

        case apEvent::AP_EXECUTION_MODE_CHANGED_EVENT:
        {
            // Set the action text:
            QString startDebugTooltip = GD_STR_StartDebuggingTooltip;

            if (afExecutionModeManager::instance().isActiveMode(GD_STR_executionMode))
            {
                if (!afProjectManager::instance().currentProjectSettings().executablePath().isEmpty())
                {
                    gtString fileName;
                    afProjectManager::instance().currentProjectSettings().executablePath().getFileNameAndExtension(fileName);
                    QString argsStr = acGTStringToQString(fileName);
                    startDebugTooltip = QString(GD_STR_StartDebuggingTooltipWithParam).arg(argsStr);
                }
            }

            // Set the start debug action tooltip:
            afExecutionModeManager::instance().UpdateStartActionTooltip(startDebugTooltip);
        }
        break;

        case apEvent::AP_DEFERRED_COMMAND_EVENT:
        {
            const apDeferredCommandEvent& deferredCommandEvent = (const apDeferredCommandEvent&)eve;
            apDeferredCommandEvent::apDeferredCommand command = deferredCommandEvent.command();
            apDeferredCommandEvent::apDeferredCommandTarget target = deferredCommandEvent.commandTarget();

            if (apDeferredCommandEvent::AP_GW_EVENT_OBSERVER == target)
            {
                switch (command)
                {
                    case apDeferredCommandEvent::AP_DEFERRED_COMMAND_RESUME_DEBUGGED_PROCESS:
                    {
                        // Resume debugged process command:
                        bool rcRes = gaResumeDebuggedProcess();
                        GT_ASSERT(rcRes);
                    }
                    break;

                    default:
                        GT_ASSERT_EX(false, L"gwEventObserver: Invalid deferred command parameter in deferred command event");
                        break;
                }
            }
            else
            {
                // Got an event meant for someone else, note it in the log:
                gtString logMsg;
                logMsg.appendFormattedString(L"gwEventObserver got deferred command %d with target %d", command, target);
                OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
                GT_ASSERT(apDeferredCommandEvent::AP_UNKNOWN_COMMAND_TARGET != target);
            }
        }
        break;

        default:
            // Do nothing...
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        gwEventObserver::openBreakpointSourceCode
// Description: Is called on process suspension - opens the current debugged
//              process code
//              threadId - The breakpoint thread id.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        3/8/2011
// ---------------------------------------------------------------------------
bool gwEventObserver::openBreakpointSourceCode(osThreadId threadId)
{
    bool retVal = false;

    // Get the current call stack:
    bool inKernelDebugging = gaIsInKernelDebugging();
    bool inHSAKernelDebugging = gaIsInHSAKernelBreakpoint();

    if (inKernelDebugging || inHSAKernelDebugging || (threadId != OS_NO_THREAD_ID))
    {
        // Get the debugged process threads:
        osCallStack callStack;

        if (!inKernelDebugging)
        {
            retVal = gaGetThreadCallStack(threadId, callStack);
        }
        else if (inHSAKernelDebugging)
        {
            retVal = gaHSAGetCallStack(callStack);
        }
        else
        {
            retVal = gaGetCurrentlyDebuggedKernelCallStack(callStack);
        }

        GT_IF_WITH_ASSERT(retVal)
        {
            // Get the amount of the debugged process threads
            int callStackSize = callStack.amountOfStackFrames();
            int callStackIndex = 0;

            while (callStackIndex < (callStackSize - 1))
            {
                // Get the top frame:
                const osCallStackFrame* pCurrentFrame = NULL;
                pCurrentFrame = callStack.stackFrame(callStackIndex);
                GT_IF_WITH_ASSERT(pCurrentFrame != NULL)
                {
                    // Get the source code details for the current frame:
                    osFilePath sourceCodeFilePath = pCurrentFrame->sourceCodeFilePath();
                    const int& sourceCodeFileLineNumber = pCurrentFrame->sourceCodeFileLineNumber();
                    gtString moduleName = pCurrentFrame->moduleFilePath().asString();

                    if (!sourceCodeFilePath.asString().isEmpty())
                    {
                        GT_IF_WITH_ASSERT(_pGDApplicationCommands != NULL)
                        {
                            // Open the source code file:
                            _pGDApplicationCommands->openFileAtLineWithAdditionSourceDir(sourceCodeFilePath, moduleName, sourceCodeFileLineNumber, true);
                        }
                        break;
                    }

                    // Try the next frame:
                    callStackIndex ++;
                }
            }
        }
    }


    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gwEventObserver::updateLayout
// Description:
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        31/8/2011
// ---------------------------------------------------------------------------
void gwEventObserver::updateLayout()
{
    // assume this is normal debugging:
    afMainAppWindow::LayoutFormats currentLayout = afMainAppWindow::LayoutDebug;

    // check if this is kernel debugging:
    afRunModes runModes = afPluginConnectionManager::instance().getCurrentRunModeMask();

    if (runModes & AF_DEBUGGED_PROCESS_IN_KERNEL_DEBUGGING)
    {
        currentLayout = afMainAppWindow::LayoutDebugKernel;
    }

    afMainAppWindow::instance()->updateLayoutMode(currentLayout);
}

// ---------------------------------------------------------------------------
// Name:        gwEventObserver::updateApplicationTitle
// Description: update the application title
// Return Val:  void
// Author:      Gilad Yarnitzky
// Date:        26/8/2012
// ---------------------------------------------------------------------------
void gwEventObserver::updateApplicationTitle()
{
    // Calculate the title bar's string:
    gtString titleBarString;
    afCalculateCodeXLTitleBarString(titleBarString);

    GT_IF_WITH_ASSERT(_pApplicationCommands != NULL)
    {
        // Set the caption:
        _pApplicationCommands->setApplicationCaption(titleBarString);
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        onAddWatch
/// \brief Description: Is handling add watch event
/// \param[in]          addWatchEvent
/// \return void
/// -----------------------------------------------------------------------------------------------
void gwEventObserver::onAddWatch(const apAddWatchEvent& addWatchEvent)
{
    GT_IF_WITH_ASSERT(_pGDApplicationCommands != NULL)
    {
        if (addWatchEvent.isMultiwatch())
        {
            gdMultiWatchView* pMultiWatchView = _pGDApplicationCommands->multiWatchView(0);
            GT_IF_WITH_ASSERT(pMultiWatchView != NULL)
            {
                pMultiWatchView->show();
                pMultiWatchView->displayVariable(addWatchEvent.watchExpression());
                QDockWidget* pDockWidget = qobject_cast<QDockWidget*>(pMultiWatchView->parent());
                GT_IF_WITH_ASSERT(pDockWidget != NULL)
                {
                    pDockWidget->show();
                    pDockWidget->setFocus();
                    pDockWidget->raise();
                    pDockWidget->activateWindow();
                }
            }
        }
        else
        {
            _pGDApplicationCommands->addWatchVariable(addWatchEvent.watchExpression());
            gdWatchView* pWatchView = _pGDApplicationCommands->watchView();
            GT_IF_WITH_ASSERT(pWatchView != NULL)
            {
                pWatchView->show();
                QDockWidget* pDockWidget = qobject_cast<QDockWidget*>(pWatchView->parent());
                GT_IF_WITH_ASSERT(pDockWidget != NULL)
                {
                    pDockWidget->show();
                    pDockWidget->setFocus();
                    pDockWidget->raise();
                    pDockWidget->activateWindow();
                }
            }
        }
    }
}

/// -----------------------------------------------------------------------------------------------
/// \brief Name:        updateWIToolbar
/// \brief Description: Updates the state of the WI toolbar
/// \return void
/// -----------------------------------------------------------------------------------------------
void gwEventObserver::updateWIToolbar(bool rebuildThreadValues, bool rebuildWIValues)
{
    // Get the kernel work items toolbar:
    gwKernelWorkItemToolbar* pKernelWorkItemsToolbar = gwgDEBuggerAppWrapper::kernelWorkItemToolbar();
    GT_IF_WITH_ASSERT(pKernelWorkItemsToolbar != NULL)
    {
        if (rebuildThreadValues || rebuildWIValues)
        {
            pKernelWorkItemsToolbar->resetTooblarValues(rebuildThreadValues, rebuildWIValues);
        }

        pKernelWorkItemsToolbar->updateToolbarValues();
    }
}

/// -----------------------------------------------------------------------------------------------
QString gwEventObserver::Tooltip(QString& highlightedString)
{
    QString retVal;

    // Will get the new values:
    apExpression variableValue;

    // Get the current work item index:
    int currentWorkItemCoord[3] = { -1, -1, -1 };

    if (gaIsInKernelDebugging())
    {
        bool rcCo = gaGetKernelDebuggingCurrentWorkItem(currentWorkItemCoord[0], currentWorkItemCoord[1], currentWorkItemCoord[2]);
        GT_IF_WITH_ASSERT(rcCo)
        {
            if (!highlightedString.isEmpty())
            {
                gtString currentVariableName;
                currentVariableName.fromASCIIString(highlightedString.toLatin1());
                bool rcVal = gaGetKernelDebuggingExpressionValue(currentVariableName, currentWorkItemCoord, 0, variableValue);

                if (rcVal)
                {
                    retVal = acGTStringToQString(variableValue.m_type) + " " + highlightedString + " = " + acGTStringToQString(variableValue.m_value);
                }
            }
        }
    }

    return retVal;
}
