//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vsdDebugPort.cpp
///
//==================================================================================

//------------------------------ vsdDebugPort.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>

// Local:
#include <src/vsdDebugPort.h>

#define VSD_IID_IDebugNativePort2 __uuidof(IDebugNativePort2)

// ---------------------------------------------------------------------------
// Name:        vsdCDebugPort::vsdCDebugPort
// Description: Constructor
// Author:      Uri Shomroni
// Date:        26/12/2011
// ---------------------------------------------------------------------------
vsdCDebugPort::vsdCDebugPort(IDebugPort2& riUnderlyingPort)
    : m_piUnderlyingPort(&riUnderlyingPort), m_piUnderlyingDefaultPort(NULL), m_piUnderlyingNativePort(NULL), m_piUnderlyingPortNotify(NULL), m_piUnderlyingWindowsComputerPort(nullptr)
{
    m_piUnderlyingPort->AddRef();

    HRESULT hr = m_piUnderlyingPort->QueryInterface(IID_IDebugDefaultPort2, (void**)(&m_piUnderlyingDefaultPort));
    GT_ASSERT(SUCCEEDED(hr));

    hr = m_piUnderlyingPort->QueryInterface(IID_IDebugPortNotify2, (void**)(&m_piUnderlyingPortNotify));

    if (m_piUnderlyingPortNotify == NULL)
    {
        hr = m_piUnderlyingDefaultPort->GetPortNotify(&m_piUnderlyingPortNotify);
    }

    GT_ASSERT(SUCCEEDED(hr));

    hr = m_piUnderlyingPort->QueryInterface(VSD_IID_IDebugNativePort2, (void**)(&m_piUnderlyingNativePort)); // TO_DO: find where IID_IDebugNativePort2 is exported)

    if (!SUCCEEDED(hr))
    {
        OS_OUTPUT_DEBUG_LOG(L"Native debug engine does not implement IDebugNativePort2.", OS_DEBUG_LOG_DEBUG);
    }

    hr = m_piUnderlyingPort->QueryInterface(IID_IDebugWindowsComputerPort2, (void**)(&m_piUnderlyingWindowsComputerPort));
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugPort::~vsdCDebugPort
// Description: Destructor
// Author:      Uri Shomroni
// Date:        26/12/2011
// ---------------------------------------------------------------------------
vsdCDebugPort::~vsdCDebugPort()
{
    GT_IF_WITH_ASSERT(m_piUnderlyingPort != NULL)
    {
        m_piUnderlyingPort->Release();
        m_piUnderlyingPort = NULL;
    }

    if (m_piUnderlyingDefaultPort != NULL)
    {
        m_piUnderlyingDefaultPort->Release();
        m_piUnderlyingDefaultPort = NULL;
    }

    if (m_piUnderlyingPortNotify != NULL)
    {
        m_piUnderlyingPortNotify->Release();
        m_piUnderlyingPortNotify = NULL;
    }

    if (m_piUnderlyingNativePort != NULL)
    {
        m_piUnderlyingNativePort->Release();
        m_piUnderlyingNativePort = NULL;
    }

    if (m_piUnderlyingWindowsComputerPort != NULL)
    {
        m_piUnderlyingWindowsComputerPort->Release();
        m_piUnderlyingWindowsComputerPort = NULL;
    }

    for (IDebugProgramNode2* pNode : m_debugProgramNodes)
    {
        if (nullptr != pNode)
        {
            pNode->Release();
        }
    }

    m_debugProgramNodes.clear();
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugPort::RegisteredProgramNode
// Description: If at least one program node was registered through this class's
//              IDebugPortNotify2 interface, return the first one. Otherwise, return nullptr
// Author:      Uri Shomroni
// Date:        28/3/2016
// ---------------------------------------------------------------------------
IDebugProgramNode2* vsdCDebugPort::RegisteredProgramNode() const
{
    IDebugProgramNode2* retVal = nullptr;

    int nodeCount = (int)m_debugProgramNodes.size();

    if (0 < nodeCount)
    {
        retVal = m_debugProgramNodes[0];
        GT_ASSERT(1 == nodeCount);
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugPort::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        26/12/2011
// ---------------------------------------------------------------------------
ULONG vsdCDebugPort::AddRef(void)
{
    return vsdCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugPort::Release
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        26/12/2010
// ---------------------------------------------------------------------------
ULONG vsdCDebugPort::Release(void)
{
    return vsdCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugPort::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        26/12/2011
// ---------------------------------------------------------------------------
HRESULT vsdCDebugPort::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if (ppvObj == NULL)
    {
        retVal = E_POINTER;
    }
    else if (riid == IID_IUnknown)
    {
        // Since multiple interfaces inherit IUnknown, we need to cast through one of them.
        // Note that we have to cast through the same one each time, to be consistent.
        *ppvObj = (IUnknown*)((IDebugPort2*)this);
        AddRef();
    }
    else if (riid == IID_IDebugPort2)
    {
        *ppvObj = (IDebugPort2*)this;
        AddRef();
    }
    else if (riid == IID_IDebugDefaultPort2)
    {
        *ppvObj = (IDebugDefaultPort2*)this;
        AddRef();
    }
    else if (riid == VSD_IID_IDebugNativePort2) // TO_DO: find where IID_IDebugNativePort2 is exported
    {
        if (NULL == m_piUnderlyingNativePort)
        {
            retVal = E_NOINTERFACE;
        }
        else
        {
            *ppvObj = (IDebugNativePort2*)this;
            AddRef();
        }
    }
    else if (riid == IID_IDebugPortNotify2)
    {
        *ppvObj = (IDebugPortNotify2*)this;
        AddRef();
    }
    else if (riid == IID_IDebugWindowsComputerPort2)
    {
        if (nullptr == m_piUnderlyingWindowsComputerPort)
        {
            retVal = E_NOINTERFACE;
        }
        else
        {
            *ppvObj = (IDebugWindowsComputerPort2*)this;
            AddRef();
        }
    }
    else // riid != IID_IUnknown, IID_IDebugPort2, IID_IDebugDefaultPort2, IID_IDebugNativePort2
    {
        // Getting here often with __uuidof(sdm::CDebugPort) = {C121E238-E7F7-492B-8EA1-E3B35908576B}
        // and IID_IMarshal = {00000003-0000-0000-C000-000000000046}.
        retVal = E_NOINTERFACE;
        static const GUID sdm_CDebugPort_guid = {0xC121E238, 0xE7F7, 0x492B, {0x8E, 0xA1, 0xE3, 0xB3, 0x59, 0x08, 0x57, 0x6B}};

        if ((sdm_CDebugPort_guid == riid) || (IID_IMarshal == riid))
        {
            retVal = m_piUnderlyingPort->QueryInterface(riid, ppvObj);
        }
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// IDebugPort2 methods
HRESULT vsdCDebugPort::GetPortName(BSTR* pbstrName)
{
    HRESULT hr = m_piUnderlyingPort->GetPortName(pbstrName);
    return hr;
}
HRESULT vsdCDebugPort::GetPortId(GUID* pguidPort)
{ GT_UNREFERENCED_PARAMETER(pguidPort); return E_NOTIMPL; }
HRESULT vsdCDebugPort::GetPortRequest(IDebugPortRequest2** ppRequest)
{ GT_UNREFERENCED_PARAMETER(ppRequest); return E_NOTIMPL; }
HRESULT vsdCDebugPort::GetPortSupplier(IDebugPortSupplier2** ppSupplier)
{ GT_UNREFERENCED_PARAMETER(ppSupplier); return E_NOTIMPL; }
HRESULT vsdCDebugPort::GetProcess(AD_PROCESS_ID ProcessId, IDebugProcess2** ppProcess)
{
    HRESULT hr = E_POINTER;

    if (ppProcess != NULL)
    {
        // Check our cache for this process:
        gtMap<AD_PROCESS_ID, vsdCDebugProcess*>::iterator findIter = m_processIDToProcess.find(ProcessId);
        gtMap<AD_PROCESS_ID, vsdCDebugProcess*>::iterator endIter = m_processIDToProcess.end();

        if (findIter != endIter)
        {
            // This process was already cached:
            vsdCDebugProcess* piProcess = (*findIter).second;
            *ppProcess = piProcess;
            piProcess->AddRef();
            hr = S_OK;
        }
        else // findIter == endIter
        {
            // This process was not yet cached, get the process from the real port and wrap it:
            IDebugProcess2* piRealProcess = NULL;
            hr = m_piUnderlyingPort->GetProcess(ProcessId, &piRealProcess);

            if (piRealProcess != NULL)
            {
                vsdCDebugProcess* piProcess = new vsdCDebugProcess(*piRealProcess, *this);
                *ppProcess = piProcess;
                piRealProcess->Release();

                // Cache it:
                m_processIDToProcess[ProcessId] = piProcess;
                piProcess->AddRef();
            }
        }
    }

    return hr;
}
HRESULT vsdCDebugPort::EnumProcesses(IEnumDebugProcesses2** ppEnum)
{ GT_UNREFERENCED_PARAMETER(ppEnum); return E_NOTIMPL; }

////////////////////////////////////////////////////////////
// IDebugDefaultPort2 methods
HRESULT vsdCDebugPort::GetPortNotify(IDebugPortNotify2** ppPortNotify)
{
    // HRESULT hr = m_piUnderlyingDefaultPort->GetPortNotify(ppPortNotify);
    HRESULT hr = E_POINTER;

    if (ppPortNotify != NULL)
    {
        *ppPortNotify = this;
        AddRef();
        hr = S_OK;
    }

    return hr;
}
HRESULT vsdCDebugPort::GetServer(IDebugCoreServer3** ppServer)
{
    HRESULT hr = m_piUnderlyingDefaultPort->GetServer(ppServer);
    return hr;
}
HRESULT vsdCDebugPort::QueryIsLocal(void)
{ return E_NOTIMPL; }

////////////////////////////////////////////////////////////
// IDebugNativePort2 methods
HRESULT vsdCDebugPort::AddProcess(AD_PROCESS_ID processId, LPCOLESTR pszProcessName, BOOL fCanDetach, IDebugProcess2** ppPortProcess)
{
    HRESULT hr = S_OK;

    if (NULL != m_piUnderlyingNativePort)
    {
        m_piUnderlyingNativePort->AddProcess(processId, pszProcessName, fCanDetach, ppPortProcess);
    }
    else
    {
        hr = E_NOTIMPL;
    }

    return hr;
}

////////////////////////////////////////////////////////////
// IDebugPortNotify2 methods
HRESULT vsdCDebugPort::AddProgramNode(IDebugProgramNode2* pProgramNode)
{
    HRESULT hr = m_piUnderlyingPortNotify->AddProgramNode(pProgramNode);

    m_debugProgramNodes.push_back(pProgramNode);
    pProgramNode->AddRef();

    return hr;
}
HRESULT vsdCDebugPort::RemoveProgramNode(IDebugProgramNode2* pProgramNode)
{
    HRESULT hr = m_piUnderlyingPortNotify->RemoveProgramNode(pProgramNode);

    int nodeCount = (int)m_debugProgramNodes.size();
    bool foundNode = false;

    for (int i = 0; i < nodeCount; ++i)
    {
        // This can only be with i > 0:
        if (foundNode)
        {
            m_debugProgramNodes[i - 1] = m_debugProgramNodes[i];
        }
        else if (m_debugProgramNodes[i] == pProgramNode)
        {
            foundNode = true;
        }
    }

    // Make sure we're not removing a node that was never added:
    GT_IF_WITH_ASSERT(foundNode)
    {
        // Remove the extra node (the one we're removing or the one that's a duplicate):
        m_debugProgramNodes.pop_back();

        // Release the object:
        pProgramNode->Release();
    }

    return hr;
}

////////////////////////////////////////////////////////////
// IDebugWindowsComputerPort2 methods
HRESULT vsdCDebugPort::GetComputerInfo(COMPUTER_INFO* pInfo)
{
    HRESULT hr = m_piUnderlyingWindowsComputerPort->GetComputerInfo(pInfo);

    return hr;
}

