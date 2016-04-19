//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file BasicHelper.h
///
//==================================================================================

#ifndef _BASICHELPER_H_
#define _BASICHELPER_H_

#include "baseheader.h"


/***************************************************************************************
********************                                               ********************
********************            ScopedLock Declaration             ********************
********************                                               ********************
***************************************************************************************/
class ScopedLock
{
public:
    ScopedLock(CRITICAL_SECTION& criticalSection);
    ~ScopedLock();
    ScopedLock(const ScopedLock&) = delete;
    ScopedLock& operator=(const ScopedLock&) = delete;

private:
    CRITICAL_SECTION& m_CriticalSection;
};


struct RegKeyInfo
{
    DWORD    cSubKeys;                  // number of subkeys
    DWORD    cbMaxSubKey;               // longest subkey size
    DWORD    cchMaxClass;               // longest class string
    DWORD    cValues;                   // number of values for key
    DWORD    cchMaxValue;               // longest value name
    DWORD    cbMaxValueData;            // longest value data
    DWORD    cbSecurityDescriptor;      // size of security descriptor
    FILETIME ftLastWriteTime;           // last write time
};


/***************************************************************************************
********************                                               ********************
********************        Basic Helper Function Declaration      ********************
********************                                               ********************
***************************************************************************************/
class BasicHelper
{
public:
    // Get Registry key info
    static DWORD QueryRegKeyInfo(HKEY regKey, RegKeyInfo* pRegKeyInfo);

    // return a string for a CorElementValue
    static int ElementTypeToString(CorElementType elementType, WCHAR* buffer, size_t buflen);

    // print element type
    static PCCOR_SIGNATURE ParseElementType(IMetaDataImport* pMDImport,
                                            PCCOR_SIGNATURE signature,
                                            char* buffer);

    // process metadata for a function given its functionID
    static HRESULT GetFunctionProperties(ICorProfilerInfo* pPrfInfo,
                                         FunctionID functionID,
                                         BOOL* isStatic,
                                         ULONG* argCount,
                                         WCHAR* returnTypeStr,
                                         size_t returnTypeStrLen,
                                         WCHAR* functionParameters,
                                         size_t functionParametersLen,
                                         WCHAR* functionName,
                                         size_t functionNameLen);

    static HRESULT GetClassName(IMetaDataImport* pMDImport, mdToken classToken, WCHAR className[]);

    // decodes a type from the signature.
    // the type returned will be, depending on the last parameter,
    // either the outermost type, (e.g. ARRAY for an array of I4s)
    // or the innermost (I4 in the example above),
    //
    static ULONG GetElementType(PCCOR_SIGNATURE pSignature,
                                CorElementType* pType,
                                BOOL bDeepParse = FALSE);

    // helper function for decoding arrays
    //
    static ULONG ProcessArray(PCCOR_SIGNATURE pSignature, CorElementType* pType);

    // helper function for decoding FNPTRs (NOT IMPL)
    static ULONG ProcessMethodDefRef(PCCOR_SIGNATURE pSignature, CorElementType* pType);
};

#endif // _BASICHELPER_H_
