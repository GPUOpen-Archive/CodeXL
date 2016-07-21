//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file suAPIFunctionsImplementations.cpp
///
//==================================================================================

//------------------------------ suAPIFunctionsImplementations.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osCallsStackReader.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osRawMemoryBuffer.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osTime.h>
#include <AMDTAPIClasses/Include/apApiFunctionsInitializationData.h>
#include <AMDTAPIClasses/Include/apDetectedErrorParameters.h>
#include <AMDTAPIClasses/Include/apKernelFunctionNameBreakpoint.h>
#include <AMDTAPIClasses/Include/apKernelSourceCodeBreakpoint.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>

// Local:
#include <src/suAPIFunctionsImplementations.h>
#include <src/suSpiesUtilitiesDLLInitializationFunctions.h>
#include <src/suSpyToAPIConnector.h>

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    #include <AMDTServerUtilities/Include/suLinuxThrdsSuspender.h>
#endif
#include <AMDTServerUtilities/Include/suTechnologyMonitorsManager.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>


// ---------------------------------------------------------------------------
// Name:        suRestoreDebuggedProcessWorkDirecroty
// Description:
//   If the debugged process working directory was changed by pdWin32ProcessDebugger,
//   restore it.
//
// Arguments:
//  debuggedProcessWorkDir - The debugged process original working directory.
//
// Author:      Yaki Tebeka
// Date:        23/8/2007
//
// Implementation notes:
//   pdWin32ProcessDebugger::launchDebuggedProcess sometimes needs to change the
//   debugged process working directory to make the debugged process load spy DLLs.
//   (See "Loading the Spy dlls" comment at pdWin32ProcessDebugger::launchDebuggedProcess
//    for more details)
//   This function restores back the original debugged process work directory.
// ---------------------------------------------------------------------------
void suRestoreDebuggedProcessWorkDirecroty(const osDirectory& debuggedProcessWorkDir)
{
    (void)(debuggedProcessWorkDir); // unused
    // This function is relevant only on Windows:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        // Get the current work directory:
        wchar_t workDirBuff[MAX_PATH + 1];
        DWORD rc1 = ::GetCurrentDirectory(MAX_PATH, workDirBuff);
        GT_IF_WITH_ASSERT(rc1 != FALSE)
        {
            // Get the spies dir name:
            gtString spiesDir = L"\\";

#if AMDT_ADDRESS_SPACE_TYPE == AMDT_32_BIT_ADDRESS_SPACE
            {
                spiesDir += OS_SPIES_SUB_DIR_NAME;
            }
#elif AMDT_ADDRESS_SPACE_TYPE == AMDT_64_BIT_ADDRESS_SPACE
            {
                spiesDir += OS_SPIES_64_SUB_DIR_NAME;
            }
#else
            {
#error Unknown address space size!
            }
#endif

            // If the working directory contains the spies dir:
            gtString workDir = workDirBuff;
            workDir.toLowerCase();

            if (workDir.find(spiesDir) != -1)
            {
                // Restore the debugged process work directory to be the directory
                // stored at the init data:
                const osDirectory& workDirInner = debuggedProcessWorkDir;
                DWORD rc2 = ::SetCurrentDirectory(workDirInner.asString().asCharArray());
                GT_ASSERT(rc2 != FALSE);
            }
        }
    }
#endif
}


// ---------------------------------------------------------------------------
// Name:        gaIntializeAPIImpl
// Description: Implementation of gaIntializeAPI
// Arguments:   initData - API initialization data.
//              APIThreadId - Output parameter - the API thread id.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
bool gaIntializeAPIImpl(const apApiFunctionsInitializationData& initData)
{
    bool retVal = true;

    // Set the debugger install directory:
    const osFilePath& debuggerInstDir = initData.debuggerInstallDir();
    suSetDebuggerInstallDir(debuggerInstDir);

    // Set the OpenGL frame terminators:
    unsigned int frameTerminators = initData.openGLRenderFrameTerminators();
    suSetFrameTerminatorsMask(frameTerminators);

    // Set the logged textures file type:
    apFileType loggedTexturesFileType = initData.loggedTexturesFileType();
    suSetLoggedTexturesFileType(loggedTexturesFileType);

    // Set the logging limits:
    unsigned int maxOpenGLCallsPerContext = initData.maxLoggedOpenGLCallsPerContext();
    unsigned int maxOpenCLCallsPerContext = initData.maxLoggedOpenCLCallsPerContext();
    unsigned int maxOpenCLCommandPerQueue = initData.maxLoggedOpenCLCommandsPerQueue();
    suSetLoggingLimits(maxOpenGLCallsPerContext, maxOpenCLCallsPerContext, maxOpenCLCommandPerQueue);

    // On iPhone OS only:
#ifdef _GR_IPHONE_DEVICE_BUILD
    // On the iPhone, we can only write files to specific locations:
    osFilePath logFilesDirPath(osFilePath::OS_TEMP_DIRECTORY);
    gtString subdirectoryName = '-';

    osTime now;
    now.setFromCurrentTime();
    gtString currentDate, currentTime;
    now.dateAsString(currentDate, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);
    now.timeAsString(currentTime, osTime::UNDERSCORE_SAPERATOR, osTime::LOCAL);

    subdirectoryName.append(currentTime).prepend(currentDate);

    logFilesDirPath.appendSubDirectory(subdirectoryName);

    // Create the dir if it doesn't exist:
    if (!logFilesDirPath.exists())
    {
        osDirectory logFilesDir(logFilesDirPath);
        bool rcCreate = logFilesDir.create();

        if (!rcCreate)
        {
            // We couldn't create the subdirectory, just use the temporary dir itself:
            GT_ASSERT(rcCreate);
            logFilesDirPath.setPath(osFilePath::OS_TEMP_DIRECTORY);
        }
    }

#else
    const osFilePath& logFilesDirPath = initData.logFilesDirectoryPath();
#endif

    // Create the dir if it doesn't exist:
    if (!logFilesDirPath.exists())
    {
        osDirectory logFilesDir(logFilesDirPath);
        bool rcCreate = logFilesDir.create();
        GT_ASSERT(rcCreate);
    }

    // Set the log files directory:
    suSetCurrentSessionLogFilesDirectory(logFilesDirPath);

    // The project directory should be one level up from the log files directory:
    osDirectory currentProjectDirectory(logFilesDirPath);
    currentProjectDirectory.upOneLevel();

    // Make sure that the directory exists:
    GT_ASSERT(currentProjectDirectory.exists());

    // Set the log files directory:
    suSetCurrentProjectLogFilesDirectory(currentProjectDirectory.directoryPath());

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    {
        // If the working directory was changed by pdWin32ProcessDebugger,
        // restore it:
        suRestoreDebuggedProcessWorkDirecroty(initData.debuggedProcessWorkDir());
    }
#endif

    // If we are working with a debugger (not in a standalone mode), reaching here
    // means that the debugger is communicating with us:
    bool isRunningInStandaloneMode = suIsRunningInStandaloneMode();

    if (!isRunningInStandaloneMode)
    {
        // Output an "API connection to debugger process established" debug log printout:
        gtString dbgString = SU_STR_DebugLog_APIConnectionWasEstablished;
        OS_OUTPUT_DEBUG_LOG(dbgString.asCharArray(), OS_DEBUG_LOG_INFO);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetAPIThreadIdImpl
// Description:
//   Returns the Servers API thread id.
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
osThreadId gaGetAPIThreadIdImpl()
{
    osThreadId retVal = suSpiesAPIThreadId();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaBeforeTerminateDebuggedProcessImpl
// Description: Implementation of gaTerminateDebuggedProcess().
//              Terminated the process in which this DLL is running.
//              This part is called before reporting success to the client
// Author:      Uri Shomroni
// Date:        25/2/2014
// ---------------------------------------------------------------------------
void gaBeforeTerminateDebuggedProcessImpl()
{
    // Note that we are initializing this termination voluntarily:
    suSetTerminationInitiatedByAPI();

    // Alert about the process termination:
    // (No need to report to the debugged process debugger, since this is a termination
    //  asked by the debugger and not a normal termination)
    suReportDebuggedProcessTermination();
}

// ---------------------------------------------------------------------------
// Name:        gaTerminateDebuggedProcessImpl
// Description: Implementation of gaTerminateDebuggedProcessImpl().
//              Terminated the process in which this DLL is running.
// Author:      Yaki Tebeka
// Date:        25/10/2004
// ---------------------------------------------------------------------------
void gaTerminateDebuggedProcessImpl()
{
    // Terminate the spies utilities module:
    suTerminateSpiesUtilitiesModule();

    // Exit this process:
    // (The process return code will be 1)
    osProcessId currProcessId = osGetCurrentProcessId();
    osTerminateProcess(currProcessId, 1);
}


// ---------------------------------------------------------------------------
// Name:        gaSuspendDebuggedProcessImpl
// Description:
//   Implementation of gaSuspendDebuggedProcess()
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        3/10/2004
// ---------------------------------------------------------------------------
bool gaSuspendDebuggedProcessImpl()
{
    bool retVal = true;

    su_stat_theBreakpointsManager.breakDebuggedProcessExecution();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaResumeDebuggedProcessImpl
// Description:
//   Implementation of gaResumeDebuggedProcess()
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        17/1/2007
// ---------------------------------------------------------------------------
bool gaResumeDebuggedProcessImpl()
{
    bool retVal = false;

    retVal = su_stat_theBreakpointsManager.resumeDebuggedProcessRun();

    // Notify the technology monitors we just resumed:
    suTechnologyMonitorsManager::instance().notifyMonitorsAfterDebuggedProcessResumed();

    return retVal;
}

/////////////////////////////////////////////////////
/// \brief Suspend threads
///
/// \param thrds a vector of threads native handles
/// \return true - success / false - failed
/// \author Vadim Entov
/// \date 12/17/2015
bool gaSuspendThreadsImpl(const std::vector<osThreadId>& thrds)
{
    (void)(thrds);

#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    return suLinuxThrdsSuspender::getInstance().SuspendThreads(thrds);
#else
    return false;
#endif
}

//////////////////////////////////////////////////////////////////
/// \brief Resume threads. All previously suspended threads
///   stored into the internal structure
///
/// \return true - success / false - failed
/// \author Vadim Entov
/// \date 12/17/2015
bool gaResumeThreadsImpl()
{
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
    return suLinuxThrdsSuspender::getInstance().ResumeThreads();
#else
    return false;
#endif
}


// ---------------------------------------------------------------------------
// Name:        gaGetCurrentThreadCallStackImpl
// Description: Gets this thread's current calls stack
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        19/11/2009
// ---------------------------------------------------------------------------
bool gaGetCurrentThreadCallStackImpl(osCallStack& threadCallStack, bool hideSpyFunctions)
{
    osCallsStackReader stackReader;

    bool retVal = stackReader.getCurrentCallsStack(threadCallStack, hideSpyFunctions);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaSetDebuggedProcessExecutionModeImpl
// Description: Implementation of gaSetDeubggedProcessExecutionMode.
//              See its documentation for more details.
// Author:      Sigal Algranaty
// Date:        29/6/2008
// ---------------------------------------------------------------------------
bool gaSetDebuggedProcessExecutionModeImpl(apExecutionMode executionMode)
{
    suSetDebuggedProcessExecutionMode(executionMode);

    return true;
}

// ---------------------------------------------------------------------------
// Name:        gaSetBreakpointImpl
// Description:
//   Implementation of gaSetBreakpoint().
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        14/6/2004
// ---------------------------------------------------------------------------
bool gaSetBreakpointImpl(const apBreakPoint& breakpoint)
{
    bool retVal = false;

    osTransferableObjectType breakpointType = breakpoint.type();

    if (breakpointType == OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT)
    {
        // Get the monitored function id:
        const apMonitoredFunctionBreakPoint& monitoredFuncBreakPoint = (const apMonitoredFunctionBreakPoint&)breakpoint;
        apMonitoredFunctionId functionId = monitoredFuncBreakPoint.monitoredFunctionId();

        if (breakpoint.isEnabled())
        {
            // Set a breakpoint at the input monitored function:
            retVal = su_stat_theBreakpointsManager.setBreakpointAtMonitoredFunction(functionId);
        }
        else
        {
            // Set a breakpoint at the input monitored function:
            retVal = su_stat_theBreakpointsManager.clearBreakpointAtMonitoredFunction(functionId);
        }
    }
    else if (breakpointType == OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT)
    {
        const apKernelSourceCodeBreakpoint& kernelSourceBreakpoint = (const apKernelSourceCodeBreakpoint&)breakpoint;
        int lineNum = kernelSourceBreakpoint.lineNumber();

        if (kernelSourceBreakpoint.isHSAILBreakpoint())
        {
            const gtString& kernelName = kernelSourceBreakpoint.hsailKernelName();
            gtUInt64 lineNum64 = (gtUInt64)((unsigned int)lineNum);

            if (breakpoint.isEnabled())
            {
                retVal = su_stat_theBreakpointsManager.addHSABreakpointForKernel(kernelName, lineNum64);
            }
            else
            {
                retVal = su_stat_theBreakpointsManager.removeHSABreakpointFromKernel(kernelName, lineNum64);
            }
        }
        else // !kernelSourceBreakpoint.isHSAILBreakpoint()
        {
            oaCLProgramHandle progHandle = kernelSourceBreakpoint.programHandle();

            if (breakpoint.isEnabled())
            {
                // Set a breakpoint at the input location:
                retVal = su_stat_theBreakpointsManager.setKernelSourceCodeBreakpoint(progHandle, lineNum);
            }
            else
            {
                // Set a breakpoint at the input location:
                retVal = su_stat_theBreakpointsManager.clearKernelSourceCodeBreakpoint(progHandle, lineNum);
            }
        }
    }
    else if (breakpointType == OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT)
    {
        const apKernelFunctionNameBreakpoint& kernelFuncNameBreakpoint = (const apKernelFunctionNameBreakpoint&)breakpoint;
        const gtString& kernelFuncName = kernelFuncNameBreakpoint.kernelFunctionName();

        if (breakpoint.isEnabled())
        {
            // Set a breakpoint at the input name:
            retVal = su_stat_theBreakpointsManager.setKernelFunctionNameBreakpoint(kernelFuncName);
        }
        else
        {
            // Set a breakpoint at the input name:
            retVal = su_stat_theBreakpointsManager.clearKernelFunctionNameBreakpoint(kernelFuncName);
        }
    }
    else if (breakpointType == OS_TOBJ_ID_GENERIC_BREAKPOINT)
    {
        // Set a breakpoint at the input name:
        const apGenericBreakpoint& genericBreakpoint = (const apGenericBreakpoint&)breakpoint;
        retVal = su_stat_theBreakpointsManager.setGenericBreakpoint(genericBreakpoint.breakpointType(), breakpoint.isEnabled());
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaRemoveBreakpointImpl
// Description:
// Arguments: const apBreakPoint& breakpoint
// Return Val: bool  - Success / failure.
// Author:      Sigal Algranaty
// Date:        16/6/2008
// ---------------------------------------------------------------------------
bool gaRemoveBreakpointImpl(const apBreakPoint& breakpoint)
{
    bool retVal = false;

    osTransferableObjectType breakpointType = breakpoint.type();

    if (breakpointType == OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT)
    {
        // Get the monitored function id:
        apMonitoredFunctionBreakPoint& monitoredFuncBreakPoint = (apMonitoredFunctionBreakPoint&)breakpoint;
        apMonitoredFunctionId functionId = monitoredFuncBreakPoint.monitoredFunctionId();

        // Set a breakpoint at the input monitored function:
        retVal = su_stat_theBreakpointsManager.clearBreakpointAtMonitoredFunction(functionId);
    }
    else if (breakpointType == OS_TOBJ_ID_KERNEL_SOURCE_CODE_BREAKPOINT)
    {
        const apKernelSourceCodeBreakpoint& kernelSourceBreakpoint = (const apKernelSourceCodeBreakpoint&)breakpoint;
        oaCLProgramHandle progHandle = kernelSourceBreakpoint.programHandle();
        int lineNum = kernelSourceBreakpoint.lineNumber();

        // Set a breakpoint at the input location:
        retVal = su_stat_theBreakpointsManager.clearKernelSourceCodeBreakpoint(progHandle, lineNum);
    }
    else if (breakpointType == OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT)
    {
        const apKernelFunctionNameBreakpoint& kernelFuncNameBreakpoint = (const apKernelFunctionNameBreakpoint&)breakpoint;
        const gtString& kernelFuncName = kernelFuncNameBreakpoint.kernelFunctionName();

        // Clear the breakpoint at the input name:
        retVal = su_stat_theBreakpointsManager.clearKernelFunctionNameBreakpoint(kernelFuncName);
    }
    else if (breakpointType == OS_TOBJ_ID_GENERIC_BREAKPOINT)
    {
        const apGenericBreakpoint& genericBreakpoint = (const apGenericBreakpoint&)breakpoint;

        // Clear the breakpoint at the input name:
        retVal = su_stat_theBreakpointsManager.setGenericBreakpoint(genericBreakpoint.breakpointType(), false);
    }

    return retVal;
}
// ---------------------------------------------------------------------------
// Name:        gaRemoveAllBreakpointsImpl
// Description:
//   Implementation of gaRemoveAllBreakpoints().
//   See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        14/6/2004
// ---------------------------------------------------------------------------
bool gaRemoveAllBreakpointsImpl()
{
    bool retVal = true;

    // Remove all the breakpoints:
    su_stat_theBreakpointsManager.clearAllBreakPoints();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaBreakOnNextMonitoredFunctionCallImpl
// Description:
//   Implementation of gaBreakOnNextMonitoredFunctionCall().
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        15/6/2004
// ---------------------------------------------------------------------------
bool gaBreakOnNextMonitoredFunctionCallImpl()
{
    bool retVal = true;

    su_stat_theBreakpointsManager.setBreakpointAtNextMonitoredFunctionCall();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaBreakOnNextDrawFunctionCallImpl
// Description:
//   Implementation of gaBreakOnNextDrawFunctionCall().
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Avi Shapira
// Date:        25/5/2006
// ---------------------------------------------------------------------------
bool gaBreakOnNextDrawFunctionCallImpl()
{
    bool retVal = true;

    su_stat_theBreakpointsManager.setBreakpointAtNextDrawFunctionCall();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaBreakOnNextFrameImpl
// Description:
//   Implementation of gaBreakOnNextFrame().
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        8/4/2010
// ---------------------------------------------------------------------------
bool gaBreakOnNextFrameImpl()
{
    bool retVal = true;

    su_stat_theBreakpointsManager.setBreakAtTheNextFrame(true);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaBreakInMonitoredFunctionCallImpl
// Description:
//   Implementation of gaBreakPointInMonitoredFunctionCall().
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        27/10/2010
// ---------------------------------------------------------------------------
bool gaBreakInMonitoredFunctionCallImpl(bool& replacedWithStepOver)
{
    bool retVal = true;

    su_stat_theBreakpointsManager.setBreakInMonitoredFunctionCall(true, replacedWithStepOver);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaClearAllStepFlagsImpl
// Description:
//   Implementation of gaClearAllStepFlags().
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        29/2/2016
// ---------------------------------------------------------------------------
bool gaClearAllStepFlagsImpl()
{
    bool retVal = true;

    su_stat_theBreakpointsManager.clearAllStepFlags();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetDetectedErrorParametersImpl
// Description: Implementation of gaGetDetectedErrorParameters.
//              See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        8/10/2007
// ---------------------------------------------------------------------------
bool gaGetDetectedErrorParametersImpl(const apDetectedErrorParameters*& pDetectedErrorParameters)
{
    bool retVal = true;

    // Get the detected error parameters:
    pDetectedErrorParameters = &su_stat_theBreakpointsManager.getDetectedErrorPrameters();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetBreakReasonImpl
// Description:
//   Implementation of gaGetBreakReason().
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        24/8/2004
// ---------------------------------------------------------------------------
bool gaGetBreakReasonImpl(apBreakReason& breakReason)
{
    bool retVal = true;

    breakReason = su_stat_theBreakpointsManager.breakReason();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetBreakpointTriggeringContextIdImpl
// Description:
//   Implementation of gaGetBreakpointTriggeringContext()
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Sigal Algranaty
// Date:        25/11/2009
// ---------------------------------------------------------------------------
bool gaGetBreakpointTriggeringContextIdImpl(const apContextID*& pContextId)
{
    bool retVal = true;

    pContextId = &su_stat_theBreakpointsManager.breakpointTriggeringContextId();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaCreateEventForwardingTCPConnectionImpl
// Description: Implementation of gaCreateEventForwardingTCPConnection.
//              See its documentation for more details.
// Author:      Uri Shomroni
// Date:        29/10/2009
// ---------------------------------------------------------------------------
bool gaCreateEventForwardingTCPConnectionImpl(const osPortAddress& portAddress)
{
    bool retVal = false;

    retVal = suSpyToAPIConnector::instance().setEventForwardingSocket(portAddress);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaCreateEventForwardingPipeConnectionImpl
// Description: Implementation of gaCreateEventForwardingTCPConnection.
//              See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        16/12/2009
// ---------------------------------------------------------------------------
bool gaCreateEventForwardingPipeConnectionImpl(const gtString& eventsPipeName)
{
    bool retVal = false;

    retVal = suSpyToAPIConnector::instance().setEventForwardingPipe(eventsPipeName);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaGetAmountOfRegisteredAllocatedObjectsImpl
// Description:
//   Implementation of gaGetAmountOfRegisteredAllocatedObjects()
//   See its documentation for more details.
// Author:      Uri Shomroni
// Date:        12/11/2008
// ---------------------------------------------------------------------------
bool gaGetAmountOfRegisteredAllocatedObjectsImpl(unsigned int& amountOfAllocatedObjects)
{
    bool retVal = true;

    amountOfAllocatedObjects = su_stat_theAllocatedObjectsMonitor.numberOfAllocatedObjects();

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaGetAllocatedObjectCreationStackImpl
// Description:
//   Implementation of gaGetAllocatedObjectCreationStack()
//   See its documentation for more details.
// Author:      Uri Shomroni
// Date:        26/10/2008
// ---------------------------------------------------------------------------
bool gaGetAllocatedObjectCreationStackImpl(int allocatedObjectId, const osCallStack*& pCallsStack)
{
    bool retVal = false;

    // Get the creation stack of the item:
    retVal = su_stat_theAllocatedObjectsMonitor.getAllocatedObjectCreationCallStack(allocatedObjectId, pCallsStack);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaCollectAllocatedObjectsCreationCallsStacksImpl
// Description: Implementation of gaCollectAllocatedObjectsCreationCallsStacks.
//              See its documentation for more details.
// Arguments: bool collectCreationStacks
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        3/2/2009
// ---------------------------------------------------------------------------
bool gaCollectAllocatedObjectsCreationCallsStacksImpl(bool collectCreationStacks)
{
    // Collect the allocated creation call stacks:
    // We do not use the static reference here since this function is called during
    // the initialization API loop (which happens before the static members initializations
    // on Linux and Mac)
    suAllocatedObjectsMonitor::instance().collectAllocatedObjectsCreationCallsStacks(collectCreationStacks);

    return true;
}

// ---------------------------------------------------------------------------
// Name:        gaReadFileImpl
// Description: Implementation of gaReadFile.
//              See its documentation for more details.
// Author:      Uri Shomroni
// Date:        5/11/2009
// ---------------------------------------------------------------------------
bool gaReadFileImpl(const osFilePath& filePath, osRawMemoryBuffer& memoryBuffer)
{
    bool retVal = memoryBuffer.fromFile(filePath);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gaWriteFileImpl
// Description: Implementation of gaWriteFile.
//              See its documentation for more details.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        5/11/2009
// ---------------------------------------------------------------------------
bool gaWriteFileImpl(const osFilePath& filePath, const osRawMemoryBuffer& memoryBuffer)
{
    bool retVal = memoryBuffer.toFile(filePath);

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaSetSlowMotionDelayImpl
// Description: Implementation of gaSetSlowMotionDelay
//              See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        9/11/2004
// ---------------------------------------------------------------------------
bool gaSetSlowMotionDelayImpl(int delayTimeUnits)
{
    // Set the slow motion delay time units:
    suSetSlowMotionDelayTimeUnits(delayTimeUnits);

    return true;
}

// ---------------------------------------------------------------------------
// Name:        gaStartMonitoredFunctionsCallsLogFileRecordingImpl
// Description:
//   Implementation of gaStartMonitoredFunctionsCallsLogFileRecording().
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        17/8/2004
// ---------------------------------------------------------------------------
bool gaStartMonitoredFunctionsCallsLogFileRecordingImpl()
{
    bool retVal = true;

    // Start HTML log file recoding on spy utilities:
    suStartHTMLLogFileRecording();

    // Notify the technology monitors we just resumed:
    suTechnologyMonitorsManager::instance().notifyMonitorsStartMonitoredFunctionCallsLogFileRecording();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaStopMonitoredFunctionsCallsLogFileRecordingImpl
// Description:
//   Implementation of gaStopMonitoredFunctionsCallsLogFileRecording().
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        17/8/2004
// ---------------------------------------------------------------------------
bool gaStopMonitoredFunctionsCallsLogFileRecordingImpl()
{
    bool retVal = true;

    // Notify the technology monitors we just resumed:
    suTechnologyMonitorsManager::instance().notifyMonitorsStopMonitoredFunctionCallsLogFileRecording();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gaIsMonitoredFunctionsCallsLogFileRecordingActiveImpl
// Description:
//   Implementation of gaIsMonitoredFunctionsCallsLogFileRecordingActive().
//   See its documentation for more details.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        19/8/2004
// ---------------------------------------------------------------------------
bool gaIsMonitoredFunctionsCallsLogFileRecordingActiveImpl(bool& isActive)
{
    bool retVal = true;

    isActive = suAreHTMLLogFilesActive();

    return retVal;
}



// ---------------------------------------------------------------------------
// Name:        gaEnableImagesDataLoggingImpl
// Description: Implementation of gaEnableTexturesImageDataLogging.
//              See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        15/3/2005
// ---------------------------------------------------------------------------
bool gaEnableImagesDataLoggingImpl(bool isImagesDataLogged)
{
    // Get the state variable value:
    suEnableImagesDataLogging(isImagesDataLogged);
    return true;
}

// ---------------------------------------------------------------------------
// Name:        gaFlushLogFileAfterEachFunctionCallImpl
// Description: Implementation of gaEnableMonitoredFunctionCallsLogging()
//              See its documentation for more details.
// Author:      Yaki Tebeka
// Date:        3/3/2005
// ---------------------------------------------------------------------------
bool gaFlushLogFileAfterEachFunctionCallImpl(bool flushAfterEachFunctionCall)
{
    // Set the "flush after every function call" flag:
    suFlushLogFileAfterEachFunctionCall(flushAfterEachFunctionCall);
    return true;
}


