//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file hsModuleInitializer.cpp
///
//==================================================================================


// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTServerUtilities/Include/suSpyAPIFunctions.h>

// Local:
#include <src/hsAPIFunctionsStubs.h>
#include <src/hsDebuggingManager.h>
#include <src/hsHSAMonitor.h>
#include <src/hsStringConstants.h>

#ifndef OS_MODULE_CONSTRUCTOR
    #define OS_MODULE_CONSTRUCTOR
#endif // OS_MODULE_CONSTRUCTOR

#ifndef OS_MODULE_DESTRUCTOR
    #define OS_MODULE_DESTRUCTOR
#endif // OS_MODULE_DESTRUCTOR

static void OS_MODULE_CONSTRUCTOR hsInitializeHSAServer()
{
    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogInitModuleStart, OS_DEBUG_LOG_DEBUG);

    // Initialize HSA monitor:
    hsHSAMonitor::instance();

    // Register API functions:
    hsRegisterAPIStubFunctions();

    // Mark our API as active:
    suRegisterAPIConnectionAsActive(AP_HSA_API_CONNECTION);

    // Intercept HSA:
    hsDebuggingManager::instance().InitializeInterception();

    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogInitModuleEnd, OS_DEBUG_LOG_DEBUG);
}

static void OS_MODULE_DESTRUCTOR hsTerminateHSAServer()
{
    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogUninitModuleStart, OS_DEBUG_LOG_DEBUG);

    // Clear the interception:
    hsDebuggingManager::instance().UninitializeInterception();

    // Unregister us from the "Active APIs" list:
    suRegisterAPIConnectionAsInactive(AP_HSA_API_CONNECTION);

    OS_OUTPUT_DEBUG_LOG(HS_STR_debugLogUninitModuleEnd, OS_DEBUG_LOG_DEBUG);
}

#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    GT_UNREFERENCED_PARAMETER(hModule);
    GT_UNREFERENCED_PARAMETER(lpReserved);

    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_DETACH:
        {
            hsTerminateHSAServer();
        }
        break;

        case DLL_PROCESS_ATTACH:
        {
            hsInitializeHSAServer();
        }
        break;

        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;
}

#endif // AMDT_BUILD_TARGET == AMDT_WINDOWS_OS

