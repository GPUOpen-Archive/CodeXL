//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osWin32CallStackReader.cpp
///
//=====================================================================

//------------------------------ osWin32CallStackReader.cpp ------------------------------

// Windows:
#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>

// Windows Debug Help library:
#pragma warning( push )
#pragma warning( disable : 4091)
#include <dbghelp.h>
#pragma warning( pop )

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osWin32CallStackReader.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>

#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    #include <intrin.h>
#endif

// Definitions to find registries in assembly, depending on windows version
// Definitions for 32-bit and 64-bit Architecture: x86 and x64
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
    #define AX_REGISTER eax
    #define BP_REGISTER ebp
    #define SP_REGISTER esp
    #define ARCHITECTURE_IMAGE_FILE IMAGE_FILE_MACHINE_I386
    #define SIZE_OF_POINTER (4)
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    #define AX_REGISTER rax
    #define BP_REGISTER rbp
    #define SP_REGISTER rsp
    #define ARCHITECTURE_IMAGE_FILE IMAGE_FILE_MACHINE_AMD64
    #define SIZE_OF_POINTER (8)
#else
    #error Unknown address space size!
#endif // AMDT_ADDRESS_SPACE_TYPE

// The maximal call stack depth:
// TO_DO: This should be moved to the Options dialog:
#define OS_MAX_CALL_STACK_DEPTH 100



#pragma auto_inline(off)
DWORD_PTR GetAddressOfProgramCounter()
{
    DWORD_PTR addressOfProgramCounter;

#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
    __asm mov AX_REGISTER, [BP_REGISTER + SIZE_OF_POINTER];   // Get the return address out of the current stack frame
    __asm mov [addressOfProgramCounter], AX_REGISTER;      // Put the return address into the variable we'll return
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    addressOfProgramCounter = (DWORD_PTR)_ReturnAddress() + SIZE_OF_POINTER;
#else
#error Unknown address space size!
#endif

    return addressOfProgramCounter;
}
#pragma auto_inline(on)

// ---------------------------------------------------------------------------
// Name:        osWin32CallStackReader::osWin32CallStackReader
// Description: Constructor to be used in the spy, to get the current thread / process
//              calls stack
// Author:      AMD Developer Tools Team
// Date:        22/10/2008
// ---------------------------------------------------------------------------
osWin32CallStackReader::osWin32CallStackReader(osCallStack& callStack)
    : _debugInfoReader(GetCurrentProcess()), _hProcess(GetCurrentProcess()), _hThread(GetCurrentThread()), _pThreadExecutionContext(NULL),
      _callStack(callStack), _isReadingCurrentThread(true), m_maxFrames(OS_MAX_CALL_STACK_DEPTH)
{

}

// ---------------------------------------------------------------------------
// Name:        osWin32CallStackReader::osWin32CallStackReader
// Description: Constructor
// Arguments:   hProcess - A handle to the process that contains the thread.
//              hThread - A handle to the thread who's call stack will be read.
//              callStack - Will get the output call stack.
// Author:      AMD Developer Tools Team
// Date:        11/10/2004
// ---------------------------------------------------------------------------
osWin32CallStackReader::osWin32CallStackReader(osProcessHandle hProcess, osThreadHandle hThread,
                                               osCallStack& callStack)
    : _debugInfoReader(hProcess), _hProcess(hProcess), _hThread(hThread), _pThreadExecutionContext(NULL),
      _callStack(callStack), _isReadingCurrentThread(false), m_maxFrames(OS_MAX_CALL_STACK_DEPTH)
{
}


// ---------------------------------------------------------------------------
// Name:        osWin32CallStackReader::osWin32CallStackReader
// Description: Constructor.
// Arguments:
//  hProcess - A handle to the process that contains the thread.
//  pThreadExecutionContext - An execution context of a thread who's call
//                                      stack will be read.
//            callStack - Will get the output call stack.
// Author:      AMD Developer Tools Team
// Date:        13/5/2009
// ---------------------------------------------------------------------------
osWin32CallStackReader::osWin32CallStackReader(osProcessHandle hProcess, CONTEXT* pThreadExecutionContext, osCallStack& callStack)
    : _debugInfoReader(hProcess), _hProcess(hProcess), _hThread(NULL), _pThreadExecutionContext(pThreadExecutionContext),
      _callStack(callStack), _isReadingCurrentThread(false), m_maxFrames(OS_MAX_CALL_STACK_DEPTH)
{
}


// ---------------------------------------------------------------------------
// Name:        osWin32CallStackReader::execute
// Description: Does this class action - fills the output call stack.
// Arguments: hideSpyDLLsFunctions - if true, stack frames that contain spy DLLs
//                                    functions (and all stack frames that appear beneath
//                                    them) will be removed from the output call stack.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        11/10/2004
// ---------------------------------------------------------------------------
bool osWin32CallStackReader::execute(bool hideSpyDLLsFunctions)
{
    bool retVal = true;
    BOOL rc = FALSE;

    // Get the windows system once for performance improvements
    static bool needsToFindWindowSystem = true;
    static osWindowsVersion windowsSystem = OS_WIN_98;

    if (needsToFindWindowSystem)
    {
        needsToFindWindowSystem = false;
        osGetWindowsVersion(windowsSystem);
    }

    // prepare the window frames Stack
    int maxStackSize = m_maxFrames;
    if (OS_MAX_CALL_STACK_DEPTH < maxStackSize)
    {
        maxStackSize = OS_MAX_CALL_STACK_DEPTH;
    }
    else if (1 > maxStackSize)
    {
        maxStackSize = 1;
    }

    gtVector<STACKFRAME64> winStackFrames;
    winStackFrames.resize(maxStackSize);

    // Set the call stack's address size:
    setStackAddressSpaceType();

    // Will contain the thread's context in case that we don't get it from outside:
    CONTEXT threadContext;

    // A Windows structure that will contain the current stack frames:
    STACKFRAME64 currentWinStackFrame;

    // Initialize the structure:
    ZeroMemory(&currentWinStackFrame , sizeof(STACKFRAME64));

    // The code segment in the "is reading current thread" block was influenced by ideas
    // explained in the Visual Leak Detector sample on Code Project website.
    // The code in CodeXL was not copied from this sample. Some of the ideas in the
    // sample code that performs stack walk influenced the way the CodeXL code
    // performs stack walk. There is a general similarity because both codes achieve the
    // same goal, and use the same Windows API calls that any stack walk code must use.
    if (_isReadingCurrentThread)
    {
        // Get the program counter:
        currentWinStackFrame.AddrPC.Offset  = GetAddressOfProgramCounter();
        currentWinStackFrame.AddrPC.Mode    = AddrModeFlat;
        // Get the frame address:
        DWORD_PTR frameAddr = 0;
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
        __asm mov [frameAddr], BP_REGISTER // Get the frame pointer (aka base pointer)
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
        CONTEXT currentContext;
        RtlCaptureContext(&currentContext);
        frameAddr = currentContext.Rbp;
        // frameAddr = _ReturnAddress();
#else
#error Unknown address space size!
#endif
        currentWinStackFrame.AddrFrame.Offset   = frameAddr;
        currentWinStackFrame.AddrFrame.Mode     = AddrModeFlat;

        // Get the stack address:
        DWORD_PTR stackAddr = 0;
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
        __asm mov [stackAddr], SP_REGISTER
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
        stackAddr = currentContext.Rsp;
#else
#error Unknown address space size!
#endif
        currentWinStackFrame.AddrStack.Offset   = stackAddr;
        currentWinStackFrame.AddrStack.Mode     = AddrModeFlat;

        // Fill the context:
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
        threadContext.Eip = (DWORD)currentWinStackFrame.AddrPC.Offset;
        threadContext.Esp = (DWORD)currentWinStackFrame.AddrStack.Offset;
        threadContext.Ebp = (DWORD)currentWinStackFrame.AddrFrame.Offset;
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
        threadContext.Rip = (DWORD64)currentWinStackFrame.AddrPC.Offset;
        threadContext.Rsp = (DWORD64)currentWinStackFrame.AddrStack.Offset;
        threadContext.Rbp = (DWORD64)currentWinStackFrame.AddrFrame.Offset;
#else
#error Unknown address space size!
#endif
        _pThreadExecutionContext = &threadContext;

        rc = TRUE;
    }
    else if (_pThreadExecutionContext != NULL)
    {
        // We already have the thread's execution context:
        rc = TRUE;
    }
    else
    {
        // Get the input thread's execution context:
        threadContext.ContextFlags = CONTEXT_FULL;
        _pThreadExecutionContext = &threadContext;
        rc = ::GetThreadContext(_hThread, _pThreadExecutionContext);
    }

    if (!rc)
    {
        retVal = false;
    }
    else
    {
        // The Machine (CPU) type:
        DWORD machineType = ARCHITECTURE_IMAGE_FILE;

        if (!_isReadingCurrentThread)
        {
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
            currentWinStackFrame.AddrPC.Offset      = _pThreadExecutionContext->Eip;
            currentWinStackFrame.AddrPC.Mode        = AddrModeFlat;
            currentWinStackFrame.AddrStack.Offset   = _pThreadExecutionContext->Esp;
            currentWinStackFrame.AddrStack.Mode     = AddrModeFlat;
            currentWinStackFrame.AddrFrame.Offset   = _pThreadExecutionContext->Ebp;
            currentWinStackFrame.AddrFrame.Mode     = AddrModeFlat;
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
            currentWinStackFrame.AddrPC.Offset      = _pThreadExecutionContext->Rip;
            currentWinStackFrame.AddrPC.Mode        = AddrModeFlat;
            currentWinStackFrame.AddrStack.Offset   = _pThreadExecutionContext->Rsp;
            currentWinStackFrame.AddrStack.Mode     = AddrModeFlat;
            currentWinStackFrame.AddrFrame.Offset   = _pThreadExecutionContext->Rbp;
            currentWinStackFrame.AddrFrame.Mode     = AddrModeFlat;
#else
#error Unknown address space size!
#endif
        }

        // Contains the current call stack depth:
        int currentCallStackDepth = 0;
        bool shouldUseStackWalk64 = true;

#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE

        // If we are reading the current call stack in 64 bit use RtlCaptureStackBackTrace and not StackWalk64
        if (_isReadingCurrentThread)
        {
            shouldUseStackWalk64 = false;
        }

#endif

        if (shouldUseStackWalk64)
        {
            // Iterate on the stack frames:
            while (StackWalk64(machineType, _hProcess, _hThread, &currentWinStackFrame,
                               _pThreadExecutionContext, NULL, SymFunctionTableAccess64, SymGetModuleBase64, NULL) == TRUE)
            {
                // Basic sanity check (make sure the stack frame points a function):
                if (currentWinStackFrame.AddrFrame.Offset != 0)
                {
                    // If this is not the last stack frame:
                    // (The last stack frame in Win32 applications represents a function named
                    // "RegisterWaitForInputIdle", that we wouldn't like to display).
                    if (_isReadingCurrentThread || (currentWinStackFrame.AddrReturn.Offset != 0))
                    {
                        // We need to take the PC 1 instruction back (since the PC was already
                        // advanced to the next instruction):
                        currentWinStackFrame.AddrPC.Offset = currentWinStackFrame.AddrPC.Offset - 1;

                        // Add the current frame to the temp frame stack
                        // winStackFrames.push_back(currentWinStackFrame);
                        winStackFrames[currentCallStackDepth] = currentWinStackFrame;
                    }
                }
                else
                {
                    // We have a failure, but since Win32 StackWalk function has a lot of failures,
                    // we do not consider this as a failure that affects the success of this function.
                    // retVal = false;
                    break;
                }

                // If we passed the call stack depth limitation - exit the StackWalk loop:
                currentCallStackDepth++;

                if (currentCallStackDepth >= maxStackSize)
                {
                    break;
                }
            }
        }

#if AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE

        // If we are reading the current call stack in 64 bit use RtlCaptureStackBackTrace
        if (_isReadingCurrentThread)
        {
            // Quote from Microsoft Documentation:
            // ## Windows Server 2003 and Windows XP:
            // ## The sum of the FramesToSkip and FramesToCapture parameters must be less than 63.
            const int kMaxCallers = (windowsSystem > OS_WIN_SERVER_2003_R2 ? OS_MAX_CALL_STACK_DEPTH : 62);

            // the stack depth is maximum OS_MAX_CALL_STACK_DEPTH
            void* callers[OS_MAX_CALL_STACK_DEPTH];

            // but the sum of 0 + kMaxCallers will be less then the limit based on system
            if (kMaxCallers < maxStackSize)
            {
                maxStackSize = kMaxCallers;
            }
            int callersCount = RtlCaptureStackBackTrace(0, maxStackSize, callers, NULL);

            // Copy the win stack frames to our stack frame
            // First clear the old useless stack
            _callStack.clearStack();

            // Convert the direct function call to the STACKFRAME64 format so osCallStackFrame
            // Can be filled correctly later
            for (int aFrame = 0 ; aFrame < callersCount ; aFrame ++)
            {
                currentWinStackFrame.AddrPC.Offset = (DWORD64)callers[aFrame];
                winStackFrames[aFrame] = currentWinStackFrame;
            }

            // Update the number of calls in the stack using the new method
            currentCallStackDepth = callersCount;
        }

#endif
        int winStackFrameSize = currentCallStackDepth;
        //int winStackFrameSize = winStackFrames.size();

        // Set initial size to the right one:
        _callStack.reserveStack(winStackFrameSize);

        for (int iFrame = 0 ; iFrame < winStackFrameSize ; iFrame++)
        {
            // Fill the current call stack frame:
            osCallStackFrame currentStackFrame;

            // Fill current stack frame from win32 frame
            currentWinStackFrame = winStackFrames[iFrame];

            fillStackFrame(currentWinStackFrame, currentStackFrame);

            // If we were requested to hide spy DLL functions and the current stack frame
            // contains a Spy DLL function::
            if (hideSpyDLLsFunctions && currentStackFrame.isSpyFunction())
            {
                // Clear the entire call stack (we would like to remove the current stack
                // frame, and all stack frames that reside beneath it):
                _callStack.clearStack();
            }
            else
            {
                // Add the stack frame to the call stack:
                _callStack.addStackFrame(currentStackFrame);
            }
        }
    }

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osWin32CallStackReader::setStackAddressSpaceType
// Description: Sets the call stacks address space type.
// Author:      AMD Developer Tools Team
// Date:        11/7/2010
// ---------------------------------------------------------------------------
void osWin32CallStackReader::setStackAddressSpaceType()
{
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
    {
        _callStack.setAddressSpaceType(false);
    }
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
    {
        _callStack.setAddressSpaceType(true);
    }
#endif
}


// ---------------------------------------------------------------------------
// Name:        osWin32CallStackReader::fillStackFrame
// Description: Inputs a windows stack frame and uses it to fill an osCallStackFrame.
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        12/10/2004
// ---------------------------------------------------------------------------
bool osWin32CallStackReader::fillStackFrame(const STACKFRAME64& winStackFrame,
                                            osCallStackFrame& stackFrame)
{
    bool retVal = false;

    // Get the instruction address:
    DWORD64 instructionAddress = winStackFrame.AddrPC.Offset;

    // Fill the instructing counter address:
    stackFrame.setInstructionCounterAddress(instructionAddress);

    // Fill the rest of the info:
    /*retVal =*/ _debugInfoReader.fillStackFrame(stackFrame);

    // We require only the module details to succeed:
    retVal = stackFrame.moduleFilePath().isRegularFile();

    return retVal;
}
