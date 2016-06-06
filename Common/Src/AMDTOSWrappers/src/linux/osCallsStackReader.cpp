//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osCallsStackReader.cpp
///
//=====================================================================

//------------------------------ osCallsStackReader.cpp ------------------------------

// Includes for Linux backtrace:
#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osCallsStackReader.h>
#include <AMDTOSWrappers/Include/osCallStack.h>

// The maximum depth of calls stack we can read
#define OS_LINUX_CALLS_STACK_MAX_DEPTH 100

// ---------------------------------------------------------------------------
// Name:        osCallsStackReader::osCallsStackReader
// Description: Constructor
// Author:      AMD Developer Tools Team
// Date:        23/10/2008
// ---------------------------------------------------------------------------
osCallsStackReader::osCallsStackReader()
{

}

// ---------------------------------------------------------------------------
// Name:        osCallsStackReader::~osCallsStackReader
// Description: Destructor
// Author:      AMD Developer Tools Team
// Date:        23/10/2008
// ---------------------------------------------------------------------------
osCallsStackReader::~osCallsStackReader()
{

}

// ---------------------------------------------------------------------------
// Name:        osCallsStackReader::getCurrentCallsStack
// Description: populates callStack with the current thread's calls stack. If
//              shouldIgnoreSpyFrames is true, stack frames belonging to the
//              spy are discarded.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        23/10/2008
// ---------------------------------------------------------------------------
bool osCallsStackReader::getCurrentCallsStack(osCallStack& callStack, bool shouldIgnoreSpyFrames, bool forceWindowsSymbolLoading)
{
    bool retVal = true;

    // Unused parameter:
    (void)forceWindowsSymbolLoading;

    // We use osProcedureAddress instead of osInstructionPointer here since osInstructionPointer is supposed to be
    // address-space-size independant, but backtrace() and dladdr() require pointer-sized items.
    osProcedureAddress callsStackPointers[OS_LINUX_CALLS_STACK_MAX_DEPTH];
    int callsStackDepth = backtrace((void**)callsStackPointers, OS_LINUX_CALLS_STACK_MAX_DEPTH);
    GT_ASSERT_EX((callsStackDepth < OS_LINUX_CALLS_STACK_MAX_DEPTH), L"Maximum calls stack depth reached, some data may have been lost");

    osCallStack pendingCallsStack;
    int numberOfIgnoredFrames = 0;

    for (int i = 0; i < callsStackDepth; i++)
    {
        osCallStackFrame currentCallsStackFrame;

        // We move the instruction pointer one step back so it points to the current instruction and not
        // to the next one
        currentCallsStackFrame.setInstructionCounterAddress((osInstructionPointer)((gtSize_t)callsStackPointers[i] - 1));

        // Note that we use dladdr instead of backtrace_symbols for two reasons:
        //  1. The implementation of dladdr is identical on Linux and Mac, while
        //  backtrace_symbols gives out different output on these two platforms
        //  2. The Mac implementation of backtrace_symbols doesn't give the module name.
        Dl_info dynamicLinkerInfo;
        int errCode = dladdr((void*)(callsStackPointers[i]), &dynamicLinkerInfo);

        if (errCode != 0)
        {
            if (dynamicLinkerInfo.dli_fname != NULL)
            {
                gtString moduleNameAsString;
                moduleNameAsString.fromASCIIString(dynamicLinkerInfo.dli_fname);

                currentCallsStackFrame.setModuleFilePath(osFilePath(moduleNameAsString));
            }

            if (dynamicLinkerInfo.dli_sname != NULL)
            {
                gtString symbolName;
                symbolName.fromASCIIString(dynamicLinkerInfo.dli_sname);

                gtString demangledSymbolName;

                int demanglingStatus = 0;
                char* demangledStringAsCString = abi::__cxa_demangle(symbolName.asASCIICharArray(), NULL, NULL, &demanglingStatus);

                if ((demanglingStatus == 0) && (demangledStringAsCString != NULL))
                {
                    demangledSymbolName.fromASCIIString(demangledStringAsCString);
                    int firstBracketPosition = demangledSymbolName.find('(');

                    if (firstBracketPosition > 0)
                    {
                        demangledSymbolName.truncate(0, (firstBracketPosition - 1));
                    }
                }
                else
                {
                    demangledSymbolName.makeEmpty();
                }

                if (demangledSymbolName.isEmpty())
                {
                    demangledSymbolName = symbolName;
                }

                currentCallsStackFrame.setFunctionName(demangledSymbolName);
            }

            if (dynamicLinkerInfo.dli_saddr != NULL)
            {
                osInstructionPointer symbolAddress = (osInstructionPointer)dynamicLinkerInfo.dli_saddr;
                currentCallsStackFrame.setFunctionStartAddress(symbolAddress);
            }

            if (dynamicLinkerInfo.dli_fbase != NULL)
            {
                osInstructionPointer fileBase = (osInstructionPointer)dynamicLinkerInfo.dli_fbase;
                currentCallsStackFrame.setModuleStartAddress(fileBase);
            }
        }

        // We don't have source code file names here so can only check the module name:
#ifdef _GR_IPHONE_BUILD
#ifdef _GR_IPHONE_DEVICE_BUILD
        static const gtString openGLSpyModuleName = OS_OPENGL_ES_DEVICE_COMMON_DLL_NAME;
#else // ndef _GR_IPHONE_DEVICE_BUILD
        static const gtString openGLSpyModuleName = OS_OPENGL_ES_COMMON_DLL_NAME;
#endif // _GR_IPHONE_DEVICE_BUILD
#else // ndef _GR_IPHONE_BUILD
        static const gtString openGLSpyModuleName = OS_GREMEDY_OPENGL_SERVER_MODULE_NAME;
#endif // _GR_IPHONE_BUILD
        static const gtString openCLSpyModuleName = OS_GREMEDY_OPENCL_SERVER_MODULE_NAME;

        const gtString& moduleFilePathAsString = currentCallsStackFrame.moduleFilePath().asString();
        bool isSpyFileName = (moduleFilePathAsString.find(openGLSpyModuleName) != -1) || (moduleFilePathAsString.find(openCLSpyModuleName) != -1);
        static const gtString linuxSystemPathPrefix = L"/usr/lib";

        if (isSpyFileName && !(moduleFilePathAsString.startsWith(linuxSystemPathPrefix)))
        {
            currentCallsStackFrame.markAsSpyFunction();
        }
        else
        {
            currentCallsStackFrame.markAsSpyFunction(false);
        }

        // Add the frame to the calls stack:
        if (shouldIgnoreSpyFrames && currentCallsStackFrame.isSpyFunction())
        {
            pendingCallsStack.clearStack();
            numberOfIgnoredFrames = i + 1;
        }
        else
        {
            pendingCallsStack.addStackFrame(currentCallsStackFrame);
        }
    }

    // We currently do not verify if we got all the information (as we allow some of the frames
    // to be without debug info). However, we do check that no frames "disappeared" on the way:
    GT_IF_WITH_ASSERT((pendingCallsStack.amountOfStackFrames() + numberOfIgnoredFrames) == callsStackDepth)
    {
        callStack = pendingCallsStack;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCallsStackReader::getCallStack
// Description: Inputs a thread's execution context and outputs the call stack
//              associated with it.
// Arguments:
//  callStack - Will get the output call stack.
//  shouldIgnoreSpyFrames - If true, the output call stack will not include
//                          spy related call stack threds.
//  pThreadExecutionContext - A pointer to a thread's execution context
//                                      cast into void*.
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        13/5/2009
// ---------------------------------------------------------------------------
bool osCallsStackReader::getCallStack(osCallStack& callStack, void* pThreadExecutionContext, bool shouldIgnoreSpyFrames)
{
    (void)(callStack); // unused
    (void)(pThreadExecutionContext); // unused
    (void)(shouldIgnoreSpyFrames); // unused
    bool retVal = false;

    // This functionality is not implemented yet on Linux!
    GT_ASSERT(false);

    return retVal;
}
