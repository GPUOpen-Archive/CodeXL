//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vsdDebugPort.h
///
//==================================================================================

//------------------------------ vsdDebugPort.h ------------------------------

#ifndef __VSDDEBUGPORT_H
#define __VSDDEBUGPORT_H

// Visual Studio:
#include <msdbg.h>
#include <portpriv.h>

// Infra:
#include <AMDTBaseTools/Include/gtMap.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <src/vsdDebugProcess.h>
#include <CodeXLVSPackageDebugger/Include/vsdPackageDLLBuild.h>
#include <CodeXLVSPackageDebugger/Include/vsdUnknown.h>
#include <src/vsdAidOperators.h>

// ----------------------------------------------------------------------------------
// Class Name:          vsdCDebugPort : public IDebugPort2, vsdCUnknown
// General Description: Implements IDebugPort2, representing a debugging port
// Author:              Uri Shomroni
// Creation Date:       26/12/2011
// ----------------------------------------------------------------------------------
class VSD_API vsdCDebugPort : public IDebugDefaultPort2, public IDebugNativePort2, public IDebugPortNotify2, public IDebugWindowsComputerPort2, vsdCUnknown
{
public:
    vsdCDebugPort(IDebugPort2& riUnderlyingPort);
    virtual ~vsdCDebugPort();

    IDebugProgramNode2* RegisteredProgramNode() const;

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugPort2 methods
    STDMETHOD(GetPortName)(BSTR* pbstrName);
    STDMETHOD(GetPortId)(GUID* pguidPort);
    STDMETHOD(GetPortRequest)(IDebugPortRequest2** ppRequest);
    STDMETHOD(GetPortSupplier)(IDebugPortSupplier2** ppSupplier);
    STDMETHOD(GetProcess)(AD_PROCESS_ID ProcessId, IDebugProcess2** ppProcess);
    STDMETHOD(EnumProcesses)(IEnumDebugProcesses2** ppEnum);

    ////////////////////////////////////////////////////////////
    // IDebugDefaultPort2 methods
    STDMETHOD(GetPortNotify)(IDebugPortNotify2** ppPortNotify);
    STDMETHOD(GetServer)(IDebugCoreServer3** ppServer);
    STDMETHOD(QueryIsLocal)(void);

    ////////////////////////////////////////////////////////////
    // IDebugNativePort2 methods
    STDMETHOD(AddProcess)(AD_PROCESS_ID processId, LPCOLESTR pszProcessName, BOOL fCanDetach, IDebugProcess2** ppPortProcess);

    ////////////////////////////////////////////////////////////
    // IDebugPortNotify2 methods
    STDMETHOD(AddProgramNode)(IDebugProgramNode2* pProgramNode);
    STDMETHOD(RemoveProgramNode)(IDebugProgramNode2* pProgramNode);

    ////////////////////////////////////////////////////////////
    // IDebugWindowsComputerPort2 methods
    STDMETHOD(GetComputerInfo)(COMPUTER_INFO* pInfo);

private:
    IDebugPort2* m_piUnderlyingPort;
    IDebugDefaultPort2* m_piUnderlyingDefaultPort;
    IDebugNativePort2* m_piUnderlyingNativePort;
    IDebugPortNotify2* m_piUnderlyingPortNotify;
    IDebugWindowsComputerPort2* m_piUnderlyingWindowsComputerPort;

    // A cache of debug processes:
    gtMap<AD_PROCESS_ID, vsdCDebugProcess*> m_processIDToProcess;

    // A cache of program nodes:
    gtVector<IDebugProgramNode2*> m_debugProgramNodes;
};

#endif //__VSDDEBUGPORT_H

