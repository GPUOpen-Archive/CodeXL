//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csWrappersCommon.cpp
///
//==================================================================================

//------------------------------ csWrappersCommon.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTAPIClasses/Include/apFunctionType.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionsManager.h>
#include <AMDTServerUtilities/Include/suTechnologyMonitorsManager.h>

// Local:
#include <src/csExtensionsManager.h>
#include <src/csGlobalVariables.h>
#include <src/csMonitoredFunctionPointers.h>
#include <src/csStringConstants.h>
#include <src/csWrappersCommon.h>


// Forward decelerations:
osModuleHandle csLoadSystemsOpenCLICDModule();
bool csConnectOpenCLGenericFunctionWrappers(osModuleHandle hSystemOpenCLModule);


// ---------------------------------------------------------------------------
// Name:        csInitializeWrapperFunctions
// Description: Initialize the OpenCL Wrappers package.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
bool csInitializeWrapperFunctions()
{
    static bool s_retVal = false;
    static bool s_isFirstTime = true;

    if (s_isFirstTime)
    {
        // Only try this once:
        s_isFirstTime = false;

        // Load the OpenCL ICD module:
        osModuleHandle hSystemOpenCLModule = csLoadSystemsOpenCLICDModule();
        GT_IF_WITH_ASSERT(hSystemOpenCLModule != NULL)
        {
            // Connect the OpenCL wrapper functions to the system's OpenCL functions:
            bool rc1 = csConnectOpenCLGenericFunctionWrappers(hSystemOpenCLModule);
            GT_IF_WITH_ASSERT(rc1)
            {
                s_retVal = true;
            }
        }
    }

    return s_retVal;
}


bool csTerminateWrapperFunctions()
{
    // TO_DO: OpenCL - implement me!
    return false;
}

// ---------------------------------------------------------------------------
// Name:        csGetSystemsOCLModuleProcAddress
// Description: Returns the address of a function that reside in the systems
//              OpenCL module.
// Arguments: procname - The queried function name.
// Return Val:  osProcedureAddress - The queried function address, or NULL in case of failure.
// Author:      Uri Shomroni
// Date:        26/11/2009
// ---------------------------------------------------------------------------
osProcedureAddress csGetSystemsOCLModuleProcAddress(const char* procname)
{
    osProcedureAddress retVal = NULL;

    // First try getting the address by querying the real OpenCL module:
    osProcedureAddress addressFromModule = NULL;
    osModuleHandle hSystemOpenCLModule = csSystemOpenCLICDModuleHandle();
    bool rcGetProc = osGetProcedureAddress(hSystemOpenCLModule, procname, addressFromModule, false);

    if (rcGetProc)
    {
        retVal = addressFromModule;
    }

    if (retVal == NULL)
    {
        // Try clGetExtensionFunctionAddress():
        retVal = (osProcedureAddress)cs_stat_realFunctionPointers.clGetExtensionFunctionAddress(procname);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csGetSystemsOCLModuleProcAddress
// Description: Returns the address of a function that reside in the systems
//              OpenCL module.
// Arguments: procname - The queried function name.
// Return Val:  osProcedureAddress - The queried function address, or NULL in case of failure.
//              oaCLPlatformID platformId - the platform id
// Author:      Uri Shomroni
// Date:        26/11/2009
// ---------------------------------------------------------------------------
osProcedureAddress csGetSystemsOCLModuleProcAddress(oaCLPlatformID platformId, const char* procname)
{
    osProcedureAddress retVal = NULL;

    // First try getting the address by querying the real OpenCL module:
    osProcedureAddress addressFromModule = NULL;
    osModuleHandle hSystemOpenCLModule = csSystemOpenCLICDModuleHandle();
    bool rcGetProc = osGetProcedureAddress(hSystemOpenCLModule, procname, addressFromModule, false);

    if (rcGetProc)
    {
        retVal = addressFromModule;
    }

    if (retVal == NULL)
    {
        // Try clGetExtensionFunctionAddressForPlatform():
        retVal = (osProcedureAddress)cs_stat_realFunctionPointers.clGetExtensionFunctionAddressForPlatform((cl_platform_id)platformId, procname);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csLoadSystemsOpenCLICDModule
//
// Description:
//  Loads the system's OpenCL ICD module into the calling process address space.
//  (ICD = Installable Client Driver)
//
// Return Val:  osModuleHandle - Will get a handle to the systems OpenCL ICD module.
//
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
osModuleHandle csLoadSystemsOpenCLICDModule()
{
    osModuleHandle retVal = OS_NO_MODULE_HANDLE;

    // Will get the system's OpenCL ICD module path:
    gtVector<osFilePath> systemOpenCLICDModulePaths;
    bool rcOCLPath = osGetSystemOpenCLModulePath(systemOpenCLICDModulePaths);
    GT_IF_WITH_ASSERT(rcOCLPath)
    {
        // Load the system OpenGL module:
        bool rc = false;
        int numberOfCLPaths = (int)systemOpenCLICDModulePaths.size();
        gtString moduleLoadError = L"System OpenCL module not found.";

        for (int i = 0; (i < numberOfCLPaths) && (!rc); i++)
        {
            // Output debug log printout:
            const osFilePath& currentModulePath = systemOpenCLICDModulePaths[i];
            const gtString& currentModulePathStr = currentModulePath.asString();
            gtString dbgLogMsg = CS_STR_DebugLog_loadingSystemOCLICDModule;
            dbgLogMsg.append(currentModulePathStr);
            OS_OUTPUT_DEBUG_LOG(dbgLogMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);

            // Attempt to load:
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
            }
        }

        // If we failed to load the system OpenCL module:
        if (!rc)
        {
            // Trigger an assertion failure:
            GT_ASSERT_EX(false, CS_STR_DebugLog_systemOCLICDModuleLoadFailed);
            GT_ASSERT_EX(false, moduleLoadError.asCharArray());

            suTechnologyMonitorsManager::reportFailedSystemModuleLoad(moduleLoadError);
        }
        else
        {
            // Output debug log printout:
            OS_OUTPUT_DEBUG_LOG(CS_STR_DebugLog_systemOCLICDModuleLoaded, OS_DEBUG_LOG_INFO);

            // Log the system's OpenGL module handle:
            csSetSystemOpenCLICDModuleHandle(retVal);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csUnloadSystemsOpenCLICDModule
// Description: Unloads the loaded system's OpenCL ICD module from this process address space.
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
bool csUnloadSystemsOpenCLICDModule()
{
    bool retVal = false;

    // Get the loaded systems OpenCL ICD module handle:
    osModuleHandle systemOCLModuleHandle = csSystemOpenCLICDModuleHandle();

    // If the system OpenCL ICD module was loaded:
    GT_IF_WITH_ASSERT(systemOCLModuleHandle != NULL)
    {
        // Unload it:
        bool rc1 = osReleaseModule(systemOCLModuleHandle);
        GT_IF_WITH_ASSERT(rc1)
        {
            // Mark that the system's OpenGL module handle was released:
            csSetSystemOpenCLICDModuleHandle(NULL);

            retVal = true;
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csConnectOpenCLGenericFunctionWrappers
// Description:
// Arguments: osModuleHandle hSystemOpenCLModule
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        12/11/2009
// ---------------------------------------------------------------------------
bool csConnectOpenCLGenericFunctionWrappers(osModuleHandle hSystemOpenCLModule)
{
    bool retVal = true;

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(CS_STR_DebugLog_wrappingSystemOCLFunctions, OS_DEBUG_LOG_DEBUG);

    // Define the "base" OpenCL functions types mask:
    unsigned int OpenCLBaseFunctionsType = AP_OPENCL_GENERIC_FUNC | AP_OPENCL_EXTENSION_FUNC;

    // Get the Monitored functions manager instance:
    apMonitoredFunctionsManager& theMonitoredFunctionsManager = apMonitoredFunctionsManager::instance();

    // For each monitored function:
    int amountOfMonitoredFuncs = theMonitoredFunctionsManager.amountOfMonitoredFunctions();

    for (int i = 0; i < amountOfMonitoredFuncs; i++)
    {
        // Get the function API type:
        unsigned int functionAPIType = theMonitoredFunctionsManager.monitoredFunctionAPIType((apMonitoredFunctionId)i);

        // If this is a "base" OpenCL function:
        if (functionAPIType & OpenCLBaseFunctionsType)
        {
            // Get a the function name:
            gtString currFunctionName = theMonitoredFunctionsManager.monitoredFunctionName((apMonitoredFunctionId)i);

            // Get a pointer to the function implementation in the system's OpenCL ICD module:
            osProcedureAddress pRealFunctionImplementation = NULL;
            bool rc1 = osGetProcedureAddress(hSystemOpenCLModule, currFunctionName.asASCIICharArray(), pRealFunctionImplementation, false);

            if (!rc1)
            {
                // Do not fail the function for an extension function:
                if (functionAPIType & AP_OPENCL_GENERIC_FUNC)
                {
                    // Output an error message:
                    gtString errorMessage = CS_STR_DebugLog_cannotGetOCLFuncPtr;
                    errorMessage += currFunctionName;
                    GT_ASSERT_EX(false, errorMessage.asCharArray());
                }
                else
                {
                    rc1 = true;
                }
            }
            else
            {
                int functionIndex = cs_stat_extensionsManager.functionIndexFromMonitoredFunctionId((apMonitoredFunctionId)i);
                // Connects gs_stat_realFunctionPointers[i] to point the real functions implementation:
                ((osProcedureAddress*)(&cs_stat_realFunctionPointers))[functionIndex] = pRealFunctionImplementation;
            }

            retVal = retVal && rc1;
        }
    }

    // Output debug log printout:
    OS_OUTPUT_DEBUG_LOG(CS_STR_DebugLog_wrappingSystemOCLFunctionsEnded, OS_DEBUG_LOG_DEBUG);

    return retVal;
}

