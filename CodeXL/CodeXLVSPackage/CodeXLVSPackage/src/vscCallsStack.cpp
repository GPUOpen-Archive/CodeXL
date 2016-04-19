//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscCallsStack.cpp
///
//==================================================================================

//------------------------------ vspCallsStack.cpp ------------------------------

#include "stdafx.h"

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTOSWrappers/Include/osCallStack.h>
#include <AMDTOSWrappers/Include/osDebugLog.h>
#include <AMDTAPIClasses/Include/apCLKernel.h>
#include <AMDTApplicationFramework/Include/afAidFunctions.h>
#include <AMDTGpuDebuggingComponents/Include/gdAidFunctions.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// Local:
#include <CodeXLVSPackage/Include/vspStringConstants.h>
#include <src/vscCallsStack.h>
#include <src/vscDebugContext.h>
#include <src/vscDebugEngine.h>
#include <src/vscDebugModule.h>
#include <src/vspExpressionEvaluator.h>
#include <src/vspSourceCodeViewer.h>
#include <src/Guids.h>

#define VSP_FIF_ALL 0xFFFFFFFF
#define VSP_FIF_FUNCNAME_ALL (FIF_FUNCNAME | FIF_FUNCNAME_FORMAT | FIF_FUNCNAME_RETURNTYPE | FIF_FUNCNAME_ARGS | FIF_FUNCNAME_LANGUAGE | FIF_FUNCNAME_MODULE | FIF_FUNCNAME_LINES | FIF_FUNCNAME_OFFSET | FIF_FUNCNAME_ARGS_TYPES | FIF_FUNCNAME_ARGS_NAMES | FIF_FUNCNAME_ARGS_VALUES)
#define VSP_FIF_ARGS_ALL (FIF_ARGS | FIF_ARGS_TYPES | FIF_ARGS_NAMES | FIF_ARGS_VALUES /*| FIF_ARGS_NOFORMAT | FIF_ARGS_NO_FUNC_EVAL | FIF_ARGS_NO_TOSTRING*/)
// ---------------------------------------------------------------------------
// Name:        vspCopyFrameInfo
// Description: Copies src into dst, filtered by requestedFields
// Author:      Uri Shomroni
// Date:        13/9/2010
// ---------------------------------------------------------------------------
void vspCopyFrameInfo(const FRAMEINFO& src, FRAMEINFO& dst, FRAMEINFO_FLAGS requestedFields)
{
    // TO_DO: Uri, 13/9/10 - the header file in the VS2010 SDK and the MSDN documentation do not
    // match on this struct: FIF_DEBUG_MODULEP is called FIF_MODULEP and there is another field added,
    // m_dwFlags (FIF_FLAGS) vs. m_fAnnotatedFrame (FIF_ANNOTATEDFRAME).
    // We need to check which one is correct, though it seems the flags version is newer, as one of the flags is FIFV_ANNOTATEDFRAME.

    // Release the interfaces from the destination struct, if necessary:
    FRAMEINFO_FLAGS dstValidFields = dst.m_dwValidFields;

    if (((dstValidFields & FIF_FRAME) != 0) && (dst.m_pFrame != NULL))
    {
        dst.m_pFrame->Release();
        dst.m_pFrame = NULL;
    }

    if (((dstValidFields & FIF_DEBUG_MODULEP) != 0) && (dst.m_pModule != NULL))
    {
        dst.m_pModule->Release();
        dst.m_pModule = NULL;
    }

    // Release any allocated strings:
    if (((dstValidFields & FIF_ARGS) != 0) && (dst.m_bstrArgs != NULL))
    {
        SysFreeString(dst.m_bstrArgs);
        dst.m_bstrArgs = NULL;
    }

    if (((dstValidFields & FIF_FUNCNAME) != 0) && (dst.m_bstrFuncName != NULL))
    {
        SysFreeString(dst.m_bstrFuncName);
        dst.m_bstrFuncName = NULL;
    }

    if (((dstValidFields & FIF_LANGUAGE) != 0) && (dst.m_bstrLanguage != NULL))
    {
        SysFreeString(dst.m_bstrLanguage);
        dst.m_bstrLanguage = NULL;
    }

    if (((dstValidFields & FIF_MODULE) != 0) && (dst.m_bstrModule != NULL))
    {
        SysFreeString(dst.m_bstrModule);
        dst.m_bstrModule = NULL;
    }

    if (((dstValidFields & FIF_RETURNTYPE) != 0) && (dst.m_bstrReturnType != NULL))
    {
        SysFreeString(dst.m_bstrReturnType);
        dst.m_bstrReturnType = NULL;
    }

    // Clear dst:
    ::memset(&dst, 0, sizeof(FRAMEINFO));

    // Reset the valid fields mask:
    dst.m_dwValidFields = 0;
    FRAMEINFO_FLAGS srcValidFields = src.m_dwValidFields;

    // Copy each field only if it is relevant:
    if (srcValidFields & requestedFields & VSP_FIF_FUNCNAME_ALL)
    {
        dst.m_bstrFuncName = SysAllocString(src.m_bstrFuncName);
        dst.m_dwValidFields |= (srcValidFields & requestedFields & VSP_FIF_FUNCNAME_ALL);
    }

    if (srcValidFields & requestedFields & FIF_RETURNTYPE)
    {
        dst.m_bstrReturnType = SysAllocString(src.m_bstrReturnType);
        dst.m_dwValidFields |= FIF_RETURNTYPE;
    }

    if (srcValidFields & requestedFields & VSP_FIF_ARGS_ALL)
    {
        dst.m_bstrArgs = SysAllocString(src.m_bstrArgs);
        dst.m_dwValidFields |= (srcValidFields & requestedFields & VSP_FIF_ARGS_ALL);
    }

    if (srcValidFields & requestedFields & FIF_LANGUAGE)
    {
        dst.m_bstrLanguage = SysAllocString(src.m_bstrLanguage);
        dst.m_dwValidFields |= FIF_LANGUAGE;
    }

    if (srcValidFields & requestedFields & FIF_MODULE)
    {
        dst.m_bstrModule = SysAllocString(src.m_bstrModule);
        dst.m_dwValidFields |= FIF_MODULE;
    }

    if (srcValidFields & requestedFields & FIF_STACKRANGE)
    {
        dst.m_addrMin = src.m_addrMin;
        dst.m_addrMax = src.m_addrMax;
        dst.m_dwValidFields |= FIF_STACKRANGE;
    }

    if (srcValidFields & requestedFields & FIF_FRAME)
    {
        dst.m_pFrame = src.m_pFrame;

        if (dst.m_pFrame != NULL)
        {
            dst.m_pFrame->AddRef();
        }

        dst.m_dwValidFields |= FIF_FRAME;
    }

    if (srcValidFields & requestedFields & FIF_DEBUG_MODULEP)
    {
        dst.m_pModule = src.m_pModule;

        if (dst.m_pModule != NULL)
        {
            dst.m_pModule->AddRef();
        }

        dst.m_dwValidFields |= FIF_DEBUG_MODULEP;
    }

    if (srcValidFields & requestedFields & FIF_DEBUGINFO)
    {
        dst.m_fHasDebugInfo = src.m_fHasDebugInfo;
        dst.m_dwValidFields |= FIF_DEBUGINFO;
    }

    if (srcValidFields & requestedFields & FIF_STALECODE)
    {
        dst.m_fStaleCode = src.m_fStaleCode;
        dst.m_dwValidFields |= FIF_STALECODE;
    }

    if (srcValidFields & requestedFields & FIF_FLAGS)
    {
        dst.m_dwFlags = src.m_dwFlags;
        dst.m_dwValidFields |= FIF_FLAGS;
    }

    /*
    if (srcValidFields & requestedFields & FIF_ANNOTATEDFRAME)
    {
        dst.m_fAnnotatedFrame = src.m_fAnnotatedFrame;
        dst.m_dwValidFields |= FIF_ANNOTATEDFRAME;
    }
    */
}

// ---------------------------------------------------------------------------
// Name:        vspFunctionArgument::toString
// Description: Formats an argument with the supplied and requested parts:
//              type, name and value; to the following values:
// TNV
// YYY      int num=14
// YYN      int num
// YNY      int 14
// NYY      num=14
// YNN      int
// NYN      num
// NNY      14
// Author:      Uri Shomroni
// Date:        14/9/2010
// ---------------------------------------------------------------------------
void vspFunctionArgument::toString(gtString& valueAsString, bool withType, bool withName, bool withValue)
{
    valueAsString.makeEmpty();

    // Only show a part of the argument if we have it:
    bool showType = withType && (!_argType.isEmpty());
    bool showName = withName && (!_argName.isEmpty());
    bool showValue = withValue && (!_argValue.isEmpty());

    // Add the parts and separators as needed:
    if (showType)
    {
        valueAsString = _argType;

        if (showName || showValue)
        {
            valueAsString.append(' ');
        }
    }

    if (showName)
    {
        valueAsString.append(_argName);

        if (showValue)
        {
            valueAsString.append('=');
        }
    }

    if (showValue)
    {
        valueAsString.append(_argValue);
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugStackFrame::vspCDebugStackFrame
// Description: Constructor
// Author:      Uri Shomroni
// Date:        14/9/2010
// ---------------------------------------------------------------------------
vspCDebugStackFrame::vspCDebugStackFrame(const osCallStackFrame& frameData, osThreadId threadId, int frameStackDepth, DWORD requestedFields, bool useHexStrings, vspCDebugEngine& debugEngine)
    : _pFrameDebugContext(NULL), m_threadId(threadId), _frameStackDepth(frameStackDepth), _frameFunctionStart((osInstructionPointer)NULL), _frameFunctionOffset((osInstructionPointer)NULL), _frameSourceLineNumber(0), _frameRepresentsKernelSource(false), m_pDebugEngine(&debugEngine)
{
    // Initialize FRAMEINFO members:
    ::memset(&_frameInfo, 0, sizeof(FRAMEINFO));
    ::memset(&_fullFrameInfo, 0, sizeof(FRAMEINFO));

    // Fill out as many fields as possible into the full frame info:
    fillMembersFromFrameData(frameData, requestedFields, useHexStrings, debugEngine);

    // Copy the requested fields into the frame info:
    vspCopyFrameInfo(_fullFrameInfo, _frameInfo, requestedFields);

    // If we copied the frame, we'd increased our own reference count by 1 in the process. Reduce the count back down to 1:
    if ((requestedFields & FIF_FRAME) != 0)
    {
        ULONG refCount = getReferenceCount();
        GT_IF_WITH_ASSERT(refCount > 1)
        {
            GT_ASSERT(refCount == 2);
            Release();
        }
    }

    // If this is a kernel frame, we expect the document it represents to disappear when debugging is over.
    if (_frameRepresentsKernelSource)
    {
        // Mark it as a controlled document so it will be closed:
        vspSourceCodeViewer::instance().addDocumentToControlledVector(_frameSourceFilePath);
    }

    // Retain the debug engine:
    m_pDebugEngine->AddRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugStackFrame::~vspCDebugStackFrame
// Description: Destructor
// Author:      Uri Shomroni
// Date:        14/9/2010
// ---------------------------------------------------------------------------
vspCDebugStackFrame::~vspCDebugStackFrame()
{
    // Clear our pointer from the FRAMEINFO struct:
    _frameInfo.m_pFrame = NULL;
    _fullFrameInfo.m_pFrame = NULL;

    // Copy an empty FRAMEINFO over both our members, to release any string or interface members:
    FRAMEINFO emptyFrameInfo = {0};
    vspCopyFrameInfo(emptyFrameInfo, _frameInfo, VSP_FIF_ALL);
    vspCopyFrameInfo(emptyFrameInfo, _fullFrameInfo, VSP_FIF_ALL);

    // Clear the debug context:
    if (_pFrameDebugContext != NULL)
    {
        _pFrameDebugContext->Release();
        _pFrameDebugContext = NULL;
    }

    // Release the debug engine:
    GT_IF_WITH_ASSERT(NULL != m_pDebugEngine)
    {
        m_pDebugEngine->Release();
        m_pDebugEngine = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugStackFrame::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        14/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugStackFrame::AddRef(void)
{
    return vspCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugStackFrame::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        14/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugStackFrame::Release(void)
{
    return vspCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugStackFrame::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        14/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugStackFrame::QueryInterface(REFIID riid, LPVOID* ppvObj)
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
    else if (riid == IID_IDebugStackFrame2)
    {
        *ppvObj = (IDebugStackFrame2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IDebugStackFrame2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// IDebugStackFrame2 methods
HRESULT vspCDebugStackFrame::GetCodeContext(IDebugCodeContext2** ppCodeCxt)
{
    HRESULT retVal = S_OK;

    if (ppCodeCxt != NULL)
    {
        // If this is the first time it is requested, create the debug context:
        if (_pFrameDebugContext == NULL)
        {
            _pFrameDebugContext = new vspCDebugContext(_frameModulePath, _frameFunctionName, _frameSourceFilePath, _frameSourceLineNumber, _frameFunctionStart + _frameFunctionOffset, _frameFunctionOffset, _frameRepresentsKernelSource, m_threadId, _frameStackDepth, m_pDebugEngine);
        }

        // Cast the debug context to a code context:
        *ppCodeCxt = (IDebugCodeContext2*)_pFrameDebugContext;
        _pFrameDebugContext->AddRef();

    }
    else // ppCodeCxt == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vspCDebugStackFrame::GetDocumentContext(IDebugDocumentContext2** ppCxt)
{
    HRESULT retVal = S_OK;

    if (ppCxt != NULL)
    {
        // If this is the first time it is requested, create the debug context:
        if (_pFrameDebugContext == NULL)
        {
            _pFrameDebugContext = new vspCDebugContext(_frameModulePath, _frameFunctionName, _frameSourceFilePath, _frameSourceLineNumber, _frameFunctionStart + _frameFunctionOffset, _frameFunctionOffset, _frameRepresentsKernelSource, m_threadId, _frameStackDepth, m_pDebugEngine);
        }

        // Cast the debug context to a document context:
        *ppCxt = (IDebugDocumentContext2*)_pFrameDebugContext;
        _pFrameDebugContext->AddRef();
    }
    else // ppCodeCxt == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vspCDebugStackFrame::GetName(BSTR* pbstrName)
{
    GT_UNREFERENCED_PARAMETER(pbstrName);
    HRESULT retVal = E_NOTIMPL;

    return retVal;
}
HRESULT vspCDebugStackFrame::GetInfo(FRAMEINFO_FLAGS dwFieldSpec, UINT nRadix, FRAMEINFO* pFrameInfo)
{
    HRESULT retVal = S_OK;

    if (pFrameInfo != NULL)
    {
        // Copy the requested data:
        vspCopyFrameInfo(_fullFrameInfo, *pFrameInfo, dwFieldSpec);

        bool useHexStrings = (nRadix == 16);

        // If the caller requested a function name, format it as requested:
        if (dwFieldSpec & FIF_FUNCNAME)
        {
            // Format with the new flags:
            gtString newFuncName;
            functionNameFromFrameInfoFields(newFuncName, dwFieldSpec, useHexStrings);

            // Release the old string:
            SysFreeString(pFrameInfo->m_bstrFuncName);

            // Allocate a new one:
            pFrameInfo->m_bstrFuncName = SysAllocString(newFuncName.asCharArray());

            // Update the field spec for this name option:
            pFrameInfo->m_dwValidFields &= (~VSP_FIF_FUNCNAME_ALL);
            pFrameInfo->m_dwValidFields |= (dwFieldSpec & VSP_FIF_FUNCNAME_ALL);
        }

        // If the caller requested function arguments, format them as requested
        if (dwFieldSpec & FIF_ARGS)
        {
            // Format with the new flags:
            gtString newArgs;
            bool withTypes = ((dwFieldSpec & FIF_ARGS_TYPES) != 0);
            bool withNames = ((dwFieldSpec & FIF_ARGS_NAMES) != 0);
            bool withValues = ((dwFieldSpec & FIF_ARGS_VALUES) != 0);
            argumentsAsString(newArgs, withTypes, withNames, withValues, useHexStrings);

            // Release the old string:
            SysFreeString(pFrameInfo->m_bstrArgs);

            // Allocate a new one:
            pFrameInfo->m_bstrArgs = SysAllocString(newArgs.asCharArray());

            // Update the field spec for this arguments option:
            pFrameInfo->m_dwValidFields &= (~VSP_FIF_ARGS_ALL);
            pFrameInfo->m_dwValidFields |= (dwFieldSpec & VSP_FIF_ARGS_ALL);
        }
    }
    else // pFrameInfo == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vspCDebugStackFrame::GetPhysicalStackRange(UINT64* paddrMin, UINT64* paddrMax)
{
    HRESULT retVal = S_OK;

    if ((paddrMin != NULL) || (paddrMax != NULL))
    {
        if ((_fullFrameInfo.m_dwValidFields & FIF_STACKRANGE) != 0)
        {
            // Return the values:
            if (paddrMin != NULL)
            {
                *paddrMin = _fullFrameInfo.m_addrMin;
            }

            if (paddrMax != NULL)
            {
                *paddrMax = _fullFrameInfo.m_addrMax;
            }
        }
        else //(_fullFrameInfo.m_dwValidFields & FIF_STACKRANGE) == 0
        {
            // Unavailable information:
            retVal = E_FAIL;
        }
    }
    else // (paddrMin == NULL) && (paddrMax == NULL)
    {
        // Invalid pointers:
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vspCDebugStackFrame::GetExpressionContext(IDebugExpressionContext2** ppExprCxt)
{
    HRESULT retVal = S_OK;

    if (ppExprCxt != NULL)
    {
        // If this is the first time it is requested, create the debug context:
        if (_pFrameDebugContext == NULL)
        {
            _pFrameDebugContext = new vspCDebugContext(_frameModulePath, _frameFunctionName, _frameSourceFilePath, _frameSourceLineNumber, _frameFunctionStart + _frameFunctionOffset, _frameFunctionOffset, _frameRepresentsKernelSource, m_threadId, _frameStackDepth, m_pDebugEngine);
        }

        // Cast the debug context to a code context:
        *ppExprCxt = (IDebugExpressionContext2*)_pFrameDebugContext;
        _pFrameDebugContext->AddRef();
    }
    else // ppExprCxt == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vspCDebugStackFrame::GetLanguageInfo(BSTR* pbstrLanguage, GUID* pguidLanguage)
{
    HRESULT retVal = S_OK;

    if (_frameRepresentsKernelSource)
    {
        // This is kernel source:
        if (pbstrLanguage != NULL)
        {
            *pbstrLanguage = SysAllocString(L"OpenCL C");
        }

        if (pguidLanguage != NULL)
        {
            *pguidLanguage = CLSID_CodeXLVSPackageEditorDocument;
        }
    }
    else // !_frameRepresentsKernelSource
    {
        // This is C/C++ source:
        if (pbstrLanguage != NULL)
        {
            *pbstrLanguage = SysAllocString(VSP_STR_CallStackC_CppLanguage);
        }

        if (pguidLanguage != NULL)
        {
            *pguidLanguage = guidCPPLang;
        }
    }

    return retVal;
}
HRESULT vspCDebugStackFrame::GetDebugProperty(IDebugProperty2** ppDebugProp)
{
    GT_UNREFERENCED_PARAMETER(ppDebugProp);

    HRESULT retVal = E_NOTIMPL;

    return retVal;
}
HRESULT vspCDebugStackFrame::EnumProperties(DEBUGPROP_INFO_FLAGS dwFields, UINT nRadix, REFGUID guidFilter, DWORD dwTimeout, ULONG* pcelt, IEnumDebugPropertyInfo2** ppepi)
{
    GT_UNREFERENCED_PARAMETER(dwFields);
    GT_UNREFERENCED_PARAMETER(dwTimeout);
    GT_UNREFERENCED_PARAMETER(pcelt);

    HRESULT retVal = S_OK;

    if ((guidFilter == guidFilterLocals) || (guidFilter == guidFilterAllLocals) || (guidFilter == guidFilterArgs) || (guidFilter == guidFilterLocalsPlusArgs) || (guidFilter == guidFilterAllLocalsPlusArgs))
    {
        // We don't currently differentiate Locals, all locals, and arguments:
        gtVector<vspCDebugProperty*> currentLocals;
        vspExpressionEvaluator::instance().getCurrentLocals(currentLocals, _frameRepresentsKernelSource, m_threadId, _frameStackDepth);

        bool useHexValues = (nRadix == 16);
        // Return the enumerator even if it is empty:
        *ppepi = new vspCEnumDebugPropertyInfo(currentLocals, useHexValues);

        // Release the properties' extra reference:
        int numberOfLocals = (int)currentLocals.size();

        for (int i = 0; i < numberOfLocals; i++)
        {
            // Sanity check:
            vspCDebugProperty* pCurrentLocal = currentLocals[i];
            GT_IF_WITH_ASSERT(pCurrentLocal != NULL)
            {
                pCurrentLocal->Release();
            }
            currentLocals[i] = NULL;
        }
    }
    else // guidFilterRegisters, guidFilterThis, guidFilterAutoRegisters
    {
        // We do not implement these fields:
        retVal = E_NOTIMPL;
    }

    return retVal;
}
HRESULT vspCDebugStackFrame::GetThread(IDebugThread2** ppThread)
{
    GT_UNREFERENCED_PARAMETER(ppThread);

    HRESULT retVal = E_NOTIMPL;

    return retVal;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugStackFrame::fillMembersFromFrameData
// Description: Attempts to fill as much data into _fullFrameInfo, _frameFunctionName,
//              _frameFunctionReturnType, _frameFunctionArgs, _frameModulePath,
//              _frameFunctionOffset, _frameSourceFilePath and _frameSourceLineNumber
//              from frameData
// Author:      Uri Shomroni
// Date:        14/9/2010
// ---------------------------------------------------------------------------
void vspCDebugStackFrame::fillMembersFromFrameData(const osCallStackFrame& frameData, DWORD requestedFields, bool useHexStrings, vspCDebugEngine& debugEngine)
{
    // Fill members:
    _frameFunctionName = frameData.functionName();
    // _frameFunctionReturnType = ; // Return type - not currently supported
    bool gotArguments = false;
    // _frameFunctionArgs.push_back(); // Function arguments not currently supported
    _frameModulePath = frameData.moduleFilePath();
    _frameFunctionStart = frameData.functionStartAddress();
    // If we don't have the function frame start, we'd rather show it as 0xfeedface + 0 than 0x00000000 + 0xfeedface:
    osInstructionPointer icAddr = frameData.instructionCounterAddress();

    if (_frameFunctionStart == (osInstructionPointer)NULL)
    {
        _frameFunctionStart = icAddr;
    }

    _frameFunctionOffset = (icAddr > _frameFunctionStart) ? (icAddr - _frameFunctionStart) : 0;
    _frameSourceFilePath = frameData.sourceCodeFilePath();
    _frameSourceLineNumber = frameData.sourceCodeFileLineNumber();
    _frameRepresentsKernelSource = frameData.isKernelSourceCode();

    // The source line number, if we have it, points to the return address, i.e. one line after the function:
    if (_frameSourceLineNumber > 0)
    {
        // Decrement it:
        _frameSourceLineNumber--;
    }
    else if ((_frameSourceLineNumber == -1) && (_frameRepresentsKernelSource))
    {
        // If this is kernel source, but we don't have a line number, show the kernel declaration instead:
        apCLKernel currentlyDebuggedKernelDetails(OA_CL_NULL_HANDLE, 0, OA_CL_NULL_HANDLE, _frameFunctionName);
        bool rcKer = gaGetCurrentlyDebuggedKernelDetails(currentlyDebuggedKernelDetails);
        GT_IF_WITH_ASSERT(rcKer)
        {
            _frameSourceLineNumber = gdSearchForKernelDeclarationInSourceFile(_frameSourceFilePath, currentlyDebuggedKernelDetails.kernelFunctionName());

            // Subtract one as normal:
            _frameSourceLineNumber--;
        }
    }

    // Fill _fullFrameInfo:
    // Function Name:
    if ((!_frameFunctionName.isEmpty()) || (_frameFunctionStart != (osInstructionPointer)NULL) || _frameRepresentsKernelSource)
    {
        // Format the function name according to the FIF_FUNCNAME_* flags:
        gtString funcName;
        functionNameFromFrameInfoFields(funcName, requestedFields, useHexStrings);
        _fullFrameInfo.m_bstrFuncName = SysAllocString(funcName.asCharArray());
        _fullFrameInfo.m_dwValidFields |= FIF_FUNCNAME;
    }

    // Return type:
    if (!_frameFunctionReturnType.isEmpty())
    {
        _fullFrameInfo.m_bstrReturnType = SysAllocString(_frameFunctionReturnType.asCharArray());
        _fullFrameInfo.m_dwValidFields |= FIF_RETURNTYPE;
    }

    // Function arguments:
    if (gotArguments)
    {
        // Format the function name according to the FIF_ARGS_* flags:
        gtString argsString;
        bool withTypes = ((requestedFields & FIF_ARGS_TYPES) != 0);
        bool withNames = ((requestedFields & FIF_ARGS_NAMES) != 0);
        bool withValues = ((requestedFields & FIF_ARGS_VALUES) != 0);
        argumentsAsString(argsString, withTypes, withNames, withValues, true, false);
        _fullFrameInfo.m_bstrArgs = SysAllocString(argsString.asCharArray());
        _fullFrameInfo.m_dwValidFields |= FIF_ARGS | (requestedFields & (FIF_ARGS_TYPES | FIF_ARGS_NAMES | FIF_ARGS_VALUES));
    }

    // Language:
    // TO_DO: support multiple languages
    _fullFrameInfo.m_bstrLanguage = SysAllocString(VSP_STR_CallStackC_CppLanguage);
    _fullFrameInfo.m_dwValidFields |= FIF_LANGUAGE;

    // Module path:
    gtString moduleFilePathAsString = _frameModulePath.asString();

    if (!moduleFilePathAsString.isEmpty())
    {
        _fullFrameInfo.m_bstrModule = SysAllocString(moduleFilePathAsString.asCharArray());
        _fullFrameInfo.m_dwValidFields |= FIF_MODULE;
    }

    /*  // Stack range - not currently supported:
        {
            _fullFrameInfo.m_addrMin = 0x0000000000000000LL;
            _fullFrameInfo.m_addrMax = 0xFFFFFFFFFFFFFFFFLL;
            _fullFrameInfo.m_dwValidFields |= FIF_STACKRANGE;
        }                                                       */

    // Frame interface:
    _fullFrameInfo.m_pFrame = (IDebugStackFrame2*)this;
    _fullFrameInfo.m_dwValidFields |= FIF_FRAME;

    // Module interface:
    vspCDebugModule* pDebugModule = debugEngine.getModule(_frameModulePath);

    if (pDebugModule != NULL)
    {
        _fullFrameInfo.m_pModule = pDebugModule;
        _fullFrameInfo.m_pModule->AddRef();
        _fullFrameInfo.m_dwValidFields |= FIF_DEBUG_MODULEP;
    }

    // Debug information availability:
    bool isLineValid = _frameRepresentsKernelSource ? (_frameSourceLineNumber > -1) : (_frameSourceLineNumber > 0);
    bool debugInfoAvailable = ((!_frameSourceFilePath.asString().isEmpty()) && isLineValid);

    if (frameData.isKernelSourceCode() && (!debugInfoAvailable))
    {
        // For kernel call stack frames, just the file name is enough to
        // consist some debug information:
        debugInfoAvailable = !_frameSourceFilePath.asString().isEmpty();
    }

    _fullFrameInfo.m_fHasDebugInfo = debugInfoAvailable ? TRUE : FALSE;
    _fullFrameInfo.m_dwValidFields |= FIF_DEBUGINFO;

    /*  // Code staleness - not currently supported:
        {
            _fullFrameInfo.m_fStaleCode = isCodeStale ? TRUE : FALSE;
            _fullFrameInfo.m_dwValidFields |= FIF_STALECODE;
        }                                                               */

    /*  // Frame flags (annotated code, non-user code, can intercept exceptions, function evaluation frame) - not currently supported:
        {
            _fullFrameInfo.m_dwFlags;
            _fullFrameInfo.m_dwValidFields |= FIF_FLAGS;
        }*/
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugStackFrame::functionNameFromFrameInfoFields
// Description: Using the FIF_FUNCNAME_* flags in requestedFields, builds funcName
//              as the function name
// Author:      Uri Shomroni
// Date:        14/9/2010
// ---------------------------------------------------------------------------
void vspCDebugStackFrame::functionNameFromFrameInfoFields(gtString& funcName, DWORD requestedFields, bool useHexStrings)
{
    funcName.makeEmpty();

    // Get the flags:
    bool formatFunctionName = (requestedFields & FIF_FUNCNAME_FORMAT) != 0;
    bool addReturnType = (requestedFields & FIF_FUNCNAME_RETURNTYPE) != 0;
    bool addArguments = (requestedFields & FIF_FUNCNAME_ARGS) != 0;
    bool addLanguage = (requestedFields & FIF_FUNCNAME_LANGUAGE) != 0;
    bool addModule = (requestedFields & FIF_FUNCNAME_MODULE) != 0;
    bool addSourceLine = (requestedFields & FIF_FUNCNAME_LINES) != 0;
    bool addOffset = (requestedFields & FIF_FUNCNAME_OFFSET) != 0;
    bool addArgumentTypes = (requestedFields & FIF_FUNCNAME_ARGS_TYPES) != 0;
    bool addArgumentNames = (requestedFields & FIF_FUNCNAME_ARGS_NAMES) != 0;
    bool addArgumentValues = (requestedFields & FIF_FUNCNAME_ARGS_VALUES) != 0;
    bool formatArguments = (requestedFields & FIF_ARGS_NOFORMAT) == 0;

    if (formatFunctionName)
    {
        // Add the function name
        funcName.append(_frameFunctionName);

        if (funcName.isEmpty())
        {
            // If we don't have a name, use the function pointer:
            gdUserApplicationAddressToDisplayString(_frameFunctionStart, funcName);
        }
    }

    if (addReturnType && (!_frameFunctionReturnType.isEmpty()))
    {
        // Add the return type
        if (!funcName.isEmpty())
        {
            funcName.prepend(' ');
        }

        funcName.prepend(_frameFunctionReturnType);
    }

    if (addArguments)
    {
        // Add the arguments:
        gtString argsString;
        argumentsAsString(argsString, addArgumentTypes, addArgumentNames, addArgumentValues, formatArguments);
        funcName.append(argsString);
    }

    if (addLanguage)
    {
        // Add the language:
        funcName.append(' ').append('[').append(_fullFrameInfo.m_bstrLanguage).append(']');
    }

    if (addModule)
    {
        // Add the module (as myModule.dll![funcname(args)...]):
        gtString moduleFileName;
        _frameModulePath.getFileNameAndExtension(moduleFileName);

        if (!moduleFileName.isEmpty())
        {
            funcName.prepend('!').prepend(moduleFileName);
        }
    }

    int displayedLineNumber = _frameSourceLineNumber + 1;

    if ((addSourceLine) && (displayedLineNumber > 0))
    {
        // Add the source location:
        // The Visual Studio standard does not add the source file name here:
        // funcName.append(' ').append(_frameSourceFilePath.asString());
        // Kernel code does not compensate for the instruction counter being on the next line, so we need to do so here:
        funcName.appendFormattedString(VSP_STR_CallStackSourceLineFormat, displayedLineNumber);
    }
    else if ((addOffset) && (_frameFunctionOffset > 0))
    {
        // Add the offset:
        funcName.appendFormattedString(useHexStrings ? VSP_STR_CallStackFunctionOffsetAsHexString : VSP_STR_CallStackFunctionOffsetAsDecString, _frameFunctionOffset);
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugStackFrame::argumentsAsString
// Description: Outputs a string of arguments, with types, names and value as
//              required / available. If formatArguments is true, adds commas
//              between the arguments.
// Author:      Uri Shomroni
// Date:        14/9/2010
// ---------------------------------------------------------------------------
void vspCDebugStackFrame::argumentsAsString(gtString& argsString, bool withTypes, bool withNames, bool withValues, bool formatArguments, bool addParentheses)
{
    argsString.makeEmpty();

    gtString argumentSeparator = ' ';

    if (formatArguments)
    {
        argumentSeparator.prepend(',');
    }

    // Iterate the arguments by order:
    int numberOfArguments = (int)_frameFunctionArgs.size();

    for (int i = 0; i < numberOfArguments; i++)
    {
        // Format the current argument:
        gtString currentArgAsString;
        _frameFunctionArgs[i].toString(currentArgAsString, withTypes, withNames, withValues);

        // Add the separator as needed:
        if (i > 0)
        {
            argsString.append(argumentSeparator);
        }

        // Add the argument:
        argsString.append(currentArgAsString);
    }

    // Add the parantheses:
    if (addParentheses)
    {
        argsString.prepend('(').append(')');
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugFrameInfo::vspCEnumDebugFrameInfo
// Description: Constructor
// Author:      Uri Shomroni
// Date:        14/9/2010
// ---------------------------------------------------------------------------
vspCEnumDebugFrameInfo::vspCEnumDebugFrameInfo(osThreadId threadId, const osCallStack& stackData, DWORD requestedFields, bool useHexStrings, vspCDebugEngine& debugEngine)
    : _currentPosition(0)
{
    int numberOfFrames = stackData.amountOfStackFrames();

    for (int i = 0; i < numberOfFrames; i++)
    {
        const osCallStackFrame* pCurrentFrameData = stackData.stackFrame(i);
        GT_IF_WITH_ASSERT(pCurrentFrameData != NULL)
        {
            vspCDebugStackFrame* pCurrentFrame = new vspCDebugStackFrame(*pCurrentFrameData, threadId, i, requestedFields, useHexStrings, debugEngine);

            // Add the frame to our vector.
            _enumStackFrames.push_back(pCurrentFrame);
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugFrameInfo::vspCEnumDebugFrameInfo
// Description: Private constructor, used in cloning
// Author:      Uri Shomroni
// Date:        14/9/2010
// ---------------------------------------------------------------------------
vspCEnumDebugFrameInfo::vspCEnumDebugFrameInfo(const gtVector<vspCDebugStackFrame*>& stackFrames)
    : _currentPosition(0)
{
    unsigned int numberOfFrames = (unsigned int)stackFrames.size();

    for (unsigned int i = 0; i < numberOfFrames; i++)
    {
        vspCDebugStackFrame* pCurrentFrame = stackFrames[i];
        GT_IF_WITH_ASSERT(pCurrentFrame != NULL)
        {
            // Add the thread to our vector and retain it.
            _enumStackFrames.push_back(pCurrentFrame);
            pCurrentFrame->AddRef();
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugFrameInfo::~vspCEnumDebugFrameInfo
// Description: Destructor
// Author:      Uri Shomroni
// Date:        14/9/2010
// ---------------------------------------------------------------------------
vspCEnumDebugFrameInfo::~vspCEnumDebugFrameInfo()
{
    // Reduce the debug frames' reference counts:
    unsigned int amountOfFrames = (unsigned int)_enumStackFrames.size();

    for (unsigned int i = 0; i < amountOfFrames; i++)
    {
        // Sanity check:
        vspCDebugStackFrame* pCurrentFrame = _enumStackFrames[i];
        GT_IF_WITH_ASSERT(pCurrentFrame != NULL)
        {
            // Release the current frame and set the vector item to NULL:
            pCurrentFrame->Release();
            _enumStackFrames[i] = NULL;
        }
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugFrameInfo::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        14/9/2010
// ---------------------------------------------------------------------------
ULONG vspCEnumDebugFrameInfo::AddRef(void)
{
    return vspCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugFrameInfo::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        14/9/2010
// ---------------------------------------------------------------------------
ULONG vspCEnumDebugFrameInfo::Release(void)
{
    return vspCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugFrameInfo::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        14/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCEnumDebugFrameInfo::QueryInterface(REFIID riid, LPVOID* ppvObj)
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
    else if (riid == IID_IEnumDebugFrameInfo2)
    {
        *ppvObj = (IEnumDebugFrameInfo2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IEnumDebugFrameInfo2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
// IEnumDebugFrameInfo2 methods
HRESULT vspCEnumDebugFrameInfo::Next(ULONG celt, FRAMEINFO* rgelt, ULONG* pceltFetched)
{
    HRESULT retVal = S_OK;

    // Will get the amount of frames we successfully returned:
    ULONG fetchedItems = 0;

    if (rgelt != NULL)
    {
        unsigned int amountOfFrames = (unsigned int)_enumStackFrames.size();

        // Try to fill as many items as the caller requested:
        for (ULONG i = 0; i < celt; i++)
        {
            // If we are overflowing
            if (_currentPosition >= amountOfFrames)
            {
                retVal = S_FALSE;
                break;
            }

            // Get the current item:
            vspCDebugStackFrame* pCurrentFrame = _enumStackFrames[_currentPosition];
            GT_IF_WITH_ASSERT(pCurrentFrame != NULL)
            {
                // Get its frame info, return it and increment the amount of items returned:
                const FRAMEINFO& currentFrameInfo = pCurrentFrame->frameInfo();
                vspCopyFrameInfo(currentFrameInfo, rgelt[fetchedItems], VSP_FIF_ALL);
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
HRESULT vspCEnumDebugFrameInfo::Skip(ULONG celt)
{
    HRESULT retVal = S_OK;

    // Get the amount of frames:
    unsigned int amountOfFrames = (unsigned int)_enumStackFrames.size();

    // Advance the current position:
    _currentPosition += (unsigned int)celt;

    // If we moved past the end, return S_FALSE and reset the position to the end:
    if (_currentPosition > amountOfFrames)
    {
        retVal = S_FALSE;
        _currentPosition = amountOfFrames;
    }

    return retVal;
}
HRESULT vspCEnumDebugFrameInfo::Reset(void)
{
    HRESULT retVal = S_OK;

    // Reset the position to the beginning:
    _currentPosition = 0;

    return retVal;
}
HRESULT vspCEnumDebugFrameInfo::Clone(IEnumDebugFrameInfo2** ppEnum)
{
    HRESULT retVal = S_OK;

    if (ppEnum != NULL)
    {
        // Create a duplicate of this item (note that this will increment the debug frames' reference counts:
        vspCEnumDebugFrameInfo* pClone = new vspCEnumDebugFrameInfo(_enumStackFrames);

        // Set its position to equal ours:
        pClone->_currentPosition = _currentPosition;

        // Return it:
        *ppEnum = (IEnumDebugFrameInfo2*)pClone;
    }
    else // ppEnum == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vspCEnumDebugFrameInfo::GetCount(ULONG* pcelt)
{
    HRESULT retVal = S_OK;

    if (pcelt != NULL)
    {
        // Return the count:
        *pcelt = (ULONG)_enumStackFrames.size();
    }
    else
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

