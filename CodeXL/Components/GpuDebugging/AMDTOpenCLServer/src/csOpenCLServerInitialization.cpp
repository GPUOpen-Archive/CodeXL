//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csOpenCLServerInitialization.cpp
///
//==================================================================================

//------------------------------ csOpenCLServerInitialization.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osMessageBox.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTAPIClasses/Include/apiClassesInitFunc.h>
#include <AMDTAPIClasses/Include/apFrameTerminators.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suIKernelDebuggingManager.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>

// Currently only enable hardware debugging on Windows:
#if defined(CS_USE_HD_HSA_HW_BASED_DEBUGGER) && (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
    #include <AMDTHsaDebugging/Include/hdGlobalVariables.h>
    #include <AMDTHsaDebugging/Include/hdHSAHardwareBasedDebuggingManager.h>
#endif

#if (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    // Mac only:
    #include <AMDTServerUtilities/Include/suMacOSXInterception.h>
    #include <src/csMacOSXInterception.h>
#endif

// Local:
#include <src/csAMDKernelDebuggingManager.h>
#include <src/csAPIFunctionsImplementations.h>
#include <src/csAPIFunctionsStubs.h>
#include <src/csExtensionsManager.h>
#include <src/csGlobalVariables.h>
#include <src/csOpenCLMonitor.h>
#include <src/csOpenCLServerInitialization.h>
#include <src/csSpyToAPIConnector.h>
#include <src/csStringConstants.h>
#include <src/csWrappersCommon.h>


// ---------------------------------------------------------------------------
// Name:        csInitializeOpenCLServer
// Description: Initializes the OpenCL server.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
bool csInitializeOpenCLServer()
{
    static bool stat_retVal = false;

    // Verify that we are only initialized once:
    static bool stat_wasInitialized = false;

    if (!stat_wasInitialized)
    {
        stat_wasInitialized = true;

        // Log that the OpenCL Server is initializing:
        OS_OUTPUT_DEBUG_LOG(CS_STR_DebugLog_OpenCLServerInitializing, OS_DEBUG_LOG_INFO);

        // Initialize the OpenCL wrappers:
        bool rc1 = csInitializeWrapperFunctions();
        GT_IF_WITH_ASSERT(rc1)
        {
            stat_retVal = true;

            // Initialize the AMD kernel debugging facilities:
            // Uri, 4/9/12 - this has to be done before the OpenCL API init, since otherwise
            // the enable kernel debugging function fails.
            bool rcSWDbg = cs_stat_amdKernelDebuggingManager.initialize();
            cs_stat_pIKernelDebuggingManager = &cs_stat_amdKernelDebuggingManager;
            GT_ASSERT(rcSWDbg);

            // Currently only enable hardware debugging on Windows:
#if defined(CS_USE_HD_HSA_HW_BASED_DEBUGGER) && (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
            // Initialize the Hardware debugger (HSA) based debugger:
            // This must be done before any call to openCL (also csExtensionsManager::instance().initialize() below) because the hardware debugging
            // manager intercepts HSA calls which are also called via openCL initialization
            bool rcHWDbg = hd_stat_hsaHardwareBasedDebuggingManager.initialize();

            if (rcHWDbg)
            {
                OS_OUTPUT_DEBUG_LOG(L"HSA hardware based debugging manager initialized, setting it as default debugging manager", OS_DEBUG_LOG_INFO);
                cs_stat_pIKernelDebuggingManager = &hd_stat_hsaHardwareBasedDebuggingManager;
            }

#endif

            // Initialize the extensions manager:
            csExtensionsManager::instance().initialize();

            // If we are running in API mode:
            bool isRunningInStandaloneMode = suIsRunningInStandaloneMode();

            if (!isRunningInStandaloneMode)
            {
                // Register API Stub functions:
                csRegisterAPIStubFunctions();

                // Mark our API as active:
                suRegisterAPIConnectionAsActive(AP_OPENCL_API_CONNECTION);

                // Handle the OpenCL server API initialization calls:
                csHandleAPIInitializationCalls();
            }

            stat_retVal = true;
            // Initialize Mac interception:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
            {
                suBeforeInitializingMacOSXInterception();

                static bool isInterceptionInitialized = false;

                if (!isInterceptionInitialized)
                {
                    // Apply the function interception to all functions:
                    isInterceptionInitialized = csInitializeMacOSXOpenCLInterception();
                }

                suAfterInitializingMacOSXInterception();

                stat_retVal = isInterceptionInitialized;
            }
#endif // Mac interception
        }

        if (stat_retVal)
        {
            // Display initialization succeeded message:
            csDisplayInitializationSuccededMessage();
        }
        else
        {
            // Display a failure message and exit:
            csOnInitializationError();
        }

        // Log that the OpenGL Server initializing ended:
        OS_OUTPUT_DEBUG_LOG(CS_STR_DebugLog_OpenCLServerInitialionEnded, OS_DEBUG_LOG_INFO);
    }

    return stat_retVal;
}


// ---------------------------------------------------------------------------
// Name:        csTerminateOpenCLServer
// Description: Terminates the OpenCL Server.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
bool csTerminateOpenCLServer()
{
    bool retVal = true;

    // Clean up the OpenCL monitor:
    bool rcMon = csPerformOpenCLMonitorTerminationActions();

    // Unregister us from the "Active APIs" list:
    suRegisterAPIConnectionAsInactive(AP_OPENCL_API_CONNECTION);

    retVal = retVal && rcMon;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csPerformOpenCLMonitorTerminationActions
// Description: Performs all actions related to the OpenCL Monitor while the
//              OpenCL spy is being terminated. Since these actions require both
//              the monitor to be alive and the API connection to not have been
//              closed, this function will be called from the monitor's destructor
//              AND the module destructor, but the actions will only be performed
//              once.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        7/12/2010
// ---------------------------------------------------------------------------
bool csPerformOpenCLMonitorTerminationActions()
{
    bool retVal = true;

    static bool alreadyExecuted = false;

    if (!alreadyExecuted)
    {
        // Notify the OpenCL monitor that it is about to be deleted:
        cs_stat_openCLMonitorInstance.onDebuggedProcessTerminationAlert();

        // Report to the debugger, if we have one:
        bool isRunningInStandAloneMode = suIsRunningInStandaloneMode();

        if (!isRunningInStandAloneMode)
        {
            // If we need to report the debugged process debugger about the termination:
            bool reportToDebugger = !suIsTerminationInitiatedByAPI();

            if (reportToDebugger)
            {
                // If we are not in profile mode:
                apExecutionMode currExecMode = suDebuggedProcessExecutionMode();

                if (currExecMode != AP_PROFILING_MODE)
                {
                    // Check for memory leak for openCL spy:
                    cs_stat_openCLMonitorInstance.checkForProcessMemoryLeaks();
                }
            }
        }

        // Clear the reference to the debugging manager when it is terminated:
        cs_stat_pIKernelDebuggingManager = NULL;

        // Terminate the AMD kernel debugging facilities:
        cs_stat_amdKernelDebuggingManager.terminate();

        // Currently only enable hardware debugging on Windows:
#if defined(CS_USE_HD_HSA_HW_BASED_DEBUGGER) && (AMDT_BUILD_TARGET == AMDT_WINDOWS_OS)
        hd_stat_hsaHardwareBasedDebuggingManager.terminate();
#endif

        // Unregister us from the "Active APIs" list:
        suRegisterAPIConnectionAsInactive(AP_OPENCL_API_CONNECTION);

        alreadyExecuted = true;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csDisplayInitializationSuccededMessage
// Description: Outputs an initialization succeeded message to the debugger.
// Author:      Yaki Tebeka
// Date:        11/1/2010
// ---------------------------------------------------------------------------
void csDisplayInitializationSuccededMessage()
{
    osOutputDebugString(CS_STR_OpenCLServerInitializedSuccessfully);
}


// ---------------------------------------------------------------------------
// Name:        csOnInitializationError
// Description:
//   a. Displays an initialization error.
//   b. Exits the debugged application.
// Author:      Yaki Tebeka
// Date:        11/1/2010
// ---------------------------------------------------------------------------
void csOnInitializationError()
{
    // Write a failure message into the log file:
    OS_OUTPUT_DEBUG_LOG(CS_STR_DebugLog_OpenCLServerInitializationFailed, OS_DEBUG_LOG_ERROR);
    osOutputDebugString(CS_STR_DebugLog_OpenCLServerInitializationFailed);

    // Output a similar error message in a message box:
    gtString applicationName = L"UNKNOWN";
    osGetCurrentApplicationName(applicationName);
    gtString errMsg;
    errMsg.appendFormattedString(CS_STR_OpenCLServerInitializationFailureMessage, applicationName.asCharArray());
    osMessageBox messageBox(SU_STR_CodeXLError, errMsg.asCharArray(), osMessageBox::OS_STOP_SIGN_ICON);
    messageBox.display();

    // Exit the debugged application:
    osExitCurrentProcess(7);
}

