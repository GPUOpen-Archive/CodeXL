//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdLoadedModulesManager.cpp
///
//==================================================================================

//------------------------------ pdLoadedModulesManager.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osMutexLocker.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <src/pdStringConstants.h>
#include <src/pdLoadedModule.h>
#include <src/pdLoadedModulesManager.h>


// ---------------------------------------------------------------------------
// Name:        pdLoadedModulesManager::pdLoadedModulesManager
// Description: Constructor
// Author:      Yaki Tebeka
// Date:        30/6/2010
// ---------------------------------------------------------------------------
pdLoadedModulesManager::pdLoadedModulesManager()
{
    initialize();
}


// ---------------------------------------------------------------------------
// Name:        pdLoadedModulesManager::~pdLoadedModulesManager
// Description: Destructor
// Author:      Yaki Tebeka
// Date:        30/6/2010
// ---------------------------------------------------------------------------
pdLoadedModulesManager::~pdLoadedModulesManager()
{
}


// ---------------------------------------------------------------------------
// Name:        pdLoadedModulesManager::initialize
// Description: Clears the class held data and initializes it.
// Author:      Yaki Tebeka
// Date:        30/6/2010
// ---------------------------------------------------------------------------
void pdLoadedModulesManager::initialize()
{
    // Clear loaded modules data:
    _loadedModulesTree.clear();
}


// ---------------------------------------------------------------------------
// Name:        pdLoadedModulesManager::onDebuggedProcessCreation
// Description: Is called when the debugged process is created.
// Arguments:   event - Contains process creation event details.
// Author:      Yaki Tebeka
// Date:        30/6/2010
// ---------------------------------------------------------------------------
void pdLoadedModulesManager::onDebuggedProcessCreation(const apDebuggedProcessCreatedEvent& event)
{
    GT_UNREFERENCED_PARAMETER(&event);

    initialize();
}


// ---------------------------------------------------------------------------
// Name:        pdLoadedModulesManager::onDebuggedProcessTermination
// Description:  Is called when the debugged process is terminated.
// Author:      Yaki Tebeka
// Date:        30/6/2010
// ---------------------------------------------------------------------------
void pdLoadedModulesManager::onDebuggedProcessTermination()
{
    initialize();
}


// ---------------------------------------------------------------------------
// Name:        pdLoadedModulesManager::onModuleLoaded
// Description: Is called when a module is loaded.
// Arguments: modulePath - The module file path.
//            moduleBaseAddress - The module loaded address.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/10/2005
// ---------------------------------------------------------------------------
bool pdLoadedModulesManager::onModuleLoaded(const osFilePath& modulePath, osInstructionPointer moduleBaseAddress)
{
    bool retVal = false;

    // Lock the access to the loaded modules data:
    osMutexLocker mtxLocker(_loadedModulesAccessMutex);

    // If this module is already registered as a loaded module:
    gtRedBlackTreeNode* pTreeNodeRepresentingModule = _loadedModulesTree.search(gtUInt64(moduleBaseAddress));

    if (pTreeNodeRepresentingModule != NULL)
    {
        // Nothing to be done:
        retVal = true;
    }
    else
    {
        // This is the first time we are notified about this module:

        // Create a structure that will hold the module data:
        pdLoadedModule* pLoadedModuleStruct = new pdLoadedModule;
        GT_IF_WITH_ASSERT(pLoadedModuleStruct != NULL)
        {
            pLoadedModuleStruct->_loadedModuleData._pModuleStartAddress = moduleBaseAddress;
            pLoadedModuleStruct->_loadedModuleData._moduleFilePath = modulePath;

            // Register the module in the loaded modules red-black tree:
            gtAutoPtr<gtRedBlackTreeValue> aptrTreeNodeValue(pLoadedModuleStruct);
            _loadedModulesTree.insert(aptrTreeNodeValue);

            // Notify sub-classes about the new loaded module:
            onNewLoadedModule(*pLoadedModuleStruct);

            if (OS_DEBUG_LOG_EXTENSIVE <= osDebugLog::instance().loggedSeverity())
            {
                // Output the loaded module to the log file:
                gtString dbgMsg;
                dbgMsg.appendFormattedString(L"Module Loaded. Module path: %ls. Module base address: %lu ", modulePath.asString().asCharArray(), (gtUInt64)moduleBaseAddress);
                OS_OUTPUT_DEBUG_LOG(dbgMsg.asCharArray(), OS_DEBUG_LOG_EXTENSIVE);
            }

            retVal = true;
        }
    }

    // Unlock the access to the loaded modules data:
    mtxLocker.unlockMutex();

    GT_RETURN_WITH_ASSERT(retVal);
}


// ---------------------------------------------------------------------------
// Name:        pdLoadedModulesManager::onModuleUnloaded
// Description: Is called when a module is unloaded.
// Arguments: moduleBaseAddress - The module loaded address.
//            unLoadedModulePath - Output parameter, will get the path of the module that was unloaded.
// Return Val: bool  - Success / failure.
// Author:      Yaki Tebeka
// Date:        9/10/2005
// ---------------------------------------------------------------------------
bool pdLoadedModulesManager::onModuleUnloaded(osInstructionPointer moduleBaseAddress, osFilePath& unLoadedModulePath)
{
    bool retVal = false;

    unLoadedModulePath.clear();

    // Lock the access to the loaded modules data:
    osMutexLocker mtxLocker(_loadedModulesAccessMutex);

    // If the module is NOT registered as a loaded module:
    gtRedBlackTreeNode* pTreeNodeRepresentingModule = _loadedModulesTree.search(gtUInt64(moduleBaseAddress));

    if (pTreeNodeRepresentingModule == NULL)
    {
        // Yaki 4/2/2010:
        // We didn't get a module loaded notification about this unloaded module. This sometimes happens when debugging 32 bit
        // applications on top of Vista 64 bit (google for 'UNLOAD_DLL_DEBUG_EVENT vista 64' to see other people who complain about this).
        // Unfortunately, using moduleBaseAddress, pdWin32ProcessDebugger::getDebuggedProcessDLLName() does not manage to figure out
        // the path of the unloaded module. Therefore, we will return an empty module path.
        unLoadedModulePath.clear();
    }
    else
    {
        // Get the tree node value:
        gtRedBlackTreeValue* pTreeNodeValue = pTreeNodeRepresentingModule->getValue();
        GT_IF_WITH_ASSERT(pTreeNodeValue != NULL)
        {
            // Get the unloaded module path:
            unLoadedModulePath = ((pdLoadedModule*)pTreeNodeValue)->_loadedModuleData._moduleFilePath;
        }

        // Remove module info from the loaded modules tree and delete the representing tree item:
        _loadedModulesTree.deleteNode(pTreeNodeRepresentingModule);

        retVal = true;
    }

    // Unlock the access to the loaded modules data:
    mtxLocker.unlockMutex();

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLoadedModulesManager::loadedModuleDetails
// Description: Inputs the base address of a loaded module and returns its details.
// Arguments:   moduleBaseAddress - The module's base address.
// Return Val:  const apLoadedModule* - The loaded module details, or NULL if a module
//                                      having the input base address was not loaded (or unloaded).
// Author:      Yaki Tebeka
// Date:        7/7/2010
// ---------------------------------------------------------------------------
const apLoadedModule* pdLoadedModulesManager::loadedModuleDetails(osInstructionPointer moduleBaseAddress) const
{
    const apLoadedModule* retVal = NULL;

    // Lock the access to the loaded modules data:
    osMutexLocker mtxLocker((osMutex&)_loadedModulesAccessMutex);

    // If this module is registered as a loaded module:
    gtRedBlackTreeNode* pTreeNodeRepresentingModule = _loadedModulesTree.search(gtUInt64(moduleBaseAddress));

    if (pTreeNodeRepresentingModule != NULL)
    {
        // Get the tree node value:
        gtRedBlackTreeValue* pTreeNodeValue = pTreeNodeRepresentingModule->getValue();
        GT_IF_WITH_ASSERT(pTreeNodeValue != NULL)
        {
            // Get the loaded module details:
            retVal = &(((pdLoadedModule*)pTreeNodeValue)->_loadedModuleData);
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLoadedModulesManager::moduleContainingAddress
// Description: Inputs a virtual memory address and returns the details of a module who's loaded virtual
//              addresses range contain this address.
// Arguments:   address - The input address.
// Return Val:  const apLoadedModule* - Will get the details of a module, or NULL if no module
//                                      loaded virtual addresses range contains the input address.
// Author:      Yaki Tebeka
// Date:        7/7/2010
// ---------------------------------------------------------------------------
const pdLoadedModule* pdLoadedModulesManager::moduleContainingAddress(osInstructionPointer address) const
{
    const pdLoadedModule* retVal = NULL;

    // Get the module who's base address is the highest address which is lower than the input address:
    gtUInt64 searchKey = address;
    gtRedBlackTreeNode* pTreeNode = _loadedModulesTree.searchEqualOrLowerThan(searchKey);

    if (pTreeNode != NULL)
    {
        // Get the tree node value:
        gtRedBlackTreeValue* pTreeNodeValue = pTreeNode->getValue();
        GT_IF_WITH_ASSERT(pTreeNodeValue != NULL)
        {
            // Get the represented module details:
            const pdLoadedModule& loadedModuleInfo = *((pdLoadedModule*)pTreeNodeValue);
            osInstructionPointer moduleStartAddress = loadedModuleInfo._loadedModuleData._pModuleStartAddress;
            size_t moduleLoadedSize = loadedModuleInfo._loadedModuleData._pModuleLoadedSize;
            osInstructionPointer moduleEndAddress = moduleStartAddress + moduleLoadedSize - 1;

            // If the input address resides in this module:
            if ((moduleStartAddress <= address) && (address < moduleEndAddress))
            {
                retVal = &loadedModuleInfo;
            }
        }
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        pdLoadedModulesManager::onModuleLoaded
// Description: Is called when a debugged module is loaded for the first time.
// Arguments:   loadedModuleStruct - The loaded module's details.
// Author:      Yaki Tebeka
// Date:        13/7/2010
// ---------------------------------------------------------------------------
void pdLoadedModulesManager::onNewLoadedModule(const pdLoadedModule& loadedModuleStruct)
{
    GT_UNREFERENCED_PARAMETER(&loadedModuleStruct);
}

