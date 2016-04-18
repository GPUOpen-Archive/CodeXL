//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdGDBCommandInfo.cpp
///
//==================================================================================

//------------------------------ pdGDBCommandInfo.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Local:
#include <src/pdGDBCommandInfo.h>

// Array containing the GDB commands informations.
// Notice - this array must be ordered according to the pdGDBCommands
//          enumeration ordering.
static pdGDBCommandInfo stat_gdbCommandsInfos[] =
{
    { PD_GDB_NULL_CMD, PD_GDB_SYNCHRONOUS_CMD, "echo" },
    { PD_GDB_SET_DEBUGGED_PROGRAM_CMD, PD_GDB_SYNCHRONOUS_CMD, "-file-exec-and-symbols" },
    { PD_GDB_SET_ABI_CMD, PD_GDB_SYNCHRONOUS_CMD, "set osabi" },
    { PD_GDB_SET_ARCHITECTURE_CMD, PD_GDB_SYNCHRONOUS_CMD, "set architecture" },
    { PD_GDB_START_DEBUGGED_PROCESS_CMD, PD_GDB_SYNCHRONOUS_CMD, "start" },
    { PD_GDB_SUSPEND_DEBUGGED_PROCESS_CMD, PD_GDB_ASYNCHRONOUS_NO_ANSWER_CMD, "-exec-interrupt" },
    { PD_GDB_STOP_THREAD_SYNC, PD_GDB_SYNCHRONOUS_CMD, "-exec-interrupt" },
    { PD_GDB_RESUME_DEBUGGED_PROCESS_CMD, PD_GDB_SYNCHRONOUS_CMD, "-exec-continue" },
    // { PD_GDB_RUN_DEBUGGED_PROCESS_CMD, PD_GDB_ASYNCHRONOUS_CMD, "-exec-run" },
    { PD_GDB_RUN_DEBUGGED_PROCESS_CMD, PD_GDB_ASYNCHRONOUS_CMD, "run" },
    { PD_GDB_ABORT_DEBUGGED_PROCESS_CMD, PD_GDB_ASYNCHRONOUS_CMD, "kill" },
    { PD_GDB_WAIT_FOR_PROCESS_CMD, PD_GDB_ASYNCHRONOUS_NO_ANSWER_CMD, "attach -waitfor" },
    { PD_SET_COMMAND_LINE_ARGS_CMD, PD_GDB_SYNCHRONOUS_CMD, "-exec-arguments" },
    { PD_SET_WORK_DIR_CMD, PD_GDB_SYNCHRONOUS_CMD, "cd" },
    { PD_SET_ENV_VARIABLE_CMD, PD_GDB_SYNCHRONOUS_CMD, "set environment" },
    { PD_SET_GDB_VARIABLE_CMD, PD_GDB_SYNCHRONOUS_CMD, "set" },
    { PD_CONTINUE_CMD, PD_GDB_ASYNCHRONOUS_CMD, "-exec-continue" },
    { PD_CONTINUE_THREAD_CMD, PD_GDB_SYNCHRONOUS_CMD, "-exec-continue" },
    { PD_GET_EXECUTABLE_PID_CMD, PD_GDB_SYNCHRONOUS_CMD, "info proc" },
    { PD_GET_THREADS_INFO_CMD, PD_GDB_SYNCHRONOUS_CMD, "info threads" },
    { PD_GET_THREADS_INFO_VIA_MI_CMD, PD_GDB_SYNCHRONOUS_CMD, "-thread-list-ids" },
    { PD_GET_THREAD_INFO_CMD, PD_GDB_SYNCHRONOUS_CMD, "info thread" },
    { PD_GET_CUR_THREAD_CALL_STACK_CMD, PD_GDB_SYNCHRONOUS_CMD, "-stack-list-frames" },
    { PD_SET_ACTIVE_THREAD_CMD, PD_GDB_SYNCHRONOUS_CMD, "-thread-select" },
    { PD_SET_ACTIVE_THREAD_ASYNC_CMD, PD_GDB_SYNCHRONOUS_CMD, "-thread-select" },
    { PD_SET_ACTIVE_FRAME_CMD, PD_GDB_SYNCHRONOUS_CMD, "frame " },
    { PD_GET_LOCALS_INFO_CMD, PD_GDB_SYNCHRONOUS_CMD, "-stack-list-variables " },
    { PD_GET_LOCAL_VARIABLE_CMD, PD_GDB_SYNCHRONOUS_CMD, "-data-evaluate-expression " },
    { PD_GET_VARIABLE_TYPE_CMD, PD_GDB_SYNCHRONOUS_CMD, "whatis " },
    { PD_GDB_SET_BREAKPOINT_CMD, PD_GDB_SYNCHRONOUS_CMD, "-break-insert " },
    { PD_GDB_DELETE_BREAKPOINT_CMD, PD_GDB_SYNCHRONOUS_CMD, "-break-delete " },
    { PD_GDB_STEP_INTO_CMD, PD_GDB_ASYNCHRONOUS_CMD, "-exec-step " },
    { PD_GDB_STEP_OUT_CMD, PD_GDB_ASYNCHRONOUS_CMD, "-exec-finish " },
    { PD_GDB_STEP_OVER_CMD, PD_GDB_ASYNCHRONOUS_CMD, "-exec-next " },
    { PD_GDB_UNTIL_CMD, PD_GDB_SYNCHRONOUS_CMD, "tbreak" },
    { PD_SUSPEND_THREAD_CMD, PD_GDB_SYNCHRONOUS_CMD, "thread suspend" },                // (Available only in Mac)
    { PD_RESUME_THREAD_CMD, PD_GDB_SYNCHRONOUS_CMD, "thread resume" },                  // (Available only in Mac)
    { PD_GET_SYMBOL_AT_ADDRESS, PD_GDB_SYNCHRONOUS_CMD, "info symbol" },
    { PD_GET_DEBUG_INFO_AT_ADDRESS, PD_GDB_SYNCHRONOUS_CMD, "info line" },
    { PD_GET_LIBRARY_AT_ADDRESS, PD_GDB_SYNCHRONOUS_CMD, "info sharedlibrary" },
    { PD_EXECUTE_FUNCTION_CMD, PD_GDB_SYNCHRONOUS_CMD, "call" },
    { PD_TEST_CMD, PD_GDB_SYNCHRONOUS_CMD, "" },
    { PD_EXIT_GDB_CMD, PD_GDB_SYNCHRONOUS_CMD, "-gdb-exit" }
};


// ---------------------------------------------------------------------------
// Name:        pdGetGDBCommandInfo
// Description: Inputs a GDB command id and returns its information structure,
//              or NULL in case of error.
// Author:      Yaki Tebeka
// Date:        17/12/2006
// ---------------------------------------------------------------------------
const pdGDBCommandInfo* pdGetGDBCommandInfo(pdGDBCommandId gdbCommand)
{
    const pdGDBCommandInfo* retVal = NULL;

    // Sanity check:
    GT_IF_WITH_ASSERT((0 <= gdbCommand) && (gdbCommand <= PD_LAST_GDB_CMD_INDEX))
    {
        retVal = &(stat_gdbCommandsInfos[gdbCommand]);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        pdDoesCommandRequireFlush
// Description: Returns true iff gdbCommand is a command that requires us to
//              call pdGDBDriver::flushCommandOutput after it is written.
// Author:      Uri Shomroni
// Date:        28/12/2009
// ---------------------------------------------------------------------------
bool pdDoesCommandRequireFlush(pdGDBCommandId gdbCommand)
{
    bool retVal = (gdbCommand == PD_GET_LIBRARY_AT_ADDRESS); // || (gdbCommand == PD_EXECUTE_FUNCTION_CMD);

    return retVal;
}

