//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file DiaFrameData.h
///
//==================================================================================

#ifndef _FRAMEDATA_H_
#define _FRAMEDATA_H_

#include <dia2.h>

#pragma warning(push)
#pragma warning(disable : 4201)  // nameless struct/union

class CDiaFrameData : public IDiaFrameData
{
public:
    CDiaFrameData();
    ~CDiaFrameData();

    virtual HRESULT STDMETHODCALLTYPE get_addressSection(DWORD* pRetVal);
    virtual HRESULT STDMETHODCALLTYPE get_addressOffset(DWORD* pRetVal);
    virtual HRESULT STDMETHODCALLTYPE get_relativeVirtualAddress(DWORD* pRetVal);
    virtual HRESULT STDMETHODCALLTYPE get_virtualAddress(ULONGLONG* pRetVal);
    virtual HRESULT STDMETHODCALLTYPE get_lengthBlock(DWORD* pRetVal);
    virtual HRESULT STDMETHODCALLTYPE get_lengthLocals(DWORD* pRetVal);
    virtual HRESULT STDMETHODCALLTYPE get_lengthParams(DWORD* pRetVal);
    virtual HRESULT STDMETHODCALLTYPE get_maxStack(DWORD* pRetVal);
    virtual HRESULT STDMETHODCALLTYPE get_lengthProlog(DWORD* pRetVal);
    virtual HRESULT STDMETHODCALLTYPE get_lengthSavedRegisters(DWORD* pRetVal);
    virtual HRESULT STDMETHODCALLTYPE get_program(BSTR* pRetVal);
    virtual HRESULT STDMETHODCALLTYPE get_systemExceptionHandling(BOOL* pRetVal);
    virtual HRESULT STDMETHODCALLTYPE get_cplusplusExceptionHandling(BOOL* pRetVal);
    virtual HRESULT STDMETHODCALLTYPE get_functionStart(BOOL* pRetVal);
    virtual HRESULT STDMETHODCALLTYPE get_allocatesBasePointer(BOOL* pRetVal);
    virtual HRESULT STDMETHODCALLTYPE get_type(DWORD* pRetVal);

    virtual HRESULT STDMETHODCALLTYPE get_functionParent(IDiaFrameData** pRetVal);
    virtual HRESULT STDMETHODCALLTYPE execute(IDiaStackWalkFrame* pFrame);

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();

private:
    volatile long m_refCount;

    DWORD m_relativeVirtualAddress;
    DWORD m_lengthBlock;
    DWORD m_lengthLocals;
    DWORD m_lengthParams;
    DWORD m_maxStack;

    WORD m_lengthProlog;
    WORD m_lengthSavedRegisters;
    ULONG m_systemExceptionHandling : 1;
    ULONG m_cplusplusExceptionHandling : 1;
    ULONG m_functionStart : 1;
    ULONG m_allocatesBasePointer : 1;
    BSTR m_pFrameCommand;
    DWORD m_addressSection;
    DWORD m_addressOffset;

    ULONGLONG m_virtualAddress;
    StackFrameTypeEnum m_type;


    union
    {
        struct
        {
            ULONG program : 1;
            ULONG addressSection : 1;
            ULONG addressOffset : 1;
            ULONG virtualAddress : 1;
            ULONG allocatesBasePointer : 1;
            ULONG relativeVirtualAddress : 1;
            ULONG lengthBlock : 1;
            ULONG lengthLocals : 1;
            ULONG lengthParams : 1;
            ULONG maxStack : 1;
            ULONG lengthProlog : 1;
            ULONG lengthSavedRegisters : 1;
            ULONG systemExceptionHandling : 1;
            ULONG cplusplusExceptionHandling : 1;
            ULONG functionStart : 1;
            ULONG type : 1;
            ULONG functionParent : 1;
        };

        ULONG value;
    } m_valid;


    friend class FrameChainWalker;
};

#pragma warning(pop)

#endif // _FRAMEDATA_H_
