//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdGDBCommandInfo.h
///
//==================================================================================

//------------------------------ pdGDBCommandInfo.h ------------------------------

#ifndef __PDGDCCOMMANDINFO_H
#define __PDGDCCOMMANDINFO_H

// Enumerates the supported GDB commands:
enum pdGDBCommandId
{
    PD_GDB_NULL_CMD,                        // A NULL command - a command that does nothing.
    PD_GDB_SET_DEBUGGED_PROGRAM_CMD,        // Set debugged program command.
    PD_GDB_SET_ABI_CMD,                     // Set the OS ABI (Available only in Mac)
    PD_GDB_SET_ARCHITECTURE_CMD,            // Set the executable architecture (Used only in Mac)
    PD_GDB_START_DEBUGGED_PROCESS_CMD,      // Start debugging the debugged program, but break before the main procedure.
    PD_GDB_SUSPEND_DEBUGGED_PROCESS_CMD,    // Suspend the debugged program.
    PD_GDB_STOP_THREAD_SYNC,                // Synchronous thread stop
    PD_GDB_RESUME_DEBUGGED_PROCESS_CMD,     // Resume the debugged program.
    PD_GDB_RUN_DEBUGGED_PROCESS_CMD,        // Launch / resume the run of the debugged program.
    PD_GDB_ABORT_DEBUGGED_PROCESS_CMD,      // Abort (kill) the debugged process.
    PD_GDB_WAIT_FOR_PROCESS_CMD,            // Waits for an attaches to a process of a given name.
    PD_SET_COMMAND_LINE_ARGS_CMD,           // Set the debugged process command line arguments.
    PD_SET_WORK_DIR_CMD,                    // Set the debugged process working directory.
    PD_SET_ENV_VARIABLE_CMD,                // Set the value of a debugged process environment variable.
    PD_SET_GDB_VARIABLE_CMD,                // Set the value of a GDB debugger self variable.
    PD_CONTINUE_CMD,                        // Continue the debugged process execution.
    PD_CONTINUE_THREAD_CMD,                 // Start only one thread in non-stop async mode. Difference with PD_CONTINUE_CMD: synchronious command
    PD_GET_EXECUTABLE_PID_CMD,              // Get the debugged process pid.
    PD_GET_THREADS_INFO_CMD,                // Get debugged process threads information.
    PD_GET_THREADS_INFO_VIA_MI_CMD,         // Get debugged process threads information via the machine interface (Available only in Mac)
    PD_GET_THREAD_INFO_CMD,                 // Get information for a specific debugged process thread.
    PD_GET_CUR_THREAD_CALL_STACK_CMD,       // Get current thread's call stack.
    PD_SET_ACTIVE_THREAD_CMD,               // Set GDB's active (and displayed) thread.
    PD_SET_ACTIVE_THREAD_ASYNC_CMD,         // Async thread select
    PD_SET_ACTIVE_FRAME_CMD,                // Set GDB's active frame
    PD_GET_LOCALS_INFO_CMD,                 // Get locals info
    PD_GET_LOCAL_VARIABLE_CMD,              // Get local variable value
    PD_GET_VARIABLE_TYPE_CMD,               // Get requested variable type
    PD_GDB_SET_BREAKPOINT_CMD,              // Set breakpoint on specified file and line number or function name
    PD_GDB_DELETE_BREAKPOINT_CMD,           // Delete host breakpoint on specified line number or function name
    PD_GDB_STEP_INTO_CMD,                   // Step into execution command
    PD_GDB_STEP_OUT_CMD,                    // Step out excecution command
    PD_GDB_STEP_OVER_CMD,                   // Step over execution command
    PD_GDB_UNTIL_CMD,                       // Step until file name and line number
    PD_SUSPEND_THREAD_CMD,                  // Suspend a thread (Available only in Mac)
    PD_RESUME_THREAD_CMD,                   // Resume a thread (Available only in Mac)
    PD_GET_SYMBOL_AT_ADDRESS,               // Get the name of a symbol that resides at a given address.
    PD_GET_DEBUG_INFO_AT_ADDRESS,           // Get the debug information (source code file and line) at a given address.
    PD_GET_LIBRARY_AT_ADDRESS,              // Get the name of the shared library at a given address.
    PD_EXECUTE_FUNCTION_CMD,                // Execute a given debugged process function.
    PD_TEST_CMD,                            // Used for developers testings.
    PD_EXIT_GDB_CMD,                        // Exit GDB.
    PD_LAST_GDB_CMD_INDEX
};


// Enums GDB commands types:
enum pdGDBCommandType
{
    PD_GDB_SYNCHRONOUS_CMD,     // Synchronous command - a command that returns
    // a result immediately.
    PD_GDB_ASYNCHRONOUS_CMD,     // A-synchronous command - a command that does
    // not return a result immediately.
    PD_GDB_ASYNCHRONOUS_NO_ANSWER_CMD // // A-synchronous command - a command that does
    // not return a result immediately. And answer waiting by GDBListenerThread
};


// ----------------------------------------------------------------------------------
// Struct Name:          pdGDBCommandInfo
// General Description:
//   Contains GDB commands information.
//
// Author:               Yaki Tebeka
// Creation Date:        20/12/2006
// ----------------------------------------------------------------------------------
struct pdGDBCommandInfo
{
    // The GDB command that this struct represents:
    pdGDBCommandId _commandId;

    // The command type:
    pdGDBCommandType _commandType;

    // The GDB MI2 interface command line string that executes this command:
    const char* _commandExecutionString;
};

const pdGDBCommandInfo* pdGetGDBCommandInfo(pdGDBCommandId gdbCommand);
bool pdDoesCommandRequireFlush(pdGDBCommandId gdbCommand);

#endif  // __PDGDCCOMMANDINFO_H
