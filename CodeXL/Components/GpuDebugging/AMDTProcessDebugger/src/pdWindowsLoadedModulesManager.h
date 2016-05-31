//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file pdWindowsLoadedModulesManager.h
///
//==================================================================================

//------------------------------ pdWindowsLoadedModulesManager.h ------------------------------

#ifndef __PDWINDOWSLOADEDMODULESMANAGER_H
#define __PDWINDOWSLOADEDMODULESMANAGER_H

// Forward decelerations:
class apDebuggedProcessCreatedEvent;
struct pdLoadedModule;

// Infra:
#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osWin32DebugSymbolsManager.h>

// Local:
#include <src/pdLoadedModulesManager.h>


// ----------------------------------------------------------------------------------
// Class Name:           pdWindowsLoadedModulesManager : public pdLoadedModulesManager
// General Description:
//   Windows OS version of the loaded modules manager.
//   On Windows, we manage also the loaded modules debug symbols.
//
// Author:               Yaki Tebeka
// Creation Date:        30/6/2010
// ----------------------------------------------------------------------------------
class pdWindowsLoadedModulesManager : public pdLoadedModulesManager
{
public:
    pdWindowsLoadedModulesManager();
    virtual ~pdWindowsLoadedModulesManager();

    void setDebuggedProcessHandle(HANDLE hDebuggedProcess) { _hDebuggedProcess = hDebuggedProcess; };
    bool wasInitialized() const { return m_wasInitialized; };

    // Overrides pdLoadedModulesManager:
    virtual void onDebuggedProcessCreation(const apDebuggedProcessCreatedEvent& event);
    virtual void onDebuggedProcessTermination();
    virtual bool onModuleUnloaded(osInstructionPointer moduleBaseAddress, osFilePath& unLoadedModulePath);
    virtual bool isDriverAddress(osInstructionPointer address) const;
    virtual bool isSpyServerAddress(osInstructionPointer address) const;

    bool loadLoadedModulesDebugSymbols();
    void markDriverModules(pdLoadedModule& moduleDetails);
    void markSpyModules(pdLoadedModule& moduleDetails);

protected:
    // Overrides pdLoadedModulesManager:
    virtual void initialize();
    virtual void onNewLoadedModule(const pdLoadedModule& loadedModuleStruct);

private:
    // Was this class initialized?
    bool m_wasInitialized;

    // The debugged process handle:
    HANDLE _hDebuggedProcess;

    // Manages loaded modules debug symbols:
    osWin32DebugSymbolsManager _debugSymbolsManager;

    // Contains a list of keys of modules to which we need to load debug symbols for:
    gtVector<gtUInt64> _needToLoadDebugSymbols;
};


#endif //__PDWINDOWSLOADEDMODULESMANAGER_H

