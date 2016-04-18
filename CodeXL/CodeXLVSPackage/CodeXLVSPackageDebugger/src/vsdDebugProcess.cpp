//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vsdDebugProcess.cpp
///
//==================================================================================

//------------------------------ vsdDebugProcess.cpp ------------------------------

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>

// Local:
#include <src/vsdDebugProcess.h>


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// vsdCDebugProcess
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vsdCDebugProcess::vsdCDebugProcess
// Description: Constructor
// Author:      Uri Shomroni
// Date:        17/1/2012
// ---------------------------------------------------------------------------
vsdCDebugProcess::vsdCDebugProcess(IDebugProcess2& riUnderlyingProcess, IDebugPort2& riControllingPort)
    : m_piUnderlyingProcess(&riUnderlyingProcess), m_piUnderlyingProcess3(nullptr), m_riControllingPort(riControllingPort), m_piControlledEnumPrograms(NULL)
{
    m_piUnderlyingProcess->AddRef();

    // Get the IDebugProcess3 interface:
    HRESULT hr = m_piUnderlyingProcess->QueryInterface(IID_IDebugProcess3, (void**)(&m_piUnderlyingProcess3));
    GT_IF_WITH_ASSERT(SUCCEEDED(hr))
    {
        GT_ASSERT(nullptr != m_piUnderlyingProcess3);
    }

    m_riControllingPort.AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugProcess::~vsdCDebugProcess
// Description: Destructor
// Author:      Uri Shomroni
// Date:        17/1/2012
// ---------------------------------------------------------------------------
vsdCDebugProcess::~vsdCDebugProcess()
{
    GT_IF_WITH_ASSERT(m_piUnderlyingProcess != NULL)
    {
        m_piUnderlyingProcess->Release();
        m_piUnderlyingProcess = NULL;
    }

    if (nullptr != m_piUnderlyingProcess3)
    {
        m_piUnderlyingProcess3->Release();
        m_piUnderlyingProcess3 = nullptr;
    }

    m_riControllingPort.Release();

    if (m_piControlledEnumPrograms != NULL)
    {
        m_piControlledEnumPrograms->Release();
        m_piControlledEnumPrograms = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugProcess::setControlledEnumPrograms
// Description: Set an IEnumDebugPrograms2 interface to be provided by this process
//              instead of the real one
// Author:      Uri Shomroni
// Date:        25/1/2012
// ---------------------------------------------------------------------------
void vsdCDebugProcess::setControlledEnumPrograms(IEnumDebugPrograms2* piControlledEnumPrograms)
{
    GT_IF_WITH_ASSERT(m_piControlledEnumPrograms == NULL)
    {
        m_piControlledEnumPrograms = piControlledEnumPrograms;
        m_piControlledEnumPrograms->AddRef();
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugProcess::UnderlyingProcessEnumPrograms
// Description: Access the real IEnumDebugPrograms2 interface
// Author:      Uri Shomroni
// Date:        5/1/2016
// ---------------------------------------------------------------------------
HRESULT vsdCDebugProcess::UnderlyingProcessEnumPrograms(IEnumDebugPrograms2** ppEnum)
{
    HRESULT retVal = m_piUnderlyingProcess->EnumPrograms(ppEnum);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugProcess::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        17/1/2012
// ---------------------------------------------------------------------------
ULONG vsdCDebugProcess::AddRef(void)
{
    return vsdCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugProcess::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        26/12/2010
// ---------------------------------------------------------------------------
ULONG vsdCDebugProcess::Release(void)
{
    return vsdCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugProcess::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        17/1/2012
// ---------------------------------------------------------------------------
HRESULT vsdCDebugProcess::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if (ppvObj == NULL)
    {
        retVal = E_POINTER;
    }
    else if (riid == IID_IUnknown)
    {
        /* *ppvObj = (IUnknown*)(this);
        AddRef();*/

        // Uri, 18/1/12 - The comparison function CComPtrBase::IsEqualObject(IUnknown* pOther) is causing us to be misidentified by the SDM as a different
        // port than the real port. Therefore, we need to return the underlying port's IUnknown (and hope noone uses it to query for the port interface behind our backs.
        retVal = m_piUnderlyingProcess->QueryInterface(riid, ppvObj);
    }
    else if (riid == IID_IDebugProcess2)
    {
        *ppvObj = (IDebugProcess2*)this;
        AddRef();
    }
    else if ((IID_IDebugProcess3 == riid) && (nullptr != m_piUnderlyingProcess3))
    {
        *ppvObj = (IDebugProcess3*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IDebugProcess2
    {
        // Interfaces known to be queried here:
        // riid                                     interface
        // {0000013D-0000-0000-C000-000000000046}   IClientSecurity     not implemented
        // {00000003-0000-0000-C000-000000000046}   IMarshal            not implemented
        // {DE34E4B4-500B-487F-B643-CEE143F423FF}   sdm::CDebugProcess  implemented
        retVal = m_piUnderlyingProcess->QueryInterface(riid, ppvObj);
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// IDebugProcess2 methods
HRESULT vsdCDebugProcess::GetInfo(PROCESS_INFO_FIELDS Fields, PROCESS_INFO* pProcessInfo)
{
    HRESULT retVal = m_piUnderlyingProcess->GetInfo(Fields, pProcessInfo);

    return retVal;
}

HRESULT vsdCDebugProcess::EnumPrograms(IEnumDebugPrograms2** ppEnum)
{
    IEnumDebugPrograms2* pEnum = NULL;
    IEnumDebugPrograms2** ppEn = (nullptr != ppEnum) ? &pEnum : ppEnum;

    HRESULT retVal = S_OK;

    if (m_piControlledEnumPrograms != NULL)
    {
        retVal = m_piControlledEnumPrograms->Clone(ppEnum);
    }
    else // m_piControlledEnumPrograms == NULL
    {
        // Get the real enum programs interface:
        retVal = m_piUnderlyingProcess->EnumPrograms(ppEn);

        if (ppEnum != NULL)
        {
            vsdCEnumDebugPrograms* pEnumWrap = NULL;

            if (pEnum != NULL)
            {
                pEnumWrap = new vsdCEnumDebugPrograms(*pEnum);
            }

            *ppEnum = pEnumWrap;
        }

        setControlledEnumPrograms(*ppEn);
    }

    return retVal;
}

HRESULT vsdCDebugProcess::GetName(GETNAME_TYPE gnType, BSTR* pbstrName)
{
    HRESULT retVal = m_piUnderlyingProcess->GetName(gnType, pbstrName);

    return retVal;
}

HRESULT vsdCDebugProcess::GetServer(IDebugCoreServer2** ppServer)
{
    HRESULT retVal = m_piUnderlyingProcess->GetServer(ppServer);

    return retVal;
}

HRESULT vsdCDebugProcess::Terminate(void)
{
    HRESULT retVal = m_piUnderlyingProcess->Terminate();

    return retVal;
}

HRESULT vsdCDebugProcess::Attach(IDebugEventCallback2* pCallback, GUID* rgguidSpecificEngines, DWORD celtSpecificEngines, HRESULT* rghrEngineAttach)
{
    HRESULT retVal = m_piUnderlyingProcess->Attach(pCallback, rgguidSpecificEngines, celtSpecificEngines, rghrEngineAttach);

    return retVal;
}

HRESULT vsdCDebugProcess::CanDetach(void)
{
    HRESULT retVal = m_piUnderlyingProcess->CanDetach();

    return retVal;
}

HRESULT vsdCDebugProcess::Detach(void)
{
    HRESULT retVal = m_piUnderlyingProcess->Detach();

    return retVal;
}

HRESULT vsdCDebugProcess::GetPhysicalProcessId(AD_PROCESS_ID* pProcessId)
{
    HRESULT retVal = m_piUnderlyingProcess->GetPhysicalProcessId(pProcessId);

    return retVal;
}

HRESULT vsdCDebugProcess::GetProcessId(GUID* pguidProcessId)
{
    HRESULT retVal = m_piUnderlyingProcess->GetProcessId(pguidProcessId);

    return retVal;
}

HRESULT vsdCDebugProcess::GetAttachedSessionName(BSTR* pbstrSessionName)
{
    HRESULT retVal = m_piUnderlyingProcess->GetAttachedSessionName(pbstrSessionName);

    return retVal;
}

HRESULT vsdCDebugProcess::EnumThreads(IEnumDebugThreads2** ppEnum)
{
    HRESULT retVal = m_piUnderlyingProcess->EnumThreads(ppEnum);

    return retVal;
}

HRESULT vsdCDebugProcess::CauseBreak(void)
{
    // Filter out this ability:
    HRESULT retVal = m_piUnderlyingProcess->CauseBreak();

    return (S_OK == retVal) ? retVal : E_NOTIMPL;
}

HRESULT vsdCDebugProcess::GetPort(IDebugPort2** ppPort)
{
    HRESULT retVal = S_OK;

    if (ppPort == NULL)
    {
        retVal = E_POINTER;
    }
    else // ppPort != NULL
    {
        // retVal = m_piUnderlyingProcess->GetPort(ppPort);
        *ppPort = &m_riControllingPort;
        m_riControllingPort.AddRef();
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// IDebugProcess3 methods
HRESULT vsdCDebugProcess::Execute(IDebugThread2* pThread)
{
    HRESULT retVal = E_NOTIMPL;

    if (nullptr != m_piUnderlyingProcess3)
    {
        retVal = m_piUnderlyingProcess3->Execute(pThread);
    }

    return retVal;
}

HRESULT vsdCDebugProcess::Continue(IDebugThread2* pThread)
{
    HRESULT retVal = E_NOTIMPL;

    if (nullptr != m_piUnderlyingProcess3)
    {
        retVal = m_piUnderlyingProcess3->Continue(pThread);
    }

    return retVal;
}

HRESULT vsdCDebugProcess::Step(IDebugThread2* pThread, STEPKIND sk, STEPUNIT step)
{
    HRESULT retVal = E_NOTIMPL;

    if (nullptr != m_piUnderlyingProcess3)
    {
        retVal = m_piUnderlyingProcess3->Step(pThread, sk, step);
    }

    return retVal;
}

HRESULT vsdCDebugProcess::GetDebugReason(DEBUG_REASON* pReason)
{
    HRESULT retVal = E_NOTIMPL;

    if (nullptr != m_piUnderlyingProcess3)
    {
        retVal = m_piUnderlyingProcess3->GetDebugReason(pReason);
    }

    return retVal;
}

HRESULT vsdCDebugProcess::SetHostingProcessLanguage(REFGUID guidLang)
{
    HRESULT retVal = E_NOTIMPL;

    if (nullptr != m_piUnderlyingProcess3)
    {
        retVal = m_piUnderlyingProcess3->SetHostingProcessLanguage(guidLang);
    }

    return retVal;
}

HRESULT vsdCDebugProcess::GetHostingProcessLanguage(GUID* pguidLang)
{
    HRESULT retVal = E_NOTIMPL;

    if (nullptr != m_piUnderlyingProcess3)
    {
        retVal = m_piUnderlyingProcess3->GetHostingProcessLanguage(pguidLang);
    }

    return retVal;
}

HRESULT vsdCDebugProcess::DisableENC(EncUnavailableReason reason)
{
    HRESULT retVal = E_NOTIMPL;

    if (nullptr != m_piUnderlyingProcess3)
    {
        retVal = m_piUnderlyingProcess3->DisableENC(reason);
    }

    return retVal;
}

HRESULT vsdCDebugProcess::GetENCAvailableState(EncUnavailableReason* preason)
{
    HRESULT retVal = E_NOTIMPL;

    if (nullptr != m_piUnderlyingProcess3)
    {
        retVal = m_piUnderlyingProcess3->GetENCAvailableState(preason);
    }

    return retVal;
}

HRESULT vsdCDebugProcess::GetEngineFilter(GUID_ARRAY* pEngineArray)
{
    HRESULT retVal = E_NOTIMPL;

    if (nullptr != m_piUnderlyingProcess3)
    {
        retVal = m_piUnderlyingProcess3->GetEngineFilter(pEngineArray);
    }

    return retVal;
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// vsdCEnumDebugPrograms
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vsdCEnumDebugPrograms::vsdCEnumDebugPrograms
// Description: Constructor
// Author:      Uri Shomroni
// Date:        19/1/2012
// ---------------------------------------------------------------------------
vsdCEnumDebugPrograms::vsdCEnumDebugPrograms(IEnumDebugPrograms2& riUnderlyingEnumPrograms)
    : m_piUnderlyingEnumPrograms(&riUnderlyingEnumPrograms)
{
    m_piUnderlyingEnumPrograms->AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vsdCEnumDebugPrograms::~vsdCEnumDebugPrograms
// Description: Destructor
// Author:      Uri Shomroni
// Date:        19/1/2012
// ---------------------------------------------------------------------------
vsdCEnumDebugPrograms::~vsdCEnumDebugPrograms()
{
    GT_IF_WITH_ASSERT(m_piUnderlyingEnumPrograms != NULL)
    {
        m_piUnderlyingEnumPrograms->Release();
        m_piUnderlyingEnumPrograms = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdCEnumDebugPrograms::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        19/1/2012
// ---------------------------------------------------------------------------
ULONG vsdCEnumDebugPrograms::AddRef(void)
{
    return vsdCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vsdCEnumDebugPrograms::Release
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        19/1/2012
// ---------------------------------------------------------------------------
ULONG vsdCEnumDebugPrograms::Release(void)
{
    return vsdCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vsdCEnumDebugPrograms::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        19/1/2012
// ---------------------------------------------------------------------------
HRESULT vsdCEnumDebugPrograms::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if (ppvObj == NULL)
    {
        retVal = E_POINTER;
    }
    else if (riid == IID_IUnknown)
    {
        *ppvObj = (IUnknown*)(this);
        AddRef();
    }
    else if (riid == IID_IEnumDebugPrograms2)
    {
        *ppvObj = (IEnumDebugPrograms2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IEnumDebugPrograms2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// IEnumDebugPrograms2 methods
HRESULT vsdCEnumDebugPrograms::Next(ULONG celt, IDebugProgram2** rgelt, ULONG* pceltFetched)
{
    ULONG fetched = 0;
    ULONG* pFetched = (pceltFetched != NULL) ? &fetched : pceltFetched;
    IDebugProgram2** ppRealPrograms = new IDebugProgram2*[celt];
    IDebugProgram2** ppRGElt = (rgelt != NULL) ? ppRealPrograms : rgelt;

    HRESULT retVal = m_piUnderlyingEnumPrograms->Next(celt, ppRGElt, pFetched);

    if (rgelt != NULL)
    {
        for (ULONG i = 0; (i < fetched) && (i < celt); i++)
        {
            if (ppRealPrograms[i] != NULL)
            {
                rgelt[i] = new vsdCDebugProgram(*(ppRealPrograms[i]));
            }
            else
            {
                rgelt[i] = NULL;
            }
        }
    }

    delete[] ppRealPrograms;

    if (pceltFetched != NULL)
    {
        *pceltFetched = fetched;
    }

    return retVal;
}
HRESULT vsdCEnumDebugPrograms::Skip(ULONG celt)
{
    HRESULT retVal = m_piUnderlyingEnumPrograms->Skip(celt);

    return retVal;
}
HRESULT vsdCEnumDebugPrograms::Reset(void)
{
    HRESULT retVal = m_piUnderlyingEnumPrograms->Reset();

    return retVal;
}
HRESULT vsdCEnumDebugPrograms::Clone(IEnumDebugPrograms2** ppEnum)
{
    IEnumDebugPrograms2* pEnum = NULL;
    IEnumDebugPrograms2** ppEn = (ppEnum != NULL) ? &pEnum : ppEnum;
    HRESULT retVal = m_piUnderlyingEnumPrograms->Clone(ppEn);

    if (ppEnum != NULL)
    {
        vsdCEnumDebugPrograms* pEnumWrap = NULL;

        if (pEnum != NULL)
        {
            pEnumWrap = new vsdCEnumDebugPrograms(*pEnum);
        }

        *ppEnum = pEnumWrap;
    }

    return retVal;
}
HRESULT vsdCEnumDebugPrograms::GetCount(ULONG* pcelt)
{
    HRESULT retVal = m_piUnderlyingEnumPrograms->GetCount(pcelt);

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// vsdCSingleProgramEnumDebugPrograms
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vsdCSingleProgramEnumDebugPrograms::vsdCSingleProgramEnumDebugPrograms
// Description: Constructor
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
vsdCSingleProgramEnumDebugPrograms::vsdCSingleProgramEnumDebugPrograms(IDebugProgram2* piProgram)
    : _piEnumProgram(piProgram), _wasEnumeratorUsed(false)
{
    if (_piEnumProgram != NULL)
    {
        _piEnumProgram->AddRef();
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdCSingleProgramEnumDebugPrograms::~vsdCSingleProgramEnumDebugPrograms
// Description: Destructor
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
vsdCSingleProgramEnumDebugPrograms::~vsdCSingleProgramEnumDebugPrograms()
{
    if (_piEnumProgram != NULL)
    {
        _piEnumProgram->Release();
        _piEnumProgram = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdCSingleProgramEnumDebugPrograms::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
ULONG vsdCSingleProgramEnumDebugPrograms::AddRef(void)
{
    return vsdCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vsdCSingleProgramEnumDebugPrograms::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
ULONG vsdCSingleProgramEnumDebugPrograms::Release(void)
{
    return vsdCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vsdCSingleProgramEnumDebugPrograms::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
HRESULT vsdCSingleProgramEnumDebugPrograms::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if (ppvObj == NULL)
    {
        retVal = E_POINTER;
    }
    else if (riid == IID_IUnknown)
    {
        *ppvObj = (IUnknown*)this;
        AddRef();
    }
    else if (riid == IID_IEnumDebugPrograms2)
    {
        *ppvObj = (IEnumDebugPrograms2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IEnumDebugPrograms2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
// IEnumDebugPrograms2 methods
HRESULT vsdCSingleProgramEnumDebugPrograms::Next(ULONG celt, IDebugProgram2** rgelt, ULONG* pceltFetched)
{
    HRESULT retVal = S_OK;

    // Will get the amount of breakpoints (1 or 0) we successfully returned:
    ULONG fetchedItems = 0;

    if (rgelt != NULL)
    {
        ULONG canReturnItems = (_wasEnumeratorUsed || (_piEnumProgram == NULL)) ? 0 : 1;

        // If the caller requested more items than we can supply, return S_FALSE.
        if (celt > canReturnItems)
        {
            retVal = S_FALSE;
        }

        // If we can return the breakpoint and the user requested it, set it into rgelt:
        if ((celt > 0) && (!_wasEnumeratorUsed) && (_piEnumProgram != NULL))
        {
            rgelt[0] = _piEnumProgram;
            _piEnumProgram->AddRef();
            _wasEnumeratorUsed = true;
            fetchedItems = 1;
        }
    }
    else // rgelt == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    // If the caller requested the fetched amount, return it:
    if (pceltFetched != NULL)
    {
        *pceltFetched = fetchedItems;
    }

    return retVal;
}
HRESULT vsdCSingleProgramEnumDebugPrograms::Skip(ULONG celt)
{
    HRESULT retVal = S_OK;

    ULONG canReturnItems = (_wasEnumeratorUsed || (_piEnumProgram == NULL)) ? 0 : 1;

    // If the caller requested more items than we can supply, return S_FALSE.
    if (celt > canReturnItems)
    {
        retVal = S_FALSE;
    }

    // If the enumerator was not yet used and the user requested it, mark it as used:
    if (celt > 0)
    {
        _wasEnumeratorUsed = true;
    }

    return retVal;
}
HRESULT vsdCSingleProgramEnumDebugPrograms::Reset(void)
{
    HRESULT retVal = S_OK;

    // Mark the enumerator as new:
    _wasEnumeratorUsed = false;

    return retVal;
}
HRESULT vsdCSingleProgramEnumDebugPrograms::Clone(IEnumDebugPrograms2** ppEnum)
{
    HRESULT retVal = S_OK;

    if (ppEnum != NULL)
    {
        // Create a duplicate of this item (note that this will increment the breakpoint's reference count:
        vsdCSingleProgramEnumDebugPrograms* pClone = new vsdCSingleProgramEnumDebugPrograms(_piEnumProgram);

        // Set its used status to equal ours:
        pClone->_wasEnumeratorUsed = _wasEnumeratorUsed;

        // Return it:
        *ppEnum = (IEnumDebugPrograms2*)pClone;
    }
    else // ppEnum == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vsdCSingleProgramEnumDebugPrograms::GetCount(ULONG* pcelt)
{
    HRESULT retVal = S_OK;

    if (pcelt != NULL)
    {
        // Return the count:
        *pcelt = (_piEnumProgram != NULL) ? 1 : 0;
    }
    else
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// vsdCDebugProgram
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vsdCDebugProgram::vsdCDebugProgram
// Description: Constructor
// Author:      Uri Shomroni
// Date:        19/1/2012
// ---------------------------------------------------------------------------
vsdCDebugProgram::vsdCDebugProgram(IDebugProgram2& riUnderlyingProgram)
    : m_piUnderlyingProgram(&riUnderlyingProgram)
{
    m_piUnderlyingProgram->AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugProgram::~vsdCDebugProgram
// Description: Destructor
// Author:      Uri Shomroni
// Date:        19/1/2012
// ---------------------------------------------------------------------------
vsdCDebugProgram::~vsdCDebugProgram()
{
    GT_IF_WITH_ASSERT(m_piUnderlyingProgram != NULL)
    {
        m_piUnderlyingProgram->Release();
        m_piUnderlyingProgram = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugProgram::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        19/1/2012
// ---------------------------------------------------------------------------
ULONG vsdCDebugProgram::AddRef(void)
{
    return vsdCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugProgram::Release
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        19/1/2012
// ---------------------------------------------------------------------------
ULONG vsdCDebugProgram::Release(void)
{
    return vsdCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugProgram::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        19/1/2012
// ---------------------------------------------------------------------------
HRESULT vsdCDebugProgram::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if (ppvObj == NULL)
    {
        retVal = E_POINTER;
    }
    else if (riid == IID_IUnknown)
    {
        *ppvObj = (IUnknown*)(this);
        AddRef();
    }
    else if (riid == IID_IDebugProgram2)
    {
        *ppvObj = (IDebugProgram2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IDebugProgram2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// IDebugProgram2 methods
HRESULT vsdCDebugProgram::EnumThreads(IEnumDebugThreads2** ppEnum)
{
    HRESULT retVal = m_piUnderlyingProgram->EnumThreads(ppEnum);

    return retVal;
}
HRESULT vsdCDebugProgram::GetName(BSTR* pbstrName)
{
    HRESULT retVal = m_piUnderlyingProgram->GetName(pbstrName);

    return retVal;
}
HRESULT vsdCDebugProgram::GetProcess(IDebugProcess2** ppProcess)
{
    HRESULT retVal = m_piUnderlyingProgram->GetProcess(ppProcess);

    return retVal;
}
HRESULT vsdCDebugProgram::Terminate(void)
{
    HRESULT retVal = m_piUnderlyingProgram->Terminate();

    return retVal;
}
HRESULT vsdCDebugProgram::Attach(IDebugEventCallback2* pCallback)
{
    HRESULT retVal = m_piUnderlyingProgram->Attach(pCallback);

    return retVal;
}
HRESULT vsdCDebugProgram::CanDetach(void)
{
    HRESULT retVal = m_piUnderlyingProgram->CanDetach();

    return retVal;
}
HRESULT vsdCDebugProgram::Detach(void)
{
    HRESULT retVal = m_piUnderlyingProgram->Detach();

    return retVal;
}
HRESULT vsdCDebugProgram::GetProgramId(GUID* pguidProgramId)
{
    HRESULT retVal = m_piUnderlyingProgram->GetProgramId(pguidProgramId);

    return retVal;
}
HRESULT vsdCDebugProgram::GetDebugProperty(IDebugProperty2** ppProperty)
{
    HRESULT retVal = m_piUnderlyingProgram->GetDebugProperty(ppProperty);

    return retVal;
}
HRESULT vsdCDebugProgram::Execute(void)
{
    HRESULT retVal = m_piUnderlyingProgram->Execute();

    return retVal;
}
HRESULT vsdCDebugProgram::Continue(IDebugThread2* pThread)
{
    HRESULT retVal = m_piUnderlyingProgram->Continue(pThread);

    return retVal;
}
HRESULT vsdCDebugProgram::Step(IDebugThread2* pThread, STEPKIND sk, STEPUNIT step)
{
    HRESULT retVal = m_piUnderlyingProgram->Step(pThread, sk, step);

    return retVal;
}
HRESULT vsdCDebugProgram::CauseBreak(void)
{
    HRESULT retVal = m_piUnderlyingProgram->CauseBreak();

    return retVal;
}
HRESULT vsdCDebugProgram::GetEngineInfo(BSTR* pbstrEngine, GUID* pguidEngine)
{
    HRESULT retVal = m_piUnderlyingProgram->GetEngineInfo(pbstrEngine, pguidEngine);

    return retVal;
}
HRESULT vsdCDebugProgram::EnumCodeContexts(IDebugDocumentPosition2* pDocPos, IEnumDebugCodeContexts2** ppEnum)
{
    HRESULT retVal = m_piUnderlyingProgram->EnumCodeContexts(pDocPos, ppEnum);

    return retVal;
}
HRESULT vsdCDebugProgram::GetMemoryBytes(IDebugMemoryBytes2** ppMemoryBytes)
{
    HRESULT retVal = m_piUnderlyingProgram->GetMemoryBytes(ppMemoryBytes);

    return retVal;
}
HRESULT vsdCDebugProgram::GetDisassemblyStream(DISASSEMBLY_STREAM_SCOPE dwScope, IDebugCodeContext2* pCodeContext, IDebugDisassemblyStream2** ppDisassemblyStream)
{
    HRESULT retVal = m_piUnderlyingProgram->GetDisassemblyStream(dwScope, pCodeContext, ppDisassemblyStream);

    return retVal;
}
HRESULT vsdCDebugProgram::EnumModules(IEnumDebugModules2** ppEnum)
{
    HRESULT retVal = m_piUnderlyingProgram->EnumModules(ppEnum);

    return retVal;
}
HRESULT vsdCDebugProgram::GetENCUpdate(IDebugENCUpdate** ppUpdate)
{
    HRESULT retVal = m_piUnderlyingProgram->GetENCUpdate(ppUpdate);

    return retVal;
}
HRESULT vsdCDebugProgram::EnumCodePaths(LPCOLESTR pszHint, IDebugCodeContext2* pStart, IDebugStackFrame2* pFrame, BOOL fSource, IEnumCodePaths2** ppEnum, IDebugCodeContext2** ppSafety)
{
    HRESULT retVal = m_piUnderlyingProgram->EnumCodePaths(pszHint, pStart, pFrame, fSource, ppEnum, ppSafety);

    return retVal;
}
HRESULT vsdCDebugProgram::WriteDump(DUMPTYPE DumpType, LPCOLESTR pszDumpUrl)
{
    HRESULT retVal = m_piUnderlyingProgram->WriteDump(DumpType, pszDumpUrl);

    return retVal;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// vsdCDebugProgramNode
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vsdCDebugProgramNode::vsdCDebugProgramNode
// Description: Constructor
// Author:      Uri Shomroni
// Date:        19/1/2012
// ---------------------------------------------------------------------------
vsdCDebugProgramNode::vsdCDebugProgramNode(IDebugProgramNode2& riUnderlyingProgramNode)
    : m_piUnderlyingProgramNode(&riUnderlyingProgramNode)
{
    m_piUnderlyingProgramNode->AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugProgramNode::~vsdCDebugProgramNode
// Description: Destructor
// Author:      Uri Shomroni
// Date:        19/1/2012
// ---------------------------------------------------------------------------
vsdCDebugProgramNode::~vsdCDebugProgramNode()
{
    GT_IF_WITH_ASSERT(m_piUnderlyingProgramNode != NULL)
    {
        m_piUnderlyingProgramNode->Release();
        m_piUnderlyingProgramNode = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugProgramNode::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        19/1/2012
// ---------------------------------------------------------------------------
ULONG vsdCDebugProgramNode::AddRef(void)
{
    return vsdCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugProgramNode::Release
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        19/1/2012
// ---------------------------------------------------------------------------
ULONG vsdCDebugProgramNode::Release(void)
{
    return vsdCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vsdCDebugProgramNode::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        19/1/2012
// ---------------------------------------------------------------------------
HRESULT vsdCDebugProgramNode::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if (ppvObj == NULL)
    {
        retVal = E_POINTER;
    }
    else if (riid == IID_IUnknown)
    {
        *ppvObj = (IUnknown*)(this);
        AddRef();
    }
    else if (riid == IID_IDebugProgramNode2)
    {
        *ppvObj = (IDebugProgramNode2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IDebugProgramNode2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// IDebugProgramNode2 methods
HRESULT vsdCDebugProgramNode::GetProgramName(BSTR* pbstrProgramName)
{
    HRESULT retVal = m_piUnderlyingProgramNode->GetProgramName(pbstrProgramName);

    return retVal;
}
HRESULT vsdCDebugProgramNode::GetHostName(GETHOSTNAME_TYPE dwHostNameType, BSTR* pbstrHostName)
{
    HRESULT retVal = m_piUnderlyingProgramNode->GetHostName(dwHostNameType, pbstrHostName);

    return retVal;
}
HRESULT vsdCDebugProgramNode::GetHostPid(AD_PROCESS_ID* pHostProcessId)
{
    HRESULT retVal = m_piUnderlyingProgramNode->GetHostPid(pHostProcessId);

    return retVal;
}
HRESULT vsdCDebugProgramNode::GetHostMachineName_V7(BSTR* pbstrHostMachineName)
{
    HRESULT retVal = m_piUnderlyingProgramNode->GetHostMachineName_V7(pbstrHostMachineName);

    return retVal;
}
HRESULT vsdCDebugProgramNode::Attach_V7(IDebugProgram2* pMDMProgram, IDebugEventCallback2* pCallback, DWORD dwReason)
{
    HRESULT retVal = m_piUnderlyingProgramNode->Attach_V7(pMDMProgram, pCallback, dwReason);

    return retVal;
}
HRESULT vsdCDebugProgramNode::GetEngineInfo(BSTR* pbstrEngine, GUID* pguidEngine)
{
    HRESULT retVal = m_piUnderlyingProgramNode->GetEngineInfo(pbstrEngine, pguidEngine);

    return retVal;
}
HRESULT vsdCDebugProgramNode::DetachDebugger_V7(void)
{
    HRESULT retVal = m_piUnderlyingProgramNode->DetachDebugger_V7();

    return retVal;
}

