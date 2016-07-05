//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspExpressionEvaluator.h
///
//==================================================================================

//------------------------------ vspExpressionEvaluator.h ------------------------------

#ifndef __VSPEXPRESSIONEVALUATOR_H
#define __VSPEXPRESSIONEVALUATOR_H

// Forward declarations:
struct apExpression;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTBaseTools/Include/gtVector.h>

// Local:
#include <src/vspUnknown.h>


// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugExpression : public IDebugExpression2, vspCUnknown
// General Description: Implements IDebugExpression2, representing an expression that
//                      can be evaluated.
// Author:               Uri Shomroni
// Creation Date:        10/11/2010
// ----------------------------------------------------------------------------------
class vspCDebugExpression : public IDebugExpression2, vspCUnknown
{
public:
    vspCDebugExpression(const gtString& expressionCode, bool isKernelDebuggingExpression, osThreadId threadId, int frameIndex, bool useHexStrings);
    virtual ~vspCDebugExpression();

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugExpression2 methods
    STDMETHOD(EvaluateAsync)(EVALFLAGS dwFlags, IDebugEventCallback2* pExprCallback);
    STDMETHOD(Abort)(void);
    STDMETHOD(EvaluateSync)(EVALFLAGS dwFlags, DWORD dwTimeout, IDebugEventCallback2* pExprCallback, IDebugProperty2** ppResult);

    bool canEvaluate(gtString& errorText, int& errorCharIndex);
    const gtString& expressionCode() const {return _expressionCode;};
    bool useHexStrings() const {return _useHexStrings;};
    bool isKernelDebuggingExpression() const { return m_isKernelDebuggingExpression; };
    osThreadId threadId() const { return m_threadId; };
    int frameIndex() const { return m_frameIndex; };

private:
    // Do not allow use of my default constructor:
    vspCDebugExpression();

private:
    gtString _expressionCode;
    bool m_isKernelDebuggingExpression;
    osThreadId m_threadId;
    int m_frameIndex;
    bool _useHexStrings;
};

// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugProperty : public IDebugProperty2, vspCUnknown
// General Description: Implements IDebugProperty2, representing an evaluated value
// Author:               Uri Shomroni
// Creation Date:        10/11/2010
// ----------------------------------------------------------------------------------
class vspCDebugProperty : public IDebugProperty2, vspCUnknown
{
public:
    vspCDebugProperty(const apExpression& expressionValue);
    // vspCDebugProperty(const gtString& nameAsString, const gtString& valueAsString, const gtString& valueAsHexString, const gtString& typeAsString);
    virtual ~vspCDebugProperty();

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugProperty2 methods
    STDMETHOD(GetPropertyInfo)(DEBUGPROP_INFO_FLAGS dwFields, DWORD dwRadix, DWORD dwTimeout, IDebugReference2** rgpArgs, DWORD dwArgCount, DEBUG_PROPERTY_INFO* pPropertyInfo);
    STDMETHOD(SetValueAsString)(LPCOLESTR pszValue, DWORD dwRadix, DWORD dwTimeout);
    STDMETHOD(SetValueAsReference)(IDebugReference2** rgpArgs, DWORD dwArgCount, IDebugReference2* pValue, DWORD dwTimeout);
    STDMETHOD(EnumChildren)(DEBUGPROP_INFO_FLAGS dwFields, DWORD dwRadix, REFGUID guidFilter, DBG_ATTRIB_FLAGS dwAttribFilter, LPCOLESTR pszNameFilter, DWORD dwTimeout, IEnumDebugPropertyInfo2** ppEnum);
    STDMETHOD(GetParent)(IDebugProperty2** ppParent);
    STDMETHOD(GetDerivedMostProperty)(IDebugProperty2** ppDerivedMost);
    STDMETHOD(GetMemoryBytes)(IDebugMemoryBytes2** ppMemoryBytes);
    STDMETHOD(GetMemoryContext)(IDebugMemoryContext2** ppMemory);
    STDMETHOD(GetSize)(DWORD* pdwSize);
    STDMETHOD(GetReference)(IDebugReference2** ppReference);
    STDMETHOD(GetExtendedInfo)(REFGUID guidExtendedInfo, VARIANT* pExtendedInfo);

private:
    // Do not allow use of my default constructor:
    vspCDebugProperty() = delete;

    void addChild(vspCDebugProperty* pChild);

private:
    gtVector<vspCDebugProperty*> _children;
    DEBUG_PROPERTY_INFO _propertyInfo;
    gtString _hexValueString;
};

// ----------------------------------------------------------------------------------
// Class Name:           vspCEnumDebugPropetyInfo : public IEnumDebugPropertyInfo2
// General Description: Implements IEnumDebugPropertyInfo2, enumerating properties.
// Author:               Uri Shomroni
// Creation Date:        21/2/2011
// ----------------------------------------------------------------------------------
class vspCEnumDebugPropertyInfo : public IEnumDebugPropertyInfo2, vspCUnknown
{
public:
    vspCEnumDebugPropertyInfo(const gtVector<vspCDebugProperty*>& properties, bool useHexValues);
    virtual ~vspCEnumDebugPropertyInfo();

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IEnumDebugPropertyInfo2 methods
    STDMETHOD(Next)(
        ULONG celt,
        DEBUG_PROPERTY_INFO* rgelt,
        ULONG* pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumDebugPropertyInfo2** ppEnum);
    STDMETHOD(GetCount)(ULONG* pcelt);

private:
    // Do not allow use of my default constructor:
    vspCEnumDebugPropertyInfo();

private:
    gtVector<vspCDebugProperty*> _enumProperties;
    unsigned int _currentPosition;
    bool _useHexValues;
};


// ----------------------------------------------------------------------------------
// Class Name:           vspExpressionEvaluator
// General Description: Evaluates expressions and sends out events relevant to this evaluation
// Author:               Uri Shomroni
// Creation Date:        10/11/2010
// ----------------------------------------------------------------------------------
class vspExpressionEvaluator
{
public:
    vspExpressionEvaluator();
    ~vspExpressionEvaluator();
    void setDebugEventCallback(IDebugEventCallback2* piDebugEventCallback);

    static vspExpressionEvaluator& instance();

    bool canEvaluate(const vspCDebugExpression& expression, gtString& errorText, int& errorCharIndex);
    vspCDebugProperty* evaluateExpression(vspCDebugExpression* pExpression, EVALFLAGS dwFlags, bool sendEvent);
    bool getCurrentLocals(gtVector<vspCDebugProperty*>& currentLocals, bool kernelDebuggingContext, osThreadId threadId, int stackFrameDepth);

private:
    friend class vspSingletonsDelete;

private:
    static vspExpressionEvaluator* _pMySingleInstance;
    IDebugEventCallback2* _piDebugEventCallback;
};

#endif //__VSPEXPRESSIONEVALUATOR_H

