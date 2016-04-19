//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscDebugContext.h
///
//==================================================================================

//------------------------------ vspDebugContext.h ------------------------------

#ifndef __VSPDEBUGCONTEXT_H
#define __VSPDEBUGCONTEXT_H

// Visual Studio:
#include <msdbg100.h>

// Infra:
#include <AMDTBaseTools/Include/gtVector.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osOSDefinitions.h>

// Local:
#include <src/vspUnknown.h>

class vspCDebugEngine;


// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugContext : public IDebugDocumentContext2, IDebugCodeContext2
// General Description: A debug context.
//                      Implements IDebugDocumentContext2, representing a location in the source
//                      Implements IDebugCodeContext2, representing a location in the binary
//                      Implements IDebugExpressionContext2, representing an expression handler
// Author:               Uri Shomroni
// Creation Date:        15/9/2010
// ----------------------------------------------------------------------------------
class vspCDebugContext : public IDebugDocumentContext2, public IDebugCodeContext2, public IDebugCodeContext100, public IDebugExpressionContext2, vspCUnknown
{
public:
    vspCDebugContext(const osFilePath& moduleFilePath, gtString& functionName, const osFilePath& sourceFilePath, int sourceLineNumber, osInstructionPointer instructionCounter, osInstructionPointer offset, bool isKernelDebuggingContext, osThreadId threadId, int frameIndex, vspCDebugEngine* pDebugEngine);
    virtual ~vspCDebugContext();

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugDocumentContext2 methods
    STDMETHOD(GetDocument)(IDebugDocument2** ppDocument);
    STDMETHOD(GetName)(GETNAME_TYPE gnType, BSTR* pbstrFileName);
    STDMETHOD(EnumCodeContexts)(IEnumDebugCodeContexts2** ppEnumCodeCxts);
    STDMETHOD(GetLanguageInfo)(BSTR* pbstrLanguage, GUID* pguidLanguage);
    STDMETHOD(GetStatementRange)(
        TEXT_POSITION* pBegPosition,
        TEXT_POSITION* pEndPosition);
    STDMETHOD(GetSourceRange)(TEXT_POSITION* pBegPosition, TEXT_POSITION* pEndPosition);
    STDMETHOD(Compare)(
        DOCCONTEXT_COMPARE compare,
        IDebugDocumentContext2** rgpDocContextSet,
        DWORD dwDocContextSetLen,
        DWORD* pdwDocContext);
    STDMETHOD(Seek)(int ncount, IDebugDocumentContext2** ppDocContext);

    ////////////////////////////////////////////////////////////
    // IDebugCodeContext2 methods
    STDMETHOD(GetDocumentContext)(IDebugDocumentContext2** ppSrcCxt);
    // GetLanguageInfo is declared in the IDebugDocumentContext2 interface.

    ////////////////////////////////////////////////////////////
    // IDebugCodeContext2 methods inherited from IDebugMemoryContext2
    STDMETHOD(GetName)(BSTR* pbstrName);
    STDMETHOD(GetInfo)(CONTEXT_INFO_FIELDS dwFields, CONTEXT_INFO* pInfo);
    STDMETHOD(Add)(UINT64 dwCount, IDebugMemoryContext2** ppMemCxt);
    STDMETHOD(Subtract)(UINT64 dwCount, IDebugMemoryContext2** ppMemCxt);
    STDMETHOD(Compare)(
        CONTEXT_COMPARE compare,
        IDebugMemoryContext2** rgpMemoryContextSet,
        DWORD dwMemoryContextSetLen,
        DWORD* pdwMemoryContext);

    ////////////////////////////////////////////////////////////
    // IDebugCodeContext100 methods
    STDMETHOD(GetProgram)(IDebugProgram2** ppProgram);

    ////////////////////////////////////////////////////////////
    // IDebugExpressionContext2 methods
    // GetName is declared in the IDebugDocumentContext2 interface.
    STDMETHOD(ParseText)(LPCOLESTR pszCode, PARSEFLAGS dwFlags, UINT nRadix, IDebugExpression2** ppExpr, BSTR* pbstrError, UINT* pichError);

private:
    // Do not allow use of my default constructor:
    vspCDebugContext();

private:
    osFilePath _moduleFilePath;
    gtString _functionName;
    osFilePath _sourceFilePath;
    int _sourceLineNumber;
    osInstructionPointer _instructionCounter;
    osInstructionPointer _offset;
    bool m_isKernelDebuggingContext;
    osThreadId m_threadId;
    int m_frameIndex;
    vspCDebugEngine* m_pDebugEngine;
};

// ----------------------------------------------------------------------------------
// Class Name:          vspCEnumDebugCodeContexts : public IEnumDebugCodeContexts2
// General Description: Implements IEnumDebugContexts2, Enumerating the currently existing Contexts
// Author:               Uri Shomroni
// Creation Date:        23/7/2013
// ----------------------------------------------------------------------------------
class vspCEnumDebugCodeContexts : public IEnumDebugCodeContexts2, vspCUnknown
{
public:
    vspCEnumDebugCodeContexts(const gtVector<vspCDebugContext*>& currentContexts);
    virtual ~vspCEnumDebugCodeContexts();

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IEnumDebugContexts2 methods
    STDMETHOD(Next)(
        ULONG celt,
        IDebugCodeContext2** rgelt,
        ULONG* pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumDebugCodeContexts2** ppEnum);
    STDMETHOD(GetCount)(ULONG* pcelt);

private:
    // Do not allow use of my default constructor:
    vspCEnumDebugCodeContexts();

private:
    // The enumerated Contexts:
    gtVector<vspCDebugContext*> _enumContexts;


    unsigned int _currentPosition;
};

#endif //__VSPDEBUGCONTEXT_H

