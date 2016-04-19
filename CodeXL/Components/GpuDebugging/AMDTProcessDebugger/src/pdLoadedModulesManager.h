//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdLoadedModulesManager.h
///
//==================================================================================

//------------------------------ pdLoadedModulesManager.h ------------------------------

#ifndef __PDLOADEDMODULESMANAGER_H
#define __PDLOADEDMODULESMANAGER_H

// Forward decelerations:
class apDebuggedProcessCreatedEvent;

// Infra:
#include <AMDTBaseTools/Include/gtRedBlackTree.h>
#include <AMDTOSWrappers/Include/osMutex.h>
#include <AMDTAPIClasses/Include/apLoadedModule.h>


// ----------------------------------------------------------------------------------
// Class Name:           pdLoadedModulesManager
// General Description:
//   Holds a list of modules, loaded into the debugged process address space.
//   Enables efficient insertion, deletion and queries on this module's list.
//
// Author:               Yaki Tebeka
// Creation Date:        30/6/2010
// ----------------------------------------------------------------------------------
class pdLoadedModulesManager
{
public:
    pdLoadedModulesManager();
    virtual ~pdLoadedModulesManager();

    virtual void onDebuggedProcessCreation(const apDebuggedProcessCreatedEvent& event);
    virtual void onDebuggedProcessTermination();
    virtual bool onModuleLoaded(const osFilePath& modulePath, osInstructionPointer moduleBaseAddress);
    virtual bool onModuleUnloaded(osInstructionPointer moduleBaseAddress, osFilePath& unLoadedModulePath);
    virtual bool isDriverAddress(osInstructionPointer address) const = 0;
    virtual bool isSpyServerAddress(osInstructionPointer address) const = 0;

    const apLoadedModule* loadedModuleDetails(osInstructionPointer moduleBaseAddress) const;
    const pdLoadedModule* moduleContainingAddress(osInstructionPointer address) const;

protected:
    virtual void initialize();
    virtual void onNewLoadedModule(const pdLoadedModule& loadedModuleStruct);

protected:
    // A red-black tree holding the loaded modules information:
    gtRedBlackTree _loadedModulesTree;

    // A Mutex coordinating access to loaded modules data:
    osMutex _loadedModulesAccessMutex;
};


#endif //__PDLOADEDMODULESMANAGER_H

