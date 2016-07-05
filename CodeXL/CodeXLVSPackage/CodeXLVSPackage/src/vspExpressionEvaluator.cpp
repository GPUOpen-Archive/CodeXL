//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vspExpressionEvaluator.cpp
///
//==================================================================================

//------------------------------ vspExpressionEvaluator.cpp ------------------------------

#include "stdafx.h"

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// Local:
#include <src/vspExpressionEvaluator.h>
#include <src/vscDebugEvents.h>
#include <CodeXLVSPackage/Include/vspStringConstants.h>

// Static member initializations:
vspExpressionEvaluator* vspExpressionEvaluator::_pMySingleInstance = NULL;

// ---------------------------------------------------------------------------
// Name:        vspCopyDebugPropertyInfo
// Description: Copies src into dst, filtered by requestedFields
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
void vspCopyDebugPropertyInfo(const DEBUG_PROPERTY_INFO& src, DEBUG_PROPERTY_INFO& dst, DEBUGPROP_INFO_FLAGS requestedFields)
{
    // Release the interfaces from the destination struct, if necessary:
    DEBUGPROP_INFO_FLAGS dstValidFields = dst.dwFields;

    if (((dstValidFields & DEBUGPROP_INFO_PROP) != 0) && (dst.pProperty != NULL))
    {
        dst.pProperty->Release();
        dst.pProperty = NULL;
    }

    // Release any allocated strings:
    if (((dstValidFields & DEBUGPROP_INFO_FULLNAME) != 0) && (dst.bstrFullName != NULL))
    {
        SysFreeString(dst.bstrFullName);
        dst.bstrFullName = NULL;
    }

    if (((dstValidFields & DEBUGPROP_INFO_NAME) != 0) && (dst.bstrName != NULL))
    {
        SysFreeString(dst.bstrName);
        dst.bstrName = NULL;
    }

    if (((dstValidFields & DEBUGPROP_INFO_TYPE) != 0) && (dst.bstrType != NULL))
    {
        SysFreeString(dst.bstrType);
        dst.bstrType = NULL;
    }

    if (((dstValidFields & DEBUGPROP_INFO_VALUE) != 0) && (dst.bstrValue != NULL))
    {
        SysFreeString(dst.bstrValue);
        dst.bstrValue = NULL;
    }

    // Clear dst:
    ::memset(&dst, 0, sizeof(DEBUG_PROPERTY_INFO));

    // Reset the valid fields mask:
    dst.dwFields = 0;
    DEBUGPROP_INFO_FLAGS srcValidFields = src.dwFields;

    // Copy each field only if it is relevant:
    if (srcValidFields & requestedFields & DEBUGPROP_INFO_FULLNAME)
    {
        dst.bstrFullName = SysAllocString(src.bstrFullName);
        dst.dwFields |= DEBUGPROP_INFO_FULLNAME;
    }

    if (srcValidFields & requestedFields & DEBUGPROP_INFO_NAME)
    {
        dst.bstrName = SysAllocString(src.bstrName);
        dst.dwFields |= DEBUGPROP_INFO_NAME;
    }

    if (srcValidFields & requestedFields & DEBUGPROP_INFO_TYPE)
    {
        dst.bstrType = SysAllocString(src.bstrType);
        dst.dwFields |= DEBUGPROP_INFO_TYPE;
    }

    if (srcValidFields & requestedFields & DEBUGPROP_INFO_VALUE)
    {
        dst.bstrValue = SysAllocString(src.bstrValue);
        dst.dwFields |= DEBUGPROP_INFO_VALUE;
    }

    if (srcValidFields & requestedFields & DEBUGPROP_INFO_PROP)
    {
        dst.pProperty = src.pProperty;

        if (dst.pProperty != NULL)
        {
            dst.pProperty->AddRef();
        }

        dst.dwFields |= DEBUGPROP_INFO_PROP;
    }

    if (srcValidFields & requestedFields & DEBUGPROP_INFO_ATTRIB)
    {
        dst.dwAttrib = src.dwAttrib;
        dst.dwFields |= DEBUGPROP_INFO_ATTRIB;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExpression::vspCDebugExpression
// Description: Constructor
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
vspCDebugExpression::vspCDebugExpression(const gtString& expressionCode, bool isKernelDebuggingExpression, osThreadId threadId, int frameIndex, bool useHexStrings)
    : _expressionCode(expressionCode), m_isKernelDebuggingExpression(isKernelDebuggingExpression), m_threadId(threadId), m_frameIndex(frameIndex), _useHexStrings(useHexStrings)
{

}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExpression::~vspCDebugExpression
// Description: Destructor
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
vspCDebugExpression::~vspCDebugExpression()
{

}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExpression::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugExpression::AddRef(void)
{
    return vspCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExpression::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugExpression::Release(void)
{
    return vspCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExpression::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugExpression::QueryInterface(REFIID riid, LPVOID* ppvObj)
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
    else if (riid == IID_IDebugExpression2)
    {
        *ppvObj = (IDebugExpression2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IDebugExpression2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

HRESULT vspCDebugExpression::EvaluateAsync(EVALFLAGS dwFlags, IDebugEventCallback2* pExprCallback)
{
    GT_UNREFERENCED_PARAMETER(pExprCallback);

    HRESULT retVal = S_OK;

    // We currently evaluate synchronously even when requested not to:
    vspCDebugProperty* pValueAsProperty = vspExpressionEvaluator::instance().evaluateExpression(this, dwFlags, true);

    // Release the extra reference added to this property:
    if (pValueAsProperty != NULL)
    {
        pValueAsProperty->Release();
    }
    else // pValueAsProperty == NULL
    {
        // We failed and an event was not sent:
        retVal = E_FAIL;
    }

    return retVal;
}
HRESULT vspCDebugExpression::Abort(void)
{
    // We only evaluate synchronously, so we cannot abort:
    return E_NOTIMPL;
}
HRESULT vspCDebugExpression::EvaluateSync(EVALFLAGS dwFlags, DWORD dwTimeout, IDebugEventCallback2* pExprCallback, IDebugProperty2** ppResult)
{
    GT_UNREFERENCED_PARAMETER(dwTimeout);
    GT_UNREFERENCED_PARAMETER(pExprCallback);

    HRESULT retVal = S_OK;

    if (ppResult != NULL)
    {
        vspCDebugProperty* pValueAsProperty = vspExpressionEvaluator::instance().evaluateExpression(this, dwFlags, false);

        *ppResult = pValueAsProperty;

        if (pValueAsProperty == NULL)
        {
            // We failed to evaluate:
            retVal = E_FAIL;
        }
    }
    else // ppResult == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugExpression::canEvaluate
// Description: Returns true if this expression can be evaluated, or false and fills
//              the values if it cannot.
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
bool vspCDebugExpression::canEvaluate(gtString& errorText, int& errorCharIndex)
{
    bool retVal = vspExpressionEvaluator::instance().canEvaluate(*this, errorText, errorCharIndex);

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugProperty::vspCDebugProperty
// Description: Constructor
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
vspCDebugProperty::vspCDebugProperty(const gtString& nameAsString, const gtString& valueAsString, const gtString& valueAsHexString, const gtString& typeAsString) : _hexValueString(valueAsHexString)
{
    // Build the info:
    ::memset(&_propertyInfo, 0, sizeof(DEBUG_PROPERTY_INFO));
    const wchar_t* nameAsCharArray = nameAsString.asCharArray();
    _propertyInfo.dwFields = DEBUGPROP_INFO_FULLNAME | DEBUGPROP_INFO_NAME | DEBUGPROP_INFO_TYPE | DEBUGPROP_INFO_VALUE | DEBUGPROP_INFO_PROP | DEBUGPROP_INFO_ATTRIB;
    _propertyInfo.bstrFullName = SysAllocString(nameAsCharArray);
    _propertyInfo.bstrName = SysAllocString(nameAsCharArray);
    _propertyInfo.bstrType = SysAllocString(typeAsString.asCharArray());
    _propertyInfo.bstrValue = SysAllocString(valueAsString.asCharArray());
    _propertyInfo.pProperty = this;
    // TO_DO: fill with real attributes:
    _propertyInfo.dwAttrib = DBG_ATTRIB_VALUE_READONLY | DBG_ATTRIB_VALUE_RAW_STRING | DBG_ATTRIB_ACCESS_PUBLIC | DBG_ATTRIB_STORAGE_REGISTER | DBG_ATTRIB_TYPE_NONE | DBG_ATTRIB_CHILD_ALL;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugProperty::~vspCDebugProperty
// Description: Destructor
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
vspCDebugProperty::~vspCDebugProperty()
{
    // Clear our pointer from the property info:
    _propertyInfo.pProperty = NULL;

    // Clear the property, releasing the strings and values:
    DEBUG_PROPERTY_INFO emptyProp = {0};
    vspCopyDebugPropertyInfo(emptyProp, _propertyInfo, (DEBUGPROP_INFO_FLAGS)DEBUGPROP_INFO_ALL);

    // Clear the children vector:
    int numberOfChildren = (int)_children.size();

    for (int i = 0; i < numberOfChildren; i++)
    {
        // Sanity check:
        vspCDebugProperty* pCurrentChild = _children[i];
        GT_IF_WITH_ASSERT(pCurrentChild != NULL)
        {
            // Release the child:
            pCurrentChild->Release();
            _children[i] = NULL;
        }
    }
}
// ---------------------------------------------------------------------------
// Name:        vspCDebugProperty::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugProperty::AddRef(void)
{
    return vspCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugProperty::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugProperty::Release(void)
{
    return vspCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugProperty::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugProperty::QueryInterface(REFIID riid, LPVOID* ppvObj)
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
    else if (riid == IID_IDebugProperty2)
    {
        *ppvObj = (IDebugProperty2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IDebugProperty2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

HRESULT vspCDebugProperty::GetPropertyInfo(DEBUGPROP_INFO_FLAGS dwFields, DWORD dwRadix, DWORD dwTimeout, IDebugReference2** rgpArgs, DWORD dwArgCount, DEBUG_PROPERTY_INFO* pPropertyInfo)
{
    GT_UNREFERENCED_PARAMETER(dwTimeout);
    GT_UNREFERENCED_PARAMETER(rgpArgs);
    GT_UNREFERENCED_PARAMETER(dwArgCount);

    HRESULT retVal = S_OK;

    if (pPropertyInfo != NULL)
    {
        // TO_DO: Uri, 24/5/11 - for some reason, VS sometimes calls this function with garbage data in the output struct.
        // This should not happen (and indeed, the struct is zero-ed usually), but we need to compensate for it, else we crash
        // when trying to release the corrupted structure's members. Note that if VS ever passes us a non-garbage struct that is
        // not empty, this would probably cause a memory leak:
        ::memset(pPropertyInfo, 0, sizeof(DEBUG_PROPERTY_INFO));

        // Copy the requested data:
        vspCopyDebugPropertyInfo(_propertyInfo, *pPropertyInfo, dwFields);

        // If we want to use hex strings, use the hex value:
        bool useHexStrings = (dwRadix == 16);

        if (useHexStrings)
        {
            if (pPropertyInfo->bstrValue != NULL)
            {
                // Release the decimal string:
                SysFreeString(pPropertyInfo->bstrValue);
                pPropertyInfo->bstrValue = NULL;

                // Allocate the hex string instead:
                pPropertyInfo->bstrValue = SysAllocString(_hexValueString.asCharArray());
            }
        }
    }
    else // pPropertyInfo == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

HRESULT vspCDebugProperty::SetValueAsString(LPCOLESTR pszValue, DWORD dwRadix, DWORD dwTimeout)
{
    GT_UNREFERENCED_PARAMETER(pszValue);
    GT_UNREFERENCED_PARAMETER(dwRadix);
    GT_UNREFERENCED_PARAMETER(dwTimeout);

    return E_NOTIMPL;
}

HRESULT vspCDebugProperty::SetValueAsReference(IDebugReference2** rgpArgs, DWORD dwArgCount, IDebugReference2* pValue, DWORD dwTimeout)
{
    GT_UNREFERENCED_PARAMETER(rgpArgs);
    GT_UNREFERENCED_PARAMETER(dwArgCount);
    GT_UNREFERENCED_PARAMETER(pValue);
    GT_UNREFERENCED_PARAMETER(dwTimeout);

    return E_NOTIMPL;
}

HRESULT vspCDebugProperty::EnumChildren(DEBUGPROP_INFO_FLAGS dwFields, DWORD dwRadix, REFGUID guidFilter, DBG_ATTRIB_FLAGS dwAttribFilter, LPCOLESTR pszNameFilter, DWORD dwTimeout, IEnumDebugPropertyInfo2** ppEnum)
{
    GT_UNREFERENCED_PARAMETER(dwFields);
    GT_UNREFERENCED_PARAMETER(guidFilter);
    GT_UNREFERENCED_PARAMETER(dwAttribFilter);
    GT_UNREFERENCED_PARAMETER(pszNameFilter);
    GT_UNREFERENCED_PARAMETER(dwTimeout);

    HRESULT retVal = S_OK;

    if (ppEnum != NULL)
    {
        // We do not consider the dwFields value since it will be requested again when the children are called with GetPropertyInfo.
        // TO_DO: consider filtering with the filter attributes.
        *ppEnum = new vspCEnumDebugPropertyInfo(_children, (dwRadix == 16));
    }
    else // ppEnum == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }


    return retVal;
}

HRESULT vspCDebugProperty::GetParent(IDebugProperty2** ppParent)
{
    GT_UNREFERENCED_PARAMETER(ppParent);

    return E_NOTIMPL;
}

HRESULT vspCDebugProperty::GetDerivedMostProperty(IDebugProperty2** ppDerivedMost)
{
    GT_UNREFERENCED_PARAMETER(ppDerivedMost);

    return E_NOTIMPL;
}

HRESULT vspCDebugProperty::GetMemoryBytes(IDebugMemoryBytes2** ppMemoryBytes)
{
    GT_UNREFERENCED_PARAMETER(ppMemoryBytes);

    return E_NOTIMPL;
}

HRESULT vspCDebugProperty::GetMemoryContext(IDebugMemoryContext2** ppMemory)
{
    GT_UNREFERENCED_PARAMETER(ppMemory);

    return E_NOTIMPL;
}

HRESULT vspCDebugProperty::GetSize(DWORD* pdwSize)
{
    GT_UNREFERENCED_PARAMETER(pdwSize);

    return E_NOTIMPL;
}

HRESULT vspCDebugProperty::GetReference(IDebugReference2** ppReference)
{
    GT_UNREFERENCED_PARAMETER(ppReference);

    return E_NOTIMPL;
}

HRESULT vspCDebugProperty::GetExtendedInfo(REFGUID guidExtendedInfo, VARIANT* pExtendedInfo)
{
    GT_UNREFERENCED_PARAMETER(guidExtendedInfo);
    GT_UNREFERENCED_PARAMETER(pExtendedInfo);

    return E_NOTIMPL;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugProperty::addChild
// Description: Adds a child property to the children vector
// Author:      Uri Shomroni
// Date:        23/5/2011
// ---------------------------------------------------------------------------
void vspCDebugProperty::addChild(vspCDebugProperty* pChild)
{
    // Sanity check:
    GT_IF_WITH_ASSERT(pChild != NULL)
    {
        // Add the child property to the vector:
        _children.push_back(pChild);
        pChild->AddRef();

        // Mark this property is expandable:
        if ((_propertyInfo.dwFields & DEBUGPROP_INFO_ATTRIB) != 0)
        {
            _propertyInfo.dwAttrib |= DBG_ATTRIB_OBJ_IS_EXPANDABLE;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugProperty::setName
// Description: Changes the property's name fields
// Author:      Uri Shomroni
// Date:        23/5/2011
// ---------------------------------------------------------------------------
void vspCDebugProperty::setName(const gtString& newName)
{
    // Set the name field if it is relevant:
    if ((_propertyInfo.dwFields & DEBUGPROP_INFO_NAME) != 0)
    {
        SysFreeString(_propertyInfo.bstrName);
        _propertyInfo.bstrName = SysAllocString(newName.asCharArray());
    }

    // Set the full name field if it is relevant:
    if ((_propertyInfo.dwFields & DEBUGPROP_INFO_FULLNAME) != 0)
    {
        SysFreeString(_propertyInfo.bstrFullName);
        _propertyInfo.bstrFullName = SysAllocString(newName.asCharArray());
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugPropertyInfo::vspCEnumDebugPropertyInfo
// Description: Private constructor, used in cloning
// Author:      Uri Shomroni
// Date:        21/2/2011
// ---------------------------------------------------------------------------
vspCEnumDebugPropertyInfo::vspCEnumDebugPropertyInfo(const gtVector<vspCDebugProperty*>& properties, bool useHexValues)
    : _currentPosition(0), _useHexValues(useHexValues)
{
    // Iterate the properties:
    unsigned int numberOfProperties = (unsigned int)properties.size();

    for (unsigned int i = 0; i < numberOfProperties; i++)
    {
        // Sanity check:
        vspCDebugProperty* pCurrentProperty = properties[i];
        GT_IF_WITH_ASSERT(pCurrentProperty != NULL)
        {
            // Add the property to our vector and retain it.
            _enumProperties.push_back(pCurrentProperty);
            pCurrentProperty->AddRef();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugPropertyInfo::~vspCEnumDebugPropertyInfo
// Description: Destructor
// Author:      Uri Shomroni
// Date:        21/2/2011
// ---------------------------------------------------------------------------
vspCEnumDebugPropertyInfo::~vspCEnumDebugPropertyInfo()
{
    // Reduce the properties' reference counts:
    unsigned int amountOfProperties = (unsigned int)_enumProperties.size();

    for (unsigned int i = 0; i < amountOfProperties; i++)
    {
        // Sanity check:
        vspCDebugProperty* pCurrentProperty = _enumProperties[i];
        GT_IF_WITH_ASSERT(pCurrentProperty != NULL)
        {
            // Release the current property and set the vector item to NULL:
            pCurrentProperty->Release();
            _enumProperties[i] = NULL;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugPropertyInfo::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        21/2/2011
// ---------------------------------------------------------------------------
ULONG vspCEnumDebugPropertyInfo::AddRef(void)
{
    return vspCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugPropertyInfo::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        21/2/2011
// ---------------------------------------------------------------------------
ULONG vspCEnumDebugPropertyInfo::Release(void)
{
    return vspCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugPropertyInfo::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        21/2/2011
// ---------------------------------------------------------------------------
HRESULT vspCEnumDebugPropertyInfo::QueryInterface(REFIID riid, LPVOID* ppvObj)
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
    else if (riid == IID_IEnumDebugPropertyInfo2)
    {
        *ppvObj = (IEnumDebugPropertyInfo2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IEnumDebugPropertyInfo2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
// IEnumDebugPropertyInfo2 methods
HRESULT vspCEnumDebugPropertyInfo::Next(ULONG celt, DEBUG_PROPERTY_INFO* rgelt, ULONG* pceltFetched)
{
    HRESULT retVal = S_OK;

    // Will get the amount of properties we successfully returned:
    ULONG fetchedItems = 0;

    if (rgelt != NULL)
    {
        unsigned int amountOfProperties = (unsigned int)_enumProperties.size();

        // Try to fill as many items as the caller requested:
        for (ULONG i = 0; i < celt; i++)
        {
            // If we are overflowing
            if (_currentPosition >= amountOfProperties)
            {
                retVal = S_FALSE;
                break;
            }

            // Get the current item:
            vspCDebugProperty* pCurrentProperty = _enumProperties[_currentPosition];
            GT_IF_WITH_ASSERT(pCurrentProperty != NULL)
            {
                // Get its Property info into the output array and increment the amount of items returned:
                pCurrentProperty->GetPropertyInfo((DEBUGPROP_INFO_FLAGS)DEBUGPROP_INFO_ALL, _useHexValues ? 16 : 10, ULONG_MAX, NULL, 0, &(rgelt[fetchedItems]));
                fetchedItems++;
            }

            // Advance the current position:
            _currentPosition++;
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
HRESULT vspCEnumDebugPropertyInfo::Skip(ULONG celt)
{
    HRESULT retVal = S_OK;

    // Get the amount of properties:
    unsigned int amountOfProperties = (unsigned int)_enumProperties.size();

    // Advance the current position:
    _currentPosition += (unsigned int)celt;

    // If we moved past the end, return S_FALSE and reset the position to the end:
    if (_currentPosition > amountOfProperties)
    {
        retVal = S_FALSE;
        _currentPosition = amountOfProperties;
    }

    return retVal;
}
HRESULT vspCEnumDebugPropertyInfo::Reset(void)
{
    HRESULT retVal = S_OK;

    // Reset the position to the beginning:
    _currentPosition = 0;

    return retVal;
}
HRESULT vspCEnumDebugPropertyInfo::Clone(IEnumDebugPropertyInfo2** ppEnum)
{
    HRESULT retVal = S_OK;

    if (ppEnum != NULL)
    {
        // Create a duplicate of this item (note that this will increment the debug properties' reference counts:
        vspCEnumDebugPropertyInfo* pClone = new vspCEnumDebugPropertyInfo(_enumProperties, _useHexValues);

        // Set its position to equal ours:
        pClone->_currentPosition = _currentPosition;

        // Return it:
        *ppEnum = (IEnumDebugPropertyInfo2*)pClone;
    }
    else // ppEnum == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vspCEnumDebugPropertyInfo::GetCount(ULONG* pcelt)
{
    HRESULT retVal = S_OK;

    if (pcelt != NULL)
    {
        // Return the count:
        *pcelt = (ULONG)_enumProperties.size();
    }
    else
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspExpressionEvaluator::vspExpressionEvaluator
// Description: Constructor
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
vspExpressionEvaluator::vspExpressionEvaluator()
    : _piDebugEventCallback(NULL)
{

}

// ---------------------------------------------------------------------------
// Name:        vspExpressionEvaluator::~vspExpressionEvaluator
// Description: Destructor
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
vspExpressionEvaluator::~vspExpressionEvaluator()
{
    // Release the callback interface:
    if (_piDebugEventCallback != NULL)
    {
        _piDebugEventCallback->Release();
        _piDebugEventCallback = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspExpressionEvaluator::setDebugEventCallback
// Description: Sets the debug event callback interface.
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
void vspExpressionEvaluator::setDebugEventCallback(IDebugEventCallback2* piDebugEventCallback)
{
    // Release any previous interface we were holding:
    if (_piDebugEventCallback != NULL)
    {
        _piDebugEventCallback->Release();
        _piDebugEventCallback = NULL;
    }

    // Set the member:
    GT_IF_WITH_ASSERT(piDebugEventCallback != NULL)
    {
        _piDebugEventCallback = piDebugEventCallback;
        _piDebugEventCallback->AddRef();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspExpressionEvaluator::instance
// Description: Returns the single instance of this class. Creates it on
//              the first call to this function.
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
vspExpressionEvaluator& vspExpressionEvaluator::instance()
{
    if (_pMySingleInstance == NULL)
    {
        _pMySingleInstance = new vspExpressionEvaluator;
    }

    return *_pMySingleInstance;
}

// ---------------------------------------------------------------------------
// Name:        vspExpressionEvaluator::canEvaluate
// Description: Attempts to parse the expression given. If the value cannot be
//              parsed, returns false and fills the reason.
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
bool vspExpressionEvaluator::canEvaluate(const vspCDebugExpression& expression, gtString& errorText, int& errorCharIndex)
{
    bool retVal = false;

    const gtString& exprCode = expression.expressionCode();

    if (!exprCode.isEmpty())
    {
        if (gaIsInKernelDebugging())
        {
            retVal = true;

            // We don't need to really parse the pseudo variable we use for refreshing the values:
            if (exprCode != VSP_STR_ForceVariableRefreshPseudoVariable)
            {
                // Check if the expression is valid:
                retVal = gaCanGetKernelVariableValue(exprCode);
            }
        }
        else if (gaCanGetHostVariables())
        {
            retVal = true;
        }
    }

    // If the expression isn't valid:
    if (!retVal)
    {
        // Return an error:
        errorText = L"Expression cannot be evaluated";
        errorCharIndex = 0;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspExpressionEvaluator::evaluateExpression
// Description: Evaluates an expression. If sendEvent is true, also sends a
//              IDebugExpressionEvaluationCompleteEvent2 event.
// Author:      Uri Shomroni
// Date:        10/11/2010
// ---------------------------------------------------------------------------
vspCDebugProperty* vspExpressionEvaluator::evaluateExpression(vspCDebugExpression* pExpression, EVALFLAGS dwFlags, bool sendEvent)
{
    vspCDebugProperty* retVal = NULL;

    // Check we have an expression. If we are required to send an event but cannot, don't even try evaluating:
    if (pExpression != NULL && ((_piDebugEventCallback != NULL) || (!sendEvent)))
    {
        const gtString& exprCode = pExpression->expressionCode();
        bool isKernelDebuggingExpression = pExpression->isKernelDebuggingExpression();
        osThreadId threadId = pExpression->threadId();
        int frameIndex = pExpression->frameIndex();

        apExpression variableValue;
        bool rcVal = true;

        // We don't need to really evaluate the pseudo variable we use for refreshing the values:
        bool isPseudoVariable = (exprCode == VSP_STR_ForceVariableRefreshPseudoVariable);

        // Attempt to evaluate the expression:
        if (!isPseudoVariable)
        {
            if (isKernelDebuggingExpression)
            {
                int currentWorkItemCoordinate[3] = { -1, -1, -1 };
                gaGetKernelDebuggingCurrentWorkItemCoordinate(0, currentWorkItemCoordinate[0]);
                gaGetKernelDebuggingCurrentWorkItemCoordinate(1, currentWorkItemCoordinate[1]);
                gaGetKernelDebuggingCurrentWorkItemCoordinate(2, currentWorkItemCoordinate[2]);

                // TO_DO: handle children more effectively:
                rcVal = gaGetKernelDebuggingExpressionValue(exprCode, currentWorkItemCoordinate, 1, variableValue);
            }
            else if (gaCanGetHostVariables())
            {
                rcVal = gaGetThreadExpressionValue(threadId, frameIndex, exprCode, 3, variableValue);
            }
        }

        // If we succeeded:
        if (rcVal)
        {
            // Create the property:
            retVal = new vspCDebugProperty(exprCode, variableValue.m_value, variableValue.m_valueHex, variableValue.m_type);

            // If this is a real variable:
            if (!isPseudoVariable)
            {
                const gtVector<apExpression*>& children = variableValue.children();

                // Add all the children:
                for (const apExpression* pChild : children)
                {
                    // Sanity check:
                    GT_IF_WITH_ASSERT(nullptr != pChild)
                    {
                        // Sanity check:
                        const gtString& currentMemberName = pChild->m_name;
                        GT_IF_WITH_ASSERT(!currentMemberName.isEmpty())
                        {
                            // Get the full name for evaluation:
                            gtString currentMemberFullName = exprCode;
                            currentMemberFullName.append('.').append(currentMemberName);

                            // Evaluate the child expression:
                            vspCDebugExpression* pMemberAsExpression = new vspCDebugExpression(currentMemberFullName, isKernelDebuggingExpression, threadId, frameIndex, false);
                            vspCDebugProperty* pMemberAsProperty = evaluateExpression(pMemberAsExpression, dwFlags, false);
                            pMemberAsExpression->Release();
                            GT_IF_WITH_ASSERT(pMemberAsProperty != NULL)
                            {
                                // Set the member's name to be just the partial name:
                                pMemberAsProperty->setName(currentMemberName);

                                // Add it as a child:
                                retVal->addChild(pMemberAsProperty);

                                // Release the extra reference count:
                                pMemberAsProperty->Release();
                            }
                        }
                    }
                }
            }
        }

        // If we succeeded and we were asked to, send an event:
        if (sendEvent && (retVal != NULL))
        {
            // Send the expression evaluation completed event:
            vspCDebugExpressionEvaluationCompleteEvent* pDebugExpressionEvaluationCompleteEvent = new vspCDebugExpressionEvaluationCompleteEvent(pExpression, retVal);
            pDebugExpressionEvaluationCompleteEvent->send(_piDebugEventCallback, NULL, NULL, NULL);
            pDebugExpressionEvaluationCompleteEvent->Release();
        }
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspExpressionEvaluator::getCurrentLocals
// Description: Gets the locals in the current debugging context and evaluates them
// Return Val:  bool - Success / failure.
// Author:      Uri Shomroni
// Date:        22/2/2011
// ---------------------------------------------------------------------------
bool vspExpressionEvaluator::getCurrentLocals(gtVector<vspCDebugProperty*>& currentLocals, bool kernelDebuggingContext, osThreadId threadId, int stackFrameDepth)
{
    bool retVal = false;
    currentLocals.clear();

    // If we are in kernel debugging, get the variables in the kernel:
    gtVector<apExpression> localNames;
    bool rcNm = false;
    bool isInKernelDebugging = gaIsInKernelDebugging() && kernelDebuggingContext;

    if (isInKernelDebugging)
    {
        // TO_DO: Handle locals more effectively
        rcNm = gaGetKernelDebuggingAvailableVariables(0, localNames, false, stackFrameDepth, true);
    }
    else if (gaCanGetHostVariables())
    {
        rcNm = gaGetThreadLocals(threadId, stackFrameDepth, 0, localNames, true);
    }

    if (rcNm)
    {
        retVal = true;

        // Get each variable as an expression and evaluate it:
        int numberOfLocals = (int)localNames.size();

        for (int i = 0; i < numberOfLocals; i++)
        {
            // Make sure this variable exists:
            const gtString& currentName = localNames[i].m_name;

            if (!currentName.isEmpty())
            {
                // Create the variable as an expression for evaluation:
                vspCDebugExpression* pLocalAsExpression = new vspCDebugExpression(currentName, isInKernelDebugging, threadId, stackFrameDepth, false);

                // Evaluate it:
                vspCDebugProperty* pLocalAsProperty = evaluateExpression(pLocalAsExpression, 0, false);
                GT_IF_WITH_ASSERT(pLocalAsProperty != NULL)
                {
                    // Add the property to the output vector:
                    currentLocals.push_back(pLocalAsProperty);
                }

                // Release the expression, as we no longer need it:
                pLocalAsExpression->Release();
            }
        }
    }

    return retVal;
}

