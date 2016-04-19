//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscDebugModule.h
///
//==================================================================================

//----------------------------- vspDebugModule.h ------------------------------

#ifndef __VSPDEBUGMODULE_H
#define __VSPDEBUGMODULE_H

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <src/vspUnknown.h>

// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugModule : public IDebugModule2
// General Description: Implementes IDebugModule2, representing a module in the debugged
//                      process
// Author:               Uri Shomroni
// Creation Date:        22/9/2010
// ----------------------------------------------------------------------------------
class vspCDebugModule : public IDebugModule2, vspCUnknown
{
public:
    vspCDebugModule(const osFilePath& moduleFilePath, osInstructionPointer moduleLoadAddress, bool areDebugSymbolsLoaded);
    virtual ~vspCDebugModule();

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugModule2 methods
    STDMETHOD(GetInfo)(MODULE_INFO_FIELDS dwFields, MODULE_INFO* pInfo);
    STDMETHOD(ReloadSymbols_Deprecated)(LPCOLESTR pszUrlToSymbols, BSTR* pbstrDebugMessage);

    const osFilePath& moduleFilePath() const {return _moduleFilePath;};

private:
    // Do not allow use of my default constructor:
    vspCDebugModule();

    void fillMembersFromModuleData(osInstructionPointer moduleLoadAddress, bool areDebugSymbolsLoaded);

private:
    osFilePath _moduleFilePath;
    MODULE_INFO _moduleInfo;
};

// ----------------------------------------------------------------------------------
// Class Name:          vspCEnumDebugModules : public IEnumDebugModules2
// General Description: Implements IEnumDebugModules2, Enumerating the currently loaded modules
// Author:               Uri Shomroni
// Creation Date:        26/9/2010
// ----------------------------------------------------------------------------------
class vspCEnumDebugModules : public IEnumDebugModules2, vspCUnknown
{
public:
    vspCEnumDebugModules(const gtVector<vspCDebugModule*>& loadedModules);
    virtual ~vspCEnumDebugModules();

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IEnumDebugModules2 methods
    STDMETHOD(Next)(
        ULONG celt,
        IDebugModule2** rgelt,
        ULONG* pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumDebugModules2** ppEnum);
    STDMETHOD(GetCount)(ULONG* pcelt);

private:
    // Do not allow use of my default constructor:
    vspCEnumDebugModules();

private:
    // The enumerated modules:
    gtVector<vspCDebugModule*> _enumModules;

    unsigned int _currentPosition;
};

#endif //__VSPDEBUGMODULE_H

