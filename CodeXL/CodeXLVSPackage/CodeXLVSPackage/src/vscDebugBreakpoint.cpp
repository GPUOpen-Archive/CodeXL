//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscDebugBreakpoint.cpp
///
//==================================================================================

//------------------------------ vspDebugBreakpoint.cpp ------------------------------

#include "stdafx.h"

// Infra:
#include <AMDTBaseTools/Include/gtAssert.h>
#include <AMDTAPIClasses/Include/apGenericBreakpoint.h>
#include <AMDTAPIClasses/Include/apKernelFunctionNameBreakpoint.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionBreakPoint.h>
#include <AMDTApiFunctions/Include/gaGRApiFunctions.h>

// Local:
#include <src/vscBreakpointsManager.h>
#include <src/vscDebugBreakpoint.h>
#include <src/vscDebugEngine.h>
#include <src/vscDebugContext.h>

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpoint::vspCDebugBreakpoint
// Description: Constructor
// Author:      Uri Shomroni
// Date:        29/9/2010
// ---------------------------------------------------------------------------
vspCDebugBreakpoint::vspCDebugBreakpoint(vspCDebugEngine* pDebugEngine, IDebugBreakpointRequest2* piBreakpointRequest, int hitCount)
    : _pDebugEngine(pDebugEngine), _breakpointStatus(VSP_BREAKPOINT_PENDING), _breakpointType(VSP_BREAKPOINT_UNBOUND), _openCLProgramHandle(OA_CL_NULL_HANDLE),
      _openCLProgramLineNumber(-1), _openCLProgramBoundLineNumber(-1), m_hostLineNumber(-1), _monitoredFunctionId(apMonitoredFunctionsAmount), _genericBreakpointType(AP_BREAK_TYPE_UNKNOWN),
      _piBreakpointRequest(piBreakpointRequest), _errorType(BPET_NONE), _wasDeleted(false), _isEnabled(true), _bindCount(0), _hitCount(hitCount), _requestedLineNumber(-1)
{
    // Retain the debug engine:
    GT_IF_WITH_ASSERT(_pDebugEngine != NULL)
    {
        _pDebugEngine->AddRef();
    }

    // Retain the request, so we'll be able to return it as needed:
    GT_IF_WITH_ASSERT(_piBreakpointRequest != NULL)
    {
        _piBreakpointRequest->AddRef();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpoint::~vspCDebugBreakpoint
// Description: Destructor
// Author:      Uri Shomroni
// Date:        29/9/2010
// ---------------------------------------------------------------------------
vspCDebugBreakpoint::~vspCDebugBreakpoint()
{
    // Release the engine:
    if (_pDebugEngine != NULL)
    {
        _pDebugEngine->Release();
        _pDebugEngine = NULL;
    }

    // Release the request:
    if (_piBreakpointRequest != NULL)
    {
        _piBreakpointRequest->Release();
        _piBreakpointRequest = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpoint::bind
// Description: Called to set the breakpoint to bound status
// Author:      Uri Shomroni
// Date:        29/9/2010
// ---------------------------------------------------------------------------
void vspCDebugBreakpoint::bind()
{
    // Make sure this breakpoint was not bound or set to an error before, unless it's an error breakpoint we can rebind:
    bool statusIsError = ((VSP_BREAKPOINT_ERROR == _breakpointStatus) || (VSP_BREAKPOINT_QUERY_ERROR == _breakpointStatus));
    bool typeIsError = ((VSP_BREAKPOINT_OPENCL_C_CODE_ERROR == _breakpointType) || (VSP_BREAKPOINT_C_CPP_CODE_ERROR == _breakpointType));

    GT_ASSERT((VSP_BREAKPOINT_PENDING == _breakpointStatus) ||
              (statusIsError && typeIsError) ||
              ((VSP_BREAKPOINT_OPENCL_C_CODE == _breakpointType) && (-1 < _openCLProgramBoundLineNumber)));

    // Mark the breakpoint as bound:
    _breakpointStatus = VSP_BREAKPOINT_BOUND;
    _bindCount++;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpoint::setError
// Description: Called to set the breakpoint to error status
// Author:      Uri Shomroni
// Date:        29/9/2010
// ---------------------------------------------------------------------------
void vspCDebugBreakpoint::setError(BP_ERROR_TYPE errorType, const gtString& errorMessage, bool wasBindAttempted)
{
    // Make sure this breakpoint was not bound or set to an error before, unless it's an error breakpoint we can attempt to rebind:
    bool typeIsError = ((VSP_BREAKPOINT_OPENCL_C_CODE_ERROR == _breakpointType) || (VSP_BREAKPOINT_C_CPP_CODE_ERROR == _breakpointType));
    GT_ASSERT((_breakpointStatus == VSP_BREAKPOINT_PENDING) || (_breakpointStatus == VSP_BREAKPOINT_QUERY_ERROR) ||
              ((_breakpointStatus == VSP_BREAKPOINT_ERROR) && typeIsError));

    if (wasBindAttempted)
    {
        // Mark the breakpoint as an error:
        _breakpointStatus = VSP_BREAKPOINT_ERROR;
    }
    else // !wasBindAttempted
    {
        // This breakpoint was marked as an error as a result of a call to canBind():
        _breakpointStatus = VSP_BREAKPOINT_QUERY_ERROR;
    }

    // Set the error type and message:
    _errorType = errorType;
    _errorMessage = errorMessage;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpoint::setMonitoredFunctionBreakpoint
// Description: Called by the breakpoints manager to let this breakpoint know its type
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
void vspCDebugBreakpoint::setMonitoredFunctionBreakpoint(apMonitoredFunctionId funcId)
{
    // Make sure we didn't set the type twice:
    GT_ASSERT(_breakpointType == VSP_BREAKPOINT_UNBOUND);

    _breakpointType = VSP_BREAKPOINT_MONITORED_FUNCTION;
    _monitoredFunctionId = funcId;
    GT_ASSERT(_monitoredFunctionId < apMonitoredFunctionsAmount);
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpoint::setGenericBreakpointType
// Description: Called by the breakpoints manager to let this breakpoint know its type
// Arguments:   apGenericBreakpointType
// Return Val:  void
// Author:      Sigal Algranaty
// Date:        10/7/2011
// ---------------------------------------------------------------------------
void vspCDebugBreakpoint::setGenericBreakpointType(apGenericBreakpointType breakpointType)
{
    // Make sure we didn't set the type twice:
    GT_ASSERT(_breakpointType == VSP_BREAKPOINT_UNBOUND);

    _breakpointType = VSP_BREAKPOINT_GENERIC;
    _genericBreakpointType = breakpointType;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpoint::setKernelFunctionNameBreakpoint
// Description: Called by the breakpoints manager to let this breakpoint know its type
// Author:      Uri Shomroni
// Date:        27/2/2011
// ---------------------------------------------------------------------------
void vspCDebugBreakpoint::setKernelFunctionNameBreakpoint(const gtString& kernelFuncName)
{
    // Make sure we didn't set the type twice:
    GT_ASSERT(_breakpointType == VSP_BREAKPOINT_UNBOUND);

    _breakpointType = VSP_BREAKPOINT_KERNEL_FUNCTION_NAME;
    _kernelFunctionName = kernelFuncName;
    GT_ASSERT(!_kernelFunctionName.isEmpty());
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpoint::setNonMonitoredFunctionBreakpoint
// Description: Called by the breakpoints manager to let this breakpoint know its type
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
void vspCDebugBreakpoint::setNonMonitoredFunctionBreakpoint()
{
    // Make sure we didn't set the type twice:
    GT_ASSERT(_breakpointType == VSP_BREAKPOINT_UNBOUND);

    _breakpointType = VSP_BREAKPOINT_NON_MONITORED_FUNCTION;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpoint::setCCppCodeBreakpoint
// Description: Called by the breakpoints manager to let this breakpoint know its type
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
void vspCDebugBreakpoint::setCCppCodeBreakpoint(const osFilePath& hostSrcPath, int hostLineNum)
{
    // Make sure we didn't set the type twice, or that we are rebinding an error breakpoint, or that the breakpoint was updated:
    GT_ASSERT((_breakpointType == VSP_BREAKPOINT_UNBOUND) || (_breakpointType == VSP_BREAKPOINT_C_CPP_CODE_ERROR) ||
              ((_breakpointType == VSP_BREAKPOINT_C_CPP_CODE) && (m_hostLineNumber > -1)));

    _breakpointType = VSP_BREAKPOINT_C_CPP_CODE;
    m_hostSourcePath = hostSrcPath;
    m_hostLineNumber = hostLineNum;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpoint::setCCppCodeErrorBreakpoint
// Description: Called by the breakpoints manager to let this breakpoint know its type
// Author:      Uri Shomroni
// Date:        11/1/2016
// ---------------------------------------------------------------------------
void vspCDebugBreakpoint::setCCppCodeErrorBreakpoint()
{
    // Make sure we didn't set the type twice:
    GT_ASSERT((VSP_BREAKPOINT_UNBOUND == _breakpointType) || (VSP_BREAKPOINT_C_CPP_CODE == _breakpointType));

    _breakpointType = VSP_BREAKPOINT_C_CPP_CODE_ERROR;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpoint::setOpenCLCCodeBreakpoint
// Description: Called by the breakpoints manager to let this breakpoint know its type
// Author:      Uri Shomroni
// Date:        9/11/2010
// ---------------------------------------------------------------------------
void vspCDebugBreakpoint::setOpenCLCCodeBreakpoint(oaCLProgramHandle openCLProgramHandle, int openCLProgramLineNumber)
{
    // Make sure we didn't set the type twice, or that we are rebinding an error breakpoint, or that the breakpoint was updated:
    GT_ASSERT((_breakpointType == VSP_BREAKPOINT_UNBOUND) || (_breakpointType == VSP_BREAKPOINT_OPENCL_C_CODE_ERROR) ||
              ((_breakpointType == VSP_BREAKPOINT_OPENCL_C_CODE) && (_openCLProgramBoundLineNumber > -1)));

    _breakpointType = VSP_BREAKPOINT_OPENCL_C_CODE;
    _openCLProgramHandle = openCLProgramHandle;
    _openCLProgramLineNumber = openCLProgramLineNumber;
    GT_ASSERT(_openCLProgramHandle != OA_CL_NULL_HANDLE);
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpoint::setOpenCLCCodeBreakpointBinding
// Description: Sets the location where a kernel breakpoint was bound
// Author:      Uri Shomroni
// Date:        8/5/2011
// ---------------------------------------------------------------------------
void vspCDebugBreakpoint::setOpenCLCCodeBreakpointBinding(int openCLProgramBoundLineNumber)
{
    GT_ASSERT(_breakpointType == VSP_BREAKPOINT_OPENCL_C_CODE);

    _openCLProgramBoundLineNumber = openCLProgramBoundLineNumber;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpoint::setOpenCLCCodeErrorBreakpoint
// Description: Called by the breakpoints manager to let this breakpoint know its type
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
void vspCDebugBreakpoint::setOpenCLCCodeErrorBreakpoint()
{
    // Make sure we didn't set the type twice:
    GT_ASSERT((_breakpointType == VSP_BREAKPOINT_UNBOUND) || (_breakpointType == VSP_BREAKPOINT_OPENCL_C_CODE_ERROR));

    _breakpointType = VSP_BREAKPOINT_OPENCL_C_CODE_ERROR;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpoint::setUnknownBreakpoint
// Description: Called by the breakpoints manager to let this breakpoint know its type
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
void vspCDebugBreakpoint::setUnknownBreakpoint()
{
    // Make sure we didn't set the type twice:
    GT_ASSERT(_breakpointType == VSP_BREAKPOINT_UNBOUND);

    _breakpointType = VSP_BREAKPOINT_TYPE_UNKNOWN;
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpoint::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        29/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugBreakpoint::AddRef(void)
{
    return vspCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpoint::Release
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        29/9/2010
// ---------------------------------------------------------------------------
ULONG vspCDebugBreakpoint::Release(void)
{
    return vspCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpoint::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        29/9/2010
// ---------------------------------------------------------------------------
HRESULT vspCDebugBreakpoint::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
    HRESULT retVal = S_OK;

    if (ppvObj == NULL)
    {
        retVal = E_POINTER;
    }
    else if (riid == IID_IUnknown)
    {
        // Since all three IDebug*Breakpoint2inherit IUnknown, we need to cast through one of them.
        // Note that we have to cast through the same one each time, to be consistent.
        *ppvObj = (IUnknown*)((IDebugPendingBreakpoint2*)this);
        AddRef();
    }
    else if (riid == IID_IDebugPendingBreakpoint2)
    {
        *ppvObj = (IDebugPendingBreakpoint2*)this;
        AddRef();
    }
    else if (riid == IID_IDebugBoundBreakpoint2)
    {
        if (_breakpointStatus == VSP_BREAKPOINT_BOUND)
        {
            *ppvObj = (IDebugBoundBreakpoint2*)this;
            AddRef();
        }
        else // _breakpointStatus != VSP_BREAKPOINT_BOUND
        {
            retVal = E_NOINTERFACE;
        }
    }
    else if (riid == IID_IDebugErrorBreakpoint2)
    {
        if ((_breakpointStatus == VSP_BREAKPOINT_ERROR) || (_breakpointStatus == VSP_BREAKPOINT_QUERY_ERROR))
        {
            *ppvObj = (IDebugErrorBreakpoint2*)this;
            AddRef();
        }
        else // _breakpointStatus != VSP_BREAKPOINT_BOUND
        {
            retVal = E_NOINTERFACE;
        }
    }
    else // riid != IID_IUnknown, IID_IDebugPendingBreakpoint2, IID_IDebugBoundBreakpoint2, IID_IDebugErrorBreakpoint2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
// IDebugPendingBreakpoint2 methods
HRESULT vspCDebugBreakpoint::CanBind(IEnumDebugErrorBreakpoints2** ppErrorEnum)
{
    // Run through the binding operation to see if the breakpoint request is valid and supported:
    HRESULT retVal = vscBreakpointsManager::instance().bindBreakpoint(*this, false);

    if (retVal != S_OK)
    {
        // Any error in binding the breakpoint means it cannot be bound - return false:
        retVal = S_FALSE;
    }

    // If the caller wants the error breakpoints list:
    if (ppErrorEnum != NULL)
    {
        if (retVal == S_OK)
        {
            // We succeeded, make sure the pointer is initialized (if you don't, it causes crashes in VS):
            *ppErrorEnum = NULL;
        }
        else // retVal != S_OK
        {
            // We failed, return this breakpoint as the error breakpoint:
            vspCEnumDebugErrorBreakpoints* pEnumDebugErrorBreakpoint = new vspCEnumDebugErrorBreakpoints(this);
            *ppErrorEnum = pEnumDebugErrorBreakpoint;
        }
    }

    return retVal;
}

HRESULT vspCDebugBreakpoint::GetState(PENDING_BP_STATE_INFO* pState)
{
    GT_UNREFERENCED_PARAMETER(pState);

    return E_NOTIMPL;
}

HRESULT vspCDebugBreakpoint::Virtualize(BOOL fVirtualize)
{
    GT_UNREFERENCED_PARAMETER(fVirtualize);

    return S_OK;
}

HRESULT vspCDebugBreakpoint::Enable(BOOL fEnable)
{
    // Set the member:
    _isEnabled = (fEnable == TRUE);

    // If we're already bound, we need to enable / disable the breakpoint:
    vscBreakpointsManager::instance().setBreakpointEnableStatus(*this);

    return S_OK;
}

HRESULT vspCDebugBreakpoint::SetCondition(BP_CONDITION bpCondition)
{
    GT_UNREFERENCED_PARAMETER(bpCondition);

    return E_NOTIMPL;
}

HRESULT vspCDebugBreakpoint::SetPassCount(BP_PASSCOUNT bpPassCount)
{
    GT_UNREFERENCED_PARAMETER(bpPassCount);

    return E_NOTIMPL;
}

HRESULT vspCDebugBreakpoint::EnumBoundBreakpoints(IEnumDebugBoundBreakpoints2** ppEnum)
{
    HRESULT retVal = S_OK;

    if (ppEnum != NULL)
    {
        // This breakpoint should be returned here only if it is bound
        bool breakpointMatchesRequest = (_breakpointStatus == VSP_BREAKPOINT_BOUND);

        // Enum this breakpoint as a bound breakpoint or return an empty enum if isn't bound:
        vspCEnumDebugBoundBreakpoints* pEnumDebugBoundBreakpoint = new vspCEnumDebugBoundBreakpoints(breakpointMatchesRequest ? this : NULL);
        *ppEnum = pEnumDebugBoundBreakpoint;
    }
    else // ppEnum == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vspCDebugBreakpoint::EnumErrorBreakpoints(BP_ERROR_TYPE bpErrorType,
                                                  IEnumDebugErrorBreakpoints2** ppEnum)
{
    HRESULT retVal = S_OK;

    if (ppEnum != NULL)
    {
        // we can return something if breakpoint is in error mode and it has at least one type and severity that match the request:
        BP_ERROR_TYPE typeMatchingRequest = bpErrorType & _errorType;
        bool breakpointMatchesRequest = ((_breakpointStatus == VSP_BREAKPOINT_ERROR) &&
                                         ((typeMatchingRequest & BPET_TYPE_MASK) != 0) &&
                                         ((typeMatchingRequest & BPET_SEV_MASK) != 0));

        // Enum this breakpoint as an error breakpoint or return an empty enum if it doesn't match:
        vspCEnumDebugErrorBreakpoints* pEnumDebugErrorBreakpoint = new vspCEnumDebugErrorBreakpoints(breakpointMatchesRequest ? this : NULL);
        *ppEnum = pEnumDebugErrorBreakpoint;
    }
    else // ppEnum == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vspCDebugBreakpoint::Delete(void)
{
    HRESULT retVal = S_OK;

    _bindCount--;

    if (_bindCount <= 0)
    {
        // Mark the breakpoint was deleted:
        _wasDeleted = true;

        // Remove the breakpoint implementation:
        vscBreakpointsManager::instance().deleteBreakpoint(*this);
    }

    return retVal;
}

HRESULT vspCDebugBreakpoint::GetBreakpointRequest(IDebugBreakpointRequest2** ppBPRequest)
{
    HRESULT retVal = S_OK;

    if (ppBPRequest != NULL)
    {
        // Return the request:
        *ppBPRequest = _piBreakpointRequest;

        if (_piBreakpointRequest != NULL)
        {
            _piBreakpointRequest->AddRef();
        }
        else // _piBreakpointRequest == NULL
        {
            retVal = E_FAIL;
        }
    }
    else // ppBPRequest == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

HRESULT vspCDebugBreakpoint::Bind(void)
{
    HRESULT retVal = vscBreakpointsManager::instance().bindBreakpoint(*this, true);

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
// IDebugBoundBreakpoint2 methods
HRESULT vspCDebugBreakpoint::GetState(BP_STATE* pState)
{
    HRESULT retVal = S_OK;

    if (pState != NULL)
    {
        // Get the bound breakpoint state:
        BP_STATE breakpointState = BPS_NONE;

        if (_breakpointStatus == VSP_BREAKPOINT_BOUND)
        {
            if (_wasDeleted)
            {
                breakpointState = BPS_DELETED;
            }
            else if (_isEnabled)
            {
                breakpointState = BPS_ENABLED;
            }
            else // !_wasDeleted && !_isEnabled
            {
                breakpointState = BPS_DISABLED;
            }
        }

        // Return the state:
        *pState = breakpointState;
    }
    else // pState == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vspCDebugBreakpoint::GetHitCount(DWORD* pdwHitCount)
{
    HRESULT retVal = S_OK;

    if (pdwHitCount != NULL)
    {
        *pdwHitCount = _hitCount;
    }
    else // pdwHitCount == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
/*HRESULT vspCDebugBreakpoint::Enable(BOOL fEnable)
{ return E_NOTIMPL; }*/
HRESULT vspCDebugBreakpoint::SetHitCount(DWORD dwHitCount)
{
    HRESULT retVal = S_OK;

    _hitCount = dwHitCount;

    return retVal;
}
/*HRESULT vspCDebugBreakpoint::SetCondition(BP_CONDITION bpCondition)
{ return E_NOTIMPL; }
HRESULT vspCDebugBreakpoint::SetPassCount(BP_PASSCOUNT bpPassCount)
{ return E_NOTIMPL; }
HRESULT vspCDebugBreakpoint::Delete(void)
{ return E_NOTIMPL; }*/

HRESULT vspCDebugBreakpoint::GetPendingBreakpoint(IDebugPendingBreakpoint2** ppPendingBreakpoint)
{
    HRESULT retVal = S_OK;

    if (ppPendingBreakpoint != NULL)
    {
        // Return the pending breakpoint interface and retain this object:
        *ppPendingBreakpoint = this;
        AddRef();
    }
    else // ppPendingBreakpoint == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

HRESULT vspCDebugBreakpoint::GetBreakpointResolution(IDebugBreakpointResolution2** ppBPResolution)
{
    HRESULT retVal = S_OK;

    if (ppBPResolution != NULL)
    {
        // Return our IDebugBreakpointResolution2 interface:
        *ppBPResolution = this;
        AddRef();
    }
    else // ppBPResolution == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
////////////////////////////////////////////////////////////
// IDebugBreakpointResolution2 methods
HRESULT vspCDebugBreakpoint::GetBreakpointType(BP_TYPE* pBPType)
{
    HRESULT retVal = S_OK;

    if (pBPType != NULL)
    {
        if (_piBreakpointRequest != NULL)
        {
            // Get the type from the request used to create this breakpoint:
            BP_LOCATION_TYPE locationType;
            retVal = _piBreakpointRequest->GetLocationType(&locationType);

            if (S_OK == retVal)
            {
                *pBPType = locationType & BPLT_TYPE_MASK;
            }

            if (retVal != S_OK)
            {
                retVal = E_FAIL;
            }
        }
        else
        {
            // We don't have a request, so we don't know the breakpoint type:
            retVal = E_FAIL;
        }
    }
    else // pBPType == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vspCDebugBreakpoint::GetResolutionInfo(BPRESI_FIELDS dwFields, BP_RESOLUTION_INFO* pBPResolutionInfo)
{
    HRESULT retVal = S_OK;

    if (pBPResolutionInfo != NULL)
    {
        // Clear the struct:
        ::memset(pBPResolutionInfo, 0, sizeof(BP_RESOLUTION_INFO));

        BPRESI_FIELDS& resolutionFields = pBPResolutionInfo->dwFields;

        if ((dwFields & BPRESI_BPRESLOCATION) != 0)
        {
            // The breakpoint type is the request location type without the exact details:
            BP_RESOLUTION_LOCATION& bpResolutionLocation =  pBPResolutionInfo->bpResLocation;
            HRESULT hrTyp = _piBreakpointRequest->GetLocationType(&(bpResolutionLocation.bpType));

            if (SUCCEEDED(hrTyp))
            {
                bpResolutionLocation.bpType &= BPLT_TYPE_MASK;

                if (bpResolutionLocation.bpType == BPT_CODE)
                {
                    gtString emptyString;
                    osFilePath nullPath;

                    if (VSP_BREAKPOINT_OPENCL_C_CODE == _breakpointType)
                    {
                        int lineNumber = _openCLProgramBoundLineNumber - 1;

                        if (lineNumber < 0)
                        {
                            lineNumber = _openCLProgramLineNumber - 1;
                        }

                        bpResolutionLocation.bpResLocation.bpresCode.pCodeContext = new vspCDebugContext(nullPath, emptyString, _requestedFilePath, lineNumber, NULL, NULL, true, OS_NO_THREAD_ID, 0, _pDebugEngine);
                    }
                    else if (VSP_BREAKPOINT_C_CPP_CODE == _breakpointType)
                    {
                        bpResolutionLocation.bpResLocation.bpresCode.pCodeContext = new vspCDebugContext(nullPath, emptyString, m_hostSourcePath, m_hostLineNumber, NULL, NULL, true, OS_NO_THREAD_ID, 0, _pDebugEngine);
                    }
                }
                else

                    // We currently can only supply information about the location type
                    // TO_DO: We should normally consider the type to fill out bpResolutionLocation.bpResLocation
                {
                    resolutionFields |= BPRESI_BPRESLOCATION;
                }
            }
            else // FAILED(hrTyp)
            {
                bpResolutionLocation.bpType = 0;
            }
        }

        if ((dwFields & BPRESI_PROGRAM) != 0)
        {
            // Pass our debug engine as an IDebugProgram2:
            pBPResolutionInfo->pProgram = _pDebugEngine;
            _pDebugEngine->AddRef();

            resolutionFields |= BPRESI_PROGRAM;
        }

        if ((dwFields & BPRESI_THREAD) != 0)
        {
            // A breakpoint binding does not happen on a specific thread as far as we know:
            pBPResolutionInfo->pThread = NULL;

            // Uri, 5/10/10 - Setting this flag causes the breakpoint window to experience a problem, hiding all the
            // breakpoints. Since we don't have a value anyway, we don't set it:
            // resolutionFields |= BPRESI_THREAD;
        }
    }
    else // pBPResolutionInfo == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
// IDebugErrorBreakpoint2 methods
/*HRESULT vspCDebugBreakpoint::GetPendingBreakpoint(IDebugPendingBreakpoint2** ppPendingBreakpoint)
{ return E_NOTIMPL; }*/
HRESULT vspCDebugBreakpoint::GetBreakpointResolution(IDebugErrorBreakpointResolution2** ppErrorResolution)
{
    HRESULT retVal = S_OK;

    if (ppErrorResolution != NULL)
    {
        // Return our IDebugErrorBreakpointResolution2 interface:
        *ppErrorResolution = this;
        AddRef();
    }
    else // ppErrorResolution == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

////////////////////////////////////////////////////////////
// IDebugErrorBreakpointResolution2 methods
/*HRESULT vspCDebugBreakpoint::GetBreakpointType(BP_TYPE *pBPType)
{ return E_NOTIMPL }*/
HRESULT vspCDebugBreakpoint::GetResolutionInfo(BPERESI_FIELDS dwFields, BP_ERROR_RESOLUTION_INFO* pErrorResolutionInfo)
{
    HRESULT retVal = S_OK;

    if (pErrorResolutionInfo != NULL)
    {
        // Clear the struct:
        ::memset(pErrorResolutionInfo, 0, sizeof(BP_ERROR_RESOLUTION_INFO));

        BPERESI_FIELDS& resolutionFields = pErrorResolutionInfo->dwFields;

        if ((dwFields & BPERESI_BPRESLOCATION) != 0)
        {
            // The breakpoint type is the request location type without the exact details:
            BP_RESOLUTION_LOCATION& bpResolutionLocation =  pErrorResolutionInfo->bpResLocation;
            HRESULT hrTyp = _piBreakpointRequest->GetLocationType(&(bpResolutionLocation.bpType));

            if (SUCCEEDED(hrTyp))
            {
                bpResolutionLocation.bpType &= BPLT_TYPE_MASK;

                // We currently can only supply information about the location type
                // TO_DO: We should normally consider the type to fill out bpResolutionLocation.bpResLocation
                resolutionFields |= BPERESI_BPRESLOCATION;
            }
            else // FAILED(hrTyp)
            {
                bpResolutionLocation.bpType = 0;
            }
        }

        if ((dwFields & BPERESI_PROGRAM) != 0)
        {
            // Pass our debug engine as an IDebugProgram2:
            pErrorResolutionInfo->pProgram = _pDebugEngine;
            _pDebugEngine->AddRef();

            resolutionFields |= BPERESI_PROGRAM;
        }

        if ((dwFields & BPERESI_THREAD) != 0)
        {
            // A breakpoint error does not happen on a specific thread as far as we know:
            pErrorResolutionInfo->pThread = NULL;

            // Uri, 5/10/10 - Setting this flag causes the breakpoint window to experience a problem, hiding all the
            // breakpoints. Since we don't have a value anyway, we don't set it:
            // resolutionFields |= BPERESI_THREAD;
        }

        if ((dwFields & BPERESI_MESSAGE) != 0)
        {
            pErrorResolutionInfo->bstrMessage = SysAllocString(_errorMessage.asCharArray());

            resolutionFields |= BPERESI_MESSAGE;
        }

        if ((dwFields & BPERESI_TYPE) != 0)
        {
            pErrorResolutionInfo->dwType = _errorType;

            resolutionFields |= BPERESI_TYPE;
        }
    }
    else // pErrorResolutionInfo == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}


// ---------------------------------------------------------------------------
// Name:        vspCDebugBreakpoint::onBreakpointHit
// Description: Is called when the breakpoint hits
// Author:      Sigal Algranaty
// Date:        3/11/2011
// ---------------------------------------------------------------------------
void vspCDebugBreakpoint::onBreakpointHit()
{
    // Increase the breakpoint hit count:
    _hitCount++;

    // Look for the current breakpoint within the breakpoints:
    // Get the amount of breakpoints:
    int amountOfBreakpoints = 0;
    bool rc = gaGetAmountOfBreakpoints(amountOfBreakpoints);
    GT_IF_WITH_ASSERT(rc)
    {
        for (int i = 0 ; i < amountOfBreakpoints ; i++)
        {
            // Get the current breakpoint
            gtAutoPtr<apBreakPoint> aptrBreakpoint;
            rc = gaGetBreakpoint(i, aptrBreakpoint);
            GT_IF_WITH_ASSERT(rc)
            {
                // Get the breakpoint type:
                osTransferableObjectType breakpointType = aptrBreakpoint->type();

                if ((_breakpointType == VSP_BREAKPOINT_MONITORED_FUNCTION) && (breakpointType == OS_TOBJ_ID_MONITORED_FUNC_BREAKPOINT))
                {
                    // Down cast it to apMonitoredFunctionBreakPoint:
                    apMonitoredFunctionBreakPoint* pFunctionBreakpoint = (apMonitoredFunctionBreakPoint*)(aptrBreakpoint.pointedObject());
                    GT_IF_WITH_ASSERT(pFunctionBreakpoint != NULL)
                    {
                        if (pFunctionBreakpoint->monitoredFunctionId() == _monitoredFunctionId)
                        {
                            // Set the API breakpoint hit count:
                            gaGRApiFunctions::instance().gaSetBreakpointHitCount(i, _hitCount);
                            break;
                        }
                    }
                }

                else if ((_breakpointType == VSP_BREAKPOINT_GENERIC) && (breakpointType == OS_TOBJ_ID_GENERIC_BREAKPOINT))
                {
                    // Down cast it to apGenericBreakpoint:
                    apGenericBreakpoint* pGenericBreakpoint = (apGenericBreakpoint*)(aptrBreakpoint.pointedObject());
                    GT_IF_WITH_ASSERT(pGenericBreakpoint != NULL)
                    {
                        // Check if this API breakpoint represents the VSP breakpoint object:
                        if (pGenericBreakpoint->breakpointType() == _genericBreakpointType)
                        {
                            // Set the API breakpoint hit count:
                            gaGRApiFunctions::instance().gaSetBreakpointHitCount(i, _hitCount);
                            break;
                        }
                    }
                }

                else if ((_breakpointType == VSP_BREAKPOINT_KERNEL_FUNCTION_NAME) && (breakpointType == OS_TOBJ_ID_KERNEL_FUNC_NAME_BREAKPOINT))
                {
                    // Down cast it to apGenericBreakpoint:
                    apKernelFunctionNameBreakpoint* pKernelFunctionNameBreakpoint = (apKernelFunctionNameBreakpoint*)(aptrBreakpoint.pointedObject());
                    GT_IF_WITH_ASSERT(pKernelFunctionNameBreakpoint != NULL)
                    {
                        if (pKernelFunctionNameBreakpoint->kernelFunctionName() == _kernelFunctionName)
                        {
                            // Set the API breakpoint hit count:
                            gaGRApiFunctions::instance().gaSetBreakpointHitCount(i, _hitCount);
                            break;
                        }
                    }
                }
            }
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// vspCEnumDebugBoundBreakpoints
//////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugBoundBreakpoints::vspCEnumDebugBoundBreakpoints
// Description: Constructor
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
vspCEnumDebugBoundBreakpoints::vspCEnumDebugBoundBreakpoints(vspCDebugBreakpoint* pBreakpoint)
    : _enumBreakpoint(pBreakpoint), _wasEnumeratorUsed(false)
{
    if (_enumBreakpoint != NULL)
    {
        _enumBreakpoint->AddRef();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugBoundBreakpoints::~vspCEnumDebugBoundBreakpoints
// Description: Destructor
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
vspCEnumDebugBoundBreakpoints::~vspCEnumDebugBoundBreakpoints()
{
    if (_enumBreakpoint != NULL)
    {
        _enumBreakpoint->Release();
        _enumBreakpoint = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugBoundBreakpoints::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
ULONG vspCEnumDebugBoundBreakpoints::AddRef(void)
{
    return vspCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugBoundBreakpoints::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
ULONG vspCEnumDebugBoundBreakpoints::Release(void)
{
    return vspCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugBoundBreakpoints::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        5/10/2010
// ---------------------------------------------------------------------------
HRESULT vspCEnumDebugBoundBreakpoints::QueryInterface(REFIID riid, LPVOID* ppvObj)
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
    else if (riid == IID_IEnumDebugBoundBreakpoints2)
    {
        *ppvObj = (IEnumDebugBoundBreakpoints2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IEnumDebugBoundBreakpoints2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
// IEnumDebugBoundBreakpoints2 methods
HRESULT vspCEnumDebugBoundBreakpoints::Next(ULONG celt, IDebugBoundBreakpoint2** rgelt, ULONG* pceltFetched)
{
    HRESULT retVal = S_OK;

    // Will get the amount of breakpoints (1 or 0) we successfully returned:
    ULONG fetchedItems = 0;

    if (rgelt != NULL)
    {
        ULONG canReturnItems = (_wasEnumeratorUsed || (_enumBreakpoint == NULL)) ? 0 : 1;

        // If the caller requested more items than we can supply, return S_FALSE.
        if (celt > canReturnItems)
        {
            retVal = S_FALSE;
        }

        // If we can return the breakpoint and the user requested it, set it into rgelt:
        if ((celt > 0) && (!_wasEnumeratorUsed) && (_enumBreakpoint != NULL))
        {
            rgelt[0] = _enumBreakpoint;
            _enumBreakpoint->AddRef();
            _wasEnumeratorUsed = true;
            fetchedItems = 1;
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
HRESULT vspCEnumDebugBoundBreakpoints::Skip(ULONG celt)
{
    HRESULT retVal = S_OK;

    ULONG canReturnItems = (_wasEnumeratorUsed || (_enumBreakpoint == NULL)) ? 0 : 1;

    // If the caller requested more items than we can supply, return S_FALSE.
    if (celt > canReturnItems)
    {
        retVal = S_FALSE;
    }

    // If the enumerator was not yet used and the user requested it, mark it as used:
    if (celt > 0)
    {
        _wasEnumeratorUsed = true;
    }

    return retVal;
}
HRESULT vspCEnumDebugBoundBreakpoints::Reset(void)
{
    HRESULT retVal = S_OK;

    // Mark the enumerator as new:
    _wasEnumeratorUsed = false;

    return retVal;
}
HRESULT vspCEnumDebugBoundBreakpoints::Clone(IEnumDebugBoundBreakpoints2** ppEnum)
{
    HRESULT retVal = S_OK;

    if (ppEnum != NULL)
    {
        // Create a duplicate of this item (note that this will increment the breakpoint's reference count:
        vspCEnumDebugBoundBreakpoints* pClone = new vspCEnumDebugBoundBreakpoints(_enumBreakpoint);

        // Set its used status to equal ours:
        pClone->_wasEnumeratorUsed = _wasEnumeratorUsed;

        // Return it:
        *ppEnum = (IEnumDebugBoundBreakpoints2*)pClone;
    }
    else // ppEnum == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vspCEnumDebugBoundBreakpoints::GetCount(ULONG* pcelt)
{
    HRESULT retVal = S_OK;

    if (pcelt != NULL)
    {
        // Return the count:
        *pcelt = (_enumBreakpoint != NULL) ? 1 : 0;
    }
    else
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
// vspCEnumDebugErrorBreakpoints
//////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugErrorBreakpoints::vspCEnumDebugErrorBreakpoints
// Description: Constructor
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
vspCEnumDebugErrorBreakpoints::vspCEnumDebugErrorBreakpoints(vspCDebugBreakpoint* pBreakpoint)
    : _enumBreakpoint(pBreakpoint), _wasEnumeratorUsed(false)
{
    if (_enumBreakpoint != NULL)
    {
        _enumBreakpoint->AddRef();
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugErrorBreakpoints::~vspCEnumDebugErrorBreakpoints
// Description: Destructor
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
vspCEnumDebugErrorBreakpoints::~vspCEnumDebugErrorBreakpoints()
{
    if (_enumBreakpoint != NULL)
    {
        _enumBreakpoint->Release();
        _enumBreakpoint = NULL;
    }
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugErrorBreakpoints::AddRef
// Description: Adds 1 to the reference count and returns the new value
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
ULONG vspCEnumDebugErrorBreakpoints::AddRef(void)
{
    return vspCUnknown::addRef();
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugErrorBreakpoints::AddRef
// Description: Reduces the reference count by 1 and returns the new value. If
//              the new reference count is 0, also destroys the object.
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
ULONG vspCEnumDebugErrorBreakpoints::Release(void)
{
    return vspCUnknown::release();
}

// ---------------------------------------------------------------------------
// Name:        vspCEnumDebugErrorBreakpoints::QueryInterface
// Description: if riid is an interface ID implemented by this class, sets ppvObj
//              to it, adds to the reference count and returns S_OK. Otherwise,
//              returns E_NOINTERFACE.
// Author:      Uri Shomroni
// Date:        4/10/2010
// ---------------------------------------------------------------------------
HRESULT vspCEnumDebugErrorBreakpoints::QueryInterface(REFIID riid, LPVOID* ppvObj)
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
    else if (riid == IID_IEnumDebugErrorBreakpoints2)
    {
        *ppvObj = (IEnumDebugErrorBreakpoints2*)this;
        AddRef();
    }
    else // riid != IID_IUnknown, IID_IEnumDebugErrorBreakpoints2
    {
        retVal = E_NOINTERFACE;
    }

    return retVal;
}

//////////////////////////////////////////////////////////////////////////////
// IEnumDebugErrorBreakpoints2 methods
HRESULT vspCEnumDebugErrorBreakpoints::Next(ULONG celt, IDebugErrorBreakpoint2** rgelt, ULONG* pceltFetched)
{
    HRESULT retVal = S_OK;

    // Will get the amount of breakpoints (1 or 0) we successfully returned:
    ULONG fetchedItems = 0;

    if (rgelt != NULL)
    {
        ULONG canReturnItems = (_wasEnumeratorUsed || (_enumBreakpoint == NULL)) ? 0 : 1;

        // If the caller requested more items than we can supply, return S_FALSE.
        if (celt > canReturnItems)
        {
            retVal = S_FALSE;
        }

        // If we can return the breakpoint and the user requested it, set it into rgelt:
        if ((celt > 0) && (!_wasEnumeratorUsed) && (_enumBreakpoint != NULL))
        {
            rgelt[0] = _enumBreakpoint;
            _enumBreakpoint->AddRef();
            _wasEnumeratorUsed = true;
            fetchedItems = 1;
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
HRESULT vspCEnumDebugErrorBreakpoints::Skip(ULONG celt)
{
    HRESULT retVal = S_OK;

    ULONG canReturnItems = (_wasEnumeratorUsed || (_enumBreakpoint == NULL)) ? 0 : 1;

    // If the caller requested more items than we can supply, return S_FALSE.
    if (celt > canReturnItems)
    {
        retVal = S_FALSE;
    }

    // If the enumerator was not yet used and the user requested it, mark it as used:
    if (celt > 0)
    {
        _wasEnumeratorUsed = true;
    }

    return retVal;
}
HRESULT vspCEnumDebugErrorBreakpoints::Reset(void)
{
    HRESULT retVal = S_OK;

    // Mark the enumerator as new:
    _wasEnumeratorUsed = false;

    return retVal;
}
HRESULT vspCEnumDebugErrorBreakpoints::Clone(IEnumDebugErrorBreakpoints2** ppEnum)
{
    HRESULT retVal = S_OK;

    if (ppEnum != NULL)
    {
        // Create a duplicate of this item (note that this will increment the breakpoint's reference count:
        vspCEnumDebugErrorBreakpoints* pClone = new vspCEnumDebugErrorBreakpoints(_enumBreakpoint);

        // Set its used status to equal ours:
        pClone->_wasEnumeratorUsed = _wasEnumeratorUsed;

        // Return it:
        *ppEnum = (IEnumDebugErrorBreakpoints2*)pClone;
    }
    else // ppEnum == NULL
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}
HRESULT vspCEnumDebugErrorBreakpoints::GetCount(ULONG* pcelt)
{
    HRESULT retVal = S_OK;

    if (pcelt != NULL)
    {
        // Return the count:
        *pcelt = (_enumBreakpoint != NULL) ? 1 : 0;
    }
    else
    {
        // Invalid pointer:
        retVal = E_POINTER;
    }

    return retVal;
}

