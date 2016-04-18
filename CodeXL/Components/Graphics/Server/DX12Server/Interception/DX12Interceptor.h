//=============================================================================
/// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file   DX12Interceptor.h
/// \brief  The DX12Interceptor contains the mechanisms responsible for
///         instrumenting DX12 objects and function calls through hooking.
//=============================================================================

#ifndef DX12INTERCEPTOR_H
#define DX12INTERCEPTOR_H

#include "DX12Defines.h"
#include "../Tracing/DX12TraceAnalyzerLayer.h"
#include "../../Common/Tracing/APIEntry.h"

//-----------------------------------------------------------------------------
/// The DX12Interceptor contains the mechanisms responsible for instrumenting
/// DX12 objects and function calls through hooking.
//-----------------------------------------------------------------------------
class DX12Interceptor
{
public:
    //-----------------------------------------------------------------------------
    /// DX12Interceptor constructor.
    //-----------------------------------------------------------------------------
    DX12Interceptor() {}

    //-----------------------------------------------------------------------------
    /// DX12Interceptor destructor.
    //-----------------------------------------------------------------------------
    virtual ~DX12Interceptor() {}

    //-----------------------------------------------------------------------------
    /// Retrieve a pointer to the LayerManager that owns this InterceptorBase instance.
    /// \returns A pointer to the DX12LayerManager.
    //-----------------------------------------------------------------------------
    virtual ModernAPILayerManager* GetParentLayerManager();

    //-----------------------------------------------------------------------------
    /// Shutdown the DX12Interceptor.
    /// \returns True if shutdown was successful. False if it failed.
    //-----------------------------------------------------------------------------
    bool Shutdown() { return true; }

    //-----------------------------------------------------------------------------
    /// Handler used before the real runtime implementation of an API call has been invoked.
    /// \param inWrappedInterface An instance of a wrapped DX12 interface.
    /// \param inFunctionId The function ID for the call being traced.
    /// \param inNumParameters The number of parameters for this API call.
    /// \param pParameters pointer to an array of parameter attributes.
    /// \returns Pointer to an APIEntry structure.
    //-----------------------------------------------------------------------------
    DX12APIEntry* PreCall(IUnknown* inWrappedInterface, FuncId inFunctionId, int inNumParameters = 0, ParameterEntry* pParameters = nullptr);

    //-----------------------------------------------------------------------------
    /// Responsible for the post-call instrumentation of every DX12 API call.
    /// \param pNewEntry Pointer to an APIEntry structure returned from PreCall()
    /// \param inReturnValue The return value for the function. If void, use FUNCTION_RETURNS_VOID.
    /// \param inReturnValueFlags A flag indicating how the return value should be displayed
    //-----------------------------------------------------------------------------
    void PostCall(DX12APIEntry* pNewEntry, INT64 inReturnValue = FUNCTION_RETURNS_VOID, ReturnDisplayType inReturnValueFlags = RETURN_VALUE_DECIMAL);

    //-----------------------------------------------------------------------------
    /// Should we trace each API call?
    /// \returns True if each API should be traced.
    //-----------------------------------------------------------------------------
    bool ShouldCollectTrace() { return DX12TraceAnalyzerLayer::Instance()->ShouldCollectTrace(); }
};

#endif // DX12INTERCEPTOR_H
