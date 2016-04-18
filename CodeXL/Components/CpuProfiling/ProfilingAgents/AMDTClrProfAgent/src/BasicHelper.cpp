//==================================================================================
// Copyright (c) 2013-2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file BasicHelper.cpp
///
//==================================================================================

#include "BasicHelper.h"
#include <AMDTOSWrappers/Include/osOSDefinitions.h>


/***************************************************************************************
********************                                               ********************
********************            Synchronize Implementation         ********************
********************                                               ********************
***************************************************************************************/

/* public */
ScopedLock::ScopedLock(CRITICAL_SECTION& criticalSection) :
    m_CriticalSection(criticalSection)
{
    EnterCriticalSection(&m_CriticalSection);
}

/* public */
ScopedLock::~ScopedLock()
{
    LeaveCriticalSection(&m_CriticalSection);
}


/***************************************************************************************
********************                                               ********************
********************      BASIC HELPER Function Implementation     ********************
********************                                               ********************
***************************************************************************************/

/* static public */
DWORD BasicHelper::QueryRegKeyInfo(HKEY regKey, RegKeyInfo* pRegKeyInfo)
{
    DWORD retCode;
    wchar_t  achClass[OS_MAX_PATH] = L"";   // buffer for class name
    DWORD    cchClassName = OS_MAX_PATH;    // size of class string

    retCode = RegQueryInfoKey(regKey,     // key handle
                              achClass,               // buffer for class name
                              &cchClassName,          // size of class string
                              NULL,                   // reserved
                              &pRegKeyInfo->cSubKeys,             // number of subkeys
                              &pRegKeyInfo->cbMaxSubKey,          // longest subkey size
                              &pRegKeyInfo->cchMaxClass,          // longest class string
                              &pRegKeyInfo->cValues,                // number of values for this key
                              &pRegKeyInfo->cchMaxValue,            // longest value name
                              &pRegKeyInfo->cbMaxValueData,         // longest value data
                              &pRegKeyInfo->cbSecurityDescriptor,   // security descriptor
                              &pRegKeyInfo->ftLastWriteTime);       // last write time

    return retCode;
}

/* static public */
int BasicHelper::ElementTypeToString(CorElementType elementType, WCHAR* buffer, size_t buflen)
{
    int ret = 0; // success


    switch (elementType)
    {
        case ELEMENT_TYPE_END:
            wcsncpy(buffer, L"ELEMENT_TYPE_END", buflen);
            break;

        case ELEMENT_TYPE_VOID:
            wcsncpy(buffer, L"ELEMENT_TYPE_VOID", buflen);
            break;

        case ELEMENT_TYPE_BOOLEAN:
            wcsncpy(buffer, L"ELEMENT_TYPE_BOOLEAN", buflen);
            break;

        case ELEMENT_TYPE_CHAR:
            wcsncpy(buffer, L"ELEMENT_TYPE_CHAR", buflen);
            break;

        case ELEMENT_TYPE_I1:
            wcsncpy(buffer, L"ELEMENT_TYPE_I1", buflen);
            break;

        case ELEMENT_TYPE_U1:
            wcsncpy(buffer, L"ELEMENT_TYPE_U1", buflen);
            break;

        case ELEMENT_TYPE_I2:
            wcsncpy(buffer, L"ELEMENT_TYPE_I2", buflen);
            break;

        case ELEMENT_TYPE_U2:
            wcsncpy(buffer, L"ELEMENT_TYPE_U2", buflen);
            break;

        case ELEMENT_TYPE_I4:
            wcsncpy(buffer, L"ELEMENT_TYPE_I4", buflen);
            break;

        case ELEMENT_TYPE_U4:
            wcsncpy(buffer, L"ELEMENT_TYPE_U4", buflen);
            break;

        case ELEMENT_TYPE_I8:
            wcsncpy(buffer, L"ELEMENT_TYPE_I8", buflen);
            break;

        case ELEMENT_TYPE_U8:
            wcsncpy(buffer, L"ELEMENT_TYPE_U8", buflen);
            break;

        case ELEMENT_TYPE_R4:
            wcsncpy(buffer, L"ELEMENT_TYPE_R4", buflen);
            break;

        case ELEMENT_TYPE_R8:
            wcsncpy(buffer, L"ELEMENT_TYPE_R8", buflen);
            break;

        case ELEMENT_TYPE_STRING:
            wcsncpy(buffer, L"ELEMENT_TYPE_STRING", buflen);
            break;

        case ELEMENT_TYPE_PTR:
            wcsncpy(buffer, L"ELEMENT_TYPE_PTR", buflen);
            break;

        case ELEMENT_TYPE_BYREF:
            wcsncpy(buffer, L"ELEMENT_TYPE_BYREF", buflen);
            break;

        case ELEMENT_TYPE_VALUETYPE:
            wcsncpy(buffer, L"ELEMENT_TYPE_VALUETYPE", buflen);
            break;

        case ELEMENT_TYPE_CLASS:
            wcsncpy(buffer, L"ELEMENT_TYPE_CLASS", buflen);
            break;

        case ELEMENT_TYPE_ARRAY:
            wcsncpy(buffer, L"ELEMENT_TYPE_ARRAY", buflen);
            break;

        case ELEMENT_TYPE_TYPEDBYREF:
            wcsncpy(buffer, L"ELEMENT_TYPE_TYPEDBYREF", buflen);
            break;

        case ELEMENT_TYPE_I:
            wcsncpy(buffer, L"ELEMENT_TYPE_I", buflen);
            break;

        case ELEMENT_TYPE_U:
            wcsncpy(buffer, L"ELEMENT_TYPE_U", buflen);
            break;

        case ELEMENT_TYPE_FNPTR:
            wcsncpy(buffer, L"ELEMENT_TYPE_FNPTR", buflen);
            break;

        case ELEMENT_TYPE_OBJECT:
            wcsncpy(buffer, L"ELEMENT_TYPE_OBJECT", buflen);
            break;

        case ELEMENT_TYPE_SZARRAY:
            wcsncpy(buffer, L"ELEMENT_TYPE_SZARRAY", buflen);
            break;

        case ELEMENT_TYPE_CMOD_REQD:
            wcsncpy(buffer, L"ELEMENT_TYPE_CMOD_REQD", buflen);
            break;

        case ELEMENT_TYPE_CMOD_OPT:
            wcsncpy(buffer, L"ELEMENT_TYPE_CMOD_OPT", buflen);
            break;

        case ELEMENT_TYPE_MAX:
            wcsncpy(buffer, L"ELEMENT_TYPE_MAX", buflen);
            break;

        case ELEMENT_TYPE_MODIFIER:
            wcsncpy(buffer, L"ELEMENT_TYPE_MODIFIER", buflen);
            break;

        case ELEMENT_TYPE_SENTINEL:
            wcsncpy(buffer, L"ELEMENT_TYPE_SENTINEL", buflen);
            break;

        case ELEMENT_TYPE_PINNED:
            wcsncpy(buffer, L"ELEMENT_TYPE_PINNED", buflen);
            break;

        default:
            ret = -1;
            wcsncpy(buffer, L"<UNKNOWN>", buflen);
            break;
    }

    buffer[buflen - 1] = L'\0';

    return ret;

}

/* static public */
PCCOR_SIGNATURE BasicHelper::ParseElementType(IMetaDataImport* pMDImport,
                                              PCCOR_SIGNATURE signature,
                                              char* buffer)
{
    switch (*signature++)
    {
        case ELEMENT_TYPE_VOID:
            strcat(buffer, "void");
            break;


        case ELEMENT_TYPE_BOOLEAN:
            strcat(buffer, "bool");
            break;


        case ELEMENT_TYPE_CHAR:
            strcat(buffer, "wchar");
            break;


        case ELEMENT_TYPE_I1:
            strcat(buffer, "int8");
            break;


        case ELEMENT_TYPE_U1:
            strcat(buffer, "unsigned int8");
            break;


        case ELEMENT_TYPE_I2:
            strcat(buffer, "int16");
            break;


        case ELEMENT_TYPE_U2:
            strcat(buffer, "unsigned int16");
            break;


        case ELEMENT_TYPE_I4:
            strcat(buffer, "int32");
            break;


        case ELEMENT_TYPE_U4:
            strcat(buffer, "unsigned int32");
            break;


        case ELEMENT_TYPE_I8:
            strcat(buffer, "int64");
            break;


        case ELEMENT_TYPE_U8:
            strcat(buffer, "unsigned int64");
            break;


        case ELEMENT_TYPE_R4:
            strcat(buffer, "float32");
            break;


        case ELEMENT_TYPE_R8:
            strcat(buffer, "float64");
            break;


        case ELEMENT_TYPE_U:
            strcat(buffer, "unsigned int");
            break;


        case ELEMENT_TYPE_I:
            strcat(buffer, "int");
            break;


        case ELEMENT_TYPE_OBJECT:
            strcat(buffer, "Object");
            break;


        case ELEMENT_TYPE_STRING:
            strcat(buffer, "String");
            break;


        case ELEMENT_TYPE_TYPEDBYREF:
            strcat(buffer, "refany");
            break;

        case ELEMENT_TYPE_CLASS:
        case ELEMENT_TYPE_VALUETYPE:
        case ELEMENT_TYPE_CMOD_REQD:
        case ELEMENT_TYPE_CMOD_OPT:
        {
            mdToken token;
            char classname[MAX_LENGTH];


            classname[0] = '\0';
            signature += CorSigUncompressToken(signature, &token);

            if (TypeFromToken(token) != mdtTypeRef)
            {
                HRESULT hr;
                WCHAR zName[MAX_LENGTH];


                hr = pMDImport->GetTypeDefProps(token,
                                                zName,
                                                MAX_LENGTH,
                                                NULL,
                                                NULL,
                                                NULL);

                if (SUCCEEDED(hr))
                {
                    wcstombs(classname, zName, MAX_LENGTH);
                }
            }

            strcat(buffer, classname);
        }
        break;


        case ELEMENT_TYPE_SZARRAY:
            signature = BasicHelper::ParseElementType(pMDImport, signature, buffer);
            strcat(buffer, "[]");
            break;


        case ELEMENT_TYPE_ARRAY:
        {
            ULONG rank;


            signature = BasicHelper::ParseElementType(pMDImport, signature, buffer);
            rank = CorSigUncompressData(signature);

            if (rank == 0)
            {
                strcat(buffer, "[?]");
            }

            else
            {
                ULONG* lower;
                ULONG* sizes;
                ULONG numsizes;
                ULONG arraysize = (sizeof(ULONG) * 2 * rank);


                lower = (ULONG*)_alloca(arraysize);
                memset(lower, 0, arraysize);
                sizes = &lower[rank];

                numsizes = CorSigUncompressData(signature);

                if (numsizes <= rank)
                {
                    ULONG numlower;


                    for (ULONG i = 0; i < numsizes; i++)
                    {
                        sizes[i] = CorSigUncompressData(signature);
                    }


                    numlower = CorSigUncompressData(signature);

                    if (numlower <= rank)
                    {
                        for (unsigned int i = 0; i < numlower; i++)
                        {
                            lower[i] = CorSigUncompressData(signature);
                        }


                        strcat(buffer, "[");

                        for (unsigned int i = 0; i < rank; i++)
                        {
                            if ((sizes[i] != 0) && (lower[i] != 0))
                            {
                                if (lower[i] == 0)
                                {
                                    sprintf(buffer, "%d", sizes[i]);
                                }

                                else
                                {
                                    sprintf(buffer, "%d", lower[i]);
                                    strcat(buffer, "...");

                                    if (sizes[i] != 0)
                                    {
                                        sprintf(buffer, "%d", (lower[i] + sizes[i] + 1));
                                    }
                                }
                            }

                            if (i < (rank - 1))
                            {
                                strcat(buffer, ",");
                            }
                        }

                        strcat(buffer, "]");
                    }
                }
            }
        }
        break;


        case ELEMENT_TYPE_PINNED:
            signature = BasicHelper::ParseElementType(pMDImport, signature, buffer);
            strcat(buffer, "pinned");
            break;


        case ELEMENT_TYPE_PTR:
            signature = BasicHelper::ParseElementType(pMDImport, signature, buffer);
            strcat(buffer, "*");
            break;


        case ELEMENT_TYPE_BYREF:
            signature = BasicHelper::ParseElementType(pMDImport, signature, buffer);
            strcat(buffer, "&");
            break;


        default:
        case ELEMENT_TYPE_END:
        case ELEMENT_TYPE_SENTINEL:
            strcat(buffer, "<UNKNOWN>");
            break;

    } // switch


    return signature;

}

/* static public */
HRESULT BasicHelper::GetClassName(IMetaDataImport* pMDImport, mdToken classToken, WCHAR className[])
{
    DWORD dwTypeDefFlags = 0;
    HRESULT hr = S_OK;
    hr = pMDImport->GetTypeDefProps(classToken,
                                    className,
                                    MAX_LENGTH,
                                    NULL,
                                    &dwTypeDefFlags,
                                    NULL);

    if (FAILED(hr))
    {
        return hr;
    }

    if (IsTdNested(dwTypeDefFlags))
    {
        //      printf("%S is a nested class\n", className);
        mdToken enclosingClass = mdTokenNil;
        hr = pMDImport->GetNestedClassProps(classToken, &enclosingClass);

        if (FAILED(hr))
        {
            return hr;
        }

        //      printf("Enclosing class for %S is %d\n", className, enclosingClass);
        hr = GetClassName(pMDImport, enclosingClass, className);

        //      printf("Enclosing class name %S\n", className);
        if (FAILED(hr))
        {
            return hr;
        }

        size_t length = wcslen(className);

        if (length + 2 < MAX_LENGTH)
        {
            className[length++] = '.';
            hr = pMDImport->GetTypeDefProps(classToken,
                                            className + length,
                                            (ULONG)(MAX_LENGTH - length),
                                            NULL,
                                            NULL,
                                            NULL);

            if (FAILED(hr))
            {
                return hr;
            }

            //          printf("%S is a nested class\n", className);
        }
    }

    return hr;
}

/* static public */
HRESULT BasicHelper::GetFunctionProperties(ICorProfilerInfo* pPrfInfo,
                                           FunctionID functionID,
                                           BOOL* isStatic,
                                           ULONG* argCount,
                                           WCHAR* returnTypeStr,
                                           size_t returnTypeStrLen,
                                           WCHAR* functionParameters,
                                           size_t functionParametersLen,
                                           WCHAR* functionName,
                                           size_t functionNameLen)
{
    HRESULT hr = E_FAIL; // assume success


    if (functionID != NULL)
    {
        mdToken token;
        ClassID classID;
        IMetaDataImport* pMDImport = NULL;
        WCHAR funName[MAX_LENGTH] = L"UNKNOWN";



        //
        // Get the classID
        //
        hr = pPrfInfo->GetFunctionInfo(functionID,
                                       &classID,
                                       NULL,
                                       NULL);

        if (SUCCEEDED(hr))
        {
            //
            // Get the MetadataImport interface and the metadata token
            //
            hr = pPrfInfo->GetTokenAndMetaDataFromFunction(functionID,
                                                           IID_IMetaDataImport,
                                                           (IUnknown**)&pMDImport,
                                                           &token);

            if (SUCCEEDED(hr))
            {
                hr = pMDImport->GetMethodProps(token,
                                               NULL,
                                               funName,
                                               MAX_LENGTH,
                                               0,
                                               0,
                                               NULL,
                                               NULL,
                                               NULL,
                                               NULL);

                if (SUCCEEDED(hr))
                {
                    mdTypeDef classToken = NULL;
                    WCHAR className[MAX_LENGTH] = L"UNKNOWN";


                    hr = pPrfInfo->GetClassIDInfo(classID,
                                                  NULL,
                                                  &classToken);

                    if SUCCEEDED(hr)
                    {
                        if (classToken != mdTypeDefNil)
                        {
                            hr = GetClassName(pMDImport, classToken, className);
                        }

                        _snwprintf(functionName, functionNameLen, L"%s::%s", className, funName);
                        functionName[functionNameLen - 1] = L'\0';


                        DWORD methodAttr = 0;
                        PCCOR_SIGNATURE sigBlob = NULL;


                        hr = pMDImport->GetMethodProps((mdMethodDef) token,
                                                       NULL,
                                                       NULL,
                                                       0,
                                                       NULL,
                                                       &methodAttr,
                                                       &sigBlob,
                                                       NULL,
                                                       NULL,
                                                       NULL);

                        if (SUCCEEDED(hr))
                        {
                            ULONG callConv;


                            //
                            // Is the method static ?
                            //
                            (*isStatic) = (BOOL)((methodAttr & mdStatic) != 0);

                            //
                            // Make sure we have a method signature.
                            //
                            char buffer[2 * MAX_LENGTH];


                            sigBlob += CorSigUncompressData(sigBlob, &callConv);

                            if (callConv != IMAGE_CEE_CS_CALLCONV_FIELD)
                            {
                                static WCHAR* callConvNames[8] =
                                {
                                    L"",
                                    L"unmanaged cdecl ",
                                    L"unmanaged stdcall ",
                                    L"unmanaged thiscall ",
                                    L"unmanaged fastcall ",
                                    L"vararg ",
                                    L"<error> "
                                    L"<error> "
                                };
                                buffer[0] = '\0';

                                if ((callConv & 7) != 0)
                                {
                                    sprintf(buffer, "%ws ", callConvNames[callConv & 7]);
                                }

                                //
                                // Grab the argument count
                                //
                                sigBlob += CorSigUncompressData(sigBlob, argCount);

                                //
                                // Get the return type
                                //
                                sigBlob = ParseElementType(pMDImport, sigBlob, buffer);

                                //
                                // if the return typ returned back empty, write void
                                //
                                if (buffer[0] == '\0')
                                {
                                    sprintf(buffer, "void");
                                }

                                _snwprintf(returnTypeStr, returnTypeStrLen, L"%S", buffer);
                                returnTypeStr[returnTypeStrLen - 1] = L'\0';

                                //
                                // Get the parameters
                                //
                                for (ULONG i = 0;
                                     (SUCCEEDED(hr) && (sigBlob != NULL) && (i < (*argCount)));
                                     i++)
                                {
                                    buffer[0] = '\0';

                                    sigBlob = ParseElementType(pMDImport, sigBlob, buffer);

                                    if (i == 0)
                                    {
                                        _snwprintf(functionParameters, functionParametersLen, L"%S", buffer);
                                        functionParameters[functionParametersLen - 1] = L'\0';
                                    }

                                    else if (sigBlob != NULL)
                                    {
                                        _snwprintf(functionParameters, functionParametersLen, L"%s+%S", functionParameters, buffer);
                                        functionParameters[functionParametersLen - 1] = L'\0';
                                    }

                                    else
                                    {
                                        hr = E_FAIL;
                                    }
                                }
                            }
                            else
                            {
                                //
                                // Get the return type
                                //
                                buffer[0] = '\0';
                                sigBlob = ParseElementType(pMDImport, sigBlob, buffer);
                                _snwprintf(returnTypeStr, returnTypeStrLen, L"%s %S", returnTypeStr, buffer);
                                returnTypeStr[returnTypeStrLen - 1] = L'\0';
                            }
                        }
                    }
                }

                pMDImport->Release();
            }
        }
    }
    //
    // This corresponds to an unmanaged frame
    //
    else
    {
        //
        // Set up return parameters
        //
        hr = S_OK;
        *argCount = 0;
        *isStatic = FALSE;
        returnTypeStr[0] = NULL;
        functionParameters[0] = NULL;
        wcsncpy(functionName, L"UNMANAGED FRAME", functionNameLen);
        functionName[functionNameLen - 1] = L'\0';
    }


    return hr;

}

/* static public */
ULONG BasicHelper::GetElementType(PCCOR_SIGNATURE pSignature, CorElementType* pType, BOOL bDeepParse)
{
    ULONG index = 0;
    mdToken typeRef;
    ULONG elementType;
    ULONG tempType;


    // picking apart primitive types is easy;
    // the ones below require a bit more processing
    index += CorSigUncompressData(&pSignature[index], &elementType);

    switch (elementType)
    {
        // SENTINEL, PINNED and BYREF are not types, just modifiers
        case ELEMENT_TYPE_SENTINEL:
        case ELEMENT_TYPE_BYREF:
        case ELEMENT_TYPE_PINNED:
            //          DEBUG_OUT( ("**** PROCESSING SENTINEL/PINNED/BYREF ****") )
            index += GetElementType(&pSignature[index], (CorElementType*)&elementType, bDeepParse);
            break;


        case ELEMENT_TYPE_PTR:
        case ELEMENT_TYPE_SZARRAY:

            //          DEBUG_OUT( ("**** PROCESSING PTR/SZARRAY ****") )
            if (bDeepParse)
            {
                index += GetElementType(&pSignature[index], (CorElementType*)&elementType);
            }
            else
            {
                index += GetElementType(&pSignature[index], (CorElementType*)&tempType);
            }

            break;


        case ELEMENT_TYPE_CLASS:
        case ELEMENT_TYPE_VALUETYPE:
            //          DEBUG_OUT( ("**** PROCESSING CLASS/OBJECT/VALUECLASS ****") )
            index += CorSigUncompressToken(&pSignature[index], &typeRef);
            break;


        case ELEMENT_TYPE_CMOD_OPT:
        case ELEMENT_TYPE_CMOD_REQD:
            //          DEBUG_OUT( ("**** PROCESSING CMOD_OPT/CMOD_REQD ****") )
            index += CorSigUncompressToken(&pSignature[index], &typeRef);

            if (bDeepParse)
            {
                index += GetElementType(&pSignature[index], (CorElementType*)&elementType);
            }
            else
            {
                index += GetElementType(&pSignature[index], (CorElementType*)&tempType);
            }

            break;


        case ELEMENT_TYPE_ARRAY:

            //          DEBUG_OUT( ("**** PROCESSING ARRAY ****") )
            if (bDeepParse)
            {
                index += ProcessArray(&pSignature[index], (CorElementType*)&elementType);
            }
            else
            {
                index += ProcessArray(&pSignature[index], (CorElementType*)&tempType);
            }

            break;


        case ELEMENT_TYPE_FNPTR:
            //          DEBUG_OUT( ("**** PROCESSING FNPTR ****") )

            // !!! this will throw exception !!!
            //index += ProcessMethodDefRef( &pSignature[index], (CorElementType *)&tempType );

            break;

    } // switch

    *pType = (CorElementType)elementType;

    return index;

}

/* static public */
ULONG BasicHelper::ProcessArray(PCCOR_SIGNATURE pSignature, CorElementType* pType)
{
    ULONG index = 0;
    ULONG rank;


    index += GetElementType(&pSignature[index], pType);
    index += CorSigUncompressData(&pSignature[index], &rank);

    if (rank > 0)
    {
        UINT i;
        ULONG sizes;
        ULONG lowers;

        index += CorSigUncompressData(&pSignature[index], &sizes);

        for (i = 0; i < sizes; i++)
        {
            ULONG dimension;


            index += CorSigUncompressData(&pSignature[index], &dimension);
        } // for


        index += CorSigUncompressData(&pSignature[index], &lowers);

        for (i = 0; i < lowers; i++)
        {
            int lowerBound;

            index += CorSigUncompressSignedInt(&pSignature[index], &lowerBound);
        } // for
    }

    return index;
}
