//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suBreakpointsManager.cpp
///
//==================================================================================

//------------------------------ suBreakpointsManager.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osSocket.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTAPIClasses/Include/Events/apDebuggedProcessDetectedErrorEvent.h>
#include <AMDTAPIClasses/Include/apFunctionType.h>

// Local:
#include <AMDTServerUtilities/Include/suBreakpointsManager.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>
#include <src/suSpyToAPIConnector.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>
#include <AMDTServerUtilities/Include/suTechnologyMonitorsManager.h>


// Static members initializations:
suBreakpointsManager* suBreakpointsManager::_pMySingleInstance = NULL;

#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    // Linux generic only stuff:
    #include <AMDTOSWrappers/Include/osCondition.h>

    // A condition object that blocks debugged threads from running
    // when the debugged process is suspended:
    static osCondition stat_isDebuggedProcessSuspendedCond;
#elif defined (_GR_IPHONE_DEVICE_BUILD)
    #include <AMDTServerUtilities/Include/suSpyBreakpointImplementation.h>
#endif


// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::instance
// Description: Returns the single instance of the suBreakpointsManager class
// Author:      Sigal Algranaty
// Date:        24/11/2009
// ---------------------------------------------------------------------------
suBreakpointsManager& suBreakpointsManager::instance()
{
    // If this class single instance was not already created:
    if (_pMySingleInstance == NULL)
    {
        // Create it:
        _pMySingleInstance = new suBreakpointsManager;
        GT_ASSERT(_pMySingleInstance);
    }

    return *_pMySingleInstance;
}


// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::suBreakpointsManager
// Description: Constructor
// Author:      Sigal Algranaty
// Date:        24/11/2009
// ---------------------------------------------------------------------------
suBreakpointsManager::suBreakpointsManager():
    _kernelSourceBreakpointsDirty(true),
    _breakDebuggedProcessExecution(false),
    _breakAtTheNextMonitoredFunctionCall(false),
    _breakAtTheNextDrawFunctionCall(false),
    _breakAtTheNextFrame(false),
    _breakInMonitoredFunction(false),
    _breakReason(AP_FOREIGN_BREAK_HIT),
    _breakpointTriggeringContextId(AP_OPENGL_CONTEXT, 0),
    _breakpointTriggeringFunctionId(apMonitoredFunctionsAmount),
    _functionToBeExecutedDuringBreak(NULL)
{
    // Clear all breakpoints:
    clearAllBreakPoints();
}


// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::~suBreakpointsManager
// Description: Destructor.
// Author:      Sigal Algranaty
// Date:        24/11/2009
// ---------------------------------------------------------------------------
suBreakpointsManager::~suBreakpointsManager()
{
}


// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::setBreakpointAtMonitoredFunction
// Description: Sets a breakpoint for function with the input id
// Arguments: int functionId
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/11/2009
// ---------------------------------------------------------------------------
bool suBreakpointsManager::setBreakpointAtMonitoredFunction(apMonitoredFunctionId functionId)
{
    bool retVal = false;

    // Verify that the function id is in the right range:
    GT_IF_WITH_ASSERT((0 <= functionId) && (functionId < apMonitoredFunctionsAmount))
    {
        _breakpointAtMonitoredFunction[functionId] = true;
        retVal = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::clearBreakpointAtMonitoredFunction
// Description: Clears a breakpoint for function with the input id
// Arguments: int functionId
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        24/11/2009
// ---------------------------------------------------------------------------
bool suBreakpointsManager::clearBreakpointAtMonitoredFunction(apMonitoredFunctionId functionId)
{
    bool retVal = false;

    // Verify that the function id is in the right range:
    GT_IF_WITH_ASSERT((0 <= functionId) && (functionId < apMonitoredFunctionsAmount))
    {
        _breakpointAtMonitoredFunction[functionId] = false;
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::setKernelSourceCodeBreakpoint
// Description: Sets a breakpoint in programHandle at lineNumber.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        1/11/2010
// ---------------------------------------------------------------------------
bool suBreakpointsManager::setKernelSourceCodeBreakpoint(oaCLProgramHandle programHandle, int lineNumber)
{
    bool retVal = true;

    // See if we have any breakpoints on this program:
    gtMap<oaCLProgramHandle, gtList<int> >::iterator mapFindIter = _kernelSourceBreakpoints.find(programHandle);
    gtMap<oaCLProgramHandle, gtList<int> >::iterator mapEndIter = _kernelSourceBreakpoints.end();

    if (mapFindIter == mapEndIter)
    {
        // Create a new list with the line number, and set it as the program's breakpoints list:
        gtList<int> breakpointsInProgram;
        breakpointsInProgram.push_back(lineNumber);
        _kernelSourceBreakpoints[programHandle] = breakpointsInProgram;
    }
    else // mapFindIter != mapEndIter
    {
        // Iterate the breakpoints list to see if this breakpoint is new:
        gtList<int>& breakpointsInProgram = _kernelSourceBreakpoints[programHandle];
        bool foundBreakpoint = false;
        gtList<int>::iterator listIter = breakpointsInProgram.begin();
        gtList<int>::iterator listEndIter = breakpointsInProgram.end();

        while (listIter != listEndIter)
        {
            // Check if the current breakpoint is the one we're looking for:
            if (*listIter == lineNumber)
            {
                foundBreakpoint = true;
                break;
            }

            listIter++;
        }

        if (!foundBreakpoint)
        {
            // This is a new breakpoint, add it:
            breakpointsInProgram.push_back(lineNumber);
        }
    }

    // We added a kernel source breakpoint, mark that the data has changed:
    _kernelSourceBreakpointsDirty = true;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::clearKernelSourceCodeBreakpoint
// Description: Clears the breakpoint in programHandle at lineNumber.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        1/11/2010
// ---------------------------------------------------------------------------
bool suBreakpointsManager::clearKernelSourceCodeBreakpoint(oaCLProgramHandle programHandle, int lineNumber)
{
    bool retVal = true;

    // See if we have any breakpoints on this program:
    gtMap<oaCLProgramHandle, gtList<int> >::iterator mapFindIter = _kernelSourceBreakpoints.find(programHandle);
    gtMap<oaCLProgramHandle, gtList<int> >::iterator mapEndIter = _kernelSourceBreakpoints.end();

    if (mapFindIter != mapEndIter)
    {
        // Iterate the breakpoints list to find if the breakpoint exists there:
        gtList<int>& breakpointsInProgram = _kernelSourceBreakpoints[programHandle];
        gtList<int>::iterator listIter = breakpointsInProgram.begin();
        gtList<int>::iterator listEndIter = breakpointsInProgram.end();

        while (listIter != listEndIter)
        {
            // Check if the current breakpoint is the one we're looking for:
            if (*listIter == lineNumber)
            {
                // TO_DO: check if this is the proper way:
                breakpointsInProgram.remove(lineNumber);
                break;
            }

            listIter++;
        }
    }

    // We removed a kernel source breakpoint, mark that the data has changed:
    _kernelSourceBreakpointsDirty = true;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::setKernelFunctionNameBreakpoint
// Description: Adds a breakpoint on kernels with the kernel function named.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        27/2/2011
// ---------------------------------------------------------------------------
bool suBreakpointsManager::setKernelFunctionNameBreakpoint(const gtString& kernelFuncName)
{
    bool retVal = true;

    // See if this kernel function is already set:
    bool wasFuncNameFound = false;
    int numberOfBreakpoints = (int)_kernelFunctionNameBreakpoints.size();

    for (int i = 0; i < numberOfBreakpoints; i++)
    {
        // Get each kernel name and compare it to our new name:
        if (kernelFuncName == _kernelFunctionNameBreakpoints[i])
        {
            // We found the function:
            wasFuncNameFound = true;
            break;
        }
    }

    // If this is a new kernel function name:
    if (!wasFuncNameFound)
    {
        // Add it:
        _kernelFunctionNameBreakpoints.push_back(kernelFuncName);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::setGenericBreakpoint
// Description: Adds a generic breakpoint
// Arguments:   apGenericBreakpointType breakpointType
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        7/7/2011
// ---------------------------------------------------------------------------
bool suBreakpointsManager::setGenericBreakpoint(apGenericBreakpointType breakpointType, bool isOn)
{
    bool retVal = true;

    // Set the flag for the specific generic breakpoint:
    _shouldBreakOnGeneric[(int)breakpointType] = isOn;

    if ((breakpointType == AP_BREAK_ON_GL_ERROR) || (breakpointType == AP_BREAK_ON_CL_ERROR))
    {
        // Notify the technology monitors we just resumed:
        suTechnologyMonitorsManager::instance().notifyMonitorsGenericBreakpointSet(breakpointType, isOn);
    }

#if (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)

    if (breakpointType == AP_BREAK_ON_DEBUG_OUTPUT)
    {
        // Notify the technology monitors we just resumed:
        suTechnologyMonitorsManager::instance().notifyMonitorsGenericBreakpointSet(breakpointType, isOn);
    }

#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::clearKernelFunctionNameBreakpoint
// Description: Clears the breakpoint on kernels with the kernel function named.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        27/2/2011
// ---------------------------------------------------------------------------
bool suBreakpointsManager::clearKernelFunctionNameBreakpoint(const gtString& kernelFuncName)
{
    bool retVal = true;

    // See if this kernel function is set:
    bool wasFuncNameFound = false;
    int numberOfBreakpoints = (int)_kernelFunctionNameBreakpoints.size();

    for (int i = 0; i < numberOfBreakpoints; i++)
    {
        if (wasFuncNameFound)
        {
            // If we got here, it means we went through the loop at least once, so i - 1 is a valid index:
            _kernelFunctionNameBreakpoints[i - 1] = _kernelFunctionNameBreakpoints[i];
        }
        else if (kernelFuncName == _kernelFunctionNameBreakpoints[i])
        {
            // Get each kernel name and compare it to our new name:
            wasFuncNameFound = true;
        }
    }

    // If we found the name:
    if (wasFuncNameFound)
    {
        // The last item in the vector is either our found name or a duplicate item:
        _kernelFunctionNameBreakpoints.pop_back();
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::clearAllBreakPoints
// Description: Clears all the registered breakpoints.
// Author:      Yaki Tebeka
// Date:        20/5/2004
// ---------------------------------------------------------------------------
void suBreakpointsManager::clearAllBreakPoints()
{
    for (int i = 0; i < apMonitoredFunctionsAmount; i++)
    {
        _breakpointAtMonitoredFunction[i] = false;
    }

    _kernelSourceBreakpoints.clear();
    _kernelSourceBreakpointsDirty = true;
    _kernelFunctionNameBreakpoints.clear();

    for (int i = (int)AP_BREAK_ON_GL_ERROR ; i < (int)AP_AMOUNT_OF_GENERIC_BREAKPOINT_TYPES ; i++)
    {
        _shouldBreakOnGeneric[i] = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::amountOfMonitoredFunctionBreakPoints
// Description: Returns the amount of breakpoints.
// Author:      Yaki Tebeka
// Date:        15/6/2004
// ---------------------------------------------------------------------------
int suBreakpointsManager::amountOfMonitoredFunctionBreakPoints() const
{
    int breakPointsAmount = 0;

    // Count the amount of monitored function breakpoints:
    for (int i = 0; i < apMonitoredFunctionsAmount; i++)
    {
        if (_breakpointAtMonitoredFunction[i] == true)
        {
            breakPointsAmount++;
        }
    }

    return breakPointsAmount;
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::shouldBreakAt
// Description: Returns true iff a breakpoint is set at functionId
// Author:      Uri Shomroni
// Date:        21/1/2010
// ---------------------------------------------------------------------------
bool suBreakpointsManager::shouldBreakAt(apMonitoredFunctionId functionId)
{
    bool retVal = false;

    // Verify that the function id is in the right range:
    if ((0 <= functionId) && (functionId < apMonitoredFunctionsAmount))
    {
        retVal = _breakpointAtMonitoredFunction[functionId];
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::getBreakpointsInProgram
// Description: Returns all the breakpoints set in programHandle into the vector
// Author:      Uri Shomroni
// Date:        1/11/2010
// ---------------------------------------------------------------------------
void suBreakpointsManager::getBreakpointsInProgram(oaCLProgramHandle programHandle, gtVector<int>& breakpointLineNumbers)
{
    // Clear the vector:
    breakpointLineNumbers.clear();

    // See if we have any breakpoints on this program:
    gtMap<oaCLProgramHandle, gtList<int> >::const_iterator mapFindIter = _kernelSourceBreakpoints.find(programHandle);
    gtMap<oaCLProgramHandle, gtList<int> >::const_iterator mapEndIter = _kernelSourceBreakpoints.end();

    if (mapFindIter != mapEndIter)
    {
        // Iterate the breakpoints list:
        const gtList<int>& breakpointsInProgram = _kernelSourceBreakpoints[programHandle];
        gtList<int>::const_iterator listIter = breakpointsInProgram.begin();
        gtList<int>::const_iterator listEndIter = breakpointsInProgram.end();

        while (listIter != listEndIter)
        {
            // Add the current breakpoint to the vector:
            breakpointLineNumbers.push_back(*listIter);

            listIter++;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::shouldBreakAtKernelFunction
// Description: Returns true iff the kernel function name supplied is set as a breakpoint
// Author:      Uri Shomroni
// Date:        27/2/2011
// ---------------------------------------------------------------------------
bool suBreakpointsManager::shouldBreakAtKernelFunction(const gtString& kernelFuncName)
{
    bool retVal = false;

    // See if this kernel function is set:
    int numberOfBreakpoints = (int)_kernelFunctionNameBreakpoints.size();

    for (int i = 0; i < numberOfBreakpoints; i++)
    {
        // Get each kernel name and compare it to our new name:
        if (kernelFuncName == _kernelFunctionNameBreakpoints[i])
        {
            // We found the function:
            retVal = true;
            break;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLMonitor::reportDetectedError
// Description: Reports a detected error to my debugger.
// Arguments:   errorCode - The detected error error code.
//              associatedFuncId - The id of a monitored function that is associated
//                                 with the function call (or -1 if no monitored function
//                                 is associated with this error.
//              errorDescription - The detected error description.
// Author:      Yaki Tebeka
// Date:        9/4/2005
// Implementation notes:
//   The string format of debug strings that will be interpreted as detected error events is:
//   AP_ERROR_MESSAGE_PREFIX [error code] [associated function id] [Error description]
// Author:      Yaki Tebeka
// Date:        21/4/2005
// ---------------------------------------------------------------------------
void suBreakpointsManager::reportDetectedError(apContextID triggeringContextId, apErrorCode errorCode, const gtString& errorDescription, apMonitoredFunctionId associatedFuncId)
{
    // We currently do not report AP_OBJECT_NAME_DOES_NOT_EXIST errors, since there are extensions
    // that might generate them (eg sharing shaders between contexts) - Uri 2/6/08 case 3776
    if (errorCode == AP_OBJECT_NAME_DOES_NOT_EXIST_ERROR)
    {
        return;
    }

    // We report errors only if we should break on detected errors:
    // (Otherwise, you can get zillions of detected error reports that will make the debugger crash)
    if (_shouldBreakOnGeneric[AP_BREAK_ON_DETECTED_ERROR])
    {
        // Log the detected error parameters:
        _detectedErrorParameters._detectedErrorCode = errorCode;
        _detectedErrorParameters._detectedErrorDescription = errorDescription;
        _detectedErrorParameters._detectedErrorAssociatedFunction = associatedFuncId;

        // If this is an OpenGL ES spy dll build:
#if (defined OS_OGL_ES_IMPLEMENTATION_DLL_BUILD) || (defined _GR_IPHONE_BUILD)
        // There is no immediate mode in OpenGL ES:
        bool isInGLBeginEndBlock = false;
#else
        // We don't know our status here, so we mark true to be on the safe side:
        bool isInGLBeginEndBlock = true;
#endif

        // Trigger a breakpoint exception:
        beforeTriggeringBreakpoint();
        setBreakReason(AP_DETECTED_ERROR_BREAKPOINT_HIT);
        triggerBreakpointException(triggeringContextId, associatedFuncId, isInGLBeginEndBlock);

        // Clear the detected error parameters:
        _detectedErrorParameters.clearParameters();
        afterTriggeringBreakpoint();
    }
    else // !_breakOnDetectedErrors
    {
        // Get the events pipe:
        osSocket* pEventsSocket = suSpyToAPIConnector::instance().eventForwardingSocket();
        GT_IF_WITH_ASSERT(pEventsSocket != NULL)
        {
            // Create an event and send it through the API pipe:
            apDetectedErrorParameters errorParams;
            errorParams._detectedErrorCode = errorCode;
            errorParams._detectedErrorDescription = errorDescription;
            errorParams._detectedErrorAssociatedFunction = associatedFuncId;
            apDebuggedProcessDetectedErrorEvent detectedErrorEvent(osGetCurrentThreadId(), errorParams, false);

            *pEventsSocket << detectedErrorEvent;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::testCommonFunctionExecutionBreakpoint
// Description: Tests if there is a breakpoint at the input monitored function.
//  If there is - triggers a breakpoint event.
//  This function tests for breakpoints that should be raised BEFORE the
//  monitored function is executed.
// Arguments: apMonitoredFunctionId monitoredFunctionId - the monitored function id
// Author:      Sigal Algranaty
// Date:        24/11/2009
// ---------------------------------------------------------------------------
void suBreakpointsManager::testCommonFunctionExecutionBreakpoint(apMonitoredFunctionId monitoredFunctionId, bool isInOpenGLBeginEndBlock)
{
    _breakReason = AP_FOREIGN_BREAK_HIT;

    // If the function is a breakpoint:
    if (_breakpointAtMonitoredFunction[monitoredFunctionId])
    {
        _breakReason = AP_MONITORED_FUNCTION_BREAKPOINT_HIT;
    }
    // Breakpoints by function type:
    else if (!isInOpenGLBeginEndBlock)
    {
        // Get the function type:
        unsigned int funcType = su_stat_theMonitoredFunMgr.monitoredFunctionType(monitoredFunctionId);

        if (_breakAtTheNextDrawFunctionCall == true)
        {
            // If the function is a draw function:
            if (funcType & AP_DRAW_FUNC)
            {
                // Set the break reason:
                _breakReason = AP_DRAW_MONITORED_FUNCTION_BREAKPOINT_HIT;

                // Clear the "break at next" flag:
                _breakAtTheNextDrawFunctionCall = false;
            }
        }

        // Deprecated function reporting is not supported in OpenGL ES:
#if !(defined(_GR_OPENGLES_COMMON) || defined(_GR_OPENGLES_IPHONE))

        if (_shouldBreakOnGeneric[AP_BREAK_ON_DEPRECATED_FUNCTION])
        {
            // Get the called function type:
            if (funcType & AP_DEPRECATED_FUNC)
            {
                // NOTICE: On analyze mode we check the function call deprecation status, so we do not need to check this out here!
                apExecutionMode debuggedProcessExecutionMode = suDebuggedProcessExecutionMode();

                if (debuggedProcessExecutionMode != AP_ANALYZE_MODE)
                {
                    // Set the break reason:
                    _breakReason = AP_DEPRECATED_FUNCTION_BREAKPOINT_HIT;
                }
            }
        }

#endif
    }

    // The break reasons below this point are weaker, so they should not be triggered if any other breakpoint occurs:
    if (_breakReason == AP_FOREIGN_BREAK_HIT)
    {
        // Break on next function call only if we have no other break reason:
        if (_breakAtTheNextMonitoredFunctionCall)
        {
            // Set the break reason:
            _breakReason = AP_NEXT_MONITORED_FUNCTION_BREAKPOINT_HIT;
        }
        // Break on "suspend application" only if no other reason or step was triggered:
        else if ((_breakDebuggedProcessExecution) && (!isInOpenGLBeginEndBlock))
        {
            // Set the break reason:
            _breakReason = AP_BREAK_COMMAND_HIT;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::testAndTriggerBeforeFuncExecutionBreakpoints
// Description:
//  Tests if there is a breakpoint at the input monitored function.
//  If there is - triggers a breakpoint event.
//
//  This function tests for breakpoints that should be raised BEFORE the
//  monitored function is executed.
// Arguments: apMonitoredFunctionId monitoredFunctionId - the monitored function id
// Author:      Yaki Tebeka
// Date:        16/6/2004
// ---------------------------------------------------------------------------
void suBreakpointsManager::testAndTriggerBeforeFuncExecutionBreakpoints(apMonitoredFunctionId monitoredFunctionId, apContextID contextId, bool isInOpenGLBeginEndBlock)
{
    // Check if this is a breakpoint:
    beforeTriggeringBreakpoint();

    testCommonFunctionExecutionBreakpoint(monitoredFunctionId, isInOpenGLBeginEndBlock);

    // If we "hit" a breakpoint:
    if (_breakReason != AP_FOREIGN_BREAK_HIT)
    {
        // Trigger a breakpoint exception:
        triggerBreakpointException(contextId, monitoredFunctionId, isInOpenGLBeginEndBlock);
    }

    afterTriggeringBreakpoint();
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::testAndTriggerProfileModeBreakpoints
// Description: Tests if there is a breakpoint for profiling mode
//  This function tests for breakpoints that should be raised BEFORE the
//  monitored function is executed.
// Arguments:   apMonitoredFunctionId monitoredFunctionId - the monitored function id
// Author:      Sigal Algranaty
// Date:        17/5/2010
// ---------------------------------------------------------------------------
void suBreakpointsManager::testAndTriggerProfileModeBreakpoints(apMonitoredFunctionId monitoredFunctionId, apContextID contextId)
{
    // Check if this is a breakpoint:
    // If there is a breakpoint at the called monitored function:
    beforeTriggeringBreakpoint();

    if (_breakDebuggedProcessExecution == true)
    {
        // Set the break reason:
        _breakReason = AP_BREAK_COMMAND_HIT;

        // Clear the "break debugged process execution" flag:
        _breakDebuggedProcessExecution = false;
    }

    // If we "hit" a breakpoint:
    if (_breakReason != AP_FOREIGN_BREAK_HIT)
    {
        // Trigger a breakpoint exception:
        triggerBreakpointException(contextId, monitoredFunctionId, false);
    }

    afterTriggeringBreakpoint();
}


// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::triggerBreakpointException
// Description: Allows clients to trigger a breakpoint exception.
// Author:      Yaki Tebeka
// Date:        8/3/2006
// ---------------------------------------------------------------------------
void suBreakpointsManager::triggerBreakpointException(apContextID contextId, apMonitoredFunctionId funcId, bool isInOpenGLBeginEndBlock)
{
    // Notify the technology monitors we are about to trigger a breakpoint:
    suTechnologyMonitorsManager& theTechnologyMonitorsManager = suTechnologyMonitorsManager::instance();
    theTechnologyMonitorsManager.notifyMonitorsBeforeBreakpointException(isInOpenGLBeginEndBlock);

    bool shouldLockCondition = (_breakReason != AP_BEFORE_KERNEL_DEBUGGING_HIT) && (_breakReason != AP_AFTER_KERNEL_DEBUGGING_HIT);

    if (shouldLockCondition)
    {
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
        // Lock all threads from running (except the Spy API thread):
        bool rc1 = stat_isDebuggedProcessSuspendedCond.lockCondition();
        GT_ASSERT(rc1)
#endif
    }

    // If this is a break reason that is not transparent to the user, we should clear the "next function" and
    // "suspend application" break flags, to avoid duplicate stops (case 7699):
    if ((_breakReason != AP_BEFORE_KERNEL_DEBUGGING_HIT) && (_breakReason != AP_AFTER_KERNEL_DEBUGGING_HIT) && (_breakReason != AP_FOREIGN_BREAK_HIT))
    {
        // Clear the "break at next" flag and "suspend application" flag:
        _breakAtTheNextMonitoredFunctionCall = false;
        _breakDebuggedProcessExecution = false;
    }

    // Output debug log:
    OS_OUTPUT_DEBUG_LOG(SU_STR_triggeringBreakpointException, OS_DEBUG_LOG_DEBUG);

    // Set the breakpoint context and function id:
    _breakpointTriggeringContextId = contextId;
    _breakpointTriggeringFunctionId = funcId;

    // Notify the technology monitors are going to be suspended:
    theTechnologyMonitorsManager.notifyMonitorsBeforeDebuggedProcessSuspended();

    // Trigger a breakpoint exception:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        // Yaki 14/2/2008:
        // Under windows, under release build configuration, when using osThrowBreakpointException()
        // pdWin32CallStackReader returns a call stack that contains only 1 stack frame (DbgBreak frame).
        // This is probably due to the fact that GROSWrappers is optimized in release configuration and
        // StackWalk64 does not manage to parse through it.
        // To solve the problem, we trigger the breakpoint exception here and don't use a GROSWrappers function.
#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
        __asm int 3;
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
        __debugbreak();
#else
#error Unknown address space size!
#endif
    }
#else
    {
#ifdef _GR_IPHONE_DEVICE_BUILD
        su_stat_spyBreakpointsImplemenation.breakAllThreads();
#else
        osThrowBreakpointException();
#endif
    }
#endif

    // Output debug log:
    OS_OUTPUT_DEBUG_LOG(SU_STR_passedBreakpointException, OS_DEBUG_LOG_DEBUG);

    if (shouldLockCondition)
    {
        waitOnDebuggedProcessSuspension();
    }

    handleFunctionExecutionDuringBreak();

    // Notify technology monitors we passed the breakpoint:
    theTechnologyMonitorsManager.notifyMonitorsAfterBreakpointException(isInOpenGLBeginEndBlock);

    // TO_DO: memory leaks: add comment Yaki:
    // Sigal 18/7/10
    // When we trigger a breakpoint exception after the API thread had exit, we don't want to reset the breakpoint
    // exception ??
    bool apiThreadExists = suIsAPIThreadRunning();
    bool isBreakingOnMemoryLeak = ((_breakReason == AP_MEMORY_LEAK_BREAKPOINT_HIT) && !apiThreadExists);

    if (!isBreakingOnMemoryLeak)
    {
        // Clear the break reason:
        _breakReason = AP_FOREIGN_BREAK_HIT;
    }
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::handleFunctionExecutionDuringBreak
// Description: Handles the _functionToBeExecutedDuringBreak member until we are released
// Author:      Uri Shomroni
// Date:        12/2/2010
// Implementation Notes: On Windows 7 64-bit, there are problem (probably a bug
//      in SetThreadContext() ), that cause crashes inside makeThreadExecuteFunction
//      function implementations (when using things like OpenGL get functions or
//      stl vectors). This is a workaround for those cases - note it can only
//      run functions on the thread that triggered the breakpoint.
// ---------------------------------------------------------------------------
void suBreakpointsManager::handleFunctionExecutionDuringBreak()
{
    // Loop until there is no function to execute and this thread is released:
    while (_functionToBeExecutedDuringBreak != NULL)
    {
        // Catch exceptions in the execution stubs:
        try
        {
            // Call the function, they are all of type "void gaXXXXCurrentThreadXXXXStub()".
            // We do not need to call suAfterDirectFunctionExecution() here, as the stub functions do it for us.
            ((void(*)())_functionToBeExecutedDuringBreak)();
        }
        catch (...)
        {
            // Output a log message:
            OS_OUTPUT_DEBUG_LOG(SU_STR_DebugLog_exceptionWhileExecutingDirectFunction, OS_DEBUG_LOG_ERROR);

            // Release the direct execution flag and throw another breakpoint:
            suAfterDirectFunctionExecution();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::breakDebuggedProcessExecution
// Description: Breaks the debugged process execution.
//              The break will occur on the next monitored function call.
// Author:      Yaki Tebeka
// Date:        3/10/2004
// ---------------------------------------------------------------------------
void suBreakpointsManager::breakDebuggedProcessExecution()
{
    _breakDebuggedProcessExecution = true;
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::resumeDebuggedProcessRun
// Description: Resumes the debugged process execution.
// Author:      Yaki Tebeka
// Date:        17/1/2007
// ---------------------------------------------------------------------------
bool suBreakpointsManager::resumeDebuggedProcessRun()
{
    bool retVal = false;

#if ((AMDT_BUILD_TARGET == AMDT_WINDOWS_OS) || ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)))

#ifdef _GR_IPHONE_DEVICE_BUILD
    retVal = su_stat_spyBreakpointsImplemenation.resumeAllThreads();
#else
    // All resume / suspend work is done on the debugger side.
    retVal = true;
#endif

#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))

    // Unlock the condition that prevents the debugged process threads from running:
    bool rc1 = stat_isDebuggedProcessSuspendedCond.unlockCondition();
    GT_IF_WITH_ASSERT(rc1)
    {
        // Notify the threads that the condition state was changed:
        bool rc2 = stat_isDebuggedProcessSuspendedCond.signalAllThreads();
        GT_IF_WITH_ASSERT(rc2)
        {
            retVal = true;
        }
    }

#else
#error Error: Unknown build target!
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::waitOnDebuggedProcessSuspension
// Description: On platforms where the debugger (or lack thereof) does not allow
//              us to suspend a specific thread (Linux, iPhone on device), this
//              causes a "thread suspension"-like delay.
//              On other platforms, this function does nothing.
// Author:      Uri Shomroni
// Date:        13/12/2009
// ---------------------------------------------------------------------------
void suBreakpointsManager::waitOnDebuggedProcessSuspension()
{
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    {
        // If the debugged process is suspended:
        if (stat_isDebuggedProcessSuspendedCond.isConditionLocked())
        {
            // Wait for the debugged process run to be resumed:
            stat_isDebuggedProcessSuspendedCond.waitForCondition();
        }
    }
#elif defined (_GR_IPHONE_DEVICE_BUILD)
    {
        su_stat_spyBreakpointsImplemenation.breakpointImplementation();
    }
#endif
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::setBreakpointAtNextMonitoredFunctionCall
// Description:
//   Causes a breakpoint exception at the next call to a monitored function.
//   This enables the implementation of "Single step" functionality.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/6/2004
// ---------------------------------------------------------------------------
void suBreakpointsManager::setBreakpointAtNextMonitoredFunctionCall()
{
    _breakAtTheNextMonitoredFunctionCall = true;
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::setBreakpointAtNextDrawFunctionCall
// Description:
//   Causes a breakpoint exception at the next draw function call.
//   This enables the implementation of "Draw step" functionality.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        15/6/2004
// ---------------------------------------------------------------------------
void suBreakpointsManager::setBreakpointAtNextDrawFunctionCall()
{
    _breakAtTheNextDrawFunctionCall = true;
}

bool suBreakpointsManager::getHSABreakpointsForKernel(const gtString& hsaKernelName, gtVector<gtUInt64>& bpLines) const
{
    bool retVal = false;

    if (!hsaKernelName.isEmpty())
    {
        auto findIter = m_hsaKernelSourceCodeBreakpoints.find(hsaKernelName);

        if (m_hsaKernelSourceCodeBreakpoints.end() != findIter)
        {
            const gtVector<gtUInt64>& bps = findIter->second;
            size_t bpCount = bps.size();
            bpLines.resize(bpCount);

            for (size_t i = 0; bpCount > i; ++i)
            {
                bpLines[i] = bps[i];
            }
        }

        // Also add a breakpoint at "line 0", if there's a kernel name breakpoint:
        if (((suBreakpointsManager*)this)->shouldBreakAtKernelFunction(hsaKernelName))
        {
            bpLines.push_back(0);
        }

        retVal = true;
    }

    return retVal;
}

bool suBreakpointsManager::addHSABreakpointForKernel(const gtString& hsaKernelName, const gtUInt64 bpLine)
{
    bool retVal = false;

    if (!hsaKernelName.isEmpty())
    {
        auto findIter = m_hsaKernelSourceCodeBreakpoints.find(hsaKernelName);

        if (m_hsaKernelSourceCodeBreakpoints.end() != findIter)
        {
            findIter->second.push_back(bpLine);
        }
        else // m_hsaKernelSourceCodeBreakpoints.end() == findIter
        {
            gtVector<gtUInt64> bps;
            m_hsaKernelSourceCodeBreakpoints[hsaKernelName] = bps;
            m_hsaKernelSourceCodeBreakpoints[hsaKernelName].push_back(bpLine);
        }

        retVal = true;
    }

    return retVal;
}

bool suBreakpointsManager::removeHSABreakpointFromKernel(const gtString& hsaKernelName, const gtUInt64 bpLine)
{
    bool retVal = false;

    if (!hsaKernelName.isEmpty())
    {
        auto findIter = m_hsaKernelSourceCodeBreakpoints.find(hsaKernelName);

        if (m_hsaKernelSourceCodeBreakpoints.end() != findIter)
        {
            gtVector<gtUInt64>& breakpoints = findIter->second;
            const size_t bpCount = breakpoints.size();
            bool found = false;

            for (size_t i = 0; bpCount > i; ++i)
            {
                if (found)
                {
                    // This cannot happen on the first iteration:
                    breakpoints[i - 1] = breakpoints[i];
                }
                else
                {
                    found = (breakpoints[i] == bpLine);
                }
            }

            if (found)
            {
                // Remove the last item, which is a duplicate or the item we looked for:
                breakpoints.pop_back();
                retVal = true;
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::setBreakInMonitoredFunctionCall
// Description: If the function we are currently stopped at allows stepping
//              into, sets / clears a breakpoint at the function's first line.
//              Otherwise, sets a breakpoint at the next function (same as
//              setBreakpointAtNextMonitoredFunctionCall).
// Author:      Uri Shomroni
// Date:        27/10/2010
// ---------------------------------------------------------------------------
void suBreakpointsManager::setBreakInMonitoredFunctionCall(bool breakInFunction, bool& replacedWithStepOver)
{
    replacedWithStepOver = false;

    if (breakInFunction)
    {
        // Get the Id of the last called function. If it is one of the functions that allows stepping into:
        if ((_breakpointTriggeringFunctionId == ap_clEnqueueNDRangeKernel) || (_breakpointTriggeringFunctionId == ap_clEnqueueTask))
        {
            // Set the "step into" flag:
            _breakInMonitoredFunction = true;
        }
        else
        {
            // If it is not a function that allows stepping into, perform a single step instead:
            _breakAtTheNextMonitoredFunctionCall = true;
            replacedWithStepOver = true;
        }
    }
    else
    {
        // Clear the flag:
        _breakInMonitoredFunction = false;
    }
}

// ---------------------------------------------------------------------------
// Name:        suBreakpointsManager::setBreakReason
// Description: Set the breakpoint manager
// Arguments:   apBreakReason reason
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        18/7/2010
// ---------------------------------------------------------------------------
void suBreakpointsManager::setBreakReason(apBreakReason reason)
{
    // Set the breakpoint reason:
    _breakReason = reason;
}
