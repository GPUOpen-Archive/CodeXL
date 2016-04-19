//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DiaFrameData.cpp
///
//==================================================================================

#include <AMDTBaseTools/Include/AMDTDefinitions.h>
#include "DiaFrameData.h"

CDiaFrameData::CDiaFrameData() : m_refCount(0), m_pFrameCommand(NULL)
{
    m_valid.value = 0UL;
}

CDiaFrameData::~CDiaFrameData()
{
    if (NULL != m_pFrameCommand)
    {
        SysFreeString(m_pFrameCommand);
    }
}

HRESULT STDMETHODCALLTYPE CDiaFrameData::get_program(BSTR* pRetVal)
{
    HRESULT hr;

    if (NULL != pRetVal)
    {
        if (FALSE != m_valid.program)
        {
            *pRetVal = SysAllocString(m_pFrameCommand);
            hr = (NULL != *pRetVal) ? S_OK : E_OUTOFMEMORY;
        }
        else
        {
            *pRetVal = 0;
            hr = S_FALSE;
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

HRESULT STDMETHODCALLTYPE CDiaFrameData::get_functionParent(IDiaFrameData** pRetVal)
{
    GT_UNREFERENCED_PARAMETER(pRetVal);
    return E_UNEXPECTED;
}

HRESULT STDMETHODCALLTYPE CDiaFrameData::execute(IDiaStackWalkFrame* pFrame)
{
    GT_UNREFERENCED_PARAMETER(pFrame);
    return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE CDiaFrameData::QueryInterface(REFIID riid, void** ppvObject)
{
    HRESULT hr;

    if (NULL == ppvObject)
    {
        hr = E_POINTER;
    }
    else
    {
        if (__uuidof(IDiaFrameData) == riid || __uuidof(IUnknown) == riid)
        {
            *ppvObject = this;
            AddRef();
            hr = S_OK;
        }
        else
        {
            hr = E_NOINTERFACE;
        }
    }

    return hr;
}

ULONG STDMETHODCALLTYPE CDiaFrameData::AddRef()
{
    return static_cast<ULONG>(_InterlockedIncrement(&m_refCount));
}

ULONG STDMETHODCALLTYPE CDiaFrameData::Release()
{
    ULONG refCount = static_cast<ULONG>(_InterlockedDecrement(&m_refCount));

    if (0UL == refCount)
    {
        delete this;
    }

    return refCount;
}

#define DEF_VIRTUAL_PROPERTY(Type, name) \
    HRESULT STDMETHODCALLTYPE CDiaFrameData::get_##name(Type *pRetVal) \
    {\
        HRESULT hr;\
        \
        if (NULL != pRetVal)\
        {\
            if (FALSE != m_valid.name)\
            {\
                *pRetVal = m_##name;\
                hr = S_OK;\
            }\
            else\
            {\
                *pRetVal = 0;\
                hr = S_FALSE;\
            }\
        }\
        else\
        {\
            hr = E_INVALIDARG;\
        }\
        \
        return hr;\
    }

DEF_VIRTUAL_PROPERTY(DWORD, addressSection)
DEF_VIRTUAL_PROPERTY(DWORD, addressOffset)
DEF_VIRTUAL_PROPERTY(DWORD, relativeVirtualAddress)
DEF_VIRTUAL_PROPERTY(ULONGLONG, virtualAddress)
DEF_VIRTUAL_PROPERTY(DWORD, lengthBlock)
DEF_VIRTUAL_PROPERTY(DWORD, lengthLocals)
DEF_VIRTUAL_PROPERTY(DWORD, lengthParams)
DEF_VIRTUAL_PROPERTY(DWORD, maxStack)
DEF_VIRTUAL_PROPERTY(DWORD, lengthProlog)
DEF_VIRTUAL_PROPERTY(DWORD, lengthSavedRegisters)
DEF_VIRTUAL_PROPERTY(BOOL, systemExceptionHandling)
DEF_VIRTUAL_PROPERTY(BOOL, cplusplusExceptionHandling)
DEF_VIRTUAL_PROPERTY(BOOL, functionStart)
DEF_VIRTUAL_PROPERTY(BOOL, allocatesBasePointer)
DEF_VIRTUAL_PROPERTY(DWORD, type)
