//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file gsWrappersCommon.cpp
///
//==================================================================================

//------------------------------ gsWrappersCommon.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osBundle.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/apFunctionType.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
#include <AMDTServerUtilities/Include/suTechnologyMonitorsManager.h>

// Local:
#include <src/gsStringConstants.h>
#include <src/gsMonitoredFunctionPointers.h>
#include <src/gsWrappersCommon.h>

// Contains true iff we should log the calls to initialization functions:
// (See gsEnableInitializationFunctionsLogging for more details)
static bool stat_areInitializationFunctionsLogged = true;

// TLS currently only supported on Linux:
#if AMDT_BUILD_TARGET == AMDT_LINUX_OS
#define GS_EXPORT_SERVER_TLS_VARIABLES 1
#endif

// ---------------------------------------------------------------------------
// Name:        gsGetBaseOpenGLFunctionTypes
// Description: Returns functions types that gsOpenGLWrappers wrappes.
//              These are generic (none extension) OGL functions and
//              platform connection (WGL / GLX) functions.
// Return Val:  unsigned int - A mask of apFunctionType enums.
// Author:      Yaki Tebeka
// Date:        21/11/2006
// ---------------------------------------------------------------------------
unsigned int gsGetBaseOpenGLFunctionTypes()
{
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    unsigned int retVal = AP_OPENGL_GENERIC_FUNC | AP_WGL_FUNC;
#elif AMDT_BUILD_TARGET == AMDT_LINUX_OS
#if AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT
    unsigned int retVal = AP_OPENGL_GENERIC_FUNC | AP_GLX_FUNC;
#elif AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT
#ifndef _GR_IPHONE_BUILD
    unsigned int retVal = AP_OPENGL_GENERIC_FUNC | AP_OPENGL_EXTENSION_FUNC;
#else
    unsigned int retVal = AP_OPENGL_ES_1_MAC_GENERIC_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC | AP_OPENGL_ES_MAC_EXTENSION_FUNC;
#endif
#else
#error Unknown Linux Variant!
#endif
#else
#error Error: unknown platform!
#endif

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsConnectOpenGLWrappers
// Description: Connects the wrapper functions to the real OpenGL functions
//              implementations.
// Arguments:   hSystemOpenGLModule - A handle to the system's OpenGL module.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        5/7/2003
// ---------------------------------------------------------------------------
bool gsConnectOpenGLWrappers(osModuleHandle hSystemOpenGLModule)
{
    bool retVal = true;

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_wrappingSystemOGLFunctions, OS_DEBUG_LOG_DEBUG);

    // Get the "base" OGL functions types mask:
    unsigned int baseOGLFunctionsTypes = gsGetBaseOpenGLFunctionTypes();

    // Get the Monitored functions manager instance:
    apMonitoredFunctionsManager& theMonitoredFunctionsManager = apMonitoredFunctionsManager::instance();

    // For each monitored function:
    int amountOfMonitoredFuncs = theMonitoredFunctionsManager.amountOfMonitoredFunctions();

    for (int i = 0; i < amountOfMonitoredFuncs; i++)
    {
        // Get the function type:
        unsigned int functionAPIType = theMonitoredFunctionsManager.monitoredFunctionAPIType((apMonitoredFunctionId)i);

        // If this is a "base" OGL function:
        if (functionAPIType & baseOGLFunctionsTypes)
        {
            bool functionOkay = true;

            // Get a the function name:
            gtString currFunctionName = theMonitoredFunctionsManager.monitoredFunctionName((apMonitoredFunctionId)i);

            // Get a pointer to the function implementation in the system's OGL module:
            osProcedureAddress pRealFunctionImplementation = NULL;
            bool rc = osGetProcedureAddress(hSystemOpenGLModule, currFunctionName.asASCIICharArray(), pRealFunctionImplementation, false);

            if (!rc)
            {
                bool shouldReportError = true;
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
                {
                    // On Linux, not all functions are supported by all variants, so it's alright if some
                    // are missing.
                }
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
                {
                    // On Mac, we only consider failure if a non-extension function is not found:
#ifndef _GR_IPHONE_BUILD
                    functionOkay = ((functionAPIType & AP_OPENGL_EXTENSION_FUNC) != 0);
#else
                    // The iPhone OS 3.0 OpenGLES library exports both OpenGL ES 1.1 and OpenGL ES 2.0 functions, but the older ones
                    // only export the 1.1 ones. So, to support both cases, we do not fail here if an OpenGL ES 2.0-only function is missing.
                    functionOkay = ((functionAPIType & (AP_OPENGL_ES_MAC_EXTENSION_FUNC | AP_OPENGL_ES_2_MAC_GENERIC_FUNC)) != 0);
#endif

                    if (functionOkay)
                    {
                        shouldReportError = false;
                    }
                }
#else
                {
                    // On Windows, failure to get a function pointers is considered a failure:
                    functionOkay = false;
                }
#endif

                if (shouldReportError)
                {
                    // Output an error message:
                    gtString errorMessage = GS_STR_DebugLog_cannotGetOGLFuncPtr;
                    errorMessage += currFunctionName;
                    GT_ASSERT_EX(false, errorMessage.asCharArray());
                }
            }

            // Connects gs_stat_realFunctionPointers[i] to point the real functions implementation:
            ((osProcedureAddress*)(&gs_stat_realFunctionPointers))[i] = pRealFunctionImplementation;

            retVal = retVal && functionOkay;
        }
    }

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_wrappingSystemOGLFunctionsEnded, OS_DEBUG_LOG_DEBUG);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsConnectDriverInternalFunctionPointers
// Description: Connects the driver internal functions to the real implementations.
// Arguments:   hSystemOpenGLModule - A handle to the system's OpenGL module.
// Author:      Uri Shomroni
// Date:        29/6/2016
// ---------------------------------------------------------------------------
void gsConnectDriverInternalFunctionPointers(osModuleHandle hSystemOpenGLModule)
{
    static bool callOnce = true;
    GT_ASSERT_EX(callOnce, L"gsConnectDriverInternalFunctionPointers called multiple times!");
    callOnce = false;

    // These number and array must be updated every time gsDriverInternalFunctionPointers is changed:
#define GS_NUMBER_OF_INTERNAL_FUNCTIONS 20
    const char* driverInternalFunctionNames[GS_NUMBER_OF_INTERNAL_FUNCTIONS] = {
        // The order in this array must be exactly like the entry point order in gsDriverInternalFunctionPointers:
        "loader_get_dispatch_table_size",
        "_loader_get_proc_offset",
        "_loader_add_dispatch",
        "_loader_set_dispatch",

        "_glapi_noop_enable_warnings",
        "_glapi_set_warning_func",
        "_glapi_check_multithread",
        "_glapi_set_context",
        "_glapi_get_context",
        "_glapi_set_dispatch",
        "_glapi_get_dispatch",
        "_glapi_begin_dispatch_override",
        "_glapi_end_dispatch_override",
        "_glapi_get_override_dispatch",
        "_glapi_get_dispatch_table_size",
        "_glapi_check_table",
        "_glapi_add_dispatch",
        "_glapi_get_proc_offset",
        "_glapi_get_proc_address",
        "_glapi_get_proc_name",
    };

    // Initialize the structure:
    ::memset(&gs_stat_realDriverInternalFunctionPointers, 0, sizeof(gsDriverInternalFunctionPointers));

    for (int i = 0; GS_NUMBER_OF_INTERNAL_FUNCTIONS > i; ++i)
    {
        // Get the functions pointer:
        osProcedureAddress pRealFunction = nullptr;
        bool rcPtr = osGetProcedureAddress(hSystemOpenGLModule, driverInternalFunctionNames[i], pRealFunction, false);
        if (rcPtr && (nullptr != pRealFunction))
        {
            // Place it in the structure:
            ((osProcedureAddress*)(&gs_stat_realDriverInternalFunctionPointers))[i] = pRealFunction;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        gsLoadSystemsOpenGLModule
// Description: Loads the system's OpenGL module into the calling process address space.
// Return Val:  osModuleHandle - Will get a handle to the systems OpenGL module.
// Author:      Yaki Tebeka
// Date:        5/7/2003
// ---------------------------------------------------------------------------
osModuleHandle gsLoadSystemsOpenGLModule()
{
    osModuleHandle retVal = OS_NO_MODULE_HANDLE;

    // Will get the system's OpenGL module path:
    gtVector<osFilePath> systemOGLModulePath;
    osGetSystemOpenGLModulePath(systemOGLModulePath);
    gtString moduleLoadError = L"System OpenGL module not found.";

    bool rc = false;
    int numberOfGLPaths = (int)systemOGLModulePath.size();
    GT_ASSERT(numberOfGLPaths > 0);

    for (int i = 0; (i < numberOfGLPaths) && (!rc); i++)
    {
        // Output debug log printout:
        const osFilePath& currentModulePath = systemOGLModulePath[i];
        const gtString& currentModulePathStr = currentModulePath.asString();
        gtString dbgLogMsg = GS_STR_DebugLog_loadingSystemOGLServer;
        dbgLogMsg.append(currentModulePathStr);
        OS_OUTPUT_DEBUG_LOG(dbgLogMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);

        // Load the system OpenGL module:
        if (currentModulePath.exists())
        {
            // Some of the paths may fail:
            gtString currentModuleError;
            rc = osLoadModule(currentModulePath, retVal, &currentModuleError, false);

            if (!rc)
            {
                // Log the error for each attempted path:
                moduleLoadError.append('\n').append(currentModulePathStr).append(L":\n    ").append(currentModuleError);
            }
            else
            {
                // Output debug log printout of the module that was successfully loaded:
                dbgLogMsg = GS_STR_DebugLog_systemOGLServerLoadedOk;
                dbgLogMsg.append(currentModulePathStr);
                OS_OUTPUT_DEBUG_LOG(dbgLogMsg.asCharArray(), OS_DEBUG_LOG_INFO);
            }
        }
    }

    // If we failed to load the system OpenGL module:
    if (!rc)
    {
        // Trigger an assertion failure:
        GT_ASSERT_EX(false, GS_STR_DebugLog_systemOGLServerLoadFailed);
        GT_ASSERT_EX(false, moduleLoadError.asCharArray());

        suTechnologyMonitorsManager::reportFailedSystemModuleLoad(moduleLoadError);
    }
    else
    {
        // Log the system's OpenGL module handle:
        gsSetSystemsOpenGLModuleHandle(retVal);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsUnloadSystemOpenGLModule
// Description: Unloads the loaded system's OpenGL module from this process address space.
// Author:      Yaki Tebeka
// Date:        5/7/2003
// ---------------------------------------------------------------------------
bool gsUnloadSystemOpenGLModule()
{
    bool retVal = false;

    // Get the loaded systems OpenGL module handle:
    osModuleHandle systemOGLModuleHandle = gsSystemsOpenGLModuleHandle();

    // If the system OGL module was loaded:
    if (systemOGLModuleHandle != NULL)
    {
        // Unload it:
        bool rc1 = osReleaseModule(systemOGLModuleHandle);
        GT_IF_WITH_ASSERT(rc1)
        {
            // Mark that the system's OpenGL module handle was released:
            gsSetSystemsOpenGLModuleHandle(NULL);

            retVal = true;
        }
    }

    return retVal;
}

// Mac OSX code only:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))

// ---------------------------------------------------------------------------
// Name:        gsConnectCGLWrappers
// Description: Connects the wrapper functions to the system's CGL functions
//              implementations.
// Arguments:   hSystemOpenGLFramework - A handle to the system's OpenGL framework.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        5/7/2003
// ---------------------------------------------------------------------------
bool gsConnectCGLWrappers(osModuleHandle hSystemOpenGLFramework)
{
    bool retVal = true;

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_wrappingSystemOGLFunctions, OS_DEBUG_LOG_DEBUG);

    // Get the Monitored functions manager instance:
    apMonitoredFunctionsManager& theMonitoredFunctionsManager = apMonitoredFunctionsManager::instance();

    // For each monitored function:
    int amountOfMonitoredFuncs = theMonitoredFunctionsManager.amountOfMonitoredFunctions();

    for (int i = 0; i < amountOfMonitoredFuncs; i++)
    {
        // Get the function type:
        unsigned int functionType = theMonitoredFunctionsManager.monitoredFunctionAPIType((apMonitoredFunctionId)i);

        // If this is a CGL function:
        if (functionType & AP_CGL_FUNC)
        {
            bool functionOkay = true;

            // Get a the function name:
            gtString currFunctionName = theMonitoredFunctionsManager.monitoredFunctionName((apMonitoredFunctionId)i);

            // Get a pointer to the function implementation in the system's OpenGL framework:
            osProcedureAddress pRealFunctionImplementation = NULL;
            bool rc = osGetProcedureAddress(hSystemOpenGLFramework, currFunctionName, pRealFunctionImplementation, false);

            if (!rc)
            {
                functionOkay = false;

                // Output an error message:
                gtString errorMessage = GS_STR_DebugLog_cannotGetOGLFuncPtr;
                errorMessage += currFunctionName;
                GT_ASSERT_EX(false, errorMessage.asCharArray());
            }

            // Connects gs_stat_realFunctionPointers[i] to point the real functions implementation:
            ((osProcedureAddress*)(&gs_stat_realFunctionPointers))[i] = pRealFunctionImplementation;

            retVal = retVal && functionOkay;
        }
    }

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(GS_STR_DebugLog_wrappingSystemCGLFunctionsEnded, OS_DEBUG_LOG_DEBUG);

    return retVal;
}


#endif // Mac OSX code only


// --------------------------------------------------------
//             Public functions
// --------------------------------------------------------


// ---------------------------------------------------------------------------
// Name:        gsInitializeWrapperFunctions
// Description: Initialize the OpenGL Wrappers package.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        5/7/2003
// ---------------------------------------------------------------------------
bool gsInitializeWrapperFunctions()
{
    static bool s_retVal = false;
    static bool s_isFirstCall = true;

    // If this is the first call to this function:
    if (s_isFirstCall)
    {
        s_isFirstCall = false;

        // Load the system's OpenGL module:
        osModuleHandle hSystemOpenGLModule = gsLoadSystemsOpenGLModule();
        GT_IF_WITH_ASSERT(hSystemOpenGLModule != NULL)
        {
            // Connect the OpenGL wrapper functions to the system's OpenGL functions:
            bool rc1 = gsConnectOpenGLWrappers(hSystemOpenGLModule);
            s_retVal = rc1;

            // Also connect the driver-internal functions. Since these are driver-dependant,
            // there is no return value:
            gsConnectDriverInternalFunctionPointers(hSystemOpenGLModule);
        }

        // On Mac OS X only:
#if ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
        {
#ifdef _GR_IPHONE_BUILD
            // Uri, 11/6/09: EAGL wrappers are found in the same library as the normal OpenGL ES ones.
#else

            if (s_retVal)
            {
                bool CGLWrappersConnected = false;

                // Connect the CGL wrapper functions to the system's CGL functions:
                bool rc2 = gsConnectCGLWrappers(hSystemOpenGLModule);
                GT_IF_WITH_ASSERT(rc2)
                {
                    CGLWrappersConnected = true;
                }

                s_retVal = CGLWrappersConnected;
            }

#endif
        }
#endif // Mac OS X only
    }

    return s_retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsTerminateWrapperFunctions
// Description: Terminate the OpenGL Wrappers package.
// Return Val:  bool - Success / failure.
// Author:      Yaki Tebeka
// Date:        5/7/2003
// ---------------------------------------------------------------------------
bool gsTerminateWrapperFunctions()
{
    // Unload the system's OpenGL module:
    bool retVal = gsUnloadSystemOpenGLModule();
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        gsGetSystemsOGLModuleProcAddress
// Description: Returns the address of a function that reside in the systems
//              OpenGL module.
// Arguments: procname - The queried function name.
// Return Val:  osProcedureAddress - The queried function address, or NULL in case of failure.
// Author:      Yaki Tebeka
// Date:        5/12/2006
// ---------------------------------------------------------------------------
osProcedureAddress gsGetSystemsOGLModuleProcAddress(const char* procname)
{
    osProcedureAddress retVal = NULL;

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
    retVal = (osProcedureAddress)(gs_stat_realFunctionPointers.wglGetProcAddress(procname));
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_GENERIC_LINUX_VARIANT))
    retVal = (osProcedureAddress)(gs_stat_realFunctionPointers.glXGetProcAddress((const GLubyte*)procname));
#elif ((AMDT_BUILD_TARGET == AMDT_LINUX_OS) && (AMDT_LINUX_VARIANT == AMDT_MAC_OS_X_LINUX_VARIANT))
    retVal = (osProcedureAddress)osGetOpenGLFrameworkFunctionAddress(procname);
#else
#error Unknown build target!
#endif

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        gsTextureCoordinateString
// Description: Translated GLenum that describes texture coordinates to
//              a string.
// Arguments:   coord - The input texture coordinate.
// Return Val:  const char* - The output string (or "Unknown in case of error).
// Author:      Yaki Tebeka
// Date:        13/9/2004
// Implementation Notes:
//   See comments at apGLenumParameter::valueAsString() implementation notes.
// ---------------------------------------------------------------------------
const char* gsTextureCoordinateString(GLenum coord)
{
    // The strings array:
    static const char* stat_stringsArr[5] =
    {
        "Unknown",
        "GL_S",
        "GL_T",
        "GL_R",
        "GL_Q"
    };

    int stringIndex = 0;

    if (coord == GL_S)
    {
        stringIndex = 1;
    }
    else if (coord == GL_T)
    {
        stringIndex = 2;
    }
    else if (coord == GL_R)
    {
        stringIndex = 3;
    }
    else if (coord == GL_Q)
    {
        stringIndex = 4;
    }

    return stat_stringsArr[stringIndex];
}


// ---------------------------------------------------------------------------
// Name:        gsEnableInitializationFunctionsLogging
// Description:
//  Causes the OpenGL server to log / do not log initialization function calls.
//  For example:
//  - Sometimes, we would like to call SwapBuffers ourselves, but since
//    SwapBuffers calls wglSwapBuffers, we need to disable the logging of the
//    wglSwapBuffers call.
//  - NVIDIA functions sometimes calls wglGetProcAddress and glGetString.
//    We would like to disable function calls logging while this happens.
//
// Arguments: isLoggingEnabled - true - to get into a mode where initialization functions are logged.
//                               false - to disable initialization function's logging.
// Author:      Yaki Tebeka
// Date:        16/9/2009
// ---------------------------------------------------------------------------
void gsEnableInitializationFunctionsLogging(bool isLoggingEnabled)
{
    stat_areInitializationFunctionsLogged = isLoggingEnabled;
}


// ---------------------------------------------------------------------------
// Name:        gsAreInitializationFunctionsLogged
// Description:
//  Returns true when initialization functions are logged, false otherwise
//  (For more details, see gsEnableInitializationFunctionsLogging)
//
// Author:      Yaki Tebeka
// Date:        16/9/2008
// ---------------------------------------------------------------------------
bool gsAreInitializationFunctionsLogged()
{
    return stat_areInitializationFunctionsLogged;
}

#ifdef GS_EXPORT_SERVER_TLS_VARIABLES
// These values are updated by the OpenGL runtimes after glXMakeCurrent is called. We need to
// update the corresponding values in the real libGL.so.1 each time that happens:
__thread struct _glapi_table *_glapi_tls_Dispatch __attribute__((tls_model("initial-exec"))) = nullptr;
__thread void * _glapi_tls_Context __attribute__((tls_model("initial-exec"))) = nullptr;
#endif

// ---------------------------------------------------------------------------
// Name:        gsUpdateTLSVariableValues
// Description: Updates the TLS variables to/from the real OpenGL driver
// Author:      Uri Shomroni
// Date:        5/7/2016
// ---------------------------------------------------------------------------
void gsUpdateTLSVariableValues()
{
#ifdef GS_EXPORT_SERVER_TLS_VARIABLES

    // The system module must already be loaded, but make sure anyway:
    osModuleHandle hSystemOpenGLModule = gsSystemsOpenGLModuleHandle();
    GT_IF_WITH_ASSERT(nullptr != hSystemOpenGLModule)
    {
        // Get the TLS location by calling dlsym:
        osProcedureAddress realTLSDispatchAsProcAddress = nullptr;
        bool rcDsp = osGetProcedureAddress(hSystemOpenGLModule, "_glapi_tls_Dispatch", realTLSDispatchAsProcAddress, false);

        if (rcDsp && (nullptr != realTLSContextAsProcAddress))
        {
            // Note that we do not check the return value as nullptr is a value that could appear:
            *(_glapi_table**)realTLSDispatchAsProcAddress = _glapi_tls_Dispatch;
        }

        // Get the TLS location by calling dlsym:
        osProcedureAddress realTLSContextAsProcAddress = nullptr;
        bool rcCtx = osGetProcedureAddress(hSystemOpenGLModule, "_glapi_tls_Context", realTLSContextAsProcAddress, false);

        if (rcCtx && (nullptr != realTLSContextAsProcAddress))
        {
            // Note that we do not check the return value as nullptr is a value that could appear:
            *(void**)realTLSContextAsProcAddress = _glapi_tls_Context;
        }
    }

#endif // GS_EXPORT_SERVER_TLS_VARIABLES
}

