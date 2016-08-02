//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsOpenGLSpyInitFuncs.cpp
///
//==================================================================================

//------------------------------ gsOpenGLSpyInitFuncs.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osBundle.h>
#include <AMDTOSWrappers/Include/osDebuggingFunctions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osMachine.h>
#include <AMDTOSWrappers/Include/osMessageBox.h>
#include <AMDTOSWrappers/Include/osPortAddress.h>
#include <AMDTOSWrappers/Include/osProcess.h>
#include <AMDTOSWrappers/Include/osThread.h>
#include <AMDTOSWrappers/Include/osProductVersion.h>
#include <AMDTOSAPIWrappers/Include/oaDisplay.h>
#include <AMDTAPIClasses/Include/apiClassesInitFunc.h>
#include <AMDTAPIClasses/Include/apFileType.h>
#include <AMDTAPIClasses/Include/apFrameTerminators.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>
#include <AMDTServerUtilities/Include/suStringConstants.h>

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    // Windows only:
    #define WIN32_LEAN_AND_MEAN 1
    #include "Windows.h"
    #include <ddraw.h>
#elif (AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT)
    // Mac only:
    #include <AMDTServerUtilities/Include/suMacOSXInterception.h>
    #include <src/gsMacOSXInterception.h>
#endif

// EGL:
#ifdef OS_OGL_ES_IMPLEMENTATION_DLL_BUILD
    #include <src/eglInitFunc.h>
#endif

// Local:
#include <src/gsAPIFunctionsStubs.h>
#include <src/gsAPIFunctionsImplementations.h>
#include <src/gsExtensionsManager.h>
#include <src/gsGLDebugOutputManager.h>
#include <src/gsOpenGLMonitor.h>
#include <src/gsOpenGLSpyInitFuncs.h>
#include <src/gsSingletonsDelete.h>
#include <src/gsStringConstants.h>
#include <src/gsWrappersCommon.h>
#include <AMDTServerUtilities/Include/suSWMRInstance.h>

// Desktop OpenGL Servers only:
#ifdef _AMDT_OPENGLSERVER_EXPORTS
    #include <src/gsDeprecationAnalyzer.h>
#endif


// Windows only code:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
// ---------------------------------------------------------------------------
// Name:        gsIntializeDirectDraw
//
// Description:
//  Initializes the direct draw library. This imitates actions done by the real OpenGL server.
//  Few debugged applications crash if we don't initialize DirectDraw.
//  This crash seems to happen at applications that their DLLMain function uses DirectDraw
//  but do not initialize it (they relay on the OpenGL32.dll DLLMain function to initialize DirectDraw)
//  (See cases 4529 and 4982)
//
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        10/3/2009
//
// Implementation note: (Sigal 20/10/2009)
//              On Vista, on ATI new graphic cards, the function DirectDrawCreate does not return.
//              Therefore, we skip this function call on ATI graphic cards.
//              This bug is reproduced with the following card details:
//              Graphics Chipset: ATI FireGL V7600
//              Driver Packaging Version: 8.634-090729b1-086101C-ATI
// ---------------------------------------------------------------------------
bool gsIntializeDirectDraw()
{
    bool retVal = false;

    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_directDrawStart, OS_DEBUG_LOG_DEBUG);

    bool shouldInitializeDirectDraw = false;

    // Check the environment variable:
    gtString ddrawEnvVarValue;
    bool rcDDrawEnv = osGetCurrentProcessEnvVariableValue(GS_STR_envVar_initializeDirectDrawLibrary, ddrawEnvVarValue);

    if (rcDDrawEnv)
    {
        // If this environment variable is set to TRUE
        if (ddrawEnvVarValue == GS_STR_envVar_valueTrue)
        {
            shouldInitializeDirectDraw = true;
        }
        else // ddrawEnvVarValue != GS_STR_envVar_valueTrue
        {
            // If this variable is set at all, we expect it to be TRUE or FALSE:
            GT_ASSERT(ddrawEnvVarValue == GS_STR_envVar_valueFalse);
        }
    }

    // If the environment variable is set, make sure the display device supports enabling ddraw:
    if (shouldInitializeDirectDraw)
    {
        // Get the Monitor name
        gtString deviceName;
        gtString monitorName;
        OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_directDrawCheckDisplayMonitor, OS_DEBUG_LOG_DEBUG);
        bool rc = oaGetDisplayMonitorInfo(deviceName, monitorName);
        GT_IF_WITH_ASSERT(rc)
        {
            if (deviceName.startsWith(L"ATI "))
            {
                shouldInitializeDirectDraw = false;
                OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_directDrawIgnoreATI, OS_DEBUG_LOG_DEBUG);
            }
        }
    }

    // If direct draw initialization is possible for this card:
    if (shouldInitializeDirectDraw)
    {
        // Initialize COM facilities:
        ::CoInitialize(NULL);

        // Create an instance of the DirectDraw object:
        IDirectDraw* pIDirectDraw = NULL;

        // Print log messages, since this function can halt:
        OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_directDrawBeforeDirectDrawCreate, OS_DEBUG_LOG_DEBUG);
        HRESULT rc1 = ::DirectDrawCreate(NULL, &pIDirectDraw, NULL);
        OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_directDrawAfterDirectDrawCreate, OS_DEBUG_LOG_DEBUG);
        GT_IF_WITH_ASSERT(rc1 == DD_OK)
        {
            // Release the instance we just created:
            pIDirectDraw->Release();

            retVal = true;
        }
    }
    else // !shouldInitializeDirectDraw
    {
        OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_directDrawDidNotInitialize, OS_DEBUG_LOG_DEBUG);
        retVal = true;
    }

    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_directDrawEnd, OS_DEBUG_LOG_DEBUG);

    return retVal;
}
#endif // Windows only code


// Mac only code
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

// ---------------------------------------------------------------------------
// Name:        gsPerformMacInterceptionActions
// Description: Performs Mac OpenGL and EGL functions interception.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        12/12/2010
// ---------------------------------------------------------------------------
bool gsPerformMacInterceptionActions()
{
    bool retVal = true;

    suBeforeInitializingMacOSXInterception();

    static bool isInterceptionInitialized = false;

    if (!isInterceptionInitialized)
    {
        // Apply the function interception to all functions:
        isInterceptionInitialized = gsInitializeMacOSXOpenGLInterception();
        isInterceptionInitialized = gsInitializeEAGLInterception() && isInterceptionInitialized;
    }

    suAfterInitializingMacOSXInterception();

    GT_RETURN_WITH_ASSERT(isInterceptionInitialized);
}

#endif // Mac only code




// ---------------------------------------------------------------------------
// Name:        gsDisplayInitializationSuccededMessage
// Description: Outputs an initialization succeeded message into the debugger.
// Author:      Yaki Tebeka
// Date:        6/10/2004
// ---------------------------------------------------------------------------
void gsDisplayInitializationSuccededMessage()
{
    osOutputDebugString(GS_STR_OpenGLServerInitializedSuccessfully);
}


// ---------------------------------------------------------------------------
// Name:        gsOnInitializationError
// Description:
//   a. Displays an initialization error.
//   b. Exits the debugged application.
// Author:      Yaki Tebeka
// Date:        6/10/2004
// ---------------------------------------------------------------------------
void gsOnInitializationError()
{
    // Write a failure message into the log file:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_OpenGLServerInitializationFailed, OS_DEBUG_LOG_ERROR);
    osOutputDebugString(GS_STR_DebugLog_OpenGLServerInitializationFailed);

    // Output a similar error message in a message box:
    gtString applicationName = L"UNKNOWN";
    osGetCurrentApplicationName(applicationName);
    gtString errMsg;
    errMsg.appendFormattedString(GS_STR_OpenGLServerInitializationFailureMessage, applicationName.asCharArray());
    osMessageBox messageBox(SU_STR_CodeXLError, errMsg.asCharArray(), osMessageBox::OS_STOP_SIGN_ICON);
    messageBox.display();

    // Exit the debugged application:
    osExitCurrentProcess(6);
}


// ---------------------------------------------------------------------------
// Name:        gsSetOpenGLESFrameworkPathFromEnvironmentVariable
// Description: Gets the value of the GS_OPENGL_ES_FRAMEWORK_PATH environment
//              variable and sets it as OpenGL ES framework path
// Return Val:  bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        8/6/2009
// ---------------------------------------------------------------------------
bool gsSetOpenGLESFrameworkPathFromEnvironmentVariable()
{
#ifdef _GR_IPHONE_BUILD
#ifdef _GR_IPHONE_DEVICE_BUILD
    bool retVal = true;

    // On the iPhone device, the OpenGL ES framework is always in the /System/Library/Frameworks/ dir:
    osSetOpenGLESFrameworkPath(GS_STR_iPhoneDeviceOpenGLESFrameworkPath);
#else // ndef _GR_IPHONE_DEVICE_BUILD
    bool retVal = false;

    // This path is only needed on the iPhone Simulator:
    gtString frameworkPath;
    bool rcEnvVar = osGetCurrentProcessEnvVariableValue(GS_STR_envVar_openglesFrameworkPath, frameworkPath);

    if (rcEnvVar)
    {
        osDirectory frameworkPathAsDir;
        frameworkPathAsDir.setDirectoryFullPathFromString(frameworkPath);

        if (frameworkPathAsDir.exists() && !frameworkPath.isEmpty())
        {
            osSetOpenGLESFrameworkPath(frameworkPath.asCharArray());
            retVal = true;
        }
        else
        {
            gtString errMsg = GS_STR_OpenGLESFrameworkPathNotValid;
            errMsg.append('\"').append(frameworkPath).append('\"');
            GT_ASSERT_EX(false, errMsg.asCharArray());
        }
    }
    else
    {
        GT_ASSERT_EX(rcEnvVar, GS_STR_OpenGLESFrameworkPathNotSet);
    }

#endif // _GR_IPHONE_DEVICE_BUILD
#else // ndef _GR_IPHONE_BUILD
    bool retVal = true;
#endif // _GR_IPHONE_BUILD

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsOpenGLSpyInit
// Description: Initialize this OpenGL spy dll.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        3/6/2004
// ---------------------------------------------------------------------------
bool gsOpenGLSpyInit()
{
    static bool stat_retVal = false;

    suSWMRInstance::SetUnlockMode();

    // Verify that we are only initialized once:
    static bool stat_wasInitialized = false;

    if (!stat_wasInitialized)
    {
        stat_wasInitialized = true;

        // Log that the OpenGL Server is initializing:
        OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_OpenGLServerInitializing, OS_DEBUG_LOG_INFO);

        // Yes - we have a valid license:
        stat_retVal = true;

        // On Windows only:
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        {
            // Initialize the direct draw library:
            bool ddrawInit = gsIntializeDirectDraw();
            GT_ASSERT(ddrawInit);
        }
#endif

#ifdef _GR_IPHONE_BUILD
        {
            // If we're in the iPhone, we need to read the OpenGL ES framework path
            stat_retVal = gsSetOpenGLESFrameworkPathFromEnvironmentVariable();
        }
#endif

        // Initialize the OpenGL wrappers:
        stat_retVal = stat_retVal && gsInitializeWrapperFunctions();
        GT_IF_WITH_ASSERT(stat_retVal)
        {
            // If we are building an OpenGL ES implementation -
            // initialize the EGL Library implementation:
#ifdef OS_OGL_ES_IMPLEMENTATION_DLL_BUILD
            {
                eglInitFunc();
            }
#endif

            // Initialize the extensions manager:
            stat_retVal = gsExtensionsManager::instance().initialize();

            GT_IF_WITH_ASSERT(stat_retVal)
            {

                // Desktop OpenGL Servers only:
#ifdef _AMDT_OPENGLSERVER_EXPORTS
                {
                    // Initialize the deprecation analyzer:
                    gsDeprecationAnalyzer::instance().initialize();
                }
#endif

                // If we are running in API mode:
                bool isRunningInStandaloneMode = suIsRunningInStandaloneMode();

                if (!isRunningInStandaloneMode)
                {
                    // Register API Stub functions:
                    gsRegisterAPIStubFunctions();

                    // Mark our API as active:
                    suRegisterAPIConnectionAsActive(AP_OPENGL_API_CONNECTION);

                    // Handle the API initialization calls:
                    gsHandleAPIInitializationCalls();
                }

                // Initialize Mac functions interception:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                {
                    stat_retVal = gsPerformMacInterceptionActions();
                }
#endif
            }
        }

        if (stat_retVal)
        {
            // Display initialization succeeded message:
            gsDisplayInitializationSuccededMessage();
        }
        else
        {
            // Display a failure message and exit:
            gsOnInitializationError();
        }

        // Log that the OpenGL Server initializing ended:
        OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_OpenGLServerInitialionEnded, OS_DEBUG_LOG_INFO);
    }

    return stat_retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsOpenGLSpyTerminate
// Description: Terminate this OpenGL spy dll
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        3/6/2004
// ---------------------------------------------------------------------------
bool gsOpenGLSpyTerminate()
{
    bool retVal = false;

    // Output debug log:
    OS_OUTPUT_DEBUG_LOG(GS_STR_OpenGLServerIsTerminating, OS_DEBUG_LOG_INFO);

    // If we need to report the debugged process debugger about the termination:
    bool reportToDebugger = !suIsTerminationInitiatedByAPI();

    // Clean up the OpenGL monitor:
    bool rcMon = gsPerformOpenGLMonitorTerminationActions();

    // Unregister us from the "Active APIs" list:
    suRegisterAPIConnectionAsInactive(AP_OPENGL_API_CONNECTION);

    // Perform this even in standalone mode:
    if (reportToDebugger)
    {
        // Terminate the wrapper functions:
        retVal = gsTerminateWrapperFunctions();

        if (!retVal)
        {
            OS_OUTPUT_DEBUG_LOG(GS_STR_FailedToTerminateOGLWrappers, OS_DEBUG_LOG_ERROR);
        }
    }
    else
    {
        retVal = true;
    }

    // Delete this DLL singletons:
    gsSingletonsDelete singeltonDeleter;
    singeltonDeleter.deleteSingeltonObjects();

    retVal = retVal && rcMon;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsPerformOpenGLMonitorTerminationActions
// Description: Performs all actions related to the OpenGL Monitor while the
//              OpenGL spy is being terminated. Since these actions require both
//              the monitor to be alive and the API connection to not have been
//              closed, this function will be called from the monitor's destructor
//              AND the module destructor, but the actions will only be performed
//              once.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        7/12/2010
// ---------------------------------------------------------------------------
bool gsPerformOpenGLMonitorTerminationActions()
{
    bool retVal = true;

    static bool alreadyExecuted = false;

    if (!alreadyExecuted)
    {
        alreadyExecuted = true;

        // Notify the OpenGL monitor it is about to be destroyed:
        gs_stat_openGLMonitorInstance.onDebuggedProcessTerminationAlert();

        // Report to the debugger, if we have one:
        bool isRunningInStandAloneMode = suIsRunningInStandaloneMode();

        // If we need to report the debugged process debugger about the termination:
        bool reportToDebugger = !suIsTerminationInitiatedByAPI();

        if (!isRunningInStandAloneMode)
        {
            if (reportToDebugger)
            {
                // If we are not in profile mode:
                apExecutionMode currExecMode = suDebuggedProcessExecutionMode();

                if (currExecMode != AP_PROFILING_MODE)
                {
                    // Check for memory leak for openGL spy:
                    gs_stat_openGLMonitorInstance.checkForProcessMemoryLeaks();
                }
            }
        }
    }

    return retVal;
}



