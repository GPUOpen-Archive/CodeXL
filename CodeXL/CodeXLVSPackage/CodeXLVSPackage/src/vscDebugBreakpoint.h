//==================================================================================
// Copyright (c) 2016 , Advanced Micro Devices, Inc.  All rights reserved.
//
/// \author AMD Developer Tools Team
/// \file vscDebugBreakpoint.h
///
//==================================================================================

//------------------------------ vspDebugBreakpoint.h ------------------------------

#ifndef __VSPDEBUGBREAKPOINT_H
#define __VSPDEBUGBREAKPOINT_H

// Forward declarations:
class vspCDebugEngine;

// Infra:
#include <AMDTBaseTools/Include/gtString.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSAPIWrappers/Include/oaOSAPIDefinitions.h>
#include <AMDTAPIClasses/Include/apMonitoredFunctionId.h>
#include <AMDTAPIClasses/Include/apGenericBreakpoint.h>

// Local:
#include <src/vspUnknown.h>


// ----------------------------------------------------------------------------------
// Class Name:           vspCDebugBreakpoint : public IDebugPendingBreakpoint2, IDebugBoundBreakpoint2, IDebugErrorBreakpoint2, vspCUnknown
// General Description: Describes a breakpoint, regardless of its status. Functions that require getting from one type to another
//                      will get this same class cast into the various interfaces
// Author:               Uri Shomroni
// Creation Date:        29/9/2010
// ----------------------------------------------------------------------------------
class vspCDebugBreakpoint : public IDebugPendingBreakpoint2, public IDebugBoundBreakpoint2, public IDebugBreakpointResolution2, public IDebugErrorBreakpoint2, public IDebugErrorBreakpointResolution2, vspCUnknown
{
public:
    enum vspBreakpointStatus
    {
        VSP_BREAKPOINT_PENDING,     // This breakpoint was not yet bound
        VSP_BREAKPOINT_BOUND,       // This breakpoint was bound successfully
        VSP_BREAKPOINT_QUERY_ERROR, // This breakpoint has an error, but no bind attempt was made on it (only canBind queries)
        VSP_BREAKPOINT_ERROR,       // This breakpoint has an error and a bind attempt was made on it
    };

    enum vspBreakpointType
    {
        VSP_BREAKPOINT_UNBOUND,                 // Unbound breakpoint
        VSP_BREAKPOINT_MONITORED_FUNCTION,      // This is a monitored function breakpoint
        VSP_BREAKPOINT_KERNEL_FUNCTION_NAME,    // This is a kernel function name breakpoint
        VSP_BREAKPOINT_GENERIC,                 // This is a generic breakpoint
        VSP_BREAKPOINT_NON_MONITORED_FUNCTION,  // This is a C/C++ function breakpoint on a non-monitored function
        VSP_BREAKPOINT_C_CPP_CODE,              // This is a C/C++ file + line code breakpoint
        VSP_BREAKPOINT_C_CPP_CODE_ERROR,        // This is a C/C++ file + line code breakpoint that cannot be bound yet
        VSP_BREAKPOINT_OPENCL_C_CODE,           // This is an OpenCL C program + line breakpoint
        VSP_BREAKPOINT_OPENCL_C_CODE_ERROR,     // This is an OpenCL C program + line breakpoint that cannot be bound yet
        VSP_BREAKPOINT_TYPE_UNKNOWN,            // Unknown / unsupported breakpoint
    };

public:
    vspCDebugBreakpoint(vspCDebugEngine* pDebugEngine, IDebugBreakpointRequest2* piBreakpointRequest, int hitCount);
    virtual ~vspCDebugBreakpoint();

    void bind();
    void setError(BP_ERROR_TYPE errorType, const gtString& errorMessage, bool wasBindAttempted);

    // Accessors:
    vspBreakpointStatus breakpointStatus() const {return _breakpointStatus;};
    IDebugBreakpointRequest2* breakpointRequest() const {return _piBreakpointRequest;};
    bool wasDeleted() const {return _wasDeleted;};
    bool isEnabled() const {return _isEnabled;};
    vspBreakpointType breakpointType() const {return _breakpointType;};
    apMonitoredFunctionId breakpointFunctionId() const {return (_breakpointType == VSP_BREAKPOINT_MONITORED_FUNCTION) ? _monitoredFunctionId : apMonitoredFunctionsAmount;};
    apGenericBreakpointType genericBreakpointType() const {return _genericBreakpointType;};
    const gtString& breakpointKernelFunctionName() const {return /*(_breakpointType == VSP_BREAKPOINT_KERNEL_FUNCTION_NAME) ?*/ _kernelFunctionName;};
    oaCLProgramHandle openCLProgramHandle() const {return (_breakpointType == VSP_BREAKPOINT_OPENCL_C_CODE) ? _openCLProgramHandle : OA_CL_NULL_HANDLE;};
    int openCLProgramLineNumber() const {return (_breakpointType == VSP_BREAKPOINT_OPENCL_C_CODE) ? _openCLProgramLineNumber : -1;};
    int openCLProgramBoundLineNumber() const {return (_breakpointType == VSP_BREAKPOINT_OPENCL_C_CODE) ? _openCLProgramBoundLineNumber : -1;};
    const osFilePath& hostSourcePath() const { static const osFilePath nullPath; return (VSP_BREAKPOINT_C_CPP_CODE == _breakpointType) ? m_hostSourcePath : nullPath; };
    int hostLineNumber() const { return (VSP_BREAKPOINT_C_CPP_CODE == _breakpointType) ? m_hostLineNumber : -1; };

    // Events:
    void setMonitoredFunctionBreakpoint(apMonitoredFunctionId funcId);
    void setGenericBreakpointType(apGenericBreakpointType breakpointType);
    void setKernelFunctionNameBreakpoint(const gtString& kernelFuncName);
    void setNonMonitoredFunctionBreakpoint();
    void setCCppCodeBreakpoint(const osFilePath& hostSrcPath, int hostLineNum);
    void setCCppCodeErrorBreakpoint();
    void setOpenCLCCodeBreakpoint(oaCLProgramHandle openCLProgramHandle, int openCLProgramLineNumber);
    void setOpenCLCCodeBreakpointBinding(int openCLProgramBoundLineNumber);
    void setOpenCLCCodeErrorBreakpoint();
    void setUnknownBreakpoint();
    void setSourceRequest(const osFilePath& requestedFilePath, int requestedLineNumber) {_requestedFilePath = requestedFilePath; _requestedLineNumber = requestedLineNumber;};
    void onBreakpointHit();

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IDebugPendingBreakpoint2 methods
    STDMETHOD(CanBind)(IEnumDebugErrorBreakpoints2** ppErrorEnum);
    STDMETHOD(Bind)(void);
    STDMETHOD(GetState)(PENDING_BP_STATE_INFO* pState);
    STDMETHOD(GetBreakpointRequest)(IDebugBreakpointRequest2** ppBPRequest);
    STDMETHOD(Virtualize)(BOOL fVirtualize);
    STDMETHOD(Enable)(BOOL fEnable);
    STDMETHOD(SetCondition)(BP_CONDITION bpCondition);
    STDMETHOD(SetPassCount)(BP_PASSCOUNT bpPassCount);
    STDMETHOD(EnumBoundBreakpoints)(IEnumDebugBoundBreakpoints2** ppEnum);
    STDMETHOD(EnumErrorBreakpoints)(
        BP_ERROR_TYPE bpErrorType,
        IEnumDebugErrorBreakpoints2** ppEnum);
    STDMETHOD(Delete)(void);

    ////////////////////////////////////////////////////////////
    // IDebugBoundBreakpoint2 methods
    STDMETHOD(GetPendingBreakpoint)(IDebugPendingBreakpoint2** ppPendingBreakpoint);
    STDMETHOD(GetState)(BP_STATE* pState);
    STDMETHOD(GetHitCount)(DWORD* pdwHitCount);
    STDMETHOD(GetBreakpointResolution)(IDebugBreakpointResolution2** ppBPResolution);
    // STDMETHOD(Enable)(BOOL fEnable); already defined for IDebugPendingBreakpoint2
    STDMETHOD(SetHitCount)(DWORD dwHitCount);
    // STDMETHOD(SetCondition)(BP_CONDITION bpCondition); already defined for IDebugPendingBreakpoint2
    // STDMETHOD(SetPassCount)(BP_PASSCOUNT bpPassCount); already defined for IDebugPendingBreakpoint2
    // STDMETHOD(Delete)(void); already defined for IDebugPendingBreakpoint2

    ////////////////////////////////////////////////////////////
    // IDebugBreakpointResolution2 methods
    STDMETHOD(GetBreakpointType)(BP_TYPE* pBPType);
    STDMETHOD(GetResolutionInfo)(BPRESI_FIELDS dwFields, BP_RESOLUTION_INFO* pBPResolutionInfo);

    ////////////////////////////////////////////////////////////
    // IDebugErrorBreakpoint2 methods
    // STDMETHOD(GetPendingBreakpoint)(IDebugPendingBreakpoint2 **ppPendingBreakpoint); already defined for IDebugBoundBreakpoint2
    STDMETHOD(GetBreakpointResolution)(IDebugErrorBreakpointResolution2** ppErrorResolution);

    ////////////////////////////////////////////////////////////
    // IDebugErrorBreakpointResolution2 methods
    // STDMETHOD(GetBreakpointType)(BP_TYPE *pBPType);already defined for IDebugBreakpointResolution2
    STDMETHOD(GetResolutionInfo)(BPERESI_FIELDS dwFields, BP_ERROR_RESOLUTION_INFO* pErrorResolutionInfo);

private:
    // Do not allow use of my default constructor:
    vspCDebugBreakpoint();

private:
    vspCDebugEngine* _pDebugEngine;
    vspBreakpointStatus _breakpointStatus;
    vspBreakpointType _breakpointType;
    oaCLProgramHandle _openCLProgramHandle;
    int _openCLProgramLineNumber;
    int _openCLProgramBoundLineNumber;
    osFilePath m_hostSourcePath;
    int m_hostLineNumber;
    apMonitoredFunctionId _monitoredFunctionId;
    apGenericBreakpointType _genericBreakpointType;
    gtString _kernelFunctionName;
    IDebugBreakpointRequest2* _piBreakpointRequest;
    BP_ERROR_TYPE _errorType;
    gtString _errorMessage;
    bool _wasDeleted;
    bool _isEnabled;
    int _bindCount;
    DWORD _hitCount;

    // Members used for internal information:
    osFilePath _requestedFilePath;
    int _requestedLineNumber;
};

// ----------------------------------------------------------------------------------
// Class Name:          vspCEnumDebugDebugBoundBreakpoints : public IEnumDebugBoundBreakpoints2
// General Description: Implements IEnumDebugBoundBreakpoints2, Enumerating
//                      the bound breakpoint object for a breakpoint that succeeded to bind.
// Author:               Uri Shomroni
// Creation Date:        4/10/2010
// ----------------------------------------------------------------------------------
class vspCEnumDebugBoundBreakpoints : public IEnumDebugBoundBreakpoints2, vspCUnknown
{
public:
    vspCEnumDebugBoundBreakpoints(vspCDebugBreakpoint* pBreakpoint);
    virtual ~vspCEnumDebugBoundBreakpoints();

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IEnumDebugBoundBreakpoints2 methods
    STDMETHOD(Next)(
        ULONG celt,
        IDebugBoundBreakpoint2** rgelt,
        ULONG* pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumDebugBoundBreakpoints2** ppEnum);
    STDMETHOD(GetCount)(ULONG* pcelt);

private:
    // Do not allow use of my default constructor:
    vspCEnumDebugBoundBreakpoints();

private:
    // The enumerated breakpoint:
    vspCDebugBreakpoint* _enumBreakpoint;

    bool _wasEnumeratorUsed;
};

// ----------------------------------------------------------------------------------
// Class Name:          vspCEnumDebugDebugErrorBreakpoints : public IEnumDebugErrorBreakpoints2
// General Description: Implements IEnumDebugErrorBreakpoints2, Enumerating
//                      the error breakpoint object for a breakpoint that failed to bind.
// Author:               Uri Shomroni
// Creation Date:        4/10/2010
// ----------------------------------------------------------------------------------
class vspCEnumDebugErrorBreakpoints : public IEnumDebugErrorBreakpoints2, vspCUnknown
{
public:
    vspCEnumDebugErrorBreakpoints(vspCDebugBreakpoint* pBreakpoint);
    virtual ~vspCEnumDebugErrorBreakpoints();

    ////////////////////////////////////////////////////////////
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(void);
    STDMETHOD_(ULONG, Release)(void);
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj);

    ////////////////////////////////////////////////////////////
    // IEnumDebugErrorBreakpoints2 methods
    STDMETHOD(Next)(
        ULONG celt,
        IDebugErrorBreakpoint2** rgelt,
        ULONG* pceltFetched);
    STDMETHOD(Skip)(ULONG celt);
    STDMETHOD(Reset)(void);
    STDMETHOD(Clone)(IEnumDebugErrorBreakpoints2** ppEnum);
    STDMETHOD(GetCount)(ULONG* pcelt);

private:
    // Do not allow use of my default constructor:
    vspCEnumDebugErrorBreakpoints();

private:
    // The enumerated breakpoint:
    vspCDebugBreakpoint* _enumBreakpoint;

    bool _wasEnumeratorUsed;
};

#endif //__VSPDEBUGBREAKPOINT_H

