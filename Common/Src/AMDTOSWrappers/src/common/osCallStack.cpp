//=====================================================================
// Copyright 2016 (c), Advanced Micro Devices, Inc. All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file osCallStack.cpp
///
//=====================================================================

//------------------------------ osCallStack.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <AMDTOSWrappers/Include/osStringConstants.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osChannel.h>
#include <AMDTOSWrappers/Include/osGeneralFunctions.h>
#include <AMDTOSWrappers/Include/osModule.h>


// ---------------------------------------------------------------------------
// Name:        osCallStack::osCallStack
// Description: Constructor
// Arguments:   threadId - The id of the thread to which the call stack belongs.
//              is64BitCallStack - true for 64 bit address space call stack, false for 32 bit address space call stack:
// Author:      AMD Developer Tools Team
// Date:        11/10/2004
// ---------------------------------------------------------------------------
osCallStack::osCallStack(osThreadId threadId, bool is64BitCallStack)
    : _threadId(threadId), _is64BitCallStack(is64BitCallStack)
{
}


// ---------------------------------------------------------------------------
// Name:        osCallStack::stackFrame
// Description: Inputs a stack frame index and returns a pointer to it
//              (Or null if it does not exist).
// Author:      AMD Developer Tools Team
// Date:        11/10/2004
// ---------------------------------------------------------------------------
const osCallStackFrame* osCallStack::stackFrame(int frameIndex) const
{
    const osCallStackFrame* retVal = NULL;

    // Index range check:
    int stackFramesAmount = amountOfStackFrames();

    if ((0 <= frameIndex) && (frameIndex < stackFramesAmount))
    {
        retVal = &(_stackFrames[frameIndex]);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCallStack::asString
// Description: Outputs a string representing the call stack.
// Arguments:
//  callStackBriefString - Will get a short string representing the call stack (usually the call stack's start address).
//  callStackString - Will get a string representing the call stack.
//  isSpyRelatedCallStack - Will get true iff the stack contains frames that are related to a spy module.
// Author:      AMD Developer Tools Team
// Date:        7/7/2010
// ---------------------------------------------------------------------------
void osCallStack::asString(gtString& callStackBriefString, gtString& callStackString, bool& isSpyModuleRelatedCallStack, bool allowDifferentSystemPathInSpy) const
{
    callStackBriefString.makeEmpty();
    isSpyModuleRelatedCallStack = false;

    // Initialize the call output call stack string to contain the string's columns headers:
    callStackString = OS_STR_callStackAsStringHeader;

    // Iterate the call stack frames:
    int callStackSize = amountOfStackFrames();

    for (int frameIndex = 0; frameIndex < callStackSize; frameIndex++)
    {
        // Get the current stack frame:
        const osCallStackFrame* pCurrStackFrame = NULL;
        pCurrStackFrame = stackFrame(frameIndex);
        GT_IF_WITH_ASSERT(pCurrStackFrame != NULL)
        {
            // Get the current stack frame's data:
            const gtString& functionName = pCurrStackFrame->functionName();
            osInstructionPointer functionStartAddress = pCurrStackFrame->functionStartAddress();
            const osFilePath& sourceCodeFilePath = pCurrStackFrame->sourceCodeFilePath();
            const gtString& sourceCodeFilePathString = sourceCodeFilePath.asString();
            const int& sourceCodeFileLineNumber = pCurrStackFrame->sourceCodeFileLineNumber();
            const osFilePath& sourceCodeModulePath = pCurrStackFrame->moduleFilePath();
            osInstructionPointer moduleStartAddress = pCurrStackFrame->moduleStartAddress();
            const osInstructionPointer instructionCounterAddress = pCurrStackFrame->instructionCounterAddress();
            bool isSpyFunction = pCurrStackFrame->isSpyFunction();

            // Divide the file path to name and extension
            gtString sourceCodeFileName;
            gtString sourceCodeFileExtension;
            sourceCodeFilePath.getFileName(sourceCodeFileName);
            sourceCodeFilePath.getFileExtension(sourceCodeFileExtension);

            // The first non-empty function name will be used as the call stack brief:
            if ((callStackBriefString.isEmpty()) && (!functionName.isEmpty()))
            {
                callStackBriefString = functionName;

                // Append the pointer offset from the module start:
                osInstructionPointer offsetFromModuleBase = (instructionCounterAddress > moduleStartAddress) ? (osInstructionPointer)((osProcedureAddress64)instructionCounterAddress - (osProcedureAddress64)moduleStartAddress) : 0;

                if (offsetFromModuleBase != 0)
                {
                    gtString offsetFromModuleBaseAsString;
                    osProcedureAddressToString((osProcedureAddress64)offsetFromModuleBase, _is64BitCallStack, false, offsetFromModuleBaseAsString);

                    callStackBriefString.append(' ').append('(').append(offsetFromModuleBaseAsString).append(')');
                }
            }

            // If the stack contains a frame that is related to a spy module (if we found such a frame, stop checking):
            if (isSpyFunction && (!isSpyModuleRelatedCallStack))
            {
                isSpyModuleRelatedCallStack = true;

                // When a user call to an API function crashes, we should recognize this scenario and not report it as a spy crash. It is identifiable by the following checklist:
                // 1. There is only one spy frame in the call stack - verifiable by checking there is no spy frame directly below (avoids queries maintenance wrappers, e.g. onProgramBuilt)
                // 2. The frame is not the top frame (avoids crashes in the wrapper and uninitialized real function addresses)
                // 3. The frame above the current frame has the same function name (avoids function replacement, e.g. stub modes), or the current frame is one of the wrapper source files
                // 4. The frame above the current frame is in the system OpenCL / OpenGL module (avoids calls to other code, e.g. the OpenCL debug API)

                // The checks are performed in order from quickest to slowest, to save time on irrelevant scenarios:
                // 2: are we the top frame:
                if (frameIndex > 0)
                {
                    bool isFrameBelowASpyFrame = false;

                    if (frameIndex < (callStackSize - 1))
                    {
                        const osCallStackFrame* pNextFrame = stackFrame(frameIndex + 1);

                        if (pNextFrame != NULL)
                        {
                            isFrameBelowASpyFrame = pNextFrame->isSpyFunction();
                        }
                    }

                    // 1: are we above a non-spy frame (i.e. a user code frame):
                    if (!isFrameBelowASpyFrame)
                    {
                        const osCallStackFrame* pPreviousFrame = stackFrame(frameIndex - 1);

                        // 3: does the frame above have the same function name:
                        if (pPreviousFrame != NULL)
                        {
                            const gtString& prevFunctionName = pPreviousFrame->functionName();
                            bool isWrapperCallingRealFunc = (prevFunctionName == functionName);

                            if (!isWrapperCallingRealFunc)
                            {
                                // Uri, 27/10/2015 - in the OpenCL ICD, if the passed argument (e.g. cl_context handle) is invalid,
                                // The ICD dispatch might fail before jumping to the real function in the specific runtime. In that case,
                                // The function "name" will be incorrect. To handle these cases, we check here in "bad identification" cases:
                                static const gtString pref1 = L"cl"; // The dispatch table is ususally identified as the first function (e.g. see CODEXL-1236).
                                static const gtString pref2 = L"0x"; // If the function name is missing, it might have been replaced with the function address.

                                if (prevFunctionName.isEmpty() || prevFunctionName.startsWith(pref1) || prevFunctionName.startsWith(pref2))
                                {
                                    static const gtString cppExtension = L"cpp";

                                    if ((cppExtension == sourceCodeFileExtension) && !sourceCodeFileName.isEmpty())
                                    {
                                        static const gtString wrapperSourceFileNames[] = { L"csDirectXIntegrationWrappers",
                                                                                           L"csOpenCLExtensionsWrappers",
                                                                                           L"csOpenCLWrappers",
                                                                                           L"csOpenGLIntegrationWrappers",
                                                                                           L"gsCGLWrappers",
                                                                                           L"gsGLXWrappers",
                                                                                           L"gsOpenGLExtensionsWrappers",
                                                                                           L"gsOpenGLWrappers",
                                                                                           L"gsWGLWrappers",
                                                                                           L"hdHSAWrappers"
                                                                                         };

                                        for (const gtString& currentSourceFileName : wrapperSourceFileNames)
                                        {
                                            if (0 == (currentSourceFileName.compareNoCase(currentSourceFileName)))
                                            {
                                                isWrapperCallingRealFunc = true;
                                                break;
                                            }
                                        }
                                    }
                                }
                            }

                            if (isWrapperCallingRealFunc)
                            {
                                const osFilePath& previousFrameModulePath = pPreviousFrame->moduleFilePath();

                                if (!allowDifferentSystemPathInSpy)
                                {
                                    static gtVector<osFilePath> systemGLModulePath;
                                    static gtVector<osFilePath> systemCLModulePath;
                                    static bool systemPathsInitialized = false;

                                    if (!systemPathsInitialized)
                                    {
                                        osGetSystemOpenGLModulePath(systemGLModulePath);
                                        osGetSystemOpenCLModulePath(systemCLModulePath);
                                        systemPathsInitialized = true;
                                    }

                                    // 4: does the frame above match any of the system module paths:
                                    int numberOfCLPaths = (int)systemCLModulePath.size();

                                    for (int i = 0; i < numberOfCLPaths; i++)
                                    {
                                        if (previousFrameModulePath == systemCLModulePath[i])
                                        {
                                            isSpyModuleRelatedCallStack = false;
                                            break;
                                        }
                                    }

                                    if (isSpyModuleRelatedCallStack)
                                    {
                                        int numberOfGLPaths = (int)systemGLModulePath.size();

                                        for (int i = 0; i < numberOfGLPaths; i++)
                                        {
                                            if (previousFrameModulePath == systemGLModulePath[i])
                                            {
                                                isSpyModuleRelatedCallStack = false;
                                                break;
                                            }
                                        }
                                    }

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS

                                    // 4: Linux only - allow modules that start in /usr/lib* - as they are definitely the system path:
                                    if (isSpyModuleRelatedCallStack)
                                    {
                                        static const gtString linuxSystemPathPrefix = L"/usr/lib";

                                        if (previousFrameModulePath.asString().startsWith(linuxSystemPathPrefix))
                                        {
                                            // verify it's an opengl or opencl dll:
                                            gtString currentFrameFileName;
                                            sourceCodeModulePath.getFileName(currentFrameFileName);
                                            int periodIndex = currentFrameFileName.find('.', 1);

                                            if (-1 != periodIndex)
                                            {
                                                currentFrameFileName.truncate(0, periodIndex - 1);
                                            }

                                            gtString previousFrameFileName;
                                            previousFrameModulePath.getFileName(previousFrameFileName);
                                            periodIndex = previousFrameFileName.find('.', 1);

                                            if (-1 != periodIndex)
                                            {
                                                previousFrameFileName.truncate(0, periodIndex - 1);
                                            }

                                            if (currentFrameFileName == previousFrameFileName)
                                            {
                                                isSpyModuleRelatedCallStack = false;
                                            }
                                        }
                                    }

#endif // AMDT_BUILD_TARGET == AMDT_LINUX_OS
                                }
                                else // allowDifferentSystemPathInSpy
                                {
                                    // If we allow a different system path in the spy and client, we just need to check that it has the right module name:
                                    gtString previousFrameModuleFileName;
                                    previousFrameModulePath.getFileNameAndExtension(previousFrameModuleFileName);
                                    previousFrameModuleFileName.toLowerCase();
                                    static gtString systemGLModuleName;
                                    static gtString systemCLModuleName1;
                                    static gtString systemCLModuleName2;
                                    static gtString amdCLModuleName1;
                                    static gtString amdCLModuleName2;
                                    static bool systemNamesInitialized = false;

                                    if (!systemNamesInitialized)
                                    {
                                        systemGLModuleName = OS_OPENGL_MODULE_NAME;
                                        systemGLModuleName.toLowerCase();
                                        systemCLModuleName1 = OS_OPENCL_ICD_MODULE_NAME;
                                        systemCLModuleName1.toLowerCase();
                                        systemCLModuleName2 = OS_OPENCL_ICD_MODULE_ALTERNATIVE_NAME;
                                        systemCLModuleName2.toLowerCase();
                                        amdCLModuleName1 = OS_AMD_OPENCL_RUNTIME_MODULE_NAME;
                                        amdCLModuleName1.toLowerCase();
                                        amdCLModuleName2 = OS_AMD_OPENCL_RUNTIME_MODULE_NAME_OTHER_BITNESS;
                                        amdCLModuleName2.toLowerCase();
                                        systemNamesInitialized = true;
                                    }

                                    // 4: does the frame above match any of the system module names:
                                    if ((previousFrameModuleFileName == systemGLModuleName) || (previousFrameModuleFileName == systemCLModuleName1) || (previousFrameModuleFileName == systemCLModuleName2) || (previousFrameModuleFileName == amdCLModuleName1) || (previousFrameModuleFileName == amdCLModuleName2))
                                    {
                                        isSpyModuleRelatedCallStack = false;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Start with the file if it exists:
            gtString fullFileName;

            if (!sourceCodeFileName.isEmpty())
            {
                fullFileName = sourceCodeFileName;
                fullFileName.append(osFilePath::osExtensionSeparator);
                fullFileName.append(sourceCodeFileExtension);
            }

            // Add the function name:
            if (!functionName.isEmpty())
            {
                callStackString.append(functionName);
            }
            else
            {
                callStackString.append(OS_STR_NotAvailable);
            }

            callStackString.append(L" - ");

            // Add the file path:
            if (!sourceCodeFilePathString.isEmpty())
            {
                callStackString.append(sourceCodeFilePathString);
            }
            else
            {
                callStackString.append(OS_STR_NotAvailable);
            }

            callStackString.append(L" - ");

            // Add the line number:
            if (sourceCodeFileLineNumber > 1)
            {
                callStackString.appendFormattedString(L"%d", sourceCodeFileLineNumber);
            }
            else
            {
                callStackString.append(OS_STR_NotAvailable);
            }

            callStackString.append(L" - ");


            // Add the module path:
            if (!sourceCodeModulePath.asString().isEmpty())
            {
                callStackString.append(sourceCodeModulePath.asString());
            }
            else
            {
                callStackString.append(OS_STR_NotAvailable);
            }

            callStackString.append(L" - ");

            // Add the function start address:
            if (functionStartAddress != 0)
            {
                gtString functionStartAddressAsString;
                osProcedureAddressToString((osProcedureAddress64)functionStartAddress, _is64BitCallStack, false, functionStartAddressAsString);
                callStackString.append(functionStartAddressAsString);
            }
            else
            {
                callStackString.append(OS_STR_NotAvailable);
            }

            callStackString.append(L" - ");

            // Add the function start address:
            if (moduleStartAddress != 0)
            {
                gtString moduleStartAddressAsString;
                osProcedureAddressToString((osProcedureAddress64)moduleStartAddress, _is64BitCallStack, false, moduleStartAddressAsString);
                callStackString.append(moduleStartAddressAsString);
            }
            else
            {
                callStackString.append(OS_STR_NotAvailable);
            }

            callStackString.append(L" - ");

            // Add the instruction counter address:
            if (instructionCounterAddress != 0)
            {
                gtString instructionCounterAddressAsString;
                osProcedureAddressToString((osProcedureAddress64)instructionCounterAddress, _is64BitCallStack, false, instructionCounterAddressAsString);
                callStackString.append(instructionCounterAddressAsString);
            }
            else
            {
                callStackString.append(OS_STR_NotAvailable);
            }

            // Append a new line:
            callStackString.append(L"\n");
        }
    }

    // If we did not find a non-empty function name:
    if (callStackBriefString.isEmpty())
    {
        // If the stack is empty:
        if (callStackSize <= 0)
        {
            callStackBriefString = OS_STR_NotAvailable;
        }
        else
        {
            // Set the first call stack frame's function address as the stack's brief string:
            const osCallStackFrame* pFirstStackFrame = stackFrame(0);
            GT_IF_WITH_ASSERT(pFirstStackFrame != NULL)
            {
                osInstructionPointer firstFunctionStartAddress = pFirstStackFrame->functionStartAddress();
                osProcedureAddressToString((osProcedureAddress64)firstFunctionStartAddress, _is64BitCallStack, false, callStackBriefString);
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        osCallStack::addStackFrame
// Description: Adds a call stack frame to this call stack.
// Author:      AMD Developer Tools Team
// Date:        11/10/2004
// ---------------------------------------------------------------------------
void osCallStack::addStackFrame(const osCallStackFrame& stackFrame)
{
    _stackFrames.push_back(stackFrame);
}


// ---------------------------------------------------------------------------
// Name:        osCallStack::setStackFrame
// Description: set a stackframe if the range is valid
// Return Val:  bool - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        16/7/2012
// ---------------------------------------------------------------------------
bool osCallStack::setStackFrame(const osCallStackFrame& stackFrame, int index)
{
    bool retVal = false;

    int amountOfFrames = amountOfStackFrames();

    if (0 <= index && amountOfFrames > index)
    {
        _stackFrames[index] = stackFrame;
        retVal = true;
    }
    else if (amountOfFrames == index)
    {
        addStackFrame(stackFrame);
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        osCallStack::append
// Description: Appends substack to the top of the current calls stack
// Author:      AMD Developer Tools Team
// Date:        28/10/2010
// ---------------------------------------------------------------------------
void osCallStack::append(const osCallStack& subStack)
{
    int amountOfFramesToAdd = subStack.amountOfStackFrames();

    for (int i = 0; i < amountOfFramesToAdd; i++)
    {
        const osCallStackFrame* pCurrentFrame = subStack.stackFrame(i);
        GT_IF_WITH_ASSERT(pCurrentFrame != NULL)
        {
            addStackFrame(*pCurrentFrame);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        osCallStack::writeSelfIntoChannel
// Description: Writes self into channel
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        26/10/2008
// ---------------------------------------------------------------------------
bool osCallStack::writeSelfIntoChannel(osChannel& ipcChannel) const
{
    bool retVal = true;

    gtInt64 numberOfStackFrames = (gtInt64)amountOfStackFrames();

    // Send the number of frames in the stack.
    ipcChannel << numberOfStackFrames;

    // Send the frames:
    for (int i = 0; i < numberOfStackFrames; i++)
    {
        retVal = _stackFrames[i].writeSelfIntoChannel(ipcChannel) && retVal;
    }

    // Send the thread id:
    ipcChannel << (gtUInt64)_threadId;

    // Send the address space:
    ipcChannel << _is64BitCallStack;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCallStack::readSelfFromChannel
// Description: Reads self from channel
// Return Val: bool  - Success / failure.
// Author:      AMD Developer Tools Team
// Date:        26/10/2008
// ---------------------------------------------------------------------------
bool osCallStack::readSelfFromChannel(osChannel& ipcChannel)
{
    clearStack();
    gtInt64 numberOfStackFrames = -1;

    // Get the number of frames in the stack.
    ipcChannel >> numberOfStackFrames;
    bool retVal = (numberOfStackFrames >= 0);
    osCallStackFrame currentStackFrame;

    // Get the frames:
    for (int i = 0; i < numberOfStackFrames; i++)
    {
        retVal = currentStackFrame.readSelfFromChannel(ipcChannel) && retVal;
        addStackFrame(currentStackFrame);
    }

    // Get the thread id:
    gtUInt64 threadIdAsUInt64 = (gtUInt64)OS_NO_THREAD_ID;
    ipcChannel >> threadIdAsUInt64;
    _threadId = (osThreadId)threadIdAsUInt64;

    // Get the address space:
    ipcChannel >> _is64BitCallStack;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        osCallStack::clearStack
// Description: Clears the call stack.
// Author:      AMD Developer Tools Team
// Date:        11/10/2004
// ---------------------------------------------------------------------------
void osCallStack::clearStack()
{
    _stackFrames.clear();
}

