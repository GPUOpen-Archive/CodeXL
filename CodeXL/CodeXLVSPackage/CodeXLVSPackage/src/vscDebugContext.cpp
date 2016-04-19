//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscDebugContext.cpp
///
//==================================================================================

//------------------------------ vspDebugContext.cpp ------------------------------

#include "stdafx.h"

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTApplicationFramework/Include/afAppStringConstants.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>

// Local:
#include <src/vscDebugContext.h>
#include <src/vscDebugEngine.h>
#include <src/vspExpressionEvaluator.h>


// ---------------------------------------------------------------------------
// Name:        vspCDebugContext::vspCDebugContext
// Description: Constructor
// Author:      Uri Shomroni
// Date:        15/9/2010
// ---------------------------------------------------------------------------
vspCDebugContext::vspCDebugContext(const osFilePath& moduleFilePath, gtString& functionName, const osFilePath& sourceFilePath, int sourceLineNumber, osInstructionPointer instructionCounter, osInstructionPointer offset, bool isKernelDebuggingContext, osThreadId threadId, int frameIndex, vspCDebugEngine* pDebugEngine)
    : _moduleFilePath(moduleFilePath), _functionName(functionName), _sourceFilePath(sourceFilePath), _sourceLineNumber(sourceLineNumber), _instructionCounter(instructionCounter), _offset(offset), m_isKernelDebuggingContext(isKernelDebuggingContext), m_threadId(threadId), m_frameIndex(frameIndex), m_pDebugEngine(pDebugEngine)
{
    if (NULL != m_pDebugEngine)
    {
        m_pDebugEngine->AddRef();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugContext::~vspCDebugContext
// Description: Destructor
// Author:      Uri Shomroni
// Date:        15/9/2010
// ---------------------------------------------------------------------------
vspCDebugContext::~vspCDebugContext()
{
    if (NULL != m_pDebugEngine)
    {
        m_pDebugEngine->Release();
        m_pDebugEngine = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugContext::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        15/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugContext::AddRef(void)
{
    return vspCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugContext::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        15/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugContext::Release(void)
{
    return vspCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugContext::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        15/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugContext::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if (ppvObj == NULL)
    {
        retVal = E_POINTER;
    }
    else if (riid == IID_IUnknown)
    {
        // Since both IDebugDocumentContext2 and IDebugCodeContext2 inherit IUnknown, we need to cast through one of them.
        // Note that we have to cast through the same one each time, to be consistent.
        *ppvObj = (IUnknown*)((IDebugDocumentContext2*)this);
        AddRef();
    }
    else if (riid == IID_IDebugDocumentContext2)
    {
        *ppvObj = (IDebugDocumentContext2*)this;
        AddRef();
    }
    else if (riid == IID_IDebugCodeContext2)
    {
        *ppvObj = (IDebugCodeContext2*)this;
        AddRef();
    }
    else if (riid == IID_IDebugMemoryContext2)
    {
        *ppvObj = (IDebugMemoryContext2*)this;
        AddRef();
    }
    else if (riid == IID_IDebugCodeContext100)
    {
        *ppvObj = (IDebugCodeContext100*)this;
        AddRef();
    }
    else if (riid == IID_IDebugExpressionContext2)
    {
        *ppvObj = (IDebugExpressionContext2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IDebugDocumentContext2, IDebugCodeContext2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
// IDebugMemoryContext2 methods
HRESULT vspCDebugContext::GetInfo(CONTEXT_INFO_FIELDS dwFields, CONTEXT_INFO* pInfo)
{
    HRESULT retVal = S_OK;

    if (pInfo != NULL)
    {
        // Clear the struct:
        ::memset(pInfo, 0, sizeof(CONTEXT_INFO));

        if ((dwFields & CIF_MODULEURL) != 0)
        {
            // Module path:
            const gtString& moduleFullPathAsString = _moduleFilePath.asString();

            if (!moduleFullPathAsString.isEmpty())
            {
                pInfo->bstrModuleUrl = SysAllocString(moduleFullPathAsString.asCharArray());
                pInfo->dwFields |= CIF_MODULEURL;
            }
        }

        if ((dwFields & CIF_FUNCTION) != 0)
        {
            // Function Name:
            if (!_functionName.isEmpty())
            {
                pInfo->bstrFunction = SysAllocString(_functionName.asCharArray());
                pInfo->dwFields |= CIF_FUNCTION;
            }
        }

        if ((dwFields & CIF_FUNCTIONOFFSET) != 0)
        {
            // Source offset:
            if (_sourceLineNumber > -1)
            {
                pInfo->posFunctionOffset.dwLine = _sourceLineNumber;
                pInfo->dwFields |= CIF_FUNCTIONOFFSET;
            }
        }

        if ((dwFields & CIF_ADDRESS) != 0)
        {
            // Address:
            gtString addressAsString;
            gdUserApplicationAddressToDisplayString(_instructionCounter, addressAsString);
            pInfo->bstrAddress = SysAllocString(addressAsString.asCharArray());
            pInfo->dwFields |= CIF_ADDRESS;
        }

        if ((dwFields & CIF_ADDRESSOFFSET) != 0)
        {
            // Address offset:
            gtString offsetAsString;
            gdUserApplicationAddressToDisplayString(_offset, offsetAsString);
            pInfo->bstrAddressOffset = SysAllocString(offsetAsString.asCharArray());
            pInfo->dwFields |= CIF_ADDRESSOFFSET;
        }

        if ((dwFields & CIF_ADDRESSABSOLUTE) != 0)
        {
            // Address:
            gtString addressAsString;
            gdUserApplicationAddressToDisplayString(_instructionCounter, addressAsString);
            pInfo->bstrAddressAbsolute = SysAllocString(addressAsString.asCharArray());
            pInfo->dwFields |= CIF_ADDRESSABSOLUTE;
        }
    }
    else // pInfo == NULL
    {
        // Invalid Pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

HRESULT vspCDebugContext::Add(UINT64 dwCount, IDebugMemoryContext2** ppMemCxt)
{
    GT_UNREFERENCED_PARAMETER(dwCount);
    GT_UNREFERENCED_PARAMETER(ppMemCxt);

    return E_NOTIMPL;
}

HRESULT vspCDebugContext::Subtract(UINT64 dwCount, IDebugMemoryContext2** ppMemCxt)
{
    GT_UNREFERENCED_PARAMETER(dwCount);
    GT_UNREFERENCED_PARAMETER(ppMemCxt);

    return E_NOTIMPL;
}

HRESULT vspCDebugContext::Compare(CONTEXT_COMPARE compare, IDebugMemoryContext2** rgpMemoryContextSet, DWORD dwMemoryContextSetLen, DWORD* pdwMemoryContext)
{
    GT_UNREFERENCED_PARAMETER(compare);
    GT_UNREFERENCED_PARAMETER(rgpMemoryContextSet);
    GT_UNREFERENCED_PARAMETER(dwMemoryContextSetLen);
    GT_UNREFERENCED_PARAMETER(pdwMemoryContext);

    return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////////
// IDebugCodeContext100 methods
HRESULT vspCDebugContext::GetProgram(IDebugProgram2** ppProgram)
{
    HRESULT retVal = S_OK;

    if (NULL != ppProgram)
    {
        *ppProgram = (IDebugProgram2*)m_pDebugEngine;
        m_pDebugEngine->AddRef();
    }
    else
    {
        retVal = E_POINTER;
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
// IDebugDocumentContext2 methods
HRESULT vspCDebugContext::GetDocument(IDebugDocument2** ppDocument)
{
    GT_UNREFERENCED_PARAMETER(ppDocument);

    return E_NOTIMPL;
}

HRESULT vspCDebugContext::GetName(BSTR* pbstrName)
{
    return GetName(GN_NAME, pbstrName);
}

HRESULT vspCDebugContext::EnumCodeContexts(IEnumDebugCodeContexts2** ppEnumCodeCxts)
{
    HRESULT retVal = S_OK;

    if (NULL != ppEnumCodeCxts)
    {
        gtVector<vspCDebugContext*> contexts;
        contexts.push_back(this);

        vspCEnumDebugCodeContexts* pOutEnum = new vspCEnumDebugCodeContexts(contexts);
        *ppEnumCodeCxts = pOutEnum;
    }
    else
    {
        retVal = E_POINTER;
    }

    return retVal;
}

HRESULT vspCDebugContext::GetLanguageInfo(BSTR* pbstrLanguage, GUID* pguidLanguage)
{
    GT_UNREFERENCED_PARAMETER(pbstrLanguage);
    GT_UNREFERENCED_PARAMETER(pguidLanguage);

    return E_NOTIMPL;
}

HRESULT vspCDebugContext::Compare(DOCCONTEXT_COMPARE compare, IDebugDocumentContext2** rgpDocContextSet, DWORD dwDocContextSetLen, DWORD* pdwDocContext)
{
    GT_UNREFERENCED_PARAMETER(compare);
    GT_UNREFERENCED_PARAMETER(rgpDocContextSet);
    GT_UNREFERENCED_PARAMETER(dwDocContextSetLen);
    GT_UNREFERENCED_PARAMETER(pdwDocContext);

    return E_NOTIMPL;
}

HRESULT vspCDebugContext::Seek(int ncount, IDebugDocumentContext2** ppDocContext)
{
    GT_UNREFERENCED_PARAMETER(ncount);
    GT_UNREFERENCED_PARAMETER(ppDocContext);

    return E_NOTIMPL;
}

//////////////////////////////////////////////////////////////////////////////
// IDebugCodeContext2 methods
HRESULT vspCDebugContext::GetDocumentContext(IDebugDocumentContext2** ppSrcCxt)
{
    HRESULT retVal = S_OK;

    if (ppSrcCxt != NULL)
    {
        // Return us as a document context and add to our reference count:
        *ppSrcCxt = (IDebugDocumentContext2*)this;
        AddRef();
    }
    else // ppSrcCxt == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

HRESULT vspCDebugContext::GetName(GETNAME_TYPE gnType, BSTR* pbstrFileName)
{
    HRESULT retVal = S_OK;

    if (pbstrFileName != NULL)
    {
        // Which name type was requested?
        switch (gnType)
        {
            case GN_NAME:
            case GN_BASENAME:
            case GN_MONIKERNAME:
            {
                // File name (with extension):
                gtString fileFullName;
                _sourceFilePath.getFileNameAndExtension(fileFullName);

                if (!fileFullName.isEmpty())
                {
                    *pbstrFileName = SysAllocString(fileFullName.asCharArray());
                }
                else // fileFullName.isEmpty()
                {
                    *pbstrFileName = NULL;
                }

            }
            break;

            case GN_FILENAME:
            case GN_URL:
            {
                // Full path:
                gtString fullPath = _sourceFilePath.asString();

                if (!fullPath.isEmpty())
                {
                    *pbstrFileName = SysAllocString(fullPath.asCharArray());
                }
                else // fullPath.isEmpty()
                {
                    *pbstrFileName = NULL;
                }
            }
            break;

            case GN_TITLE:
            case GN_STARTPAGEURL:
            {
                // Unsupported:
                retVal = E_NOTIMPL;
            }
            break;

            default:
            {
                // Unexpected value:
                retVal = E_NOTIMPL;
            }
            break;
        }
    }
    else // pbstrFileName == NULL
    {
    }

    return retVal;
}

HRESULT vspCDebugContext::GetSourceRange(
    TEXT_POSITION* pBegPosition,
    TEXT_POSITION* pEndPosition)
{
    HRESULT retVal = S_OK;

    // If we have source information:
    if (_sourceLineNumber > -1)
    {
        // Return the requested values:
        if (pBegPosition != NULL)
        {
            pBegPosition->dwLine = (DWORD)_sourceLineNumber;
        }

        if (pEndPosition != NULL)
        {
            pEndPosition->dwLine = (DWORD)_sourceLineNumber;
        }
    }
    else // _sourceLineNumber <= -1
    {
        retVal = E_FAIL;
    }

    return retVal;
}

HRESULT vspCDebugContext::GetStatementRange(
    TEXT_POSITION* pBegPosition,
    TEXT_POSITION* pEndPosition)
{
    HRESULT retVal = S_OK;

    // If we have source information:
    if (_sourceLineNumber > -1)
    {
        // Return the requested values:
        if (pBegPosition != NULL)
        {
            pBegPosition->dwLine = (DWORD)_sourceLineNumber;
        }

        if (pEndPosition != NULL)
        {
            pEndPosition->dwLine = (DWORD)_sourceLineNumber;
        }
    }
    else // _sourceLineNumber <= -1
    {
        retVal = E_FAIL;
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
// IDebugExpressionContext2 methods
HRESULT vspCDebugContext::ParseText(LPCOLESTR pszCode, PARSEFLAGS dwFlags, UINT nRadix, IDebugExpression2** ppExpr, BSTR* pbstrError, UINT* pichError)
{
    HRESULT retVal = S_OK;

    if (ppExpr != NULL)
    {
        // Check the parsing type requested:
        switch (dwFlags)
        {
            case PARSE_EXPRESSION:
            {
                // Create the expression and parse (don't evaluate) it:
                bool useHexStrings = (nRadix == 16);
                vspCDebugExpression* pDebugExpression = new vspCDebugExpression(pszCode, m_isKernelDebuggingContext, m_threadId, m_frameIndex, useHexStrings);
                gtString errorText;
                int errorCharIndex = -1;
                bool canEval = pDebugExpression->canEvaluate(errorText, errorCharIndex);

                if (canEval)
                {
                    // Return the expression, as it can be evaluated:
                    *ppExpr = pDebugExpression;

                    if (pbstrError != NULL)
                    {
                        *pbstrError = NULL;
                    }

                    if (pichError != NULL)
                    {
                        *pichError = 0;
                    }
                }
                else // !canEval
                {
                    // We encountered a problem while parsing, return this information:
                    *ppExpr = NULL;

                    if (pbstrError != NULL)
                    {
                        *pbstrError = SysAllocString(errorText.asCharArray());
                    }

                    if (pichError != NULL)
                    {
                        *pichError = (UINT)errorCharIndex;
                    }

                    retVal = E_FAIL;
                    pDebugExpression->Release();
                    pDebugExpression = NULL;
                }
            }
            break;

            case PARSE_FUNCTION_AS_ADDRESS:
            case PARSE_DESIGN_TIME_EXPR_EVAL:
            {
                // We do not currently support this parsing type:
                retVal = E_NOTIMPL;
            }
            break;

            default:
            {
                // Unexpected value!
                retVal = E_NOTIMPL;
            }
            break;
        }
    }
    else // ppExpr == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugCodeContexts::vspCEnumDebugCodeContexts
// Description: Constructor
// Author:      Uri Shomroni
// Date:        23/7/2013
// ---------------------------------------------------------------------------
vspCEnumDebugCodeContexts::vspCEnumDebugCodeContexts(const gtVector<vspCDebugContext*>& currentContexts)
    : _currentPosition(0)
{
    unsigned int numberOfContexts = (unsigned int)currentContexts.size();

    for (unsigned int i = 0; i < numberOfContexts; i++)
    {
        vspCDebugContext* pCurrentContext = currentContexts[i];
        GT_IF_WITH_ASSERT(pCurrentContext != NULL)
        {
            // Add the Context to our vector and retain it.
            _enumContexts.push_back(pCurrentContext);
            pCurrentContext->AddRef();

        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugCodeContexts::~vspCEnumDebugCodeContexts
// Description: Destructor
// Author:      Uri Shomroni
// Date:        23/7/2013
// ---------------------------------------------------------------------------
vspCEnumDebugCodeContexts::~vspCEnumDebugCodeContexts()
{
    // Reduce the Contexts' reference counts:
    unsigned int amountOfContexts = (unsigned int)_enumContexts.size();

    for (unsigned int i = 0; i < amountOfContexts; i++)
    {
        // Sanity check:
        vspCDebugContext* pCurrentContext = _enumContexts[i];
        GT_IF_WITH_ASSERT(pCurrentContext != NULL)
        {
            // Release the current Context and set the vector item to NULL:
            pCurrentContext->Release();
            _enumContexts[i] = NULL;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugCodeContexts::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        23/7/2013
// ---------------------------------------------------------------------------
ULONG vspCEnumDebugCodeContexts::AddRef(void)
{
    return vspCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugCodeContexts::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        23/7/2013
// ---------------------------------------------------------------------------
ULONG vspCEnumDebugCodeContexts::Release(void)
{
    return vspCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugCodeContexts::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        23/7/2013
// ---------------------------------------------------------------------------
HRESULT vspCEnumDebugCodeContexts::QueryInterface(REFIID riid, LPVOID* ppvObj)
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
    else if (riid == IID_IEnumDebugCodeContexts2)
    {
        *ppvObj = (IEnumDebugCodeContexts2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IEnumDebugCodeContexts2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
// IEnumDebugCodeContexts2 methods
HRESULT vspCEnumDebugCodeContexts::Next(ULONG celt, IDebugCodeContext2** rgelt, ULONG* pceltFetched)
{
    HRESULT retVal = S_OK;

    // Will get the amount of Contexts we successfully returned:
    ULONG fetchedItems = 0;

    if (rgelt != NULL)
    {
        unsigned int amountOfContexts = (unsigned int)_enumContexts.size();

        // Try to fill as many items as the caller requested:
        for (ULONG i = 0; i < celt; i++)
        {
            // If we are overflowing
            if (_currentPosition >= amountOfContexts)
            {
                retVal = S_FALSE;
                break;
            }

            // Get the current item:
            vspCDebugContext* pCurrentContext = _enumContexts[_currentPosition];
            GT_IF_WITH_ASSERT(pCurrentContext != NULL)
            {
                // Return it and increment its reference count and the amount of items returned:
                rgelt[fetchedItems] = pCurrentContext;
                pCurrentContext->AddRef();
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
HRESULT vspCEnumDebugCodeContexts::Skip(ULONG celt)
{
    HRESULT retVal = S_OK;

    // Get the amount of Contexts:
    unsigned int amountOfContexts = (unsigned int)_enumContexts.size();

    // Advance the current position:
    _currentPosition += (unsigned int)celt;

    // If we moved past the end, return S_FALSE and reset the position to the end:
    if (_currentPosition > amountOfContexts)
    {
        retVal = S_FALSE;
        _currentPosition = amountOfContexts;
    }

    return retVal;
}
HRESULT vspCEnumDebugCodeContexts::Reset(void)
{
    HRESULT retVal = S_OK;

    // Reset the position to the beginning:
    _currentPosition = 0;

    return retVal;
}
HRESULT vspCEnumDebugCodeContexts::Clone(IEnumDebugCodeContexts2** ppEnum)
{
    HRESULT retVal = S_OK;

    if (ppEnum != NULL)
    {
        // Create a duplicate of this item (note that this will increment the Contexts' reference counts:
        vspCEnumDebugCodeContexts* pClone = new vspCEnumDebugCodeContexts(_enumContexts);

        // Set its position to equal ours:
        pClone->_currentPosition = _currentPosition;

        // Return it:
        *ppEnum = (IEnumDebugCodeContexts2*)pClone;
    }
    else // ppEnum == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vspCEnumDebugCodeContexts::GetCount(ULONG* pcelt)
{
    HRESULT retVal = S_OK;

    if (pcelt != NULL)
    {
        // Return the count:
        *pcelt = (ULONG)_enumContexts.size();
    }
    else
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
