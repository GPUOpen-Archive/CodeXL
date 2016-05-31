//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdWindowsLoadedModulesManager.cpp
///
//==================================================================================

//------------------------------ pdWindowsLoadedModulesManager.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTOSWrappers/Include/osMutexLocker.h>

// Local:
#include <src/pdStringConstants.h>
#include <src/pdLoadedModule.h>
#include <src/pdWindowsLoadedModulesManager.h>


// ---------------------------------------------------------------------------
// Name:        pdWindowsLoadedModulesManager::pdWindowsLoadedModulesManager
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        30/6/2010
// ---------------------------------------------------------------------------
pdWindowsLoadedModulesManager::pdWindowsLoadedModulesManager()
    : _hDebuggedProcess(nullptr), m_wasInitialized(false)
{
}


// ---------------------------------------------------------------------------
// Name:        pdWindowsLoadedModulesManager::~pdWindowsLoadedModulesManager
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        30/6/2010
// ---------------------------------------------------------------------------
pdWindowsLoadedModulesManager::~pdWindowsLoadedModulesManager()
{
}


// ---------------------------------------------------------------------------
// Name:        pdWindowsLoadedModulesManager::onDebuggedProcessCreation
// Description: Is called when the debugged process is created.
// Arguments:   event - Contains process creation event details.
// Author:      Yaki Tebeka
// Date:        30/6/2010
// ---------------------------------------------------------------------------
void pdWindowsLoadedModulesManager::onDebuggedProcessCreation(const apDebuggedProcessCreatedEvent& event)
{
    GT_UNREFERENCED_PARAMETER(&event);

    // Initialize the debugged process symbols server:
    m_wasInitialized = _debugSymbolsManager.initializeProcessSymbolsServer(_hDebuggedProcess);
    GT_ASSERT(m_wasInitialized);

    // Yaki 29/9/2005:
    // We cannot load the debugged process .exe module debug information here.
    // Therefore, we load it on the first dll load event (See case 405).
}


// ---------------------------------------------------------------------------
// Name:        pdWindowsLoadedModulesManager::onModuleUnloaded
// Description: Is called when a module is unloaded.
// Arguments: moduleBaseAddress - The module loaded address.
//            unLoadedModulePath - Output parameter, will get the path of the module that was unloaded.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/10/2005
// ---------------------------------------------------------------------------
bool pdWindowsLoadedModulesManager::onModuleUnloaded(osInstructionPointer moduleBaseAddress, osFilePath& unLoadedModulePath)
{
    bool retVal = false;

    // Get the details of the module to be unloaded:
    // We sometimes get a unloaded notification on modules for which we didn't get loaded notification. Therefore, we don't assert
    // (See more details in a comment inside pdLoadedModulesManager::onModuleUnloaded)
    const apLoadedModule* pLoadedModuleDetails = loadedModuleDetails(moduleBaseAddress);

    if (pLoadedModuleDetails != NULL)
    {
        // If the module's debug information was loaded:
        if (pLoadedModuleDetails->_pModuleLoadedSize > 0)
        {
            // Unload the module's debug information:
            bool rcDbgInfo = _debugSymbolsManager.unloadModuleDebugSymbols(_hDebuggedProcess, unLoadedModulePath, pLoadedModuleDetails->_pModuleStartAddress);
            GT_ASSERT(rcDbgInfo);
        }
    }

    // Perform base class actions:
    bool rcBase = pdLoadedModulesManager::onModuleUnloaded(moduleBaseAddress, unLoadedModulePath);
    retVal = rcBase;

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWindowsLoadedModulesManager::onDebuggedProcessTermination
// Description:  Is called when the debugged process is terminated.
// Author:      Yaki Tebeka
// Date:        30/6/2010
// ---------------------------------------------------------------------------
void pdWindowsLoadedModulesManager::onDebuggedProcessTermination()
{
    // Clean up the resources used to store the debugged process symbols:
    bool rcSym = _debugSymbolsManager.clearProcessSymbolsServer(_hDebuggedProcess);
    GT_ASSERT(rcSym != 0);

    // Clear the debugged process handle:
    _hDebuggedProcess = NULL;
    m_wasInitialized = false;

    // Perform base class actions:
    pdLoadedModulesManager::onDebuggedProcessTermination();
}


// ---------------------------------------------------------------------------
// Name:        pdWindowsLoadedModulesManager::loadLoadedModulesDebugSymbols
// Description: Iterates the loaded modules and loads their debug information.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/10/2005
// Implementation notes:
//   We cannot load the dll debug info when catching LOAD_DLL_DEBUG_EVENT since
//   when we get this event, we sometimes cannot get the dll loaded size and therefore
//   cannot use SymLoadModuleEx to load its debug info (see more details in case 405 resolution).
// ---------------------------------------------------------------------------
bool pdWindowsLoadedModulesManager::loadLoadedModulesDebugSymbols()
{
    bool retVal = true;

    // If there are modules for which we didn't load debug symbols:
    size_t amountOfNeedToLoadDebugSymbols = _needToLoadDebugSymbols.size();

    if (0 < amountOfNeedToLoadDebugSymbols)
    {
        // Lock the access to the loaded modules:
        osMutexLocker mtxLocker(_loadedModulesAccessMutex);

        // Iterate the modules to which we need to load debug symbols:
        gtVector<gtUInt64>::iterator iter = _needToLoadDebugSymbols.begin();
        gtVector<gtUInt64>::iterator endIter = _needToLoadDebugSymbols.end();

        while (iter != endIter)
        {
            // Get the current module base address:
            gtUInt64 moduleBaseAddress = *iter;

            // If this module is registered as a loaded module:
            gtRedBlackTreeNode* pTreeNodeRepresentingModule = _loadedModulesTree.search(moduleBaseAddress);

            if (pTreeNodeRepresentingModule != NULL)
            {
                // Get the tree node value:
                gtRedBlackTreeValue* pTreeNodeValue = pTreeNodeRepresentingModule->getValue();
                GT_IF_WITH_ASSERT(pTreeNodeValue != NULL)
                {
                    // Down cast the tree node value:
                    pdLoadedModule& currLoadedModuleInfo = *((pdLoadedModule*)pTreeNodeValue);

                    // Get the loaded module details:
                    apLoadedModule& currLoadedModuleData = currLoadedModuleInfo._loadedModuleData;

                    // If we didn't load the module debug info so far:
                    if (currLoadedModuleData._pModuleLoadedSize == 0)
                    {
                        // Get the module file path:
                        const osFilePath& moduleFilePath = currLoadedModuleData._moduleFilePath;

                        // Get the module size:
                        ULONG moduleSize = 0;
                        bool rc = _debugSymbolsManager.getLoadedModuleSize(_hDebuggedProcess, currLoadedModuleData._pModuleStartAddress, moduleSize);

                        if (!rc)
                        {
                            // Trigger an assertion failure:
                            gtString errorMessage = PD_STR_FailedToGetLoadedModuleSize;
                            errorMessage += currLoadedModuleData._moduleFilePath.asString();
                            GT_ASSERT_EX(false, errorMessage.asCharArray());
                            retVal = false;
                        }
                        else
                        {
                            // Store the module size:
                            currLoadedModuleData._pModuleLoadedSize = moduleSize;

                            // Get the module loaded address:
                            osInstructionPointer pModuleLoadedAddress = currLoadedModuleData._pModuleStartAddress;

                            // Load the module debug symbols:
                            rc = _debugSymbolsManager.loadModuleDebugSymbols(_hDebuggedProcess, moduleFilePath, pModuleLoadedAddress, moduleSize);
                            retVal = retVal && rc;
                        }
                    }

                    // If the module is a driver or a "spy" module - mark it as such:
                    markDriverModules(currLoadedModuleInfo);
                    markSpyModules(currLoadedModuleInfo);
                }
            }

            // Next loaded module:
            iter++;
        }
    }


    // Mark the we loaded (or at least tried to) debug symbols to all the listed modules:
    _needToLoadDebugSymbols.clear();

    GT_ASSERT(retVal);
    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWindowsLoadedModulesManager::isDriverAddress
//
// Description:
//   Inputs a virtual memory in debugged process address space, and
//   returns true iff it resides in a driver dll (OpenGL / OpenCL driver).
//
// Arguments: address - The input address, given in debugged process address space.
//
// Author:      Yaki Tebeka
// Date:        20/4/2006
//
// Implementation notes:
//   Currently identifies only the NVIDIA OpenGL and ATI OpenCL drivers dlls.
// ---------------------------------------------------------------------------
bool pdWindowsLoadedModulesManager::isDriverAddress(osInstructionPointer address) const
{
    bool retVal = false;

    // Check the debug log level:
    bool isLoggedLevelDEBUGOrHigher = (osDebugLog::instance().loggedSeverity() >= OS_DEBUG_LOG_DEBUG);

    // If needed, load loaded modules debug symbols:
    bool rcLoadModulesDbgSym = ((pdWindowsLoadedModulesManager&)(*this)).loadLoadedModulesDebugSymbols();
    GT_ASSERT(rcLoadModulesDbgSym);

    // Will true iff we found a module containing the input address:
    bool containingModuleFound = false;

    // Get a module containing the input address:
    const pdLoadedModule* pLoadedModuleInfo = moduleContainingAddress(address);

    if (pLoadedModuleInfo != NULL)
    {
        // Mark that we found the module that contains the input address:
        containingModuleFound = true;

        // Debug output:
        if (isLoggedLevelDEBUGOrHigher)
        {
            gtString loggedMessage = pLoadedModuleInfo->_loadedModuleData._moduleFilePath.asString();
            loggedMessage.prependFormattedString(PD_STR_addressInsideLoadedModule, address);
            OS_OUTPUT_DEBUG_LOG(loggedMessage.asCharArray(), OS_DEBUG_LOG_INFO)
        }

        // If the module is a driver module:
        if (pLoadedModuleInfo->_isDriverModule)
        {
            retVal = true;
        }
    }

    if (!containingModuleFound)
    {
        // We didn't find a module containing the input address, output a debug log prinout:
        gtString loggedMessage;
        loggedMessage.appendFormattedString(PD_STR_addressIsNotInsideLoadedModule, address);
        OS_OUTPUT_DEBUG_LOG(loggedMessage.asCharArray(), OS_DEBUG_LOG_ERROR)
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWindowsLoadedModulesManager::isSpyServerAddress
// Description: Inputs a virtual memory in debugged process address space, and
//              returns true iff it resides in the spy utilities dll
// Arguments: address - The input address, given in debugged process address space.
// Author:      Sigal Algranaty
// Date:        1/9/2009
// ---------------------------------------------------------------------------
bool pdWindowsLoadedModulesManager::isSpyServerAddress(osInstructionPointer address) const
{
    bool retVal = false;

    // Check the debug log level:
    bool isLoggedLevelDEBUGOrHigher = (osDebugLog::instance().loggedSeverity() >= OS_DEBUG_LOG_DEBUG);

    // If needed, load loaded modules debug symbols:
    bool rcLoadModulesDbgSym = ((pdWindowsLoadedModulesManager&)(*this)).loadLoadedModulesDebugSymbols();
    GT_ASSERT(rcLoadModulesDbgSym);

    // Will true iff we found a module containing the input address:
    bool containingModuleFound = false;

    // Get a module containing the input address:
    const pdLoadedModule* pLoadedModuleInfo = moduleContainingAddress(address);

    if (pLoadedModuleInfo != NULL)
    {
        // Mark that we found the module that contains the input address:
        containingModuleFound = true;

        // Debug output:
        if (isLoggedLevelDEBUGOrHigher)
        {
            gtString loggedMessage = pLoadedModuleInfo->_loadedModuleData._moduleFilePath.asString();
            loggedMessage.prependFormattedString(PD_STR_addressInsideLoadedModule, address);
            OS_OUTPUT_DEBUG_LOG(loggedMessage.asCharArray(), OS_DEBUG_LOG_INFO)
        }

        // If the module is a "spy" module:
        if (pLoadedModuleInfo->_isSpyModule)
        {
            retVal = true;
        }
    }

    if (!containingModuleFound)
    {
        gtString loggedMessage;
        loggedMessage.appendFormattedString(PD_STR_addressIsNotInsideLoadedModule, address);
        OS_OUTPUT_DEBUG_LOG(loggedMessage.asCharArray(), OS_DEBUG_LOG_ERROR);
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdWindowsLoadedModulesManager::markDriverModules
// Description: Input a loaded module. If it is a driver module marks it as such.
// Author:      Yaki Tebeka
// Date:        12/7/2010
// ---------------------------------------------------------------------------
void pdWindowsLoadedModulesManager::markDriverModules(pdLoadedModule& moduleDetails)
{
    // Get the module's file name:
    const osFilePath& moduleFilePath = moduleDetails._loadedModuleData._moduleFilePath;
    gtString moduleFileName;
    bool rcFileName = moduleFilePath.getFileName(moduleFileName);

    if (rcFileName)
    {
        moduleFileName.toLowerCase();

        if ((moduleFileName == OS_NVIDIA_OGL_DRIVER_DLL_NAME) || (moduleFileName == OS_NVIDIA_OGL32_DRIVER_DLL_NAME) ||
            (moduleFileName == OS_NVIDIA_VISTA32_OGL_DRIVER_DLL_NAME) || (moduleFileName == OS_NVIDIA_VISTA64_OGL_DRIVER_DLL_NAME))
        {
            // The input module is an NVIDIA OpenGL driver module:
            moduleDetails._isDriverModule = true;
        }
        else if ((moduleFileName == OS_ATI_OGL_DRIVER_DLL_NAME)     || (moduleFileName == OS_ATI_O6_DRIVER_DLL_NAME)    ||
                 (moduleFileName == OS_ATI_CFX32_DRIVER_DLL_NAME)   || (moduleFileName == OS_ATI_CFX64_DRIVER_DLL_NAME) ||
                 (moduleFileName == OS_ATI_G632_DRIVER_DLL_NAME)    || (moduleFileName == OS_ATI_G664_DRIVER_DLL_NAME))
        {
            // The input module is an ATI OpenGL driver module:
            moduleDetails._isDriverModule = true;
        }
        else if (moduleFileName == OS_ATI_OCL_DRIVER_DLL_NAME)
        {
            // The input module is an ATI OpenCL driver module:
            moduleDetails._isDriverModule = true;
        }
        else if ((moduleFileName == OS_ATI_CALCL_DRIVER_DLL_NAME) || (moduleFileName == OS_ATI_CALCL64_DRIVER_DLL_NAME) ||
                 (moduleFileName == OS_ATI_CALRT_DRIVER_DLL_NAME) || (moduleFileName == OS_ATI_CALRT64_DRIVER_DLL_NAME) ||
                 (moduleFileName == OS_ATI_CALDD_DRIVER_DLL_NAME) || (moduleFileName == OS_ATI_CALDD64_DRIVER_DLL_NAME) ||
                 (moduleFileName == OS_ATI_OVDECODE_DRIVER_DLL_NAME) || (moduleFileName == OS_ATI_OVDECODE64_DRIVER_DLL_NAME))
        {
            // The input module is an ATI OpenCL driver module:
            moduleDetails._isDriverModule = true;
        }
        else if ((moduleFileName == OS_AMD_OCL_DRIVER_DLL_NAME) || (moduleFileName == OS_AMD_OCL64_DRIVER_DLL_NAME))
        {
            // The input module is an ATI OpenCL driver module:
            moduleDetails._isDriverModule = true;
        }
        else if ((moduleFileName == OS_INTEL_32_DRIVER_DLL_NAME) || (moduleFileName == OS_INTEL_64_DRIVER_DLL_NAME))
        {
            // The input module is an Intel driver module:
            moduleDetails._isDriverModule = true;
        }

        // If this is a driver module:
        if (moduleDetails._isDriverModule)
        {
            // If we don't have the driver module size:
            size_t moduleSize = moduleDetails._loadedModuleData._pModuleLoadedSize;

            if (moduleSize == 0)
            {
                GT_ASSERT(false);

                // Give the module an estimated size:
                moduleDetails._loadedModuleData._pModuleLoadedSize = 6000000;
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        pdWindowsLoadedModulesManager::markSpyModules
// Description: Input a loaded module. If it is a driver module marks it as such.
// Author:      Yaki Tebeka
// Date:        12/7/2010
// ---------------------------------------------------------------------------
void pdWindowsLoadedModulesManager::markSpyModules(pdLoadedModule& moduleDetails)
{
    // Get the module's file name:
    const osFilePath& moduleFilePath = moduleDetails._loadedModuleData._moduleFilePath;
    gtString moduleFileName;
    bool rcFileName = moduleFilePath.getFileName(moduleFileName);

    if (rcFileName)
    {
        moduleFileName.toLowerCase();

        if (moduleFileName.startsWith(OS_SPY_UTILS_FILE_PREFIX))
        {
            // Mark the module as a spy module:
            moduleDetails._isSpyModule = true;

            // Get the Spy module size:
            size_t moduleSize = moduleDetails._loadedModuleData._pModuleLoadedSize;

            // If we don't manage to get the size - hard code it to version 5.3 size:
            // (This should not happen, the below is only a safety net).
            if (moduleSize <= 0)
            {
                moduleDetails._loadedModuleData._pModuleLoadedSize = moduleSize;
            }
        }
    }
}


// ---------------------------------------------------------------------------
// Name:        pdWindowsLoadedModulesManager::initialize
// Description: Clears the class held data and initializes it.
// Author:      Yaki Tebeka
// Date:        30/6/2010
// ---------------------------------------------------------------------------
void pdWindowsLoadedModulesManager::initialize()
{
    // Clear the list of modules to which we need to load debug info for:
    _needToLoadDebugSymbols.clear();

    // Perform base class actions:
    pdLoadedModulesManager::initialize();
}


// ---------------------------------------------------------------------------
// Name:        pdWindowsLoadedModulesManager::onModuleLoaded
// Description: Is called when a debugged module is loaded for the first time.
// Arguments:   loadedModuleStruct - The loaded module's details.
// Author:      Yaki Tebeka
// Date:        13/7/2010
// ---------------------------------------------------------------------------
void pdWindowsLoadedModulesManager::onNewLoadedModule(const pdLoadedModule& loadedModuleStruct)
{
    // Add the key of the newly loaded module to the list of modules to which we need to load debug symbols for:
    gtUInt64 newModuleKey = loadedModuleStruct.getKey();
    _needToLoadDebugSymbols.push_back(newModuleKey);
}

