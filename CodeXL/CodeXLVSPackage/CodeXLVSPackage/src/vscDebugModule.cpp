//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscDebugModule.cpp
///
//==================================================================================

//------------------------------ vspDebugModule.cpp ------------------------------

#include "stdafx.h"

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osTime.h>

// Local:
#include <CodeXLVSPackage/Include/vspStringConstants.h>
#include <src/vscDebugModule.h>

#define VSP_MIF_ALL 0xFFFFFFFF
// ---------------------------------------------------------------------------
// Name:        vspCopyModuleInfo
// Description: Copies src into dst, filtered by requestedFields
// Author:      Uri Shomroni
// Date:        22/9/2010
// ---------------------------------------------------------------------------
void vspCopyModuleInfo(const MODULE_INFO& src, MODULE_INFO& dst, MODULE_INFO_FIELDS requestedFields)
{
    // Release any allocated strings:
    MODULE_INFO_FLAGS dstValidFields = dst.dwValidFields;

    if (((dstValidFields & MIF_NAME) != 0) && (dst.m_bstrName != NULL))
    {
        SysFreeString(dst.m_bstrName);
        dst.m_bstrName = NULL;
    }

    if (((dstValidFields & MIF_URL) != 0) && (dst.m_bstrUrl != NULL))
    {
        SysFreeString(dst.m_bstrUrl);
        dst.m_bstrUrl = NULL;
    }

    if (((dstValidFields & MIF_VERSION) != 0) && (dst.m_bstrVersion != NULL))
    {
        SysFreeString(dst.m_bstrVersion);
        dst.m_bstrVersion = NULL;
    }

    if (((dstValidFields & MIF_DEBUGMESSAGE) != 0) && (dst.m_bstrDebugMessage != NULL))
    {
        SysFreeString(dst.m_bstrDebugMessage);
        dst.m_bstrDebugMessage = NULL;
    }

    if (((dstValidFields & MIF_URLSYMBOLLOCATION) != 0) && (dst.m_bstrUrlSymbolLocation != NULL))
    {
        SysFreeString(dst.m_bstrUrlSymbolLocation);
        dst.m_bstrUrlSymbolLocation = NULL;
    }

    // Clear dst:
    ::memset(&dst, 0, sizeof(MODULE_INFO));

    // Reset the valid fields mask:
    dst.dwValidFields = 0;
    DWORD srcValidFields = src.dwValidFields;

    // Copy each field only if it is relevant:
    if (srcValidFields & requestedFields & MIF_NAME)
    {
        dst.m_bstrName = SysAllocString(src.m_bstrName);
        dst.dwValidFields |= MIF_NAME;
    }

    if (srcValidFields & requestedFields & MIF_URL)
    {
        dst.m_bstrUrl = SysAllocString(src.m_bstrUrl);
        dst.dwValidFields |= MIF_URL;
    }

    if (srcValidFields & requestedFields & MIF_VERSION)
    {
        dst.m_bstrVersion = SysAllocString(src.m_bstrVersion);
        dst.dwValidFields |= MIF_VERSION;
    }

    if (srcValidFields & requestedFields & MIF_DEBUGMESSAGE)
    {
        dst.m_bstrDebugMessage = SysAllocString(src.m_bstrDebugMessage);
        dst.dwValidFields |= MIF_DEBUGMESSAGE;
    }

    if (srcValidFields & requestedFields & MIF_LOADADDRESS)
    {
        dst.m_addrLoadAddress = src.m_addrLoadAddress;
        dst.dwValidFields |= MIF_LOADADDRESS;
    }

    if (srcValidFields & requestedFields & MIF_PREFFEREDADDRESS)
    {
        dst.m_addrPreferredLoadAddress = src.m_addrPreferredLoadAddress;
        dst.dwValidFields |= MIF_PREFFEREDADDRESS;
    }

    if (srcValidFields & requestedFields & MIF_SIZE)
    {
        dst.m_dwSize = src.m_dwSize;
        dst.dwValidFields |= MIF_SIZE;
    }

    if (srcValidFields & requestedFields & MIF_LOADORDER)
    {
        dst.m_dwLoadOrder = src.m_dwLoadOrder;
        dst.dwValidFields |= MIF_LOADORDER;
    }

    if (srcValidFields & requestedFields & MIF_TIMESTAMP)
    {
        dst.m_TimeStamp.dwLowDateTime = src.m_TimeStamp.dwLowDateTime;
        dst.m_TimeStamp.dwHighDateTime = src.m_TimeStamp.dwHighDateTime;
        dst.dwValidFields |= MIF_TIMESTAMP;
    }

    if (srcValidFields & requestedFields & MIF_URLSYMBOLLOCATION)
    {
        dst.m_bstrUrlSymbolLocation = SysAllocString(src.m_bstrUrlSymbolLocation);
        dst.dwValidFields |= MIF_URLSYMBOLLOCATION;
    }

    if (srcValidFields & requestedFields & MIF_FLAGS)
    {
        dst.m_dwModuleFlags = src.m_dwModuleFlags;
        dst.dwValidFields |= MIF_FLAGS;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugModule::vspCDebugModule
// Description: Constructor
// Author:      Uri Shomroni
// Date:        22/9/2010
// ---------------------------------------------------------------------------
vspCDebugModule::vspCDebugModule(const osFilePath& moduleFilePath, osInstructionPointer moduleLoadAddress, bool areDebugSymbolsLoaded)
    : _moduleFilePath(moduleFilePath)
{
    // Initialize _moduleInfo:
    ::memset(&_moduleInfo, 0, sizeof(MODULE_INFO));

    // Fill out as many fields as possible into the module info:
    fillMembersFromModuleData(moduleLoadAddress, areDebugSymbolsLoaded);
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugModule::~vspCDebugModule
// Description: Destructor
// Author:      Uri Shomroni
// Date:        22/9/2010
// ---------------------------------------------------------------------------
vspCDebugModule::~vspCDebugModule()
{
    // Copy an empty MODULE_INFO over our member, to release any string members:
    MODULE_INFO emptyModuleInfo = {0};
    vspCopyModuleInfo(emptyModuleInfo, _moduleInfo, VSP_MIF_ALL);
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugModule::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        22/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugModule::AddRef(void)
{
    return vspCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugModule::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        22/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugModule::Release(void)
{
    return vspCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugModule::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        22/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugModule::QueryInterface(REFIID riid, LPVOID* ppvObj)
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
    else if (riid == IID_IDebugModule2)
    {
        *ppvObj = (IDebugModule2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IDebugModule2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// IDebugModule2 methods
HRESULT vspCDebugModule::GetInfo(MODULE_INFO_FIELDS dwFields, MODULE_INFO* pInfo)
{
    HRESULT retVal = S_OK;

    if (pInfo != NULL)
    {
        // Copy as many fields as we can:
        vspCopyModuleInfo(_moduleInfo, *pInfo, dwFields);
    }
    else // pInfo == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

HRESULT vspCDebugModule::ReloadSymbols_Deprecated(LPCOLESTR pszUrlToSymbols, BSTR* pbstrDebugMessage)
{
    GT_UNREFERENCED_PARAMETER(pszUrlToSymbols);
    GT_UNREFERENCED_PARAMETER(pbstrDebugMessage);

    return E_NOTIMPL;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugModule::fillMembersFromModuleData
// Description: Fills _moudleInfo with information from moduleData
// Author:      Uri Shomroni
// Date:        22/9/2010
// ---------------------------------------------------------------------------
void vspCDebugModule::fillMembersFromModuleData(osInstructionPointer moduleLoadAddress, bool areDebugSymbolsLoaded)
{
    // Get the values:
    gtString moduleFileName;
    _moduleFilePath.getFileNameAndExtension(moduleFileName);

    const gtString& modulePathAsString = _moduleFilePath.asString();

    gtString moduleVersion; // TO_DO: get this value

    gtString debugMessage = areDebugSymbolsLoaded ? VSP_STR_ModuleDebugSymbolsLoaded : VSP_STR_ModuleDebugSymbolsNotLoaded;

    osInstructionPointer modulePreferredAddress = moduleLoadAddress; // TO_DO: verify this

    DWORD moduleSize = 0; // TO_DO: get this value

    DWORD moduleLoadOrder = 0; // TO_DO: get this value

    osTime moduleLoadTime;
    moduleLoadTime.setFromCurrentTime(); // TO_DO: get real value
    gtInt64 moduleLoadTimeAsInt64 = moduleLoadTime.secondsFrom1970();

    gtString moduleSymbolsDir = _moduleFilePath.fileDirectoryAsString(); // TO_DO: verify this

    bool is64BitModule = false;
    bool rc64Bit = osIs64BitModule(_moduleFilePath, is64BitModule);
    bool isSystemModule = false; // TO_DO: get real value
    bool isOptimized = false; // TO_DO: get real value

    if (!moduleFileName.isEmpty())
    {
        _moduleInfo.m_bstrName = SysAllocString(moduleFileName.asCharArray());
        _moduleInfo.dwValidFields |= MIF_NAME;
    }

    if (!modulePathAsString.isEmpty())
    {
        _moduleInfo.m_bstrUrl = SysAllocString(modulePathAsString.asCharArray());
        _moduleInfo.dwValidFields |= MIF_URL;
    }

    if (!moduleVersion.isEmpty())
    {
        _moduleInfo.m_bstrVersion = SysAllocString(moduleVersion.asCharArray());
        _moduleInfo.dwValidFields |= MIF_VERSION;
    }

    if (!debugMessage.isEmpty())
    {
        _moduleInfo.m_bstrDebugMessage = SysAllocString(debugMessage.asCharArray());
        _moduleInfo.dwValidFields |= MIF_DEBUGMESSAGE;
    }

    if (moduleLoadAddress > 0)
    {
        _moduleInfo.m_addrLoadAddress = (UINT64)moduleLoadAddress;
        _moduleInfo.dwValidFields |= MIF_LOADADDRESS;
    }

    if (modulePreferredAddress > 0)
    {
        _moduleInfo.m_addrPreferredLoadAddress = (UINT64)modulePreferredAddress;
        _moduleInfo.dwValidFields |= MIF_PREFFEREDADDRESS;
    }

    if (moduleSize > 0)
    {
        _moduleInfo.m_dwSize = moduleSize;
        _moduleInfo.dwValidFields |= MIF_SIZE;
    }

    if (moduleLoadOrder > 0)
    {
        _moduleInfo.m_dwLoadOrder = moduleLoadOrder;
        _moduleInfo.dwValidFields |= MIF_LOADORDER;
    }

    if (moduleLoadTimeAsInt64 != 0)
    {
        // The windows FILETIME measures nanoseconds from 1601, while time_t which we
        // use measures seconds from 1970.
        // Taken from http://support.microsoft.com/kb/167296:
        gtUInt64 moduleLoadTimeAsNanosecondsFrom1601 = moduleLoadTimeAsInt64 * 10000000LL; // Seconds to 100*nanoseconds.
        moduleLoadTimeAsNanosecondsFrom1601 += 116444736000000000LL; // 1601 to 1970.
        _moduleInfo.m_TimeStamp.dwLowDateTime = (DWORD)moduleLoadTimeAsNanosecondsFrom1601;
        _moduleInfo.m_TimeStamp.dwHighDateTime = (DWORD)(moduleLoadTimeAsNanosecondsFrom1601 >> 32LL);
        _moduleInfo.dwValidFields |= MIF_TIMESTAMP;
    }

    if (!moduleSymbolsDir.isEmpty())
    {
        _moduleInfo.m_bstrUrlSymbolLocation = SysAllocString(moduleSymbolsDir.asCharArray());
        _moduleInfo.dwValidFields |= MIF_URLSYMBOLLOCATION;
    }

    _moduleInfo.m_dwModuleFlags = 0;

    if (isSystemModule)
    {
        _moduleInfo.m_dwModuleFlags |= MODULE_FLAG_SYSTEM;
    }

    if (areDebugSymbolsLoaded)
    {
        _moduleInfo.m_dwModuleFlags |= MODULE_FLAG_SYMBOLS;
    }

    if (rc64Bit && is64BitModule)
    {
        _moduleInfo.m_dwModuleFlags |= MODULE_FLAG_64BIT;
    }

    _moduleInfo.m_dwModuleFlags |= isOptimized ? MODULE_FLAG_OPTIMIZED : MODULE_FLAG_UNOPTIMIZED;
    _moduleInfo.dwValidFields |= MIF_FLAGS;
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugModules::vspCEnumDebugModules
// Description: Constructor
// Author:      Uri Shomroni
// Date:        26/9/2010
// ---------------------------------------------------------------------------
vspCEnumDebugModules::vspCEnumDebugModules(const gtVector<vspCDebugModule*>& loadedModules)
    : _currentPosition(0)
{
    unsigned int numberOfModules = (unsigned int)loadedModules.size();

    for (unsigned int i = 0; i < numberOfModules; i++)
    {
        vspCDebugModule* pCurrentModule = loadedModules[i];
        GT_IF_WITH_ASSERT(pCurrentModule != NULL)
        {
            // Add the module to our vector and retain it.
            _enumModules.push_back(pCurrentModule);
            pCurrentModule->AddRef();

        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugModules::~vspCEnumDebugModules
// Description: Destructor
// Author:      Uri Shomroni
// Date:        26/9/2010
// ---------------------------------------------------------------------------
vspCEnumDebugModules::~vspCEnumDebugModules()
{
    // Reduce the modules' reference counts:
    unsigned int amountOfModules = (unsigned int)_enumModules.size();

    for (unsigned int i = 0; i < amountOfModules; i++)
    {
        // Sanity check:
        vspCDebugModule* pCurrentModule = _enumModules[i];
        GT_IF_WITH_ASSERT(pCurrentModule != NULL)
        {
            // Release the current module and set the vector item to NULL:
            pCurrentModule->Release();
            _enumModules[i] = NULL;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugModules::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        26/9/2010
// ---------------------------------------------------------------------------
ULONG vspCEnumDebugModules::AddRef(void)
{
    return vspCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugModules::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        26/9/2010
// ---------------------------------------------------------------------------
ULONG vspCEnumDebugModules::Release(void)
{
    return vspCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugModules::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        26/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCEnumDebugModules::QueryInterface(REFIID riid, LPVOID* ppvObj)
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
    else if (riid == IID_IEnumDebugModules2)
    {
        *ppvObj = (IEnumDebugModules2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IEnumDebugModules2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
// IEnumDebugModules2 methods
HRESULT vspCEnumDebugModules::Next(ULONG celt, IDebugModule2** rgelt, ULONG* pceltFetched)
{
    HRESULT retVal = S_OK;

    // Will get the amount of modules we successfully returned:
    ULONG fetchedItems = 0;

    if (rgelt != NULL)
    {
        unsigned int amountOfModules = (unsigned int)_enumModules.size();

        // Try to fill as many items as the caller requested:
        for (ULONG i = 0; i < celt; i++)
        {
            // If we are overflowing
            if (_currentPosition >= amountOfModules)
            {
                retVal = S_FALSE;
                break;
            }

            // Get the current item:
            vspCDebugModule* pCurrentModule = _enumModules[_currentPosition];
            GT_IF_WITH_ASSERT(pCurrentModule != NULL)
            {
                // Return it and increment its reference count and the amount of items returned:
                rgelt[fetchedItems] = pCurrentModule;
                pCurrentModule->AddRef();
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
HRESULT vspCEnumDebugModules::Skip(ULONG celt)
{
    HRESULT retVal = S_OK;

    // Get the amount of modules:
    unsigned int amountOfModules = (unsigned int)_enumModules.size();

    // Advance the current position:
    _currentPosition += (unsigned int)celt;

    // If we moved past the end, return S_FALSE and reset the position to the end:
    if (_currentPosition > amountOfModules)
    {
        retVal = S_FALSE;
        _currentPosition = amountOfModules;
    }

    return retVal;
}
HRESULT vspCEnumDebugModules::Reset(void)
{
    HRESULT retVal = S_OK;

    // Reset the position to the beginning:
    _currentPosition = 0;

    return retVal;
}
HRESULT vspCEnumDebugModules::Clone(IEnumDebugModules2** ppEnum)
{
    HRESULT retVal = S_OK;

    if (ppEnum != NULL)
    {
        // Create a duplicate of this item (note that this will increment the modules' reference counts:
        vspCEnumDebugModules* pClone = new vspCEnumDebugModules(_enumModules);

        // Set its position to equal ours:
        pClone->_currentPosition = _currentPosition;

        // Return it:
        *ppEnum = (IEnumDebugModules2*)pClone;
    }
    else // ppEnum == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vspCEnumDebugModules::GetCount(ULONG* pcelt)
{
    HRESULT retVal = S_OK;

    if (pcelt != NULL)
    {
        // Return the count:
        *pcelt = (ULONG)_enumModules.size();
    }
    else
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

