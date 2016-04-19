//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscCallsStack.h
///
//==================================================================================

//------------------------------ vspCallsStack.h ------------------------------

#ifndef __VSPCALLSSTACK_H
#define __VSPCALLSSTACK_H

// Forward declarations:
class osCallStack;
class osCallStackFrame;
class vspCDebugContext;
class vspCDebugEngine;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <src/vspUnknown.h>


// ----------------------------------------------------------------------------------
// Class Name:           vspFunctionArgument
// General Description: Describes a function argument in terms required by the FRAMEINFO
//                      formatting standards.
// Author:               Uri Shomroni
// Creation Date:        14/9/2010
// ----------------------------------------------------------------------------------
struct vspFunctionArgument
{
public:
    vspFunctionArgument()
    {};
    vspFunctionArgument(const vspFunctionArgument& other)
        : _argType(other._argType), _argName(other._argName), _argValue(other._argValue) {};
    ~vspFunctionArgument()
    {};
    vspFunctionArgument& operator=(const vspFunctionArgument& other)
    { _argType = other._argType; _argName = other._argName; _argValue = other._argValue; return *this; };
    void toString(gtString& valueAsString, bool withType, bool withName, bool withValue);

public:
    gtString _argType;
    gtString _argName;
    gtString _argValue;
};

// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugStackFrame : public IDebugStackFrame2
// General Description: Implements IDebugStackFrame2, represents a call stack frame
// Author:               Uri Shomroni
// Creation Date:        13/9/2010
// ----------------------------------------------------------------------------------
class vspCDebugStackFrame : public IDebugStackFrame2, vspCUnknown
{
public:
    vspCDebugStackFrame(const osCallStackFrame& frameData, osThreadId threadId, int frameStackDepth, DWORD requestedFields, bool useHexStrings, vspCDebugEngine& debugEngine);
    virtual ~vspCDebugStackFrame();

    const FRAMEINFO& frameInfo() const {return _frameInfo;};

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugStackFrame2 methods
    STDMETHOD(GetCodeContext)(IDebugCodeContext2** ppCodeCxt);
    STDMETHOD(GetDocumentContext)(IDebugDocumentContext2** ppCxt);
    STDMETHOD(GetName)(BSTR* pbstrName);
    STDMETHOD(GetInfo)(FRAMEINFO_FLAGS dwFieldSpec, UINT nRadix, FRAMEINFO* pFrameInfo);
    STDMETHOD(GetPhysicalStackRange)(UINT64* paddrMin, UINT64* paddrMax);
    STDMETHOD(GetExpressionContext)(IDebugExpressionContext2** ppExprCxt);
    STDMETHOD(GetLanguageInfo)(BSTR* pbstrLanguage, GUID* pguidLanguage);
    STDMETHOD(GetDebugProperty)(IDebugProperty2** ppDebugProp);
    STDMETHOD(EnumProperties)(
        DEBUGPROP_INFO_FLAGS dwFields,
        UINT nRadix,
        REFGUID guidFilter,
        DWORD dwTimeout,
        ULONG* pcelt,
        IEnumDebugPropertyInfo2** ppepi);
    STDMETHOD(GetThread)(IDebugThread2** ppThread);

private:
    void fillMembersFromFrameData(const osCallStackFrame& frameData, DWORD requestedFields, bool useHexStrings, vspCDebugEngine& debugEngine);
    void functionNameFromFrameInfoFields(gtString& funcName, DWORD requestedFields, bool useHexStrings);
    void argumentsAsString(gtString& argsString, bool withTypes, bool withNames, bool withValues, bool formatArguments, bool addParentheses = true);

    // Do not allow use of my default constructor:
    vspCDebugStackFrame();

private:
    FRAMEINFO _frameInfo;
    FRAMEINFO _fullFrameInfo;
    vspCDebugContext* _pFrameDebugContext;
    vspCDebugEngine* m_pDebugEngine;

    osThreadId m_threadId;
    int _frameStackDepth;
    gtString _frameFunctionName;
    gtString _frameFunctionReturnType;
    gtVector<vspFunctionArgument> _frameFunctionArgs;
    osFilePath _frameModulePath;
    osInstructionPointer _frameFunctionStart;
    osInstructionPointer _frameFunctionOffset;
    osFilePath _frameSourceFilePath;
    int _frameSourceLineNumber;
    bool _frameRepresentsKernelSource;
};


// ----------------------------------------------------------------------------------
// Class Name:           vspCEnumDebugFrameInfo : public IEnumDebugFrameInfo2
// General Description: Implements IEnumDebugFrameInfo2, enumerating call stack frames,
//                      effectively representing a calls stack.
// Author:               Uri Shomroni
// Creation Date:        13/9/2010
// ----------------------------------------------------------------------------------
class vspCEnumDebugFrameInfo : public IEnumDebugFrameInfo2, vspCUnknown
{
public:
    vspCEnumDebugFrameInfo(osThreadId threadId, const osCallStack& stackData, DWORD requestedFields, bool useHexStrings, vspCDebugEngine& debugEngine);
    virtual ~vspCEnumDebugFrameInfo();

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IEnumDebugFrameInfo2 methods
    STDMETHOD(Next)(
        ULONG celt,
        FRAMEINFO* rgelt,
        ULONG* pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumDebugFrameInfo2** ppEnum);
    STDMETHOD(GetCount)(ULONG* pcelt);

private:
    // Constructor used in cloning:
    vspCEnumDebugFrameInfo(const gtVector<vspCDebugStackFrame*>& stackData);

    // Do not allow use of my default constructor:
    vspCEnumDebugFrameInfo();

private:
    gtVector<vspCDebugStackFrame*> _enumStackFrames;
    unsigned int _currentPosition;
};

#endif //__VSPCALLSSTACK_H

