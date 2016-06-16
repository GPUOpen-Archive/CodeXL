//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file rdDebuggerCommandExecutor.cpp
///
//==================================================================================

//------------------------------ rdDebuggerCommandExecutor.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTAPIClasses/Include/apDebugProjectSettings.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessCreationFailureEvent.h>
#include <AMDTAPIClasses/Include/Events/apEventsHandler.h>
#include <AMDTAPIClasses/Include/Events/apThreadCreatedEvent.h>
#include <AMDTProcessDebugger/Include/pdProcessDebugger.h>
#include <AMDTProcessDebugger/Include/pdProcessDebuggersManager.h>
#include <AMDTProcessDebugger/Include/pdRemoteProcessDebuggerCommandId.h>

// Local:
#include <src/rdDebuggerCommandExecutor.h>


// ---------------------------------------------------------------------------
// Name:        rdDebuggerCommandExecutor::rdDebuggerCommandExecutor
// Description: Constructor
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
rdDebuggerCommandExecutor::rdDebuggerCommandExecutor(osChannel& processDebuggerConnectionChannel)
    : _processDebuggerConnectionChannel(processDebuggerConnectionChannel), _continueLoop(true)
      // In the Windows 32-bit remote debugging server, this "member" variable may change at runtime:
#if !((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) && (AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE))
    , _theProcessDebugger(pdProcessDebugger::instance())
#endif
{

}

// ---------------------------------------------------------------------------
// Name:        rdDebuggerCommandExecutor::~rdDebuggerCommandExecutor
// Description: Destructor
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
rdDebuggerCommandExecutor::~rdDebuggerCommandExecutor()
{

}

// ---------------------------------------------------------------------------
// Name:        rdDebuggerCommandExecutor::listenToDebuggingCommands
// Description: Runs the main loop for this server - reading debugging commands
//              and passing them on to the process debugger
// Author:      Uri Shomroni
// Date:        11/8/2009
// ---------------------------------------------------------------------------
void rdDebuggerCommandExecutor::listenToDebuggingCommands()
{
    while (_continueLoop)
    {
        // If there is a pipe error, stop running the loop:
        gtInt32 funcId = -1;
        static const gtSize_t sizeOfInt32 = sizeof(gtInt32);
        bool rcRead = _processDebuggerConnectionChannel.read((gtByte*)&funcId, sizeOfInt32);

        GT_IF_WITH_ASSERT(rcRead)
        {
            // If we failed at some command, stop running
            _continueLoop = handleDebuggingCommand((pdRemoteProcessDebuggerCommandId)funcId);
        }
        else
        {
            // The pipe was closed, exit:
            _continueLoop = false;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        rdDebuggerCommandExecutor::handleDebuggingCommand
// Description: Handle debugging commands (see pdProcessDebugger.h and pdRemoteProcessDebugger.cpp
//              by calling the real process debugger and returning any results.
// Arguments: funcId - which process debugger commands do we need to use?
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        12/8/2009
// ---------------------------------------------------------------------------
bool rdDebuggerCommandExecutor::handleDebuggingCommand(pdRemoteProcessDebuggerCommandId cmdId)
{
    bool handledCommand = true;

    // In the Windows 32-bit remote debugging server, this "member" variable may change at runtime:
#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) && (AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE))
    pdProcessDebugger& _theProcessDebugger = pdProcessDebugger::instance();
#endif

    // Uri, 3/7/2012: Do not perform most commands unless the process exists:
    bool executeDebuggingCommand = _theProcessDebugger.debuggedProcessExists() ||
                                   (PD_INITIALIZE_PROCESS_DEBUGGER_CMD == cmdId) ||
                                   (PD_LAUNCH_DEBUGGED_PROCESS_CMD == cmdId) ||
                                   (PD_LAUNCH_DEBUGGED_PROCESS_IN_SUSPENDED_MODE_CMD == cmdId) ||
                                   (PD_CONTINUE_DEBUGGED_PROCESS_FROM_SUSPENDED_CREATION_CMD == cmdId) ||
                                   (PD_DEBUGGED_PROCESS_EXISTS_CMD == cmdId) ||
                                   (PD_TERMINATE_DEBUGGED_PROCESS_CMD == cmdId) ||
                                   (PD_IS_DEBUGGING_64_BIT_APPLICATION_CMD == cmdId) ||
                                   (PD_DEBUGGED_PROCESS_ID_CMD == cmdId) ||
                                   (PD_HANDLE_DEBUG_EVENT == cmdId) ||
                                   (PD_REMOTE_CAN_PERFORM_HOST_DEBUGGING == cmdId) ||
                                   (PD_IS_SPY_API_THREAD_RUNNING_CMD == cmdId) ||
                                   (PD_REMOTE_SET_HOST_BP_CMD == cmdId) ||
                                   (PD_REMOTE_DELETE_HOST_BP_CMD == cmdId) ||
                                   (PD_REMOTE_IS_API_OR_KERNEL_BP_CMD == cmdId);

    gtString debugString;
    debugString.appendFormattedString(executeDebuggingCommand ? L"Handling debugging Command %d" : L"Ignoring debugging Command %d", cmdId);
    OS_OUTPUT_DEBUG_LOG(debugString.asCharArray(), OS_DEBUG_LOG_DEBUG);

    if (executeDebuggingCommand)
    {
        switch (cmdId)
        {
            case PD_INITIALIZE_PROCESS_DEBUGGER_CMD:
            {
                apDebugProjectSettings processCreationData;
                processCreationData.readSelfFromChannel(_processDebuggerConnectionChannel);
                bool retVal = processDebugProjectSettingsForLocalDebugger(processCreationData);

                if (retVal)
                {
                    // Get the instance again, since it may have changed by selecting a new process debugger:
                    retVal = pdProcessDebugger::instance().initializeDebugger(processCreationData);
                }

                // Send the return value:
                _processDebuggerConnectionChannel << retVal;

                if (retVal)
                {
                    processCreationData.writeSelfIntoChannel(_processDebuggerConnectionChannel);
                }
            }
            break;

            case PD_LAUNCH_DEBUGGED_PROCESS_CMD:
            {
                bool retVal = pdProcessDebugger::instance().launchDebuggedProcess();
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_LAUNCH_DEBUGGED_PROCESS_IN_SUSPENDED_MODE_CMD:
            {
                bool retVal = pdProcessDebugger::instance().launchDebuggedProcessInSuspendedMode();
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_CONTINUE_DEBUGGED_PROCESS_FROM_SUSPENDED_CREATION_CMD:
            {
                bool retVal = _theProcessDebugger.continueDebuggedProcessFromSuspendedCreation();
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_DEBUGGED_PROCESS_EXISTS_CMD:
            {
                bool retVal = _theProcessDebugger.debuggedProcessExists();
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_TERMINATE_DEBUGGED_PROCESS_CMD:
            {
                bool retVal = _theProcessDebugger.terminateDebuggedProcess();
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_IS_DEBUGGING_64_BIT_APPLICATION_CMD:
            {
                bool is64Bit = false;
                bool retVal = _theProcessDebugger.isDebugging64BitApplication(is64Bit);
                _processDebuggerConnectionChannel << retVal;

                if (retVal)
                {
                    _processDebuggerConnectionChannel << is64Bit;
                }
            }
            break;

            case PD_AMOUNT_OF_DEBUGGED_PROCESS_THREADS_CMD:
            {
                int retVal = _theProcessDebugger.amountOfDebuggedProcessThreads();
                _processDebuggerConnectionChannel << (gtInt32)retVal;
            }
            break;

            case PD_GET_THREAD_ID_CMD:
            {
                gtInt32 threadIndexAsInt32 = -1;
                _processDebuggerConnectionChannel >> threadIndexAsInt32;
                osThreadId threadId = OS_NO_THREAD_ID;
                bool retVal = _theProcessDebugger.getThreadId((int)threadIndexAsInt32, threadId);
                _processDebuggerConnectionChannel << retVal;

                if (retVal)
                {
                    _processDebuggerConnectionChannel << (gtUInt64)threadId;
                }
            }
            break;

            case PD_SET_SPY_API_THREAD_ID_CMD:
            {
                gtUInt64 spyAPIThreadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
                _processDebuggerConnectionChannel >> spyAPIThreadIdAsUInt64;
                _theProcessDebugger.setSpiesAPIThreadId((osThreadId)spyAPIThreadIdAsUInt64);
            }
            break;

            case PD_SPIES_API_THREAD_INDEX_CMD:
            {
                int retVal = _theProcessDebugger.spiesAPIThreadIndex();
                _processDebuggerConnectionChannel << (gtInt32)retVal;
            }
            break;

            case PD_MAIN_THREAD_ID_CMD:
            {
                osThreadId retVal = _theProcessDebugger.mainThreadId();
                _processDebuggerConnectionChannel << (gtUInt64)retVal;
            }
            break;

            case PD_SPIES_API_THREAD_ID_CMD:
            {
                osThreadId retVal = _theProcessDebugger.spiesAPIThreadId();
                _processDebuggerConnectionChannel << (gtUInt64)retVal;
            }
            break;

            case PD_DEBUGGED_PROCESS_ID_CMD:
            {
                osProcessId retVal = _theProcessDebugger.debuggedProcessId();
                _processDebuggerConnectionChannel << (gtUInt64)retVal;
            }
            break;

            case PD_IS_SPY_API_THREAD_RUNNING_CMD:
            {
                bool retVal = _theProcessDebugger.isSpiesAPIThreadRunning();
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_SUSPEND_DEBUGGED_PROCESS_CMD:
            {
                bool retVal = _theProcessDebugger.suspendDebuggedProcess();
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_RESUME_DEBUGGED_PROCESS_CMD:
            {
                bool retVal = _theProcessDebugger.resumeDebuggedProcess();
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_IS_DEBUGGED_PROCESS_SUSPENDED_CMD:
            {
                bool retVal = _theProcessDebugger.isDebuggedProcssSuspended();
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_SUSPEND_DEBUGGED_PROCESS_THREAD_CMD:
            {
                gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
                _processDebuggerConnectionChannel >> threadIdAsUInt64;
                bool retVal = _theProcessDebugger.suspendDebuggedProcessThread((osThreadId)threadIdAsUInt64);
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_RESUME_DEBUGGED_PROCESS_THREAD_CMD:
            {
                gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
                _processDebuggerConnectionChannel >> threadIdAsUInt64;
                bool retVal = _theProcessDebugger.resumeDebuggedProcessThread((osThreadId)threadIdAsUInt64);
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_GET_DEBUGGED_THREAD_CALL_STACK_CMD:
            {
                gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
                _processDebuggerConnectionChannel >> threadIdAsUInt64;
                bool hideSpyDLLsFunctions = false;
                _processDebuggerConnectionChannel >> hideSpyDLLsFunctions;
                osCallStack callStack;
                bool retVal = _theProcessDebugger.getDebuggedThreadCallStack((osThreadId)threadIdAsUInt64, callStack, hideSpyDLLsFunctions);
                _processDebuggerConnectionChannel << retVal;

                if (retVal)
                {
                    callStack.writeSelfIntoChannel(_processDebuggerConnectionChannel);
                }
            }
            break;

            case PD_FILL_CALL_STACK_DEBUG_INFO_CMD:
            {
                osCallStack callsStack;
                bool hideSpyDLLsFunctions = false;
                callsStack.readSelfFromChannel(_processDebuggerConnectionChannel);
                _processDebuggerConnectionChannel >> hideSpyDLLsFunctions;
                _theProcessDebugger.fillCallsStackDebugInfo(callsStack, hideSpyDLLsFunctions);
                callsStack.writeSelfIntoChannel(_processDebuggerConnectionChannel);
            }
            break;

            case PD_FILL_THERAD_CREATED_EVENT_CMD:
            {
                apThreadCreatedEvent eve(OS_NO_THREAD_ID, OS_NO_THREAD_ID, osTime(), NULL);
                eve.readSelfFromChannel(_processDebuggerConnectionChannel);
                _theProcessDebugger.fillThreadCreatedEvent(eve);
                eve.writeSelfIntoChannel(_processDebuggerConnectionChannel);
            }
            break;

            case PD_CAN_GET_CALL_STACKS_CMD:
            {
                bool retVal = _theProcessDebugger.canGetCallStacks();
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_CAN_MAKE_THREAD_EXECUTE_FUNCTION_CMD:
            {
                gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
                _processDebuggerConnectionChannel >> threadIdAsUInt64;
                bool retVal = _theProcessDebugger.canMakeThreadExecuteFunction((osThreadId)threadIdAsUInt64);
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_MAKE_THREAD_EXECUTE_FUNCTION_CMD:
            {
                gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
                _processDebuggerConnectionChannel >> threadIdAsUInt64;
                gtUInt64 funcAddressAsUInt64 = 0;
                _processDebuggerConnectionChannel >> funcAddressAsUInt64;
                bool retVal = _theProcessDebugger.makeThreadExecuteFunction((osThreadId)threadIdAsUInt64, (osProcedureAddress64)funcAddressAsUInt64);
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_FUNCTION_EXECUTION_MODE_CMD:
            {
                pdProcessDebugger::FunctionExecutionMode retVal = _theProcessDebugger.functionExecutionMode();
                _processDebuggerConnectionChannel << (gtUInt32)retVal;
            }
            break;

            case PD_AFTER_API_CALL_ISSUED_CMD:
            {
                _theProcessDebugger.afterAPICallIssued();
            }
            break;

            case PD_REMOTE_TO_LOCAL_FILE_PATH_CMD:
            {
                bool useCache = false;
                _processDebuggerConnectionChannel >> useCache;
                osFilePath pathToLocalize;
                pathToLocalize.readSelfFromChannel(_processDebuggerConnectionChannel);
                _theProcessDebugger.remoteToLocalFilePath(pathToLocalize, useCache);
                pathToLocalize.writeSelfIntoChannel(_processDebuggerConnectionChannel);
            }
            break;

            case PD_HANDLE_DEBUG_EVENT:
            {
                bool retVal = false;

                // This should only be used to handle relevant events that went through the Events pipe:
                gtAutoPtr<osTransferableObject> aptrEventAsTransferableObject;
                _processDebuggerConnectionChannel >> aptrEventAsTransferableObject;

                // Make sure we got a transferableObject and it is an event:
                GT_IF_WITH_ASSERT(aptrEventAsTransferableObject.pointedObject() != NULL)
                {
                    GT_IF_WITH_ASSERT(aptrEventAsTransferableObject->isEventObject())
                    {
                        apEvent* pEve = (apEvent*)aptrEventAsTransferableObject.pointedObject();

                        // Register the event:
                        if (pEve != NULL)
                        {
                            retVal = true;

                            // We do not expect events passed this way to be vetoed:
                            bool vetoEvent = false;
                            _theProcessDebugger.onEventRegistration(*pEve, vetoEvent);
                            _theProcessDebugger.onEvent(*pEve, vetoEvent);
                            GT_ASSERT(!vetoEvent);
                        }
                    }
                }

                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_REMOTE_CAN_GET_HOST_VARIABLES_CMD:
            {
                bool retVal = _theProcessDebugger.canGetHostVariables();
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_REMOTE_GET_HOST_LOCALS_CMD:
            {
                gtUInt64 threadId = (gtUInt64)OS_NO_THREAD_ID;
                gtInt32 callStackFrameIndex = -1;
                gtVector<gtString> locals;

                _processDebuggerConnectionChannel >> threadId;
                _processDebuggerConnectionChannel >> callStackFrameIndex;

                bool retVal = _theProcessDebugger.getHostLocals((osThreadId)threadId, (int)callStackFrameIndex, locals);

                _processDebuggerConnectionChannel << retVal;

                if (retVal)
                {
                    gtInt32 numLocals = (gtInt32)locals.size();
                    _processDebuggerConnectionChannel << numLocals;

                    for (gtInt32 i = 0; i < numLocals; i++)
                    {
                        _processDebuggerConnectionChannel << locals[i];
                    }
                }
            }
            break;

            case PD_REMOTE_GET_HOST_VARIABLE_VALUE_CMD:
            {
                gtUInt64 threadId = (gtUInt64)OS_NO_THREAD_ID;
                gtInt32 callStackFrameIndex = -1;
                gtString variableName;
                gtString o_varValue;
                gtString o_varValueHex;
                gtString o_varType;

                _processDebuggerConnectionChannel >> threadId;
                _processDebuggerConnectionChannel >> callStackFrameIndex;
                _processDebuggerConnectionChannel >> variableName;

                bool retVal = _theProcessDebugger.getHostVariableValue((osThreadId)threadId, (gtInt32)callStackFrameIndex, variableName, o_varValue, o_varValueHex, o_varType);

                _processDebuggerConnectionChannel << retVal;

                if (retVal)
                {
                    _processDebuggerConnectionChannel << o_varValue;
                    _processDebuggerConnectionChannel << o_varValueHex;
                    _processDebuggerConnectionChannel << o_varType;
                }
            }
            break;

            case PD_REMOTE_CAN_PERFORM_HOST_DEBUGGING:
            {
                bool retVal = _theProcessDebugger.canPerformHostDebugging();
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_REMOTE_IS_API_OR_KERNEL_BP_CMD:
            {
                gtUInt64 threadId = (gtUInt64)OS_NO_THREAD_ID;
                _processDebuggerConnectionChannel >> threadId;

                bool retVal = _theProcessDebugger.isAtAPIOrKernelBreakpoint((osThreadId)threadId);
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_REMOTE_HOST_BREAK_REASON_CMD:
            {
                apBreakReason retVal = _theProcessDebugger.hostBreakReason();
                _processDebuggerConnectionChannel << (gtInt32)retVal;
            }
            break;

            case PD_REMOTE_HOST_BREAKPOINT_LOCATION_CMD:
            {
                osFilePath bpFile;
                int bpLine = -1;

                bool retVal = _theProcessDebugger.getHostBreakpointLocation(bpFile, bpLine);

                _processDebuggerConnectionChannel << retVal;

                if (retVal)
                {
                    bpFile.writeSelfIntoChannel(_processDebuggerConnectionChannel);
                    _processDebuggerConnectionChannel << (gtInt32)bpLine;
                }
            }
            break;

            case PD_REMOTE_SET_HOST_BP_CMD:
            {
                osFilePath fileName;
                gtInt32 lineNumber = -1;

                fileName.readSelfFromChannel(_processDebuggerConnectionChannel);
                _processDebuggerConnectionChannel >> lineNumber;

                bool retVal = _theProcessDebugger.setHostSourceBreakpoint(fileName, (int)lineNumber);
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_REMOTE_DELETE_HOST_BP_CMD:
            {
                osFilePath fileName;
                gtInt32 lineNumber = -1;

                fileName.readSelfFromChannel(_processDebuggerConnectionChannel);
                _processDebuggerConnectionChannel >> lineNumber;

                bool retVal = _theProcessDebugger.deleteHostSourceBreakpoint(fileName, (int)lineNumber);
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_REMOTE_PERFORM_HOST_STEP_CMD:
            {
                gtUInt64 threadId = (gtUInt64)OS_NO_THREAD_ID;
                gtInt32 stepType = (gtInt32)pdProcessDebugger::PD_STEP_OVER;

                _processDebuggerConnectionChannel >> threadId;
                _processDebuggerConnectionChannel >> stepType;

                bool retVal = _theProcessDebugger.performHostStep((osThreadId)threadId, (pdProcessDebugger::StepType)stepType);
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_REMOTE_SUSPEND_HOST_DEBUGGED_PROCESS:
            {
                bool retVal = _theProcessDebugger.suspendHostDebuggedProcess();
                _processDebuggerConnectionChannel << retVal;
            }
            break;

            case PD_REMOTE_GET_BP_TRIGGERING_THREAD_INDEX_CMD:
            {
                int index = -1;
                bool retVal = _theProcessDebugger.getBreakpointTriggeringThreadIndex(index);

                _processDebuggerConnectionChannel << retVal;

                if (retVal)
                {
                    _processDebuggerConnectionChannel << (gtInt32)index;
                }
            }
            break;

            default:
            {
                // Unidentified command!
                handledCommand = false;
                GT_ASSERT(handledCommand);
            }
            break;
        }

    }
    else // !executeDebuggingCommand
    {
        GT_ASSERT(executeDebuggingCommand);
        handledCommand = false;
    }

    return handledCommand;
}

// ---------------------------------------------------------------------------
// Name:        rdDebuggerCommandExecutor::processDebugProjectSettingsForLocalDebugger
// Description: Performs any needed processing on the debug project settings to
//              make it appropriate for the local debugging environment
// Author:      Uri Shomroni
// Date:        29/8/2013
// ---------------------------------------------------------------------------
bool rdDebuggerCommandExecutor::processDebugProjectSettingsForLocalDebugger(apDebugProjectSettings& debugProjectSettings)
{
    bool retVal = true;

    // Error message for failure events:
    gtString errMsg = L"The remote debugging server could not launch the process";

    // Check if the file even exists (and abort if it doesn't):
    const osFilePath& executablePth = debugProjectSettings.executablePath();
    retVal = executablePth.exists();

    // Check if we are a remote target (i.e. if we have a remote host):
    bool isRemoteHost = debugProjectSettings.isRemoteTarget();

    if (retVal)
    {
        // If we are, there are some adjustments needed to the debug project settings:
        if (isRemoteHost)
        {
            // Remove the remote target information:
            debugProjectSettings.setLocalDebugging();

            // Set the log files directory:
            osFilePath logFilesBaseFolder(osFilePath::OS_TEMP_DIRECTORY);
            osDirectory logFilesBaseFolderDir(logFilesBaseFolder);
            debugProjectSettings.setLogFilesFolder(logFilesBaseFolderDir);

            // Get this process's installation folder:
            osFilePath remoteDebuggingServerFolderPath;
            bool rcPth = osGetCurrentApplicationPath(remoteDebuggingServerFolderPath);
            GT_IF_WITH_ASSERT(rcPth)
            {
                remoteDebuggingServerFolderPath.clearFileExtension().clearFileName().reinterpretAsDirectory();

                // Use it to set the debugger directory:
                debugProjectSettings.setDebuggerInstallDir(remoteDebuggingServerFolderPath);

                // Spies directory. Currently only Windows 64-bit debugging uses a specifically different spies directory:
                osFilePath spiesDirectory = remoteDebuggingServerFolderPath;
                gtString spiesSubdirectory = OS_SPIES_SUB_DIR_NAME;

                // Get the server and executable's bitness:
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
                bool is64BitRemoteDebuggingServer = false;
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
                bool is64BitRemoteDebuggingServer = true;
#else
#error Unknown address space depth!
#endif

                // Check if we're trying to debug a 64-bit app:
                bool is64BitApplication = is64BitRemoteDebuggingServer;
                bool rc64 = osIs64BitModule(executablePth, is64BitApplication);

                // This is not an "if-with-assert", since we want to assume by default the debugged application is the same
                // as the remote debugging server:
                GT_ASSERT(rc64);

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
                {
                    // Set the spies folder name:
                    if (is64BitApplication)
                    {
                        spiesSubdirectory = OS_SPIES_64_SUB_DIR_NAME;
                    }

                    // If the processes don't match, update the process debugger type:
                    if (is64BitApplication != is64BitRemoteDebuggingServer)
                    {
                        pdProcessDebuggersManager::instance().adjustProcessDebuggerToProcessCreationData(debugProjectSettings);
                    }
                }
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
                {
                    // We do not support cross-bitness debugging in Linux:
                    if (is64BitApplication != is64BitRemoteDebuggingServer)
                    {
                        // Send a failure event:
                        errMsg.appendFormattedString(L"The application selected for debug is a %d-bit application.\nThis version of CodeXL only supports debugging %d-bit applications.",
                                                     is64BitApplication ? 64 : 32, is64BitRemoteDebuggingServer ? 64 : 32);

                        // Also report the failure to the remote process debugger:
                        retVal = false;
                    }
                }
#else
#error Unknown build target!
#endif

                // Add the spies subdirectory to the spies directory:
                spiesDirectory.appendSubDirectory(spiesSubdirectory);

                // Set it into the process creation data:
                debugProjectSettings.setSpiesDirectory(spiesDirectory);
                debugProjectSettings.setEnvironmentVariable(OS_STR_envVar_spiesDirectory, spiesDirectory.asString());

                // Force the suppression of unnecessary events, since remote connections are heavy:
                debugProjectSettings.setEnvironmentVariable(OS_STR_envVar_suppressSpyEvents, OS_STR_envVar_valueTrue);
            }
        }
    }
    else // !retVal = !executablePth.exists()
    {
        // The path does not exist on this machine, abort:
        errMsg = L"The executable path does not exist";

        if (isRemoteHost)
        {
            errMsg.append(L" on the target machine");
        }
    }

    // If we failed, send a failure event:
    if (!retVal)
    {
        gtString commandLine = executablePth.asString();
        const gtString& commandArgs = debugProjectSettings.commandLineArguments();

        if (!commandArgs.isEmpty())
        {
            commandLine.append(' ').append(commandArgs);
        }

        apDebuggedProcessCreationFailureEvent failureEve(apDebuggedProcessCreationFailureEvent::COULD_NOT_CREATE_PROCESS,
                                                         commandLine, debugProjectSettings.workDirectory().asString(), errMsg);
        apEventsHandler::instance().registerPendingDebugEvent(failureEve);
    }

    return retVal;
}

