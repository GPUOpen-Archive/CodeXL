//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file csExtensionsManager.cpp
///
//==================================================================================

//------------------------------ csExtensionsManager.cpp ------------------------------

// Infra:
#include <AMDTAPIClasses/Include/apFunctionType.h>
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osApplication.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osDirectory.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTServerUtilities/Include/suGlobalVariables.h>
#include <AMDTServerUtilities/Include/suInterceptionFunctions.h>

// Local:
#include <src/csExtensionsManager.h>
#include <src/csGlobalVariables.h>
#include <src/csMonitoredFunctionPointers.h>
#include <src/csStringConstants.h>
#include <src/csWrappersCommon.h>

// Static members initializations:
csExtensionsManager* csExtensionsManager::_pMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        csExtensionsManager::instance
// Description: Returns the single instance of the csExtensionsManager class
// Author:      Uri Shomroni
// Date:        26/11/2009
// ---------------------------------------------------------------------------
csExtensionsManager& csExtensionsManager::instance()
{
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new csExtensionsManager;
        GT_ASSERT(_pMySingleInstance);
    }

    return *_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        csExtensionsManager::csExtensionsManager
// Description: Constructor
// Author:      Uri Shomroni
// Date:        26/11/2009
// ---------------------------------------------------------------------------
csExtensionsManager::csExtensionsManager()
{
}

// ---------------------------------------------------------------------------
// Name:        csExtensionsManager::~csExtensionsManager
// Description: Destructor
// Author:      Uri Shomroni
// Date:        26/11/2009
// ---------------------------------------------------------------------------
csExtensionsManager::~csExtensionsManager()
{

}


// ---------------------------------------------------------------------------
// Name:        csExtensionsManager::initialize
// Description: Initializes the OpenCL extensions manager.
// Return Val:  bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        13/12/2010
// ---------------------------------------------------------------------------
bool csExtensionsManager::initialize()
{
    bool retVal = false;

    // Initialize the OpenCL wrappers:
    bool rcWrappers = csInitializeWrapperFunctions();
    GT_ASSERT(rcWrappers);

    // Initialize the _functionIdToWrapperAddress map:
    bool rcWrappersAddresses = initializeExtensionWrapperAddresses();
    GT_ASSERT(rcWrappersAddresses);

    retVal = rcWrappers && rcWrappersAddresses;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csExtensionsManager::wrapperFunctionAddress
// Description: Gets a function name and returns:
//              * The wrapper function address if it is supported by the real
//                  OpenCL and is wrapped
//              * The real function address if it is supported by the real
//                  OpenCL but isn't wrapped
//              * NULL if it isn't supported by the real OpenCL
// Arguments: const gtString& functionName
// Return Val: osProcedureAddress
// Author:      Uri Shomroni
// Date:        26/11/2009
// ---------------------------------------------------------------------------
osProcedureAddress csExtensionsManager::wrapperFunctionAddress(const gtString& functionName)
{
    osProcedureAddress retVal = NULL;

    // Output debug log printout:
    gtString debugLogMsg = CS_STR_retrievingFunctionPointer;
    debugLogMsg += functionName;
    OS_OUTPUT_DEBUG_LOG(debugLogMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);

    osProcedureAddress realExtensionAddress = csGetSystemsOCLModuleProcAddress(functionName.asASCIICharArray());

    if (realExtensionAddress == NULL)
    {
        // Get the OCL procedure address, or extension function address:
        realExtensionAddress = (osProcedureAddress)cs_stat_realFunctionPointers.clGetExtensionFunctionAddress(functionName.asASCIICharArray());
    }

    if (realExtensionAddress == NULL)
    {
        // If its a function implemented by the spy - get its pointer:
        // (Otherwise - we return NULL)
        retVal = spyImplementedExtensionAddress(functionName);
    }
    else // realExtensionAddress != NULL
    {
        retVal = realExtensionAddress;

        // Get the function id:
        apMonitoredFunctionId funcId = su_stat_theMonitoredFunMgr.monitoredFunctionId(functionName.asCharArray());

        if (funcId == -1)
        {
            // This extension is not supported (yet :-) by the gremedy OpenGL server:
            gtString debugMsg = CS_STR_unsupportedExtensionUse;
            debugMsg += L" (";
            debugMsg += functionName;
            debugMsg += L")";
            OS_OUTPUT_DEBUG_LOG(debugMsg.asCharArray(), OS_DEBUG_LOG_INFO);
        }
        else
        {
            // Find the appropriate wrapper function address:
            osProcedureAddress wrapperFuncAddress = NULL;

            int functionIndex = functionIndexFromMonitoredFunctionId(funcId);

            // Sanity check:
            if ((functionIndex >= 0) && (functionIndex < (int)_functionIdToWrapperAddress.size()))
            {
                wrapperFuncAddress = _functionIdToWrapperAddress[functionIndex];
            }

            if (wrapperFuncAddress != NULL)
            {
                // Store the address of the "real" function implementation struct for the
                // current render context:
                csMonitoredFunctionPointers* pExtensionsRealImplPtrs = extensionsRealImplementationPointers();

                if (nullptr != pExtensionsRealImplPtrs)
                {
                    // If the get function got the wrong value, we might have the correct value cached:
                    osProcedureAddress& realPointersStructSlot = ((osProcedureAddress*)(pExtensionsRealImplPtrs))[functionIndex];
                    if ((nullptr == realExtensionAddress) || (wrapperFuncAddress == realExtensionAddress))
                    {
                        realExtensionAddress = realPointersStructSlot;
                    }

                    // If we somehow got the wrapper address as the "real" address - don't save it,
                    // since it will cause a stack overflow when the real function is called.
                    GT_IF_WITH_ASSERT(realExtensionAddress != wrapperFuncAddress)
                    {
                        // Sanity check - we should be overwriting a null value or the same value:
                        GT_ASSERT((realExtensionAddress == realPointersStructSlot) || (nullptr == realPointersStructSlot));

                        if ((realExtensionAddress != realPointersStructSlot) && (nullptr != realPointersStructSlot))
                        {
                            OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Warning: Overwriting real function pointer %p with %p (function %ls)", realPointersStructSlot, realExtensionAddress, functionName.asCharArray());
                        }

                        realPointersStructSlot = realExtensionAddress;

                        // Return the wrapper function address:
                        retVal = wrapperFuncAddress;
                    }
                    else
                    {
                        // We do not have the real function pointer anywhere, so we should not export the wrapper:
                        retVal = nullptr;
                    }
                }
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        csExtensionsManager::wrapperFunctionAddress
// Description: Gets a function name and returns:
//              * The wrapper function address if it is supported by the real
//                  OpenCL and is wrapped
//              * The real function address if it is supported by the real
//                  OpenCL but isn't wrapped
//              * NULL if it isn't supported by the real OpenCL
// Arguments:   const gtString& functionName
// Return Val:  osProcedureAddress
// Author:      Sigal Algranaty
// Date:        6/3/2012
// ---------------------------------------------------------------------------
osProcedureAddress csExtensionsManager::wrapperFunctionAddress(cl_platform_id platformId, const gtString& functionName)
{
    osProcedureAddress retVal = NULL;

    if (NULL != cs_stat_realFunctionPointers.clGetExtensionFunctionAddressForPlatform)
    {
        // Output debug log printout:
        gtString debugLogMsg = CS_STR_retrievingFunctionPointer;
        debugLogMsg += functionName;
        OS_OUTPUT_DEBUG_LOG(debugLogMsg.asCharArray(), OS_DEBUG_LOG_DEBUG);

        // Get the OCL procedure address, or extension function address:
        osProcedureAddress realExtensionAddress = (osProcedureAddress)cs_stat_realFunctionPointers.clGetExtensionFunctionAddressForPlatform((cl_platform_id)platformId, functionName.asASCIICharArray());

        if (realExtensionAddress == NULL)
        {
            // If its a function implemented by the spy - get its pointer:
            // (Otherwise - we return NULL)
            retVal = spyImplementedExtensionAddress(functionName);
        }
        else // realExtensionAddress != NULL
        {
            retVal = realExtensionAddress;

            // Get the function id:
            apMonitoredFunctionId funcId = su_stat_theMonitoredFunMgr.monitoredFunctionId(functionName.asCharArray());

            if (funcId == -1)
            {
                // This extension is not supported (yet :-) by the gremedy OpenGL server:
                gtString debugMsg = CS_STR_unsupportedExtensionUse;
                debugMsg += L" (";
                debugMsg += functionName;
                debugMsg += L")";
                OS_OUTPUT_DEBUG_LOG(debugMsg.asCharArray(), OS_DEBUG_LOG_INFO);
            }
            else
            {
                // Find the appropriate wrapper function address:
                osProcedureAddress wrapperFuncAddress = NULL;

                int functionIndex = functionIndexFromMonitoredFunctionId(funcId);

                // Sanity check:
                if ((functionIndex >= 0) && (functionIndex < (int)_functionIdToWrapperAddress.size()))
                {
                    wrapperFuncAddress = _functionIdToWrapperAddress[functionIndex];
                }

                if (wrapperFuncAddress != NULL)
                {
                    // Store the address of the "real" function implementation struct for the
                    // current render context:
                    csMonitoredFunctionPointers* pExtensionsRealImplPtrs = extensionsRealImplementationPointers();

                    if (nullptr != pExtensionsRealImplPtrs)
                    {
                        // If the get function got the wrong value, we might have the correct value cached:
                        osProcedureAddress& realPointersStructSlot = ((osProcedureAddress*)(pExtensionsRealImplPtrs))[functionIndex];
                        if ((nullptr == realExtensionAddress) || (wrapperFuncAddress == realExtensionAddress))
                        {
                            realExtensionAddress = realPointersStructSlot;
                        }

                        // If we somehow got the wrapper address as the "real" address - don't save it,
                        // since it will cause a stack overflow when the real function is called.
                        GT_IF_WITH_ASSERT(realExtensionAddress != wrapperFuncAddress)
                        {
                            // Sanity check - we should be overwriting a null value or the same value:
                            GT_ASSERT((realExtensionAddress == realPointersStructSlot) || (nullptr == realPointersStructSlot));

                            if ((realExtensionAddress != realPointersStructSlot) && (nullptr != realPointersStructSlot))
                            {
                                OS_OUTPUT_FORMAT_DEBUG_LOG(OS_DEBUG_LOG_DEBUG, L"Warning: Overwriting real function pointer %p with %p (function %ls)", realPointersStructSlot, realExtensionAddress, functionName.asCharArray());
                            }

                            realPointersStructSlot = realExtensionAddress;

                            // Return the wrapper function address:
                            retVal = wrapperFuncAddress;
                        }
                        else
                        {
                            // We do not have the real function pointer anywhere, so we should not export the wrapper:
                            retVal = nullptr;
                        }
                    }
                }
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csExtensionsManager::spyImplementedExtensionAddress
// Description:
//   Inputs an OpenCL extension function name. If it is an extension that is
//   implemented by the OpenCL spy, returns the spy implementing function pointer,
//   Otherwise - returns NULL.
// Author:      Uri Shomroni
// Date:        22/7/2010
// ---------------------------------------------------------------------------
osProcedureAddress csExtensionsManager::spyImplementedExtensionAddress(const gtString& functionName) const
{
    osProcedureAddress retVal = NULL;

    // Get the extension function id:
    apMonitoredFunctionId extensionFuncId = su_stat_theMonitoredFunMgr.monitoredFunctionId(functionName.asCharArray());

    if ((extensionFuncId >= 0) && (extensionFuncId < apMonitoredFunctionsAmount))
    {
        // Spy implemented function types:
        static unsigned int spyImplementedFunctionTypes = AP_OPENCL_AMD_EXTENSION_FUNC;

        // Verify that this is a spy implemented function:
        unsigned int functionAPIType = su_stat_theMonitoredFunMgr.monitoredFunctionAPIType(extensionFuncId);

        if (functionAPIType & spyImplementedFunctionTypes)
        {
            // Get its wrapper address:
            int functionIndex = functionIndexFromMonitoredFunctionId(extensionFuncId);

            if ((functionIndex >= 0) && (functionIndex < (int)_functionIdToWrapperAddress.size()))
            {
                retVal = _functionIdToWrapperAddress[functionIndex];
            }
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csExtensionsManager::extensionsRealImplementationPointers
// Description: Return a structure that holds the extension functions real
//              implementation pointers.
//              It does not seem that OpenCL can have different pointers for
//              extensions based on pointers, but this allows us to change the
//              mechanism if the need arises.
// Author:      Uri Shomroni
// Date:        26/11/2009
// ---------------------------------------------------------------------------
csMonitoredFunctionPointers* csExtensionsManager::extensionsRealImplementationPointers()
{
    // Return the struct:
    csMonitoredFunctionPointers* retVal = &cs_stat_realFunctionPointers;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        functionIndexFromMonitoredFunctionId
// Description: Takes a monitored function Id and returns its index in the
//              csMonitoredFunctionPointers struct, or -1 if it isn't an OpenCL
//              function.
// Author:      Uri Shomroni
// Date:        26/11/2009
// ---------------------------------------------------------------------------
int csExtensionsManager::functionIndexFromMonitoredFunctionId(apMonitoredFunctionId funcId) const
{
    int retVal = -1;

    GT_IF_WITH_ASSERT((funcId >= apFirstOpenCLFunction) && (funcId <= apLastOpenCLFunction))
    {
        retVal = (int)funcId - (int)apFirstOpenCLFunction;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csExtensionsManager::initializeExtensionWrapperAddresses
// Description: Initialize the _functionIdToWrapperAddress map.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        26/11/2009
// ---------------------------------------------------------------------------
bool csExtensionsManager::initializeExtensionWrapperAddresses()
{
    bool retVal = false;

    // Get this module handle:
    osModuleHandle thisModuleHandle;
    bool rc1 = getOCLServerModuleHandle(thisModuleHandle);
    GT_IF_WITH_ASSERT(rc1)
    {
        retVal = true;

        // Get the monitored functions manager instance:
        apMonitoredFunctionsManager& theMonitoredFunMgr = apMonitoredFunctionsManager::instance();

        _functionIdToWrapperAddress.reserve(apLastOpenCLFunction - apFirstOpenCLFunction + 1);

        // Iterate the monitored OpenCL functions:
        for (int i = apFirstOpenCLFunction; i <= apLastOpenCLFunction; i++)
        {
            // Will get the current extension function address:
            osProcedureAddress procedureAddress = NULL;

            // Get the current extension wrapper function type:
            unsigned int funcType = theMonitoredFunMgr.monitoredFunctionAPIType((apMonitoredFunctionId)i);

            // Get the extension function types supported by this spy:
            // ------------------------------------------------------
            static unsigned int supportedFunctionTypes = AP_OPENCL_EXTENSION_FUNC | AP_OPENCL_AMD_EXTENSION_FUNC;

            // Get the current extension wrapper procedure name:
            gtString procName = theMonitoredFunMgr.monitoredFunctionName((apMonitoredFunctionId)i);

            // If the extension function is supported by this spy:
            if (funcType & supportedFunctionTypes)
            {
                // Get the wrapper function address:
                bool rc2 = osGetProcedureAddress(thisModuleHandle, procName.asASCIICharArray(), procedureAddress, false);

                if ((procedureAddress == NULL) || !rc2)
                {
                    // We didn't manage to to get the spy queried function.
                    // This means that either:
                    // a. It was not exported through the spy OpenGL32Spy.def file.
                    // b. It was not implemented in the gsExtensionsWrappers.cpp.
                    // THIS PROBLEM HAS TO BE FIXED !!!
                    gtString errMsg;
                    errMsg.appendFormattedString(L"Could not find procedure address of functions %ls.", procName.asCharArray());
                    GT_ASSERT_EX(false, errMsg.asCharArray());
                    retVal = false;
                }
            }

            // Add the current extension to the _extensionIdToWrapperAddress array:
            _functionIdToWrapperAddress.push_back(procedureAddress);

            // This is needed to initialize the real function pointer. We do not use the return value:
            wrapperFunctionAddress(procName);
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csExtensionsManager::getOCLServerModulePath
// Description: Returns the path of the OpenCL server module.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        26/11/2009
// ---------------------------------------------------------------------------
bool csExtensionsManager::getOCLServerModulePath(osFilePath& oclModulePath)
{
    bool retVal = false;

    gtString thisModuleFileName = OS_GREMEDY_OPENCL_SERVER_MODULE_NAME;

    gtString spiesDirectory;
    bool isRunningInStandaloneMode = csIsRunningInStandaloneMode();
    bool rc1 = suGetSpiesDirectory(spiesDirectory, isRunningInStandaloneMode);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Calculate this module full path:
        gtString thisModuleFullPathAsStr = spiesDirectory;
        thisModuleFullPathAsStr += osFilePath::osPathSeparator;
        thisModuleFullPathAsStr += thisModuleFileName;

        oclModulePath.setFullPathFromString(thisModuleFullPathAsStr);
        retVal = true;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csExtensionsManager::getOCLServerModuleHandle
// Description: Returns a handle to this module (the OpenCL server).
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        26/11/2009
// ---------------------------------------------------------------------------
bool csExtensionsManager::getOCLServerModuleHandle(osModuleHandle& thisModuleHandle)
{
    bool retVal = false;

    // Get the OpenCL server module path:
    osFilePath oclModulePath;
    bool rc1 = getOCLServerModulePath(oclModulePath);
    GT_IF_WITH_ASSERT(rc1)
    {
        // Get the loaded OpenCL module handle:
        bool gotOCLServerModuleHandle = osGetLoadedModuleHandle(oclModulePath, thisModuleHandle);

        // Under Windows, it might be that we are running under a .NET application.
        // (See getOGLServerModuleHandleUnderDotNetApp documentation for more details)
#if AMDT_BUILD_TARGET == AMDT_WINDOWS_OS
        {
            if (!gotOCLServerModuleHandle)
            {
                gotOCLServerModuleHandle = getOCLServerModuleHandleUnderDotNetApp(thisModuleHandle);
            }
        }
#endif

        // If we managed to the the OpenGL server module handle:
        GT_IF_WITH_ASSERT(gotOCLServerModuleHandle)
        {
            retVal = true;
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        csExtensionsManager::getOCLServerModuleHandleUnderDotNetApp
// Description:
//   When debugging .NET application, we ask the user to copy the spy into the debugged
//   application directory. In this case, the osGetLoadedModuleHandle() call that resides in
//   getOGLServerModuleHandle() will fail since the loaded OpenGL server now resides under the
//   debugged application directory and not under the spies directory.
//   This function assumes that the OpenGL server resides under the debugged application
//   directory and tried to get a handle to it.
// Return Val: bool  - Success / failure.
// Author:      Uri Shomroni
// Date:        26/11/2009
// ---------------------------------------------------------------------------
bool csExtensionsManager::getOCLServerModuleHandleUnderDotNetApp(osModuleHandle& thisModuleHandle)
{
    bool retVal = false;

    // Get the current application path:
    osFilePath currentApplicationPath;
    bool rc2 = osGetCurrentApplicationPath(currentApplicationPath);
    GT_IF_WITH_ASSERT(rc2)
    {
        // Get the current application directory:
        osDirectory thisAppDir;
        bool rc3 = currentApplicationPath.getFileDirectory(thisAppDir);
        GT_IF_WITH_ASSERT(rc3)
        {
            // Build the OpenCL server module path:
            gtString oclServerModuleFullPathAsStr = thisAppDir.directoryPath().asString();
            oclServerModuleFullPathAsStr += osFilePath::osPathSeparator;
            oclServerModuleFullPathAsStr += OS_GREMEDY_OPENCL_SERVER_MODULE_NAME;

            osFilePath oclServerModuleFullPath(oclServerModuleFullPathAsStr);

            // Try to get a handle to the OpenCL module server:
            bool gotOCLServerModuleHandle = osGetLoadedModuleHandle(oclServerModuleFullPath, thisModuleHandle);
            GT_IF_WITH_ASSERT(gotOCLServerModuleHandle)
            {
                retVal = true;
            }
        }
    }

    return retVal;
}
