//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspEventObserver.cpp
///
//==================================================================================

//------------------------------ vspEventObserver.cpp ------------------------------

#include "stdafx.h"

// Windows:
#include <intsafe.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/Events/apApiConnectionEndedEvent.h>
#include <AMDTAPIClasses/Include/Events/apApiConnectionEstablishedEvent.h>
#include <AMDTAPIClasses/Include/Events/apBreakpointHitEvent.h>
#include <AMDTAPIClasses/Include/Events/apComputeContextCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apComputeContextDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apExceptionEvent.h>
#include <AMDTAPIClasses/Include/Events/apExecutionModeChangedEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apFlushTextureImageEvent.h>
#include <AMDTAPIClasses/Include/Events/apKernelDebuggingFailedEvent.h>
#include <AMDTAPIClasses/Include/Events/apKernelSourceBreakpointsUpdatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apModuleLoadedEvent.h>
#include <AMDTAPIClasses/Include/Events/apModuleUnloadedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLErrorEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLQueueCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLQueueDeletedEvent.h>
#include <AMDTAPIClasses/Include/Events/apOpenCLProgramBuildFailedWithDebugFlagsEvent.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessTerminatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apThreadCreatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apThreadTerminatedEvent.h>
#include <AMDTAPIClasses/Include/Events/apDeferredCommandEvent.h>
#include <AMDTAPIClasses/Include/apCLKernel.h>
#include <AMDTApplicationComponents/Include/acMessageBox.h>
#include <AMDTApplicationComponents/Include/acProgressDlg.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTApplicationComponents/Include/acSendErrorReportDialog.h>


// Application:
#include <AMDTApplicationFramework/Include/afExecutionModeManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariableChangedEvent.h>
#include <AMDTApplicationFramework/Include/afProjectManager.h>
#include <AMDTApplicationFramework/Include/afPluginConnectionManager.h>
#include <AMDTApplicationFramework/Include/afGlobalVariablesManager.h>

// AMDTGpuDebuggingComponents:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdEventStringBuilder.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/commands/gdFlushTexturesImagesCommand.h>

// Local:
#include <src/vscBreakpointsManager.h>
#include <src/vscDebugEngine.h>
#include <src/vscDebugEvents.h>
#include <src/vscDebugThread.h>
#include <src/vspEventObserver.h>
#include <src/vspImagesAndBuffersManager.h>
#include <src/vspSourceCodeViewer.h>

// VS:
#include <Include/Public/vscVspDTEInvoker.h>

#define _CTC_GUIDS_
#include <vsdebugguids.h>
#undef _CTC_GUIDS_

// Static members definitions:
IVscEventObserverOwner* vspEventObserver::_pOwner = NULL;

// ---------------------------------------------------------------------------
// Name:        vspEventObserver::vspEventObserver
// Description: Constructor
// Author:      Uri Shomroni
// Date:        16/9/2010
// ---------------------------------------------------------------------------
vspEventObserver::vspEventObserver(vspCDebugEngine& debugEngine, IDebugEventCallback2* piDebugEventCallback)
    : _debugEngine(debugEngine), _piDebugEventCallback(piDebugEventCallback), m_pWaitingForDeferredCommandDlg(nullptr)
{
    // Retain the (VS) event callback interface:
    if (nullptr != _piDebugEventCallback)
    {
        _piDebugEventCallback->AddRef();
    }

    // vspCDebugEngineCreateEvent Should theoretically be sent here, but since the debug engine has not
    // yet finished construction, this causes all sorts of issues.

    // Register as an events observer:
    apEventsHandler::instance().registerEventsObserver(*this, AP_APPLICATION_COMPONENTS_EVENTS_HANDLING_PRIORITY);
}

// ---------------------------------------------------------------------------
// Name:        vspEventObserver::~vspEventObserver
// Description: Destructor
// Author:      Uri Shomroni
// Date:        16/9/2010
// ---------------------------------------------------------------------------
vspEventObserver::~vspEventObserver()
{
    // Register as an events observer:
    apEventsHandler::instance().unregisterEventsObserver(*this);

    // Release the (VS) event callback interface:
    if (nullptr != _piDebugEventCallback)
    {
        _piDebugEventCallback->Release();
        _piDebugEventCallback = nullptr;
    }

    if (nullptr != m_pWaitingForDeferredCommandDlg)
    {
        delete m_pWaitingForDeferredCommandDlg;
        m_pWaitingForDeferredCommandDlg = nullptr;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspEventObserver::onEvent
// Description: Is called when a debugged process event occurs. Translates the
//              event to an appropriate IDebugXxxEvent2 class and sends it to
//              the IDebugEventCallback2 interface.
// Author:      Uri Shomroni
// Date:        16/9/2010
// ---------------------------------------------------------------------------
void vspEventObserver::onEvent(const apEvent& eve, bool& vetoEvent)
{
    GT_UNREFERENCED_PARAMETER(vetoEvent);

    // Get the event type:
    apEvent::EventType eveType = eve.eventType();
    osThreadId triggeringThreadId = eve.triggeringThreadId();
    vspCDebugThread* pThread = _debugEngine.getThread(triggeringThreadId);

    // Checks if the string should be output to log:
    bool outputOnlyToLog = false;

    switch (eveType)
    {
        case apEvent::AP_API_CONNECTION_ESTABLISHED:
        {
            const apApiConnectionEstablishedEvent& apiConnectionEstablishedEvent = (const apApiConnectionEstablishedEvent&)eve;
            apAPIConnectionType establishedConnectionType = apiConnectionEstablishedEvent.establishedConnectionType();

            if (establishedConnectionType == AP_OPENCL_API_CONNECTION)
            {
                wchar_t** pClProgramFiles = nullptr;
                int clProgramFilesItemCount = 0;
                vscVspDTEInvoker_GetKernelSourceFilePath(pClProgramFiles, clProgramFilesItemCount, false);

                GT_IF_WITH_ASSERT(nullptr != pClProgramFiles && clProgramFilesItemCount > 0)
                {
                    // Create a vector of osFilePath objects.
                    gtVector<osFilePath> clProgramFiles;

                    for (int i = 0; i < clProgramFilesItemCount; i++)
                    {
                        osFilePath filePath((nullptr != pClProgramFiles[i]) ? pClProgramFiles[i] : L"");
                        clProgramFiles.push_back(filePath);
                    }

                    // Set the kernel source file paths.
                    gaSetKernelSourceFilePath(clProgramFiles);

                    // Release the allocated strings.
                    vscVspDTEInvoker_DeleteWcharStrBuffersArray(pClProgramFiles, clProgramFilesItemCount);
                }
            }

            outputOnlyToLog = true;
        }
        break;

        case apEvent::AP_API_CONNECTION_ENDED:
        {
            outputOnlyToLog = true;
        }
        break;

        case apEvent::AP_BREAKPOINT_HIT:
        {
            // Handle the breakpoint:
            onBreakpointHitEvent((const apBreakpointHitEvent&)eve, pThread);
        }
        break;

        case apEvent::AP_EXCEPTION:
        {
            vspCDebugExceptionEvent* pDebugExceptionEvent = new vspCDebugExceptionEvent(&_debugEngine, (const apExceptionEvent&)eve);
            pDebugExceptionEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), (IDebugThread2*)pThread);
            pDebugExceptionEvent->Release();
        }
        break;

        case apEvent::AP_MODULE_LOADED:
        {
            // Let the debug engine know the module was loaded:
            const apModuleLoadedEvent& moduleLoadedEve = (const apModuleLoadedEvent&)eve;
            const osFilePath& loadedModulePath = moduleLoadedEve.modulePath();
            _debugEngine.onModuleLoaded(loadedModulePath, moduleLoadedEve.moduleLoadAddress(), moduleLoadedEve.areDebugSymbolsLoaded());

            // Get the newly-created module object:
            vspCDebugModule* pModule = _debugEngine.getModule(loadedModulePath);

            GT_IF_WITH_ASSERT(nullptr != pModule)
            {
                // Send a load event:
                vspCDebugModuleLoadEvent* pDebugModuleLoadEvent = new vspCDebugModuleLoadEvent(pModule, true);
                pDebugModuleLoadEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), (IDebugThread2*)pThread);
                pDebugModuleLoadEvent->Release();
            }
        }
        break;

        case apEvent::AP_MODULE_UNLOADED:
        {
            // Get the module path:
            const osFilePath& unloadedModulePath = ((const apModuleUnloadedEvent&)eve).modulePath();

            // Get the thread interface:
            vspCDebugModule* pModule = _debugEngine.getModule(unloadedModulePath);

            if (nullptr != pModule)
            {
                // Send an unload event:
                vspCDebugModuleLoadEvent* pDebugModuleLoadEvent = new vspCDebugModuleLoadEvent(pModule, false);
                pDebugModuleLoadEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), (IDebugThread2*)pThread);
                pDebugModuleLoadEvent->Release();
            }

            // Let the debug engine know the thread was unloaded:
            _debugEngine.onModuleUnloaded(unloadedModulePath);
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        {
            // Amit: fixing the disassembly window crash.
            OS_OUTPUT_DEBUG_LOG(L"On AP_DEBUGGED_PROCESS_CREATED: Trying to close Disassembly window.", OS_DEBUG_LOG_DEBUG);
            GT_IF_WITH_ASSERT(nullptr != _pOwner)
            {
                _pOwner->CloseDisassemblyWindow();

                // Update each of the open image / buffers / thumbnails views:
                vspImagesAndBuffersManager::instance().updateOpenedViewsOnEvent(false);

                // Make the pane empty:
                _pOwner->ClearMessagePane();

                // Create the event:
                vspCDebugEngineCreateEvent* pEngCreateEvent = new vspCDebugEngineCreateEvent(&_debugEngine);
                pEngCreateEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), nullptr, nullptr);
                pEngCreateEvent->Release();
                vspCDebugProgramCreateEvent* pProgramCreateEvent = new vspCDebugProgramCreateEvent;
                pProgramCreateEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), nullptr);
                pProgramCreateEvent->Release();

                // Our Debug engine also implements the IDebugProcess2 interface:
                _debugEngine.AddRef();
            }
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_RESUMED:
        {
            // Amit: bug fix - close the disassembly window before resuming.
            OS_OUTPUT_DEBUG_LOG(L"On AP_DEBUGGED_PROCESS_RUN_RESUMED: Trying to close Disassembly window.", OS_DEBUG_LOG_DEBUG);
            GT_IF_WITH_ASSERT(nullptr != _pOwner)
            {
                _pOwner->CloseDisassemblyWindow();

                // There is no need to generate an event, as Visual Studio considers an S_OK return value from IDebugProgram2::Execute() to be
                // a successful resume.
                vspImagesAndBuffersManager::instance().updateOpenedViewsOnEvent(false);
            }

            // If we were waiting for a deferred command, hide the dialog:
            hideDeferredCommandDialog();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED:
        {
            vspImagesAndBuffersManager::instance().updateOpenedViewsOnEvent(false);

            _debugEngine.setIsCurrentlyKernelDebugging(false);
            vspCDebugLoadCompleteEvent* pLoadCompleteEvent = new vspCDebugLoadCompleteEvent;
            pLoadCompleteEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), (IDebugThread2*)pThread);
            pLoadCompleteEvent->Release();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_SUSPENDED:
        {
            // Update each of the open image / buffers / thumbnails views:
            vspImagesAndBuffersManager::instance().updateOpenedViewsOnEvent(true);

            // Select the debugged kernel in the monitored tree:
            gdDebugApplicationTreeHandler::instance()->handleKernelDebuggingSelection();

            // Check if we need to flush all textures since openGL log was recorded
            bool wasRecorded = gaWasOpenGLDataRecordedInDebugSession();

            if (wasRecorded)
            {
                // Trigger a flush textures event. This event will be handled after the AP_DEBUGGED_PROCESS_RUN_SUSPENDED was fully
                // handled, letting other clients registered to AP_DEBUGGED_PROCESS_RUN_SUSPENDED finish their work.
                apFlushTextureImageEvent flushTexturedEvent;

                apEventsHandler::instance().registerPendingDebugEvent(flushTexturedEvent);
            }

            // If we were waiting for a deferred command, hide the dialog:
            hideDeferredCommandDialog();
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        {
            // Amit: for extra safety, close the disassembly window before terminating the debugging session.
            OS_OUTPUT_DEBUG_LOG(L"On AP_DEBUGGED_PROCESS_TERMINATED: Trying to close Disassembly window.", OS_DEBUG_LOG_DEBUG);

            GT_IF_WITH_ASSERT(nullptr != _pOwner)
            {
                _pOwner->CloseDisassemblyWindow();
            }

            // If we were waiting for a deferred command, hide the dialog:
            hideDeferredCommandDialog();

            vspImagesAndBuffersManager::instance().updateOpenedViewsOnEvent(false);

            // Close the document windows that we opened:
            vspSourceCodeViewer::instance().closeAllOpenedSourceWindows();

            // Reset the kernel debugging flag:
            _debugEngine.setIsCurrentlyKernelDebugging(false);

            int exitCode = ((const apDebuggedProcessTerminatedEvent&)eve).processExitCode();
            vspCDebugProgramDestroyEvent* pProgramDestroyEvent = new vspCDebugProgramDestroyEvent(exitCode);
            pProgramDestroyEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), nullptr);
            pProgramDestroyEvent->Release();

            // Go through all the existing threads and send thread destroy events:
            // NOTICE: this solves the situation where not all threads are destroyed, and debug engine is not released because of this
            // (because vspDebugThread holds the debug engine com pointer):
            const gtVector<vspCDebugThread*>& existingDebugProcessThread = _debugEngine.debugProcessThreads();
            gtVector <DWORD> existingThreadIds;

            for (int i = 0; i < (int)existingDebugProcessThread.size(); i++)
            {
                // Get the thread pointer:
                vspCDebugThread* pThreadDebugProcessThread = existingDebugProcessThread[i];

                if (nullptr != pThreadDebugProcessThread)
                {
                    // Get the thread id:
                    DWORD threadID = pThreadDebugProcessThread->threadId();

                    // Send the thread destruction notification:
                    vspCDebugThreadDestroyEvent* pThreadDestroyedEvent = new vspCDebugThreadDestroyEvent(threadID);
                    pThreadDestroyedEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), (IDebugThread2*)pThreadDebugProcessThread);
                    pThreadDestroyedEvent->Release();

                    // Save the thread id for later release:
                    existingThreadIds.push_back(threadID);
                }
            }

            // Notify the breakpoints manager on the process termination:
            vscBreakpointsManager::instance().onProcessTerminate();

            // Go through the thread ids and notify the debug engine for the thread release:
            for (int i = 0 ; i < (int)existingThreadIds.size(); i++)
            {
                // Get the current thread id:
                DWORD threadID = existingThreadIds[i];
                _debugEngine.onThreadDestroyed(threadID);
            }

            // Destroy the handles in the debug engine:
            _debugEngine.onProcessTermination();

            // Our Debug engine implementation also represents the IDebugProcess2 interface, so we need to release it when the process dies:
            _debugEngine.Release();
        }
        break;

        case apEvent::AP_THREAD_CREATED:
        {
            // Let the debug engine know the thread was created:
            osThreadId createdThreadId = ((const apThreadCreatedEvent&)eve).threadOSId();
            _debugEngine.onThreadCreated(createdThreadId);

            // Get the newly-created thread object:
            pThread = _debugEngine.getThread(createdThreadId);

            // Create a thread creation event:
            vspCDebugThreadCreateEvent* pThreadCreatedEvent = new vspCDebugThreadCreateEvent;
            pThreadCreatedEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), (IDebugThread2*)pThread);
            pThreadCreatedEvent->Release();
        }
        break;

        case apEvent::AP_THREAD_TERMINATED:
        {
            // Get the parameters:
            const apThreadTerminatedEvent& threadTermEve = (const apThreadTerminatedEvent&)eve;
            osThreadId destroyedThreadId = threadTermEve.threadOSId();
            int threadExitCode = threadTermEve.threadExitCode();

            // Get the thread interface:
            pThread = _debugEngine.getThread(destroyedThreadId);

            // Send the thread destruction notification:
            vspCDebugThreadDestroyEvent* pThreadDestroyedEvent = new vspCDebugThreadDestroyEvent(threadExitCode);
            pThreadDestroyedEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), (IDebugThread2*)pThread);
            pThreadDestroyedEvent->Release();

            // Let the debug engine know the thread was destroyed:
            _debugEngine.onThreadDestroyed(destroyedThreadId);
        }
        break;

        case apEvent::AP_OPENCL_PROGRAM_CREATED_EVENT:
        {
            const apOpenCLProgramCreatedEvent& programCreatedEve = (const apOpenCLProgramCreatedEvent&)eve;

            // Localize the path as needed:
            osFilePath programSourcePath = programCreatedEve.programSourceFilePath();
            gaRemoteToLocalFile(programSourcePath, true);

            // If there are any breakpoints that were on this source file, attempt to re-bind them:
            vscBreakpointsManager::instance().rebindBreakpointsInKernelSourceFile(programSourcePath);
        }
        break;

        case apEvent::AP_BEFORE_KERNEL_DEBUGGING_EVENT:
        {
            // Create threads for each kernel debugging wavefront:
            // TO_DO: Consider attaching this information to the apBeforeKernelDebuggingEvent.
            int numberOfWavefronts = 1;
            bool rcNum = gaGetKernelDebuggingAmountOfActiveWavefronts(numberOfWavefronts);
            GT_ASSERT(rcNum);

            for (int i = 0; i < numberOfWavefronts; i++)
            {
                // Let the debug engine know of the wavefront creation:
                _debugEngine.onKernelDebuggingWavefrontCreated(i);

                // Get the newly-created thread object:
                osThreadId spoofThreadId = DWORD_MAX - i;
                pThread = _debugEngine.getThread(spoofThreadId);

                // Create a thread creation event:
                vspCDebugThreadCreateEvent* pThreadCreatedEvent = new vspCDebugThreadCreateEvent;
                pThreadCreatedEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), (IDebugThread2*)pThread);
                pThreadCreatedEvent->Release();
            }

            _debugEngine.setIsCurrentlyKernelDebugging(true);
        }
        break;

        case apEvent::AP_AFTER_KERNEL_DEBUGGING_EVENT:
        {
            _debugEngine.setIsCurrentlyKernelDebugging(false);

            // Get all kernel debugging threads:
            int threadExitCode = 0;

            // Get the thread interface:
            const gtVector<vspCDebugThread*>& processThreads = _debugEngine.debugProcessThreads();

            int numberOfThreads = (int)processThreads.size();

            for (int i = 0; i < numberOfThreads; i++)
            {
                const vspCDebugThread* pCurrentProcessThread = processThreads[i];
                GT_IF_WITH_ASSERT(nullptr != pCurrentProcessThread)
                {
                    if (pCurrentProcessThread->isKernelDebugging())
                    {
                        // Send the thread destruction notification:
                        vspCDebugThreadDestroyEvent* pThreadDestroyedEvent = new vspCDebugThreadDestroyEvent(threadExitCode);
                        pThreadDestroyedEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), (IDebugThread2*)pCurrentProcessThread);
                        pThreadDestroyedEvent->Release();
                    }
                }
            }

            // Remove the threads from the debug engine:
            _debugEngine.onKernelDebuggingEnded();
        }
        break;

        case apEvent::AP_KERNEL_CURRENT_WORK_ITEM_CHANGED_EVENT:
        {
            // Refresh the variable views (watch, etc):
            GT_IF_WITH_ASSERT(nullptr != _pOwner)
            {
                _pOwner->ForceVariablesReevaluation();
            }
        }
        break;

        case apEvent::AP_KERNEL_DEBUGGING_FAILED_EVENT:
        {
            // Show an error to the user:
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
            acMessageBox::instance().critical(AF_STR_ErrorA, failureMessage.asASCIICharArray());
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
                apDeferredCommandEvent deferredCommandEvent(apDeferredCommandEvent::AP_DEFERRED_COMMAND_RESUME_DEBUGGED_PROCESS, apDeferredCommandEvent::AP_VSP_EVENT_OBSERVER);
                apEventsHandler::instance().registerPendingDebugEvent(deferredCommandEvent);

                // Show a waiting dialog:
                showDeferredCommandDialog(GD_STR_resumingDebuggedApplication);
            }
        }
        break;

        case apEvent::AP_FLUSH_TEXTURE_IMAGES_EVENT:
        {
            if (!gaIsInKernelDebugging())
            {
                gdFlushTexturesImagesCommand flushTexturesCommand(nullptr);
                flushTexturesCommand.execute();
            }
        }
        break;

        case apEvent::AP_KERNEL_SOURCE_BREAKPOINTS_UPDATED_EVENT:
        {
            const apKernelSourceBreakpointsUpdatedEvent& kernelSourceBreakpointsUpdateEvent = (const apKernelSourceBreakpointsUpdatedEvent&)eve;
            vscBreakpointsManager::instance().onKernelSourceCodeBreakpointsUpdated(kernelSourceBreakpointsUpdateEvent);
        }
        break;

        case apEvent::AP_OPENCL_PROGRAM_BUILD_FAILED_WITH_DEBUG_FLAGS_EVENT:
        {
            // Build a message:
            const apOpenCLProgramBuildFailedWithDebugFlagsEvent& programBuildFailedEvent = (const apOpenCLProgramBuildFailedWithDebugFlagsEvent&)eve;
            gtString warningMessage;
            gdOpenCLErrorToString((int)programBuildFailedEvent.buildErrorCode(), warningMessage);

            int contextId = programBuildFailedEvent.contextIndex();
            int programId = programBuildFailedEvent.programIndex();

            if ((contextId != -1) && (programId != -1))
            {
                warningMessage.prependFormattedString(GD_STR_WarningProgramBuildErrorWithDebugFlags, programId + 1, contextId);
            }
            else // (contextId == -1) || (programId == -1)
            {
                warningMessage.prepend(GD_STR_WarningProgramBuildErrorWithDebugFlagsNoIndices);
            }

            // Display it to the user:
            acMessageBox::instance().warning(AF_STR_WarningA, warningMessage.asASCIICharArray());
        }
        break;

        case apEvent::AP_DEFERRED_COMMAND_EVENT:
        {
            apDeferredCommandEvent deferredCommandEvent = (const apDeferredCommandEvent&)eve;
            apDeferredCommandEvent::apDeferredCommand command = deferredCommandEvent.command();
            apDeferredCommandEvent::apDeferredCommandTarget target = deferredCommandEvent.commandTarget();

            if (apDeferredCommandEvent::AP_VSP_EVENT_OBSERVER == target)
            {
                switch (command)
                {
                    case apDeferredCommandEvent::AP_DEFERRED_COMMAND_RESUME_DEBUGGED_PROCESS:
                    {
                        // Hide the dialog shown:
                        hideDeferredCommandDialog();

                        // Resume debugged process command:
                        bool rcRes = vscVspDTEInvoker_ResumeDebugging();
                        GT_ASSERT(rcRes);
                    }
                    break;

                    default:
                        GT_ASSERT_EX(false, L"vspEventObserver: Invalid deferred command parameter in deferred command event");
                        break;
                }
            }
            else
            {
                // Got an event meant for someone else, note it in the log:
                gtString logMsg;
                logMsg.appendFormattedString(L"vspEventObserver got deferred command %d with target %d", command, target);
                OS_OUTPUT_DEBUG_LOG(logMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
                GT_ASSERT(apDeferredCommandEvent::AP_UNKNOWN_COMMAND_TARGET != target);
            }
        }
        break;

        default:
        {
            // Ignore other events
        }
        break;
    }

    // Build the string and add to the list view:
    gtString eventAsString;
    gdEventStringBuilder stringEventBuilder;
    stringEventBuilder.buildEventString(eve, eventAsString);

    if (!eventAsString.isEmpty())
    {
        GT_IF_WITH_ASSERT(nullptr != _pOwner)
        {
            const wchar_t* pBuf = eventAsString.asCharArray();
            _pOwner->OutputMessage(pBuf, outputOnlyToLog);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspEventObserver::setDebugEventCallbackInterface
// Description: Set the callback interface
// Author:      Uri Shomroni
// Date:        16/9/2010
// ---------------------------------------------------------------------------
void vspEventObserver::setDebugEventCallbackInterface(IDebugEventCallback2* piDebugEventCallback)
{
    // Retain the old event callback interface:
    if (nullptr != _piDebugEventCallback)
    {
        _piDebugEventCallback->Release();
    }

    // Set the member:
    _piDebugEventCallback = piDebugEventCallback;

    // Retain the new event callback interface:
    if (nullptr != _piDebugEventCallback)
    {
        _piDebugEventCallback->AddRef();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspEventObserver::onBreakpointHitEvent
// Description: handles a (ap)breakpoint hit event
// Author:      Uri Shomroni
// Date:        10/10/2010
// ---------------------------------------------------------------------------
void vspEventObserver::onBreakpointHitEvent(const apBreakpointHitEvent& bkptEve, IDebugThread2* pTriggeringThread)
{
    bool breakpointHandled = false;
    apBreakReason breakReason = bkptEve.breakReason();
    vscBreakpointsManager& theBreakpointsMgr = vscBreakpointsManager::instance();

    switch (breakReason)
    {
        case AP_BREAK_COMMAND_HIT:
        {
            // A "break" command caused this break:
            vspCDebugBreakEvent* pDebugBreakEvent = new vspCDebugBreakEvent;
            pDebugBreakEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), (IDebugThread2*)pTriggeringThread);
            pDebugBreakEvent->Release();

            // No need to continue:
            breakpointHandled = true;
        }
        break;

        case AP_NEXT_MONITORED_FUNCTION_BREAKPOINT_HIT:
        case AP_DRAW_MONITORED_FUNCTION_BREAKPOINT_HIT:
        case AP_FRAME_BREAKPOINT_HIT:
        {
            // A step command caused this break:
            vspCDebugStepCompleteEvent* pDebugStepCompleteEvent = new vspCDebugStepCompleteEvent;
            pDebugStepCompleteEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), (IDebugThread2*)pTriggeringThread);
            pDebugStepCompleteEvent->Release();

            // No need to continue:
            breakpointHandled = true;
        }
        break;

        case AP_STEP_IN_BREAKPOINT_HIT:
        case AP_STEP_OVER_BREAKPOINT_HIT:
        case AP_STEP_OUT_BREAKPOINT_HIT:
        {
            IDebugThread2* piThread = pTriggeringThread;

            if (!gaIsHostBreakPoint() && gaIsInKernelDebugging())
            {
                // A kernel debugging step command caused this break:
                // Get the first wavefront:
                vspCDebugThread* pFirstWavefront = _debugEngine.getThread(DWORD_MAX);
                piThread = pFirstWavefront;
            }

            vspCDebugStepCompleteEvent* pDebugStepCompleteEvent = new vspCDebugStepCompleteEvent;
            pDebugStepCompleteEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), (IDebugThread2*)piThread);
            pDebugStepCompleteEvent->Release();

            // No need to continue:
            breakpointHandled = true;
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
            // Get the generic breakpoint type from the break reason:
            apGenericBreakpointType breakpointType = apGenericBreakpoint::breakpointTypeFromBreakReason(breakReason);
            GT_IF_WITH_ASSERT(breakpointType != AP_BREAK_TYPE_UNKNOWN)
            {
                vspCDebugBreakpoint* pBreakpoint = theBreakpointsMgr.getGenericBreakpoint(breakpointType);
                GT_IF_WITH_ASSERT(pBreakpoint)
                {
                    if (pBreakpoint->isEnabled())
                    {
                        // Let the breakpoint know it was hit:
                        // TO_DO: check if the built-in visual studio breakpoints increment their hit count when disabled, when a condition is on, etc.
                        pBreakpoint->onBreakpointHit();

                        // Send a breakpoint event:
                        vspCDebugBreakpointEvent* pDebugBreakpointEvent = new vspCDebugBreakpointEvent(pBreakpoint);
                        pDebugBreakpointEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), (IDebugThread2*)pTriggeringThread);
                        pDebugBreakpointEvent->Release();

                        // No need to continue:
                        breakpointHandled = true;
                    }
                }
            }
        }
        break;

        case AP_MONITORED_FUNCTION_BREAKPOINT_HIT:
        {
            // Get the broken-on function ID:
            const apFunctionCall* pFunctionCall = bkptEve.breakedOnFunctionCall();
            GT_IF_WITH_ASSERT(nullptr != pFunctionCall)
            {
                // Get the breakpoint object:
                apMonitoredFunctionId breakedOnFuncId = pFunctionCall->functionId();
                vspCDebugBreakpoint* pBreakpoint = theBreakpointsMgr.getMonitoredFunctionBreakpoint(breakedOnFuncId);
                GT_IF_WITH_ASSERT(pBreakpoint)
                {
                    if (pBreakpoint->isEnabled())
                    {
                        // Let the breakpoint know it was hit:
                        // TO_DO: check if the built-in visual studio breakpoints increment their hit count when disabled, when a condition is on, etc.
                        pBreakpoint->onBreakpointHit();

                        // Send a breakpoint event:
                        vspCDebugBreakpointEvent* pDebugBreakpointEvent = new vspCDebugBreakpointEvent(pBreakpoint);
                        pDebugBreakpointEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), (IDebugThread2*)pTriggeringThread);
                        pDebugBreakpointEvent->Release();

                        // No need to continue:
                        breakpointHandled = true;
                    }
                }
            }
        }
        break;

        case AP_KERNEL_FUNCTION_NAME_BREAKPOINT_HIT:
        {
            // Get the first wavefront:
            vspCDebugThread* pFirstWavefront = _debugEngine.getThread(DWORD_MAX);

            // Get the currently debugged kernel function name:
            apCLKernel currentlyDebuggedKernel(OA_CL_NULL_HANDLE, 0, OA_CL_NULL_HANDLE, L"");
            bool rcKer = gaGetCurrentlyDebuggedKernelDetails(currentlyDebuggedKernel);
            GT_IF_WITH_ASSERT(rcKer)
            {
                const gtString& currentlyDebuggedKernelFunctionName = currentlyDebuggedKernel.kernelFunctionName();
                vspCDebugBreakpoint* pBreakpoint = theBreakpointsMgr.getKernelFunctionNameBreakpoint(currentlyDebuggedKernelFunctionName);
                GT_IF_WITH_ASSERT(pBreakpoint)
                {
                    if (pBreakpoint->isEnabled())
                    {
                        // Let the breakpoint know it was hit:
                        // TO_DO: check if the built-in visual studio breakpoints increment their hit count when disabled, when a condition is on, etc.
                        pBreakpoint->onBreakpointHit();

                        // Send a breakpoint event:
                        vspCDebugBreakpointEvent* pDebugBreakpointEvent = new vspCDebugBreakpointEvent(pBreakpoint);
                        pDebugBreakpointEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), (IDebugThread2*)pFirstWavefront);
                        pDebugBreakpointEvent->Release();

                        // No need to continue:
                        breakpointHandled = true;
                    }
                }
            }
        }
        break;

        case AP_KERNEL_SOURCE_CODE_BREAKPOINT_HIT:
        {
            // Get the first wavefront:
            vspCDebugThread* pFirstWavefront = _debugEngine.getThread(DWORD_MAX);

            // This is a kernel source breakpoint:
            oaCLProgramHandle programHandle = OA_CL_NULL_HANDLE;
            int lineNumber = -1;
            bool rcLoc = gaGetKernelDebuggingLocation(programHandle, lineNumber);
            GT_IF_WITH_ASSERT(rcLoc)
            {
                // Find the breakpoint which matches this location:
                vspCDebugBreakpoint* pBreakpoint = theBreakpointsMgr.getKernelSourceCodeBreakpoint(programHandle, lineNumber);
                GT_IF_WITH_ASSERT(pBreakpoint)
                {
                    if (pBreakpoint->isEnabled())
                    {
                        // Let the breakpoint know it was hit:
                        // TO_DO: check if the built-in visual studio breakpoints increment their hit count when disabled, when a condition is on, etc.
                        pBreakpoint->onBreakpointHit();

                        // Send a breakpoint event:
                        vspCDebugBreakpointEvent* pDebugBreakpointEvent = new vspCDebugBreakpointEvent(pBreakpoint);
                        pDebugBreakpointEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), (IDebugThread2*)pFirstWavefront);
                        pDebugBreakpointEvent->Release();

                        // No need to continue:
                        breakpointHandled = true;
                    }
                }
            }

            if (!breakpointHandled)
            {
                // This is also the default reason for kernel debugging - if we stopped for any other reason, mark it as "step ended" to show the user the call stack, etc.
                vspCDebugThread* pFirstWavefrontDebugEngine = _debugEngine.getThread(DWORD_MAX);
                vspCDebugStepCompleteEvent* pDebugStepCompleteEvent = new vspCDebugStepCompleteEvent;
                pDebugStepCompleteEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), (IDebugThread2*)pFirstWavefrontDebugEngine);
                pDebugStepCompleteEvent->Release();

                // No need to continue:
                breakpointHandled = true;
            }

        }
        break;

        case AP_BEFORE_KERNEL_DEBUGGING_HIT:
        case AP_AFTER_KERNEL_DEBUGGING_HIT:
        {
            // These should not be sent to visual studio.
            // However, we also do not expect them to get here at all, since the process debugger should filter them out.
            GT_ASSERT(false);
        }
        break;

        case AP_HOST_BREAKPOINT_HIT:
        {
            // Host Breakpoint
            osFilePath sourcePath;
            int lineNumber = -1;

            // If we can't get the location from the process debugger, get it from the call stack
            bool rcLoc = gaGetHostBreakpointLocation(sourcePath, lineNumber);

            if (!rcLoc)
            {
                osCallStack triggeringThreadStack;
                bool rcStk = gaGetThreadCallStack(bkptEve.triggeringThreadId(), triggeringThreadStack);
                GT_IF_WITH_ASSERT(rcStk && (0 < triggeringThreadStack.amountOfStackFrames()))
                {
                    const osCallStackFrame* pTopFrame = triggeringThreadStack.stackFrame(0);
                    GT_IF_WITH_ASSERT(nullptr != pTopFrame)
                    {
                        sourcePath = pTopFrame->sourceCodeFilePath();
                        lineNumber = pTopFrame->sourceCodeFileLineNumber();
                    }
                }
            }

            vspCDebugBreakpoint* pBreakpoint = theBreakpointsMgr.getHostSourceBreakpoint(sourcePath, lineNumber);
            GT_IF_WITH_ASSERT(pBreakpoint)
            {
                if (pBreakpoint->isEnabled())
                {
                    // Let the breakpoint know it was hit:
                    // TO_DO: check if the built-in visual studio breakpoints increment their hit count when disabled, when a condition is on, etc.
                    pBreakpoint->onBreakpointHit();

                    // Send a breakpoint event:
                    vspCDebugBreakpointEvent* pDebugBreakpointEvent = new vspCDebugBreakpointEvent(pBreakpoint);
                    pDebugBreakpointEvent->send(_piDebugEventCallback, (IDebugEngine2*)(&_debugEngine), (IDebugProgram2*)(&_debugEngine), (IDebugThread2*)pTriggeringThread);
                    pDebugBreakpointEvent->Release();

                    // No need to continue:
                    breakpointHandled = true;
                }
            }
        }
        break;

        default:
        {
            // Unexpected value:
            GT_ASSERT(false);
        }
        break;
    }

    // If we couldn't tell VS that this breakpoint happened:
    if (!breakpointHandled)
    {
        // We are still in run mode (and not break mode). So let's resume debugging:
        gaResumeDebuggedProcess();
    }
}

void vspEventObserver::setOwner(IVscEventObserverOwner* pOwner)
{
    _pOwner = pOwner;
}

void vspEventObserver::OnExceptionEvent(const apExceptionEvent& exceptionEvent)
{

    // If this is a second chance event, or a fatal Linux signal, the debugged process is going to die:
    if (exceptionEvent.isSecondChance() || exceptionEvent.isFatalLinuxSignal())
    {
        // Get the crash details from the plugin that reported it:
        osCallStack crashStack;
        bool openCLEnglineLoaded = false;
        bool openGLEnglineLoaded = false;
        bool kernelDebuggingEnteredAtLeastOnce = false;
        afPluginConnectionManager::instance().getExceptionEventDetails(exceptionEvent, crashStack, openCLEnglineLoaded, openGLEnglineLoaded, kernelDebuggingEnteredAtLeastOnce);

        // Update the addition information:
        //updateAdditionInformationString(openCLEnglineLoaded, openGLEnglineLoaded, kernelDebuggingEnteredAtLeastOnce);
        // Build the string:
        QString m_additionalInformation;
        m_additionalInformation.append(AF_STR_SendErrorReportAdditionalStringHeader);
        m_additionalInformation.append(AF_STR_NewLineA);

        // Add opencl engine info:
        m_additionalInformation.append(AF_STR_SendErrorReportAdditionalStringOpenCL);
        m_additionalInformation.append(openCLEnglineLoaded ? "Yes" : "No");
        m_additionalInformation.append(AF_STR_NewLineA);

        // Add opengl engine info:
        m_additionalInformation.append(AF_STR_SendErrorReportAdditionalStringOpenGL);
        m_additionalInformation.append(openGLEnglineLoaded ? "Yes" : "No");
        m_additionalInformation.append(AF_STR_NewLineA);

        // Add Kernel debugging info:
        m_additionalInformation.append(AF_STR_SendErrorReportAdditionalStringKernel);
        m_additionalInformation.append(kernelDebuggingEnteredAtLeastOnce ? "Yes" : "No");
        m_additionalInformation.append(AF_STR_NewLineA);

        // Display the error report dialog:
        QPixmap iconPixMap;
        acSetIconInPixmap(iconPixMap, afGlobalVariablesManager::ProductIconID(), AC_64x64_ICON);
        acSendErrorReportDialog* pSendErrorReportDialog = new acSendErrorReportDialog(nullptr, afGlobalVariablesManager::ProductNameA(), iconPixMap);
        GT_IF_WITH_ASSERT(nullptr != pSendErrorReportDialog)
        {
            // Get the exception reason:
            osExceptionReason exceptionReason = exceptionEvent.exceptionReason();

            pSendErrorReportDialog->displayErrorReportDialog(exceptionReason, crashStack, m_additionalInformation);
            delete pSendErrorReportDialog;
        }
    }
}

void vspEventObserver::showDeferredCommandDialog(const char* message)
{
    // Hide on empty string:
    if ((nullptr == message) || ((char)0 == message[0]))
    {
        hideDeferredCommandDialog();
    }
    else
    {
        if (nullptr == m_pWaitingForDeferredCommandDlg)
        {
            m_pWaitingForDeferredCommandDlg = new(std::nothrow) acProgressDlg(nullptr);
            GT_IF_WITH_ASSERT(nullptr != m_pWaitingForDeferredCommandDlg)
            {
                // Leave m_pWaitingForDeferredCommandDlg->SetLabelText(label); as the default AC_PROGRESSDLG_DEAFULT_MSG
                m_pWaitingForDeferredCommandDlg->ShowCancelButton(false);
            }
        }

        if (nullptr != m_pWaitingForDeferredCommandDlg)
        {
            m_pWaitingForDeferredCommandDlg->SetHeader(message);
            m_pWaitingForDeferredCommandDlg->show();
        }
    }
}
void vspEventObserver::hideDeferredCommandDialog()
{
    if (nullptr != m_pWaitingForDeferredCommandDlg)
    {
        m_pWaitingForDeferredCommandDlg->hide();
    }
}

