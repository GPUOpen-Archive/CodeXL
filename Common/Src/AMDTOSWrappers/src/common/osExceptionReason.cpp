//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osExceptionReason.cpp
///
//=====================================================================

//------------------------------ osExceptionReason.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>

// Windows:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    #define WIN32_LEAN_AND_MEAN 1
    #include <Windows.h>
#endif

// Local:
#include <AMDTOSWrappers/Include/osExceptionReason.h>


// Maps exception reason to a string:
static const wchar_t* stat_osExceptionReasonToString[OS_AMOUNT_OF_EXCEPTION_REASONS] =
{
    L"Unknown",
    L"Standalone Thread stopped",
    L"Insufficient Memory",
    L"Access violation",
    L"Array Bounds Exceeded",
    L"Data Type Misalignment",
    L"FLT_DENORMAL_OPERAND",
    L"FLT_DIVIDE_BY_ZERO",
    L"FLT_INEXACT_RESULT",
    L"FLT_INVALID_OPERATION",
    L"FLT_OVERFLOW",
    L"FLT_STACK_CHECK",
    L"FLT_UNDERFLOW",
    L"ILLEGAL_INSTRUCTION",
    L"IN_PAGE_ERROR",
    L"INT_DIVIDE_BY_ZERO",
    L"INT_OVERFLOW",
    L"INVALID_DISPOSITION",
    L"NONCONTINUABLE_EXCEPTION",
    L"PRIV_INSTRUCTION",
    L"SINGLE_STEP",
    L"STACK_OVERFLOW",
    L"DLL_NOT_FOUND",
    L"SIGHUP",
    L"SIGINT",
    L"SIGQUIT",
    L"SIGILL",
    L"SIGTRAP",
    L"SIGABRT",
    L"SIGBUS",
    L"SIGFPE",
    L"SIGKILL",
    L"SIGSEGV",
    L"SIGPIPE",
    L"SIGALRM",
    L"SIGTERM",
    L"SIGUSR1",
    L"SIGUSR2",
    L"SIGEMT",
    L"SIGSYS",
    L"SIGURG",
    L"SIGSTOP",
    L"SIGTSTP",
    L"SIGCONT",
    L"SIGCHLD",
    L"SIGTTIN",
    L"SIGTTOU",
    L"SIGIO",
    L"SIGXCPU",
    L"SIGXFSZ",
    L"SIGVTALRM",
    L"SIGPROF",
    L"SIGWINCH",
    L"SIGLOST",
    L"SIGPWR",
    L"SIGPOLL",
    L"SIGWIND",
    L"SIGPHONE",
    L"SIGWAITING",
    L"SIGLWP",
    L"SIGDANGER",
    L"SIGGRANT",
    L"SIGRETRACT",
    L"SIGMSG",
    L"SIGSOUND",
    L"SIGSAK",
    L"SIGPRIO",
    L"SIGCANCEL",
    L"EXC_BAD_ACCESS",
    L"EXC_BAD_INSTRUCTION",
    L"EXC_ARITHMETIC",
    L"EXC_EMULATION",
    L"EXC_SOFTWARE",
    L"EXC_BREAKPOINT",
    L"SIG32",
    L"SIG33",
    L"SIG34",
    L"SIG35",
    L"SIG36",
    L"SIG37",
    L"SIG38",
    L"SIG39",
    L"SIG40",
    L"SIG41",
    L"SIG42",
    L"SIG43",
    L"SIG44",
    L"SIG45",
    L"SIG46",
    L"SIG47",
    L"SIG48",
    L"SIG49",
    L"SIG50",
    L"SIG51",
    L"SIG52",
    L"SIG53",
    L"SIG54",
    L"SIG55",
    L"SIG56",
    L"SIG57",
    L"SIG58",
    L"SIG59",
    L"SIG60",
    L"SIG61",
    L"SIG62",
    L"SIG63",
    L"SIG64",
    L"SIG65",
    L"SIG66",
    L"SIG67",
    L"SIG68",
    L"SIG69",
    L"SIG70",
    L"SIG71",
    L"SIG72",
    L"SIG73",
    L"SIG74",
    L"SIG75",
    L"SIG76",
    L"SIG77",
    L"SIG78",
    L"SIG79",
    L"SIG80",
    L"SIG81",
    L"SIG82",
    L"SIG83",
    L"SIG84",
    L"SIG85",
    L"SIG86",
    L"SIG87",
    L"SIG88",
    L"SIG89",
    L"SIG90",
    L"SIG91",
    L"SIG92",
    L"SIG93",
    L"SIG94",
    L"SIG95",
    L"SIG96",
    L"SIG97",
    L"SIG98",
    L"SIG99",
    L"SIG100",
    L"SIG101",
    L"SIG102",
    L"SIG103",
    L"SIG104",
    L"SIG105",
    L"SIG106",
    L"SIG107",
    L"SIG108",
    L"SIG109",
    L"SIG110",
    L"SIG111",
    L"SIG112",
    L"SIG113",
    L"SIG114",
    L"SIG115",
    L"SIG116",
    L"SIG117",
    L"SIG118",
    L"SIG119",
    L"SIG120",
    L"SIG121",
    L"SIG122",
    L"SIG123",
    L"SIG124",
    L"SIG125",
    L"SIG126",
    L"SIG127",
    L"SIGINFO"
};

static const wchar_t* stat_osExceptionReasonDescriptionToString[OS_AMOUNT_OF_EXCEPTION_REASONS] =
{
    // The terminology is:
    // - Windows - exception
    // - Linux - signal
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    L"An unknown exception was encountered.",
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
    L"An unknown signal was encountered.",
#else
#error Unknown target!
#endif

    L"A standalone thread has stopped running"
    L"Insufficient memory (used in cases an exception like std::bad_alloc is being thrown)"
    L"The thread tried to read from or write to a virtual address to which it does not have access.",
    L"The thread tried to access an array element that is out of bounds and the underlying hardware supports bounds checking.",
    L"The thread tried to read or write data that is misaligned on hardware that does not provide alignment. For example, 16-bit values must be aligned on 2-byte boundaries; 32-bit values on 4-byte boundaries, and so on.",
    L"One of the operands in a floating-point operation is denormal. A denormal value is one that is too small to represent as a standard floating-point value.",
    L"The thread tried to divide a floating-point value by a floating-point divisor of zero.",
    L"The result of a floating-point operation cannot be represented exactly as a decimal fraction.",
    L"This exception represents any floating-point exception not included in this list.",
    L"The exponent of a floating-point operation is greater than the magnitude allowed by the corresponding type.",
    L"The stack overflowed or underflowed as the result of a floating-point operation.",
    L"The exponent of a floating-point operation is less than the magnitude allowed by the corresponding type.",
    L"The thread tried to execute an invalid instruction.",
    L"The thread tried to access a page that was not present, and the system was unable to load the page. For example, this exception might occur if a network connection is lost while running a program over the network.",
    L"The thread tried to divide an integer value by an integer divisor of zero.",
    L"The result of an integer operation caused a carry out of the most significant bit of the result.",
    L"An exception handler returned an invalid disposition to the exception dispatcher. Programmers using a high-level language such as C should never encounter this exception.",
    L"The thread tried to continue execution after a noncontinuable exception occurred.",
    L"The thread tried to execute an instruction whose operation is not allowed in the current machine mode.",
    L"A trace trap or other single-instruction mechanism signaled that one instruction has been executed.",
    L"The thread used up its stack.",
    L"A required dll file was not found.",
    L"Hangup detected on controlling terminal or death of controlling process.",
    L"Interrupt from keyboard.",
    L"Quit from keyboard.",
    L"Illegal Instruction.",
    L"Trace/breakpoint trap.",
    L"Abort signal from abort(3).",
    L"Bus error.",
    L"Floating point exception.",
    L"Kill signal.",
    L"Invalid memory reference.",
    L"Broken pipe: write to pipe with no readers.",
    L"Timer signal from alarm(2).",
    L"Termination signal.",
    L"User-defined signal 1.",
    L"User-defined signal 2.",
    L"Emulation trap",
    L"Bad system call",
    L"Urgent I/O condition",
    L"Stopped (signal)",
    L"Stopped (user)",
    L"Continued",
    L"Child status changed",
    L"Stopped (tty input)",
    L"Stopped (tty output)",
    L"I/O possible",
    L"CPU time limit exceeded",
    L"File size limit exceeded",
    L"Virtual timer expired",
    L"Profiling timer expired",
    L"Window size changed",
    L"Resource lost",
    L"Power fail/restart",
    L"Pollable event occurred",
    L"SIGWIND",
    L"SIGPHONE",
    L"Process's LWPs are blocked",
    L"Signal LWP",
    L"Swap space dangerously low",
    L"Monitor mode granted",
    L"Need to relinquish monitor mode",
    L"Monitor mode data available",
    L"Sound completed",
    L"Secure attention",
    L"SIGPRIO",
    L"LWP internal signal",
    L"Could not access memory",
    L"Illegal instruction/operand",
    L"Arithmetic exception",
    L"Emulation instruction",
    L"Software generated exception",
    L"Breakpoint",
    L"Real-time event 32",
    L"Real-time event 33",
    L"Real-time event 34",
    L"Real-time event 35",
    L"Real-time event 36",
    L"Real-time event 37",
    L"Real-time event 38",
    L"Real-time event 39",
    L"Real-time event 40",
    L"Real-time event 41",
    L"Real-time event 42",
    L"Real-time event 43",
    L"Real-time event 44",
    L"Real-time event 45",
    L"Real-time event 46",
    L"Real-time event 47",
    L"Real-time event 48",
    L"Real-time event 49",
    L"Real-time event 50",
    L"Real-time event 51",
    L"Real-time event 52",
    L"Real-time event 53",
    L"Real-time event 54",
    L"Real-time event 55",
    L"Real-time event 56",
    L"Real-time event 57",
    L"Real-time event 58",
    L"Real-time event 59",
    L"Real-time event 60",
    L"Real-time event 61",
    L"Real-time event 62",
    L"Real-time event 63",
    L"Real-time event 64",
    L"Real-time event 65",
    L"Real-time event 66",
    L"Real-time event 67",
    L"Real-time event 68",
    L"Real-time event 69",
    L"Real-time event 70",
    L"Real-time event 71",
    L"Real-time event 72",
    L"Real-time event 73",
    L"Real-time event 74",
    L"Real-time event 75",
    L"Real-time event 76",
    L"Real-time event 77",
    L"Real-time event 78",
    L"Real-time event 79",
    L"Real-time event 80",
    L"Real-time event 81",
    L"Real-time event 82",
    L"Real-time event 83",
    L"Real-time event 84",
    L"Real-time event 85",
    L"Real-time event 86",
    L"Real-time event 87",
    L"Real-time event 88",
    L"Real-time event 89",
    L"Real-time event 90",
    L"Real-time event 91",
    L"Real-time event 92",
    L"Real-time event 93",
    L"Real-time event 94",
    L"Real-time event 95",
    L"Real-time event 96",
    L"Real-time event 97",
    L"Real-time event 98",
    L"Real-time event 99",
    L"Real-time event 100",
    L"Real-time event 101",
    L"Real-time event 102",
    L"Real-time event 103",
    L"Real-time event 104",
    L"Real-time event 105",
    L"Real-time event 106",
    L"Real-time event 107",
    L"Real-time event 108",
    L"Real-time event 109",
    L"Real-time event 110",
    L"Real-time event 111",
    L"Real-time event 112",
    L"Real-time event 113",
    L"Real-time event 114",
    L"Real-time event 115",
    L"Real-time event 116",
    L"Real-time event 117",
    L"Real-time event 118",
    L"Real-time event 119",
    L"Real-time event 120",
    L"Real-time event 121",
    L"Real-time event 122",
    L"Real-time event 123",
    L"Real-time event 124",
    L"Real-time event 125",
    L"Real-time event 126",
    L"Real-time event 127",
    L"Information request"
};


// ---------------------------------------------------------------------------
// Name:        osExceptionReasonToString
// Description: Returns a Human readable string instead of reason number.
// Author:      AMD Developer Tools Team
// Date:        7/7/2004
// ---------------------------------------------------------------------------
void osExceptionReasonToString(osExceptionReason exceptionReason, gtString& reasonAsString)
{
    reasonAsString = stat_osExceptionReasonToString[exceptionReason];
}


// ---------------------------------------------------------------------------
// Name:        osExceptionReasonToExplanationString
// Description: Returns a Human readable explanation to the exception
// Author:      AMD Developer Tools Team
// Date:        7/7/2004
// ---------------------------------------------------------------------------
void osExceptionReasonToExplanationString(osExceptionReason exceptionReason, gtString& reasonAsExplanationString)
{
    reasonAsExplanationString = stat_osExceptionReasonDescriptionToString[exceptionReason];
}


// Windows only code:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

// ---------------------------------------------------------------------------
// Name:        osExceptionCodeToExceptionReason
// Description: Translates win32 exception codes to osExceptionReason
//              values.
// Author:      AMD Developer Tools Team
// Date:        20/12/2003
// ---------------------------------------------------------------------------
osExceptionReason osExceptionCodeToExceptionReason(osExceptionCode exceptionCode)
{
    osExceptionReason retVal = OS_UNKNOWN_EXCEPTION_REASON;

    switch (exceptionCode)
    {
        case EXCEPTION_ACCESS_VIOLATION:
            retVal = OS_ACCESS_VIOLATION;
            break;

        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
            retVal = OS_ARRAY_BOUNDS_EXCEEDED;
            break;

        case EXCEPTION_BREAKPOINT:
            // We should not reach this code, since we handle exceptions as a
            // standalone event !!!
            GT_ASSERT(0);
            retVal = OS_UNKNOWN_EXCEPTION_REASON;
            break;

        case EXCEPTION_DATATYPE_MISALIGNMENT:
            retVal = OS_DATATYPE_MISALIGNMENT;
            break;

        case EXCEPTION_FLT_DENORMAL_OPERAND:
            retVal = OS_FLT_DENORMAL_OPERAND;
            break;

        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
            retVal = OS_FLT_DIVIDE_BY_ZERO;
            break;

        case EXCEPTION_FLT_INEXACT_RESULT:
            retVal = OS_FLT_INEXACT_RESULT;
            break;

        case EXCEPTION_FLT_INVALID_OPERATION:
            retVal = OS_FLT_INVALID_OPERATION;
            break;

        case EXCEPTION_FLT_OVERFLOW:
            retVal = OS_FLT_OVERFLOW;
            break;

        case EXCEPTION_FLT_STACK_CHECK:
            retVal = OS_FLT_STACK_CHECK;
            break;

        case EXCEPTION_FLT_UNDERFLOW:
            retVal = OS_FLT_UNDERFLOW;
            break;

        case EXCEPTION_ILLEGAL_INSTRUCTION:
            retVal = OS_ILLEGAL_INSTRUCTION;
            break;

        case EXCEPTION_IN_PAGE_ERROR:
            retVal = OS_IN_PAGE_ERROR;
            break;

        case EXCEPTION_INT_DIVIDE_BY_ZERO:
            retVal = OS_INT_DIVIDE_BY_ZERO;
            break;

        case EXCEPTION_INT_OVERFLOW:
            retVal = OS_INT_OVERFLOW;
            break;

        case EXCEPTION_INVALID_DISPOSITION:
            retVal = OS_INVALID_DISPOSITION;
            break;

        case EXCEPTION_NONCONTINUABLE_EXCEPTION:
            retVal = OS_NONCONTINUABLE_EXCEPTION;
            break;

        case EXCEPTION_PRIV_INSTRUCTION:
            retVal = OS_PRIV_INSTRUCTION;
            break;

        case EXCEPTION_SINGLE_STEP:
            retVal = OS_SINGLE_STEP;
            break;

        case EXCEPTION_STACK_OVERFLOW:
            retVal = OS_STACK_OVERFLOW;
            break;

        case 0xC0000135L:   // STATUS_DLL_NOT_FOUND, defined in ntstatus.h
            // Uri, 1/2/09: we do not include htstatus.h as it causes a lot of redefinitions.
            retVal = OS_DLL_NOT_FOUND;
            break;

        default:
            // We set the default ret val at the beginning of this function.
            break;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osExceptionReasonToExceptionCode
// Description: Translates osExceptionReason values to win32 exception codes.
// Author:      AMD Developer Tools Team
// Date:        10/10/2010
// ---------------------------------------------------------------------------
osExceptionCode osExceptionReasonToExceptionCode(osExceptionReason exceptionReason)
{
    osExceptionCode retVal = 0;

    switch (exceptionReason)
    {
        case OS_ACCESS_VIOLATION:
            retVal = EXCEPTION_ACCESS_VIOLATION;
            break;

        case OS_ARRAY_BOUNDS_EXCEEDED:
            retVal = EXCEPTION_ARRAY_BOUNDS_EXCEEDED;
            break;

        case OS_DATATYPE_MISALIGNMENT:
            retVal = EXCEPTION_DATATYPE_MISALIGNMENT;
            break;

        case OS_FLT_DENORMAL_OPERAND:
            retVal = EXCEPTION_FLT_DENORMAL_OPERAND;
            break;

        case OS_FLT_DIVIDE_BY_ZERO:
            retVal = EXCEPTION_FLT_DIVIDE_BY_ZERO;
            break;

        case OS_FLT_INEXACT_RESULT:
            retVal = EXCEPTION_FLT_INEXACT_RESULT;
            break;

        case OS_FLT_INVALID_OPERATION:
            retVal = EXCEPTION_FLT_INVALID_OPERATION;
            break;

        case OS_FLT_OVERFLOW:
            retVal = EXCEPTION_FLT_OVERFLOW;
            break;

        case OS_FLT_STACK_CHECK:
            retVal = EXCEPTION_FLT_STACK_CHECK;
            break;

        case OS_FLT_UNDERFLOW:
            retVal = EXCEPTION_FLT_UNDERFLOW;
            break;

        case OS_ILLEGAL_INSTRUCTION:
            retVal = EXCEPTION_ILLEGAL_INSTRUCTION;
            break;

        case OS_IN_PAGE_ERROR:
            retVal = EXCEPTION_IN_PAGE_ERROR;
            break;

        case OS_INT_DIVIDE_BY_ZERO:
            retVal = EXCEPTION_INT_DIVIDE_BY_ZERO;
            break;

        case OS_INT_OVERFLOW:
            retVal = EXCEPTION_INT_OVERFLOW;
            break;

        case OS_INVALID_DISPOSITION:
            retVal = EXCEPTION_INVALID_DISPOSITION;
            break;

        case OS_NONCONTINUABLE_EXCEPTION:
            retVal = EXCEPTION_NONCONTINUABLE_EXCEPTION;
            break;

        case OS_PRIV_INSTRUCTION:
            retVal = EXCEPTION_PRIV_INSTRUCTION;
            break;

        case OS_SINGLE_STEP:
            retVal = EXCEPTION_SINGLE_STEP;
            break;

        case OS_STACK_OVERFLOW:
            retVal = EXCEPTION_STACK_OVERFLOW;
            break;

        case OS_DLL_NOT_FOUND:
            // Uri, 1/2/09: we do not include htstatus.h as it causes a lot of redefinitions.
            retVal = 0xC0000135L; // STATUS_DLL_NOT_FOUND, defined in ntstatus.h
            break;

        case OS_UNKNOWN_EXCEPTION_REASON:
        default:
            // We set the default retVal at the beginning of this function.
            // We should also not get here on Windows:
            GT_ASSERT(false);
            break;
    }

    return retVal;
}

#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
// ---------------------------------------------------------------------------
// Name:        apExceptionCodeToExceptionReason
// Description: Translates Linux singal numbers to osExceptionReason values.
// Author:      AMD Developer Tools Team
// Date:        20/12/2003
// ---------------------------------------------------------------------------
osExceptionReason osExceptionCodeToExceptionReason(osExceptionCode exceptionCode)
{
    // TO_DO: LNX: This function is not yet implemented on Linux:
    GT_ASSERT(false);

    (void)(exceptionCode); // Remove the compiler error on Linux variant
    osExceptionReason retVal = OS_UNKNOWN_EXCEPTION_REASON;
    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        apExceptionReasonToExceptionCode
// Description: Translates osExceptionReason values to Linux singal numbers.
// Author:      AMD Developer Tools Team
// Date:        10/10/2010
// ---------------------------------------------------------------------------
osExceptionCode osExceptionReasonToExceptionCode(osExceptionReason exceptionReason)
{
    // TO_DO: LNX: This function is not yet implemented on Linux:
    GT_ASSERT(false);

    (void)(exceptionReason); // Remove the compiler error on Linux variant
    osExceptionCode retVal = 0;
    return retVal;
}
#else
#error Error: unknown OS!!
#endif
