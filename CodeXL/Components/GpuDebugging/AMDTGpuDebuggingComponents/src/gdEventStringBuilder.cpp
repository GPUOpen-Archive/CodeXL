//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gdEventStringBuilder.cpp
///
//==================================================================================

//------------------------------ gdEventStringBuilder.cpp ------------------------------

#include <QtWidgets>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAPIClasses/Include/apCLKernel.h>
#include <AMDTAPIClasses/Include/apCLProgram.h>
#include <AMDTAPIClasses/Include/Events/apSearchingForMemoryLeaksEvent.h>
#include <AMDTAPIClasses/Include/Events/apTechnologyMonitorFailureEvent.h>
#include <AMDTAPIClasses/Include/Events/apUserWarningEvent.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// AMDTApplicationFramework:
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>

// Local:
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdEventStringBuilder.h>
#include <AMDTGpuDebuggingComponents/Include/gdStringConstants.h>

// ---------------------------------------------------------------------------
// Name:        gdDebuggedProcessEventsView::buildEventString
// Description: Build an output message from the requested event
// Arguments:   const apEvent& eve - the input event
//              gtString& eventMessage - the output message for the incoming event
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        5/7/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::buildEventString(const apEvent& eve, gtString& eventMessage)
{
    // Get the event type:
    apEvent::EventType eventType = eve.eventType();

    // Down cast the event according to its type, and call the appropriate
    // event handling function:
    switch (eventType)
    {
        case apEvent::AP_DEBUGGED_PROCESS_CREATED:
        {
            const apDebuggedProcessCreatedEvent& processCreatedEvent = (const apDebuggedProcessCreatedEvent&)eve;
            onProcessCreationString(processCreatedEvent, eventMessage);
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_RUN_STARTED:
        {
            eventMessage = GD_STR_ProcessEventsViewProcessStarted;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_TERMINATED:
        {
            eventMessage = AF_STR_ProcessEventsViewProcessExit;
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_CREATION_FAILURE:
        {
            eventMessage = GD_STR_ProcessEventsViewProcessCreationFailed;
        }
        break;

        case apEvent::AP_MODULE_LOADED:
        {
            const apModuleLoadedEvent& moduleLoadedEvent = (const apModuleLoadedEvent&)eve;
            onModuleLoadedString(moduleLoadedEvent, eventMessage);
        }
        break;

        case apEvent::AP_MODULE_UNLOADED:
        {
            const apModuleUnloadedEvent& moduleUnloadedEvent = (const apModuleUnloadedEvent&)eve;
            onModuleUnloadedString(moduleUnloadedEvent, eventMessage);
        }
        break;

        case apEvent::AP_EXCEPTION:
        {
            const apExceptionEvent& exceptionEvent = (const apExceptionEvent&)eve;
            onExceptionString(exceptionEvent, eventMessage);
        }
        break;

        case apEvent::AP_OUTPUT_DEBUG_STRING:
        {
            const apOutputDebugStringEvent& outputDebugStringEvent = (const apOutputDebugStringEvent&)eve;
            onOutputDebugString(outputDebugStringEvent, eventMessage);
        }
        break;

        case apEvent::AP_GDB_OUTPUT_STRING:
        {
            const apGDBOutputStringEvent& outputGDBStringEvent = (const apGDBOutputStringEvent&)eve;
            onOutputGDBString(outputGDBStringEvent, eventMessage);
        }
        break;

        case apEvent::AP_GDB_ERROR:
        {
            const apGDBErrorEvent& gdbErrorEvent = (const apGDBErrorEvent&)eve;
            onGDBErrorString(gdbErrorEvent, eventMessage);
        }
        break;

        case apEvent::AP_DEBUGGED_PROCESS_OUTPUT_STRING:
        {
            const apDebuggedProcessOutputStringEvent outputStringEvent = (const apDebuggedProcessOutputStringEvent&)eve;
            onDebuggedProceessOutputString(outputStringEvent, eventMessage);
        }
        break;

        case apEvent::AP_BREAKPOINT_HIT:
        {
            const apBreakpointHitEvent& breakpointEvent = (const apBreakpointHitEvent&)eve;

            if (breakpointEvent.breakReason() != AP_MEMORY_LEAK_BREAKPOINT_HIT)
            {
                onBreakHitString(breakpointEvent, eventMessage);
            }

        }
        break;

        case apEvent::AP_MEMORY_LEAK:
        {
            const apMemoryLeakEvent& memoryLeakEvent = (const apMemoryLeakEvent&)eve;
            onMemoryLeakEventString(memoryLeakEvent, eventMessage);
            break;
        }

        case apEvent::AP_SEARCHING_FOR_MEMORY_LEAKS:
        {
            const apSearchingForMemoryLeaksEvent& searchingForMemoryLeaksEvent = (const apSearchingForMemoryLeaksEvent&)eve;
            eventMessage = searchingForMemoryLeaksEvent.message();
            break;
        }

        case apEvent::AP_DETECTED_ERROR_EVENT:
        {
            const apDebuggedProcessDetectedErrorEvent& errorEvent = (const apDebuggedProcessDetectedErrorEvent&)eve;
            onDebuggedProcessErrorEventString(errorEvent, eventMessage);
        }
        break;

        case apEvent::AP_THREAD_CREATED:
        {
            const apThreadCreatedEvent& threadCreatedEvent = (const apThreadCreatedEvent&)eve;
            onDebuggedProcessThreadCreatedEventString(threadCreatedEvent, eventMessage);
        }
        break;

        case apEvent::AP_THREAD_TERMINATED:
        {
            const apThreadTerminatedEvent& threadTerminatedEvent = (const apThreadTerminatedEvent&)eve;
            onDebuggedProcessThreadTerminatedEventString(threadTerminatedEvent, eventMessage);
        }
        break;

        case apEvent::AP_API_CONNECTION_ESTABLISHED:
        {
            const apApiConnectionEstablishedEvent& apiConnectionEstablishedEvent = (const apApiConnectionEstablishedEvent&)eve;
            onAPIConnectionEstablishedEventString(apiConnectionEstablishedEvent, eventMessage);
        }
        break;

        case apEvent::AP_API_CONNECTION_ENDED:
        {
            const apApiConnectionEndedEvent& apiConnectionEndedEvent = (const apApiConnectionEndedEvent&)eve;
            onAPIConnectionEndedEventString(apiConnectionEndedEvent, eventMessage);
        }
        break;

        case apEvent::AP_RENDER_CONTEXT_CREATED_EVENT:
        {
            const apRenderContextCreatedEvent& renderContextCreatedEvent = (const apRenderContextCreatedEvent&)eve;
            onRenderContextCreatedEventString(renderContextCreatedEvent, eventMessage);
        }
        break;

        case apEvent::AP_RENDER_CONTEXT_DELETED_EVENT:
        {
            const apRenderContextDeletedEvent& renderContextDeletedEvent = (const apRenderContextDeletedEvent&)eve;
            onRenderContextDeletedEventString(renderContextDeletedEvent, eventMessage);
        }
        break;

        case apEvent::AP_COMPUTE_CONTEXT_CREATED_EVENT:
        {
            const apComputeContextCreatedEvent& computeContextCreatedEvent = (const apComputeContextCreatedEvent&)eve;
            onComputeContextCreatedEventString(computeContextCreatedEvent, eventMessage);
        }
        break;

        case apEvent::AP_COMPUTE_CONTEXT_DELETED_EVENT:
        {
            const apComputeContextDeletedEvent& computeContextDeletedEvent = (const apComputeContextDeletedEvent&)eve;
            onComputeContextDeletedEventString(computeContextDeletedEvent, eventMessage);
        }
        break;

        case apEvent::AP_GL_DEBUG_OUTPUT_MESSAGE:
        {
            const apGLDebugOutputMessageEvent& debugOutputMessageEvent = (const apGLDebugOutputMessageEvent&)eve;
            onDebugOutputMessageEventString(debugOutputMessageEvent, eventMessage);
        }
        break;

        case apEvent::AP_OPENCL_ERROR:
        {
            const apOpenCLErrorEvent& clErrorEvent = (const apOpenCLErrorEvent&)eve;
            onOpenCLErrorMessageEventString(clErrorEvent, eventMessage);
        }
        break;

        case apEvent::AP_OPENCL_QUEUE_CREATED_EVENT:
        {
            const apOpenCLQueueCreatedEvent& queueCreatedEvent = (const apOpenCLQueueCreatedEvent&)eve;
            onOpenCLQueueCreatedEventString(queueCreatedEvent, eventMessage);
        }
        break;

        case apEvent::AP_OPENCL_QUEUE_DELETED_EVENT:
        {
            const apOpenCLQueueDeletedEvent& queueDeletedEvent = (const apOpenCLQueueDeletedEvent&)eve;
            onOpenCLQueueDeletedEventString(queueDeletedEvent, eventMessage);
        }
        break;

        case apEvent::AP_OPENCL_PROGRAM_CREATED_EVENT:
        {
            const apOpenCLProgramCreatedEvent& programCreatedEvent = (const apOpenCLProgramCreatedEvent&)eve;
            onOpenCLProgramCreatedEventString(programCreatedEvent, eventMessage);
        }
        break;

        case apEvent::AP_OPENCL_PROGRAM_DELETED_EVENT:
        {
            const apOpenCLProgramDeletedEvent& programDeletedEvent = (const apOpenCLProgramDeletedEvent&)eve;
            onOpenCLProgramDeletedEventString(programDeletedEvent, eventMessage);
        }
        break;

        case apEvent::AP_OPENCL_PROGRAM_BUILD_EVENT:
        {
            const apOpenCLProgramBuildEvent& programBuildEvent = (const apOpenCLProgramBuildEvent&)eve;
            onOpenCLProgramBuildEventString(programBuildEvent, eventMessage);
        }
        break;

        case apEvent::AP_OPENCL_PROGRAM_BUILD_FAILED_WITH_DEBUG_FLAGS_EVENT:
        {
            const apOpenCLProgramBuildFailedWithDebugFlagsEvent& programBuildFailedEvent = (const apOpenCLProgramBuildFailedWithDebugFlagsEvent&)eve;
            onOpenCLProgramBuildFailedWithDebugFlagsEventString(programBuildFailedEvent, eventMessage);
        }
        break;

        case apEvent::AP_TECHNOLOGY_MONITOR_FAILURE_EVENT:
        {
            const apTechnologyMonitorFailureEvent& monitorFailedEvent = (const apTechnologyMonitorFailureEvent&)eve;
            eventMessage = GD_STR_ProcessEventsViewTechnologyMonitorFailure;
            eventMessage.append(monitorFailedEvent.failInformation());
        }
        break;

        case apEvent::AP_USER_WARNING:
        {
            const apUserWarningEvent& userWarningEvent = (const apUserWarningEvent&)eve;
            eventMessage = userWarningEvent.userWarningString();
        }
        break;

        default:
            // We do not report these events to the user.
            break;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onProcessCreationString
// Description: Build string for onProcessCreation event
// Author:      Gilad Yarnitzky
// Date:        21/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onProcessCreationString(const apDebuggedProcessCreatedEvent& processCreatedEvent, gtString& messageString)
{
    (void)(messageString);  // unused
    // Get the process creation data:
    const apDebugProjectSettings& processCreationData = processCreatedEvent.processCreationData();

    // Get the process executable path:
    const osFilePath& executablePath = processCreationData.executablePath();

    // Get the executable file name:
    gtString fileName;
    executablePath.getFileNameAndExtension(fileName);

    // Report the process creation:
    gtString createdReportString = GD_STR_ProcessEventsViewProcessCreated;
    createdReportString.append(AF_STR_Space);
    createdReportString.append(fileName.asCharArray());

}
// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onModuleLoadedString
// Description: Build string for onModuleUnloaded event
// Author:      Gilad Yarnitzky
// Date:        20/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onModuleLoadedString(const apModuleLoadedEvent& dllLoadedEvent, gtString& messageString)
{
    // Get the loaded module path:
    const gtString& modulePath = dllLoadedEvent.modulePath();

    // Insert the event report string into the debugged process events list.
    // The terminology is:
    // - Windows - dll
    // - Linux / Mac - module

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    messageString = GD_STR_ProcessEventsViewDllLoad;
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    messageString = GD_STR_ProcessEventsViewModuleLoad;
#else
#error Unknown build configuration!
#endif
    messageString += modulePath;
}


// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onModuleUnloadedString
// Description: Build string for onModuleUnloaded event
// Author:      Gilad Yarnitzky
// Date:        20/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onModuleUnloadedString(const apModuleUnloadedEvent& dllUnloadedEvent, gtString& messageString)
{
    // Get the DLL path:
    const gtString& modulePath = dllUnloadedEvent.modulePath();

    // Insert the module unload report into the list.
    // The terminology is:
    // - Windows - dll
    // - Linux / Mac - module
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    messageString = GD_STR_ProcessEventsViewDllUnload;
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    messageString = GD_STR_ProcessEventsViewModuleUnload;
#else
#error Unknown build configuration!
#endif

    if (!modulePath.isEmpty())
    {
        messageString += modulePath.asCharArray();
    }
    else
    {
        messageString += AF_STR_NotAvailable;
    }
}


// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::OnExceptionString
// Description: Build string for onModuleUnloaded event
// Author:      Gilad Yarnitzky
// Date:        20/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onExceptionString(const apExceptionEvent& exceptionEvent, gtString& messageString)
{
    // If this is a second chance exception:
    bool isSecondChanceException = exceptionEvent.isSecondChance();

    if (isSecondChanceException)
    {
        // Second chance exceptions exist on Windows only:
        messageString = GD_STR_ProcessEventsViewSecondChanceException;
    }
    else
    {
        // The terminology is:
        // - Windows - exception
        // - Linux / Mac - signal
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        messageString = GD_STR_ProcessEventsViewException;
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
        messageString = GD_STR_ProcessEventsViewSignal;
#endif

        // Add the reason as a string:
        osExceptionReason exceptionReason = exceptionEvent.exceptionReason();
        gtString exceptionReasonString;
        osExceptionReasonToString(exceptionReason, exceptionReasonString);
        messageString += exceptionReasonString.asCharArray();
    }
}


// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onOutputDebugString
// Description: Build string for onOutputDebugStrinStringg event
// Author:      Gilad Yarnitzky
// Date:        20/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onOutputDebugString(const apOutputDebugStringEvent& outputDebugStringEvent, gtString& messageString)
{
    // build the string only if it is for the event view:
    if (outputDebugStringEvent.targetView() == apOutputDebugStringEvent::AP_EVENT_VIEW)
    {
        // Get the outputted debug string:
        const gtString& debugString = outputDebugStringEvent.debugString();

        messageString += GD_STR_ProcessEventsViewOutputDebugString;

        // Insert the debug string into this view list:
        messageString += debugString.asCharArray();
    }
}


// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onOutputGDBString
// Description: Build string for onOutputGDB event
// Author:      Gilad Yarnitzky
// Date:        20/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onOutputGDBString(const apGDBOutputStringEvent& outputGDBStringEvent, gtString& messageString)
{
    // Get the outputted debug string:
    const gtString& gdbOutputString = outputGDBStringEvent.gdbOutputString();

    messageString += GD_STR_ProcessEventsViewGDBString;

    // Insert the debug string into this view list:
    messageString += gdbOutputString.asCharArray();
}


// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onGDBErrorString
// Description: Build string for onGDBError event
// Author:      Gilad Yarnitzky
// Date:        20/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onGDBErrorString(const apGDBErrorEvent& gdbErrorEvent, gtString& messageString)
{
    messageString = GD_STR_ProcessEventsViewGDBError;

    // Get the outputted debug string:
    const gtString& gdbErrorString = gdbErrorEvent.gdbErrorString();

    // Insert the debug string into this view list:
    messageString += gdbErrorString.asCharArray();
}


// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onDebuggedProceessOutputString
// Description: Build string for onDebuggedProceessOutputString event
// Author:      Gilad Yarnitzky
// Date:        20/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onDebuggedProceessOutputString(const apDebuggedProcessOutputStringEvent& outputStringEvent, gtString& messageString)
{
    // Get the outputted string:
    const gtString& outputString = outputStringEvent.debuggedProcessOutputString();

    // Insert the debug string into this view list:
    messageString = GD_STR_ProcessEventsViewOutputString;
    messageString += outputString;
}


// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onBreakHitString
// Description: Build string for onBreakHit event
// Author:      Gilad Yarnitzky
// Date:        20/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onBreakHitString(const apBreakpointHitEvent& breakpointEvent, gtString& messageString)
{
    gtString funcName;
    gtString funcArgs;

    // Get the break reason:
    apBreakReason breakReason = breakpointEvent.breakReason();

    if ((AP_HOST_BREAKPOINT_HIT != breakReason) && (!gaIsHostBreakPoint()))
    {
        // Get the breakpoint function name and arguments
        bool rc = gdGetCurrentBreakpointFunction(breakpointEvent.breakedOnFunctionCall(), funcName, funcArgs);
        GT_ASSERT(rc);
    }

    switch (breakReason)
    {
        case AP_MONITORED_FUNCTION_BREAKPOINT_HIT:
        {
            // Monitored function break point:
            messageString = GD_STR_ProcessEventsViewBreakpointHit;
            messageString.append(funcName);
        }
        break;

        case AP_KERNEL_SOURCE_CODE_BREAKPOINT_HIT:
        {
            // Kernel source breakpoint:
            messageString = GD_STR_ProcessEventsViewBreakpointHit;

            // Get the currently debugged kernel function name:
            apCLKernel currentlyDebuggedKernel(OA_CL_NULL_HANDLE, 0, OA_CL_NULL_HANDLE, L"");
            bool rcKer = gaGetCurrentlyDebuggedKernelDetails(currentlyDebuggedKernel);
            GT_IF_WITH_ASSERT(rcKer)
            {
                funcName = currentlyDebuggedKernel.kernelFunctionName();

                // Append the function name to the output string:
                messageString.append(funcName);
            }
        }
        break;

        case AP_KERNEL_FUNCTION_NAME_BREAKPOINT_HIT:
        {
            // Kernel source breakpoint:
            messageString = GD_STR_ProcessEventsViewBreakpointHit;

            // Get the currently debugged kernel function name:
            apCLKernel currentlyDebuggedKernel(OA_CL_NULL_HANDLE, 0, OA_CL_NULL_HANDLE, L"");
            bool rcKer = gaGetCurrentlyDebuggedKernelDetails(currentlyDebuggedKernel);
            GT_IF_WITH_ASSERT(rcKer)
            {
                funcName = currentlyDebuggedKernel.kernelFunctionName();

                // Append the function name to the output string:
                messageString.append(funcName);
            }
        }
        break;

        case AP_NEXT_MONITORED_FUNCTION_BREAKPOINT_HIT:
        {
            // "Step" command break point:
            messageString = GD_STR_ProcessEventsViewStepHit;
            messageString.append(funcName);
        }
        break;

        case AP_DRAW_MONITORED_FUNCTION_BREAKPOINT_HIT:
        {
            // "Draw Step" command break point:
            messageString = GD_STR_ProcessEventsViewDrawStepHit;
            messageString.append(funcName);
        }
        break;

        case AP_FRAME_BREAKPOINT_HIT:
        {
            // "Frame Step" command break point:
            messageString = GD_STR_ProcessEventsViewFrameStepHit;
            messageString.append(funcName);
        }
        break;

        case AP_STEP_IN_BREAKPOINT_HIT:
        {
            // "Step in" command break point:
            messageString = GD_STR_ProcessEventsViewStepInHit;
        }
        break;

        case AP_STEP_OVER_BREAKPOINT_HIT:
        {
            // "Step over" command break point:
            messageString = GD_STR_ProcessEventsViewStepOverHit;
        }
        break;

        case AP_STEP_OUT_BREAKPOINT_HIT:
        {
            // "Step out" command break point:
            messageString = GD_STR_ProcessEventsViewStepOutHit;
        }
        break;

        case AP_BREAK_COMMAND_HIT:
        {
            // "Break" command break point:
            messageString = GD_STR_ProcessEventsViewBreakHit;
        }
        break;

        case AP_OPENGL_ERROR_BREAKPOINT_HIT:
        {
            // OpenGL error occurred:
            messageString = GD_STR_ProcessEventsViewOpenGLErrorHit;

            // Get the OpenGL error:
            GLenum openGLError = breakpointEvent.openGLErrorCode();

            if (!funcName.isEmpty())
            {
                messageString.append(funcName).append(L" - ");
            }

            gtString errorAsString;
            gdOpenGLErrorToString(openGLError, errorAsString);
            messageString += errorAsString;
        }
        break;

        case AP_REDUNDANT_STATE_CHANGE_BREAKPOINT_HIT:
        {
            // A redundant state changes break point:
            messageString = GD_STR_ProcessEventsViewRedundantStateChangeHit;
            messageString.append(funcName);
        }
        break;

        case AP_DEPRECATED_FUNCTION_BREAKPOINT_HIT:
        {
            // "Deprecated Function" command break point:
            messageString = GD_STR_ProcessEventsViewDeprecatedFunctionHit;
            messageString.append(funcName);
        }
        break;

        case AP_SOFTWARE_FALLBACK_BREAKPOINT_HIT:
        {
            // A software fallback break point
            messageString = GD_STR_ProcessEventsViewSoftwareFallbackHit;
        }
        break;

        case AP_HOST_BREAKPOINT_HIT:
        {
            // A host break point
            messageString = GD_STR_ProcessEventsViewHostBreakpointHit;
        }
        break;

        case AP_FOREIGN_BREAK_HIT:
        {
            // A "foreign" break point
            messageString = GD_STR_ProcessEventsViewForeignBreakHit;
        }
        break;

        case AP_BEFORE_KERNEL_DEBUGGING_HIT:
        case AP_AFTER_KERNEL_DEBUGGING_HIT:
        {
            // These should not be sent to the user
            GT_ASSERT(false);
        }
        break;

        default:
        {
            // Unexpected break reason:
            GT_ASSERT(false);
        }
        break;
    }
}

// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onMemoryLeakEventString
// Description: Displays a memory leak as a debugged process event
// Author:      Uri Shomroni
// Date:        2/11/2008
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onMemoryLeakEventString(const apMemoryLeakEvent& memoryLeakEvent, gtString& eventString)
{
    // Get the memory leak as string:
    bool rc = memoryLeakEvent.asString(eventString);
    GT_ASSERT(rc);
}

// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onDebuggedProcessErrorEventString
// Description: Build string for onDebuggedProcessErrorEvent event
// Author:      Gilad Yarnitzky
// Date:        20/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onDebuggedProcessErrorEventString(const apDebuggedProcessDetectedErrorEvent& errorEvent, gtString& messageString)
{
    // Get the detected error parameters:
    const apDetectedErrorParameters& detectedErrorParameters = errorEvent.detectedErrorParameters();

    // Get the associated function:
    apMonitoredFunctionId errorAssociatedFunctionId = detectedErrorParameters._detectedErrorAssociatedFunction;

    // Build the displayed string:
    messageString = GD_STR_ProcessEventsViewDebuggedProcessErrorTitle;

    // If there is a function associated with this error code:
    if (errorAssociatedFunctionId != -1)
    {
        // Add the associated function name to the error report string:
        gtString assosiatedErrorFunctionName;
        gaGetMonitoredFunctionName(errorAssociatedFunctionId, assosiatedErrorFunctionName);

        if (!assosiatedErrorFunctionName.isEmpty())
        {
            messageString.appendFormattedString(L": %ls", assosiatedErrorFunctionName.asCharArray());
        }
    }
    else
    {
        // Get the associated error code:
        int errorAssociatedErrorCode = detectedErrorParameters._detectedErrorCode;

        // Translate the error code to string and add it to the event report string:
        gtString assosiatedErrorCodeAsString;
        apDetectedErrorCodeToString((apErrorCode)errorAssociatedErrorCode, assosiatedErrorCodeAsString);

        if (!assosiatedErrorCodeAsString.isEmpty())
        {
            messageString.appendFormattedString(L": %ls", assosiatedErrorCodeAsString.asCharArray());
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onDebuggedProcessThreadCreatedEventString
// Description: Build string for onDebuggedProcessThreadCreatedEvent event
// Author:      Gilad Yarnitzky
// Date:        20/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onDebuggedProcessThreadCreatedEventString(const apThreadCreatedEvent& threadCreatedEvent, gtString& messageString)
{
    // Get the thread ID
    osThreadId threadId = threadCreatedEvent.threadOSId();

    // Translate it to a string:
    gtString threadIdAsStr;
    osThreadIdAsString(threadId, threadIdAsStr);

    // Report the thread creation:
    messageString = GD_STR_ProcessEventsViewThreadCreated;
    messageString += threadIdAsStr.asCharArray();

    // Handle LWP (Linux and Mac only)
    // If we are building on Windows:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        // Do nothing - there is no LWP on windows
    }
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    {
        // Get the LWP ID
        osThreadId lwpId = threadCreatedEvent.lwpOSId();
        gtString lwpReportString;
        lwpReportString.appendFormattedString(GD_STR_ThreadEventsViewOsIdLWP, (unsigned long)lwpId);

        // Append it to the reported string:
        messageString.append(lwpReportString);
    }
#else
    {
#error Unsupported platform...
    }
#endif
}


// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onDebuggedProcessThreadTerminatedEventString
// Description: Build string for onDebuggedProcessThreadTerminatedEvent event
// Author:      Gilad Yarnitzky
// Date:        20/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onDebuggedProcessThreadTerminatedEventString(const apThreadTerminatedEvent& threadTerminatedEvent, gtString& messageString)
{
    // Get the thread ID
    osThreadId threadId = threadTerminatedEvent.threadOSId();

    // Translate it to a string:
    gtString threadIdAsStr;
    osThreadIdAsString(threadId, threadIdAsStr);

    // Report the thread creation:
    messageString = GD_STR_ProcessEventsViewThreadTerminated;
    messageString += threadIdAsStr;
}


// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onAPIConnectionEstablishedEventString
// Description: Build string for onAPIConnectionEstablishedEvent event
// Author:      Gilad Yarnitzky
// Date:        20/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onAPIConnectionEstablishedEventString(const apApiConnectionEstablishedEvent& apiConnectionEstablishedEvent, gtString& messageString)
{
    // Get the established API connection type:
    apAPIConnectionType establishedConnectionType = apiConnectionEstablishedEvent.establishedConnectionType();

    // Display only connections relevant to the user:
    if (establishedConnectionType != AP_INCOMING_EVENTS_API_CONNECTION)
    {
        // Get the established API connection type:
        establishedConnectionType = apiConnectionEstablishedEvent.establishedConnectionType();

        // Translate the event type to a string to be displayed to the user:
        gtString establishedConnectionAsStr;
        apAPIConnectionTypeToString(establishedConnectionType, establishedConnectionAsStr);

        // Report the the API connection event:
        messageString = GD_STR_ProcessEventsViewPropertiesTitleAPIConnectionEstablished;
        messageString.append(L": ");
        messageString.append(establishedConnectionAsStr.asCharArray());
    }
}


// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onAPIConnectionEndedEventString
// Description: Build string for onAPIConnectionEndedEvent event
// Author:      Gilad Yarnitzky
// Date:        20/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onAPIConnectionEndedEventString(const apApiConnectionEndedEvent& apiConnectionEndedEvent, gtString& messageString)
{
    // Get the established API connection type:
    apAPIConnectionType endedConnectionType = apiConnectionEndedEvent.connectionType();

    // Display only events relevant for the user:
    if (endedConnectionType != AP_INCOMING_EVENTS_API_CONNECTION)
    {
        // Get the established API connection type:
        endedConnectionType = apiConnectionEndedEvent.connectionType();

        // Translate the event type to a string to be displayed to the user:
        gtString endedConnectionAsStr;
        apAPIConnectionTypeToString(endedConnectionType, endedConnectionAsStr);

        // Report the the API connection event:
        messageString = GD_STR_ProcessEventsViewPropertiesTitleAPIConnectionEnded;
        messageString.append(L": ");
        messageString.append(endedConnectionAsStr);
    }
}


// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onRenderContextCreatedEventString
// Description: Build string for onRenderContextCreatedEvent event
// Author:      Gilad Yarnitzky
// Date:        20/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onRenderContextCreatedEventString(const apRenderContextCreatedEvent& renderContextCreatedEvent, gtString& messageString)
{
    // Get the created render context id:
    int createdContextId = renderContextCreatedEvent.contextId();

    messageString.appendFormattedString(GD_STR_ProcessEventsRenderContextWasCreated, createdContextId);
}

// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onRenderContextDeletedEventString
// Description: Build string for onRenderContextDeletedEvent event
// Author:      Gilad Yarnitzky
// Date:        20/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onRenderContextDeletedEventString(const apRenderContextDeletedEvent& renderContextDeletedEvent, gtString& messageString)
{
    // Get the deleted render context id:
    int deletedContextId = renderContextDeletedEvent.contextId();

    messageString.appendFormattedString(GD_STR_ProcessEventsRenderContextWasDeleted, deletedContextId);
}


// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onComputeContextCreatedEventString
// Description: Build string for onComputeContextCreatedEvent event
// Author:      Gilad Yarnitzky
// Date:        21/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onComputeContextCreatedEventString(const apComputeContextCreatedEvent& computeContextCreatedEvent, gtString& messageString)
{
    // Get the created render context id:
    int createdContextId = computeContextCreatedEvent.contextId();

    // Report the context creation event:
    messageString.appendFormattedString(GD_STR_ProcessEventsComputeContextWasCreated, createdContextId);
}


// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onComputeContextDeletedEventString
// Description: Build string for onComputeContextDeletedEvent event
// Author:      Gilad Yarnitzky
// Date:        21/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onComputeContextDeletedEventString(const apComputeContextDeletedEvent& computeContextDeletedEvent, gtString& messageString)
{
    // Get the deleted render context id:
    int deletedContextId = computeContextDeletedEvent.contextId();

    messageString.appendFormattedString(GD_STR_ProcessEventsComputeContextWasDeleted, deletedContextId);
}

// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onDebugOutputMessageEventString
// Description: Build string for onDebugOutputMessageEvent event
// Author:      Gilad Yarnitzky
// Date:        21/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onDebugOutputMessageEventString(const apGLDebugOutputMessageEvent& debugOutputMessageEvent, gtString& messageString)
{
    messageString = GD_STR_ProcessEventsViewPropertiesTitleOpenGLDebugOutput;
    messageString.appendFormattedString(L": %ls - %ls, %ls", debugOutputMessageEvent.debugOutputSource().asCharArray(), debugOutputMessageEvent.debugOutputType().asCharArray(), debugOutputMessageEvent.debugOutputMessageContent().asCharArray());

}

// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onOpenCLErrorMessageEventString
// Description: Build string for onOpenCLErrorMessageEvent event
// Author:      Gilad Yarnitzky
// Date:        21/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onOpenCLErrorMessageEventString(const apOpenCLErrorEvent& clErrorEvent, gtString& messageString)
{
    messageString.append(GD_STR_ProcessEventsViewPropertiesTitleOpenCLErrorHit);
    apMonitoredFunctionId funcId = apMonitoredFunctionsAmount;
    const apOpenCLErrorParameters& openCLErrorParameters = clErrorEvent.openCLErrorParameters();
    apFunctionCall* pFunctionCall = openCLErrorParameters._aptrBreakedOnFunctionCall.pointedObject();
    GT_IF_WITH_ASSERT(pFunctionCall != NULL)
    {
        // Get the function id:
        funcId = pFunctionCall->functionId();
        // Get the current function type:
        gtString functionName;
        bool rc2 = gaGetMonitoredFunctionName(funcId, functionName);
        GT_ASSERT(rc2);

        messageString.appendFormattedString(GD_STR_ProcessEventsViewPropertiesInFunctionSuffix, functionName.asCharArray());
    }
}


// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onOpenCLQueueCreatedEventString
// Description: Build string for onOpenCLQueueCreatedEvent event
// Author:      Gilad Yarnitzky
// Date:        21/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onOpenCLQueueCreatedEventString(const apOpenCLQueueCreatedEvent& queueCreatedEvent, gtString& messageString)
{
    messageString.appendFormattedString(GD_STR_ProcessEventsQueueWasCreated, queueCreatedEvent.queueID() + 1, queueCreatedEvent.contextID());
}

// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onOpenCLQueueDeletedEventString
// Description: Build string for onOpenCLQueueDeletedEvent event
// Author:      Gilad Yarnitzky
// Date:        21/2/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onOpenCLQueueDeletedEventString(const apOpenCLQueueDeletedEvent& queueDeletedEvent, gtString& messageString)
{
    messageString.appendFormattedString(GD_STR_ProcessEventsQueueWasDeleted, queueDeletedEvent.queueID() + 1, queueDeletedEvent.contextID());
}

// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onOpenCLProgramCreatedEventString
// Description: Build string for onOpenCLProgramCreatedEvent event
// Author:      Uri Shomroni
// Date:        1/5/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onOpenCLProgramCreatedEventString(const apOpenCLProgramCreatedEvent& programCreatedEvent, gtString& messageString)
{
    messageString.appendFormattedString(GD_STR_ProcessEventsProgramWasCreated, programCreatedEvent.programIndex() + 1, programCreatedEvent.contextID());
}

// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onOpenCLProgramDeletedEventString
// Description: Build string for onOpenCLProgramDeletedEvent event
// Author:      Uri Shomroni
// Date:        1/5/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onOpenCLProgramDeletedEventString(const apOpenCLProgramDeletedEvent& programDeletedEvent, gtString& messageString)
{
    messageString.appendFormattedString(GD_STR_ProcessEventsProgramWasDeleted, programDeletedEvent.programIndex() + 1, programDeletedEvent.contextID());
}


// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onOpenCLProgramBuildEventString
// Description: Build string for apOpenCLProgramBuildEvent event
// Arguments:   const apOpenCLProgramBuildEvent& programBuildEvent
//              gtString& messageString
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        3/7/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onOpenCLProgramBuildEventString(const apOpenCLProgramBuildEvent& programBuildEvent, gtString& messageString)
{
    if (programBuildEvent.wasBuildEnded())
    {
        // Add a message stating that the build had ended:
        messageString.appendFormattedString(GD_STR_ProcessEventsProgramBuildEnding, programBuildEvent.programName().asCharArray(), programBuildEvent.contextID());
        messageString.append(AF_STR_NewLine);
        messageString.append(AF_STR_Tab);
        messageString.append(AF_STR_Tab);
        messageString.append(AF_STR_Tab);
        messageString.append(GD_STR_ProcessEventsProgramBuildLog);

        // Add a new line before the build log:
        messageString.append(AF_STR_NewLine);

        // Get the program build data:
        gtVector<apCLProgram::programBuildData> programBuildData = programBuildEvent.devicesBuildData();

        // Get the length of the build data vector:
        int sizeOfBuildData = (int)programBuildData.size();

        // Iterate the devices and add the build log:
        for (int i = 0; i < sizeOfBuildData; i++)
        {
            // Get the current build data:
            apCLProgram::programBuildData& currentBuildData = programBuildData[i];

            // Append the current build log:
            messageString.append(currentBuildData._buildLog);

            if (i != sizeOfBuildData)
            {
                messageString.append(AF_STR_NewLine);
            }
        }
    }
    else
    {
        // Add a message stating that the build is starting:
        messageString.appendFormattedString(GD_STR_ProcessEventsProgramBuildStarting, programBuildEvent.programName().asCharArray(), programBuildEvent.contextID());
    }
}

// ---------------------------------------------------------------------------
// Name:        gdEventStringBuilder::onOpenCLProgramBuildFailedWithDebugFlagsEventString
// Description: Builds a string for apOpenCLProgramBuildFailedWithDebugFlagsEvent events
// Author:      Uri Shomroni
// Date:        14/11/2011
// ---------------------------------------------------------------------------
void gdEventStringBuilder::onOpenCLProgramBuildFailedWithDebugFlagsEventString(const apOpenCLProgramBuildFailedWithDebugFlagsEvent& programBuildFailedEvent, gtString& messageString)
{
    // Start the message:
    int contextId = programBuildFailedEvent.contextIndex();
    int programId = programBuildFailedEvent.programIndex();

    if ((contextId != -1) && (programId != -1))
    {
        messageString.appendFormattedString(GD_STR_ProcessEventsProgramBuildFailedWithDebugFlags, programId, contextId);
    }
    else // (contextId == -1) || (programId == -1)
    {
        messageString.append(GD_STR_ProcessEventsProgramBuildFailedWithDebugFlagsNoIndices);
    }

    // Add the error code:
    gtString errorStr;
    gdOpenCLErrorToString((int)programBuildFailedEvent.buildErrorCode(), errorStr);
    messageString.append(errorStr);
}

